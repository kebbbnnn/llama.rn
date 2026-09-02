#pragma once
#include "JSINativeHeaders.h"
#include <functional>
#include <cstdint>
#include <mutex>
#include <unordered_map>
#include <vector>

namespace rnllama_jsi {
    // Pointers are stored as uintptr_t (not long) so they stay full-width on
    // Windows LLP64, where `long` is 32-bit and would truncate a 64-bit ptr.
    template<typename T>
    class ContextManager {
    private:
        std::unordered_map<int, uintptr_t> contextMap;
        std::mutex contextMutex;

    public:
        void add(int contextId, uintptr_t contextPtr) {
            std::lock_guard<std::mutex> lock(contextMutex);
            contextMap[contextId] = contextPtr;
        }

        void remove(int contextId) {
            std::lock_guard<std::mutex> lock(contextMutex);
            contextMap.erase(contextId);
        }

        uintptr_t get(int contextId) {
            std::lock_guard<std::mutex> lock(contextMutex);
            auto it = contextMap.find(contextId);
            return (it != contextMap.end()) ? it->second : 0;
        }

        size_t size() {
            std::lock_guard<std::mutex> lock(contextMutex);
            return contextMap.size();
        }

        std::vector<std::pair<int, uintptr_t>> snapshot() {
            std::lock_guard<std::mutex> lock(contextMutex);
            std::vector<std::pair<int, uintptr_t>> items;
            items.reserve(contextMap.size());
            for (const auto& entry : contextMap) {
                items.push_back(entry);
            }
            return items;
        }
        
        void clear(std::function<void(uintptr_t)> deleter = nullptr) {
            // Take a snapshot and clear the map first, then delete objects.
            // This ensures any concurrent lookups via get() will return 0 (not found)
            // rather than a dangling pointer while deletion is in progress.
            std::vector<uintptr_t> toDelete;
            {
                std::lock_guard<std::mutex> lock(contextMutex);
                if (deleter) {
                    toDelete.reserve(contextMap.size());
                    for (auto& pair : contextMap) {
                        toDelete.push_back(pair.second);
                    }
                }
                contextMap.clear();
            }
            // Delete outside the lock to avoid potential deadlocks
            if (deleter) {
                for (uintptr_t ptr : toDelete) {
                    deleter(ptr);
                }
            }
        }
    };

    extern ContextManager<rnllama::llama_rn_context> g_llamaContexts;
}

#pragma once

#include "pch.h"

#include <NativeModules.h>

using namespace winrt::Microsoft::ReactNative;

namespace winrt::RNLlama {

// RNW attribute-based C++/WinRT TurboModule. REACT_TURBO_MODULE registers it
// under the "RNLlama" name that src/NativeRNLlama.ts resolves via
// TurboModuleRegistry.
REACT_TURBO_MODULE(TurboModule, L"RNLlama")
struct TurboModule {
  ReactContext m_context{nullptr};

  // Capture the ReactContext so `install()` can reach the live JSI runtime
  // and the JS thread dispatcher on demand.
  REACT_INIT(Initialize)
  void Initialize(ReactContext const &reactContext) noexcept {
    m_context = reactContext;
  }

  // Called from JS as RNLlama.install() -> Promise<boolean>. Installs the
  // shared JSI bindings (cpp/jsi/RNLlamaJSI.cpp) on the JS thread, then
  // resolves the Promise. Resolves false if the JS runtime can't be reached.
  REACT_METHOD(Install, L"install")
  void Install(ReactPromise<bool> result) noexcept;
};

} // namespace winrt::RNLlama
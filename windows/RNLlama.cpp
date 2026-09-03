#include "pch.h"

#include "RNLlama.h"

#include <jsi/jsi.h>
#include <ReactCommon/CallInvoker.h>
#include <atomic>

// Core JSI bindings (shared with iOS/Android).
#include "jsi/RNLlamaJSI.h"

// ExecuteJsi / ReactContext::CallInvoker come from RNW Cxx JSI helpers.
#include <ReactContext.h>

namespace winrt::RNLlama {

// Installs the shared llama.cpp JSI bindings on the live JS runtime. Idempotent
// (re-running just re-registers the global functions). Returns false if the
// install threw (broken runtime).
static bool installBindings(ReactContext const &context, facebook::jsi::Runtime &runtime) noexcept {
  try {
    auto callInvoker = context.CallInvoker();
    rnllama_jsi::installJSIBindings(runtime, callInvoker);
    return true;
  } catch (...) {
    return false;
  }
}

// RNW invokes this when the JS runtime is ready, before any JS runs. Installing
// the bindings here gives Windows consumers the "it just works" experience with
// no explicit installJsi() call needed, while remaining idempotent if install()
// is also called later.
void TurboModule::InitializeJsi(ReactContext const &reactContext, facebook::jsi::Runtime &runtime) noexcept {
  m_context = reactContext;
  if (installBindings(reactContext, runtime)) {
    m_installed.store(true, std::memory_order_relaxed);
  }
}

void TurboModule::Install(ReactPromise<bool> result) noexcept {
  // If InitializeJsi already installed the bindings, this is an idempotent
  // success report. Otherwise attempt the install now on the JS thread.
  if (m_installed.load(std::memory_order_relaxed)) {
    try {
      result.Resolve(true);
    } catch (...) {
    }
    return;
  }

  try {
    // ExecuteJsi runs the lambda on the JS thread with the live runtime.
    ExecuteJsi(m_context, [this, result](facebook::jsi::Runtime &runtime) {
      bool ok = installBindings(m_context, runtime);
      if (ok) {
        m_installed.store(true, std::memory_order_relaxed);
      }
      try {
        result.Resolve(ok);
      } catch (...) {
      }
    });
  } catch (...) {
    try {
      result.Resolve(false);
    } catch (...) {
    }
  }
}

} // namespace winrt::RNLlama
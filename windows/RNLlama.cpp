#include "pch.h"

#include "RNLlama.h"

#include <jsi/jsi.h>
#include <ReactCommon/CallInvoker.h>

// Core JSI bindings (shared with iOS/Android). Compiled from source so it
// matches the consuming app's RN version.
#include "jsi/RNLlamaJSI.h"

// ExecuteJsi / TryGetOrCreateContextRuntime / MakeAbiCallInvoker come from the
// Cxx JSI helpers.
#include <ReactContext.h>

namespace winrt::RNLlama {

void TurboModule::Install(ReactPromise<bool> result) noexcept {
  try {
    // ExecuteJsi runs the lambda on the JS thread with the live
    // facebook::jsi::Runtime&, and MakeAbiCallInvoker produces a CallInvoker
    // rooted at this ReactContext (the JS thread dispatcher).
    ExecuteJsi(m_context, [this, result](facebook::jsi::Runtime &runtime) {
      try {
        auto callInvoker = MakeAbiCallInvoker(m_context);
        rnllama_jsi::installJSIBindings(runtime, callInvoker);
        result.Resolve(true);
      } catch (...) {
        // installJSIBindings throws on a broken runtime; degrade gracefully so
        // JS surfaces "JSI bindings not installed".
        try {
          result.Resolve(false);
        } catch (...) {
        }
      }
    });
  } catch (...) {
    // If ExecuteJsi itself failed to schedule, reject the Promise.
    try {
      result.Reject(L"Failed to install RNLlama JSI bindings");
    } catch (...) {
    }
  }
}

} // namespace winrt::RNLlama

namespace winrt::RNLlama::implementation {

void ReactPackageProvider::CreatePackage(
    winrt::Microsoft::ReactNative::IReactPackageBuilder const &packageBuilder) noexcept {
  // Register the attributed "RNLlama" TurboModule from RNLlama.h.
  AddAttributedModules(packageBuilder, /* includeExports */ true);
}

} // namespace winrt::RNLlama::implementation
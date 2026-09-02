#include "pch.h"

#include "RNLlama.h"

#include <jsi/jsi.h>
#include <ReactCommon/CallInvoker.h>

// Core JSI bindings (shared with iOS/Android). Compiled from source so it
// matches the consuming app's RN version — same rule as the other platforms.
#include "jsi/RNLlamaJSI.h"

#include <ReactContext.h> // winrt::Microsoft::ReactNative::ExecuteJsi / ReactContext

namespace winrt::RNLlama::implementation {

void RNLlamaModule::Initialize(winrt::Microsoft::ReactNative::ReactContext const &context) noexcept {
  // m_context is assigned by the module-builder initializer below.
  (void)context;
}

void RNLlamaModule::CreateNativeModule(
    winrt::Microsoft::ReactNative::IReactModuleBuilder const &moduleBuilder) noexcept {
  auto weakThis{winrt::make_weak(this)};

  // Capture the ReactContext at module init so "install" can later reach the
  // live JSI runtime + call invoker on demand (matching how install() works on
  // iOS/Android instead of installing at app startup).
  moduleBuilder.AddInitializer([weakThis](winrt::Microsoft::ReactNative::ReactContext const &context) noexcept {
    if (auto strongThis{weakThis.get()}) {
      strongThis->m_context = context;
    }
  });

  moduleBuilder.AddMethod(
      L"install",
      [weakThis](
          winrt::Microsoft::ReactNative::ReactContext const &context,
          winrt::Microsoft::ReactNative::JSValueArray const & /*args*/,
          std::function<void(winrt::Microsoft::ReactNative::JSValue const &)> const &resolve,
          std::function<void(winrt::Microsoft::ReactNative::JSValue const &)> const &reject) noexcept {
        auto strongThis{weakThis.get()};
        if (!strongThis) {
          reject(winrt::Microsoft::ReactNative::JSValue{"RNLlama module was destroyed"});
          return;
        }

        auto ctx{strongThis->m_context ? strongThis->m_context : context};

        try {
          // ExecuteJsi runs the lambda on the JS thread with the live
          // facebook::jsi::Runtime& (scheduling via JSDispatcher if needed),
          // which is exactly where installJSIBindings expects to run.
          winrt::Microsoft::ReactNative::ExecuteJsi(
              ctx, [ctx, resolve](facebook::jsi::Runtime &runtime) noexcept {
                try {
                  auto callInvoker = winrt::Microsoft::ReactNative::MakeAbiCallInvoker(ctx);
                  rnllama_jsi::installJSIBindings(runtime, callInvoker);
                  resolve(true);
                } catch (...) {
                  // installJSIBindings throws on a broken runtime; degrade
                  // gracefully so JS surfaces "JSI bindings not installed".
                  try {
                    resolve(false);
                  } catch (...) {
                  }
                }
              });
        } catch (...) {
          reject(winrt::Microsoft::ReactNative::JSValue{"Failed to install RNLlama JSI bindings"});
        }
      });
}

void ReactPackageProvider::CreatePackage(
    winrt::Microsoft::ReactNative::IReactPackageBuilder const &packageBuilder) noexcept {
  // Register the "RNLlama" native module that src/NativeRNLlama.ts resolves.
  packageBuilder.AddModule(L"RNLlama", winrt::make<RNLlamaModule>());
}

} // namespace winrt::RNLlama::implementation
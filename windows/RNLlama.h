#pragma once

#include "pch.h"

// RNW 0.82 native-module attribute API (Microsoft.ReactNative.Cxx). llama.rn
// is registered as a TurboModule exposing a single `install()` method that
// triggers the JSI binding setup (src/NativeRNLlama.ts), mirroring the
// iOS/Android install trigger over RNW.
#include <NativeModules.h>

using namespace winrt::Microsoft::ReactNative;

namespace winrt::RNLlama {

// RNW attribute-based C++/WinRT TurboModule. REACT_TURBO_MODULE registers it
// under the "RNLlama" name that src/NativeRNLlama.ts resolves via
// TurboModuleRegistry. It is a plain struct (default-constructible) per the
// macro contract.
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
  REACT_METHOD(Install)
  void Install(ReactPromise<bool> result) noexcept;
};

} // namespace winrt::RNLlama

namespace winrt::RNLlama::implementation {

// Package provider that registers the attributed TurboModule. This is what RNW
// autolinking discovers and the app registers to expose "RNLlama".
struct ReactPackageProvider
    : winrt::implements<ReactPackageProvider, winrt::Microsoft::ReactNative::IReactPackageProvider> {
  void CreatePackage(winrt::Microsoft::ReactNative::IReactPackageBuilder const &packageBuilder) noexcept;
};

} // namespace winrt::RNLlama::implementation

namespace winrt::RNLlama::factory_implementation {

struct ReactPackageProvider
    : winrt::implements<ReactPackageProvider, winrt::Microsoft::ReactNative::IReactPackageProvider> {
  void CreatePackage(winrt::Microsoft::ReactNative::IReactPackageBuilder const &packageBuilder) noexcept {
    winrt::RNLlama::implementation::ReactPackageProvider provider;
    provider.CreatePackage(packageBuilder);
  }
};

} // namespace winrt::RNLlama::factory_implementation
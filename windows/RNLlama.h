#pragma once

#include "pch.h"

#include <functional>
#include <memory>
#include <string>

namespace winrt::Microsoft::ReactNative {
struct ReactContext;
struct IReactPackageBuilder;
} // namespace winrt::Microsoft::ReactNative

namespace winrt::RNLlama::implementation {

// RNW-native module that triggers llama.cpp JSI binding install. Unlike a
// codegen TurboModule (which requires generated *Spec.h files against the exact
// RN version), it registers the "RNLlama" module + "install" method directly
// via ReactModuleBuilder, so it matches src/NativeRNLlama.ts and builds against
// stock RNW headers.
struct RNLlamaModule : winrt::implements<RNLlamaModule, winrt::Microsoft::ReactNative::IReactModuleBuilder> {
  RNLlamaModule() = default;

  void Initialize(winrt::Microsoft::ReactNative::ReactContext const &context) noexcept;
  void CreateNativeModule(winrt::Microsoft::ReactNative::IReactModuleBuilder const &moduleBuilder) noexcept;

  // Set through the module-builder initializer; used by "install" to reach the
  // live JSI runtime + call invoker on demand.
  winrt::Microsoft::ReactNative::ReactContext m_context{nullptr};
};

// Autolinked package entry point. The app lists ReactPackageProvider in its
// App.cpp package providers, mirroring android/ReactPackage for RNW.
struct ReactPackageProvider
    : winrt::implements<ReactPackageProvider, winrt::Microsoft::ReactNative::IReactPackageProvider> {
  ReactPackageProvider() = default;
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
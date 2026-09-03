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

  // Captures the ReactContext so install() can reach the live JSI runtime and
  // the JS thread dispatcher on demand.
  REACT_INIT(Initialize)
  void Initialize(ReactContext const &reactContext) noexcept {
    m_context = reactContext;
  }

  // JSI runtime initializer: RNW invokes this when the JS runtime is ready, so
  // the llama.cpp JSI bindings are installed early and reliably on Windows.
  REACT_INIT(InitializeJsi)
  void InitializeJsi(ReactContext const &reactContext, facebook::jsi::Runtime &runtime) noexcept;

  // Called from JS as RNLlama.install() -> Promise<boolean>. Serves as an
  // idempotent fallback and report: resolves true if the shared JSI bindings are
  // installed (either here or by InitializeJsi), false otherwise.
  REACT_METHOD(Install, L"install")
  void Install(ReactPromise<bool> result) noexcept;

 private:
  // True once installJSIBindings has succeeded (via InitializeJsi or Install).
  // Cross-thread but benign: worst case a slightly stale value around the
  // install window is harmless because install is idempotent.
  std::atomic<bool> m_installed{false};
};

} // namespace winrt::RNLlama
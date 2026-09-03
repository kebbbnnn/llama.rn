#include "pch.h"

#include "RNLlama.h"

#include <jsi/jsi.h>
#include <ReactCommon/CallInvoker.h>
#include <cstdlib>

#include <winrt/Windows.Storage.h>

// Core JSI bindings (shared with iOS/Android).
#include "jsi/RNLlamaJSI.h"

// ExecuteJsi / MakeAbiCallInvoker come from Cxx JSI helpers.
#include <ReactContext.h>

namespace winrt::RNLlama {

// Test-only hook: when the RNLLAMA_SMOKE_MARKER env var is set, write a marker
// file into the app's LocalFolder after the JSI bindings install successfully.
// CI reads it back as a deterministic proof that installJSIBindings actually
// ran on the live Windows runtime. The app is a packaged UWP (AppContainer), so
// the marker must live in ApplicationData.LocalFolder (an arbitrary absolute
// path would be blocked by the sandbox). Inert for normal consumers.
static void writeSmokeMarkerIfRequested() noexcept {
  if (!std::getenv("RNLLAMA_SMOKE_MARKER")) {
    return;
  }
  try {
    auto localFolder = winrt::Windows::Storage::ApplicationData::Current().LocalFolder();
    // CreateFileAsync creates or opens the file; WriteTextAsync writes it.
    auto file = localFolder.CreateFileAsync(
        L"rnllama-install-ok.txt",
        winrt::Windows::Storage::CreationCollisionOption::ReplaceExisting).get();
    winrt::Windows::Storage::FileIO::WriteTextAsync(file, L"ok").get();
  } catch (...) {
  }
}

void TurboModule::Install(ReactPromise<bool> result) noexcept {
  try {
    // ExecuteJsi runs the lambda on the JS thread with the live
    // facebook::jsi::Runtime&, and ReactContext::CallInvoker produces a
    // CallInvoker rooted at this ReactContext (the JS thread dispatcher).
    ExecuteJsi(m_context, [this, result](facebook::jsi::Runtime &runtime) {
      try {
        auto callInvoker = m_context.CallInvoker();
        rnllama_jsi::installJSIBindings(runtime, callInvoker);
        writeSmokeMarkerIfRequested();
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
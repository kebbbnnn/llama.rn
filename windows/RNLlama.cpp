#include "pch.h"

#include "RNLlama.h"

#include <jsi/jsi.h>
#include <ReactCommon/CallInvoker.h>

#include <winrt/Windows.Storage.h>

// Core JSI bindings (shared with iOS/Android).
#include "jsi/RNLlamaJSI.h"

// ExecuteJsi / MakeAbiCallInvoker come from Cxx JSI helpers.
#include <ReactContext.h>

namespace winrt::RNLlama {

// Diagnostic marker (test hook): on successful installJSIBindings, write a tiny
// file into the app's LocalFolder so CI can assert the bindings actually ran on
// the live Windows runtime. Written unconditionally (UWP apps are launched via
// shell activation and do NOT inherit the launching process's environment, so an
// env-var gate would silently disable it). A ~3-byte file in the sandbox is
// inert for normal consumers.
static void writeInstallMarker() noexcept {
  try {
    auto localFolder = winrt::Windows::Storage::ApplicationData::Current().LocalFolder();
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
        writeInstallMarker();
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
#include "pch.h"

#include "RNLlama.h"

#include <jsi/jsi.h>
#include <ReactCommon/CallInvoker.h>

#include <winrt/Windows.Storage.h>

// Core JSI bindings (shared with iOS/Android).
#include "jsi/RNLlamaJSI.h"

// ExecuteJsi / MakeAbiCallInvoker come from Cxx JSI helpers.
#include <ReactContext.h>

#include <cstdlib>
#include <filesystem>
#include <fstream>

namespace winrt::RNLlama {

// Diagnostic marker (test hook): on successful installJSIBindings, write a marker
// so CI can assert the bindings actually ran on the live Windows runtime.
// Supports both unpackaged Win32 applications (via env var, temp dir, or LocalAppData)
// and packaged UWP/MSIX applications (via ApplicationData::Current().LocalFolder()).
static void writeMarkerToFile(std::filesystem::path const &p) noexcept {
  try {
    std::error_code ec;
    if (p.has_parent_path()) {
      std::filesystem::create_directories(p.parent_path(), ec);
    }
    std::ofstream out(p, std::ios::out | std::ios::trunc);
    if (out.is_open()) {
      out << "ok\n";
      out.flush();
    }
  } catch (...) {
  }
}

static void writeInstallMarker() noexcept {
  // 1. Explicit marker path from environment variable (Win32 desktop app)
  if (const char *envPath = std::getenv("RNLLAMA_INSTALL_MARKER")) {
    if (envPath[0] != '\0') {
      writeMarkerToFile(envPath);
    }
  }

  // 2. Standard temp directory (%TEMP% / std::filesystem::temp_directory_path())
  try {
    std::error_code ec;
    auto tempDir = std::filesystem::temp_directory_path(ec);
    if (!ec) {
      writeMarkerToFile(tempDir / "rnllama-install-ok.txt");
    }
  } catch (...) {
  }

  // 3. LocalAppData (%LOCALAPPDATA%\rnllama-install-ok.txt)
  if (const char *localAppData = std::getenv("LOCALAPPDATA")) {
    if (localAppData[0] != '\0') {
      writeMarkerToFile(std::filesystem::path(localAppData) / "rnllama-install-ok.txt");
    }
  }

  // 4. UWP / MSIX LocalFolder (if running with package identity in AppContainer)
  try {
    auto localFolder = winrt::Windows::Storage::ApplicationData::Current().LocalFolder();
    auto folderPath = std::wstring(localFolder.Path());
    writeMarkerToFile(std::filesystem::path(folderPath) / "rnllama-install-ok.txt");
  } catch (...) {
    // Expected when running as an unpackaged Win32 process (no package identity)
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
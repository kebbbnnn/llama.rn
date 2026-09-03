#pragma once

#include "targetver.h"

// Match the iterator debug level of prebuilt Release rnllama.lib
#ifndef _ITERATOR_DEBUG_LEVEL
#define _ITERATOR_DEBUG_LEVEL 0
#endif

#define NOMINMAX 1
#define WIN32_LEAN_AND_MEAN 1
#define WINRT_LEAN_AND_MEAN 1

// Windows Header Files
#include <windows.h>
#undef GetCurrentTime
#include <unknwn.h>

// WinRT Header Files
#include <winrt/base.h>
#include <CppWinRTIncludes.h>
#include <winrt/Microsoft.ReactNative.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>

// JSI & ReactCommon
#include <jsi/jsi.h>
#include <ReactCommon/CallInvoker.h>
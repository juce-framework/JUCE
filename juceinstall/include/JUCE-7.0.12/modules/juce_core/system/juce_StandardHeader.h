/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#pragma once

//==============================================================================
/** Current JUCE version number.

    See also SystemStats::getJUCEVersion() for a string version.
*/
#define JUCE_MAJOR_VERSION      7
#define JUCE_MINOR_VERSION      0
#define JUCE_BUILDNUMBER        12

/** Current JUCE version number.

    Bits 16 to 32 = major version.
    Bits 8 to 16 = minor version.
    Bits 0 to 8 = point release.

    See also SystemStats::getJUCEVersion() for a string version.
*/
#define JUCE_VERSION   ((JUCE_MAJOR_VERSION << 16) + (JUCE_MINOR_VERSION << 8) + JUCE_BUILDNUMBER)

#if ! DOXYGEN
#define JUCE_VERSION_ID \
    [[maybe_unused]] volatile auto juceVersionId = "juce_version_" JUCE_STRINGIFY(JUCE_MAJOR_VERSION) "_" JUCE_STRINGIFY(JUCE_MINOR_VERSION) "_" JUCE_STRINGIFY(JUCE_BUILDNUMBER);
#endif

//==============================================================================
#include <algorithm>
#include <array>
#include <atomic>
#include <cmath>
#include <condition_variable>
#include <cstddef>
#include <functional>
#include <future>
#include <iomanip>
#include <iostream>
#include <limits>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <numeric>
#include <optional>
#include <queue>
#include <set>
#include <sstream>
#include <string_view>
#include <thread>
#include <typeindex>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <variant>
#include <vector>

//==============================================================================
#include "juce_CompilerSupport.h"
#include "juce_CompilerWarnings.h"
#include "juce_PlatformDefs.h"

//==============================================================================
// Now we'll include some common OS headers..
JUCE_BEGIN_IGNORE_WARNINGS_MSVC (4514 4245 4100)

#if JUCE_MSVC
 #include <intrin.h>
#endif


#if JUCE_MAC || JUCE_IOS
 #include <libkern/OSAtomic.h>
 #include <libkern/OSByteOrder.h>
 #include <xlocale.h>
 #include <signal.h>
#endif

#if JUCE_LINUX || JUCE_BSD
 #include <cstring>
 #include <signal.h>

 #if __INTEL_COMPILER
  #if __ia64__
   #include <ia64intrin.h>
  #else
   #include <ia32intrin.h>
  #endif
 #endif
#endif

#if JUCE_MSVC && JUCE_DEBUG
 #include <crtdbg.h>
#endif

JUCE_END_IGNORE_WARNINGS_MSVC

#if JUCE_MINGW
 #include <cstring>
 #include <sys/types.h>
#endif

#if JUCE_ANDROID
 #include <cstring>
 #include <byteswap.h>
#endif

// undef symbols that are sometimes set by misguided 3rd-party headers..
#undef TYPE_BOOL
#undef max
#undef min
#undef major
#undef minor
#undef KeyPress

//==============================================================================
// DLL building settings on Windows
#if JUCE_MSVC
 #ifdef JUCE_DLL_BUILD
  #define JUCE_API __declspec (dllexport)
  #pragma warning (disable: 4251)
 #elif defined (JUCE_DLL)
  #define JUCE_API __declspec (dllimport)
  #pragma warning (disable: 4251)
 #endif
 #ifdef __INTEL_COMPILER
  #pragma warning (disable: 1125) // (virtual override warning)
 #endif
#elif defined (JUCE_DLL) || defined (JUCE_DLL_BUILD)
 #define JUCE_API __attribute__ ((visibility ("default")))
#endif

//==============================================================================
#ifndef JUCE_API
 #define JUCE_API   /**< This macro is added to all JUCE public class declarations. */
#endif

#if JUCE_MSVC && JUCE_DLL_BUILD
 #define JUCE_PUBLIC_IN_DLL_BUILD(declaration)  public: declaration; private:
#else
 #define JUCE_PUBLIC_IN_DLL_BUILD(declaration)  declaration;
#endif

/** This macro is added to all JUCE public function declarations. */
#define JUCE_PUBLIC_FUNCTION        JUCE_API JUCE_CALLTYPE

#ifndef DOXYGEN
 #define JUCE_NAMESPACE juce  // This old macro is deprecated: you should just use the juce namespace directly.
#endif

/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2016 - ROLI Ltd.

   Permission is granted to use this software under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license/

   Permission to use, copy, modify, and/or distribute this software for any
   purpose with or without fee is hereby granted, provided that the above
   copyright notice and this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND ISC DISCLAIMS ALL WARRANTIES WITH REGARD
   TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
   FITNESS. IN NO EVENT SHALL ISC BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT,
   OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
   USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
   TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
   OF THIS SOFTWARE.

   -----------------------------------------------------------------------------

   To release a closed-source product which uses other parts of JUCE not
   licensed under the ISC terms, commercial licenses are available: visit
   www.juce.com for more information.

  ==============================================================================
*/

#ifndef JUCE_STANDARDHEADER_H_INCLUDED
#define JUCE_STANDARDHEADER_H_INCLUDED

//==============================================================================
/** Current JUCE version number.

    See also SystemStats::getJUCEVersion() for a string version.
*/
#define JUCE_MAJOR_VERSION      4
#define JUCE_MINOR_VERSION      3
#define JUCE_BUILDNUMBER        1

/** Current Juce version number.

    Bits 16 to 32 = major version.
    Bits 8 to 16 = minor version.
    Bits 0 to 8 = point release.

    See also SystemStats::getJUCEVersion() for a string version.
*/
#define JUCE_VERSION   ((JUCE_MAJOR_VERSION << 16) + (JUCE_MINOR_VERSION << 8) + JUCE_BUILDNUMBER)


//==============================================================================
#include <memory>
#include <cmath>
#include <vector>
#include <iostream>
#include <functional>
#include <algorithm>
#include <limits>

//==============================================================================
#include "juce_CompilerSupport.h"
#include "juce_PlatformDefs.h"

//==============================================================================
// Now we'll include some common OS headers..
#if JUCE_MSVC
 #pragma warning (push)
 #pragma warning (disable: 4514 4245 4100)
 #include <intrin.h>
#endif

#if JUCE_MAC || JUCE_IOS
 #include <libkern/OSAtomic.h>
#endif

#if JUCE_LINUX
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

#if JUCE_MSVC
 #pragma warning (pop)
#endif

#if JUCE_MINGW
 #include <cstring>
 #include <sys/types.h>
#endif

#if JUCE_ANDROID
 #include <cstring>
 #include <atomic>
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
 #define JUCE_API __attribute__ ((visibility("default")))
#endif

//==============================================================================
#ifndef JUCE_API
 #define JUCE_API   /**< This macro is added to all juce public class declarations. */
#endif

#if JUCE_MSVC && JUCE_DLL_BUILD
 #define JUCE_PUBLIC_IN_DLL_BUILD(declaration)  public: declaration; private:
#else
 #define JUCE_PUBLIC_IN_DLL_BUILD(declaration)  declaration;
#endif

/** This macro is added to all juce public function declarations. */
#define JUCE_PUBLIC_FUNCTION        JUCE_API JUCE_CALLTYPE

#if (! defined (JUCE_CATCH_DEPRECATED_CODE_MISUSE)) && JUCE_DEBUG && ! DOXYGEN
 /** This turns on some non-essential bits of code that should prevent old code from compiling
     in cases where method signatures have changed, etc.
 */
 #define JUCE_CATCH_DEPRECATED_CODE_MISUSE 1
#endif

#ifndef DOXYGEN
 #define JUCE_NAMESPACE juce  // This old macro is deprecated: you should just use the juce namespace directly.
#endif

#endif   // JUCE_STANDARDHEADER_H_INCLUDED

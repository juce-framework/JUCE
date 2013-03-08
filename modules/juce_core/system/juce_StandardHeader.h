/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

#ifndef __JUCE_STANDARDHEADER_JUCEHEADER__
#define __JUCE_STANDARDHEADER_JUCEHEADER__

//==============================================================================
/** Current JUCE version number.

    See also SystemStats::getJUCEVersion() for a string version.
*/
#define JUCE_MAJOR_VERSION      2
#define JUCE_MINOR_VERSION      0
#define JUCE_BUILDNUMBER        38

/** Current Juce version number.

    Bits 16 to 32 = major version.
    Bits 8 to 16 = minor version.
    Bits 0 to 8 = point release.

    See also SystemStats::getJUCEVersion() for a string version.
*/
#define JUCE_VERSION   ((JUCE_MAJOR_VERSION << 16) + (JUCE_MINOR_VERSION << 8) + JUCE_BUILDNUMBER)


//==============================================================================
#include "juce_TargetPlatform.h"  // (sets up the various JUCE_WINDOWS, JUCE_MAC, etc flags)
#include "juce_PlatformDefs.h"

//==============================================================================
// Now we'll include some common OS headers..
#if JUCE_MSVC
 #pragma warning (push)
 #pragma warning (disable: 4514 4245 4100)
#endif

#include <cstdlib>
#include <cstdarg>
#include <climits>
#include <limits>
#include <cmath>
#include <cwchar>
#include <stdexcept>
#include <typeinfo>
#include <cstring>
#include <cstdio>
#include <iostream>
#include <vector>

#if JUCE_USE_INTRINSICS
 #include <intrin.h>
#endif

#if JUCE_MAC || JUCE_IOS
 #include <libkern/OSAtomic.h>
#endif

#if JUCE_LINUX
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

#if JUCE_ANDROID
 #include <sys/atomics.h>
 #include <byteswap.h>
#endif

// undef symbols that are sometimes set by misguided 3rd-party headers..
#undef check
#undef TYPE_BOOL
#undef max
#undef min

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

//==============================================================================
// Now include some common headers...
namespace juce
{
    extern JUCE_API bool JUCE_CALLTYPE juce_isRunningUnderDebugger();

    #if JUCE_LOG_ASSERTIONS
     extern JUCE_API void logAssertion (const char* file, int line) noexcept;
    #endif

    #include "../memory/juce_Memory.h"
    #include "../maths/juce_MathsFunctions.h"
    #include "../memory/juce_ByteOrder.h"
    #include "../logging/juce_Logger.h"
    #include "../memory/juce_LeakedObjectDetector.h"
}

#endif   // __JUCE_STANDARDHEADER_JUCEHEADER__

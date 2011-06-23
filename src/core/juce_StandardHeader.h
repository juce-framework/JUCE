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
/** Current Juce version number.

    See also SystemStats::getJUCEVersion() for a string version.
*/
#define JUCE_MAJOR_VERSION      1
#define JUCE_MINOR_VERSION      53
#define JUCE_BUILDNUMBER        107

/** Current Juce version number.

    Bits 16 to 32 = major version.
    Bits 8 to 16 = minor version.
    Bits 0 to 8 = point release (not currently used).

    See also SystemStats::getJUCEVersion() for a string version.
*/
#define JUCE_VERSION            ((JUCE_MAJOR_VERSION << 16) + (JUCE_MINOR_VERSION << 8) + JUCE_BUILDNUMBER)


//==============================================================================
#include "juce_TargetPlatform.h"  // (sets up the various JUCE_WINDOWS, JUCE_MAC, etc flags)

#include "../../juce_Config.h"

//==============================================================================
#ifdef JUCE_NAMESPACE
  #define BEGIN_JUCE_NAMESPACE    namespace JUCE_NAMESPACE {
  #define END_JUCE_NAMESPACE      }
#else
  #define BEGIN_JUCE_NAMESPACE
  #define END_JUCE_NAMESPACE
#endif

//==============================================================================
#include "juce_PlatformDefs.h"

// Now we'll include any OS headers we need.. (at this point we are outside the Juce namespace).
#if JUCE_MSVC
  #if JUCE_VC6
    #pragma warning (disable: 4284 4786)  // (spurious VC6 warnings)

    namespace std   // VC6 doesn't have sqrt/sin/cos/tan/abs in std, so declare them here:
    {
        template <typename Type> Type abs (Type a)              { if (a < 0) return -a; return a; }
        template <typename Type> Type tan (Type a)              { return static_cast<Type> (::tan (static_cast<double> (a))); }
        template <typename Type> Type sin (Type a)              { return static_cast<Type> (::sin (static_cast<double> (a))); }
        template <typename Type> Type cos (Type a)              { return static_cast<Type> (::cos (static_cast<double> (a))); }
        template <typename Type> Type sqrt (Type a)             { return static_cast<Type> (::sqrt (static_cast<double> (a))); }
        template <typename Type> Type floor (Type a)            { return static_cast<Type> (::floor (static_cast<double> (a))); }
        template <typename Type> Type ceil (Type a)             { return static_cast<Type> (::ceil (static_cast<double> (a))); }
        template <typename Type> Type atan2 (Type a, Type b)    { return static_cast<Type> (::atan2 (static_cast<double> (a), static_cast<double> (b))); }
    }
  #endif

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
  #include <malloc.h>
  #pragma warning (pop)

  #if ! JUCE_PUBLIC_INCLUDES
    #pragma warning (4: 4511 4512 4100)  // (enable some warnings that are turned off in VC8)
  #endif
#endif

#if JUCE_ANDROID
  #include <sys/atomics.h>
  #include <byteswap.h>
#endif

//==============================================================================
// DLL building settings on Win32
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
#elif defined (__GNUC__) && ((__GNUC__ >= 4) || (__GNUC__ == 3 && __GNUC_MINOR__ >= 4))
  #ifdef JUCE_DLL_BUILD
    #define JUCE_API __attribute__ ((visibility("default")))
  #endif
#endif

#ifndef JUCE_API
  /** This macro is added to all juce public class declarations. */
  #define JUCE_API
#endif

/** This macro is added to all juce public function declarations. */
#define JUCE_PUBLIC_FUNCTION        JUCE_API JUCE_CALLTYPE

/** This turns on some non-essential bits of code that should prevent old code from compiling
    in cases where method signatures have changed, etc.
*/
#if (! defined (JUCE_CATCH_DEPRECATED_CODE_MISUSE)) && JUCE_DEBUG && ! DOXYGEN
 #define JUCE_CATCH_DEPRECATED_CODE_MISUSE 1
#endif

//==============================================================================
// Now include some basics that are needed by most of the Juce classes...
BEGIN_JUCE_NAMESPACE

extern JUCE_API bool JUCE_CALLTYPE juce_isRunningUnderDebugger();

#if JUCE_LOG_ASSERTIONS
  extern JUCE_API void juce_LogAssertion (const char* filename, int lineNum) noexcept;
#endif

#include "../memory/juce_Memory.h"
#include "../maths/juce_MathsFunctions.h"
#include "../memory/juce_ByteOrder.h"
#include "juce_Logger.h"
#include "../memory/juce_LeakedObjectDetector.h"

#undef TYPE_BOOL  // (stupidly-named CoreServices definition which interferes with other libraries).

END_JUCE_NAMESPACE


#endif   // __JUCE_STANDARDHEADER_JUCEHEADER__

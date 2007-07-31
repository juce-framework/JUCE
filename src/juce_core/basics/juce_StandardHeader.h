/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

  ==============================================================================
*/

#ifndef __JUCE_STANDARDHEADER_JUCEHEADER__
#define __JUCE_STANDARDHEADER_JUCEHEADER__

//==============================================================================
/** Current Juce version number.

    See also SystemStats::getJUCEVersion() for a string version.
*/
#define JUCE_MAJOR_VERSION      1
#define JUCE_MINOR_VERSION      44

/** Current Juce version number.

    Bits 16 to 32 = major version.
    Bits 8 to 16 = minor version.
    Bits 0 to 8 = point release (not currently used).

    See also SystemStats::getJUCEVersion() for a string version.
*/
#define JUCE_VERSION            ((JUCE_MAJOR_VERSION << 16) + (JUCE_MINOR_VERSION << 8))


//==============================================================================
#include "../../../juce_Config.h"

// This sets up the JUCE_WIN32, JUCE_MAC, or JUCE_LINUX macros
#include "juce_PlatformDefs.h"

//==============================================================================
// Now we'll include any OS headers we need.. (at this point we are outside the Juce namespace).

#if JUCE_MSVC
  #pragma warning (push)
  #pragma warning (disable: 4514 4245 4100)
#endif

#include <cstdlib>
#include <cstdarg>
#include <climits>
#include <cmath>
#include <cwchar>
#include <stdexcept>
#include <typeinfo>

#if JUCE_MAC || JUCE_LINUX
  #include <pthread.h>
#endif

#if JUCE_USE_INTRINSICS
  #include <intrin.h>
#endif

#if JUCE_MAC && ! MACOS_10_3_OR_EARLIER
  #include <libkern/OSAtomic.h>
#endif

#if JUCE_MSVC && JUCE_DEBUG
  #include <crtdbg.h>
#endif

#if JUCE_MSVC
  #pragma warning (pop)
#endif

//==============================================================================
#ifdef JUCE_NAMESPACE
  #define BEGIN_JUCE_NAMESPACE    namespace JUCE_NAMESPACE {
  #define END_JUCE_NAMESPACE      }
#else
  #define BEGIN_JUCE_NAMESPACE
  #define END_JUCE_NAMESPACE
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
#endif

#ifndef JUCE_API
  /** This macro is added to all juce public class declarations. */
  #define JUCE_API
#endif

/** This macro defines the C calling convention used as the standard for Juce calls. */
#if JUCE_MSVC
  #define JUCE_CALLTYPE             __stdcall
#else
  #define JUCE_CALLTYPE
#endif

/** This macro is added to all juce public function declarations. */
#define JUCE_PUBLIC_FUNCTION        JUCE_API JUCE_CALLTYPE


//==============================================================================
// Now include some basics that are needed by most of the Juce classes...
BEGIN_JUCE_NAMESPACE

#if JUCE_LOG_ASSERTIONS
  extern void juce_LogAssertion (const char* filename, const int lineNum) throw();
#endif

#include "juce_Memory.h"
#include "juce_MathsFunctions.h"
#include "juce_DataConversions.h"
#include "juce_Logger.h"

END_JUCE_NAMESPACE


#endif   // __JUCE_STANDARDHEADER_JUCEHEADER__

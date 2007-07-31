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

#ifndef __JUCE_PLATFORMDEFS_JUCEHEADER__
#define __JUCE_PLATFORMDEFS_JUCEHEADER__

//==============================================================================
/*  This file figures out which platform is being built, and defines some macros
    that the rest of the code can use for OS-specific compilation.

    Macros that will be set here are:

    - One of JUCE_WIN32, JUCE_MAC or JUCE_LINUX.
    - Either JUCE_32BIT or JUCE_64BIT, depending on the architecture.
    - Either JUCE_LITTLE_ENDIAN or JUCE_BIG_ENDIAN.
    - Either JUCE_INTEL or JUCE_PPC
    - Either JUCE_GCC or JUCE_MSVC

    On the Mac, it also defines MACOS_10_2_OR_EARLIER if the build is targeting OSX10.2,
    and MACOS_10_3_OR_EARLIER if it is targeting either 10.2 or 10.3

    It also includes a set of macros for debug console output and assertions.

*/

//==============================================================================
#if (defined (_WIN32) || defined (_WIN64))
  #define       JUCE_WIN32 1
#else
  #ifdef LINUX
    #define     JUCE_LINUX 1
  #else
    #define     JUCE_MAC 1
  #endif
#endif

//==============================================================================
#if JUCE_WIN32
  #ifdef _MSC_VER
    #ifdef _WIN64
      #define JUCE_64BIT 1
    #else
      #define JUCE_32BIT 1
    #endif
  #endif

  #ifdef _DEBUG
    #define JUCE_DEBUG 1
  #endif

  /** If defined, this indicates that the processor is little-endian. */
  #define JUCE_LITTLE_ENDIAN 1

  #define JUCE_INTEL 1
#endif

//==============================================================================
#if JUCE_MAC

  #include <CoreServices/CoreServices.h>

  #ifndef NDEBUG
    #define JUCE_DEBUG 1
  #endif

  #ifdef __LITTLE_ENDIAN__
    #define JUCE_LITTLE_ENDIAN 1
  #else
    #define JUCE_BIG_ENDIAN 1
  #endif

  #if defined (__ppc__) || defined (__ppc64__)
    #define JUCE_PPC 1
  #else
    #define JUCE_INTEL 1
  #endif

  #ifdef __LP64__
    #define JUCE_64BIT 1
  #else
    #define JUCE_32BIT 1
  #endif

  #if (MAC_OS_X_VERSION_MIN_REQUIRED < MAC_OS_X_VERSION_10_3)
    #define MACOS_10_2_OR_EARLIER 1
  #endif

  #if (MAC_OS_X_VERSION_MIN_REQUIRED < MAC_OS_X_VERSION_10_4)
    #define MACOS_10_3_OR_EARLIER 1
  #endif
#endif

//==============================================================================
#if JUCE_LINUX

  #ifdef _DEBUG
    #define JUCE_DEBUG 1
  #endif

  // Allow override for big-endian Linux platforms
  #ifndef JUCE_BIG_ENDIAN
    #define JUCE_LITTLE_ENDIAN 1
  #endif

  #if defined (__LP64__) || defined (_LP64)
    #define JUCE_64BIT 1
  #else
    #define JUCE_32BIT 1
  #endif

  #define JUCE_INTEL 1
#endif


//==============================================================================
// Compiler type macros.

#ifdef __GNUC__
  #define JUCE_GCC 1
#elif defined (_MSC_VER)
  #define JUCE_MSVC 1

  #if _MSC_VER >= 1400
    #define JUCE_USE_INTRINSICS 1
  #endif
#else
  #error unknown compiler
#endif

//==============================================================================
// Debugging and assertion macros

// (For info about JUCE_LOG_ASSERTIONS, have a look in juce_Config.h)
#if JUCE_LOG_ASSERTIONS
  #define juce_LogCurrentAssertion    juce_LogAssertion (__FILE__, __LINE__);
#elif defined (JUCE_DEBUG)
  #define juce_LogCurrentAssertion    fprintf (stderr, "JUCE Assertion failure in %s, line %d\n", __FILE__, __LINE__);
#else
  #define juce_LogCurrentAssertion
#endif

#ifdef JUCE_DEBUG
  //==============================================================================
  // If debugging is enabled..

  /** Writes a string to the standard error stream.

    This is only compiled in a debug build.

    @see Logger::outputDebugString
  */
  #define DBG(dbgtext)                  Logger::outputDebugString (dbgtext);

  /** Printf's a string to the standard error stream.

    This is only compiled in a debug build.

    @see Logger::outputDebugString
  */
  #define DBG_PRINTF(dbgprintf)         Logger::outputDebugPrintf dbgprintf;

  //==============================================================================
  // Assertions..

  #if JUCE_MSVC || DOXYGEN

    #if JUCE_USE_INTRINSICS
      #pragma intrinsic (__debugbreak)

      /** This will always cause an assertion failure.

          It is only compiled in a debug build, (unless JUCE_LOG_ASSERTIONS is enabled
          in juce_Config.h).

          @see jassert()
      */
      #define jassertfalse              { juce_LogCurrentAssertion; __debugbreak(); }
    #else
      /** This will always cause an assertion failure.

          This is only compiled in a debug build.

          @see jassert()
      */
      #define jassertfalse              { juce_LogCurrentAssertion; __asm int 3 }
    #endif
  #elif defined (JUCE_MAC)
    #define jassertfalse                { juce_LogCurrentAssertion; Debugger(); }
  #elif defined (JUCE_GCC) || defined (JUCE_LINUX)
    #define jassertfalse                { juce_LogCurrentAssertion; asm("int $3"); }
  #endif

  //==============================================================================
  /** Platform-independent assertion macro.

      This gets optimised out when not being built with debugging turned on.

      Be careful not to call any functions within its arguments that are vital to
      the behaviour of the program, because these won't get called in the release
      build.

      @see jassertfalse
  */
  #define jassert(expression)           { if (! (expression)) jassertfalse }

#else
  //==============================================================================
  // If debugging is disabled, these dummy debug and assertion macros are used..

  #define DBG(dbgtext)
  #define DBG_PRINTF(dbgprintf)

  #define jassertfalse                  { juce_LogCurrentAssertion }

  #if JUCE_LOG_ASSERTIONS
    #define jassert(expression)         { if (! (expression)) jassertfalse }
  #else
    #define jassert(a)                  { }
  #endif

#endif

//==============================================================================
#ifndef DOXYGEN
  template <bool b> struct JuceStaticAssert;
  template <> struct JuceStaticAssert <true> { static void dummy() {} };
#endif

/** A compile-time assertion macro.

    If the expression parameter is false, the macro will cause a compile error.
*/
#define static_jassert(expression)      JuceStaticAssert<expression>::dummy();


//==============================================================================
#if JUCE_CATCH_UNHANDLED_EXCEPTIONS

  #define JUCE_TRY try

  /** Used in try-catch blocks, this macro will send exceptions to the JUCEApplication
      object so they can be logged by the application if it wants to.
  */
  #define JUCE_CATCH_EXCEPTION \
    catch (const std::exception& e)  \
    { \
        JUCEApplication::sendUnhandledException (&e, __FILE__, __LINE__); \
    } \
    catch (...) \
    { \
        JUCEApplication::sendUnhandledException (0, __FILE__, __LINE__); \
    }

  #define JUCE_CATCH_ALL            catch (...) {}
  #define JUCE_CATCH_ALL_ASSERT     catch (...) { jassertfalse }

#else

  #define JUCE_TRY
  #define JUCE_CATCH_EXCEPTION
  #define JUCE_CATCH_ALL
  #define JUCE_CATCH_ALL_ASSERT

#endif

//==============================================================================
// Macros for inlining.

#if JUCE_MSVC
  /** A platform-independent way of forcing an inline function.

      Use the syntax: @code
      forcedinline void myfunction (int x)
      @endcode
  */
  #ifdef JUCE_DEBUG
    #define forcedinline  __forceinline
  #else
    #define forcedinline  inline
  #endif

  /** A platform-independent way of stopping the compiler inlining a function.

      Use the syntax: @code
      juce_noinline void myfunction (int x)
      @endcode
  */
  #define juce_noinline

#else
  /** A platform-independent way of forcing an inline function.

      Use the syntax: @code
      forcedinline void myfunction (int x)
      @endcode
  */
  #ifndef JUCE_DEBUG
    #define forcedinline  inline __attribute__((always_inline))
  #else
    #define forcedinline  inline
  #endif

  /** A platform-independent way of stopping the compiler inlining a function.

      Use the syntax: @code
      juce_noinline void myfunction (int x)
      @endcode
  */
  #define juce_noinline __attribute__((noinline))

#endif

#endif   // __JUCE_PLATFORMDEFS_JUCEHEADER__

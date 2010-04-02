/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

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

#ifndef __JUCE_AMALGAMATED_TEMPLATE_JUCEHEADER__
#define __JUCE_AMALGAMATED_TEMPLATE_JUCEHEADER__

#define DONT_AUTOLINK_TO_JUCE_LIBRARY 1


/*** Start of inlined file: juce.h ***/
#ifndef __JUCE_JUCEHEADER__
#define __JUCE_JUCEHEADER__

#define JUCE_PUBLIC_INCLUDES 1

// (this includes things that need defining outside of the JUCE namespace)

/*** Start of inlined file: juce_StandardHeader.h ***/
#ifndef __JUCE_STANDARDHEADER_JUCEHEADER__
#define __JUCE_STANDARDHEADER_JUCEHEADER__

#define JUCE_MAJOR_VERSION	  1
#define JUCE_MINOR_VERSION	  51
#define JUCE_BUILDNUMBER	14

#define JUCE_VERSION		((JUCE_MAJOR_VERSION << 16) + (JUCE_MINOR_VERSION << 8) + JUCE_BUILDNUMBER)


/*** Start of inlined file: juce_TargetPlatform.h ***/
#ifndef __JUCE_TARGETPLATFORM_JUCEHEADER__
#define __JUCE_TARGETPLATFORM_JUCEHEADER__

#if (defined (_WIN32) || defined (_WIN64))
  #define	   JUCE_WIN32 1
  #define	   JUCE_WINDOWS 1
#elif defined (LINUX) || defined (__linux__)
  #define	 JUCE_LINUX 1
#elif defined(__APPLE_CPP__) || defined(__APPLE_CC__)
  #include <CoreFoundation/CoreFoundation.h> // (needed to find out what platform we're using)

  #if TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR
	#define	 JUCE_IPHONE 1
  #else
	#define	 JUCE_MAC 1
  #endif
#else
  #error "Unknown platform!"
#endif

#if JUCE_WINDOWS
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

  #ifdef __MINGW32__
	#define JUCE_MINGW 1
  #endif

  #define JUCE_LITTLE_ENDIAN 1

  #define JUCE_INTEL 1
#endif

#if JUCE_MAC

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
	#undef MAC_OS_X_VERSION_MAX_ALLOWED
	#define MAC_OS_X_VERSION_MAX_ALLOWED	MAC_OS_X_VERSION_10_4
  #else
	#define JUCE_INTEL 1
  #endif

  #ifdef __LP64__
	#define JUCE_64BIT 1
  #else
	#define JUCE_32BIT 1
  #endif

  #if MAC_OS_X_VERSION_MIN_REQUIRED < MAC_OS_X_VERSION_10_4
	#error "Building for OSX 10.3 is no longer supported!"
  #endif

  #ifndef MAC_OS_X_VERSION_10_5
	#error "To build with 10.4 compatibility, use a 10.5 or 10.6 SDK and set the deployment target to 10.4"
  #endif

#endif

#if JUCE_IPHONE

  #ifndef NDEBUG
	#define JUCE_DEBUG 1
  #endif

  #ifdef __LITTLE_ENDIAN__
	#define JUCE_LITTLE_ENDIAN 1
  #else
	#define JUCE_BIG_ENDIAN 1
  #endif
#endif

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

#endif   // __JUCE_TARGETPLATFORM_JUCEHEADER__
/*** End of inlined file: juce_TargetPlatform.h ***/

  // (sets up the various JUCE_WINDOWS, JUCE_MAC, etc flags)


/*** Start of inlined file: juce_Config.h ***/
#ifndef __JUCE_CONFIG_JUCEHEADER__
#define __JUCE_CONFIG_JUCEHEADER__

#ifndef JUCE_NAMESPACE
  #define JUCE_NAMESPACE juce
#endif

#ifndef JUCE_FORCE_DEBUG
  //#define JUCE_FORCE_DEBUG 1
#endif

#ifndef JUCE_LOG_ASSERTIONS
//  #define JUCE_LOG_ASSERTIONS 1
#endif

#ifndef JUCE_ASIO
  #define JUCE_ASIO 0
#endif

#ifndef JUCE_WASAPI
//  #define JUCE_WASAPI 1
#endif

#ifndef JUCE_DIRECTSOUND
  #define JUCE_DIRECTSOUND 1
#endif

#ifndef JUCE_ALSA
  #define JUCE_ALSA 1
#endif

#ifndef JUCE_JACK
  #define JUCE_JACK 1
#endif

#if ! (defined (JUCE_QUICKTIME) || JUCE_LINUX || JUCE_IPHONE || (JUCE_WINDOWS && ! JUCE_MSVC))
  #define JUCE_QUICKTIME 0
#endif

#if (JUCE_IPHONE || JUCE_LINUX) && JUCE_QUICKTIME
  #undef JUCE_QUICKTIME
#endif

#ifndef JUCE_OPENGL
  #define JUCE_OPENGL 1
#endif

#ifndef JUCE_USE_FLAC
  #define JUCE_USE_FLAC 1
#endif

#ifndef JUCE_USE_OGGVORBIS
  #define JUCE_USE_OGGVORBIS 1
#endif

#if (! defined (JUCE_USE_CDBURNER)) && ! (JUCE_WINDOWS && ! JUCE_MSVC)
  #define JUCE_USE_CDBURNER 0
#endif

#ifndef JUCE_USE_CDREADER
  #define JUCE_USE_CDREADER 1
#endif

#if (JUCE_QUICKTIME || JUCE_WINDOWS) && ! defined (JUCE_USE_CAMERA)
//  #define JUCE_USE_CAMERA 1
#endif

#ifndef JUCE_ENABLE_REPAINT_DEBUGGING
//  #define JUCE_ENABLE_REPAINT_DEBUGGING 1
#endif

#ifndef JUCE_USE_XINERAMA
  #define JUCE_USE_XINERAMA 1
#endif

#ifndef JUCE_USE_XSHM
  #define JUCE_USE_XSHM 1
#endif

#ifndef JUCE_USE_XRENDER
  //#define JUCE_USE_XRENDER 1
#endif

#ifndef JUCE_USE_XCURSOR
  #define JUCE_USE_XCURSOR 1
#endif

#ifndef JUCE_PLUGINHOST_VST
//  #define JUCE_PLUGINHOST_VST 1
#endif

#ifndef JUCE_PLUGINHOST_AU
//  #define JUCE_PLUGINHOST_AU 1
#endif

#ifndef JUCE_ONLY_BUILD_CORE_LIBRARY
  //#define JUCE_ONLY_BUILD_CORE_LIBRARY  1
#endif

#ifndef JUCE_WEB_BROWSER
  #define JUCE_WEB_BROWSER 0
#endif

#ifndef JUCE_SUPPORT_CARBON
  #define JUCE_SUPPORT_CARBON 1
#endif

#ifndef JUCE_INCLUDE_ZLIB_CODE
  #define JUCE_INCLUDE_ZLIB_CODE	1
#endif

#ifndef JUCE_INCLUDE_FLAC_CODE
  #define JUCE_INCLUDE_FLAC_CODE	1
#endif

#ifndef JUCE_INCLUDE_OGGVORBIS_CODE
  #define JUCE_INCLUDE_OGGVORBIS_CODE   1
#endif

#ifndef JUCE_INCLUDE_PNGLIB_CODE
  #define JUCE_INCLUDE_PNGLIB_CODE	  1
#endif

#ifndef JUCE_INCLUDE_JPEGLIB_CODE
  #define JUCE_INCLUDE_JPEGLIB_CODE	 1
#endif

#ifndef JUCE_CHECK_MEMORY_LEAKS
  #define JUCE_CHECK_MEMORY_LEAKS 1
#endif

#ifndef JUCE_CATCH_UNHANDLED_EXCEPTIONS
  #define JUCE_CATCH_UNHANDLED_EXCEPTIONS 1
#endif

// If only building the core classes, we can explicitly turn off some features to avoid including them:
#if JUCE_ONLY_BUILD_CORE_LIBRARY
  #undef  JUCE_QUICKTIME
  #define JUCE_QUICKTIME 0
  #undef  JUCE_OPENGL
  #define JUCE_OPENGL 0
  #undef JUCE_USE_CDBURNER
  #define JUCE_USE_CDBURNER 0
  #undef JUCE_USE_CDREADER
  #define JUCE_USE_CDREADER 0
  #undef JUCE_WEB_BROWSER
  #define JUCE_WEB_BROWSER 0
  #undef JUCE_PLUGINHOST_AU
  #define JUCE_PLUGINHOST_AU 0
  #undef JUCE_PLUGINHOST_VST
  #define JUCE_PLUGINHOST_VST 0
#endif

#endif
/*** End of inlined file: juce_Config.h ***/

#ifdef JUCE_NAMESPACE
  #define BEGIN_JUCE_NAMESPACE	namespace JUCE_NAMESPACE {
  #define END_JUCE_NAMESPACE	  }
#else
  #define BEGIN_JUCE_NAMESPACE
  #define END_JUCE_NAMESPACE
#endif


/*** Start of inlined file: juce_PlatformDefs.h ***/
#ifndef __JUCE_PLATFORMDEFS_JUCEHEADER__
#define __JUCE_PLATFORMDEFS_JUCEHEADER__

#ifdef JUCE_FORCE_DEBUG
  #undef JUCE_DEBUG

  #if JUCE_FORCE_DEBUG
	#define JUCE_DEBUG 1
  #endif
#endif

#if JUCE_MSVC
  #define JUCE_CALLTYPE		 __stdcall
#else
  #define JUCE_CALLTYPE
#endif

// Debugging and assertion macros

// (For info about JUCE_LOG_ASSERTIONS, have a look in juce_Config.h)
#if JUCE_LOG_ASSERTIONS
  #define juce_LogCurrentAssertion	juce_LogAssertion (__FILE__, __LINE__);
#elif defined (JUCE_DEBUG)
  #define juce_LogCurrentAssertion    std::cerr << "JUCE Assertion failure in " << __FILE__ << ", line " << __LINE__ << std::endl;
#else
  #define juce_LogCurrentAssertion
#endif

#ifdef JUCE_DEBUG

  // If debugging is enabled..

  #define DBG(dbgtext)		  Logger::outputDebugString (dbgtext);

  // Assertions..

  #if JUCE_WINDOWS || DOXYGEN

	#if JUCE_USE_INTRINSICS
	  #pragma intrinsic (__debugbreak)

	  #define juce_breakDebugger		__debugbreak();

	#elif JUCE_GCC
	  #define juce_breakDebugger            asm("int $3");
	#else
	  #define juce_breakDebugger		{ __asm int 3 }
	#endif
  #elif JUCE_MAC
	#define juce_breakDebugger		  Debugger();
  #elif JUCE_IPHONE
	#define juce_breakDebugger		  kill (0, SIGTRAP);
  #elif JUCE_LINUX
	#define juce_breakDebugger		  kill (0, SIGTRAP);
  #endif

  /** This will always cause an assertion failure.

	  It is only compiled in a debug build, (unless JUCE_LOG_ASSERTIONS is enabled
	  in juce_Config.h).

	  @see jassert()
  */
  #define jassertfalse		  { juce_LogCurrentAssertion; if (JUCE_NAMESPACE::juce_isRunningUnderDebugger()) juce_breakDebugger; }

  /** Platform-independent assertion macro.

	  This gets optimised out when not being built with debugging turned on.

	  Be careful not to call any functions within its arguments that are vital to
	  the behaviour of the program, because these won't get called in the release
	  build.

	  @see jassertfalse
  */
  #define jassert(expression)	   { if (! (expression)) jassertfalse }

#else

  // If debugging is disabled, these dummy debug and assertion macros are used..

  #define DBG(dbgtext)

  #define jassertfalse		  { juce_LogCurrentAssertion }

  #if JUCE_LOG_ASSERTIONS
	#define jassert(expression)	 { if (! (expression)) jassertfalse }
  #else
	#define jassert(a)		  { }
  #endif

#endif

#ifndef DOXYGEN
  template <bool b> struct JuceStaticAssert;
  template <> struct JuceStaticAssert <true> { static void dummy() {} };
#endif

/** A compile-time assertion macro.

	If the expression parameter is false, the macro will cause a compile error.
*/
#define static_jassert(expression)	  JuceStaticAssert<expression>::dummy();

#if JUCE_CATCH_UNHANDLED_EXCEPTIONS

  #define JUCE_TRY try

  #define JUCE_CATCH_EXCEPTION \
	catch (const std::exception& e)  \
	{ \
		JUCEApplication::sendUnhandledException (&e, __FILE__, __LINE__); \
	} \
	catch (...) \
	{ \
		JUCEApplication::sendUnhandledException (0, __FILE__, __LINE__); \
	}

  #define JUCE_CATCH_ALL		catch (...) {}
  #define JUCE_CATCH_ALL_ASSERT	 catch (...) { jassertfalse }

#else

  #define JUCE_TRY
  #define JUCE_CATCH_EXCEPTION
  #define JUCE_CATCH_ALL
  #define JUCE_CATCH_ALL_ASSERT

#endif

// Macros for inlining.

#if JUCE_MSVC
  #ifndef JUCE_DEBUG
	#define forcedinline  __forceinline
  #else
	#define forcedinline  inline
  #endif

#else
  #ifndef JUCE_DEBUG
	#define forcedinline  inline __attribute__((always_inline))
  #else
	#define forcedinline  inline
  #endif

#endif

#endif   // __JUCE_PLATFORMDEFS_JUCEHEADER__
/*** End of inlined file: juce_PlatformDefs.h ***/

// Now we'll include any OS headers we need.. (at this point we are outside the Juce namespace).
#if JUCE_MSVC
  #if (defined(_MSC_VER) && (_MSC_VER <= 1200))
	#pragma warning (disable: 4284 4786)  // (spurious VC6 warnings)
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

#if JUCE_USE_INTRINSICS
  #include <intrin.h>
#endif

#if JUCE_MAC || JUCE_IPHONE
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

// DLL building settings on Win32
#if JUCE_MSVC
  #ifdef JUCE_DLL_BUILD
	#define JUCE_API __declspec (dllexport)
	#pragma warning (disable: 4251)
  #elif defined (JUCE_DLL)
	#define JUCE_API __declspec (dllimport)
	#pragma warning (disable: 4251)
  #endif
#elif defined (__GNUC__) && ((__GNUC__ >= 4) || (__GNUC__ == 3 && __GNUC_MINOR__ >= 4))
  #ifdef JUCE_DLL_BUILD
	#define JUCE_API __attribute__ ((visibility("default")))
  #endif
#endif

#ifndef JUCE_API
  #define JUCE_API
#endif

#define JUCE_PUBLIC_FUNCTION	JUCE_API JUCE_CALLTYPE

// Now include some basics that are needed by most of the Juce classes...
BEGIN_JUCE_NAMESPACE

extern bool JUCE_PUBLIC_FUNCTION juce_isRunningUnderDebugger();

#if JUCE_LOG_ASSERTIONS
  extern void JUCE_API juce_LogAssertion (const char* filename, const int lineNum) throw();
#endif


/*** Start of inlined file: juce_Memory.h ***/
#ifndef __JUCE_MEMORY_JUCEHEADER__
#define __JUCE_MEMORY_JUCEHEADER__

#if defined (JUCE_DEBUG) && JUCE_MSVC && JUCE_CHECK_MEMORY_LEAKS
  #ifndef JUCE_DLL

	// Win32 debug non-DLL versions..

	#define juce_malloc(numBytes)		 _malloc_dbg  (numBytes, _NORMAL_BLOCK, __FILE__, __LINE__)

	#define juce_calloc(numBytes)		 _calloc_dbg  (1, numBytes, _NORMAL_BLOCK, __FILE__, __LINE__)

	#define juce_realloc(location, numBytes)	  _realloc_dbg (location, numBytes, _NORMAL_BLOCK, __FILE__, __LINE__)

	#define juce_free(location)		   _free_dbg	(location, _NORMAL_BLOCK)

  #else

	// Win32 debug DLL versions..

	// For the DLL, we'll define some functions in the DLL that will be used for allocation - that
	// way all juce calls in the DLL and in the host API will all use the same allocator.
	extern JUCE_API void* juce_DebugMalloc (const int size, const char* file, const int line);
	extern JUCE_API void* juce_DebugCalloc (const int size, const char* file, const int line);
	extern JUCE_API void* juce_DebugRealloc (void* const block, const int size, const char* file, const int line);
	extern JUCE_API void juce_DebugFree (void* const block);

	#define juce_malloc(numBytes)		 JUCE_NAMESPACE::juce_DebugMalloc (numBytes, __FILE__, __LINE__)

	#define juce_calloc(numBytes)		 JUCE_NAMESPACE::juce_DebugCalloc (numBytes, __FILE__, __LINE__)

	#define juce_realloc(location, numBytes)	  JUCE_NAMESPACE::juce_DebugRealloc (location, numBytes, __FILE__, __LINE__)

	#define juce_free(location)		   JUCE_NAMESPACE::juce_DebugFree (location)
  #endif

  #if ! defined (_AFXDLL)
	#define juce_UseDebuggingNewOperator \
	  static void* operator new (size_t sz)	   { void* const p = juce_malloc ((int) sz); return (p != 0) ? p : ::operator new (sz); } \
	  static void* operator new (size_t, void* p)	 { return p; } \
	  static void operator delete (void* p)	   { juce_free (p); } \
	  static void operator delete (void*, void*)	  { }
  #endif

#elif defined (JUCE_DLL)

  // Win32 DLL (release) versions..

  // For the DLL, we'll define some functions in the DLL that will be used for allocation - that
  // way all juce calls in the DLL and in the host API will all use the same allocator.
  extern JUCE_API void* juce_Malloc (const int size);
  extern JUCE_API void* juce_Calloc (const int size);
  extern JUCE_API void* juce_Realloc (void* const block, const int size);
  extern JUCE_API void juce_Free (void* const block);

  #define juce_malloc(numBytes)		 JUCE_NAMESPACE::juce_Malloc (numBytes)

  #define juce_calloc(numBytes)		 JUCE_NAMESPACE::juce_Calloc (numBytes)

  #define juce_realloc(location, numBytes)	  JUCE_NAMESPACE::juce_Realloc (location, numBytes)

  #define juce_free(location)		   JUCE_NAMESPACE::juce_Free (location)

  #define juce_UseDebuggingNewOperator \
	static void* operator new (size_t sz)	   { void* const p = juce_malloc ((int) sz); return (p != 0) ? p : ::operator new (sz); } \
	static void* operator new (size_t, void* p)	 { return p; } \
	static void operator delete (void* p)	   { juce_free (p); } \
	static void operator delete (void*, void*)	  { }

#else

  // Mac, Linux and Win32 (release) versions..

  #define juce_malloc(numBytes)		 malloc (numBytes)

  #define juce_calloc(numBytes)		 calloc (1, numBytes)

  #define juce_realloc(location, numBytes)	  realloc (location, numBytes)

  #define juce_free(location)		   free (location)

#endif

#ifndef juce_UseDebuggingNewOperator
  #define juce_UseDebuggingNewOperator
#endif

#if JUCE_MSVC
  #define juce_ThreadLocal	__declspec(thread)
#else
  #define juce_ThreadLocal	__thread
#endif

#if JUCE_MINGW
  #define alloca		  __builtin_alloca
#endif

inline void zeromem (void* memory, size_t numBytes)		 { memset (memory, 0, numBytes); }

template <typename Type>
inline void zerostruct (Type& structure)			{ memset (&structure, 0, sizeof (structure)); }

template <typename Type>
inline void deleteAndZero (Type& pointer)			   { delete pointer; pointer = 0; }

#endif   // __JUCE_MEMORY_JUCEHEADER__
/*** End of inlined file: juce_Memory.h ***/


/*** Start of inlined file: juce_MathsFunctions.h ***/
#ifndef __JUCE_MATHSFUNCTIONS_JUCEHEADER__
#define __JUCE_MATHSFUNCTIONS_JUCEHEADER__

// Definitions for the int8, int16, int32, int64 and pointer_sized_int types.

typedef signed char		 int8;
typedef unsigned char		   uint8;
typedef signed short		int16;
typedef unsigned short		  uint16;
typedef signed int		  int32;
typedef unsigned int		uint32;

#if JUCE_MSVC
  typedef __int64		   int64;
  typedef unsigned __int64	  uint64;
  #define literal64bit(longLiteral)	 ((__int64) longLiteral)
#else
  typedef long long		 int64;
  typedef unsigned long long	uint64;
  #define literal64bit(longLiteral)	 (longLiteral##LL)
#endif

#if JUCE_64BIT
  typedef int64			 pointer_sized_int;
  typedef uint64			pointer_sized_uint;
#elif _MSC_VER >= 1300
  typedef _W64 int		  pointer_sized_int;
  typedef _W64 unsigned int	 pointer_sized_uint;
#else
  typedef int			   pointer_sized_int;
  typedef unsigned int		  pointer_sized_uint;
#endif

typedef wchar_t			 juce_wchar;

// Some indispensible min/max functions

template <typename Type>
inline Type jmax (const Type a, const Type b)						   { return (a < b) ? b : a; }

template <typename Type>
inline Type jmax (const Type a, const Type b, const Type c)				 { return (a < b) ? ((b < c) ? c : b) : ((a < c) ? c : a); }

template <typename Type>
inline Type jmax (const Type a, const Type b, const Type c, const Type d)		   { return jmax (a, jmax (b, c, d)); }

template <typename Type>
inline Type jmin (const Type a, const Type b)						   { return (b < a) ? b : a; }

template <typename Type>
inline Type jmin (const Type a, const Type b, const Type c)				 { return (b < a) ? ((c < b) ? c : b) : ((c < a) ? c : a); }

template <typename Type>
inline Type jmin (const Type a, const Type b, const Type c, const Type d)		   { return jmin (a, jmin (b, c, d)); }

template <typename Type>
inline Type jlimit (const Type lowerLimit,
					const Type upperLimit,
					const Type valueToConstrain) throw()
{
	jassert (lowerLimit <= upperLimit); // if these are in the wrong order, results are unpredictable..

	return (valueToConstrain < lowerLimit) ? lowerLimit
										   : ((upperLimit < valueToConstrain) ? upperLimit
																			  : valueToConstrain);
}

template <typename Type>
inline void swapVariables (Type& variable1, Type& variable2)
{
	const Type tempVal = variable1;
	variable1 = variable2;
	variable2 = tempVal;
}

template <typename Type>
inline int numElementsInArray (Type& array)	 { return static_cast<int> (sizeof (array) / sizeof (array[0])); }

// Some useful maths functions that aren't always present with all compilers and build settings.

inline double juce_hypot (double a, double b)
{
  #if JUCE_WINDOWS
	return _hypot (a, b);
  #else
	return hypot (a, b);
  #endif
}

inline float juce_hypotf (float a, float b)
{
  #if JUCE_WINDOWS
	return (float) _hypot (a, b);
  #else
	return hypotf (a, b);
  #endif
}

inline int64 abs64 (const int64 n)
{
	return (n >= 0) ? n : -n;
}

const double  double_Pi  = 3.1415926535897932384626433832795;

const float   float_Pi   = 3.14159265358979323846f;

template <typename FloatingPointType>
inline bool juce_isfinite (FloatingPointType value)
{
	#if JUCE_WINDOWS
	  return _finite (value);
	#else
	  return std::isfinite (value);
	#endif
}

template <typename FloatType>
inline int roundToInt (const FloatType value) throw()
{
	union { int asInt[2]; double asDouble; } n;
	n.asDouble = ((double) value) + 6755399441055744.0;

	#if JUCE_BIG_ENDIAN
	  return n.asInt [1];
	#else
	  return n.asInt [0];
	#endif
}

inline int roundToIntAccurate (const double value) throw()
{
	return roundToInt (value + 1.5e-8);
}

inline int roundDoubleToInt (const double value) throw()
{
	return roundToInt (value);
}

inline int roundFloatToInt (const float value) throw()
{
	return roundToInt (value);
}

namespace TypeHelpers
{
#if defined (_MSC_VER) && _MSC_VER <= 1400
	#define PARAMETER_TYPE(a) a
#else
	template <typename Type> struct ParameterType		   { typedef const Type& type; };
	template <typename Type> struct ParameterType <Type&>	   { typedef Type& type; };
	template <typename Type> struct ParameterType <Type*>	   { typedef Type* type; };
	template <>		  struct ParameterType <char>		{ typedef char type; };
	template <>		  struct ParameterType <unsigned char>   { typedef unsigned char type; };
	template <>		  struct ParameterType <short>	   { typedef short type; };
	template <>		  struct ParameterType <unsigned short>  { typedef unsigned short type; };
	template <>		  struct ParameterType <int>		 { typedef int type; };
	template <>		  struct ParameterType <unsigned int>	{ typedef unsigned int type; };
	template <>		  struct ParameterType <long>		{ typedef long type; };
	template <>		  struct ParameterType <unsigned long>   { typedef unsigned long type; };
	template <>		  struct ParameterType <int64>	   { typedef int64 type; };
	template <>		  struct ParameterType <uint64>	  { typedef uint64 type; };
	template <>		  struct ParameterType <bool>		{ typedef bool type; };
	template <>		  struct ParameterType <float>	   { typedef float type; };
	template <>		  struct ParameterType <double>	  { typedef double type; };

	#define PARAMETER_TYPE(a)	typename TypeHelpers::ParameterType<a>::type
#endif
}

#endif   // __JUCE_MATHSFUNCTIONS_JUCEHEADER__
/*** End of inlined file: juce_MathsFunctions.h ***/


/*** Start of inlined file: juce_ByteOrder.h ***/
#ifndef __JUCE_BYTEORDER_JUCEHEADER__
#define __JUCE_BYTEORDER_JUCEHEADER__

class JUCE_API  ByteOrder
{
public:

	static uint16 swap (uint16 value);

	static uint32 swap (uint32 value);

	static uint64 swap (uint64 value);

	static uint16 swapIfBigEndian (uint16 value);

	static uint32 swapIfBigEndian (uint32 value);

	static uint64 swapIfBigEndian (uint64 value);

	static uint16 swapIfLittleEndian (uint16 value);

	static uint32 swapIfLittleEndian (uint32 value);

	static uint64 swapIfLittleEndian (uint64 value);

	static uint32 littleEndianInt (const void* bytes);

	static uint16 littleEndianShort (const void* bytes);

	static uint32 bigEndianInt (const void* bytes);

	static uint16 bigEndianShort (const void* bytes);

	static int littleEndian24Bit (const char* bytes);

	static int bigEndian24Bit (const char* bytes);

	static void littleEndian24BitToChars (int value, char* destBytes);

	static void bigEndian24BitToChars (int value, char* destBytes);

	static bool isBigEndian();

private:
	ByteOrder();
	ByteOrder (const ByteOrder&);
	ByteOrder& operator= (const ByteOrder&);
};

#if JUCE_USE_INTRINSICS
  #pragma intrinsic (_byteswap_ulong)
#endif

inline uint16 ByteOrder::swap (uint16 n)
{
#if JUCE_USE_INTRINSICSxxx // agh - the MS compiler has an internal error when you try to use this intrinsic!
	return static_cast <uint16> (_byteswap_ushort (n));
#else
	return static_cast <uint16> ((n << 8) | (n >> 8));
#endif
}

inline uint32 ByteOrder::swap (uint32 n)
{
#if JUCE_MAC || JUCE_IPHONE
	return OSSwapInt32 (n);
#elif JUCE_GCC
	asm("bswap %%eax" : "=a"(n) : "a"(n));
	return n;
#elif JUCE_USE_INTRINSICS
	return _byteswap_ulong (n);
#else
	__asm {
		mov eax, n
		bswap eax
		mov n, eax
	}
	return n;
#endif
}

inline uint64 ByteOrder::swap (uint64 value)
{
#if JUCE_MAC || JUCE_IPHONE
	return OSSwapInt64 (value);
#elif JUCE_USE_INTRINSICS
	return _byteswap_uint64 (value);
#else
	return (((int64) swap ((uint32) value)) << 32) | swap ((uint32) (value >> 32));
#endif
}

#if JUCE_LITTLE_ENDIAN
 inline uint16 ByteOrder::swapIfBigEndian (const uint16 v)				  { return v; }
 inline uint32 ByteOrder::swapIfBigEndian (const uint32 v)				  { return v; }
 inline uint64 ByteOrder::swapIfBigEndian (const uint64 v)				  { return v; }
 inline uint16 ByteOrder::swapIfLittleEndian (const uint16 v)				   { return swap (v); }
 inline uint32 ByteOrder::swapIfLittleEndian (const uint32 v)				   { return swap (v); }
 inline uint64 ByteOrder::swapIfLittleEndian (const uint64 v)				   { return swap (v); }
 inline uint32 ByteOrder::littleEndianInt (const void* const bytes)			 { return *static_cast <const uint32*> (bytes); }
 inline uint16 ByteOrder::littleEndianShort (const void* const bytes)			   { return *static_cast <const uint16*> (bytes); }
 inline uint32 ByteOrder::bigEndianInt (const void* const bytes)				{ return swap (*static_cast <const uint32*> (bytes)); }
 inline uint16 ByteOrder::bigEndianShort (const void* const bytes)			  { return swap (*static_cast <const uint16*> (bytes)); }
 inline bool ByteOrder::isBigEndian()							   { return false; }
#else
 inline uint16 ByteOrder::swapIfBigEndian (const uint16 v)				  { return swap (v); }
 inline uint32 ByteOrder::swapIfBigEndian (const uint32 v)				  { return swap (v); }
 inline uint64 ByteOrder::swapIfBigEndian (const uint64 v)				  { return swap (v); }
 inline uint16 ByteOrder::swapIfLittleEndian (const uint16 v)				   { return v; }
 inline uint32 ByteOrder::swapIfLittleEndian (const uint32 v)				   { return v; }
 inline uint64 ByteOrder::swapIfLittleEndian (const uint64 v)				   { return v; }
 inline uint32 ByteOrder::littleEndianInt (const void* const bytes)			 { return swap (*static_cast <const uint32*> (bytes)); }
 inline uint16 ByteOrder::littleEndianShort (const void* const bytes)			   { return swap (*static_cast <const uint16*> (bytes)); }
 inline uint32 ByteOrder::bigEndianInt (const void* const bytes)				{ return *static_cast <const uint32*> (bytes); }
 inline uint16 ByteOrder::bigEndianShort (const void* const bytes)			  { return *static_cast <const uint16*> (bytes); }
 inline bool ByteOrder::isBigEndian()							   { return true; }
#endif

inline int  ByteOrder::littleEndian24Bit (const char* const bytes)			  { return (((int) bytes[2]) << 16) | (((uint32) (uint8) bytes[1]) << 8) | ((uint32) (uint8) bytes[0]); }
inline int  ByteOrder::bigEndian24Bit (const char* const bytes)				 { return (((int) bytes[0]) << 16) | (((uint32) (uint8) bytes[1]) << 8) | ((uint32) (uint8) bytes[2]); }
inline void ByteOrder::littleEndian24BitToChars (const int value, char* const destBytes)	{ destBytes[0] = (char)(value & 0xff); destBytes[1] = (char)((value >> 8) & 0xff); destBytes[2] = (char)((value >> 16) & 0xff); }
inline void ByteOrder::bigEndian24BitToChars (const int value, char* const destBytes)	   { destBytes[0] = (char)((value >> 16) & 0xff); destBytes[1] = (char)((value >> 8) & 0xff); destBytes[2] = (char)(value & 0xff); }

#endif   // __JUCE_BYTEORDER_JUCEHEADER__
/*** End of inlined file: juce_ByteOrder.h ***/


/*** Start of inlined file: juce_Logger.h ***/
#ifndef __JUCE_LOGGER_JUCEHEADER__
#define __JUCE_LOGGER_JUCEHEADER__


/*** Start of inlined file: juce_String.h ***/
#ifndef __JUCE_STRING_JUCEHEADER__
#define __JUCE_STRING_JUCEHEADER__


/*** Start of inlined file: juce_CharacterFunctions.h ***/
#ifndef __JUCE_CHARACTERFUNCTIONS_JUCEHEADER__
#define __JUCE_CHARACTERFUNCTIONS_JUCEHEADER__

#define JUCE_T(stringLiteral)	 (L##stringLiteral)
typedef juce_wchar		tchar;

#if ! JUCE_DONT_DEFINE_MACROS

#define T(stringLiteral)		JUCE_T(stringLiteral)

#endif

class JUCE_API  CharacterFunctions
{
public:
	static int length (const char* const s) throw();
	static int length (const juce_wchar* const s) throw();

	static void copy (char* dest, const char* src, const int maxBytes) throw();
	static void copy (juce_wchar* dest, const juce_wchar* src, const int maxChars) throw();

	static void copy (juce_wchar* dest, const char* src, const int maxChars) throw();
	static void copy (char* dest, const juce_wchar* src, const int maxBytes) throw();
	static int bytesRequiredForCopy (const juce_wchar* src) throw();

	static void append (char* dest, const char* src) throw();
	static void append (juce_wchar* dest, const juce_wchar* src) throw();

	static int compare (const char* const s1, const char* const s2) throw();
	static int compare (const juce_wchar* s1, const juce_wchar* s2) throw();
	static int compare (const juce_wchar* s1, const char* s2) throw();
	static int compare (const char* s1, const juce_wchar* s2) throw();

	static int compare (const char* const s1, const char* const s2, const int maxChars) throw();
	static int compare (const juce_wchar* s1, const juce_wchar* s2, int maxChars) throw();

	static int compareIgnoreCase (const char* const s1, const char* const s2) throw();
	static int compareIgnoreCase (const juce_wchar* s1, const juce_wchar* s2) throw();
	static int compareIgnoreCase (const juce_wchar* s1, const char* s2) throw();

	static int compareIgnoreCase (const char* const s1, const char* const s2, const int maxChars) throw();
	static int compareIgnoreCase (const juce_wchar* s1, const juce_wchar* s2, int maxChars) throw();

	static const char* find (const char* const haystack, const char* const needle) throw();
	static const juce_wchar* find (const juce_wchar* haystack, const juce_wchar* const needle) throw();

	static int indexOfChar (const char* const haystack, const char needle, const bool ignoreCase) throw();
	static int indexOfChar (const juce_wchar* const haystack, const juce_wchar needle, const bool ignoreCase) throw();

	static int indexOfCharFast (const char* const haystack, const char needle) throw();
	static int indexOfCharFast (const juce_wchar* const haystack, const juce_wchar needle) throw();

	static int getIntialSectionContainingOnly (const char* const text, const char* const allowedChars) throw();
	static int getIntialSectionContainingOnly (const juce_wchar* const text, const juce_wchar* const allowedChars) throw();

	static int ftime (char* const dest, const int maxChars, const char* const format, const struct tm* const tm) throw();
	static int ftime (juce_wchar* const dest, const int maxChars, const juce_wchar* const format, const struct tm* const tm) throw();

	static int getIntValue (const char* const s) throw();
	static int getIntValue (const juce_wchar* s) throw();

	static int64 getInt64Value (const char* s) throw();
	static int64 getInt64Value (const juce_wchar* s) throw();

	static double getDoubleValue (const char* const s) throw();
	static double getDoubleValue (const juce_wchar* const s) throw();

	static char toUpperCase (const char character) throw();
	static juce_wchar toUpperCase (const juce_wchar character) throw();
	static void toUpperCase (char* s) throw();

	static void toUpperCase (juce_wchar* s) throw();
	static bool isUpperCase (const char character) throw();
	static bool isUpperCase (const juce_wchar character) throw();

	static char toLowerCase (const char character) throw();
	static juce_wchar toLowerCase (const juce_wchar character) throw();
	static void toLowerCase (char* s) throw();
	static void toLowerCase (juce_wchar* s) throw();
	static bool isLowerCase (const char character) throw();
	static bool isLowerCase (const juce_wchar character) throw();

	static bool isWhitespace (const char character) throw();
	static bool isWhitespace (const juce_wchar character) throw();

	static bool isDigit (const char character) throw();
	static bool isDigit (const juce_wchar character) throw();

	static bool isLetter (const char character) throw();
	static bool isLetter (const juce_wchar character) throw();

	static bool isLetterOrDigit (const char character) throw();
	static bool isLetterOrDigit (const juce_wchar character) throw();

	static int getHexDigitValue (const juce_wchar digit) throw();
};

#endif   // __JUCE_CHARACTERFUNCTIONS_JUCEHEADER__
/*** End of inlined file: juce_CharacterFunctions.h ***/

class OutputStream;

class JUCE_API  String
{
public:

	String() throw();

	String (const String& other) throw();

	String (const char* text);

	String (const char* text, size_t maxChars);

	String (const juce_wchar* unicodeText);

	String (const juce_wchar* unicodeText, size_t maxChars);

	static const String charToString (juce_wchar character);

	~String() throw();

	//========================juce_wchar======================================================
	static const String empty;

	int hashCode() const throw();

	int64 hashCode64() const throw();

	int length() const throw();

	// Assignment and concatenation operators..

	String& operator= (const String& other) throw();

	String& operator+= (const juce_wchar* textToAppend);
	String& operator+= (const String& stringToAppend);
	String& operator+= (char characterToAppend);
	String& operator+= (juce_wchar characterToAppend);
	String& operator+= (int numberToAppend);
	String& operator+= (unsigned int numberToAppend);

	void append (const juce_wchar* textToAppend, int maxCharsToTake);

	// Comparison methods..

	inline bool isEmpty() const throw()			 { return text[0] == 0; }

	inline bool isNotEmpty() const throw()		  { return text[0] != 0; }

	bool equalsIgnoreCase (const String& other) const throw();

	bool equalsIgnoreCase (const juce_wchar* other) const throw();

	bool equalsIgnoreCase (const char* other) const throw();

	int compare (const String& other) const throw();

	int compare (const char* other) const throw();

	int compare (const juce_wchar* other) const throw();

	int compareIgnoreCase (const String& other) const throw();

	int compareLexicographically (const String& other) const throw();

	bool startsWith (const String& text) const throw();

	bool startsWithChar (juce_wchar character) const throw();

	bool startsWithIgnoreCase (const String& text) const throw();

	bool endsWith (const String& text) const throw();

	bool endsWithChar (juce_wchar character) const throw();

	bool endsWithIgnoreCase (const String& text) const throw();

	bool contains (const String& text) const throw();

	bool containsChar (juce_wchar character) const throw();

	bool containsIgnoreCase (const String& text) const throw();

	bool containsWholeWord (const String& wordToLookFor) const throw();

	bool containsWholeWordIgnoreCase (const String& wordToLookFor) const throw();

	int indexOfWholeWord (const String& wordToLookFor) const throw();

	int indexOfWholeWordIgnoreCase (const String& wordToLookFor) const throw();

	bool containsAnyOf (const String& charactersItMightContain) const throw();

	bool containsOnly (const String& charactersItMightContain) const throw();

	bool containsNonWhitespaceChars() const throw();

	bool matchesWildcard (const String& wildcard, bool ignoreCase) const throw();

	// Substring location methods..

	int indexOfChar (juce_wchar characterToLookFor) const throw();

	int indexOfChar (int startIndex, juce_wchar characterToLookFor) const throw();

	int indexOfAnyOf (const String& charactersToLookFor,
					  int startIndex = 0,
					  bool ignoreCase = false) const throw();

	int indexOf (const String& text) const throw();

	int indexOf (int startIndex,
				 const String& textToLookFor) const throw();

	int indexOfIgnoreCase (const String& textToLookFor) const throw();

	int indexOfIgnoreCase (int startIndex,
						   const String& textToLookFor) const throw();

	int lastIndexOfChar (juce_wchar character) const throw();

	int lastIndexOf (const String& textToLookFor) const throw();

	int lastIndexOfIgnoreCase (const String& textToLookFor) const throw();

	int lastIndexOfAnyOf (const String& charactersToLookFor,
						  bool ignoreCase = false) const throw();

	// Substring extraction and manipulation methods..

	/** Returns the character at this index in the string.

		No checks are made to see if the index is within a valid range, so be careful!
	*/
	inline const juce_wchar& operator[] (int index) const throw()  { jassert (((unsigned int) index) <= (unsigned int) length()); return text [index]; }

	juce_wchar& operator[] (int index);

	juce_wchar getLastCharacter() const throw();

	const String substring (int startIndex, int endIndex) const;

	const String substring (int startIndex) const;

	const String dropLastCharacters (int numberToDrop) const;

	const String getLastCharacters (int numCharacters) const;

	const String fromFirstOccurrenceOf (const String& substringToStartFrom,
										bool includeSubStringInResult,
										bool ignoreCase) const;

	const String fromLastOccurrenceOf (const String& substringToFind,
									   bool includeSubStringInResult,
									   bool ignoreCase) const;

	const String upToFirstOccurrenceOf (const String& substringToEndWith,
										bool includeSubStringInResult,
										bool ignoreCase) const;

	const String upToLastOccurrenceOf (const String& substringToFind,
									   bool includeSubStringInResult,
									   bool ignoreCase) const;

	const String trim() const;
	const String trimStart() const;
	const String trimEnd() const;

	const String trimCharactersAtStart (const String& charactersToTrim) const;

	const String trimCharactersAtEnd (const String& charactersToTrim) const;

	const String toUpperCase() const;

	const String toLowerCase() const;

	const String replaceSection (int startIndex,
								 int numCharactersToReplace,
								 const String& stringToInsert) const;

	const String replace (const String& stringToReplace,
						  const String& stringToInsertInstead,
						  bool ignoreCase = false) const;

	const String replaceCharacter (juce_wchar characterToReplace,
								   juce_wchar characterToInsertInstead) const;

	const String replaceCharacters (const String& charactersToReplace,
									const String& charactersToInsertInstead) const;

	const String retainCharacters (const String& charactersToRetain) const;

	const String removeCharacters (const String& charactersToRemove) const;

	const String initialSectionContainingOnly (const String& permittedCharacters) const;

	const String initialSectionNotContaining (const String& charactersToStopAt) const;

	bool isQuotedString() const;

	const String unquoted() const;

	const String quoted (juce_wchar quoteCharacter = '"') const;

	static const String repeatedString (const String& stringToRepeat,
										int numberOfTimesToRepeat);

	const String paddedLeft (juce_wchar padCharacter, int minimumLength) const;

	const String paddedRight (juce_wchar padCharacter, int minimumLength) const;

	static const String createStringFromData (const void* data, int size);

	static const String formatted (const juce_wchar* formatString, ... );

	// Numeric conversions..

	explicit String (int decimalInteger);

	explicit String (unsigned int decimalInteger);

	explicit String (short decimalInteger);

	explicit String (unsigned short decimalInteger);

	explicit String (int64 largeIntegerValue);

	explicit String (uint64 largeIntegerValue);

	explicit String (float floatValue,
					 int numberOfDecimalPlaces = 0);

	explicit String (double doubleValue,
					 int numberOfDecimalPlaces = 0);

	int getIntValue() const throw();

	int64 getLargeIntValue() const throw();

	int getTrailingIntValue() const throw();

	float getFloatValue() const throw();

	double getDoubleValue() const throw();

	int getHexValue32() const throw();

	int64 getHexValue64() const throw();

	static const String toHexString (int number);

	static const String toHexString (int64 number);

	static const String toHexString (short number);

	static const String toHexString (const unsigned char* data,
									 int size,
									 int groupSize = 1);

	inline operator const juce_wchar*() const throw()   { return text; }

	inline operator juce_wchar*() throw()		   { return text; }

	const char* toUTF8() const;

	static const String fromUTF8 (const char* utf8buffer, int bufferSizeBytes = -1);

	int getNumBytesAsUTF8() const throw();

	int copyToUTF8 (char* destBuffer, int maxBufferSizeBytes) const throw();

	const char* toCString() const;

	int getNumBytesAsCString() const throw();

	int copyToCString (char* destBuffer, int maxBufferSizeBytes) const throw();

	void copyToUnicode (juce_wchar* destBuffer, int maxCharsToCopy) const throw();

	void preallocateStorage (size_t numCharsNeeded);

	void swapWith (String& other) throw();

	class JUCE_API  Concatenator
	{
	public:
		Concatenator (String& stringToAppendTo);
		~Concatenator();

		void append (const String& s);

	private:
		String& result;
		int nextIndex;

		Concatenator (const Concatenator&);
		Concatenator& operator= (const Concatenator&);
	};

	juce_UseDebuggingNewOperator // (adds debugging info to find leaked objects)

private:

	juce_wchar* text;

	// internal constructor that preallocates a certain amount of memory
	String (size_t numChars, int dummyVariable);
	String (const String& stringToCopy, size_t charsToAllocate);

	void createInternal (const juce_wchar* text, size_t numChars);
	void appendInternal (const juce_wchar* text, int numExtraChars);
};

const String JUCE_CALLTYPE operator+  (const char* string1,	   const String& string2);
const String JUCE_CALLTYPE operator+  (const juce_wchar* string1, const String& string2);
const String JUCE_CALLTYPE operator+  (char string1,		  const String& string2);
const String JUCE_CALLTYPE operator+  (juce_wchar string1,	const String& string2);

const String JUCE_CALLTYPE operator+  (String string1, const String& string2);
const String JUCE_CALLTYPE operator+  (String string1, const char* string2);
const String JUCE_CALLTYPE operator+  (String string1, const juce_wchar* string2);
const String JUCE_CALLTYPE operator+  (String string1, char characterToAppend);
const String JUCE_CALLTYPE operator+  (String string1, juce_wchar characterToAppend);

String& JUCE_CALLTYPE operator<< (String& string1, char characterToAppend);
String& JUCE_CALLTYPE operator<< (String& string1, juce_wchar characterToAppend);
String& JUCE_CALLTYPE operator<< (String& string1, const char* string2);
String& JUCE_CALLTYPE operator<< (String& string1, const juce_wchar* string2);
String& JUCE_CALLTYPE operator<< (String& string1, const String& string2);

String& JUCE_CALLTYPE operator<< (String& string1, short number);
String& JUCE_CALLTYPE operator<< (String& string1, int number);
String& JUCE_CALLTYPE operator<< (String& string1, unsigned int number);
String& JUCE_CALLTYPE operator<< (String& string1, long number);
String& JUCE_CALLTYPE operator<< (String& string1, unsigned long number);
String& JUCE_CALLTYPE operator<< (String& string1, float number);
String& JUCE_CALLTYPE operator<< (String& string1, double number);

bool JUCE_CALLTYPE operator== (const String& string1, const String& string2) throw();
bool JUCE_CALLTYPE operator== (const String& string1, const char* string2) throw();
bool JUCE_CALLTYPE operator== (const String& string1, const juce_wchar* string2) throw();
bool JUCE_CALLTYPE operator!= (const String& string1, const String& string2) throw();
bool JUCE_CALLTYPE operator!= (const String& string1, const char* string2) throw();
bool JUCE_CALLTYPE operator!= (const String& string1, const juce_wchar* string2) throw();
bool JUCE_CALLTYPE operator>  (const String& string1, const String& string2) throw();
bool JUCE_CALLTYPE operator<  (const String& string1, const String& string2) throw();
bool JUCE_CALLTYPE operator>= (const String& string1, const String& string2) throw();
bool JUCE_CALLTYPE operator<= (const String& string1, const String& string2) throw();

template <class charT, class traits>
std::basic_ostream <charT, traits>& JUCE_CALLTYPE operator<< (std::basic_ostream <charT, traits>& stream, const String& stringToWrite)
{
	return stream << stringToWrite.toUTF8();
}

OutputStream& JUCE_CALLTYPE operator<< (OutputStream& stream, const String& text);

#endif   // __JUCE_STRING_JUCEHEADER__
/*** End of inlined file: juce_String.h ***/

class JUCE_API  Logger
{
public:

	virtual ~Logger();

	static void JUCE_CALLTYPE setCurrentLogger (Logger* const newLogger,
												const bool deleteOldLogger = false);

	static void JUCE_CALLTYPE writeToLog (const String& message);

	static void JUCE_CALLTYPE outputDebugString (const String& text) throw();

protected:

	Logger();

	virtual void logMessage (const String& message) = 0;
};

#endif   // __JUCE_LOGGER_JUCEHEADER__
/*** End of inlined file: juce_Logger.h ***/

END_JUCE_NAMESPACE

#endif   // __JUCE_STANDARDHEADER_JUCEHEADER__
/*** End of inlined file: juce_StandardHeader.h ***/


BEGIN_JUCE_NAMESPACE

#if JUCE_MSVC
  // this is set explicitly in case the app is using a different packing size.
  #pragma pack (push, 8)
  #pragma warning (push)
  #pragma warning (disable: 4786) // (old vc6 warning about long class names)
#endif

// this is where all the class header files get brought in..

/*** Start of inlined file: juce_core_includes.h ***/
#ifndef __JUCE_JUCE_CORE_INCLUDES_INCLUDEFILES__
#define __JUCE_JUCE_CORE_INCLUDES_INCLUDEFILES__

#ifndef __JUCE_ARRAY_JUCEHEADER__

/*** Start of inlined file: juce_Array.h ***/
#ifndef __JUCE_ARRAY_JUCEHEADER__
#define __JUCE_ARRAY_JUCEHEADER__


/*** Start of inlined file: juce_ArrayAllocationBase.h ***/
#ifndef __JUCE_ARRAYALLOCATIONBASE_JUCEHEADER__
#define __JUCE_ARRAYALLOCATIONBASE_JUCEHEADER__


/*** Start of inlined file: juce_HeapBlock.h ***/
#ifndef __JUCE_HEAPBLOCK_JUCEHEADER__
#define __JUCE_HEAPBLOCK_JUCEHEADER__

template <class ElementType>
class HeapBlock
{
public:

	HeapBlock() throw() : data (0)
	{
	}

	explicit HeapBlock (const size_t numElements)
		: data (static_cast <ElementType*> (::juce_malloc (numElements * sizeof (ElementType))))
	{
	}

	~HeapBlock()
	{
		::juce_free (data);
	}

	inline operator ElementType*() const throw()				{ return data; }

	inline ElementType* getData() const throw()				 { return data; }

	inline operator void*() const throw()				   { return static_cast <void*> (data); }

	inline ElementType* operator->() const  throw()			 { return data; }

	template <typename IndexType>
	inline ElementType& operator[] (IndexType index) const throw()	  { return data [index]; }

	template <typename IndexType>
	inline ElementType* operator+ (IndexType index) const throw()	   { return data + index; }

	inline ElementType* const* operator&() const throw()			{ return static_cast <ElementType* const*> (&data); }

	inline ElementType** operator&() throw()				{ return static_cast <ElementType**> (&data); }

	inline bool operator== (const ElementType* const otherPointer) const throw()	{ return otherPointer == data; }

	inline bool operator!= (const ElementType* const otherPointer) const throw()	{ return otherPointer != data; }

	void malloc (const size_t newNumElements, const size_t elementSize = sizeof (ElementType))
	{
		::juce_free (data);
		data = static_cast <ElementType*> (::juce_malloc (newNumElements * elementSize));
	}

	void calloc (const size_t newNumElements, const size_t elementSize = sizeof (ElementType))
	{
		::juce_free (data);
		data = static_cast <ElementType*> (::juce_calloc (newNumElements * elementSize));
	}

	void allocate (const size_t newNumElements, const bool initialiseToZero)
	{
		::juce_free (data);

		if (initialiseToZero)
			data = static_cast <ElementType*> (::juce_calloc (newNumElements * sizeof (ElementType)));
		else
			data = static_cast <ElementType*> (::juce_malloc (newNumElements * sizeof (ElementType)));
	}

	void realloc (const size_t newNumElements, const size_t elementSize = sizeof (ElementType))
	{
		if (data == 0)
			data = static_cast <ElementType*> (::juce_malloc (newNumElements * elementSize));
		else
			data = static_cast <ElementType*> (::juce_realloc (data, newNumElements * elementSize));
	}

	void free()
	{
		::juce_free (data);
		data = 0;
	}

	void swapWith (HeapBlock <ElementType>& other) throw()
	{
		swapVariables (data, other.data);
	}

private:

	ElementType* data;

	HeapBlock (const HeapBlock&);
	HeapBlock& operator= (const HeapBlock&);
};

#endif   // __JUCE_HEAPBLOCK_JUCEHEADER__
/*** End of inlined file: juce_HeapBlock.h ***/

template <class ElementType, class TypeOfCriticalSectionToUse>
class ArrayAllocationBase  : public TypeOfCriticalSectionToUse
{
public:

	ArrayAllocationBase() throw()
		: numAllocated (0)
	{
	}

	~ArrayAllocationBase()
	{
	}

	void setAllocatedSize (const int numElements)
	{
		if (numAllocated != numElements)
		{
			if (numElements > 0)
				elements.realloc (numElements);
			else
				elements.free();

			numAllocated = numElements;
		}
	}

	void ensureAllocatedSize (const int minNumElements)
	{
		if (minNumElements > numAllocated)
			setAllocatedSize ((minNumElements + minNumElements / 2 + 8) & ~7);
	}

	void shrinkToNoMoreThan (const int maxNumElements)
	{
		if (maxNumElements < numAllocated)
			setAllocatedSize (maxNumElements);
	}

	void swapWith (ArrayAllocationBase <ElementType, TypeOfCriticalSectionToUse>& other) throw()
	{
		elements.swapWith (other.elements);
		swapVariables (numAllocated, other.numAllocated);
	}

	HeapBlock <ElementType> elements;
	int numAllocated;

private:
	ArrayAllocationBase (const ArrayAllocationBase&);
	ArrayAllocationBase& operator= (const ArrayAllocationBase&);
};

#endif   // __JUCE_ARRAYALLOCATIONBASE_JUCEHEADER__
/*** End of inlined file: juce_ArrayAllocationBase.h ***/


/*** Start of inlined file: juce_ElementComparator.h ***/
#ifndef __JUCE_ELEMENTCOMPARATOR_JUCEHEADER__
#define __JUCE_ELEMENTCOMPARATOR_JUCEHEADER__

template <class ElementType, class ElementComparator>
static void sortArray (ElementComparator& comparator,
					   ElementType* const array,
					   int firstElement,
					   int lastElement,
					   const bool retainOrderOfEquivalentItems)
{
	(void) comparator;  // if you pass in an object with a static compareElements() method, this
						// avoids getting warning messages about the parameter being unused

	if (lastElement > firstElement)
	{
		if (retainOrderOfEquivalentItems)
		{
			for (int i = firstElement; i < lastElement; ++i)
			{
				if (comparator.compareElements (array[i], array [i + 1]) > 0)
				{
					const ElementType temp = array [i];
					array [i] = array[i + 1];
					array [i + 1] = temp;

					if (i > firstElement)
						i -= 2;
				}
			}
		}
		else
		{
			int fromStack[30], toStack[30];
			int stackIndex = 0;

			for (;;)
			{
				const int size = (lastElement - firstElement) + 1;

				if (size <= 8)
				{
					int j = lastElement;
					int maxIndex;

					while (j > firstElement)
					{
						maxIndex = firstElement;
						for (int k = firstElement + 1; k <= j; ++k)
							if (comparator.compareElements (array[k], array [maxIndex]) > 0)
								maxIndex = k;

						const ElementType temp = array [maxIndex];
						array [maxIndex] = array[j];
						array [j] = temp;

						--j;
					}
				}
				else
				{
					const int mid = firstElement + (size >> 1);
					ElementType temp = array [mid];
					array [mid] = array [firstElement];
					array [firstElement] = temp;

					int i = firstElement;
					int j = lastElement + 1;

					for (;;)
					{
						while (++i <= lastElement
								&& comparator.compareElements (array[i], array [firstElement]) <= 0)
						{}

						while (--j > firstElement
								&& comparator.compareElements (array[j], array [firstElement]) >= 0)
						{}

						if (j < i)
							break;

						temp = array[i];
						array[i] = array[j];
						array[j] = temp;
					}

					temp = array [firstElement];
					array [firstElement] = array[j];
					array [j] = temp;

					if (j - 1 - firstElement >= lastElement - i)
					{
						if (firstElement + 1 < j)
						{
							fromStack [stackIndex] = firstElement;
							toStack [stackIndex] = j - 1;
							++stackIndex;
						}

						if (i < lastElement)
						{
							firstElement = i;
							continue;
						}
					}
					else
					{
						if (i < lastElement)
						{
							fromStack [stackIndex] = i;
							toStack [stackIndex] = lastElement;
							++stackIndex;
						}

						if (firstElement + 1 < j)
						{
							lastElement = j - 1;
							continue;
						}
					}
				}

				if (--stackIndex < 0)
					break;

				jassert (stackIndex < numElementsInArray (fromStack));

				firstElement = fromStack [stackIndex];
				lastElement = toStack [stackIndex];
			}
		}
	}
}

template <class ElementType, class ElementComparator>
static int findInsertIndexInSortedArray (ElementComparator& comparator,
										 ElementType* const array,
										 const ElementType newElement,
										 int firstElement,
										 int lastElement)
{
	jassert (firstElement <= lastElement);

	(void) comparator;  // if you pass in an object with a static compareElements() method, this
						// avoids getting warning messages about the parameter being unused

	while (firstElement < lastElement)
	{
		if (comparator.compareElements (newElement, array [firstElement]) == 0)
		{
			++firstElement;
			break;
		}
		else
		{
			const int halfway = (firstElement + lastElement) >> 1;

			if (halfway == firstElement)
			{
				if (comparator.compareElements (newElement, array [halfway]) >= 0)
					++firstElement;

				break;
			}
			else if (comparator.compareElements (newElement, array [halfway]) >= 0)
			{
				firstElement = halfway;
			}
			else
			{
				lastElement = halfway;
			}
		}
	}

	return firstElement;
}

template <class ElementType>
class IntegerElementComparator
{
public:
	static int compareElements (const ElementType first,
								const ElementType second) throw()
	{
		return (first < second) ? -1 : ((first == second) ? 0 : 1);
	}
};

template <class ElementType>
class FloatElementComparator
{
public:
	static int compareElements (const ElementType first,
								const ElementType second) throw()
	{
		return (first < second) ? -1 : ((first == second) ? 0 : 1);
	}
};

#endif   // __JUCE_ELEMENTCOMPARATOR_JUCEHEADER__
/*** End of inlined file: juce_ElementComparator.h ***/


/*** Start of inlined file: juce_CriticalSection.h ***/
#ifndef __JUCE_CRITICALSECTION_JUCEHEADER__
#define __JUCE_CRITICALSECTION_JUCEHEADER__

class JUCE_API  ScopedLock;
class JUCE_API  ScopedUnlock;

class JUCE_API  CriticalSection
{
public:

	CriticalSection() throw();

	~CriticalSection() throw();

	void enter() const throw();

	bool tryEnter() const throw();

	void exit() const throw();

	typedef ScopedLock	  ScopedLockType;

	typedef ScopedUnlock	ScopedUnlockType;

	juce_UseDebuggingNewOperator

private:

#if JUCE_WIN32
  #if JUCE_64BIT
	// To avoid including windows.h in the public Juce includes, we'll just allocate a
	// block of memory here that's big enough to be used internally as a windows critical
	// section object.
	uint8 internal [44];
  #else
	uint8 internal [24];
  #endif
#else
	mutable pthread_mutex_t internal;
#endif

	CriticalSection (const CriticalSection&);
	CriticalSection& operator= (const CriticalSection&);
};

class JUCE_API  DummyCriticalSection
{
public:
	inline DummyCriticalSection() throw()	 {}
	inline ~DummyCriticalSection() throw()	{}

	inline void enter() const throw()		 {}
	inline void exit() const throw()		  {}

	struct ScopedLockType
	{
		ScopedLockType (const DummyCriticalSection&) throw() {}
	};

	typedef ScopedLockType ScopedUnlockType;

private:
	DummyCriticalSection (const DummyCriticalSection&);
	DummyCriticalSection& operator= (const DummyCriticalSection&);
};

#endif   // __JUCE_CRITICALSECTION_JUCEHEADER__
/*** End of inlined file: juce_CriticalSection.h ***/

template <typename ElementType,
		  typename TypeOfCriticalSectionToUse = DummyCriticalSection>
class Array
{
private:
  #if defined (_MSC_VER) && _MSC_VER <= 1400
	typedef const ElementType& ParameterType;
  #else
	typedef PARAMETER_TYPE (ElementType) ParameterType;
  #endif

public:

	Array() throw()
	   : numUsed (0)
	{
	}

	Array (const Array<ElementType, TypeOfCriticalSectionToUse>& other)
	{
		const ScopedLockType lock (other.getLock());
		numUsed = other.numUsed;
		data.setAllocatedSize (other.numUsed);

		for (int i = 0; i < numUsed; ++i)
			new (data.elements + i) ElementType (other.data.elements[i]);
	}

	explicit Array (const ElementType* values)
	   : numUsed (0)
	{
		while (*values != 0)
			add (*values++);
	}

	Array (const ElementType* values, int numValues)
	   : numUsed (numValues)
	{
		data.setAllocatedSize (numValues);

		for (int i = 0; i < numValues; ++i)
			new (data.elements + i) ElementType (values[i]);
	}

	~Array()
	{
		for (int i = 0; i < numUsed; ++i)
			data.elements[i].~ElementType();
	}

	Array& operator= (const Array& other)
	{
		if (this != &other)
		{
			Array<ElementType, TypeOfCriticalSectionToUse> otherCopy (other);
			swapWithArray (otherCopy);
		}

		return *this;
	}

	template <class OtherArrayType>
	bool operator== (const OtherArrayType& other) const
	{
		const ScopedLockType lock (getLock());

		if (numUsed != other.numUsed)
			return false;

		for (int i = numUsed; --i >= 0;)
			if (data.elements [i] != other.data.elements [i])
				return false;

		return true;
	}

	template <class OtherArrayType>
	bool operator!= (const OtherArrayType& other) const
	{
		return ! operator== (other);
	}

	void clear()
	{
		const ScopedLockType lock (getLock());

		for (int i = 0; i < numUsed; ++i)
			data.elements[i].~ElementType();

		data.setAllocatedSize (0);
		numUsed = 0;
	}

	void clearQuick()
	{
		const ScopedLockType lock (getLock());

		for (int i = 0; i < numUsed; ++i)
			data.elements[i].~ElementType();

		numUsed = 0;
	}

	inline int size() const throw()
	{
		return numUsed;
	}

	inline ElementType operator[] (const int index) const
	{
		const ScopedLockType lock (getLock());
		return (((unsigned int) index) < (unsigned int) numUsed) ? data.elements [index]
																 : ElementType();
	}

	inline const ElementType getUnchecked (const int index) const
	{
		const ScopedLockType lock (getLock());
		jassert (((unsigned int) index) < (unsigned int) numUsed);
		return data.elements [index];
	}

	inline ElementType& getReference (const int index) const throw()
	{
		const ScopedLockType lock (getLock());
		jassert (((unsigned int) index) < (unsigned int) numUsed);
		return data.elements [index];
	}

	inline ElementType getFirst() const
	{
		const ScopedLockType lock (getLock());
		return (numUsed > 0) ? data.elements [0]
							 : ElementType();
	}

	inline ElementType getLast() const
	{
		const ScopedLockType lock (getLock());
		return (numUsed > 0) ? data.elements [numUsed - 1]
							 : ElementType();
	}

	int indexOf (ParameterType elementToLookFor) const
	{
		const ScopedLockType lock (getLock());
		const ElementType* e = data.elements.getData();
		const ElementType* const end = e + numUsed;

		while (e != end)
		{
			if (elementToLookFor == *e)
				return static_cast <int> (e - data.elements.getData());

			++e;
		}

		return -1;
	}

	bool contains (ParameterType elementToLookFor) const
	{
		const ScopedLockType lock (getLock());
		const ElementType* e = data.elements.getData();
		const ElementType* const end = e + numUsed;

		while (e != end)
		{
			if (elementToLookFor == *e)
				return true;

			++e;
		}

		return false;
	}

	void add (ParameterType newElement)
	{
		const ScopedLockType lock (getLock());
		data.ensureAllocatedSize (numUsed + 1);
		new (data.elements + numUsed++) ElementType (newElement);
	}

	void insert (int indexToInsertAt, ParameterType newElement)
	{
		const ScopedLockType lock (getLock());
		data.ensureAllocatedSize (numUsed + 1);

		if (((unsigned int) indexToInsertAt) < (unsigned int) numUsed)
		{
			ElementType* const insertPos = data.elements + indexToInsertAt;
			const int numberToMove = numUsed - indexToInsertAt;

			if (numberToMove > 0)
				memmove (insertPos + 1, insertPos, numberToMove * sizeof (ElementType));

			new (insertPos) ElementType (newElement);
			++numUsed;
		}
		else
		{
			new (data.elements + numUsed++) ElementType (newElement);
		}
	}

	void insertMultiple (int indexToInsertAt, ParameterType newElement,
						 int numberOfTimesToInsertIt)
	{
		if (numberOfTimesToInsertIt > 0)
		{
			const ScopedLockType lock (getLock());
			data.ensureAllocatedSize (numUsed + numberOfTimesToInsertIt);
			ElementType* insertPos;

			if (((unsigned int) indexToInsertAt) < (unsigned int) numUsed)
			{
				insertPos = data.elements + indexToInsertAt;
				const int numberToMove = numUsed - indexToInsertAt;
				memmove (insertPos + numberOfTimesToInsertIt, insertPos, numberToMove * sizeof (ElementType));
			}
			else
			{
				insertPos = data.elements + numUsed;
			}

			numUsed += numberOfTimesToInsertIt;

			while (--numberOfTimesToInsertIt >= 0)
				new (insertPos++) ElementType (newElement);
		}
	}

	void insertArray (int indexToInsertAt,
					  const ElementType* newElements,
					  int numberOfElements)
	{
		if (numberOfElements > 0)
		{
			const ScopedLockType lock (getLock());
			data.ensureAllocatedSize (numUsed + numberOfElements);
			ElementType* insertPos;

			if (((unsigned int) indexToInsertAt) < (unsigned int) numUsed)
			{
				insertPos = data.elements + indexToInsertAt;
				const int numberToMove = numUsed - indexToInsertAt;
				memmove (insertPos + numberOfElements, insertPos, numberToMove * sizeof (ElementType));
			}
			else
			{
				insertPos = data.elements + numUsed;
			}

			numUsed += numberOfElements;

			while (--numberOfElements >= 0)
				new (insertPos++) ElementType (*newElements++);
		}
	}

	void addIfNotAlreadyThere (ParameterType newElement)
	{
		const ScopedLockType lock (getLock());

		if (! contains (newElement))
			add (newElement);
	}

	void set (const int indexToChange, ParameterType newValue)
	{
		jassert (indexToChange >= 0);
		const ScopedLockType lock (getLock());

		if (((unsigned int) indexToChange) < (unsigned int) numUsed)
		{
			data.elements [indexToChange] = newValue;
		}
		else if (indexToChange >= 0)
		{
			data.ensureAllocatedSize (numUsed + 1);
			new (data.elements + numUsed++) ElementType (newValue);
		}
	}

	void setUnchecked (const int indexToChange, ParameterType newValue)
	{
		const ScopedLockType lock (getLock());
		jassert (((unsigned int) indexToChange) < (unsigned int) numUsed);
		data.elements [indexToChange] = newValue;
	}

	void addArray (const ElementType* elementsToAdd, int numElementsToAdd)
	{
		const ScopedLockType lock (getLock());

		if (numElementsToAdd > 0)
		{
			data.ensureAllocatedSize (numUsed + numElementsToAdd);

			while (--numElementsToAdd >= 0)
				new (data.elements + numUsed++) ElementType (*elementsToAdd++);
		}
	}

	void swapWithArray (Array& otherArray) throw()
	{
		const ScopedLockType lock1 (getLock());
		const ScopedLockType lock2 (otherArray.getLock());

		data.swapWith (otherArray.data);
		swapVariables (numUsed, otherArray.numUsed);
	}

	template <class OtherArrayType>
	void addArray (const OtherArrayType& arrayToAddFrom,
				   int startIndex = 0,
				   int numElementsToAdd = -1)
	{
		const typename OtherArrayType::ScopedLockType lock1 (arrayToAddFrom.getLock());
		const ScopedLockType lock2 (getLock());

		if (startIndex < 0)
		{
			jassertfalse
			startIndex = 0;
		}

		if (numElementsToAdd < 0 || startIndex + numElementsToAdd > arrayToAddFrom.size())
			numElementsToAdd = arrayToAddFrom.size() - startIndex;

		while (--numElementsToAdd >= 0)
			add (arrayToAddFrom.getUnchecked (startIndex++));
	}

	template <class ElementComparator>
	void addSorted (ElementComparator& comparator, ParameterType newElement)
	{
		const ScopedLockType lock (getLock());
		insert (findInsertIndexInSortedArray (comparator, data.elements.getData(), newElement, 0, numUsed), newElement);
	}

	template <class ElementComparator>
	int indexOfSorted (ElementComparator& comparator, ParameterType elementToLookFor) const
	{
		(void) comparator;  // if you pass in an object with a static compareElements() method, this
							// avoids getting warning messages about the parameter being unused

		const ScopedLockType lock (getLock());
		int start = 0;
		int end = numUsed;

		for (;;)
		{
			if (start >= end)
			{
				return -1;
			}
			else if (comparator.compareElements (elementToLookFor, data.elements [start]) == 0)
			{
				return start;
			}
			else
			{
				const int halfway = (start + end) >> 1;

				if (halfway == start)
					return -1;
				else if (comparator.compareElements (elementToLookFor, data.elements [halfway]) >= 0)
					start = halfway;
				else
					end = halfway;
			}
		}
	}

	ElementType remove (const int indexToRemove)
	{
		const ScopedLockType lock (getLock());

		if (((unsigned int) indexToRemove) < (unsigned int) numUsed)
		{
			--numUsed;

			ElementType* const e = data.elements + indexToRemove;
			ElementType removed (*e);
			e->~ElementType();
			const int numberToShift = numUsed - indexToRemove;

			if (numberToShift > 0)
				memmove (e, e + 1, numberToShift * sizeof (ElementType));

			if ((numUsed << 1) < data.numAllocated)
				minimiseStorageOverheads();

			return removed;
		}
		else
		{
			return ElementType();
		}
	}

	void removeValue (ParameterType valueToRemove)
	{
		const ScopedLockType lock (getLock());
		ElementType* e = data.elements;

		for (int i = numUsed; --i >= 0;)
		{
			if (valueToRemove == *e)
			{
				remove (static_cast <int> (e - data.elements.getData()));
				break;
			}

			++e;
		}
	}

	void removeRange (int startIndex, int numberToRemove)
	{
		const ScopedLockType lock (getLock());
		const int endIndex = jlimit (0, numUsed, startIndex + numberToRemove);
		startIndex = jlimit (0, numUsed, startIndex);

		if (endIndex > startIndex)
		{
			ElementType* const e = data.elements + startIndex;

			numberToRemove = endIndex - startIndex;
			for (int i = 0; i < numberToRemove; ++i)
				e[i].~ElementType();

			const int numToShift = numUsed - endIndex;
			if (numToShift > 0)
				memmove (e, e + numberToRemove, numToShift * sizeof (ElementType));

			numUsed -= numberToRemove;

			if ((numUsed << 1) < data.numAllocated)
				minimiseStorageOverheads();
		}
	}

	void removeLast (int howManyToRemove = 1)
	{
		const ScopedLockType lock (getLock());

		if (howManyToRemove > numUsed)
			howManyToRemove = numUsed;

		for (int i = 0; i < howManyToRemove; ++i)
			data.elements [numUsed - i].~ElementType();

		numUsed -= howManyToRemove;

		if ((numUsed << 1) < data.numAllocated)
			minimiseStorageOverheads();
	}

	template <class OtherArrayType>
	void removeValuesIn (const OtherArrayType& otherArray)
	{
		const typename OtherArrayType::ScopedLockType lock1 (otherArray.getLock());
		const ScopedLockType lock2 (getLock());

		if (this == &otherArray)
		{
			clear();
		}
		else
		{
			if (otherArray.size() > 0)
			{
				for (int i = numUsed; --i >= 0;)
					if (otherArray.contains (data.elements [i]))
						remove (i);
			}
		}
	}

	template <class OtherArrayType>
	void removeValuesNotIn (const OtherArrayType& otherArray)
	{
		const typename OtherArrayType::ScopedLockType lock1 (otherArray.getLock());
		const ScopedLockType lock2 (getLock());

		if (this != &otherArray)
		{
			if (otherArray.size() <= 0)
			{
				clear();
			}
			else
			{
				for (int i = numUsed; --i >= 0;)
					if (! otherArray.contains (data.elements [i]))
						remove (i);
			}
		}
	}

	void swap (const int index1,
			   const int index2)
	{
		const ScopedLockType lock (getLock());

		if (((unsigned int) index1) < (unsigned int) numUsed
			&& ((unsigned int) index2) < (unsigned int) numUsed)
		{
			swapVariables (data.elements [index1],
						   data.elements [index2]);
		}
	}

	void move (const int currentIndex, int newIndex) throw()
	{
		if (currentIndex != newIndex)
		{
			const ScopedLockType lock (getLock());

			if (((unsigned int) currentIndex) < (unsigned int) numUsed)
			{
				if (((unsigned int) newIndex) >= (unsigned int) numUsed)
					newIndex = numUsed - 1;

				char tempCopy [sizeof (ElementType)];
				memcpy (tempCopy, data.elements + currentIndex, sizeof (ElementType));

				if (newIndex > currentIndex)
				{
					memmove (data.elements + currentIndex,
							 data.elements + currentIndex + 1,
							 (newIndex - currentIndex) * sizeof (ElementType));
				}
				else
				{
					memmove (data.elements + newIndex + 1,
							 data.elements + newIndex,
							 (currentIndex - newIndex) * sizeof (ElementType));
				}

				memcpy (data.elements + newIndex, tempCopy, sizeof (ElementType));
			}
		}
	}

	void minimiseStorageOverheads()
	{
		const ScopedLockType lock (getLock());
		data.shrinkToNoMoreThan (numUsed);
	}

	void ensureStorageAllocated (const int minNumElements)
	{
		const ScopedLockType lock (getLock());
		data.ensureAllocatedSize (minNumElements);
	}

	template <class ElementComparator>
	void sort (ElementComparator& comparator,
			   const bool retainOrderOfEquivalentItems = false) const
	{
		const ScopedLockType lock (getLock());
		(void) comparator;  // if you pass in an object with a static compareElements() method, this
							// avoids getting warning messages about the parameter being unused
		sortArray (comparator, data.elements.getData(), 0, size() - 1, retainOrderOfEquivalentItems);
	}

	inline const TypeOfCriticalSectionToUse& getLock() const throw()	   { return data; }

	typedef typename TypeOfCriticalSectionToUse::ScopedLockType ScopedLockType;

	juce_UseDebuggingNewOperator

private:
	ArrayAllocationBase <ElementType, TypeOfCriticalSectionToUse> data;
	int numUsed;
};

#endif   // __JUCE_ARRAY_JUCEHEADER__
/*** End of inlined file: juce_Array.h ***/


#endif
#ifndef __JUCE_ARRAYALLOCATIONBASE_JUCEHEADER__

#endif
#ifndef __JUCE_BITARRAY_JUCEHEADER__

/*** Start of inlined file: juce_BitArray.h ***/
#ifndef __JUCE_BITARRAY_JUCEHEADER__
#define __JUCE_BITARRAY_JUCEHEADER__

class MemoryBlock;

class JUCE_API  BigInteger
{
public:

	BigInteger();

	BigInteger (unsigned int value);

	BigInteger (int value);

	BigInteger (int64 value);

	BigInteger (const BigInteger& other);

	~BigInteger();

	BigInteger& operator= (const BigInteger& other);

	void swapWith (BigInteger& other) throw();

	bool operator[] (int bit) const throw();

	bool isZero() const throw();

	bool isOne() const throw();

	int toInteger() const throw();

	void clear();

	void clearBit (int bitNumber) throw();

	void setBit (int bitNumber);

	void setBit (int bitNumber, bool shouldBeSet);

	void setRange (int startBit, int numBits, bool shouldBeSet);

	void insertBit (int bitNumber, bool shouldBeSet);

	const BigInteger getBitRange (int startBit, int numBits) const;

	int getBitRangeAsInt (int startBit, int numBits) const throw();

	void setBitRangeAsInt (int startBit, int numBits, unsigned int valueToSet);

	void shiftBits (int howManyBitsLeft, int startBit);

	int countNumberOfSetBits() const throw();

	int findNextSetBit (int startIndex = 0) const throw();

	int findNextClearBit (int startIndex = 0) const throw();

	int getHighestBit() const throw();

	// All the standard arithmetic ops...

	BigInteger& operator+= (const BigInteger& other);
	BigInteger& operator-= (const BigInteger& other);
	BigInteger& operator*= (const BigInteger& other);
	BigInteger& operator/= (const BigInteger& other);
	BigInteger& operator|= (const BigInteger& other);
	BigInteger& operator&= (const BigInteger& other);
	BigInteger& operator^= (const BigInteger& other);
	BigInteger& operator%= (const BigInteger& other);
	BigInteger& operator<<= (int numBitsToShift);
	BigInteger& operator>>= (int numBitsToShift);
	BigInteger& operator++();
	BigInteger& operator--();
	const BigInteger operator++ (int);
	const BigInteger operator-- (int);

	const BigInteger operator-() const;
	const BigInteger operator+ (const BigInteger& other) const;
	const BigInteger operator- (const BigInteger& other) const;
	const BigInteger operator* (const BigInteger& other) const;
	const BigInteger operator/ (const BigInteger& other) const;
	const BigInteger operator| (const BigInteger& other) const;
	const BigInteger operator& (const BigInteger& other) const;
	const BigInteger operator^ (const BigInteger& other) const;
	const BigInteger operator% (const BigInteger& other) const;
	const BigInteger operator<< (int numBitsToShift) const;
	const BigInteger operator>> (int numBitsToShift) const;

	bool operator== (const BigInteger& other) const throw();
	bool operator!= (const BigInteger& other) const throw();
	bool operator<  (const BigInteger& other) const throw();
	bool operator<= (const BigInteger& other) const throw();
	bool operator>  (const BigInteger& other) const throw();
	bool operator>= (const BigInteger& other) const throw();

	int compare (const BigInteger& other) const throw();

	int compareAbsolute (const BigInteger& other) const throw();

	void divideBy (const BigInteger& divisor, BigInteger& remainder);

	const BigInteger findGreatestCommonDivisor (BigInteger other) const;

	void exponentModulo (const BigInteger& exponent, const BigInteger& modulus);

	void inverseModulo (const BigInteger& modulus);

	bool isNegative() const throw();

	void setNegative (const bool shouldBeNegative) throw();

	void negate() throw();

	const String toString (int base, int minimumNumCharacters = 1) const;

	void parseString (const String& text, int base);

	const MemoryBlock toMemoryBlock() const;

	void loadFromMemoryBlock (const MemoryBlock& data);

	juce_UseDebuggingNewOperator

private:
	HeapBlock <unsigned int> values;
	int numValues, highestBit;
	bool negative;

	void ensureSize (int numVals);
	static const BigInteger simpleGCD (BigInteger* m, BigInteger* n);
};

OutputStream& JUCE_CALLTYPE operator<< (OutputStream& stream, const BigInteger& value);

typedef BigInteger BitArray;

#endif   // __JUCE_BITARRAY_JUCEHEADER__
/*** End of inlined file: juce_BitArray.h ***/


#endif
#ifndef __JUCE_DYNAMICOBJECT_JUCEHEADER__

/*** Start of inlined file: juce_DynamicObject.h ***/
#ifndef __JUCE_DYNAMICOBJECT_JUCEHEADER__
#define __JUCE_DYNAMICOBJECT_JUCEHEADER__


/*** Start of inlined file: juce_NamedValueSet.h ***/
#ifndef __JUCE_NAMEDVALUESET_JUCEHEADER__
#define __JUCE_NAMEDVALUESET_JUCEHEADER__


/*** Start of inlined file: juce_Variant.h ***/
#ifndef __JUCE_VARIANT_JUCEHEADER__
#define __JUCE_VARIANT_JUCEHEADER__


/*** Start of inlined file: juce_OutputStream.h ***/
#ifndef __JUCE_OUTPUTSTREAM_JUCEHEADER__
#define __JUCE_OUTPUTSTREAM_JUCEHEADER__


/*** Start of inlined file: juce_InputStream.h ***/
#ifndef __JUCE_INPUTSTREAM_JUCEHEADER__
#define __JUCE_INPUTSTREAM_JUCEHEADER__


/*** Start of inlined file: juce_MemoryBlock.h ***/
#ifndef __JUCE_MEMORYBLOCK_JUCEHEADER__
#define __JUCE_MEMORYBLOCK_JUCEHEADER__

class JUCE_API  MemoryBlock
{
public:

	MemoryBlock() throw();

	MemoryBlock (const size_t initialSize,
				 const bool initialiseToZero = false) throw();

	MemoryBlock (const MemoryBlock& other) throw();

	MemoryBlock (const void* const dataToInitialiseFrom,
				 const size_t sizeInBytes) throw();

	~MemoryBlock() throw();

	MemoryBlock& operator= (const MemoryBlock& other) throw();

	bool operator== (const MemoryBlock& other) const throw();

	bool operator!= (const MemoryBlock& other) const throw();

	bool matches (const void* data, size_t dataSize) const throw();

	void* getData() const throw()				   { return data; }

	template <typename Type>
	char& operator[] (const Type offset) const throw()		  { return data [offset]; }

	size_t getSize() const throw()				  { return size; }

	void setSize (const size_t newSize,
				  const bool initialiseNewSpaceToZero = false) throw();

	void ensureSize (const size_t minimumSize,
					 const bool initialiseNewSpaceToZero = false) throw();

	void fillWith (const uint8 valueToUse) throw();

	void append (const void* const data,
				 const size_t numBytes) throw();

	void swapWith (MemoryBlock& other) throw();

	void copyFrom (const void* srcData,
				   int destinationOffset,
				   size_t numBytes) throw();

	void copyTo (void* destData,
				 int sourceOffset,
				 size_t numBytes) const throw();

	void removeSection (size_t startByte, size_t numBytesToRemove) throw();

	const String toString() const throw();

	void loadFromHexString (const String& sourceHexString) throw();

	void setBitRange (size_t bitRangeStart,
					  size_t numBits,
					  int binaryNumberToApply) throw();

	int getBitRange (size_t bitRangeStart,
					 size_t numBitsToRead) const throw();

	const String toBase64Encoding() const throw();

	bool fromBase64Encoding  (const String& encodedString) throw();

	juce_UseDebuggingNewOperator

private:

	HeapBlock <char> data;
	size_t size;
};

#endif   // __JUCE_MEMORYBLOCK_JUCEHEADER__
/*** End of inlined file: juce_MemoryBlock.h ***/

class JUCE_API  InputStream
{
public:
	virtual ~InputStream()  {}

	virtual int64 getTotalLength() = 0;

	virtual bool isExhausted() = 0;

	virtual int read (void* destBuffer, int maxBytesToRead) = 0;

	virtual char readByte();

	virtual bool readBool();

	virtual short readShort();

	virtual short readShortBigEndian();

	virtual int readInt();

	virtual int readIntBigEndian();

	virtual int64 readInt64();

	virtual int64 readInt64BigEndian();

	virtual float readFloat();

	virtual float readFloatBigEndian();

	virtual double readDouble();

	virtual double readDoubleBigEndian();

	virtual int readCompressedInt();

	virtual const String readNextLine();

	virtual const String readString();

	virtual const String readEntireStreamAsString();

	virtual int readIntoMemoryBlock (MemoryBlock& destBlock,
									 int maxNumBytesToRead = -1);

	virtual int64 getPosition() = 0;

	virtual bool setPosition (int64 newPosition) = 0;

	virtual void skipNextBytes (int64 numBytesToSkip);

	juce_UseDebuggingNewOperator

protected:

	InputStream() throw()  {}
};

#endif   // __JUCE_INPUTSTREAM_JUCEHEADER__
/*** End of inlined file: juce_InputStream.h ***/

class JUCE_API  OutputStream
{
protected:

	OutputStream();

public:
	virtual ~OutputStream();

	virtual void flush() = 0;

	virtual bool setPosition (int64 newPosition) = 0;

	virtual int64 getPosition() = 0;

	virtual bool write (const void* dataToWrite,
						int howManyBytes) = 0;

	virtual void writeByte (char byte);

	virtual void writeBool (bool boolValue);

	virtual void writeShort (short value);

	virtual void writeShortBigEndian (short value);

	virtual void writeInt (int value);

	virtual void writeIntBigEndian (int value);

	virtual void writeInt64 (int64 value);

	virtual void writeInt64BigEndian (int64 value);

	virtual void writeFloat (float value);

	virtual void writeFloatBigEndian (float value);

	virtual void writeDouble (double value);

	virtual void writeDoubleBigEndian (double value);

	virtual void writeCompressedInt (int value);

	virtual void writeString (const String& text);

	virtual void writeText (const String& text,
							bool asUnicode,
							bool writeUnicodeHeaderBytes);

	virtual int writeFromInputStream (InputStream& source, int maxNumBytesToWrite);

	juce_UseDebuggingNewOperator
};

OutputStream& JUCE_CALLTYPE operator<< (OutputStream& stream, int number);

OutputStream& JUCE_CALLTYPE operator<< (OutputStream& stream, double number);

OutputStream& JUCE_CALLTYPE operator<< (OutputStream& stream, char character);

OutputStream& JUCE_CALLTYPE operator<< (OutputStream& stream, const char* text);

#endif   // __JUCE_OUTPUTSTREAM_JUCEHEADER__
/*** End of inlined file: juce_OutputStream.h ***/

class JUCE_API  DynamicObject;

class JUCE_API  var
{
public:

	typedef const var (DynamicObject::*MethodFunction) (const var* arguments, int numArguments);

	var() throw();

	~var() throw();

	static const var null;

	var (const var& valueToCopy);
	var (int value) throw();
	var (bool value) throw();
	var (double value) throw();
	var (const char* value);
	var (const juce_wchar* value);
	var (const String& value);
	var (DynamicObject* object);
	var (MethodFunction method) throw();

	var& operator= (const var& valueToCopy);
	var& operator= (int value);
	var& operator= (bool value);
	var& operator= (double value);
	var& operator= (const char* value);
	var& operator= (const juce_wchar* value);
	var& operator= (const String& value);
	var& operator= (DynamicObject* object);
	var& operator= (MethodFunction method);

	void swapWith (var& other) throw();

	operator int() const;
	operator bool() const;
	operator float() const;
	operator double() const;
	operator const String() const;
	const String toString() const;
	DynamicObject* getObject() const;

	bool isVoid() const throw()	 { return type == voidType; }
	bool isInt() const throw()	  { return type == intType; }
	bool isBool() const throw()	 { return type == boolType; }
	bool isDouble() const throw()	   { return type == doubleType; }
	bool isString() const throw()	   { return type == stringType; }
	bool isObject() const throw()	   { return type == objectType; }
	bool isMethod() const throw()	   { return type == methodType; }

	void writeToStream (OutputStream& output) const;

	static const var readFromStream (InputStream& input);

	class JUCE_API  identifier
	{
	public:
		identifier() throw();

		identifier (const char* name);

		identifier (const String& name);

		~identifier();

		bool operator== (const identifier& other) const throw()
		{
			jassert (hashCode != other.hashCode || name == other.name); // check for name hash collisions
			return hashCode == other.hashCode;
		}

		String name;
		int hashCode;
	};

	const var operator[] (const identifier& propertyName) const;

	const var call (const identifier& method) const;
	const var call (const identifier& method, const var& arg1) const;
	const var call (const identifier& method, const var& arg1, const var& arg2) const;
	const var call (const identifier& method, const var& arg1, const var& arg2, const var& arg3);
	const var call (const identifier& method, const var& arg1, const var& arg2, const var& arg3, const var& arg4) const;
	const var call (const identifier& method, const var& arg1, const var& arg2, const var& arg3, const var& arg4, const var& arg5) const;

	const var invoke (const identifier& method, const var* arguments, int numArguments) const;

	const var invoke (const var& targetObject, const var* arguments, int numArguments) const;

	juce_UseDebuggingNewOperator

	bool equals (const var& other) const throw();

private:
	enum Type
	{
		voidType = 0,
		intType,
		boolType,
		doubleType,
		stringType,
		objectType,
		methodType
	};

	union ValueUnion
	{
		int intValue;
		bool boolValue;
		double doubleValue;
		String* stringValue;
		DynamicObject* objectValue;
		MethodFunction methodValue;
	};

	Type type;
	ValueUnion value;
};

bool operator== (const var& v1, const var& v2) throw();
bool operator!= (const var& v1, const var& v2) throw();
bool operator== (const var& v1, const String& v2) throw();
bool operator!= (const var& v1, const String& v2) throw();

#endif   // __JUCE_VARIANT_JUCEHEADER__
/*** End of inlined file: juce_Variant.h ***/

class JUCE_API  NamedValueSet
{
public:
	NamedValueSet() throw();

	NamedValueSet (const NamedValueSet& other);

	NamedValueSet& operator= (const NamedValueSet& other);

	~NamedValueSet();

	int size() const throw();

	const var& operator[] (const var::identifier& name) const;

	const var getWithDefault (const var::identifier& name, const var& defaultReturnValue) const;

	var* getItem (const var::identifier& name) const;

	bool set (const var::identifier& name, const var& newValue);

	bool contains (const var::identifier& name) const;

	bool remove (const var::identifier& name);

	const var::identifier getName (int index) const;

	void clear();

	juce_UseDebuggingNewOperator

private:
	struct NamedValue
	{
		NamedValue() throw();
		NamedValue (const var::identifier& name, const var& value);

		var::identifier name;
		var value;
	};

	Array <NamedValue> values;
};

#endif   // __JUCE_NAMEDVALUESET_JUCEHEADER__
/*** End of inlined file: juce_NamedValueSet.h ***/


/*** Start of inlined file: juce_ReferenceCountedObject.h ***/
#ifndef __JUCE_REFERENCECOUNTEDOBJECT_JUCEHEADER__
#define __JUCE_REFERENCECOUNTEDOBJECT_JUCEHEADER__


/*** Start of inlined file: juce_Atomic.h ***/
#ifndef __JUCE_ATOMIC_JUCEHEADER__
#define __JUCE_ATOMIC_JUCEHEADER__

class JUCE_API  Atomic
{
public:
	static void increment (int32& variable);

	static int32 incrementAndReturn (int32& variable);

	static void decrement (int32& variable);

	static int32 decrementAndReturn (int32& variable);

	static int32 compareAndExchange (int32& destination, int32 newValue, int32 requiredCurrentValue);

	static void* swapPointers (void* volatile* value1, void* value2);

private:
	Atomic();
	Atomic (const Atomic&);
	Atomic& operator= (const Atomic&);
};

#if (JUCE_MAC || JUCE_IPHONE)	   //  Mac and iPhone...

	inline void  Atomic::increment (int32& variable)		{ OSAtomicIncrement32 (static_cast <int32_t*> (&variable)); }
	inline int32 Atomic::incrementAndReturn (int32& variable)	   { return OSAtomicIncrement32 (static_cast <int32_t*> (&variable)); }
	inline void  Atomic::decrement (int32& variable)		{ OSAtomicDecrement32 (static_cast <int32_t*> (&variable)); }
	inline int32 Atomic::decrementAndReturn (int32& variable)	   { return OSAtomicDecrement32 (static_cast <int32_t*> (&variable)); }
	inline int32 Atomic::compareAndExchange (int32& destination, int32 newValue, int32 oldValue)
																	{ return OSAtomicCompareAndSwap32Barrier (oldValue, newValue, static_cast <int32_t*> (&destination)); }
	inline void* Atomic::swapPointers (void* volatile* value1, void* value2)
	{
		void* currentVal = *value1;
	  #if MAC_OS_X_VERSION_MIN_REQUIRED < MAC_OS_X_VERSION_10_5 && ! JUCE_64BIT
		while (! OSAtomicCompareAndSwap32 (reinterpret_cast <int32_t> (currentVal), reinterpret_cast <int32_t> (value2),
										   const_cast <int32_t*> (reinterpret_cast <volatile int32_t*> (value1)))) { currentVal = *value1; }
	  #else
		while (! OSAtomicCompareAndSwapPtr (currentVal, value2, value1)) { currentVal = *value1; }
	  #endif
		return currentVal;
	}

#elif JUCE_LINUX			// Linux...

  #if __INTEL_COMPILER
	inline void  Atomic::increment (int32& variable)		{ _InterlockedIncrement (&variable); }
	inline int32 Atomic::incrementAndReturn (int32& variable)	   { return _InterlockedIncrement (&variable); }
	inline void  Atomic::decrement (int32& variable)		{ _InterlockedDecrement (&variable); }
	inline int32 Atomic::decrementAndReturn (int32& variable)	   { return _InterlockedDecrement (&variable); }
	inline int32 Atomic::compareAndExchange (int32& destination, int32 newValue, int32 oldValue)
																	{ return _InterlockedCompareExchange (&destination, newValue, oldValue); }

	inline void* Atomic::swapPointers (void* volatile* value1, void* value2)
	{
	  #if __ia64__
		return reinterpret_cast<void*> (_InterlockedExchange64 (const_cast<void**> (value1), reinterpret_cast<__int64> (value2)));
	  #else
		return reinterpret_cast<void*> (_InterlockedExchange (const_cast<void**> (value1), reinterpret_cast<long> (value2)));
	  #endif
	}

  #else
	inline void  Atomic::increment (int32& variable)		{ __sync_add_and_fetch (&variable, 1); }
	inline int32 Atomic::incrementAndReturn (int32& variable)	   { return __sync_add_and_fetch (&variable, 1); }
	inline void  Atomic::decrement (int32& variable)		{ __sync_add_and_fetch (&variable, -1); }
	inline int32 Atomic::decrementAndReturn (int32& variable)	   { return __sync_add_and_fetch (&variable, -1); }
	inline int32 Atomic::compareAndExchange (int32& destination, int32 newValue, int32 oldValue)
																	{ return __sync_val_compare_and_swap (&destination, oldValue, newValue); }

	inline void* Atomic::swapPointers (void* volatile* value1, void* value2)
	{
		void* currentVal = *value1;
		while (! __sync_bool_compare_and_swap (value1, currentVal, value2)) { currentVal = *value1; }
		return currentVal;
	}
  #endif

#elif JUCE_USE_INTRINSICS		   // Windows...

	// (If JUCE_USE_INTRINSICS isn't enabled, a fallback version of these methods is declared in juce_win32_Threads.cpp)
	#pragma intrinsic (_InterlockedIncrement)
	#pragma intrinsic (_InterlockedDecrement)
	#pragma intrinsic (_InterlockedCompareExchange)

	inline void  Atomic::increment (int32& variable)		{ _InterlockedIncrement (reinterpret_cast <volatile long*> (&variable)); }
	inline int32 Atomic::incrementAndReturn (int32& variable)	   { return _InterlockedIncrement (reinterpret_cast <volatile long*> (&variable)); }
	inline void  Atomic::decrement (int32& variable)		{ _InterlockedDecrement (reinterpret_cast <volatile long*> (&variable)); }
	inline int32 Atomic::decrementAndReturn (int32& variable)	   { return _InterlockedDecrement (reinterpret_cast <volatile long*> (&variable)); }
	inline int32 Atomic::compareAndExchange (int32& destination, int32 newValue, int32 oldValue)
																	{ return _InterlockedCompareExchange (reinterpret_cast <volatile long*> (&destination), newValue, oldValue); }
#endif

#endif   // __JUCE_ATOMIC_JUCEHEADER__
/*** End of inlined file: juce_Atomic.h ***/

class JUCE_API  ReferenceCountedObject
{
public:

	inline void incReferenceCount() throw()
	{
		Atomic::increment (refCounts);

		jassert (refCounts > 0);
	}

	inline void decReferenceCount() throw()
	{
		jassert (refCounts > 0);

		if (Atomic::decrementAndReturn (refCounts) == 0)
			delete this;
	}

	inline int getReferenceCount() const throw()
	{
		return refCounts;
	}

protected:

	ReferenceCountedObject()
		: refCounts (0)
	{
	}

	virtual ~ReferenceCountedObject()
	{
		// it's dangerous to delete an object that's still referenced by something else!
		jassert (refCounts == 0);
	}

private:

	int refCounts;
};

template <class ReferenceCountedObjectClass>
class ReferenceCountedObjectPtr
{
public:

	inline ReferenceCountedObjectPtr() throw()
		: referencedObject (0)
	{
	}

	inline ReferenceCountedObjectPtr (ReferenceCountedObjectClass* const refCountedObject) throw()
		: referencedObject (refCountedObject)
	{
		if (refCountedObject != 0)
			refCountedObject->incReferenceCount();
	}

	inline ReferenceCountedObjectPtr (const ReferenceCountedObjectPtr<ReferenceCountedObjectClass>& other) throw()
		: referencedObject (other.referencedObject)
	{
		if (referencedObject != 0)
			referencedObject->incReferenceCount();
	}

	ReferenceCountedObjectPtr<ReferenceCountedObjectClass>& operator= (const ReferenceCountedObjectPtr<ReferenceCountedObjectClass>& other)
	{
		ReferenceCountedObjectClass* const newObject = other.referencedObject;

		if (newObject != referencedObject)
		{
			if (newObject != 0)
				newObject->incReferenceCount();

			ReferenceCountedObjectClass* const oldObject = referencedObject;
			referencedObject = newObject;

			if (oldObject != 0)
				oldObject->decReferenceCount();
		}

		return *this;
	}

	ReferenceCountedObjectPtr<ReferenceCountedObjectClass>& operator= (ReferenceCountedObjectClass* const newObject)
	{
		if (referencedObject != newObject)
		{
			if (newObject != 0)
				newObject->incReferenceCount();

			ReferenceCountedObjectClass* const oldObject = referencedObject;
			referencedObject = newObject;

			if (oldObject != 0)
				oldObject->decReferenceCount();
		}

		return *this;
	}

	inline ~ReferenceCountedObjectPtr()
	{
		if (referencedObject != 0)
			referencedObject->decReferenceCount();
	}

	inline operator ReferenceCountedObjectClass*() const throw()
	{
		return referencedObject;
	}

	inline bool operator== (ReferenceCountedObjectClass* const object) const throw()
	{
		return referencedObject == object;
	}

	inline bool operator!= (ReferenceCountedObjectClass* const object) const throw()
	{
		return referencedObject != object;
	}

	// the -> operator is called on the referenced object
	inline ReferenceCountedObjectClass* operator->() const throw()
	{
		return referencedObject;
	}

private:

	ReferenceCountedObjectClass* referencedObject;
};

#endif   // __JUCE_REFERENCECOUNTEDOBJECT_JUCEHEADER__
/*** End of inlined file: juce_ReferenceCountedObject.h ***/

class JUCE_API  DynamicObject  : public ReferenceCountedObject
{
public:

	DynamicObject();

	virtual ~DynamicObject();

	virtual bool hasProperty (const var::identifier& propertyName) const;

	virtual const var getProperty (const var::identifier& propertyName) const;

	virtual void setProperty (const var::identifier& propertyName, const var& newValue);

	virtual void removeProperty (const var::identifier& propertyName);

	virtual bool hasMethod (const var::identifier& methodName) const;

	virtual const var invokeMethod (const var::identifier& methodName,
									const var* parameters,
									int numParameters);

	void setMethod (const var::identifier& methodName,
					var::MethodFunction methodFunction);

	void clear();

	juce_UseDebuggingNewOperator

private:
	NamedValueSet properties;
};

#endif   // __JUCE_DYNAMICOBJECT_JUCEHEADER__
/*** End of inlined file: juce_DynamicObject.h ***/


#endif
#ifndef __JUCE_ELEMENTCOMPARATOR_JUCEHEADER__

#endif
#ifndef __JUCE_HEAPBLOCK_JUCEHEADER__

#endif
#ifndef __JUCE_MEMORYBLOCK_JUCEHEADER__

#endif
#ifndef __JUCE_NAMEDVALUESET_JUCEHEADER__

#endif
#ifndef __JUCE_OWNEDARRAY_JUCEHEADER__

/*** Start of inlined file: juce_OwnedArray.h ***/
#ifndef __JUCE_OWNEDARRAY_JUCEHEADER__
#define __JUCE_OWNEDARRAY_JUCEHEADER__


/*** Start of inlined file: juce_ScopedPointer.h ***/
#ifndef __JUCE_SCOPEDPOINTER_JUCEHEADER__
#define __JUCE_SCOPEDPOINTER_JUCEHEADER__

template <class ObjectType>
class JUCE_API  ScopedPointer
{
public:

	inline ScopedPointer() throw()  : object (0)
	{
	}

	inline ScopedPointer (ObjectType* const objectToTakePossessionOf) throw()
		: object (objectToTakePossessionOf)
	{
	}

	ScopedPointer (ScopedPointer& objectToTransferFrom) throw()
		: object (objectToTransferFrom.object)
	{
		objectToTransferFrom.object = 0;
	}

	inline ~ScopedPointer()							 { delete object; }

	ScopedPointer& operator= (ScopedPointer& objectToTransferFrom)
	{
		if (this != objectToTransferFrom.getAddress())
		{
			// Two ScopedPointers should never be able to refer to the same object - if
			// this happens, you must have done something dodgy!
			jassert (object == 0 || object != objectToTransferFrom.object);

			ObjectType* const oldObject = object;
			object = objectToTransferFrom.object;
			objectToTransferFrom.object = 0;
			delete oldObject;
		}

		return *this;
	}

	ScopedPointer& operator= (ObjectType* const newObjectToTakePossessionOf)
	{
		if (object != newObjectToTakePossessionOf)
		{
			ObjectType* const oldObject = object;
			object = newObjectToTakePossessionOf;
			delete oldObject;
		}

		return *this;
	}

	inline operator ObjectType*() const throw()					 { return object; }

	inline ObjectType& operator*() const throw()					{ return *object; }

	inline ObjectType* operator->() const throw()				   { return object; }

	inline ObjectType* const* operator&() const throw()				 { return static_cast <ObjectType* const*> (&object); }

	inline ObjectType** operator&() throw()					 { return static_cast <ObjectType**> (&object); }

	ObjectType* release() throw()						   { ObjectType* const o = object; object = 0; return o; }

	void swapWith (ScopedPointer <ObjectType>& other) throw()
	{
		// Two ScopedPointers should never be able to refer to the same object - if
		// this happens, you must have done something dodgy!
		jassert (object != other.object);

		swapVariables (object, other.object);
	}

private:

	ObjectType* object;

	// (Required as an alternative to the overloaded & operator).
	const ScopedPointer* getAddress() const throw()				 { return this; }
};

template <class ObjectType>
inline bool operator== (const ScopedPointer<ObjectType>& pointer1, const ObjectType* const pointer2) throw()
{
	return static_cast <ObjectType*> (pointer1) == pointer2;
}

template <class ObjectType>
inline bool operator!= (const ScopedPointer<ObjectType>& pointer1, const ObjectType* const pointer2) throw()
{
	return static_cast <ObjectType*> (pointer1) != pointer2;
}

#endif   // __JUCE_SCOPEDPOINTER_JUCEHEADER__
/*** End of inlined file: juce_ScopedPointer.h ***/

template <class ObjectClass,
		  class TypeOfCriticalSectionToUse = DummyCriticalSection>

class OwnedArray
{
public:

	OwnedArray() throw()
		: numUsed (0)
	{
	}

	~OwnedArray()
	{
		clear (true);
	}

	void clear (const bool deleteObjects = true)
	{
		const ScopedLockType lock (getLock());

		if (deleteObjects)
		{
			while (numUsed > 0)
				delete data.elements [--numUsed];
		}

		data.setAllocatedSize (0);
		numUsed = 0;
	}

	inline int size() const throw()
	{
		return numUsed;
	}

	inline ObjectClass* operator[] (const int index) const throw()
	{
		const ScopedLockType lock (getLock());
		return (((unsigned int) index) < (unsigned int) numUsed) ? data.elements [index]
																 : static_cast <ObjectClass*> (0);
	}

	inline ObjectClass* getUnchecked (const int index) const throw()
	{
		const ScopedLockType lock (getLock());
		jassert (((unsigned int) index) < (unsigned int) numUsed);
		return data.elements [index];
	}

	inline ObjectClass* getFirst() const throw()
	{
		const ScopedLockType lock (getLock());
		return numUsed > 0 ? data.elements [0]
						   : static_cast <ObjectClass*> (0);
	}

	inline ObjectClass* getLast() const throw()
	{
		const ScopedLockType lock (getLock());
		return numUsed > 0 ? data.elements [numUsed - 1]
						   : static_cast <ObjectClass*> (0);
	}

	int indexOf (const ObjectClass* const objectToLookFor) const throw()
	{
		const ScopedLockType lock (getLock());
		ObjectClass* const* e = data.elements.getData();
		ObjectClass* const* const end = e + numUsed;

		while (e != end)
		{
			if (objectToLookFor == *e)
				return static_cast <int> (e - data.elements.getData());

			++e;
		}

		return -1;
	}

	bool contains (const ObjectClass* const objectToLookFor) const throw()
	{
		const ScopedLockType lock (getLock());
		ObjectClass* const* e = data.elements.getData();
		ObjectClass* const* const end = e + numUsed;

		while (e != end)
		{
			if (objectToLookFor == *e)
				return true;

			++e;
		}

		return false;
	}

	void add (const ObjectClass* const newObject) throw()
	{
		const ScopedLockType lock (getLock());
		data.ensureAllocatedSize (numUsed + 1);
		data.elements [numUsed++] = const_cast <ObjectClass*> (newObject);
	}

	void insert (int indexToInsertAt,
				 const ObjectClass* const newObject) throw()
	{
		if (indexToInsertAt >= 0)
		{
			const ScopedLockType lock (getLock());

			if (indexToInsertAt > numUsed)
				indexToInsertAt = numUsed;

			data.ensureAllocatedSize (numUsed + 1);

			ObjectClass** const e = data.elements + indexToInsertAt;
			const int numToMove = numUsed - indexToInsertAt;

			if (numToMove > 0)
				memmove (e + 1, e, numToMove * sizeof (ObjectClass*));

			*e = const_cast <ObjectClass*> (newObject);
			++numUsed;
		}
		else
		{
			add (newObject);
		}
	}

	void addIfNotAlreadyThere (const ObjectClass* const newObject) throw()
	{
		const ScopedLockType lock (getLock());

		if (! contains (newObject))
			add (newObject);
	}

	void set (const int indexToChange,
			  const ObjectClass* const newObject,
			  const bool deleteOldElement = true)
	{
		if (indexToChange >= 0)
		{
			ScopedPointer <ObjectClass> toDelete;
			const ScopedLockType lock (getLock());

			if (indexToChange < numUsed)
			{
				if (deleteOldElement)
				{
					toDelete = data.elements [indexToChange];

					if (toDelete == newObject)
						toDelete = 0;
				}

				data.elements [indexToChange] = const_cast <ObjectClass*> (newObject);
			}
			else
			{
				data.ensureAllocatedSize (numUsed + 1);
				data.elements [numUsed++] = const_cast <ObjectClass*> (newObject);
			}
		}
	}

	template <class OtherArrayType>
	void addArray (const OtherArrayType& arrayToAddFrom,
				   int startIndex = 0,
				   int numElementsToAdd = -1)
	{
		const typename OtherArrayType::ScopedLockType lock1 (arrayToAddFrom.getLock());
		const ScopedLockType lock2 (getLock());

		if (startIndex < 0)
		{
			jassertfalse
			startIndex = 0;
		}

		if (numElementsToAdd < 0 || startIndex + numElementsToAdd > arrayToAddFrom.size())
			numElementsToAdd = arrayToAddFrom.size() - startIndex;

		while (--numElementsToAdd >= 0)
			add (arrayToAddFrom.getUnchecked (startIndex++));
	}

	template <class ElementComparator>
	void addSorted (ElementComparator& comparator,
					ObjectClass* const newObject) throw()
	{
		(void) comparator;  // if you pass in an object with a static compareElements() method, this
							// avoids getting warning messages about the parameter being unused
		const ScopedLockType lock (getLock());
		insert (findInsertIndexInSortedArray (comparator, data.elements.getData(), newObject, 0, numUsed), newObject);
	}

	template <class ElementComparator>
	int indexOfSorted (ElementComparator& comparator,
					   const ObjectClass* const objectToLookFor) const throw()
	{
		(void) comparator;  // if you pass in an object with a static compareElements() method, this
							// avoids getting warning messages about the parameter being unused
		const ScopedLockType lock (getLock());

		int start = 0;
		int end = numUsed;

		for (;;)
		{
			if (start >= end)
			{
				return -1;
			}
			else if (comparator.compareElements (objectToLookFor, data.elements [start]) == 0)
			{
				return start;
			}
			else
			{
				const int halfway = (start + end) >> 1;

				if (halfway == start)
					return -1;
				else if (comparator.compareElements (objectToLookFor, data.elements [halfway]) >= 0)
					start = halfway;
				else
					end = halfway;
			}
		}
	}

	void remove (const int indexToRemove,
				 const bool deleteObject = true)
	{
		ScopedPointer <ObjectClass> toDelete;
		const ScopedLockType lock (getLock());

		if (((unsigned int) indexToRemove) < (unsigned int) numUsed)
		{
			ObjectClass** const e = data.elements + indexToRemove;

			if (deleteObject)
				toDelete = *e;

			--numUsed;
			const int numToShift = numUsed - indexToRemove;

			if (numToShift > 0)
				memmove (e, e + 1, numToShift * sizeof (ObjectClass*));

			if ((numUsed << 1) < data.numAllocated)
				minimiseStorageOverheads();
		}
	}

	void removeObject (const ObjectClass* const objectToRemove,
					   const bool deleteObject = true)
	{
		const ScopedLockType lock (getLock());
		ObjectClass** e = data.elements.getData();

		for (int i = numUsed; --i >= 0;)
		{
			if (objectToRemove == *e)
			{
				remove (static_cast <int> (e - data.elements.getData()), deleteObject);
				break;
			}

			++e;
		}
	}

	void removeRange (int startIndex,
					  const int numberToRemove,
					  const bool deleteObjects = true)
	{
		const ScopedLockType lock (getLock());
		const int endIndex = jlimit (0, numUsed, startIndex + numberToRemove);
		startIndex = jlimit (0, numUsed, startIndex);

		if (endIndex > startIndex)
		{
			if (deleteObjects)
			{
				for (int i = startIndex; i < endIndex; ++i)
				{
					delete data.elements [i];
					data.elements [i] = 0; // (in case one of the destructors accesses this array and hits a dangling pointer)
				}
			}

			const int rangeSize = endIndex - startIndex;
			ObjectClass** e = data.elements + startIndex;
			int numToShift = numUsed - endIndex;
			numUsed -= rangeSize;

			while (--numToShift >= 0)
			{
				*e = e [rangeSize];
				++e;
			}

			if ((numUsed << 1) < data.numAllocated)
				minimiseStorageOverheads();
		}
	}

	void removeLast (int howManyToRemove = 1,
					 const bool deleteObjects = true)
	{
		const ScopedLockType lock (getLock());

		if (howManyToRemove >= numUsed)
		{
			clear (deleteObjects);
		}
		else
		{
			while (--howManyToRemove >= 0)
				remove (numUsed - 1, deleteObjects);
		}
	}

	void swap (const int index1,
			   const int index2) throw()
	{
		const ScopedLockType lock (getLock());

		if (((unsigned int) index1) < (unsigned int) numUsed
			 && ((unsigned int) index2) < (unsigned int) numUsed)
		{
			swapVariables (data.elements [index1],
						   data.elements [index2]);
		}
	}

	void move (const int currentIndex,
			   int newIndex) throw()
	{
		if (currentIndex != newIndex)
		{
			const ScopedLockType lock (getLock());

			if (((unsigned int) currentIndex) < (unsigned int) numUsed)
			{
				if (((unsigned int) newIndex) >= (unsigned int) numUsed)
					newIndex = numUsed - 1;

				ObjectClass* const value = data.elements [currentIndex];

				if (newIndex > currentIndex)
				{
					memmove (data.elements + currentIndex,
							 data.elements + currentIndex + 1,
							 (newIndex - currentIndex) * sizeof (ObjectClass*));
				}
				else
				{
					memmove (data.elements + newIndex + 1,
							 data.elements + newIndex,
							 (currentIndex - newIndex) * sizeof (ObjectClass*));
				}

				data.elements [newIndex] = value;
			}
		}
	}

	void swapWithArray (OwnedArray& otherArray) throw()
	{
		const ScopedLockType lock1 (getLock());
		const ScopedLockType lock2 (otherArray.getLock());

		data.swapWith (otherArray.data);
		swapVariables (numUsed, otherArray.numUsed);
	}

	void minimiseStorageOverheads() throw()
	{
		const ScopedLockType lock (getLock());
		data.shrinkToNoMoreThan (numUsed);
	}

	void ensureStorageAllocated (const int minNumElements) throw()
	{
		const ScopedLockType lock (getLock());
		data.ensureAllocatedSize (minNumElements);
	}

	template <class ElementComparator>
	void sort (ElementComparator& comparator,
			   const bool retainOrderOfEquivalentItems = false) const throw()
	{
		(void) comparator;  // if you pass in an object with a static compareElements() method, this
							// avoids getting warning messages about the parameter being unused

		const ScopedLockType lock (getLock());
		sortArray (comparator, data.elements.getData(), 0, size() - 1, retainOrderOfEquivalentItems);
	}

	inline const TypeOfCriticalSectionToUse& getLock() const throw()	   { return data; }

	typedef typename TypeOfCriticalSectionToUse::ScopedLockType ScopedLockType;

	juce_UseDebuggingNewOperator

private:
	ArrayAllocationBase <ObjectClass*, TypeOfCriticalSectionToUse> data;
	int numUsed;

	// disallow copy constructor and assignment
	OwnedArray (const OwnedArray&);
	OwnedArray& operator= (const OwnedArray&);
};

#endif   // __JUCE_OWNEDARRAY_JUCEHEADER__
/*** End of inlined file: juce_OwnedArray.h ***/


#endif
#ifndef __JUCE_PROPERTYSET_JUCEHEADER__

/*** Start of inlined file: juce_PropertySet.h ***/
#ifndef __JUCE_PROPERTYSET_JUCEHEADER__
#define __JUCE_PROPERTYSET_JUCEHEADER__


/*** Start of inlined file: juce_StringPairArray.h ***/
#ifndef __JUCE_STRINGPAIRARRAY_JUCEHEADER__
#define __JUCE_STRINGPAIRARRAY_JUCEHEADER__


/*** Start of inlined file: juce_StringArray.h ***/
#ifndef __JUCE_STRINGARRAY_JUCEHEADER__
#define __JUCE_STRINGARRAY_JUCEHEADER__

class JUCE_API  StringArray
{
public:

	StringArray() throw();

	StringArray (const StringArray& other);

	explicit StringArray (const String& firstValue);

	StringArray (const juce_wchar** strings, int numberOfStrings);

	StringArray (const char** strings, int numberOfStrings);

	explicit StringArray (const juce_wchar** strings);

	explicit StringArray (const char** strings);

	~StringArray();

	StringArray& operator= (const StringArray& other);

	bool operator== (const StringArray& other) const throw();

	bool operator!= (const StringArray& other) const throw();

	inline int size() const throw()					 { return strings.size(); };

	const String& operator[] (int index) const throw();

	bool contains (const String& stringToLookFor,
				   bool ignoreCase = false) const;

	int indexOf (const String& stringToLookFor,
				 bool ignoreCase = false,
				 int startIndex = 0) const;

	void add (const String& stringToAdd);

	void insert (int index, const String& stringToAdd);

	void addIfNotAlreadyThere (const String& stringToAdd, bool ignoreCase = false);

	void set (int index, const String& newString);

	void addArray (const StringArray& other,
				   int startIndex = 0,
				   int numElementsToAdd = -1);

	int addTokens (const String& stringToTokenise,
				   bool preserveQuotedStrings);

	int addTokens (const String& stringToTokenise,
				   const String& breakCharacters,
				   const String& quoteCharacters);

	int addLines (const String& stringToBreakUp);

	void clear();

	void remove (int index);

	void removeString (const String& stringToRemove,
					   bool ignoreCase = false);

	void removeDuplicates (bool ignoreCase);

	void removeEmptyStrings (bool removeWhitespaceStrings = true);

	void move (int currentIndex, int newIndex) throw();

	void trim();

	void appendNumbersToDuplicates (bool ignoreCaseWhenComparing,
									bool appendNumberToFirstInstance,
									const juce_wchar* preNumberString = 0,
									const juce_wchar* postNumberString = 0);

	const String joinIntoString (const String& separatorString,
								 int startIndex = 0,
								 int numberOfElements = -1) const;

	void sort (bool ignoreCase);

	void minimiseStorageOverheads();

	juce_UseDebuggingNewOperator

private:
	Array <String> strings;
};

#endif   // __JUCE_STRINGARRAY_JUCEHEADER__
/*** End of inlined file: juce_StringArray.h ***/

class JUCE_API  StringPairArray
{
public:

	StringPairArray (bool ignoreCaseWhenComparingKeys = true);

	StringPairArray (const StringPairArray& other);

	~StringPairArray();

	StringPairArray& operator= (const StringPairArray& other);

	bool operator== (const StringPairArray& other) const;

	bool operator!= (const StringPairArray& other) const;

	const String& operator[] (const String& key) const;

	const String getValue (const String& key, const String& defaultReturnValue) const;

	const StringArray& getAllKeys() const throw()	   { return keys; }

	const StringArray& getAllValues() const throw()	 { return values; }

	inline int size() const throw()			 { return keys.size(); };

	void set (const String& key, const String& value);

	void addArray (const StringPairArray& other);

	void clear();

	void remove (const String& key);

	void remove (int index);

	void setIgnoresCase (bool shouldIgnoreCase);

	const String getDescription() const;

	void minimiseStorageOverheads();

	juce_UseDebuggingNewOperator

private:
	StringArray keys, values;
	bool ignoreCase;
};

#endif   // __JUCE_STRINGPAIRARRAY_JUCEHEADER__
/*** End of inlined file: juce_StringPairArray.h ***/


/*** Start of inlined file: juce_XmlElement.h ***/
#ifndef __JUCE_XMLELEMENT_JUCEHEADER__
#define __JUCE_XMLELEMENT_JUCEHEADER__


/*** Start of inlined file: juce_File.h ***/
#ifndef __JUCE_FILE_JUCEHEADER__
#define __JUCE_FILE_JUCEHEADER__


/*** Start of inlined file: juce_Time.h ***/
#ifndef __JUCE_TIME_JUCEHEADER__
#define __JUCE_TIME_JUCEHEADER__


/*** Start of inlined file: juce_RelativeTime.h ***/
#ifndef __JUCE_RELATIVETIME_JUCEHEADER__
#define __JUCE_RELATIVETIME_JUCEHEADER__

class JUCE_API  RelativeTime
{
public:

	explicit RelativeTime (double seconds = 0.0) throw();

	RelativeTime (const RelativeTime& other) throw();

	RelativeTime& operator= (const RelativeTime& other) throw();

	~RelativeTime() throw();

	static const RelativeTime milliseconds (int milliseconds) throw();

	static const RelativeTime milliseconds (int64 milliseconds) throw();

	static const RelativeTime minutes (double numberOfMinutes) throw();

	static const RelativeTime hours (double numberOfHours) throw();

	static const RelativeTime days (double numberOfDays) throw();

	static const RelativeTime weeks (double numberOfWeeks) throw();

	int64 inMilliseconds() const throw();

	double inSeconds() const throw()	{ return seconds; }

	double inMinutes() const throw();

	double inHours() const throw();

	double inDays() const throw();

	double inWeeks() const throw();

	const String getDescription (const String& returnValueForZeroTime = "0") const throw();

	bool operator== (const RelativeTime& other) const throw();
	bool operator!= (const RelativeTime& other) const throw();

	bool operator>  (const RelativeTime& other) const throw();
	bool operator<  (const RelativeTime& other) const throw();
	bool operator>= (const RelativeTime& other) const throw();
	bool operator<= (const RelativeTime& other) const throw();

	const RelativeTime  operator+  (const RelativeTime& timeToAdd) const throw();
	const RelativeTime  operator-  (const RelativeTime& timeToSubtract) const throw();

	const RelativeTime  operator+  (double secondsToAdd) const throw();
	const RelativeTime  operator-  (double secondsToSubtract) const throw();

	const RelativeTime& operator+= (const RelativeTime& timeToAdd) throw();
	const RelativeTime& operator-= (const RelativeTime& timeToSubtract) throw();

	const RelativeTime& operator+= (double secondsToAdd) throw();

	const RelativeTime& operator-= (double secondsToSubtract) throw();

	juce_UseDebuggingNewOperator

private:
	double seconds;
};

#endif   // __JUCE_RELATIVETIME_JUCEHEADER__
/*** End of inlined file: juce_RelativeTime.h ***/

class JUCE_API  Time
{
public:

	Time() throw();

	Time (const Time& other) throw();

	Time (int64 millisecondsSinceEpoch) throw();

	Time (int year,
		  int month,
		  int day,
		  int hours,
		  int minutes,
		  int seconds = 0,
		  int milliseconds = 0,
		  bool useLocalTime = true) throw();

	~Time() throw();

	Time& operator= (const Time& other) throw();

	static const Time JUCE_CALLTYPE getCurrentTime() throw();

	int64 toMilliseconds() const throw()				{ return millisSinceEpoch; }

	int getYear() const throw();

	int getMonth() const throw();

	const String getMonthName (bool threeLetterVersion) const throw();

	int getDayOfMonth() const throw();

	int getDayOfWeek() const throw();

	const String getWeekdayName (bool threeLetterVersion) const throw();

	int getHours() const throw();

	bool isAfternoon() const throw();

	int getHoursInAmPmFormat() const throw();

	int getMinutes() const throw();

	int getSeconds() const throw();

	int getMilliseconds() const throw();

	bool isDaylightSavingTime() const throw();

	const String getTimeZone() const throw();

	const String toString (bool includeDate,
						   bool includeTime,
						   bool includeSeconds = true,
						   bool use24HourClock = false) const throw();

	const String formatted (const juce_wchar* format) const throw();

	const Time operator+ (const RelativeTime& delta) const throw()  { return Time (millisSinceEpoch + delta.inMilliseconds()); }

	const Time operator- (const RelativeTime& delta) const throw()  { return Time (millisSinceEpoch - delta.inMilliseconds()); }

	const RelativeTime operator- (const Time& other) const throw()  { return RelativeTime::milliseconds (millisSinceEpoch - other.millisSinceEpoch); }

	bool operator== (const Time& other) const throw()		   { return millisSinceEpoch == other.millisSinceEpoch; }

	bool operator!= (const Time& other) const throw()		   { return millisSinceEpoch != other.millisSinceEpoch; }

	bool operator<  (const Time& other) const throw()		   { return millisSinceEpoch < other.millisSinceEpoch; }

	bool operator<= (const Time& other) const throw()		   { return millisSinceEpoch <= other.millisSinceEpoch; }

	bool operator>  (const Time& other) const throw()		   { return millisSinceEpoch > other.millisSinceEpoch; }

	bool operator>= (const Time& other) const throw()		   { return millisSinceEpoch >= other.millisSinceEpoch; }

	bool setSystemTimeToThisTime() const throw();

	static const String getWeekdayName (int dayNumber,
										bool threeLetterVersion) throw();

	static const String getMonthName (int monthNumber,
									  bool threeLetterVersion) throw();

	// Static methods for getting system timers directly..

	static int64 currentTimeMillis() throw();

	static uint32 getMillisecondCounter() throw();

	static double getMillisecondCounterHiRes() throw();

	static void waitForMillisecondCounter (uint32 targetTime) throw();

	static uint32 getApproximateMillisecondCounter() throw();

	// High-resolution timers..

	static int64 getHighResolutionTicks() throw();

	static int64 getHighResolutionTicksPerSecond() throw();

	static double highResolutionTicksToSeconds (int64 ticks) throw();

	static int64 secondsToHighResolutionTicks (double seconds) throw();

private:

	int64 millisSinceEpoch;
};

#endif   // __JUCE_TIME_JUCEHEADER__
/*** End of inlined file: juce_Time.h ***/

class FileInputStream;
class FileOutputStream;

class JUCE_API  File
{
public:

	File()   {}

	File (const String& path);

	File (const File& other);

	~File()  {}

	File& operator= (const String& newFilePath);

	File& operator= (const File& otherFile);

	static const File nonexistent;

	bool exists() const;

	bool existsAsFile() const;

	bool isDirectory() const;

	int64 getSize() const;

	static const String descriptionOfSizeInBytes (int64 bytes);

	const String& getFullPathName() const throw()	   { return fullPath; }

	const String getFileName() const;

	const String getRelativePathFrom (const File& directoryToBeRelativeTo) const;

	const String getFileExtension() const;

	bool hasFileExtension (const String& extensionToTest) const;

	const File withFileExtension (const String& newExtension) const;

	const String getFileNameWithoutExtension() const;

	int hashCode() const;

	int64 hashCode64() const;

	const File getChildFile (String relativePath) const;

	const File getSiblingFile (const String& siblingFileName) const;

	const File getParentDirectory() const;

	bool isAChildOf (const File& potentialParentDirectory) const;

	const File getNonexistentChildFile (const String& prefix,
										const String& suffix,
										bool putNumbersInBrackets = true) const;

	const File getNonexistentSibling (bool putNumbersInBrackets = true) const;

	bool operator== (const File& otherFile) const;
	bool operator!= (const File& otherFile) const;

	bool hasWriteAccess() const;

	bool setReadOnly (bool shouldBeReadOnly,
					  bool applyRecursively = false) const;

	bool isHidden() const;

	const File getLinkedTarget() const;

	const Time getLastModificationTime() const;

	const Time getLastAccessTime() const;

	const Time getCreationTime() const;

	bool setLastModificationTime (const Time& newTime) const;

	bool setLastAccessTime (const Time& newTime) const;

	bool setCreationTime (const Time& newTime) const;

	const String getVersion() const;

	bool create() const;

	bool createDirectory() const;

	bool deleteFile() const;

	bool deleteRecursively() const;

	bool moveToTrash() const;

	bool moveFileTo (const File& targetLocation) const;

	bool copyFileTo (const File& targetLocation) const;

	bool copyDirectoryTo (const File& newDirectory) const;

	enum TypesOfFileToFind
	{
		findDirectories		 = 1,	/**< Use this flag to indicate that you want to find directories. */
		findFiles		   = 2,	/**< Use this flag to indicate that you want to find files. */
		findFilesAndDirectories	 = 3,	/**< Use this flag to indicate that you want to find both files and directories. */
		ignoreHiddenFiles	   = 4	 /**< Add this flag to avoid returning any hidden files in the results. */
	};

	int findChildFiles (Array<File>& results,
						int whatToLookFor,
						bool searchRecursively,
						const String& wildCardPattern = "*") const;

	int getNumberOfChildFiles (int whatToLookFor,
							   const String& wildCardPattern = "*") const;

	bool containsSubDirectories() const;

	FileInputStream* createInputStream() const;

	FileOutputStream* createOutputStream (int bufferSize = 0x8000) const;

	bool loadFileAsData (MemoryBlock& result) const;

	const String loadFileAsString() const;

	bool appendData (const void* dataToAppend,
					 int numberOfBytes) const;

	bool replaceWithData (const void* dataToWrite,
						  int numberOfBytes) const;

	bool appendText (const String& textToAppend,
					 bool asUnicode = false,
					 bool writeUnicodeHeaderBytes = false) const;

	bool replaceWithText (const String& textToWrite,
						  bool asUnicode = false,
						  bool writeUnicodeHeaderBytes = false) const;

	static void findFileSystemRoots (Array<File>& results);

	const String getVolumeLabel() const;

	int getVolumeSerialNumber() const;

	int64 getBytesFreeOnVolume() const;

	int64 getVolumeTotalSize() const;

	bool isOnCDRomDrive() const;

	bool isOnHardDisk() const;

	bool isOnRemovableDrive() const;

	bool startAsProcess (const String& parameters = String::empty) const;

	void revealToUser() const;

	enum SpecialLocationType
	{
		userHomeDirectory,

		userDocumentsDirectory,

		userDesktopDirectory,

		userApplicationDataDirectory,

		commonApplicationDataDirectory,

		tempDirectory,

		currentExecutableFile,

		currentApplicationFile,

		invokedExecutableFile,

		globalApplicationsDirectory,

		userMusicDirectory,

		userMoviesDirectory,
	};

	static const File JUCE_CALLTYPE getSpecialLocation (const SpecialLocationType type);

	static const File createTempFile (const String& fileNameEnding);

	static const File getCurrentWorkingDirectory();

	bool setAsCurrentWorkingDirectory() const;

	static const juce_wchar separator;

	static const juce_wchar* separatorString;

	static const String createLegalFileName (const String& fileNameToFix);

	static const String createLegalPathName (const String& pathNameToFix);

	static bool areFileNamesCaseSensitive();

	static bool isAbsolutePath (const String& path);

	static const File createFileWithoutCheckingPath (const String& path);

	juce_UseDebuggingNewOperator

private:

	String fullPath;

	// internal way of contructing a file without checking the path
	friend class DirectoryIterator;
	File (const String&, int);
	const String getPathUpToLastSlash() const;
};

#endif   // __JUCE_FILE_JUCEHEADER__
/*** End of inlined file: juce_File.h ***/

#define forEachXmlChildElement(parentXmlElement, childElementVariableName) \
\
	for (XmlElement* childElementVariableName = (parentXmlElement).getFirstChildElement(); \
		 childElementVariableName != 0; \
		 childElementVariableName = childElementVariableName->getNextElement())

#define forEachXmlChildElementWithTagName(parentXmlElement, childElementVariableName, requiredTagName) \
\
	for (XmlElement* childElementVariableName = (parentXmlElement).getChildByName (requiredTagName); \
		 childElementVariableName != 0; \
		 childElementVariableName = childElementVariableName->getNextElementWithTagName (requiredTagName))

class JUCE_API  XmlElement
{
public:

	explicit XmlElement (const String& tagName) throw();

	XmlElement (const XmlElement& other) throw();

	XmlElement& operator= (const XmlElement& other) throw();

	~XmlElement() throw();

	bool isEquivalentTo (const XmlElement* other,
						 bool ignoreOrderOfAttributes) const throw();

	const String createDocument (const String& dtdToUse,
								 bool allOnOneLine = false,
								 bool includeXmlHeader = true,
								 const String& encodingType = "UTF-8",
								 int lineWrapLength = 60) const;

	void writeToStream (OutputStream& output,
						const String& dtdToUse,
						bool allOnOneLine = false,
						bool includeXmlHeader = true,
						const String& encodingType = "UTF-8",
						int lineWrapLength = 60) const;

	bool writeToFile (const File& destinationFile,
					  const String& dtdToUse,
					  const String& encodingType = "UTF-8",
					  int lineWrapLength = 60) const;

	inline const String& getTagName() const throw()  { return tagName; }

	bool hasTagName (const String& possibleTagName) const throw();

	int getNumAttributes() const throw();

	const String& getAttributeName (int attributeIndex) const throw();

	const String& getAttributeValue (int attributeIndex) const throw();

	// Attribute-handling methods..

	bool hasAttribute (const String& attributeName) const throw();

	const String& getStringAttribute (const String& attributeName) const throw();

	const String getStringAttribute (const String& attributeName,
									 const String& defaultReturnValue) const;

	bool compareAttribute (const String& attributeName,
						   const String& stringToCompareAgainst,
						   bool ignoreCase = false) const throw();

	int getIntAttribute (const String& attributeName,
						 int defaultReturnValue = 0) const;

	double getDoubleAttribute (const String& attributeName,
							   double defaultReturnValue = 0.0) const;

	bool getBoolAttribute (const String& attributeName,
						   bool defaultReturnValue = false) const;

	void setAttribute (const String& attributeName,
					   const String& newValue);

	void setAttribute (const String& attributeName,
					   int newValue);

	void setAttribute (const String& attributeName,
					   double newValue);

	void removeAttribute (const String& attributeName) throw();

	void removeAllAttributes() throw();

	// Child element methods..

	XmlElement* getFirstChildElement() const throw()	{ return firstChildElement; }

	inline XmlElement* getNextElement() const throw()	   { return nextElement; }

	XmlElement* getNextElementWithTagName (const String& requiredTagName) const;

	int getNumChildElements() const throw();

	XmlElement* getChildElement (int index) const throw();

	XmlElement* getChildByName (const String& tagNameToLookFor) const throw();

	void addChildElement (XmlElement* const newChildElement) throw();

	void insertChildElement (XmlElement* newChildNode,
							 int indexToInsertAt) throw();

	XmlElement* createNewChildElement (const String& tagName);

	bool replaceChildElement (XmlElement* currentChildElement,
							  XmlElement* newChildNode) throw();

	void removeChildElement (XmlElement* childToRemove,
							 bool shouldDeleteTheChild) throw();

	void deleteAllChildElements() throw();

	void deleteAllChildElementsWithTagName (const String& tagName) throw();

	bool containsChildElement (const XmlElement* const possibleChild) const throw();

	XmlElement* findParentElementOf (const XmlElement* elementToLookFor) throw();

	template <class ElementComparator>
	void sortChildElements (ElementComparator& comparator,
							const bool retainOrderOfEquivalentItems = false) throw()
	{
		const int num = getNumChildElements();

		if (num > 1)
		{
			HeapBlock <XmlElement*> elems (num);
			getChildElementsAsArray (elems);
			sortArray (comparator, (XmlElement**) elems, 0, num - 1, retainOrderOfEquivalentItems);
			reorderChildElements (elems, num);
		}
	}

	bool isTextElement() const throw();

	const String getText() const throw();

	void setText (const String& newText) throw();

	const String getAllSubText() const throw();

	const String getChildElementAllSubText (const String& childTagName,
											const String& defaultReturnValue) const throw();

	void addTextElement (const String& text) throw();

	void deleteAllTextElements() throw();

	static XmlElement* createTextElement (const String& text) throw();

	juce_UseDebuggingNewOperator

private:
	friend class XmlDocument;

	String tagName;
	XmlElement* firstChildElement;
	XmlElement* nextElement;

	struct XmlAttributeNode
	{
		XmlAttributeNode (const XmlAttributeNode& other) throw();
		XmlAttributeNode (const String& name, const String& value) throw();

		String name, value;
		XmlAttributeNode* next;

	private:
		XmlAttributeNode& operator= (const XmlAttributeNode&);
	};

	XmlAttributeNode* attributes;

	XmlElement (int) throw();
	void copyChildrenAndAttributesFrom (const XmlElement& other) throw();
	void writeElementAsText (OutputStream& out, int indentationLevel, int lineWrapLength) const;
	void getChildElementsAsArray (XmlElement**) const throw();
	void reorderChildElements (XmlElement** const, const int) throw();
};

#endif   // __JUCE_XMLELEMENT_JUCEHEADER__
/*** End of inlined file: juce_XmlElement.h ***/

class JUCE_API  PropertySet
{
public:

	PropertySet (const bool ignoreCaseOfKeyNames = false) throw();

	PropertySet (const PropertySet& other) throw();

	PropertySet& operator= (const PropertySet& other) throw();

	virtual ~PropertySet();

	const String getValue (const String& keyName,
						   const String& defaultReturnValue = String::empty) const throw();

	int getIntValue (const String& keyName,
					 const int defaultReturnValue = 0) const throw();

	double getDoubleValue (const String& keyName,
						   const double defaultReturnValue = 0.0) const throw();

	bool getBoolValue (const String& keyName,
					   const bool defaultReturnValue = false) const throw();

	XmlElement* getXmlValue (const String& keyName) const;

	void setValue (const String& keyName, const String& value) throw();

	void setValue (const String& keyName, const tchar* const value) throw();

	void setValue (const String& keyName, const int value) throw();

	void setValue (const String& keyName, const double value) throw();

	void setValue (const String& keyName, const bool value) throw();

	void setValue (const String& keyName, const XmlElement* const xml);

	void removeValue (const String& keyName) throw();

	bool containsKey (const String& keyName) const throw();

	void clear();

	StringPairArray& getAllProperties() throw()			 { return properties; }

	const CriticalSection& getLock() const throw()			  { return lock; }

	XmlElement* createXml (const String& nodeName) const throw();

	void restoreFromXml (const XmlElement& xml) throw();

	void setFallbackPropertySet (PropertySet* fallbackProperties) throw();

	PropertySet* getFallbackPropertySet() const throw()		 { return fallbackProperties; }

	juce_UseDebuggingNewOperator

protected:

	virtual void propertyChanged();

private:

	StringPairArray properties;
	PropertySet* fallbackProperties;
	CriticalSection lock;
	bool ignoreCaseOfKeys;
};

#endif   // __JUCE_PROPERTYSET_JUCEHEADER__
/*** End of inlined file: juce_PropertySet.h ***/


#endif
#ifndef __JUCE_RANGE_JUCEHEADER__

/*** Start of inlined file: juce_Range.h ***/
#ifndef __JUCE_RANGE_JUCEHEADER__
#define __JUCE_RANGE_JUCEHEADER__

template <typename ValueType>
class Range
{
public:

	Range() throw()
		: start (ValueType()), end (ValueType())
	{
	}

	Range (const ValueType start_, const ValueType end_) throw()
		: start (start_), end (jmax (start_, end_))
	{
	}

	Range (const Range& other) throw()
		: start (other.start), end (other.end)
	{
	}

	Range& operator= (const Range& other) throw()
	{
		start = other.start;
		end = other.end;
		return *this;
	}

	~Range() throw()
	{
	}

	static const Range between (const ValueType position1, const ValueType position2) throw()
	{
		return (position1 < position2) ? Range (position1, position2)
									   : Range (position2, position1);
	}

	static const Range emptyRange (const ValueType start) throw()
	{
		return Range (start, start);
	}

	inline ValueType getStart() const throw()	   { return start; }

	inline ValueType getLength() const throw()	  { return end - start; }

	inline ValueType getEnd() const throw()		 { return end; }

	inline bool isEmpty() const throw()		 { return start == end; }

	void setStart (const ValueType newStart) throw()
	{
		start = newStart;
		if (newStart > end)
			end = newStart;
	}

	const Range withStart (const ValueType newStart) const throw()
	{
		return Range (newStart, jmax (newStart, end));
	}

	const Range movedToStartAt (const ValueType newStart) const throw()
	{
		return Range (newStart, newStart + getLength());
	}

	void setEnd (const ValueType newEnd) throw()
	{
		end = newEnd;
		if (newEnd < start)
			start = newEnd;
	}

	const Range withEnd (const ValueType newEnd) const throw()
	{
		return Range (jmin (start, newEnd), newEnd);
	}

	const Range movedToEndAt (const ValueType newEnd) const throw()
	{
		return Range (newEnd - getLength(), newEnd);
	}

	void setLength (const ValueType newLength) throw()
	{
		end = start + jmax (ValueType(), newLength);
	}

	const Range withLength (const ValueType newLength) const throw()
	{
		return Range (start, start + newLength);
	}

	inline const Range& operator+= (const ValueType amountToAdd) throw()
	{
		start += amountToAdd;
		end += amountToAdd;
		return *this;
	}

	inline const Range& operator-= (const ValueType amountToSubtract) throw()
	{
		start -= amountToSubtract;
		end -= amountToSubtract;
		return *this;
	}

	const Range operator+ (const ValueType amountToAdd) const throw()
	{
		return Range (start + amountToAdd, end + amountToAdd);
	}

	const Range operator- (const ValueType amountToSubtract) const throw()
	{
		return Range (start - amountToSubtract, end - amountToSubtract);
	}

	bool operator== (const Range& other) const throw()	  { return start == other.start && end == other.end; }
	bool operator!= (const Range& other) const throw()	  { return start != other.start || end != other.end; }

	bool contains (const ValueType position) const throw()
	{
		return position >= start && position < end;
	}

	ValueType clipValue (const ValueType value) const throw()
	{
		return jlimit (start, end, value);
	}

	bool intersects (const Range& other) const throw()
	{
		return other.start < end && other.end > start;
	}

	const Range getIntersectionWith (const Range& other) const throw()
	{
		return Range (jmax (start, other.start),
					  jmin (end, other.end));
	}

	const Range getUnionWith (const Range& other) const throw()
	{
		return Range (jmin (start, other.start),
					  jmax (end, other.end));
	}

	const Range constrainRange (const Range& rangeToConstrain) const throw()
	{
		const ValueType otherLen = rangeToConstrain.getLength();
		return otherLen >= getLength()
				? *this
				: rangeToConstrain.movedToStartAt (jlimit (start, end - otherLen, rangeToConstrain.getStart()));
	}

	juce_UseDebuggingNewOperator

private:
	ValueType start, end;
};

#endif   // __JUCE_RANGE_JUCEHEADER__
/*** End of inlined file: juce_Range.h ***/


#endif
#ifndef __JUCE_REFERENCECOUNTEDARRAY_JUCEHEADER__

/*** Start of inlined file: juce_ReferenceCountedArray.h ***/
#ifndef __JUCE_REFERENCECOUNTEDARRAY_JUCEHEADER__
#define __JUCE_REFERENCECOUNTEDARRAY_JUCEHEADER__

template <class ObjectClass, class TypeOfCriticalSectionToUse = DummyCriticalSection>
class ReferenceCountedArray
{
public:

	ReferenceCountedArray() throw()
		: numUsed (0)
	{
	}

	ReferenceCountedArray (const ReferenceCountedArray<ObjectClass, TypeOfCriticalSectionToUse>& other) throw()
	{
		const ScopedLockType lock (other.getLock());
		numUsed = other.numUsed;
		data.setAllocatedSize (numUsed);
		memcpy (data.elements, other.data.elements, numUsed * sizeof (ObjectClass*));

		for (int i = numUsed; --i >= 0;)
			if (data.elements[i] != 0)
				data.elements[i]->incReferenceCount();
	}

	ReferenceCountedArray<ObjectClass, TypeOfCriticalSectionToUse>& operator= (const ReferenceCountedArray<ObjectClass, TypeOfCriticalSectionToUse>& other) throw()
	{
		if (this != &other)
		{
			ReferenceCountedArray<ObjectClass, TypeOfCriticalSectionToUse> otherCopy (other);
			swapWithArray (other);
		}

		return *this;
	}

	~ReferenceCountedArray()
	{
		clear();
	}

	void clear()
	{
		const ScopedLockType lock (getLock());

		while (numUsed > 0)
			if (data.elements [--numUsed] != 0)
				data.elements [numUsed]->decReferenceCount();

		jassert (numUsed == 0);
		data.setAllocatedSize (0);
	}

	inline int size() const throw()
	{
		return numUsed;
	}

	inline const ReferenceCountedObjectPtr<ObjectClass> operator[] (const int index) const throw()
	{
		const ScopedLockType lock (getLock());
		return (((unsigned int) index) < (unsigned int) numUsed) ? data.elements [index]
																 : static_cast <ObjectClass*> (0);
	}

	inline const ReferenceCountedObjectPtr<ObjectClass> getUnchecked (const int index) const throw()
	{
		const ScopedLockType lock (getLock());
		jassert (((unsigned int) index) < (unsigned int) numUsed);
		return data.elements [index];
	}

	inline const ReferenceCountedObjectPtr<ObjectClass> getFirst() const throw()
	{
		const ScopedLockType lock (getLock());
		return numUsed > 0 ? data.elements [0]
						   : static_cast <ObjectClass*> (0);
	}

	inline const ReferenceCountedObjectPtr<ObjectClass> getLast() const throw()
	{
		const ScopedLockType lock (getLock());
		return numUsed > 0 ? data.elements [numUsed - 1]
						   : static_cast <ObjectClass*> (0);
	}

	int indexOf (const ObjectClass* const objectToLookFor) const throw()
	{
		const ScopedLockType lock (getLock());
		ObjectClass** e = data.elements.getData();
		ObjectClass** const end = e + numUsed;

		while (e != end)
		{
			if (objectToLookFor == *e)
				return static_cast <int> (e - data.elements.getData());

			++e;
		}

		return -1;
	}

	bool contains (const ObjectClass* const objectToLookFor) const throw()
	{
		const ScopedLockType lock (getLock());
		ObjectClass** e = data.elements.getData();
		ObjectClass** const end = e + numUsed;

		while (e != end)
		{
			if (objectToLookFor == *e)
				return true;

			++e;
		}

		return false;
	}

	void add (ObjectClass* const newObject) throw()
	{
		const ScopedLockType lock (getLock());
		data.ensureAllocatedSize (numUsed + 1);
		data.elements [numUsed++] = newObject;

		if (newObject != 0)
			newObject->incReferenceCount();
	}

	void insert (int indexToInsertAt,
				 ObjectClass* const newObject) throw()
	{
		if (indexToInsertAt >= 0)
		{
			const ScopedLockType lock (getLock());

			if (indexToInsertAt > numUsed)
				indexToInsertAt = numUsed;

			data.ensureAllocatedSize (numUsed + 1);

			ObjectClass** const e = data.elements + indexToInsertAt;
			const int numToMove = numUsed - indexToInsertAt;

			if (numToMove > 0)
				memmove (e + 1, e, numToMove * sizeof (ObjectClass*));

			*e = newObject;

			if (newObject != 0)
				newObject->incReferenceCount();

			++numUsed;
		}
		else
		{
			add (newObject);
		}
	}

	void addIfNotAlreadyThere (ObjectClass* const newObject) throw()
	{
		const ScopedLockType lock (getLock());
		if (! contains (newObject))
			add (newObject);
	}

	void set (const int indexToChange,
			  ObjectClass* const newObject)
	{
		if (indexToChange >= 0)
		{
			const ScopedLockType lock (getLock());

			if (newObject != 0)
				newObject->incReferenceCount();

			if (indexToChange < numUsed)
			{
				if (data.elements [indexToChange] != 0)
					data.elements [indexToChange]->decReferenceCount();

				data.elements [indexToChange] = newObject;
			}
			else
			{
				data.ensureAllocatedSize (numUsed + 1);
				data.elements [numUsed++] = newObject;
			}
		}
	}

	void addArray (const ReferenceCountedArray<ObjectClass, TypeOfCriticalSectionToUse>& arrayToAddFrom,
				   int startIndex = 0,
				   int numElementsToAdd = -1) throw()
	{
		arrayToAddFrom.lockArray();
		const ScopedLockType lock (getLock());

		if (startIndex < 0)
		{
			jassertfalse
			startIndex = 0;
		}

		if (numElementsToAdd < 0 || startIndex + numElementsToAdd > arrayToAddFrom.size())
			numElementsToAdd = arrayToAddFrom.size() - startIndex;

		if (numElementsToAdd > 0)
		{
			data.ensureAllocatedSize (numUsed + numElementsToAdd);

			while (--numElementsToAdd >= 0)
				add (arrayToAddFrom.getUnchecked (startIndex++));
		}

		arrayToAddFrom.unlockArray();
	}

	template <class ElementComparator>
	void addSorted (ElementComparator& comparator,
					ObjectClass* newObject) throw()
	{
		const ScopedLockType lock (getLock());
		insert (findInsertIndexInSortedArray (comparator, data.elements.getData(), newObject, 0, numUsed), newObject);
	}

	template <class ElementComparator>
	void addOrReplaceSorted (ElementComparator& comparator,
							 ObjectClass* newObject) throw()
	{
		const ScopedLockType lock (getLock());
		const int index = findInsertIndexInSortedArray (comparator, data.elements.getData(), newObject, 0, numUsed);

		if (index > 0 && comparator.compareElements (newObject, data.elements [index - 1]) == 0)
			set (index - 1, newObject); // replace an existing object that matches
		else
			insert (index, newObject);  // no match, so insert the new one
	}

	void remove (const int indexToRemove)
	{
		const ScopedLockType lock (getLock());

		if (((unsigned int) indexToRemove) < (unsigned int) numUsed)
		{
			ObjectClass** const e = data.elements + indexToRemove;

			if (*e != 0)
				(*e)->decReferenceCount();

			--numUsed;
			const int numberToShift = numUsed - indexToRemove;

			if (numberToShift > 0)
				memmove (e, e + 1, numberToShift * sizeof (ObjectClass*));

			if ((numUsed << 1) < data.numAllocated)
				minimiseStorageOverheads();
		}
	}

	void removeObject (ObjectClass* const objectToRemove)
	{
		const ScopedLockType lock (getLock());
		remove (indexOf (objectToRemove));
	}

	void removeRange (const int startIndex,
					  const int numberToRemove)
	{
		const ScopedLockType lock (getLock());

		const int start = jlimit (0, numUsed, startIndex);
		const int end   = jlimit (0, numUsed, startIndex + numberToRemove);

		if (end > start)
		{
			int i;
			for (i = start; i < end; ++i)
			{
				if (data.elements[i] != 0)
				{
					data.elements[i]->decReferenceCount();
					data.elements[i] = 0; // (in case one of the destructors accesses this array and hits a dangling pointer)
				}
			}

			const int rangeSize = end - start;
			ObjectClass** e = data.elements + start;
			i = numUsed - end;
			numUsed -= rangeSize;

			while (--i >= 0)
			{
				*e = e [rangeSize];
				++e;
			}

			if ((numUsed << 1) < data.numAllocated)
				minimiseStorageOverheads();
		}
	}

	void removeLast (int howManyToRemove = 1)
	{
		const ScopedLockType lock (getLock());

		if (howManyToRemove > numUsed)
			howManyToRemove = numUsed;

		while (--howManyToRemove >= 0)
			remove (numUsed - 1);
	}

	void swap (const int index1,
			   const int index2) throw()
	{
		const ScopedLockType lock (getLock());

		if (((unsigned int) index1) < (unsigned int) numUsed
			 && ((unsigned int) index2) < (unsigned int) numUsed)
		{
			swapVariables (data.elements [index1],
						   data.elements [index2]);
		}
	}

	void move (const int currentIndex,
			   int newIndex) throw()
	{
		if (currentIndex != newIndex)
		{
			const ScopedLockType lock (getLock());

			if (((unsigned int) currentIndex) < (unsigned int) numUsed)
			{
				if (((unsigned int) newIndex) >= (unsigned int) numUsed)
					newIndex = numUsed - 1;

				ObjectClass* const value = data.elements [currentIndex];

				if (newIndex > currentIndex)
				{
					memmove (data.elements + currentIndex,
							 data.elements + currentIndex + 1,
							 (newIndex - currentIndex) * sizeof (ObjectClass*));
				}
				else
				{
					memmove (data.elements + newIndex + 1,
							 data.elements + newIndex,
							 (currentIndex - newIndex) * sizeof (ObjectClass*));
				}

				data.elements [newIndex] = value;
			}
		}
	}

	void swapWithArray (ReferenceCountedArray& otherArray) throw()
	{
		const ScopedLockType lock1 (getLock());
		const ScopedLockType lock2 (otherArray.getLock());

		data.swapWith (otherArray.data);
		swapVariables (numUsed, otherArray.numUsed);
	}

	bool operator== (const ReferenceCountedArray& other) const throw()
	{
		const ScopedLockType lock2 (other.getLock());
		const ScopedLockType lock1 (getLock());

		if (numUsed != other.numUsed)
			return false;

		for (int i = numUsed; --i >= 0;)
			if (data.elements [i] != other.data.elements [i])
				return false;

		return true;
	}

	bool operator!= (const ReferenceCountedArray<ObjectClass, TypeOfCriticalSectionToUse>& other) const throw()
	{
		return ! operator== (other);
	}

	template <class ElementComparator>
	void sort (ElementComparator& comparator,
			   const bool retainOrderOfEquivalentItems = false) const throw()
	{
		(void) comparator;  // if you pass in an object with a static compareElements() method, this
							// avoids getting warning messages about the parameter being unused

		const ScopedLockType lock (getLock());
		sortArray (comparator, data.elements.getData(), 0, size() - 1, retainOrderOfEquivalentItems);
	}

	void minimiseStorageOverheads() throw()
	{
		const ScopedLockType lock (getLock());
		data.shrinkToNoMoreThan (numUsed);
	}

	inline const TypeOfCriticalSectionToUse& getLock() const throw()	   { return data; }

	typedef typename TypeOfCriticalSectionToUse::ScopedLockType ScopedLockType;

	juce_UseDebuggingNewOperator

private:
	ArrayAllocationBase <ObjectClass*, TypeOfCriticalSectionToUse> data;
	int numUsed;
};

#endif   // __JUCE_REFERENCECOUNTEDARRAY_JUCEHEADER__
/*** End of inlined file: juce_ReferenceCountedArray.h ***/


#endif
#ifndef __JUCE_REFERENCECOUNTEDOBJECT_JUCEHEADER__

#endif
#ifndef __JUCE_SCOPEDPOINTER_JUCEHEADER__

#endif
#ifndef __JUCE_SORTEDSET_JUCEHEADER__

/*** Start of inlined file: juce_SortedSet.h ***/
#ifndef __JUCE_SORTEDSET_JUCEHEADER__
#define __JUCE_SORTEDSET_JUCEHEADER__

#if JUCE_MSVC
  #pragma warning (push)
  #pragma warning (disable: 4512)
#endif

template <class ElementType, class TypeOfCriticalSectionToUse = DummyCriticalSection>
class SortedSet
{
public:

	SortedSet() throw()
	   : numUsed (0)
	{
	}

	SortedSet (const SortedSet& other) throw()
	{
		const ScopedLockType lock (other.getLock());
		numUsed = other.numUsed;
		data.setAllocatedSize (other.numUsed);
		memcpy (data.elements, other.data.elements, numUsed * sizeof (ElementType));
	}

	~SortedSet() throw()
	{
	}

	SortedSet& operator= (const SortedSet& other) throw()
	{
		if (this != &other)
		{
			const ScopedLockType lock1 (other.getLock());
			const ScopedLockType lock2 (getLock());

			data.ensureAllocatedSize (other.size());
			numUsed = other.numUsed;
			memcpy (data.elements, other.data.elements, numUsed * sizeof (ElementType));
			minimiseStorageOverheads();
		}

		return *this;
	}

	bool operator== (const SortedSet<ElementType>& other) const throw()
	{
		const ScopedLockType lock (getLock());

		if (numUsed != other.numUsed)
			return false;

		for (int i = numUsed; --i >= 0;)
			if (data.elements[i] != other.data.elements[i])
				return false;

		return true;
	}

	bool operator!= (const SortedSet<ElementType>& other) const throw()
	{
		return ! operator== (other);
	}

	void clear() throw()
	{
		const ScopedLockType lock (getLock());
		data.setAllocatedSize (0);
		numUsed = 0;
	}

	void clearQuick() throw()
	{
		const ScopedLockType lock (getLock());
		numUsed = 0;
	}

	inline int size() const throw()
	{
		return numUsed;
	}

	inline ElementType operator[] (const int index) const throw()
	{
		const ScopedLockType lock (getLock());
		return (((unsigned int) index) < (unsigned int) numUsed) ? data.elements [index]
																 : ElementType();
	}

	inline ElementType getUnchecked (const int index) const throw()
	{
		const ScopedLockType lock (getLock());
		jassert (((unsigned int) index) < (unsigned int) numUsed);
		return data.elements [index];
	}

	inline ElementType getFirst() const throw()
	{
		const ScopedLockType lock (getLock());
		return numUsed > 0 ? data.elements [0] : ElementType();
	}

	inline ElementType getLast() const throw()
	{
		const ScopedLockType lock (getLock());
		return numUsed > 0 ? data.elements [numUsed - 1] : ElementType();
	}

	int indexOf (const ElementType elementToLookFor) const throw()
	{
		const ScopedLockType lock (getLock());

		int start = 0;
		int end = numUsed;

		for (;;)
		{
			if (start >= end)
			{
				return -1;
			}
			else if (elementToLookFor == data.elements [start])
			{
				return start;
			}
			else
			{
				const int halfway = (start + end) >> 1;

				if (halfway == start)
					return -1;
				else if (elementToLookFor >= data.elements [halfway])
					start = halfway;
				else
					end = halfway;
			}
		}
	}

	bool contains (const ElementType elementToLookFor) const throw()
	{
		const ScopedLockType lock (getLock());

		int start = 0;
		int end = numUsed;

		for (;;)
		{
			if (start >= end)
			{
				return false;
			}
			else if (elementToLookFor == data.elements [start])
			{
				return true;
			}
			else
			{
				const int halfway = (start + end) >> 1;

				if (halfway == start)
					return false;
				else if (elementToLookFor >= data.elements [halfway])
					start = halfway;
				else
					end = halfway;
			}
		}
	}

	void add (const ElementType newElement) throw()
	{
		const ScopedLockType lock (getLock());

		int start = 0;
		int end = numUsed;

		for (;;)
		{
			if (start >= end)
			{
				jassert (start <= end);
				insertInternal (start, newElement);
				break;
			}
			else if (newElement == data.elements [start])
			{
				break;
			}
			else
			{
				const int halfway = (start + end) >> 1;

				if (halfway == start)
				{
					if (newElement >= data.elements [halfway])
						insertInternal (start + 1, newElement);
					else
						insertInternal (start, newElement);

					break;
				}
				else if (newElement >= data.elements [halfway])
					start = halfway;
				else
					end = halfway;
			}
		}
	}

	void addArray (const ElementType* elementsToAdd,
				   int numElementsToAdd) throw()
	{
		const ScopedLockType lock (getLock());

		while (--numElementsToAdd >= 0)
			add (*elementsToAdd++);
	}

	template <class OtherSetType>
	void addSet (const OtherSetType& setToAddFrom,
				 int startIndex = 0,
				 int numElementsToAdd = -1) throw()
	{
		const typename OtherSetType::ScopedLockType lock1 (setToAddFrom.getLock());
		const ScopedLockType lock2 (getLock());
		jassert (this != &setToAddFrom);

		if (this != &setToAddFrom)
		{
			if (startIndex < 0)
			{
				jassertfalse
				startIndex = 0;
			}

			if (numElementsToAdd < 0 || startIndex + numElementsToAdd > setToAddFrom.size())
				numElementsToAdd = setToAddFrom.size() - startIndex;

			addArray (setToAddFrom.elements + startIndex, numElementsToAdd);
		}
	}

	ElementType remove (const int indexToRemove) throw()
	{
		const ScopedLockType lock (getLock());

		if (((unsigned int) indexToRemove) < (unsigned int) numUsed)
		{
			--numUsed;

			ElementType* const e = data.elements + indexToRemove;
			ElementType const removed = *e;
			const int numberToShift = numUsed - indexToRemove;

			if (numberToShift > 0)
				memmove (e, e + 1, numberToShift * sizeof (ElementType));

			if ((numUsed << 1) < data.numAllocated)
				minimiseStorageOverheads();

			return removed;
		}

		return 0;
	}

	void removeValue (const ElementType valueToRemove) throw()
	{
		const ScopedLockType lock (getLock());
		remove (indexOf (valueToRemove));
	}

	template <class OtherSetType>
	void removeValuesIn (const OtherSetType& otherSet) throw()
	{
		const typename OtherSetType::ScopedLockType lock1 (otherSet.getLock());
		const ScopedLockType lock2 (getLock());

		if (this == &otherSet)
		{
			clear();
		}
		else
		{
			if (otherSet.size() > 0)
			{
				for (int i = numUsed; --i >= 0;)
					if (otherSet.contains (data.elements [i]))
						remove (i);
			}
		}
	}

	template <class OtherSetType>
	void removeValuesNotIn (const OtherSetType& otherSet) throw()
	{
		const typename OtherSetType::ScopedLockType lock1 (otherSet.getLock());
		const ScopedLockType lock2 (getLock());

		if (this != &otherSet)
		{
			if (otherSet.size() <= 0)
			{
				clear();
			}
			else
			{
				for (int i = numUsed; --i >= 0;)
					if (! otherSet.contains (data.elements [i]))
						remove (i);
			}
		}
	}

	void minimiseStorageOverheads() throw()
	{
		const ScopedLockType lock (getLock());
		data.shrinkToNoMoreThan (numUsed);
	}

	inline const TypeOfCriticalSectionToUse& getLock() const throw()	   { return data; }

	typedef typename TypeOfCriticalSectionToUse::ScopedLockType ScopedLockType;

	juce_UseDebuggingNewOperator

private:
	ArrayAllocationBase <ElementType, TypeOfCriticalSectionToUse> data;
	int numUsed;

	void insertInternal (const int indexToInsertAt, const ElementType newElement) throw()
	{
		data.ensureAllocatedSize (numUsed + 1);

		ElementType* const insertPos = data.elements + indexToInsertAt;
		const int numberToMove = numUsed - indexToInsertAt;

		if (numberToMove > 0)
			memmove (insertPos + 1, insertPos, numberToMove * sizeof (ElementType));

		*insertPos = newElement;
		++numUsed;
	}
};

#if JUCE_MSVC
  #pragma warning (pop)
#endif

#endif   // __JUCE_SORTEDSET_JUCEHEADER__
/*** End of inlined file: juce_SortedSet.h ***/


#endif
#ifndef __JUCE_SPARSESET_JUCEHEADER__

/*** Start of inlined file: juce_SparseSet.h ***/
#ifndef __JUCE_SPARSESET_JUCEHEADER__
#define __JUCE_SPARSESET_JUCEHEADER__

template <class Type>
class SparseSet
{
public:

	SparseSet() throw()
	{
	}

	SparseSet (const SparseSet<Type>& other) throw()
		: values (other.values)
	{
	}

	~SparseSet() throw()
	{
	}

	void clear() throw()
	{
		values.clear();
	}

	bool isEmpty() const throw()
	{
		return values.size() == 0;
	}

	Type size() const throw()
	{
		Type num = 0;

		for (int i = 0; i < values.size(); i += 2)
			num += values[i + 1] - values[i];

		return num;
	}

	Type operator[] (int index) const throw()
	{
		for (int i = 0; i < values.size(); i += 2)
		{
			const Type s = values.getUnchecked(i);
			const Type e = values.getUnchecked(i + 1);

			if (index < e - s)
				return s + index;

			index -= e - s;
		}

		return (Type) 0;
	}

	bool contains (const Type valueToLookFor) const throw()
	{
		bool on = false;

		for (int i = 0; i < values.size(); ++i)
		{
			if (values.getUnchecked(i) > valueToLookFor)
				return on;

			on = ! on;
		}

		return false;
	}

	int getNumRanges() const throw()
	{
		return values.size() >> 1;
	}

	bool getRange (const int rangeIndex,
				   Type& startValue,
				   Type& numValues) const throw()
	{
		if (((unsigned int) rangeIndex) < (unsigned int) getNumRanges())
		{
			startValue = values [rangeIndex << 1];
			numValues = values [(rangeIndex << 1) + 1] - startValue;

			return true;
		}

		return false;
	}

	bool getTotalRange (Type& lowestValue,
						Type& highestValue) const throw()
	{
		if (values.size() > 0)
		{
			lowestValue = values.getUnchecked (0);
			highestValue = values.getUnchecked (values.size() - 1);
			return true;
		}

		return false;
	}

	void addRange (const Type firstValue,
				   const Type numValuesToAdd) throw()
	{
		jassert (numValuesToAdd >= 0);

		if (numValuesToAdd > 0)
		{
			removeRange (firstValue, numValuesToAdd);

			IntegerElementComparator<Type> sorter;
			values.addSorted (sorter, firstValue);
			values.addSorted (sorter, firstValue + numValuesToAdd);

			simplify();
		}
	}

	void removeRange (const Type firstValue,
					  const Type numValuesToRemove) throw()
	{
		jassert (numValuesToRemove >= 0);

		if (numValuesToRemove >= 0
			 && firstValue < values.getLast())
		{
			const bool onAtStart = contains (firstValue - 1);
			const Type lastValue = firstValue + jmin (numValuesToRemove, values.getLast() - firstValue);
			const bool onAtEnd = contains (lastValue);

			for (int i = values.size(); --i >= 0;)
			{
				if (values.getUnchecked(i) <= lastValue)
				{
					while (values.getUnchecked(i) >= firstValue)
					{
						values.remove (i);

						if (--i < 0)
							break;
					}

					break;
				}
			}

			IntegerElementComparator<Type> sorter;

			if (onAtStart)
				values.addSorted (sorter, firstValue);

			if (onAtEnd)
				values.addSorted (sorter, lastValue);

			simplify();
		}
	}

	void invertRange (const Type firstValue,
					  const Type numValues)
	{
		SparseSet newItems;
		newItems.addRange (firstValue, numValues);

		int i;
		for (i = getNumRanges(); --i >= 0;)
		{
			const int start = values [i << 1];
			const int end = values [(i << 1) + 1];

			newItems.removeRange (start, end);
		}

		removeRange (firstValue, numValues);

		for (i = newItems.getNumRanges(); --i >= 0;)
		{
			const int start = newItems.values [i << 1];
			const int end = newItems.values [(i << 1) + 1];

			addRange (start, end);
		}
	}

	bool overlapsRange (const Type firstValue,
						const Type numValues) throw()
	{
		jassert (numValues >= 0);

		if (numValues > 0)
		{
			for (int i = getNumRanges(); --i >= 0;)
			{
				if (firstValue >= values.getUnchecked ((i << 1) + 1))
					return false;

				if (firstValue + numValues > values.getUnchecked (i << 1))
					return true;
			}
		}

		return false;
	}

	bool containsRange (const Type firstValue,
						const Type numValues) throw()
	{
		jassert (numValues >= 0);

		if (numValues > 0)
		{
			for (int i = getNumRanges(); --i >= 0;)
			{
				if (firstValue >= values.getUnchecked ((i << 1) + 1))
					return false;

				if (firstValue >= values.getUnchecked (i << 1)
					 && firstValue + numValues <= values.getUnchecked ((i << 1) + 1))
					return true;
			}
		}

		return false;
	}

	bool operator== (const SparseSet<Type>& other) throw()
	{
		return values == other.values;
	}

	bool operator!= (const SparseSet<Type>& other) throw()
	{
		return values != other.values;
	}

	juce_UseDebuggingNewOperator

private:
	// alternating start/end values of ranges of values that are present.
	Array<Type, DummyCriticalSection> values;

	void simplify() throw()
	{
		jassert ((values.size() & 1) == 0);

		for (int i = values.size(); --i > 0;)
			if (values.getUnchecked(i) == values.getUnchecked (i - 1))
				values.removeRange (i - 1, 2);
	}
};

#endif   // __JUCE_SPARSESET_JUCEHEADER__
/*** End of inlined file: juce_SparseSet.h ***/


#endif
#ifndef __JUCE_VALUE_JUCEHEADER__

/*** Start of inlined file: juce_Value.h ***/
#ifndef __JUCE_VALUE_JUCEHEADER__
#define __JUCE_VALUE_JUCEHEADER__


/*** Start of inlined file: juce_AsyncUpdater.h ***/
#ifndef __JUCE_ASYNCUPDATER_JUCEHEADER__
#define __JUCE_ASYNCUPDATER_JUCEHEADER__


/*** Start of inlined file: juce_MessageListener.h ***/
#ifndef __JUCE_MESSAGELISTENER_JUCEHEADER__
#define __JUCE_MESSAGELISTENER_JUCEHEADER__


/*** Start of inlined file: juce_Message.h ***/
#ifndef __JUCE_MESSAGE_JUCEHEADER__
#define __JUCE_MESSAGE_JUCEHEADER__

class MessageListener;
class MessageManager;

class JUCE_API  Message
{
public:

	Message() throw();

	Message (int intParameter1,
			 int intParameter2,
			 int intParameter3,
			 void* pointerParameter) throw();

	virtual ~Message() throw();

	// These values can be used for carrying simple data that the application needs to
	// pass around. For more complex messages, just create a subclass.

	int intParameter1;	  /**< user-defined integer value. */
	int intParameter2;	  /**< user-defined integer value. */
	int intParameter3;	  /**< user-defined integer value. */
	void* pointerParameter;	 /**< user-defined pointer value. */

	juce_UseDebuggingNewOperator

private:
	friend class MessageListener;
	friend class MessageManager;
	MessageListener* messageRecipient;

	Message (const Message&);
	Message& operator= (const Message&);
};

#endif   // __JUCE_MESSAGE_JUCEHEADER__
/*** End of inlined file: juce_Message.h ***/

class JUCE_API  MessageListener
{
protected:

	MessageListener() throw();

public:

	virtual ~MessageListener();

	virtual void handleMessage (const Message& message) = 0;

	void postMessage (Message* message) const throw();

	bool isValidMessageListener() const throw();
};

#endif   // __JUCE_MESSAGELISTENER_JUCEHEADER__
/*** End of inlined file: juce_MessageListener.h ***/

class JUCE_API  AsyncUpdater
{
public:

	AsyncUpdater() throw();

	virtual ~AsyncUpdater();

	void triggerAsyncUpdate() throw();

	void cancelPendingUpdate() throw();

	void handleUpdateNowIfNeeded();

	virtual void handleAsyncUpdate() = 0;

private:

	class AsyncUpdaterInternal  : public MessageListener
	{
	public:
		AsyncUpdaterInternal() throw() {}
		~AsyncUpdaterInternal() {}

		void handleMessage (const Message&);

		AsyncUpdater* owner;

	private:
		AsyncUpdaterInternal (const AsyncUpdaterInternal&);
		AsyncUpdaterInternal& operator= (const AsyncUpdaterInternal&);
	};

	AsyncUpdaterInternal internalAsyncHandler;
	bool asyncMessagePending;
};

#endif   // __JUCE_ASYNCUPDATER_JUCEHEADER__
/*** End of inlined file: juce_AsyncUpdater.h ***/


/*** Start of inlined file: juce_ListenerList.h ***/
#ifndef __JUCE_LISTENERLIST_JUCEHEADER__
#define __JUCE_LISTENERLIST_JUCEHEADER__

template <class ListenerClass,
		  class ArrayType = Array <ListenerClass*> >
class ListenerList
{
	// Horrible macros required to support VC6/7..
	#if defined (_MSC_VER) && _MSC_VER <= 1400
	  #define LL_TEMPLATE(a)   typename P##a, typename Q##a
	  #define LL_PARAM(a)	  Q##a& param##a
	#else
	  #define LL_TEMPLATE(a)   typename P##a
	  #define LL_PARAM(a)	  PARAMETER_TYPE(P##a) param##a
	#endif

public:

	ListenerList()
	{
	}

	~ListenerList()
	{
	}

	void add (ListenerClass* const listenerToAdd)
	{
		// Listeners can't be null pointers!
		jassert (listenerToAdd != 0);

		if (listenerToAdd != 0)
			listeners.addIfNotAlreadyThere (listenerToAdd);
	}

	void remove (ListenerClass* const listenerToRemove)
	{
		// Listeners can't be null pointers!
		jassert (listenerToRemove != 0);

		listeners.removeValue (listenerToRemove);
	}

	int size() const throw()
	{
		return listeners.size();
	}

	bool isEmpty() const throw()
	{
		return listeners.size() == 0;
	}

	bool contains (ListenerClass* const listener) const throw()
	{
		return listeners.contains (listener);
	}

	void call (void (ListenerClass::*callbackFunction) ())
	{
		callChecked (static_cast <const DummyBailOutChecker&> (DummyBailOutChecker()), callbackFunction);
	}

	template <class BailOutCheckerType>
	void callChecked (const BailOutCheckerType& bailOutChecker,
					  void (ListenerClass::*callbackFunction) ())
	{
		for (Iterator<BailOutCheckerType, ThisType> iter (*this, bailOutChecker); iter.next();)
			(iter.getListener()->*callbackFunction) ();
	}

	template <LL_TEMPLATE(1)>
	void call (void (ListenerClass::*callbackFunction) (P1), LL_PARAM(1))
	{
		for (Iterator<DummyBailOutChecker, ThisType> iter (*this, DummyBailOutChecker()); iter.next();)
			(iter.getListener()->*callbackFunction) (param1);
	}

	template <class BailOutCheckerType, LL_TEMPLATE(1)>
	void callChecked (const BailOutCheckerType& bailOutChecker,
					  void (ListenerClass::*callbackFunction) (P1),
					  LL_PARAM(1))
	{
		for (Iterator<BailOutCheckerType, ThisType> iter (*this, bailOutChecker); iter.next();)
			(iter.getListener()->*callbackFunction) (param1);
	}

	template <LL_TEMPLATE(1), LL_TEMPLATE(2)>
	void call (void (ListenerClass::*callbackFunction) (P1, P2),
			   LL_PARAM(1), LL_PARAM(2))
	{
		for (Iterator<DummyBailOutChecker, ThisType> iter (*this, DummyBailOutChecker()); iter.next();)
			(iter.getListener()->*callbackFunction) (param1, param2);
	}

	template <class BailOutCheckerType, LL_TEMPLATE(1), LL_TEMPLATE(2)>
	void callChecked (const BailOutCheckerType& bailOutChecker,
					  void (ListenerClass::*callbackFunction) (P1, P2),
					  LL_PARAM(1), LL_PARAM(2))
	{
		for (Iterator<BailOutCheckerType, ThisType> iter (*this, bailOutChecker); iter.next();)
			(iter.getListener()->*callbackFunction) (param1, param2);
	}

	template <LL_TEMPLATE(1), LL_TEMPLATE(2), LL_TEMPLATE(3)>
	void call (void (ListenerClass::*callbackFunction) (P1, P2, P3),
			   LL_PARAM(1), LL_PARAM(2), LL_PARAM(3))
	{
		for (Iterator<DummyBailOutChecker, ThisType> iter (*this, DummyBailOutChecker()); iter.next();)
			(iter.getListener()->*callbackFunction) (param1, param2, param3);
	}

	template <class BailOutCheckerType, LL_TEMPLATE(1), LL_TEMPLATE(2), LL_TEMPLATE(3)>
	void callChecked (const BailOutCheckerType& bailOutChecker,
					  void (ListenerClass::*callbackFunction) (P1, P2, P3),
					  LL_PARAM(1), LL_PARAM(2), LL_PARAM(3))
	{
		for (Iterator<BailOutCheckerType, ThisType> iter (*this, bailOutChecker); iter.next();)
			(iter.getListener()->*callbackFunction) (param1, param2, param3);
	}

	template <LL_TEMPLATE(1), LL_TEMPLATE(2), LL_TEMPLATE(3), LL_TEMPLATE(4)>
	void call (void (ListenerClass::*callbackFunction) (P1, P2, P3, P4),
			   LL_PARAM(1), LL_PARAM(2), LL_PARAM(3), LL_PARAM(4))
	{
		for (Iterator<DummyBailOutChecker, ThisType> iter (*this, DummyBailOutChecker()); iter.next();)
			(iter.getListener()->*callbackFunction) (param1, param2, param3, param4);
	}

	template <class BailOutCheckerType, LL_TEMPLATE(1), LL_TEMPLATE(2), LL_TEMPLATE(3), LL_TEMPLATE(4)>
	void callChecked (const BailOutCheckerType& bailOutChecker,
					  void (ListenerClass::*callbackFunction) (P1, P2, P3, P4),
					  LL_PARAM(1), LL_PARAM(2), LL_PARAM(3), LL_PARAM(4))
	{
		for (Iterator<BailOutCheckerType, ThisType> iter (*this, bailOutChecker); iter.next();)
			(iter.getListener()->*callbackFunction) (param1, param2, param3, param4);
	}

	template <LL_TEMPLATE(1), LL_TEMPLATE(2), LL_TEMPLATE(3), LL_TEMPLATE(4), LL_TEMPLATE(5)>
	void call (void (ListenerClass::*callbackFunction) (P1, P2, P3, P4, P5),
			   LL_PARAM(1), LL_PARAM(2), LL_PARAM(3), LL_PARAM(4), LL_PARAM(5))
	{
		for (Iterator<DummyBailOutChecker, ThisType> iter (*this, DummyBailOutChecker()); iter.next();)
			(iter.getListener()->*callbackFunction) (param1, param2, param3, param4, param5);
	}

	template <class BailOutCheckerType, LL_TEMPLATE(1), LL_TEMPLATE(2), LL_TEMPLATE(3), LL_TEMPLATE(4), LL_TEMPLATE(5)>
	void callChecked (const BailOutCheckerType& bailOutChecker,
					  void (ListenerClass::*callbackFunction) (P1, P2, P3, P4, P5),
					  LL_PARAM(1), LL_PARAM(2), LL_PARAM(3), LL_PARAM(4), LL_PARAM(5))
	{
		for (Iterator<BailOutCheckerType, ThisType> iter (*this, bailOutChecker); iter.next();)
			(iter.getListener()->*callbackFunction) (param1, param2, param3, param4, param5);
	}

	class DummyBailOutChecker
	{
	public:
		inline bool shouldBailOut() const throw()	  { return false; }
	};

	template <class BailOutCheckerType, class ListType>
	class Iterator
	{
	public:

		Iterator (const ListType& list_, const BailOutCheckerType& bailOutChecker_)
			: list (list_), bailOutChecker (bailOutChecker_), index (list_.size())
		{}

		~Iterator() {}

		bool next()
		{
			if (index <= 0 || bailOutChecker.shouldBailOut())
				return false;

			const int listSize = list.size();

			if (--index < listSize)
				return true;

			index = listSize - 1;
			return index >= 0;
		}

		typename ListType::ListenerType* getListener() const throw()
		{
			return list.getListeners().getUnchecked (index);
		}

	private:
		const ListType& list;
		const BailOutCheckerType& bailOutChecker;
		int index;

		Iterator (const Iterator&);
		Iterator& operator= (const Iterator&);
	};

	typedef ListenerList<ListenerClass, ArrayType> ThisType;
	typedef ListenerClass ListenerType;

	const ArrayType& getListeners() const throw()	   { return listeners; }

private:

	ArrayType listeners;

	ListenerList (const ListenerList&);
	ListenerList& operator= (const ListenerList&);

	#undef LL_TEMPLATE
	#undef LL_PARAM
};

#endif   // __JUCE_LISTENERLIST_JUCEHEADER__
/*** End of inlined file: juce_ListenerList.h ***/

class JUCE_API  Value
{
public:

	Value();

	Value (const Value& other);

	Value (const var& initialValue);

	~Value();

	const var getValue() const;

	operator const var() const;

	const String toString() const;

	void setValue (const var& newValue);

	Value& operator= (const var& newValue);

	void referTo (const Value& valueToReferTo);

	bool refersToSameSourceAs (const Value& other) const;

	bool operator== (const Value& other) const;

	bool operator!= (const Value& other) const;

	class JUCE_API  Listener
	{
	public:
		Listener()	  {}
		virtual ~Listener() {}

		virtual void valueChanged (Value& value) = 0;
	};

	void addListener (Listener* const listener);

	void removeListener (Listener* const listener);

	class JUCE_API  ValueSource   : public ReferenceCountedObject,
									public AsyncUpdater
	{
	public:
		ValueSource();
		virtual ~ValueSource();

		virtual const var getValue() const = 0;
		virtual void setValue (const var& newValue) = 0;

		void sendChangeMessage (const bool dispatchSynchronously);

		juce_UseDebuggingNewOperator

	protected:
		friend class Value;
		SortedSet <Value*> valuesWithListeners;

		void handleAsyncUpdate();

		ValueSource (const ValueSource&);
		ValueSource& operator= (const ValueSource&);
	};

	explicit Value (ValueSource* const valueSource);
	ValueSource& getValueSource()	   { return *value; }

	juce_UseDebuggingNewOperator

private:
	friend class ValueSource;
	ReferenceCountedObjectPtr <ValueSource> value;
	ListenerList <Listener> listeners;

	void callListeners();

	// This is disallowed to avoid confusion about whether it should
	// do a by-value or by-reference copy.
	Value& operator= (const Value& other);
};

OutputStream& JUCE_CALLTYPE operator<< (OutputStream& stream, const Value& value);

#endif   // __JUCE_VALUE_JUCEHEADER__
/*** End of inlined file: juce_Value.h ***/


#endif
#ifndef __JUCE_VALUETREE_JUCEHEADER__

/*** Start of inlined file: juce_ValueTree.h ***/
#ifndef __JUCE_VALUETREE_JUCEHEADER__
#define __JUCE_VALUETREE_JUCEHEADER__


/*** Start of inlined file: juce_UndoManager.h ***/
#ifndef __JUCE_UNDOMANAGER_JUCEHEADER__
#define __JUCE_UNDOMANAGER_JUCEHEADER__


/*** Start of inlined file: juce_ChangeBroadcaster.h ***/
#ifndef __JUCE_CHANGEBROADCASTER_JUCEHEADER__
#define __JUCE_CHANGEBROADCASTER_JUCEHEADER__


/*** Start of inlined file: juce_ChangeListenerList.h ***/
#ifndef __JUCE_CHANGELISTENERLIST_JUCEHEADER__
#define __JUCE_CHANGELISTENERLIST_JUCEHEADER__


/*** Start of inlined file: juce_ChangeListener.h ***/
#ifndef __JUCE_CHANGELISTENER_JUCEHEADER__
#define __JUCE_CHANGELISTENER_JUCEHEADER__

class JUCE_API  ChangeListener
{
public:
	virtual ~ChangeListener()  {}

	virtual void changeListenerCallback (void* objectThatHasChanged) = 0;
};

#endif   // __JUCE_CHANGELISTENER_JUCEHEADER__
/*** End of inlined file: juce_ChangeListener.h ***/


/*** Start of inlined file: juce_ScopedLock.h ***/
#ifndef __JUCE_SCOPEDLOCK_JUCEHEADER__
#define __JUCE_SCOPEDLOCK_JUCEHEADER__

class JUCE_API  ScopedLock
{
public:

	inline explicit ScopedLock (const CriticalSection& lock) throw()	: lock_ (lock) { lock.enter(); }

	inline ~ScopedLock() throw()					{ lock_.exit(); }

private:

	const CriticalSection& lock_;

	ScopedLock (const ScopedLock&);
	ScopedLock& operator= (const ScopedLock&);
};

class ScopedUnlock
{
public:

	inline explicit ScopedUnlock (const CriticalSection& lock) throw()	: lock_ (lock) { lock.exit(); }

	inline ~ScopedUnlock() throw()					{ lock_.enter(); }

private:

	const CriticalSection& lock_;

	ScopedUnlock (const ScopedLock&);
	ScopedUnlock& operator= (const ScopedUnlock&);
};

#endif   // __JUCE_SCOPEDLOCK_JUCEHEADER__
/*** End of inlined file: juce_ScopedLock.h ***/

class JUCE_API  ChangeListenerList  : public MessageListener
{
public:

	ChangeListenerList() throw();

	~ChangeListenerList() throw();

	void addChangeListener (ChangeListener* listener) throw();

	void removeChangeListener (ChangeListener* listener) throw();

	void removeAllChangeListeners() throw();

	void sendChangeMessage (void* objectThatHasChanged) throw();

	void sendSynchronousChangeMessage (void* objectThatHasChanged);

	void dispatchPendingMessages();

	void handleMessage (const Message&);

	juce_UseDebuggingNewOperator

private:
	SortedSet <void*> listeners;
	CriticalSection lock;
	void* lastChangedObject;
	bool messagePending;

	ChangeListenerList (const ChangeListenerList&);
	ChangeListenerList& operator= (const ChangeListenerList&);
};

#endif   // __JUCE_CHANGELISTENERLIST_JUCEHEADER__
/*** End of inlined file: juce_ChangeListenerList.h ***/

class JUCE_API  ChangeBroadcaster
{
public:

	ChangeBroadcaster() throw();

	virtual ~ChangeBroadcaster();

	void addChangeListener (ChangeListener* listener) throw();

	void removeChangeListener (ChangeListener* listener) throw();

	void removeAllChangeListeners() throw();

	void sendChangeMessage (void* objectThatHasChanged) throw();

	void sendSynchronousChangeMessage (void* objectThatHasChanged);

	void dispatchPendingMessages();

private:

	ChangeListenerList changeListenerList;

	ChangeBroadcaster (const ChangeBroadcaster&);
	ChangeBroadcaster& operator= (const ChangeBroadcaster&);
};

#endif   // __JUCE_CHANGEBROADCASTER_JUCEHEADER__
/*** End of inlined file: juce_ChangeBroadcaster.h ***/


/*** Start of inlined file: juce_UndoableAction.h ***/
#ifndef __JUCE_UNDOABLEACTION_JUCEHEADER__
#define __JUCE_UNDOABLEACTION_JUCEHEADER__

class JUCE_API  UndoableAction
{
protected:
	UndoableAction() throw()	{}

public:
	virtual ~UndoableAction()   {}

	virtual bool perform() = 0;

	virtual bool undo() = 0;

	virtual int getSizeInUnits()	{ return 10; }
};

#endif   // __JUCE_UNDOABLEACTION_JUCEHEADER__
/*** End of inlined file: juce_UndoableAction.h ***/

class JUCE_API  UndoManager  : public ChangeBroadcaster
{
public:

	UndoManager (int maxNumberOfUnitsToKeep = 30000,
				 int minimumTransactionsToKeep = 30);

	~UndoManager();

	void clearUndoHistory();

	int getNumberOfUnitsTakenUpByStoredCommands() const;

	void setMaxNumberOfStoredUnits (int maxNumberOfUnitsToKeep,
									int minimumTransactionsToKeep);

	bool perform (UndoableAction* action,
				  const String& actionName = String::empty);

	void beginNewTransaction (const String& actionName = String::empty);

	void setCurrentTransactionName (const String& newName);

	bool canUndo() const;

	const String getUndoDescription() const;

	bool undo();

	bool undoCurrentTransactionOnly();

	void getActionsInCurrentTransaction (Array <const UndoableAction*>& actionsFound) const;

	int getNumActionsInCurrentTransaction() const;

	bool canRedo() const;

	const String getRedoDescription() const;

	bool redo();

	juce_UseDebuggingNewOperator

private:

	OwnedArray <OwnedArray <UndoableAction> > transactions;
	StringArray transactionNames;
	String currentTransactionName;
	int totalUnitsStored, maxNumUnitsToKeep, minimumTransactionsToKeep, nextIndex;
	bool newTransaction, reentrancyCheck;

	// disallow copy constructor
	UndoManager (const UndoManager&);
	UndoManager& operator= (const UndoManager&);
};

#endif   // __JUCE_UNDOMANAGER_JUCEHEADER__
/*** End of inlined file: juce_UndoManager.h ***/

class JUCE_API  ValueTree
{
public:

	explicit ValueTree (const String& type);

	ValueTree (const ValueTree& other);

	ValueTree& operator= (const ValueTree& other);

	~ValueTree();

	bool operator== (const ValueTree& other) const;

	bool operator!= (const ValueTree& other) const;

	bool isValid() const				{ return object != 0; }

	ValueTree createCopy() const;

	const String getType() const;

	bool hasType (const String& typeName) const;

	const var& getProperty (const var::identifier& name) const;

	const var& operator[] (const var::identifier& name) const;

	void setProperty (const var::identifier& name, const var& newValue, UndoManager* const undoManager);

	bool hasProperty (const var::identifier& name) const;

	void removeProperty (const var::identifier& name, UndoManager* const undoManager);

	void removeAllProperties (UndoManager* const undoManager);

	int getNumProperties() const;

	const var::identifier getPropertyName (int index) const;

	Value getPropertyAsValue (const var::identifier& name, UndoManager* const undoManager) const;

	int getNumChildren() const;

	ValueTree getChild (int index) const;

	ValueTree getChildWithName (const String& type) const;

	ValueTree getChildWithProperty (const var::identifier& propertyName, const var& propertyValue) const;

	void addChild (ValueTree child, int index, UndoManager* const undoManager);

	void removeChild (ValueTree& child, UndoManager* const undoManager);

	void removeChild (const int childIndex, UndoManager* const undoManager);

	void removeAllChildren (UndoManager* const undoManager);

	bool isAChildOf (const ValueTree& possibleParent) const;

	ValueTree getParent() const;

	XmlElement* createXml() const;

	static ValueTree fromXml (const XmlElement& xml);

	void writeToStream (OutputStream& output);

	static ValueTree readFromStream (InputStream& input);

	class JUCE_API  Listener
	{
	public:
		virtual ~Listener() {}

		virtual void valueTreePropertyChanged (ValueTree& treeWhosePropertyHasChanged,
											   const var::identifier& property) = 0;

		virtual void valueTreeChildrenChanged (ValueTree& treeWhoseChildHasChanged) = 0;

		virtual void valueTreeParentChanged (ValueTree& treeWhoseParentHasChanged) = 0;
	};

	void addListener (Listener* listener);

	void removeListener (Listener* listener);

	template <typename ElementComparator>
	void sort (ElementComparator& comparator, const bool retainOrderOfEquivalentItems = false)
	{
		if (object != 0)
		{
			ComparatorAdapter <ElementComparator> adapter (comparator);
			object->children.sort (adapter, retainOrderOfEquivalentItems);
			object->sendChildChangeMessage();
		}
	}

	static ValueTree invalid;

	juce_UseDebuggingNewOperator

private:
	friend class ValueTreeSetPropertyAction;
	friend class ValueTreeChildChangeAction;

	class JUCE_API  SharedObject	: public ReferenceCountedObject
	{
	public:
		SharedObject (const String& type);
		SharedObject (const SharedObject& other);
		~SharedObject();

		const String type;
		NamedValueSet properties;
		ReferenceCountedArray <SharedObject> children;
		SortedSet <ValueTree*> valueTreesWithListeners;
		SharedObject* parent;

		void sendPropertyChangeMessage (const var::identifier& property);
		void sendPropertyChangeMessage (ValueTree& tree, const var::identifier& property);
		void sendChildChangeMessage();
		void sendChildChangeMessage (ValueTree& tree);
		void sendParentChangeMessage();
		const var& getProperty (const var::identifier& name) const;
		void setProperty (const var::identifier& name, const var& newValue, UndoManager* const undoManager);
		bool hasProperty (const var::identifier& name) const;
		void removeProperty (const var::identifier& name, UndoManager* const undoManager);
		void removeAllProperties (UndoManager* const undoManager);
		bool isAChildOf (const SharedObject* const possibleParent) const;
		ValueTree getChildWithName (const String& type) const;
		ValueTree getChildWithProperty (const var::identifier& propertyName, const var& propertyValue) const;
		void addChild (SharedObject* child, int index, UndoManager* const undoManager);
		void removeChild (const int childIndex, UndoManager* const undoManager);
		void removeAllChildren (UndoManager* const undoManager);
		XmlElement* createXml() const;

		juce_UseDebuggingNewOperator

	private:
		SharedObject& operator= (const SharedObject&);
	};

	template <typename ElementComparator>
	class ComparatorAdapter
	{
	public:
		ComparatorAdapter (ElementComparator& comparator_) throw()  : comparator (comparator_) {}

		int compareElements (SharedObject* const first, SharedObject* const second)
		{
			return comparator.compareElements (ValueTree (first), ValueTree (second));
		}

	private:
		ElementComparator& comparator;
	};

	friend class SharedObject;

	typedef ReferenceCountedObjectPtr <SharedObject> SharedObjectPtr;

	ReferenceCountedObjectPtr <SharedObject> object;
	ListenerList <Listener> listeners;

public:
	explicit ValueTree (SharedObject* const object_);  // (can be made private when VC6 support is finally dropped)
};

#endif   // __JUCE_VALUETREE_JUCEHEADER__
/*** End of inlined file: juce_ValueTree.h ***/


#endif
#ifndef __JUCE_VARIANT_JUCEHEADER__

#endif
#ifndef __JUCE_VOIDARRAY_JUCEHEADER__

/*** Start of inlined file: juce_VoidArray.h ***/
#ifndef __JUCE_VOIDARRAY_JUCEHEADER__
#define __JUCE_VOIDARRAY_JUCEHEADER__

typedef Array <void*> VoidArray;

#endif   // __JUCE_VOIDARRAY_JUCEHEADER__
/*** End of inlined file: juce_VoidArray.h ***/


#endif
#ifndef __JUCE_ATOMIC_JUCEHEADER__

#endif
#ifndef __JUCE_BYTEORDER_JUCEHEADER__

#endif
#ifndef __JUCE_FILELOGGER_JUCEHEADER__

/*** Start of inlined file: juce_FileLogger.h ***/
#ifndef __JUCE_FILELOGGER_JUCEHEADER__
#define __JUCE_FILELOGGER_JUCEHEADER__

class JUCE_API  FileLogger  : public Logger
{
public:

	FileLogger (const File& fileToWriteTo,
				const String& welcomeMessage,
				const int maxInitialFileSizeBytes = 128 * 1024);

	~FileLogger();

	void logMessage (const String& message);

	const File getLogFile() const		   { return logFile; }

	static FileLogger* createDefaultAppLogger (const String& logFileSubDirectoryName,
											   const String& logFileName,
											   const String& welcomeMessage,
											   const int maxInitialFileSizeBytes = 128 * 1024);

	juce_UseDebuggingNewOperator

private:
	File logFile;
	CriticalSection logLock;
	ScopedPointer <FileOutputStream> logStream;

	void trimFileSize (int maxFileSizeBytes) const;

	FileLogger (const FileLogger&);
	FileLogger& operator= (const FileLogger&);
};

#endif   // __JUCE_FILELOGGER_JUCEHEADER__
/*** End of inlined file: juce_FileLogger.h ***/


#endif
#ifndef __JUCE_INITIALISATION_JUCEHEADER__

/*** Start of inlined file: juce_Initialisation.h ***/
#ifndef __JUCE_INITIALISATION_JUCEHEADER__
#define __JUCE_INITIALISATION_JUCEHEADER__

void JUCE_PUBLIC_FUNCTION  initialiseJuce_GUI();

void JUCE_PUBLIC_FUNCTION  shutdownJuce_GUI();

void JUCE_PUBLIC_FUNCTION  initialiseJuce_NonGUI();

void JUCE_PUBLIC_FUNCTION  shutdownJuce_NonGUI();

class ScopedJuceInitialiser_NonGUI
{
public:
	ScopedJuceInitialiser_NonGUI()	  { initialiseJuce_NonGUI(); }

	~ScopedJuceInitialiser_NonGUI()	 { shutdownJuce_NonGUI(); }
};

class ScopedJuceInitialiser_GUI
{
public:
	ScopedJuceInitialiser_GUI()	 { initialiseJuce_GUI(); }

	~ScopedJuceInitialiser_GUI()	{ shutdownJuce_GUI(); }
};

#endif   // __JUCE_INITIALISATION_JUCEHEADER__
/*** End of inlined file: juce_Initialisation.h ***/


#endif
#ifndef __JUCE_LOGGER_JUCEHEADER__

#endif
#ifndef __JUCE_MATHSFUNCTIONS_JUCEHEADER__

#endif
#ifndef __JUCE_MEMORY_JUCEHEADER__

#endif
#ifndef __JUCE_PERFORMANCECOUNTER_JUCEHEADER__

/*** Start of inlined file: juce_PerformanceCounter.h ***/
#ifndef __JUCE_PERFORMANCECOUNTER_JUCEHEADER__
#define __JUCE_PERFORMANCECOUNTER_JUCEHEADER__

class JUCE_API  PerformanceCounter
{
public:

	PerformanceCounter (const String& counterName,
						int runsPerPrintout = 100,
						const File& loggingFile = File::nonexistent);

	~PerformanceCounter();

	void start();

	void stop();

	void printStatistics();

	juce_UseDebuggingNewOperator

private:

	String name;
	int numRuns, runsPerPrint;
	double totalTime;
	int64 started;
	File outputFile;
};

#endif   // __JUCE_PERFORMANCECOUNTER_JUCEHEADER__
/*** End of inlined file: juce_PerformanceCounter.h ***/


#endif
#ifndef __JUCE_PLATFORMDEFS_JUCEHEADER__

#endif
#ifndef __JUCE_PLATFORMUTILITIES_JUCEHEADER__

/*** Start of inlined file: juce_PlatformUtilities.h ***/
#ifndef __JUCE_PLATFORMUTILITIES_JUCEHEADER__
#define __JUCE_PLATFORMUTILITIES_JUCEHEADER__

class JUCE_API  PlatformUtilities
{
public:

	static void beep();

	static bool launchEmailWithAttachments (const String& targetEmailAddress,
											const String& emailSubject,
											const String& bodyText,
											const StringArray& filesToAttach);

#if JUCE_MAC || JUCE_IPHONE || DOXYGEN

	static const String cfStringToJuceString (CFStringRef cfString);

	static CFStringRef juceStringToCFString (const String& s);

	static bool makeFSRefFromPath (FSRef* destFSRef, const String& path);

	static const String makePathFromFSRef (FSRef* file);

	static const String convertToPrecomposedUnicode (const String& s);

	static OSType getTypeOfFile (const String& filename);

	static bool isBundle (const String& filename);

	static void addItemToDock (const File& file);

	static int getOSXMinorVersionNumber();
#endif

#if JUCE_WINDOWS || DOXYGEN

	// Some registry helper functions:

	static const String getRegistryValue (const String& regValuePath,
										  const String& defaultValue = String::empty);

	static void setRegistryValue (const String& regValuePath,
								  const String& value);

	static bool registryValueExists (const String& regValuePath);

	static void deleteRegistryValue (const String& regValuePath);

	static void deleteRegistryKey (const String& regKeyPath);

	static void registerFileAssociation (const String& fileExtension,
										 const String& symbolicDescription,
										 const String& fullDescription,
										 const File& targetExecutable,
										 int iconResourceNumber);

	static void* JUCE_CALLTYPE getCurrentModuleInstanceHandle() throw();

	static void JUCE_CALLTYPE setCurrentModuleInstanceHandle (void* newHandle) throw();

	static const String JUCE_CALLTYPE getCurrentCommandLineParams() throw();
#endif

	static void fpuReset();

#if JUCE_LINUX || JUCE_WINDOWS

	static void* loadDynamicLibrary (const String& pathOrFilename);

	static void freeDynamicLibrary (void* libraryHandle);

	static void* getProcedureEntryPoint (void* libraryHandle,
										 const String& procedureName);
#endif

#if JUCE_LINUX || DOXYGEN

#endif

private:
	PlatformUtilities();
	PlatformUtilities (const PlatformUtilities&);
	PlatformUtilities& operator= (const PlatformUtilities&);
};

#if JUCE_MAC || JUCE_IPHONE

class ScopedAutoReleasePool
{
public:
	ScopedAutoReleasePool();
	~ScopedAutoReleasePool();

private:
	void* pool;

	ScopedAutoReleasePool (const ScopedAutoReleasePool&);
	ScopedAutoReleasePool& operator= (const ScopedAutoReleasePool&);
};

#endif

#if JUCE_LINUX

class ScopedXLock
{
public:
	ScopedXLock();

	~ScopedXLock();
};

#endif

#if JUCE_MAC

class JUCE_API  AppleRemoteDevice
{
public:

	AppleRemoteDevice();
	virtual ~AppleRemoteDevice();

	enum ButtonType
	{
		menuButton = 0,	 /**< The menu button (if it's held for a short time). */
		playButton,	 /**< The play button. */
		plusButton,	 /**< The plus or volume-up button. */
		minusButton,	/**< The minus or volume-down button. */
		rightButton,	/**< The right button (if it's held for a short time). */
		leftButton,	 /**< The left button (if it's held for a short time). */
		rightButton_Long,   /**< The right button (if it's held for a long time). */
		leftButton_Long,	/**< The menu button (if it's held for a long time). */
		menuButton_Long,	/**< The menu button (if it's held for a long time). */
		playButtonSleepMode,
		switched
	};

	virtual void buttonPressed (const ButtonType buttonId, const bool isDown) = 0;

	bool start (const bool inExclusiveMode);

	void stop();

	bool isActive() const;

	int getRemoteId() const			 { return remoteId; }

	juce_UseDebuggingNewOperator

	void handleCallbackInternal();

private:
	void* device;
	void* queue;
	int remoteId;

	bool open (const bool openInExclusiveMode);

	AppleRemoteDevice (const AppleRemoteDevice&);
	AppleRemoteDevice& operator= (const AppleRemoteDevice&);
};

#endif

#endif   // __JUCE_PLATFORMUTILITIES_JUCEHEADER__
/*** End of inlined file: juce_PlatformUtilities.h ***/


#endif
#ifndef __JUCE_RANDOM_JUCEHEADER__

/*** Start of inlined file: juce_Random.h ***/
#ifndef __JUCE_RANDOM_JUCEHEADER__
#define __JUCE_RANDOM_JUCEHEADER__

class JUCE_API  Random
{
public:

	explicit Random (int64 seedValue) throw();

	~Random() throw();

	int nextInt() throw();

	int nextInt (int maxValue) throw();

	int64 nextInt64() throw();

	float nextFloat() throw();

	double nextDouble() throw();

	bool nextBool() throw();

	const BigInteger nextLargeNumber (const BigInteger& maximumValue);

	void fillBitsRandomly (BigInteger& arrayToChange, int startBit, int numBits);

	static Random& getSystemRandom() throw();

	void setSeed (int64 newSeed) throw();

	void combineSeed (int64 seedValue) throw();

	void setSeedRandomly();

	juce_UseDebuggingNewOperator

private:
	int64 seed;
};

#endif   // __JUCE_RANDOM_JUCEHEADER__
/*** End of inlined file: juce_Random.h ***/


#endif
#ifndef __JUCE_RELATIVETIME_JUCEHEADER__

#endif
#ifndef __JUCE_SINGLETON_JUCEHEADER__

/*** Start of inlined file: juce_Singleton.h ***/
#ifndef __JUCE_SINGLETON_JUCEHEADER__
#define __JUCE_SINGLETON_JUCEHEADER__

#define juce_DeclareSingleton(classname, doNotRecreateAfterDeletion) \
\
	static classname* _singletonInstance;  \
	static JUCE_NAMESPACE::CriticalSection _singletonLock; \
\
	static classname* getInstance() \
	{ \
		if (_singletonInstance == 0) \
		{\
			const JUCE_NAMESPACE::ScopedLock sl (_singletonLock); \
\
			if (_singletonInstance == 0) \
			{ \
				static bool alreadyInside = false; \
				static bool createdOnceAlready = false; \
\
				const bool problem = alreadyInside || ((doNotRecreateAfterDeletion) && createdOnceAlready); \
				jassert (! problem); \
				if (! problem) \
				{ \
					createdOnceAlready = true; \
					alreadyInside = true; \
					classname* newObject = new classname();  /* (use a stack variable to avoid setting the newObject value before the class has finished its constructor) */ \
					alreadyInside = false; \
\
					_singletonInstance = newObject; \
				} \
			} \
		} \
\
		return _singletonInstance; \
	} \
\
	static inline classname* getInstanceWithoutCreating() throw() \
	{ \
		return _singletonInstance; \
	} \
\
	static void deleteInstance() \
	{ \
		const JUCE_NAMESPACE::ScopedLock sl (_singletonLock); \
		if (_singletonInstance != 0) \
		{ \
			classname* const old = _singletonInstance; \
			_singletonInstance = 0; \
			delete old; \
		} \
	} \
\
	void clearSingletonInstance() throw() \
	{ \
		if (_singletonInstance == this) \
			_singletonInstance = 0; \
	}

#define juce_ImplementSingleton(classname) \
\
	classname* classname::_singletonInstance = 0; \
	JUCE_NAMESPACE::CriticalSection classname::_singletonLock;

#define juce_DeclareSingleton_SingleThreaded(classname, doNotRecreateAfterDeletion) \
\
	static classname* _singletonInstance;  \
\
	static classname* getInstance() \
	{ \
		if (_singletonInstance == 0) \
		{ \
			static bool alreadyInside = false; \
			static bool createdOnceAlready = false; \
\
			const bool problem = alreadyInside || ((doNotRecreateAfterDeletion) && createdOnceAlready); \
			jassert (! problem); \
			if (! problem) \
			{ \
				createdOnceAlready = true; \
				alreadyInside = true; \
				classname* newObject = new classname();  /* (use a stack variable to avoid setting the newObject value before the class has finished its constructor) */ \
				alreadyInside = false; \
\
				_singletonInstance = newObject; \
			} \
		} \
\
		return _singletonInstance; \
	} \
\
	static inline classname* getInstanceWithoutCreating() throw() \
	{ \
		return _singletonInstance; \
	} \
\
	static void deleteInstance() \
	{ \
		if (_singletonInstance != 0) \
		{ \
			classname* const old = _singletonInstance; \
			_singletonInstance = 0; \
			delete old; \
		} \
	} \
\
	void clearSingletonInstance() throw() \
	{ \
		if (_singletonInstance == this) \
			_singletonInstance = 0; \
	}

#define juce_DeclareSingleton_SingleThreaded_Minimal(classname) \
\
	static classname* _singletonInstance;  \
\
	static classname* getInstance() \
	{ \
		if (_singletonInstance == 0) \
			_singletonInstance = new classname(); \
\
		return _singletonInstance; \
	} \
\
	static inline classname* getInstanceWithoutCreating() throw() \
	{ \
		return _singletonInstance; \
	} \
\
	static void deleteInstance() \
	{ \
		if (_singletonInstance != 0) \
		{ \
			classname* const old = _singletonInstance; \
			_singletonInstance = 0; \
			delete old; \
		} \
	} \
\
	void clearSingletonInstance() throw() \
	{ \
		if (_singletonInstance == this) \
			_singletonInstance = 0; \
	}

#define juce_ImplementSingleton_SingleThreaded(classname) \
\
	classname* classname::_singletonInstance = 0;

#endif   // __JUCE_SINGLETON_JUCEHEADER__
/*** End of inlined file: juce_Singleton.h ***/


#endif
#ifndef __JUCE_STANDARDHEADER_JUCEHEADER__

#endif
#ifndef __JUCE_SYSTEMSTATS_JUCEHEADER__

/*** Start of inlined file: juce_SystemStats.h ***/
#ifndef __JUCE_SYSTEMSTATS_JUCEHEADER__
#define __JUCE_SYSTEMSTATS_JUCEHEADER__

class JUCE_API  SystemStats
{
public:

	static const String getJUCEVersion() throw();

	enum OperatingSystemType
	{
		UnknownOS   = 0,

		MacOSX	  = 0x1000,
		Linux	   = 0x2000,

		Win95	   = 0x4001,
		Win98	   = 0x4002,
		WinNT351	= 0x4103,
		WinNT40	 = 0x4104,
		Win2000	 = 0x4105,
		WinXP	   = 0x4106,
		WinVista	= 0x4107,
		Windows7	= 0x4108,

		Windows	 = 0x4000,   /**< To test whether any version of Windows is running,
									 you can use the expression ((getOperatingSystemType() & Windows) != 0). */
		WindowsNT   = 0x0100,   /**< To test whether the platform is Windows NT or later (i.e. not Win95 or 98),
									 you can use the expression ((getOperatingSystemType() & WindowsNT) != 0). */
	};

	static OperatingSystemType getOperatingSystemType() throw();

	static const String getOperatingSystemName() throw();

	static bool isOperatingSystem64Bit() throw();

	static const String getLogonName();

	static const String getFullUserName();

	// CPU and memory information..

	static int getCpuSpeedInMegaherz() throw();

	static const String getCpuVendor() throw();

	static bool hasMMX() throw();

	static bool hasSSE() throw();

	static bool hasSSE2() throw();

	static bool has3DNow() throw();

	static int getNumCpus() throw();

	static int64 getClockCycleCounter() throw();

	static int getMemorySizeInMegabytes() throw();

	static int getPageSize() throw();

	static int getMACAddresses (int64* addresses, int maxNum,
#if JUCE_MAC
								bool littleEndian = true);
#else
								bool littleEndian = false);
#endif

	static const StringArray getMACAddressStrings();

	// not-for-public-use platform-specific method gets called at startup to initialise things.
	static void initialiseStats() throw();

private:
	SystemStats();
	SystemStats (const SystemStats&);
	SystemStats& operator= (const SystemStats&);
};

#endif   // __JUCE_SYSTEMSTATS_JUCEHEADER__
/*** End of inlined file: juce_SystemStats.h ***/


#endif
#ifndef __JUCE_TARGETPLATFORM_JUCEHEADER__

#endif
#ifndef __JUCE_TIME_JUCEHEADER__

#endif
#ifndef __JUCE_UUID_JUCEHEADER__

/*** Start of inlined file: juce_Uuid.h ***/
#ifndef __JUCE_UUID_JUCEHEADER__
#define __JUCE_UUID_JUCEHEADER__

class JUCE_API  Uuid
{
public:

	Uuid();

	~Uuid() throw();

	Uuid (const Uuid& other);

	Uuid& operator= (const Uuid& other);

	bool isNull() const throw();

	bool operator== (const Uuid& other) const;

	bool operator!= (const Uuid& other) const;

	const String toString() const;

	Uuid (const String& uuidString);

	Uuid& operator= (const String& uuidString);

	const uint8* getRawData() const throw()		 { return value.asBytes; }

	Uuid (const uint8* const rawData);

	Uuid& operator= (const uint8* const rawData);

	juce_UseDebuggingNewOperator

private:
	union
	{
		uint8 asBytes [16];
		int asInt[4];
		int64 asInt64[2];

	} value;
};

#endif   // __JUCE_UUID_JUCEHEADER__
/*** End of inlined file: juce_Uuid.h ***/


#endif
#ifndef __JUCE_BLOWFISH_JUCEHEADER__

/*** Start of inlined file: juce_BlowFish.h ***/
#ifndef __JUCE_BLOWFISH_JUCEHEADER__
#define __JUCE_BLOWFISH_JUCEHEADER__

class JUCE_API  BlowFish
{
public:

	BlowFish (const void* keyData, int keyBytes);

	BlowFish (const BlowFish& other);

	BlowFish& operator= (const BlowFish& other);

	~BlowFish();

	void encrypt (uint32& data1, uint32& data2) const throw();

	void decrypt (uint32& data1, uint32& data2) const throw();

	juce_UseDebuggingNewOperator

private:
	uint32 p[18];
	HeapBlock <uint32> s[4];

	uint32 F (uint32 x) const throw();
};

#endif   // __JUCE_BLOWFISH_JUCEHEADER__
/*** End of inlined file: juce_BlowFish.h ***/


#endif
#ifndef __JUCE_MD5_JUCEHEADER__

/*** Start of inlined file: juce_MD5.h ***/
#ifndef __JUCE_MD5_JUCEHEADER__
#define __JUCE_MD5_JUCEHEADER__

class JUCE_API  MD5
{
public:

	MD5();

	MD5 (const MD5& other);

	MD5& operator= (const MD5& other);

	explicit MD5 (const MemoryBlock& data);

	MD5 (const void* data, const size_t numBytes);

	explicit MD5 (const String& text);

	MD5 (InputStream& input, int64 numBytesToRead = -1);

	explicit MD5 (const File& file);

	~MD5();

	const MemoryBlock getRawChecksumData() const;

	const String toHexString() const;

	bool operator== (const MD5& other) const;

	bool operator!= (const MD5& other) const;

	juce_UseDebuggingNewOperator

private:
	uint8 result [16];

	struct ProcessContext
	{
		uint8 buffer [64];
		uint32 state [4];
		uint32 count [2];

		ProcessContext();

		void processBlock (const void* data, size_t dataSize);
		void transform (const void* buffer);
		void finish (void* const result);
	};

	void processStream (InputStream& input, int64 numBytesToRead);
};

#endif   // __JUCE_MD5_JUCEHEADER__
/*** End of inlined file: juce_MD5.h ***/


#endif
#ifndef __JUCE_PRIMES_JUCEHEADER__

/*** Start of inlined file: juce_Primes.h ***/
#ifndef __JUCE_PRIMES_JUCEHEADER__
#define __JUCE_PRIMES_JUCEHEADER__

class JUCE_API  Primes
{
public:

	static const BigInteger createProbablePrime (int bitLength,
												 int certainty,
												 const int* randomSeeds = 0,
												 int numRandomSeeds = 0);

	static bool isProbablyPrime (const BigInteger& number, int certainty);

private:
	Primes();
	Primes (const Primes&);
	Primes& operator= (const Primes&);
};

#endif   // __JUCE_PRIMES_JUCEHEADER__
/*** End of inlined file: juce_Primes.h ***/


#endif
#ifndef __JUCE_RSAKEY_JUCEHEADER__

/*** Start of inlined file: juce_RSAKey.h ***/
#ifndef __JUCE_RSAKEY_JUCEHEADER__
#define __JUCE_RSAKEY_JUCEHEADER__

class JUCE_API  RSAKey
{
public:

	RSAKey();

	explicit RSAKey (const String& stringRepresentation);

	~RSAKey();

	const String toString() const;

	bool applyToValue (BigInteger& value) const;

	static void createKeyPair (RSAKey& publicKey,
							   RSAKey& privateKey,
							   int numBits,
							   const int* randomSeeds = 0,
							   int numRandomSeeds = 0);

	juce_UseDebuggingNewOperator

protected:
	BigInteger part1, part2;
};

#endif   // __JUCE_RSAKEY_JUCEHEADER__
/*** End of inlined file: juce_RSAKey.h ***/


#endif
#ifndef __JUCE_DIRECTORYITERATOR_JUCEHEADER__

/*** Start of inlined file: juce_DirectoryIterator.h ***/
#ifndef __JUCE_DIRECTORYITERATOR_JUCEHEADER__
#define __JUCE_DIRECTORYITERATOR_JUCEHEADER__

class JUCE_API  DirectoryIterator
{
public:

	DirectoryIterator (const File& directory,
					   bool isRecursive,
					   const String& wildCard = "*",
					   int whatToLookFor = File::findFiles);

	~DirectoryIterator();

	bool next();

	const File getFile() const;

	float getEstimatedProgress() const;

	juce_UseDebuggingNewOperator

private:
	Array <File> filesFound;
	Array <File> dirsFound;
	String wildCard;
	int index;
	const int whatToLookFor;
	ScopedPointer <DirectoryIterator> subIterator;

	DirectoryIterator (const DirectoryIterator&);
	DirectoryIterator& operator= (const DirectoryIterator&);
};

#endif   // __JUCE_DIRECTORYITERATOR_JUCEHEADER__
/*** End of inlined file: juce_DirectoryIterator.h ***/


#endif
#ifndef __JUCE_FILE_JUCEHEADER__

#endif
#ifndef __JUCE_FILEINPUTSTREAM_JUCEHEADER__

/*** Start of inlined file: juce_FileInputStream.h ***/
#ifndef __JUCE_FILEINPUTSTREAM_JUCEHEADER__
#define __JUCE_FILEINPUTSTREAM_JUCEHEADER__

class JUCE_API  FileInputStream  : public InputStream
{
public:

	explicit FileInputStream (const File& fileToRead);

	~FileInputStream();

	const File& getFile() const throw()			 { return file; }

	int64 getTotalLength();
	int read (void* destBuffer, int maxBytesToRead);
	bool isExhausted();
	int64 getPosition();
	bool setPosition (int64 pos);

	juce_UseDebuggingNewOperator

private:
	File file;
	void* fileHandle;
	int64 currentPosition, totalSize;
	bool needToSeek;

	FileInputStream (const FileInputStream&);
	FileInputStream& operator= (const FileInputStream&);
};

#endif   // __JUCE_FILEINPUTSTREAM_JUCEHEADER__
/*** End of inlined file: juce_FileInputStream.h ***/


#endif
#ifndef __JUCE_FILEOUTPUTSTREAM_JUCEHEADER__

/*** Start of inlined file: juce_FileOutputStream.h ***/
#ifndef __JUCE_FILEOUTPUTSTREAM_JUCEHEADER__
#define __JUCE_FILEOUTPUTSTREAM_JUCEHEADER__

class JUCE_API  FileOutputStream  : public OutputStream
{
public:

	FileOutputStream (const File& fileToWriteTo,
					  int bufferSizeToUse = 16384);

	~FileOutputStream();

	const File& getFile() const			 { return file; }

	bool failedToOpen() const			   { return fileHandle == 0; }

	void flush();
	int64 getPosition();
	bool setPosition (int64 pos);
	bool write (const void* data, int numBytes);

	juce_UseDebuggingNewOperator

private:
	File file;
	void* fileHandle;
	int64 currentPosition;
	int bufferSize, bytesInBuffer;
	HeapBlock <char> buffer;

	FileOutputStream (const FileOutputStream&);
	FileOutputStream& operator= (const FileOutputStream&);
};

#endif   // __JUCE_FILEOUTPUTSTREAM_JUCEHEADER__
/*** End of inlined file: juce_FileOutputStream.h ***/


#endif
#ifndef __JUCE_FILESEARCHPATH_JUCEHEADER__

/*** Start of inlined file: juce_FileSearchPath.h ***/
#ifndef __JUCE_FILESEARCHPATH_JUCEHEADER__
#define __JUCE_FILESEARCHPATH_JUCEHEADER__

class JUCE_API  FileSearchPath
{
public:

	FileSearchPath();

	FileSearchPath (const String& path);

	FileSearchPath (const FileSearchPath& other);

	~FileSearchPath();

	FileSearchPath& operator= (const String& path);

	int getNumPaths() const;

	const File operator[] (int index) const;

	const String toString() const;

	void add (const File& directoryToAdd,
			  int insertIndex = -1);

	void addIfNotAlreadyThere (const File& directoryToAdd);

	void remove (int indexToRemove);

	void addPath (const FileSearchPath& other);

	void removeRedundantPaths();

	void removeNonExistentPaths();

	int findChildFiles (Array<File>& results,
						int whatToLookFor,
						bool searchRecursively,
						const String& wildCardPattern = "*") const;

	bool isFileInPath (const File& fileToCheck,
					   bool checkRecursively) const;

	juce_UseDebuggingNewOperator

private:
	StringArray directories;

	void init (const String& path);
};

#endif   // __JUCE_FILESEARCHPATH_JUCEHEADER__
/*** End of inlined file: juce_FileSearchPath.h ***/


#endif
#ifndef __JUCE_NAMEDPIPE_JUCEHEADER__

/*** Start of inlined file: juce_NamedPipe.h ***/
#ifndef __JUCE_NAMEDPIPE_JUCEHEADER__
#define __JUCE_NAMEDPIPE_JUCEHEADER__

class JUCE_API  NamedPipe
{
public:

	NamedPipe();

	~NamedPipe();

	bool openExisting (const String& pipeName);

	bool createNewPipe (const String& pipeName);

	void close();

	bool isOpen() const;

	const String getName() const;

	int read (void* destBuffer, int maxBytesToRead, int timeOutMilliseconds = 5000);

	int write (const void* sourceBuffer, int numBytesToWrite,
			   int timeOutMilliseconds = 2000);

	void cancelPendingReads();

	juce_UseDebuggingNewOperator

private:
	void* internal;
	String currentPipeName;
	CriticalSection lock;

	NamedPipe (const NamedPipe&);
	NamedPipe& operator= (const NamedPipe&);

	bool openInternal (const String& pipeName, const bool createPipe);
};

#endif   // __JUCE_NAMEDPIPE_JUCEHEADER__
/*** End of inlined file: juce_NamedPipe.h ***/


#endif
#ifndef __JUCE_TEMPORARYFILE_JUCEHEADER__

/*** Start of inlined file: juce_TemporaryFile.h ***/
#ifndef __JUCE_TEMPORARYFILE_JUCEHEADER__
#define __JUCE_TEMPORARYFILE_JUCEHEADER__

class JUCE_API  TemporaryFile
{
public:

	enum OptionFlags
	{
		useHiddenFile = 1,	  /**< Indicates that the temporary file should be hidden -
										 i.e. its name should start with a dot. */
		putNumbersInBrackets = 2	/**< Indicates that when numbers are appended to make sure
										 the file is unique, they should go in brackets rather
										 than just being appended (see File::getNonexistentSibling() )*/
	};

	TemporaryFile (const String& suffix = String::empty,
				   int optionFlags = 0);

	TemporaryFile (const File& targetFile,
				   int optionFlags = 0);

	~TemporaryFile();

	const File getFile() const		  { return temporaryFile; }

	const File getTargetFile() const		{ return targetFile; }

	bool overwriteTargetFileWithTemporary() const;

	juce_UseDebuggingNewOperator

private:

	File temporaryFile, targetFile;

	void createTempFile (const File& parentDirectory, String name, const String& suffix, int optionFlags);

	TemporaryFile (const TemporaryFile&);
	TemporaryFile& operator= (const TemporaryFile&);
};

#endif   // __JUCE_TEMPORARYFILE_JUCEHEADER__
/*** End of inlined file: juce_TemporaryFile.h ***/


#endif
#ifndef __JUCE_ZIPFILE_JUCEHEADER__

/*** Start of inlined file: juce_ZipFile.h ***/
#ifndef __JUCE_ZIPFILE_JUCEHEADER__
#define __JUCE_ZIPFILE_JUCEHEADER__


/*** Start of inlined file: juce_InputSource.h ***/
#ifndef __JUCE_INPUTSOURCE_JUCEHEADER__
#define __JUCE_INPUTSOURCE_JUCEHEADER__

class JUCE_API  InputSource
{
public:

	InputSource() throw()	   {}

	virtual ~InputSource()	  {}

	virtual InputStream* createInputStream() = 0;

	virtual InputStream* createInputStreamFor (const String& relatedItemPath) = 0;

	virtual int64 hashCode() const = 0;

	juce_UseDebuggingNewOperator
};

#endif   // __JUCE_INPUTSOURCE_JUCEHEADER__
/*** End of inlined file: juce_InputSource.h ***/

class JUCE_API  ZipFile
{
public:

	ZipFile (InputStream* inputStream,
			 bool deleteStreamWhenDestroyed) throw();

	ZipFile (const File& file);

	ZipFile (InputSource* inputSource);

	~ZipFile() throw();

	struct ZipEntry
	{
		String filename;

		unsigned int uncompressedSize;

		Time fileTime;
	};

	int getNumEntries() const throw();

	const ZipEntry* getEntry (int index) const throw();

	int getIndexOfFileName (const String& fileName) const throw();

	const ZipEntry* getEntry (const String& fileName) const throw();

	void sortEntriesByFilename();

	InputStream* createStreamForEntry (int index);

	void uncompressTo (const File& targetDirectory,
					   bool shouldOverwriteFiles = true);

	juce_UseDebuggingNewOperator

private:
	class ZipInputStream;
	class ZipFilenameComparator;
	class ZipEntryInfo;
	friend class ZipInputStream;
	friend class ZipFilenameComparator;
	friend class ZipEntryInfo;

	OwnedArray <ZipEntryInfo> entries;
	CriticalSection lock;
	InputStream* inputStream;
	ScopedPointer <InputStream> streamToDelete;
	ScopedPointer <InputSource> inputSource;

#ifdef JUCE_DEBUG
	int numOpenStreams;
#endif

	void init();
	int findEndOfZipEntryTable (InputStream* in, int& numEntries);
	static int compareElements (const ZipEntryInfo* first, const ZipEntryInfo* second);

	ZipFile (const ZipFile&);
	ZipFile& operator= (const ZipFile&);
};

#endif   // __JUCE_ZIPFILE_JUCEHEADER__
/*** End of inlined file: juce_ZipFile.h ***/


#endif
#ifndef __JUCE_SOCKET_JUCEHEADER__

/*** Start of inlined file: juce_Socket.h ***/
#ifndef __JUCE_SOCKET_JUCEHEADER__
#define __JUCE_SOCKET_JUCEHEADER__

class JUCE_API  StreamingSocket
{
public:

	StreamingSocket();

	~StreamingSocket();

	bool bindToPort (int localPortNumber);

	bool connect (const String& remoteHostname,
				  int remotePortNumber,
				  int timeOutMillisecs = 3000);

	bool isConnected() const throw()				{ return connected; }

	void close();

	const String& getHostName() const throw()		   { return hostName; }

	int getPort() const throw()				 { return portNumber; }

	bool isLocal() const throw();

	int waitUntilReady (bool readyForReading,
						int timeoutMsecs) const;

	int read (void* destBuffer, int maxBytesToRead,
			  bool blockUntilSpecifiedAmountHasArrived);

	int write (const void* sourceBuffer, int numBytesToWrite);

	bool createListener (int portNumber, const String& localHostName = String::empty);

	StreamingSocket* waitForNextConnection() const;

	juce_UseDebuggingNewOperator

private:
	String hostName;
	int volatile portNumber, handle;
	bool connected, isListener;

	StreamingSocket (const String& hostname, int portNumber, int handle);
	StreamingSocket (const StreamingSocket&);
	StreamingSocket& operator= (const StreamingSocket&);
};

class JUCE_API  DatagramSocket
{
public:

	DatagramSocket (int localPortNumber,
					bool enableBroadcasting = false);

	~DatagramSocket();

	bool bindToPort (int localPortNumber);

	bool connect (const String& remoteHostname,
				  int remotePortNumber,
				  int timeOutMillisecs = 3000);

	bool isConnected() const throw()				{ return connected; }

	void close();

	const String& getHostName() const throw()		   { return hostName; }

	int getPort() const throw()				 { return portNumber; }

	bool isLocal() const throw();

	int waitUntilReady (bool readyForReading,
						int timeoutMsecs) const;

	int read (void* destBuffer, int maxBytesToRead,
			  bool blockUntilSpecifiedAmountHasArrived);

	int write (const void* sourceBuffer, int numBytesToWrite);

	DatagramSocket* waitForNextConnection() const;

	juce_UseDebuggingNewOperator

private:
	String hostName;
	int volatile portNumber, handle;
	bool connected, allowBroadcast;
	void* serverAddress;

	DatagramSocket (const String& hostname, int portNumber, int handle, int localPortNumber);
	DatagramSocket (const DatagramSocket&);
	DatagramSocket& operator= (const DatagramSocket&);
};

#endif   // __JUCE_SOCKET_JUCEHEADER__
/*** End of inlined file: juce_Socket.h ***/


#endif
#ifndef __JUCE_URL_JUCEHEADER__

/*** Start of inlined file: juce_URL.h ***/
#ifndef __JUCE_URL_JUCEHEADER__
#define __JUCE_URL_JUCEHEADER__

class JUCE_API  URL
{
public:

	URL();

	URL (const String& url);

	URL (const URL& other);

	~URL();

	URL& operator= (const URL& other);

	const String toString (bool includeGetParameters) const;

	bool isWellFormed() const;

	const String getDomain() const;

	const String getSubPath() const;

	const String getScheme() const;

	const URL withNewSubPath (const String& newPath) const;

	const URL withParameter (const String& parameterName,
							 const String& parameterValue) const;

	const URL withFileToUpload (const String& parameterName,
								const File& fileToUpload,
								const String& mimeType) const;

	const StringPairArray& getParameters() const;

	const StringPairArray& getFilesToUpload() const;

	const StringPairArray& getMimeTypesOfUploadFiles() const;

	const URL withPOSTData (const String& postData) const;

	const String getPostData() const				{ return postData; }

	bool launchInDefaultBrowser() const;

	static bool isProbablyAWebsiteURL (const String& possibleURL);

	static bool isProbablyAnEmailAddress (const String& possibleEmailAddress);

	typedef bool (OpenStreamProgressCallback) (void* context, int bytesSent, int totalBytes);

	InputStream* createInputStream (bool usePostCommand,
									OpenStreamProgressCallback* progressCallback = 0,
									void* progressCallbackContext = 0,
									const String& extraHeaders = String::empty,
									int connectionTimeOutMs = 0) const;

	bool readEntireBinaryStream (MemoryBlock& destData,
								 bool usePostCommand = false) const;

	const String readEntireTextStream (bool usePostCommand = false) const;

	XmlElement* readEntireXmlStream (bool usePostCommand = false) const;

	static const String addEscapeChars (const String& stringToAddEscapeCharsTo,
										bool isParameter);

	static const String removeEscapeChars (const String& stringToRemoveEscapeCharsFrom);

	juce_UseDebuggingNewOperator

private:
	String url, postData;
	StringPairArray parameters, filesToUpload, mimeTypes;
};

#endif   // __JUCE_URL_JUCEHEADER__
/*** End of inlined file: juce_URL.h ***/


#endif
#ifndef __JUCE_BUFFEREDINPUTSTREAM_JUCEHEADER__

/*** Start of inlined file: juce_BufferedInputStream.h ***/
#ifndef __JUCE_BUFFEREDINPUTSTREAM_JUCEHEADER__
#define __JUCE_BUFFEREDINPUTSTREAM_JUCEHEADER__

class JUCE_API  BufferedInputStream  : public InputStream
{
public:

	BufferedInputStream (InputStream* sourceStream,
						 int bufferSize,
						 bool deleteSourceWhenDestroyed);

	~BufferedInputStream();

	int64 getTotalLength();
	int64 getPosition();
	bool setPosition (int64 newPosition);
	int read (void* destBuffer, int maxBytesToRead);
	const String readString();
	bool isExhausted();

	juce_UseDebuggingNewOperator

private:
	InputStream* const source;
	ScopedPointer <InputStream> sourceToDelete;
	int bufferSize;
	int64 position, lastReadPos, bufferStart, bufferOverlap;
	HeapBlock <char> buffer;
	void ensureBuffered();

	BufferedInputStream (const BufferedInputStream&);
	BufferedInputStream& operator= (const BufferedInputStream&);
};

#endif   // __JUCE_BUFFEREDINPUTSTREAM_JUCEHEADER__
/*** End of inlined file: juce_BufferedInputStream.h ***/


#endif
#ifndef __JUCE_FILEINPUTSOURCE_JUCEHEADER__

/*** Start of inlined file: juce_FileInputSource.h ***/
#ifndef __JUCE_FILEINPUTSOURCE_JUCEHEADER__
#define __JUCE_FILEINPUTSOURCE_JUCEHEADER__

class JUCE_API  FileInputSource	 : public InputSource
{
public:

	FileInputSource (const File& file);
	~FileInputSource();

	InputStream* createInputStream();
	InputStream* createInputStreamFor (const String& relatedItemPath);
	int64 hashCode() const;

	juce_UseDebuggingNewOperator

private:
	const File file;

	FileInputSource (const FileInputSource&);
	FileInputSource& operator= (const FileInputSource&);
};

#endif   // __JUCE_FILEINPUTSOURCE_JUCEHEADER__
/*** End of inlined file: juce_FileInputSource.h ***/


#endif
#ifndef __JUCE_GZIPCOMPRESSOROUTPUTSTREAM_JUCEHEADER__

/*** Start of inlined file: juce_GZIPCompressorOutputStream.h ***/
#ifndef __JUCE_GZIPCOMPRESSOROUTPUTSTREAM_JUCEHEADER__
#define __JUCE_GZIPCOMPRESSOROUTPUTSTREAM_JUCEHEADER__

class GZIPCompressorHelper;

class JUCE_API  GZIPCompressorOutputStream  : public OutputStream
{
public:

	GZIPCompressorOutputStream (OutputStream* destStream,
								int compressionLevel = 0,
								bool deleteDestStreamWhenDestroyed = false,
								bool noWrap = false);

	~GZIPCompressorOutputStream();

	void flush();
	int64 getPosition();
	bool setPosition (int64 newPosition);
	bool write (const void* destBuffer, int howMany);

	juce_UseDebuggingNewOperator

private:
	OutputStream* const destStream;
	ScopedPointer <OutputStream> streamToDelete;
	HeapBlock <uint8> buffer;
	ScopedPointer <GZIPCompressorHelper> helper;
	bool doNextBlock();

	GZIPCompressorOutputStream (const GZIPCompressorOutputStream&);
	GZIPCompressorOutputStream& operator= (const GZIPCompressorOutputStream&);
};

#endif   // __JUCE_GZIPCOMPRESSOROUTPUTSTREAM_JUCEHEADER__
/*** End of inlined file: juce_GZIPCompressorOutputStream.h ***/


#endif
#ifndef __JUCE_GZIPDECOMPRESSORINPUTSTREAM_JUCEHEADER__

/*** Start of inlined file: juce_GZIPDecompressorInputStream.h ***/
#ifndef __JUCE_GZIPDECOMPRESSORINPUTSTREAM_JUCEHEADER__
#define __JUCE_GZIPDECOMPRESSORINPUTSTREAM_JUCEHEADER__

class GZIPDecompressHelper;

class JUCE_API  GZIPDecompressorInputStream  : public InputStream
{
public:

	GZIPDecompressorInputStream (InputStream* sourceStream,
								 bool deleteSourceWhenDestroyed,
								 bool noWrap = false,
								 int64 uncompressedStreamLength = -1);

	~GZIPDecompressorInputStream();

	int64 getPosition();
	bool setPosition (int64 pos);
	int64 getTotalLength();
	bool isExhausted();
	int read (void* destBuffer, int maxBytesToRead);

	juce_UseDebuggingNewOperator

private:
	InputStream* const sourceStream;
	ScopedPointer <InputStream> streamToDelete;
	const int64 uncompressedStreamLength;
	const bool noWrap;
	bool isEof;
	int activeBufferSize;
	int64 originalSourcePos, currentPos;
	HeapBlock <uint8> buffer;
	ScopedPointer <GZIPDecompressHelper> helper;

	GZIPDecompressorInputStream (const GZIPDecompressorInputStream&);
	GZIPDecompressorInputStream& operator= (const GZIPDecompressorInputStream&);
};

#endif   // __JUCE_GZIPDECOMPRESSORINPUTSTREAM_JUCEHEADER__
/*** End of inlined file: juce_GZIPDecompressorInputStream.h ***/


#endif
#ifndef __JUCE_INPUTSOURCE_JUCEHEADER__

#endif
#ifndef __JUCE_INPUTSTREAM_JUCEHEADER__

#endif
#ifndef __JUCE_MEMORYINPUTSTREAM_JUCEHEADER__

/*** Start of inlined file: juce_MemoryInputStream.h ***/
#ifndef __JUCE_MEMORYINPUTSTREAM_JUCEHEADER__
#define __JUCE_MEMORYINPUTSTREAM_JUCEHEADER__

class JUCE_API  MemoryInputStream  : public InputStream
{
public:

	MemoryInputStream (const void* sourceData,
					   size_t sourceDataSize,
					   bool keepInternalCopyOfData);

	MemoryInputStream (const MemoryBlock& data,
					   bool keepInternalCopyOfData);

	~MemoryInputStream();

	int64 getPosition();
	bool setPosition (int64 pos);
	int64 getTotalLength();
	bool isExhausted();
	int read (void* destBuffer, int maxBytesToRead);

	juce_UseDebuggingNewOperator

private:
	const char* data;
	size_t dataSize, position;
	MemoryBlock internalCopy;

	MemoryInputStream (const MemoryInputStream&);
	MemoryInputStream& operator= (const MemoryInputStream&);
};

#endif   // __JUCE_MEMORYINPUTSTREAM_JUCEHEADER__
/*** End of inlined file: juce_MemoryInputStream.h ***/


#endif
#ifndef __JUCE_MEMORYOUTPUTSTREAM_JUCEHEADER__

/*** Start of inlined file: juce_MemoryOutputStream.h ***/
#ifndef __JUCE_MEMORYOUTPUTSTREAM_JUCEHEADER__
#define __JUCE_MEMORYOUTPUTSTREAM_JUCEHEADER__

class JUCE_API  MemoryOutputStream  : public OutputStream
{
public:

	MemoryOutputStream (size_t initialSize = 256,
						size_t granularity = 256,
						MemoryBlock* memoryBlockToWriteTo = 0);

	~MemoryOutputStream();

	const char* getData() const throw();

	size_t getDataSize() const throw()		  { return size; }

	void reset() throw();

	const String toUTF8() const;

	void flush();
	bool write (const void* buffer, int howMany);
	int64 getPosition()				 { return position; }
	bool setPosition (int64 newPosition);

	juce_UseDebuggingNewOperator

private:
	MemoryBlock* data;
	ScopedPointer <MemoryBlock> dataToDelete;
	size_t position, size, blockSize;

	MemoryOutputStream (const MemoryOutputStream&);
	MemoryOutputStream& operator= (const MemoryOutputStream&);
};

#endif   // __JUCE_MEMORYOUTPUTSTREAM_JUCEHEADER__
/*** End of inlined file: juce_MemoryOutputStream.h ***/


#endif
#ifndef __JUCE_OUTPUTSTREAM_JUCEHEADER__

#endif
#ifndef __JUCE_SUBREGIONSTREAM_JUCEHEADER__

/*** Start of inlined file: juce_SubregionStream.h ***/
#ifndef __JUCE_SUBREGIONSTREAM_JUCEHEADER__
#define __JUCE_SUBREGIONSTREAM_JUCEHEADER__

class JUCE_API  SubregionStream  : public InputStream
{
public:

	SubregionStream (InputStream* sourceStream,
					 int64 startPositionInSourceStream,
					 int64 lengthOfSourceStream,
					 bool deleteSourceWhenDestroyed) throw();

	~SubregionStream() throw();

	int64 getTotalLength();
	int64 getPosition();
	bool setPosition (int64 newPosition);
	int read (void* destBuffer, int maxBytesToRead);
	bool isExhausted();

	juce_UseDebuggingNewOperator

private:
	InputStream* const source;
	ScopedPointer <InputStream> sourceToDelete;
	const int64 startPositionInSourceStream, lengthOfSourceStream;

	SubregionStream (const SubregionStream&);
	SubregionStream& operator= (const SubregionStream&);
};

#endif   // __JUCE_SUBREGIONSTREAM_JUCEHEADER__
/*** End of inlined file: juce_SubregionStream.h ***/


#endif
#ifndef __JUCE_CHARACTERFUNCTIONS_JUCEHEADER__

#endif
#ifndef __JUCE_LOCALISEDSTRINGS_JUCEHEADER__

/*** Start of inlined file: juce_LocalisedStrings.h ***/
#ifndef __JUCE_LOCALISEDSTRINGS_JUCEHEADER__
#define __JUCE_LOCALISEDSTRINGS_JUCEHEADER__

#define TRANS(stringLiteral)	 \
	LocalisedStrings::translateWithCurrentMappings (stringLiteral)

class JUCE_API  LocalisedStrings
{
public:

	LocalisedStrings (const String& fileContents);

	LocalisedStrings (const File& fileToLoad);

	~LocalisedStrings();

	static void setCurrentMappings (LocalisedStrings* newTranslations);

	static LocalisedStrings* getCurrentMappings();

	static const String translateWithCurrentMappings (const String& text);

	static const String translateWithCurrentMappings (const char* text);

	const String translate (const String& text) const;

	const String getLanguageName() const			{ return languageName; }

	const StringArray getCountryCodes() const		   { return countryCodes; }

	void setIgnoresCase (const bool shouldIgnoreCase);

	juce_UseDebuggingNewOperator

private:
	String languageName;
	StringArray countryCodes;
	StringPairArray translations;

	void loadFromText (const String& fileContents);
};

#endif   // __JUCE_LOCALISEDSTRINGS_JUCEHEADER__
/*** End of inlined file: juce_LocalisedStrings.h ***/


#endif
#ifndef __JUCE_STRING_JUCEHEADER__

#endif
#ifndef __JUCE_STRINGARRAY_JUCEHEADER__

#endif
#ifndef __JUCE_STRINGPAIRARRAY_JUCEHEADER__

#endif
#ifndef __JUCE_XMLDOCUMENT_JUCEHEADER__

/*** Start of inlined file: juce_XmlDocument.h ***/
#ifndef __JUCE_XMLDOCUMENT_JUCEHEADER__
#define __JUCE_XMLDOCUMENT_JUCEHEADER__

class JUCE_API  XmlDocument
{
public:

	XmlDocument (const String& documentText);

	XmlDocument (const File& file);

	~XmlDocument();

	XmlElement* getDocumentElement (const bool onlyReadOuterDocumentElement = false);

	const String& getLastParseError() const throw();

	void setInputSource (InputSource* const newSource) throw();

	void setEmptyTextElementsIgnored (const bool shouldBeIgnored) throw();

	juce_UseDebuggingNewOperator

private:
	String originalText;
	const juce_wchar* input;
	bool outOfData, errorOccurred;

	bool identifierLookupTable [128];
	String lastError, dtdText;
	StringArray tokenisedDTD;
	bool needToLoadDTD, ignoreEmptyTextElements;
	ScopedPointer <InputSource> inputSource;

	void setLastError (const String& desc, const bool carryOn);
	void skipHeader();
	void skipNextWhiteSpace();
	juce_wchar readNextChar() throw();
	XmlElement* readNextElement (const bool alsoParseSubElements);
	void readChildElements (XmlElement* parent);
	int findNextTokenLength() throw();
	void readQuotedString (String& result);
	void readEntity (String& result);
	static bool isXmlIdentifierCharSlow (juce_wchar c) throw();
	bool isXmlIdentifierChar (juce_wchar c) const throw();

	const String getFileContents (const String& filename) const;
	const String expandEntity (const String& entity);
	const String expandExternalEntity (const String& entity);
	const String getParameterEntity (const String& entity);

	XmlDocument (const XmlDocument&);
	XmlDocument& operator= (const XmlDocument&);
};

#endif   // __JUCE_XMLDOCUMENT_JUCEHEADER__
/*** End of inlined file: juce_XmlDocument.h ***/


#endif
#ifndef __JUCE_XMLELEMENT_JUCEHEADER__

#endif
#ifndef __JUCE_CRITICALSECTION_JUCEHEADER__

#endif
#ifndef __JUCE_INTERPROCESSLOCK_JUCEHEADER__

/*** Start of inlined file: juce_InterProcessLock.h ***/
#ifndef __JUCE_INTERPROCESSLOCK_JUCEHEADER__
#define __JUCE_INTERPROCESSLOCK_JUCEHEADER__

class JUCE_API  InterProcessLock
{
public:

	explicit InterProcessLock (const String& name);

	~InterProcessLock();

	bool enter (int timeOutMillisecs = -1);

	void exit();

	juce_UseDebuggingNewOperator

private:

  #if JUCE_WINDOWS
	void* internal;
//  #elif JUCE_64BIT
  //  long long internal;
  #else
	int internal;
  #endif

	String name;
	int reentrancyLevel;

	InterProcessLock (const InterProcessLock&);
	InterProcessLock& operator= (const InterProcessLock&);
};

#endif   // __JUCE_INTERPROCESSLOCK_JUCEHEADER__
/*** End of inlined file: juce_InterProcessLock.h ***/


#endif
#ifndef __JUCE_PROCESS_JUCEHEADER__

/*** Start of inlined file: juce_Process.h ***/
#ifndef __JUCE_PROCESS_JUCEHEADER__
#define __JUCE_PROCESS_JUCEHEADER__

class JUCE_API  Process
{
public:

	enum ProcessPriority
	{
		LowPriority	 = 0,
		NormalPriority	  = 1,
		HighPriority	= 2,
		RealtimePriority	= 3
	};

	static void setPriority (const ProcessPriority priority);

	static void terminate();

	static bool isForegroundProcess();

	static void raisePrivilege();

	static void lowerPrivilege();

	static bool JUCE_CALLTYPE isRunningUnderDebugger();

private:
	Process();
	Process (const Process&);
	Process& operator= (const Process&);
};

#endif   // __JUCE_PROCESS_JUCEHEADER__
/*** End of inlined file: juce_Process.h ***/


#endif
#ifndef __JUCE_READWRITELOCK_JUCEHEADER__

/*** Start of inlined file: juce_ReadWriteLock.h ***/
#ifndef __JUCE_READWRITELOCK_JUCEHEADER__
#define __JUCE_READWRITELOCK_JUCEHEADER__


/*** Start of inlined file: juce_WaitableEvent.h ***/
#ifndef __JUCE_WAITABLEEVENT_JUCEHEADER__
#define __JUCE_WAITABLEEVENT_JUCEHEADER__

class JUCE_API  WaitableEvent
{
public:

	WaitableEvent() throw();

	~WaitableEvent() throw();

	bool wait (int timeOutMilliseconds = -1) const throw();

	void signal() const throw();

	void reset() const throw();

	juce_UseDebuggingNewOperator

private:
	void* internal;

	WaitableEvent (const WaitableEvent&);
	WaitableEvent& operator= (const WaitableEvent&);
};

#endif   // __JUCE_WAITABLEEVENT_JUCEHEADER__
/*** End of inlined file: juce_WaitableEvent.h ***/


/*** Start of inlined file: juce_Thread.h ***/
#ifndef __JUCE_THREAD_JUCEHEADER__
#define __JUCE_THREAD_JUCEHEADER__

class JUCE_API  Thread
{
public:

	explicit Thread (const String& threadName);

	virtual ~Thread();

	virtual void run() = 0;

	// Thread control functions..

	void startThread();

	void startThread (int priority);

	void stopThread (int timeOutMilliseconds);

	bool isThreadRunning() const;

	void signalThreadShouldExit();

	inline bool threadShouldExit() const		{ return threadShouldExit_; }

	bool waitForThreadToExit (int timeOutMilliseconds) const;

	bool setPriority (int priority);

	static bool setCurrentThreadPriority (int priority);

	void setAffinityMask (uint32 affinityMask);

	static void setCurrentThreadAffinityMask (uint32 affinityMask);

	// this can be called from any thread that needs to pause..
	static void JUCE_CALLTYPE sleep (int milliseconds);

	static void JUCE_CALLTYPE yield();

	bool wait (int timeOutMilliseconds) const;

	void notify() const;

	typedef void* ThreadID;

	static ThreadID getCurrentThreadId();

	static Thread* getCurrentThread();

	ThreadID getThreadId() const throw()				{ return threadId_; }

	const String getThreadName() const				  { return threadName_; }

	static int getNumRunningThreads();

	static void stopAllThreads (int timeoutInMillisecs);

	juce_UseDebuggingNewOperator

private:
	const String threadName_;
	void* volatile threadHandle_;
	CriticalSection startStopLock;
	WaitableEvent startSuspensionEvent_, defaultEvent_;

	int threadPriority_;
	ThreadID threadId_;
	uint32 affinityMask_;
	bool volatile threadShouldExit_;

	friend void JUCE_API juce_threadEntryPoint (void*);
	static void threadEntryPoint (Thread* thread);
	static Array<Thread*> runningThreads;
	static CriticalSection runningThreadsLock;

	Thread (const Thread&);
	Thread& operator= (const Thread&);
};

#endif   // __JUCE_THREAD_JUCEHEADER__
/*** End of inlined file: juce_Thread.h ***/

class JUCE_API  ReadWriteLock
{
public:

	ReadWriteLock() throw();

	~ReadWriteLock() throw();

	void enterRead() const throw();

	void exitRead() const throw();

	void enterWrite() const throw();

	bool tryEnterWrite() const throw();

	void exitWrite() const throw();

	juce_UseDebuggingNewOperator

private:

	CriticalSection accessLock;
	WaitableEvent waitEvent;
	mutable int numWaitingWriters, numWriters;
	mutable Thread::ThreadID writerThreadId;
	mutable Array <Thread::ThreadID> readerThreads;

	ReadWriteLock (const ReadWriteLock&);
	ReadWriteLock& operator= (const ReadWriteLock&);
};

#endif   // __JUCE_READWRITELOCK_JUCEHEADER__
/*** End of inlined file: juce_ReadWriteLock.h ***/


#endif
#ifndef __JUCE_SCOPEDLOCK_JUCEHEADER__

#endif
#ifndef __JUCE_SCOPEDREADLOCK_JUCEHEADER__

/*** Start of inlined file: juce_ScopedReadLock.h ***/
#ifndef __JUCE_SCOPEDREADLOCK_JUCEHEADER__
#define __JUCE_SCOPEDREADLOCK_JUCEHEADER__

class JUCE_API  ScopedReadLock
{
public:

	inline explicit ScopedReadLock (const ReadWriteLock& lock) throw()	: lock_ (lock) { lock.enterRead(); }

	inline ~ScopedReadLock() throw()					  { lock_.exitRead(); }

private:

	const ReadWriteLock& lock_;

	ScopedReadLock (const ScopedReadLock&);
	ScopedReadLock& operator= (const ScopedReadLock&);
};

#endif   // __JUCE_SCOPEDREADLOCK_JUCEHEADER__
/*** End of inlined file: juce_ScopedReadLock.h ***/


#endif
#ifndef __JUCE_SCOPEDTRYLOCK_JUCEHEADER__

/*** Start of inlined file: juce_ScopedTryLock.h ***/
#ifndef __JUCE_SCOPEDTRYLOCK_JUCEHEADER__
#define __JUCE_SCOPEDTRYLOCK_JUCEHEADER__

class JUCE_API  ScopedTryLock
{
public:

	inline explicit ScopedTryLock (const CriticalSection& lock) throw()   : lock_ (lock), lockWasSuccessful (lock.tryEnter()) {}

	inline ~ScopedTryLock() throw()					   { if (lockWasSuccessful) lock_.exit(); }

	bool isLocked() const throw()					 { return lockWasSuccessful; }

private:

	const CriticalSection& lock_;
	const bool lockWasSuccessful;

	ScopedTryLock (const ScopedTryLock&);
	ScopedTryLock& operator= (const ScopedTryLock&);
};

#endif   // __JUCE_SCOPEDTRYLOCK_JUCEHEADER__
/*** End of inlined file: juce_ScopedTryLock.h ***/


#endif
#ifndef __JUCE_SCOPEDWRITELOCK_JUCEHEADER__

/*** Start of inlined file: juce_ScopedWriteLock.h ***/
#ifndef __JUCE_SCOPEDWRITELOCK_JUCEHEADER__
#define __JUCE_SCOPEDWRITELOCK_JUCEHEADER__

class JUCE_API  ScopedWriteLock
{
public:

	inline explicit ScopedWriteLock (const ReadWriteLock& lock) throw() : lock_ (lock) { lock.enterWrite(); }

	inline ~ScopedWriteLock() throw()				   { lock_.exitWrite(); }

private:

	const ReadWriteLock& lock_;

	ScopedWriteLock (const ScopedWriteLock&);
	ScopedWriteLock& operator= (const ScopedWriteLock&);
};

#endif   // __JUCE_SCOPEDWRITELOCK_JUCEHEADER__
/*** End of inlined file: juce_ScopedWriteLock.h ***/


#endif
#ifndef __JUCE_THREAD_JUCEHEADER__

#endif
#ifndef __JUCE_THREADPOOL_JUCEHEADER__

/*** Start of inlined file: juce_ThreadPool.h ***/
#ifndef __JUCE_THREADPOOL_JUCEHEADER__
#define __JUCE_THREADPOOL_JUCEHEADER__

class ThreadPool;
class ThreadPoolThread;

class JUCE_API  ThreadPoolJob
{
public:

	explicit ThreadPoolJob (const String& name);

	virtual ~ThreadPoolJob();

	const String getJobName() const;

	void setJobName (const String& newName);

	enum JobStatus
	{
		jobHasFinished = 0,	 /**< indicates that the job has finished and can be
									 removed from the pool. */

		jobHasFinishedAndShouldBeDeleted,  /**< indicates that the job has finished and that it
												should be automatically deleted by the pool. */

		jobNeedsRunningAgain	/**< indicates that the job would like to be called
									 again when a thread is free. */
	};

	virtual JobStatus runJob() = 0;

	bool isRunning() const		  { return isActive; }

	bool shouldExit() const		 { return shouldStop; }

	void signalJobShouldExit();

	juce_UseDebuggingNewOperator

private:
	friend class ThreadPool;
	friend class ThreadPoolThread;
	String jobName;
	ThreadPool* pool;
	bool shouldStop, isActive, shouldBeDeleted;

	ThreadPoolJob (const ThreadPoolJob&);
	ThreadPoolJob& operator= (const ThreadPoolJob&);
};

class JUCE_API  ThreadPool
{
public:

	ThreadPool (int numberOfThreads,
				bool startThreadsOnlyWhenNeeded = true,
				int stopThreadsWhenNotUsedTimeoutMs = 5000);

	~ThreadPool();

	class JUCE_API  JobSelector
	{
	public:
		virtual ~JobSelector() {}

		virtual bool isJobSuitable (ThreadPoolJob* job) = 0;
	};

	void addJob (ThreadPoolJob* job);

	bool removeJob (ThreadPoolJob* job,
					bool interruptIfRunning,
					int timeOutMilliseconds);

	bool removeAllJobs (bool interruptRunningJobs,
						int timeOutMilliseconds,
						bool deleteInactiveJobs = false,
						JobSelector* selectedJobsToRemove = 0);

	int getNumJobs() const;

	ThreadPoolJob* getJob (int index) const;

	bool contains (const ThreadPoolJob* job) const;

	bool isJobRunning (const ThreadPoolJob* job) const;

	bool waitForJobToFinish (const ThreadPoolJob* job,
							 int timeOutMilliseconds) const;

	const StringArray getNamesOfAllJobs (bool onlyReturnActiveJobs) const;

	bool setThreadPriorities (int newPriority);

	juce_UseDebuggingNewOperator

private:
	const int threadStopTimeout;
	int priority;
	class ThreadPoolThread;
	OwnedArray <ThreadPoolThread> threads;
	Array <ThreadPoolJob*> jobs;

	CriticalSection lock;
	uint32 lastJobEndTime;
	WaitableEvent jobFinishedSignal;

	friend class ThreadPoolThread;
	bool runNextJob();

	ThreadPool (const ThreadPool&);
	ThreadPool& operator= (const ThreadPool&);
};

#endif   // __JUCE_THREADPOOL_JUCEHEADER__
/*** End of inlined file: juce_ThreadPool.h ***/


#endif
#ifndef __JUCE_TIMESLICETHREAD_JUCEHEADER__

/*** Start of inlined file: juce_TimeSliceThread.h ***/
#ifndef __JUCE_TIMESLICETHREAD_JUCEHEADER__
#define __JUCE_TIMESLICETHREAD_JUCEHEADER__

class JUCE_API  TimeSliceClient
{
public:
	virtual ~TimeSliceClient()   {}

	virtual bool useTimeSlice() = 0;
};

class JUCE_API  TimeSliceThread   : public Thread
{
public:

	explicit TimeSliceThread (const String& threadName);

	~TimeSliceThread();

	void addTimeSliceClient (TimeSliceClient* client);

	void removeTimeSliceClient (TimeSliceClient* client);

	int getNumClients() const;

	TimeSliceClient* getClient (int index) const;

	void run();

	juce_UseDebuggingNewOperator

private:
	CriticalSection callbackLock, listLock;
	Array <TimeSliceClient*> clients;
	int index;
	TimeSliceClient* clientBeingCalled;
	bool clientsChanged;

	TimeSliceThread (const TimeSliceThread&);
	TimeSliceThread& operator= (const TimeSliceThread&);
};

#endif   // __JUCE_TIMESLICETHREAD_JUCEHEADER__
/*** End of inlined file: juce_TimeSliceThread.h ***/


#endif
#ifndef __JUCE_WAITABLEEVENT_JUCEHEADER__

#endif

#endif
/*** End of inlined file: juce_core_includes.h ***/


// if you're compiling a command-line app, you might want to just include the core headers,
// so you can set this macro before including juce.h
#if ! JUCE_ONLY_BUILD_CORE_LIBRARY

/*** Start of inlined file: juce_app_includes.h ***/
#ifndef __JUCE_JUCE_APP_INCLUDES_INCLUDEFILES__
#define __JUCE_JUCE_APP_INCLUDES_INCLUDEFILES__

#ifndef __JUCE_APPLICATION_JUCEHEADER__

/*** Start of inlined file: juce_Application.h ***/
#ifndef __JUCE_APPLICATION_JUCEHEADER__
#define __JUCE_APPLICATION_JUCEHEADER__


/*** Start of inlined file: juce_ApplicationCommandTarget.h ***/
#ifndef __JUCE_APPLICATIONCOMMANDTARGET_JUCEHEADER__
#define __JUCE_APPLICATIONCOMMANDTARGET_JUCEHEADER__


/*** Start of inlined file: juce_Component.h ***/
#ifndef __JUCE_COMPONENT_JUCEHEADER__
#define __JUCE_COMPONENT_JUCEHEADER__


/*** Start of inlined file: juce_MouseCursor.h ***/
#ifndef __JUCE_MOUSECURSOR_JUCEHEADER__
#define __JUCE_MOUSECURSOR_JUCEHEADER__

class Image;
class SharedMouseCursorInternal;
class ComponentPeer;
class Component;

class JUCE_API  MouseCursor
{
public:

	enum StandardCursorType
	{
		NoCursor = 0,		   /**< An invisible cursor. */
		NormalCursor,		   /**< The stardard arrow cursor. */

		WaitCursor,			 /**< The normal hourglass or spinning-beachball 'busy' cursor. */
		IBeamCursor,			/**< A vertical I-beam for positioning within text. */
		CrosshairCursor,		/**< A pair of crosshairs. */
		CopyingCursor,                  /**< The normal arrow cursor, but with a "+" on it to indicate
											 that you're dragging a copy of something. */

		PointingHandCursor,		 /**< A hand with a pointing finger, for clicking on web-links. */
		DraggingHandCursor,		 /**< An open flat hand for dragging heavy objects around. */

		LeftRightResizeCursor,	  /**< An arrow pointing left and right. */
		UpDownResizeCursor,		 /**< an arrow pointing up and down. */
		UpDownLeftRightResizeCursor,	/**< An arrow pointing up, down, left and right. */

		TopEdgeResizeCursor,		/**< A platform-specific cursor for resizing the top-edge of a window. */
		BottomEdgeResizeCursor,	 /**< A platform-specific cursor for resizing the bottom-edge of a window. */
		LeftEdgeResizeCursor,	   /**< A platform-specific cursor for resizing the left-edge of a window. */
		RightEdgeResizeCursor,	  /**< A platform-specific cursor for resizing the right-edge of a window. */
		TopLeftCornerResizeCursor,	  /**< A platform-specific cursor for resizing the top-left-corner of a window. */
		TopRightCornerResizeCursor,	 /**< A platform-specific cursor for resizing the top-right-corner of a window. */
		BottomLeftCornerResizeCursor,   /**< A platform-specific cursor for resizing the bottom-left-corner of a window. */
		BottomRightCornerResizeCursor   /**< A platform-specific cursor for resizing the bottom-right-corner of a window. */
	};

	MouseCursor() throw();

	MouseCursor (StandardCursorType type) throw();

	MouseCursor (const Image& image, int hotSpotX, int hotSpotY) throw();

	MouseCursor (const MouseCursor& other) throw();

	MouseCursor& operator= (const MouseCursor& other) throw();

	~MouseCursor() throw();

	bool operator== (const MouseCursor& other) const throw();

	bool operator!= (const MouseCursor& other) const throw();

	static void showWaitCursor() throw();

	static void hideWaitCursor() throw();

	juce_UseDebuggingNewOperator

private:
	ReferenceCountedObjectPtr <SharedMouseCursorInternal> cursorHandle;

	friend class MouseInputSourceInternal;
	void showInWindow (ComponentPeer* window) const throw();
	void showInAllWindows() const throw();
	void* getHandle() const throw();
};

#endif   // __JUCE_MOUSECURSOR_JUCEHEADER__
/*** End of inlined file: juce_MouseCursor.h ***/


/*** Start of inlined file: juce_MouseListener.h ***/
#ifndef __JUCE_MOUSELISTENER_JUCEHEADER__
#define __JUCE_MOUSELISTENER_JUCEHEADER__

class MouseEvent;

class JUCE_API  MouseListener
{
public:
	virtual ~MouseListener()  {}

	virtual void mouseMove	  (const MouseEvent& e);

	virtual void mouseEnter	 (const MouseEvent& e);

	virtual void mouseExit	  (const MouseEvent& e);

	virtual void mouseDown	  (const MouseEvent& e);

	virtual void mouseDrag	  (const MouseEvent& e);

	virtual void mouseUp		(const MouseEvent& e);

	virtual void mouseDoubleClick   (const MouseEvent& e);

	virtual void mouseWheelMove	 (const MouseEvent& e,
									 float wheelIncrementX,
									 float wheelIncrementY);
};

#endif   // __JUCE_MOUSELISTENER_JUCEHEADER__
/*** End of inlined file: juce_MouseListener.h ***/


/*** Start of inlined file: juce_MouseEvent.h ***/
#ifndef __JUCE_MOUSEEVENT_JUCEHEADER__
#define __JUCE_MOUSEEVENT_JUCEHEADER__

class Component;
class MouseInputSource;

/*** Start of inlined file: juce_ModifierKeys.h ***/
#ifndef __JUCE_MODIFIERKEYS_JUCEHEADER__
#define __JUCE_MODIFIERKEYS_JUCEHEADER__

class JUCE_API  ModifierKeys
{
public:

	ModifierKeys (int flags = 0) throw();

	ModifierKeys (const ModifierKeys& other) throw();

	ModifierKeys& operator= (const ModifierKeys& other) throw();

	inline bool isCommandDown() const throw()	   { return (flags & commandModifier) != 0; }

	inline bool isPopupMenu() const throw()		 { return (flags & popupMenuClickModifier) != 0; }

	inline bool isLeftButtonDown() const throw()	{ return (flags & leftButtonModifier) != 0; }

	inline bool isRightButtonDown() const throw()	   { return (flags & rightButtonModifier) != 0; }

	inline bool isMiddleButtonDown() const throw()	  { return (flags & middleButtonModifier) != 0; }

	inline bool isAnyMouseButtonDown() const throw()	{ return (flags & allMouseButtonModifiers) != 0; }

	inline bool isAnyModifierKeyDown() const throw()	{ return (flags & (shiftModifier | ctrlModifier | altModifier | commandModifier)) != 0; }

	inline bool isShiftDown() const throw()		 { return (flags & shiftModifier) != 0; }

	inline bool isCtrlDown() const throw()		  { return (flags & ctrlModifier) != 0; }

	inline bool isAltDown() const throw()		   { return (flags & altModifier) != 0; }

	enum Flags
	{
		shiftModifier			   = 1,

		ctrlModifier				= 2,

		altModifier				 = 4,

		leftButtonModifier			  = 16,

		rightButtonModifier			 = 32,

		middleButtonModifier			= 64,

#if JUCE_MAC
		commandModifier			 = 8,

		popupMenuClickModifier		  = rightButtonModifier | ctrlModifier,
#else
		commandModifier			 = ctrlModifier,

		popupMenuClickModifier		  = rightButtonModifier,
#endif

		allKeyboardModifiers			= shiftModifier | ctrlModifier | altModifier | commandModifier,

		allMouseButtonModifiers		 = leftButtonModifier | rightButtonModifier | middleButtonModifier,
	};

	const ModifierKeys withOnlyMouseButtons() const throw()		 { return ModifierKeys (flags & allMouseButtonModifiers); }

	const ModifierKeys withoutMouseButtons() const throw()		  { return ModifierKeys (flags & ~allMouseButtonModifiers); }

	bool operator== (const ModifierKeys& other) const throw()	   { return flags == other.flags; }
	bool operator!= (const ModifierKeys& other) const throw()	   { return flags != other.flags; }

	inline int getRawFlags() const throw()				  { return flags; }

	inline const ModifierKeys withoutFlags (int rawFlagsToClear) const throw()  { return ModifierKeys (flags & ~rawFlagsToClear); }
	inline const ModifierKeys withFlags (int rawFlagsToSet) const throw()	   { return ModifierKeys (flags | rawFlagsToSet); }

	inline bool testFlags (const int flagsToTest) const throw()	 { return (flags & flagsToTest) != 0; }

	int getNumMouseButtonsDown() const throw();

	static const ModifierKeys getCurrentModifiers() throw();

	static const ModifierKeys getCurrentModifiersRealtime() throw();

private:

	int flags;

	static ModifierKeys currentModifiers;

	friend class ComponentPeer;
	friend class MouseInputSource;
	friend class MouseInputSourceInternal;
	static void updateCurrentModifiers() throw();
};

#endif   // __JUCE_MODIFIERKEYS_JUCEHEADER__
/*** End of inlined file: juce_ModifierKeys.h ***/



/*** Start of inlined file: juce_Point.h ***/
#ifndef __JUCE_POINT_JUCEHEADER__
#define __JUCE_POINT_JUCEHEADER__


/*** Start of inlined file: juce_AffineTransform.h ***/
#ifndef __JUCE_AFFINETRANSFORM_JUCEHEADER__
#define __JUCE_AFFINETRANSFORM_JUCEHEADER__

class JUCE_API  AffineTransform
{
public:

	AffineTransform() throw();

	AffineTransform (const AffineTransform& other) throw();

	AffineTransform (const float mat00, const float mat01, const float mat02,
					 const float mat10, const float mat11, const float mat12) throw();

	AffineTransform& operator= (const AffineTransform& other) throw();

	bool operator== (const AffineTransform& other) const throw();

	bool operator!= (const AffineTransform& other) const throw();

	static const AffineTransform identity;

	void transformPoint (float& x,
						 float& y) const throw();

	void transformPoint (double& x,
						 double& y) const throw();

	const AffineTransform translated (const float deltaX,
									  const float deltaY) const throw();

	static const AffineTransform translation (const float deltaX,
											  const float deltaY) throw();

	const AffineTransform rotated (const float angleInRadians) const throw();

	const AffineTransform rotated (const float angleInRadians,
								   const float pivotX,
								   const float pivotY) const throw();

	static const AffineTransform rotation (const float angleInRadians) throw();

	static const AffineTransform rotation (const float angleInRadians,
										   const float pivotX,
										   const float pivotY) throw();

	const AffineTransform scaled (const float factorX,
								  const float factorY) const throw();

	static const AffineTransform scale (const float factorX,
										const float factorY) throw();

	const AffineTransform sheared (const float shearX,
								   const float shearY) const throw();

	const AffineTransform inverted() const throw();

	const AffineTransform followedBy (const AffineTransform& other) const throw();

	bool isIdentity() const throw();

	bool isSingularity() const throw();

	bool isOnlyTranslation() const throw();

	float getTranslationX() const throw()		   { return mat02; }

	float getTranslationY() const throw()		   { return mat12; }

	juce_UseDebuggingNewOperator

	float mat00, mat01, mat02;
	float mat10, mat11, mat12;

private:

	const AffineTransform followedBy (const float mat00, const float mat01, const float mat02,
									  const float mat10, const float mat11, const float mat12) const throw();
};

#endif   // __JUCE_AFFINETRANSFORM_JUCEHEADER__
/*** End of inlined file: juce_AffineTransform.h ***/

template <typename ValueType>
class Point
{
public:

	Point() throw()  : x (0), y (0) {}

	Point (const Point& other) throw()  : x (other.x), y (other.y)  {}

	Point (const ValueType initialX, const ValueType initialY) throw()  : x (initialX), y (initialY) {}

	~Point() throw() {}

	Point& operator= (const Point& other) throw()			   { x = other.x; y = other.y; return *this; }

	inline ValueType getX() const throw()				   { return x; }

	inline ValueType getY() const throw()				   { return y; }

	inline bool operator== (const Point& other) const throw()	   { return x == other.x && y == other.y; }
	inline bool operator!= (const Point& other) const throw()	   { return x != other.x || y != other.y; }

	bool isOrigin() const throw()					   { return x == ValueType() && y == ValueType(); }

	void setXY (const ValueType newX, const ValueType newY) throw()	 { x = newX; y = newY; }

	void addXY (const ValueType xToAdd, const ValueType yToAdd) throw() { x += xToAdd; y += yToAdd; }

	const Point operator+ (const Point& other) const throw()		{ return Point (x + other.x, y + other.y); }

	Point& operator+= (const Point& other) throw()			  { x += other.x; y += other.y; return *this; }

	const Point operator- (const Point& other) const throw()		{ return Point (x - other.x, y - other.y); }

	Point& operator-= (const Point& other) throw()			  { x -= other.x; y -= other.y; return *this; }

	const Point operator-() const throw()				   { return Point (-x, -y); }

	ValueType getDistanceFrom (const Point& other) const throw()	{ return (ValueType) juce_hypot (x - other.x, y - other.y); }

	void applyTransform (const AffineTransform& transform) throw()	  { transform.transformPoint (x, y); }

	const String toString() const                                       { return String (x) + ", " + String (y); }

	juce_UseDebuggingNewOperator

private:
	ValueType x, y;
};

#endif   // __JUCE_POINT_JUCEHEADER__
/*** End of inlined file: juce_Point.h ***/

class JUCE_API  MouseEvent
{
public:

	MouseEvent (MouseInputSource& source,
				const Point<int>& position,
				const ModifierKeys& modifiers,
				Component* originator,
				const Time& eventTime,
				const Point<int> mouseDownPos,
				const Time& mouseDownTime,
				int numberOfClicks,
				bool mouseWasDragged) throw();

	~MouseEvent() throw();

	const int x;

	const int y;

	const ModifierKeys mods;

	Component* const eventComponent;

	Component* const originalComponent;

	const Time eventTime;

	MouseInputSource& source;

	int getMouseDownX() const throw();

	int getMouseDownY() const throw();

	const Point<int> getMouseDownPosition() const throw();

	int getDistanceFromDragStart() const throw();

	int getDistanceFromDragStartX() const throw();

	int getDistanceFromDragStartY() const throw();

	const Point<int> getOffsetFromDragStart() const throw();

	bool mouseWasClicked() const throw();

	int getNumberOfClicks() const throw()				   { return numberOfClicks; }

	int getLengthOfMousePress() const throw();

	const Point<int> getPosition() const throw();

	int getScreenX() const;

	int getScreenY() const;

	const Point<int> getScreenPosition() const;

	int getMouseDownScreenX() const;

	int getMouseDownScreenY() const;

	const Point<int> getMouseDownScreenPosition() const;

	const MouseEvent getEventRelativeTo (Component* otherComponent) const throw();

	const MouseEvent withNewPosition (const Point<int>& newPosition) const throw();

	static void setDoubleClickTimeout (int timeOutMilliseconds) throw();

	static int getDoubleClickTimeout() throw();

	juce_UseDebuggingNewOperator

private:
	const Point<int> mouseDownPos;
	const Time mouseDownTime;
	const int numberOfClicks;
	const bool wasMovedSinceMouseDown;

	MouseEvent& operator= (const MouseEvent&);
};

#endif   // __JUCE_MOUSEEVENT_JUCEHEADER__
/*** End of inlined file: juce_MouseEvent.h ***/


/*** Start of inlined file: juce_ComponentListener.h ***/
#ifndef __JUCE_COMPONENTLISTENER_JUCEHEADER__
#define __JUCE_COMPONENTLISTENER_JUCEHEADER__

class Component;

class JUCE_API  ComponentListener
{
public:
	virtual ~ComponentListener()  {}

	virtual void componentMovedOrResized (Component& component,
										  bool wasMoved,
										  bool wasResized);

	virtual void componentBroughtToFront (Component& component);

	virtual void componentVisibilityChanged (Component& component);

	virtual void componentChildrenChanged (Component& component);

	virtual void componentParentHierarchyChanged (Component& component);

	virtual void componentNameChanged (Component& component);

	virtual void componentBeingDeleted (Component& component);
};

#endif   // __JUCE_COMPONENTLISTENER_JUCEHEADER__
/*** End of inlined file: juce_ComponentListener.h ***/


/*** Start of inlined file: juce_KeyListener.h ***/
#ifndef __JUCE_KEYLISTENER_JUCEHEADER__
#define __JUCE_KEYLISTENER_JUCEHEADER__


/*** Start of inlined file: juce_KeyPress.h ***/
#ifndef __JUCE_KEYPRESS_JUCEHEADER__
#define __JUCE_KEYPRESS_JUCEHEADER__

class JUCE_API  KeyPress
{
public:

	KeyPress() throw();

	KeyPress (int keyCode,
			  const ModifierKeys& modifiers,
			  juce_wchar textCharacter) throw();

	KeyPress (int keyCode) throw();

	KeyPress (const KeyPress& other) throw();

	KeyPress& operator= (const KeyPress& other) throw();

	bool operator== (const KeyPress& other) const throw();

	bool operator!= (const KeyPress& other) const throw();

	bool isValid() const throw()				{ return keyCode != 0; }

	int getKeyCode() const throw()				  { return keyCode; }

	const ModifierKeys getModifiers() const throw()		 { return mods; }

	juce_wchar getTextCharacter() const throw()		 { return textCharacter; }

	bool isKeyCode (int keyCodeToCompare) const throw()	 { return keyCode == keyCodeToCompare; }

	static const KeyPress createFromDescription (const String& textVersion);

	const String getTextDescription() const;

	bool isCurrentlyDown() const;

	static bool isKeyCurrentlyDown (int keyCode);

	// Key codes
	//
	// Note that the actual values of these are platform-specific and may change
	// without warning, so don't store them anywhere as constants. For persisting/retrieving
	// KeyPress objects, use getTextDescription() and createFromDescription() instead.
	//

	static const int spaceKey;	  /**< key-code for the space bar */
	static const int escapeKey;	 /**< key-code for the escape key */
	static const int returnKey;	 /**< key-code for the return key*/
	static const int tabKey;	/**< key-code for the tab key*/

	static const int deleteKey;	 /**< key-code for the delete key (not backspace) */
	static const int backspaceKey;  /**< key-code for the backspace key */
	static const int insertKey;	 /**< key-code for the insert key */

	static const int upKey;	 /**< key-code for the cursor-up key */
	static const int downKey;	   /**< key-code for the cursor-down key */
	static const int leftKey;	   /**< key-code for the cursor-left key */
	static const int rightKey;	  /**< key-code for the cursor-right key */
	static const int pageUpKey;	 /**< key-code for the page-up key */
	static const int pageDownKey;   /**< key-code for the page-down key */
	static const int homeKey;	   /**< key-code for the home key */
	static const int endKey;	/**< key-code for the end key */

	static const int F1Key;	 /**< key-code for the F1 key */
	static const int F2Key;	 /**< key-code for the F2 key */
	static const int F3Key;	 /**< key-code for the F3 key */
	static const int F4Key;	 /**< key-code for the F4 key */
	static const int F5Key;	 /**< key-code for the F5 key */
	static const int F6Key;	 /**< key-code for the F6 key */
	static const int F7Key;	 /**< key-code for the F7 key */
	static const int F8Key;	 /**< key-code for the F8 key */
	static const int F9Key;	 /**< key-code for the F9 key */
	static const int F10Key;	/**< key-code for the F10 key */
	static const int F11Key;	/**< key-code for the F11 key */
	static const int F12Key;	/**< key-code for the F12 key */
	static const int F13Key;	/**< key-code for the F13 key */
	static const int F14Key;	/**< key-code for the F14 key */
	static const int F15Key;	/**< key-code for the F15 key */
	static const int F16Key;	/**< key-code for the F16 key */

	static const int numberPad0;	 /**< key-code for the 0 on the numeric keypad. */
	static const int numberPad1;	 /**< key-code for the 1 on the numeric keypad. */
	static const int numberPad2;	 /**< key-code for the 2 on the numeric keypad. */
	static const int numberPad3;	 /**< key-code for the 3 on the numeric keypad. */
	static const int numberPad4;	 /**< key-code for the 4 on the numeric keypad. */
	static const int numberPad5;	 /**< key-code for the 5 on the numeric keypad. */
	static const int numberPad6;	 /**< key-code for the 6 on the numeric keypad. */
	static const int numberPad7;	 /**< key-code for the 7 on the numeric keypad. */
	static const int numberPad8;	 /**< key-code for the 8 on the numeric keypad. */
	static const int numberPad9;	 /**< key-code for the 9 on the numeric keypad. */

	static const int numberPadAdd;		/**< key-code for the add sign on the numeric keypad. */
	static const int numberPadSubtract;	   /**< key-code for the subtract sign on the numeric keypad. */
	static const int numberPadMultiply;	   /**< key-code for the multiply sign on the numeric keypad. */
	static const int numberPadDivide;	 /**< key-code for the divide sign on the numeric keypad. */
	static const int numberPadSeparator;	  /**< key-code for the comma on the numeric keypad. */
	static const int numberPadDecimalPoint;   /**< key-code for the decimal point sign on the numeric keypad. */
	static const int numberPadEquals;	 /**< key-code for the equals key on the numeric keypad. */
	static const int numberPadDelete;	 /**< key-code for the delete key on the numeric keypad. */

	static const int playKey;	/**< key-code for a multimedia 'play' key, (not all keyboards will have one) */
	static const int stopKey;	/**< key-code for a multimedia 'stop' key, (not all keyboards will have one) */
	static const int fastForwardKey; /**< key-code for a multimedia 'fast-forward' key, (not all keyboards will have one) */
	static const int rewindKey;	  /**< key-code for a multimedia 'rewind' key, (not all keyboards will have one) */

	juce_UseDebuggingNewOperator

private:

	int keyCode;
	ModifierKeys mods;
	juce_wchar textCharacter;
};

#endif   // __JUCE_KEYPRESS_JUCEHEADER__
/*** End of inlined file: juce_KeyPress.h ***/

class Component;

class JUCE_API  KeyListener
{
public:
	virtual ~KeyListener()  {}

	virtual bool keyPressed (const KeyPress& key,
							 Component* originatingComponent) = 0;

	virtual bool keyStateChanged (bool isKeyDown, Component* originatingComponent);
};

#endif   // __JUCE_KEYLISTENER_JUCEHEADER__
/*** End of inlined file: juce_KeyListener.h ***/


/*** Start of inlined file: juce_KeyboardFocusTraverser.h ***/
#ifndef __JUCE_KEYBOARDFOCUSTRAVERSER_JUCEHEADER__
#define __JUCE_KEYBOARDFOCUSTRAVERSER_JUCEHEADER__

class Component;

class JUCE_API  KeyboardFocusTraverser
{
public:
	KeyboardFocusTraverser();

	virtual ~KeyboardFocusTraverser();

	virtual Component* getNextComponent (Component* current);

	virtual Component* getPreviousComponent (Component* current);

	virtual Component* getDefaultComponent (Component* parentComponent);
};

#endif   // __JUCE_KEYBOARDFOCUSTRAVERSER_JUCEHEADER__
/*** End of inlined file: juce_KeyboardFocusTraverser.h ***/


/*** Start of inlined file: juce_ImageEffectFilter.h ***/
#ifndef __JUCE_IMAGEEFFECTFILTER_JUCEHEADER__
#define __JUCE_IMAGEEFFECTFILTER_JUCEHEADER__


/*** Start of inlined file: juce_Graphics.h ***/
#ifndef __JUCE_GRAPHICS_JUCEHEADER__
#define __JUCE_GRAPHICS_JUCEHEADER__


/*** Start of inlined file: juce_Font.h ***/
#ifndef __JUCE_FONT_JUCEHEADER__
#define __JUCE_FONT_JUCEHEADER__


/*** Start of inlined file: juce_Typeface.h ***/
#ifndef __JUCE_TYPEFACE_JUCEHEADER__
#define __JUCE_TYPEFACE_JUCEHEADER__


/*** Start of inlined file: juce_Path.h ***/
#ifndef __JUCE_PATH_JUCEHEADER__
#define __JUCE_PATH_JUCEHEADER__


/*** Start of inlined file: juce_Rectangle.h ***/
#ifndef __JUCE_RECTANGLE_JUCEHEADER__
#define __JUCE_RECTANGLE_JUCEHEADER__

class RectangleList;

template <typename ValueType>
class Rectangle
{
public:

	Rectangle() throw()
	  : x (0), y (0), w (0), h (0)
	{
	}

	Rectangle (const Rectangle& other) throw()
	  : x (other.x), y (other.y),
		w (other.w), h (other.h)
	{
	}

	Rectangle (const ValueType initialX, const ValueType initialY,
			   const ValueType width, const ValueType height) throw()
	  : x (initialX), y (initialY),
		w (width), h (height)
	{
	}

	Rectangle (const ValueType width, const ValueType height) throw()
	  : x (0), y (0), w (width), h (height)
	{
	}

	Rectangle (const Point<ValueType>& corner1, const Point<ValueType>& corner2) throw()
	  : x (jmin (corner1.getX(), corner2.getX())),
		y (jmin (corner1.getY(), corner2.getY())),
		w (corner1.getX() - corner2.getX()),
		h (corner1.getY() - corner2.getY())
	{
		if (w < 0) w = -w;
		if (h < 0) h = -h;
	}

	Rectangle& operator= (const Rectangle& other) throw()
	{
		x = other.x; y = other.y;
		w = other.w; h = other.h;
		return *this;
	}

	~Rectangle() throw() {}

	bool isEmpty() const throw()					{ return w <= 0 || h <= 0; }

	inline ValueType getX() const throw()			   { return x; }

	inline ValueType getY() const throw()			   { return y; }

	inline ValueType getWidth() const throw()			   { return w; }

	inline ValueType getHeight() const throw()			  { return h; }

	inline ValueType getRight() const throw()			   { return x + w; }

	inline ValueType getBottom() const throw()			  { return y + h; }

	ValueType getCentreX() const throw()				{ return x + w / (ValueType) 2; }

	ValueType getCentreY() const throw()				{ return y + h / (ValueType) 2; }

	const Point<ValueType> getCentre() const throw()		{ return Point<ValueType> (x + w / (ValueType) 2, y + h / (ValueType) 2); }

	ValueType getAspectRatio (const bool widthOverHeight = true) const throw()			  { return widthOverHeight ? w / h : h / w; }

	const Point<ValueType> getPosition() const throw()						  { return Point<ValueType> (x, y); }

	void setPosition (const Point<ValueType>& newPos) throw()					   { x = newPos.getX(); y = newPos.getY(); }

	void setPosition (const ValueType newX, const ValueType newY) throw()			   { x = newX; y = newY; }

	const Rectangle withPosition (const Point<ValueType>& newPos) const throw()			 { return Rectangle (newPos.getX(), newPos.getY(), w, h); }

	void setSize (const ValueType newWidth, const ValueType newHeight) throw()			  { w = newWidth; h = newHeight; }

	const Rectangle withSize (const ValueType newWidth, const ValueType newHeight) const throw()	{ return Rectangle (x, y, newWidth, newHeight); }

	void setBounds (const ValueType newX, const ValueType newY,
					const ValueType newWidth, const ValueType newHeight) throw()
	{
		x = newX; y = newY; w = newWidth; h = newHeight;
	}

	void setWidth (const ValueType newWidth) throw()		{ w = newWidth; }

	void setHeight (const ValueType newHeight) throw()		  { h = newHeight; }

	void setLeft (const ValueType newLeft) throw()
	{
		w = jmax (ValueType(), x + w - newLeft);
		x = newLeft;
	}

	void setTop (const ValueType newTop) throw()
	{
		h = jmax (ValueType(), y + h - newTop);
		y = newTop;
	}

	void setRight (const ValueType newRight) throw()
	{
		x = jmin (x, newRight);
		w = newRight - x;
	}

	void setBottom (const ValueType newBottom) throw()
	{
		y = jmin (y, newBottom);
		h = newBottom - y;
	}

	void translate (const ValueType deltaX,
					const ValueType deltaY) throw()
	{
		x += deltaX;
		y += deltaY;
	}

	const Rectangle translated (const ValueType deltaX,
								const ValueType deltaY) const throw()
	{
		return Rectangle (x + deltaX, y + deltaY, w, h);
	}

	const Rectangle operator+ (const Point<ValueType>& deltaPosition) const throw()
	{
		return Rectangle (x + deltaPosition.getX(), y + deltaPosition.getY(), w, h);
	}

	void expand (const ValueType deltaX,
				 const ValueType deltaY) throw()
	{
		const ValueType nw = jmax (ValueType(), w + deltaX * 2);
		const ValueType nh = jmax (ValueType(), h + deltaY * 2);
		setBounds (x - deltaX, y - deltaY, nw, nh);
	}

	const Rectangle expanded (const ValueType deltaX,
							  const ValueType deltaY) const throw()
	{
		const ValueType nw = jmax (ValueType(), w + deltaX * 2);
		const ValueType nh = jmax (ValueType(), h + deltaY * 2);
		return Rectangle (x - deltaX, y - deltaY, nw, nh);
	}

	void reduce (const ValueType deltaX,
				 const ValueType deltaY) throw()
	{
		expand (-deltaX, -deltaY);
	}

	const Rectangle reduced (const ValueType deltaX,
							 const ValueType deltaY) const throw()
	{
		return expanded (-deltaX, -deltaY);
	}

	bool operator== (const Rectangle& other) const throw()
	{
		return x == other.x && y == other.y
			&& w == other.w && h == other.h;
	}

	bool operator!= (const Rectangle& other) const throw()
	{
		return x != other.x || y != other.y
			|| w != other.w || h != other.h;
	}

	bool contains (const ValueType xCoord, const ValueType yCoord) const throw()
	{
		return xCoord >= x && yCoord >= y && xCoord < x + w && yCoord < y + h;
	}

	bool contains (const Point<ValueType>& point) const throw()
	{
		return point.getX() >= x && point.getY() >= y && point.getX() < x + w && point.getY() < y + h;
	}

	bool contains (const Rectangle& other) const throw()
	{
		return x <= other.x && y <= other.y
			&& x + w >= other.x + other.w && y + h >= other.y + other.h;
	}

	const Point<ValueType> getConstrainedPoint (const Point<ValueType>& point) const throw()
	{
		return Point<ValueType> (jlimit (x, x + w, point.getX()),
								 jlimit (y, y + h, point.getY()));
	}

	bool intersects (const Rectangle& other) const throw()
	{
		return x + w > other.x
			&& y + h > other.y
			&& x < other.x + other.w
			&& y < other.y + other.h
			&& w > ValueType() && h > ValueType();
	}

	const Rectangle getIntersection (const Rectangle& other) const throw()
	{
		const ValueType nx = jmax (x, other.x);
		const ValueType ny = jmax (y, other.y);
		const ValueType nw = jmin (x + w, other.x + other.w) - nx;
		const ValueType nh = jmin (y + h, other.y + other.h) - ny;

		if (nw >= ValueType() && nh >= ValueType())
			return Rectangle (nx, ny, nw, nh);

		return Rectangle();
	}

	bool intersectRectangle (ValueType& otherX, ValueType& otherY, ValueType& otherW, ValueType& otherH) const throw()
	{
		const int maxX = jmax (otherX, x);
		otherW = jmin (otherX + otherW, x + w) - maxX;

		if (otherW > 0)
		{
			const int maxY = jmax (otherY, y);
			otherH = jmin (otherY + otherH, y + h) - maxY;

			if (otherH > 0)
			{
				otherX = maxX; otherY = maxY;
				return true;
			}
		}

		return false;
	}

	const Rectangle getUnion (const Rectangle& other) const throw()
	{
		const ValueType newX = jmin (x, other.x);
		const ValueType newY = jmin (y, other.y);

		return Rectangle (newX, newY,
						  jmax (x + w, other.x + other.w) - newX,
						  jmax (y + h, other.y + other.h) - newY);
	}

	bool enlargeIfAdjacent (const Rectangle& other) throw()
	{
		if (x == other.x && getRight() == other.getRight()
			 && (other.getBottom() >= y && other.y <= getBottom()))
		{
			const ValueType newY = jmin (y, other.y);
			h = jmax (getBottom(), other.getBottom()) - newY;
			y = newY;
			return true;
		}
		else if (y == other.y && getBottom() == other.getBottom()
				  && (other.getRight() >= x && other.x <= getRight()))
		{
			const ValueType newX = jmin (x, other.x);
			w = jmax (getRight(), other.getRight()) - newX;
			x = newX;
			return true;
		}

		return false;
	}

	bool reduceIfPartlyContainedIn (const Rectangle& other) throw()
	{
		int inside = 0;
		const int otherR = other.getRight();
		if (x >= other.x && x < otherR) inside = 1;
		const int otherB = other.getBottom();
		if (y >= other.y && y < otherB) inside |= 2;
		const int r = x + w;
		if (r >= other.x && r < otherR) inside |= 4;
		const int b = y + h;
		if (b >= other.y && b < otherB) inside |= 8;

		switch (inside)
		{
			case 1 + 2 + 8:	 w = r - otherR; x = otherR; return true;
			case 1 + 2 + 4:	 h = b - otherB; y = otherB; return true;
			case 2 + 4 + 8:	 w = other.x - x; return true;
			case 1 + 4 + 8:	 h = other.y - y; return true;
		}

		return false;
	}

	const Rectangle<ValueType> transformed (const AffineTransform& transform) const throw()
	{
		float x1 = x,	 y1 = y;
		float x2 = x + w, y2 = y;
		float x3 = x,	 y3 = y + h;
		float x4 = x2,	y4 = y3;

		transform.transformPoint (x1, y1);
		transform.transformPoint (x2, y2);
		transform.transformPoint (x3, y3);
		transform.transformPoint (x4, y4);

		const float x = jmin (x1, x2, x3, x4);
		const float y = jmin (y1, y2, y3, y4);

		return Rectangle (x, y,
						  jmax (x1, x2, x3, x4) - x,
						  jmax (y1, y2, y3, y4) - y);
	}

	const Rectangle<int> getSmallestIntegerContainer() const throw()
	{
		const int x1 = (int) floorf ((float) x);
		const int y1 = (int) floorf ((float) y);
		const int x2 = (int) floorf ((float) (x + w + 0.9999f));
		const int y2 = (int) floorf ((float) (y + h + 0.9999f));

		return Rectangle<int> (x1, y1, x2 - x1, y2 - y1);
	}

	static bool intersectRectangles (ValueType& x1, ValueType& y1, ValueType& w1, ValueType& h1,
									 const ValueType x2, const ValueType y2, const ValueType w2, const ValueType h2) throw()
	{
		const ValueType x = jmax (x1, x2);
		w1 = jmin (x1 + w1, x2 + w2) - x;

		if (w1 > 0)
		{
			const ValueType y = jmax (y1, y2);
			h1 = jmin (y1 + h1, y2 + h2) - y;

			if (h1 > 0)
			{
				x1 = x; y1 = y;
				return true;
			}
		}

		return false;
	}

	const String toString() const
	{
		String s;
		s.preallocateStorage (16);
		s << x << ' ' << y << ' ' << w << ' ' << h;
		return s;
	}

	static const Rectangle fromString (const String& stringVersion)
	{
		StringArray toks;
		toks.addTokens (stringVersion.trim(), ",; \t\r\n", String::empty);

		return Rectangle (toks[0].trim().getIntValue(),
						  toks[1].trim().getIntValue(),
						  toks[2].trim().getIntValue(),
						  toks[3].trim().getIntValue());
	}

	juce_UseDebuggingNewOperator

private:
	friend class RectangleList;
	ValueType x, y, w, h;
};

#endif   // __JUCE_RECTANGLE_JUCEHEADER__
/*** End of inlined file: juce_Rectangle.h ***/


/*** Start of inlined file: juce_Justification.h ***/
#ifndef __JUCE_JUSTIFICATION_JUCEHEADER__
#define __JUCE_JUSTIFICATION_JUCEHEADER__

class JUCE_API  Justification
{
public:

	inline Justification (int flags_) throw()  : flags (flags_) {}

	Justification (const Justification& other) throw();

	Justification& operator= (const Justification& other) throw();

	inline int getFlags() const throw()				 { return flags; }

	inline bool testFlags (int flagsToTest) const throw()	   { return (flags & flagsToTest) != 0; }

	int getOnlyVerticalFlags() const throw();

	int getOnlyHorizontalFlags() const throw();

	void applyToRectangle (int& x, int& y, int w, int h,
						   int spaceX, int spaceY, int spaceW, int spaceH) const throw();

	enum
	{

		left				= 1,

		right			   = 2,

		horizontallyCentred		 = 4,

		top				 = 8,

		bottom			  = 16,

		verticallyCentred		   = 32,

		horizontallyJustified	   = 64,

		centred			 = 36,

		centredLeft			 = 33,

		centredRight			= 34,

		centredTop			  = 12,

		centredBottom		   = 20,

		topLeft			 = 9,

		topRight			= 10,

		bottomLeft			  = 17,

		bottomRight			 = 18
	};

private:

	int flags;
};

#endif   // __JUCE_JUSTIFICATION_JUCEHEADER__
/*** End of inlined file: juce_Justification.h ***/


/*** Start of inlined file: juce_EdgeTable.h ***/
#ifndef __JUCE_EDGETABLE_JUCEHEADER__
#define __JUCE_EDGETABLE_JUCEHEADER__

class Path;
class RectangleList;
class Image;

class JUCE_API  EdgeTable
{
public:

	EdgeTable (const Rectangle<int>& clipLimits,
			   const Path& pathToAdd,
			   const AffineTransform& transform);

	EdgeTable (const Rectangle<int>& rectangleToAdd);

	EdgeTable (const RectangleList& rectanglesToAdd);

	EdgeTable (float x, float y, float w, float h);

	EdgeTable (const EdgeTable& other);

	EdgeTable& operator= (const EdgeTable& other);

	~EdgeTable();

	void clipToRectangle (const Rectangle<int>& r) throw();
	void excludeRectangle (const Rectangle<int>& r) throw();
	void clipToEdgeTable (const EdgeTable& other);
	void clipLineToMask (int x, int y, const uint8* mask, int maskStride, int numPixels) throw();
	bool isEmpty() throw();
	const Rectangle<int>& getMaximumBounds() const throw()	   { return bounds; }
	void translate (float dx, int dy) throw();

	void optimiseTable() throw();

	template <class EdgeTableIterationCallback>
	void iterate (EdgeTableIterationCallback& iterationCallback) const throw()
	{
		const int* lineStart = table;

		for (int y = 0; y < bounds.getHeight(); ++y)
		{
			const int* line = lineStart;
			lineStart += lineStrideElements;
			int numPoints = line[0];

			if (--numPoints > 0)
			{
				int x = *++line;
				jassert ((x >> 8) >= bounds.getX() && (x >> 8) < bounds.getRight());
				int levelAccumulator = 0;

				iterationCallback.setEdgeTableYPos (bounds.getY() + y);

				while (--numPoints >= 0)
				{
					const int level = *++line;
					jassert (((unsigned int) level) < (unsigned int) 256);
					const int endX = *++line;
					jassert (endX >= x);
					const int endOfRun = (endX >> 8);

					if (endOfRun == (x >> 8))
					{
						// small segment within the same pixel, so just save it for the next
						// time round..
						levelAccumulator += (endX - x) * level;
					}
					else
					{
						// plot the fist pixel of this segment, including any accumulated
						// levels from smaller segments that haven't been drawn yet
						levelAccumulator += (0xff - (x & 0xff)) * level;
						levelAccumulator >>= 8;
						x >>= 8;

						if (levelAccumulator > 0)
						{
							if (levelAccumulator >> 8)
								levelAccumulator = 0xff;

							iterationCallback.handleEdgeTablePixel (x, levelAccumulator);
						}

						// if there's a run of similar pixels, do it all in one go..
						if (level > 0)
						{
							jassert (endOfRun <= bounds.getRight());
							const int numPix = endOfRun - ++x;

							if (numPix > 0)
								iterationCallback.handleEdgeTableLine (x, numPix, level);
						}

						// save the bit at the end to be drawn next time round the loop.
						levelAccumulator = (endX & 0xff) * level;
					}

					x = endX;
				}

				if (levelAccumulator > 0)
				{
					levelAccumulator >>= 8;
					if (levelAccumulator >> 8)
						levelAccumulator = 0xff;

					x >>= 8;
					jassert (x >= bounds.getX() && x < bounds.getRight());
					iterationCallback.handleEdgeTablePixel (x, levelAccumulator);
				}
			}
		}
	}

	juce_UseDebuggingNewOperator

private:
	// table line format: number of points; point0 x, point0 levelDelta, point1 x, point1 levelDelta, etc
	HeapBlock<int> table;
	Rectangle<int> bounds;
	int maxEdgesPerLine, lineStrideElements;
	bool needToCheckEmptinesss;

	void addEdgePoint (int x, int y, int winding) throw();
	void remapTableForNumEdges (int newNumEdgesPerLine) throw();
	void intersectWithEdgeTableLine (int y, const int* otherLine) throw();
	void clipEdgeTableLineToRange (int* line, int x1, int x2) throw();
	void sanitiseLevels (bool useNonZeroWinding) throw();
	static void copyEdgeTableData (int* dest, int destLineStride, const int* src, int srcLineStride, int numLines) throw();
};

#endif   // __JUCE_EDGETABLE_JUCEHEADER__
/*** End of inlined file: juce_EdgeTable.h ***/

class Image;

class JUCE_API  Path
{
public:

	Path();

	Path (const Path& other);

	~Path();

	Path& operator= (const Path& other);

	bool isEmpty() const throw();

	const Rectangle<float> getBounds() const throw();

	const Rectangle<float> getBoundsTransformed (const AffineTransform& transform) const throw();

	bool contains (float x, float y,
				   float tolerence = 10.0f) const;

	bool intersectsLine (float x1, float y1,
						 float x2, float y2,
						 float tolerence = 10.0f);

	void clear() throw();

	void startNewSubPath (float startX, float startY);

	void closeSubPath();

	void lineTo (float endX, float endY);

	void quadraticTo (float controlPointX,
					  float controlPointY,
					  float endPointX,
					  float endPointY);

	void cubicTo (float controlPoint1X,
				  float controlPoint1Y,
				  float controlPoint2X,
				  float controlPoint2Y,
				  float endPointX,
				  float endPointY);

	const Point<float> getCurrentPosition() const;

	void addRectangle (float x, float y, float width, float height);

	void addRectangle (const Rectangle<int>& rectangle);

	void addRoundedRectangle (float x, float y, float width, float height,
							  float cornerSize);

	void addRoundedRectangle (float x, float y, float width, float height,
							  float cornerSizeX,
							  float cornerSizeY);

	void addTriangle (float x1, float y1,
					  float x2, float y2,
					  float x3, float y3);

	void addQuadrilateral (float x1, float y1,
						   float x2, float y2,
						   float x3, float y3,
						   float x4, float y4);

	void addEllipse (float x, float y, float width, float height);

	void addArc (float x, float y, float width, float height,
				 float fromRadians,
				 float toRadians,
				 bool startAsNewSubPath = false);

	void addCentredArc (float centreX, float centreY,
						float radiusX, float radiusY,
						float rotationOfEllipse,
						float fromRadians,
						float toRadians,
						bool startAsNewSubPath = false);

	void addPieSegment (float x, float y,
						float width, float height,
						float fromRadians,
						float toRadians,
						float innerCircleProportionalSize);

	void addLineSegment (float startX, float startY,
						 float endX, float endY,
						 float lineThickness);

	void addArrow (float startX, float startY,
				   float endX, float endY,
				   float lineThickness,
				   float arrowheadWidth,
				   float arrowheadLength);

	void addStar (float centreX,
				  float centreY,
				  int numberOfPoints,
				  float innerRadius,
				  float outerRadius,
				  float startAngle = 0.0f);

	void addBubble (float bodyX, float bodyY,
					float bodyW, float bodyH,
					float cornerSize,
					float arrowTipX,
					float arrowTipY,
					int whichSide,
					float arrowPositionAlongEdgeProportional,
					float arrowWidth);

	void addPath (const Path& pathToAppend);

	void addPath (const Path& pathToAppend,
				  const AffineTransform& transformToApply);

	void swapWithPath (Path& other);

	void applyTransform (const AffineTransform& transform) throw();

	void scaleToFit (float x, float y, float width, float height,
					 bool preserveProportions) throw();

	const AffineTransform getTransformToScaleToFit (float x, float y, float width, float height,
													bool preserveProportions,
													const Justification& justificationType = Justification::centred) const;

	const Path createPathWithRoundedCorners (float cornerRadius) const;

	void setUsingNonZeroWinding (bool isNonZeroWinding) throw();

	bool isUsingNonZeroWinding() const		  { return useNonZeroWinding; }

	class JUCE_API  Iterator
	{
	public:

		Iterator (const Path& path);
		~Iterator();

		bool next();

		enum PathElementType
		{
			startNewSubPath,	/**< For this type, x1 and y1 will be set to indicate the first point in the subpath.  */
			lineTo,		 /**< For this type, x1 and y1 indicate the end point of the line.  */
			quadraticTo,	/**< For this type, x1, y1, x2, y2 indicate the control point and endpoint of a quadratic curve. */
			cubicTo,		/**< For this type, x1, y1, x2, y2, x3, y3 indicate the two control points and the endpoint of a cubic curve. */
			closePath	   /**< Indicates that the sub-path is being closed. None of the x or y values are valid in this case. */
		};

		PathElementType elementType;

		float x1, y1, x2, y2, x3, y3;

	private:
		const Path& path;
		size_t index;

		Iterator (const Iterator&);
		Iterator& operator= (const Iterator&);
	};

	void loadPathFromStream (InputStream& source);

	void loadPathFromData (const void* data, int numberOfBytes);

	void writePathToStream (OutputStream& destination) const;

	const String toString() const;

	void restoreFromString (const String& stringVersion);

	juce_UseDebuggingNewOperator

private:
	friend class PathFlatteningIterator;
	friend class Path::Iterator;
	ArrayAllocationBase <float, DummyCriticalSection> data;
	size_t numElements;
	float pathXMin, pathXMax, pathYMin, pathYMax;
	bool useNonZeroWinding;

	static const float lineMarker;
	static const float moveMarker;
	static const float quadMarker;
	static const float cubicMarker;
	static const float closeSubPathMarker;
};

#endif   // __JUCE_PATH_JUCEHEADER__
/*** End of inlined file: juce_Path.h ***/

class Font;

class JUCE_API  Typeface  : public ReferenceCountedObject
{
public:

	typedef ReferenceCountedObjectPtr <Typeface> Ptr;

	const String getName() const throw()	   { return name; }

	static const Ptr createSystemTypefaceFor (const Font& font);

	virtual ~Typeface();

	virtual float getAscent() const = 0;

	virtual float getDescent() const = 0;

	virtual float getStringWidth (const String& text) = 0;

	virtual void getGlyphPositions (const String& text, Array <int>& glyphs, Array<float>& xOffsets) = 0;

	virtual bool getOutlineForGlyph (int glyphNumber, Path& path) = 0;

	juce_UseDebuggingNewOperator

protected:
	String name;

	explicit Typeface (const String& name) throw();

private:
	Typeface (const Typeface&);
	Typeface& operator= (const Typeface&);
};

class JUCE_API  CustomTypeface  : public Typeface
{
public:

	CustomTypeface();

	explicit CustomTypeface (InputStream& serialisedTypefaceStream);

	~CustomTypeface();

	void clear();

	void setCharacteristics (const String& name, float ascent,
							 bool isBold, bool isItalic,
							 juce_wchar defaultCharacter) throw();

	void addGlyph (juce_wchar character, const Path& path, float width) throw();

	void addKerningPair (juce_wchar char1, juce_wchar char2, float extraAmount) throw();

	void addGlyphsFromOtherTypeface (Typeface& typefaceToCopy, juce_wchar characterStartIndex, int numCharacters) throw();

	bool writeToStream (OutputStream& outputStream);

	// The following methods implement the basic Typeface behaviour.
	float getAscent() const;
	float getDescent() const;
	float getStringWidth (const String& text);
	void getGlyphPositions (const String& text, Array <int>& glyphs, Array<float>& xOffsets);
	bool getOutlineForGlyph (int glyphNumber, Path& path);
	int getGlyphForCharacter (juce_wchar character);

	juce_UseDebuggingNewOperator

protected:
	juce_wchar defaultCharacter;
	float ascent;
	bool isBold, isItalic;

	virtual bool loadGlyphIfPossible (juce_wchar characterNeeded);

private:

	class GlyphInfo;
	friend class OwnedArray<GlyphInfo>;
	OwnedArray <GlyphInfo> glyphs;
	short lookupTable [128];

	CustomTypeface (const CustomTypeface&);
	CustomTypeface& operator= (const CustomTypeface&);

	GlyphInfo* findGlyph (const juce_wchar character, bool loadIfNeeded) throw();
	GlyphInfo* findGlyphSubstituting (juce_wchar character) throw();
};

#endif   // __JUCE_TYPEFACE_JUCEHEADER__
/*** End of inlined file: juce_Typeface.h ***/

class LowLevelGraphicsContext;

class JUCE_API  Font
{
public:

	enum FontStyleFlags
	{
		plain	   = 0,	/**< indicates a plain, non-bold, non-italic version of the font. @see setStyleFlags */
		bold	= 1,	/**< boldens the font. @see setStyleFlags */
		italic	  = 2,	/**< finds an italic version of the font. @see setStyleFlags */
		underlined  = 4	 /**< underlines the font. @see setStyleFlags */
	};

	Font (float fontHeight,
		  int styleFlags = plain) throw();

	Font (const String& typefaceName,
		  float fontHeight,
		  int styleFlags) throw();

	Font (const Font& other) throw();

	Font (const Typeface::Ptr& typeface) throw();

	Font() throw();

	Font& operator= (const Font& other) throw();

	bool operator== (const Font& other) const throw();
	bool operator!= (const Font& other) const throw();

	~Font() throw();

	void setTypefaceName (const String& faceName) throw();

	const String& getTypefaceName() const throw()		   { return font->typefaceName; }

	static const String getDefaultSansSerifFontName() throw();

	static const String getDefaultSerifFontName() throw();

	static const String getDefaultMonospacedFontName() throw();

	static void getPlatformDefaultFontNames (String& defaultSans, String& defaultSerif, String& defaultFixed);

	float getHeight() const throw()				 { return font->height; }

	void setHeight (float newHeight) throw();

	void setHeightWithoutChangingWidth (float newHeight) throw();

	float getAscent() const throw();

	float getDescent() const throw();

	int getStyleFlags() const throw()			   { return font->styleFlags; }

	void setStyleFlags (const int newFlags) throw();

	void setBold (bool shouldBeBold) throw();
	bool isBold() const throw();

	void setItalic (bool shouldBeItalic) throw();
	bool isItalic() const throw();

	void setUnderline (bool shouldBeUnderlined) throw();
	bool isUnderlined() const throw();

	void setHorizontalScale (float scaleFactor) throw();

	float getHorizontalScale() const throw()		{ return font->horizontalScale; }

	void setExtraKerningFactor (float extraKerning) throw();

	float getExtraKerningFactor() const throw()		 { return font->kerning; }

	void setSizeAndStyle (float newHeight,
						  int newStyleFlags,
						  float newHorizontalScale,
						  float newKerningAmount) throw();

	int getStringWidth (const String& text) const throw();

	float getStringWidthFloat (const String& text) const throw();

	void getGlyphPositions (const String& text, Array <int>& glyphs, Array <float>& xOffsets) const throw();

	Typeface* getTypeface() const throw();

	static void findFonts (Array<Font>& results) throw();

	static const StringArray findAllTypefaceNames();

	static const String getFallbackFontName() throw();

	static void setFallbackFontName (const String& name) throw();

	juce_UseDebuggingNewOperator

private:

	friend class FontGlyphAlphaMap;
	friend class TypefaceCache;

	class SharedFontInternal  : public ReferenceCountedObject
	{
	public:
		SharedFontInternal (const String& typefaceName, float height, float horizontalScale,
							float kerning, float ascent, int styleFlags,
							Typeface* typeface) throw();
		SharedFontInternal (const SharedFontInternal& other) throw();

		String typefaceName;
		float height, horizontalScale, kerning, ascent;
		int styleFlags;
		Typeface::Ptr typeface;
	};

	ReferenceCountedObjectPtr <SharedFontInternal> font;
	void dupeInternalIfShared() throw();
};

#endif   // __JUCE_FONT_JUCEHEADER__
/*** End of inlined file: juce_Font.h ***/


/*** Start of inlined file: juce_PathStrokeType.h ***/
#ifndef __JUCE_PATHSTROKETYPE_JUCEHEADER__
#define __JUCE_PATHSTROKETYPE_JUCEHEADER__

class JUCE_API  PathStrokeType
{
public:

	enum JointStyle
	{
		mitered,	/**< Indicates that corners should be drawn with sharp joints.
						 Note that for angles that curve back on themselves, drawing a
						 mitre could require extending the point too far away from the
						 path, so a mitre limit is imposed and any corners that exceed it
						 are drawn as bevelled instead. */
		curved,	 /**< Indicates that corners should be drawn as rounded-off. */
		beveled	 /**< Indicates that corners should be drawn with a line flattening their
						 outside edge. */
	};

	enum EndCapStyle
	{
		butt,	   /**< Ends of lines are flat and don't extend beyond the end point. */
		square,	 /**< Ends of lines are flat, but stick out beyond the end point for half
						 the thickness of the stroke. */
		rounded	 /**< Ends of lines are rounded-off with a circular shape. */
	};

	PathStrokeType (float strokeThickness,
					JointStyle jointStyle = mitered,
					EndCapStyle endStyle = butt) throw();

	PathStrokeType (const PathStrokeType& other) throw();

	PathStrokeType& operator= (const PathStrokeType& other) throw();

	~PathStrokeType() throw();

	void createStrokedPath (Path& destPath,
							const Path& sourcePath,
							const AffineTransform& transform = AffineTransform::identity,
							float extraAccuracy = 1.0f) const;

	void createDashedStroke (Path& destPath,
							 const Path& sourcePath,
							 const float* dashLengths,
							 int numDashLengths,
							 const AffineTransform& transform = AffineTransform::identity,
							 float extraAccuracy = 1.0f) const;

	float getStrokeThickness() const throw()			{ return thickness; }

	JointStyle getJointStyle() const throw()			{ return jointStyle; }

	EndCapStyle getEndStyle() const throw()			 { return endStyle; }

	juce_UseDebuggingNewOperator

	bool operator== (const PathStrokeType& other) const throw();

	bool operator!= (const PathStrokeType& other) const throw();

private:

	float thickness;
	JointStyle jointStyle;
	EndCapStyle endStyle;
};

#endif   // __JUCE_PATHSTROKETYPE_JUCEHEADER__
/*** End of inlined file: juce_PathStrokeType.h ***/


/*** Start of inlined file: juce_Line.h ***/
#ifndef __JUCE_LINE_JUCEHEADER__
#define __JUCE_LINE_JUCEHEADER__

class JUCE_API  Line
{
public:

	Line() throw();

	Line (const Line& other) throw();

	Line (float startX,
		  float startY,
		  float endX,
		  float endY) throw();

	Line (const Point<float>& start,
		  const Point<float>& end) throw();

	Line& operator= (const Line& other) throw();

	~Line() throw();

	inline float getStartX() const throw()				  { return startX; }

	inline float getStartY() const throw()				  { return startY; }

	inline float getEndX() const throw()				{ return endX; }

	inline float getEndY() const throw()				{ return endY; }

	const Point<float> getStart() const throw();

	const Point<float> getEnd() const throw();

	void setStart (float newStartX,
				   float newStartY) throw();

	void setEnd (float newEndX,
				 float newEndY) throw();

	void setStart (const Point<float>& newStart) throw();

	void setEnd (const Point<float>& newEnd) throw();

	void applyTransform (const AffineTransform& transform) throw();

	float getLength() const throw();

	bool isVertical() const throw();

	bool isHorizontal() const throw();

	float getAngle() const throw();

	bool operator== (const Line& other) const throw();

	bool operator!= (const Line& other) const throw();

	bool intersects (const Line& line,
					 float& intersectionX,
					 float& intersectionY) const throw();

	const Point<float> getPointAlongLine (float distanceFromStart) const throw();

	const Point<float> getPointAlongLine (float distanceFromStart,
										  float perpendicularDistance) const throw();

	const Point<float> getPointAlongLineProportionally (float proportionOfLength) const throw();

	float getDistanceFromLine (float x, float y) const throw();

	float findNearestPointTo (float x, float y) const throw();

	bool isPointAbove (float x, float y) const throw();

	const Line withShortenedStart (float distanceToShortenBy) const throw();

	const Line withShortenedEnd (float distanceToShortenBy) const throw();

	bool clipToPath (const Path& path, bool keepSectionOutsidePath) throw();

	juce_UseDebuggingNewOperator

private:
	float startX, startY, endX, endY;
};

#endif   // __JUCE_LINE_JUCEHEADER__
/*** End of inlined file: juce_Line.h ***/


/*** Start of inlined file: juce_Colours.h ***/
#ifndef __JUCE_COLOURS_JUCEHEADER__
#define __JUCE_COLOURS_JUCEHEADER__


/*** Start of inlined file: juce_Colour.h ***/
#ifndef __JUCE_COLOUR_JUCEHEADER__
#define __JUCE_COLOUR_JUCEHEADER__


/*** Start of inlined file: juce_PixelFormats.h ***/
#ifndef __JUCE_PIXELFORMATS_JUCEHEADER__
#define __JUCE_PIXELFORMATS_JUCEHEADER__

#if JUCE_MSVC
  #pragma pack (push, 1)
  #define PACKED
#elif JUCE_GCC
  #define PACKED __attribute__((packed))
#else
  #define PACKED
#endif

class PixelRGB;
class PixelAlpha;

class JUCE_API  PixelARGB
{
public:
	PixelARGB() throw() {}
	~PixelARGB() throw() {}

	PixelARGB (const uint32 argb_) throw()
		: argb (argb_)
	{
	}

	forcedinline uint32 getARGB() const throw()	 { return argb; }
	forcedinline uint32 getRB() const throw()	   { return 0x00ff00ff & argb; }
	forcedinline uint32 getAG() const throw()	   { return 0x00ff00ff & (argb >> 8); }

	forcedinline uint8 getAlpha() const throw()	 { return components.a; }
	forcedinline uint8 getRed() const throw()	   { return components.r; }
	forcedinline uint8 getGreen() const throw()	 { return components.g; }
	forcedinline uint8 getBlue() const throw()	  { return components.b; }

	forcedinline void blend (const PixelARGB& src) throw()
	{
		uint32 sargb = src.getARGB();
		const uint32 alpha = 0x100 - (sargb >> 24);

		sargb += 0x00ff00ff & ((getRB() * alpha) >> 8);
		sargb += 0xff00ff00 & (getAG() * alpha);

		argb = sargb;
	}

	forcedinline void blend (const PixelAlpha& src) throw();

	forcedinline void blend (const PixelRGB& src) throw();

	template <class Pixel>
	forcedinline void blend (const Pixel& src, uint32 extraAlpha) throw()
	{
		++extraAlpha;

		uint32 sargb = ((extraAlpha * src.getAG()) & 0xff00ff00)
						 | (((extraAlpha * src.getRB()) >> 8) & 0x00ff00ff);

		const uint32 alpha = 0x100 - (sargb >> 24);

		sargb += 0x00ff00ff & ((getRB() * alpha) >> 8);
		sargb += 0xff00ff00 & (getAG() * alpha);

		argb = sargb;
	}

	template <class Pixel>
	forcedinline void tween (const Pixel& src, const uint32 amount) throw()
	{
		uint32 drb = getRB();
		drb += (((src.getRB() - drb) * amount) >> 8);
		drb &= 0x00ff00ff;

		uint32 dag = getAG();
		dag += (((src.getAG() - dag) * amount) >> 8);
		dag &= 0x00ff00ff;
		dag <<= 8;

		dag |= drb;
		argb = dag;
	}

	template <class Pixel>
	forcedinline void set (const Pixel& src) throw()
	{
		argb = src.getARGB();
	}

	forcedinline void setAlpha (const uint8 newAlpha) throw()
	{
		components.a = newAlpha;
	}

	forcedinline void multiplyAlpha (int multiplier) throw()
	{
		++multiplier;

		argb = ((multiplier * getAG()) & 0xff00ff00)
				| (((multiplier * getRB()) >> 8) & 0x00ff00ff);
	}

	forcedinline void multiplyAlpha (const float multiplier) throw()
	{
		multiplyAlpha ((int) (multiplier * 256.0f));
	}

	void setARGB (const uint8 a, const uint8 r, const uint8 g, const uint8 b) throw()
	{
		components.b = b;
		components.g = g;
		components.r = r;
		components.a = a;
	}

	forcedinline void premultiply() throw()
	{
		const uint32 alpha = components.a;

		if (alpha < 0xff)
		{
			if (alpha == 0)
			{
				components.b = 0;
				components.g = 0;
				components.r = 0;
			}
			else
			{
				components.b = (uint8) ((components.b * alpha + 0x7f) >> 8);
				components.g = (uint8) ((components.g * alpha + 0x7f) >> 8);
				components.r = (uint8) ((components.r * alpha + 0x7f) >> 8);
			}
		}
	}

	forcedinline void unpremultiply() throw()
	{
		const uint32 alpha = components.a;

		if (alpha < 0xff)
		{
			if (alpha == 0)
			{
				components.b = 0;
				components.g = 0;
				components.r = 0;
			}
			else
			{
				components.b = (uint8) jmin ((uint32) 0xff, (components.b * 0xff) / alpha);
				components.g = (uint8) jmin ((uint32) 0xff, (components.g * 0xff) / alpha);
				components.r = (uint8) jmin ((uint32) 0xff, (components.r * 0xff) / alpha);
			}
		}
	}

	forcedinline void desaturate() throw()
	{
		if (components.a < 0xff && components.a > 0)
		{
			const int newUnpremultipliedLevel = (0xff * ((int) components.r + (int) components.g + (int) components.b) / (3 * components.a));

			components.r = components.g = components.b
				= (uint8) ((newUnpremultipliedLevel * components.a + 0x7f) >> 8);
		}
		else
		{
			components.r = components.g = components.b
				= (uint8) (((int) components.r + (int) components.g + (int) components.b) / 3);
		}
	}

	#if JUCE_BIG_ENDIAN
	enum { indexA = 0, indexR = 1, indexG = 2, indexB = 3 };
	#else
	enum { indexA = 3, indexR = 2, indexG = 1, indexB = 0 };
	#endif

private:

	union
	{
		uint32 argb;

		struct
		{
#if JUCE_BIG_ENDIAN
			uint8 a : 8, r : 8, g : 8, b : 8;
#else
			uint8 b, g, r, a;
#endif
		} PACKED components;
	};

} PACKED;

class JUCE_API  PixelRGB
{
public:
	PixelRGB() throw() {}
	~PixelRGB() throw() {}

	PixelRGB (const uint32 argb) throw()
	{
		r = (uint8) (argb >> 16);
		g = (uint8) (argb >> 8);
		b = (uint8) (argb);
	}

	forcedinline uint32 getARGB() const throw()	 { return 0xff000000 | b | (g << 8) | (r << 16); }
	forcedinline uint32 getRB() const throw()	   { return b | (uint32) (r << 16); }
	forcedinline uint32 getAG() const throw()	   { return 0xff0000 | g; }

	forcedinline uint8 getAlpha() const throw()	 { return 0xff; }
	forcedinline uint8 getRed() const throw()	   { return r; }
	forcedinline uint8 getGreen() const throw()	 { return g; }
	forcedinline uint8 getBlue() const throw()	  { return b; }

	forcedinline void blend (const PixelARGB& src) throw()
	{
		uint32 sargb = src.getARGB();
		const uint32 alpha = 0x100 - (sargb >> 24);

		sargb += 0x00ff00ff & ((getRB() * alpha) >> 8);
		sargb += 0x0000ff00 & (g * alpha);

		r = (uint8) (sargb >> 16);
		g = (uint8) (sargb >> 8);
		b = (uint8) sargb;
	}

	forcedinline void blend (const PixelRGB& src) throw()
	{
		set (src);
	}

	forcedinline void blend (const PixelAlpha& src) throw();

	template <class Pixel>
	forcedinline void blend (const Pixel& src, uint32 extraAlpha) throw()
	{
		++extraAlpha;
		const uint32 srb = (extraAlpha * src.getRB()) >> 8;
		const uint32 sag = extraAlpha * src.getAG();
		uint32 sargb = (sag & 0xff00ff00) | (srb & 0x00ff00ff);

		const uint32 alpha = 0x100 - (sargb >> 24);

		sargb += 0x00ff00ff & ((getRB() * alpha) >> 8);
		sargb += 0x0000ff00 & (g * alpha);

		b = (uint8) sargb;
		g = (uint8) (sargb >> 8);
		r = (uint8) (sargb >> 16);
	}

	template <class Pixel>
	forcedinline void tween (const Pixel& src, const uint32 amount) throw()
	{
		uint32 drb = getRB();
		drb += (((src.getRB() - drb) * amount) >> 8);

		uint32 dag = getAG();
		dag += (((src.getAG() - dag) * amount) >> 8);

		b = (uint8) drb;
		g = (uint8) dag;
		r = (uint8) (drb >> 16);
	}

	template <class Pixel>
	forcedinline void set (const Pixel& src) throw()
	{
		b = src.getBlue();
		g = src.getGreen();
		r = src.getRed();
	}

	forcedinline void setAlpha (const uint8) throw() {}

	forcedinline void multiplyAlpha (int) throw() {}

	void setARGB (const uint8, const uint8 r_, const uint8 g_, const uint8 b_) throw()
	{
		r = r_;
		g = g_;
		b = b_;
	}

	forcedinline void premultiply() throw() {}

	forcedinline void unpremultiply() throw() {}

	forcedinline void desaturate() throw()
	{
		r = g = b = (uint8) (((int) r + (int) g + (int) b) / 3);
	}

	#if JUCE_MAC
	enum { indexR = 0, indexG = 1, indexB = 2 };
	#else
	enum { indexR = 2, indexG = 1, indexB = 0 };
	#endif

private:

#if JUCE_MAC
	uint8 r, g, b;
#else
	uint8 b, g, r;
#endif

} PACKED;

forcedinline void PixelARGB::blend (const PixelRGB& src) throw()
{
	set (src);
}

class JUCE_API  PixelAlpha
{
public:
	PixelAlpha() throw() {}
	~PixelAlpha() throw() {}

	PixelAlpha (const uint32 argb) throw()
	{
		a = (uint8) (argb >> 24);
	}

	forcedinline uint32 getARGB() const throw()	 { return (((uint32) a) << 24) | (((uint32) a) << 16) | (((uint32) a) << 8) | a; }
	forcedinline uint32 getRB() const throw()	   { return (((uint32) a) << 16) | a; }
	forcedinline uint32 getAG() const throw()	   { return (((uint32) a) << 16) | a; }

	forcedinline uint8 getAlpha() const throw()	 { return a; }
	forcedinline uint8 getRed() const throw()	   { return 0; }
	forcedinline uint8 getGreen() const throw()	 { return 0; }
	forcedinline uint8 getBlue() const throw()	  { return 0; }

	template <class Pixel>
	forcedinline void blend (const Pixel& src) throw()
	{
		const int srcA = src.getAlpha();
		a = (uint8) ((a * (0x100 - srcA) >> 8) + srcA);
	}

	template <class Pixel>
	forcedinline void blend (const Pixel& src, uint32 extraAlpha) throw()
	{
		++extraAlpha;
		const int srcAlpha = (extraAlpha * src.getAlpha()) >> 8;
		a = (uint8) ((a * (0x100 - srcAlpha) >> 8) + srcAlpha);
	}

	template <class Pixel>
	forcedinline void tween (const Pixel& src, const uint32 amount) throw()
	{
		a += ((src,getAlpha() - a) * amount) >> 8;
	}

	template <class Pixel>
	forcedinline void set (const Pixel& src) throw()
	{
		a = src.getAlpha();
	}

	forcedinline void setAlpha (const uint8 newAlpha) throw()
	{
		a = newAlpha;
	}

	forcedinline void multiplyAlpha (int multiplier) throw()
	{
		++multiplier;
		a = (uint8) ((a * multiplier) >> 8);
	}

	forcedinline void multiplyAlpha (const float multiplier) throw()
	{
		a = (uint8) (a * multiplier);
	}

	forcedinline void setARGB (const uint8 a_, const uint8 /*r*/, const uint8 /*g*/, const uint8 /*b*/) throw()
	{
		a = a_;
	}

	forcedinline void premultiply() throw()
	{
	}

	forcedinline void unpremultiply() throw()
	{
	}

	forcedinline void desaturate() throw()
	{
	}

	enum { indexA = 0 };

private:

	uint8 a : 8;
} PACKED;

forcedinline void PixelRGB::blend (const PixelAlpha& src) throw()
{
	blend (PixelARGB (src.getARGB()));
}

forcedinline void PixelARGB::blend (const PixelAlpha& src) throw()
{
	uint32 sargb = src.getARGB();
	const uint32 alpha = 0x100 - (sargb >> 24);

	sargb += 0x00ff00ff & ((getRB() * alpha) >> 8);
	sargb += 0xff00ff00 & (getAG() * alpha);

	argb = sargb;
}

#if JUCE_MSVC
  #pragma pack (pop)
#endif

#undef PACKED

#endif   // __JUCE_PIXELFORMATS_JUCEHEADER__
/*** End of inlined file: juce_PixelFormats.h ***/

class JUCE_API  Colour
{
public:

	Colour() throw();

	Colour (const Colour& other) throw();

	explicit Colour (uint32 argb) throw();

	Colour (uint8 red,
			uint8 green,
			uint8 blue) throw();

	static const Colour fromRGB (uint8 red,
								 uint8 green,
								 uint8 blue) throw();

	Colour (uint8 red,
			uint8 green,
			uint8 blue,
			uint8 alpha) throw();

	static const Colour fromRGBA (uint8 red,
								  uint8 green,
								  uint8 blue,
								  uint8 alpha) throw();

	Colour (uint8 red,
			uint8 green,
			uint8 blue,
			float alpha) throw();

	static const Colour fromRGBAFloat (uint8 red,
									   uint8 green,
									   uint8 blue,
									   float alpha) throw();

	Colour (float hue,
			float saturation,
			float brightness,
			uint8 alpha) throw();

	Colour (float hue,
			float saturation,
			float brightness,
			float alpha) throw();

	static const Colour fromHSV (float hue,
								 float saturation,
								 float brightness,
								 float alpha) throw();

	~Colour() throw();

	Colour& operator= (const Colour& other) throw();

	bool operator== (const Colour& other) const throw();
	bool operator!= (const Colour& other) const throw();

	uint8 getRed() const throw()			{ return argb.getRed(); }

	uint8 getGreen() const throw()			  { return argb.getGreen(); }

	uint8 getBlue() const throw()			   { return argb.getBlue(); }

	float getFloatRed() const throw();

	float getFloatGreen() const throw();

	float getFloatBlue() const throw();

	const PixelARGB getPixelARGB() const throw();

	uint32 getARGB() const throw();

	uint8 getAlpha() const throw()			  { return argb.getAlpha(); }

	float getFloatAlpha() const throw();

	bool isOpaque() const throw();

	bool isTransparent() const throw();

	const Colour withAlpha (uint8 newAlpha) const throw();

	const Colour withAlpha (float newAlpha) const throw();

	const Colour withMultipliedAlpha (float alphaMultiplier) const throw();

	const Colour overlaidWith (const Colour& foregroundColour) const throw();

	const Colour interpolatedWith (const Colour& other, float proportionOfOther) const throw();

	float getHue() const throw();

	float getSaturation() const throw();

	float getBrightness() const throw();

	void getHSB (float& hue,
				 float& saturation,
				 float& brightness) const throw();

	const Colour withHue (float newHue) const throw();

	const Colour withSaturation (float newSaturation) const throw();

	const Colour withBrightness (float newBrightness) const throw();

	const Colour withRotatedHue (float amountToRotate) const throw();

	const Colour withMultipliedSaturation (float multiplier) const throw();

	const Colour withMultipliedBrightness (float amount) const throw();

	const Colour brighter (float amountBrighter = 0.4f) const throw();

	const Colour darker (float amountDarker = 0.4f) const throw();

	const Colour contrasting (float amount = 1.0f) const throw();

	static const Colour contrasting (const Colour& colour1,
									 const Colour& colour2) throw();

	static const Colour greyLevel (float brightness) throw();

	const String toString() const;

	static const Colour fromString (const String& encodedColourString);

	const String toDisplayString (bool includeAlphaValue) const;

	juce_UseDebuggingNewOperator

private:
	PixelARGB argb;
};

#endif   // __JUCE_COLOUR_JUCEHEADER__
/*** End of inlined file: juce_Colour.h ***/

class Colours
{
public:
	static JUCE_API const Colour

	transparentBlack,   /**< ARGB = 0x00000000 */
	transparentWhite,   /**< ARGB = 0x00ffffff */

	black,		  /**< ARGB = 0xff000000 */
	white,		  /**< ARGB = 0xffffffff */
	blue,		   /**< ARGB = 0xff0000ff */
	grey,		   /**< ARGB = 0xff808080 */
	green,		  /**< ARGB = 0xff008000 */
	red,		/**< ARGB = 0xffff0000 */
	yellow,		 /**< ARGB = 0xffffff00 */

	aliceblue,		  antiquewhite,	   aqua,		   aquamarine,
	azure,		  beige,		  bisque,		 blanchedalmond,
	blueviolet,		 brown,		  burlywood,	  cadetblue,
	chartreuse,		 chocolate,	  coral,		  cornflowerblue,
	cornsilk,		   crimson,		cyan,		   darkblue,
	darkcyan,		   darkgoldenrod,	  darkgrey,	   darkgreen,
	darkkhaki,		  darkmagenta,	darkolivegreen,	 darkorange,
	darkorchid,		 darkred,		darksalmon,	 darkseagreen,
	darkslateblue,	  darkslategrey,	  darkturquoise,	  darkviolet,
	deeppink,		   deepskyblue,	dimgrey,		dodgerblue,
	firebrick,		  floralwhite,	forestgreen,	fuchsia,
	gainsboro,		  gold,		   goldenrod,	  greenyellow,
	honeydew,		   hotpink,		indianred,	  indigo,
	ivory,		  khaki,		  lavender,	   lavenderblush,
	lemonchiffon,	   lightblue,	  lightcoral,	 lightcyan,
	lightgoldenrodyellow,   lightgreen,	 lightgrey,	  lightpink,
	lightsalmon,		lightseagreen,	  lightskyblue,	   lightslategrey,
	lightsteelblue,	 lightyellow,	lime,		   limegreen,
	linen,		  magenta,		maroon,		 mediumaquamarine,
	mediumblue,		 mediumorchid,	   mediumpurple,	   mediumseagreen,
	mediumslateblue,	mediumspringgreen,  mediumturquoise,	mediumvioletred,
	midnightblue,	   mintcream,	  mistyrose,	  navajowhite,
	navy,		   oldlace,		olive,		  olivedrab,
	orange,		 orangered,	  orchid,		 palegoldenrod,
	palegreen,		  paleturquoise,	  palevioletred,	  papayawhip,
	peachpuff,		  peru,		   pink,		   plum,
	powderblue,		 purple,		 rosybrown,	  royalblue,
	saddlebrown,		salmon,		 sandybrown,	 seagreen,
	seashell,		   sienna,		 silver,		 skyblue,
	slateblue,		  slategrey,	  snow,		   springgreen,
	steelblue,		  tan,		teal,		   thistle,
	tomato,		 turquoise,	  violet,		 wheat,
	whitesmoke,		 yellowgreen;

	static JUCE_API const Colour findColourForName (const String& colourName,
													const Colour& defaultColour);

private:

	// this isn't a class you should ever instantiate - it's just here for the
	// static values in it.
	Colours();
	Colours (const Colours&);
	Colours& operator= (const Colours&);
};

#endif   // __JUCE_COLOURS_JUCEHEADER__
/*** End of inlined file: juce_Colours.h ***/


/*** Start of inlined file: juce_FillType.h ***/
#ifndef __JUCE_FILLTYPE_JUCEHEADER__
#define __JUCE_FILLTYPE_JUCEHEADER__


/*** Start of inlined file: juce_ColourGradient.h ***/
#ifndef __JUCE_COLOURGRADIENT_JUCEHEADER__
#define __JUCE_COLOURGRADIENT_JUCEHEADER__

class JUCE_API  ColourGradient
{
public:

	ColourGradient (const Colour& colour1, float x1, float y1,
					const Colour& colour2, float x2, float y2,
					bool isRadial) throw();

	ColourGradient() throw();

	~ColourGradient() throw();

	void clearColours() throw();

	void addColour (double proportionAlongGradient,
					const Colour& colour) throw();

	void multiplyOpacity (float multiplier) throw();

	int getNumColours() const throw();

	double getColourPosition (int index) const throw();

	const Colour getColour (int index) const throw();

	const Colour getColourAtPosition (float position) const throw();

	int createLookupTable (const AffineTransform& transform, HeapBlock <PixelARGB>& resultLookupTable) const throw();

	bool isOpaque() const throw();

	bool isInvisible() const throw();

	float x1;
	float y1;

	float x2;
	float y2;

	bool isRadial;

	juce_UseDebuggingNewOperator

private:
	Array <uint32> colours;
};

#endif   // __JUCE_COLOURGRADIENT_JUCEHEADER__
/*** End of inlined file: juce_ColourGradient.h ***/

class Image;

class JUCE_API  FillType
{
public:
	FillType() throw();

	FillType (const Colour& colour) throw();

	FillType (const ColourGradient& gradient) throw();

	FillType (const Image& image, const AffineTransform& transform) throw();

	FillType (const FillType& other) throw();

	FillType& operator= (const FillType& other) throw();

	~FillType() throw();

	bool isColour() const throw()	   { return gradient == 0 && image == 0; }

	bool isGradient() const throw()	 { return gradient != 0; }

	bool isTiledImage() const throw()	   { return image != 0; }

	void setColour (const Colour& newColour) throw();

	void setGradient (const ColourGradient& newGradient) throw();

	void setTiledImage (const Image& image, const AffineTransform& transform) throw();

	void setOpacity (float newOpacity) throw();

	float getOpacity() const throw()	{ return colour.getFloatAlpha(); }

	Colour colour;

	ScopedPointer <ColourGradient> gradient;

	const Image* image;

	AffineTransform transform;

	juce_UseDebuggingNewOperator
};

#endif   // __JUCE_FILLTYPE_JUCEHEADER__
/*** End of inlined file: juce_FillType.h ***/


/*** Start of inlined file: juce_RectanglePlacement.h ***/
#ifndef __JUCE_RECTANGLEPLACEMENT_JUCEHEADER__
#define __JUCE_RECTANGLEPLACEMENT_JUCEHEADER__

class JUCE_API  RectanglePlacement
{
public:

	inline RectanglePlacement (int flags_) throw()  : flags (flags_) {}

	RectanglePlacement (const RectanglePlacement& other) throw();

	RectanglePlacement& operator= (const RectanglePlacement& other) throw();

	enum
	{

		xLeft				   = 1,

		xRight				  = 2,

		xMid					= 4,

		yTop					= 8,

		yBottom				 = 16,

		yMid					= 32,

		stretchToFit				= 64,

		fillDestination			 = 128,

		onlyReduceInSize			= 256,

		onlyIncreaseInSize			  = 512,

		doNotResize				 = (onlyIncreaseInSize | onlyReduceInSize),

		centred				 = 4 + 32
	};

	inline int getFlags() const throw()				 { return flags; }

	inline bool testFlags (int flagsToTest) const throw()	   { return (flags & flagsToTest) != 0; }

	void applyTo (double& sourceX,
				  double& sourceY,
				  double& sourceW,
				  double& sourceH,
				  double destinationX,
				  double destinationY,
				  double destinationW,
				  double destinationH) const throw();

	const AffineTransform getTransformToFit (float sourceX,
											 float sourceY,
											 float sourceW,
											 float sourceH,
											 float destinationX,
											 float destinationY,
											 float destinationW,
											 float destinationH) const throw();

private:

	int flags;
};

#endif   // __JUCE_RECTANGLEPLACEMENT_JUCEHEADER__
/*** End of inlined file: juce_RectanglePlacement.h ***/

class LowLevelGraphicsContext;
class Image;
class RectangleList;

class JUCE_API  Graphics
{
public:

	explicit Graphics (Image& imageToDrawOnto);

	~Graphics();

	void setColour (const Colour& newColour);

	void setOpacity (const float newOpacity);

	void setGradientFill (const ColourGradient& gradient);

	void setTiledImageFill (const Image& imageToUse,
							int anchorX, int anchorY,
							float opacity);

	void setFillType (const FillType& newFill);

	void setFont (const Font& newFont);

	void setFont (float newFontHeight, int fontStyleFlags = Font::plain);

	void drawSingleLineText (const String& text,
							 int startX, int baselineY) const;

	void drawMultiLineText (const String& text,
							int startX, int baselineY,
							int maximumLineWidth) const;

	void drawTextAsPath (const String& text,
						 const AffineTransform& transform) const;

	void drawText (const String& text,
				   int x, int y, int width, int height,
				   const Justification& justificationType,
				   bool useEllipsesIfTooBig) const;

	void drawFittedText (const String& text,
						 int x, int y, int width, int height,
						 const Justification& justificationFlags,
						 int maximumNumberOfLines,
						 float minimumHorizontalScale = 0.7f) const;

	void fillAll() const;

	void fillAll (const Colour& colourToUse) const;

	void fillRect (int x, int y, int width, int height) const;

	void fillRect (const Rectangle<int>& rectangle) const;

	void fillRect (float x, float y, float width, float height) const;

	void fillRoundedRectangle (float x, float y, float width, float height,
							   float cornerSize) const;

	void fillRoundedRectangle (const Rectangle<float>& rectangle,
							   float cornerSize) const;

	void fillCheckerBoard (int x, int y, int width, int height,
						   int checkWidth, int checkHeight,
						   const Colour& colour1, const Colour& colour2) const;

	void drawRect (int x, int y, int width, int height,
				   int lineThickness = 1) const;

	void drawRect (float x, float y, float width, float height,
				   float lineThickness = 1.0f) const;

	void drawRect (const Rectangle<int>& rectangle,
				   int lineThickness = 1) const;

	void drawRoundedRectangle (float x, float y, float width, float height,
							   float cornerSize, float lineThickness) const;

	void drawRoundedRectangle (const Rectangle<float>& rectangle,
							   float cornerSize, float lineThickness) const;

	void drawBevel (int x, int y, int width, int height,
					int bevelThickness,
					const Colour& topLeftColour = Colours::white,
					const Colour& bottomRightColour = Colours::black,
					bool useGradient = true,
					bool sharpEdgeOnOutside = true) const;

	void setPixel (int x, int y) const;

	void fillEllipse (float x, float y, float width, float height) const;

	void drawEllipse (float x, float y, float width, float height,
					  float lineThickness) const;

	void drawLine (float startX, float startY, float endX, float endY) const;

	void drawLine (float startX, float startY, float endX, float endY,
				   float lineThickness) const;

	void drawLine (const Line& line) const;

	void drawLine (const Line& line, float lineThickness) const;

	void drawDashedLine (float startX, float startY,
						 float endX, float endY,
						 const float* dashLengths, int numDashLengths,
						 float lineThickness = 1.0f) const;

	void drawVerticalLine (int x, float top, float bottom) const;

	void drawHorizontalLine (int y, float left, float right) const;

	void fillPath (const Path& path,
				   const AffineTransform& transform = AffineTransform::identity) const;

	void strokePath (const Path& path,
					 const PathStrokeType& strokeType,
					 const AffineTransform& transform = AffineTransform::identity) const;

	void drawArrow (float startX, float startY,
					float endX, float endY,
					float lineThickness,
					float arrowheadWidth,
					float arrowheadLength) const;

	enum ResamplingQuality
	{
		lowResamplingQuality	 = 0,	/**< Just uses a nearest-neighbour algorithm for resampling. */
		mediumResamplingQuality  = 1,	/**< Uses bilinear interpolation for upsampling and area-averaging for downsampling. */
		highResamplingQuality	= 2	 /**< Uses bicubic interpolation for upsampling and area-averaging for downsampling. */
	};

	void setImageResamplingQuality (const ResamplingQuality newQuality);

	void drawImageAt (const Image* const imageToDraw, int topLeftX, int topLeftY,
					  bool fillAlphaChannelWithCurrentBrush = false) const;

	void drawImage (const Image* const imageToDraw,
					int destX, int destY, int destWidth, int destHeight,
					int sourceX, int sourceY, int sourceWidth, int sourceHeight,
					bool fillAlphaChannelWithCurrentBrush = false) const;

	void drawImageTransformed (const Image* imageToDraw,
							   const Rectangle<int>& imageSubRegion,
							   const AffineTransform& transform,
							   bool fillAlphaChannelWithCurrentBrush = false) const;

	void drawImageWithin (const Image* imageToDraw,
						  int destX, int destY, int destWidth, int destHeight,
						  const RectanglePlacement& placementWithinTarget,
						  bool fillAlphaChannelWithCurrentBrush = false) const;

	const Rectangle<int> getClipBounds() const;

	bool clipRegionIntersects (int x, int y, int width, int height) const;

	bool reduceClipRegion (int x, int y, int width, int height);

	bool reduceClipRegion (const RectangleList& clipRegion);

	bool reduceClipRegion (const Path& path, const AffineTransform& transform = AffineTransform::identity);

	bool reduceClipRegion (const Image& image, const Rectangle<int>& sourceClipRegion,
						   const AffineTransform& transform);

	void excludeClipRegion (const Rectangle<int>& rectangleToExclude);

	bool isClipEmpty() const;

	void saveState();

	void restoreState();

	void setOrigin (int newOriginX, int newOriginY);

	void resetToDefaultState();

	bool isVectorDevice() const;

	juce_UseDebuggingNewOperator

	Graphics (LowLevelGraphicsContext* const internalContext) throw();

	LowLevelGraphicsContext* getInternalContext() const throw()	 { return context; }

private:

	LowLevelGraphicsContext* const context;
	ScopedPointer <LowLevelGraphicsContext> contextToDelete;

	bool saveStatePending;
	void saveStateIfPending();

	Graphics (const Graphics&);
	Graphics& operator= (const Graphics& other);
};

#endif   // __JUCE_GRAPHICS_JUCEHEADER__
/*** End of inlined file: juce_Graphics.h ***/

class JUCE_API  ImageEffectFilter
{
public:

	virtual void applyEffect (Image& sourceImage,
							  Graphics& destContext) = 0;

	virtual ~ImageEffectFilter() {}

};

#endif   // __JUCE_IMAGEEFFECTFILTER_JUCEHEADER__
/*** End of inlined file: juce_ImageEffectFilter.h ***/


/*** Start of inlined file: juce_RectangleList.h ***/
#ifndef __JUCE_RECTANGLELIST_JUCEHEADER__
#define __JUCE_RECTANGLELIST_JUCEHEADER__

class JUCE_API  RectangleList
{
public:

	RectangleList() throw();

	RectangleList (const RectangleList& other) throw();

	RectangleList (const Rectangle<int>& rect) throw();

	RectangleList& operator= (const RectangleList& other) throw();

	~RectangleList() throw();

	bool isEmpty() const throw();

	int getNumRectangles() const throw()			{ return rects.size(); }

	const Rectangle<int> getRectangle (const int index) const throw();

	void clear() throw();

	void add (int x, int y, int width, int height) throw();

	void add (const Rectangle<int>& rect) throw();

	void addWithoutMerging (const Rectangle<int>& rect) throw();

	void add (const RectangleList& other) throw();

	void subtract (const Rectangle<int>& rect) throw();

	void subtract (const RectangleList& otherList) throw();

	bool clipTo (const Rectangle<int>& rect) throw();

	bool clipTo (const RectangleList& other) throw();

	bool getIntersectionWith (const Rectangle<int>& rect, RectangleList& destRegion) const throw();

	void swapWith (RectangleList& otherList) throw();

	bool containsPoint (int x, int y) const throw();

	bool containsRectangle (const Rectangle<int>& rectangleToCheck) const throw();

	bool intersectsRectangle (const Rectangle<int>& rectangleToCheck) const throw();

	bool intersects (const RectangleList& other) const throw();

	const Rectangle<int> getBounds() const throw();

	void consolidate() throw();

	void offsetAll (int dx, int dy) throw();

	const Path toPath() const throw();

	class Iterator
	{
	public:

		Iterator (const RectangleList& list) throw();
		~Iterator() throw();

		bool next() throw();

		const Rectangle<int>* getRectangle() const throw()	   { return current; }

		juce_UseDebuggingNewOperator

	private:
		const Rectangle<int>* current;
		const RectangleList& owner;
		int index;

		Iterator (const Iterator&);
		Iterator& operator= (const Iterator&);
	};

	juce_UseDebuggingNewOperator

private:
	friend class Iterator;
	Array <Rectangle<int> > rects;
};

#endif   // __JUCE_RECTANGLELIST_JUCEHEADER__
/*** End of inlined file: juce_RectangleList.h ***/


/*** Start of inlined file: juce_BorderSize.h ***/
#ifndef __JUCE_BORDERSIZE_JUCEHEADER__
#define __JUCE_BORDERSIZE_JUCEHEADER__

class JUCE_API  BorderSize
{
public:

	BorderSize() throw();

	BorderSize (const BorderSize& other) throw();

	BorderSize (int topGap,
				int leftGap,
				int bottomGap,
				int rightGap) throw();

	explicit BorderSize (int allGaps) throw();

	~BorderSize() throw();

	int getTop() const throw()			  { return top; }

	int getLeft() const throw()			 { return left; }

	int getBottom() const throw()			   { return bottom; }

	int getRight() const throw()			{ return right; }

	int getTopAndBottom() const throw()		 { return top + bottom; }

	int getLeftAndRight() const throw()		 { return left + right; }

	void setTop (int newTopGap) throw();

	void setLeft (int newLeftGap) throw();

	void setBottom (int newBottomGap) throw();

	void setRight (int newRightGap) throw();

	const Rectangle<int> subtractedFrom (const Rectangle<int>& original) const throw();

	void subtractFrom (Rectangle<int>& rectangle) const throw();

	const Rectangle<int> addedTo (const Rectangle<int>& original) const throw();

	void addTo (Rectangle<int>& original) const throw();

	bool operator== (const BorderSize& other) const throw();
	bool operator!= (const BorderSize& other) const throw();

	juce_UseDebuggingNewOperator

private:
	int top, left, bottom, right;
};

#endif   // __JUCE_BORDERSIZE_JUCEHEADER__
/*** End of inlined file: juce_BorderSize.h ***/

class LookAndFeel;
class MouseInputSource;
class MouseInputSourceInternal;
class ComponentPeer;

class JUCE_API  Component  : public MouseListener,
							 protected MessageListener
{
public:

	Component();

	virtual ~Component();

	explicit Component (const String& componentName);

	const String& getName() const throw()		   { return componentName_; }

	virtual void setName (const String& newName);

	bool isValidComponent() const;

	virtual void setVisible (bool shouldBeVisible);

	bool isVisible() const throw()			  { return flags.visibleFlag; }

	virtual void visibilityChanged();

	bool isShowing() const;

	void fadeOutComponent (int lengthOfFadeOutInMilliseconds,
						   int deltaXToMove = 0,
						   int deltaYToMove = 0,
						   float scaleFactorAtEnd = 1.0f);

	virtual void addToDesktop (int windowStyleFlags,
							   void* nativeWindowToAttachTo = 0);

	void removeFromDesktop();

	bool isOnDesktop() const throw();

	ComponentPeer* getPeer() const;

	virtual void userTriedToCloseWindow();

	virtual void minimisationStateChanged (bool isNowMinimised);

	void toFront (bool shouldAlsoGainFocus);

	void toBack();

	void toBehind (Component* other);

	void setAlwaysOnTop (bool shouldStayOnTop);

	bool isAlwaysOnTop() const throw();

	inline int getX() const throw()			 { return bounds_.getX(); }

	inline int getY() const throw()			 { return bounds_.getY(); }

	inline int getWidth() const throw()			 { return bounds_.getWidth(); }

	inline int getHeight() const throw()			{ return bounds_.getHeight(); }

	int getRight() const throw()				{ return bounds_.getRight(); }

	const Point<int> getPosition() const throw()		{ return bounds_.getPosition(); }

	int getBottom() const throw()			   { return bounds_.getBottom(); }

	const Rectangle<int>& getBounds() const throw()	 { return bounds_; }

	const Rectangle<int> getLocalBounds() const throw();

	void getVisibleArea (RectangleList& result,
						 bool includeSiblings) const;

	int getScreenX() const;

	int getScreenY() const;

	const Point<int> getScreenPosition() const;

	const Rectangle<int> getScreenBounds() const;

	const Point<int> relativePositionToGlobal (const Point<int>& relativePosition) const;

	const Point<int> globalPositionToRelative (const Point<int>& screenPosition) const;

	const Point<int> relativePositionToOtherComponent (const Component* targetComponent,
													   const Point<int>& positionRelativeToThis) const;

	void setTopLeftPosition (int x, int y);

	void setTopRightPosition (int x, int y);

	void setSize (int newWidth, int newHeight);

	void setBounds (int x, int y, int width, int height);

	void setBounds (const Rectangle<int>& newBounds);

	void setBoundsRelative (float proportionalX, float proportionalY,
							float proportionalWidth, float proportionalHeight);

	void setBoundsInset (const BorderSize& borders);

	void setBoundsToFit (int x, int y, int width, int height,
						 const Justification& justification,
						 bool onlyReduceInSize);

	void setCentrePosition (int x, int y);

	void setCentreRelative (float x, float y);

	void centreWithSize (int width, int height);

	int proportionOfWidth (float proportion) const throw();

	int proportionOfHeight (float proportion) const throw();

	int getParentWidth() const throw();

	int getParentHeight() const throw();

	const Rectangle<int> getParentMonitorArea() const;

	int getNumChildComponents() const throw();

	Component* getChildComponent (int index) const throw();

	int getIndexOfChildComponent (const Component* child) const throw();

	void addChildComponent (Component* child, int zOrder = -1);

	void addAndMakeVisible (Component* child, int zOrder = -1);

	void removeChildComponent (Component* childToRemove);

	Component* removeChildComponent (int childIndexToRemove);

	void removeAllChildren();

	void deleteAllChildren();

	Component* getParentComponent() const throw()		   { return parentComponent_; }

	template <class TargetClass>
	TargetClass* findParentComponentOfClass (TargetClass* const dummyParameter = 0) const
	{
		(void) dummyParameter;
		Component* p = parentComponent_;
		while (p != 0)
		{
			TargetClass* target = dynamic_cast <TargetClass*> (p);
			if (target != 0)
				return target;

			p = p->parentComponent_;
		}

		return 0;
	}

	Component* getTopLevelComponent() const throw();

	bool isParentOf (const Component* possibleChild) const throw();

	virtual void parentHierarchyChanged();

	virtual void childrenChanged();

	virtual bool hitTest (int x, int y);

	void setInterceptsMouseClicks (bool allowClicksOnThisComponent,
								   bool allowClicksOnChildComponents) throw();

	void getInterceptsMouseClicks (bool& allowsClicksOnThisComponent,
								   bool& allowsClicksOnChildComponents) const throw();

	virtual bool contains (int x, int y);

	bool reallyContains (int x, int y, bool returnTrueIfWithinAChild);

	Component* getComponentAt (int x, int y);

	Component* getComponentAt (const Point<int>& position);

	void repaint();

	void repaint (int x, int y, int width, int height);

	void setBufferedToImage (bool shouldBeBuffered);

	Image* createComponentSnapshot (const Rectangle<int>& areaToGrab,
									bool clipImageToComponentBounds = true);

	void paintEntireComponent (Graphics& context);

	void setComponentEffect (ImageEffectFilter* newEffect);

	ImageEffectFilter* getComponentEffect() const throw()		   { return effect_; }

	LookAndFeel& getLookAndFeel() const throw();

	void setLookAndFeel (LookAndFeel* newLookAndFeel);

	virtual void lookAndFeelChanged();

	void sendLookAndFeelChange();

	void setOpaque (bool shouldBeOpaque);

	bool isOpaque() const throw();

	void setBroughtToFrontOnMouseClick (bool shouldBeBroughtToFront) throw();

	bool isBroughtToFrontOnMouseClick() const throw();

	// Keyboard focus methods

	void setWantsKeyboardFocus (bool wantsFocus) throw();

	bool getWantsKeyboardFocus() const throw();

	void setMouseClickGrabsKeyboardFocus (const bool shouldGrabFocus);

	bool getMouseClickGrabsKeyboardFocus() const throw();

	void grabKeyboardFocus();

	bool hasKeyboardFocus (bool trueIfChildIsFocused) const;

	static Component* JUCE_CALLTYPE getCurrentlyFocusedComponent() throw();

	void moveKeyboardFocusToSibling (bool moveToNext);

	virtual KeyboardFocusTraverser* createFocusTraverser();

	int getExplicitFocusOrder() const;

	void setExplicitFocusOrder (int newFocusOrderIndex);

	void setFocusContainer (bool shouldBeFocusContainer) throw();

	bool isFocusContainer() const throw();

	bool isEnabled() const throw();

	void setEnabled (bool shouldBeEnabled);

	virtual void enablementChanged();

	void setMouseCursor (const MouseCursor& cursorType);

	virtual const MouseCursor getMouseCursor();

	void updateMouseCursor() const;

	virtual void paint (Graphics& g);

	virtual void paintOverChildren (Graphics& g);

	virtual void mouseMove	 (const MouseEvent& e);

	virtual void mouseEnter	(const MouseEvent& e);

	virtual void mouseExit	 (const MouseEvent& e);

	virtual void mouseDown	 (const MouseEvent& e);

	virtual void mouseDrag	 (const MouseEvent& e);

	virtual void mouseUp	   (const MouseEvent& e);

	virtual void mouseDoubleClick  (const MouseEvent& e);

	virtual void mouseWheelMove	(const MouseEvent& e,
									float wheelIncrementX,
									float wheelIncrementY);

	static void beginDragAutoRepeat (int millisecondIntervalBetweenCallbacks);

	void setRepaintsOnMouseActivity (bool shouldRepaint) throw();

	void addMouseListener (MouseListener* newListener,
						   bool wantsEventsForAllNestedChildComponents);

	void removeMouseListener (MouseListener* listenerToRemove);

	void addKeyListener (KeyListener* newListener);

	void removeKeyListener (KeyListener* listenerToRemove);

	virtual bool keyPressed (const KeyPress& key);

	virtual bool keyStateChanged (bool isKeyDown);

	virtual void modifierKeysChanged (const ModifierKeys& modifiers);

	enum FocusChangeType
	{
		focusChangedByMouseClick,   /**< Means that the user clicked the mouse to change focus. */
		focusChangedByTabKey,	   /**< Means that the user pressed the tab key to move the focus. */
		focusChangedDirectly	/**< Means that the focus was changed by a call to grabKeyboardFocus(). */
	};

	virtual void focusGained (FocusChangeType cause);

	virtual void focusLost (FocusChangeType cause);

	virtual void focusOfChildComponentChanged (FocusChangeType cause);

	bool isMouseOver() const throw();

	bool isMouseButtonDown() const throw();

	bool isMouseOverOrDragging() const throw();

	static bool JUCE_CALLTYPE isMouseButtonDownAnywhere() throw();

	const Point<int> getMouseXYRelative() const;

	virtual void resized();

	virtual void moved();

	virtual void childBoundsChanged (Component* child);

	virtual void parentSizeChanged();

	virtual void broughtToFront();

	void addComponentListener (ComponentListener* newListener);

	void removeComponentListener (ComponentListener* listenerToRemove);

	void postCommandMessage (int commandId);

	virtual void handleCommandMessage (int commandId);

	int runModalLoop();

	void enterModalState (bool takeKeyboardFocus = true);

	void exitModalState (int returnValue);

	bool isCurrentlyModal() const throw();

	static int JUCE_CALLTYPE getNumCurrentlyModalComponents() throw();

	static Component* JUCE_CALLTYPE getCurrentlyModalComponent (int index = 0) throw();

	bool isCurrentlyBlockedByAnotherModalComponent() const;

	virtual bool canModalEventBeSentToComponent (const Component* targetComponent);

	virtual void inputAttemptWhenModal();

	NamedValueSet& getProperties() throw()				  { return properties; }

	const NamedValueSet& getProperties() const throw()		  { return properties; }

	const Colour findColour (int colourId, bool inheritFromParent = false) const;

	void setColour (int colourId, const Colour& colour);

	void removeColour (int colourId);

	bool isColourSpecified (int colourId) const;

	void copyAllExplicitColoursTo (Component& target) const;

	virtual void colourChanged();

	void* getWindowHandle() const;

	uint32 getComponentUID() const throw()		{ return componentUID; }

	template <class ComponentType>
	class SafePointer   : private ComponentListener
	{
	public:
		SafePointer()					   : comp (0) {}

		SafePointer (ComponentType* const component)	: comp (component)   { attach(); }

		SafePointer (const SafePointer& other)		  : comp (other.comp)  { attach(); }

		~SafePointer()					  { detach(); }

		SafePointer& operator= (const SafePointer& other)   { return operator= (other.comp); }

		SafePointer& operator= (ComponentType* const newComponent)
		{
			detach();
			comp = newComponent;
			attach();
			return *this;
		}

		operator ComponentType*() const throw()		 { return comp; }

		ComponentType* getComponent() const throw()	 { return comp; }

		/** Returns the component that this pointer refers to, or null if the component no longer exists. */
		ComponentType* operator->() throw()		 { jassert (comp != 0); return comp; }

		/** Returns the component that this pointer refers to, or null if the component no longer exists. */
		const ComponentType* operator->() const throw()	 { jassert (comp != 0); return comp; }

		juce_UseDebuggingNewOperator

	private:
		ComponentType* comp;

		void attach()   { if (comp != 0) comp->addComponentListener (this); }
		void detach()   { if (comp != 0) comp->removeComponentListener (this); }
		void componentBeingDeleted (Component&)	 { comp = 0; }
	};

	class BailOutChecker
	{
	public:
		BailOutChecker (Component* component1,
						Component* component2 = 0);

		bool shouldBailOut() const throw();

	private:
		typedef SafePointer<Component> SafeComponentPtr;
		SafeComponentPtr safePointer1, safePointer2;
		Component* const component2;

		BailOutChecker (const BailOutChecker&);
		BailOutChecker& operator= (const BailOutChecker&);
	};

	juce_UseDebuggingNewOperator

private:

	friend class ComponentPeer;
	friend class InternalDragRepeater;
	friend class MouseInputSource;
	friend class MouseInputSourceInternal;

	static Component* currentlyFocusedComponent;

	String componentName_;
	Component* parentComponent_;
	uint32 componentUID;
	Rectangle<int> bounds_;
	int numDeepMouseListeners;
	Array <Component*> childComponentList_;
	LookAndFeel* lookAndFeel_;
	MouseCursor cursor_;
	ImageEffectFilter* effect_;
	Image* bufferedImage_;
	VoidArray* mouseListeners_;
	VoidArray* keyListeners_;
	ListenerList <ComponentListener> componentListeners;
	NamedValueSet properties;

	struct ComponentFlags
	{
		bool hasHeavyweightPeerFlag	 : 1;
		bool visibleFlag		: 1;
		bool opaqueFlag		 : 1;
		bool ignoresMouseClicksFlag	 : 1;
		bool allowChildMouseClicksFlag  : 1;
		bool wantsFocusFlag		 : 1;
		bool isFocusContainerFlag	   : 1;
		bool dontFocusOnMouseClickFlag  : 1;
		bool alwaysOnTopFlag		: 1;
		bool bufferToImageFlag	  : 1;
		bool bringToFrontOnClickFlag	: 1;
		bool repaintOnMouseActivityFlag : 1;
		bool draggingFlag		   : 1;
		bool mouseOverFlag		  : 1;
		bool mouseInsideFlag		: 1;
		bool currentlyModalFlag	 : 1;
		bool isDisabledFlag		 : 1;
		bool childCompFocusedFlag	   : 1;
#ifdef JUCE_DEBUG
		bool isInsidePaintCall	  : 1;
#endif
	};

	union
	{
		uint32 componentFlags_;
		ComponentFlags flags;
	};

	void internalMouseEnter (MouseInputSource& source, const Point<int>& relativePos, const Time& time);
	void internalMouseExit  (MouseInputSource& source, const Point<int>& relativePos, const Time& time);
	void internalMouseDown  (MouseInputSource& source, const Point<int>& relativePos, const Time& time);
	void internalMouseUp	(MouseInputSource& source, const Point<int>& relativePos, const Time& time, const ModifierKeys& oldModifiers);
	void internalMouseDrag  (MouseInputSource& source, const Point<int>& relativePos, const Time& time);
	void internalMouseMove  (MouseInputSource& source, const Point<int>& relativePos, const Time& time);
	void internalMouseWheel (MouseInputSource& source, const Point<int>& relativePos, const Time& time, float amountX, float amountY);
	void internalBroughtToFront();
	void internalFocusGain (const FocusChangeType cause);
	void internalFocusLoss (const FocusChangeType cause);
	void internalChildFocusChange (FocusChangeType cause);
	void internalModalInputAttempt();
	void internalModifierKeysChanged();
	void internalChildrenChanged();
	void internalHierarchyChanged();
	void renderComponent (Graphics& context);
	void sendMovedResizedMessages (bool wasMoved, bool wasResized);
	void repaintParent();
	void sendFakeMouseMove() const;
	void takeKeyboardFocus (const FocusChangeType cause);
	void grabFocusInternal (const FocusChangeType cause, bool canTryParent = true);
	static void giveAwayFocus();
	void sendEnablementChangeMessage();
	static void* runModalLoopCallback (void*);
	static void bringModalComponentToFront();
	void subtractObscuredRegions (RectangleList& result, const Point<int>& delta,
								  const Rectangle<int>& clipRect,
								  const Component* const compToAvoid) const;
	void clipObscuredRegions (Graphics& g, const Rectangle<int>& clipRect,
							  int deltaX, int deltaY) const;

	// how much of the component is not off the edges of its parents
	const Rectangle<int> getUnclippedArea() const;
	void sendVisibilityChangeMessage();

	// This is included here just to cause a compile error if your code is still handling
	// drag-and-drop with this method. If so, just update it to use the new FileDragAndDropTarget
	// class, which is easy (just make your class inherit from FileDragAndDropTarget, and
	// implement its methods instead of this Component method).
	virtual void filesDropped (const StringArray&, int, int) {}

	// components aren't allowed to have copy constructors, as this would mess up parent
	// hierarchies. You might need to give your subclasses a private dummy constructor like
	// this one to avoid compiler warnings.
	Component (const Component&);

	Component& operator= (const Component&);

	// (dummy method to cause a deliberate compile error - if you hit this, you need to update your
	// subclass to use the new parameters to keyStateChanged)
	virtual void keyStateChanged() {};

protected:
	virtual void internalRepaint (int x, int y, int w, int h);

	virtual ComponentPeer* createNewPeer (int styleFlags, void* nativeWindowToAttachTo);

	void handleMessage (const Message&);
};

#endif   // __JUCE_COMPONENT_JUCEHEADER__
/*** End of inlined file: juce_Component.h ***/


/*** Start of inlined file: juce_ApplicationCommandInfo.h ***/
#ifndef __JUCE_APPLICATIONCOMMANDINFO_JUCEHEADER__
#define __JUCE_APPLICATIONCOMMANDINFO_JUCEHEADER__


/*** Start of inlined file: juce_ApplicationCommandID.h ***/
#ifndef __JUCE_APPLICATIONCOMMANDID_JUCEHEADER__
#define __JUCE_APPLICATIONCOMMANDID_JUCEHEADER__

typedef int CommandID;

namespace StandardApplicationCommandIDs
{
	static const CommandID  quit	   = 0x1001;

	static const CommandID  del		= 0x1002;

	static const CommandID  cut		= 0x1003;

	static const CommandID  copy	   = 0x1004;

	static const CommandID  paste	  = 0x1005;

	static const CommandID  selectAll	  = 0x1006;

	static const CommandID  deselectAll	= 0x1007;
}

#endif   // __JUCE_APPLICATIONCOMMANDID_JUCEHEADER__
/*** End of inlined file: juce_ApplicationCommandID.h ***/

struct JUCE_API  ApplicationCommandInfo
{

	explicit ApplicationCommandInfo (CommandID commandID) throw();

	void setInfo (const String& shortName,
				  const String& description,
				  const String& categoryName,
				  int flags) throw();

	void setActive (bool isActive) throw();

	void setTicked (bool isTicked) throw();

	void addDefaultKeypress (int keyCode,
							 const ModifierKeys& modifiers) throw();

	CommandID commandID;

	String shortName;

	String description;

	String categoryName;

	Array <KeyPress> defaultKeypresses;

	enum CommandFlags
	{
		isDisabled		  = 1 << 0,

		isTicked			= 1 << 1,

		wantsKeyUpDownCallbacks	 = 1 << 2,

		hiddenFromKeyEditor	 = 1 << 3,

		readOnlyInKeyEditor	 = 1 << 4,

		dontTriggerVisualFeedback   = 1 << 5
	};

	int flags;
};

#endif   // __JUCE_APPLICATIONCOMMANDINFO_JUCEHEADER__
/*** End of inlined file: juce_ApplicationCommandInfo.h ***/

class JUCE_API  ApplicationCommandTarget
{
public:

	ApplicationCommandTarget();

	virtual ~ApplicationCommandTarget();

	struct JUCE_API  InvocationInfo
	{

		InvocationInfo (const CommandID commandID) throw();

		CommandID commandID;

		int commandFlags;

		enum InvocationMethod
		{
			direct = 0,	 /**< The command is being invoked directly by a piece of code. */
			fromKeyPress,   /**< The command is being invoked by a key-press. */
			fromMenu,	   /**< The command is being invoked by a menu selection. */
			fromButton	  /**< The command is being invoked by a button click. */
		};

		InvocationMethod invocationMethod;

		Component* originatingComponent;

		KeyPress keyPress;

		bool isKeyDown;

		int millisecsSinceKeyPressed;
	};

	virtual ApplicationCommandTarget* getNextCommandTarget() = 0;

	virtual void getAllCommands (Array <CommandID>& commands) = 0;

	virtual void getCommandInfo (CommandID commandID, ApplicationCommandInfo& result) = 0;

	virtual bool perform (const InvocationInfo& info) = 0;

	bool invoke (const InvocationInfo& invocationInfo,
				 const bool asynchronously);

	bool invokeDirectly (const CommandID commandID,
						 const bool asynchronously);

	ApplicationCommandTarget* getTargetForCommand (const CommandID commandID);

	bool isCommandActive (const CommandID commandID);

	ApplicationCommandTarget* findFirstTargetParentComponent();

	juce_UseDebuggingNewOperator

private:
	// (for async invocation of commands)
	class CommandTargetMessageInvoker  : public MessageListener
	{
	public:
		CommandTargetMessageInvoker (ApplicationCommandTarget* const owner);
		~CommandTargetMessageInvoker();

		void handleMessage (const Message& message);

	private:
		ApplicationCommandTarget* const owner;

		CommandTargetMessageInvoker (const CommandTargetMessageInvoker&);
		CommandTargetMessageInvoker& operator= (const CommandTargetMessageInvoker&);
	};

	ScopedPointer <CommandTargetMessageInvoker> messageInvoker;

	friend class CommandTargetMessageInvoker;
	bool tryToInvoke (const InvocationInfo& info, const bool async);

	ApplicationCommandTarget (const ApplicationCommandTarget&);
	ApplicationCommandTarget& operator= (const ApplicationCommandTarget&);
};

#endif   // __JUCE_APPLICATIONCOMMANDTARGET_JUCEHEADER__
/*** End of inlined file: juce_ApplicationCommandTarget.h ***/


/*** Start of inlined file: juce_ActionListener.h ***/
#ifndef __JUCE_ACTIONLISTENER_JUCEHEADER__
#define __JUCE_ACTIONLISTENER_JUCEHEADER__

class JUCE_API  ActionListener
{
public:
	virtual ~ActionListener()  {}

	virtual void actionListenerCallback (const String& message) = 0;
};

#endif   // __JUCE_ACTIONLISTENER_JUCEHEADER__
/*** End of inlined file: juce_ActionListener.h ***/

class JUCE_API  JUCEApplication  : public ApplicationCommandTarget,
								   private ActionListener
{
protected:

	JUCEApplication();

public:
	virtual ~JUCEApplication();

	static JUCEApplication* getInstance() throw();

	virtual void initialise (const String& commandLineParameters) = 0;

	bool isInitialising() const throw();

	virtual void shutdown() = 0;

	virtual const String getApplicationName() = 0;

	virtual const String getApplicationVersion();

	virtual bool moreThanOneInstanceAllowed();

	virtual void anotherInstanceStarted (const String& commandLine);

	virtual void systemRequestedQuit();

	virtual void unhandledException (const std::exception* e,
									 const String& sourceFilename,
									 int lineNumber);

	static void quit();

	void setApplicationReturnValue (int newReturnValue) throw();

	int getApplicationReturnValue() const throw()		   { return appReturnValue; }

	const String getCommandLineParameters() const throw()	   { return commandLineParameters; }

	// These are used by the START_JUCE_APPLICATION() macro and aren't for public use.

	static int main (String& commandLine, JUCEApplication* newApp);
	static int main (int argc, const char* argv[], JUCEApplication* newApp);

	static void sendUnhandledException (const std::exception* e,
										const char* sourceFile,
										int lineNumber);

	ApplicationCommandTarget* getNextCommandTarget();
	void getCommandInfo (CommandID commandID, ApplicationCommandInfo& result);
	void getAllCommands (Array <CommandID>& commands);
	bool perform (const InvocationInfo& info);
	void actionListenerCallback (const String& message);

private:

	String commandLineParameters;
	int appReturnValue;
	bool stillInitialising;
	ScopedPointer<InterProcessLock> appLock;

	JUCEApplication (const JUCEApplication&);
	JUCEApplication& operator= (const JUCEApplication&);

public:
	bool initialiseApp (String& commandLine);
	static int shutdownAppAndClearUp();
};

#endif   // __JUCE_APPLICATION_JUCEHEADER__
/*** End of inlined file: juce_Application.h ***/


#endif
#ifndef __JUCE_APPLICATIONCOMMANDID_JUCEHEADER__

#endif
#ifndef __JUCE_APPLICATIONCOMMANDINFO_JUCEHEADER__

#endif
#ifndef __JUCE_APPLICATIONCOMMANDMANAGER_JUCEHEADER__

/*** Start of inlined file: juce_ApplicationCommandManager.h ***/
#ifndef __JUCE_APPLICATIONCOMMANDMANAGER_JUCEHEADER__
#define __JUCE_APPLICATIONCOMMANDMANAGER_JUCEHEADER__


/*** Start of inlined file: juce_Desktop.h ***/
#ifndef __JUCE_DESKTOP_JUCEHEADER__
#define __JUCE_DESKTOP_JUCEHEADER__


/*** Start of inlined file: juce_DeletedAtShutdown.h ***/
#ifndef __JUCE_DELETEDATSHUTDOWN_JUCEHEADER__
#define __JUCE_DELETEDATSHUTDOWN_JUCEHEADER__

class JUCE_API  DeletedAtShutdown
{
protected:
	DeletedAtShutdown();

	virtual ~DeletedAtShutdown();

public:
	static void deleteAll();

private:
	DeletedAtShutdown (const DeletedAtShutdown&);
	DeletedAtShutdown& operator= (const DeletedAtShutdown&);
};

#endif   // __JUCE_DELETEDATSHUTDOWN_JUCEHEADER__
/*** End of inlined file: juce_DeletedAtShutdown.h ***/


/*** Start of inlined file: juce_Timer.h ***/
#ifndef __JUCE_TIMER_JUCEHEADER__
#define __JUCE_TIMER_JUCEHEADER__

class InternalTimerThread;

class JUCE_API  Timer
{
protected:

	Timer() throw();

	Timer (const Timer& other) throw();

public:

	virtual ~Timer();

	virtual void timerCallback() = 0;

	void startTimer (int intervalInMilliseconds) throw();

	void stopTimer() throw();

	bool isTimerRunning() const throw()			 { return periodMs > 0; }

	int getTimerInterval() const throw()			{ return periodMs; }

private:
	friend class InternalTimerThread;
	int countdownMs, periodMs;
	Timer* previous;
	Timer* next;

	Timer& operator= (const Timer&);
};

#endif   // __JUCE_TIMER_JUCEHEADER__
/*** End of inlined file: juce_Timer.h ***/

class MouseInputSource;
class MouseInputSourceInternal;
class MouseListener;

class JUCE_API  FocusChangeListener
{
public:
	virtual ~FocusChangeListener()  {}

	virtual void globalFocusChanged (Component* focusedComponent) = 0;
};

class JUCE_API  Desktop  : private DeletedAtShutdown,
						   private Timer,
						   private AsyncUpdater
{
public:

	static Desktop& JUCE_CALLTYPE getInstance();

	const RectangleList getAllMonitorDisplayAreas (bool clippedToWorkArea = true) const throw();

	const Rectangle<int> getMainMonitorArea (bool clippedToWorkArea = true) const throw();

	const Rectangle<int> getMonitorAreaContaining (const Point<int>& position, bool clippedToWorkArea = true) const;

	static const Point<int> getMousePosition();

	static void setMousePosition (const Point<int>& newPosition);

	static const Point<int> getLastMouseDownPosition() throw();

	static int getMouseButtonClickCounter() throw();

	static void setScreenSaverEnabled (bool isEnabled) throw();

	static bool isScreenSaverEnabled() throw();

	void addGlobalMouseListener (MouseListener* listener);

	void removeGlobalMouseListener (MouseListener* listener);

	void addFocusChangeListener (FocusChangeListener* listener);

	void removeFocusChangeListener (FocusChangeListener* listener);

	void setKioskModeComponent (Component* componentToUse,
								bool allowMenusAndBars = true);

	Component* getKioskModeComponent() const throw()		{ return kioskModeComponent; }

	int getNumComponents() const throw();

	Component* getComponent (int index) const throw();

	Component* findComponentAt (const Point<int>& screenPosition) const;

	int getNumMouseSources() const throw()			  { return mouseSources.size(); }

	MouseInputSource* getMouseSource (int index) const throw()	  { return mouseSources [index]; }

	MouseInputSource& getMainMouseSource() const throw()		{ return *mouseSources.getUnchecked(0); }

	int getNumDraggingMouseSources() const throw();

	MouseInputSource* getDraggingMouseSource (int index) const throw();

	juce_UseDebuggingNewOperator

	void refreshMonitorSizes();

	static bool canUseSemiTransparentWindows() throw();

private:

	static Desktop* instance;

	friend class Component;
	friend class ComponentPeer;
	friend class MouseInputSource;
	friend class MouseInputSourceInternal;
	friend class DeletedAtShutdown;
	friend class TopLevelWindowManager;

	OwnedArray <MouseInputSource> mouseSources;
	void createMouseInputSources();

	ListenerList <MouseListener> mouseListeners;
	ListenerList <FocusChangeListener> focusListeners;

	Array <Component*> desktopComponents;
	Array <Rectangle<int> > monitorCoordsClipped, monitorCoordsUnclipped;

	Point<int> lastFakeMouseMove;
	void sendMouseMove();

	int mouseClickCounter;
	void incrementMouseClickCounter() throw();

	Component* kioskModeComponent;
	Rectangle<int> kioskComponentOriginalBounds;

	void timerCallback();
	void resetTimer();

	int getNumDisplayMonitors() const throw();
	const Rectangle<int> getDisplayMonitorCoordinates (int index, bool clippedToWorkArea) const throw();

	void addDesktopComponent (Component* c);
	void removeDesktopComponent (Component* c);
	void componentBroughtToFront (Component* c);

	void triggerFocusCallback();
	void handleAsyncUpdate();

	Desktop();
	~Desktop();

	Desktop (const Desktop&);
	Desktop& operator= (const Desktop&);
};

#endif   // __JUCE_DESKTOP_JUCEHEADER__
/*** End of inlined file: juce_Desktop.h ***/

class KeyPressMappingSet;
class ApplicationCommandManagerListener;

class JUCE_API  ApplicationCommandManager   : private AsyncUpdater,
											  private FocusChangeListener
{
public:

	ApplicationCommandManager();

	virtual ~ApplicationCommandManager();

	void clearCommands();

	void registerCommand (const ApplicationCommandInfo& newCommand);

	void registerAllCommandsForTarget (ApplicationCommandTarget* target);

	void removeCommand (CommandID commandID);

	void commandStatusChanged();

	int getNumCommands() const throw()						  { return commands.size(); }

	const ApplicationCommandInfo* getCommandForIndex (int index) const throw()	{ return commands [index]; }

	const ApplicationCommandInfo* getCommandForID (CommandID commandID) const throw();

	const String getNameOfCommand (CommandID commandID) const throw();

	const String getDescriptionOfCommand (CommandID commandID) const throw();

	const StringArray getCommandCategories() const throw();

	const Array <CommandID> getCommandsInCategory (const String& categoryName) const throw();

	KeyPressMappingSet* getKeyMappings() const throw()			  { return keyMappings; }

	bool invokeDirectly (CommandID commandID, bool asynchronously);

	bool invoke (const ApplicationCommandTarget::InvocationInfo& invocationInfo,
				 bool asynchronously);

	virtual ApplicationCommandTarget* getFirstCommandTarget (CommandID commandID);

	void setFirstCommandTarget (ApplicationCommandTarget* newTarget) throw();

	ApplicationCommandTarget* getTargetForCommand (CommandID commandID,
												   ApplicationCommandInfo& upToDateInfo);

	void addListener (ApplicationCommandManagerListener* listener) throw();

	void removeListener (ApplicationCommandManagerListener* listener) throw();

	static ApplicationCommandTarget* findDefaultComponentTarget();

	static ApplicationCommandTarget* findTargetForComponent (Component* component);

	juce_UseDebuggingNewOperator

private:

	OwnedArray <ApplicationCommandInfo> commands;
	ListenerList <ApplicationCommandManagerListener> listeners;
	ScopedPointer <KeyPressMappingSet> keyMappings;
	ApplicationCommandTarget* firstTarget;

	void sendListenerInvokeCallback (const ApplicationCommandTarget::InvocationInfo& info);
	void handleAsyncUpdate();
	void globalFocusChanged (Component*);

	// xxx this is just here to cause a compile error in old code that hasn't been changed to use the new
	// version of this method.
	virtual short getFirstCommandTarget() { return 0; }

	ApplicationCommandManager (const ApplicationCommandManager&);
	ApplicationCommandManager& operator= (const ApplicationCommandManager&);
};

class JUCE_API  ApplicationCommandManagerListener
{
public:

	virtual ~ApplicationCommandManagerListener()  {}

	virtual void applicationCommandInvoked (const ApplicationCommandTarget::InvocationInfo& info) = 0;

	virtual void applicationCommandListChanged() = 0;
};

#endif   // __JUCE_APPLICATIONCOMMANDMANAGER_JUCEHEADER__
/*** End of inlined file: juce_ApplicationCommandManager.h ***/


#endif
#ifndef __JUCE_APPLICATIONCOMMANDTARGET_JUCEHEADER__

#endif
#ifndef __JUCE_APPLICATIONPROPERTIES_JUCEHEADER__

/*** Start of inlined file: juce_ApplicationProperties.h ***/
#ifndef __JUCE_APPLICATIONPROPERTIES_JUCEHEADER__
#define __JUCE_APPLICATIONPROPERTIES_JUCEHEADER__


/*** Start of inlined file: juce_PropertiesFile.h ***/
#ifndef __JUCE_PROPERTIESFILE_JUCEHEADER__
#define __JUCE_PROPERTIESFILE_JUCEHEADER__

class JUCE_API  PropertiesFile  : public PropertySet,
								  public ChangeBroadcaster,
								  private Timer
{
public:

	enum FileFormatOptions
	{
		ignoreCaseOfKeyNames		= 1,
		storeAsBinary		   = 2,
		storeAsCompressedBinary	 = 4,
		storeAsXML			  = 8
	};

	PropertiesFile (const File& file,
					int millisecondsBeforeSaving,
					int optionFlags);

	~PropertiesFile();

	bool isValidFile() const throw()		{ return loadedOk; }

	bool saveIfNeeded();

	bool save();

	bool needsToBeSaved() const;

	void setNeedsToBeSaved (bool needsToBeSaved);

	const File getFile() const				  { return file; }

	static PropertiesFile* createDefaultAppPropertiesFile (const String& applicationName,
														   const String& fileNameSuffix,
														   const String& folderName,
														   bool commonToAllUsers,
														   int millisecondsBeforeSaving,
														   int propertiesFileOptions);

	static const File getDefaultAppSettingsFile (const String& applicationName,
												 const String& fileNameSuffix,
												 const String& folderName,
												 bool commonToAllUsers);

	juce_UseDebuggingNewOperator

protected:
	virtual void propertyChanged();

private:

	File file;
	int timerInterval;
	const int options;
	bool loadedOk, needsWriting;

	void timerCallback();

	PropertiesFile (const PropertiesFile&);
	PropertiesFile& operator= (const PropertiesFile&);
};

#endif   // __JUCE_PROPERTIESFILE_JUCEHEADER__
/*** End of inlined file: juce_PropertiesFile.h ***/

class JUCE_API  ApplicationProperties   : public DeletedAtShutdown
{
public:

	ApplicationProperties() throw();

	~ApplicationProperties();

	juce_DeclareSingleton (ApplicationProperties, false)

	void setStorageParameters (const String& applicationName,
							   const String& fileNameSuffix,
							   const String& folderName,
							   int millisecondsBeforeSaving,
							   int propertiesFileOptions) throw();

	bool testWriteAccess (bool testUserSettings,
						  bool testCommonSettings,
						  bool showWarningDialogOnFailure);

	PropertiesFile* getUserSettings() throw();

	PropertiesFile* getCommonSettings (bool returnUserPropsIfReadOnly) throw();

	bool saveIfNeeded();

	void closeFiles();

	juce_UseDebuggingNewOperator

private:

	ScopedPointer <PropertiesFile> userProps, commonProps;

	String appName, fileSuffix, folderName;
	int msBeforeSaving, options;
	int commonSettingsAreReadOnly;

	ApplicationProperties (const ApplicationProperties&);
	ApplicationProperties& operator= (const ApplicationProperties&);

	void openFiles() throw();
};

#endif   // __JUCE_APPLICATIONPROPERTIES_JUCEHEADER__
/*** End of inlined file: juce_ApplicationProperties.h ***/


#endif
#ifndef __JUCE_AIFFAUDIOFORMAT_JUCEHEADER__

/*** Start of inlined file: juce_AiffAudioFormat.h ***/
#ifndef __JUCE_AIFFAUDIOFORMAT_JUCEHEADER__
#define __JUCE_AIFFAUDIOFORMAT_JUCEHEADER__


/*** Start of inlined file: juce_AudioFormat.h ***/
#ifndef __JUCE_AUDIOFORMAT_JUCEHEADER__
#define __JUCE_AUDIOFORMAT_JUCEHEADER__


/*** Start of inlined file: juce_AudioFormatReader.h ***/
#ifndef __JUCE_AUDIOFORMATREADER_JUCEHEADER__
#define __JUCE_AUDIOFORMATREADER_JUCEHEADER__

class AudioFormat;

class JUCE_API  AudioFormatReader
{
protected:

	AudioFormatReader (InputStream* sourceStream,
					   const String& formatName);

public:
	virtual ~AudioFormatReader();

	const String getFormatName() const throw()	  { return formatName; }

	bool read (int** destSamples,
			   int numDestChannels,
			   int64 startSampleInSource,
			   int numSamplesToRead,
			   bool fillLeftoverChannelsWithCopies);

	virtual void readMaxLevels (int64 startSample,
								int64 numSamples,
								float& lowestLeft,
								float& highestLeft,
								float& lowestRight,
								float& highestRight);

	int64 searchForLevel (int64 startSample,
						  int64 numSamplesToSearch,
						  double magnitudeRangeMinimum,
						  double magnitudeRangeMaximum,
						  int minimumConsecutiveSamples);

	double sampleRate;

	unsigned int bitsPerSample;

	int64 lengthInSamples;

	unsigned int numChannels;

	bool usesFloatingPointData;

	StringPairArray metadataValues;

	InputStream* input;

	virtual bool readSamples (int** destSamples,
							  int numDestChannels,
							  int startOffsetInDestBuffer,
							  int64 startSampleInFile,
							  int numSamples) = 0;

	juce_UseDebuggingNewOperator

private:
	String formatName;

	AudioFormatReader (const AudioFormatReader&);
	AudioFormatReader& operator= (const AudioFormatReader&);
};

#endif   // __JUCE_AUDIOFORMATREADER_JUCEHEADER__
/*** End of inlined file: juce_AudioFormatReader.h ***/


/*** Start of inlined file: juce_AudioFormatWriter.h ***/
#ifndef __JUCE_AUDIOFORMATWRITER_JUCEHEADER__
#define __JUCE_AUDIOFORMATWRITER_JUCEHEADER__


/*** Start of inlined file: juce_AudioSource.h ***/
#ifndef __JUCE_AUDIOSOURCE_JUCEHEADER__
#define __JUCE_AUDIOSOURCE_JUCEHEADER__


/*** Start of inlined file: juce_AudioSampleBuffer.h ***/
#ifndef __JUCE_AUDIOSAMPLEBUFFER_JUCEHEADER__
#define __JUCE_AUDIOSAMPLEBUFFER_JUCEHEADER__

class AudioFormatReader;
class AudioFormatWriter;

class JUCE_API  AudioSampleBuffer
{
public:

	AudioSampleBuffer (int numChannels,
					   int numSamples) throw();

	AudioSampleBuffer (float** dataToReferTo,
					   int numChannels,
					   int numSamples) throw();

	AudioSampleBuffer (const AudioSampleBuffer& other) throw();

	AudioSampleBuffer& operator= (const AudioSampleBuffer& other) throw();

	virtual ~AudioSampleBuffer() throw();

	int getNumChannels() const throw()	  { return numChannels; }

	int getNumSamples() const throw()	   { return size; }

	float* getSampleData (const int channelNumber) const throw()
	{
		jassert (((unsigned int) channelNumber) < (unsigned int) numChannels);
		return channels [channelNumber];
	}

	float* getSampleData (const int channelNumber,
						  const int sampleOffset) const throw()
	{
		jassert (((unsigned int) channelNumber) < (unsigned int) numChannels);
		jassert (((unsigned int) sampleOffset) < (unsigned int) size);
		return channels [channelNumber] + sampleOffset;
	}

	float** getArrayOfChannels() const throw()	  { return channels; }

	void setSize (int newNumChannels,
				  int newNumSamples,
				  bool keepExistingContent = false,
				  bool clearExtraSpace = false,
				  bool avoidReallocating = false) throw();

	void setDataToReferTo (float** dataToReferTo,
						   int numChannels,
						   int numSamples) throw();

	void clear() throw();

	void clear (int startSample,
				int numSamples) throw();

	void clear (int channel,
				int startSample,
				int numSamples) throw();

	void applyGain (int channel,
					int startSample,
					int numSamples,
					float gain) throw();

	void applyGain (int startSample,
					int numSamples,
					float gain) throw();

	void applyGainRamp (int channel,
						int startSample,
						int numSamples,
						float startGain,
						float endGain) throw();

	void addFrom (int destChannel,
				  int destStartSample,
				  const AudioSampleBuffer& source,
				  int sourceChannel,
				  int sourceStartSample,
				  int numSamples,
				  float gainToApplyToSource = 1.0f) throw();

	void addFrom (int destChannel,
				  int destStartSample,
				  const float* source,
				  int numSamples,
				  float gainToApplyToSource = 1.0f) throw();

	void addFromWithRamp (int destChannel,
						  int destStartSample,
						  const float* source,
						  int numSamples,
						  float startGain,
						  float endGain) throw();

	void copyFrom (int destChannel,
				   int destStartSample,
				   const AudioSampleBuffer& source,
				   int sourceChannel,
				   int sourceStartSample,
				   int numSamples) throw();

	void copyFrom (int destChannel,
				   int destStartSample,
				   const float* source,
				   int numSamples) throw();

	void copyFrom (int destChannel,
				   int destStartSample,
				   const float* source,
				   int numSamples,
				   float gain) throw();

	void copyFromWithRamp (int destChannel,
						   int destStartSample,
						   const float* source,
						   int numSamples,
						   float startGain,
						   float endGain) throw();

	void findMinMax (int channel,
					 int startSample,
					 int numSamples,
					 float& minVal,
					 float& maxVal) const throw();

	float getMagnitude (int channel,
						int startSample,
						int numSamples) const throw();

	float getMagnitude (int startSample,
						int numSamples) const throw();

	float getRMSLevel (int channel,
					   int startSample,
					   int numSamples) const throw();

	void readFromAudioReader (AudioFormatReader* reader,
							  int startSample,
							  int numSamples,
							  int readerStartSample,
							  bool useReaderLeftChan,
							  bool useReaderRightChan) throw();

	void writeToAudioWriter (AudioFormatWriter* writer,
							 int startSample,
							 int numSamples) const throw();

	juce_UseDebuggingNewOperator

private:
	int numChannels, size;
	size_t allocatedBytes;
	float** channels;
	HeapBlock <char> allocatedData;
	float* preallocatedChannelSpace [32];

	void allocateData();
	void allocateChannels (float** dataToReferTo);
};

#endif   // __JUCE_AUDIOSAMPLEBUFFER_JUCEHEADER__
/*** End of inlined file: juce_AudioSampleBuffer.h ***/

struct JUCE_API  AudioSourceChannelInfo
{
	AudioSampleBuffer* buffer;

	int startSample;

	int numSamples;

	void clearActiveBufferRegion() const
	{
		if (buffer != 0)
			buffer->clear (startSample, numSamples);
	}
};

class JUCE_API  AudioSource
{
protected:

	AudioSource() throw()	   {}

public:
	virtual ~AudioSource()	  {}

	virtual void prepareToPlay (int samplesPerBlockExpected,
								double sampleRate) = 0;

	virtual void releaseResources() = 0;

	virtual void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) = 0;
};

#endif   // __JUCE_AUDIOSOURCE_JUCEHEADER__
/*** End of inlined file: juce_AudioSource.h ***/

class JUCE_API  AudioFormatWriter
{
protected:

	AudioFormatWriter (OutputStream* destStream,
					   const String& formatName,
					   double sampleRate,
					   unsigned int numberOfChannels,
					   unsigned int bitsPerSample);

public:
	virtual ~AudioFormatWriter();

	const String getFormatName() const throw()	  { return formatName; }

	virtual bool write (const int** samplesToWrite,
						int numSamples) = 0;

	bool writeFromAudioReader (AudioFormatReader& reader,
							   int64 startSample,
							   int64 numSamplesToRead);

	bool writeFromAudioSource (AudioSource& source,
							   int numSamplesToRead,
							   int samplesPerBlock = 2048);

	double getSampleRate() const throw()	{ return sampleRate; }

	int getNumChannels() const throw()	  { return numChannels; }

	int getBitsPerSample() const throw()	{ return bitsPerSample; }

	bool isFloatingPoint() const throw()	{ return usesFloatingPointData; }

	juce_UseDebuggingNewOperator

protected:
	double sampleRate;

	unsigned int numChannels;

	unsigned int bitsPerSample;

	bool usesFloatingPointData;

	OutputStream* output;

private:
	String formatName;

	AudioFormatWriter (const AudioFormatWriter&);
	AudioFormatWriter& operator= (const AudioFormatWriter&);
};

#endif   // __JUCE_AUDIOFORMATWRITER_JUCEHEADER__
/*** End of inlined file: juce_AudioFormatWriter.h ***/

class JUCE_API  AudioFormat
{
public:

	virtual ~AudioFormat();

	const String& getFormatName() const;

	const StringArray& getFileExtensions() const;

	virtual bool canHandleFile (const File& fileToTest);

	virtual const Array <int> getPossibleSampleRates() = 0;

	virtual const Array <int> getPossibleBitDepths() = 0;

	virtual bool canDoStereo() = 0;

	virtual bool canDoMono() = 0;

	virtual bool isCompressed();

	virtual const StringArray getQualityOptions();

	virtual AudioFormatReader* createReaderFor (InputStream* sourceStream,
												const bool deleteStreamIfOpeningFails) = 0;

	virtual AudioFormatWriter* createWriterFor (OutputStream* streamToWriteTo,
												double sampleRateToUse,
												unsigned int numberOfChannels,
												int bitsPerSample,
												const StringPairArray& metadataValues,
												int qualityOptionIndex) = 0;

protected:
	AudioFormat (const String& formatName,
				 const tchar** const fileExtensions);

private:

	String formatName;
	StringArray fileExtensions;
};

#endif   // __JUCE_AUDIOFORMAT_JUCEHEADER__
/*** End of inlined file: juce_AudioFormat.h ***/

class JUCE_API  AiffAudioFormat  : public AudioFormat
{
public:

	AiffAudioFormat();

	~AiffAudioFormat();

	const Array <int> getPossibleSampleRates();
	const Array <int> getPossibleBitDepths();
	bool canDoStereo();
	bool canDoMono();
#if JUCE_MAC
	bool canHandleFile (const File& fileToTest);
#endif

	AudioFormatReader* createReaderFor (InputStream* sourceStream,
										const bool deleteStreamIfOpeningFails);

	AudioFormatWriter* createWriterFor (OutputStream* streamToWriteTo,
										double sampleRateToUse,
										unsigned int numberOfChannels,
										int bitsPerSample,
										const StringPairArray& metadataValues,
										int qualityOptionIndex);

	juce_UseDebuggingNewOperator
};

#endif   // __JUCE_AIFFAUDIOFORMAT_JUCEHEADER__
/*** End of inlined file: juce_AiffAudioFormat.h ***/


#endif
#ifndef __JUCE_AUDIOCDBURNER_JUCEHEADER__

/*** Start of inlined file: juce_AudioCDBurner.h ***/
#ifndef __JUCE_AUDIOCDBURNER_JUCEHEADER__
#define __JUCE_AUDIOCDBURNER_JUCEHEADER__

#if JUCE_USE_CDBURNER

class AudioCDBurner	 : public ChangeBroadcaster
{
public:

	static const StringArray findAvailableDevices();

	static AudioCDBurner* openDevice (const int deviceIndex);

	~AudioCDBurner();

	enum DiskState
	{
		unknown,		/**< An error condition, if the device isn't responding. */
		trayOpen,		   /**< The drive is currently open. Note that a slot-loading drive
									 may seem to be permanently open. */
		noDisc,		 /**< The drive has no disk in it. */
		writableDiskPresent,	/**< The drive contains a writeable disk. */
		readOnlyDiskPresent	 /**< The drive contains a read-only disk. */
	};

	DiskState getDiskState() const;

	bool isDiskPresent() const;

	bool openTray();

	DiskState waitUntilStateChange (int timeOutMilliseconds);

	const Array<int> getAvailableWriteSpeeds() const;

	bool setBufferUnderrunProtection (const bool shouldBeEnabled);

	int getNumAvailableAudioBlocks() const;

	bool addAudioTrack (AudioSource* source, int numSamples);

	class BurnProgressListener
	{
	public:
		BurnProgressListener() throw() {}
		virtual ~BurnProgressListener() {}

		virtual bool audioCDBurnProgress (float proportionComplete) = 0;
	};

	const String burn (BurnProgressListener* listener,
					   bool ejectDiscAfterwards,
					   bool performFakeBurnForTesting,
					   int writeSpeed);

	void abortBurn();

	juce_UseDebuggingNewOperator

private:
	AudioCDBurner (const int deviceIndex);

	class Pimpl;
	friend class ScopedPointer<Pimpl>;
	ScopedPointer<Pimpl> pimpl;
};

#endif
#endif   // __JUCE_AUDIOCDBURNER_JUCEHEADER__
/*** End of inlined file: juce_AudioCDBurner.h ***/


#endif
#ifndef __JUCE_AUDIOCDREADER_JUCEHEADER__

/*** Start of inlined file: juce_AudioCDReader.h ***/
#ifndef __JUCE_AUDIOCDREADER_JUCEHEADER__
#define __JUCE_AUDIOCDREADER_JUCEHEADER__

#if JUCE_USE_CDREADER

#if JUCE_MAC

#endif

class JUCE_API  AudioCDReader  : public AudioFormatReader
{
public:

	static const StringArray getAvailableCDNames();

	static AudioCDReader* createReaderForCD (const int index);

	~AudioCDReader();

	bool readSamples (int** destSamples, int numDestChannels, int startOffsetInDestBuffer,
					  int64 startSampleInFile, int numSamples);

	bool isCDStillPresent() const;

	int getNumTracks() const;

	int getPositionOfTrackStart (int trackNum) const;

	bool isTrackAudio (int trackNum) const;

	void refreshTrackLengths();

	void enableIndexScanning (bool enabled);

	int getLastIndex() const;

	const Array <int> findIndexesInTrack (const int trackNumber);

	int getCDDBId();

	void ejectDisk();

	juce_UseDebuggingNewOperator

private:

#if JUCE_MAC
	File volumeDir;
	Array<File> tracks;
	Array<int> trackStartSamples;
	int currentReaderTrack;
	ScopedPointer <AudioFormatReader> reader;
	AudioCDReader (const File& volume);
public:
	static int compareElements (const File&, const File&);
private:

#elif JUCE_WINDOWS
	int numTracks;
	int trackStarts[100];
	bool audioTracks [100];
	void* handle;
	bool indexingEnabled;
	int lastIndex, firstFrameInBuffer, samplesInBuffer;
	MemoryBlock buffer;
	AudioCDReader (void* handle);
	int getIndexAt (int samplePos);

#elif JUCE_LINUX
	AudioCDReader();
#endif

	AudioCDReader (const AudioCDReader&);
	AudioCDReader& operator= (const AudioCDReader&);
};

#endif
#endif   // __JUCE_AUDIOCDREADER_JUCEHEADER__
/*** End of inlined file: juce_AudioCDReader.h ***/


#endif
#ifndef __JUCE_AUDIOFORMAT_JUCEHEADER__

#endif
#ifndef __JUCE_AUDIOFORMATMANAGER_JUCEHEADER__

/*** Start of inlined file: juce_AudioFormatManager.h ***/
#ifndef __JUCE_AUDIOFORMATMANAGER_JUCEHEADER__
#define __JUCE_AUDIOFORMATMANAGER_JUCEHEADER__

class JUCE_API  AudioFormatManager
{
public:

	AudioFormatManager();

	~AudioFormatManager();

	juce_DeclareSingleton (AudioFormatManager, false);

	void registerFormat (AudioFormat* newFormat,
						 bool makeThisTheDefaultFormat);

	void registerBasicFormats();

	void clearFormats();

	int getNumKnownFormats() const;

	AudioFormat* getKnownFormat (int index) const;

	AudioFormat* findFormatForFileExtension (const String& fileExtension) const;

	AudioFormat* getDefaultFormat() const;

	const String getWildcardForAllFormats() const;

	AudioFormatReader* createReaderFor (const File& audioFile);

	AudioFormatReader* createReaderFor (InputStream* audioFileStream);

	juce_UseDebuggingNewOperator

private:
	VoidArray knownFormats;
	int defaultFormatIndex;
};

#endif   // __JUCE_AUDIOFORMATMANAGER_JUCEHEADER__
/*** End of inlined file: juce_AudioFormatManager.h ***/


#endif
#ifndef __JUCE_AUDIOFORMATREADER_JUCEHEADER__

#endif
#ifndef __JUCE_AUDIOFORMATWRITER_JUCEHEADER__

#endif
#ifndef __JUCE_AUDIOSUBSECTIONREADER_JUCEHEADER__

/*** Start of inlined file: juce_AudioSubsectionReader.h ***/
#ifndef __JUCE_AUDIOSUBSECTIONREADER_JUCEHEADER__
#define __JUCE_AUDIOSUBSECTIONREADER_JUCEHEADER__

class JUCE_API  AudioSubsectionReader  : public AudioFormatReader
{
public:

	AudioSubsectionReader (AudioFormatReader* sourceReader,
						   int64 subsectionStartSample,
						   int64 subsectionLength,
						   bool deleteSourceWhenDeleted);

	~AudioSubsectionReader();

	bool readSamples (int** destSamples, int numDestChannels, int startOffsetInDestBuffer,
					  int64 startSampleInFile, int numSamples);

	void readMaxLevels (int64 startSample,
						int64 numSamples,
						float& lowestLeft,
						float& highestLeft,
						float& lowestRight,
						float& highestRight);

	juce_UseDebuggingNewOperator

private:
	AudioFormatReader* const source;
	int64 startSample, length;
	const bool deleteSourceWhenDeleted;

	AudioSubsectionReader (const AudioSubsectionReader&);
	AudioSubsectionReader& operator= (const AudioSubsectionReader&);
};

#endif   // __JUCE_AUDIOSUBSECTIONREADER_JUCEHEADER__
/*** End of inlined file: juce_AudioSubsectionReader.h ***/


#endif
#ifndef __JUCE_AUDIOTHUMBNAIL_JUCEHEADER__

/*** Start of inlined file: juce_AudioThumbnail.h ***/
#ifndef __JUCE_AUDIOTHUMBNAIL_JUCEHEADER__
#define __JUCE_AUDIOTHUMBNAIL_JUCEHEADER__

class AudioThumbnailCache;

class JUCE_API  AudioThumbnail	: public ChangeBroadcaster,
									public TimeSliceClient,
									private Timer
{
public:

	AudioThumbnail (int sourceSamplesPerThumbnailSample,
					AudioFormatManager& formatManagerToUse,
					AudioThumbnailCache& cacheToUse);

	~AudioThumbnail();

	void setSource (InputSource* newSource);

	void loadFrom (InputStream& input);

	void saveTo (OutputStream& output) const;

	int getNumChannels() const throw();

	double getTotalLength() const throw();

	void drawChannel (Graphics& g,
					  int x, int y, int w, int h,
					  double startTimeSeconds,
					  double endTimeSeconds,
					  int channelNum,
					  float verticalZoomFactor);

	bool isFullyLoaded() const throw();

	bool useTimeSlice();
	void timerCallback();

	juce_UseDebuggingNewOperator

private:
	AudioFormatManager& formatManagerToUse;
	AudioThumbnailCache& cache;
	ScopedPointer <InputSource> source;

	CriticalSection readerLock;
	ScopedPointer <AudioFormatReader> reader;

	MemoryBlock data, cachedLevels;
	int orginalSamplesPerThumbnailSample;

	int numChannelsCached, numSamplesCached;
	double cachedStart, cachedTimePerPixel;
	bool cacheNeedsRefilling;

	void clear();
	AudioFormatReader* createReader() const;
	void generateSection (AudioFormatReader& reader, int64 startSample, int numSamples);
	char* getChannelData (int channel) const;
	void refillCache (int numSamples, double startTime, double timePerPixel);

	friend class AudioThumbnailCache;

	// true if it needs more callbacks from the readNextBlockFromAudioFile() method
	bool initialiseFromAudioFile (AudioFormatReader& reader);

	// returns true if more needs to be read
	bool readNextBlockFromAudioFile (AudioFormatReader& reader);
};

#endif   // __JUCE_AUDIOTHUMBNAIL_JUCEHEADER__
/*** End of inlined file: juce_AudioThumbnail.h ***/


#endif
#ifndef __JUCE_AUDIOTHUMBNAILCACHE_JUCEHEADER__

/*** Start of inlined file: juce_AudioThumbnailCache.h ***/
#ifndef __JUCE_AUDIOTHUMBNAILCACHE_JUCEHEADER__
#define __JUCE_AUDIOTHUMBNAILCACHE_JUCEHEADER__

struct ThumbnailCacheEntry;

class JUCE_API  AudioThumbnailCache   : public TimeSliceThread
{
public:

	explicit AudioThumbnailCache (int maxNumThumbsToStore);

	~AudioThumbnailCache();

	void clear();

	bool loadThumb (AudioThumbnail& thumb, int64 hashCode);

	void storeThumb (const AudioThumbnail& thumb, int64 hashCode);

	juce_UseDebuggingNewOperator

private:

	OwnedArray <ThumbnailCacheEntry> thumbs;
	int maxNumThumbsToStore;

	friend class AudioThumbnail;
	void addThumbnail (AudioThumbnail* thumb);
	void removeThumbnail (AudioThumbnail* thumb);
};

#endif   // __JUCE_AUDIOTHUMBNAILCACHE_JUCEHEADER__
/*** End of inlined file: juce_AudioThumbnailCache.h ***/


#endif
#ifndef __JUCE_FLACAUDIOFORMAT_JUCEHEADER__

/*** Start of inlined file: juce_FlacAudioFormat.h ***/
#ifndef __JUCE_FLACAUDIOFORMAT_JUCEHEADER__
#define __JUCE_FLACAUDIOFORMAT_JUCEHEADER__

#if JUCE_USE_FLAC || defined (DOXYGEN)

class JUCE_API  FlacAudioFormat	: public AudioFormat
{
public:

	FlacAudioFormat();
	~FlacAudioFormat();

	const Array <int> getPossibleSampleRates();
	const Array <int> getPossibleBitDepths();
	bool canDoStereo();
	bool canDoMono();
	bool isCompressed();

	AudioFormatReader* createReaderFor (InputStream* sourceStream,
										const bool deleteStreamIfOpeningFails);

	AudioFormatWriter* createWriterFor (OutputStream* streamToWriteTo,
										double sampleRateToUse,
										unsigned int numberOfChannels,
										int bitsPerSample,
										const StringPairArray& metadataValues,
										int qualityOptionIndex);

	juce_UseDebuggingNewOperator
};

#endif
#endif   // __JUCE_FLACAUDIOFORMAT_JUCEHEADER__
/*** End of inlined file: juce_FlacAudioFormat.h ***/


#endif
#ifndef __JUCE_OGGVORBISAUDIOFORMAT_JUCEHEADER__

/*** Start of inlined file: juce_OggVorbisAudioFormat.h ***/
#ifndef __JUCE_OGGVORBISAUDIOFORMAT_JUCEHEADER__
#define __JUCE_OGGVORBISAUDIOFORMAT_JUCEHEADER__

#if JUCE_USE_OGGVORBIS || defined (DOXYGEN)

class JUCE_API  OggVorbisAudioFormat : public AudioFormat
{
public:

	OggVorbisAudioFormat();
	~OggVorbisAudioFormat();

	const Array <int> getPossibleSampleRates();
	const Array <int> getPossibleBitDepths();
	bool canDoStereo();
	bool canDoMono();
	bool isCompressed();
	const StringArray getQualityOptions();

	int estimateOggFileQuality (const File& source);

	AudioFormatReader* createReaderFor (InputStream* sourceStream,
										const bool deleteStreamIfOpeningFails);

	AudioFormatWriter* createWriterFor (OutputStream* streamToWriteTo,
										double sampleRateToUse,
										unsigned int numberOfChannels,
										int bitsPerSample,
										const StringPairArray& metadataValues,
										int qualityOptionIndex);

	juce_UseDebuggingNewOperator
};

#endif
#endif   // __JUCE_OGGVORBISAUDIOFORMAT_JUCEHEADER__
/*** End of inlined file: juce_OggVorbisAudioFormat.h ***/


#endif
#ifndef __JUCE_QUICKTIMEAUDIOFORMAT_JUCEHEADER__

/*** Start of inlined file: juce_QuickTimeAudioFormat.h ***/
#ifndef __JUCE_QUICKTIMEAUDIOFORMAT_JUCEHEADER__
#define __JUCE_QUICKTIMEAUDIOFORMAT_JUCEHEADER__

#if JUCE_QUICKTIME

class JUCE_API  QuickTimeAudioFormat  : public AudioFormat
{
public:

	QuickTimeAudioFormat();

	~QuickTimeAudioFormat();

	const Array <int> getPossibleSampleRates();
	const Array <int> getPossibleBitDepths();
	bool canDoStereo();
	bool canDoMono();

	AudioFormatReader* createReaderFor (InputStream* sourceStream,
										const bool deleteStreamIfOpeningFails);

	AudioFormatWriter* createWriterFor (OutputStream* streamToWriteTo,
										double sampleRateToUse,
										unsigned int numberOfChannels,
										int bitsPerSample,
										const StringPairArray& metadataValues,
										int qualityOptionIndex);

	juce_UseDebuggingNewOperator
};

#endif
#endif   // __JUCE_QUICKTIMEAUDIOFORMAT_JUCEHEADER__
/*** End of inlined file: juce_QuickTimeAudioFormat.h ***/


#endif
#ifndef __JUCE_WAVAUDIOFORMAT_JUCEHEADER__

/*** Start of inlined file: juce_WavAudioFormat.h ***/
#ifndef __JUCE_WAVAUDIOFORMAT_JUCEHEADER__
#define __JUCE_WAVAUDIOFORMAT_JUCEHEADER__

class JUCE_API  WavAudioFormat  : public AudioFormat
{
public:

	WavAudioFormat();

	~WavAudioFormat();

	static const tchar* const bwavDescription;

	static const tchar* const bwavOriginator;

	static const tchar* const bwavOriginatorRef;

	static const tchar* const bwavOriginationDate;

	static const tchar* const bwavOriginationTime;

	static const tchar* const bwavTimeReference;

	static const tchar* const bwavCodingHistory;

	static const StringPairArray createBWAVMetadata (const String& description,
													 const String& originator,
													 const String& originatorRef,
													 const Time& dateAndTime,
													 const int64 timeReferenceSamples,
													 const String& codingHistory);

	const Array <int> getPossibleSampleRates();
	const Array <int> getPossibleBitDepths();
	bool canDoStereo();
	bool canDoMono();

	AudioFormatReader* createReaderFor (InputStream* sourceStream,
										const bool deleteStreamIfOpeningFails);

	AudioFormatWriter* createWriterFor (OutputStream* streamToWriteTo,
										double sampleRateToUse,
										unsigned int numberOfChannels,
										int bitsPerSample,
										const StringPairArray& metadataValues,
										int qualityOptionIndex);

	bool replaceMetadataInFile (const File& wavFile, const StringPairArray& newMetadata);

	juce_UseDebuggingNewOperator
};

#endif   // __JUCE_WAVAUDIOFORMAT_JUCEHEADER__
/*** End of inlined file: juce_WavAudioFormat.h ***/


#endif
#ifndef __JUCE_AUDIOFORMATREADERSOURCE_JUCEHEADER__

/*** Start of inlined file: juce_AudioFormatReaderSource.h ***/
#ifndef __JUCE_AUDIOFORMATREADERSOURCE_JUCEHEADER__
#define __JUCE_AUDIOFORMATREADERSOURCE_JUCEHEADER__


/*** Start of inlined file: juce_PositionableAudioSource.h ***/
#ifndef __JUCE_POSITIONABLEAUDIOSOURCE_JUCEHEADER__
#define __JUCE_POSITIONABLEAUDIOSOURCE_JUCEHEADER__

class JUCE_API  PositionableAudioSource  : public AudioSource
{
protected:

	PositionableAudioSource() throw()   {}

public:
	~PositionableAudioSource()	  {}

	virtual void setNextReadPosition (int newPosition) = 0;

	virtual int getNextReadPosition() const = 0;

	virtual int getTotalLength() const = 0;

	virtual bool isLooping() const = 0;
};

#endif   // __JUCE_POSITIONABLEAUDIOSOURCE_JUCEHEADER__
/*** End of inlined file: juce_PositionableAudioSource.h ***/

class JUCE_API  AudioFormatReaderSource  : public PositionableAudioSource
{
public:

	AudioFormatReaderSource (AudioFormatReader* const sourceReader,
							 const bool deleteReaderWhenThisIsDeleted);

	~AudioFormatReaderSource();

	void setLooping (const bool shouldLoop) throw();

	bool isLooping() const					  { return looping; }

	AudioFormatReader* getAudioFormatReader() const throw()	 { return reader; }

	void prepareToPlay (int samplesPerBlockExpected, double sampleRate);

	void releaseResources();

	void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill);

	void setNextReadPosition (int newPosition);

	int getNextReadPosition() const;

	int getTotalLength() const;

	juce_UseDebuggingNewOperator

private:
	AudioFormatReader* reader;
	bool deleteReader;

	int volatile nextPlayPos;
	bool volatile looping;

	void readBufferSection (int start, int length, AudioSampleBuffer& buffer, int startSample);

	AudioFormatReaderSource (const AudioFormatReaderSource&);
	AudioFormatReaderSource& operator= (const AudioFormatReaderSource&);
};

#endif   // __JUCE_AUDIOFORMATREADERSOURCE_JUCEHEADER__
/*** End of inlined file: juce_AudioFormatReaderSource.h ***/


#endif
#ifndef __JUCE_AUDIOSOURCE_JUCEHEADER__

#endif
#ifndef __JUCE_AUDIOSOURCEPLAYER_JUCEHEADER__

/*** Start of inlined file: juce_AudioSourcePlayer.h ***/
#ifndef __JUCE_AUDIOSOURCEPLAYER_JUCEHEADER__
#define __JUCE_AUDIOSOURCEPLAYER_JUCEHEADER__


/*** Start of inlined file: juce_AudioIODevice.h ***/
#ifndef __JUCE_AUDIOIODEVICE_JUCEHEADER__
#define __JUCE_AUDIOIODEVICE_JUCEHEADER__

class AudioIODevice;

class JUCE_API  AudioIODeviceCallback
{
public:
	virtual ~AudioIODeviceCallback()  {}

	virtual void audioDeviceIOCallback (const float** inputChannelData,
										int numInputChannels,
										float** outputChannelData,
										int numOutputChannels,
										int numSamples) = 0;

	virtual void audioDeviceAboutToStart (AudioIODevice* device) = 0;

	virtual void audioDeviceStopped() = 0;
};

class JUCE_API  AudioIODevice
{
public:
	virtual ~AudioIODevice();

	const String& getName() const throw()			   { return name; }

	const String& getTypeName() const throw()			   { return typeName; }

	virtual const StringArray getOutputChannelNames() = 0;

	virtual const StringArray getInputChannelNames() = 0;

	virtual int getNumSampleRates() = 0;

	virtual double getSampleRate (int index) = 0;

	virtual int getNumBufferSizesAvailable() = 0;

	virtual int getBufferSizeSamples (int index) = 0;

	virtual int getDefaultBufferSize() = 0;

	virtual const String open (const BigInteger& inputChannels,
							   const BigInteger& outputChannels,
							   double sampleRate,
							   int bufferSizeSamples) = 0;

	virtual void close() = 0;

	virtual bool isOpen() = 0;

	virtual void start (AudioIODeviceCallback* callback) = 0;

	virtual void stop() = 0;

	virtual bool isPlaying() = 0;

	virtual const String getLastError() = 0;

	virtual int getCurrentBufferSizeSamples() = 0;

	virtual double getCurrentSampleRate() = 0;

	virtual int getCurrentBitDepth() = 0;

	virtual const BigInteger getActiveOutputChannels() const = 0;

	virtual const BigInteger getActiveInputChannels() const = 0;

	virtual int getOutputLatencyInSamples() = 0;

	virtual int getInputLatencyInSamples() = 0;

	virtual bool hasControlPanel() const;

	virtual bool showControlPanel();

protected:
	AudioIODevice (const String& deviceName,
				   const String& typeName);

	String name, typeName;
};

#endif   // __JUCE_AUDIOIODEVICE_JUCEHEADER__
/*** End of inlined file: juce_AudioIODevice.h ***/

class JUCE_API  AudioSourcePlayer  : public AudioIODeviceCallback
{
public:

	AudioSourcePlayer();

	virtual ~AudioSourcePlayer();

	void setSource (AudioSource* newSource);

	AudioSource* getCurrentSource() const throw()	   { return source; }

	void setGain (const float newGain) throw();

	void audioDeviceIOCallback (const float** inputChannelData,
								int totalNumInputChannels,
								float** outputChannelData,
								int totalNumOutputChannels,
								int numSamples);

	void audioDeviceAboutToStart (AudioIODevice* device);

	void audioDeviceStopped();

	juce_UseDebuggingNewOperator

private:

	CriticalSection readLock;
	AudioSource* source;
	double sampleRate;
	int bufferSize;
	float* channels [128];
	float* outputChans [128];
	const float* inputChans [128];
	AudioSampleBuffer tempBuffer;
	float lastGain, gain;

	AudioSourcePlayer (const AudioSourcePlayer&);
	AudioSourcePlayer& operator= (const AudioSourcePlayer&);
};

#endif   // __JUCE_AUDIOSOURCEPLAYER_JUCEHEADER__
/*** End of inlined file: juce_AudioSourcePlayer.h ***/


#endif
#ifndef __JUCE_AUDIOTRANSPORTSOURCE_JUCEHEADER__

/*** Start of inlined file: juce_AudioTransportSource.h ***/
#ifndef __JUCE_AUDIOTRANSPORTSOURCE_JUCEHEADER__
#define __JUCE_AUDIOTRANSPORTSOURCE_JUCEHEADER__


/*** Start of inlined file: juce_BufferingAudioSource.h ***/
#ifndef __JUCE_BUFFERINGAUDIOSOURCE_JUCEHEADER__
#define __JUCE_BUFFERINGAUDIOSOURCE_JUCEHEADER__

class JUCE_API  BufferingAudioSource  : public PositionableAudioSource
{
public:

	BufferingAudioSource (PositionableAudioSource* source,
						  const bool deleteSourceWhenDeleted,
						  int numberOfSamplesToBuffer);

	~BufferingAudioSource();

	void prepareToPlay (int samplesPerBlockExpected, double sampleRate);

	void releaseResources();

	void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill);

	void setNextReadPosition (int newPosition);

	int getNextReadPosition() const;

	int getTotalLength() const		  { return source->getTotalLength(); }

	bool isLooping() const			  { return source->isLooping(); }

	juce_UseDebuggingNewOperator

private:

	PositionableAudioSource* source;
	bool deleteSourceWhenDeleted;
	int numberOfSamplesToBuffer;
	AudioSampleBuffer buffer;
	CriticalSection bufferStartPosLock;
	int volatile bufferValidStart, bufferValidEnd, nextPlayPos;
	bool wasSourceLooping;
	double volatile sampleRate;

	friend class SharedBufferingAudioSourceThread;
	bool readNextBufferChunk();
	void readBufferSection (int start, int length, int bufferOffset);

	BufferingAudioSource (const BufferingAudioSource&);
	BufferingAudioSource& operator= (const BufferingAudioSource&);
};

#endif   // __JUCE_BUFFERINGAUDIOSOURCE_JUCEHEADER__
/*** End of inlined file: juce_BufferingAudioSource.h ***/


/*** Start of inlined file: juce_ResamplingAudioSource.h ***/
#ifndef __JUCE_RESAMPLINGAUDIOSOURCE_JUCEHEADER__
#define __JUCE_RESAMPLINGAUDIOSOURCE_JUCEHEADER__

class JUCE_API  ResamplingAudioSource  : public AudioSource
{
public:

	ResamplingAudioSource (AudioSource* const inputSource,
						   const bool deleteInputWhenDeleted);

	~ResamplingAudioSource();

	void setResamplingRatio (const double samplesInPerOutputSample);

	double getResamplingRatio() const throw()		   { return ratio; }

	void prepareToPlay (int samplesPerBlockExpected, double sampleRate);
	void releaseResources();
	void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill);

	juce_UseDebuggingNewOperator

private:
	AudioSource* const input;
	const bool deleteInputWhenDeleted;
	double ratio, lastRatio;
	AudioSampleBuffer buffer;
	int bufferPos, sampsInBuffer;
	double subSampleOffset;
	double coefficients[6];
	CriticalSection ratioLock;

	void setFilterCoefficients (double c1, double c2, double c3, double c4, double c5, double c6);
	void createLowPass (const double proportionalRate);

	struct FilterState
	{
		double x1, x2, y1, y2;
	};

	FilterState filterStates[2];
	void resetFilters();

	void applyFilter (float* samples, int num, FilterState& fs);

	ResamplingAudioSource (const ResamplingAudioSource&);
	ResamplingAudioSource& operator= (const ResamplingAudioSource&);
};

#endif   // __JUCE_RESAMPLINGAUDIOSOURCE_JUCEHEADER__
/*** End of inlined file: juce_ResamplingAudioSource.h ***/

class JUCE_API  AudioTransportSource  : public PositionableAudioSource,
										public ChangeBroadcaster
{
public:

	AudioTransportSource();

	~AudioTransportSource();

	void setSource (PositionableAudioSource* const newSource,
					int readAheadBufferSize = 0,
					double sourceSampleRateToCorrectFor = 0.0);

	void setPosition (double newPosition);

	double getCurrentPosition() const;

	bool hasStreamFinished() const throw()		  { return inputStreamEOF; }

	void start();

	void stop();

	bool isPlaying() const throw()	  { return playing; }

	void setGain (const float newGain) throw();

	float getGain() const throw()	   { return gain; }

	void prepareToPlay (int samplesPerBlockExpected, double sampleRate);

	void releaseResources();

	void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill);

	void setNextReadPosition (int newPosition);

	int getNextReadPosition() const;

	int getTotalLength() const;

	bool isLooping() const;

	juce_UseDebuggingNewOperator

private:
	PositionableAudioSource* source;
	ResamplingAudioSource* resamplerSource;
	BufferingAudioSource* bufferingSource;
	PositionableAudioSource* positionableSource;
	AudioSource* masterSource;

	CriticalSection callbackLock;
	float volatile gain, lastGain;
	bool volatile playing, stopped;
	double sampleRate, sourceSampleRate;
	int blockSize, readAheadBufferSize;
	bool isPrepared, inputStreamEOF;

	AudioTransportSource (const AudioTransportSource&);
	AudioTransportSource& operator= (const AudioTransportSource&);
};

#endif   // __JUCE_AUDIOTRANSPORTSOURCE_JUCEHEADER__
/*** End of inlined file: juce_AudioTransportSource.h ***/


#endif
#ifndef __JUCE_BUFFERINGAUDIOSOURCE_JUCEHEADER__

#endif
#ifndef __JUCE_CHANNELREMAPPINGAUDIOSOURCE_JUCEHEADER__

/*** Start of inlined file: juce_ChannelRemappingAudioSource.h ***/
#ifndef __JUCE_CHANNELREMAPPINGAUDIOSOURCE_JUCEHEADER__
#define __JUCE_CHANNELREMAPPINGAUDIOSOURCE_JUCEHEADER__

class ChannelRemappingAudioSource  : public AudioSource
{
public:

	ChannelRemappingAudioSource (AudioSource* const source,
								 const bool deleteSourceWhenDeleted);

	~ChannelRemappingAudioSource();

	void setNumberOfChannelsToProduce (const int requiredNumberOfChannels) throw();

	void clearAllMappings() throw();

	void setInputChannelMapping (const int destChannelIndex,
								 const int sourceChannelIndex) throw();

	void setOutputChannelMapping (const int sourceChannelIndex,
								  const int destChannelIndex) throw();

	int getRemappedInputChannel (const int inputChannelIndex) const throw();

	int getRemappedOutputChannel (const int outputChannelIndex) const throw();

	XmlElement* createXml() const throw();

	void restoreFromXml (const XmlElement& e) throw();

	void prepareToPlay (int samplesPerBlockExpected, double sampleRate);
	void releaseResources();
	void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill);

	juce_UseDebuggingNewOperator

private:
	int requiredNumberOfChannels;
	Array <int> remappedInputs, remappedOutputs;

	AudioSource* const source;
	const bool deleteSourceWhenDeleted;

	AudioSampleBuffer buffer;
	AudioSourceChannelInfo remappedInfo;

	CriticalSection lock;

	ChannelRemappingAudioSource (const ChannelRemappingAudioSource&);
	ChannelRemappingAudioSource& operator= (const ChannelRemappingAudioSource&);
};

#endif   // __JUCE_CHANNELREMAPPINGAUDIOSOURCE_JUCEHEADER__
/*** End of inlined file: juce_ChannelRemappingAudioSource.h ***/


#endif
#ifndef __JUCE_IIRFILTERAUDIOSOURCE_JUCEHEADER__

/*** Start of inlined file: juce_IIRFilterAudioSource.h ***/
#ifndef __JUCE_IIRFILTERAUDIOSOURCE_JUCEHEADER__
#define __JUCE_IIRFILTERAUDIOSOURCE_JUCEHEADER__


/*** Start of inlined file: juce_IIRFilter.h ***/
#ifndef __JUCE_IIRFILTER_JUCEHEADER__
#define __JUCE_IIRFILTER_JUCEHEADER__

class JUCE_API  IIRFilter
{
public:

	IIRFilter();

	IIRFilter (const IIRFilter& other);

	~IIRFilter();

	void reset() throw();

	void processSamples (float* samples,
						 int numSamples) throw();

	float processSingleSampleRaw (float sample) throw();

	void makeLowPass (double sampleRate,
					  double frequency) throw();

	void makeHighPass (double sampleRate,
					   double frequency) throw();

	void makeLowShelf (double sampleRate,
					   double cutOffFrequency,
					   double Q,
					   float gainFactor) throw();

	void makeHighShelf (double sampleRate,
						double cutOffFrequency,
						double Q,
						float gainFactor) throw();

	void makeBandPass (double sampleRate,
					   double centreFrequency,
					   double Q,
					   float gainFactor) throw();

	void makeInactive() throw();

	void copyCoefficientsFrom (const IIRFilter& other) throw();

	juce_UseDebuggingNewOperator

protected:
	CriticalSection processLock;

	void setCoefficients (double c1, double c2, double c3,
						  double c4, double c5, double c6) throw();

	bool active;
	float coefficients[6];
	float x1, x2, y1, y2;

	// (use the copyCoefficientsFrom() method instead of this operator)
	IIRFilter& operator= (const IIRFilter&);
};

#endif   // __JUCE_IIRFILTER_JUCEHEADER__
/*** End of inlined file: juce_IIRFilter.h ***/

class JUCE_API  IIRFilterAudioSource  : public AudioSource
{
public:

	IIRFilterAudioSource (AudioSource* const inputSource,
						  const bool deleteInputWhenDeleted);

	~IIRFilterAudioSource();

	void setFilterParameters (const IIRFilter& newSettings);

	void prepareToPlay (int samplesPerBlockExpected, double sampleRate);
	void releaseResources();
	void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill);

	juce_UseDebuggingNewOperator

private:

	AudioSource* const input;
	const bool deleteInputWhenDeleted;
	OwnedArray <IIRFilter> iirFilters;

	IIRFilterAudioSource (const IIRFilterAudioSource&);
	IIRFilterAudioSource& operator= (const IIRFilterAudioSource&);
};

#endif   // __JUCE_IIRFILTERAUDIOSOURCE_JUCEHEADER__
/*** End of inlined file: juce_IIRFilterAudioSource.h ***/


#endif
#ifndef __JUCE_MIXERAUDIOSOURCE_JUCEHEADER__

/*** Start of inlined file: juce_MixerAudioSource.h ***/
#ifndef __JUCE_MIXERAUDIOSOURCE_JUCEHEADER__
#define __JUCE_MIXERAUDIOSOURCE_JUCEHEADER__

class JUCE_API  MixerAudioSource  : public AudioSource
{
public:

	MixerAudioSource();

	~MixerAudioSource();

	void addInputSource (AudioSource* newInput,
						 const bool deleteWhenRemoved);

	void removeInputSource (AudioSource* input,
							const bool deleteSource);

	void removeAllInputs();

	void prepareToPlay (int samplesPerBlockExpected, double sampleRate);

	void releaseResources();

	void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill);

	juce_UseDebuggingNewOperator

private:

	VoidArray inputs;
	BigInteger inputsToDelete;
	CriticalSection lock;
	AudioSampleBuffer tempBuffer;
	double currentSampleRate;
	int bufferSizeExpected;

	MixerAudioSource (const MixerAudioSource&);
	MixerAudioSource& operator= (const MixerAudioSource&);
};

#endif   // __JUCE_MIXERAUDIOSOURCE_JUCEHEADER__
/*** End of inlined file: juce_MixerAudioSource.h ***/


#endif
#ifndef __JUCE_POSITIONABLEAUDIOSOURCE_JUCEHEADER__

#endif
#ifndef __JUCE_RESAMPLINGAUDIOSOURCE_JUCEHEADER__

#endif
#ifndef __JUCE_TONEGENERATORAUDIOSOURCE_JUCEHEADER__

/*** Start of inlined file: juce_ToneGeneratorAudioSource.h ***/
#ifndef __JUCE_TONEGENERATORAUDIOSOURCE_JUCEHEADER__
#define __JUCE_TONEGENERATORAUDIOSOURCE_JUCEHEADER__

class JUCE_API  ToneGeneratorAudioSource  : public AudioSource
{
public:

	ToneGeneratorAudioSource();

	~ToneGeneratorAudioSource();

	void setAmplitude (const float newAmplitude);

	void setFrequency (const double newFrequencyHz);

	void prepareToPlay (int samplesPerBlockExpected, double sampleRate);

	void releaseResources();

	void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill);

	juce_UseDebuggingNewOperator

private:

	double frequency, sampleRate;
	double currentPhase, phasePerSample;
	float amplitude;

	ToneGeneratorAudioSource (const ToneGeneratorAudioSource&);
	ToneGeneratorAudioSource& operator= (const ToneGeneratorAudioSource&);
};

#endif   // __JUCE_TONEGENERATORAUDIOSOURCE_JUCEHEADER__
/*** End of inlined file: juce_ToneGeneratorAudioSource.h ***/


#endif
#ifndef __JUCE_AUDIODEVICEMANAGER_JUCEHEADER__

/*** Start of inlined file: juce_AudioDeviceManager.h ***/
#ifndef __JUCE_AUDIODEVICEMANAGER_JUCEHEADER__
#define __JUCE_AUDIODEVICEMANAGER_JUCEHEADER__


/*** Start of inlined file: juce_AudioIODeviceType.h ***/
#ifndef __JUCE_AUDIOIODEVICETYPE_JUCEHEADER__
#define __JUCE_AUDIOIODEVICETYPE_JUCEHEADER__

class AudioDeviceManager;
class Component;

class JUCE_API  AudioIODeviceType
{
public:

	const String& getTypeName() const throw()			   { return typeName; }

	virtual void scanForDevices() = 0;

	virtual const StringArray getDeviceNames (bool wantInputNames = false) const = 0;

	virtual int getDefaultDeviceIndex (bool forInput) const = 0;

	virtual int getIndexOfDevice (AudioIODevice* device, bool asInput) const = 0;

	virtual bool hasSeparateInputsAndOutputs() const = 0;

	virtual AudioIODevice* createDevice (const String& outputDeviceName,
										 const String& inputDeviceName) = 0;

	struct DeviceSetupDetails
	{
		AudioDeviceManager* manager;
		int minNumInputChannels, maxNumInputChannels;
		int minNumOutputChannels, maxNumOutputChannels;
		bool useStereoPairs;
	};

	virtual ~AudioIODeviceType();

protected:
	explicit AudioIODeviceType (const String& typeName);

private:
	String typeName;

	AudioIODeviceType (const AudioIODeviceType&);
	AudioIODeviceType& operator= (const AudioIODeviceType&);
};

#endif   // __JUCE_AUDIOIODEVICETYPE_JUCEHEADER__
/*** End of inlined file: juce_AudioIODeviceType.h ***/


/*** Start of inlined file: juce_MidiInput.h ***/
#ifndef __JUCE_MIDIINPUT_JUCEHEADER__
#define __JUCE_MIDIINPUT_JUCEHEADER__


/*** Start of inlined file: juce_MidiMessage.h ***/
#ifndef __JUCE_MIDIMESSAGE_JUCEHEADER__
#define __JUCE_MIDIMESSAGE_JUCEHEADER__

class JUCE_API  MidiMessage
{
public:

	MidiMessage (int byte1, int byte2, int byte3, double timeStamp = 0) throw();

	MidiMessage (int byte1, int byte2, double timeStamp = 0) throw();

	MidiMessage (int byte1, double timeStamp = 0) throw();

	MidiMessage (const void* data, int numBytes, double timeStamp = 0);

	MidiMessage (const void* data, int maxBytesToUse,
				 int& numBytesUsed, uint8 lastStatusByte,
				 double timeStamp = 0);

	MidiMessage (const MidiMessage& other);

	MidiMessage (const MidiMessage& other, double newTimeStamp);

	~MidiMessage();

	MidiMessage& operator= (const MidiMessage& other);

	uint8* getRawData() const throw()			   { return data; }

	int getRawDataSize() const throw()			  { return size; }

	double getTimeStamp() const throw()			 { return timeStamp; }

	void setTimeStamp (double newTimestamp) throw()	   { timeStamp = newTimestamp; }

	void addToTimeStamp (double delta) throw()		{ timeStamp += delta; }

	int getChannel() const throw();

	bool isForChannel (int channelNumber) const throw();

	void setChannel (int newChannelNumber) throw();

	bool isSysEx() const throw();

	const uint8* getSysExData() const throw();

	int getSysExDataSize() const throw();

	bool isNoteOn (bool returnTrueForVelocity0 = false) const throw();

	static const MidiMessage noteOn (int channel, int noteNumber, float velocity) throw();

	static const MidiMessage noteOn (int channel, int noteNumber, uint8 velocity) throw();

	bool isNoteOff (bool returnTrueForNoteOnVelocity0 = true) const throw();

	static const MidiMessage noteOff (int channel, int noteNumber) throw();

	bool isNoteOnOrOff() const throw();

	int getNoteNumber() const throw();

	void setNoteNumber (int newNoteNumber) throw();

	uint8 getVelocity() const throw();

	float getFloatVelocity() const throw();

	void setVelocity (float newVelocity) throw();

	void multiplyVelocity (float scaleFactor) throw();

	bool isProgramChange() const throw();

	int getProgramChangeNumber() const throw();

	static const MidiMessage programChange (int channel, int programNumber) throw();

	bool isPitchWheel() const throw();

	int getPitchWheelValue() const throw();

	static const MidiMessage pitchWheel (int channel, int position) throw();

	bool isAftertouch() const throw();

	int getAfterTouchValue() const throw();

	static const MidiMessage aftertouchChange (int channel,
											   int noteNumber,
											   int aftertouchAmount) throw();

	bool isChannelPressure() const throw();

	int getChannelPressureValue() const throw();

	static const MidiMessage channelPressureChange (int channel, int pressure) throw();

	bool isController() const throw();

	int getControllerNumber() const throw();

	int getControllerValue() const throw();

	static const MidiMessage controllerEvent (int channel,
											  int controllerType,
											  int value) throw();

	bool isAllNotesOff() const throw();

	bool isAllSoundOff() const throw();

	static const MidiMessage allNotesOff (int channel) throw();

	static const MidiMessage allSoundOff (int channel) throw();

	static const MidiMessage allControllersOff (int channel) throw();

	bool isMetaEvent() const throw();

	int getMetaEventType() const throw();

	const uint8* getMetaEventData() const throw();

	int getMetaEventLength() const throw();

	bool isTrackMetaEvent() const throw();

	bool isEndOfTrackMetaEvent() const throw();

	static const MidiMessage endOfTrack() throw();

	bool isTrackNameEvent() const throw();

	bool isTextMetaEvent() const throw();

	const String getTextFromTextMetaEvent() const;

	bool isTempoMetaEvent() const throw();

	double getTempoMetaEventTickLength (short timeFormat) const throw();

	double getTempoSecondsPerQuarterNote() const throw();

	static const MidiMessage tempoMetaEvent (int microsecondsPerQuarterNote) throw();

	bool isTimeSignatureMetaEvent() const throw();

	void getTimeSignatureInfo (int& numerator, int& denominator) const throw();

	static const MidiMessage timeSignatureMetaEvent (int numerator, int denominator);

	bool isKeySignatureMetaEvent() const throw();

	int getKeySignatureNumberOfSharpsOrFlats() const throw();

	bool isMidiChannelMetaEvent() const throw();

	int getMidiChannelMetaEventChannel() const throw();

	static const MidiMessage midiChannelMetaEvent (int channel) throw();

	bool isActiveSense() const throw();

	bool isMidiStart() const throw();

	static const MidiMessage midiStart() throw();

	bool isMidiContinue() const throw();

	static const MidiMessage midiContinue() throw();

	bool isMidiStop() const throw();

	static const MidiMessage midiStop() throw();

	bool isMidiClock() const throw();

	static const MidiMessage midiClock() throw();

	bool isSongPositionPointer() const throw();

	int getSongPositionPointerMidiBeat() const throw();

	static const MidiMessage songPositionPointer (int positionInMidiBeats) throw();

	bool isQuarterFrame() const throw();

	int getQuarterFrameSequenceNumber() const throw();

	int getQuarterFrameValue() const throw();

	static const MidiMessage quarterFrame (int sequenceNumber, int value) throw();

	enum SmpteTimecodeType
	{
		fps24	   = 0,
		fps25	   = 1,
		fps30drop   = 2,
		fps30	   = 3
	};

	bool isFullFrame() const throw();

	void getFullFrameParameters (int& hours,
								 int& minutes,
								 int& seconds,
								 int& frames,
								 SmpteTimecodeType& timecodeType) const throw();

	static const MidiMessage fullFrame (int hours,
										int minutes,
										int seconds,
										int frames,
										SmpteTimecodeType timecodeType);

	enum MidiMachineControlCommand
	{
		mmc_stop		= 1,
		mmc_play		= 2,
		mmc_deferredplay	= 3,
		mmc_fastforward	 = 4,
		mmc_rewind	  = 5,
		mmc_recordStart	 = 6,
		mmc_recordStop	  = 7,
		mmc_pause	   = 9
	};

	bool isMidiMachineControlMessage() const throw();

	MidiMachineControlCommand getMidiMachineControlCommand() const throw();

	static const MidiMessage midiMachineControlCommand (MidiMachineControlCommand command);

	bool isMidiMachineControlGoto (int& hours,
								   int& minutes,
								   int& seconds,
								   int& frames) const throw();

	static const MidiMessage midiMachineControlGoto (int hours,
													 int minutes,
													 int seconds,
													 int frames);

	static const MidiMessage masterVolume (float volume);

	static const MidiMessage createSysExMessage (const uint8* sysexData,
												 int dataSize);

	static int readVariableLengthVal (const uint8* data,
									  int& numBytesUsed) throw();

	static int getMessageLengthFromFirstByte (const uint8 firstByte) throw();

	static const String getMidiNoteName (int noteNumber,
										 bool useSharps,
										 bool includeOctaveNumber,
										 int octaveNumForMiddleC) throw();

	static const double getMidiNoteInHertz (int noteNumber) throw();

	static const String getGMInstrumentName (int midiInstrumentNumber) throw();

	static const String getGMInstrumentBankName (int midiBankNumber) throw();

	static const String getRhythmInstrumentName (int midiNoteNumber) throw();

	static const String getControllerName (int controllerNumber) throw();

	juce_UseDebuggingNewOperator

private:
	double timeStamp;
	uint8* data;
	int size;

	union
	{
		uint8 asBytes[4];
		uint32 asInt32;
	} preallocatedData;
};

#endif   // __JUCE_MIDIMESSAGE_JUCEHEADER__
/*** End of inlined file: juce_MidiMessage.h ***/

class MidiInput;

class JUCE_API  MidiInputCallback
{
public:
	virtual ~MidiInputCallback()  {}

	virtual void handleIncomingMidiMessage (MidiInput* source,
											const MidiMessage& message) = 0;

	virtual void handlePartialSysexMessage (MidiInput* source,
											const uint8* messageData,
											const int numBytesSoFar,
											const double timestamp)
	{
		// (this bit is just to avoid compiler warnings about unused variables)
		(void) source; (void) messageData; (void) numBytesSoFar; (void) timestamp;
	}
};

class JUCE_API  MidiInput
{
public:

	static const StringArray getDevices();

	static int getDefaultDeviceIndex();

	static MidiInput* openDevice (int deviceIndex,
								  MidiInputCallback* callback);

#if JUCE_LINUX || JUCE_MAC || DOXYGEN
	static MidiInput* createNewDevice (const String& deviceName,
									   MidiInputCallback* callback);
#endif

	virtual ~MidiInput();

	virtual const String getName() const throw()			{ return name; }

	virtual void setName (const String& newName) throw()		{ name = newName; }

	virtual void start();

	virtual void stop();

	juce_UseDebuggingNewOperator

protected:
	String name;
	void* internal;

	explicit MidiInput (const String& name);

private:
	MidiInput (const MidiInput&);
	MidiInput& operator= (const MidiInput&);
};

#endif   // __JUCE_MIDIINPUT_JUCEHEADER__
/*** End of inlined file: juce_MidiInput.h ***/


/*** Start of inlined file: juce_MidiOutput.h ***/
#ifndef __JUCE_MIDIOUTPUT_JUCEHEADER__
#define __JUCE_MIDIOUTPUT_JUCEHEADER__


/*** Start of inlined file: juce_MidiBuffer.h ***/
#ifndef __JUCE_MIDIBUFFER_JUCEHEADER__
#define __JUCE_MIDIBUFFER_JUCEHEADER__

class JUCE_API  MidiBuffer
{
public:

	MidiBuffer() throw();

	explicit MidiBuffer (const MidiMessage& message) throw();

	MidiBuffer (const MidiBuffer& other) throw();

	MidiBuffer& operator= (const MidiBuffer& other) throw();

	~MidiBuffer() throw();

	void clear() throw();

	void clear (const int start,
				const int numSamples) throw();

	bool isEmpty() const throw();

	int getNumEvents() const throw();

	void addEvent (const MidiMessage& midiMessage,
				   const int sampleNumber) throw();

	void addEvent (const uint8* const rawMidiData,
				   const int maxBytesOfMidiData,
				   const int sampleNumber) throw();

	void addEvents (const MidiBuffer& otherBuffer,
					const int startSample,
					const int numSamples,
					const int sampleDeltaToAdd) throw();

	int getFirstEventTime() const throw();

	int getLastEventTime() const throw();

	void swapWith (MidiBuffer& other);

	class Iterator
	{
	public:

		Iterator (const MidiBuffer& buffer) throw();

		~Iterator() throw();

		void setNextSamplePosition (const int samplePosition) throw();

		bool getNextEvent (MidiMessage& result,
						   int& samplePosition) throw();

		bool getNextEvent (const uint8* &midiData,
						   int& numBytesOfMidiData,
						   int& samplePosition) throw();

		juce_UseDebuggingNewOperator

	private:
		const MidiBuffer& buffer;
		const uint8* data;

		Iterator (const Iterator&);
		Iterator& operator= (const Iterator&);
	};

	juce_UseDebuggingNewOperator

private:
	friend class MidiBuffer::Iterator;
	MemoryBlock data;
	int bytesUsed;

	uint8* getData() const throw();
	uint8* findEventAfter (uint8* d, const int samplePosition) const throw();
	static int getEventTime (const void* d) throw();
	static uint16 getEventDataSize (const void* d) throw();
	static uint16 getEventTotalSize (const void* d) throw();
};

#endif   // __JUCE_MIDIBUFFER_JUCEHEADER__
/*** End of inlined file: juce_MidiBuffer.h ***/

class JUCE_API  MidiOutput  : private Thread
{
public:

	static const StringArray getDevices();

	static int getDefaultDeviceIndex();

	static MidiOutput* openDevice (int deviceIndex);

#if JUCE_LINUX || JUCE_MAC || DOXYGEN
	static MidiOutput* createNewDevice (const String& deviceName);
#endif

	virtual ~MidiOutput();

	virtual void sendMessageNow (const MidiMessage& message);

	virtual void reset();

	virtual bool getVolume (float& leftVol,
							float& rightVol);

	virtual void setVolume (float leftVol,
							float rightVol);

	virtual void sendBlockOfMessages (const MidiBuffer& buffer,
									  double millisecondCounterToStartAt,
									  double samplesPerSecondForBuffer);

	virtual void clearAllPendingMessages();

	virtual void startBackgroundThread();

	virtual void stopBackgroundThread();

	juce_UseDebuggingNewOperator

protected:
	void* internal;

	struct PendingMessage
	{
		PendingMessage (const uint8* data, int len, double sampleNumber);

		MidiMessage message;
		PendingMessage* next;

		juce_UseDebuggingNewOperator
	};

	CriticalSection lock;
	PendingMessage* firstMessage;

	MidiOutput();
	void run();

private:
	MidiOutput (const MidiOutput&);
	MidiOutput& operator= (const MidiOutput&);
};

#endif   // __JUCE_MIDIOUTPUT_JUCEHEADER__
/*** End of inlined file: juce_MidiOutput.h ***/


/*** Start of inlined file: juce_ComboBox.h ***/
#ifndef __JUCE_COMBOBOX_JUCEHEADER__
#define __JUCE_COMBOBOX_JUCEHEADER__


/*** Start of inlined file: juce_Label.h ***/
#ifndef __JUCE_LABEL_JUCEHEADER__
#define __JUCE_LABEL_JUCEHEADER__


/*** Start of inlined file: juce_TextEditor.h ***/
#ifndef __JUCE_TEXTEDITOR_JUCEHEADER__
#define __JUCE_TEXTEDITOR_JUCEHEADER__


/*** Start of inlined file: juce_Viewport.h ***/
#ifndef __JUCE_VIEWPORT_JUCEHEADER__
#define __JUCE_VIEWPORT_JUCEHEADER__


/*** Start of inlined file: juce_ScrollBar.h ***/
#ifndef __JUCE_SCROLLBAR_JUCEHEADER__
#define __JUCE_SCROLLBAR_JUCEHEADER__


/*** Start of inlined file: juce_Button.h ***/
#ifndef __JUCE_BUTTON_JUCEHEADER__
#define __JUCE_BUTTON_JUCEHEADER__


/*** Start of inlined file: juce_TooltipWindow.h ***/
#ifndef __JUCE_TOOLTIPWINDOW_JUCEHEADER__
#define __JUCE_TOOLTIPWINDOW_JUCEHEADER__


/*** Start of inlined file: juce_TooltipClient.h ***/
#ifndef __JUCE_TOOLTIPCLIENT_JUCEHEADER__
#define __JUCE_TOOLTIPCLIENT_JUCEHEADER__

class JUCE_API  TooltipClient
{
public:
	virtual ~TooltipClient()  {}

	virtual const String getTooltip() = 0;
};

class JUCE_API  SettableTooltipClient   : public TooltipClient
{
public:

	virtual ~SettableTooltipClient()				{}

	virtual void setTooltip (const String& newTooltip)		  { tooltipString = newTooltip; }

	virtual const String getTooltip()				   { return tooltipString; }

	juce_UseDebuggingNewOperator

protected:
	String tooltipString;
};

#endif   // __JUCE_TOOLTIPCLIENT_JUCEHEADER__
/*** End of inlined file: juce_TooltipClient.h ***/

class JUCE_API  TooltipWindow  : public Component,
								 private Timer
{
public:

	explicit TooltipWindow (Component* parentComponent = 0,
							int millisecondsBeforeTipAppears = 700);

	~TooltipWindow();

	void setMillisecondsBeforeTipAppears (int newTimeMs = 700) throw();

	enum ColourIds
	{
		backgroundColourId	  = 0x1001b00,	/**< The colour to fill the background with. */
		textColourId		= 0x1001c00,	/**< The colour to use for the text. */
		outlineColourId	 = 0x1001c10	 /**< The colour to use to draw an outline around the tooltip. */
	};

	juce_UseDebuggingNewOperator

private:

	int millisecondsBeforeTipAppears;
	Point<int> lastMousePos;
	int mouseClicks;
	unsigned int lastCompChangeTime, lastHideTime;
	Component* lastComponentUnderMouse;
	bool changedCompsSinceShown;
	String tipShowing, lastTipUnderMouse;

	void paint (Graphics& g);
	void mouseEnter (const MouseEvent& e);
	void timerCallback();

	static const String getTipFor (Component* c);
	void showFor (const String& tip);
	void hide();

	TooltipWindow (const TooltipWindow&);
	TooltipWindow& operator= (const TooltipWindow&);
};

#endif   // __JUCE_TOOLTIPWINDOW_JUCEHEADER__
/*** End of inlined file: juce_TooltipWindow.h ***/

class Button;

class JUCE_API  ButtonListener
{
public:
	virtual ~ButtonListener()				   {}

	virtual void buttonClicked (Button* button) = 0;

	virtual void buttonStateChanged (Button*)		   {}
};

class JUCE_API  Button  : public Component,
						  public SettableTooltipClient,
						  public ApplicationCommandManagerListener,
						  public Value::Listener,
						  private KeyListener
{
protected:

	explicit Button (const String& buttonName);

public:
	virtual ~Button();

	void setButtonText (const String& newText);

	const String getButtonText() const			  { return text; }

	bool isDown() const throw();

	bool isOver() const throw();

	void setToggleState (bool shouldBeOn,
						 bool sendChangeNotification);

	bool getToggleState() const throw()			 { return isOn.getValue(); }

	Value& getToggleStateValue()				{ return isOn; }

	void setClickingTogglesState (bool shouldToggle) throw();

	bool getClickingTogglesState() const throw();

	void setRadioGroupId (int newGroupId);

	int getRadioGroupId() const throw()			 { return radioGroupId; }

	void addButtonListener (ButtonListener* newListener);

	void removeButtonListener (ButtonListener* listener);

	virtual void triggerClick();

	void setCommandToTrigger (ApplicationCommandManager* commandManagerToUse,
							  int commandID,
							  bool generateTooltip);

	int getCommandID() const throw()		{ return commandID; }

	void addShortcut (const KeyPress& key);

	void clearShortcuts();

	bool isRegisteredForShortcut (const KeyPress& key) const;

	void setRepeatSpeed (int initialDelayInMillisecs,
						 int repeatDelayInMillisecs,
						 int minimumDelayInMillisecs = -1) throw();

	void setTriggeredOnMouseDown (bool isTriggeredOnMouseDown) throw();

	uint32 getMillisecondsSinceButtonDown() const throw();

	void setVisible (bool shouldBeVisible);

	void setTooltip (const String& newTooltip);

	// (implementation of the TooltipClient method)
	const String getTooltip();

	enum ConnectedEdgeFlags
	{
		ConnectedOnLeft = 1,
		ConnectedOnRight = 2,
		ConnectedOnTop = 4,
		ConnectedOnBottom = 8
	};

	void setConnectedEdges (int connectedEdgeFlags);

	int getConnectedEdgeFlags() const throw()		   { return connectedEdgeFlags; }

	bool isConnectedOnLeft() const throw()			  { return (connectedEdgeFlags & ConnectedOnLeft) != 0; }

	bool isConnectedOnRight() const throw()			 { return (connectedEdgeFlags & ConnectedOnRight) != 0; }

	bool isConnectedOnTop() const throw()			   { return (connectedEdgeFlags & ConnectedOnTop) != 0; }

	bool isConnectedOnBottom() const throw()			{ return (connectedEdgeFlags & ConnectedOnBottom) != 0; }

	enum ButtonState
	{
		buttonNormal,
		buttonOver,
		buttonDown
	};

	void setState (const ButtonState newState);

	juce_UseDebuggingNewOperator

protected:

	virtual void clicked();

	virtual void clicked (const ModifierKeys& modifiers);

	virtual void paintButton (Graphics& g,
							  bool isMouseOverButton,
							  bool isButtonDown) = 0;

	virtual void buttonStateChanged();

	virtual void internalClickCallback (const ModifierKeys& modifiers);
	void handleCommandMessage (int commandId);
	void mouseEnter (const MouseEvent& e);
	void mouseExit (const MouseEvent& e);
	void mouseDown (const MouseEvent& e);
	void mouseDrag (const MouseEvent& e);
	void mouseUp (const MouseEvent& e);
	bool keyPressed (const KeyPress& key);
	bool keyPressed (const KeyPress& key, Component* originatingComponent);
	bool keyStateChanged (bool isKeyDown, Component* originatingComponent);
	void paint (Graphics& g);
	void parentHierarchyChanged();
	void focusGained (FocusChangeType cause);
	void focusLost (FocusChangeType cause);
	void enablementChanged();
	void applicationCommandInvoked (const ApplicationCommandTarget::InvocationInfo&);
	void applicationCommandListChanged();
	void valueChanged (Value& value);

private:

	Array <KeyPress> shortcuts;
	Component::SafePointer<Component> keySource;
	String text;
	ListenerList <ButtonListener> buttonListeners;

	class RepeatTimer;
	friend class RepeatTimer;
	friend class ScopedPointer <RepeatTimer>;
	ScopedPointer <RepeatTimer> repeatTimer;
	uint32 buttonPressTime, lastTimeCallbackTime;
	ApplicationCommandManager* commandManagerToUse;
	int autoRepeatDelay, autoRepeatSpeed, autoRepeatMinimumDelay;
	int radioGroupId, commandID, connectedEdgeFlags;
	ButtonState buttonState;

	Value isOn;
	bool lastToggleState : 1;
	bool clickTogglesState : 1;
	bool needsToRelease : 1;
	bool needsRepainting : 1;
	bool isKeyDown : 1;
	bool triggerOnMouseDown : 1;
	bool generateTooltip : 1;

	void repeatTimerCallback();
	RepeatTimer& getRepeatTimer();

	ButtonState updateState (const MouseEvent* const e);
	bool isShortcutPressed() const;
	void turnOffOtherButtonsInGroup (const bool sendChangeNotification);

	void flashButtonState();
	void sendClickMessage (const ModifierKeys& modifiers);
	void sendStateMessage();

	Button (const Button&);
	Button& operator= (const Button&);
};

#endif   // __JUCE_BUTTON_JUCEHEADER__
/*** End of inlined file: juce_Button.h ***/

class ScrollBar;

class JUCE_API  ScrollBarListener
{
public:
	virtual ~ScrollBarListener() {}

	virtual void scrollBarMoved (ScrollBar* scrollBarThatHasMoved,
								 double newRangeStart) = 0;
};

class JUCE_API  ScrollBar  : public Component,
							 public AsyncUpdater,
							 private Timer
{
public:

	ScrollBar (bool isVertical,
			   bool buttonsAreVisible = true);

	~ScrollBar();

	bool isVertical() const throw()				 { return vertical; }

	void setOrientation (bool shouldBeVertical);

	void setButtonVisibility (bool buttonsAreVisible);

	void setAutoHide (bool shouldHideWhenFullRange);

	void setRangeLimits (const Range<double>& newRangeLimit);

	void setRangeLimits (double minimum, double maximum);

	const Range<double> getRangeLimit() const throw()		   { return totalRange; }

	double getMinimumRangeLimit() const throw()			 { return totalRange.getStart(); }

	double getMaximumRangeLimit() const throw()			 { return totalRange.getEnd(); }

	void setCurrentRange (const Range<double>& newRange);

	void setCurrentRange (double newStart, double newSize);

	void setCurrentRangeStart (double newStart);

	const Range<double> getCurrentRange() const throw()		 { return visibleRange; }

	double getCurrentRangeStart() const throw()			 { return visibleRange.getStart(); }

	double getCurrentRangeSize() const throw()			  { return visibleRange.getLength(); }

	void setSingleStepSize (double newSingleStepSize);

	void moveScrollbarInSteps (int howManySteps);

	void moveScrollbarInPages (int howManyPages);

	void scrollToTop();

	void scrollToBottom();

	void setButtonRepeatSpeed (int initialDelayInMillisecs,
							   int repeatDelayInMillisecs,
							   int minimumDelayInMillisecs = -1);

	enum ColourIds
	{
		backgroundColourId	  = 0x1000300,	/**< The background colour of the scrollbar. */
		thumbColourId		   = 0x1000400,	/**< A base colour to use for the thumb. The look and feel will probably use variations on this colour. */
		trackColourId		   = 0x1000401	 /**< A base colour to use for the slot area of the bar. The look and feel will probably use variations on this colour. */
	};

	void addListener (ScrollBarListener* listener);

	void removeListener (ScrollBarListener* listener);

	bool keyPressed (const KeyPress& key);
	void mouseWheelMove (const MouseEvent& e, float wheelIncrementX, float wheelIncrementY);
	void lookAndFeelChanged();
	void handleAsyncUpdate();
	void mouseDown (const MouseEvent& e);
	void mouseDrag (const MouseEvent& e);
	void mouseUp   (const MouseEvent& e);
	void paint (Graphics& g);
	void resized();

	juce_UseDebuggingNewOperator

private:

	Range <double> totalRange, visibleRange;
	double singleStepSize, dragStartRange;
	int thumbAreaStart, thumbAreaSize, thumbStart, thumbSize;
	int dragStartMousePos, lastMousePos;
	int initialDelayInMillisecs, repeatDelayInMillisecs, minimumDelayInMillisecs;
	bool vertical, isDraggingThumb, alwaysVisible;
	Button* upButton;
	Button* downButton;
	ListenerList <ScrollBarListener> listeners;

	void updateThumbPosition();
	void timerCallback();

	ScrollBar (const ScrollBar&);
	ScrollBar& operator= (const ScrollBar&);
};

#endif   // __JUCE_SCROLLBAR_JUCEHEADER__
/*** End of inlined file: juce_ScrollBar.h ***/

class JUCE_API  Viewport  : public Component,
							private ComponentListener,
							private ScrollBarListener
{
public:

	explicit Viewport (const String& componentName = String::empty);

	~Viewport();

	void setViewedComponent (Component* newViewedComponent);

	Component* getViewedComponent() const throw()		   { return contentComp; }

	void setViewPosition (int xPixelsOffset, int yPixelsOffset);

	void setViewPositionProportionately (double proportionX, double proportionY);

	bool autoScroll (int mouseX, int mouseY, int distanceFromEdge, int maximumSpeed);

	int getViewPositionX() const throw()			{ return lastVX; }

	int getViewPositionY() const throw()			{ return lastVY; }

	int getViewWidth() const throw()			{ return lastVW; }

	int getViewHeight() const throw()			   { return lastVH; }

	int getMaximumVisibleWidth() const throw();

	int getMaximumVisibleHeight() const throw();

	virtual void visibleAreaChanged (int visibleX, int visibleY,
									 int visibleW, int visibleH);

	void setScrollBarsShown (bool showVerticalScrollbarIfNeeded,
							 bool showHorizontalScrollbarIfNeeded);

	bool isVerticalScrollBarShown() const throw()		   { return showVScrollbar; }

	bool isHorizontalScrollBarShown() const throw()		 { return showHScrollbar; }

	void setScrollBarThickness (int thickness);

	int getScrollBarThickness() const throw();

	void setSingleStepSizes (int stepX, int stepY);

	void setScrollBarButtonVisibility (bool buttonsVisible);

	ScrollBar* getVerticalScrollBar() const throw()		 { return verticalScrollBar; }

	ScrollBar* getHorizontalScrollBar() const throw()	   { return horizontalScrollBar; }

	juce_UseDebuggingNewOperator

	void resized();
	void scrollBarMoved (ScrollBar* scrollBarThatHasMoved, double newRangeStart);
	void mouseWheelMove (const MouseEvent& e, float wheelIncrementX, float wheelIncrementY);
	bool keyPressed (const KeyPress& key);
	void componentMovedOrResized (Component& component, bool wasMoved, bool wasResized);
	bool useMouseWheelMoveIfNeeded (const MouseEvent& e, float wheelIncrementX, float wheelIncrementY);

private:
	Component::SafePointer<Component> contentComp;
	int lastVX, lastVY, lastVW, lastVH;
	int scrollBarThickness;
	int singleStepX, singleStepY;
	bool showHScrollbar, showVScrollbar;
	Component* contentHolder;
	ScrollBar* verticalScrollBar;
	ScrollBar* horizontalScrollBar;

	void updateVisibleRegion();
	Viewport (const Viewport&);
	Viewport& operator= (const Viewport&);
};

#endif   // __JUCE_VIEWPORT_JUCEHEADER__
/*** End of inlined file: juce_Viewport.h ***/


/*** Start of inlined file: juce_PopupMenu.h ***/
#ifndef __JUCE_POPUPMENU_JUCEHEADER__
#define __JUCE_POPUPMENU_JUCEHEADER__

class PopupMenuCustomComponent;

class JUCE_API  PopupMenu
{
public:

	PopupMenu();

	PopupMenu (const PopupMenu& other);

	~PopupMenu();

	PopupMenu& operator= (const PopupMenu& other);

	void clear();

	void addItem (int itemResultId,
				  const String& itemText,
				  bool isActive = true,
				  bool isTicked = false,
				  const Image* iconToUse = 0);

	void addCommandItem (ApplicationCommandManager* commandManager,
						 int commandID,
						 const String& displayName = String::empty);

	void addColouredItem (int itemResultId,
						  const String& itemText,
						  const Colour& itemTextColour,
						  bool isActive = true,
						  bool isTicked = false,
						  const Image* iconToUse = 0);

	void addCustomItem (int itemResultId, PopupMenuCustomComponent* customComponent);

	void addCustomItem (int itemResultId,
						Component* customComponent,
						int idealWidth, int idealHeight,
						bool triggerMenuItemAutomaticallyWhenClicked);

	void addSubMenu (const String& subMenuName,
					 const PopupMenu& subMenu,
					 bool isActive = true,
					 Image* iconToUse = 0,
					 bool isTicked = false);

	void addSeparator();

	void addSectionHeader (const String& title);

	int getNumItems() const throw();

	bool containsCommandItem (int commandID) const;

	bool containsAnyActiveItems() const throw();

	int show (int itemIdThatMustBeVisible = 0,
			  int minimumWidth = 0,
			  int maximumNumColumns = 0,
			  int standardItemHeight = 0);

	int showAt (int screenX,
				int screenY,
				int itemIdThatMustBeVisible = 0,
				int minimumWidth = 0,
				int maximumNumColumns = 0,
				int standardItemHeight = 0);

	int showAt (Component* componentToAttachTo,
				int itemIdThatMustBeVisible = 0,
				int minimumWidth = 0,
				int maximumNumColumns = 0,
				int standardItemHeight = 0);

	static void JUCE_CALLTYPE dismissAllActiveMenus();

	void setLookAndFeel (LookAndFeel* newLookAndFeel);

	enum ColourIds
	{
		backgroundColourId		 = 0x1000700,  /**< The colour to fill the menu's background with. */
		textColourId		   = 0x1000600,  /**< The colour for normal menu item text, (unless the
														  colour is specified when the item is added). */
		headerTextColourId		 = 0x1000601,  /**< The colour for section header item text (see the
														  addSectionHeader() method). */
		highlightedBackgroundColourId  = 0x1000900,  /**< The colour to fill the background of the currently
														  highlighted menu item. */
		highlightedTextColourId	= 0x1000800,  /**< The colour to use for the text of the currently
														  highlighted item. */
	};

	class JUCE_API  MenuItemIterator
	{
	public:

		MenuItemIterator (const PopupMenu& menu);

		~MenuItemIterator();

		bool next();

		String itemName;
		const PopupMenu* subMenu;
		int itemId;
		bool isSeparator;
		bool isTicked;
		bool isEnabled;
		bool isCustomComponent;
		bool isSectionHeader;
		const Colour* customColour;
		const Image* customImage;
		ApplicationCommandManager* commandManager;

		juce_UseDebuggingNewOperator

	private:
		const PopupMenu& menu;
		int index;

		MenuItemIterator (const MenuItemIterator&);
		MenuItemIterator& operator= (const MenuItemIterator&);
	};

	juce_UseDebuggingNewOperator

private:
	class Item;
	class ItemComponent;
	class Window;

	friend class MenuItemIterator;
	friend class ItemComponent;
	friend class Window;
	friend class PopupMenuCustomComponent;
	friend class OwnedArray <Item>;
	friend class ScopedPointer <Window>;

	OwnedArray <Item> items;
	LookAndFeel* lookAndFeel;
	bool separatorPending;

	void addSeparatorIfPending();

	int showMenu (int x, int y, int w, int h,
				  int itemIdThatMustBeVisible,
				  int minimumWidth,
				  int maximumNumColumns,
				  int standardItemHeight,
				  bool alignToRectangle,
				  Component* componentAttachedTo);

	friend class MenuBarComponent;
	Component* createMenuComponent (int x, int y, int w, int h,
									int itemIdThatMustBeVisible,
									int minimumWidth,
									int maximumNumColumns,
									int standardItemHeight,
									bool alignToRectangle,
									Component* menuBarComponent,
									ApplicationCommandManager** managerOfChosenCommand,
									Component* componentAttachedTo);
};

#endif   // __JUCE_POPUPMENU_JUCEHEADER__
/*** End of inlined file: juce_PopupMenu.h ***/


/*** Start of inlined file: juce_TextInputTarget.h ***/
#ifndef __JUCE_TEXTINPUTTARGET_JUCEHEADER__
#define __JUCE_TEXTINPUTTARGET_JUCEHEADER__

class JUCE_API  TextInputTarget
{
public:

	TextInputTarget() {}

	virtual ~TextInputTarget() {}

	virtual const Range<int> getHighlightedRegion() const = 0;

	virtual void setHighlightedRegion (const Range<int>& newRange) = 0;

	virtual const String getTextInRange (const Range<int>& range) const = 0;

	virtual void insertTextAtCaret (const String& textToInsert) = 0;
};

#endif   // __JUCE_TEXTINPUTTARGET_JUCEHEADER__
/*** End of inlined file: juce_TextInputTarget.h ***/

class TextEditor;
class TextHolderComponent;

class JUCE_API  TextEditorListener
{
public:
	virtual ~TextEditorListener()  {}

	virtual void textEditorTextChanged (TextEditor& editor) = 0;

	virtual void textEditorReturnKeyPressed (TextEditor& editor) = 0;

	virtual void textEditorEscapeKeyPressed (TextEditor& editor) = 0;

	virtual void textEditorFocusLost (TextEditor& editor) = 0;
};

class JUCE_API  TextEditor  : public Component,
							  public TextInputTarget,
							  public SettableTooltipClient
{
public:

	explicit TextEditor (const String& componentName = String::empty,
						 tchar passwordCharacter = 0);

	virtual ~TextEditor();

	void setMultiLine (bool shouldBeMultiLine,
					   bool shouldWordWrap = true);

	bool isMultiLine() const;

	void setReturnKeyStartsNewLine (bool shouldStartNewLine);

	bool getReturnKeyStartsNewLine() const			  { return returnKeyStartsNewLine; }

	void setTabKeyUsedAsCharacter (bool shouldTabKeyBeUsed);

	bool isTabKeyUsedAsCharacter() const			{ return tabKeyUsed; }

	void setReadOnly (bool shouldBeReadOnly);

	bool isReadOnly() const;

	void setCaretVisible (bool shouldBeVisible);

	bool isCaretVisible() const				 { return caretVisible; }

	void setScrollbarsShown (bool shouldBeEnabled);

	bool areScrollbarsShown() const				 { return scrollbarVisible; }

	void setPasswordCharacter (tchar passwordCharacter);

	tchar getPasswordCharacter() const			  { return passwordCharacter; }

	void setPopupMenuEnabled (bool menuEnabled);

	bool isPopupMenuEnabled() const				 { return popupMenuEnabled; }

	bool isPopupMenuCurrentlyActive() const			 { return menuActive; }

	enum ColourIds
	{
		backgroundColourId	   = 0x1000200, /**< The colour to use for the text component's background - this can be
												   transparent if necessary. */

		textColourId		 = 0x1000201, /**< The colour that will be used when text is added to the editor. Note
												   that because the editor can contain multiple colours, calling this
												   method won't change the colour of existing text - to do that, call
												   applyFontToAllText() after calling this method.*/

		highlightColourId	= 0x1000202, /**< The colour with which to fill the background of highlighted sections of
												   the text - this can be transparent if you don't want to show any
												   highlighting.*/

		highlightedTextColourId  = 0x1000203, /**< The colour with which to draw the text in highlighted sections. */

		caretColourId		= 0x1000204, /**< The colour with which to draw the caret. */

		outlineColourId	  = 0x1000205, /**< If this is non-transparent, it will be used to draw a box around
												   the edge of the component. */

		focusedOutlineColourId   = 0x1000206, /**< If this is non-transparent, it will be used to draw a box around
												   the edge of the component when it has focus. */

		shadowColourId	   = 0x1000207, /**< If this is non-transparent, it'll be used to draw an inner shadow
												   around the edge of the editor. */
	};

	void setFont (const Font& newFont);

	void applyFontToAllText (const Font& newFont);

	const Font getFont() const;

	void setSelectAllWhenFocused (bool b);

	void setInputRestrictions (int maxTextLength,
							   const String& allowedCharacters = String::empty);

	void setTextToShowWhenEmpty (const String& text, const Colour& colourToUse);

	void setScrollBarThickness (int newThicknessPixels);

	void setScrollBarButtonVisibility (bool buttonsVisible);

	void addListener (TextEditorListener* newListener);

	void removeListener (TextEditorListener* listenerToRemove);

	const String getText() const;

	const String getTextInRange (const Range<int>& textRange) const;

	bool isEmpty() const;

	void setText (const String& newText,
				  bool sendTextChangeMessage = true);

	Value& getTextValue();

	void insertTextAtCaret (const String& textToInsert);

	void clear();

	void cut();

	void copy();

	void paste();

	void setCaretPosition (int newIndex);

	int getCaretPosition() const;

	void scrollEditorToPositionCaret (int desiredCaretX, int desiredCaretY);

	const Rectangle<int> getCaretRectangle();

	void setHighlightedRegion (const Range<int>& newSelection);

	const Range<int> getHighlightedRegion() const		   { return selection; }

	const String getHighlightedText() const;

	int getTextIndexAt (int x, int y);

	int getTotalNumChars() const;

	int getTextWidth() const;

	int getTextHeight() const;

	void setIndents (int newLeftIndent, int newTopIndent);

	void setBorder (const BorderSize& border);

	const BorderSize getBorder() const;

	void setScrollToShowCursor (bool shouldScrollToShowCursor);

	void paint (Graphics& g);
	void paintOverChildren (Graphics& g);
	void mouseDown (const MouseEvent& e);
	void mouseUp (const MouseEvent& e);
	void mouseDrag (const MouseEvent& e);
	void mouseDoubleClick (const MouseEvent& e);
	void mouseWheelMove (const MouseEvent& e, float wheelIncrementX, float wheelIncrementY);
	bool keyPressed (const KeyPress& key);
	bool keyStateChanged (bool isKeyDown);
	void focusGained (FocusChangeType cause);
	void focusLost (FocusChangeType cause);
	void resized();
	void enablementChanged();
	void colourChanged();

	juce_UseDebuggingNewOperator

protected:

	virtual void addPopupMenuItems (PopupMenu& menuToAddTo,
									const MouseEvent* mouseClickEvent);

	virtual void performPopupMenuAction (int menuItemID);

	void scrollToMakeSureCursorIsVisible();

	void moveCaret (int newCaretPos);

	void moveCursorTo (int newPosition, bool isSelecting);

	void textChanged();

	void newTransaction();

	void doUndoRedo (bool isRedo);

	virtual void returnPressed();

	virtual void escapePressed();

	void handleCommandMessage (int commandId);

private:

	ScopedPointer <Viewport> viewport;
	TextHolderComponent* textHolder;
	BorderSize borderSize;

	bool readOnly		   : 1;
	bool multiline		  : 1;
	bool wordWrap		   : 1;
	bool returnKeyStartsNewLine	 : 1;
	bool caretVisible		   : 1;
	bool popupMenuEnabled	   : 1;
	bool selectAllTextWhenFocused   : 1;
	bool scrollbarVisible	   : 1;
	bool wasFocused		 : 1;
	bool caretFlashState		: 1;
	bool keepCursorOnScreen	 : 1;
	bool tabKeyUsed		 : 1;
	bool menuActive		 : 1;
	bool valueTextNeedsUpdating	 : 1;

	UndoManager undoManager;
	float cursorX, cursorY, cursorHeight;
	int maxTextLength;
	Range<int> selection;
	int leftIndent, topIndent;
	unsigned int lastTransactionTime;
	Font currentFont;
	mutable int totalNumChars;
	int caretPosition;
	VoidArray sections;
	String textToShowWhenEmpty;
	Colour colourForTextWhenEmpty;
	tchar passwordCharacter;
	Value textValue;

	enum
	{
		notDragging,
		draggingSelectionStart,
		draggingSelectionEnd
	} dragType;

	String allowedCharacters;
	ListenerList <TextEditorListener> listeners;

	friend class TextEditorInsertAction;
	friend class TextEditorRemoveAction;

	void coalesceSimilarSections();
	void splitSection (int sectionIndex, int charToSplitAt);
	void clearInternal (UndoManager* um);
	void insert (const String& text, int insertIndex, const Font& font,
				 const Colour& colour, UndoManager* um, int caretPositionToMoveTo);
	void reinsert (int insertIndex, const VoidArray& sections);
	void remove (const Range<int>& range, UndoManager* um, int caretPositionToMoveTo);
	void getCharPosition (int index, float& x, float& y, float& lineHeight) const;
	void updateCaretPosition();
	void textWasChangedByValue();
	int indexAtPosition (float x, float y);
	int findWordBreakAfter (int position) const;
	int findWordBreakBefore (int position) const;

	friend class TextHolderComponent;
	friend class TextEditorViewport;
	void drawContent (Graphics& g);
	void updateTextHolderSize();
	float getWordWrapWidth() const;
	void timerCallbackInt();
	void repaintCaret();
	void repaintText (const Range<int>& range);
	UndoManager* getUndoManager() throw();

	TextEditor (const TextEditor&);
	TextEditor& operator= (const TextEditor&);
};

#endif   // __JUCE_TEXTEDITOR_JUCEHEADER__
/*** End of inlined file: juce_TextEditor.h ***/

class Label;

class JUCE_API  LabelListener
{
public:
	virtual ~LabelListener() {}

	virtual void labelTextChanged (Label* labelThatHasChanged) = 0;
};

class JUCE_API  Label  : public Component,
						 public SettableTooltipClient,
						 protected TextEditorListener,
						 private ComponentListener,
						 private Value::Listener
{
public:

	Label (const String& componentName,
		   const String& labelText);

	~Label();

	void setText (const String& newText,
				  bool broadcastChangeMessage);

	const String getText (bool returnActiveEditorContents = false) const throw();

	Value& getTextValue()				   { return textValue; }

	void setFont (const Font& newFont) throw();

	const Font& getFont() const throw();

	enum ColourIds
	{
		backgroundColourId	 = 0x1000280, /**< The background colour to fill the label with. */
		textColourId	   = 0x1000281, /**< The colour for the text. */
		outlineColourId	= 0x1000282  /**< An optional colour to use to draw a border around the label.
												 Leave this transparent to not have an outline. */
	};

	void setJustificationType (const Justification& justification) throw();

	const Justification getJustificationType() const throw()			{ return justification; }

	void setBorderSize (int horizontalBorder, int verticalBorder);

	int getHorizontalBorderSize() const throw()				 { return horizontalBorderSize; }

	int getVerticalBorderSize() const throw()				   { return verticalBorderSize; }

	void attachToComponent (Component* owner, bool onLeft);

	Component* getAttachedComponent() const;

	bool isAttachedOnLeft() const throw()					   { return leftOfOwnerComp; }

	void setMinimumHorizontalScale (float newScale);

	float getMinimumHorizontalScale() const throw()				 { return minimumHorizontalScale; }

	void addListener (LabelListener* listener) throw();

	void removeListener (LabelListener* listener) throw();

	void setEditable (bool editOnSingleClick,
					  bool editOnDoubleClick = false,
					  bool lossOfFocusDiscardsChanges = false) throw();

	bool isEditableOnSingleClick() const throw()			{ return editSingleClick; }

	bool isEditableOnDoubleClick() const throw()			{ return editDoubleClick; }

	bool doesLossOfFocusDiscardChanges() const throw()		  { return lossOfFocusDiscardsChanges; }

	bool isEditable() const throw()					 { return editSingleClick || editDoubleClick; }

	void showEditor();

	void hideEditor (bool discardCurrentEditorContents);

	bool isBeingEdited() const throw();

	juce_UseDebuggingNewOperator

protected:
	virtual TextEditor* createEditorComponent();

	virtual void textWasEdited();

	virtual void textWasChanged();

	virtual void editorShown (TextEditor* editorComponent);

	virtual void editorAboutToBeHidden (TextEditor* editorComponent);

	void paint (Graphics& g);
	void resized();
	void mouseUp (const MouseEvent& e);
	void mouseDoubleClick (const MouseEvent& e);
	void componentMovedOrResized (Component& component, bool wasMoved, bool wasResized);
	void componentParentHierarchyChanged (Component& component);
	void componentVisibilityChanged (Component& component);
	void inputAttemptWhenModal();
	void focusGained (FocusChangeType);
	void enablementChanged();
	KeyboardFocusTraverser* createFocusTraverser();
	void textEditorTextChanged (TextEditor& editor);
	void textEditorReturnKeyPressed (TextEditor& editor);
	void textEditorEscapeKeyPressed (TextEditor& editor);
	void textEditorFocusLost (TextEditor& editor);
	void colourChanged();
	void valueChanged (Value&);

private:
	Value textValue;
	String lastTextValue;
	Font font;
	Justification justification;
	ScopedPointer <TextEditor> editor;
	ListenerList <LabelListener> listeners;
	Component::SafePointer<Component> ownerComponent;
	int horizontalBorderSize, verticalBorderSize;
	float minimumHorizontalScale;
	bool editSingleClick : 1;
	bool editDoubleClick : 1;
	bool lossOfFocusDiscardsChanges : 1;
	bool leftOfOwnerComp : 1;

	bool updateFromTextEditorContents();
	void callChangeListeners();

	Label (const Label&);
	Label& operator= (const Label&);
};

#endif   // __JUCE_LABEL_JUCEHEADER__
/*** End of inlined file: juce_Label.h ***/

class ComboBox;

class JUCE_API  ComboBoxListener
{
public:
	virtual ~ComboBoxListener() {}

	virtual void comboBoxChanged (ComboBox* comboBoxThatHasChanged) = 0;
};

class JUCE_API  ComboBox  : public Component,
							public SettableTooltipClient,
							private LabelListener,
							private AsyncUpdater,
							private Value::Listener
{
public:

	explicit ComboBox (const String& componentName);

	~ComboBox();

	void setEditableText (bool isEditable);

	bool isTextEditable() const throw();

	void setJustificationType (const Justification& justification) throw();

	const Justification getJustificationType() const throw();

	void addItem (const String& newItemText,
				  int newItemId) throw();

	void addSeparator() throw();

	void addSectionHeading (const String& headingName) throw();

	void setItemEnabled (int itemId,
						 bool shouldBeEnabled) throw();

	void changeItemText (int itemId,
						 const String& newText) throw();

	void clear (bool dontSendChangeMessage = false);

	int getNumItems() const throw();

	const String getItemText (int index) const throw();

	int getItemId (int index) const throw();

	int indexOfItemId (int itemId) const throw();

	int getSelectedId() const throw();

	Value& getSelectedIdAsValue() throw()		   { return currentId; }

	void setSelectedId (int newItemId,
						bool dontSendChangeMessage = false) throw();

	int getSelectedItemIndex() const throw();

	void setSelectedItemIndex (int newItemIndex,
							   bool dontSendChangeMessage = false) throw();

	const String getText() const throw();

	void setText (const String& newText,
				  bool dontSendChangeMessage = false) throw();

	void showEditor();

	void addListener (ComboBoxListener* listener) throw();

	void removeListener (ComboBoxListener* listener) throw();

	void setTextWhenNothingSelected (const String& newMessage) throw();

	const String getTextWhenNothingSelected() const throw();

	void setTextWhenNoChoicesAvailable (const String& newMessage) throw();

	const String getTextWhenNoChoicesAvailable() const throw();

	void setTooltip (const String& newTooltip);

	enum ColourIds
	{
		backgroundColourId  = 0x1000b00,	/**< The background colour to fill the box with. */
		textColourId	= 0x1000a00,	/**< The colour for the text in the box. */
		outlineColourId	 = 0x1000c00,	/**< The colour for an outline around the box. */
		buttonColourId	  = 0x1000d00,	/**< The base colour for the button (a LookAndFeel class will probably use variations on this). */
		arrowColourId	   = 0x1000e00,	/**< The colour for the arrow shape that pops up the menu */
	};

	void labelTextChanged (Label*);
	void enablementChanged();
	void colourChanged();
	void focusGained (Component::FocusChangeType cause);
	void focusLost (Component::FocusChangeType cause);
	void handleAsyncUpdate();
	const String getTooltip()					   { return label->getTooltip(); }
	void mouseDown (const MouseEvent&);
	void mouseDrag (const MouseEvent&);
	void mouseUp (const MouseEvent&);
	void lookAndFeelChanged();
	void paint (Graphics&);
	void resized();
	bool keyStateChanged (bool isKeyDown);
	bool keyPressed (const KeyPress&);
	void valueChanged (Value&);

	juce_UseDebuggingNewOperator

private:
	struct ItemInfo
	{
		String name;
		int itemId;
		bool isEnabled : 1, isHeading : 1;

		bool isSeparator() const throw();
		bool isRealItem() const throw();
	};

	OwnedArray <ItemInfo> items;
	Value currentId;
	int lastCurrentId;
	bool isButtonDown, separatorPending, menuActive, textIsCustom;
	ListenerList <ComboBoxListener> listeners;
	ScopedPointer<Label> label;
	String textWhenNothingSelected, noChoicesMessage;

	void showPopup();

	ItemInfo* getItemForId (int itemId) const throw();
	ItemInfo* getItemForIndex (int index) const throw();

	ComboBox (const ComboBox&);
	ComboBox& operator= (const ComboBox&);
};

#endif   // __JUCE_COMBOBOX_JUCEHEADER__
/*** End of inlined file: juce_ComboBox.h ***/

class JUCE_API  AudioDeviceManager  : public ChangeBroadcaster
{
public:

	AudioDeviceManager();

	~AudioDeviceManager();

	struct JUCE_API  AudioDeviceSetup
	{
		AudioDeviceSetup();
		bool operator== (const AudioDeviceSetup& other) const;

		String outputDeviceName;

		String inputDeviceName;

		double sampleRate;

		int bufferSize;

		BigInteger inputChannels;

		bool useDefaultInputChannels;

		BigInteger outputChannels;

		bool useDefaultOutputChannels;
	};

	const String initialise (int numInputChannelsNeeded,
							 int numOutputChannelsNeeded,
							 const XmlElement* savedState,
							 bool selectDefaultDeviceOnFailure,
							 const String& preferredDefaultDeviceName = String::empty,
							 const AudioDeviceSetup* preferredSetupOptions = 0);

	XmlElement* createStateXml() const;

	void getAudioDeviceSetup (AudioDeviceSetup& setup);

	const String setAudioDeviceSetup (const AudioDeviceSetup& newSetup,
									  bool treatAsChosenDevice);

	AudioIODevice* getCurrentAudioDevice() const throw()		{ return currentAudioDevice; }

	const String getCurrentAudioDeviceType() const			  { return currentDeviceType; }

	AudioIODeviceType* getCurrentDeviceTypeObject() const;

	void setCurrentAudioDeviceType (const String& type,
									bool treatAsChosenDevice);

	void closeAudioDevice();

	void restartLastAudioDevice();

	void addAudioCallback (AudioIODeviceCallback* newCallback);

	void removeAudioCallback (AudioIODeviceCallback* callback);

	double getCpuUsage() const;

	void setMidiInputEnabled (const String& midiInputDeviceName,
							  bool enabled);

	bool isMidiInputEnabled (const String& midiInputDeviceName) const;

	void addMidiInputCallback (const String& midiInputDeviceName,
							   MidiInputCallback* callback);

	void removeMidiInputCallback (const String& midiInputDeviceName,
								  MidiInputCallback* callback);

	void setDefaultMidiOutput (const String& deviceName);

	const String getDefaultMidiOutputName() const		   { return defaultMidiOutputName; }

	MidiOutput* getDefaultMidiOutput() const throw()		{ return defaultMidiOutput; }

	const OwnedArray <AudioIODeviceType>& getAvailableDeviceTypes();

	virtual void createAudioDeviceTypes (OwnedArray <AudioIODeviceType>& types);

	void playTestSound();

	void enableInputLevelMeasurement (bool enableMeasurement);

	double getCurrentInputLevel() const;

	juce_UseDebuggingNewOperator

private:

	OwnedArray <AudioIODeviceType> availableDeviceTypes;
	OwnedArray <AudioDeviceSetup> lastDeviceTypeConfigs;

	AudioDeviceSetup currentSetup;
	ScopedPointer <AudioIODevice> currentAudioDevice;
	SortedSet <AudioIODeviceCallback*> callbacks;
	int numInputChansNeeded, numOutputChansNeeded;
	String currentDeviceType;
	BigInteger inputChannels, outputChannels;
	ScopedPointer <XmlElement> lastExplicitSettings;
	mutable bool listNeedsScanning;
	bool useInputNames;
	int inputLevelMeasurementEnabledCount;
	double inputLevel;
	ScopedPointer <AudioSampleBuffer> testSound;
	int testSoundPosition;
	AudioSampleBuffer tempBuffer;

	StringArray midiInsFromXml;
	OwnedArray <MidiInput> enabledMidiInputs;
	Array <MidiInputCallback*> midiCallbacks;
	Array <MidiInput*> midiCallbackDevices;
	String defaultMidiOutputName;
	ScopedPointer <MidiOutput> defaultMidiOutput;
	CriticalSection audioCallbackLock, midiCallbackLock;

	double cpuUsageMs, timeToCpuScale;

	class CallbackHandler  : public AudioIODeviceCallback,
							 public MidiInputCallback
	{
	public:
		AudioDeviceManager* owner;

		void audioDeviceIOCallback (const float** inputChannelData,
									int totalNumInputChannels,
									float** outputChannelData,
									int totalNumOutputChannels,
									int numSamples);

		void audioDeviceAboutToStart (AudioIODevice*);

		void audioDeviceStopped();

		void handleIncomingMidiMessage (MidiInput* source, const MidiMessage& message);
	};

	CallbackHandler callbackHandler;
	friend class CallbackHandler;

	void audioDeviceIOCallbackInt (const float** inputChannelData,
								   int totalNumInputChannels,
								   float** outputChannelData,
								   int totalNumOutputChannels,
								   int numSamples);
	void audioDeviceAboutToStartInt (AudioIODevice* device);
	void audioDeviceStoppedInt();

	void handleIncomingMidiMessageInt (MidiInput* source, const MidiMessage& message);

	const String restartDevice (int blockSizeToUse, double sampleRateToUse,
								const BigInteger& ins, const BigInteger& outs);
	void stopDevice();

	void updateXml();

	void createDeviceTypesIfNeeded();
	void scanDevicesIfNeeded();
	void deleteCurrentDevice();
	double chooseBestSampleRate (double preferred) const;
	void insertDefaultDeviceNames (AudioDeviceSetup& setup) const;

	AudioIODeviceType* findType (const String& inputName, const String& outputName);

	AudioDeviceManager (const AudioDeviceManager&);
	AudioDeviceManager& operator= (const AudioDeviceManager&);
};

#endif   // __JUCE_AUDIODEVICEMANAGER_JUCEHEADER__
/*** End of inlined file: juce_AudioDeviceManager.h ***/


#endif
#ifndef __JUCE_AUDIOIODEVICE_JUCEHEADER__

#endif
#ifndef __JUCE_AUDIOIODEVICETYPE_JUCEHEADER__

#endif
#ifndef __JUCE_MIDIINPUT_JUCEHEADER__

#endif
#ifndef __JUCE_MIDIOUTPUT_JUCEHEADER__

#endif
#ifndef __JUCE_AUDIODATACONVERTERS_JUCEHEADER__

/*** Start of inlined file: juce_AudioDataConverters.h ***/
#ifndef __JUCE_AUDIODATACONVERTERS_JUCEHEADER__
#define __JUCE_AUDIODATACONVERTERS_JUCEHEADER__

class JUCE_API  AudioDataConverters
{
public:

	static void convertFloatToInt16LE (const float* source, void* dest, int numSamples, int destBytesPerSample = 2);
	static void convertFloatToInt16BE (const float* source, void* dest, int numSamples, int destBytesPerSample = 2);

	static void convertFloatToInt24LE (const float* source, void* dest, int numSamples, int destBytesPerSample = 3);
	static void convertFloatToInt24BE (const float* source, void* dest, int numSamples, int destBytesPerSample = 3);

	static void convertFloatToInt32LE (const float* source, void* dest, int numSamples, int destBytesPerSample = 4);
	static void convertFloatToInt32BE (const float* source, void* dest, int numSamples, int destBytesPerSample = 4);

	static void convertFloatToFloat32LE (const float* source, void* dest, int numSamples, int destBytesPerSample = 4);
	static void convertFloatToFloat32BE (const float* source, void* dest, int numSamples, int destBytesPerSample = 4);

	static void convertInt16LEToFloat (const void* source, float* dest, int numSamples, int srcBytesPerSample = 2);
	static void convertInt16BEToFloat (const void* source, float* dest, int numSamples, int srcBytesPerSample = 2);

	static void convertInt24LEToFloat (const void* source, float* dest, int numSamples, int srcBytesPerSample = 3);
	static void convertInt24BEToFloat (const void* source, float* dest, int numSamples, int srcBytesPerSample = 3);

	static void convertInt32LEToFloat (const void* source, float* dest, int numSamples, int srcBytesPerSample = 4);
	static void convertInt32BEToFloat (const void* source, float* dest, int numSamples, int srcBytesPerSample = 4);

	static void convertFloat32LEToFloat (const void* source, float* dest, int numSamples, int srcBytesPerSample = 4);
	static void convertFloat32BEToFloat (const void* source, float* dest, int numSamples, int srcBytesPerSample = 4);

	enum DataFormat
	{
		int16LE,
		int16BE,
		int24LE,
		int24BE,
		int32LE,
		int32BE,
		float32LE,
		float32BE,
	};

	static void convertFloatToFormat (DataFormat destFormat,
									  const float* source, void* dest, int numSamples);

	static void convertFormatToFloat (DataFormat sourceFormat,
									  const void* source, float* dest, int numSamples);

	static void interleaveSamples (const float** source, float* dest,
								   int numSamples, int numChannels);

	static void deinterleaveSamples (const float* source, float** dest,
									 int numSamples, int numChannels);

private:
	AudioDataConverters();
	AudioDataConverters (const AudioDataConverters&);
	AudioDataConverters& operator= (const AudioDataConverters&);
};

#endif   // __JUCE_AUDIODATACONVERTERS_JUCEHEADER__
/*** End of inlined file: juce_AudioDataConverters.h ***/


#endif
#ifndef __JUCE_AUDIOSAMPLEBUFFER_JUCEHEADER__

#endif
#ifndef __JUCE_IIRFILTER_JUCEHEADER__

#endif
#ifndef __JUCE_MIDIBUFFER_JUCEHEADER__

#endif
#ifndef __JUCE_MIDIFILE_JUCEHEADER__

/*** Start of inlined file: juce_MidiFile.h ***/
#ifndef __JUCE_MIDIFILE_JUCEHEADER__
#define __JUCE_MIDIFILE_JUCEHEADER__


/*** Start of inlined file: juce_MidiMessageSequence.h ***/
#ifndef __JUCE_MIDIMESSAGESEQUENCE_JUCEHEADER__
#define __JUCE_MIDIMESSAGESEQUENCE_JUCEHEADER__

class JUCE_API  MidiMessageSequence
{
public:

	MidiMessageSequence();

	MidiMessageSequence (const MidiMessageSequence& other);

	MidiMessageSequence& operator= (const MidiMessageSequence& other);

	~MidiMessageSequence();

	class MidiEventHolder
	{
	public:

		~MidiEventHolder();

		MidiMessage message;

		MidiEventHolder* noteOffObject;

		juce_UseDebuggingNewOperator

	private:
		friend class MidiMessageSequence;
		MidiEventHolder (const MidiMessage& message);
	};

	void clear();

	int getNumEvents() const;

	MidiEventHolder* getEventPointer (int index) const;

	double getTimeOfMatchingKeyUp (int index) const;

	int getIndexOfMatchingKeyUp (int index) const;

	int getIndexOf (MidiEventHolder* event) const;

	int getNextIndexAtTime (double timeStamp) const;

	double getStartTime() const;

	double getEndTime() const;

	double getEventTime (int index) const;

	void addEvent (const MidiMessage& newMessage,
				   double timeAdjustment = 0);

	void deleteEvent (int index, bool deleteMatchingNoteUp);

	void addSequence (const MidiMessageSequence& other,
					  double timeAdjustmentDelta,
					  double firstAllowableDestTime,
					  double endOfAllowableDestTimes);

	void updateMatchedPairs();

	void extractMidiChannelMessages (int channelNumberToExtract,
									 MidiMessageSequence& destSequence,
									 bool alsoIncludeMetaEvents) const;

	void extractSysExMessages (MidiMessageSequence& destSequence) const;

	void deleteMidiChannelMessages (int channelNumberToRemove);

	void deleteSysExMessages();

	void addTimeToMessages (double deltaTime);

	void createControllerUpdatesForTime (int channelNumber, double time,
										 OwnedArray<MidiMessage>& resultMessages);

	void swapWith (MidiMessageSequence& other) throw();

	juce_UseDebuggingNewOperator

	static int compareElements (const MidiMessageSequence::MidiEventHolder* first,
								const MidiMessageSequence::MidiEventHolder* second) throw();

private:

	friend class MidiFile;
	OwnedArray <MidiEventHolder> list;

	void sort();
};

#endif   // __JUCE_MIDIMESSAGESEQUENCE_JUCEHEADER__
/*** End of inlined file: juce_MidiMessageSequence.h ***/

class JUCE_API  MidiFile
{
public:

	MidiFile();

	~MidiFile();

	int getNumTracks() const throw();

	const MidiMessageSequence* getTrack (const int index) const throw();

	void addTrack (const MidiMessageSequence& trackSequence);

	void clear();

	short getTimeFormat() const throw();

	void setTicksPerQuarterNote (const int ticksPerQuarterNote) throw();

	void setSmpteTimeFormat (const int framesPerSecond,
							 const int subframeResolution) throw();

	void findAllTempoEvents (MidiMessageSequence& tempoChangeEvents) const;

	void findAllTimeSigEvents (MidiMessageSequence& timeSigEvents) const;

	double getLastTimestamp() const;

	bool readFrom (InputStream& sourceStream);

	bool writeTo (OutputStream& destStream);

	void convertTimestampTicksToSeconds();

	juce_UseDebuggingNewOperator

	static int compareElements (const MidiMessageSequence::MidiEventHolder* const first,
								const MidiMessageSequence::MidiEventHolder* const second);

private:
	OwnedArray <MidiMessageSequence> tracks;
	short timeFormat;

	MidiFile (const MidiFile&);
	MidiFile& operator= (const MidiFile&);

	void readNextTrack (const char* data, int size);
	void writeTrack (OutputStream& mainOut, const int trackNum);
};

#endif   // __JUCE_MIDIFILE_JUCEHEADER__
/*** End of inlined file: juce_MidiFile.h ***/


#endif
#ifndef __JUCE_MIDIKEYBOARDSTATE_JUCEHEADER__

/*** Start of inlined file: juce_MidiKeyboardState.h ***/
#ifndef __JUCE_MIDIKEYBOARDSTATE_JUCEHEADER__
#define __JUCE_MIDIKEYBOARDSTATE_JUCEHEADER__

class MidiKeyboardState;

class JUCE_API  MidiKeyboardStateListener
{
public:

	MidiKeyboardStateListener() throw()	 {}
	virtual ~MidiKeyboardStateListener()	{}

	virtual void handleNoteOn (MidiKeyboardState* source,
							   int midiChannel, int midiNoteNumber, float velocity) = 0;

	virtual void handleNoteOff (MidiKeyboardState* source,
								int midiChannel, int midiNoteNumber) = 0;
};

class JUCE_API  MidiKeyboardState
{
public:

	MidiKeyboardState();
	~MidiKeyboardState();

	void reset();

	bool isNoteOn (const int midiChannel, const int midiNoteNumber) const throw();

	bool isNoteOnForChannels (const int midiChannelMask, const int midiNoteNumber) const throw();

	void noteOn (const int midiChannel, const int midiNoteNumber, const float velocity);

	void noteOff (const int midiChannel, const int midiNoteNumber);

	void allNotesOff (const int midiChannel);

	void processNextMidiEvent (const MidiMessage& message);

	void processNextMidiBuffer (MidiBuffer& buffer,
								const int startSample,
								const int numSamples,
								const bool injectIndirectEvents);

	void addListener (MidiKeyboardStateListener* const listener) throw();

	void removeListener (MidiKeyboardStateListener* const listener) throw();

	juce_UseDebuggingNewOperator

private:
	CriticalSection lock;
	uint16 noteStates [128];
	MidiBuffer eventsToAdd;
	Array <MidiKeyboardStateListener*> listeners;

	void noteOnInternal  (const int midiChannel, const int midiNoteNumber, const float velocity);
	void noteOffInternal  (const int midiChannel, const int midiNoteNumber);

	MidiKeyboardState (const MidiKeyboardState&);
	MidiKeyboardState& operator= (const MidiKeyboardState&);
};

#endif   // __JUCE_MIDIKEYBOARDSTATE_JUCEHEADER__
/*** End of inlined file: juce_MidiKeyboardState.h ***/


#endif
#ifndef __JUCE_MIDIMESSAGE_JUCEHEADER__

#endif
#ifndef __JUCE_MIDIMESSAGECOLLECTOR_JUCEHEADER__

/*** Start of inlined file: juce_MidiMessageCollector.h ***/
#ifndef __JUCE_MIDIMESSAGECOLLECTOR_JUCEHEADER__
#define __JUCE_MIDIMESSAGECOLLECTOR_JUCEHEADER__

class JUCE_API  MidiMessageCollector	: public MidiKeyboardStateListener,
										  public MidiInputCallback
{
public:

	MidiMessageCollector();

	~MidiMessageCollector();

	void reset (double sampleRate);

	void addMessageToQueue (const MidiMessage& message);

	void removeNextBlockOfMessages (MidiBuffer& destBuffer, int numSamples);

	void handleNoteOn (MidiKeyboardState* source, int midiChannel, int midiNoteNumber, float velocity);
	void handleNoteOff (MidiKeyboardState* source, int midiChannel, int midiNoteNumber);
	void handleIncomingMidiMessage (MidiInput* source, const MidiMessage& message);

	juce_UseDebuggingNewOperator

private:
	double lastCallbackTime;
	CriticalSection midiCallbackLock;
	MidiBuffer incomingMessages;
	double sampleRate;

	MidiMessageCollector (const MidiMessageCollector&);
	MidiMessageCollector& operator= (const MidiMessageCollector&);
};

#endif   // __JUCE_MIDIMESSAGECOLLECTOR_JUCEHEADER__
/*** End of inlined file: juce_MidiMessageCollector.h ***/


#endif
#ifndef __JUCE_MIDIMESSAGESEQUENCE_JUCEHEADER__

#endif
#ifndef __JUCE_AUDIOUNITPLUGINFORMAT_JUCEHEADER__

/*** Start of inlined file: juce_AudioUnitPluginFormat.h ***/
#ifndef __JUCE_AUDIOUNITPLUGINFORMAT_JUCEHEADER__
#define __JUCE_AUDIOUNITPLUGINFORMAT_JUCEHEADER__


/*** Start of inlined file: juce_AudioPluginFormat.h ***/
#ifndef __JUCE_AUDIOPLUGINFORMAT_JUCEHEADER__
#define __JUCE_AUDIOPLUGINFORMAT_JUCEHEADER__


/*** Start of inlined file: juce_AudioPluginInstance.h ***/
#ifndef __JUCE_AUDIOPLUGININSTANCE_JUCEHEADER__
#define __JUCE_AUDIOPLUGININSTANCE_JUCEHEADER__


/*** Start of inlined file: juce_AudioProcessor.h ***/
#ifndef __JUCE_AUDIOPROCESSOR_JUCEHEADER__
#define __JUCE_AUDIOPROCESSOR_JUCEHEADER__


/*** Start of inlined file: juce_AudioProcessorEditor.h ***/
#ifndef __JUCE_AUDIOPROCESSOREDITOR_JUCEHEADER__
#define __JUCE_AUDIOPROCESSOREDITOR_JUCEHEADER__

class AudioProcessor;

class JUCE_API  AudioProcessorEditor  : public Component
{
protected:

	AudioProcessorEditor (AudioProcessor* const owner);

public:
	~AudioProcessorEditor();

	AudioProcessor* getAudioProcessor() const throw()	 { return owner; }

private:

	AudioProcessor* const owner;

	AudioProcessorEditor (const AudioProcessorEditor&);
	AudioProcessorEditor& operator= (const AudioProcessorEditor&);
};

#endif   // __JUCE_AUDIOPROCESSOREDITOR_JUCEHEADER__
/*** End of inlined file: juce_AudioProcessorEditor.h ***/


/*** Start of inlined file: juce_AudioProcessorListener.h ***/
#ifndef __JUCE_AUDIOPROCESSORLISTENER_JUCEHEADER__
#define __JUCE_AUDIOPROCESSORLISTENER_JUCEHEADER__

class AudioProcessor;

class JUCE_API  AudioProcessorListener
{
public:

	virtual ~AudioProcessorListener() {}

	virtual void audioProcessorParameterChanged (AudioProcessor* processor,
												 int parameterIndex,
												 float newValue) = 0;

	virtual void audioProcessorChanged (AudioProcessor* processor) = 0;

	virtual void audioProcessorParameterChangeGestureBegin (AudioProcessor* processor,
															int parameterIndex);

	virtual void audioProcessorParameterChangeGestureEnd (AudioProcessor* processor,
														  int parameterIndex);
};

#endif   // __JUCE_AUDIOPROCESSORLISTENER_JUCEHEADER__
/*** End of inlined file: juce_AudioProcessorListener.h ***/


/*** Start of inlined file: juce_AudioPlayHead.h ***/
#ifndef __JUCE_AUDIOPLAYHEAD_JUCEHEADER__
#define __JUCE_AUDIOPLAYHEAD_JUCEHEADER__

class JUCE_API  AudioPlayHead
{
protected:

	AudioPlayHead() {}

public:
	virtual ~AudioPlayHead() {}

	enum FrameRateType
	{
		fps24	   = 0,
		fps25	   = 1,
		fps2997	 = 2,
		fps30	   = 3,
		fps2997drop	 = 4,
		fps30drop	   = 5,
		fpsUnknown	  = 99
	};

	struct CurrentPositionInfo
	{
		double bpm;

		int timeSigNumerator;
		int timeSigDenominator;

		double timeInSeconds;

		double editOriginTime;

		double ppqPosition;

		double ppqPositionOfLastBarStart;

		FrameRateType frameRate;

		bool isPlaying;

		bool isRecording;

		bool operator== (const CurrentPositionInfo& other) const throw();
		bool operator!= (const CurrentPositionInfo& other) const throw();

		void resetToDefault();
	};

	virtual bool getCurrentPosition (CurrentPositionInfo& result) = 0;
};

#endif   // __JUCE_AUDIOPLAYHEAD_JUCEHEADER__
/*** End of inlined file: juce_AudioPlayHead.h ***/

class JUCE_API  AudioProcessor
{
protected:

	AudioProcessor();

public:
	virtual ~AudioProcessor();

	virtual const String getName() const = 0;

	virtual void prepareToPlay (double sampleRate,
								int estimatedSamplesPerBlock) = 0;

	virtual void releaseResources() = 0;

	virtual void processBlock (AudioSampleBuffer& buffer,
							   MidiBuffer& midiMessages) = 0;

	AudioPlayHead* getPlayHead() const throw()		{ return playHead; }

	double getSampleRate() const throw()			  { return sampleRate; }

	int getBlockSize() const throw()			  { return blockSize; }

	int getNumInputChannels() const throw()		   { return numInputChannels; }

	int getNumOutputChannels() const throw()		  { return numOutputChannels; }

	virtual const String getInputChannelName (const int channelIndex) const = 0;

	virtual const String getOutputChannelName (const int channelIndex) const = 0;

	virtual bool isInputChannelStereoPair (int index) const = 0;

	virtual bool isOutputChannelStereoPair (int index) const = 0;

	int getLatencySamples() const throw()				 { return latencySamples; }

	void setLatencySamples (const int newLatency);

	virtual bool acceptsMidi() const = 0;

	virtual bool producesMidi() const = 0;

	const CriticalSection& getCallbackLock() const throw()		  { return callbackLock; }

	void suspendProcessing (const bool shouldBeSuspended);

	bool isSuspended() const throw()					{ return suspended; }

	virtual void reset();

	bool isNonRealtime() const throw()				  { return nonRealtime; }

	void setNonRealtime (const bool isNonRealtime) throw();

	virtual AudioProcessorEditor* createEditor() = 0;

	AudioProcessorEditor* getActiveEditor() const throw()		  { return activeEditor; }

	AudioProcessorEditor* createEditorIfNeeded();

	virtual int getNumParameters() = 0;

	virtual const String getParameterName (int parameterIndex) = 0;

	virtual float getParameter (int parameterIndex) = 0;

	virtual const String getParameterText (int parameterIndex) = 0;

	virtual void setParameter (int parameterIndex,
							   float newValue) = 0;

	void setParameterNotifyingHost (int parameterIndex,
									float newValue);

	virtual bool isParameterAutomatable (int parameterIndex) const;

	virtual bool isMetaParameter (int parameterIndex) const;

	void beginParameterChangeGesture (int parameterIndex);

	void endParameterChangeGesture (int parameterIndex);

	void updateHostDisplay();

	virtual int getNumPrograms() = 0;

	virtual int getCurrentProgram() = 0;

	virtual void setCurrentProgram (int index) = 0;

	virtual const String getProgramName (int index) = 0;

	virtual void changeProgramName (int index, const String& newName) = 0;

	virtual void getStateInformation (JUCE_NAMESPACE::MemoryBlock& destData) = 0;

	virtual void getCurrentProgramStateInformation (JUCE_NAMESPACE::MemoryBlock& destData);

	virtual void setStateInformation (const void* data, int sizeInBytes) = 0;

	virtual void setCurrentProgramStateInformation (const void* data, int sizeInBytes);

	void addListener (AudioProcessorListener* const newListener) throw();

	void removeListener (AudioProcessorListener* const listenerToRemove) throw();

	void editorBeingDeleted (AudioProcessorEditor* const editor) throw();

	void setPlayHead (AudioPlayHead* const newPlayHead) throw();

	void setPlayConfigDetails (const int numIns, const int numOuts,
							   const double sampleRate,
							   const int blockSize) throw();

	juce_UseDebuggingNewOperator

protected:

	static void copyXmlToBinary (const XmlElement& xml,
								 JUCE_NAMESPACE::MemoryBlock& destData);

	static XmlElement* getXmlFromBinary (const void* data,
										 const int sizeInBytes);

	AudioPlayHead* playHead;

	void sendParamChangeMessageToListeners (const int parameterIndex, const float newValue);

private:
	VoidArray listeners;
	AudioProcessorEditor* activeEditor;
	double sampleRate;
	int blockSize, numInputChannels, numOutputChannels, latencySamples;
	bool suspended, nonRealtime;
	CriticalSection callbackLock, listenerLock;

#ifdef JUCE_DEBUG
	BigInteger changingParams;
#endif

	AudioProcessor (const AudioProcessor&);
	AudioProcessor& operator= (const AudioProcessor&);
};

#endif   // __JUCE_AUDIOPROCESSOR_JUCEHEADER__
/*** End of inlined file: juce_AudioProcessor.h ***/


/*** Start of inlined file: juce_PluginDescription.h ***/
#ifndef __JUCE_PLUGINDESCRIPTION_JUCEHEADER__
#define __JUCE_PLUGINDESCRIPTION_JUCEHEADER__

class JUCE_API  PluginDescription
{
public:

	PluginDescription() throw();
	PluginDescription (const PluginDescription& other) throw();
	PluginDescription& operator= (const PluginDescription& other) throw();
	~PluginDescription() throw();

	String name;

	String pluginFormatName;

	String category;

	String manufacturerName;

	String version;

	String fileOrIdentifier;

	Time lastFileModTime;

	int uid;

	bool isInstrument;

	int numInputChannels;

	int numOutputChannels;

	bool isDuplicateOf (const PluginDescription& other) const;

	const String createIdentifierString() const throw();

	XmlElement* createXml() const;

	bool loadFromXml (const XmlElement& xml);

	juce_UseDebuggingNewOperator
};

#endif   // __JUCE_PLUGINDESCRIPTION_JUCEHEADER__
/*** End of inlined file: juce_PluginDescription.h ***/

class JUCE_API  AudioPluginInstance   : public AudioProcessor
{
public:

	virtual ~AudioPluginInstance();

	virtual void fillInPluginDescription (PluginDescription& description) const = 0;

	juce_UseDebuggingNewOperator

protected:
	AudioPluginInstance();

	AudioPluginInstance (const AudioPluginInstance&);
	AudioPluginInstance& operator= (const AudioPluginInstance&);
};

#endif   // __JUCE_AUDIOPLUGININSTANCE_JUCEHEADER__
/*** End of inlined file: juce_AudioPluginInstance.h ***/

class PluginDescription;

class JUCE_API  AudioPluginFormat
{
public:

	virtual ~AudioPluginFormat();

	virtual const String getName() const = 0;

	virtual void findAllTypesForFile (OwnedArray <PluginDescription>& results,
									  const String& fileOrIdentifier) = 0;

	virtual AudioPluginInstance* createInstanceFromDescription (const PluginDescription& desc) = 0;

	virtual bool fileMightContainThisPluginType (const String& fileOrIdentifier) = 0;

	virtual const String getNameOfPluginFromIdentifier (const String& fileOrIdentifier) = 0;

	virtual bool doesPluginStillExist (const PluginDescription& desc) = 0;

	virtual const StringArray searchPathsForPlugins (const FileSearchPath& directoriesToSearch,
													 bool recursive) = 0;

	virtual const FileSearchPath getDefaultLocationsToSearch() = 0;

	juce_UseDebuggingNewOperator

protected:
	AudioPluginFormat() throw();

	AudioPluginFormat (const AudioPluginFormat&);
	AudioPluginFormat& operator= (const AudioPluginFormat&);
};

#endif   // __JUCE_AUDIOPLUGINFORMAT_JUCEHEADER__
/*** End of inlined file: juce_AudioPluginFormat.h ***/

#if JUCE_PLUGINHOST_AU && JUCE_MAC

class JUCE_API  AudioUnitPluginFormat   : public AudioPluginFormat
{
public:

	AudioUnitPluginFormat();
	~AudioUnitPluginFormat();

	const String getName() const                { return "AudioUnit"; }
	void findAllTypesForFile (OwnedArray <PluginDescription>& results, const String& fileOrIdentifier);
	AudioPluginInstance* createInstanceFromDescription (const PluginDescription& desc);
	bool fileMightContainThisPluginType (const String& fileOrIdentifier);
	const String getNameOfPluginFromIdentifier (const String& fileOrIdentifier);
	const StringArray searchPathsForPlugins (const FileSearchPath& directoriesToSearch, bool recursive);
	bool doesPluginStillExist (const PluginDescription& desc);
	const FileSearchPath getDefaultLocationsToSearch();

	juce_UseDebuggingNewOperator

private:
	AudioUnitPluginFormat (const AudioUnitPluginFormat&);
	AudioUnitPluginFormat& operator= (const AudioUnitPluginFormat&);
};

#endif

#endif   // __JUCE_AUDIOUNITPLUGINFORMAT_JUCEHEADER__
/*** End of inlined file: juce_AudioUnitPluginFormat.h ***/


#endif
#ifndef __JUCE_DIRECTXPLUGINFORMAT_JUCEHEADER__

/*** Start of inlined file: juce_DirectXPluginFormat.h ***/
#ifndef __JUCE_DIRECTXPLUGINFORMAT_JUCEHEADER__
#define __JUCE_DIRECTXPLUGINFORMAT_JUCEHEADER__

#if JUCE_PLUGINHOST_DX && JUCE_WIN32

//   Sorry, this file is just a placeholder at the moment!...

class JUCE_API  DirectXPluginFormat   : public AudioPluginFormat
{
public:

	DirectXPluginFormat();
	~DirectXPluginFormat();

	const String getName() const                { return "DirectX"; }
	void findAllTypesForFile (OwnedArray <PluginDescription>& results, const String& fileOrIdentifier);
	AudioPluginInstance* createInstanceFromDescription (const PluginDescription& desc);
	bool fileMightContainThisPluginType (const String& fileOrIdentifier);
	const String getNameOfPluginFromIdentifier (const String& fileOrIdentifier)  { return fileOrIdentifier; }
	const FileSearchPath getDefaultLocationsToSearch();

	juce_UseDebuggingNewOperator

private:
	DirectXPluginFormat (const DirectXPluginFormat&);
	DirectXPluginFormat& operator= (const DirectXPluginFormat&);
};

#endif

#endif   // __JUCE_DIRECTXPLUGINFORMAT_JUCEHEADER__
/*** End of inlined file: juce_DirectXPluginFormat.h ***/


#endif
#ifndef __JUCE_LADSPAPLUGINFORMAT_JUCEHEADER__

/*** Start of inlined file: juce_LADSPAPluginFormat.h ***/
#ifndef __JUCE_LADSPAPLUGINFORMAT_JUCEHEADER__
#define __JUCE_LADSPAPLUGINFORMAT_JUCEHEADER__

#if JUCE_PLUGINHOST_LADSPA && JUCE_LINUX

//   Sorry, this file is just a placeholder at the moment!...

class JUCE_API  LADSPAPluginFormat   : public AudioPluginFormat
{
public:

	LADSPAPluginFormat();
	~LADSPAPluginFormat();

	const String getName() const                { return "LADSPA"; }
	void findAllTypesForFile (OwnedArray <PluginDescription>& results, const String& fileOrIdentifier);
	AudioPluginInstance* createInstanceFromDescription (const PluginDescription& desc);
	bool fileMightContainThisPluginType (const String& fileOrIdentifier);
	const String getNameOfPluginFromIdentifier (const String& fileOrIdentifier)  { return fileOrIdentifier; }
	const FileSearchPath getDefaultLocationsToSearch();

	juce_UseDebuggingNewOperator

private:
	LADSPAPluginFormat (const LADSPAPluginFormat&);
	LADSPAPluginFormat& operator= (const LADSPAPluginFormat&);
};

#endif

#endif   // __JUCE_LADSPAPLUGINFORMAT_JUCEHEADER__
/*** End of inlined file: juce_LADSPAPluginFormat.h ***/


#endif
#ifndef __JUCE_VSTMIDIEVENTLIST_JUCEHEADER__

/*** Start of inlined file: juce_VSTMidiEventList.h ***/
#ifdef __aeffect__

#ifndef __JUCE_VSTMIDIEVENTLIST_JUCEHEADER__
#define __JUCE_VSTMIDIEVENTLIST_JUCEHEADER__

class VSTMidiEventList
{
public:

	VSTMidiEventList()
		: numEventsUsed (0), numEventsAllocated (0)
	{
	}

	~VSTMidiEventList()
	{
		freeEvents();
	}

	void clear()
	{
		numEventsUsed = 0;

		if (events != 0)
			events->numEvents = 0;
	}

	void addEvent (const void* const midiData, const int numBytes, const int frameOffset)
	{
		ensureSize (numEventsUsed + 1);

		VstMidiEvent* const e = (VstMidiEvent*) (events->events [numEventsUsed]);
		events->numEvents = ++numEventsUsed;

		if (numBytes <= 4)
		{
			if (e->type == kVstSysExType)
			{
				juce_free (((VstMidiSysexEvent*) e)->sysexDump);
				e->type = kVstMidiType;
				e->byteSize = sizeof (VstMidiEvent);
				e->noteLength = 0;
				e->noteOffset = 0;
				e->detune = 0;
				e->noteOffVelocity = 0;
			}

			e->deltaFrames = frameOffset;
			memcpy (e->midiData, midiData, numBytes);
		}
		else
		{
			VstMidiSysexEvent* const se = (VstMidiSysexEvent*) e;

			if (se->type == kVstSysExType)
				se->sysexDump = (char*) juce_realloc (se->sysexDump, numBytes);
			else
				se->sysexDump = (char*) juce_malloc (numBytes);

			memcpy (se->sysexDump, midiData, numBytes);

			se->type = kVstSysExType;
			se->byteSize = sizeof (VstMidiSysexEvent);
			se->deltaFrames = frameOffset;
			se->flags = 0;
			se->dumpBytes = numBytes;
			se->resvd1 = 0;
			se->resvd2 = 0;
		}
	}

	// Handy method to pull the events out of an event buffer supplied by the host
	// or plugin.
	static void addEventsToMidiBuffer (const VstEvents* events, MidiBuffer& dest)
	{
		for (int i = 0; i < events->numEvents; ++i)
		{
			const VstEvent* const e = events->events[i];

			if (e != 0)
			{
				if (e->type == kVstMidiType)
				{
					dest.addEvent ((const JUCE_NAMESPACE::uint8*) ((const VstMidiEvent*) e)->midiData,
								   4, e->deltaFrames);
				}
				else if (e->type == kVstSysExType)
				{
					dest.addEvent ((const JUCE_NAMESPACE::uint8*) ((const VstMidiSysexEvent*) e)->sysexDump,
								   (int) ((const VstMidiSysexEvent*) e)->dumpBytes,
								   e->deltaFrames);
				}
			}
		}
	}

	void ensureSize (int numEventsNeeded)
	{
		if (numEventsNeeded > numEventsAllocated)
		{
			numEventsNeeded = (numEventsNeeded + 32) & ~31;

			const int size = 20 + sizeof (VstEvent*) * numEventsNeeded;

			if (events == 0)
				events.calloc (size, 1);
			else
				events.realloc (size, 1);

			for (int i = numEventsAllocated; i < numEventsNeeded; ++i)
			{
				VstMidiEvent* const e = (VstMidiEvent*) juce_calloc (jmax ((int) sizeof (VstMidiEvent),
																		   (int) sizeof (VstMidiSysexEvent)));
				e->type = kVstMidiType;
				e->byteSize = sizeof (VstMidiEvent);

				events->events[i] = (VstEvent*) e;
			}

			numEventsAllocated = numEventsNeeded;
		}
	}

	void freeEvents()
	{
		if (events != 0)
		{
			for (int i = numEventsAllocated; --i >= 0;)
			{
				VstMidiEvent* const e = (VstMidiEvent*) (events->events[i]);

				if (e->type == kVstSysExType)
					juce_free (((VstMidiSysexEvent*) e)->sysexDump);

				juce_free (e);
			}

			events.free();
			numEventsUsed = 0;
			numEventsAllocated = 0;
		}
	}

	HeapBlock <VstEvents> events;

private:
	int numEventsUsed, numEventsAllocated;
};

#endif   // __JUCE_VSTMIDIEVENTLIST_JUCEHEADER__
#endif   // __JUCE_VSTMIDIEVENTLIST_JUCEHEADER__
/*** End of inlined file: juce_VSTMidiEventList.h ***/


#endif
#ifndef __JUCE_VSTPLUGINFORMAT_JUCEHEADER__

/*** Start of inlined file: juce_VSTPluginFormat.h ***/
#ifndef __JUCE_VSTPLUGINFORMAT_JUCEHEADER__
#define __JUCE_VSTPLUGINFORMAT_JUCEHEADER__

#if JUCE_PLUGINHOST_VST

class JUCE_API  VSTPluginFormat   : public AudioPluginFormat
{
public:

	VSTPluginFormat();
	~VSTPluginFormat();

	const String getName() const                { return "VST"; }
	void findAllTypesForFile (OwnedArray <PluginDescription>& results, const String& fileOrIdentifier);
	AudioPluginInstance* createInstanceFromDescription (const PluginDescription& desc);
	bool fileMightContainThisPluginType (const String& fileOrIdentifier);
	const String getNameOfPluginFromIdentifier (const String& fileOrIdentifier);
	const StringArray searchPathsForPlugins (const FileSearchPath& directoriesToSearch, bool recursive);
	bool doesPluginStillExist (const PluginDescription& desc);
	const FileSearchPath getDefaultLocationsToSearch();

	juce_UseDebuggingNewOperator

private:
	VSTPluginFormat (const VSTPluginFormat&);
	VSTPluginFormat& operator= (const VSTPluginFormat&);

	void recursiveFileSearch (StringArray& results, const File& dir, const bool recursive);
};

#endif
#endif   // __JUCE_VSTPLUGINFORMAT_JUCEHEADER__
/*** End of inlined file: juce_VSTPluginFormat.h ***/


#endif
#ifndef __JUCE_AUDIOPLUGINFORMAT_JUCEHEADER__

#endif
#ifndef __JUCE_AUDIOPLUGINFORMATMANAGER_JUCEHEADER__

/*** Start of inlined file: juce_AudioPluginFormatManager.h ***/
#ifndef __JUCE_AUDIOPLUGINFORMATMANAGER_JUCEHEADER__
#define __JUCE_AUDIOPLUGINFORMATMANAGER_JUCEHEADER__

class JUCE_API  AudioPluginFormatManager  : public DeletedAtShutdown
{
public:

	AudioPluginFormatManager() throw();

	~AudioPluginFormatManager() throw();

	juce_DeclareSingleton_SingleThreaded (AudioPluginFormatManager, false);

	void addDefaultFormats();

	int getNumFormats() throw();

	AudioPluginFormat* getFormat (const int index) throw();

	void addFormat (AudioPluginFormat* const format) throw();

	AudioPluginInstance* createPluginInstance (const PluginDescription& description,
											   String& errorMessage) const;

	bool doesPluginStillExist (const PluginDescription& description) const;

	juce_UseDebuggingNewOperator

private:
	OwnedArray <AudioPluginFormat> formats;

	AudioPluginFormatManager (const AudioPluginFormatManager&);
	AudioPluginFormatManager& operator= (const AudioPluginFormatManager&);
};

#endif   // __JUCE_AUDIOPLUGINFORMATMANAGER_JUCEHEADER__
/*** End of inlined file: juce_AudioPluginFormatManager.h ***/


#endif
#ifndef __JUCE_AUDIOPLUGININSTANCE_JUCEHEADER__

#endif
#ifndef __JUCE_KNOWNPLUGINLIST_JUCEHEADER__

/*** Start of inlined file: juce_KnownPluginList.h ***/
#ifndef __JUCE_KNOWNPLUGINLIST_JUCEHEADER__
#define __JUCE_KNOWNPLUGINLIST_JUCEHEADER__

class JUCE_API  KnownPluginList   : public ChangeBroadcaster
{
public:

	KnownPluginList();

	~KnownPluginList();

	void clear();

	int getNumTypes() const throw()				 { return types.size(); }

	PluginDescription* getType (int index) const throw()		{ return types [index]; }

	PluginDescription* getTypeForFile (const String& fileOrIdentifier) const throw();

	PluginDescription* getTypeForIdentifierString (const String& identifierString) const throw();

	bool addType (const PluginDescription& type);

	void removeType (int index) throw();

	bool scanAndAddFile (const String& possiblePluginFileOrIdentifier,
						 bool dontRescanIfAlreadyInList,
						 OwnedArray <PluginDescription>& typesFound,
						 AudioPluginFormat& formatToUse);

	bool isListingUpToDate (const String& possiblePluginFileOrIdentifier) const throw();

	void scanAndAddDragAndDroppedFiles (const StringArray& filenames,
										OwnedArray <PluginDescription>& typesFound);

	enum SortMethod
	{
		defaultOrder = 0,
		sortAlphabetically,
		sortByCategory,
		sortByManufacturer,
		sortByFileSystemLocation
	};

	void addToMenu (PopupMenu& menu,
					const SortMethod sortMethod) const;

	int getIndexChosenByMenu (int menuResultCode) const;

	void sort (const SortMethod method);

	XmlElement* createXml() const;

	void recreateFromXml (const XmlElement& xml);

	juce_UseDebuggingNewOperator

private:
	OwnedArray <PluginDescription> types;

	KnownPluginList (const KnownPluginList&);
	KnownPluginList& operator= (const KnownPluginList&);
};

#endif   // __JUCE_KNOWNPLUGINLIST_JUCEHEADER__
/*** End of inlined file: juce_KnownPluginList.h ***/


#endif
#ifndef __JUCE_PLUGINDESCRIPTION_JUCEHEADER__

#endif
#ifndef __JUCE_PLUGINDIRECTORYSCANNER_JUCEHEADER__

/*** Start of inlined file: juce_PluginDirectoryScanner.h ***/
#ifndef __JUCE_PLUGINDIRECTORYSCANNER_JUCEHEADER__
#define __JUCE_PLUGINDIRECTORYSCANNER_JUCEHEADER__

class JUCE_API  PluginDirectoryScanner
{
public:

	PluginDirectoryScanner (KnownPluginList& listToAddResultsTo,
							AudioPluginFormat& formatToLookFor,
							FileSearchPath directoriesToSearch,
							bool searchRecursively,
							const File& deadMansPedalFile);

	~PluginDirectoryScanner();

	bool scanNextFile (bool dontRescanIfAlreadyInList);

	const String getNextPluginFileThatWillBeScanned() const throw();

	float getProgress() const					   { return progress; }

	const StringArray& getFailedFiles() const throw()		   { return failedFiles; }

	juce_UseDebuggingNewOperator

private:
	KnownPluginList& list;
	AudioPluginFormat& format;
	StringArray filesOrIdentifiersToScan;
	File deadMansPedalFile;
	StringArray failedFiles;
	int nextIndex;
	float progress;

	const StringArray getDeadMansPedalFile() throw();
	void setDeadMansPedalFile (const StringArray& newContents) throw();

	PluginDirectoryScanner (const PluginDirectoryScanner&);
	PluginDirectoryScanner& operator= (const PluginDirectoryScanner&);
};

#endif   // __JUCE_PLUGINDIRECTORYSCANNER_JUCEHEADER__
/*** End of inlined file: juce_PluginDirectoryScanner.h ***/


#endif
#ifndef __JUCE_PLUGINLISTCOMPONENT_JUCEHEADER__

/*** Start of inlined file: juce_PluginListComponent.h ***/
#ifndef __JUCE_PLUGINLISTCOMPONENT_JUCEHEADER__
#define __JUCE_PLUGINLISTCOMPONENT_JUCEHEADER__


/*** Start of inlined file: juce_ListBox.h ***/
#ifndef __JUCE_LISTBOX_JUCEHEADER__
#define __JUCE_LISTBOX_JUCEHEADER__

class ListViewport;

class JUCE_API  ListBoxModel
{
public:

	virtual ~ListBoxModel()  {}

	virtual int getNumRows() = 0;

	virtual void paintListBoxItem (int rowNumber,
								   Graphics& g,
								   int width, int height,
								   bool rowIsSelected) = 0;

	virtual Component* refreshComponentForRow (int rowNumber, bool isRowSelected,
											   Component* existingComponentToUpdate);

	virtual void listBoxItemClicked (int row, const MouseEvent& e);

	virtual void listBoxItemDoubleClicked (int row, const MouseEvent& e);

	virtual void backgroundClicked();

	virtual void selectedRowsChanged (int lastRowSelected);

	virtual void deleteKeyPressed (int lastRowSelected);

	virtual void returnKeyPressed (int lastRowSelected);

	virtual void listWasScrolled();

	virtual const String getDragSourceDescription (const SparseSet<int>& currentlySelectedRows);

	virtual const String getTooltipForRow (int row);
};

class JUCE_API  ListBox  : public Component,
						   public SettableTooltipClient
{
public:

	ListBox (const String& componentName,
			 ListBoxModel* model);

	~ListBox();

	void setModel (ListBoxModel* newModel);

	ListBoxModel* getModel() const throw()			  { return model; }

	void updateContent();

	void setMultipleSelectionEnabled (bool shouldBeEnabled);

	void setMouseMoveSelectsRows (bool shouldSelect);

	void selectRow (int rowNumber,
					bool dontScrollToShowThisRow = false,
					bool deselectOthersFirst = true);

	void selectRangeOfRows (int firstRow,
							int lastRow);

	void deselectRow (int rowNumber);

	void deselectAllRows();

	void flipRowSelection (int rowNumber);

	const SparseSet<int> getSelectedRows() const;

	void setSelectedRows (const SparseSet<int>& setOfRowsToBeSelected,
						  bool sendNotificationEventToModel = true);

	bool isRowSelected (int rowNumber) const;

	int getNumSelectedRows() const;

	int getSelectedRow (int index = 0) const;

	int getLastRowSelected() const;

	void selectRowsBasedOnModifierKeys (int rowThatWasClickedOn,
										const ModifierKeys& modifiers);

	void setVerticalPosition (double newProportion);

	double getVerticalPosition() const;

	void scrollToEnsureRowIsOnscreen (int row);

	ScrollBar* getVerticalScrollBar() const throw();

	ScrollBar* getHorizontalScrollBar() const throw();

	int getRowContainingPosition (int x, int y) const throw();

	int getInsertionIndexForPosition (int x, int y) const throw();

	const Rectangle<int> getRowPosition (int rowNumber,
										 bool relativeToComponentTopLeft) const throw();

	Component* getComponentForRowNumber (int rowNumber) const throw();

	int getRowNumberOfComponent (Component* rowComponent) const throw();

	int getVisibleRowWidth() const throw();

	void setRowHeight (int newHeight);

	int getRowHeight() const throw()   { return rowHeight; }

	int getNumRowsOnScreen() const throw();

	enum ColourIds
	{
		backgroundColourId	  = 0x1002800, /**< The background colour to fill the list with.
												  Make this transparent if you don't want the background to be filled. */
		outlineColourId	 = 0x1002810, /**< An optional colour to use to draw a border around the list.
												  Make this transparent to not have an outline. */
		textColourId		= 0x1002820  /**< The preferred colour to use for drawing text in the listbox. */
	};

	void setOutlineThickness (int outlineThickness);

	int getOutlineThickness() const throw()	{ return outlineThickness; }

	void setHeaderComponent (Component* newHeaderComponent);

	void setMinimumContentWidth (int newMinimumWidth);

	int getVisibleContentWidth() const throw();

	void repaintRow (int rowNumber) throw();

	Image* createSnapshotOfSelectedRows (int& x, int& y);

	Viewport* getViewport() const throw();

	bool keyPressed (const KeyPress& key);
	bool keyStateChanged (bool isKeyDown);
	void paint (Graphics& g);
	void paintOverChildren (Graphics& g);
	void resized();
	void visibilityChanged();
	void mouseWheelMove (const MouseEvent& e, float wheelIncrementX, float wheelIncrementY);
	void mouseMove (const MouseEvent&);
	void mouseExit (const MouseEvent&);
	void mouseUp (const MouseEvent&);
	void colourChanged();
	void startDragAndDrop (const MouseEvent& e, const String& dragDescription);

	juce_UseDebuggingNewOperator

private:

	friend class ListViewport;
	friend class TableListBox;
	ListBoxModel* model;
	ListViewport* viewport;
	Component* headerComponent;
	int totalItems, rowHeight, minimumRowWidth;
	int outlineThickness;
	int lastRowSelected;
	bool mouseMoveSelects, multipleSelection, hasDoneInitialUpdate;
	SparseSet <int> selected;

	void selectRowInternal (int rowNumber,
							bool dontScrollToShowThisRow,
							bool deselectOthersFirst,
							bool isMouseClick);

	ListBox (const ListBox&);
	ListBox& operator= (const ListBox&);
};

#endif   // __JUCE_LISTBOX_JUCEHEADER__
/*** End of inlined file: juce_ListBox.h ***/


/*** Start of inlined file: juce_TextButton.h ***/
#ifndef __JUCE_TEXTBUTTON_JUCEHEADER__
#define __JUCE_TEXTBUTTON_JUCEHEADER__

class JUCE_API  TextButton  : public Button
{
public:

	TextButton (const String& buttonName,
				const String& toolTip = String::empty);

	~TextButton();

	enum ColourIds
	{
		buttonColourId		  = 0x1000100,  /**< The colour used to fill the button shape (when the button is toggled
														   'off'). The look-and-feel class might re-interpret this to add
														   effects, etc. */
		buttonOnColourId		= 0x1000101,  /**< The colour used to fill the button shape (when the button is toggled
														   'on'). The look-and-feel class might re-interpret this to add
														   effects, etc. */
		textColourOffId                 = 0x1000102,  /**< The colour to use for the button's text when the button's toggle state is "off". */
		textColourOnId                  = 0x1000103   /**< The colour to use for the button's text.when the button's toggle state is "on". */
	};

	void changeWidthToFitText (int newHeight = -1);

	virtual const Font getFont();

	juce_UseDebuggingNewOperator

protected:
	void paintButton (Graphics& g, bool isMouseOverButton, bool isButtonDown);
	void colourChanged();

private:
	TextButton (const TextButton&);
	TextButton& operator= (const TextButton&);
};

#endif   // __JUCE_TEXTBUTTON_JUCEHEADER__
/*** End of inlined file: juce_TextButton.h ***/

class JUCE_API  PluginListComponent   : public Component,
										public ListBoxModel,
										public ChangeListener,
										public ButtonListener,
										public Timer
{
public:

	PluginListComponent (KnownPluginList& listToRepresent,
						 const File& deadMansPedalFile,
						 PropertiesFile* propertiesToUse);

	~PluginListComponent();

	void resized();
	bool isInterestedInFileDrag (const StringArray& files);
	void filesDropped (const StringArray& files, int, int);
	int getNumRows();
	void paintListBoxItem (int row, Graphics& g, int width, int height, bool rowIsSelected);
	void deleteKeyPressed (int lastRowSelected);
	void buttonClicked (Button* b);
	void changeListenerCallback (void*);
	void timerCallback();

	juce_UseDebuggingNewOperator

private:
	KnownPluginList& list;
	File deadMansPedalFile;
	ListBox* listBox;
	TextButton* optionsButton;
	PropertiesFile* propertiesToUse;
	int typeToScan;

	void scanFor (AudioPluginFormat* format);

	PluginListComponent (const PluginListComponent&);
	PluginListComponent& operator= (const PluginListComponent&);
};

#endif   // __JUCE_PLUGINLISTCOMPONENT_JUCEHEADER__
/*** End of inlined file: juce_PluginListComponent.h ***/


#endif
#ifndef __JUCE_AUDIOPLAYHEAD_JUCEHEADER__

#endif
#ifndef __JUCE_AUDIOPROCESSOR_JUCEHEADER__

#endif
#ifndef __JUCE_AUDIOPROCESSOREDITOR_JUCEHEADER__

#endif
#ifndef __JUCE_AUDIOPROCESSORGRAPH_JUCEHEADER__

/*** Start of inlined file: juce_AudioProcessorGraph.h ***/
#ifndef __JUCE_AUDIOPROCESSORGRAPH_JUCEHEADER__
#define __JUCE_AUDIOPROCESSORGRAPH_JUCEHEADER__

class JUCE_API  AudioProcessorGraph   : public AudioProcessor,
										public AsyncUpdater
{
public:

	AudioProcessorGraph();

	~AudioProcessorGraph();

	class JUCE_API  Node   : public ReferenceCountedObject
	{
	public:
		~Node();

		const uint32 id;

		AudioProcessor* const processor;

		NamedValueSet properties;

		typedef ReferenceCountedObjectPtr <Node> Ptr;

		juce_UseDebuggingNewOperator

	private:
		friend class AudioProcessorGraph;

		bool isPrepared;

		Node (uint32 id, AudioProcessor* processor);

		void prepare (double sampleRate, int blockSize, AudioProcessorGraph* graph);
		void unprepare();

		Node (const Node&);
		Node& operator= (const Node&);
	};

	struct JUCE_API  Connection
	{
	public:

		uint32 sourceNodeId;

		int sourceChannelIndex;

		uint32 destNodeId;

		int destChannelIndex;

		juce_UseDebuggingNewOperator

	private:
	};

	void clear();

	int getNumNodes() const					 { return nodes.size(); }

	Node* getNode (const int index) const			   { return nodes [index]; }

	Node* getNodeForId (const uint32 nodeId) const;

	Node* addNode (AudioProcessor* newProcessor, uint32 nodeId = 0);

	bool removeNode (uint32 nodeId);

	int getNumConnections() const					   { return connections.size(); }

	const Connection* getConnection (int index) const		   { return connections [index]; }

	const Connection* getConnectionBetween (uint32 sourceNodeId,
											int sourceChannelIndex,
											uint32 destNodeId,
											int destChannelIndex) const;

	bool isConnected (uint32 possibleSourceNodeId,
					  uint32 possibleDestNodeId) const;

	bool canConnect (uint32 sourceNodeId, int sourceChannelIndex,
					 uint32 destNodeId, int destChannelIndex) const;

	bool addConnection (uint32 sourceNodeId, int sourceChannelIndex,
						uint32 destNodeId, int destChannelIndex);

	void removeConnection (int index);

	bool removeConnection (uint32 sourceNodeId, int sourceChannelIndex,
						   uint32 destNodeId, int destChannelIndex);

	bool disconnectNode (uint32 nodeId);

	bool removeIllegalConnections();

	static const int midiChannelIndex;

	class JUCE_API  AudioGraphIOProcessor	 : public AudioPluginInstance
	{
	public:
		enum IODeviceType
		{
			audioInputNode,	 /**< In this mode, the processor has output channels
									 representing all the audio input channels that are
									 coming into its parent audio graph. */
			audioOutputNode,	/**< In this mode, the processor has input channels
									 representing all the audio output channels that are
									 going out of its parent audio graph. */
			midiInputNode,	  /**< In this mode, the processor has a midi output which
									 delivers the same midi data that is arriving at its
									 parent graph. */
			midiOutputNode	  /**< In this mode, the processor has a midi input and
									 any data sent to it will be passed out of the parent
									 graph. */
		};

		IODeviceType getType() const				{ return type; }

		AudioProcessorGraph* getParentGraph() const		 { return graph; }

		bool isInput() const;
		bool isOutput() const;

		AudioGraphIOProcessor (const IODeviceType type);
		~AudioGraphIOProcessor();

		const String getName() const;
		void fillInPluginDescription (PluginDescription& d) const;

		void prepareToPlay (double sampleRate, int estimatedSamplesPerBlock);
		void releaseResources();
		void processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages);

		const String getInputChannelName (const int channelIndex) const;
		const String getOutputChannelName (const int channelIndex) const;
		bool isInputChannelStereoPair (int index) const;
		bool isOutputChannelStereoPair (int index) const;
		bool acceptsMidi() const;
		bool producesMidi() const;

		AudioProcessorEditor* createEditor();

		int getNumParameters();
		const String getParameterName (int);
		float getParameter (int);
		const String getParameterText (int);
		void setParameter (int, float);

		int getNumPrograms();
		int getCurrentProgram();
		void setCurrentProgram (int);
		const String getProgramName (int);
		void changeProgramName (int, const String&);

		void getStateInformation (JUCE_NAMESPACE::MemoryBlock& destData);
		void setStateInformation (const void* data, int sizeInBytes);

		void setParentGraph (AudioProcessorGraph* graph);

		juce_UseDebuggingNewOperator

	private:
		const IODeviceType type;
		AudioProcessorGraph* graph;

		AudioGraphIOProcessor (const AudioGraphIOProcessor&);
		AudioGraphIOProcessor& operator= (const AudioGraphIOProcessor&);
	};

	// AudioProcessor methods:

	const String getName() const;

	void prepareToPlay (double sampleRate, int estimatedSamplesPerBlock);
	void releaseResources();
	void processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages);

	const String getInputChannelName (const int channelIndex) const;
	const String getOutputChannelName (const int channelIndex) const;
	bool isInputChannelStereoPair (int index) const;
	bool isOutputChannelStereoPair (int index) const;

	bool acceptsMidi() const;
	bool producesMidi() const;

	AudioProcessorEditor* createEditor()		{ return 0; }

	int getNumParameters()			  { return 0; }
	const String getParameterName (int)		 { return String::empty; }
	float getParameter (int)			{ return 0; }
	const String getParameterText (int)		 { return String::empty; }
	void setParameter (int, float)		  { }

	int getNumPrograms()				{ return 0; }
	int getCurrentProgram()			 { return 0; }
	void setCurrentProgram (int)			{ }
	const String getProgramName (int)		   { return String::empty; }
	void changeProgramName (int, const String&)	 { }

	void getStateInformation (JUCE_NAMESPACE::MemoryBlock& destData);
	void setStateInformation (const void* data, int sizeInBytes);

	void handleAsyncUpdate();

	juce_UseDebuggingNewOperator

private:
	ReferenceCountedArray <Node> nodes;
	OwnedArray <Connection> connections;
	int lastNodeId;
	AudioSampleBuffer renderingBuffers;
	OwnedArray <MidiBuffer> midiBuffers;

	CriticalSection renderLock;
	VoidArray renderingOps;

	friend class AudioGraphIOProcessor;
	AudioSampleBuffer* currentAudioInputBuffer;
	AudioSampleBuffer currentAudioOutputBuffer;
	MidiBuffer* currentMidiInputBuffer;
	MidiBuffer currentMidiOutputBuffer;

	void clearRenderingSequence();
	void buildRenderingSequence();

	bool isAnInputTo (uint32 possibleInputId, uint32 possibleDestinationId, int recursionCheck) const;

	AudioProcessorGraph (const AudioProcessorGraph&);
	AudioProcessorGraph& operator= (const AudioProcessorGraph&);
};

#endif   // __JUCE_AUDIOPROCESSORGRAPH_JUCEHEADER__
/*** End of inlined file: juce_AudioProcessorGraph.h ***/


#endif
#ifndef __JUCE_AUDIOPROCESSORLISTENER_JUCEHEADER__

#endif
#ifndef __JUCE_AUDIOPROCESSORPLAYER_JUCEHEADER__

/*** Start of inlined file: juce_AudioProcessorPlayer.h ***/
#ifndef __JUCE_AUDIOPROCESSORPLAYER_JUCEHEADER__
#define __JUCE_AUDIOPROCESSORPLAYER_JUCEHEADER__

class JUCE_API  AudioProcessorPlayer	: public AudioIODeviceCallback,
										  public MidiInputCallback
{
public:

	AudioProcessorPlayer();

	virtual ~AudioProcessorPlayer();

	void setProcessor (AudioProcessor* const processorToPlay);

	AudioProcessor* getCurrentProcessor() const			 { return processor; }

	MidiMessageCollector& getMidiMessageCollector()		 { return messageCollector; }

	void audioDeviceIOCallback (const float** inputChannelData,
								int totalNumInputChannels,
								float** outputChannelData,
								int totalNumOutputChannels,
								int numSamples);
	void audioDeviceAboutToStart (AudioIODevice* device);
	void audioDeviceStopped();
	void handleIncomingMidiMessage (MidiInput* source, const MidiMessage& message);

	juce_UseDebuggingNewOperator

private:
	AudioProcessor* processor;
	CriticalSection lock;
	double sampleRate;
	int blockSize;
	bool isPrepared;

	int numInputChans, numOutputChans;
	float* channels [128];
	AudioSampleBuffer tempBuffer;

	MidiBuffer incomingMidi;
	MidiMessageCollector messageCollector;

	AudioProcessorPlayer (const AudioProcessorPlayer&);
	AudioProcessorPlayer& operator= (const AudioProcessorPlayer&);
};

#endif   // __JUCE_AUDIOPROCESSORPLAYER_JUCEHEADER__
/*** End of inlined file: juce_AudioProcessorPlayer.h ***/


#endif
#ifndef __JUCE_GENERICAUDIOPROCESSOREDITOR_JUCEHEADER__

/*** Start of inlined file: juce_GenericAudioProcessorEditor.h ***/
#ifndef __JUCE_GENERICAUDIOPROCESSOREDITOR_JUCEHEADER__
#define __JUCE_GENERICAUDIOPROCESSOREDITOR_JUCEHEADER__


/*** Start of inlined file: juce_PropertyPanel.h ***/
#ifndef __JUCE_PROPERTYPANEL_JUCEHEADER__
#define __JUCE_PROPERTYPANEL_JUCEHEADER__


/*** Start of inlined file: juce_PropertyComponent.h ***/
#ifndef __JUCE_PROPERTYCOMPONENT_JUCEHEADER__
#define __JUCE_PROPERTYCOMPONENT_JUCEHEADER__

class EditableProperty;

class JUCE_API  PropertyComponent  : public Component,
									 public SettableTooltipClient
{
public:

	PropertyComponent (const String& propertyName,
					   int preferredHeight = 25);

	~PropertyComponent();

	int getPreferredHeight() const throw()		  { return preferredHeight; }

	void setPreferredHeight (int newHeight) throw()	 { preferredHeight = newHeight; }

	virtual void refresh() = 0;

	void paint (Graphics& g);

	void resized();

	void enablementChanged();

	juce_UseDebuggingNewOperator

protected:
	int preferredHeight;
};

#endif   // __JUCE_PROPERTYCOMPONENT_JUCEHEADER__
/*** End of inlined file: juce_PropertyComponent.h ***/

class JUCE_API  PropertyPanel  : public Component
{
public:

	PropertyPanel();

	~PropertyPanel();

	void clear();

	void addProperties (const Array <PropertyComponent*>& newPropertyComponents);

	void addSection (const String& sectionTitle,
					 const Array <PropertyComponent*>& newPropertyComponents,
					 bool shouldSectionInitiallyBeOpen = true);

	void refreshAll() const;

	const StringArray getSectionNames() const;

	bool isSectionOpen (int sectionIndex) const;

	void setSectionOpen (int sectionIndex, bool shouldBeOpen);

	void setSectionEnabled (int sectionIndex, bool shouldBeEnabled);

	XmlElement* getOpennessState() const;

	void restoreOpennessState (const XmlElement& newState);

	void setMessageWhenEmpty (const String& newMessage);

	const String& getMessageWhenEmpty() const;

	void paint (Graphics& g);
	void resized();

	juce_UseDebuggingNewOperator

private:
	Viewport* viewport;
	Component* propertyHolderComponent;
	String messageWhenEmpty;

	void updatePropHolderLayout() const;
	void updatePropHolderLayout (int width) const;
};

#endif   // __JUCE_PROPERTYPANEL_JUCEHEADER__
/*** End of inlined file: juce_PropertyPanel.h ***/

class JUCE_API  GenericAudioProcessorEditor	  : public AudioProcessorEditor
{
public:

	GenericAudioProcessorEditor (AudioProcessor* const owner);
	~GenericAudioProcessorEditor();

	void paint (Graphics& g);
	void resized();

	juce_UseDebuggingNewOperator

private:
	PropertyPanel* panel;

	GenericAudioProcessorEditor (const GenericAudioProcessorEditor&);
	GenericAudioProcessorEditor& operator= (const GenericAudioProcessorEditor&);
};

#endif   // __JUCE_GENERICAUDIOPROCESSOREDITOR_JUCEHEADER__
/*** End of inlined file: juce_GenericAudioProcessorEditor.h ***/


#endif
#ifndef __JUCE_SAMPLER_JUCEHEADER__

/*** Start of inlined file: juce_Sampler.h ***/
#ifndef __JUCE_SAMPLER_JUCEHEADER__
#define __JUCE_SAMPLER_JUCEHEADER__


/*** Start of inlined file: juce_Synthesiser.h ***/
#ifndef __JUCE_SYNTHESISER_JUCEHEADER__
#define __JUCE_SYNTHESISER_JUCEHEADER__

class JUCE_API  SynthesiserSound	: public ReferenceCountedObject
{
protected:

	SynthesiserSound();

public:
	virtual ~SynthesiserSound();

	virtual bool appliesToNote (const int midiNoteNumber) = 0;

	virtual bool appliesToChannel (const int midiChannel) = 0;

	typedef ReferenceCountedObjectPtr <SynthesiserSound> Ptr;

	juce_UseDebuggingNewOperator
};

class JUCE_API  SynthesiserVoice
{
public:

	SynthesiserVoice();

	virtual ~SynthesiserVoice();

	int getCurrentlyPlayingNote() const					 { return currentlyPlayingNote; }

	const SynthesiserSound::Ptr getCurrentlyPlayingSound() const		{ return currentlyPlayingSound; }

	virtual bool canPlaySound (SynthesiserSound* sound) = 0;

	virtual void startNote (const int midiNoteNumber,
							const float velocity,
							SynthesiserSound* sound,
							const int currentPitchWheelPosition) = 0;

	virtual void stopNote (const bool allowTailOff) = 0;

	virtual void pitchWheelMoved (const int newValue) = 0;

	virtual void controllerMoved (const int controllerNumber,
								  const int newValue) = 0;

	virtual void renderNextBlock (AudioSampleBuffer& outputBuffer,
								  int startSample,
								  int numSamples) = 0;

	bool isPlayingChannel (int midiChannel) const;

	void setCurrentPlaybackSampleRate (double newRate);

	juce_UseDebuggingNewOperator

protected:

	double getSampleRate() const				{ return currentSampleRate; }

	void clearCurrentNote();

private:

	friend class Synthesiser;

	double currentSampleRate;
	int currentlyPlayingNote;
	uint32 noteOnTime;
	SynthesiserSound::Ptr currentlyPlayingSound;
};

class JUCE_API  Synthesiser
{
public:

	Synthesiser();

	virtual ~Synthesiser();

	void clearVoices();

	int getNumVoices() const					{ return voices.size(); }

	SynthesiserVoice* getVoice (int index) const;

	void addVoice (SynthesiserVoice* newVoice);

	void removeVoice (int index);

	void clearSounds();

	int getNumSounds() const					{ return sounds.size(); }

	SynthesiserSound* getSound (int index) const			{ return sounds [index]; }

	void addSound (const SynthesiserSound::Ptr& newSound);

	void removeSound (int index);

	void setNoteStealingEnabled (bool shouldStealNotes);

	bool isNoteStealingEnabled() const				  { return shouldStealNotes; }

	virtual void noteOn (const int midiChannel,
						 const int midiNoteNumber,
						 const float velocity);

	virtual void noteOff (const int midiChannel,
						  const int midiNoteNumber,
						  const bool allowTailOff);

	virtual void allNotesOff (const int midiChannel,
							  const bool allowTailOff);

	virtual void handlePitchWheel (const int midiChannel,
								   const int wheelValue);

	virtual void handleController (const int midiChannel,
								   const int controllerNumber,
								   const int controllerValue);

	void setCurrentPlaybackSampleRate (const double sampleRate);

	void renderNextBlock (AudioSampleBuffer& outputAudio,
						  const MidiBuffer& inputMidi,
						  int startSample,
						  int numSamples);

	juce_UseDebuggingNewOperator

protected:

	CriticalSection lock;

	OwnedArray <SynthesiserVoice> voices;
	ReferenceCountedArray <SynthesiserSound> sounds;

	int lastPitchWheelValues [16];

	virtual SynthesiserVoice* findFreeVoice (SynthesiserSound* soundToPlay,
											 const bool stealIfNoneAvailable) const;

	void startVoice (SynthesiserVoice* voice,
					 SynthesiserSound* sound,
					 int midiChannel,
					 int midiNoteNumber,
					 float velocity);

	int findFreeVoice (const bool) const { return 0; }

private:
	double sampleRate;
	uint32 lastNoteOnCounter;
	bool shouldStealNotes;

	Synthesiser (const Synthesiser&);
	Synthesiser& operator= (const Synthesiser&);
};

#endif   // __JUCE_SYNTHESISER_JUCEHEADER__
/*** End of inlined file: juce_Synthesiser.h ***/

class JUCE_API  SamplerSound	: public SynthesiserSound
{
public:

	SamplerSound (const String& name,
				  AudioFormatReader& source,
				  const BigInteger& midiNotes,
				  int midiNoteForNormalPitch,
				  double attackTimeSecs,
				  double releaseTimeSecs,
				  double maxSampleLengthSeconds);

	~SamplerSound();

	const String& getName() const			   { return name; }

	AudioSampleBuffer* getAudioData() const		 { return data; }

	bool appliesToNote (const int midiNoteNumber);
	bool appliesToChannel (const int midiChannel);

	juce_UseDebuggingNewOperator

private:
	friend class SamplerVoice;

	String name;
	ScopedPointer <AudioSampleBuffer> data;
	double sourceSampleRate;
	BigInteger midiNotes;
	int length, attackSamples, releaseSamples;
	int midiRootNote;
};

class JUCE_API  SamplerVoice	: public SynthesiserVoice
{
public:

	SamplerVoice();

	~SamplerVoice();

	bool canPlaySound (SynthesiserSound* sound);

	void startNote (const int midiNoteNumber,
					const float velocity,
					SynthesiserSound* sound,
					const int currentPitchWheelPosition);

	void stopNote (const bool allowTailOff);

	void pitchWheelMoved (const int newValue);
	void controllerMoved (const int controllerNumber,
						  const int newValue);

	void renderNextBlock (AudioSampleBuffer& outputBuffer, int startSample, int numSamples);

	juce_UseDebuggingNewOperator

private:
	double pitchRatio;
	double sourceSamplePosition;
	float lgain, rgain, attackReleaseLevel, attackDelta, releaseDelta;
	bool isInAttack, isInRelease;
};

#endif   // __JUCE_SAMPLER_JUCEHEADER__
/*** End of inlined file: juce_Sampler.h ***/


#endif
#ifndef __JUCE_SYNTHESISER_JUCEHEADER__

#endif
#ifndef __JUCE_ACTIONBROADCASTER_JUCEHEADER__

/*** Start of inlined file: juce_ActionBroadcaster.h ***/
#ifndef __JUCE_ACTIONBROADCASTER_JUCEHEADER__
#define __JUCE_ACTIONBROADCASTER_JUCEHEADER__


/*** Start of inlined file: juce_ActionListenerList.h ***/
#ifndef __JUCE_ACTIONLISTENERLIST_JUCEHEADER__
#define __JUCE_ACTIONLISTENERLIST_JUCEHEADER__

class JUCE_API  ActionListenerList  : public MessageListener
{
public:

	ActionListenerList() throw();

	~ActionListenerList() throw();

	void addActionListener (ActionListener* listener) throw();

	void removeActionListener (ActionListener* listener) throw();

	void removeAllActionListeners() throw();

	void sendActionMessage (const String& message) const;

	void handleMessage (const Message&);

	juce_UseDebuggingNewOperator

private:
	SortedSet <void*> actionListeners_;
	CriticalSection actionListenerLock_;

	ActionListenerList (const ActionListenerList&);
	ActionListenerList& operator= (const ActionListenerList&);
};

#endif   // __JUCE_ACTIONLISTENERLIST_JUCEHEADER__
/*** End of inlined file: juce_ActionListenerList.h ***/

class JUCE_API  ActionBroadcaster
{
public:

	ActionBroadcaster() throw();

	virtual ~ActionBroadcaster();

	void addActionListener (ActionListener* listener);

	void removeActionListener (ActionListener* listener);

	void removeAllActionListeners();

	void sendActionMessage (const String& message) const;

private:

	ActionListenerList actionListenerList;

	ActionBroadcaster (const ActionBroadcaster&);
	ActionBroadcaster& operator= (const ActionBroadcaster&);
};

#endif   // __JUCE_ACTIONBROADCASTER_JUCEHEADER__
/*** End of inlined file: juce_ActionBroadcaster.h ***/


#endif
#ifndef __JUCE_ACTIONLISTENER_JUCEHEADER__

#endif
#ifndef __JUCE_ACTIONLISTENERLIST_JUCEHEADER__

#endif
#ifndef __JUCE_ASYNCUPDATER_JUCEHEADER__

#endif
#ifndef __JUCE_CALLBACKMESSAGE_JUCEHEADER__

/*** Start of inlined file: juce_CallbackMessage.h ***/
#ifndef __JUCE_CALLBACKMESSAGE_JUCEHEADER__
#define __JUCE_CALLBACKMESSAGE_JUCEHEADER__

class JUCE_API  CallbackMessage   : public Message
{
public:

	CallbackMessage() throw();

	~CallbackMessage() throw();

	virtual void messageCallback() = 0;

	void post();

	juce_UseDebuggingNewOperator

private:
	CallbackMessage (const CallbackMessage&);
	CallbackMessage& operator= (const CallbackMessage&);
};

#endif   // __JUCE_CALLBACKMESSAGE_JUCEHEADER__
/*** End of inlined file: juce_CallbackMessage.h ***/


#endif
#ifndef __JUCE_CHANGEBROADCASTER_JUCEHEADER__

#endif
#ifndef __JUCE_CHANGELISTENER_JUCEHEADER__

#endif
#ifndef __JUCE_CHANGELISTENERLIST_JUCEHEADER__

#endif
#ifndef __JUCE_INTERPROCESSCONNECTION_JUCEHEADER__

/*** Start of inlined file: juce_InterprocessConnection.h ***/
#ifndef __JUCE_INTERPROCESSCONNECTION_JUCEHEADER__
#define __JUCE_INTERPROCESSCONNECTION_JUCEHEADER__

class InterprocessConnectionServer;

class JUCE_API  InterprocessConnection	: public Thread,
											private MessageListener
{
public:

	InterprocessConnection (bool callbacksOnMessageThread = true,
							uint32 magicMessageHeaderNumber = 0xf2b49e2c);

	~InterprocessConnection();

	bool connectToSocket (const String& hostName,
						  int portNumber,
						  int timeOutMillisecs);

	bool connectToPipe (const String& pipeName,
						int pipeReceiveMessageTimeoutMs = -1);

	bool createPipe (const String& pipeName,
					 int pipeReceiveMessageTimeoutMs = -1);

	void disconnect();

	bool isConnected() const;

	StreamingSocket* getSocket() const throw()		  { return socket; }

	NamedPipe* getPipe() const throw()			  { return pipe; }

	const String getConnectedHostName() const;

	bool sendMessage (const MemoryBlock& message);

	virtual void connectionMade() = 0;

	virtual void connectionLost() = 0;

	virtual void messageReceived (const MemoryBlock& message) = 0;

	juce_UseDebuggingNewOperator

private:
	CriticalSection pipeAndSocketLock;
	ScopedPointer <StreamingSocket> socket;
	ScopedPointer <NamedPipe> pipe;
	bool callbackConnectionState;
	const bool useMessageThread;
	const uint32 magicMessageHeader;
	int pipeReceiveMessageTimeout;

	friend class InterprocessConnectionServer;

	void initialiseWithSocket (StreamingSocket* socket_);
	void initialiseWithPipe (NamedPipe* pipe_);

	void handleMessage (const Message& message);

	void connectionMadeInt();
	void connectionLostInt();
	void deliverDataInt (const MemoryBlock& data);

	bool readNextMessageInt();
	void run();

	InterprocessConnection (const InterprocessConnection&);
	InterprocessConnection& operator= (const InterprocessConnection&);
};

#endif   // __JUCE_INTERPROCESSCONNECTION_JUCEHEADER__
/*** End of inlined file: juce_InterprocessConnection.h ***/


#endif
#ifndef __JUCE_INTERPROCESSCONNECTIONSERVER_JUCEHEADER__

/*** Start of inlined file: juce_InterprocessConnectionServer.h ***/
#ifndef __JUCE_INTERPROCESSCONNECTIONSERVER_JUCEHEADER__
#define __JUCE_INTERPROCESSCONNECTIONSERVER_JUCEHEADER__

class JUCE_API  InterprocessConnectionServer	: private Thread
{
public:

	InterprocessConnectionServer();

	~InterprocessConnectionServer();

	bool beginWaitingForSocket (int portNumber);

	void stop();

protected:
	virtual InterprocessConnection* createConnectionObject() = 0;

public:

	juce_UseDebuggingNewOperator

private:
	ScopedPointer <StreamingSocket> socket;

	void run();

	InterprocessConnectionServer (const InterprocessConnectionServer&);
	InterprocessConnectionServer& operator= (const InterprocessConnectionServer&);
};

#endif   // __JUCE_INTERPROCESSCONNECTIONSERVER_JUCEHEADER__
/*** End of inlined file: juce_InterprocessConnectionServer.h ***/


#endif
#ifndef __JUCE_LISTENERLIST_JUCEHEADER__

#endif
#ifndef __JUCE_MESSAGE_JUCEHEADER__

#endif
#ifndef __JUCE_MESSAGELISTENER_JUCEHEADER__

#endif
#ifndef __JUCE_MESSAGEMANAGER_JUCEHEADER__

/*** Start of inlined file: juce_MessageManager.h ***/
#ifndef __JUCE_MESSAGEMANAGER_JUCEHEADER__
#define __JUCE_MESSAGEMANAGER_JUCEHEADER__

class Component;
class MessageManagerLock;

typedef void* (MessageCallbackFunction) (void* userData);

class JUCE_API  MessageManager
{
public:

	static MessageManager* getInstance() throw();

	void runDispatchLoop();

	void stopDispatchLoop();

	bool hasStopMessageBeenSent() const throw()	 { return quitMessagePosted; }

	bool runDispatchLoopUntil (int millisecondsToRunFor);

	void* callFunctionOnMessageThread (MessageCallbackFunction* callback,
									   void* userData);

	bool isThisTheMessageThread() const throw();

	void setCurrentThreadAsMessageThread();

	Thread::ThreadID getCurrentMessageThread() const throw()		 { return messageThreadId; }

	bool currentThreadHasLockedMessageManager() const throw();

	static void broadcastMessage (const String& messageText) throw();

	void registerBroadcastListener (ActionListener* listener) throw();

	void deregisterBroadcastListener (ActionListener* listener) throw();

	void deliverMessage (void*);
	void deliverBroadcastMessage (const String&);
	~MessageManager() throw();

	juce_UseDebuggingNewOperator

private:
	MessageManager() throw();

	friend class MessageListener;
	friend class ChangeBroadcaster;
	friend class ActionBroadcaster;
	friend class CallbackMessage;
	static MessageManager* instance;

	SortedSet <const MessageListener*> messageListeners;
	ScopedPointer <ActionListenerList> broadcastListeners;

	friend class JUCEApplication;
	bool quitMessagePosted, quitMessageReceived;
	Thread::ThreadID messageThreadId;

	VoidArray modalComponents;
	static void* exitModalLoopCallback (void*);

	void postMessageToQueue (Message* message);
	void postCallbackMessage (Message* message);

	static void doPlatformSpecificInitialisation();
	static void doPlatformSpecificShutdown();

	friend class MessageManagerLock;
	Thread::ThreadID volatile threadWithLock;
	CriticalSection lockingLock;

	MessageManager (const MessageManager&);
	MessageManager& operator= (const MessageManager&);
};

class JUCE_API MessageManagerLock
{
public:

	MessageManagerLock (Thread* threadToCheckForExitSignal = 0) throw();

	MessageManagerLock (ThreadPoolJob* jobToCheckForExitSignal) throw();

	~MessageManagerLock() throw();

	bool lockWasGained() const throw()			  { return locked; }

private:
	class SharedEvents;
	class BlockingMessage;
	friend class SharedEvents;
	friend class BlockingMessage;
	SharedEvents* sharedEvents;
	bool locked;

	void init (Thread* thread, ThreadPoolJob* job) throw();

	MessageManagerLock (const MessageManagerLock&);
	MessageManagerLock& operator= (const MessageManagerLock&);
};

#endif   // __JUCE_MESSAGEMANAGER_JUCEHEADER__
/*** End of inlined file: juce_MessageManager.h ***/


#endif
#ifndef __JUCE_MULTITIMER_JUCEHEADER__

/*** Start of inlined file: juce_MultiTimer.h ***/
#ifndef __JUCE_MULTITIMER_JUCEHEADER__
#define __JUCE_MULTITIMER_JUCEHEADER__

class JUCE_API  MultiTimer
{
protected:

	MultiTimer() throw();

	MultiTimer (const MultiTimer& other) throw();

public:

	virtual ~MultiTimer();

	virtual void timerCallback (int timerId) = 0;

	void startTimer (int timerId, int intervalInMilliseconds) throw();

	void stopTimer (int timerId) throw();

	bool isTimerRunning (int timerId) const throw();

	int getTimerInterval (int timerId) const throw();

private:
	class MultiTimerCallback;
	CriticalSection timerListLock;
	OwnedArray <MultiTimerCallback> timers;

	MultiTimer& operator= (const MultiTimer&);
};

#endif   // __JUCE_MULTITIMER_JUCEHEADER__
/*** End of inlined file: juce_MultiTimer.h ***/


#endif
#ifndef __JUCE_TIMER_JUCEHEADER__

#endif
#ifndef __JUCE_ARROWBUTTON_JUCEHEADER__

/*** Start of inlined file: juce_ArrowButton.h ***/
#ifndef __JUCE_ARROWBUTTON_JUCEHEADER__
#define __JUCE_ARROWBUTTON_JUCEHEADER__


/*** Start of inlined file: juce_DropShadowEffect.h ***/
#ifndef __JUCE_DROPSHADOWEFFECT_JUCEHEADER__
#define __JUCE_DROPSHADOWEFFECT_JUCEHEADER__

class JUCE_API  DropShadowEffect  : public ImageEffectFilter
{
public:

	DropShadowEffect();

	~DropShadowEffect();

	void setShadowProperties (const float newRadius,
							  const float newOpacity,
							  const int newShadowOffsetX,
							  const int newShadowOffsetY);

	void applyEffect (Image& sourceImage, Graphics& destContext);

	juce_UseDebuggingNewOperator

private:
	int offsetX, offsetY;
	float radius, opacity;
};

#endif   // __JUCE_DROPSHADOWEFFECT_JUCEHEADER__
/*** End of inlined file: juce_DropShadowEffect.h ***/

class JUCE_API  ArrowButton  : public Button
{
public:

	ArrowButton (const String& buttonName,
				 float arrowDirection,
				 const Colour& arrowColour);

	~ArrowButton();

	juce_UseDebuggingNewOperator

protected:
	void paintButton (Graphics& g,
					  bool isMouseOverButton,
					  bool isButtonDown);

	void buttonStateChanged();

private:

	Colour colour;
	DropShadowEffect shadow;
	Path path;
	int offset;

	ArrowButton (const ArrowButton&);
	ArrowButton& operator= (const ArrowButton&);
};

#endif   // __JUCE_ARROWBUTTON_JUCEHEADER__
/*** End of inlined file: juce_ArrowButton.h ***/


#endif
#ifndef __JUCE_BUTTON_JUCEHEADER__

#endif
#ifndef __JUCE_DRAWABLEBUTTON_JUCEHEADER__

/*** Start of inlined file: juce_DrawableButton.h ***/
#ifndef __JUCE_DRAWABLEBUTTON_JUCEHEADER__
#define __JUCE_DRAWABLEBUTTON_JUCEHEADER__


/*** Start of inlined file: juce_Drawable.h ***/
#ifndef __JUCE_DRAWABLE_JUCEHEADER__
#define __JUCE_DRAWABLE_JUCEHEADER__

class JUCE_API  Drawable
{
protected:

	Drawable();

public:
	virtual ~Drawable();

	virtual Drawable* createCopy() const = 0;

	void draw (Graphics& g, float opacity,
			   const AffineTransform& transform = AffineTransform::identity) const;

	void drawAt (Graphics& g,
				 float x, float y,
				 float opacity) const;

	void drawWithin (Graphics& g,
					 int destX,
					 int destY,
					 int destWidth,
					 int destHeight,
					 const RectanglePlacement& placement,
					 float opacity) const;

	class RenderingContext
	{
	public:
		RenderingContext (Graphics& g, const AffineTransform& transform, float opacity) throw();

		Graphics& g;
		AffineTransform transform;
		float opacity;

	private:
		RenderingContext& operator= (const RenderingContext&);
	};

	virtual void render (const RenderingContext& context) const = 0;

	virtual const Rectangle<float> getBounds() const = 0;

	virtual bool hitTest (float x, float y) const = 0;

	const String& getName() const throw()		   { return name; }

	void setName (const String& newName) throw()	{ name = newName; }

	static Drawable* createFromImageData (const void* data, size_t numBytes);

	static Drawable* createFromImageDataStream (InputStream& dataSource);

	static Drawable* createFromImageFile (const File& file);

	static Drawable* createFromSVG (const XmlElement& svgDocument);

	static Drawable* createFromValueTree (const ValueTree& tree) throw();

	virtual ValueTree createValueTree() const throw() = 0;

	juce_UseDebuggingNewOperator

private:
	Drawable (const Drawable&);
	Drawable& operator= (const Drawable&);

	String name;
};

#endif   // __JUCE_DRAWABLE_JUCEHEADER__
/*** End of inlined file: juce_Drawable.h ***/

class JUCE_API  DrawableButton  : public Button
{
public:

	enum ButtonStyle
	{
		ImageFitted,		/**< The button will just display the images, but will resize and centre them to fit inside it. */
		ImageRaw,		   /**< The button will just display the images in their normal size and position.
										 This leaves it up to the caller to make sure the images are the correct size and position for the button. */
		ImageAboveTextLabel,	/**< Draws the button as a text label across the bottom with the image resized and scaled to fit above it. */
		ImageOnButtonBackground	 /**< Draws the button as a standard rounded-rectangle button with the image on top. */
	};

	DrawableButton (const String& buttonName,
					ButtonStyle buttonStyle);

	~DrawableButton();

	void setImages (const Drawable* normalImage,
					const Drawable* overImage = 0,
					const Drawable* downImage = 0,
					const Drawable* disabledImage = 0,
					const Drawable* normalImageOn = 0,
					const Drawable* overImageOn = 0,
					const Drawable* downImageOn = 0,
					const Drawable* disabledImageOn = 0);

	void setButtonStyle (ButtonStyle newStyle);

	void setBackgroundColours (const Colour& toggledOffColour,
							   const Colour& toggledOnColour);

	const Colour& getBackgroundColour() const throw();

	void setEdgeIndent (int numPixelsIndent);

	const Drawable* getCurrentImage() const throw();
	const Drawable* getNormalImage() const throw();
	const Drawable* getOverImage() const throw();
	const Drawable* getDownImage() const throw();

	juce_UseDebuggingNewOperator

protected:
	void paintButton (Graphics& g,
					  bool isMouseOverButton,
					  bool isButtonDown);

private:

	ButtonStyle style;
	ScopedPointer <Drawable> normalImage, overImage, downImage, disabledImage;
	ScopedPointer <Drawable> normalImageOn, overImageOn, downImageOn, disabledImageOn;
	Colour backgroundOff, backgroundOn;
	int edgeIndent;

	void deleteImages();
	DrawableButton (const DrawableButton&);
	DrawableButton& operator= (const DrawableButton&);
};

#endif   // __JUCE_DRAWABLEBUTTON_JUCEHEADER__
/*** End of inlined file: juce_DrawableButton.h ***/


#endif
#ifndef __JUCE_HYPERLINKBUTTON_JUCEHEADER__

/*** Start of inlined file: juce_HyperlinkButton.h ***/
#ifndef __JUCE_HYPERLINKBUTTON_JUCEHEADER__
#define __JUCE_HYPERLINKBUTTON_JUCEHEADER__

class JUCE_API  HyperlinkButton  : public Button
{
public:

	HyperlinkButton (const String& linkText,
					 const URL& linkURL);

	~HyperlinkButton();

	void setFont (const Font& newFont,
				  bool resizeToMatchComponentHeight,
				  const Justification& justificationType = Justification::horizontallyCentred);

	enum ColourIds
	{
		textColourId		 = 0x1001f00, /**< The colour to use for the URL text. */
	};

	void setURL (const URL& newURL) throw();

	const URL& getURL() const throw()			   { return url; }

	void changeWidthToFitText();

	juce_UseDebuggingNewOperator

protected:
	void clicked();
	void colourChanged();
	void paintButton (Graphics& g,
					  bool isMouseOverButton,
					  bool isButtonDown);

private:
	URL url;
	Font font;
	bool resizeFont;
	Justification justification;

	const Font getFontToUse() const;

	HyperlinkButton (const HyperlinkButton&);
	HyperlinkButton& operator= (const HyperlinkButton&);
};

#endif   // __JUCE_HYPERLINKBUTTON_JUCEHEADER__
/*** End of inlined file: juce_HyperlinkButton.h ***/


#endif
#ifndef __JUCE_IMAGEBUTTON_JUCEHEADER__

/*** Start of inlined file: juce_ImageButton.h ***/
#ifndef __JUCE_IMAGEBUTTON_JUCEHEADER__
#define __JUCE_IMAGEBUTTON_JUCEHEADER__

class JUCE_API  ImageButton  : public Button
{
public:

	explicit ImageButton (const String& name);

	~ImageButton();

	void setImages (bool resizeButtonNowToFitThisImage,
					bool rescaleImagesWhenButtonSizeChanges,
					bool preserveImageProportions,
					Image* normalImage,
					float imageOpacityWhenNormal,
					const Colour& overlayColourWhenNormal,
					Image* overImage,
					float imageOpacityWhenOver,
					const Colour& overlayColourWhenOver,
					Image* downImage,
					float imageOpacityWhenDown,
					const Colour& overlayColourWhenDown,
					float hitTestAlphaThreshold = 0.0f);

	Image* getNormalImage() const throw();

	Image* getOverImage() const throw();

	Image* getDownImage() const throw();

	juce_UseDebuggingNewOperator

protected:
	bool hitTest (int x, int y);
	void paintButton (Graphics& g,
					  bool isMouseOverButton,
					  bool isButtonDown);

private:

	bool scaleImageToFit, preserveProportions;
	unsigned char alphaThreshold;
	int imageX, imageY, imageW, imageH;
	Image* normalImage;
	Image* overImage;
	Image* downImage;
	float normalOpacity, overOpacity, downOpacity;
	Colour normalOverlay, overOverlay, downOverlay;

	Image* getCurrentImage() const;
	void deleteImages();

	ImageButton (const ImageButton&);
	ImageButton& operator= (const ImageButton&);
};

#endif   // __JUCE_IMAGEBUTTON_JUCEHEADER__
/*** End of inlined file: juce_ImageButton.h ***/


#endif
#ifndef __JUCE_SHAPEBUTTON_JUCEHEADER__

/*** Start of inlined file: juce_ShapeButton.h ***/
#ifndef __JUCE_SHAPEBUTTON_JUCEHEADER__
#define __JUCE_SHAPEBUTTON_JUCEHEADER__

class JUCE_API  ShapeButton  : public Button
{
public:

	ShapeButton (const String& name,
				 const Colour& normalColour,
				 const Colour& overColour,
				 const Colour& downColour);

	~ShapeButton();

	void setShape (const Path& newShape,
				   bool resizeNowToFitThisShape,
				   bool maintainShapeProportions,
				   bool hasDropShadow);

	void setColours (const Colour& normalColour,
					 const Colour& overColour,
					 const Colour& downColour);

	void setOutline (const Colour& outlineColour,
					 float outlineStrokeWidth);

	juce_UseDebuggingNewOperator

protected:
	void paintButton (Graphics& g,
					  bool isMouseOverButton,
					  bool isButtonDown);

private:
	Colour normalColour, overColour, downColour, outlineColour;
	DropShadowEffect shadow;
	Path shape;
	bool maintainShapeProportions;
	float outlineWidth;

	ShapeButton (const ShapeButton&);
	ShapeButton& operator= (const ShapeButton&);
};

#endif   // __JUCE_SHAPEBUTTON_JUCEHEADER__
/*** End of inlined file: juce_ShapeButton.h ***/


#endif
#ifndef __JUCE_TEXTBUTTON_JUCEHEADER__

#endif
#ifndef __JUCE_TOGGLEBUTTON_JUCEHEADER__

/*** Start of inlined file: juce_ToggleButton.h ***/
#ifndef __JUCE_TOGGLEBUTTON_JUCEHEADER__
#define __JUCE_TOGGLEBUTTON_JUCEHEADER__

class JUCE_API  ToggleButton  : public Button
{
public:

	ToggleButton (const String& buttonText);

	~ToggleButton();

	void changeWidthToFitText();

	enum ColourIds
	{
		textColourId			= 0x1006501   /**< The colour to use for the button's text. */
	};

	juce_UseDebuggingNewOperator

protected:
	void paintButton (Graphics& g,
					  bool isMouseOverButton,
					  bool isButtonDown);

	void colourChanged();

private:

	ToggleButton (const ToggleButton&);
	ToggleButton& operator= (const ToggleButton&);
};

#endif   // __JUCE_TOGGLEBUTTON_JUCEHEADER__
/*** End of inlined file: juce_ToggleButton.h ***/


#endif
#ifndef __JUCE_TOOLBARBUTTON_JUCEHEADER__

/*** Start of inlined file: juce_ToolbarButton.h ***/
#ifndef __JUCE_TOOLBARBUTTON_JUCEHEADER__
#define __JUCE_TOOLBARBUTTON_JUCEHEADER__


/*** Start of inlined file: juce_ToolbarItemComponent.h ***/
#ifndef __JUCE_TOOLBARITEMCOMPONENT_JUCEHEADER__
#define __JUCE_TOOLBARITEMCOMPONENT_JUCEHEADER__


/*** Start of inlined file: juce_Toolbar.h ***/
#ifndef __JUCE_TOOLBAR_JUCEHEADER__
#define __JUCE_TOOLBAR_JUCEHEADER__


/*** Start of inlined file: juce_DragAndDropContainer.h ***/
#ifndef __JUCE_DRAGANDDROPCONTAINER_JUCEHEADER__
#define __JUCE_DRAGANDDROPCONTAINER_JUCEHEADER__


/*** Start of inlined file: juce_DragAndDropTarget.h ***/
#ifndef __JUCE_DRAGANDDROPTARGET_JUCEHEADER__
#define __JUCE_DRAGANDDROPTARGET_JUCEHEADER__

class JUCE_API  DragAndDropTarget
{
public:
	virtual ~DragAndDropTarget()  {}

	virtual bool isInterestedInDragSource (const String& sourceDescription,
										   Component* sourceComponent) = 0;

	virtual void itemDragEnter (const String& sourceDescription,
								Component* sourceComponent,
								int x, int y);

	virtual void itemDragMove (const String& sourceDescription,
							   Component* sourceComponent,
							   int x, int y);

	virtual void itemDragExit (const String& sourceDescription,
							   Component* sourceComponent);

	virtual void itemDropped (const String& sourceDescription,
							  Component* sourceComponent,
							  int x, int y) = 0;

	virtual bool shouldDrawDragImageWhenOver();
};

#endif   // __JUCE_DRAGANDDROPTARGET_JUCEHEADER__
/*** End of inlined file: juce_DragAndDropTarget.h ***/

class JUCE_API  DragAndDropContainer
{
public:

	DragAndDropContainer();

	virtual ~DragAndDropContainer();

	void startDragging (const String& sourceDescription,
						Component* sourceComponent,
						Image* dragImage = 0,
						bool allowDraggingToOtherJuceWindows = false,
						const Point<int>* imageOffsetFromMouse = 0);

	bool isDragAndDropActive() const;

	const String getCurrentDragDescription() const;

	static DragAndDropContainer* findParentDragContainerFor (Component* childComponent);

	static bool performExternalDragDropOfFiles (const StringArray& files, bool canMoveFiles);

	static bool performExternalDragDropOfText (const String& text);

	juce_UseDebuggingNewOperator

protected:
	virtual bool shouldDropFilesWhenDraggedExternally (const String& dragSourceDescription,
													   Component* dragSourceComponent,
													   StringArray& files,
													   bool& canMoveFiles);

private:
	friend class DragImageComponent;
	ScopedPointer <Component> dragImageComponent;
	String currentDragDesc;
};

#endif   // __JUCE_DRAGANDDROPCONTAINER_JUCEHEADER__
/*** End of inlined file: juce_DragAndDropContainer.h ***/


/*** Start of inlined file: juce_ComponentAnimator.h ***/
#ifndef __JUCE_COMPONENTANIMATOR_JUCEHEADER__
#define __JUCE_COMPONENTANIMATOR_JUCEHEADER__

class JUCE_API  ComponentAnimator  : public ChangeBroadcaster,
									 private Timer
{
public:

	ComponentAnimator();

	~ComponentAnimator();

	void animateComponent (Component* component,
						   const Rectangle<int>& finalPosition,
						   int millisecondsToSpendMoving,
						   double startSpeed = 1.0,
						   double endSpeed = 1.0);

	void cancelAnimation (Component* component,
						  bool moveComponentToItsFinalPosition);

	void cancelAllAnimations (bool moveComponentsToTheirFinalPositions);

	const Rectangle<int> getComponentDestination (Component* component);

	bool isAnimating (Component* component) const;

	juce_UseDebuggingNewOperator

private:
	VoidArray tasks;
	uint32 lastTime;

	void* findTaskFor (Component* component) const;
	void timerCallback();
};

#endif   // __JUCE_COMPONENTANIMATOR_JUCEHEADER__
/*** End of inlined file: juce_ComponentAnimator.h ***/

class ToolbarItemComponent;
class ToolbarItemFactory;
class MissingItemsComponent;

class JUCE_API  Toolbar   : public Component,
							public DragAndDropContainer,
							public DragAndDropTarget,
							private ButtonListener
{
public:

	Toolbar();

	~Toolbar();

	void setVertical (bool shouldBeVertical);

	bool isVertical() const throw()		  { return vertical; }

	int getThickness() const throw();

	int getLength() const throw();

	void clear();

	void addItem (ToolbarItemFactory& factory,
				  int itemId,
				  int insertIndex = -1);

	void removeToolbarItem (int itemIndex);

	int getNumItems() const throw();

	int getItemId (int itemIndex) const throw();

	ToolbarItemComponent* getItemComponent (int itemIndex) const throw();

	void addDefaultItems (ToolbarItemFactory& factoryToUse);

	enum ToolbarItemStyle
	{
		iconsOnly,	   /**< Means that the toolbar should just contain icons. */
		iconsWithText,   /**< Means that the toolbar should have text labels under each icon. */
		textOnly	 /**< Means that the toolbar only display text labels for each item. */
	};

	ToolbarItemStyle getStyle() const throw()		{ return toolbarStyle; }

	void setStyle (const ToolbarItemStyle& newStyle);

	enum CustomisationFlags
	{
		allowIconsOnlyChoice		= 1,	/**< If this flag is specified, the customisation dialog can
													 show the "icons only" option on its choice of toolbar styles. */
		allowIconsWithTextChoice	= 2,	/**< If this flag is specified, the customisation dialog can
													 show the "icons with text" option on its choice of toolbar styles. */
		allowTextOnlyChoice		 = 4,	/**< If this flag is specified, the customisation dialog can
													 show the "text only" option on its choice of toolbar styles. */
		showResetToDefaultsButton	   = 8,	/**< If this flag is specified, the customisation dialog can
													 show a button to reset the toolbar to its default set of items. */

		allCustomisationOptionsEnabled = (allowIconsOnlyChoice | allowIconsWithTextChoice | allowTextOnlyChoice | showResetToDefaultsButton)
	};

	void showCustomisationDialog (ToolbarItemFactory& factory,
								  int optionFlags = allCustomisationOptionsEnabled);

	void setEditingActive (bool editingEnabled);

	enum ColourIds
	{
		backgroundColourId	  = 0x1003200,  /**< A colour to use to fill the toolbar's background. For
													   more control over this, override LookAndFeel::paintToolbarBackground(). */
		separatorColourId	   = 0x1003210,  /**< A colour to use to draw the separator lines. */

		buttonMouseOverBackgroundColourId = 0x1003220,  /**< A colour used to paint the background of buttons when the mouse is
															 over them. */
		buttonMouseDownBackgroundColourId = 0x1003230,  /**< A colour used to paint the background of buttons when the mouse is
															 held down on them. */

		labelTextColourId	   = 0x1003240,	/**< A colour to use for drawing the text under buttons
															 when the style is set to iconsWithText or textOnly. */

		editingModeOutlineColourId  = 0x1003250   /**< A colour to use for an outline around buttons when
													   the customisation dialog is active and the mouse moves over them. */
	};

	const String toString() const;

	bool restoreFromString (ToolbarItemFactory& factoryToUse,
							const String& savedVersion);

	void paint (Graphics& g);
	void resized();
	void buttonClicked (Button*);
	void mouseDown (const MouseEvent&);
	bool isInterestedInDragSource (const String&, Component*);
	void itemDragMove (const String&, Component*, int, int);
	void itemDragExit (const String&, Component*);
	void itemDropped (const String&, Component*, int, int);
	void updateAllItemPositions (const bool animate);
	static ToolbarItemComponent* createItem (ToolbarItemFactory&, const int itemId);

	juce_UseDebuggingNewOperator

private:
	Button* missingItemsButton;
	bool vertical, isEditingActive;
	ToolbarItemStyle toolbarStyle;
	ComponentAnimator animator;
	friend class MissingItemsComponent;
	Array <ToolbarItemComponent*> items;

	friend class ItemDragAndDropOverlayComponent;
	static const tchar* const toolbarDragDescriptor;

	void addItemInternal (ToolbarItemFactory& factory, const int itemId, const int insertIndex);

	ToolbarItemComponent* getNextActiveComponent (int index, const int delta) const;

	Toolbar (const Toolbar&);
	Toolbar& operator= (const Toolbar&);
};

#endif   // __JUCE_TOOLBAR_JUCEHEADER__
/*** End of inlined file: juce_Toolbar.h ***/

class ItemDragAndDropOverlayComponent;

class JUCE_API  ToolbarItemComponent  : public Button
{
public:

	ToolbarItemComponent (int itemId,
						  const String& labelText,
						  bool isBeingUsedAsAButton);

	~ToolbarItemComponent();

	int getItemId() const throw()					   { return itemId; }

	Toolbar* getToolbar() const;

	bool isToolbarVertical() const;

	Toolbar::ToolbarItemStyle getStyle() const throw()		  { return toolbarStyle; }

	virtual void setStyle (const Toolbar::ToolbarItemStyle& newStyle);

	const Rectangle<int> getContentArea() const throw()		 { return contentArea; }

	virtual bool getToolbarItemSizes (int toolbarThickness,
									  bool isToolbarVertical,
									  int& preferredSize,
									  int& minSize,
									  int& maxSize) = 0;

	virtual void paintButtonArea (Graphics& g,
								  int width, int height,
								  bool isMouseOver, bool isMouseDown) = 0;

	virtual void contentAreaChanged (const Rectangle<int>& newBounds) = 0;

	enum ToolbarEditingMode
	{
		normalMode = 0,	 /**< Means that the component is active, inside a toolbar. */
		editableOnToolbar,  /**< Means that the component is on a toolbar, but the toolbar is in
								 customisation mode, and the items can be dragged around. */
		editableOnPalette   /**< Means that the component is on an new-item palette, so it can be
								 dragged onto a toolbar to add it to that bar.*/
	};

	void setEditingMode (const ToolbarEditingMode newMode);

	ToolbarEditingMode getEditingMode() const throw()		   { return mode; }

	void paintButton (Graphics& g, bool isMouseOver, bool isMouseDown);
	void resized();

	juce_UseDebuggingNewOperator

private:
	friend class Toolbar;
	friend class ItemDragAndDropOverlayComponent;
	const int itemId;
	ToolbarEditingMode mode;
	Toolbar::ToolbarItemStyle toolbarStyle;
	ScopedPointer <Component> overlayComp;
	int dragOffsetX, dragOffsetY;
	bool isActive, isBeingDragged, isBeingUsedAsAButton;
	Rectangle<int> contentArea;

	ToolbarItemComponent (const ToolbarItemComponent&);
	ToolbarItemComponent& operator= (const ToolbarItemComponent&);
};

#endif   // __JUCE_TOOLBARITEMCOMPONENT_JUCEHEADER__
/*** End of inlined file: juce_ToolbarItemComponent.h ***/

class JUCE_API  ToolbarButton   : public ToolbarItemComponent
{
public:

	ToolbarButton (int itemId,
				   const String& labelText,
				   Drawable* normalImage,
				   Drawable* toggledOnImage);

	~ToolbarButton();

	bool getToolbarItemSizes (int toolbarDepth, bool isToolbarVertical, int& preferredSize,
							  int& minSize, int& maxSize);
	void paintButtonArea (Graphics& g, int width, int height, bool isMouseOver, bool isMouseDown);
	void contentAreaChanged (const Rectangle<int>& newBounds);

	juce_UseDebuggingNewOperator

private:
	ScopedPointer <Drawable> normalImage, toggledOnImage;

	ToolbarButton (const ToolbarButton&);
	ToolbarButton& operator= (const ToolbarButton&);
};

#endif   // __JUCE_TOOLBARBUTTON_JUCEHEADER__
/*** End of inlined file: juce_ToolbarButton.h ***/


#endif
#ifndef __JUCE_CODEDOCUMENT_JUCEHEADER__

/*** Start of inlined file: juce_CodeDocument.h ***/
#ifndef __JUCE_CODEDOCUMENT_JUCEHEADER__
#define __JUCE_CODEDOCUMENT_JUCEHEADER__

class CodeDocumentLine;

class JUCE_API  CodeDocument
{
public:
	CodeDocument();

	~CodeDocument();

	class JUCE_API  Position
	{
	public:
		Position() throw();

		Position (const CodeDocument* ownerDocument,
				  int line, int indexInLine) throw();

		Position (const CodeDocument* ownerDocument,
				  int charactersFromStartOfDocument) throw();

		Position (const Position& other) throw();

		~Position() throw();

		Position& operator= (const Position& other) throw();
		bool operator== (const Position& other) const throw();
		bool operator!= (const Position& other) const throw();

		void setPosition (int charactersFromStartOfDocument) throw();

		int getPosition() const throw()		 { return characterPos; }

		void setLineAndIndex (int newLine, int newIndexInLine) throw();

		int getLineNumber() const throw()	   { return line; }

		int getIndexInLine() const throw()	  { return indexInLine; }

		void setPositionMaintained (bool isMaintained) throw();

		void moveBy (int characterDelta) throw();

		const Position movedBy (int characterDelta) const throw();

		const Position movedByLines (int deltaLines) const throw();

		const tchar getCharacter() const throw();

		const String getLineText() const throw();

	private:
		CodeDocument* owner;
		int characterPos, line, indexInLine;
		bool positionMaintained;
	};

	const String getAllContent() const throw();

	const String getTextBetween (const Position& start, const Position& end) const throw();

	const String getLine (int lineIndex) const throw();

	int getNumCharacters() const throw();

	int getNumLines() const throw()			 { return lines.size(); }

	int getMaximumLineLength() throw();

	void deleteSection (const Position& startPosition, const Position& endPosition);

	void insertText (const Position& position, const String& text);

	void replaceAllContent (const String& newContent);

	bool loadFromStream (InputStream& stream);

	bool writeToStream (OutputStream& stream);

	const String getNewLineCharacters() const throw()	   { return newLineChars; }

	void setNewLineCharacters (const String& newLine) throw();

	void newTransaction();

	void undo();

	void redo();

	void clearUndoHistory();

	UndoManager& getUndoManager() throw()		   { return undoManager; }

	void setSavePoint() throw();

	bool hasChangedSinceSavePoint() const throw();

	const Position findWordBreakAfter (const Position& position) const throw();

	const Position findWordBreakBefore (const Position& position) const throw();

	class JUCE_API  Listener
	{
	public:
		Listener() {}
		virtual ~Listener() {}

		virtual void codeDocumentChanged (const Position& affectedTextStart,
										  const Position& affectedTextEnd) = 0;
	};

	void addListener (Listener* listener) throw();

	void removeListener (Listener* listener) throw();

	class Iterator
	{
	public:
		Iterator (CodeDocument* document);
		Iterator (const Iterator& other);
		Iterator& operator= (const Iterator& other) throw();
		~Iterator() throw();

		juce_wchar nextChar();

		juce_wchar peekNextChar() const;

		void skip();

		int getPosition() const throw()	 { return position; }

		void skipWhitespace();

		void skipToEndOfLine();

		int getLine() const throw()		 { return line; }

		bool isEOF() const throw();

	private:
		CodeDocument* document;
		CodeDocumentLine* currentLine;
		int line, position;
	};

	juce_UseDebuggingNewOperator

private:
	friend class CodeDocumentInsertAction;
	friend class CodeDocumentDeleteAction;
	friend class Iterator;
	friend class Position;

	OwnedArray <CodeDocumentLine> lines;
	Array <Position*> positionsToMaintain;
	UndoManager undoManager;
	int currentActionIndex, indexOfSavedState;
	int maximumLineLength;
	VoidArray listeners;
	String newLineChars;

	void sendListenerChangeMessage (int startLine, int endLine);

	void insert (const String& text, int insertPos, bool undoable);
	void remove (int startPos, int endPos, bool undoable);
	void checkLastLineStatus();

	CodeDocument (const CodeDocument&);
	CodeDocument& operator= (const CodeDocument&);
};

#endif   // __JUCE_CODEDOCUMENT_JUCEHEADER__
/*** End of inlined file: juce_CodeDocument.h ***/


#endif
#ifndef __JUCE_CODEEDITORCOMPONENT_JUCEHEADER__

/*** Start of inlined file: juce_CodeEditorComponent.h ***/
#ifndef __JUCE_CODEEDITORCOMPONENT_JUCEHEADER__
#define __JUCE_CODEEDITORCOMPONENT_JUCEHEADER__


/*** Start of inlined file: juce_CodeTokeniser.h ***/
#ifndef __JUCE_CODETOKENISER_JUCEHEADER__
#define __JUCE_CODETOKENISER_JUCEHEADER__

class JUCE_API  CodeTokeniser
{
public:
	CodeTokeniser()		 {}
	virtual ~CodeTokeniser()	{}

	virtual int readNextToken (CodeDocument::Iterator& source) = 0;

	virtual const StringArray getTokenTypes() = 0;

	virtual const Colour getDefaultColour (int tokenType) = 0;

	juce_UseDebuggingNewOperator
};

#endif   // __JUCE_CODETOKENISER_JUCEHEADER__
/*** End of inlined file: juce_CodeTokeniser.h ***/

class JUCE_API  CodeEditorComponent   : public Component,
										public TextInputTarget,
										public Timer,
										public ScrollBarListener,
										public CodeDocument::Listener,
										public AsyncUpdater
{
public:

	CodeEditorComponent (CodeDocument& document,
						 CodeTokeniser* codeTokeniser);

	~CodeEditorComponent();

	CodeDocument& getDocument() const throw()	   { return document; }

	void loadContent (const String& newContent);

	float getCharWidth() const throw()			  { return charWidth; }

	int getLineHeight() const throw()			   { return lineHeight; }

	int getNumLinesOnScreen() const throw()			 { return linesOnScreen; }

	int getNumColumnsOnScreen() const throw()		   { return columnsOnScreen; }

	const CodeDocument::Position getCaretPos() const		{ return caretPos; }

	void moveCaretTo (const CodeDocument::Position& newPos, bool selecting);

	const Rectangle<int> getCharacterBounds (const CodeDocument::Position& pos) const throw();

	const CodeDocument::Position getPositionAt (int x, int y);

	void cursorLeft (bool moveInWholeWordSteps, bool selecting);
	void cursorRight (bool moveInWholeWordSteps, bool selecting);
	void cursorDown (bool selecting);
	void cursorUp (bool selecting);

	void pageDown (bool selecting);
	void pageUp (bool selecting);

	void scrollDown();
	void scrollUp();
	void scrollToLine (int newFirstLineOnScreen);
	void scrollBy (int deltaLines);
	void scrollToColumn (int newFirstColumnOnScreen);
	void scrollToKeepCaretOnScreen();

	void goToStartOfDocument (bool selecting);
	void goToStartOfLine (bool selecting);
	void goToEndOfDocument (bool selecting);
	void goToEndOfLine (bool selecting);

	void deselectAll();
	void selectAll();

	void insertTextAtCaret (const String& textToInsert);
	void insertTabAtCaret();
	void cut();
	void copy();
	void copyThenCut();
	void paste();
	void backspace (bool moveInWholeWordSteps);
	void deleteForward (bool moveInWholeWordSteps);

	void undo();
	void redo();

	const Range<int> getHighlightedRegion() const;
	void setHighlightedRegion (const Range<int>& newRange);
	const String getTextInRange (const Range<int>& range) const;

	void setTabSize (int numSpacesPerTab,
					 bool insertSpacesInsteadOfTabCharacters) throw();

	int getTabSize() const throw()			  { return spacesPerTab; }

	bool areSpacesInsertedForTabs() const		   { return useSpacesForTabs; }

	void setFont (const Font& newFont);

	void resetToDefaultColours();

	void setColourForTokenType (int tokenType, const Colour& colour);

	const Colour getColourForTokenType (int tokenType) const throw();

	enum ColourIds
	{
		backgroundColourId	  = 0x1004500,  /**< A colour to use to fill the editor's background. */
		caretColourId		   = 0x1004501,  /**< The colour to draw the caret. */
		highlightColourId	   = 0x1004502,  /**< The colour to use for the highlighted background under
													   selected text. */
		defaultTextColourId	 = 0x1004503   /**< The colour to use for text when no syntax colouring is
													   enabled. */
	};

	void setScrollbarThickness (int thickness) throw();

	void resized();
	void paint (Graphics& g);
	bool keyPressed (const KeyPress& key);
	void mouseDown (const MouseEvent& e);
	void mouseDrag (const MouseEvent& e);
	void mouseUp (const MouseEvent& e);
	void mouseDoubleClick (const MouseEvent& e);
	void mouseWheelMove (const MouseEvent& e, float wheelIncrementX, float wheelIncrementY);
	void timerCallback();
	void scrollBarMoved (ScrollBar* scrollBarThatHasMoved, double newRangeStart);
	void handleAsyncUpdate();
	void codeDocumentChanged (const CodeDocument::Position& affectedTextStart,
							  const CodeDocument::Position& affectedTextEnd);

	juce_UseDebuggingNewOperator

private:
	CodeDocument& document;

	Font font;
	int firstLineOnScreen, gutter, spacesPerTab;
	float charWidth;
	int lineHeight, linesOnScreen, columnsOnScreen;
	int scrollbarThickness, columnToTryToMaintain;
	bool useSpacesForTabs;
	double xOffset;

	CodeDocument::Position caretPos;
	CodeDocument::Position selectionStart, selectionEnd;
	Component* caret;
	ScrollBar* verticalScrollBar;
	ScrollBar* horizontalScrollBar;

	enum DragType
	{
		notDragging,
		draggingSelectionStart,
		draggingSelectionEnd
	};

	DragType dragType;

	CodeTokeniser* codeTokeniser;
	Array <Colour> coloursForTokenCategories;

	class CodeEditorLine;
	OwnedArray <CodeEditorLine> lines;
	void rebuildLineTokens();

	OwnedArray <CodeDocument::Iterator> cachedIterators;
	void clearCachedIterators (int firstLineToBeInvalid) throw();
	void updateCachedIterators (int maxLineNum);
	void getIteratorForPosition (int position, CodeDocument::Iterator& result);
	void moveLineDelta (int delta, bool selecting);

	void updateScrollBars();
	void scrollToLineInternal (int line);
	void scrollToColumnInternal (double column);
	void newTransaction();

	int indexToColumn (int line, int index) const throw();
	int columnToIndex (int line, int column) const throw();

	CodeEditorComponent (const CodeEditorComponent&);
	CodeEditorComponent& operator= (const CodeEditorComponent&);
};

#endif   // __JUCE_CODEEDITORCOMPONENT_JUCEHEADER__
/*** End of inlined file: juce_CodeEditorComponent.h ***/


#endif
#ifndef __JUCE_CODETOKENISER_JUCEHEADER__

#endif
#ifndef __JUCE_CPLUSPLUSCODETOKENISER_JUCEHEADER__

/*** Start of inlined file: juce_CPlusPlusCodeTokeniser.h ***/
#ifndef __JUCE_CPLUSPLUSCODETOKENISER_JUCEHEADER__
#define __JUCE_CPLUSPLUSCODETOKENISER_JUCEHEADER__

class JUCE_API  CPlusPlusCodeTokeniser	: public CodeTokeniser
{
public:

	CPlusPlusCodeTokeniser();
	~CPlusPlusCodeTokeniser();

	enum TokenType
	{
		tokenType_error = 0,
		tokenType_comment,
		tokenType_builtInKeyword,
		tokenType_identifier,
		tokenType_integerLiteral,
		tokenType_floatLiteral,
		tokenType_stringLiteral,
		tokenType_operator,
		tokenType_bracket,
		tokenType_punctuation,
		tokenType_preprocessor
	};

	int readNextToken (CodeDocument::Iterator& source);
	const StringArray getTokenTypes();
	const Colour getDefaultColour (int tokenType);

	juce_UseDebuggingNewOperator
};

#endif   // __JUCE_CPLUSPLUSCODETOKENISER_JUCEHEADER__
/*** End of inlined file: juce_CPlusPlusCodeTokeniser.h ***/


#endif
#ifndef __JUCE_COMBOBOX_JUCEHEADER__

#endif
#ifndef __JUCE_LABEL_JUCEHEADER__

#endif
#ifndef __JUCE_LISTBOX_JUCEHEADER__

#endif
#ifndef __JUCE_PROGRESSBAR_JUCEHEADER__

/*** Start of inlined file: juce_ProgressBar.h ***/
#ifndef __JUCE_PROGRESSBAR_JUCEHEADER__
#define __JUCE_PROGRESSBAR_JUCEHEADER__

class JUCE_API  ProgressBar  : public Component,
							   public SettableTooltipClient,
							   private Timer
{
public:

	explicit ProgressBar (double& progress);

	~ProgressBar();

	void setPercentageDisplay (const bool shouldDisplayPercentage);

	void setTextToDisplay (const String& text);

	enum ColourIds
	{
		backgroundColourId		  = 0x1001900,	/**< The background colour, behind the bar. */
		foregroundColourId		  = 0x1001a00,	/**< The colour to use to draw the bar itself. LookAndFeel
															 classes will probably use variations on this colour. */
	};

	juce_UseDebuggingNewOperator

protected:
	void paint (Graphics& g);
	void lookAndFeelChanged();
	void visibilityChanged();
	void colourChanged();

private:
	double& progress;
	double currentValue;
	bool displayPercentage;
	String displayedMessage, currentMessage;
	uint32 lastCallbackTime;

	void timerCallback();

	ProgressBar (const ProgressBar&);
	ProgressBar& operator= (const ProgressBar&);
};

#endif   // __JUCE_PROGRESSBAR_JUCEHEADER__
/*** End of inlined file: juce_ProgressBar.h ***/


#endif
#ifndef __JUCE_SLIDER_JUCEHEADER__

/*** Start of inlined file: juce_Slider.h ***/
#ifndef __JUCE_SLIDER_JUCEHEADER__
#define __JUCE_SLIDER_JUCEHEADER__


/*** Start of inlined file: juce_SliderListener.h ***/
#ifndef __JUCE_SLIDERLISTENER_JUCEHEADER__
#define __JUCE_SLIDERLISTENER_JUCEHEADER__

class Slider;

class JUCE_API  SliderListener
{
public:

	virtual ~SliderListener() {}

	virtual void sliderValueChanged (Slider* slider) = 0;

	virtual void sliderDragStarted (Slider* slider);

	virtual void sliderDragEnded (Slider* slider);
};

#endif   // __JUCE_SLIDERLISTENER_JUCEHEADER__
/*** End of inlined file: juce_SliderListener.h ***/

class JUCE_API  Slider  : public Component,
						  public SettableTooltipClient,
						  private AsyncUpdater,
						  private ButtonListener,
						  private LabelListener,
						  private Value::Listener
{
public:

	explicit Slider (const String& componentName);

	~Slider();

	enum SliderStyle
	{
		LinearHorizontal,	   /**< A traditional horizontal slider. */
		LinearVertical,	 /**< A traditional vertical slider. */
		LinearBar,		  /**< A horizontal bar slider with the text label drawn on top of it. */
		Rotary,		 /**< A rotary control that you move by dragging the mouse in a circular motion, like a knob.
									 @see setRotaryParameters */
		RotaryHorizontalDrag,   /**< A rotary control that you move by dragging the mouse left-to-right.
									 @see setRotaryParameters */
		RotaryVerticalDrag,	 /**< A rotary control that you move by dragging the mouse up-and-down.
									 @see setRotaryParameters */
		IncDecButtons,	  /**< A pair of buttons that increment or decrement the slider's value by the increment set in setRange(). */

		TwoValueHorizontal,	 /**< A horizontal slider that has two thumbs instead of one, so it can show a minimum and maximum value.
									 @see setMinValue, setMaxValue */
		TwoValueVertical,	   /**< A vertical slider that has two thumbs instead of one, so it can show a minimum and maximum value.
									 @see setMinValue, setMaxValue */

		ThreeValueHorizontal,   /**< A horizontal slider that has three thumbs instead of one, so it can show a minimum and maximum
									 value, with the current value being somewhere between them.
									 @see setMinValue, setMaxValue */
		ThreeValueVertical,	 /**< A vertical slider that has three thumbs instead of one, so it can show a minimum and maximum
									 value, with the current value being somewhere between them.
									 @see setMinValue, setMaxValue */
	};

	void setSliderStyle (SliderStyle newStyle);

	SliderStyle getSliderStyle() const					  { return style; }

	void setRotaryParameters (float startAngleRadians,
							  float endAngleRadians,
							  bool stopAtEnd);

	void setMouseDragSensitivity (int distanceForFullScaleDrag);

	void setVelocityBasedMode (bool isVelocityBased);

	bool getVelocityBasedMode() const			   { return isVelocityBased; }

	void setVelocityModeParameters (double sensitivity = 1.0,
									int threshold = 1,
									double offset = 0.0,
									bool userCanPressKeyToSwapMode = true);

	double getVelocitySensitivity() const			   { return velocityModeSensitivity; }

	int getVelocityThreshold() const				{ return velocityModeThreshold; }

	double getVelocityOffset() const				{ return velocityModeOffset; }

	bool getVelocityModeIsSwappable() const			 { return userKeyOverridesVelocity; }

	void setSkewFactor (double factor);

	void setSkewFactorFromMidPoint (double sliderValueToShowAtMidPoint);

	double getSkewFactor() const				{ return skewFactor; }

	enum IncDecButtonMode
	{
		incDecButtonsNotDraggable,
		incDecButtonsDraggable_AutoDirection,
		incDecButtonsDraggable_Horizontal,
		incDecButtonsDraggable_Vertical
	};

	void setIncDecButtonsMode (IncDecButtonMode mode);

	enum TextEntryBoxPosition
	{
		NoTextBox,		  /**< Doesn't display a text box.  */
		TextBoxLeft,		/**< Puts the text box to the left of the slider, vertically centred.  */
		TextBoxRight,	   /**< Puts the text box to the right of the slider, vertically centred.  */
		TextBoxAbove,	   /**< Puts the text box above the slider, horizontally centred.  */
		TextBoxBelow		/**< Puts the text box below the slider, horizontally centred.  */
	};

	void setTextBoxStyle (TextEntryBoxPosition newPosition,
						  bool isReadOnly,
						  int textEntryBoxWidth,
						  int textEntryBoxHeight);

	const TextEntryBoxPosition getTextBoxPosition() const		   { return textBoxPos; }

	int getTextBoxWidth() const						 { return textBoxWidth; }

	int getTextBoxHeight() const						{ return textBoxHeight; }

	void setTextBoxIsEditable (bool shouldBeEditable);

	bool isTextBoxEditable() const					  { return editableText; }

	void showTextBox();

	void hideTextBox (bool discardCurrentEditorContents);

	void setValue (double newValue,
				   bool sendUpdateMessage = true,
				   bool sendMessageSynchronously = false);

	double getValue() const;

	Value& getValueObject()						 { return currentValue; }

	void setRange (double newMinimum,
				   double newMaximum,
				   double newInterval = 0);

	double getMaximum() const						   { return maximum; }

	double getMinimum() const						   { return minimum; }

	double getInterval() const						  { return interval; }

	double getMinValue() const;

	Value& getMinValueObject()						  { return valueMin; }

	void setMinValue (double newValue,
					  bool sendUpdateMessage = true,
					  bool sendMessageSynchronously = false,
					  bool allowNudgingOfOtherValues = false);

	double getMaxValue() const;

	Value& getMaxValueObject()						  { return valueMax; }

	void setMaxValue (double newValue,
					  bool sendUpdateMessage = true,
					  bool sendMessageSynchronously = false,
					  bool allowNudgingOfOtherValues = false);

	void addListener (SliderListener* listener);

	void removeListener (SliderListener* listener);

	void setDoubleClickReturnValue (bool isDoubleClickEnabled,
									double valueToSetOnDoubleClick);

	double getDoubleClickReturnValue (bool& isEnabled) const;

	void setChangeNotificationOnlyOnRelease (bool onlyNotifyOnRelease);

	void setSliderSnapsToMousePosition (bool shouldSnapToMouse);

	void setPopupDisplayEnabled (bool isEnabled,
								 Component* parentComponentToUse);

	void setPopupMenuEnabled (bool menuEnabled);

	void setScrollWheelEnabled (bool enabled);

	int getThumbBeingDragged() const		{ return sliderBeingDragged; }

	virtual void startedDragging();

	virtual void stoppedDragging();

	virtual void valueChanged();

	/** Callback to indicate that the user has just moved the slider.
		Note - the valueChanged() method has changed its format and now no longer has
		any parameters. Update your code to use the new version.
		This version has been left here with an int as its return value to cause
		a syntax error if you've got existing code that uses the old version.
	*/
	virtual int valueChanged (double) { jassertfalse; return 0; }

	virtual double getValueFromText (const String& text);

	virtual const String getTextFromValue (double value);

	void setTextValueSuffix (const String& suffix);

	virtual double proportionOfLengthToValue (double proportion);

	virtual double valueToProportionOfLength (double value);

	float getPositionOfValue (double value);

	virtual double snapValue (double attemptedValue, bool userIsDragging);

	void updateText();

	bool isHorizontal() const;
	bool isVertical() const;

	enum ColourIds
	{
		backgroundColourId	  = 0x1001200,  /**< A colour to use to fill the slider's background. */
		thumbColourId		   = 0x1001300,  /**< The colour to draw the thumb with. It's up to the look
													   and feel class how this is used. */
		trackColourId		   = 0x1001310,  /**< The colour to draw the groove that the thumb moves along. */
		rotarySliderFillColourId	= 0x1001311,  /**< For rotary sliders, this colour fills the outer curve. */
		rotarySliderOutlineColourId = 0x1001312,  /**< For rotary sliders, this colour is used to draw the outer curve's outline. */

		textBoxTextColourId	 = 0x1001400,  /**< The colour for the text in the text-editor box used for editing the value. */
		textBoxBackgroundColourId   = 0x1001500,  /**< The background colour for the text-editor box. */
		textBoxHighlightColourId	= 0x1001600,  /**< The text highlight colour for the text-editor box. */
		textBoxOutlineColourId	  = 0x1001700   /**< The colour to use for a border around the text-editor box. */
	};

	juce_UseDebuggingNewOperator

protected:
	void labelTextChanged (Label*);
	void paint (Graphics& g);
	void resized();
	void mouseDown (const MouseEvent& e);
	void mouseUp (const MouseEvent& e);
	void mouseDrag (const MouseEvent& e);
	void mouseDoubleClick (const MouseEvent& e);
	void mouseWheelMove (const MouseEvent& e, float wheelIncrementX, float wheelIncrementY);
	void modifierKeysChanged (const ModifierKeys& modifiers);
	void buttonClicked (Button* button);
	void lookAndFeelChanged();
	void enablementChanged();
	void focusOfChildComponentChanged (FocusChangeType cause);
	void handleAsyncUpdate();
	void colourChanged();
	void valueChanged (Value& value);

private:
	ListenerList <SliderListener> listeners;
	Value currentValue, valueMin, valueMax;
	double lastCurrentValue, lastValueMin, lastValueMax;
	double minimum, maximum, interval, doubleClickReturnValue;
	double valueWhenLastDragged, valueOnMouseDown, skewFactor, lastAngle;
	double velocityModeSensitivity, velocityModeOffset, minMaxDiff;
	int velocityModeThreshold;
	float rotaryStart, rotaryEnd;
	int numDecimalPlaces, mouseXWhenLastDragged, mouseYWhenLastDragged;
	int mouseDragStartX, mouseDragStartY;
	int sliderRegionStart, sliderRegionSize;
	int sliderBeingDragged;
	int pixelsForFullDragExtent;
	Rectangle<int> sliderRect;
	String textSuffix;

	SliderStyle style;
	TextEntryBoxPosition textBoxPos;
	int textBoxWidth, textBoxHeight;
	IncDecButtonMode incDecButtonMode;

	bool editableText : 1, doubleClickToValue : 1;
	bool isVelocityBased : 1, userKeyOverridesVelocity : 1, rotaryStop : 1;
	bool incDecButtonsSideBySide : 1, sendChangeOnlyOnRelease : 1, popupDisplayEnabled : 1;
	bool menuEnabled : 1, menuShown : 1, mouseWasHidden : 1, incDecDragged : 1;
	bool scrollWheelEnabled : 1, snapsToMousePos : 1;
	Font font;
	Label* valueBox;
	Button* incButton;
	Button* decButton;
	ScopedPointer <Component> popupDisplay;
	Component* parentForPopupDisplay;

	float getLinearSliderPos (double value);
	void restoreMouseIfHidden();
	void sendDragStart();
	void sendDragEnd();
	double constrainedValue (double value) const;
	void triggerChangeMessage (bool synchronous);
	bool incDecDragDirectionIsHorizontal() const;

	Slider (const Slider&);
	Slider& operator= (const Slider&);
};

#endif   // __JUCE_SLIDER_JUCEHEADER__
/*** End of inlined file: juce_Slider.h ***/


#endif
#ifndef __JUCE_SLIDERLISTENER_JUCEHEADER__

#endif
#ifndef __JUCE_TABLEHEADERCOMPONENT_JUCEHEADER__

/*** Start of inlined file: juce_TableHeaderComponent.h ***/
#ifndef __JUCE_TABLEHEADERCOMPONENT_JUCEHEADER__
#define __JUCE_TABLEHEADERCOMPONENT_JUCEHEADER__

class TableHeaderComponent;

class JUCE_API  TableHeaderListener
{
public:

	TableHeaderListener() {}

	virtual ~TableHeaderListener() {}

	virtual void tableColumnsChanged (TableHeaderComponent* tableHeader) = 0;

	virtual void tableColumnsResized (TableHeaderComponent* tableHeader) = 0;

	virtual void tableSortOrderChanged (TableHeaderComponent* tableHeader) = 0;

	virtual void tableColumnDraggingChanged (TableHeaderComponent* tableHeader,
											 int columnIdNowBeingDragged);
};

class JUCE_API  TableHeaderComponent   : public Component,
										 private AsyncUpdater
{
public:

	TableHeaderComponent();

	~TableHeaderComponent();

	enum ColumnPropertyFlags
	{
		visible			 = 1,	/**< If this is set, the column will be shown; if not, it will be hidden until the user enables it with the pop-up menu. */
		resizable		   = 2,	/**< If this is set, the column can be resized by dragging it. */
		draggable		   = 4,	/**< If this is set, the column can be dragged around to change its order in the table. */
		appearsOnColumnMenu	 = 8,	/**< If this is set, the column will be shown on the pop-up menu allowing it to be hidden/shown. */
		sortable			= 16,   /**< If this is set, then clicking on the column header will set it to be the sort column, and clicking again will reverse the order. */
		sortedForwards		  = 32,   /**< If this is set, the column is currently the one by which the table is sorted (forwards). */
		sortedBackwards		 = 64,   /**< If this is set, the column is currently the one by which the table is sorted (backwards). */

		defaultFlags		= (visible | resizable | draggable | appearsOnColumnMenu | sortable),

		notResizable		= (visible | draggable | appearsOnColumnMenu | sortable),

		notResizableOrSortable	  = (visible | draggable | appearsOnColumnMenu),

		notSortable		 = (visible | resizable | draggable | appearsOnColumnMenu)
	};

	void addColumn (const String& columnName,
					int columnId,
					int width,
					int minimumWidth = 30,
					int maximumWidth = -1,
					int propertyFlags = defaultFlags,
					int insertIndex = -1);

	void removeColumn (int columnIdToRemove);

	void removeAllColumns();

	int getNumColumns (bool onlyCountVisibleColumns) const;

	const String getColumnName (int columnId) const;

	void setColumnName (int columnId, const String& newName);

	void moveColumn (int columnId, int newVisibleIndex);

	int getColumnWidth (int columnId) const;

	void setColumnWidth (int columnId, int newWidth);

	void setColumnVisible (int columnId, bool shouldBeVisible);

	bool isColumnVisible (int columnId) const;

	void setSortColumnId (int columnId, bool sortForwards);

	int getSortColumnId() const;

	bool isSortedForwards() const;

	void reSortTable();

	int getTotalWidth() const;

	int getIndexOfColumnId (int columnId, bool onlyCountVisibleColumns) const;

	int getColumnIdOfIndex (int index, bool onlyCountVisibleColumns) const;

	const Rectangle<int> getColumnPosition (int index) const;

	int getColumnIdAtX (int xToFind) const;

	void setStretchToFitActive (bool shouldStretchToFit);

	bool isStretchToFitActive() const;

	void resizeAllColumnsToFit (int targetTotalWidth);

	void setPopupMenuActive (bool hasMenu);

	bool isPopupMenuActive() const;

	const String toString() const;

	void restoreFromString (const String& storedVersion);

	void addListener (TableHeaderListener* newListener);

	void removeListener (TableHeaderListener* listenerToRemove);

	virtual void columnClicked (int columnId, const ModifierKeys& mods);

	virtual void addMenuItems (PopupMenu& menu, int columnIdClicked);

	virtual void reactToMenuItem (int menuReturnId, int columnIdClicked);

	void paint (Graphics& g);
	void resized();
	void mouseMove (const MouseEvent&);
	void mouseEnter (const MouseEvent&);
	void mouseExit (const MouseEvent&);
	void mouseDown (const MouseEvent&);
	void mouseDrag (const MouseEvent&);
	void mouseUp (const MouseEvent&);
	const MouseCursor getMouseCursor();

	virtual void showColumnChooserMenu (int columnIdClicked);

	juce_UseDebuggingNewOperator

private:
	struct ColumnInfo
	{
		String name;
		int id, propertyFlags, width, minimumWidth, maximumWidth;
		double lastDeliberateWidth;

		bool isVisible() const;
	};

	OwnedArray <ColumnInfo> columns;
	Array <TableHeaderListener*> listeners;
	ScopedPointer <Component> dragOverlayComp;

	bool columnsChanged, columnsResized, sortChanged, menuActive, stretchToFit;
	int columnIdBeingResized, columnIdBeingDragged, initialColumnWidth;
	int columnIdUnderMouse, draggingColumnOffset, draggingColumnOriginalIndex, lastDeliberateWidth;

	ColumnInfo* getInfoForId (int columnId) const;
	int visibleIndexToTotalIndex (int visibleIndex) const;
	void sendColumnsChanged();
	void handleAsyncUpdate();
	void beginDrag (const MouseEvent&);
	void endDrag (int finalIndex);
	int getResizeDraggerAt (int mouseX) const;
	void updateColumnUnderMouse (int x, int y);
	void resizeColumnsToFit (int firstColumnIndex, int targetTotalWidth);

	TableHeaderComponent (const TableHeaderComponent&);
	TableHeaderComponent operator= (const TableHeaderComponent&);
};

#endif   // __JUCE_TABLEHEADERCOMPONENT_JUCEHEADER__
/*** End of inlined file: juce_TableHeaderComponent.h ***/


#endif
#ifndef __JUCE_TABLELISTBOX_JUCEHEADER__

/*** Start of inlined file: juce_TableListBox.h ***/
#ifndef __JUCE_TABLELISTBOX_JUCEHEADER__
#define __JUCE_TABLELISTBOX_JUCEHEADER__

class JUCE_API  TableListBoxModel
{
public:

	TableListBoxModel()  {}

	virtual ~TableListBoxModel()  {}

	virtual int getNumRows() = 0;

	virtual void paintRowBackground (Graphics& g,
									 int rowNumber,
									 int width, int height,
									 bool rowIsSelected) = 0;

	virtual void paintCell (Graphics& g,
							int rowNumber,
							int columnId,
							int width, int height,
							bool rowIsSelected) = 0;

	virtual Component* refreshComponentForCell (int rowNumber, int columnId, bool isRowSelected,
												Component* existingComponentToUpdate);

	virtual void cellClicked (int rowNumber, int columnId, const MouseEvent& e);

	virtual void cellDoubleClicked (int rowNumber, int columnId, const MouseEvent& e);

	virtual void backgroundClicked();

	virtual void sortOrderChanged (int newSortColumnId, bool isForwards);

	virtual int getColumnAutoSizeWidth (int columnId);

	virtual const String getCellTooltip (int rowNumber, int columnId);

	virtual void selectedRowsChanged (int lastRowSelected);

	virtual void deleteKeyPressed (int lastRowSelected);

	virtual void returnKeyPressed (int lastRowSelected);

	virtual void listWasScrolled();

	virtual const String getDragSourceDescription (const SparseSet<int>& currentlySelectedRows);
};

class JUCE_API  TableListBox   : public ListBox,
								 private ListBoxModel,
								 private TableHeaderListener
{
public:

	TableListBox (const String& componentName,
				  TableListBoxModel* model);

	~TableListBox();

	void setModel (TableListBoxModel* newModel);

	TableListBoxModel* getModel() const				 { return model; }

	TableHeaderComponent* getHeader() const			 { return header; }

	void setHeaderHeight (int newHeight);

	int getHeaderHeight() const;

	void autoSizeColumn (int columnId);

	void autoSizeAllColumns();

	void setAutoSizeMenuOptionShown (bool shouldBeShown);

	bool isAutoSizeMenuOptionShown() const;

	const Rectangle<int> getCellPosition (int columnId,
										  int rowNumber,
										  bool relativeToComponentTopLeft) const;

	void scrollToEnsureColumnIsOnscreen (int columnId);

	int getNumRows();
	void paintListBoxItem (int, Graphics&, int, int, bool);
	Component* refreshComponentForRow (int rowNumber, bool isRowSelected, Component* existingComponentToUpdate);
	void selectedRowsChanged (int lastRowSelected);
	void deleteKeyPressed (int currentSelectedRow);
	void returnKeyPressed (int currentSelectedRow);
	void backgroundClicked();
	void listWasScrolled();
	void tableColumnsChanged (TableHeaderComponent*);
	void tableColumnsResized (TableHeaderComponent*);
	void tableSortOrderChanged (TableHeaderComponent*);
	void tableColumnDraggingChanged (TableHeaderComponent*, int);
	void resized();

	juce_UseDebuggingNewOperator

private:
	TableHeaderComponent* header;
	TableListBoxModel* model;
	int columnIdNowBeingDragged;
	bool autoSizeOptionsShown;

	void updateColumnComponents() const;

	TableListBox (const TableListBox&);
	TableListBox& operator= (const TableListBox&);
};

#endif   // __JUCE_TABLELISTBOX_JUCEHEADER__
/*** End of inlined file: juce_TableListBox.h ***/


#endif
#ifndef __JUCE_TEXTEDITOR_JUCEHEADER__

#endif
#ifndef __JUCE_TOOLBAR_JUCEHEADER__

#endif
#ifndef __JUCE_TOOLBARITEMCOMPONENT_JUCEHEADER__

#endif
#ifndef __JUCE_TOOLBARITEMFACTORY_JUCEHEADER__

/*** Start of inlined file: juce_ToolbarItemFactory.h ***/
#ifndef __JUCE_TOOLBARITEMFACTORY_JUCEHEADER__
#define __JUCE_TOOLBARITEMFACTORY_JUCEHEADER__

class JUCE_API  ToolbarItemFactory
{
public:

	ToolbarItemFactory();

	virtual ~ToolbarItemFactory();

	enum SpecialItemIds
	{
		separatorBarId	  = -1,   /**< The item ID for a vertical (or horizontal) separator bar that
										 can be placed between sets of items to break them into groups. */
		spacerId		= -2,   /**< The item ID for a fixed-width space that can be placed between
										 items.*/
		flexibleSpacerId	= -3	/**< The item ID for a gap that pushes outwards against the things on
										 either side of it, filling any available space. */
	};

	virtual void getAllToolbarItemIds (Array <int>& ids) = 0;

	virtual void getDefaultItemSet (Array <int>& ids) = 0;

	virtual ToolbarItemComponent* createItem (int itemId) = 0;
};

#endif   // __JUCE_TOOLBARITEMFACTORY_JUCEHEADER__
/*** End of inlined file: juce_ToolbarItemFactory.h ***/


#endif
#ifndef __JUCE_TOOLBARITEMPALETTE_JUCEHEADER__

/*** Start of inlined file: juce_ToolbarItemPalette.h ***/
#ifndef __JUCE_TOOLBARITEMPALETTE_JUCEHEADER__
#define __JUCE_TOOLBARITEMPALETTE_JUCEHEADER__

class JUCE_API  ToolbarItemPalette	: public Component,
										public DragAndDropContainer
{
public:

	ToolbarItemPalette (ToolbarItemFactory& factory,
						Toolbar* toolbar);

	~ToolbarItemPalette();

	void resized();

	juce_UseDebuggingNewOperator

private:
	ToolbarItemFactory& factory;
	Toolbar* toolbar;
	Viewport* viewport;

	friend class Toolbar;
	void replaceComponent (ToolbarItemComponent* comp);

	ToolbarItemPalette (const ToolbarItemPalette&);
	ToolbarItemPalette& operator= (const ToolbarItemPalette&);
};

#endif   // __JUCE_TOOLBARITEMPALETTE_JUCEHEADER__
/*** End of inlined file: juce_ToolbarItemPalette.h ***/


#endif
#ifndef __JUCE_TREEVIEW_JUCEHEADER__

/*** Start of inlined file: juce_TreeView.h ***/
#ifndef __JUCE_TREEVIEW_JUCEHEADER__
#define __JUCE_TREEVIEW_JUCEHEADER__


/*** Start of inlined file: juce_FileDragAndDropTarget.h ***/
#ifndef __JUCE_FILEDRAGANDDROPTARGET_JUCEHEADER__
#define __JUCE_FILEDRAGANDDROPTARGET_JUCEHEADER__

class JUCE_API  FileDragAndDropTarget
{
public:
	virtual ~FileDragAndDropTarget()  {}

	virtual bool isInterestedInFileDrag (const StringArray& files) = 0;

	virtual void fileDragEnter (const StringArray& files, int x, int y);

	virtual void fileDragMove (const StringArray& files, int x, int y);

	virtual void fileDragExit (const StringArray& files);

	virtual void filesDropped (const StringArray& files, int x, int y) = 0;
};

#endif   // __JUCE_FILEDRAGANDDROPTARGET_JUCEHEADER__
/*** End of inlined file: juce_FileDragAndDropTarget.h ***/

class TreeView;

class JUCE_API  TreeViewItem
{
public:

	TreeViewItem();

	virtual ~TreeViewItem();

	int getNumSubItems() const throw();

	TreeViewItem* getSubItem (int index) const throw();

	void clearSubItems();

	void addSubItem (TreeViewItem* newItem, int insertPosition = -1);

	void removeSubItem (int index, bool deleteItem = true);

	TreeView* getOwnerView() const throw()		  { return ownerView; }

	TreeViewItem* getParentItem() const throw()	 { return parentItem; }

	bool isOpen() const throw();

	void setOpen (bool shouldBeOpen);

	bool isSelected() const throw();

	void setSelected (bool shouldBeSelected,
					  bool deselectOtherItemsFirst);

	const Rectangle<int> getItemPosition (bool relativeToTreeViewTopLeft) const throw();

	void treeHasChanged() const throw();

	void repaintItem() const;

	int getRowNumberInTree() const throw();

	bool areAllParentsOpen() const throw();

	void setLinesDrawnForSubItems (bool shouldDrawLines) throw();

	virtual bool mightContainSubItems() = 0;

	virtual const String getUniqueName() const;

	virtual void itemOpennessChanged (bool isNowOpen);

	virtual int getItemWidth() const				{ return -1; }

	virtual int getItemHeight() const				   { return 20; }

	virtual bool canBeSelected() const				  { return true; }

	virtual Component* createItemComponent()			{ return 0; }

	virtual void paintItem (Graphics& g, int width, int height);

	virtual void paintOpenCloseButton (Graphics& g, int width, int height, bool isMouseOver);

	virtual void itemClicked (const MouseEvent& e);

	virtual void itemDoubleClicked (const MouseEvent& e);

	virtual void itemSelectionChanged (bool isNowSelected);

	virtual const String getTooltip();

	virtual const String getDragSourceDescription();

	virtual bool isInterestedInFileDrag (const StringArray& files);

	virtual void filesDropped (const StringArray& files, int insertIndex);

	virtual bool isInterestedInDragSource (const String& sourceDescription, Component* sourceComponent);

	virtual void itemDropped (const String& sourceDescription, Component* sourceComponent, int insertIndex);

	void setDrawsInLeftMargin (bool canDrawInLeftMargin) throw();

	XmlElement* getOpennessState() const throw();

	void restoreOpennessState (const XmlElement& xml) throw();

	int getIndexInParent() const throw();

	bool isLastOfSiblings() const throw();

	const String getItemIdentifierString() const;

	juce_UseDebuggingNewOperator

private:
	TreeView* ownerView;
	TreeViewItem* parentItem;
	OwnedArray <TreeViewItem> subItems;
	int y, itemHeight, totalHeight, itemWidth, totalWidth;
	int uid;
	bool selected	   : 1;
	bool redrawNeeded	   : 1;
	bool drawLinesInside	: 1;
	bool drawsInLeftMargin  : 1;
	unsigned int openness   : 2;

	friend class TreeView;
	friend class TreeViewContentComponent;

	void updatePositions (int newY);
	int getIndentX() const throw();
	void setOwnerView (TreeView* newOwner) throw();
	void paintRecursively (Graphics& g, int width);
	TreeViewItem* getTopLevelItem() throw();
	TreeViewItem* findItemRecursively (int y) throw();
	TreeViewItem* getDeepestOpenParentItem() throw();
	int getNumRows() const throw();
	TreeViewItem* getItemOnRow (int index) throw();
	void deselectAllRecursively();
	int countSelectedItemsRecursively() const throw();
	TreeViewItem* getSelectedItemWithIndex (int index) throw();
	TreeViewItem* getNextVisibleItem (bool recurse) const throw();
	TreeViewItem* findItemFromIdentifierString (const String& identifierString);

	TreeViewItem (const TreeViewItem&);
	TreeViewItem& operator= (const TreeViewItem&);
};

class JUCE_API  TreeView  : public Component,
							public SettableTooltipClient,
							public FileDragAndDropTarget,
							public DragAndDropTarget,
							private AsyncUpdater
{
public:

	TreeView (const String& componentName = String::empty);

	~TreeView();

	void setRootItem (TreeViewItem* newRootItem);

	TreeViewItem* getRootItem() const throw()			   { return rootItem; }

	void deleteRootItem();

	void setRootItemVisible (bool shouldBeVisible);

	bool isRootItemVisible() const throw()			  { return rootItemVisible; }

	void setDefaultOpenness (bool isOpenByDefault);

	bool areItemsOpenByDefault() const throw()			  { return defaultOpenness; }

	void setMultiSelectEnabled (bool canMultiSelect);

	bool isMultiSelectEnabled() const throw()			   { return multiSelectEnabled; }

	void setOpenCloseButtonsVisible (bool shouldBeVisible);

	bool areOpenCloseButtonsVisible() const throw()		 { return openCloseButtonsVisible; }

	void clearSelectedItems();

	int getNumSelectedItems() const throw();

	TreeViewItem* getSelectedItem (int index) const throw();

	int getNumRowsInTree() const;

	TreeViewItem* getItemOnRow (int index) const;

	TreeViewItem* getItemAt (int yPosition) const throw();

	void scrollToKeepItemVisible (TreeViewItem* item);

	Viewport* getViewport() const throw()			   { return viewport; }

	int getIndentSize() const throw()				   { return indentSize; }

	void setIndentSize (int newIndentSize);

	TreeViewItem* findItemFromIdentifierString (const String& identifierString) const;

	XmlElement* getOpennessState (bool alsoIncludeScrollPosition) const;

	void restoreOpennessState (const XmlElement& newState);

	enum ColourIds
	{
		backgroundColourId		= 0x1000500, /**< A background colour to fill the component with. */
		linesColourId		 = 0x1000501, /**< The colour to draw the lines with.*/
		dragAndDropIndicatorColourId  = 0x1000502  /**< The colour to use for the drag-and-drop target position indicator. */
	};

	void paint (Graphics& g);
	void resized();
	bool keyPressed (const KeyPress& key);
	void colourChanged();
	void enablementChanged();
	bool isInterestedInFileDrag (const StringArray& files);
	void fileDragEnter (const StringArray& files, int x, int y);
	void fileDragMove (const StringArray& files, int x, int y);
	void fileDragExit (const StringArray& files);
	void filesDropped (const StringArray& files, int x, int y);
	bool isInterestedInDragSource (const String& sourceDescription, Component* sourceComponent);
	void itemDragEnter (const String& sourceDescription, Component* sourceComponent, int x, int y);
	void itemDragMove (const String& sourceDescription, Component* sourceComponent, int x, int y);
	void itemDragExit (const String& sourceDescription, Component* sourceComponent);
	void itemDropped (const String& sourceDescription, Component* sourceComponent, int x, int y);

	juce_UseDebuggingNewOperator

private:
	friend class TreeViewItem;
	friend class TreeViewContentComponent;
	Viewport* viewport;
	CriticalSection nodeAlterationLock;
	TreeViewItem* rootItem;
	Component* dragInsertPointHighlight;
	Component* dragTargetGroupHighlight;
	int indentSize;
	bool defaultOpenness : 1;
	bool needsRecalculating : 1;
	bool rootItemVisible : 1;
	bool multiSelectEnabled : 1;
	bool openCloseButtonsVisible : 1;

	void itemsChanged() throw();
	void handleAsyncUpdate();
	void moveSelectedRow (int delta);
	void updateButtonUnderMouse (const MouseEvent& e);
	void showDragHighlight (TreeViewItem* item, int insertIndex, int x, int y) throw();
	void hideDragHighlight() throw();
	void handleDrag (const StringArray& files, const String& sourceDescription, Component* sourceComponent, int x, int y);
	void handleDrop (const StringArray& files, const String& sourceDescription, Component* sourceComponent, int x, int y);
	TreeViewItem* getInsertPosition (int& x, int& y, int& insertIndex,
									 const StringArray& files, const String& sourceDescription,
									 Component* sourceComponent) const throw();

	TreeView (const TreeView&);
	TreeView& operator= (const TreeView&);
};

#endif   // __JUCE_TREEVIEW_JUCEHEADER__
/*** End of inlined file: juce_TreeView.h ***/


#endif
#ifndef __JUCE_DIRECTORYCONTENTSDISPLAYCOMPONENT_JUCEHEADER__

/*** Start of inlined file: juce_DirectoryContentsDisplayComponent.h ***/
#ifndef __JUCE_DIRECTORYCONTENTSDISPLAYCOMPONENT_JUCEHEADER__
#define __JUCE_DIRECTORYCONTENTSDISPLAYCOMPONENT_JUCEHEADER__


/*** Start of inlined file: juce_DirectoryContentsList.h ***/
#ifndef __JUCE_DIRECTORYCONTENTSLIST_JUCEHEADER__
#define __JUCE_DIRECTORYCONTENTSLIST_JUCEHEADER__


/*** Start of inlined file: juce_FileFilter.h ***/
#ifndef __JUCE_FILEFILTER_JUCEHEADER__
#define __JUCE_FILEFILTER_JUCEHEADER__

class JUCE_API  FileFilter
{
public:

	FileFilter (const String& filterDescription);

	virtual ~FileFilter();

	const String& getDescription() const throw();

	virtual bool isFileSuitable (const File& file) const = 0;

	virtual bool isDirectorySuitable (const File& file) const = 0;

protected:

	String description;
};

#endif   // __JUCE_FILEFILTER_JUCEHEADER__
/*** End of inlined file: juce_FileFilter.h ***/


/*** Start of inlined file: juce_Image.h ***/
#ifndef __JUCE_IMAGE_JUCEHEADER__
#define __JUCE_IMAGE_JUCEHEADER__

class JUCE_API  Image
{
public:

	enum PixelFormat
	{
		RGB,		/**<< each pixel is a 3-byte packed RGB colour value. For byte order, see the PixelRGB class. */
		ARGB,		   /**<< each pixel is a 4-byte ARGB premultiplied colour value. For byte order, see the PixelARGB class. */
		SingleChannel	   /**<< each pixel is a 1-byte alpha channel value. */
	};

	Image (PixelFormat format,
		   int imageWidth,
		   int imageHeight,
		   bool clearImage);

	Image (const Image& other);

	virtual ~Image();

	static Image* createNativeImage (PixelFormat format,
									 int imageWidth,
									 int imageHeight,
									 bool clearImage);

	int getWidth() const throw()			{ return imageWidth; }

	int getHeight() const throw()		   { return imageHeight; }

	const Rectangle<int> getBounds() const throw()  { return Rectangle<int> (0, 0, imageWidth, imageHeight); }

	PixelFormat getFormat() const throw()	   { return format; }

	bool isARGB() const throw()			 { return format == ARGB; }

	bool isRGB() const throw()			  { return format == RGB; }

	bool hasAlphaChannel() const throw()		{ return format != RGB; }

	virtual void clear (int x, int y, int w, int h,
						const Colour& colourToClearTo = Colour (0x00000000));

	virtual Image* createCopy (int newWidth = -1,
							   int newHeight = -1,
							   Graphics::ResamplingQuality quality = Graphics::mediumResamplingQuality) const;

	virtual Image* createCopyOfAlphaChannel() const;

	virtual const Colour getPixelAt (int x, int y) const;

	virtual void setPixelAt (int x, int y, const Colour& colour);

	virtual void multiplyAlphaAt (int x, int y, float multiplier);

	virtual void multiplyAllAlphas (float amountToMultiplyBy);

	virtual void desaturate();

	class BitmapData
	{
	public:
		BitmapData (Image& image, int x, int y, int w, int h, bool needsToBeWritable);
		BitmapData (const Image& image, int x, int y, int w, int h);
		~BitmapData();

		inline uint8* getLinePointer (int y) const			  { return data + y * lineStride; }

		inline uint8* getPixelPointer (int x, int y) const		  { return data + y * lineStride + x * pixelStride; }

		uint8* data;
		int lineStride, pixelStride, width, height;

	private:
		BitmapData (const BitmapData&);
		BitmapData& operator= (const BitmapData&);
	};

	virtual void setPixelData (int destX, int destY, int destW, int destH,
							   const uint8* sourcePixelData, int sourceLineStride);

	virtual void moveImageSection (int destX, int destY,
								   int sourceX, int sourceY,
								   int width, int height);

	void createSolidAreaMask (RectangleList& result,
							  float alphaThreshold = 0.5f) const;

	juce_UseDebuggingNewOperator

	virtual LowLevelGraphicsContext* createLowLevelContext();

protected:
	friend class BitmapData;
	const PixelFormat format;
	const int imageWidth, imageHeight;

	Image (PixelFormat format,
		   int imageWidth,
		   int imageHeight);

	int pixelStride, lineStride;
	HeapBlock <uint8> imageDataAllocated;
	uint8* imageData;

private:

	Image& operator= (const Image&);
};

#endif   // __JUCE_IMAGE_JUCEHEADER__
/*** End of inlined file: juce_Image.h ***/

class JUCE_API  DirectoryContentsList   : public ChangeBroadcaster,
										  public TimeSliceClient
{
public:

	DirectoryContentsList (const FileFilter* fileFilter,
						   TimeSliceThread& threadToUse);

	~DirectoryContentsList();

	void setDirectory (const File& directory,
					   bool includeDirectories,
					   bool includeFiles);

	const File& getDirectory() const;

	void clear();

	void refresh();

	bool isStillLoading() const;

	void setIgnoresHiddenFiles (bool shouldIgnoreHiddenFiles);

	bool ignoresHiddenFiles() const		 { return ignoreHiddenFiles; }

	struct FileInfo
	{

		String filename;

		int64 fileSize;

		Time modificationTime;

		Time creationTime;

		bool isDirectory;

		bool isReadOnly;
	};

	int getNumFiles() const;

	bool getFileInfo (int index, FileInfo& resultInfo) const;

	const File getFile (int index) const;

	const FileFilter* getFilter() const			 { return fileFilter; }

	bool useTimeSlice();
	TimeSliceThread& getTimeSliceThread()		   { return thread; }
	static int compareElements (const DirectoryContentsList::FileInfo* first,
								const DirectoryContentsList::FileInfo* second);

	juce_UseDebuggingNewOperator

private:
	File root;
	const FileFilter* fileFilter;
	TimeSliceThread& thread;
	bool includeDirectories, includeFiles, ignoreHiddenFiles;

	CriticalSection fileListLock;
	OwnedArray <FileInfo> files;

	void* volatile fileFindHandle;
	bool volatile shouldStop;

	void changed();
	bool checkNextFile (bool& hasChanged);
	bool addFile (const String& filename, bool isDir, bool isHidden,
				  const int64 fileSize, const Time& modTime,
				  const Time& creationTime, bool isReadOnly);

	DirectoryContentsList (const DirectoryContentsList&);
	DirectoryContentsList& operator= (const DirectoryContentsList&);
};

#endif   // __JUCE_DIRECTORYCONTENTSLIST_JUCEHEADER__
/*** End of inlined file: juce_DirectoryContentsList.h ***/


/*** Start of inlined file: juce_FileBrowserListener.h ***/
#ifndef __JUCE_FILEBROWSERLISTENER_JUCEHEADER__
#define __JUCE_FILEBROWSERLISTENER_JUCEHEADER__

class JUCE_API  FileBrowserListener
{
public:

	virtual ~FileBrowserListener();

	virtual void selectionChanged() = 0;

	virtual void fileClicked (const File& file, const MouseEvent& e) = 0;

	virtual void fileDoubleClicked (const File& file) = 0;
};

#endif   // __JUCE_FILEBROWSERLISTENER_JUCEHEADER__
/*** End of inlined file: juce_FileBrowserListener.h ***/

class JUCE_API  DirectoryContentsDisplayComponent
{
public:

	DirectoryContentsDisplayComponent (DirectoryContentsList& listToShow);

	virtual ~DirectoryContentsDisplayComponent();

	virtual int getNumSelectedFiles() const = 0;

	virtual const File getSelectedFile (int index) const = 0;

	virtual void scrollToTop() = 0;

	void addListener (FileBrowserListener* listener);

	void removeListener (FileBrowserListener* listener);

	enum ColourIds
	{
		highlightColourId	  = 0x1000540, /**< The colour to use to fill a highlighted row of the list. */
		textColourId	   = 0x1000541, /**< The colour for the text. */
	};

	void sendSelectionChangeMessage();
	void sendDoubleClickMessage (const File& file);
	void sendMouseClickMessage (const File& file, const MouseEvent& e);

	juce_UseDebuggingNewOperator

protected:
	DirectoryContentsList& fileList;
	ListenerList <FileBrowserListener> listeners;

	DirectoryContentsDisplayComponent (const DirectoryContentsDisplayComponent&);
	DirectoryContentsDisplayComponent& operator= (const DirectoryContentsDisplayComponent&);
};

#endif   // __JUCE_DIRECTORYCONTENTSDISPLAYCOMPONENT_JUCEHEADER__
/*** End of inlined file: juce_DirectoryContentsDisplayComponent.h ***/


#endif
#ifndef __JUCE_DIRECTORYCONTENTSLIST_JUCEHEADER__

#endif
#ifndef __JUCE_FILEBROWSERCOMPONENT_JUCEHEADER__

/*** Start of inlined file: juce_FileBrowserComponent.h ***/
#ifndef __JUCE_FILEBROWSERCOMPONENT_JUCEHEADER__
#define __JUCE_FILEBROWSERCOMPONENT_JUCEHEADER__


/*** Start of inlined file: juce_FilePreviewComponent.h ***/
#ifndef __JUCE_FILEPREVIEWCOMPONENT_JUCEHEADER__
#define __JUCE_FILEPREVIEWCOMPONENT_JUCEHEADER__

class JUCE_API  FilePreviewComponent  : public Component
{
public:

	FilePreviewComponent();

	~FilePreviewComponent();

	virtual void selectedFileChanged (const File& newSelectedFile) = 0;

	juce_UseDebuggingNewOperator

private:
	FilePreviewComponent (const FilePreviewComponent&);
	FilePreviewComponent& operator= (const FilePreviewComponent&);
};

#endif   // __JUCE_FILEPREVIEWCOMPONENT_JUCEHEADER__
/*** End of inlined file: juce_FilePreviewComponent.h ***/

class JUCE_API  FileBrowserComponent  : public Component,
										public ChangeBroadcaster,
										private FileBrowserListener,
										private TextEditorListener,
										private ButtonListener,
										private ComboBoxListener,
										private FileFilter
{
public:

	enum FileChooserFlags
	{
		openMode		= 1,	/**< specifies that the component should allow the user to
											 choose an existing file with the intention of opening it. */
		saveMode		= 2,	/**< specifies that the component should allow the user to specify
											 the name of a file that will be used to save something. */
		canSelectFiles	  = 4,	/**< specifies that the user can select files (can be used in
											 conjunction with canSelectDirectories). */
		canSelectDirectories	= 8,	/**< specifies that the user can select directories (can be used in
											 conjuction with canSelectFiles). */
		canSelectMultipleItems  = 16,   /**< specifies that the user can select multiple items. */
		useTreeView		 = 32,   /**< specifies that a tree-view should be shown instead of a file list. */
		filenameBoxIsReadOnly   = 64	/**< specifies that the user can't type directly into the filename box. */
	};

	FileBrowserComponent (int flags,
						  const File& initialFileOrDirectory,
						  const FileFilter* fileFilter,
						  FilePreviewComponent* previewComp);

	~FileBrowserComponent();

	int getNumSelectedFiles() const throw();

	const File getSelectedFile (int index) const throw();

	bool currentFileIsValid() const;

	const File getHighlightedFile() const throw();

	const File getRoot() const;

	void setRoot (const File& newRootDirectory);

	void goUp();

	void refresh();

	virtual const String getActionVerb() const;

	bool isSaveMode() const throw();

	void addListener (FileBrowserListener* listener);

	void removeListener (FileBrowserListener* listener);

	void resized();
	void buttonClicked (Button* b);
	void comboBoxChanged (ComboBox*);
	void textEditorTextChanged (TextEditor& editor);
	void textEditorReturnKeyPressed (TextEditor& editor);
	void textEditorEscapeKeyPressed (TextEditor& editor);
	void textEditorFocusLost (TextEditor& editor);
	bool keyPressed (const KeyPress& key);
	void selectionChanged();
	void fileClicked (const File& f, const MouseEvent& e);
	void fileDoubleClicked (const File& f);
	bool isFileSuitable (const File& file) const;
	bool isDirectorySuitable (const File&) const;

	FilePreviewComponent* getPreviewComponent() const throw();

	juce_UseDebuggingNewOperator

protected:
	virtual const BigInteger getRoots (StringArray& rootNames, StringArray& rootPaths);

private:

	ScopedPointer <DirectoryContentsList> fileList;
	const FileFilter* fileFilter;

	int flags;
	File currentRoot;
	Array<File> chosenFiles;
	ListenerList <FileBrowserListener> listeners;

	DirectoryContentsDisplayComponent* fileListComponent;
	FilePreviewComponent* previewComp;
	ComboBox* currentPathBox;
	TextEditor* filenameBox;
	Button* goUpButton;

	TimeSliceThread thread;

	void sendListenerChangeMessage();
	bool isFileOrDirSuitable (const File& f) const;

	FileBrowserComponent (const FileBrowserComponent&);
	FileBrowserComponent& operator= (const FileBrowserComponent&);
};

#endif   // __JUCE_FILEBROWSERCOMPONENT_JUCEHEADER__
/*** End of inlined file: juce_FileBrowserComponent.h ***/


#endif
#ifndef __JUCE_FILEBROWSERLISTENER_JUCEHEADER__

#endif
#ifndef __JUCE_FILECHOOSER_JUCEHEADER__

/*** Start of inlined file: juce_FileChooser.h ***/
#ifndef __JUCE_FILECHOOSER_JUCEHEADER__
#define __JUCE_FILECHOOSER_JUCEHEADER__

class JUCE_API  FileChooser
{
public:

	FileChooser (const String& dialogBoxTitle,
				 const File& initialFileOrDirectory = File::nonexistent,
				 const String& filePatternsAllowed = String::empty,
				 bool useOSNativeDialogBox = true);

	~FileChooser();

	bool browseForFileToOpen (FilePreviewComponent* previewComponent = 0);

	bool browseForMultipleFilesToOpen (FilePreviewComponent* previewComponent = 0);

	bool browseForFileToSave (bool warnAboutOverwritingExistingFiles);

	bool browseForDirectory();

	bool browseForMultipleFilesOrDirectories (FilePreviewComponent* previewComponent = 0);

	const File getResult() const;

	const Array<File>& getResults() const;

	juce_UseDebuggingNewOperator

private:
	String title, filters;
	File startingFile;
	Array<File> results;
	bool useNativeDialogBox;

	bool showDialog (bool selectsDirectories, bool selectsFiles, bool isSave,
					 bool warnAboutOverwritingExistingFiles, bool selectMultipleFiles,
					 FilePreviewComponent* previewComponent);

	static void showPlatformDialog (Array<File>& results, const String& title, const File& file,
									const String& filters, bool selectsDirectories, bool selectsFiles,
									bool isSave, bool warnAboutOverwritingExistingFiles, bool selectMultipleFiles,
									FilePreviewComponent* previewComponent);
};

#endif   // __JUCE_FILECHOOSER_JUCEHEADER__
/*** End of inlined file: juce_FileChooser.h ***/


#endif
#ifndef __JUCE_FILECHOOSERDIALOGBOX_JUCEHEADER__

/*** Start of inlined file: juce_FileChooserDialogBox.h ***/
#ifndef __JUCE_FILECHOOSERDIALOGBOX_JUCEHEADER__
#define __JUCE_FILECHOOSERDIALOGBOX_JUCEHEADER__


/*** Start of inlined file: juce_ResizableWindow.h ***/
#ifndef __JUCE_RESIZABLEWINDOW_JUCEHEADER__
#define __JUCE_RESIZABLEWINDOW_JUCEHEADER__


/*** Start of inlined file: juce_TopLevelWindow.h ***/
#ifndef __JUCE_TOPLEVELWINDOW_JUCEHEADER__
#define __JUCE_TOPLEVELWINDOW_JUCEHEADER__


/*** Start of inlined file: juce_DropShadower.h ***/
#ifndef __JUCE_DROPSHADOWER_JUCEHEADER__
#define __JUCE_DROPSHADOWER_JUCEHEADER__

class JUCE_API  DropShadower  : public ComponentListener
{
public:

	DropShadower (float alpha = 0.5f,
				  int xOffset = 1,
				  int yOffset = 5,
				  float blurRadius = 10.0f);

	virtual ~DropShadower();

	void setOwner (Component* componentToFollow);

	void componentMovedOrResized (Component& component, bool wasMoved, bool wasResized);
	void componentBroughtToFront (Component& component);
	void componentChildrenChanged (Component& component);
	void componentParentHierarchyChanged (Component& component);
	void componentVisibilityChanged (Component& component);

	juce_UseDebuggingNewOperator

private:

	Component* owner;
	int numShadows;
	Component* shadowWindows[4];
	Image* shadowImageSections[12];
	const int shadowEdge, xOffset, yOffset;
	const float alpha, blurRadius;
	bool inDestructor, reentrant;

	void updateShadows();
	void setShadowImage (Image* const src,
						 const int num,
						 const int w, const int h,
						 const int sx, const int sy);

	void bringShadowWindowsToFront();
	void deleteShadowWindows();

	DropShadower (const DropShadower&);
	DropShadower& operator= (const DropShadower&);
};

#endif   // __JUCE_DROPSHADOWER_JUCEHEADER__
/*** End of inlined file: juce_DropShadower.h ***/

class JUCE_API  TopLevelWindow  : public Component
{
public:

	TopLevelWindow (const String& name, bool addToDesktop);

	~TopLevelWindow();

	bool isActiveWindow() const throw()			 { return windowIsActive_; }

	void centreAroundComponent (Component* componentToCentreAround,
								int width, int height);

	void setDropShadowEnabled (bool useShadow);

	void setUsingNativeTitleBar (bool useNativeTitleBar);

	bool isUsingNativeTitleBar() const throw()		  { return useNativeTitleBar && isOnDesktop(); }

	static int getNumTopLevelWindows() throw();

	static TopLevelWindow* getTopLevelWindow (int index) throw();

	static TopLevelWindow* getActiveTopLevelWindow() throw();

	juce_UseDebuggingNewOperator

	virtual void addToDesktop (int windowStyleFlags, void* nativeWindowToAttachTo = 0);

protected:

	virtual void activeWindowStatusChanged();

	void focusOfChildComponentChanged (FocusChangeType cause);
	void parentHierarchyChanged();
	void visibilityChanged();
	virtual int getDesktopWindowStyleFlags() const;
	void recreateDesktopWindow();

private:
	friend class TopLevelWindowManager;
	bool useDropShadow, useNativeTitleBar, windowIsActive_;
	ScopedPointer <DropShadower> shadower;

	void setWindowActive (bool isNowActive) throw();

	TopLevelWindow (const TopLevelWindow&);
	TopLevelWindow& operator= (const TopLevelWindow&);
};

#endif   // __JUCE_TOPLEVELWINDOW_JUCEHEADER__
/*** End of inlined file: juce_TopLevelWindow.h ***/


/*** Start of inlined file: juce_ComponentDragger.h ***/
#ifndef __JUCE_COMPONENTDRAGGER_JUCEHEADER__
#define __JUCE_COMPONENTDRAGGER_JUCEHEADER__


/*** Start of inlined file: juce_ComponentBoundsConstrainer.h ***/
#ifndef __JUCE_COMPONENTBOUNDSCONSTRAINER_JUCEHEADER__
#define __JUCE_COMPONENTBOUNDSCONSTRAINER_JUCEHEADER__

class JUCE_API  ComponentBoundsConstrainer
{
public:

	ComponentBoundsConstrainer() throw();

	virtual ~ComponentBoundsConstrainer();

	void setMinimumWidth (int minimumWidth) throw();

	int getMinimumWidth() const throw()			 { return minW; }

	void setMaximumWidth (int maximumWidth) throw();

	int getMaximumWidth() const throw()			 { return maxW; }

	void setMinimumHeight (int minimumHeight) throw();

	int getMinimumHeight() const throw()			{ return minH; }

	void setMaximumHeight (int maximumHeight) throw();

	int getMaximumHeight() const throw()			{ return maxH; }

	void setMinimumSize (int minimumWidth,
						 int minimumHeight) throw();

	void setMaximumSize (int maximumWidth,
						 int maximumHeight) throw();

	void setSizeLimits (int minimumWidth,
						int minimumHeight,
						int maximumWidth,
						int maximumHeight) throw();

	void setMinimumOnscreenAmounts (int minimumWhenOffTheTop,
									int minimumWhenOffTheLeft,
									int minimumWhenOffTheBottom,
									int minimumWhenOffTheRight) throw();

	void setFixedAspectRatio (double widthOverHeight) throw();

	double getFixedAspectRatio() const throw();

	virtual void checkBounds (Rectangle<int>& bounds,
							  const Rectangle<int>& previousBounds,
							  const Rectangle<int>& limits,
							  bool isStretchingTop,
							  bool isStretchingLeft,
							  bool isStretchingBottom,
							  bool isStretchingRight);

	virtual void resizeStart();

	virtual void resizeEnd();

	void setBoundsForComponent (Component* const component,
								const Rectangle<int>& bounds,
								bool isStretchingTop,
								bool isStretchingLeft,
								bool isStretchingBottom,
								bool isStretchingRight);

	void checkComponentBounds (Component* component);

	virtual void applyBoundsToComponent (Component* component,
										 const Rectangle<int>& bounds);

	juce_UseDebuggingNewOperator

private:
	int minW, maxW, minH, maxH;
	int minOffTop, minOffLeft, minOffBottom, minOffRight;
	double aspectRatio;

	ComponentBoundsConstrainer (const ComponentBoundsConstrainer&);
	ComponentBoundsConstrainer& operator= (const ComponentBoundsConstrainer&);
};

#endif   // __JUCE_COMPONENTBOUNDSCONSTRAINER_JUCEHEADER__
/*** End of inlined file: juce_ComponentBoundsConstrainer.h ***/

class JUCE_API  ComponentDragger
{
public:

	ComponentDragger();

	virtual ~ComponentDragger();

	void startDraggingComponent (Component* const componentToDrag,
								 ComponentBoundsConstrainer* constrainer);

	void dragComponent (Component* const componentToDrag,
						const MouseEvent& e);

	juce_UseDebuggingNewOperator

private:
	ComponentBoundsConstrainer* constrainer;
	Point<int> originalPos;

	ComponentDragger (const ComponentDragger&);
	ComponentDragger& operator= (const ComponentDragger&);
};

#endif   // __JUCE_COMPONENTDRAGGER_JUCEHEADER__
/*** End of inlined file: juce_ComponentDragger.h ***/


/*** Start of inlined file: juce_ResizableBorderComponent.h ***/
#ifndef __JUCE_RESIZABLEBORDERCOMPONENT_JUCEHEADER__
#define __JUCE_RESIZABLEBORDERCOMPONENT_JUCEHEADER__

class JUCE_API  ResizableBorderComponent  : public Component
{
public:

	ResizableBorderComponent (Component* componentToResize,
							  ComponentBoundsConstrainer* constrainer);

	~ResizableBorderComponent();

	void setBorderThickness (const BorderSize& newBorderSize);

	const BorderSize getBorderThickness() const;

	class Zone
	{
	public:

		enum Zones
		{
			centre  = 0,
			left	= 1,
			top	 = 2,
			right   = 4,
			bottom  = 8
		};

		explicit Zone (int zoneFlags = 0) throw();
		Zone (const Zone& other) throw();
		Zone& operator= (const Zone& other) throw();

		bool operator== (const Zone& other) const throw();
		bool operator!= (const Zone& other) const throw();

		static const Zone fromPositionOnBorder (const Rectangle<int>& totalSize,
												const BorderSize& border,
												const Point<int>& position);

		const MouseCursor getMouseCursor() const throw();

		bool isDraggingWholeObject() const throw()	  { return zone == centre; }
		bool isDraggingLeftEdge() const throw()	 { return (zone & left) != 0; }
		bool isDraggingRightEdge() const throw()	{ return (zone & right) != 0; }
		bool isDraggingTopEdge() const throw()	  { return (zone & top) != 0; }
		bool isDraggingBottomEdge() const throw()	   { return (zone & bottom) != 0; }

		const Rectangle<int> resizeRectangleBy (Rectangle<int> original,
												const Point<int>& distance) const throw();

	private:

		int zone;
	};

	juce_UseDebuggingNewOperator

protected:
	void paint (Graphics& g);
	void mouseEnter (const MouseEvent& e);
	void mouseMove (const MouseEvent& e);
	void mouseDown (const MouseEvent& e);
	void mouseDrag (const MouseEvent& e);
	void mouseUp (const MouseEvent& e);
	bool hitTest (int x, int y);

private:
	Component::SafePointer<Component> component;
	ComponentBoundsConstrainer* constrainer;
	BorderSize borderSize;
	Rectangle<int> originalBounds;
	Zone mouseZone;

	void updateMouseZone (const MouseEvent& e);

	ResizableBorderComponent (const ResizableBorderComponent&);
	ResizableBorderComponent& operator= (const ResizableBorderComponent&);
};

#endif   // __JUCE_RESIZABLEBORDERCOMPONENT_JUCEHEADER__
/*** End of inlined file: juce_ResizableBorderComponent.h ***/


/*** Start of inlined file: juce_ResizableCornerComponent.h ***/
#ifndef __JUCE_RESIZABLECORNERCOMPONENT_JUCEHEADER__
#define __JUCE_RESIZABLECORNERCOMPONENT_JUCEHEADER__

class JUCE_API  ResizableCornerComponent  : public Component
{
public:

	ResizableCornerComponent (Component* componentToResize,
							  ComponentBoundsConstrainer* constrainer);

	~ResizableCornerComponent();

	juce_UseDebuggingNewOperator

protected:
	void paint (Graphics& g);
	void mouseDown (const MouseEvent& e);
	void mouseDrag (const MouseEvent& e);
	void mouseUp (const MouseEvent& e);
	bool hitTest (int x, int y);

private:

	Component::SafePointer<Component> component;
	ComponentBoundsConstrainer* constrainer;
	Rectangle<int> originalBounds;

	ResizableCornerComponent (const ResizableCornerComponent&);
	ResizableCornerComponent& operator= (const ResizableCornerComponent&);
};

#endif   // __JUCE_RESIZABLECORNERCOMPONENT_JUCEHEADER__
/*** End of inlined file: juce_ResizableCornerComponent.h ***/

class JUCE_API  ResizableWindow  : public TopLevelWindow
{
public:

	ResizableWindow (const String& name,
					 bool addToDesktop);

	ResizableWindow (const String& name,
					 const Colour& backgroundColour,
					 bool addToDesktop);

	~ResizableWindow();

	const Colour getBackgroundColour() const throw();

	void setBackgroundColour (const Colour& newColour);

	void setResizable (bool shouldBeResizable,
					   bool useBottomRightCornerResizer);

	bool isResizable() const throw();

	void setResizeLimits (int newMinimumWidth,
						  int newMinimumHeight,
						  int newMaximumWidth,
						  int newMaximumHeight) throw();

	ComponentBoundsConstrainer* getConstrainer() throw()		{ return constrainer; }

	void setConstrainer (ComponentBoundsConstrainer* newConstrainer);

	void setBoundsConstrained (const Rectangle<int>& bounds);

	bool isFullScreen() const;

	void setFullScreen (bool shouldBeFullScreen);

	bool isMinimised() const;

	void setMinimised (bool shouldMinimise);

	const String getWindowStateAsString();

	bool restoreWindowStateFromString (const String& previousState);

	Component* getContentComponent() const throw()		  { return contentComponent; }

	void setContentComponent (Component* newContentComponent,
							  bool deleteOldOne = true,
							  bool resizeToFit = false);

	void setContentComponentSize (int width, int height);

	enum ColourIds
	{
		backgroundColourId	  = 0x1005700,  /**< A colour to use to fill the window's background. */
	};

	juce_UseDebuggingNewOperator

protected:
	void paint (Graphics& g);
	void moved();
	void resized();
	void mouseDown (const MouseEvent& e);
	void mouseDrag (const MouseEvent& e);
	void lookAndFeelChanged();
	void childBoundsChanged (Component* child);
	void parentSizeChanged();
	void visibilityChanged();
	void activeWindowStatusChanged();
	int getDesktopWindowStyleFlags() const;

	virtual const BorderSize getBorderThickness();

	virtual const BorderSize getContentComponentBorder();

#ifdef JUCE_DEBUG
	void addChildComponent (Component* child, int zOrder = -1);
	void addAndMakeVisible (Component* child, int zOrder = -1);

#endif

	ScopedPointer <ResizableCornerComponent> resizableCorner;
	ScopedPointer <ResizableBorderComponent> resizableBorder;

private:
	ScopedPointer <Component> contentComponent;
	bool resizeToFitContent, fullscreen;
	ComponentDragger dragger;
	Rectangle<int> lastNonFullScreenPos;
	ComponentBoundsConstrainer defaultConstrainer;
	ComponentBoundsConstrainer* constrainer;
	#ifdef JUCE_DEBUG
	bool hasBeenResized;
	#endif

	void updateLastPos();

	ResizableWindow (const ResizableWindow&);
	ResizableWindow& operator= (const ResizableWindow&);

	// (xxx remove these eventually)
	// temporarily here to stop old code compiling, as the parameters for these methods have changed..
	void getBorderThickness (int& left, int& top, int& right, int& bottom);
	// temporarily here to stop old code compiling, as the parameters for these methods have changed..
	void getContentComponentBorder (int& left, int& top, int& right, int& bottom);
};

#endif   // __JUCE_RESIZABLEWINDOW_JUCEHEADER__
/*** End of inlined file: juce_ResizableWindow.h ***/


/*** Start of inlined file: juce_GlyphArrangement.h ***/
#ifndef __JUCE_GLYPHARRANGEMENT_JUCEHEADER__
#define __JUCE_GLYPHARRANGEMENT_JUCEHEADER__

class JUCE_API  PositionedGlyph
{
public:

	juce_wchar getCharacter() const		 { return character; }
	bool isWhitespace() const		   { return CharacterFunctions::isWhitespace (character); }

	float getLeft() const			   { return x; }
	float getRight() const			  { return x + w; }
	float getBaselineY() const		  { return y; }
	float getTop() const			{ return y - font.getAscent(); }
	float getBottom() const			 { return y + font.getDescent(); }
	const Rectangle<float> getBounds() const	{ return Rectangle<float> (x, getTop(), w, font.getHeight()); }

	void moveBy (float deltaX, float deltaY);

	void draw (const Graphics& g) const;

	void draw (const Graphics& g, const AffineTransform& transform) const;

	void createPath (Path& path) const;

	bool hitTest (float x, float y) const;

	juce_UseDebuggingNewOperator

private:

	friend class GlyphArrangement;
	float x, y, w;
	Font font;
	juce_wchar character;
	int glyph;

	PositionedGlyph (float x, float y, float w, const Font& font, juce_wchar character, int glyph);
	PositionedGlyph (const PositionedGlyph& other);
};

class JUCE_API  GlyphArrangement
{
public:

	GlyphArrangement();

	GlyphArrangement (const GlyphArrangement& other);

	GlyphArrangement& operator= (const GlyphArrangement& other);

	~GlyphArrangement();

	int getNumGlyphs() const throw()				{ return glyphs.size(); }

	PositionedGlyph& getGlyph (int index) const;

	void clear();

	void addLineOfText (const Font& font,
						const String& text,
						float x, float y);

	void addCurtailedLineOfText (const Font& font,
								 const String& text,
								 float x, float y,
								 float maxWidthPixels,
								 bool useEllipsis);

	void addJustifiedText (const Font& font,
						   const String& text,
						   float x, float y,
						   float maxLineWidth,
						   const Justification& horizontalLayout);

	void addFittedText (const Font& font,
						const String& text,
						float x, float y, float width, float height,
						const Justification& layout,
						int maximumLinesToUse,
						float minimumHorizontalScale = 0.7f);

	void addGlyphArrangement (const GlyphArrangement& other);

	void draw (const Graphics& g) const;

	void draw (const Graphics& g, const AffineTransform& transform) const;

	void createPath (Path& path) const;

	int findGlyphIndexAt (float x, float y) const;

	const Rectangle<float> getBoundingBox (int startIndex, int numGlyphs, bool includeWhitespace) const;

	void moveRangeOfGlyphs (int startIndex, int numGlyphs,
							float deltaX, float deltaY);

	void removeRangeOfGlyphs (int startIndex, int numGlyphs);

	void stretchRangeOfGlyphs (int startIndex, int numGlyphs,
							   float horizontalScaleFactor);

	void justifyGlyphs (int startIndex, int numGlyphs,
						float x, float y, float width, float height,
						const Justification& justification);

	juce_UseDebuggingNewOperator

private:
	OwnedArray <PositionedGlyph> glyphs;

	int insertEllipsis (const Font& font, float maxXPos, int startIndex, int endIndex);
	int fitLineIntoSpace (int start, int numGlyphs, float x, float y, float w, float h, const Font& font,
						  const Justification& justification, float minimumHorizontalScale);
	void spreadOutLine (int start, int numGlyphs, float targetWidth);
};

#endif   // __JUCE_GLYPHARRANGEMENT_JUCEHEADER__
/*** End of inlined file: juce_GlyphArrangement.h ***/

class JUCE_API  FileChooserDialogBox : public ResizableWindow,
									   public ButtonListener,
									   public FileBrowserListener
{
public:

	FileChooserDialogBox (const String& title,
						  const String& instructions,
						  FileBrowserComponent& browserComponent,
						  bool warnAboutOverwritingExistingFiles,
						  const Colour& backgroundColour);

	~FileChooserDialogBox();

	bool show (int width = 0,int height = 0);

	enum ColourIds
	{
		titleTextColourId	  = 0x1000850, /**< The colour to use to draw the box's title. */
	};

	void buttonClicked (Button* button);
	void closeButtonPressed();
	void selectionChanged();
	void fileClicked (const File& file, const MouseEvent& e);
	void fileDoubleClicked (const File& file);

	juce_UseDebuggingNewOperator

private:
	class ContentComponent  : public Component
	{
	public:
		ContentComponent();
		~ContentComponent();

		void paint (Graphics& g);
		void resized();

		String instructions;
		GlyphArrangement text;

		FileBrowserComponent* chooserComponent;
		FilePreviewComponent* previewComponent;
		TextButton* okButton;
		TextButton* cancelButton;
	};

	ContentComponent* content;
	const bool warnAboutOverwritingExistingFiles;

	FileChooserDialogBox (const FileChooserDialogBox&);
	FileChooserDialogBox& operator= (const FileChooserDialogBox&);
};

#endif   // __JUCE_FILECHOOSERDIALOGBOX_JUCEHEADER__
/*** End of inlined file: juce_FileChooserDialogBox.h ***/


#endif
#ifndef __JUCE_FILEFILTER_JUCEHEADER__

#endif
#ifndef __JUCE_FILELISTCOMPONENT_JUCEHEADER__

/*** Start of inlined file: juce_FileListComponent.h ***/
#ifndef __JUCE_FILELISTCOMPONENT_JUCEHEADER__
#define __JUCE_FILELISTCOMPONENT_JUCEHEADER__

class JUCE_API  FileListComponent  : public ListBox,
									 public DirectoryContentsDisplayComponent,
									 private ListBoxModel,
									 private ChangeListener
{
public:

	FileListComponent (DirectoryContentsList& listToShow);

	~FileListComponent();

	int getNumSelectedFiles() const;

	const File getSelectedFile (int index = 0) const;

	void scrollToTop();

	void changeListenerCallback (void*);
	int getNumRows();
	void paintListBoxItem (int, Graphics&, int, int, bool);
	Component* refreshComponentForRow (int rowNumber, bool isRowSelected, Component* existingComponentToUpdate);
	void selectedRowsChanged (int lastRowSelected);
	void deleteKeyPressed (int currentSelectedRow);
	void returnKeyPressed (int currentSelectedRow);

	juce_UseDebuggingNewOperator

private:
	FileListComponent (const FileListComponent&);
	FileListComponent& operator= (const FileListComponent&);

	File lastDirectory;
};

#endif   // __JUCE_FILELISTCOMPONENT_JUCEHEADER__
/*** End of inlined file: juce_FileListComponent.h ***/


#endif
#ifndef __JUCE_FILENAMECOMPONENT_JUCEHEADER__

/*** Start of inlined file: juce_FilenameComponent.h ***/
#ifndef __JUCE_FILENAMECOMPONENT_JUCEHEADER__
#define __JUCE_FILENAMECOMPONENT_JUCEHEADER__

class FilenameComponent;

class JUCE_API  FilenameComponentListener
{
public:
	virtual ~FilenameComponentListener() {}

	virtual void filenameComponentChanged (FilenameComponent* fileComponentThatHasChanged) = 0;
};

class JUCE_API  FilenameComponent  : public Component,
									 public SettableTooltipClient,
									 public FileDragAndDropTarget,
									 private AsyncUpdater,
									 private ButtonListener,
									 private ComboBoxListener
{
public:

	FilenameComponent (const String& name,
					   const File& currentFile,
					   bool canEditFilename,
					   bool isDirectory,
					   bool isForSaving,
					   const String& fileBrowserWildcard,
					   const String& enforcedSuffix,
					   const String& textWhenNothingSelected);

	~FilenameComponent();

	const File getCurrentFile() const;

	void setCurrentFile (File newFile,
						 bool addToRecentlyUsedList,
						 bool sendChangeNotification = true);

	void setFilenameIsEditable (bool shouldBeEditable);

	void setDefaultBrowseTarget (const File& newDefaultDirectory);

	const StringArray getRecentlyUsedFilenames() const;

	void setRecentlyUsedFilenames (const StringArray& filenames);

	void addRecentlyUsedFile (const File& file);

	void setMaxNumberOfRecentFiles (int newMaximum);

	void setBrowseButtonText (const String& browseButtonText);

	void addListener (FilenameComponentListener* listener);

	void removeListener (FilenameComponentListener* listener);

	void setTooltip (const String& newTooltip);

	void paintOverChildren (Graphics& g);
	void resized();
	void lookAndFeelChanged();
	bool isInterestedInFileDrag (const StringArray& files);
	void filesDropped (const StringArray& files, int, int);
	void fileDragEnter (const StringArray& files, int, int);
	void fileDragExit (const StringArray& files);

	juce_UseDebuggingNewOperator

private:

	ComboBox* filenameBox;
	String lastFilename;
	Button* browseButton;
	int maxRecentFiles;
	bool isDir, isSaving, isFileDragOver;
	String wildcard, enforcedSuffix, browseButtonText;
	ListenerList <FilenameComponentListener> listeners;
	File defaultBrowseFile;

	void comboBoxChanged (ComboBox*);
	void buttonClicked (Button* button);
	void handleAsyncUpdate();

	FilenameComponent (const FilenameComponent&);
	FilenameComponent& operator= (const FilenameComponent&);
};

#endif   // __JUCE_FILENAMECOMPONENT_JUCEHEADER__
/*** End of inlined file: juce_FilenameComponent.h ***/


#endif
#ifndef __JUCE_FILEPREVIEWCOMPONENT_JUCEHEADER__

#endif
#ifndef __JUCE_FILESEARCHPATHLISTCOMPONENT_JUCEHEADER__

/*** Start of inlined file: juce_FileSearchPathListComponent.h ***/
#ifndef __JUCE_FILESEARCHPATHLISTCOMPONENT_JUCEHEADER__
#define __JUCE_FILESEARCHPATHLISTCOMPONENT_JUCEHEADER__

class JUCE_API  FileSearchPathListComponent  : public Component,
											   public SettableTooltipClient,
											   public FileDragAndDropTarget,
											   private ButtonListener,
											   private ListBoxModel
{
public:

	FileSearchPathListComponent();

	~FileSearchPathListComponent();

	const FileSearchPath& getPath() const throw()		   { return path; }

	void setPath (const FileSearchPath& newPath);

	void setDefaultBrowseTarget (const File& newDefaultDirectory);

	enum ColourIds
	{
		backgroundColourId	  = 0x1004100, /**< The background colour to fill the component with.
												  Make this transparent if you don't want the background to be filled. */
	};

	int getNumRows();
	void paintListBoxItem (int rowNumber, Graphics& g, int width, int height, bool rowIsSelected);
	void deleteKeyPressed (int lastRowSelected);
	void returnKeyPressed (int lastRowSelected);
	void listBoxItemDoubleClicked (int row, const MouseEvent&);
	void selectedRowsChanged (int lastRowSelected);
	void resized();
	void paint (Graphics& g);
	bool isInterestedInFileDrag (const StringArray& files);
	void filesDropped (const StringArray& files, int, int);
	void buttonClicked (Button* button);

	juce_UseDebuggingNewOperator

private:

	FileSearchPath path;
	File defaultBrowseTarget;

	ListBox* listBox;
	Button* addButton;
	Button* removeButton;
	Button* changeButton;
	Button* upButton;
	Button* downButton;

	void changed();
	void updateButtons();

	FileSearchPathListComponent (const FileSearchPathListComponent&);
	FileSearchPathListComponent& operator= (const FileSearchPathListComponent&);
};

#endif   // __JUCE_FILESEARCHPATHLISTCOMPONENT_JUCEHEADER__
/*** End of inlined file: juce_FileSearchPathListComponent.h ***/


#endif
#ifndef __JUCE_FILETREECOMPONENT_JUCEHEADER__

/*** Start of inlined file: juce_FileTreeComponent.h ***/
#ifndef __JUCE_FILETREECOMPONENT_JUCEHEADER__
#define __JUCE_FILETREECOMPONENT_JUCEHEADER__

class JUCE_API  FileTreeComponent  : public TreeView,
									 public DirectoryContentsDisplayComponent
{
public:

	FileTreeComponent (DirectoryContentsList& listToShow);

	~FileTreeComponent();

	int getNumSelectedFiles() const		 { return TreeView::getNumSelectedItems(); }

	const File getSelectedFile (int index = 0) const;

	void scrollToTop();

	void setDragAndDropDescription (const String& description);

	const String& getDragAndDropDescription() const throw()	  { return dragAndDropDescription; }

	juce_UseDebuggingNewOperator

private:
	String dragAndDropDescription;

	FileTreeComponent (const FileTreeComponent&);
	FileTreeComponent& operator= (const FileTreeComponent&);
};

#endif   // __JUCE_FILETREECOMPONENT_JUCEHEADER__
/*** End of inlined file: juce_FileTreeComponent.h ***/


#endif
#ifndef __JUCE_IMAGEPREVIEWCOMPONENT_JUCEHEADER__

/*** Start of inlined file: juce_ImagePreviewComponent.h ***/
#ifndef __JUCE_IMAGEPREVIEWCOMPONENT_JUCEHEADER__
#define __JUCE_IMAGEPREVIEWCOMPONENT_JUCEHEADER__

class JUCE_API  ImagePreviewComponent  : public FilePreviewComponent,
										 private Timer
{
public:

	ImagePreviewComponent();

	~ImagePreviewComponent();

	void selectedFileChanged (const File& newSelectedFile);
	void paint (Graphics& g);
	void timerCallback();

	juce_UseDebuggingNewOperator

private:
	File fileToLoad;
	ScopedPointer <Image> currentThumbnail;
	String currentDetails;

	void getThumbSize (int& w, int& h) const;

	ImagePreviewComponent (const ImagePreviewComponent&);
	ImagePreviewComponent& operator= (const ImagePreviewComponent&);
};

#endif   // __JUCE_IMAGEPREVIEWCOMPONENT_JUCEHEADER__
/*** End of inlined file: juce_ImagePreviewComponent.h ***/


#endif
#ifndef __JUCE_WILDCARDFILEFILTER_JUCEHEADER__

/*** Start of inlined file: juce_WildcardFileFilter.h ***/
#ifndef __JUCE_WILDCARDFILEFILTER_JUCEHEADER__
#define __JUCE_WILDCARDFILEFILTER_JUCEHEADER__

class JUCE_API  WildcardFileFilter  : public FileFilter
{
public:

	WildcardFileFilter (const String& fileWildcardPatterns,
						const String& directoryWildcardPatterns,
						const String& description);

	~WildcardFileFilter();

	bool isFileSuitable (const File& file) const;

	bool isDirectorySuitable (const File& file) const;

	juce_UseDebuggingNewOperator

private:
	StringArray fileWildcards, directoryWildcards;

	static void parse (const String& pattern, StringArray& result);
	static bool match (const File& file, const StringArray& wildcards);
};

#endif   // __JUCE_WILDCARDFILEFILTER_JUCEHEADER__
/*** End of inlined file: juce_WildcardFileFilter.h ***/


#endif
#ifndef __JUCE_COMPONENT_JUCEHEADER__

#endif
#ifndef __JUCE_COMPONENTLISTENER_JUCEHEADER__

#endif
#ifndef __JUCE_DESKTOP_JUCEHEADER__

#endif
#ifndef __JUCE_KEYBOARDFOCUSTRAVERSER_JUCEHEADER__

#endif
#ifndef __JUCE_KEYLISTENER_JUCEHEADER__

#endif
#ifndef __JUCE_KEYMAPPINGEDITORCOMPONENT_JUCEHEADER__

/*** Start of inlined file: juce_KeyMappingEditorComponent.h ***/
#ifndef __JUCE_KEYMAPPINGEDITORCOMPONENT_JUCEHEADER__
#define __JUCE_KEYMAPPINGEDITORCOMPONENT_JUCEHEADER__


/*** Start of inlined file: juce_KeyPressMappingSet.h ***/
#ifndef __JUCE_KEYPRESSMAPPINGSET_JUCEHEADER__
#define __JUCE_KEYPRESSMAPPINGSET_JUCEHEADER__

class JUCE_API  KeyPressMappingSet  : public KeyListener,
									  public ChangeBroadcaster,
									  public FocusChangeListener
{
public:

	explicit KeyPressMappingSet (ApplicationCommandManager* commandManager);

	KeyPressMappingSet (const KeyPressMappingSet& other);

	~KeyPressMappingSet();

	ApplicationCommandManager* getCommandManager() const throw()	{ return commandManager; }

	const Array <KeyPress> getKeyPressesAssignedToCommand (CommandID commandID) const;

	void addKeyPress (CommandID commandID,
					  const KeyPress& newKeyPress,
					  int insertIndex = -1);

	void resetToDefaultMappings();

	void resetToDefaultMapping (CommandID commandID);

	void clearAllKeyPresses();

	void clearAllKeyPresses (CommandID commandID);

	void removeKeyPress (CommandID commandID, int keyPressIndex);

	void removeKeyPress (const KeyPress& keypress);

	bool containsMapping (CommandID commandID, const KeyPress& keyPress) const throw();

	CommandID findCommandForKeyPress (const KeyPress& keyPress) const throw();

	bool restoreFromXml (const XmlElement& xmlVersion);

	XmlElement* createXml (bool saveDifferencesFromDefaultSet) const;

	bool keyPressed (const KeyPress& key, Component* originatingComponent);
	bool keyStateChanged (bool isKeyDown, Component* originatingComponent);
	void globalFocusChanged (Component* focusedComponent);

	juce_UseDebuggingNewOperator

private:

	ApplicationCommandManager* commandManager;

	struct CommandMapping
	{
		CommandID commandID;
		Array <KeyPress> keypresses;
		bool wantsKeyUpDownCallbacks;
	};

	OwnedArray <CommandMapping> mappings;

	struct KeyPressTime
	{
		KeyPress key;
		uint32 timeWhenPressed;
	};

	OwnedArray <KeyPressTime> keysDown;

	void handleMessage (const Message& message);

	void invokeCommand (const CommandID commandID,
						const KeyPress& keyPress,
						const bool isKeyDown,
						const int millisecsSinceKeyPressed,
						Component* const originatingComponent) const;

	KeyPressMappingSet& operator= (const KeyPressMappingSet&);
};

#endif   // __JUCE_KEYPRESSMAPPINGSET_JUCEHEADER__
/*** End of inlined file: juce_KeyPressMappingSet.h ***/

class JUCE_API  KeyMappingEditorComponent  : public Component,
											 public TreeViewItem,
											 public ChangeListener,
											 private ButtonListener
{
public:

	KeyMappingEditorComponent (KeyPressMappingSet* mappingSet,
							   bool showResetToDefaultButton);

	virtual ~KeyMappingEditorComponent();

	void setColours (const Colour& mainBackground,
					 const Colour& textColour);

	KeyPressMappingSet* getMappings() const throw()		 { return mappings; }

	virtual bool shouldCommandBeIncluded (CommandID commandID);

	virtual bool isCommandReadOnly (CommandID commandID);

	virtual const String getDescriptionForKeyPress (const KeyPress& key);

	enum ColourIds
	{
		backgroundColourId  = 0x100ad00,	/**< The background colour to fill the editor background. */
		textColourId	= 0x100ad01,	/**< The colour for the text. */
	};

	void parentHierarchyChanged();
	void resized();
	void changeListenerCallback (void*);
	bool mightContainSubItems();
	const String getUniqueName() const;
	void buttonClicked (Button* button);

	juce_UseDebuggingNewOperator

private:

	KeyPressMappingSet* mappings;
	TreeView* tree;
	friend class KeyMappingTreeViewItem;
	friend class KeyCategoryTreeViewItem;
	friend class KeyMappingItemComponent;
	friend class KeyMappingChangeButton;
	TextButton* resetButton;

	void assignNewKey (CommandID commandID, int index);

	KeyMappingEditorComponent (const KeyMappingEditorComponent&);
	KeyMappingEditorComponent& operator= (const KeyMappingEditorComponent&);
};

#endif   // __JUCE_KEYMAPPINGEDITORCOMPONENT_JUCEHEADER__
/*** End of inlined file: juce_KeyMappingEditorComponent.h ***/


#endif
#ifndef __JUCE_KEYPRESS_JUCEHEADER__

#endif
#ifndef __JUCE_KEYPRESSMAPPINGSET_JUCEHEADER__

#endif
#ifndef __JUCE_MODIFIERKEYS_JUCEHEADER__

#endif
#ifndef __JUCE_TEXTINPUTTARGET_JUCEHEADER__

#endif
#ifndef __JUCE_COMPONENTANIMATOR_JUCEHEADER__

#endif
#ifndef __JUCE_COMPONENTBOUNDSCONSTRAINER_JUCEHEADER__

#endif
#ifndef __JUCE_COMPONENTMOVEMENTWATCHER_JUCEHEADER__

/*** Start of inlined file: juce_ComponentMovementWatcher.h ***/
#ifndef __JUCE_COMPONENTMOVEMENTWATCHER_JUCEHEADER__
#define __JUCE_COMPONENTMOVEMENTWATCHER_JUCEHEADER__

class JUCE_API  ComponentMovementWatcher	: public ComponentListener
{
public:

	ComponentMovementWatcher (Component* component);

	~ComponentMovementWatcher();

	virtual void componentMovedOrResized (bool wasMoved, bool wasResized) = 0;

	virtual void componentPeerChanged() = 0;

	juce_UseDebuggingNewOperator

	void componentParentHierarchyChanged (Component& component);
	void componentMovedOrResized (Component& component, bool wasMoved, bool wasResized);

private:

	Component::SafePointer<Component> component;
	ComponentPeer* lastPeer;
	VoidArray registeredParentComps;
	bool reentrant;
	Rectangle<int> lastBounds;

	void unregister() throw();
	void registerWithParentComps() throw();

	ComponentMovementWatcher (const ComponentMovementWatcher&);
	ComponentMovementWatcher& operator= (const ComponentMovementWatcher&);
};

#endif   // __JUCE_COMPONENTMOVEMENTWATCHER_JUCEHEADER__
/*** End of inlined file: juce_ComponentMovementWatcher.h ***/


#endif
#ifndef __JUCE_GROUPCOMPONENT_JUCEHEADER__

/*** Start of inlined file: juce_GroupComponent.h ***/
#ifndef __JUCE_GROUPCOMPONENT_JUCEHEADER__
#define __JUCE_GROUPCOMPONENT_JUCEHEADER__

class JUCE_API  GroupComponent	: public Component
{
public:

	GroupComponent (const String& componentName,
					const String& labelText);

	~GroupComponent();

	void setText (const String& newText);

	const String getText() const;

	void setTextLabelPosition (const Justification& justification);

	const Justification getTextLabelPosition() const throw()		{ return justification; }

	enum ColourIds
	{
		outlineColourId	 = 0x1005400,	/**< The colour to use for drawing the line around the edge. */
		textColourId	= 0x1005410	 /**< The colour to use to draw the text label. */
	};

	void paint (Graphics& g);
	void enablementChanged();
	void colourChanged();

private:
	String text;
	Justification justification;

	GroupComponent (const GroupComponent&);
	GroupComponent& operator= (const GroupComponent&);
};

#endif   // __JUCE_GROUPCOMPONENT_JUCEHEADER__
/*** End of inlined file: juce_GroupComponent.h ***/


#endif
#ifndef __JUCE_MULTIDOCUMENTPANEL_JUCEHEADER__

/*** Start of inlined file: juce_MultiDocumentPanel.h ***/
#ifndef __JUCE_MULTIDOCUMENTPANEL_JUCEHEADER__
#define __JUCE_MULTIDOCUMENTPANEL_JUCEHEADER__


/*** Start of inlined file: juce_TabbedComponent.h ***/
#ifndef __JUCE_TABBEDCOMPONENT_JUCEHEADER__
#define __JUCE_TABBEDCOMPONENT_JUCEHEADER__


/*** Start of inlined file: juce_TabbedButtonBar.h ***/
#ifndef __JUCE_TABBEDBUTTONBAR_JUCEHEADER__
#define __JUCE_TABBEDBUTTONBAR_JUCEHEADER__

class TabbedButtonBar;

class JUCE_API  TabBarButton  : public Button
{
public:

	TabBarButton (const String& name,
				  TabbedButtonBar* ownerBar,
				  int tabIndex);

	~TabBarButton();

	virtual int getBestTabLength (int depth);

	void paintButton (Graphics& g, bool isMouseOverButton, bool isButtonDown);
	void clicked (const ModifierKeys& mods);
	bool hitTest (int x, int y);

	juce_UseDebuggingNewOperator

protected:
	friend class TabbedButtonBar;
	TabbedButtonBar* const owner;
	int tabIndex, overlapPixels;
	DropShadowEffect shadow;

	void getActiveArea (int& x, int& y, int& w, int& h);

private:
	TabBarButton (const TabBarButton&);
	TabBarButton& operator= (const TabBarButton&);
};

class JUCE_API  TabbedButtonBar  : public Component,
								   public ChangeBroadcaster,
								   public ButtonListener
{
public:

	enum Orientation
	{
		TabsAtTop,
		TabsAtBottom,
		TabsAtLeft,
		TabsAtRight
	};

	TabbedButtonBar (Orientation orientation);

	~TabbedButtonBar();

	void setOrientation (Orientation orientation);

	Orientation getOrientation() const throw()			  { return orientation; }

	void clearTabs();

	void addTab (const String& tabName,
				 const Colour& tabBackgroundColour,
				 int insertIndex = -1);

	void setTabName (int tabIndex,
					 const String& newName);

	void removeTab (int tabIndex);

	void moveTab (int currentIndex, int newIndex);

	int getNumTabs() const;

	const StringArray getTabNames() const;

	void setCurrentTabIndex (int newTabIndex, bool sendChangeMessage = true);

	const String& getCurrentTabName() const throw()			 { return tabs [currentTabIndex]; }

	int getCurrentTabIndex() const throw()				  { return currentTabIndex; }

	TabBarButton* getTabButton (int index) const;

	virtual void currentTabChanged (int newCurrentTabIndex,
									const String& newCurrentTabName);

	virtual void popupMenuClickOnTab (int tabIndex, const String& tabName);

	const Colour getTabBackgroundColour (int tabIndex);

	void setTabBackgroundColour (int tabIndex, const Colour& newColour);

	enum ColourIds
	{
		tabOutlineColourId		  = 0x1005812,	/**< The colour to use to draw an outline around the tabs.  */
		tabTextColourId		 = 0x1005813,	/**< The colour to use to draw the tab names. If this isn't specified,
															 the look and feel will choose an appropriate colour. */
		frontOutlineColourId		= 0x1005814,	/**< The colour to use to draw an outline around the currently-selected tab.  */
		frontTextColourId		   = 0x1005815,	/**< The colour to use to draw the currently-selected tab name. If
															 this isn't specified, the look and feel will choose an appropriate
															 colour. */
	};

	void resized();
	void buttonClicked (Button* button);
	void lookAndFeelChanged();

	juce_UseDebuggingNewOperator

protected:

	virtual TabBarButton* createTabButton (const String& tabName, int tabIndex);

private:
	Orientation orientation;

	StringArray tabs;
	Array <Colour> tabColours;
	int currentTabIndex;
	Component* behindFrontTab;
	Button* extraTabsButton;

	TabbedButtonBar (const TabbedButtonBar&);
	TabbedButtonBar& operator= (const TabbedButtonBar&);
};

#endif   // __JUCE_TABBEDBUTTONBAR_JUCEHEADER__
/*** End of inlined file: juce_TabbedButtonBar.h ***/

class JUCE_API  TabbedComponent  : public Component
{
public:

	explicit TabbedComponent (TabbedButtonBar::Orientation orientation);

	~TabbedComponent();

	void setOrientation (TabbedButtonBar::Orientation orientation);

	TabbedButtonBar::Orientation getOrientation() const throw();

	void setTabBarDepth (int newDepth);

	int getTabBarDepth() const throw()			  { return tabDepth; }

	void setOutline (int newThickness);

	void setIndent (int indentThickness);

	void clearTabs();

	void addTab (const String& tabName,
				 const Colour& tabBackgroundColour,
				 Component* contentComponent,
				 bool deleteComponentWhenNotNeeded,
				 int insertIndex = -1);

	void setTabName (int tabIndex, const String& newName);

	void removeTab (int tabIndex);

	int getNumTabs() const;

	const StringArray getTabNames() const;

	Component* getTabContentComponent (int tabIndex) const throw();

	const Colour getTabBackgroundColour (int tabIndex) const throw();

	void setTabBackgroundColour (int tabIndex, const Colour& newColour);

	void setCurrentTabIndex (int newTabIndex, bool sendChangeMessage = true);

	int getCurrentTabIndex() const;

	const String& getCurrentTabName() const;

	Component* getCurrentContentComponent() const throw()	   { return panelComponent; }

	virtual void currentTabChanged (int newCurrentTabIndex,
									const String& newCurrentTabName);

	virtual void popupMenuClickOnTab (int tabIndex,
									  const String& tabName);

	TabbedButtonBar& getTabbedButtonBar() const throw()		 { return *tabs; }

	enum ColourIds
	{
		backgroundColourId	  = 0x1005800,	/**< The colour to fill the background behind the tabs. */
		outlineColourId		 = 0x1005801,	/**< The colour to use to draw an outline around the content.
														 (See setOutline)  */
	};

	void paint (Graphics& g);
	void resized();
	void lookAndFeelChanged();

	juce_UseDebuggingNewOperator

protected:

	TabbedButtonBar* tabs;

	virtual TabBarButton* createTabButton (const String& tabName, int tabIndex);

private:

	Array <Component*> contentComponents;
	Component* panelComponent;
	int tabDepth;
	int outlineThickness, edgeIndent;

	friend class TabCompButtonBar;
	void changeCallback (int newCurrentTabIndex, const String& newTabName);

	TabbedComponent (const TabbedComponent&);
	TabbedComponent& operator= (const TabbedComponent&);
};

#endif   // __JUCE_TABBEDCOMPONENT_JUCEHEADER__
/*** End of inlined file: juce_TabbedComponent.h ***/


/*** Start of inlined file: juce_DocumentWindow.h ***/
#ifndef __JUCE_DOCUMENTWINDOW_JUCEHEADER__
#define __JUCE_DOCUMENTWINDOW_JUCEHEADER__


/*** Start of inlined file: juce_MenuBarComponent.h ***/
#ifndef __JUCE_MENUBARCOMPONENT_JUCEHEADER__
#define __JUCE_MENUBARCOMPONENT_JUCEHEADER__


/*** Start of inlined file: juce_MenuBarModel.h ***/
#ifndef __JUCE_MENUBARMODEL_JUCEHEADER__
#define __JUCE_MENUBARMODEL_JUCEHEADER__

class MenuBarModel;

class JUCE_API  MenuBarModelListener
{
public:
	virtual ~MenuBarModelListener() {}

	virtual void menuBarItemsChanged (MenuBarModel* menuBarModel) = 0;

	virtual void menuCommandInvoked (MenuBarModel* menuBarModel,
									 const ApplicationCommandTarget::InvocationInfo& info) = 0;
};

class JUCE_API  MenuBarModel	  : private AsyncUpdater,
									private ApplicationCommandManagerListener
{
public:

	MenuBarModel() throw();

	virtual ~MenuBarModel();

	void menuItemsChanged();

	void setApplicationCommandManagerToWatch (ApplicationCommandManager* manager) throw();

	void addListener (MenuBarModelListener* listenerToAdd) throw();

	void removeListener (MenuBarModelListener* listenerToRemove) throw();

	virtual const StringArray getMenuBarNames() = 0;

	virtual const PopupMenu getMenuForIndex (int topLevelMenuIndex,
											 const String& menuName) = 0;

	virtual void menuItemSelected (int menuItemID,
								   int topLevelMenuIndex) = 0;

#if JUCE_MAC || DOXYGEN
	static void setMacMainMenu (MenuBarModel* newMenuBarModel,
								const PopupMenu* extraAppleMenuItems = 0);

	static MenuBarModel* getMacMainMenu();

#endif

	void applicationCommandInvoked (const ApplicationCommandTarget::InvocationInfo& info);
	void applicationCommandListChanged();
	void handleAsyncUpdate();

	juce_UseDebuggingNewOperator

private:
	ApplicationCommandManager* manager;
	ListenerList <MenuBarModelListener> listeners;

	MenuBarModel (const MenuBarModel&);
	MenuBarModel& operator= (const MenuBarModel&);
};

#endif   // __JUCE_MENUBARMODEL_JUCEHEADER__
/*** End of inlined file: juce_MenuBarModel.h ***/

class JUCE_API  MenuBarComponent  : public Component,
									private MenuBarModelListener,
									private Timer
{
public:

	MenuBarComponent (MenuBarModel* model);

	~MenuBarComponent();

	void setModel (MenuBarModel* newModel);

	void showMenu (int menuIndex);

	void paint (Graphics& g);
	void resized();
	void mouseEnter (const MouseEvent& e);
	void mouseExit (const MouseEvent& e);
	void mouseDown (const MouseEvent& e);
	void mouseDrag (const MouseEvent& e);
	void mouseUp (const MouseEvent& e);
	void mouseMove (const MouseEvent& e);
	void inputAttemptWhenModal();
	void handleCommandMessage (int commandId);
	bool keyPressed (const KeyPress& key);
	void menuBarItemsChanged (MenuBarModel* menuBarModel);
	void menuCommandInvoked (MenuBarModel* menuBarModel,
							 const ApplicationCommandTarget::InvocationInfo& info);

	juce_UseDebuggingNewOperator

private:
	MenuBarModel* model;

	StringArray menuNames;
	Array <int> xPositions;
	int itemUnderMouse, currentPopupIndex, topLevelIndexClicked, indexToShowAgain;
	int lastMouseX, lastMouseY;
	bool inModalState;
	ScopedPointer <Component> currentPopup;

	int getItemAt (int x, int y);
	void updateItemUnderMouse (int x, int y);
	void hideCurrentMenu();
	void timerCallback();
	void repaintMenuItem (int index);

	MenuBarComponent (const MenuBarComponent&);
	MenuBarComponent& operator= (const MenuBarComponent&);
};

#endif   // __JUCE_MENUBARCOMPONENT_JUCEHEADER__
/*** End of inlined file: juce_MenuBarComponent.h ***/

class JUCE_API  DocumentWindow   : public ResizableWindow
{
public:

	enum TitleBarButtons
	{
		minimiseButton = 1,
		maximiseButton = 2,
		closeButton = 4,

		allButtons = 7
	};

	DocumentWindow (const String& name,
					const Colour& backgroundColour,
					int requiredButtons,
					bool addToDesktop = true);

	~DocumentWindow();

	void setName (const String& newName);

	void setIcon (const Image* imageToUse);

	void setTitleBarHeight (int newHeight);

	int getTitleBarHeight() const;

	void setTitleBarButtonsRequired (int requiredButtons,
									 bool positionTitleBarButtonsOnLeft);

	void setTitleBarTextCentred (bool textShouldBeCentred);

	void setMenuBar (MenuBarModel* menuBarModel,
					 int menuBarHeight = 0);

	virtual void closeButtonPressed();

	virtual void minimiseButtonPressed();

	virtual void maximiseButtonPressed();

	Button* getCloseButton() const throw();

	Button* getMinimiseButton() const throw();

	Button* getMaximiseButton() const throw();

	enum ColourIds
	{
		textColourId		= 0x1005701,  /**< The colour to draw any text with. It's up to the look
													   and feel class how this is used. */
	};

	void paint (Graphics& g);
	void resized();
	void lookAndFeelChanged();
	const BorderSize getBorderThickness();
	const BorderSize getContentComponentBorder();
	void mouseDoubleClick (const MouseEvent& e);
	void userTriedToCloseWindow();
	void activeWindowStatusChanged();
	int getDesktopWindowStyleFlags() const;
	void parentHierarchyChanged();
	const Rectangle<int> getTitleBarArea();

	juce_UseDebuggingNewOperator

private:
	int titleBarHeight, menuBarHeight, requiredButtons;
	bool positionTitleBarButtonsOnLeft, drawTitleTextCentred;
	ScopedPointer <Button> titleBarButtons [3];
	ScopedPointer <Image> titleBarIcon;
	ScopedPointer <MenuBarComponent> menuBar;
	MenuBarModel* menuBarModel;

	class ButtonListenerProxy;
	friend class ScopedPointer <ButtonListenerProxy>;
	ScopedPointer <ButtonListenerProxy> buttonListener;

	void repaintTitleBar();

	DocumentWindow (const DocumentWindow&);
	DocumentWindow& operator= (const DocumentWindow&);
};

#endif   // __JUCE_DOCUMENTWINDOW_JUCEHEADER__
/*** End of inlined file: juce_DocumentWindow.h ***/

class MultiDocumentPanel;
class MDITabbedComponentInternal;

class JUCE_API  MultiDocumentPanelWindow  : public DocumentWindow
{
public:

	MultiDocumentPanelWindow (const Colour& backgroundColour);

	~MultiDocumentPanelWindow();

	void maximiseButtonPressed();
	void closeButtonPressed();
	void activeWindowStatusChanged();
	void broughtToFront();

	juce_UseDebuggingNewOperator

private:
	void updateOrder();
	MultiDocumentPanel* getOwner() const throw();
};

class JUCE_API  MultiDocumentPanel  : public Component,
									  private ComponentListener
{
public:

	MultiDocumentPanel();

	~MultiDocumentPanel();

	bool closeAllDocuments (bool checkItsOkToCloseFirst);

	bool addDocument (Component* component,
					  const Colour& backgroundColour,
					  bool deleteWhenRemoved);

	bool closeDocument (Component* component,
						bool checkItsOkToCloseFirst);

	int getNumDocuments() const throw();

	Component* getDocument (int index) const throw();

	Component* getActiveDocument() const throw();

	void setActiveDocument (Component* component);

	virtual void activeDocumentChanged();

	void setMaximumNumDocuments (int maximumNumDocuments);

	void useFullscreenWhenOneDocument (bool shouldUseTabs);

	bool isFullscreenWhenOneDocument() const throw();

	enum LayoutMode
	{
		FloatingWindows,		/**< In this mode, there are overlapping DocumentWindow components for each document. */
		MaximisedWindowsWithTabs	/**< In this mode, a TabbedComponent is used to show one document at a time. */
	};

	void setLayoutMode (LayoutMode newLayoutMode);

	LayoutMode getLayoutMode() const throw()				{ return mode; }

	void setBackgroundColour (const Colour& newBackgroundColour);

	const Colour& getBackgroundColour() const throw()		   { return backgroundColour; }

	virtual bool tryToCloseDocument (Component* component) = 0;

	virtual MultiDocumentPanelWindow* createNewDocumentWindow();

	void paint (Graphics& g);
	void resized();
	void componentNameChanged (Component&);

	juce_UseDebuggingNewOperator

private:
	LayoutMode mode;
	Array <Component*> components;
	TabbedComponent* tabComponent;
	Colour backgroundColour;
	int maximumNumDocuments, numDocsBeforeTabsUsed;

	friend class MultiDocumentPanelWindow;
	friend class MDITabbedComponentInternal;

	Component* getContainerComp (Component* c) const;
	void updateOrder();

	void addWindow (Component* component);
};

#endif   // __JUCE_MULTIDOCUMENTPANEL_JUCEHEADER__
/*** End of inlined file: juce_MultiDocumentPanel.h ***/


#endif
#ifndef __JUCE_RESIZABLEBORDERCOMPONENT_JUCEHEADER__

#endif
#ifndef __JUCE_RESIZABLECORNERCOMPONENT_JUCEHEADER__

#endif
#ifndef __JUCE_SCROLLBAR_JUCEHEADER__

#endif
#ifndef __JUCE_STRETCHABLELAYOUTMANAGER_JUCEHEADER__

/*** Start of inlined file: juce_StretchableLayoutManager.h ***/
#ifndef __JUCE_STRETCHABLELAYOUTMANAGER_JUCEHEADER__
#define __JUCE_STRETCHABLELAYOUTMANAGER_JUCEHEADER__

class JUCE_API  StretchableLayoutManager
{
public:

	StretchableLayoutManager();

	~StretchableLayoutManager();

	void setItemLayout (int itemIndex,
						double minimumSize,
						double maximumSize,
						double preferredSize);

	bool getItemLayout (int itemIndex,
						double& minimumSize,
						double& maximumSize,
						double& preferredSize) const;

	void clearAllItems();

	void layOutComponents (Component** components,
						   int numComponents,
						   int x, int y, int width, int height,
						   bool vertically,
						   bool resizeOtherDimension);

	int getItemCurrentPosition (int itemIndex) const;

	int getItemCurrentAbsoluteSize (int itemIndex) const;

	double getItemCurrentRelativeSize (int itemIndex) const;

	void setItemPosition (int itemIndex,
						  int newPosition);

	juce_UseDebuggingNewOperator

private:
	struct ItemLayoutProperties
	{
		int itemIndex;
		int currentSize;
		double minSize, maxSize, preferredSize;
	};

	OwnedArray <ItemLayoutProperties> items;
	int totalSize;

	static int sizeToRealSize (double size, int totalSpace);
	ItemLayoutProperties* getInfoFor (int itemIndex) const;
	void setTotalSize (int newTotalSize);
	int fitComponentsIntoSpace (int startIndex, int endIndex, int availableSpace, int startPos);
	int getMinimumSizeOfItems (int startIndex, int endIndex) const;
	int getMaximumSizeOfItems (int startIndex, int endIndex) const;
	void updatePrefSizesToMatchCurrentPositions();

	StretchableLayoutManager (const StretchableLayoutManager&);
	StretchableLayoutManager& operator= (const StretchableLayoutManager&);
};

#endif   // __JUCE_STRETCHABLELAYOUTMANAGER_JUCEHEADER__
/*** End of inlined file: juce_StretchableLayoutManager.h ***/


#endif
#ifndef __JUCE_STRETCHABLELAYOUTRESIZERBAR_JUCEHEADER__

/*** Start of inlined file: juce_StretchableLayoutResizerBar.h ***/
#ifndef __JUCE_STRETCHABLELAYOUTRESIZERBAR_JUCEHEADER__
#define __JUCE_STRETCHABLELAYOUTRESIZERBAR_JUCEHEADER__

class JUCE_API  StretchableLayoutResizerBar  : public Component
{
public:

	StretchableLayoutResizerBar (StretchableLayoutManager* layoutToUse,
								 int itemIndexInLayout,
								 bool isBarVertical);

	~StretchableLayoutResizerBar();

	virtual void hasBeenMoved();

	void paint (Graphics& g);
	void mouseDown (const MouseEvent& e);
	void mouseDrag (const MouseEvent& e);

	juce_UseDebuggingNewOperator

private:
	StretchableLayoutManager* layout;
	int itemIndex, mouseDownPos;
	bool isVertical;

	StretchableLayoutResizerBar (const StretchableLayoutResizerBar&);
	StretchableLayoutResizerBar& operator= (const StretchableLayoutResizerBar&);
};

#endif   // __JUCE_STRETCHABLELAYOUTRESIZERBAR_JUCEHEADER__
/*** End of inlined file: juce_StretchableLayoutResizerBar.h ***/


#endif
#ifndef __JUCE_STRETCHABLEOBJECTRESIZER_JUCEHEADER__

/*** Start of inlined file: juce_StretchableObjectResizer.h ***/
#ifndef __JUCE_STRETCHABLEOBJECTRESIZER_JUCEHEADER__
#define __JUCE_STRETCHABLEOBJECTRESIZER_JUCEHEADER__

class StretchableObjectResizer
{
public:

	StretchableObjectResizer();

	~StretchableObjectResizer();

	void addItem (double currentSize,
				  double minSize,
				  double maxSize,
				  int order = 0);

	void resizeToFit (double targetSize);

	int getNumItems() const throw()			 { return items.size(); }

	double getItemSize (int index) const throw();

	juce_UseDebuggingNewOperator

private:
	struct Item
	{
		double size;
		double minSize;
		double maxSize;
		int order;
	};

	OwnedArray <Item> items;

	StretchableObjectResizer (const StretchableObjectResizer&);
	StretchableObjectResizer& operator= (const StretchableObjectResizer&);
};

#endif   // __JUCE_STRETCHABLEOBJECTRESIZER_JUCEHEADER__
/*** End of inlined file: juce_StretchableObjectResizer.h ***/


#endif
#ifndef __JUCE_TABBEDBUTTONBAR_JUCEHEADER__

#endif
#ifndef __JUCE_TABBEDCOMPONENT_JUCEHEADER__

#endif
#ifndef __JUCE_VIEWPORT_JUCEHEADER__

#endif
#ifndef __JUCE_LOOKANDFEEL_JUCEHEADER__

/*** Start of inlined file: juce_LookAndFeel.h ***/
#ifndef __JUCE_LOOKANDFEEL_JUCEHEADER__
#define __JUCE_LOOKANDFEEL_JUCEHEADER__


/*** Start of inlined file: juce_AlertWindow.h ***/
#ifndef __JUCE_ALERTWINDOW_JUCEHEADER__
#define __JUCE_ALERTWINDOW_JUCEHEADER__


/*** Start of inlined file: juce_TextLayout.h ***/
#ifndef __JUCE_TEXTLAYOUT_JUCEHEADER__
#define __JUCE_TEXTLAYOUT_JUCEHEADER__

class Graphics;

class JUCE_API  TextLayout
{
public:

	TextLayout() throw();

	TextLayout (const TextLayout& other) throw();

	TextLayout (const String& text, const Font& font) throw();

	~TextLayout() throw();

	TextLayout& operator= (const TextLayout& layoutToCopy) throw();

	void clear() throw();

	void appendText (const String& textToAppend,
					 const Font& fontToUse) throw();

	void setText (const String& newText,
				  const Font& fontToUse) throw();

	void layout (int maximumWidth,
				 const Justification& justification,
				 bool attemptToBalanceLineLengths) throw();

	int getWidth() const throw();

	int getHeight() const throw();

	int getNumLines() const throw()		 { return totalLines; }

	int getLineWidth (int lineNumber) const throw();

	void draw (Graphics& g, int topLeftX, int topLeftY) const throw();

	void drawWithin (Graphics& g,
					 int x, int y, int w, int h,
					 const Justification& layoutFlags) const throw();

	juce_UseDebuggingNewOperator

private:
	class Token;
	friend class OwnedArray <Token>;
	OwnedArray <Token> tokens;
	int totalLines;
};

#endif   // __JUCE_TEXTLAYOUT_JUCEHEADER__
/*** End of inlined file: juce_TextLayout.h ***/

class JUCE_API  AlertWindow  : public TopLevelWindow,
							   private ButtonListener
{
public:

	enum AlertIconType
	{
		NoIcon,	 /**< No icon will be shown on the dialog box. */
		QuestionIcon,   /**< A question-mark icon, for dialog boxes that need the
							 user to answer a question. */
		WarningIcon,	/**< An exclamation mark to indicate that the dialog is a
							 warning about something and shouldn't be ignored. */
		InfoIcon	/**< An icon that indicates that the dialog box is just
							 giving the user some information, which doesn't require
							 a response from them. */
	};

	AlertWindow (const String& title,
				 const String& message,
				 AlertIconType iconType,
				 Component* associatedComponent = 0);

	~AlertWindow();

	AlertIconType getAlertType() const throw()		  { return alertIconType; }

	void setMessage (const String& message);

	void addButton (const String& name,
					int returnValue,
					const KeyPress& shortcutKey1 = KeyPress(),
					const KeyPress& shortcutKey2 = KeyPress());

	int getNumButtons() const;

	void addTextEditor (const String& name,
						const String& initialContents,
						const String& onScreenLabel = String::empty,
						bool isPasswordBox = false);

	const String getTextEditorContents (const String& nameOfTextEditor) const;

	void addComboBox (const String& name,
					  const StringArray& items,
					  const String& onScreenLabel = String::empty);

	ComboBox* getComboBoxComponent (const String& nameOfList) const;

	void addTextBlock (const String& text);

	void addProgressBarComponent (double& progressValue);

	void addCustomComponent (Component* component);

	int getNumCustomComponents() const;

	Component* getCustomComponent (int index) const;

	Component* removeCustomComponent (int index);

	bool containsAnyExtraComponents() const;

	// easy-to-use message box functions:

	static void JUCE_CALLTYPE showMessageBox (AlertIconType iconType,
											  const String& title,
											  const String& message,
											  const String& buttonText = String::empty,
											  Component* associatedComponent = 0);

	static bool JUCE_CALLTYPE showOkCancelBox (AlertIconType iconType,
											   const String& title,
											   const String& message,
											   const String& button1Text = String::empty,
											   const String& button2Text = String::empty,
											   Component* associatedComponent = 0);

	static int JUCE_CALLTYPE showYesNoCancelBox (AlertIconType iconType,
												 const String& title,
												 const String& message,
												 const String& button1Text = String::empty,
												 const String& button2Text = String::empty,
												 const String& button3Text = String::empty,
												 Component* associatedComponent = 0);

	static bool JUCE_CALLTYPE showNativeDialogBox (const String& title,
												   const String& bodyText,
												   bool isOkCancel);

	enum ColourIds
	{
		backgroundColourId	  = 0x1001800,  /**< The background colour for the window. */
		textColourId		= 0x1001810,  /**< The colour for the text. */
		outlineColourId		 = 0x1001820   /**< An optional colour to use to draw a border around the window. */
	};

	juce_UseDebuggingNewOperator

protected:
	void paint (Graphics& g);
	void mouseDown (const MouseEvent& e);
	void mouseDrag (const MouseEvent& e);
	bool keyPressed (const KeyPress& key);
	void buttonClicked (Button* button);
	void lookAndFeelChanged();
	void userTriedToCloseWindow();
	int getDesktopWindowStyleFlags() const;

private:
	String text;
	TextLayout textLayout;
	AlertIconType alertIconType;
	ComponentBoundsConstrainer constrainer;
	ComponentDragger dragger;
	Rectangle<int> textArea;
	VoidArray buttons, textBoxes, comboBoxes;
	VoidArray progressBars, customComps, textBlocks, allComps;
	StringArray textboxNames, comboBoxNames;
	Font font;
	Component* associatedComponent;

	void updateLayout (bool onlyIncreaseSize);

	// disable copy constructor
	AlertWindow (const AlertWindow&);
	AlertWindow& operator= (const AlertWindow&);
};

#endif   // __JUCE_ALERTWINDOW_JUCEHEADER__
/*** End of inlined file: juce_AlertWindow.h ***/

class ToggleButton;
class TextButton;
class AlertWindow;
class TextLayout;
class ScrollBar;
class BubbleComponent;
class ComboBox;
class Button;
class FilenameComponent;
class DocumentWindow;
class ResizableWindow;
class GroupComponent;
class MenuBarComponent;
class DropShadower;
class GlyphArrangement;
class PropertyComponent;
class TableHeaderComponent;
class Toolbar;
class ToolbarItemComponent;
class PopupMenu;
class ProgressBar;
class FileBrowserComponent;
class DirectoryContentsDisplayComponent;
class FilePreviewComponent;
class ImageButton;

class JUCE_API  LookAndFeel
{
public:

	LookAndFeel();

	virtual ~LookAndFeel();

	static LookAndFeel& getDefaultLookAndFeel() throw();

	static void setDefaultLookAndFeel (LookAndFeel* newDefaultLookAndFeel) throw();

	const Colour findColour (int colourId) const throw();

	void setColour (int colourId, const Colour& colour) throw();

	bool isColourSpecified (int colourId) const throw();

	virtual const Typeface::Ptr getTypefaceForFont (const Font& font);

	void setDefaultSansSerifTypefaceName (const String& newName);

	virtual const MouseCursor getMouseCursorFor (Component& component);

	virtual void drawButtonBackground (Graphics& g,
									   Button& button,
									   const Colour& backgroundColour,
									   bool isMouseOverButton,
									   bool isButtonDown);

	virtual const Font getFontForTextButton (TextButton& button);

	virtual void drawButtonText (Graphics& g,
								 TextButton& button,
								 bool isMouseOverButton,
								 bool isButtonDown);

	virtual void drawToggleButton (Graphics& g,
								   ToggleButton& button,
								   bool isMouseOverButton,
								   bool isButtonDown);

	virtual void changeToggleButtonWidthToFitText (ToggleButton& button);

	virtual void drawTickBox (Graphics& g,
							  Component& component,
							  float x, float y, float w, float h,
							  bool ticked,
							  bool isEnabled,
							  bool isMouseOverButton,
							  bool isButtonDown);

	virtual AlertWindow* createAlertWindow (const String& title,
											const String& message,
											const String& button1,
											const String& button2,
											const String& button3,
											AlertWindow::AlertIconType iconType,
											int numButtons,
											Component* associatedComponent);

	virtual void drawAlertBox (Graphics& g,
							   AlertWindow& alert,
							   const Rectangle<int>& textArea,
							   TextLayout& textLayout);

	virtual int getAlertBoxWindowFlags();

	virtual int getAlertWindowButtonHeight();

	virtual const Font getAlertWindowFont();

	virtual void drawProgressBar (Graphics& g, ProgressBar& progressBar,
								  int width, int height,
								  double progress, const String& textToShow);

	// Draws a small image that spins to indicate that something's happening..
	// This method should use the current time to animate itself, so just keep
	// repainting it every so often.
	virtual void drawSpinningWaitAnimation (Graphics& g, const Colour& colour,
											int x, int y, int w, int h);

	virtual void drawScrollbarButton (Graphics& g,
									  ScrollBar& scrollbar,
									  int width, int height,
									  int buttonDirection,
									  bool isScrollbarVertical,
									  bool isMouseOverButton,
									  bool isButtonDown);

	virtual void drawScrollbar (Graphics& g,
								ScrollBar& scrollbar,
								int x, int y,
								int width, int height,
								bool isScrollbarVertical,
								int thumbStartPosition,
								int thumbSize,
								bool isMouseOver,
								bool isMouseDown);

	virtual ImageEffectFilter* getScrollbarEffect();

	virtual int getMinimumScrollbarThumbSize (ScrollBar& scrollbar);

	virtual int getDefaultScrollbarWidth();

	virtual int getScrollbarButtonSize (ScrollBar& scrollbar);

	virtual const Path getTickShape (float height);
	virtual const Path getCrossShape (float height);

	virtual void drawTreeviewPlusMinusBox (Graphics& g, int x, int y, int w, int h, bool isPlus, bool isMouseOver);

	virtual void fillTextEditorBackground (Graphics& g, int width, int height, TextEditor& textEditor);
	virtual void drawTextEditorOutline (Graphics& g, int width, int height, TextEditor& textEditor);

	// these return an image from the ImageCache, so use ImageCache::release() to free it
	virtual Image* getDefaultFolderImage();
	virtual Image* getDefaultDocumentFileImage();

	virtual void createFileChooserHeaderText (const String& title,
											  const String& instructions,
											  GlyphArrangement& destArrangement,
											  int width);

	virtual void drawFileBrowserRow (Graphics& g, int width, int height,
									 const String& filename, Image* icon,
									 const String& fileSizeDescription,
									 const String& fileTimeDescription,
									 bool isDirectory,
									 bool isItemSelected,
									 int itemIndex);

	virtual Button* createFileBrowserGoUpButton();

	virtual void layoutFileBrowserComponent (FileBrowserComponent& browserComp,
											 DirectoryContentsDisplayComponent* fileListComponent,
											 FilePreviewComponent* previewComp,
											 ComboBox* currentPathBox,
											 TextEditor* filenameBox,
											 Button* goUpButton);

	virtual void drawBubble (Graphics& g,
							 float tipX, float tipY,
							 float boxX, float boxY, float boxW, float boxH);

	virtual void drawPopupMenuBackground (Graphics& g, int width, int height);

	virtual void drawPopupMenuItem (Graphics& g,
									int width, int height,
									bool isSeparator,
									bool isActive,
									bool isHighlighted,
									bool isTicked,
									bool hasSubMenu,
									const String& text,
									const String& shortcutKeyText,
									Image* image,
									const Colour* const textColour);

	virtual const Font getPopupMenuFont();

	virtual void drawPopupMenuUpDownArrow (Graphics& g,
										   int width, int height,
										   bool isScrollUpArrow);

	virtual void getIdealPopupMenuItemSize (const String& text,
											bool isSeparator,
											int standardMenuItemHeight,
											int& idealWidth,
											int& idealHeight);

	virtual int getMenuWindowFlags();

	virtual void drawMenuBarBackground (Graphics& g, int width, int height,
										bool isMouseOverBar,
										MenuBarComponent& menuBar);

	virtual int getMenuBarItemWidth (MenuBarComponent& menuBar, int itemIndex, const String& itemText);

	virtual const Font getMenuBarFont (MenuBarComponent& menuBar, int itemIndex, const String& itemText);

	virtual void drawMenuBarItem (Graphics& g,
								  int width, int height,
								  int itemIndex,
								  const String& itemText,
								  bool isMouseOverItem,
								  bool isMenuOpen,
								  bool isMouseOverBar,
								  MenuBarComponent& menuBar);

	virtual void drawComboBox (Graphics& g, int width, int height,
							   bool isButtonDown,
							   int buttonX, int buttonY,
							   int buttonW, int buttonH,
							   ComboBox& box);

	virtual const Font getComboBoxFont (ComboBox& box);

	virtual Label* createComboBoxTextBox (ComboBox& box);

	virtual void positionComboBoxText (ComboBox& box, Label& labelToPosition);

	virtual void drawLabel (Graphics& g, Label& label);

	virtual void drawLinearSlider (Graphics& g,
								   int x, int y,
								   int width, int height,
								   float sliderPos,
								   float minSliderPos,
								   float maxSliderPos,
								   const Slider::SliderStyle style,
								   Slider& slider);

	virtual void drawLinearSliderBackground (Graphics& g,
											 int x, int y,
											 int width, int height,
											 float sliderPos,
											 float minSliderPos,
											 float maxSliderPos,
											 const Slider::SliderStyle style,
											 Slider& slider);

	virtual void drawLinearSliderThumb (Graphics& g,
										int x, int y,
										int width, int height,
										float sliderPos,
										float minSliderPos,
										float maxSliderPos,
										const Slider::SliderStyle style,
										Slider& slider);

	virtual int getSliderThumbRadius (Slider& slider);

	virtual void drawRotarySlider (Graphics& g,
								   int x, int y,
								   int width, int height,
								   float sliderPosProportional,
								   float rotaryStartAngle,
								   float rotaryEndAngle,
								   Slider& slider);

	virtual Button* createSliderButton (bool isIncrement);
	virtual Label* createSliderTextBox (Slider& slider);

	virtual ImageEffectFilter* getSliderEffect();

	virtual void getTooltipSize (const String& tipText, int& width, int& height);

	virtual void drawTooltip (Graphics& g, const String& text, int width, int height);

	virtual Button* createFilenameComponentBrowseButton (const String& text);

	virtual void layoutFilenameComponent (FilenameComponent& filenameComp,
										  ComboBox* filenameBox, Button* browseButton);

	virtual void drawCornerResizer (Graphics& g,
									int w, int h,
									bool isMouseOver,
									bool isMouseDragging);

	virtual void drawResizableFrame (Graphics& g,
									int w, int h,
									const BorderSize& borders);

	virtual void fillResizableWindowBackground (Graphics& g, int w, int h,
												const BorderSize& border,
												ResizableWindow& window);

	virtual void drawResizableWindowBorder (Graphics& g,
											int w, int h,
											const BorderSize& border,
											ResizableWindow& window);

	virtual void drawDocumentWindowTitleBar (DocumentWindow& window,
											 Graphics& g, int w, int h,
											 int titleSpaceX, int titleSpaceW,
											 const Image* icon,
											 bool drawTitleTextOnLeft);

	virtual Button* createDocumentWindowButton (int buttonType);

	virtual void positionDocumentWindowButtons (DocumentWindow& window,
												int titleBarX, int titleBarY,
												int titleBarW, int titleBarH,
												Button* minimiseButton,
												Button* maximiseButton,
												Button* closeButton,
												bool positionTitleBarButtonsOnLeft);

	virtual int getDefaultMenuBarHeight();

	virtual DropShadower* createDropShadowerForComponent (Component* component);

	virtual void drawStretchableLayoutResizerBar (Graphics& g,
												  int w, int h,
												  bool isVerticalBar,
												  bool isMouseOver,
												  bool isMouseDragging);

	virtual void drawGroupComponentOutline (Graphics& g, int w, int h,
											const String& text,
											const Justification& position,
											GroupComponent& group);

	virtual void createTabButtonShape (Path& p,
									   int width, int height,
									   int tabIndex,
									   const String& text,
									   Button& button,
									   TabbedButtonBar::Orientation orientation,
									   bool isMouseOver,
									   bool isMouseDown,
									   bool isFrontTab);

	virtual void fillTabButtonShape (Graphics& g,
									 const Path& path,
									 const Colour& preferredBackgroundColour,
									 int tabIndex,
									 const String& text,
									 Button& button,
									 TabbedButtonBar::Orientation orientation,
									 bool isMouseOver,
									 bool isMouseDown,
									 bool isFrontTab);

	virtual void drawTabButtonText (Graphics& g,
									int x, int y, int w, int h,
									const Colour& preferredBackgroundColour,
									int tabIndex,
									const String& text,
									Button& button,
									TabbedButtonBar::Orientation orientation,
									bool isMouseOver,
									bool isMouseDown,
									bool isFrontTab);

	virtual int getTabButtonOverlap (int tabDepth);
	virtual int getTabButtonSpaceAroundImage();

	virtual int getTabButtonBestWidth (int tabIndex,
									   const String& text,
									   int tabDepth,
									   Button& button);

	virtual void drawTabButton (Graphics& g,
								int w, int h,
								const Colour& preferredColour,
								int tabIndex,
								const String& text,
								Button& button,
								TabbedButtonBar::Orientation orientation,
								bool isMouseOver,
								bool isMouseDown,
								bool isFrontTab);

	virtual void drawTabAreaBehindFrontButton (Graphics& g,
											   int w, int h,
											   TabbedButtonBar& tabBar,
											   TabbedButtonBar::Orientation orientation);

	virtual Button* createTabBarExtrasButton();

	virtual void drawImageButton (Graphics& g, Image* image,
								  int imageX, int imageY, int imageW, int imageH,
								  const Colour& overlayColour,
								  float imageOpacity,
								  ImageButton& button);

	virtual void drawTableHeaderBackground (Graphics& g, TableHeaderComponent& header);

	virtual void drawTableHeaderColumn (Graphics& g, const String& columnName, int columnId,
										int width, int height,
										bool isMouseOver, bool isMouseDown,
										int columnFlags);

	virtual void paintToolbarBackground (Graphics& g, int width, int height, Toolbar& toolbar);

	virtual Button* createToolbarMissingItemsButton (Toolbar& toolbar);

	virtual void paintToolbarButtonBackground (Graphics& g, int width, int height,
											   bool isMouseOver, bool isMouseDown,
											   ToolbarItemComponent& component);

	virtual void paintToolbarButtonLabel (Graphics& g, int x, int y, int width, int height,
										  const String& text, ToolbarItemComponent& component);

	virtual void drawPropertyPanelSectionHeader (Graphics& g, const String& name,
												 bool isOpen, int width, int height);

	virtual void drawPropertyComponentBackground (Graphics& g, int width, int height,
												  PropertyComponent& component);

	virtual void drawPropertyComponentLabel (Graphics& g, int width, int height,
											 PropertyComponent& component);

	virtual const Rectangle<int> getPropertyComponentContentPosition (PropertyComponent& component);

	virtual void drawLevelMeter (Graphics& g, int width, int height, float level);

	virtual void drawKeymapChangeButton (Graphics& g, int width, int height, Button& button, const String& keyDescription);

	virtual void playAlertSound();

	static void drawGlassSphere (Graphics& g,
								 float x, float y,
								 float diameter,
								 const Colour& colour,
								 float outlineThickness) throw();

	static void drawGlassPointer (Graphics& g,
								  float x, float y,
								  float diameter,
								  const Colour& colour, float outlineThickness,
								  int direction) throw();

	static void drawGlassLozenge (Graphics& g,
								  float x, float y,
								  float width, float height,
								  const Colour& colour,
								  float outlineThickness,
								  float cornerSize,
								  bool flatOnLeft, bool flatOnRight,
								  bool flatOnTop, bool flatOnBottom) throw();

	juce_UseDebuggingNewOperator

private:
	friend void JUCE_PUBLIC_FUNCTION shutdownJuce_GUI();
	static void clearDefaultLookAndFeel() throw(); // called at shutdown

	Array <int> colourIds;
	Array <Colour> colours;

	// default typeface names
	String defaultSans, defaultSerif, defaultFixed;

	void drawShinyButtonShape (Graphics& g,
							   float x, float y, float w, float h, float maxCornerSize,
							   const Colour& baseColour,
							   float strokeWidth,
							   bool flatOnLeft,
							   bool flatOnRight,
							   bool flatOnTop,
							   bool flatOnBottom) throw();

	LookAndFeel (const LookAndFeel&);
	LookAndFeel& operator= (const LookAndFeel&);
};

#endif   // __JUCE_LOOKANDFEEL_JUCEHEADER__
/*** End of inlined file: juce_LookAndFeel.h ***/


#endif
#ifndef __JUCE_OLDSCHOOLLOOKANDFEEL_JUCEHEADER__

/*** Start of inlined file: juce_OldSchoolLookAndFeel.h ***/
#ifndef __JUCE_OLDSCHOOLLOOKANDFEEL_JUCEHEADER__
#define __JUCE_OLDSCHOOLLOOKANDFEEL_JUCEHEADER__

class JUCE_API  OldSchoolLookAndFeel	: public LookAndFeel
{
public:

	OldSchoolLookAndFeel();

	virtual ~OldSchoolLookAndFeel();

	virtual void drawButtonBackground (Graphics& g,
									   Button& button,
									   const Colour& backgroundColour,
									   bool isMouseOverButton,
									   bool isButtonDown);

	virtual void drawToggleButton (Graphics& g,
								   ToggleButton& button,
								   bool isMouseOverButton,
								   bool isButtonDown);

	virtual void drawTickBox (Graphics& g,
							  Component& component,
							  float x, float y, float w, float h,
							  bool ticked,
							  bool isEnabled,
							  bool isMouseOverButton,
							  bool isButtonDown);

	virtual void drawProgressBar (Graphics& g, ProgressBar& progressBar,
								  int width, int height,
								  double progress, const String& textToShow);

	virtual void drawScrollbarButton (Graphics& g,
									  ScrollBar& scrollbar,
									  int width, int height,
									  int buttonDirection,
									  bool isScrollbarVertical,
									  bool isMouseOverButton,
									  bool isButtonDown);

	virtual void drawScrollbar (Graphics& g,
								ScrollBar& scrollbar,
								int x, int y,
								int width, int height,
								bool isScrollbarVertical,
								int thumbStartPosition,
								int thumbSize,
								bool isMouseOver,
								bool isMouseDown);

	virtual ImageEffectFilter* getScrollbarEffect();

	virtual void drawTextEditorOutline (Graphics& g,
										int width, int height,
										TextEditor& textEditor);

	virtual void drawPopupMenuBackground (Graphics& g, int width, int height);

	virtual void drawMenuBarBackground (Graphics& g, int width, int height,
										bool isMouseOverBar,
										MenuBarComponent& menuBar);

	virtual void drawComboBox (Graphics& g, int width, int height,
							   bool isButtonDown,
							   int buttonX, int buttonY,
							   int buttonW, int buttonH,
							   ComboBox& box);

	virtual const Font getComboBoxFont (ComboBox& box);

	virtual void drawLinearSlider (Graphics& g,
								   int x, int y,
								   int width, int height,
								   float sliderPos,
								   float minSliderPos,
								   float maxSliderPos,
								   const Slider::SliderStyle style,
								   Slider& slider);

	virtual int getSliderThumbRadius (Slider& slider);

	virtual Button* createSliderButton (bool isIncrement);

	virtual ImageEffectFilter* getSliderEffect();

	virtual void drawCornerResizer (Graphics& g,
									int w, int h,
									bool isMouseOver,
									bool isMouseDragging);

	virtual Button* createDocumentWindowButton (int buttonType);

	virtual void positionDocumentWindowButtons (DocumentWindow& window,
												int titleBarX, int titleBarY,
												int titleBarW, int titleBarH,
												Button* minimiseButton,
												Button* maximiseButton,
												Button* closeButton,
												bool positionTitleBarButtonsOnLeft);

	juce_UseDebuggingNewOperator

private:
	DropShadowEffect scrollbarShadow;

	OldSchoolLookAndFeel (const OldSchoolLookAndFeel&);
	OldSchoolLookAndFeel& operator= (const OldSchoolLookAndFeel&);
};

#endif   // __JUCE_OLDSCHOOLLOOKANDFEEL_JUCEHEADER__
/*** End of inlined file: juce_OldSchoolLookAndFeel.h ***/


#endif
#ifndef __JUCE_MENUBARCOMPONENT_JUCEHEADER__

#endif
#ifndef __JUCE_MENUBARMODEL_JUCEHEADER__

#endif
#ifndef __JUCE_POPUPMENU_JUCEHEADER__

#endif
#ifndef __JUCE_POPUPMENUCUSTOMCOMPONENT_JUCEHEADER__

/*** Start of inlined file: juce_PopupMenuCustomComponent.h ***/
#ifndef __JUCE_POPUPMENUCUSTOMCOMPONENT_JUCEHEADER__
#define __JUCE_POPUPMENUCUSTOMCOMPONENT_JUCEHEADER__

class JUCE_API  PopupMenuCustomComponent  : public Component,
											public ReferenceCountedObject
{
public:
	~PopupMenuCustomComponent();

	virtual void getIdealSize (int& idealWidth,
							   int& idealHeight) = 0;

	void triggerMenuItem();

	bool isItemHighlighted() const throw()		   { return isHighlighted; }

protected:
	PopupMenuCustomComponent (bool isTriggeredAutomatically = true);

private:
	friend class PopupMenu;
	friend class PopupMenu::ItemComponent;
	friend class PopupMenu::Window;
	bool isHighlighted, isTriggeredAutomatically;

	PopupMenuCustomComponent (const PopupMenuCustomComponent&);
	PopupMenuCustomComponent& operator= (const PopupMenuCustomComponent&);
};

#endif   // __JUCE_POPUPMENUCUSTOMCOMPONENT_JUCEHEADER__
/*** End of inlined file: juce_PopupMenuCustomComponent.h ***/


#endif
#ifndef __JUCE_COMPONENTDRAGGER_JUCEHEADER__

#endif
#ifndef __JUCE_DRAGANDDROPCONTAINER_JUCEHEADER__

#endif
#ifndef __JUCE_DRAGANDDROPTARGET_JUCEHEADER__

#endif
#ifndef __JUCE_FILEDRAGANDDROPTARGET_JUCEHEADER__

#endif
#ifndef __JUCE_LASSOCOMPONENT_JUCEHEADER__

/*** Start of inlined file: juce_LassoComponent.h ***/
#ifndef __JUCE_LASSOCOMPONENT_JUCEHEADER__
#define __JUCE_LASSOCOMPONENT_JUCEHEADER__


/*** Start of inlined file: juce_SelectedItemSet.h ***/
#ifndef __JUCE_SELECTEDITEMSET_JUCEHEADER__
#define __JUCE_SELECTEDITEMSET_JUCEHEADER__

template <class SelectableItemType>
class JUCE_API  SelectedItemSet   : public ChangeBroadcaster
{
public:

	typedef SelectableItemType ItemType;

	SelectedItemSet()
	{
	}

	explicit SelectedItemSet (const Array <SelectableItemType>& items)
		: selectedItems (items)
	{
	}

	SelectedItemSet (const SelectedItemSet& other)
		: selectedItems (other.selectedItems)
	{
	}

	SelectedItemSet& operator= (const SelectedItemSet& other)
	{
		if (selectedItems != other.selectedItems)
		{
			selectedItems = other.selectedItems;
			changed();
		}

		return *this;
	}

	~SelectedItemSet()
	{
	}

	void selectOnly (SelectableItemType item)
	{
		if (isSelected (item))
		{
			for (int i = selectedItems.size(); --i >= 0;)
			{
				if (selectedItems.getUnchecked(i) != item)
				{
					deselect (selectedItems.getUnchecked(i));
					i = jmin (i, selectedItems.size());
				}
			}
		}
		else
		{
			deselectAll();
			changed();

			selectedItems.add (item);
			itemSelected (item);
		}
	}

	void addToSelection (SelectableItemType item)
	{
		if (! isSelected (item))
		{
			changed();

			selectedItems.add (item);
			itemSelected (item);
		}
	}

	void addToSelectionBasedOnModifiers (SelectableItemType item,
										 const ModifierKeys& modifiers)
	{
		if (modifiers.isShiftDown())
		{
			addToSelection (item);
		}
		else if (modifiers.isCommandDown())
		{
			if (isSelected (item))
				deselect (item);
			else
				addToSelection (item);
		}
		else
		{
			selectOnly (item);
		}
	}

	bool addToSelectionOnMouseDown (SelectableItemType item,
									const ModifierKeys& modifiers)
	{
		if (isSelected (item))
		{
			return ! modifiers.isPopupMenu();
		}
		else
		{
			addToSelectionBasedOnModifiers (item, modifiers);
			return false;
		}
	}

	void addToSelectionOnMouseUp (SelectableItemType item,
								  const ModifierKeys& modifiers,
								  const bool wasItemDragged,
								  const bool resultOfMouseDownSelectMethod)
	{
		if (resultOfMouseDownSelectMethod && ! wasItemDragged)
			addToSelectionBasedOnModifiers (item, modifiers);
	}

	void deselect (SelectableItemType item)
	{
		const int i = selectedItems.indexOf (item);

		if (i >= 0)
		{
			changed();
			itemDeselected (selectedItems.remove (i));
		}
	}

	void deselectAll()
	{
		if (selectedItems.size() > 0)
		{
			changed();

			for (int i = selectedItems.size(); --i >= 0;)
			{
				itemDeselected (selectedItems.remove (i));
				i = jmin (i, selectedItems.size());
			}
		}
	}

	int getNumSelected() const throw()
	{
		return selectedItems.size();
	}

	SelectableItemType getSelectedItem (const int index) const throw()
	{
		return selectedItems [index];
	}

	bool isSelected (const SelectableItemType item) const throw()
	{
		return selectedItems.contains (item);
	}

	const Array <SelectableItemType>& getItemArray() const throw()	  { return selectedItems; }

	virtual void itemSelected (SelectableItemType item)			 {}

	virtual void itemDeselected (SelectableItemType item)		   {}

	void changed (const bool synchronous = false)
	{
		if (synchronous)
			sendSynchronousChangeMessage (this);
		else
			sendChangeMessage (this);
	}

	juce_UseDebuggingNewOperator

private:
	Array <SelectableItemType> selectedItems;
};

#endif   // __JUCE_SELECTEDITEMSET_JUCEHEADER__
/*** End of inlined file: juce_SelectedItemSet.h ***/

template <class SelectableItemType>
class LassoSource
{
public:
	virtual ~LassoSource() {}

	virtual void findLassoItemsInArea (Array <SelectableItemType>& itemsFound,
									   int x, int y, int width, int height) = 0;

	virtual SelectedItemSet <SelectableItemType>& getLassoSelection() = 0;
};

template <class SelectableItemType>
class LassoComponent  : public Component
{
public:

	explicit LassoComponent (const int outlineThickness_ = 1)
		: source (0),
		  outlineThickness (outlineThickness_)
	{
	}

	~LassoComponent()
	{
	}

	void beginLasso (const MouseEvent& e,
					 LassoSource <SelectableItemType>* const lassoSource)
	{
		jassert (source == 0);  // this suggests that you didn't call endLasso() after the last drag...
		jassert (lassoSource != 0); // the source can't be null!
		jassert (getParentComponent() != 0);  // you need to add this to a parent component for it to work!

		source = lassoSource;

		if (lassoSource != 0)
			originalSelection = lassoSource->getLassoSelection().getItemArray();

		setSize (0, 0);
	}

	void dragLasso (const MouseEvent& e)
	{
		if (source != 0)
		{
			setBounds (Rectangle<int> (e.getMouseDownPosition(), e.getPosition()));
			setVisible (true);

			Array <SelectableItemType> itemsInLasso;
			source->findLassoItemsInArea (itemsInLasso, getX(), getY(), getWidth(), getHeight());

			if (e.mods.isShiftDown())
			{
				itemsInLasso.removeValuesIn (originalSelection); //  to avoid duplicates
				itemsInLasso.addArray (originalSelection);
			}
			else if (e.mods.isCommandDown() || e.mods.isAltDown())
			{
				Array <SelectableItemType> originalMinusNew (originalSelection);
				originalMinusNew.removeValuesIn (itemsInLasso);

				itemsInLasso.removeValuesIn (originalSelection);
				itemsInLasso.addArray (originalMinusNew);
			}

			source->getLassoSelection() = SelectedItemSet <SelectableItemType> (itemsInLasso);
		}
	}

	void endLasso()
	{
		source = 0;
		originalSelection.clear();
		setVisible (false);
	}

	enum ColourIds
	{
		lassoFillColourId	   = 0x1000440, /**< The colour to fill the lasso rectangle with. */
		lassoOutlineColourId	= 0x1000441, /**< The colour to draw the outline with. */
	};

	void paint (Graphics& g)
	{
		g.fillAll (findColour (lassoFillColourId));

		g.setColour (findColour (lassoOutlineColourId));
		g.drawRect (0, 0, getWidth(), getHeight(), outlineThickness);

		// this suggests that you've left a lasso comp lying around after the
		// mouse drag has finished.. Be careful to call endLasso() when you get a
		// mouse-up event.
		jassert (isMouseButtonDownAnywhere());
	}

	bool hitTest (int x, int y)	 { return false; }

	juce_UseDebuggingNewOperator

private:
	Array <SelectableItemType> originalSelection;
	LassoSource <SelectableItemType>* source;
	int outlineThickness;
};

#endif   // __JUCE_LASSOCOMPONENT_JUCEHEADER__
/*** End of inlined file: juce_LassoComponent.h ***/


#endif
#ifndef __JUCE_MOUSECURSOR_JUCEHEADER__

#endif
#ifndef __JUCE_MOUSEEVENT_JUCEHEADER__

#endif
#ifndef __JUCE_MOUSEHOVERDETECTOR_JUCEHEADER__

/*** Start of inlined file: juce_MouseHoverDetector.h ***/
#ifndef __JUCE_MOUSEHOVERDETECTOR_JUCEHEADER__
#define __JUCE_MOUSEHOVERDETECTOR_JUCEHEADER__

class JUCE_API  MouseHoverDetector
{
public:

	MouseHoverDetector (const int hoverTimeMillisecs = 400);

	virtual ~MouseHoverDetector();

	void setHoverTimeMillisecs (const int newTimeInMillisecs);

	void setHoverComponent (Component* const newSourceComponent);

protected:

	virtual void mouseHovered (int mouseX,
							   int mouseY) = 0;

	virtual void mouseMovedAfterHover() = 0;

private:

	class JUCE_API  HoverDetectorInternal  : public MouseListener,
											 public Timer
	{
	public:
		MouseHoverDetector* owner;
		int lastX, lastY;

		void timerCallback();
		void mouseEnter (const MouseEvent&);
		void mouseExit (const MouseEvent&);
		void mouseDown (const MouseEvent&);
		void mouseUp (const MouseEvent&);
		void mouseMove (const MouseEvent&);
		void mouseWheelMove (const MouseEvent&, float, float);

	} internalTimer;

	friend class HoverDetectorInternal;

	Component* source;
	int hoverTimeMillisecs;
	bool hasJustHovered;

	void hoverTimerCallback();
	void checkJustHoveredCallback();

	MouseHoverDetector (const MouseHoverDetector&);
	MouseHoverDetector& operator= (const MouseHoverDetector&);
};

#endif   // __JUCE_MOUSEHOVERDETECTOR_JUCEHEADER__
/*** End of inlined file: juce_MouseHoverDetector.h ***/


#endif
#ifndef __JUCE_MOUSEINPUTSOURCE_JUCEHEADER__

/*** Start of inlined file: juce_MouseInputSource.h ***/
#ifndef __JUCE_MOUSEINPUTSOURCE_JUCEHEADER__
#define __JUCE_MOUSEINPUTSOURCE_JUCEHEADER__

class Component;
class ComponentPeer;
class MouseInputSourceInternal;

class JUCE_API  MouseInputSource
{
public:

	MouseInputSource (int index, bool isMouseDevice);

	~MouseInputSource();

	bool isMouse() const;

	bool isTouch() const;

	bool canHover() const;

	bool hasMouseWheel() const;

	int getIndex() const;

	bool isDragging() const;

	const Point<int> getScreenPosition() const;

	const ModifierKeys getCurrentModifiers() const;

	Component* getComponentUnderMouse() const;

	void triggerFakeMove() const;

	int getNumberOfMultipleClicks() const throw();

	const Time getLastMouseDownTime() const throw();

	const Point<int> getLastMouseDownPosition() const throw();

	bool hasMouseMovedSignificantlySincePressed() const throw();

	bool hasMouseCursor() const throw();
	void showMouseCursor (const MouseCursor& cursor);
	void hideCursor();
	void revealCursor();
	void forceMouseCursorUpdate();

	bool canDoUnboundedMovement() const throw();

	void enableUnboundedMouseMovement (bool isEnabled, bool keepCursorVisibleUntilOffscreen = false);

	juce_UseDebuggingNewOperator

	void handleEvent (ComponentPeer* peer, const Point<int>& positionWithinPeer, int64 time, const ModifierKeys& mods);
	void handleWheel (ComponentPeer* peer, const Point<int>& positionWithinPeer, int64 time, float x, float y);

private:
	friend class Desktop;
	friend class ComponentPeer;
	friend class MouseInputSourceInternal;
	ScopedPointer<MouseInputSourceInternal> pimpl;

	MouseInputSource (const MouseInputSource&);
	MouseInputSource& operator= (const MouseInputSource&);
};

#endif   // __JUCE_MOUSEINPUTSOURCE_JUCEHEADER__
/*** End of inlined file: juce_MouseInputSource.h ***/


#endif
#ifndef __JUCE_MOUSELISTENER_JUCEHEADER__

#endif
#ifndef __JUCE_TOOLTIPCLIENT_JUCEHEADER__

#endif
#ifndef __JUCE_BOOLEANPROPERTYCOMPONENT_JUCEHEADER__

/*** Start of inlined file: juce_BooleanPropertyComponent.h ***/
#ifndef __JUCE_BOOLEANPROPERTYCOMPONENT_JUCEHEADER__
#define __JUCE_BOOLEANPROPERTYCOMPONENT_JUCEHEADER__

class JUCE_API  BooleanPropertyComponent  : public PropertyComponent,
											private ButtonListener
{
protected:

	BooleanPropertyComponent (const String& propertyName,
							  const String& buttonTextWhenTrue,
							  const String& buttonTextWhenFalse);

public:
	BooleanPropertyComponent (const Value& valueToControl,
							  const String& propertyName,
							  const String& buttonText);

	~BooleanPropertyComponent();

	virtual void setState (bool newState);

	virtual bool getState() const;

	void paint (Graphics& g);
	void refresh();
	void buttonClicked (Button*);

	juce_UseDebuggingNewOperator

private:
	ToggleButton* button;
	String onText, offText;

	void createButton();

	BooleanPropertyComponent (const BooleanPropertyComponent&);
	BooleanPropertyComponent& operator= (const BooleanPropertyComponent&);
};

#endif   // __JUCE_BOOLEANPROPERTYCOMPONENT_JUCEHEADER__
/*** End of inlined file: juce_BooleanPropertyComponent.h ***/


#endif
#ifndef __JUCE_BUTTONPROPERTYCOMPONENT_JUCEHEADER__

/*** Start of inlined file: juce_ButtonPropertyComponent.h ***/
#ifndef __JUCE_BUTTONPROPERTYCOMPONENT_JUCEHEADER__
#define __JUCE_BUTTONPROPERTYCOMPONENT_JUCEHEADER__

class JUCE_API  ButtonPropertyComponent  : public PropertyComponent,
										   private ButtonListener
{
public:

	ButtonPropertyComponent (const String& propertyName,
							 bool triggerOnMouseDown);

	~ButtonPropertyComponent();

	virtual void buttonClicked() = 0;

	virtual const String getButtonText() const = 0;

	void refresh();
	void buttonClicked (Button*);

	juce_UseDebuggingNewOperator

private:
	TextButton* button;

	ButtonPropertyComponent (const ButtonPropertyComponent&);
	ButtonPropertyComponent& operator= (const ButtonPropertyComponent&);
};

#endif   // __JUCE_BUTTONPROPERTYCOMPONENT_JUCEHEADER__
/*** End of inlined file: juce_ButtonPropertyComponent.h ***/


#endif
#ifndef __JUCE_CHOICEPROPERTYCOMPONENT_JUCEHEADER__

/*** Start of inlined file: juce_ChoicePropertyComponent.h ***/
#ifndef __JUCE_CHOICEPROPERTYCOMPONENT_JUCEHEADER__
#define __JUCE_CHOICEPROPERTYCOMPONENT_JUCEHEADER__

class JUCE_API  ChoicePropertyComponent	: public PropertyComponent,
											 private ComboBoxListener
{
protected:
	ChoicePropertyComponent (const String& propertyName);

public:
	ChoicePropertyComponent (const Value& valueToControl,
							 const String& propertyName,
							 const StringArray& choices,
							 const Array <int>* choiceIDs = 0);

	~ChoicePropertyComponent();

	virtual void setIndex (int newIndex);

	virtual int getIndex() const;

	const StringArray& getChoices() const;

	void refresh();
	void comboBoxChanged (ComboBox*);

	juce_UseDebuggingNewOperator

protected:
	StringArray choices;

private:
	ComboBox* comboBox;

	void createComboBox (const Array <int>* choiceIDs);

	ChoicePropertyComponent (const ChoicePropertyComponent&);
	ChoicePropertyComponent& operator= (const ChoicePropertyComponent&);
};

#endif   // __JUCE_CHOICEPROPERTYCOMPONENT_JUCEHEADER__
/*** End of inlined file: juce_ChoicePropertyComponent.h ***/


#endif
#ifndef __JUCE_PROPERTYCOMPONENT_JUCEHEADER__

#endif
#ifndef __JUCE_PROPERTYPANEL_JUCEHEADER__

#endif
#ifndef __JUCE_SLIDERPROPERTYCOMPONENT_JUCEHEADER__

/*** Start of inlined file: juce_SliderPropertyComponent.h ***/
#ifndef __JUCE_SLIDERPROPERTYCOMPONENT_JUCEHEADER__
#define __JUCE_SLIDERPROPERTYCOMPONENT_JUCEHEADER__

class JUCE_API  SliderPropertyComponent   : public PropertyComponent,
											private SliderListener
{
protected:

	SliderPropertyComponent (const String& propertyName,
							 double rangeMin,
							 double rangeMax,
							 double interval,
							 double skewFactor = 1.0);

public:

	SliderPropertyComponent (Value& valueToControl,
							 const String& propertyName,
							 double rangeMin,
							 double rangeMax,
							 double interval,
							 double skewFactor = 1.0);

	~SliderPropertyComponent();

	virtual void setValue (double newValue);

	virtual double getValue() const;

	void refresh();
	void changeListenerCallback (void*);
	void sliderValueChanged (Slider*);

	juce_UseDebuggingNewOperator

protected:

	Slider* slider;

	SliderPropertyComponent (const SliderPropertyComponent&);
	SliderPropertyComponent& operator= (const SliderPropertyComponent&);
};

#endif   // __JUCE_SLIDERPROPERTYCOMPONENT_JUCEHEADER__
/*** End of inlined file: juce_SliderPropertyComponent.h ***/


#endif
#ifndef __JUCE_TEXTPROPERTYCOMPONENT_JUCEHEADER__

/*** Start of inlined file: juce_TextPropertyComponent.h ***/
#ifndef __JUCE_TEXTPROPERTYCOMPONENT_JUCEHEADER__
#define __JUCE_TEXTPROPERTYCOMPONENT_JUCEHEADER__

class JUCE_API  TextPropertyComponent  : public PropertyComponent
{
protected:

	TextPropertyComponent (const String& propertyName,
						   int maxNumChars,
						   bool isMultiLine);

public:
	TextPropertyComponent (const Value& valueToControl,
						   const String& propertyName,
						   int maxNumChars,
						   bool isMultiLine);

	~TextPropertyComponent();

	virtual void setText (const String& newText);

	virtual const String getText() const;

	void refresh();
	void textWasEdited();

	juce_UseDebuggingNewOperator

private:
	Label* textEditor;

	void createEditor (int maxNumChars, bool isMultiLine);

	TextPropertyComponent (const TextPropertyComponent&);
	TextPropertyComponent& operator= (const TextPropertyComponent&);
};

#endif   // __JUCE_TEXTPROPERTYCOMPONENT_JUCEHEADER__
/*** End of inlined file: juce_TextPropertyComponent.h ***/


#endif
#ifndef __JUCE_ACTIVEXCONTROLCOMPONENT_JUCEHEADER__

/*** Start of inlined file: juce_ActiveXControlComponent.h ***/
#ifndef __JUCE_ACTIVEXCONTROLCOMPONENT_JUCEHEADER__
#define __JUCE_ACTIVEXCONTROLCOMPONENT_JUCEHEADER__

#if JUCE_WINDOWS || DOXYGEN

class JUCE_API  ActiveXControlComponent   : public Component
{
public:

	ActiveXControlComponent();

	~ActiveXControlComponent();

	bool createControl (const void* controlIID);

	void deleteControl();

	bool isControlOpen() const throw()		  { return control != 0; }

	void* queryInterface (const void* iid) const;

	void setMouseEventsAllowed (bool eventsCanReachControl);

	bool areMouseEventsAllowed() const throw()		  { return mouseEventsAllowed; }

	void paint (Graphics& g);
	void* originalWndProc;

	juce_UseDebuggingNewOperator

private:
	class ActiveXControlData;
	friend class ActiveXControlData;
	void* control;
	bool mouseEventsAllowed;

	ActiveXControlComponent (const ActiveXControlComponent&);
	ActiveXControlComponent& operator= (const ActiveXControlComponent&);

	void setControlBounds (const Rectangle<int>& bounds) const;
	void setControlVisible (bool b) const;
};

#endif

#endif   // __JUCE_ACTIVEXCONTROLCOMPONENT_JUCEHEADER__
/*** End of inlined file: juce_ActiveXControlComponent.h ***/


#endif
#ifndef __JUCE_AUDIODEVICESELECTORCOMPONENT_JUCEHEADER__

/*** Start of inlined file: juce_AudioDeviceSelectorComponent.h ***/
#ifndef __JUCE_AUDIODEVICESELECTORCOMPONENT_JUCEHEADER__
#define __JUCE_AUDIODEVICESELECTORCOMPONENT_JUCEHEADER__

class MidiInputSelectorComponentListBox;

class JUCE_API  AudioDeviceSelectorComponent  : public Component,
												public ComboBoxListener,
												public ButtonListener,
												public ChangeListener
{
public:

	AudioDeviceSelectorComponent (AudioDeviceManager& deviceManager,
								  const int minAudioInputChannels,
								  const int maxAudioInputChannels,
								  const int minAudioOutputChannels,
								  const int maxAudioOutputChannels,
								  const bool showMidiInputOptions,
								  const bool showMidiOutputSelector,
								  const bool showChannelsAsStereoPairs,
								  const bool hideAdvancedOptionsWithButton);

	~AudioDeviceSelectorComponent();

	void resized();
	void comboBoxChanged (ComboBox*);
	void buttonClicked (Button*);
	void changeListenerCallback (void*);
	void childBoundsChanged (Component*);

	juce_UseDebuggingNewOperator

private:
	AudioDeviceManager& deviceManager;
	ComboBox* deviceTypeDropDown;
	Label* deviceTypeDropDownLabel;
	Component* audioDeviceSettingsComp;
	String audioDeviceSettingsCompType;
	const int minOutputChannels, maxOutputChannels, minInputChannels, maxInputChannels;
	const bool showChannelsAsStereoPairs;
	const bool hideAdvancedOptionsWithButton;

	MidiInputSelectorComponentListBox* midiInputsList;
	Label* midiInputsLabel;
	ComboBox* midiOutputSelector;
	Label* midiOutputLabel;

	AudioDeviceSelectorComponent (const AudioDeviceSelectorComponent&);
	AudioDeviceSelectorComponent& operator= (const AudioDeviceSelectorComponent&);
};

#endif   // __JUCE_AUDIODEVICESELECTORCOMPONENT_JUCEHEADER__
/*** End of inlined file: juce_AudioDeviceSelectorComponent.h ***/


#endif
#ifndef __JUCE_BUBBLECOMPONENT_JUCEHEADER__

/*** Start of inlined file: juce_BubbleComponent.h ***/
#ifndef __JUCE_BUBBLECOMPONENT_JUCEHEADER__
#define __JUCE_BUBBLECOMPONENT_JUCEHEADER__

class JUCE_API  BubbleComponent  : public Component
{
protected:

	BubbleComponent();

public:
	~BubbleComponent();

	enum BubblePlacement
	{
		above   = 1,
		below   = 2,
		left	= 4,
		right   = 8
	};

	void setAllowedPlacement (int newPlacement);

	void setPosition (Component* componentToPointTo);

	void setPosition (int arrowTipX,
					  int arrowTipY);

	void setPosition (const Rectangle<int>& rectangleToPointTo);

protected:

	virtual void getContentSize (int& width, int& height) = 0;

	virtual void paintContent (Graphics& g, int width, int height) = 0;

public:

	void paint (Graphics& g);

	juce_UseDebuggingNewOperator

private:
	Rectangle<int> content;
	int side, allowablePlacements;
	float arrowTipX, arrowTipY;
	DropShadowEffect shadow;

	BubbleComponent (const BubbleComponent&);
	BubbleComponent& operator= (const BubbleComponent&);
};

#endif   // __JUCE_BUBBLECOMPONENT_JUCEHEADER__
/*** End of inlined file: juce_BubbleComponent.h ***/


#endif
#ifndef __JUCE_BUBBLEMESSAGECOMPONENT_JUCEHEADER__

/*** Start of inlined file: juce_BubbleMessageComponent.h ***/
#ifndef __JUCE_BUBBLEMESSAGECOMPONENT_JUCEHEADER__
#define __JUCE_BUBBLEMESSAGECOMPONENT_JUCEHEADER__

class JUCE_API  BubbleMessageComponent  : public BubbleComponent,
										  private Timer
{
public:

	BubbleMessageComponent (int fadeOutLengthMs = 150);

	~BubbleMessageComponent();

	void showAt (int x, int y,
				 const String& message,
				 int numMillisecondsBeforeRemoving,
				 bool removeWhenMouseClicked = true,
				 bool deleteSelfAfterUse = false);

	void showAt (Component* component,
				 const String& message,
				 int numMillisecondsBeforeRemoving,
				 bool removeWhenMouseClicked = true,
				 bool deleteSelfAfterUse = false);

	void getContentSize (int& w, int& h);
	void paintContent (Graphics& g, int w, int h);
	void timerCallback();

	juce_UseDebuggingNewOperator

private:
	int fadeOutLength, mouseClickCounter;
	TextLayout textLayout;
	int64 expiryTime;
	bool deleteAfterUse;

	void init (int numMillisecondsBeforeRemoving,
			   bool removeWhenMouseClicked,
			   bool deleteSelfAfterUse);

	BubbleMessageComponent (const BubbleMessageComponent&);
	BubbleMessageComponent& operator= (const BubbleMessageComponent&);
};

#endif   // __JUCE_BUBBLEMESSAGECOMPONENT_JUCEHEADER__
/*** End of inlined file: juce_BubbleMessageComponent.h ***/


#endif
#ifndef __JUCE_COLOURSELECTOR_JUCEHEADER__

/*** Start of inlined file: juce_ColourSelector.h ***/
#ifndef __JUCE_COLOURSELECTOR_JUCEHEADER__
#define __JUCE_COLOURSELECTOR_JUCEHEADER__

class JUCE_API  ColourSelector  : public Component,
								  public ChangeBroadcaster,
								  protected SliderListener
{
public:

	enum ColourSelectorOptions
	{
		showAlphaChannel	= 1 << 0,   /**< if set, the colour's alpha channel can be changed as well as its RGB. */

		showColourAtTop	 = 1 << 1,   /**< if set, a swatch of the colour is shown at the top of the component. */
		showSliders	 = 1 << 2,   /**< if set, RGB sliders are shown at the bottom of the component. */
		showColourspace	 = 1 << 3	/**< if set, a big HSV selector is shown. */
	};

	ColourSelector (int sectionsToShow = (showAlphaChannel | showColourAtTop | showSliders | showColourspace),
					int edgeGap = 4,
					int gapAroundColourSpaceComponent = 7);

	~ColourSelector();

	const Colour getCurrentColour() const;

	void setCurrentColour (const Colour& newColour);

	virtual int getNumSwatches() const;

	virtual const Colour getSwatchColour (int index) const;

	virtual void setSwatchColour (int index, const Colour& newColour) const;

	enum ColourIds
	{
		backgroundColourId		  = 0x1007000,	/**< the colour used to fill the component's background. */
		labelTextColourId		   = 0x1007001	 /**< the colour used for the labels next to the sliders. */
	};

	juce_UseDebuggingNewOperator

private:
	friend class ColourSpaceView;
	friend class HueSelectorComp;
	Colour colour;
	float h, s, v;
	Slider* sliders[4];
	Component* colourSpace;
	Component* hueSelector;
	class SwatchComponent;
	OwnedArray <SwatchComponent> swatchComponents;
	const int flags;
	int topSpace, edgeGap;

	void setHue (float newH);
	void setSV (float newS, float newV);
	void updateHSV();
	void update();
	void sliderValueChanged (Slider*);
	void paint (Graphics& g);
	void resized();

	ColourSelector (const ColourSelector&);
	ColourSelector& operator= (const ColourSelector&);

	// this constructor is here temporarily to prevent old code compiling, because the parameters
	// have changed - if you get an error here, update your code to use the new constructor instead..
	// (xxx - note to self: remember to remove this at some point in the future)
	ColourSelector (bool);
};

#endif   // __JUCE_COLOURSELECTOR_JUCEHEADER__
/*** End of inlined file: juce_ColourSelector.h ***/


#endif
#ifndef __JUCE_DROPSHADOWER_JUCEHEADER__

#endif
#ifndef __JUCE_MAGNIFIERCOMPONENT_JUCEHEADER__

/*** Start of inlined file: juce_MagnifierComponent.h ***/
#ifndef __JUCE_MAGNIFIERCOMPONENT_JUCEHEADER__
#define __JUCE_MAGNIFIERCOMPONENT_JUCEHEADER__

class JUCE_API  MagnifierComponent	: public Component
{
public:

	MagnifierComponent (Component* contentComponent,
						bool deleteContentCompWhenNoLongerNeeded);

	~MagnifierComponent();

	Component* getContentComponent() const		  { return content; }

	void setScaleFactor (double newScaleFactor);

	double getScaleFactor() const			   { return scaleFactor; }

	void setResamplingQuality (Graphics::ResamplingQuality newQuality);

	juce_UseDebuggingNewOperator

	void childBoundsChanged (Component*);

private:
	Component* content;
	Component* holderComp;
	double scaleFactor;
	ComponentPeer* peer;
	bool deleteContent;
	Graphics::ResamplingQuality quality;
	MouseInputSource mouseSource;

	void paint (Graphics& g);
	void mouseDown (const MouseEvent& e);
	void mouseUp (const MouseEvent& e);
	void mouseDrag (const MouseEvent& e);
	void mouseMove (const MouseEvent& e);
	void mouseEnter (const MouseEvent& e);
	void mouseExit (const MouseEvent& e);
	void mouseWheelMove (const MouseEvent& e, float, float);

	void passOnMouseEventToPeer (const MouseEvent& e);
	int scaleInt (int n) const;

	MagnifierComponent (const MagnifierComponent&);
	MagnifierComponent& operator= (const MagnifierComponent&);
};

#endif   // __JUCE_MAGNIFIERCOMPONENT_JUCEHEADER__
/*** End of inlined file: juce_MagnifierComponent.h ***/


#endif
#ifndef __JUCE_MIDIKEYBOARDCOMPONENT_JUCEHEADER__

/*** Start of inlined file: juce_MidiKeyboardComponent.h ***/
#ifndef __JUCE_MIDIKEYBOARDCOMPONENT_JUCEHEADER__
#define __JUCE_MIDIKEYBOARDCOMPONENT_JUCEHEADER__

class JUCE_API  MidiKeyboardComponent  : public Component,
										 public MidiKeyboardStateListener,
										 public ChangeBroadcaster,
										 private Timer,
										 private AsyncUpdater
{
public:

	enum Orientation
	{
		horizontalKeyboard,
		verticalKeyboardFacingLeft,
		verticalKeyboardFacingRight,
	};

	MidiKeyboardComponent (MidiKeyboardState& state,
						   Orientation orientation);

	~MidiKeyboardComponent();

	void setVelocity (float velocity, bool useMousePositionForVelocity);

	void setMidiChannel (int midiChannelNumber);

	int getMidiChannel() const throw()				  { return midiChannel; }

	void setMidiChannelsToDisplay (int midiChannelMask);

	int getMidiChannelsToDisplay() const throw()			{ return midiInChannelMask; }

	void setKeyWidth (float widthInPixels);

	float getKeyWidth() const throw()				   { return keyWidth; }

	void setOrientation (Orientation newOrientation);

	const Orientation getOrientation() const throw()		{ return orientation; }

	void setAvailableRange (int lowestNote,
							int highestNote);

	int getRangeStart() const throw()				   { return rangeStart; }

	int getRangeEnd() const throw()				 { return rangeEnd; }

	void setLowestVisibleKey (int noteNumber);

	int getLowestVisibleKey() const throw()			 { return firstKey; }

	int getBlackNoteLength() const throw()			  { return blackNoteLength; }

	void setScrollButtonsVisible (bool canScroll);

	enum ColourIds
	{
		whiteNoteColourId		   = 0x1005000,
		blackNoteColourId		   = 0x1005001,
		keySeparatorLineColourId	= 0x1005002,
		mouseOverKeyOverlayColourId	 = 0x1005003,  /**< This colour will be overlaid on the normal note colour. */
		keyDownOverlayColourId	  = 0x1005004,  /**< This colour will be overlaid on the normal note colour. */
		textLabelColourId		   = 0x1005005,
		upDownButtonBackgroundColourId  = 0x1005006,
		upDownButtonArrowColourId	   = 0x1005007
	};

	int getKeyStartPosition (const int midiNoteNumber) const;

	void clearKeyMappings();

	void setKeyPressForNote (const KeyPress& key,
							 int midiNoteOffsetFromC);

	void removeKeyPressForNote (int midiNoteOffsetFromC);

	void setKeyPressBaseOctave (int newOctaveNumber);

	void setOctaveForMiddleC (int octaveNumForMiddleC) throw();

	int getOctaveForMiddleC() const throw()		 { return octaveNumForMiddleC; }

	void paint (Graphics& g);
	void resized();
	void mouseMove (const MouseEvent& e);
	void mouseDrag (const MouseEvent& e);
	void mouseDown (const MouseEvent& e);
	void mouseUp (const MouseEvent& e);
	void mouseEnter (const MouseEvent& e);
	void mouseExit (const MouseEvent& e);
	void mouseWheelMove (const MouseEvent& e, float wheelIncrementX, float wheelIncrementY);
	void timerCallback();
	bool keyStateChanged (bool isKeyDown);
	void focusLost (FocusChangeType cause);
	void handleNoteOn (MidiKeyboardState* source, int midiChannel, int midiNoteNumber, float velocity);
	void handleNoteOff (MidiKeyboardState* source, int midiChannel, int midiNoteNumber);
	void handleAsyncUpdate();
	void colourChanged();

	juce_UseDebuggingNewOperator

protected:
	friend class MidiKeyboardUpDownButton;

	virtual void drawWhiteNote (int midiNoteNumber,
								Graphics& g,
								int x, int y, int w, int h,
								bool isDown, bool isOver,
								const Colour& lineColour,
								const Colour& textColour);

	virtual void drawBlackNote (int midiNoteNumber,
								Graphics& g,
								int x, int y, int w, int h,
								bool isDown, bool isOver,
								const Colour& noteFillColour);

	virtual const String getWhiteNoteText (const int midiNoteNumber);

	virtual void drawUpDownButton (Graphics& g, int w, int h,
								   const bool isMouseOver,
								   const bool isButtonPressed,
								   const bool movesOctavesUp);

	virtual bool mouseDownOnKey (int midiNoteNumber, const MouseEvent& e);

	virtual void mouseDraggedToKey (int midiNoteNumber, const MouseEvent& e);

	virtual void getKeyPosition (int midiNoteNumber, float keyWidth,
								 int& x, int& w) const;

private:

	MidiKeyboardState& state;
	int xOffset, blackNoteLength;
	float keyWidth;
	Orientation orientation;

	int midiChannel, midiInChannelMask;
	float velocity;
	int noteUnderMouse, mouseDownNote;
	BigInteger keysPressed, keysCurrentlyDrawnDown;

	int rangeStart, rangeEnd, firstKey;
	bool canScroll, mouseDragging, useMousePositionForVelocity;
	Button* scrollDown;
	Button* scrollUp;

	Array <KeyPress> keyPresses;
	Array <int> keyPressNotes;
	int keyMappingOctave;
	int octaveNumForMiddleC;

	void getKeyPos (int midiNoteNumber, int& x, int& w) const;
	int xyToNote (const Point<int>& pos, float& mousePositionVelocity);
	int remappedXYToNote (const Point<int>& pos, float& mousePositionVelocity) const;
	void resetAnyKeysInUse();
	void updateNoteUnderMouse (const Point<int>& pos);
	void repaintNote (const int midiNoteNumber);

	MidiKeyboardComponent (const MidiKeyboardComponent&);
	MidiKeyboardComponent& operator= (const MidiKeyboardComponent&);
};

#endif   // __JUCE_MIDIKEYBOARDCOMPONENT_JUCEHEADER__
/*** End of inlined file: juce_MidiKeyboardComponent.h ***/


#endif
#ifndef __JUCE_NSVIEWCOMPONENT_JUCEHEADER__

/*** Start of inlined file: juce_NSViewComponent.h ***/
#ifndef __JUCE_NSVIEWCOMPONENT_JUCEHEADER__
#define __JUCE_NSVIEWCOMPONENT_JUCEHEADER__

#if ! DOXYGEN
 class NSViewComponentInternal;
#endif

#if JUCE_MAC || DOXYGEN

class JUCE_API  NSViewComponent   : public Component
{
public:

	NSViewComponent();

	~NSViewComponent();

	void setView (void* nsView);

	void* getView() const;

	void paint (Graphics& g);

	juce_UseDebuggingNewOperator

private:
	friend class NSViewComponentInternal;
	ScopedPointer <NSViewComponentInternal> info;

	NSViewComponent (const NSViewComponent&);
	NSViewComponent& operator= (const NSViewComponent&);
};

#endif

#endif   // __JUCE_NSVIEWCOMPONENT_JUCEHEADER__
/*** End of inlined file: juce_NSViewComponent.h ***/


#endif
#ifndef __JUCE_OPENGLCOMPONENT_JUCEHEADER__

/*** Start of inlined file: juce_OpenGLComponent.h ***/
#ifndef __JUCE_OPENGLCOMPONENT_JUCEHEADER__
#define __JUCE_OPENGLCOMPONENT_JUCEHEADER__

// this is used to disable OpenGL, and is defined in juce_Config.h
#if JUCE_OPENGL || DOXYGEN

class JUCE_API  OpenGLPixelFormat
{
public:

	OpenGLPixelFormat (int bitsPerRGBComponent = 8,
					   int alphaBits = 8,
					   int depthBufferBits = 16,
					   int stencilBufferBits = 0);

	OpenGLPixelFormat (const OpenGLPixelFormat&);
	OpenGLPixelFormat& operator= (const OpenGLPixelFormat&);
	bool operator== (const OpenGLPixelFormat&) const;

	int redBits;	  /**< The number of bits per pixel to use for the red channel. */
	int greenBits;	/**< The number of bits per pixel to use for the green channel. */
	int blueBits;	 /**< The number of bits per pixel to use for the blue channel. */
	int alphaBits;	/**< The number of bits per pixel to use for the alpha channel. */

	int depthBufferBits;	  /**< The number of bits per pixel to use for a depth buffer. */
	int stencilBufferBits;	/**< The number of bits per pixel to use for a stencil buffer. */

	int accumulationBufferRedBits;	/**< The number of bits per pixel to use for an accumulation buffer's red channel. */
	int accumulationBufferGreenBits;  /**< The number of bits per pixel to use for an accumulation buffer's green channel. */
	int accumulationBufferBlueBits;   /**< The number of bits per pixel to use for an accumulation buffer's blue channel. */
	int accumulationBufferAlphaBits;  /**< The number of bits per pixel to use for an accumulation buffer's alpha channel. */

	uint8 fullSceneAntiAliasingNumSamples;	  /**< The number of samples to use in full-scene anti-aliasing (if available). */

	static void getAvailablePixelFormats (Component* component,
										  OwnedArray <OpenGLPixelFormat>& results);

	juce_UseDebuggingNewOperator
};

class JUCE_API  OpenGLContext
{
public:

	virtual ~OpenGLContext();

	virtual bool makeActive() const throw() = 0;
	virtual bool makeInactive() const throw() = 0;
	virtual bool isActive() const throw() = 0;

	virtual void swapBuffers() = 0;

	virtual bool setSwapInterval (int numFramesPerSwap) = 0;

	virtual int getSwapInterval() const = 0;

	virtual const OpenGLPixelFormat getPixelFormat() const = 0;

	virtual void updateWindowPosition (int x, int y, int w, int h, int outerWindowHeight) = 0;

	virtual void repaint() = 0;

	virtual void* getRawContext() const throw() = 0;

	static OpenGLContext* getCurrentContext();

	juce_UseDebuggingNewOperator

protected:
	OpenGLContext() throw();
};

class JUCE_API  OpenGLComponent  : public Component
{
public:

	enum OpenGLType
	{
		openGLDefault = 0,

#if JUCE_IPHONE
		openGLES1,  /**< On the iPhone, this selects openGL ES 1.0 */
		openGLES2   /**< On the iPhone, this selects openGL ES 2.0 */
#endif
	};

	OpenGLComponent (OpenGLType type = openGLDefault);

	~OpenGLComponent();

	void setPixelFormat (const OpenGLPixelFormat& formatToUse);

	const OpenGLPixelFormat getPixelFormat() const;

	void shareWith (OpenGLContext* contextToShareListsWith);

	OpenGLContext* getShareContext() const throw()	{ return contextToShareListsWith; }

	void swapBuffers();

	virtual void renderOpenGL() = 0;

	virtual void newOpenGLContextCreated() = 0;

	OpenGLContext* getCurrentContext() const throw()		{ return context; }

	bool makeCurrentContextActive();

	void makeCurrentContextInactive();

	bool isActiveContext() const throw();

	virtual bool renderAndSwapBuffers();

	CriticalSection& getContextLock() throw()	   { return contextLock; }

	void paint (Graphics& g);

	void* getNativeWindowHandle() const;

	juce_UseDebuggingNewOperator

private:
	const OpenGLType type;

	class OpenGLComponentWatcher;
	friend class OpenGLComponentWatcher;
	friend class ScopedPointer <OpenGLComponentWatcher>;
	ScopedPointer <OpenGLComponentWatcher> componentWatcher;
	ScopedPointer <OpenGLContext> context;
	OpenGLContext* contextToShareListsWith;

	CriticalSection contextLock;
	OpenGLPixelFormat preferredPixelFormat;
	bool needToUpdateViewport;

	OpenGLContext* createContext();
	void deleteContext();
	void updateContextPosition();
	void internalRepaint (int x, int y, int w, int h);

	OpenGLComponent (const OpenGLComponent&);
	OpenGLComponent& operator= (const OpenGLComponent&);
};

#endif
#endif   // __JUCE_OPENGLCOMPONENT_JUCEHEADER__
/*** End of inlined file: juce_OpenGLComponent.h ***/


#endif
#ifndef __JUCE_PREFERENCESPANEL_JUCEHEADER__

/*** Start of inlined file: juce_PreferencesPanel.h ***/
#ifndef __JUCE_PREFERENCESPANEL_JUCEHEADER__
#define __JUCE_PREFERENCESPANEL_JUCEHEADER__

class JUCE_API  PreferencesPanel  : public Component,
									private ButtonListener
{
public:

	PreferencesPanel();

	~PreferencesPanel();

	void addSettingsPage (const String& pageTitle,
						  const Drawable* normalIcon,
						  const Drawable* overIcon,
						  const Drawable* downIcon);

	void addSettingsPage (const String& pageTitle,
						  const char* imageData,
						  int imageDataSize);

	void showInDialogBox (const String& dialogtitle,
						  int dialogWidth,
						  int dialogHeight,
						  const Colour& backgroundColour = Colours::white);

	virtual Component* createComponentForPage (const String& pageName) = 0;

	void setCurrentPage (const String& pageName);

	void resized();
	void paint (Graphics& g);
	void buttonClicked (Button* button);

	juce_UseDebuggingNewOperator

private:

	String currentPageName;
	ScopedPointer <Component> currentPage;
	int buttonSize;

	PreferencesPanel (const PreferencesPanel&);
	PreferencesPanel& operator= (const PreferencesPanel&);
};

#endif   // __JUCE_PREFERENCESPANEL_JUCEHEADER__
/*** End of inlined file: juce_PreferencesPanel.h ***/


#endif
#ifndef __JUCE_QUICKTIMEMOVIECOMPONENT_JUCEHEADER__

/*** Start of inlined file: juce_QuickTimeMovieComponent.h ***/
#ifndef __JUCE_QUICKTIMEMOVIECOMPONENT_JUCEHEADER__
#define __JUCE_QUICKTIMEMOVIECOMPONENT_JUCEHEADER__

// (NB: This stuff mustn't go inside the "#if QUICKTIME" block, or it'll break the
// amalgamated build)
#if JUCE_WINDOWS

  typedef ActiveXControlComponent QTCompBaseClass;
#elif JUCE_MAC

  typedef NSViewComponent QTCompBaseClass;
#endif

// this is used to disable QuickTime, and is defined in juce_Config.h
#if JUCE_QUICKTIME || DOXYGEN

class JUCE_API  QuickTimeMovieComponent	 : public QTCompBaseClass
{
public:

	QuickTimeMovieComponent();

	~QuickTimeMovieComponent();

	static bool isQuickTimeAvailable() throw();

	bool loadMovie (const File& movieFile,
					bool isControllerVisible);

	bool loadMovie (const URL& movieURL,
					bool isControllerVisible);

	bool loadMovie (InputStream* movieStream,
					bool isControllerVisible);

	void closeMovie();

	const File getCurrentMovieFile() const;

	bool isMovieOpen() const;

	double getMovieDuration() const;

	void getMovieNormalSize (int& width, int& height) const;

	void setBoundsWithCorrectAspectRatio (const Rectangle<int>& spaceToFitWithin,
										  const RectanglePlacement& placement);

	void play();

	void stop();

	bool isPlaying() const;

	void goToStart();

	void setPosition (double seconds);

	double getPosition() const;

	void setSpeed (float newSpeed);

	void setMovieVolume (float newVolume);

	float getMovieVolume() const;

	void setLooping (bool shouldLoop);

	bool isLooping() const;

	bool isControllerVisible() const;

	void paint (Graphics& g);

	juce_UseDebuggingNewOperator

private:
	File movieFile;
	bool movieLoaded, controllerVisible, looping;

#if JUCE_WINDOWS
	void parentHierarchyChanged();
	void visibilityChanged();

	void createControlIfNeeded();
	bool isControlCreated() const;

	class Pimpl;
	friend class ScopedPointer <Pimpl>;
	ScopedPointer <Pimpl> pimpl;
#else
	void* movie;
#endif

	QuickTimeMovieComponent (const QuickTimeMovieComponent&);
	QuickTimeMovieComponent& operator= (const QuickTimeMovieComponent&);
};

#endif
#endif   // __JUCE_QUICKTIMEMOVIECOMPONENT_JUCEHEADER__
/*** End of inlined file: juce_QuickTimeMovieComponent.h ***/


#endif
#ifndef __JUCE_SYSTEMTRAYICONCOMPONENT_JUCEHEADER__

/*** Start of inlined file: juce_SystemTrayIconComponent.h ***/
#ifndef __JUCE_SYSTEMTRAYICONCOMPONENT_JUCEHEADER__
#define __JUCE_SYSTEMTRAYICONCOMPONENT_JUCEHEADER__

#if JUCE_WINDOWS || JUCE_LINUX || DOXYGEN

class JUCE_API  SystemTrayIconComponent  : public Component
{
public:

	SystemTrayIconComponent();

	~SystemTrayIconComponent();

	void setIconImage (const Image& newImage);

	void setIconTooltip (const String& tooltip);

#if JUCE_LINUX
	void paint (Graphics& g);
#endif

	juce_UseDebuggingNewOperator

private:

	SystemTrayIconComponent (const SystemTrayIconComponent&);
	SystemTrayIconComponent& operator= (const SystemTrayIconComponent&);
};

#endif
#endif   // __JUCE_SYSTEMTRAYICONCOMPONENT_JUCEHEADER__
/*** End of inlined file: juce_SystemTrayIconComponent.h ***/


#endif
#ifndef __JUCE_WEBBROWSERCOMPONENT_JUCEHEADER__

/*** Start of inlined file: juce_WebBrowserComponent.h ***/
#ifndef __JUCE_WEBBROWSERCOMPONENT_JUCEHEADER__
#define __JUCE_WEBBROWSERCOMPONENT_JUCEHEADER__

#if JUCE_WEB_BROWSER || DOXYGEN

#if ! DOXYGEN
 class WebBrowserComponentInternal;
#endif

class JUCE_API  WebBrowserComponent	  : public Component
{
public:

	explicit WebBrowserComponent (bool unloadPageWhenBrowserIsHidden = true);

	~WebBrowserComponent();

	void goToURL (const String& url,
				  const StringArray* headers = 0,
				  const MemoryBlock* postData = 0);

	void stop();

	void goBack();

	void goForward();

	void refresh();

	virtual bool pageAboutToLoad (const String& newURL);

	void paint (Graphics& g);
	void resized();
	void parentHierarchyChanged();
	void visibilityChanged();

	juce_UseDebuggingNewOperator

private:
	WebBrowserComponentInternal* browser;
	bool blankPageShown, unloadPageWhenBrowserIsHidden;
	String lastURL;
	StringArray lastHeaders;
	MemoryBlock lastPostData;

	void reloadLastURL();
	void checkWindowAssociation();

	WebBrowserComponent (const WebBrowserComponent&);
	WebBrowserComponent& operator= (const WebBrowserComponent&);
};

#endif
#endif   // __JUCE_WEBBROWSERCOMPONENT_JUCEHEADER__
/*** End of inlined file: juce_WebBrowserComponent.h ***/


#endif
#ifndef __JUCE_ALERTWINDOW_JUCEHEADER__

#endif
#ifndef __JUCE_COMPONENTPEER_JUCEHEADER__

/*** Start of inlined file: juce_ComponentPeer.h ***/
#ifndef __JUCE_COMPONENTPEER_JUCEHEADER__
#define __JUCE_COMPONENTPEER_JUCEHEADER__

class ComponentBoundsConstrainer;

class JUCE_API  ComponentPeer
{
public:

	enum StyleFlags
	{
		windowAppearsOnTaskbar	  = (1 << 0),	/**< Indicates that the window should have a corresponding
														entry on the taskbar (ignored on MacOSX) */
		windowIsTemporary	   = (1 << 1),	/**< Indicates that the window is a temporary popup, like a menu,
														tooltip, etc. */
		windowIgnoresMouseClicks	= (1 << 2),	/**< Indicates that the window should let mouse clicks pass
														through it (may not be possible on some platforms). */
		windowHasTitleBar	   = (1 << 3),	/**< Indicates that the window should have a normal OS-specific
														title bar and frame\. if not specified, the window will be
														borderless. */
		windowIsResizable	   = (1 << 4),	/**< Indicates that the window should have a resizable border. */
		windowHasMinimiseButton	 = (1 << 5),	/**< Indicates that if the window has a title bar, it should have a
														minimise button on it. */
		windowHasMaximiseButton	 = (1 << 6),	/**< Indicates that if the window has a title bar, it should have a
														maximise button on it. */
		windowHasCloseButton	= (1 << 7),	/**< Indicates that if the window has a title bar, it should have a
														close button on it. */
		windowHasDropShadow	 = (1 << 8),	/**< Indicates that the window should have a drop-shadow (this may
														not be possible on all platforms). */
		windowRepaintedExplictly	= (1 << 9),	/**< Not intended for public use - this tells a window not to
														do its own repainting, but only to repaint when the
														performAnyPendingRepaintsNow() method is called. */
		windowIgnoresKeyPresses	 = (1 << 10),   /**< Tells the window not to catch any keypresses. This can
														be used for things like plugin windows, to stop them interfering
														with the host's shortcut keys */
		windowIsSemiTransparent	 = (1 << 31)	/**< Not intended for public use - makes a window transparent. */

	};

	ComponentPeer (Component* component,
				   int styleFlags) throw();

	virtual ~ComponentPeer();

	Component* getComponent() const throw()		 { return component; }

	int getStyleFlags() const throw()			   { return styleFlags; }

	virtual void* getNativeHandle() const = 0;

	virtual void setVisible (bool shouldBeVisible) = 0;

	virtual void setTitle (const String& title) = 0;

	virtual void setPosition (int x, int y) = 0;

	virtual void setSize (int w, int h) = 0;

	virtual void setBounds (int x, int y, int w, int h, bool isNowFullScreen) = 0;

	virtual const Rectangle<int> getBounds() const = 0;

	virtual const Point<int> getScreenPosition() const = 0;

	virtual const Point<int> relativePositionToGlobal (const Point<int>& relativePosition) = 0;

	virtual const Point<int> globalPositionToRelative (const Point<int>& screenPosition) = 0;

	virtual void setMinimised (bool shouldBeMinimised) = 0;

	virtual bool isMinimised() const = 0;

	virtual void setFullScreen (bool shouldBeFullScreen) = 0;

	virtual bool isFullScreen() const = 0;

	void setNonFullScreenBounds (const Rectangle<int>& newBounds) throw();

	const Rectangle<int>& getNonFullScreenBounds() const throw();

	virtual void setIcon (const Image& newIcon) = 0;

	void setConstrainer (ComponentBoundsConstrainer* newConstrainer) throw();

	ComponentBoundsConstrainer* getConstrainer() const throw()		  { return constrainer; }

	virtual bool contains (const Point<int>& position, bool trueIfInAChildWindow) const = 0;

	virtual const BorderSize getFrameSize() const = 0;

	void handleMovedOrResized();

	void handleScreenSizeChange();

	void handlePaint (LowLevelGraphicsContext& contextToPaintTo);

	virtual bool setAlwaysOnTop (bool alwaysOnTop) = 0;

	virtual void toFront (bool makeActive) = 0;

	virtual void toBehind (ComponentPeer* other) = 0;

	void handleBroughtToFront();

	virtual bool isFocused() const = 0;

	virtual void grabFocus() = 0;

	virtual void textInputRequired (const Point<int>& position) = 0;

	void handleFocusGain();
	void handleFocusLoss();

	Component* getLastFocusedSubcomponent() const throw();

	bool handleKeyPress (int keyCode,
						 juce_wchar textCharacter);

	bool handleKeyUpOrDown (bool isKeyDown);

	void handleModifierKeysChange();

	TextInputTarget* findCurrentTextInputTarget();

	virtual void repaint (int x, int y, int w, int h) = 0;

	virtual void performAnyPendingRepaintsNow() = 0;

	void handleMouseEvent (int touchIndex, const Point<int>& positionWithinPeer, const ModifierKeys& newMods, int64 time);
	void handleMouseWheel (int touchIndex, const Point<int>& positionWithinPeer, int64 time, float x, float y);

	void handleUserClosingWindow();

	void handleFileDragMove (const StringArray& files, const Point<int>& position);
	void handleFileDragExit (const StringArray& files);
	void handleFileDragDrop (const StringArray& files, const Point<int>& position);

	void clearMaskedRegion() throw();

	void addMaskedRegion (int x, int y, int w, int h) throw();

	static int getNumPeers() throw();

	static ComponentPeer* getPeer (int index) throw();

	static bool isValidPeer (const ComponentPeer* peer) throw();

	static void bringModalComponentToFront();

	virtual const StringArray getAvailableRenderingEngines() throw();
	virtual int getCurrentRenderingEngine() throw();
	virtual void setCurrentRenderingEngine (int index) throw();

	juce_UseDebuggingNewOperator

protected:
	Component* const component;
	const int styleFlags;
	RectangleList maskedRegion;
	Rectangle<int> lastNonFullscreenBounds;
	uint32 lastPaintTime;
	ComponentBoundsConstrainer* constrainer;

	static void updateCurrentModifiers() throw();

private:

	Component::SafePointer<Component> lastFocusedComponent, dragAndDropTargetComponent;
	Component* lastDragAndDropCompUnderMouse;
	bool fakeMouseMessageSent : 1, isWindowMinimised : 1;

	friend class Component;
	static ComponentPeer* getPeerFor (const Component* component) throw();

	void setLastDragDropTarget (Component* comp);

	ComponentPeer (const ComponentPeer&);
	ComponentPeer& operator= (const ComponentPeer&);
};

#endif   // __JUCE_COMPONENTPEER_JUCEHEADER__
/*** End of inlined file: juce_ComponentPeer.h ***/


#endif
#ifndef __JUCE_DIALOGWINDOW_JUCEHEADER__

/*** Start of inlined file: juce_DialogWindow.h ***/
#ifndef __JUCE_DIALOGWINDOW_JUCEHEADER__
#define __JUCE_DIALOGWINDOW_JUCEHEADER__

class JUCE_API  DialogWindow   : public DocumentWindow
{
public:

	DialogWindow (const String& name,
				  const Colour& backgroundColour,
				  bool escapeKeyTriggersCloseButton,
				  bool addToDesktop = true);

	~DialogWindow();

	static int showModalDialog (const String& dialogTitle,
								Component* contentComponent,
								Component* componentToCentreAround,
								const Colour& backgroundColour,
								bool escapeKeyTriggersCloseButton,
								bool shouldBeResizable = false,
								bool useBottomRightCornerResizer = false);

	juce_UseDebuggingNewOperator

protected:
	void resized();

private:
	bool escapeKeyTriggersCloseButton;

	DialogWindow (const DialogWindow&);
	DialogWindow& operator= (const DialogWindow&);
};

#endif   // __JUCE_DIALOGWINDOW_JUCEHEADER__
/*** End of inlined file: juce_DialogWindow.h ***/


#endif
#ifndef __JUCE_DOCUMENTWINDOW_JUCEHEADER__

#endif
#ifndef __JUCE_RESIZABLEWINDOW_JUCEHEADER__

#endif
#ifndef __JUCE_SPLASHSCREEN_JUCEHEADER__

/*** Start of inlined file: juce_SplashScreen.h ***/
#ifndef __JUCE_SPLASHSCREEN_JUCEHEADER__
#define __JUCE_SPLASHSCREEN_JUCEHEADER__

class JUCE_API  SplashScreen  : public Component,
								public Timer,
								private DeletedAtShutdown
{
public:

	SplashScreen();

	~SplashScreen();

	void show (const String& title,
			   Image* backgroundImage,
			   int minimumTimeToDisplayFor,
			   bool useDropShadow,
			   bool removeOnMouseClick = true);

	void show (const String& title,
			   int width,
			   int height,
			   int minimumTimeToDisplayFor,
			   bool useDropShadow,
			   bool removeOnMouseClick = true);

	void paint (Graphics& g);
	void timerCallback();

	juce_UseDebuggingNewOperator

private:
	Image* backgroundImage;
	Time earliestTimeToDelete;
	int originalClickCounter;

	SplashScreen (const SplashScreen&);
	SplashScreen& operator= (const SplashScreen&);
};

#endif   // __JUCE_SPLASHSCREEN_JUCEHEADER__
/*** End of inlined file: juce_SplashScreen.h ***/


#endif
#ifndef __JUCE_THREADWITHPROGRESSWINDOW_JUCEHEADER__

/*** Start of inlined file: juce_ThreadWithProgressWindow.h ***/
#ifndef __JUCE_THREADWITHPROGRESSWINDOW_JUCEHEADER__
#define __JUCE_THREADWITHPROGRESSWINDOW_JUCEHEADER__

class JUCE_API  ThreadWithProgressWindow  : public Thread,
											private Timer
{
public:

	ThreadWithProgressWindow (const String& windowTitle,
							  bool hasProgressBar,
							  bool hasCancelButton,
							  int timeOutMsWhenCancelling = 10000,
							  const String& cancelButtonText = "Cancel");

	~ThreadWithProgressWindow();

	bool runThread (int threadPriority = 5);

	void setProgress (double newProgress);

	void setStatusMessage (const String& newStatusMessage);

	AlertWindow* getAlertWindow() const throw()	 { return alertWindow; }

	juce_UseDebuggingNewOperator

private:
	void timerCallback();

	double progress;
	ScopedPointer <AlertWindow> alertWindow;
	String message;
	CriticalSection messageLock;
	const int timeOutMsWhenCancelling;

	ThreadWithProgressWindow (const ThreadWithProgressWindow&);
	ThreadWithProgressWindow& operator= (const ThreadWithProgressWindow&);
};

#endif   // __JUCE_THREADWITHPROGRESSWINDOW_JUCEHEADER__
/*** End of inlined file: juce_ThreadWithProgressWindow.h ***/


#endif
#ifndef __JUCE_TOOLTIPWINDOW_JUCEHEADER__

#endif
#ifndef __JUCE_TOPLEVELWINDOW_JUCEHEADER__

#endif
#ifndef __JUCE_COLOUR_JUCEHEADER__

#endif
#ifndef __JUCE_COLOURGRADIENT_JUCEHEADER__

#endif
#ifndef __JUCE_COLOURS_JUCEHEADER__

#endif
#ifndef __JUCE_PIXELFORMATS_JUCEHEADER__

#endif
#ifndef __JUCE_EDGETABLE_JUCEHEADER__

#endif
#ifndef __JUCE_FILLTYPE_JUCEHEADER__

#endif
#ifndef __JUCE_GRAPHICS_JUCEHEADER__

#endif
#ifndef __JUCE_JUSTIFICATION_JUCEHEADER__

#endif
#ifndef __JUCE_LOWLEVELGRAPHICSCONTEXT_JUCEHEADER__

/*** Start of inlined file: juce_LowLevelGraphicsContext.h ***/
#ifndef __JUCE_LOWLEVELGRAPHICSCONTEXT_JUCEHEADER__
#define __JUCE_LOWLEVELGRAPHICSCONTEXT_JUCEHEADER__

class JUCE_API  LowLevelGraphicsContext
{
protected:

	LowLevelGraphicsContext();

public:
	virtual ~LowLevelGraphicsContext();

	virtual bool isVectorDevice() const = 0;

	virtual void setOrigin (int x, int y) = 0;

	virtual bool clipToRectangle (const Rectangle<int>& r) = 0;
	virtual bool clipToRectangleList (const RectangleList& clipRegion) = 0;
	virtual void excludeClipRectangle (const Rectangle<int>& r) = 0;
	virtual void clipToPath (const Path& path, const AffineTransform& transform) = 0;
	virtual void clipToImageAlpha (const Image& sourceImage, const Rectangle<int>& srcClip, const AffineTransform& transform) = 0;

	virtual bool clipRegionIntersects (const Rectangle<int>& r) = 0;
	virtual const Rectangle<int> getClipBounds() const = 0;
	virtual bool isClipEmpty() const = 0;

	virtual void saveState() = 0;
	virtual void restoreState() = 0;

	virtual void setFill (const FillType& fillType) = 0;
	virtual void setOpacity (float newOpacity) = 0;
	virtual void setInterpolationQuality (Graphics::ResamplingQuality quality) = 0;

	virtual void fillRect (const Rectangle<int>& r, bool replaceExistingContents) = 0;
	virtual void fillPath (const Path& path, const AffineTransform& transform) = 0;

	virtual void drawImage (const Image& sourceImage, const Rectangle<int>& srcClip,
							const AffineTransform& transform, bool fillEntireClipAsTiles) = 0;

	virtual void drawLine (double x1, double y1, double x2, double y2) = 0;
	virtual void drawVerticalLine (int x, double top, double bottom) = 0;
	virtual void drawHorizontalLine (int y, double left, double right) = 0;

	virtual void setFont (const Font& newFont) = 0;
	virtual const Font getFont() = 0;
	virtual void drawGlyph (int glyphNumber, const AffineTransform& transform) = 0;
};

#endif   // __JUCE_LOWLEVELGRAPHICSCONTEXT_JUCEHEADER__
/*** End of inlined file: juce_LowLevelGraphicsContext.h ***/


#endif
#ifndef __JUCE_LOWLEVELGRAPHICSPOSTSCRIPTRENDERER_JUCEHEADER__

/*** Start of inlined file: juce_LowLevelGraphicsPostScriptRenderer.h ***/
#ifndef __JUCE_LOWLEVELGRAPHICSPOSTSCRIPTRENDERER_JUCEHEADER__
#define __JUCE_LOWLEVELGRAPHICSPOSTSCRIPTRENDERER_JUCEHEADER__

class JUCE_API  LowLevelGraphicsPostScriptRenderer	: public LowLevelGraphicsContext
{
public:

	LowLevelGraphicsPostScriptRenderer (OutputStream& resultingPostScript,
										const String& documentTitle,
										int totalWidth,
										int totalHeight);

	~LowLevelGraphicsPostScriptRenderer();

	bool isVectorDevice() const;
	void setOrigin (int x, int y);

	bool clipToRectangle (const Rectangle<int>& r);
	bool clipToRectangleList (const RectangleList& clipRegion);
	void excludeClipRectangle (const Rectangle<int>& r);
	void clipToPath (const Path& path, const AffineTransform& transform);
	void clipToImageAlpha (const Image& sourceImage, const Rectangle<int>& srcClip, const AffineTransform& transform);

	void saveState();
	void restoreState();

	bool clipRegionIntersects (const Rectangle<int>& r);
	const Rectangle<int> getClipBounds() const;
	bool isClipEmpty() const;

	void setFill (const FillType& fillType);
	void setOpacity (float opacity);
	void setInterpolationQuality (Graphics::ResamplingQuality quality);

	void fillRect (const Rectangle<int>& r, bool replaceExistingContents);
	void fillPath (const Path& path, const AffineTransform& transform);

	void drawImage (const Image& sourceImage, const Rectangle<int>& srcClip,
					const AffineTransform& transform, bool fillEntireClipAsTiles);

	void drawLine (double x1, double y1, double x2, double y2);

	void drawVerticalLine (int x, double top, double bottom);
	void drawHorizontalLine (int x, double top, double bottom);

	const Font getFont();
	void setFont (const Font& newFont);
	void drawGlyph (int glyphNumber, const AffineTransform& transform);

	juce_UseDebuggingNewOperator

protected:

	OutputStream& out;
	int totalWidth, totalHeight;
	bool needToClip;
	Colour lastColour;

	struct SavedState
	{
		SavedState();
		~SavedState();

		RectangleList clip;
		int xOffset, yOffset;
		FillType fillType;
		Font font;

	private:
		SavedState& operator= (const SavedState&);
	};

	OwnedArray <SavedState> stateStack;

	void writeClip();
	void writeColour (const Colour& colour);
	void writePath (const Path& path) const;
	void writeXY (float x, float y) const;
	void writeTransform (const AffineTransform& trans) const;
	void writeImage (const Image& im, int sx, int sy, int maxW, int maxH) const;

	LowLevelGraphicsPostScriptRenderer (const LowLevelGraphicsPostScriptRenderer& other);
	LowLevelGraphicsPostScriptRenderer& operator= (const LowLevelGraphicsPostScriptRenderer&);
};

#endif   // __JUCE_LOWLEVELGRAPHICSPOSTSCRIPTRENDERER_JUCEHEADER__
/*** End of inlined file: juce_LowLevelGraphicsPostScriptRenderer.h ***/


#endif
#ifndef __JUCE_LOWLEVELGRAPHICSSOFTWARERENDERER_JUCEHEADER__

/*** Start of inlined file: juce_LowLevelGraphicsSoftwareRenderer.h ***/
#ifndef __JUCE_LOWLEVELGRAPHICSSOFTWARERENDERER_JUCEHEADER__
#define __JUCE_LOWLEVELGRAPHICSSOFTWARERENDERER_JUCEHEADER__

class LLGCSavedState;

class JUCE_API  LowLevelGraphicsSoftwareRenderer	: public LowLevelGraphicsContext
{
public:

	LowLevelGraphicsSoftwareRenderer (Image& imageToRenderOn);
	~LowLevelGraphicsSoftwareRenderer();

	bool isVectorDevice() const;

	void setOrigin (int x, int y);

	bool clipToRectangle (const Rectangle<int>& r);
	bool clipToRectangleList (const RectangleList& clipRegion);
	void excludeClipRectangle (const Rectangle<int>& r);
	void clipToPath (const Path& path, const AffineTransform& transform);
	void clipToImageAlpha (const Image& sourceImage, const Rectangle<int>& srcClip, const AffineTransform& transform);

	bool clipRegionIntersects (const Rectangle<int>& r);
	const Rectangle<int> getClipBounds() const;
	bool isClipEmpty() const;

	void saveState();
	void restoreState();

	void setFill (const FillType& fillType);
	void setOpacity (float opacity);
	void setInterpolationQuality (Graphics::ResamplingQuality quality);

	void fillRect (const Rectangle<int>& r, bool replaceExistingContents);
	void fillPath (const Path& path, const AffineTransform& transform);

	void drawImage (const Image& sourceImage, const Rectangle<int>& srcClip,
					const AffineTransform& transform, bool fillEntireClipAsTiles);

	void drawLine (double x1, double y1, double x2, double y2);

	void drawVerticalLine (int x, double top, double bottom);
	void drawHorizontalLine (int x, double top, double bottom);

	void setFont (const Font& newFont);
	const Font getFont();
	void drawGlyph (int glyphNumber, float x, float y);
	void drawGlyph (int glyphNumber, const AffineTransform& transform);

	juce_UseDebuggingNewOperator

protected:

	Image& image;

	ScopedPointer <LLGCSavedState> currentState;
	OwnedArray <LLGCSavedState> stateStack;

	LowLevelGraphicsSoftwareRenderer (const LowLevelGraphicsSoftwareRenderer& other);
	LowLevelGraphicsSoftwareRenderer& operator= (const LowLevelGraphicsSoftwareRenderer&);
};

#endif   // __JUCE_LOWLEVELGRAPHICSSOFTWARERENDERER_JUCEHEADER__
/*** End of inlined file: juce_LowLevelGraphicsSoftwareRenderer.h ***/


#endif
#ifndef __JUCE_RECTANGLEPLACEMENT_JUCEHEADER__

#endif
#ifndef __JUCE_DRAWABLE_JUCEHEADER__

#endif
#ifndef __JUCE_DRAWABLECOMPOSITE_JUCEHEADER__

/*** Start of inlined file: juce_DrawableComposite.h ***/
#ifndef __JUCE_DRAWABLECOMPOSITE_JUCEHEADER__
#define __JUCE_DRAWABLECOMPOSITE_JUCEHEADER__

class JUCE_API  DrawableComposite  : public Drawable
{
public:

	DrawableComposite();

	virtual ~DrawableComposite();

	void insertDrawable (Drawable* drawable,
						 const AffineTransform& transform = AffineTransform::identity,
						 int index = -1);

	void insertDrawable (const Drawable& drawable,
						 const AffineTransform& transform = AffineTransform::identity,
						 int index = -1);

	void removeDrawable (int index, bool deleteDrawable = true);

	int getNumDrawables() const throw()					 { return drawables.size(); }

	Drawable* getDrawable (int index) const throw()				 { return drawables [index]; }

	const AffineTransform* getDrawableTransform (int index) const throw()	   { return transforms [index]; }

	void bringToFront (int index);

	void render (const Drawable::RenderingContext& context) const;
	const Rectangle<float> getBounds() const;
	bool hitTest (float x, float y) const;
	Drawable* createCopy() const;
	ValueTree createValueTree() const throw();
	static DrawableComposite* createFromValueTree (const ValueTree& tree) throw();

	juce_UseDebuggingNewOperator

private:
	OwnedArray <Drawable> drawables;
	OwnedArray <AffineTransform> transforms;

	DrawableComposite (const DrawableComposite&);
	DrawableComposite& operator= (const DrawableComposite&);
};

#endif   // __JUCE_DRAWABLECOMPOSITE_JUCEHEADER__
/*** End of inlined file: juce_DrawableComposite.h ***/


#endif
#ifndef __JUCE_DRAWABLEIMAGE_JUCEHEADER__

/*** Start of inlined file: juce_DrawableImage.h ***/
#ifndef __JUCE_DRAWABLEIMAGE_JUCEHEADER__
#define __JUCE_DRAWABLEIMAGE_JUCEHEADER__

class JUCE_API  DrawableImage  : public Drawable
{
public:

	DrawableImage();

	virtual ~DrawableImage();

	void setImage (const Image& imageToCopy);

	void setImage (Image* imageToUse,
				   bool releaseWhenNotNeeded);

	Image* getImage() const throw()				 { return image; }

	void clearImage();

	void setOpacity (float newOpacity);

	float getOpacity() const throw()				{ return opacity; }

	void setOverlayColour (const Colour& newOverlayColour);

	const Colour& getOverlayColour() const throw()		  { return overlayColour; }

	void render (const Drawable::RenderingContext& context) const;
	const Rectangle<float> getBounds() const;
	bool hitTest (float x, float y) const;
	Drawable* createCopy() const;
	ValueTree createValueTree() const throw();
	static DrawableImage* createFromValueTree (const ValueTree& tree) throw();

	juce_UseDebuggingNewOperator

private:
	Image* image;
	bool canDeleteImage;
	float opacity;
	Colour overlayColour;

	DrawableImage (const DrawableImage&);
	DrawableImage& operator= (const DrawableImage&);
};

#endif   // __JUCE_DRAWABLEIMAGE_JUCEHEADER__
/*** End of inlined file: juce_DrawableImage.h ***/


#endif
#ifndef __JUCE_DRAWABLEPATH_JUCEHEADER__

/*** Start of inlined file: juce_DrawablePath.h ***/
#ifndef __JUCE_DRAWABLEPATH_JUCEHEADER__
#define __JUCE_DRAWABLEPATH_JUCEHEADER__

class JUCE_API  DrawablePath  : public Drawable
{
public:

	DrawablePath();

	virtual ~DrawablePath();

	void setPath (const Path& newPath) throw();

	const Path& getPath() const throw()			 { return path; }

	void setFill (const FillType& newFill) throw();

	const FillType& getFill() const throw()			 { return mainFill; }

	void setStrokeFill (const FillType& newStrokeFill) throw();

	const FillType& getStrokeFill() const throw()		   { return strokeFill; }

	void setStrokeType (const PathStrokeType& newStrokeType) throw();

	void setStrokeThickness (float newThickness) throw();

	const PathStrokeType& getStrokeType() const throw()	 { return strokeType; }

	void render (const Drawable::RenderingContext& context) const;
	const Rectangle<float> getBounds() const;
	bool hitTest (float x, float y) const;
	Drawable* createCopy() const;
	ValueTree createValueTree() const throw();
	static DrawablePath* createFromValueTree (const ValueTree& tree) throw();

	juce_UseDebuggingNewOperator

private:
	Path path, stroke;
	FillType mainFill, strokeFill;
	PathStrokeType strokeType;

	void updateOutline();

	DrawablePath (const DrawablePath&);
	DrawablePath& operator= (const DrawablePath&);
};

#endif   // __JUCE_DRAWABLEPATH_JUCEHEADER__
/*** End of inlined file: juce_DrawablePath.h ***/


#endif
#ifndef __JUCE_DRAWABLETEXT_JUCEHEADER__

/*** Start of inlined file: juce_DrawableText.h ***/
#ifndef __JUCE_DRAWABLETEXT_JUCEHEADER__
#define __JUCE_DRAWABLETEXT_JUCEHEADER__

class JUCE_API  DrawableText  : public Drawable
{
public:

	DrawableText();

	virtual ~DrawableText();

	void setText (const GlyphArrangement& newText);

	void setText (const String& newText, const Font& fontToUse);

	const GlyphArrangement& getText() const throw()	 { return text; }

	void setColour (const Colour& newColour);

	const Colour& getColour() const throw()		 { return colour; }

	void render (const Drawable::RenderingContext& context) const;
	const Rectangle<float> getBounds() const;
	bool hitTest (float x, float y) const;
	Drawable* createCopy() const;
	ValueTree createValueTree() const throw();
	static DrawableText* createFromValueTree (const ValueTree& tree) throw();

	juce_UseDebuggingNewOperator

private:
	GlyphArrangement text;
	Colour colour;

	DrawableText (const DrawableText&);
	DrawableText& operator= (const DrawableText&);
};

#endif   // __JUCE_DRAWABLETEXT_JUCEHEADER__
/*** End of inlined file: juce_DrawableText.h ***/


#endif
#ifndef __JUCE_DROPSHADOWEFFECT_JUCEHEADER__

#endif
#ifndef __JUCE_GLOWEFFECT_JUCEHEADER__

/*** Start of inlined file: juce_GlowEffect.h ***/
#ifndef __JUCE_GLOWEFFECT_JUCEHEADER__
#define __JUCE_GLOWEFFECT_JUCEHEADER__

class JUCE_API  GlowEffect  : public ImageEffectFilter
{
public:

	GlowEffect();

	~GlowEffect();

	void setGlowProperties (const float newRadius,
							const Colour& newColour);

	void applyEffect (Image& sourceImage, Graphics& destContext);

	juce_UseDebuggingNewOperator

private:
	float radius;
	Colour colour;
};

#endif   // __JUCE_GLOWEFFECT_JUCEHEADER__
/*** End of inlined file: juce_GlowEffect.h ***/


#endif
#ifndef __JUCE_IMAGEEFFECTFILTER_JUCEHEADER__

#endif
#ifndef __JUCE_REDUCEOPACITYEFFECT_JUCEHEADER__

/*** Start of inlined file: juce_ReduceOpacityEffect.h ***/
#ifndef __JUCE_REDUCEOPACITYEFFECT_JUCEHEADER__
#define __JUCE_REDUCEOPACITYEFFECT_JUCEHEADER__

class JUCE_API  ReduceOpacityEffect  : public ImageEffectFilter
{
public:

	ReduceOpacityEffect (const float opacity = 1.0f);

	~ReduceOpacityEffect();

	void setOpacity (const float newOpacity);

	void applyEffect (Image& sourceImage, Graphics& destContext);

	juce_UseDebuggingNewOperator

private:
	float opacity;
};

#endif   // __JUCE_REDUCEOPACITYEFFECT_JUCEHEADER__
/*** End of inlined file: juce_ReduceOpacityEffect.h ***/


#endif
#ifndef __JUCE_FONT_JUCEHEADER__

#endif
#ifndef __JUCE_GLYPHARRANGEMENT_JUCEHEADER__

#endif
#ifndef __JUCE_TEXTLAYOUT_JUCEHEADER__

#endif
#ifndef __JUCE_TYPEFACE_JUCEHEADER__

#endif
#ifndef __JUCE_AFFINETRANSFORM_JUCEHEADER__

#endif
#ifndef __JUCE_BORDERSIZE_JUCEHEADER__

#endif
#ifndef __JUCE_LINE_JUCEHEADER__

#endif
#ifndef __JUCE_PATH_JUCEHEADER__

#endif
#ifndef __JUCE_PATHITERATOR_JUCEHEADER__

/*** Start of inlined file: juce_PathIterator.h ***/
#ifndef __JUCE_PATHITERATOR_JUCEHEADER__
#define __JUCE_PATHITERATOR_JUCEHEADER__

class JUCE_API  PathFlatteningIterator
{
public:

	PathFlatteningIterator (const Path& path,
							const AffineTransform& transform = AffineTransform::identity,
							float tolerence = 6.0f);

	~PathFlatteningIterator();

	bool next();

	float x1;
	float y1;
	float x2;
	float y2;

	bool closesSubPath;

	int subPathIndex;

	bool isLastInSubpath() const		{ return stackPos == stackBase.getData()
													  && (index >= path.numElements
														   || points [index] == Path::moveMarker); }

	juce_UseDebuggingNewOperator

private:
	const Path& path;
	const AffineTransform transform;
	float* points;
	float tolerence, subPathCloseX, subPathCloseY;
	const bool isIdentityTransform;

	HeapBlock <float> stackBase;
	float* stackPos;
	size_t index, stackSize;

	PathFlatteningIterator (const PathFlatteningIterator&);
	PathFlatteningIterator& operator= (const PathFlatteningIterator&);
};

#endif   // __JUCE_PATHITERATOR_JUCEHEADER__
/*** End of inlined file: juce_PathIterator.h ***/


#endif
#ifndef __JUCE_PATHSTROKETYPE_JUCEHEADER__

#endif
#ifndef __JUCE_POINT_JUCEHEADER__

#endif
#ifndef __JUCE_POSITIONEDRECTANGLE_JUCEHEADER__

/*** Start of inlined file: juce_PositionedRectangle.h ***/
#ifndef __JUCE_POSITIONEDRECTANGLE_JUCEHEADER__
#define __JUCE_POSITIONEDRECTANGLE_JUCEHEADER__

class JUCE_API  PositionedRectangle
{
public:

	PositionedRectangle() throw();

	PositionedRectangle (const String& stringVersion) throw();

	PositionedRectangle (const PositionedRectangle& other) throw();

	PositionedRectangle& operator= (const PositionedRectangle& other) throw();

	~PositionedRectangle() throw();

	const String toString() const throw();

	const Rectangle<int> getRectangle (const Rectangle<int>& targetSpaceToBeRelativeTo) const throw();

	void getRectangleDouble (const Rectangle<int>& targetSpaceToBeRelativeTo,
							 double& x,
							 double& y,
							 double& width,
							 double& height) const throw();

	void applyToComponent (Component& comp) const throw();

	void updateFrom (const Rectangle<int>& newPosition,
					 const Rectangle<int>& targetSpaceToBeRelativeTo) throw();

	void updateFromDouble (double x, double y, double width, double height,
						   const Rectangle<int>& targetSpaceToBeRelativeTo) throw();

	void updateFromComponent (const Component& comp) throw();

	enum AnchorPoint
	{
		anchorAtLeftOrTop		  = 1 << 0,	/**< The x or y co-ordinate specifies where the left or top edge of the rectangle should be. */
		anchorAtRightOrBottom	  = 1 << 1,	/**< The x or y co-ordinate specifies where the right or bottom edge of the rectangle should be. */
		anchorAtCentre		 = 1 << 2	 /**< The x or y co-ordinate specifies where the centre of the rectangle should be. */
	};

	enum PositionMode
	{
		absoluteFromParentTopLeft	   = 1 << 3,   /**< The x or y co-ordinate specifies an absolute distance from the parent's top or left edge. */
		absoluteFromParentBottomRight   = 1 << 4,   /**< The x or y co-ordinate specifies an absolute distance from the parent's bottom or right edge. */
		absoluteFromParentCentre	= 1 << 5,   /**< The x or y co-ordinate specifies an absolute distance from the parent's centre. */
		proportionOfParentSize	  = 1 << 6	/**< The x or y co-ordinate specifies a proportion of the parent's width or height, measured from the parent's top or left. */
	};

	enum SizeMode
	{
		absoluteSize			= 1 << 0,   /**< The width or height specifies an absolute size. */
		parentSizeMinusAbsolute	 = 1 << 1,   /**< The width or height is an amount that should be subtracted from the parent's width or height. */
		proportionalSize		= 1 << 2,   /**< The width or height specifies a proportion of the parent's width or height. */
	};

	void setModes (const AnchorPoint xAnchorMode,
				   const PositionMode xPositionMode,
				   const AnchorPoint yAnchorMode,
				   const PositionMode yPositionMode,
				   const SizeMode widthMode,
				   const SizeMode heightMode,
				   const Rectangle<int>& targetSpaceToBeRelativeTo) throw();

	AnchorPoint getAnchorPointX() const throw();

	PositionMode getPositionModeX() const throw();

	double getX() const throw()				 { return x; }

	void setX (const double newX) throw()		   { x = newX; }

	AnchorPoint getAnchorPointY() const throw();

	PositionMode getPositionModeY() const throw();

	double getY() const throw()				 { return y; }

	void setY (const double newY) throw()		   { y = newY; }

	SizeMode getWidthMode() const throw();

	double getWidth() const throw()			 { return w; }

	void setWidth (const double newWidth) throw()	   { w = newWidth; }

	SizeMode getHeightMode() const throw();

	double getHeight() const throw()			{ return h; }

	void setHeight (const double newHeight) throw()	 { h = newHeight; }

	bool isPositionAbsolute() const throw();

	bool operator== (const PositionedRectangle& other) const throw();

	bool operator!= (const PositionedRectangle& other) const throw();

	juce_UseDebuggingNewOperator

private:
	double x, y, w, h;
	uint8 xMode, yMode, wMode, hMode;

	void addPosDescription (String& result, uint8 mode, double value) const throw();
	void addSizeDescription (String& result, uint8 mode, double value) const throw();
	void decodePosString (const String& s, uint8& mode, double& value) throw();
	void decodeSizeString (const String& s, uint8& mode, double& value) throw();
	void applyPosAndSize (double& xOut, double& wOut, double x, double w,
						  uint8 xMode, uint8 wMode,
						  int parentPos, int parentSize) const throw();
	void updatePosAndSize (double& xOut, double& wOut, double x, double w,
						   uint8 xMode, uint8 wMode,
						   int parentPos, int parentSize) const throw();
};

#endif   // __JUCE_POSITIONEDRECTANGLE_JUCEHEADER__
/*** End of inlined file: juce_PositionedRectangle.h ***/


#endif
#ifndef __JUCE_RECTANGLE_JUCEHEADER__

#endif
#ifndef __JUCE_RECTANGLELIST_JUCEHEADER__

#endif
#ifndef __JUCE_CAMERADEVICE_JUCEHEADER__

/*** Start of inlined file: juce_CameraDevice.h ***/
#ifndef __JUCE_CAMERADEVICE_JUCEHEADER__
#define __JUCE_CAMERADEVICE_JUCEHEADER__

#if JUCE_USE_CAMERA

class CameraImageListener
{
public:
	CameraImageListener() {}
	virtual ~CameraImageListener() {}

	virtual void imageReceived (Image& image) = 0;
};

class JUCE_API  CameraDevice
{
public:
	virtual ~CameraDevice();

	static const StringArray getAvailableDevices();

	static CameraDevice* openDevice (int deviceIndex,
									 int minWidth = 128, int minHeight = 64,
									 int maxWidth = 1024, int maxHeight = 768);

	const String getName() const		{ return name; }

	Component* createViewerComponent();

	void startRecordingToFile (const File& file, int quality = 2);

	void stopRecording();

	static const String getFileExtension();

	const Time getTimeOfFirstRecordedFrame() const;

	void addListener (CameraImageListener* listenerToAdd);

	void removeListener (CameraImageListener* listenerToRemove);

	juce_UseDebuggingNewOperator

protected:
	CameraDevice (const String& name, int index);

private:
	void* internal;
	bool isRecording;
	String name;

	CameraDevice (const CameraDevice&);
	CameraDevice& operator= (const CameraDevice&);
};

#endif
#endif   // __JUCE_CAMERADEVICE_JUCEHEADER__
/*** End of inlined file: juce_CameraDevice.h ***/


#endif
#ifndef __JUCE_IMAGE_JUCEHEADER__

#endif
#ifndef __JUCE_IMAGECACHE_JUCEHEADER__

/*** Start of inlined file: juce_ImageCache.h ***/
#ifndef __JUCE_IMAGECACHE_JUCEHEADER__
#define __JUCE_IMAGECACHE_JUCEHEADER__

class JUCE_API  ImageCache  : private DeletedAtShutdown,
							  private Timer
{
public:

	static Image* getFromFile (const File& file);

	static Image* getFromMemory (const void* imageData, int dataSize);

	static void release (Image* imageToRelease);

	static void releaseOrDelete (Image* imageToRelease);

	static bool isImageInCache (Image* imageToLookFor);

	static void incReferenceCount (Image* image);

	static Image* getFromHashCode (int64 hashCode);

	static void addImageToCache (Image* image, int64 hashCode);

	static void setCacheTimeout (int millisecs);

	juce_UseDebuggingNewOperator

private:

	CriticalSection lock;
	struct Item;
	friend class ScopedPointer<Item>;
	friend class OwnedArray<Item>;
	OwnedArray<Item> images;

	static ImageCache* instance;
	static int cacheTimeout;

	ImageCache();
	ImageCache (const ImageCache&);
	ImageCache& operator= (const ImageCache&);
	~ImageCache();

	void timerCallback();
};

#endif   // __JUCE_IMAGECACHE_JUCEHEADER__
/*** End of inlined file: juce_ImageCache.h ***/


#endif
#ifndef __JUCE_IMAGECONVOLUTIONKERNEL_JUCEHEADER__

/*** Start of inlined file: juce_ImageConvolutionKernel.h ***/
#ifndef __JUCE_IMAGECONVOLUTIONKERNEL_JUCEHEADER__
#define __JUCE_IMAGECONVOLUTIONKERNEL_JUCEHEADER__

class JUCE_API  ImageConvolutionKernel
{
public:

	ImageConvolutionKernel (int size);

	~ImageConvolutionKernel();

	void clear();

	float getKernelValue (int x, int y) const throw();

	void setKernelValue (int x, int y, float value) throw();

	void setOverallSum (float desiredTotalSum);

	void rescaleAllValues (float multiplier);

	void createGaussianBlur (float blurRadius);

	int getKernelSize() const		   { return size; }

	void applyToImage (Image& destImage,
					   const Image* sourceImage,
					   const Rectangle<int>& destinationArea) const;

	juce_UseDebuggingNewOperator

private:
	HeapBlock <float> values;
	const int size;

	// no reason not to implement these one day..
	ImageConvolutionKernel (const ImageConvolutionKernel&);
	ImageConvolutionKernel& operator= (const ImageConvolutionKernel&);
};

#endif   // __JUCE_IMAGECONVOLUTIONKERNEL_JUCEHEADER__
/*** End of inlined file: juce_ImageConvolutionKernel.h ***/


#endif
#ifndef __JUCE_IMAGEFILEFORMAT_JUCEHEADER__

/*** Start of inlined file: juce_ImageFileFormat.h ***/
#ifndef __JUCE_IMAGEFILEFORMAT_JUCEHEADER__
#define __JUCE_IMAGEFILEFORMAT_JUCEHEADER__

class JUCE_API  ImageFileFormat
{
protected:

	ImageFileFormat()		   {}

public:
	virtual ~ImageFileFormat()	  {}

	virtual const String getFormatName() = 0;

	virtual bool canUnderstand (InputStream& input) = 0;

	virtual Image* decodeImage (InputStream& input) = 0;

	virtual bool writeImageToStream (const Image& sourceImage,
									 OutputStream& destStream) = 0;

	static ImageFileFormat* findImageFormatForStream (InputStream& input);

	static Image* loadFrom (InputStream& input);

	static Image* loadFrom (const File& file);

	static Image* loadFrom (const void* rawData,
							const int numBytesOfData);

};

class JUCE_API  PNGImageFormat  : public ImageFileFormat
{
public:

	PNGImageFormat();
	~PNGImageFormat();

	const String getFormatName();
	bool canUnderstand (InputStream& input);

	Image* decodeImage (InputStream& input);

	bool writeImageToStream (const Image& sourceImage, OutputStream& destStream);
};

class JUCE_API  JPEGImageFormat  : public ImageFileFormat
{
public:

	JPEGImageFormat();
	~JPEGImageFormat();

	void setQuality (const float newQuality);

	const String getFormatName();

	bool canUnderstand (InputStream& input);

	Image* decodeImage (InputStream& input);

	bool writeImageToStream (const Image& sourceImage, OutputStream& destStream);

private:
	float quality;
};

#endif   // __JUCE_IMAGEFILEFORMAT_JUCEHEADER__
/*** End of inlined file: juce_ImageFileFormat.h ***/


#endif
#ifndef __JUCE_DELETEDATSHUTDOWN_JUCEHEADER__

#endif
#ifndef __JUCE_FILEBASEDDOCUMENT_JUCEHEADER__

/*** Start of inlined file: juce_FileBasedDocument.h ***/
#ifndef __JUCE_FILEBASEDDOCUMENT_JUCEHEADER__
#define __JUCE_FILEBASEDDOCUMENT_JUCEHEADER__

class JUCE_API FileBasedDocument  : public ChangeBroadcaster
{
public:
	FileBasedDocument (const String& fileExtension,
					   const String& fileWildCard,
					   const String& openFileDialogTitle,
					   const String& saveFileDialogTitle);

	virtual ~FileBasedDocument();

	bool hasChangedSinceSaved() const			   { return changedSinceSave; }

	virtual void changed();

	void setChangedFlag (bool hasChanged);

	bool loadFrom (const File& fileToLoadFrom,
				   bool showMessageOnFailure);

	bool loadFromUserSpecifiedFile (bool showMessageOnFailure);

	enum SaveResult
	{
		savedOk = 0,		/**< indicates that a file was saved successfully. */
		userCancelledSave,	  /**< indicates that the user aborted the save operation. */
		failedToWriteToFile	 /**< indicates that it tried to write to a file but this failed. */
	};

	SaveResult save (bool askUserForFileIfNotSpecified,
					 bool showMessageOnFailure);

	SaveResult saveIfNeededAndUserAgrees();

	SaveResult saveAs (const File& newFile,
					   bool warnAboutOverwritingExistingFiles,
					   bool askUserForFileIfNotSpecified,
					   bool showMessageOnFailure);

	SaveResult saveAsInteractive (bool warnAboutOverwritingExistingFiles);

	const File getFile() const				  { return documentFile; }

	void setFile (const File& newFile);

protected:

	virtual const String getDocumentTitle() = 0;

	virtual const String loadDocument (const File& file) = 0;

	virtual const String saveDocument (const File& file) = 0;

	virtual const File getLastDocumentOpened() = 0;

	virtual void setLastDocumentOpened (const File& file) = 0;

public:

	juce_UseDebuggingNewOperator

private:

	File documentFile;
	bool changedSinceSave;
	String fileExtension, fileWildcard, openFileDialogTitle, saveFileDialogTitle;

	FileBasedDocument (const FileBasedDocument&);
	FileBasedDocument& operator= (const FileBasedDocument&);
};

#endif   // __JUCE_FILEBASEDDOCUMENT_JUCEHEADER__
/*** End of inlined file: juce_FileBasedDocument.h ***/


#endif
#ifndef __JUCE_PROPERTIESFILE_JUCEHEADER__

#endif
#ifndef __JUCE_RECENTLYOPENEDFILESLIST_JUCEHEADER__

/*** Start of inlined file: juce_RecentlyOpenedFilesList.h ***/
#ifndef __JUCE_RECENTLYOPENEDFILESLIST_JUCEHEADER__
#define __JUCE_RECENTLYOPENEDFILESLIST_JUCEHEADER__

class JUCE_API  RecentlyOpenedFilesList
{
public:

	RecentlyOpenedFilesList();

	~RecentlyOpenedFilesList();

	void setMaxNumberOfItems (int newMaxNumber);

	int getMaxNumberOfItems() const throw()				 { return maxNumberOfItems; }

	int getNumFiles() const;

	const File getFile (int index) const;

	const StringArray& getAllFilenames() const throw()		  { return files; }

	void clear();

	void addFile (const File& file);

	void removeNonExistentFiles();

	int createPopupMenuItems (PopupMenu& menuToAddItemsTo,
							  int baseItemId,
							  bool showFullPaths,
							  bool dontAddNonExistentFiles,
							  const File** filesToAvoid = 0);

	const String toString() const;

	void restoreFromString (const String& stringifiedVersion);

	juce_UseDebuggingNewOperator

private:

	StringArray files;
	int maxNumberOfItems;
};

#endif   // __JUCE_RECENTLYOPENEDFILESLIST_JUCEHEADER__
/*** End of inlined file: juce_RecentlyOpenedFilesList.h ***/


#endif
#ifndef __JUCE_SELECTEDITEMSET_JUCEHEADER__

#endif
#ifndef __JUCE_SYSTEMCLIPBOARD_JUCEHEADER__

/*** Start of inlined file: juce_SystemClipboard.h ***/
#ifndef __JUCE_SYSTEMCLIPBOARD_JUCEHEADER__
#define __JUCE_SYSTEMCLIPBOARD_JUCEHEADER__

class JUCE_API  SystemClipboard
{
public:
	static void copyTextToClipboard (const String& text);

	static const String getTextFromClipboard();
};

#endif   // __JUCE_SYSTEMCLIPBOARD_JUCEHEADER__
/*** End of inlined file: juce_SystemClipboard.h ***/


#endif
#ifndef __JUCE_UNDOABLEACTION_JUCEHEADER__

#endif
#ifndef __JUCE_UNDOMANAGER_JUCEHEADER__

#endif

#endif
/*** End of inlined file: juce_app_includes.h ***/


#endif

#if JUCE_MSVC
  #pragma warning (pop)
  #pragma pack (pop)
#endif

END_JUCE_NAMESPACE

#ifndef DONT_SET_USING_JUCE_NAMESPACE
#ifdef JUCE_NAMESPACE

  // this will obviously save a lot of typing, but can be disabled by
  // defining DONT_SET_USING_JUCE_NAMESPACE, in case there are conflicts.
  using namespace JUCE_NAMESPACE;

  #if (JUCE_MAC || JUCE_IPHONE) && ! JUCE_DONT_DEFINE_MACROS
	#define Component	   JUCE_NAMESPACE::Component
	#define MemoryBlock	 JUCE_NAMESPACE::MemoryBlock
	#define Point	   JUCE_NAMESPACE::Point
	#define Button	  JUCE_NAMESPACE::Button
  #endif

  #if JUCE_WINDOWS && ! JUCE_DONT_DEFINE_MACROS
	#define Rectangle	   JUCE_NAMESPACE::Rectangle
  #endif
#endif
#endif

#if JUCE_MSVC

  #ifndef DONT_AUTOLINK_TO_JUCE_LIBRARY

	#ifdef JUCE_DLL
	  #ifdef JUCE_DEBUG
		#define AUTOLINKEDLIB "JUCE_debug.lib"
	  #else
		#define AUTOLINKEDLIB "JUCE.lib"
	  #endif
	#else
	  #ifdef JUCE_DEBUG
		#ifdef _WIN64
		  #define AUTOLINKEDLIB "jucelib_static_x64_debug.lib"
		#else
		  #define AUTOLINKEDLIB "jucelib_static_Win32_debug.lib"
		#endif
	  #else
		#ifdef _WIN64
		  #define AUTOLINKEDLIB "jucelib_static_x64.lib"
		#else
		  #define AUTOLINKEDLIB "jucelib_static_Win32.lib"
		#endif
	  #endif
	#endif

	#pragma comment(lib, AUTOLINKEDLIB)

	#if ! DONT_LIST_JUCE_AUTOLINKEDLIBS
	  #pragma message("JUCE! Library to link to: " AUTOLINKEDLIB)
	#endif

	// Auto-link the other win32 libs that are needed by library calls..
	#if ! (defined (DONT_AUTOLINK_TO_WIN32_LIBRARIES) || defined (JUCE_DLL))

/*** Start of inlined file: juce_win32_AutoLinkLibraries.h ***/
// Auto-links to various win32 libs that are needed by library calls..
#pragma comment(lib, "kernel32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "vfw32.lib")
#pragma comment(lib, "comdlg32.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "wininet.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "oleaut32.lib")
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "comsupp.lib")
#pragma comment(lib, "version.lib")

#if JUCE_OPENGL
 #pragma comment(lib, "OpenGL32.Lib")
 #pragma comment(lib, "GlU32.Lib")
#endif

#if JUCE_QUICKTIME
 #pragma comment (lib, "QTMLClient.lib")
#endif

#if JUCE_USE_CAMERA
 #pragma comment (lib, "Strmiids.lib")
 #pragma comment (lib, "wmvcore.lib")
#endif
/*** End of inlined file: juce_win32_AutoLinkLibraries.h ***/


	#endif

  #endif

#endif

#if defined (JUCE_GCC) || defined (__MWERKS__)

  #define START_JUCE_APPLICATION(AppClass) \
	int main (int argc, char* argv[]) \
	{ \
		return JUCE_NAMESPACE::JUCEApplication::main (argc, (const char**) argv, new AppClass()); \
	}

#elif JUCE_WINDOWS

  #ifdef _CONSOLE
	#define START_JUCE_APPLICATION(AppClass) \
		int main (int, char* argv[]) \
		{ \
			JUCE_NAMESPACE::String commandLineString (JUCE_NAMESPACE::PlatformUtilities::getCurrentCommandLineParams()); \
			return JUCE_NAMESPACE::JUCEApplication::main (commandLineString, new AppClass()); \
		}
  #elif ! defined (_AFXDLL)
	#ifdef _WINDOWS_
	  #define START_JUCE_APPLICATION(AppClass) \
		  int WINAPI WinMain (HINSTANCE, HINSTANCE, LPSTR, int) \
		  { \
			  JUCE_NAMESPACE::String commandLineString (JUCE_NAMESPACE::PlatformUtilities::getCurrentCommandLineParams()); \
			  return JUCE_NAMESPACE::JUCEApplication::main (commandLineString, new AppClass()); \
		  }
	#else
	  #define START_JUCE_APPLICATION(AppClass) \
		  int __stdcall WinMain (int, int, const char*, int) \
		  { \
			  JUCE_NAMESPACE::String commandLineString (JUCE_NAMESPACE::PlatformUtilities::getCurrentCommandLineParams()); \
			  return JUCE_NAMESPACE::JUCEApplication::main (commandLineString, new AppClass()); \
		  }
	#endif
  #endif

#endif

#endif   // __JUCE_JUCEHEADER__
/*** End of inlined file: juce.h ***/

#endif   // __JUCE_AMALGAMATED_TEMPLATE_JUCEHEADER__

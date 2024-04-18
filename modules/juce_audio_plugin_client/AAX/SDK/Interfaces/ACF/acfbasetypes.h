/***********************************************************************
	This file is part of the Avid AAX SDK.

	The AAX SDK is subject to commercial or open-source licensing.

	By using the AAX SDK, you agree to the terms of both the Avid AAX SDK License
	Agreement and Avid Privacy Policy.

	AAX SDK License: https://developer.avid.com/aax
	Privacy Policy: https://www.avid.com/legal/privacy-policy-statement

	Or: You may also use this code under the terms of the GPL v3 (see
	www.gnu.org/licenses).

	THE AAX SDK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
	EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
	DISCLAIMED.

*
* Copyright 2004, 2008-2010, 2013, 2018, 2024 Avid Technology, Inc.
*
************************************************************************/

#ifndef acfbasetypes_h
#define acfbasetypes_h

/*!
    \file acfbasetypes.h
    \brief Defines all of the C types shared by all ACF hosts and plug-ins.

    The layout for this file is based upon AAFPlatform.h which is availabled from SourceForge.
 */

/* wchar_t definition */
#include <stddef.h>


/*
 * For C++11 or greater will be used override identifier.
 */
#if __cplusplus >= 201103L
#define ACF_OVERRIDE override
#else
#define ACF_OVERRIDE
#endif

/* 
 * Construct a numeric GCC version that can easily be used to test for features that
 * were added for specific versions.
 */
#if defined(__GNUC__)
#define ACF_COMPILER_GCC_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#endif


/*!
 *  Compiler:   Microsoft Visual C++
 *  Processor:  Intel x86
 *  OS:         Windows NT
 */
#if   defined(__INTEL_COMPILER) && defined(_MSC_VER) && defined(_M_IX86) && defined(_WIN32)
#define ACF_CPU_INTEL
#define ACF_OS_WINDOWS
#define ACF_COMPILER_INTEL
#define ACF_PLATFORM_INTEL_INTEL_WINDOWS


/*!
 *  Compiler:   Microsoft Visual C++
 *  Processor:  Intel x86, 64bit
 *  OS:         Windows NT
 */
#elif defined(__INTEL_COMPILER) && defined(_MSC_VER) && (defined(_M_AMD64) || defined(_M_IA64)) && defined(_WIN64)
#define ACF_CPU_INTEL
#define ACF_OS_WINDOWS
#define ACF_OS_64
#define ACF_COMPILER_INTEL
#define ACF_PLATFORM_INTEL_INTEL_WINDOWS
#define ACF_PLATFORM_INTEL_INTEL64_WINDOWS


/*!
 *  Compiler:   Microsoft Visual C++
 *  Processor:  Intel x64
 *  OS:         Windows NT 64bit
 */
#elif defined(_MSC_VER) && (defined(_M_AMD64) || defined(_M_IA64)) && defined(_WIN64)
#define ACF_CPU_INTEL
#define ACF_OS_64
#define ACF_OS_WINDOWS
#define ACF_COMPILER_MSC
#define ACF_PLATFORM_MSC_INTEL_WINDOWS
#define ACF_PLATFORM_MSC_INTEL64_WINDOWS


/*!
 *  Compiler:   Microsoft Visual C++
 *  Processor:  Intel x86
 *  OS:         Windows NT 32bit
 */
#elif defined(_MSC_VER) && defined(_M_IX86) && defined(_WIN32)
#define ACF_CPU_INTEL
#define ACF_OS_WINDOWS
#define ACF_COMPILER_MSC
#define ACF_PLATFORM_MSC_INTEL_WINDOWS


/*!
 *  Compiler:   Metrowerks CodeWarrior
 *  Processor:  PowerPC
 *  OS:         MacOS 10
 */
#elif defined(__MWERKS__) && defined(__MACH__)
#define ACF_CPU_POWERPC
#define ACF_OS_MACOS10
#define ACF_COMPILER_MWERKS
#define ACF_PLATFORM_MWERKS_POWERPC_MACOS10


/*!
 *  Compiler:   Metrowerks CodeWarrior
 *  Processor:  PowerPC
 *  OS:         Classic MacOS
 */
#elif defined(__MWERKS__) && defined(__POWERPC__) && defined(macintosh)
#define ACF_CPU_POWERPC
#define ACF_OS_MACOS
#define ACF_COMPILER_MWERKS
#define ACF_PLATFORM_MWERKS_POWERPC_MACOS


/*!
 *  Compiler:   Metrowerks CodeWarrior
 *  Processor:  Intel x86
 *  OS:         Windows NT
 */
#elif defined(__MWERKS__) && defined(_M_IX86) && defined(_WIN32)
#define ACF_CPU_INTEL
#define ACF_OS_WINDOWS
#define ACF_COMPILER_MWERKS
#define ACF_PLATFORM_MWERKS_INTEL_WINDOWS


/*!
 *  Compiler:   GNU C++
 *  Processor:  MIPS
 *  OS:         IRIX
 */
#elif defined(__GNUC__) && defined(__mips__) && defined(__sgi__)
#define ACF_CPU_MIPS
#define ACF_OS_IRIX
#define ACF_OS_UNIX
#define ACF_COMPILER_GCC
#define ACF_PLATFORM_GCC_MIPS_IRIX


/*!
 *  Compiler:   MIPSpro C++
 *  Processor:  MIPS
 *  OS:         IRIX
 */
#elif defined(mips) && defined(sgi)
#define ACF_CPU_MIPS
#define ACF_OS_IRIX
#define ACF_OS_UNIX
#define ACF_COMPILER_MIPSPRO
#define ACF_PLATFORM_MIPSPRO_MIPS_IRIX


/*!
 *  Compiler:   GNU C++
 *  Processor:  Intel x86
 *  OS:         linux
 */
#elif defined(__GNUC__) && defined(__i386__) && defined(__linux__)
#define ACF_CPU_INTEL
#define ACF_OS_LINUX
#define ACF_OS_UNIX
#define ACF_COMPILER_GCC
#define ACF_PLATFORM_GCC_INTEL_LINUX


/*!
 *  Compiler:   GNU C++
 *  Processor:  ARM
 *  OS:         linux
 */
#elif defined(__GNUC__) && defined(__arm__) && defined(__linux__)
#define ACF_CPU_ARM
#define ACF_OS_LINUX
#define ACF_OS_UNIX
#define ACF_COMPILER_GCC
#define ACF_PLATFORM_GCC_ARM_LINUX


/*!
 *  Compiler:   GNU C++
 *  Processor:  AARCH64
 *  OS:         linux
 */
#elif defined(__GNUC__) && defined(__aarch64__) && defined(__linux__)
#define ACF_CPU_AARCH64
#define ACF_OS_64
#define ACF_OS_LINUX
#define ACF_OS_UNIX
#define ACF_COMPILER_GCC
#define ACF_PLATFORM_GCC_ARM_LINUX


/*!
 *  Compiler:   GNU C++
 *  Processor:  Intel x86, 64-bit
 *  OS:         linux
 */
#elif defined(__GNUC__) && defined(__x86_64__) && defined(__linux__)
#define ACF_CPU_INTEL
#define ACF_OS_64
#define ACF_OS_LINUX
#define ACF_OS_UNIX
#define ACF_COMPILER_GCC
#define ACF_PLATFORM_GCC_INTEL_LINUX
#define ACF_PLATFORM_GCC_INTEL64_LINUX


/*!
 *  Compiler:   GNU C++
 *  Processor:  Intel x86
 *  OS:         FreeBSD
 */
#elif defined(__GNUC__) && defined(__i386__) && defined(__FreeBSD__)
#define ACF_CPU_INTEL
#define ACF_OS_FREEBSD
#define ACF_OS_UNIX
#define ACF_COMPILER_GCC
#define ACF_PLATFORM_GCC_INTEL_FREEBSD


/*!
*  Compiler:   GNU C++
*  Processor:  Intel x86, 64-bit
*  OS:         FreeBSD
*/
#elif defined(__GNUC__) && defined(__x86_64__) && defined(__FreeBSD__)
#define ACF_CPU_INTEL
#define ACF_OS_64
#define ACF_OS_FREEBSD
#define ACF_OS_UNIX
#define ACF_COMPILER_GCC
#define ACF_PLATFORM_GCC_INTEL_FREEBSD
#define ACF_PLATFORM_GCC_INTEL64_FREEBSD


/*!
 *  Compiler:   GNU C++
 *  Processor:  PowerPC, 32-bit
 *  OS:         MacOS 10
 */
#elif defined(__GNUC__) && defined(__ppc__) && defined(__APPLE__) && defined(__APPLE_CC__)
#define ACF_CPU_POWERPC
#define ACF_OS_MACOS10
#define ACF_OS_UNIX
#define ACF_COMPILER_GCC
#define ACF_PLATFORM_GCC_POWERPC_MACOS10


/*!
 *  Compiler:   GNU C++
 *  Processor:  PowerPC, 64-bit
 *  OS:         MacOS 10
 */
#elif defined(__GNUC__) && defined(__ppc64__) && defined(__APPLE__) && defined(__APPLE_CC__)
#define ACF_CPU_POWERPC
#define ACF_OS_64
#define ACF_OS_MACOS10
#define ACF_OS_UNIX
#define ACF_COMPILER_GCC
#define ACF_PLATFORM_GCC_POWERPC_MACOS10
#define ACF_PLATFORM_GCC_POWERPC64_MACOS10


/*!
 *  Compiler:   GNU C++
 *  Processor:  Intel x86
 *  OS:         MacOS 10
 */
#elif defined(__GNUC__) && defined(__i386__) && defined(__APPLE__) && defined(__APPLE_CC__)
#define ACF_CPU_INTEL
#define ACF_OS_MACOS10
#define ACF_OS_UNIX
#define ACF_COMPILER_GCC
#define ACF_PLATFORM_GCC_INTEL_MACOS10


/*!
*  Compiler:   GNU C++
 *  Processor:  Intel x86, 64-bit
 *  OS:         MacOS 10
 */
#elif defined(__GNUC__) && defined(__x86_64__) && defined(__APPLE__) && defined(__APPLE_CC__)
#define ACF_CPU_INTEL
#define ACF_OS_64
#define ACF_OS_MACOS10
#define ACF_OS_UNIX
#define ACF_COMPILER_GCC
#define ACF_PLATFORM_GCC_INTEL_MACOS10
#define ACF_PLATFORM_GCC_INTEL64_MACOS10


/*!
 *  Compiler:   Intel Compiler
 *  Processor:  Intel x86
 *  OS:         MacOS 10
 */
#elif (defined(__INTEL_COMPILER) || defined(__ICL) || defined(__ICC)) && defined(__MACH__)
#define ACF_CPU_INTEL
#define ACF_OS_MACOS10
#define ACF_OS_UNIX
#define ACF_COMPILER_INTEL
#define ACF_PLATFORM_INTEL_INTEL_MACOS10


/*!
 *  Compiler:   GNU C++
 *  Processor:  ARM
 *  OS:         linux
 */
#elif defined(__GNUC__) && defined(__arm__) && defined(__linux__)
#define ACF_CPU_ARM
#define ACF_OS_LINUX
#define ACF_OS_UNIX
#define ACF_COMPILER_GCC
#define ACF_PLATFORM_GCC_ARM_LINUX


/*!
 *  Compiler:   Arm Compiler
 *  Processor:  Arm
 *  OS:         iOS
 */
#elif (TARGET_OS_IOS)
#define ACF_CPU_ARM
#define ACF_OS_IOS
#define ACF_OS_UNIX
#define ACF_COMPILER_ARM
#define ACF_PLATFORM_ARM_IOS


/*!
*  Compiler:   GNU C++
 *  Processor:  ARM, 64-bit
 *  OS:         MacOS 11
 */
#elif defined(__GNUC__) && defined(__arm64__) && ( defined(__APPLE_CPP__) || defined(__APPLE_CC__) )
#define ACF_CPU_ARM
#define ACF_OS_64
#define ACF_OS_MACOS10
#define ACF_OS_MACOS11
#define ACF_OS_UNIX
#define ACF_COMPILER_GCC
#define ACF_PLATFORM_GCC_ARM_MACOS11
#define ACF_PLATFORM_GCC_ARM64_MACOS11


#else
#error Unknown platform

#endif
/*! End of Platform definition */


/*!
 * For compatibility with earlier releases of the sdk
 */
#if defined(ACF_PLATFORM_MWERKS_POWERPC_MACOS10) || defined(ACF_PLATFORM_GCC_POWERPC_MACOS10) || defined(ACF_PLATFORM_GCC_INTEL_MACOS10) || defined(ACF_PLATFORM_GCC_ARM64_MACOS11)
	#define ACF_MAC 0
	#define ACF_WIN 0
	#define ACF_MACH 1
#elif defined(ACF_PLATFORM_MWERKS_POWERPC_MACOS)
	#define ACF_MAC 1
	#define ACF_WIN 0
	#define ACF_MACH 0
#elif defined(ACF_PLATFORM_MSC_INTEL_WINDOWS) || defined(ACF_PLATFORM_INTEL_INTEL_WINDOWS) || defined(ACF_PLATFORM_MWERKS_INTEL_WINDOWS)
	#define ACF_MAC 0
	#define ACF_WIN 1
	#define ACF_MACH 0
#elif defined(ACF_OS_LINUX) || defined(ACF_OS_FREEBSD)
	#define ACF_MAC 0
	#define ACF_WIN 0
	#define ACF_MACH 0
	#define ACF_UNIX 1
#elif defined(ACF_OS_IOS)
    #define ACF_IOS 1
    #define ACF_MAC 0
    #define ACF_WIN 0
    #define ACF_MACH 0
    #define ACF_UNIX 0
#else
	#error Unknown OS!
#endif


#ifdef __cplusplus
#define ACFEXTERN_C extern "C"
#else
#define ACFEXTERN_C
#endif


// 
#if defined(ACF_COMPILER_GCC_VERSION) && (ACF_COMPILER_GCC_VERSION >= 40000)
	#if defined(ACFPLUGIN_EXPORTS)
		#define ACFEXPORT __attribute__((visibility("default")))
	#else
		#define ACFEXPORT
	#endif
#elif ((defined(_MSC_VER) || defined(__INTEL_COMPILER) || defined(__MWERKS__)) && (defined(_WIN32) || defined(_WIN64)))
	#if defined(ACFPLUGIN_EXPORTS)
		#define ACFEXPORT __declspec(dllexport)
	#else
		#define ACFEXPORT
	#endif
#else
	#define ACFEXPORT
#endif



// Define used to force the size of enum types to 4 bytes.
#define ACF_FORCE_LONG				0x7FFFFFFFL


/*! Minimalist set of types
 * \note some of these definitions are compiler dependent!
 */

/*!
 *  Classic MacOS
 */
#if defined( ACF_PLATFORM_MWERKS_POWERPC_MACOS )
typedef unsigned char acfUInt8;
typedef acfUInt8 acfByte;
typedef signed char acfSInt8;
typedef acfUInt8 acfUChar;
typedef acfSInt8 acfSChar;
typedef unsigned short int acfUInt16;
typedef signed short int acfSInt16;
typedef unsigned long int acfUInt32;
typedef signed long int acfSInt32;

#if (__MWERKS__ >= 0x1100)
typedef unsigned long long int acfUInt64;
typedef signed long long int acfSInt64;
#else
#error "The ACF API required built-in support for 64-bit integers!"
#endif

typedef char acfChar;

typedef wchar_t acfUniChar;

typedef double acfFloat64;

typedef float  acfFloat32;

typedef long ACFRESULT;


/*!
 *  MacOS 10 & Windows with Metrowerks
 */
#elif defined( ACF_PLATFORM_MWERKS_POWERPC_MACOS10 ) || defined( ACF_PLATFORM_MWERKS_INTEL_WINDOWS )
typedef unsigned char acfUInt8;
typedef acfUInt8 acfByte;
typedef signed char acfSInt8;
typedef acfUInt8 acfUChar;
typedef acfSInt8 acfSChar;
typedef unsigned short int acfUInt16;
typedef signed short int acfSInt16;
typedef unsigned long int acfUInt32;
typedef signed long int acfSInt32;
typedef unsigned long long int acfUInt64;
typedef signed long long int acfSInt64;

typedef char acfChar;

typedef wchar_t acfUniChar;

typedef double acfFloat64;

typedef float  acfFloat32;

typedef long ACFRESULT;


/*!
 *  Windows
 */
#elif defined( ACF_PLATFORM_MSC_INTEL_WINDOWS ) || defined(ACF_PLATFORM_INTEL_INTEL_WINDOWS)
typedef unsigned char acfUInt8;
typedef acfUInt8 acfByte;
typedef signed char acfSInt8;
typedef acfUInt8 acfUChar;
typedef acfSInt8 acfSChar;
typedef unsigned short int acfUInt16;
typedef signed short int acfSInt16;
typedef unsigned long int acfUInt32;
typedef signed long int acfSInt32;

typedef unsigned __int64 acfUInt64;
typedef __int64 acfSInt64;

typedef char acfChar;

typedef wchar_t acfUniChar;

typedef double acfFloat64;

typedef float  acfFloat32;

typedef long ACFRESULT;


/*!
 *  IRIX
 */
#elif defined( ACF_PLATFORM_MIPSPRO_MIPS_IRIX ) || defined( ACF_PLATFORM_GCC_MIPS_IRIX )
typedef unsigned char acfUInt8;
typedef acfUInt8 acfByte;
typedef signed char acfSInt8;
typedef acfUInt8 acfUChar;
typedef acfSInt8 acfSChar;
typedef unsigned short int acfUInt16;
typedef signed short int acfSInt16;
typedef unsigned long int acfUInt32;
typedef signed long int acfSInt32;

#if defined( __LONGLONG ) || defined( _LONGLONG )
typedef unsigned long long int acfUInt64;
typedef signed long long int acfSInt64;
#else
#error "The ACF API required built-in support for 64-bit integers!"
#endif

typedef char acfChar;

typedef wchar_t acfUniChar;

typedef double acfFloat64;

typedef float  acfFloat32;

typedef long ACFRESULT;


/*!
 *  64-bit GCC: Linux, FreeBSD, Darwin, etc.
 */
#elif defined( ACF_COMPILER_GCC ) && defined( ACF_OS_64 )
typedef unsigned char acfUInt8;
typedef acfUInt8 acfByte;
typedef signed char acfSInt8;
typedef acfUInt8 acfUChar;
typedef acfSInt8 acfSChar;
typedef unsigned short acfUInt16;
typedef signed short acfSInt16;
typedef unsigned int acfUInt32;
typedef signed int acfSInt32;
typedef unsigned long long acfUInt64;
typedef signed long long acfSInt64;

typedef char acfChar;

typedef wchar_t acfUniChar;

typedef double acfFloat64;

typedef float  acfFloat32;

typedef int ACFRESULT;


/*!
 *  32-bit GCC: Linux, FreeBSD, Darwin, etc.
 */
#elif ( defined( ACF_COMPILER_GCC ) && !defined( ACF_OS_64 ) ) || defined( ACF_COMPILER_ARM )
typedef unsigned char acfUInt8;
typedef acfUInt8 acfByte;
typedef signed char acfSInt8;
typedef acfUInt8 acfUChar;
typedef acfSInt8 acfSChar;
typedef unsigned short int acfUInt16;
typedef signed short int acfSInt16;
typedef unsigned long int acfUInt32;
typedef signed long int acfSInt32;
typedef unsigned long long int acfUInt64;
typedef signed long long int acfSInt64;

typedef char acfChar;

typedef wchar_t acfUniChar;

typedef double acfFloat64;

typedef float  acfFloat32;

typedef long ACFRESULT;


#else
    #error "Unsupported compiler!"
#endif


/*!
	Fall back to define ACF_OS_64 just in case we missed a more specific compiler/platform/os above.
 */
#ifndef ACF_OS_64
#if (defined(_M_AMD64) || defined(_M_IA64) || defined(__x86_64__) || defined(__arm64__) || defined(__x86_64) || defined(_IA64) || defined(__IA64__) || defined(__ia64__) || defined(__ppc64__))
#define ACF_OS_64
#endif
#endif


/*!
	Define integer types that are the same size as pointer that can be used for architecture
	and platform independent pointer arithmetic.
 */
#if defined(ACF_OS_64)
typedef acfSInt64 acfSIntPtr;
typedef acfUInt64 acfUIntPtr;
#elif defined(ACF_OS_WINDOWS)
typedef __w64 acfSInt32 acfSIntPtr;
typedef __w64 acfUInt32 acfUIntPtr;
#else
typedef acfSInt32 acfSIntPtr;
typedef acfUInt32 acfUIntPtr;
#endif


/*!
	Define deprecated acfDouble as acfFloat64 to maintain compatibility with older interfaces.
 */
#define acfDouble acfFloat64

/*!
	Define deprecated acfFloat as acfFloat32 to maintain compatibility with older interfaces.
 */
#define acfFloat acfFloat32


/*!
    \typedef acfWChar
*/
typedef acfUniChar acfWChar;


/*!
  \typedef acfUTF8
  \brief Data type for UTF8 encoded character data. 
*/
typedef acfUInt8 acfUTF8;


/*!
  \typedef acfUTF16
  \brief Data type for UTF16 encoded character data. 
*/
typedef acfUInt16 acfUTF16;


/*!
  \typedef acfUTF32
  \brief Data type for UTF32 encoded character data. 
*/
typedef acfUInt32 acfUTF32;


/*!
    \typedef acfBoolean
    \brief C-style Boolean type for ACF
    \remarks
    \n \em kACFFalse Represents boolean false
    \n \em kACFTrue Represents boolean true
    \note
    \n Never test for equality of an acfBoolean integer variable directly with kACFTrue.
    \n For example:
    \code
    acfSInt32  l_lVal = 42;
    acfBoolean l_bCondition1 = l_lVal;
    ...
    if (l_bCondition1 == kACFTrue)
    {
        // Will never be reached
        // This example does not work as expected because the integer value of l_bCondition1 is 42.
    }
    ...
    if ((l_bCondition1 != kACFFalse) || (l_bCondition1 != 0) || (l_bCondition1) || (!!l_bCondition1))
    {
        // This block will be reached.
    }
    \endcode

    
*/
typedef acfSInt32 acfBoolean;


/*!
    \typedef acfBool
    \brief Old enum Boolean type for ACF (deprecated)
    \remarks
    \n \em kACFFalse Represents boolean false
    \n \em kACFTrue Represents boolean true
    \deprecated The use of acfBool in an ACF interface is now deprecated in favor of the integer-based \ref acfBoolean type.
    \n Any ACF interface that has been released with acfBool is frozen and will remain unchanged.
*/
typedef enum _acfBool {
	kACFFalse = 0L,
	kACFTrue = 1L,
	kACFBool_Max = ACF_FORCE_LONG
} acfBool;



/*!
    \typedef acfUID
    \brief GUID compatible structure for ACF.
*/
typedef struct _acfUID {
	acfUInt32 Data1;
	acfUInt16 Data2;
	acfUInt16 Data3;
	acfUInt8  Data4[8];
} acfUID;


/*!
    \typedef acfIID
    \brief IID compatible structure for ACF.
*/
typedef acfUID acfIID;


/*!
    \typedef acfCLSID
    \brief CLSID compatible structure for ACF.
*/
typedef acfUID acfCLSID;



/*!
    \typedef acfPoint
    \brief Represents a two dimensional integer point
    \n \em x The x coordinate.
    \n \em y The y coordinate.
*/
typedef struct _acfPoint {
    acfSInt32 x;
    acfSInt32 y;
} acfPoint;


/*!
    \typedef acfRect
    \brief Represents a two dimensional integer rectangle.
    \n \em x1 The first x coordinate.
    \n \em y1 The first y coordinate.
    \n \em x2 The second x coordinate.
    \n \em y2 The second y coordinate.
*/
typedef struct _acfRect {
	acfSInt32 x1;
	acfSInt32 y1;
	acfSInt32 x2;
	acfSInt32 y2;
} acfRect;


/*!
    \typedef acfBBox
    \brief Alias for acfRect.
*/
typedef acfRect acfBBox;


/*!
    \typedef acfBounds
    \brief Alias for acfRect.
*/
typedef acfRect acfBounds;


/*!
    \typedef acfSize
    \brief Represents a two dimensional integer size.
    \n \em dx Distance in the x dimension.
    \n \em dy Distance in the y dimension.
*/
typedef struct _acfSize {
    acfSInt32 dx;
    acfSInt32 dy;
} acfSize;


/*!
    \typedef acfRational32
    \brief Represents a rational number with 32-bit signed numerator and denominator components.
    \n \em numerator
    \n \em denominator
*/
typedef struct _acfRational32 {
	acfSInt32 numerator;
	acfSInt32 denominator;
} acfRational32;


/*!
    \typedef acfRational64
    \brief Represents a rational number with 64-bit signed numerator and denominator components.
    \n \em numerator
    \n \em denominator
*/
typedef struct _acfRational64 {
	acfSInt64 numerator;
	acfSInt64 denominator;
} acfRational64;


/*!
    \enum acfDebugLevel
    \brief Used by host to determine what console messages should be displayed.
    \n \em  kACFDebugLevel_None Used by the host to suppress all output to the console.
    \n \em  kACFDebugLevel_Error Display plug in errors in host console.
    \n \em  kACFDebugLevel_Warning Display plug in warnings in host console.
    \n \em  kACFDebugLevel_Verbose Display plug in verbose messages in host console.
    \n \em  kACFDebugLevel_Trace Display plug in trace messages in host console.
    \n \em  kACFDebugLevel_Info Display important plug in information messages in host console.
    \n \em  kACFDebugLevel_Full Display all plug in messages in host console.
    \note It is recommended that all hosts support Error, Warning and Info messages by default.
*/
typedef enum _acfDebugLevel {
	kACFDebugLevel_None = 		0,
	kACFDebugLevel_Error =		1 << 0,
	kACFDebugLevel_Warning =	1 << 1,
	kACFDebugLevel_Verbose =	1 << 2,
	kACFDebugLevel_Trace = 		1 << 3,
	kACFDebugLevel_Info =       1 << 4,
	kACFDebugLevel_Full = 		0xFFFFFFFF,
	kACFDebugLevel_Max = ACF_FORCE_LONG
} acfDebugLevel;



typedef acfUInt16 acfByteOrder;


/*!
    kACFBigendianByteOrder
    Indicates that the data is in big endian byte order. In this case the type's
	Reorder method will be called when the client that accesses the corresponding value
	data is on a little endian system.
 */
const acfByteOrder kACFBigEndianByteOrder = 0x4D4D; // 'MM'

/*!
    kACFLittleendianByteOrder
    Indicates that the data is in little endian byte order. In this case the type's
	Reorder method will be called when the client that accesses the corresponding value
	data is on a big endian system.
 */
const acfByteOrder kACFLittleEndianByteOrder = 0x4949; // 'II'

/*!
    kACFUnspecifiedByteOrder
    Indicates that the data type has an unspecified byte order. In this case the type's
	Reorder method will be called but only value meta-data will be swapped, the client that
	reads the corresponding value data must perform its own byte swapping if necessary.
 */
const acfByteOrder kACFUnspecifiedByteOrder = 0x5555; // 'UU'

/*!
	ACF_SIZE_UNK
	Used in the IACFDefinition interface when copying and defining attributes. It signals the function
	calculate, when possible, to calculate the size of the attribute. This is particularly useful with strings.
*/
const acfUInt32 ACF_SIZE_UNK               = (acfUInt32)(-1);

#endif // acfbasetypes_h

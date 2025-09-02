/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

#pragma once

//==============================================================================
/*  This file figures out which platform is being built, and defines some macros
    that the rest of the code can use for OS-specific compilation.

    Macros that will be set here are:

    - One of JUCE_WINDOWS, JUCE_MAC JUCE_LINUX, JUCE_IOS, JUCE_ANDROID, etc.
    - Either JUCE_32BIT or JUCE_64BIT, depending on the architecture.
    - Either JUCE_LITTLE_ENDIAN or JUCE_BIG_ENDIAN.
    - Either JUCE_INTEL or JUCE_ARM
    - Either JUCE_GCC or JUCE_CLANG or JUCE_MSVC
*/

//==============================================================================
#ifdef JUCE_APP_CONFIG_HEADER
 #include JUCE_APP_CONFIG_HEADER
#elif ! defined (JUCE_GLOBAL_MODULE_SETTINGS_INCLUDED)
 /*
    Most projects will contain a global header file containing various settings that
    should be applied to all the code in your project. If you use the projucer, it'll
    set up a global header file for you automatically, but if you're doing things manually,
    you may want to set the JUCE_APP_CONFIG_HEADER macro with the name of a file to include,
    or just include one before all the module cpp files, in which you set
    JUCE_GLOBAL_MODULE_SETTINGS_INCLUDED=1 to silence this error.
    (Or if you don't need a global header, then you can just define JUCE_GLOBAL_MODULE_SETTINGS_INCLUDED
    globally to avoid this error).

    Note for people who hit this error when trying to compile a JUCE project created by
    a pre-v4.2 version of the Introjucer/Projucer, it's very easy to fix: just re-save
    your project with the latest version of the Projucer, and it'll magically fix this!
 */
 #error "No global header file was included!"
#endif

//==============================================================================
#if defined (_WIN32) || defined (_WIN64)
  #define       JUCE_WINDOWS 1
#elif defined (JUCE_ANDROID)
  #undef        JUCE_ANDROID
  #define       JUCE_ANDROID 1
#elif defined (__FreeBSD__) || defined (__OpenBSD__)
  #define       JUCE_BSD 1
#elif defined (LINUX) || defined (__linux__)
  #define       JUCE_LINUX 1
#elif defined (__APPLE_CPP__) || defined (__APPLE_CC__)
  #define CF_EXCLUDE_CSTD_HEADERS 1
  #include <TargetConditionals.h> // (needed to find out what platform we're using)
  #include <AvailabilityMacros.h>

  #if TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR
    #define     JUCE_IPHONE 1
    #define     JUCE_IOS 1
  #else
    #define     JUCE_MAC 1
  #endif
#elif defined (__wasm__)
  #define       JUCE_WASM 1
#else
  #error "Unknown platform!"
#endif

//==============================================================================
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
    #error "MinGW is not supported. Please use an alternative compiler."
  #endif

  /** If defined, this indicates that the processor is little-endian. */
  #define JUCE_LITTLE_ENDIAN 1

  #if defined (_M_ARM) || defined (_M_ARM64) || defined (_M_ARM64EC) || defined (__arm__) || defined (__aarch64__)
    #define JUCE_ARM 1
  #else
    #define JUCE_INTEL 1
  #endif
#endif

//==============================================================================
#if JUCE_MAC || JUCE_IOS

// Expands to true if the API of the specified version is available at build time, false otherwise
#define JUCE_MAC_API_VERSION_CAN_BE_BUILT(major, minor) \
    ((major) * 10000 + (minor) * 100 <= MAC_OS_X_VERSION_MAX_ALLOWED)

// Expands to true if the API of the specified version is available at build time, false otherwise
#define JUCE_IOS_API_VERSION_CAN_BE_BUILT(major, minor) \
    ((major) * 10000 + (minor) * 100 <= __IPHONE_OS_VERSION_MAX_ALLOWED)

// Expands to true if the deployment target is greater or equal to the specified version, false otherwise
#define JUCE_MAC_API_VERSION_MIN_REQUIRED_AT_LEAST(major, minor) \
    ((major) * 10000 + (minor) * 100 <= MAC_OS_X_VERSION_MIN_REQUIRED)

// Expands to true if the deployment target is greater or equal to the specified version, false otherwise
#define JUCE_IOS_API_VERSION_MIN_REQUIRED_AT_LEAST(major, minor) \
    ((major) * 10000 + (minor) * 100 <= __IPHONE_OS_VERSION_MIN_REQUIRED)

#if defined (DEBUG) || defined (_DEBUG) || ! (defined (NDEBUG) || defined (_NDEBUG))
    #define JUCE_DEBUG 1
  #endif

  #if ! (defined (DEBUG) || defined (_DEBUG) || defined (NDEBUG) || defined (_NDEBUG))
    #warning "Neither NDEBUG or DEBUG has been defined - you should set one of these to make it clear whether this is a release build,"
  #endif

  #ifdef __LITTLE_ENDIAN__
    #define JUCE_LITTLE_ENDIAN 1
  #else
    #define JUCE_BIG_ENDIAN 1
  #endif

  #ifdef __LP64__
    #define JUCE_64BIT 1
  #else
    #define JUCE_32BIT 1
  #endif

  #if defined (__ppc__) || defined (__ppc64__)
    #error "PowerPC is no longer supported by JUCE!"
  #elif defined (__arm__) || defined (__arm64__)
    #define JUCE_ARM 1
  #else
    #define JUCE_INTEL 1
  #endif

  #if JUCE_MAC
    #if ! JUCE_MAC_API_VERSION_CAN_BE_BUILT (11, 1)
      #error "The macOS 11.1 SDK (Xcode 12.4+) is required to build JUCE apps. You can create apps that run on macOS 10.11+ by changing the deployment target."
    #elif ! JUCE_MAC_API_VERSION_MIN_REQUIRED_AT_LEAST (10, 11)
      #error "Building for OSX 10.10 and earlier is no longer supported!"
    #endif
  #endif
#endif

//==============================================================================
#if JUCE_LINUX || JUCE_ANDROID || JUCE_BSD

  #ifdef _DEBUG
    #define JUCE_DEBUG 1
  #endif

  // Allow override for big-endian Linux platforms
  #if defined (__LITTLE_ENDIAN__) || ! defined (JUCE_BIG_ENDIAN)
    #define JUCE_LITTLE_ENDIAN 1
    #undef JUCE_BIG_ENDIAN
  #else
    #undef JUCE_LITTLE_ENDIAN
    #define JUCE_BIG_ENDIAN 1
  #endif

  #if defined (__LP64__) || defined (_LP64) || defined (__arm64__) || defined (__aarch64__)
    #define JUCE_64BIT 1
  #else
    #define JUCE_32BIT 1
  #endif

  #if defined (__arm__) || defined (__arm64__) || defined (__aarch64__)
    #define JUCE_ARM 1
  #elif __MMX__ || __SSE__ || __amd64__
    #define JUCE_INTEL 1
  #endif
#endif

//==============================================================================
// Compiler type macros.

#if defined (__clang__)
  #define JUCE_CLANG 1

#elif defined (__GNUC__)
  #define JUCE_GCC 1

#elif defined (_MSC_VER)
  #define JUCE_MSVC 1

#else
  #error unknown compiler
#endif

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

/*
   This file provides flags for compiler features that aren't supported on all platforms.
*/

//==============================================================================
// GCC
#if JUCE_GCC

 #if (__GNUC__ * 100 + __GNUC_MINOR__) < 700
  #error "JUCE requires GCC 7.0 or later"
 #endif

 #ifndef JUCE_EXCEPTIONS_DISABLED
  #if ! __EXCEPTIONS
   #define JUCE_EXCEPTIONS_DISABLED 1
  #endif
 #endif

 #define JUCE_CXX17_IS_AVAILABLE (__cplusplus >= 201703L)
 #define JUCE_CXX20_IS_AVAILABLE (__cplusplus >= 202002L)

#endif

//==============================================================================
// Clang
#if JUCE_CLANG

 #if (__clang_major__ < 6)
  #error "JUCE requires Clang 6 or later"
 #endif

 #ifndef JUCE_COMPILER_SUPPORTS_ARC
  #define JUCE_COMPILER_SUPPORTS_ARC 1
 #endif

 #ifndef JUCE_EXCEPTIONS_DISABLED
  #if ! __has_feature (cxx_exceptions)
   #define JUCE_EXCEPTIONS_DISABLED 1
  #endif
 #endif

 #if ! defined (JUCE_SILENCE_XCODE_15_LINKER_WARNING)                          \
     && defined (__apple_build_version__)                                      \
     && __apple_build_version__ >= 15000000                                    \
     && __apple_build_version__ <  15000100

  // Due to known issues, the linker in Xcode 15.0 may produce broken binaries.
  #error Please upgrade to Xcode 15.1 or higher
 #endif

 #define JUCE_CXX17_IS_AVAILABLE (__cplusplus >= 201703L)
 #define JUCE_CXX20_IS_AVAILABLE (__cplusplus >= 202002L)

#endif

//==============================================================================
// MSVC
#if JUCE_MSVC

 #if _MSC_FULL_VER < 191025017  // VS2017
   #error "JUCE requires Visual Studio 2017 or later"
 #endif

 #ifndef JUCE_EXCEPTIONS_DISABLED
  #if ! _CPPUNWIND
   #define JUCE_EXCEPTIONS_DISABLED 1
  #endif
 #endif

  #define JUCE_CXX17_IS_AVAILABLE (_MSVC_LANG >= 201703L)
  #define JUCE_CXX20_IS_AVAILABLE (_MSVC_LANG >= 202002L)
#endif

//==============================================================================
#if ! JUCE_CXX17_IS_AVAILABLE
 #error "JUCE requires C++17 or later"
#endif

//==============================================================================
// These are old flags that are now supported on all compatible build targets
/** @cond */
#define JUCE_CXX14_IS_AVAILABLE 1
#define JUCE_COMPILER_SUPPORTS_OVERRIDE_AND_FINAL 1
#define JUCE_COMPILER_SUPPORTS_VARIADIC_TEMPLATES 1
#define JUCE_COMPILER_SUPPORTS_INITIALIZER_LISTS 1
#define JUCE_COMPILER_SUPPORTS_NOEXCEPT 1
#define JUCE_DELETED_FUNCTION = delete
#define JUCE_CONSTEXPR constexpr
#define JUCE_NODISCARD [[nodiscard]]
/** @endcond */

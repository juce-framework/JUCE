/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

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

/*
   This file provides flags for compiler features that aren't supported on all platforms.
*/

//==============================================================================
// GCC
#if JUCE_GCC

 #if (__GNUC__ * 100 + __GNUC_MINOR__) < 500
  #error "JUCE requires GCC 5.0 or later"
 #endif

 #ifndef JUCE_EXCEPTIONS_DISABLED
  #if ! __EXCEPTIONS
   #define JUCE_EXCEPTIONS_DISABLED 1
  #endif
 #endif

 #define JUCE_CXX14_IS_AVAILABLE (__cplusplus >= 201402L)
 #define JUCE_CXX17_IS_AVAILABLE (__cplusplus >= 201703L)

#endif

//==============================================================================
// Clang
#if JUCE_CLANG

 #if (__clang_major__ < 3) || (__clang_major__ == 3 && __clang_minor__ < 4)
  #error "JUCE requires Clang 3.4 or later"
 #endif

 #ifndef JUCE_COMPILER_SUPPORTS_ARC
  #define JUCE_COMPILER_SUPPORTS_ARC 1
 #endif

 #ifndef JUCE_EXCEPTIONS_DISABLED
  #if ! __has_feature (cxx_exceptions)
   #define JUCE_EXCEPTIONS_DISABLED 1
  #endif
 #endif

 #define JUCE_CXX14_IS_AVAILABLE (__cplusplus >= 201402L)
 #define JUCE_CXX17_IS_AVAILABLE (__cplusplus >= 201703L)

#endif

//==============================================================================
// MSVC
#if JUCE_MSVC

 #if _MSC_FULL_VER < 190024210  // VS2015
   #error "JUCE requires Visual Studio 2015 Update 3 or later"
 #endif

 #ifndef JUCE_EXCEPTIONS_DISABLED
  #if ! _CPPUNWIND
   #define JUCE_EXCEPTIONS_DISABLED 1
  #endif
 #endif

  #define JUCE_CXX14_IS_AVAILABLE (_MSVC_LANG >= 201402L)
  #define JUCE_CXX17_IS_AVAILABLE (_MSVC_LANG >= 201703L)
#endif

//==============================================================================
#if ! JUCE_CXX14_IS_AVAILABLE
 #error "JUCE requires C++14 or later"
#endif

//==============================================================================
#ifndef DOXYGEN
 // These are old flags that are now supported on all compatible build targets
 #define JUCE_COMPILER_SUPPORTS_OVERRIDE_AND_FINAL 1
 #define JUCE_COMPILER_SUPPORTS_VARIADIC_TEMPLATES 1
 #define JUCE_COMPILER_SUPPORTS_INITIALIZER_LISTS 1
 #define JUCE_COMPILER_SUPPORTS_NOEXCEPT 1
 #define JUCE_DELETED_FUNCTION = delete
 #define JUCE_CONSTEXPR constexpr
#endif

#if JUCE_CXX17_IS_AVAILABLE
 #define JUCE_NODISCARD [[nodiscard]]
#else
 #define JUCE_NODISCARD
#endif

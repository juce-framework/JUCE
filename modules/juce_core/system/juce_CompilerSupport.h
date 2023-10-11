/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

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

 #if (__GNUC__ * 100 + __GNUC_MINOR__) < 700
  #error "JUCE requires GCC 7.0 or later"
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
     && __clang_major__ == 15                                                  \
     && __clang_minor__ == 0
  // This is a warning because builds may be usable when LTO is disabled
  #pragma GCC warning "If you are using Link Time Optimisation (LTO), the "    \
      "new linker introduced in Xcode 15 may produce a broken binary.\n"       \
      "As a workaround, add either '-Wl,-weak_reference_mismatches,weak' or "  \
      "'-Wl,-ld_classic' to your linker flags.\n"                              \
      "Once you've selected a workaround, you can add "                        \
      "JUCE_SILENCE_XCODE_15_LINKER_WARNING to your preprocessor definitions " \
      "to silence this warning."

  #if ((defined (MAC_OS_X_VERSION_MIN_REQUIRED)                                \
        && MAC_OS_X_VERSION_MIN_REQUIRED < MAC_OS_VERSION_13_0)                \
       || (defined (__IPHONE_OS_VERSION_MIN_REQUIRED)                          \
           && __IPHONE_OS_VERSION_MIN_REQUIRED < __IPHONE_15_0))
   // This is an error because the linker _will_ produce a binary that is
   // broken on older platforms
   static_assert (std::string_view (__clang_version__)
                      != std::string_view ("15.0.0 (clang-1500.0.40.1)"),
                  "The new linker introduced in Xcode 15.0 will produce "
                  "broken binaries when targeting older platforms.\n"
                  "To work around this issue, bump your deployment target to "
                  "macOS 13 or iOS 15, re-enable the old linker by adding "
                  "'-Wl,-ld_classic' to your link flags, or update to Xcode "
                  "15.1.\n"
                  "Once you've selected a workaround, you can add "
                  "JUCE_SILENCE_XCODE_15_LINKER_WARNING to your preprocessor "
                  "definitions to silence this warning.");
  #endif
 #endif

 #define JUCE_CXX14_IS_AVAILABLE (__cplusplus >= 201402L)
 #define JUCE_CXX17_IS_AVAILABLE (__cplusplus >= 201703L)

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

  #define JUCE_CXX14_IS_AVAILABLE (_MSVC_LANG >= 201402L)
  #define JUCE_CXX17_IS_AVAILABLE (_MSVC_LANG >= 201703L)
#endif

//==============================================================================
#if ! JUCE_CXX17_IS_AVAILABLE
 #error "JUCE requires C++17 or later"
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
 #define JUCE_NODISCARD [[nodiscard]]
#endif

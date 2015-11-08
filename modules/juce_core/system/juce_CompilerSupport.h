/*
  ==============================================================================

   This file is part of the juce_core module of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

   Permission to use, copy, modify, and/or distribute this software for any purpose with
   or without fee is hereby granted, provided that the above copyright notice and this
   permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD
   TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN
   NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
   DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
   IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
   CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

   ------------------------------------------------------------------------------

   NOTE! This permissive ISC license applies ONLY to files within the juce_core module!
   All other JUCE modules are covered by a dual GPL/commercial license, so if you are
   using any other modules, be sure to check that you also comply with their license.

   For more details, visit www.juce.com

  ==============================================================================
*/

#ifndef JUCE_COMPILERSUPPORT_H_INCLUDED
#define JUCE_COMPILERSUPPORT_H_INCLUDED

/* This file has some checks to see whether the compiler supports various C++11/14 features,
   When these aren't available, the code defines a few workarounds, so that we can still use
   some of the newer language features like nullptr and noexcept, even on old compilers.
*/

//==============================================================================
// GCC
#if (__cplusplus >= 201103L || defined (__GXX_EXPERIMENTAL_CXX0X__)) && (__GNUC__ * 100 + __GNUC_MINOR__) >= 405
 #define JUCE_COMPILER_SUPPORTS_NOEXCEPT 1
 #define JUCE_COMPILER_SUPPORTS_NULLPTR 1
 #define JUCE_COMPILER_SUPPORTS_MOVE_SEMANTICS 1
 #define JUCE_COMPILER_SUPPORTS_INITIALIZER_LISTS 1
 #define JUCE_COMPILER_SUPPORTS_VARIADIC_TEMPLATES 1
 #define JUCE_COMPILER_SUPPORTS_STATIC_ASSERT 1

 #if (__GNUC__ * 100 + __GNUC_MINOR__) >= 407 && ! defined (JUCE_COMPILER_SUPPORTS_OVERRIDE_AND_FINAL)
  #define JUCE_COMPILER_SUPPORTS_OVERRIDE_AND_FINAL 1
 #endif

 #if (__GNUC__ * 100 + __GNUC_MINOR__) >= 407 && ! defined (JUCE_DELETED_FUNCTION)
  #define JUCE_DELETED_FUNCTION = delete
 #endif

 #if (__GNUC__ * 100 + __GNUC_MINOR__) >= 406 && ! defined (JUCE_COMPILER_SUPPORTS_LAMBDAS)
  #define JUCE_COMPILER_SUPPORTS_LAMBDAS 1
 #endif

 #ifndef JUCE_EXCEPTIONS_DISABLED
  #if ! __EXCEPTIONS
   #define JUCE_EXCEPTIONS_DISABLED 1
  #endif
 #endif
#endif

//==============================================================================
// Clang
#if JUCE_CLANG && defined (__has_feature)
 #if __has_feature (cxx_nullptr)
  #define JUCE_COMPILER_SUPPORTS_NULLPTR 1
 #endif

 #if __has_feature (cxx_noexcept)
  #define JUCE_COMPILER_SUPPORTS_NOEXCEPT 1
 #endif

 #if __has_feature (cxx_rvalue_references)
  #define JUCE_COMPILER_SUPPORTS_MOVE_SEMANTICS 1
 #endif

 #if __has_feature (cxx_deleted_functions)
  #define JUCE_DELETED_FUNCTION = delete
 #endif

 #if __has_feature (cxx_lambdas) && (defined (_LIBCPP_VERSION) || ! (JUCE_MAC || JUCE_IOS))
  #define JUCE_COMPILER_SUPPORTS_LAMBDAS 1
 #endif

 #if __has_feature (cxx_generalized_initializers) && (defined (_LIBCPP_VERSION) || ! (JUCE_MAC || JUCE_IOS))
  #define JUCE_COMPILER_SUPPORTS_INITIALIZER_LISTS 1
 #endif

 #if __has_feature (cxx_variadic_templates)
  #define JUCE_COMPILER_SUPPORTS_VARIADIC_TEMPLATES 1
 #endif

 #if __has_feature (cxx_static_assert)
  #define JUCE_COMPILER_SUPPORTS_STATIC_ASSERT 1
 #endif

 #ifndef JUCE_COMPILER_SUPPORTS_OVERRIDE_AND_FINAL
  #define JUCE_COMPILER_SUPPORTS_OVERRIDE_AND_FINAL 1
 #endif

 #ifndef JUCE_COMPILER_SUPPORTS_ARC
  #define JUCE_COMPILER_SUPPORTS_ARC 1
 #endif

 #ifndef JUCE_EXCEPTIONS_DISABLED
  #if ! __has_feature (cxx_exceptions)
   #define JUCE_EXCEPTIONS_DISABLED 1
  #endif
 #endif

#endif

//==============================================================================
// MSVC
#ifdef _MSC_VER
 #if _MSC_VER >= 1600
  #define JUCE_COMPILER_SUPPORTS_NULLPTR 1
  #define JUCE_COMPILER_SUPPORTS_MOVE_SEMANTICS 1
  #define JUCE_COMPILER_SUPPORTS_STATIC_ASSERT 1
 #endif

 #if _MSC_VER >= 1700
  #define JUCE_COMPILER_SUPPORTS_OVERRIDE_AND_FINAL 1
  #define JUCE_COMPILER_SUPPORTS_LAMBDAS 1
 #endif

 #if _MSC_VER >= 1800
  #define JUCE_COMPILER_SUPPORTS_INITIALIZER_LISTS 1
  #define JUCE_COMPILER_SUPPORTS_VARIADIC_TEMPLATES 1
  #define JUCE_DELETED_FUNCTION = delete
 #endif

 #if _MSC_VER >= 1900
  #define JUCE_COMPILER_SUPPORTS_NOEXCEPT 1
 #endif

 #ifndef JUCE_EXCEPTIONS_DISABLED
  #if ! _CPPUNWIND
   #define JUCE_EXCEPTIONS_DISABLED 1
  #endif
 #endif
#endif

//==============================================================================
// Declare some fake versions of nullptr and noexcept, for older compilers:

#ifndef JUCE_DELETED_FUNCTION
 /** This macro can be placed after a method declaration to allow the use of
     the C++11 feature "= delete" on all compilers.
     On newer compilers that support it, it does the C++11 "= delete", but on
     older ones it's just an empty definition.
 */
 #define JUCE_DELETED_FUNCTION
#endif

#if ! DOXYGEN
 #if ! JUCE_COMPILER_SUPPORTS_NOEXCEPT
  #ifdef noexcept
   #undef noexcept
  #endif
  #define noexcept  throw()
  #if defined (_MSC_VER) && _MSC_VER > 1600
   #define _ALLOW_KEYWORD_MACROS 1 // (to stop VC2012 complaining)
  #endif
 #endif

 #if ! JUCE_COMPILER_SUPPORTS_NULLPTR
  #ifdef nullptr
   #undef nullptr
  #endif
  #define nullptr (0)
 #endif

 #if ! JUCE_COMPILER_SUPPORTS_OVERRIDE_AND_FINAL
  #undef  override
  #define override
 #endif
#endif

#endif   // JUCE_COMPILERSUPPORT_H_INCLUDED

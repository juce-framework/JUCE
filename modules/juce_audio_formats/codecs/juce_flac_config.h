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

   JUCE End User Licence Agreement: https://juce.com/legal/juce-9-licence/
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

#ifndef JUCE_USE_FLAC
 #define JUCE_USE_FLAC 1
#endif

#include <juce_core/system/juce_CompilerWarnings.h>

JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wempty-translation-unit")

#if JUCE_USE_FLAC

#if JUCE_INCLUDE_FLAC_CODE || ! defined (JUCE_INCLUDE_FLAC_CODE)

 #define PACKAGE_VERSION "1.5.0"

 #define FLAC__NO_DLL 1
 #define FLAC__HAS_OGG 0

 JUCE_BEGIN_IGNORE_WARNINGS_MSVC (4267 4127 4244 4996 4100 4701 4702 4706 4013 4133 4206 4312 4505 4365 4005 4334 181 111 6340 6308 6297 6001 6320 4245 6239 6326 6237 6031)
 #if ! JUCE_MSVC
  #define HAVE_LROUND 1
 #endif

 #if JUCE_MAC
  #define FLAC__SYS_DARWIN 1
 #endif

 #ifndef SIZE_MAX
  #define SIZE_MAX 0xffffffff
 #endif

 JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wconversion",
                                      "-Wdeprecated-register",
                                      "-Wfloat-equal",
                                      "-Wimplicit-fallthrough",
                                      "-Wlanguage-extension-token",
                                      "-Wredundant-decls",
                                      "-Wshadow",
                                      "-Wsign-conversion",
                                      "-Wswitch-default",
                                      "-Wswitch-enum",
                                      "-Wzero-as-null-pointer-constant",
                                      "-Wstatic-in-inline",
                                      "-Wconditional-uninitialized",
                                      "-Wunused-parameter",
                                      "-Wunused-variable",
                                      "-Wcast-align",
                                      "-Wpedantic")

 #if JUCE_INTEL
  #if JUCE_32BIT
   #define FLAC__CPU_IA32 1
  #endif
  #if JUCE_64BIT
   #define FLAC__CPU_X86_64 1
  #endif
  #define FLAC__HAS_X86INTRIN 1
 #endif

 #if JUCE_ARM && JUCE_64BIT
  #define FLAC__CPU_ARM64 1

  #if JUCE_USE_ARM_NEON
    #define FLAC__HAS_NEONINTRIN 1
    #define FLAC__HAS_A64NEONINTRIN 1
  #endif
 #endif

 #undef  DEBUG  // (some flac code dumps debug trace if the app defines this macro)

 #ifndef NDEBUG
  #define NDEBUG // (some flac code prints cpu info if this isn't defined)
 #endif

#endif

#endif

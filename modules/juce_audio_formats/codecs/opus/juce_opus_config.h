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

#include <juce_core/system/juce_TargetPlatform.h>

#ifndef JUCE_USE_OPUS
 #define JUCE_USE_OPUS 1
#endif

#if JUCE_USE_OPUS && (JUCE_INCLUDE_OPUS_CODE || ! defined (JUCE_INCLUDE_OPUS_CODE))

 // The standard floating-point build of libopus. The SIMD tiers that every
 // CPU of the architecture supports (NEON on 64-bit Arm, SSE and SSE2 on
 // 64-bit x86) are presumed at compile time, so no runtime CPU detection is
 // needed. The higher x86 tiers are not enabled, as their sources require
 // per-file compiler flags.
 #define OPUS_BUILD 1
 #define USE_ALLOCA 1

 #if ! JUCE_MSVC
  #define HAVE_LRINT 1
  #define HAVE_LRINTF 1
 #endif

 #if JUCE_ARM && JUCE_64BIT
  #define OPUS_ARM_MAY_HAVE_NEON_INTR 1
  #define OPUS_ARM_PRESUME_NEON_INTR 1
 #endif

 #if JUCE_INTEL && JUCE_64BIT
  #define OPUS_X86_MAY_HAVE_SSE 1
  #define OPUS_X86_PRESUME_SSE 1
  #define OPUS_X86_MAY_HAVE_SSE2 1
  #define OPUS_X86_PRESUME_SSE2 1
 #endif

#endif

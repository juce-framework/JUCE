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


#include <juce_core/system/juce_CompilerWarnings.h>
#include <juce_graphics/image_formats/juce_webp_config.h>

#if JUCE_USE_WEBP && JUCE_INCLUDE_WEBPLIB_CODE

JUCE_BEGIN_IGNORE_WARNINGS_MSVC (4127 4244 4245 4310 4701 6001 6011 6239 6286 6287 6297 6326 6385 6387)

JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wsign-conversion",
                                     "-Wimplicit-int-conversion",
                                     "-Wimplicit-int-float-conversion",
                                     "-Wfloat-conversion",
                                     "-Wcast-align",
                                     "-Wswitch-enum",
                                     "-Wconditional-uninitialized",
                                     "-Wfloat-equal",
                                     "-Wredundant-decls",
                                     "-Wpedantic",
                                     "-Woverflow",
                                     "-Wzero-as-null-pointer-constant")

// The SSE4.1 and AVX2 kernels live in juce_graphics_libwebp_dsp_dec_sse41.c
// and juce_graphics_libwebp_dsp_dec_avx2.c, which may be compiled with
// elevated instruction-set options that must not apply to the unguarded code
// in this file. Defining WEBP_HAVE_* here enables the cpuid-guarded dispatch
// to those kernels.
#if (defined(__i386__) || defined(__x86_64__) || defined(_M_IX86) || defined(_M_X64)) \
     && ! defined(_M_ARM64EC)
 #define WEBP_HAVE_SSE41
 #define WEBP_HAVE_AVX2
#endif

#include "juce_graphics/image_formats/libwebp/src/dsp/alpha_processing.c"
#include "juce_graphics/image_formats/libwebp/src/dsp/alpha_processing_neon.c"
#include "juce_graphics/image_formats/libwebp/src/dsp/alpha_processing_sse2.c"
#include "juce_graphics/image_formats/libwebp/src/dsp/cpu.c"
#include "juce_graphics/image_formats/libwebp/src/dsp/dec.c"
#include "juce_graphics/image_formats/libwebp/src/dsp/dec_clip_tables.c"
#include "juce_graphics/image_formats/libwebp/src/dsp/dec_neon.c"
#include "juce_graphics/image_formats/libwebp/src/dsp/dec_sse2.c"
#include "juce_graphics/image_formats/libwebp/src/dsp/filters.c"
#define GradientPredictor_C juce_webp_filters_neon_GradientPredictor_C
#include "juce_graphics/image_formats/libwebp/src/dsp/filters_neon.c"
#undef GradientPredictor_C
#include "juce_graphics/image_formats/libwebp/src/dsp/filters_sse2.c"
#include "juce_graphics/image_formats/libwebp/src/dsp/lossless.c"
#include "juce_graphics/image_formats/libwebp/src/dsp/lossless_neon.c"
#include "juce_graphics/image_formats/libwebp/src/dsp/lossless_sse2.c"
#include "juce_graphics/image_formats/libwebp/src/dsp/rescaler.c"
#include "juce_graphics/image_formats/libwebp/src/dsp/rescaler_neon.c"
#include "juce_graphics/image_formats/libwebp/src/dsp/rescaler_sse2.c"
#include "juce_graphics/image_formats/libwebp/src/dsp/upsampling.c"
#include "juce_graphics/image_formats/libwebp/src/dsp/upsampling_neon.c"
#include "juce_graphics/image_formats/libwebp/src/dsp/upsampling_sse2.c"
#include "juce_graphics/image_formats/libwebp/src/dsp/yuv.c"
#include "juce_graphics/image_formats/libwebp/src/dsp/yuv_neon.c"
#include "juce_graphics/image_formats/libwebp/src/dsp/yuv_sse2.c"

JUCE_END_IGNORE_WARNINGS_GCC_LIKE
JUCE_END_IGNORE_WARNINGS_MSVC

#else
 // To get around 'empty translation unit' errors
 enum { placeholder = 0 };
#endif

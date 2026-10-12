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

#include "juce_graphics/image_formats/libwebp/sharpyuv/sharpyuv.c"
#include "juce_graphics/image_formats/libwebp/sharpyuv/sharpyuv_cpu.c"
#include "juce_graphics/image_formats/libwebp/sharpyuv/sharpyuv_csp.c"
#define clip juce_webp_sharpyuv_dsp_clip
#include "juce_graphics/image_formats/libwebp/sharpyuv/sharpyuv_dsp.c"
#undef clip
#define Shift juce_webp_sharpyuv_gamma_Shift
#include "juce_graphics/image_formats/libwebp/sharpyuv/sharpyuv_gamma.c"
#undef Shift
#include "juce_graphics/image_formats/libwebp/sharpyuv/sharpyuv_neon.c"
#include "juce_graphics/image_formats/libwebp/sharpyuv/sharpyuv_sse2.c"

JUCE_END_IGNORE_WARNINGS_GCC_LIKE
JUCE_END_IGNORE_WARNINGS_MSVC

#else
 // To get around 'empty translation unit' errors
 enum { placeholder = 0 };
#endif

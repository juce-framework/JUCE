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

#include "juce_audio_formats/codecs/opus/juce_opus_config.h"
#include <juce_core/system/juce_CompilerWarnings.h>

#if JUCE_USE_OPUS && (JUCE_INCLUDE_OPUS_CODE || ! defined (JUCE_INCLUDE_OPUS_CODE))

JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wbitwise-op-parentheses",
                                     "-Wconditional-uninitialized",
                                     "-Wconversion",
                                     "-Wfloat-equal",
                                     "-Wlanguage-extension-token",
                                     "-Wlogical-op-parentheses",
                                     "-Wmissing-prototypes",
                                     "-Wshadow",
                                     "-Wshift-op-parentheses",
                                     "-Wsign-compare",
                                     "-Wsign-conversion")
JUCE_BEGIN_IGNORE_WARNINGS_MSVC (4267 4127 4244 4100 4701 4702 4013 4133 4206 4305 4189 4706 4995 4365 4456 4457 4459 6297 6011 6001 6308 6255 6386 6385 6246 6387 6263 6262 28182 4232 4245 4018 6326)

#include "juce_audio_formats/codecs/opus/opusfile/src/info.c"
#include "juce_audio_formats/codecs/opus/opusfile/src/internal.c"
#include "juce_audio_formats/codecs/opus/opusfile/src/opusfile.c"
#include "juce_audio_formats/codecs/opus/opusfile/src/stream.c"
#include "juce_audio_formats/codecs/opus/opusfile/src/http.c"

#include "juce_audio_formats/codecs/opus/libopusenc/package_version.h"

// libopusenc's bundled speex resampler
#define OUTSIDE_SPEEX 1
#define RANDOM_PREFIX libopusenc

// ogg_packer.c is a mini-fork of libogg's page writer: its static
// ogg_page_checksum_set conflicts with the declaration in ogg.h, and its
// shift_buffer with the one in opusenc.c
#define ogg_page_checksum_set libopusenc_ogg_page_checksum_set
#define shift_buffer libopusenc_shift_buffer
#include "juce_audio_formats/codecs/opus/libopusenc/src/ogg_packer.c"
#undef ogg_page_checksum_set
#undef shift_buffer
#include "juce_audio_formats/codecs/opus/libopusenc/src/opus_header.c"
#include "juce_audio_formats/codecs/opus/libopusenc/src/opusenc.c"
#include "juce_audio_formats/codecs/opus/libopusenc/src/picture.c"
#include "juce_audio_formats/codecs/opus/libopusenc/src/resample.c"
#include "juce_audio_formats/codecs/opus/libopusenc/src/unicode_support.c"

JUCE_END_IGNORE_WARNINGS_MSVC
JUCE_END_IGNORE_WARNINGS_GCC_LIKE

#else
// To get around 'empty translation unit' errors and 'has no symbols' warnings emitted by libtool
void juce_audioFormatsOpusfilePlaceholder (void);
void juce_audioFormatsOpusfilePlaceholder (void) {}
#endif

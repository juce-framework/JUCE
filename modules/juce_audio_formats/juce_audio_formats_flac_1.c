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

#include "juce_audio_formats/codecs/flac/juce_flac_config.h"

#if JUCE_USE_FLAC && (JUCE_INCLUDE_FLAC_CODE || ! defined (JUCE_INCLUDE_FLAC_CODE))

 JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wdeprecated-declarations")

 #include "juce_audio_formats/codecs/flac/libFLAC/stream_encoder.c"
 #include "juce_audio_formats/codecs/flac/libFLAC/stream_encoder_intrin_avx2.c"
 #include "juce_audio_formats/codecs/flac/libFLAC/stream_encoder_intrin_sse2.c"
 #include "juce_audio_formats/codecs/flac/libFLAC/stream_encoder_intrin_ssse3.c"
 #include "juce_audio_formats/codecs/flac/libFLAC/fixed_intrin_avx2.c"
 #include "juce_audio_formats/codecs/flac/libFLAC/fixed_intrin_sse2.c"
 #include "juce_audio_formats/codecs/flac/libFLAC/fixed_intrin_sse42.c"
 #include "juce_audio_formats/codecs/flac/libFLAC/fixed_intrin_ssse3.c"

 #ifdef _WIN32
  #include "juce_audio_formats/codecs/flac/win_utf8_io/win_utf8_io.c"
 #endif
#else
 // To get around 'empty translation unit' errors and 'has no symbols' warnings emitted by libtool
 void juce_audioFormatsFlac1Placeholder (void);
 void juce_audioFormatsFlac1Placeholder (void) {}
#endif

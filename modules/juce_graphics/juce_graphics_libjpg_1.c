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

#if ! defined (JUCE_INCLUDE_JPEGLIB_CODE) || JUCE_INCLUDE_JPEGLIB_CODE

JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wsign-conversion",
                                     "-Wimplicit-int-conversion",
                                     "-Wunused-parameter",
                                     "-Wswitch-enum",
                                     "-Wdeprecated-declarations",
                                     "-Wcast-align")

JUCE_BEGIN_IGNORE_WARNINGS_MSVC (4100 4127 4996 6001 6011 6326 6385 6386 6387 28183)

#include "juce_graphics/image_formats/jpglib/jcapimin.c"
#include "juce_graphics/image_formats/jpglib/jcapistd.c"
#include "juce_graphics/image_formats/jpglib/jccoefct.c"
#include "juce_graphics/image_formats/jpglib/jcinit.c"
#include "juce_graphics/image_formats/jpglib/jcmainct.c"
#include "juce_graphics/image_formats/jpglib/jcmarker.c"
#include "juce_graphics/image_formats/jpglib/jcmaster.c"
#include "juce_graphics/image_formats/jpglib/jcomapi.c"
#include "juce_graphics/image_formats/jpglib/jcparam.c"
#include "juce_graphics/image_formats/jpglib/jcprepct.c"
#include "juce_graphics/image_formats/jpglib/jcsample.c"
#include "juce_graphics/image_formats/jpglib/jmemmgr.c"
#include "juce_graphics/image_formats/jpglib/jmemnobs.c"
#include "juce_graphics/image_formats/jpglib/jquant2.c"
#include "juce_graphics/image_formats/jpglib/jutils.c"

#define initial_setup dedup_initial_setup
#define per_scan_setup dedup_per_scan_setup
#include "juce_graphics/image_formats/jpglib/jdinput.c"
#undef initial_setup
#undef per_scan_setup

#define emit_byte dedup_emit_byte
#include "juce_graphics/image_formats/jpglib/jcarith.c"
#undef emit_byte

#define encode_mcu_DC_first dedup_encode_mcu_DC_first
#define encode_mcu_AC_first dedup_encode_mcu_AC_first
#define encode_mcu_DC_refine dedup_encode_mcu_DC_refine
#define encode_mcu_AC_refine dedup_encode_mcu_AC_refine
#include "juce_graphics/image_formats/jpglib/jchuff.c"
#undef encode_mcu_DC_first
#undef encode_mcu_AC_first
#undef encode_mcu_DC_refine
#undef encode_mcu_AC_refine

#include "juce_graphics/image_formats/jpglib/jaricom.c"

#undef CONST_BITS
#include "juce_graphics/image_formats/jpglib/jfdctint.c"
#include "juce_graphics/image_formats/jpglib/jfdctflt.c"

#undef CONST_BITS
#undef FIX_0_541196100
#undef MULTIPLY
#include "juce_graphics/image_formats/jpglib/jfdctfst.c"

#define savable_state dedup_savable_state
#define huff_entropy_ptr dedup_huff_entryopy_ptr
#define finish_pass_huff dedup_finish_pass_huff
#include "juce_graphics/image_formats/jpglib/jdhuff.c"
#undef savable_state
#undef huff_entropy_ptr
#undef finish_pass_huff

#endif

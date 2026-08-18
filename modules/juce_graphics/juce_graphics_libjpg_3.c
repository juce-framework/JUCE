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
                                     "-Wimplicit-fallthrough")

JUCE_BEGIN_IGNORE_WARNINGS_MSVC (4100 4996 6385 28182)

#include "juce_graphics/image_formats/jpglib/jcdctmgr.c"
#include "juce_graphics/image_formats/jpglib/jddctmgr.c"
#include "juce_graphics/image_formats/jpglib/jidctflt.c"

#undef CONST_BITS
#undef DEQUANTIZE
#include "juce_graphics/image_formats/jpglib/jidctfst.c"

#undef CONST_BITS
#undef DEQUANTIZE
#undef FIX_1_847759065
#undef MULTIPLY
#include "juce_graphics/image_formats/jpglib/jidctint.c"

#undef FIX
#include "juce_graphics/image_formats/jpglib/jdmerge.c"

#undef FIX
#include "juce_graphics/image_formats/jpglib/jccolor.c"

#endif

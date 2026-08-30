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

#if JUCE_INCLUDE_PNGLIB_CODE || ! defined (JUCE_INCLUDE_PNGLIB_CODE)

JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wsign-conversion",
                                     "-Wfloat-equal",
                                     "-Wshadow",
                                     "-Wshorten-64-to-32",
                                     "-Wdeprecated-declarations",
                                     "-Wredundant-decls")

JUCE_BEGIN_IGNORE_WARNINGS_MSVC (4146 4996)

#include "juce_graphics/image_formats/pnglib/png.c"
#include "juce_graphics/image_formats/pnglib/pngerror.c"
#include "juce_graphics/image_formats/pnglib/pngget.c"
#include "juce_graphics/image_formats/pnglib/pngmem.c"
#include "juce_graphics/image_formats/pnglib/pngread.c"
#include "juce_graphics/image_formats/pnglib/pngpread.c"
#include "juce_graphics/image_formats/pnglib/pngrio.c"
#include "juce_graphics/image_formats/pnglib/pngrtran.c"

#define png_pass_start png_pass_start_2
#define png_pass_inc png_pass_inc_2
#define png_pass_ystart png_pass_ystart_2
#define png_pass_yinc png_pass_yinc_2

#include "juce_graphics/image_formats/pnglib/pngrutil.c"

#undef png_pass_start
#undef png_pass_inc
#undef png_pass_ystart
#undef png_pass_yinc

#include "juce_graphics/image_formats/pnglib/pngset.c"
#include "juce_graphics/image_formats/pnglib/pngtrans.c"
#include "juce_graphics/image_formats/pnglib/pngwio.c"
#include "juce_graphics/image_formats/pnglib/pngwrite.c"
#include "juce_graphics/image_formats/pnglib/pngwtran.c"

#define png_pass_start png_pass_start_3
#define png_pass_inc png_pass_inc_3
#define png_pass_ystart png_pass_ystart_3
#define png_pass_yinc png_pass_yinc_3

#include "juce_graphics/image_formats/pnglib/pngwutil.c"

#undef png_pass_start
#undef png_pass_inc
#undef png_pass_ystart
#undef png_pass_yinc

#include "juce_graphics/image_formats/pnglib/arm/arm_init.c"
#include "juce_graphics/image_formats/pnglib/arm/filter_neon_intrinsics.c"
#include "juce_graphics/image_formats/pnglib/arm/palette_neon_intrinsics.c"
#include "juce_graphics/image_formats/pnglib/powerpc/powerpc_init.c"
#include "juce_graphics/image_formats/pnglib/powerpc/filter_vsx_intrinsics.c"

#endif

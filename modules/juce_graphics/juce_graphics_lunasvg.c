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

#define JUCE_PLUTOVG_BUILD
#define JUCE_PLUTOVG_BUILD_STATIC

JUCE_BEGIN_IGNORE_WARNINGS_MSVC (4018 4100 4244 4267 4324 4389 4996 6011 6308 6387 6385 6386 6262 28182)
JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wcast-align",
                                     "-Wfloat-conversion",
                                     "-Wfloat-equal",
                                     "-Wimplicit-int-conversion",
                                     "-Wmissing-prototypes",
                                     "-Wshorten-64-to-32",
                                     "-Wsign-compare",
                                     "-Wsign-conversion",
                                     "-Wswitch-enum",
                                     "-Wunused-function",
                                     "-Wunused-parameter",
                                     "-Wcomma",
                                     "-Wimplicit-int-float-conversion",
                                     "-Wunused-but-set-variable",
                                     "-Wdeprecated-declarations",
                                     "-Wimplicit-fallthrough")

#include <juce_graphics/drawables/lunasvg/plutovg/source/plutovg-blend.c>
#include <juce_graphics/drawables/lunasvg/plutovg/source/plutovg-canvas.c>
#include <juce_graphics/drawables/lunasvg/plutovg/source/plutovg-font.c>
#include <juce_graphics/drawables/lunasvg/plutovg/source/plutovg-ft-math.c>
#include <juce_graphics/drawables/lunasvg/plutovg/source/plutovg-ft-raster.c>
#include <juce_graphics/drawables/lunasvg/plutovg/source/plutovg-ft-stroker.c>
#include <juce_graphics/drawables/lunasvg/plutovg/source/plutovg-matrix.c>
#include <juce_graphics/drawables/lunasvg/plutovg/source/plutovg-paint.c>
#include <juce_graphics/drawables/lunasvg/plutovg/source/plutovg-path.c>
#include <juce_graphics/drawables/lunasvg/plutovg/source/plutovg-rasterize.c>
#include <juce_graphics/drawables/lunasvg/plutovg/source/plutovg-surface.c>

JUCE_END_IGNORE_WARNINGS_GCC_LIKE
JUCE_END_IGNORE_WARNINGS_MSVC

#undef JUCE_PLUTOVG_BUILD_STATIC
#undef JUCE_PLUTOVG_BUILD

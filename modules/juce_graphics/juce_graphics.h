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

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
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


/*******************************************************************************
 The block below describes the properties of this module, and is read by
 the Projucer to automatically generate project code that uses it.
 For details about the syntax and how to create or use a module, see the
 JUCE Module Format.md file.


 BEGIN_JUCE_MODULE_DECLARATION

  ID:                 juce_graphics
  vendor:             juce
  version:            8.0.10
  name:               JUCE graphics classes
  description:        Classes for 2D vector graphics, image loading/saving, font handling, etc.
  website:            http://www.juce.com/juce
  license:            AGPLv3/Commercial
  minimumCppStandard: 17

  dependencies:       juce_events
  OSXFrameworks:      Cocoa QuartzCore
  iOSFrameworks:      CoreGraphics CoreImage CoreText QuartzCore
  linuxPackages:      freetype2 fontconfig

 END_JUCE_MODULE_DECLARATION

*******************************************************************************/


#pragma once
#define JUCE_GRAPHICS_H_INCLUDED

#include <juce_core/juce_core.h>
#include <juce_events/juce_events.h>

//==============================================================================
/** Config: JUCE_USE_COREIMAGE_LOADER

    On OSX, enabling this flag means that the CoreImage codecs will be used to load
    PNG/JPEG/GIF files. It is enabled by default, but you may want to disable it if
    you'd rather use libpng, libjpeg, etc.
*/
#ifndef JUCE_USE_COREIMAGE_LOADER
 #define JUCE_USE_COREIMAGE_LOADER 1
#endif

/** Config: JUCE_DISABLE_COREGRAPHICS_FONT_SMOOTHING

    Setting this flag will turn off CoreGraphics font smoothing on macOS, which some people
    find makes the text too 'fat' for their taste.
*/
#ifndef JUCE_DISABLE_COREGRAPHICS_FONT_SMOOTHING
 #define JUCE_DISABLE_COREGRAPHICS_FONT_SMOOTHING 0
#endif

#ifndef JUCE_INCLUDE_PNGLIB_CODE
 #define JUCE_INCLUDE_PNGLIB_CODE 1
#endif

#ifndef JUCE_INCLUDE_JPEGLIB_CODE
 #define JUCE_INCLUDE_JPEGLIB_CODE 1
#endif

#ifndef USE_COREGRAPHICS_RENDERING
 #define USE_COREGRAPHICS_RENDERING 1
#endif

//==============================================================================
namespace juce
{
    class Image;
    class AffineTransform;
    class Path;
    class Font;
    class Graphics;
    class FillType;
    class LowLevelGraphicsContext;
}

#include "geometry/juce_AffineTransform.h"
#include "geometry/juce_Point.h"
#include "geometry/juce_Line.h"
#include "geometry/juce_Rectangle.h"
#include "geometry/juce_Parallelogram.h"
#include "placement/juce_Justification.h"
#include "geometry/juce_Path.h"
#include "geometry/juce_RectangleList.h"
#include "colour/juce_PixelFormats.h"
#include "colour/juce_Colour.h"
#include "colour/juce_ColourGradient.h"
#include "colour/juce_Colours.h"
#include "geometry/juce_BorderSize.h"
#include "geometry/juce_EdgeTable.h"
#include "geometry/juce_PathIterator.h"
#include "geometry/juce_PathStrokeType.h"
#include "placement/juce_RectanglePlacement.h"
#include "images/juce_ImageCache.h"
#include "images/juce_ImageConvolutionKernel.h"
#include "images/juce_ImageFileFormat.h"
#include "fonts/juce_GlyphArrangementOptions.h"
#include "contexts/juce_GraphicsContext.h"
#include "images/juce_Image.h"
#include "colour/juce_FillType.h"
#include "fonts/juce_FontFeatures.h"
#include "fonts/juce_Typeface.h"
#include "fonts/juce_FontOptions.h"
#include "fonts/juce_Font.h"
#include "detail/juce_Ranges.h"
#include "detail/juce_SimpleShapedText.h"
#include "detail/juce_JustifiedText.h"
#include "detail/juce_ShapedText.h"
#include "fonts/juce_AttributedString.h"
#include "fonts/juce_GlyphArrangement.h"
#include "fonts/juce_TextLayout.h"
#include "contexts/juce_LowLevelGraphicsContext.h"
#include "images/juce_ScaledImage.h"
#include "native/juce_RenderingHelpers.h"
#include "contexts/juce_LowLevelGraphicsSoftwareRenderer.h"
#include "effects/juce_ImageEffectFilter.h"
#include "effects/juce_DropShadowEffect.h"
#include "effects/juce_GlowEffect.h"
#include "detail/juce_Unicode.h"

#if JUCE_GRAPHICS_INCLUDE_COREGRAPHICS_HELPERS && (JUCE_MAC || JUCE_IOS)
 #include "native/juce_CoreGraphicsHelpers_mac.h"
 #include "native/juce_CoreGraphicsContext_mac.h"
#endif

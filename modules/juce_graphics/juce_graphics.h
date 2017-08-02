/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

/*******************************************************************************
 The block below describes the properties of this module, and is read by
 the Projucer to automatically generate project code that uses it.
 For details about the syntax and how to create or use a module, see the
 JUCE Module Format.txt file.


 BEGIN_JUCE_MODULE_DECLARATION

  ID:               juce_graphics
  vendor:           juce
  version:          5.1.1
  name:             JUCE graphics classes
  description:      Classes for 2D vector graphics, image loading/saving, font handling, etc.
  website:          http://www.juce.com/juce
  license:          GPL/Commercial

  dependencies:     juce_events
  OSXFrameworks:    Cocoa QuartzCore
  iOSFrameworks:    CoreGraphics CoreImage CoreText QuartzCore
  linuxPackages:    x11 xinerama xext freetype2

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

/** Config: JUCE_USE_DIRECTWRITE

    Enabling this flag means that DirectWrite will be used when available for font
    management and layout.
*/
#ifndef JUCE_USE_DIRECTWRITE
 #define JUCE_USE_DIRECTWRITE 1
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

#include "geometry/juce_AffineTransform.h"
#include "geometry/juce_Point.h"
#include "geometry/juce_Line.h"
#include "geometry/juce_Rectangle.h"
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
#include "fonts/juce_Typeface.h"
#include "fonts/juce_Font.h"
#include "fonts/juce_AttributedString.h"
#include "fonts/juce_GlyphArrangement.h"
#include "fonts/juce_TextLayout.h"
#include "fonts/juce_CustomTypeface.h"
#include "contexts/juce_GraphicsContext.h"
#include "contexts/juce_LowLevelGraphicsContext.h"
#include "images/juce_Image.h"
#include "colour/juce_FillType.h"
#include "native/juce_RenderingHelpers.h"
#include "contexts/juce_LowLevelGraphicsSoftwareRenderer.h"
#include "contexts/juce_LowLevelGraphicsPostScriptRenderer.h"
#include "effects/juce_ImageEffectFilter.h"
#include "effects/juce_DropShadowEffect.h"
#include "effects/juce_GlowEffect.h"

#if JUCE_GRAPHICS_INCLUDE_COREGRAPHICS_HELPERS && (JUCE_MAC || JUCE_IOS)
 #include "native/juce_mac_CoreGraphicsHelpers.h"
 #include "native/juce_mac_CoreGraphicsContext.h"
#endif

#if JUCE_DIRECT2D && JUCE_WINDOWS
#include "native/juce_win32_Direct2DGraphicsContext.h"
#endif

}

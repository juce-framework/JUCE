/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

#ifndef JUCE_GRAPHICS_H_INCLUDED // %%
#define JUCE_GRAPHICS_H_INCLUDED

#include "../juce_core/juce_core.h"
#include "../juce_events/juce_events.h"

//=============================================================================
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

//=============================================================================
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

}

#endif   // JUCE_GRAPHICS_H_INCLUDED

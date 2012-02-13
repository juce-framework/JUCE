/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

#ifndef __JUCE_GRAPHICS_MODULE_JUCEHEADER__ // %%
#define __JUCE_GRAPHICS_MODULE_JUCEHEADER__

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

// START_AUTOINCLUDE colour, geometry, placement, contexts, images,
// image_formats, fonts, effects
#ifndef __JUCE_COLOUR_JUCEHEADER__
 #include "colour/juce_Colour.h"
#endif
#ifndef __JUCE_COLOURGRADIENT_JUCEHEADER__
 #include "colour/juce_ColourGradient.h"
#endif
#ifndef __JUCE_COLOURS_JUCEHEADER__
 #include "colour/juce_Colours.h"
#endif
#ifndef __JUCE_FILLTYPE_JUCEHEADER__
 #include "colour/juce_FillType.h"
#endif
#ifndef __JUCE_PIXELFORMATS_JUCEHEADER__
 #include "colour/juce_PixelFormats.h"
#endif
#ifndef __JUCE_AFFINETRANSFORM_JUCEHEADER__
 #include "geometry/juce_AffineTransform.h"
#endif
#ifndef __JUCE_BORDERSIZE_JUCEHEADER__
 #include "geometry/juce_BorderSize.h"
#endif
#ifndef __JUCE_EDGETABLE_JUCEHEADER__
 #include "geometry/juce_EdgeTable.h"
#endif
#ifndef __JUCE_LINE_JUCEHEADER__
 #include "geometry/juce_Line.h"
#endif
#ifndef __JUCE_PATH_JUCEHEADER__
 #include "geometry/juce_Path.h"
#endif
#ifndef __JUCE_PATHITERATOR_JUCEHEADER__
 #include "geometry/juce_PathIterator.h"
#endif
#ifndef __JUCE_PATHSTROKETYPE_JUCEHEADER__
 #include "geometry/juce_PathStrokeType.h"
#endif
#ifndef __JUCE_POINT_JUCEHEADER__
 #include "geometry/juce_Point.h"
#endif
#ifndef __JUCE_RECTANGLE_JUCEHEADER__
 #include "geometry/juce_Rectangle.h"
#endif
#ifndef __JUCE_RECTANGLELIST_JUCEHEADER__
 #include "geometry/juce_RectangleList.h"
#endif
#ifndef __JUCE_JUSTIFICATION_JUCEHEADER__
 #include "placement/juce_Justification.h"
#endif
#ifndef __JUCE_RECTANGLEPLACEMENT_JUCEHEADER__
 #include "placement/juce_RectanglePlacement.h"
#endif
#ifndef __JUCE_GRAPHICSCONTEXT_JUCEHEADER__
 #include "contexts/juce_GraphicsContext.h"
#endif
#ifndef __JUCE_LOWLEVELGRAPHICSCONTEXT_JUCEHEADER__
 #include "contexts/juce_LowLevelGraphicsContext.h"
#endif
#ifndef __JUCE_LOWLEVELGRAPHICSPOSTSCRIPTRENDERER_JUCEHEADER__
 #include "contexts/juce_LowLevelGraphicsPostScriptRenderer.h"
#endif
#ifndef __JUCE_LOWLEVELGRAPHICSSOFTWARERENDERER_JUCEHEADER__
 #include "contexts/juce_LowLevelGraphicsSoftwareRenderer.h"
#endif
#ifndef __JUCE_IMAGE_JUCEHEADER__
 #include "images/juce_Image.h"
#endif
#ifndef __JUCE_IMAGECACHE_JUCEHEADER__
 #include "images/juce_ImageCache.h"
#endif
#ifndef __JUCE_IMAGECONVOLUTIONKERNEL_JUCEHEADER__
 #include "images/juce_ImageConvolutionKernel.h"
#endif
#ifndef __JUCE_IMAGEFILEFORMAT_JUCEHEADER__
 #include "images/juce_ImageFileFormat.h"
#endif
#ifndef __JUCE_ATTRIBUTEDSTRING_JUCEHEADER__
 #include "fonts/juce_AttributedString.h"
#endif
#ifndef __JUCE_CUSTOMTYPEFACE_JUCEHEADER__
 #include "fonts/juce_CustomTypeface.h"
#endif
#ifndef __JUCE_FONT_JUCEHEADER__
 #include "fonts/juce_Font.h"
#endif
#ifndef __JUCE_GLYPHARRANGEMENT_JUCEHEADER__
 #include "fonts/juce_GlyphArrangement.h"
#endif
#ifndef __JUCE_TEXTLAYOUT_JUCEHEADER__
 #include "fonts/juce_TextLayout.h"
#endif
#ifndef __JUCE_TYPEFACE_JUCEHEADER__
 #include "fonts/juce_Typeface.h"
#endif
#ifndef __JUCE_DROPSHADOWEFFECT_JUCEHEADER__
 #include "effects/juce_DropShadowEffect.h"
#endif
#ifndef __JUCE_GLOWEFFECT_JUCEHEADER__
 #include "effects/juce_GlowEffect.h"
#endif
#ifndef __JUCE_IMAGEEFFECTFILTER_JUCEHEADER__
 #include "effects/juce_ImageEffectFilter.h"
#endif
// END_AUTOINCLUDE

}

#endif   // __JUCE_GRAPHICS_JUCEHEADER__

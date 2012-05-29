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

#if defined (__JUCE_GRAPHICS_MODULE_JUCEHEADER__) && ! JUCE_AMALGAMATED_INCLUDE
 /* When you add this cpp file to your project, you mustn't include it in a file where you've
    already included any other headers - just put it inside a file on its own, possibly with your config
    flags preceding it, but don't include anything else. That also includes avoiding any automatic prefix
    header files that the compiler may be using.
 */
 #error "Incorrect use of JUCE cpp file"
#endif

// Your project must contain an AppConfig.h file with your project-specific settings in it,
// and your header search path must make it accessible to the module's files.
#include "AppConfig.h"

#include "../juce_core/native/juce_BasicNativeHeaders.h"
#include "juce_graphics.h"

//==============================================================================
#if JUCE_MAC
 #import <QuartzCore/QuartzCore.h>

#elif JUCE_WINDOWS
 #if JUCE_USE_DIRECTWRITE
  /* If you hit a compile error trying to include these files, you may need to update
     your version of the Windows SDK to the latest one. The DirectWrite and Direct2D
     headers are in the version 7 SDKs.
  */
  #include <d2d1.h>
  #include <dwrite.h>
 #endif

#elif JUCE_IOS
 #import <QuartzCore/QuartzCore.h>
 #import <CoreText/CoreText.h>

 #if __IPHONE_OS_VERSION_MIN_REQUIRED < __IPHONE_3_2
  #error "JUCE no longer supports targets earlier than iOS 3.2"
 #endif

#elif JUCE_LINUX
 #include <ft2build.h>
 #include FT_FREETYPE_H
 #undef SIZEOF
#endif

#if (JUCE_MAC || JUCE_IOS) && USE_COREGRAPHICS_RENDERING && JUCE_USE_COREIMAGE_LOADER
 #define JUCE_USING_COREIMAGE_LOADER 1
#else
 #define JUCE_USING_COREIMAGE_LOADER 0
#endif

//==============================================================================
namespace juce
{

#include "colour/juce_Colour.cpp"
#include "colour/juce_ColourGradient.cpp"
#include "colour/juce_Colours.cpp"
#include "colour/juce_FillType.cpp"
#include "geometry/juce_AffineTransform.cpp"
#include "geometry/juce_EdgeTable.cpp"
#include "geometry/juce_Path.cpp"
#include "geometry/juce_PathIterator.cpp"
#include "geometry/juce_PathStrokeType.cpp"
#include "geometry/juce_RectangleList.cpp"
#include "placement/juce_Justification.cpp"
#include "placement/juce_RectanglePlacement.cpp"
#include "contexts/juce_GraphicsContext.cpp"
#include "contexts/juce_LowLevelGraphicsPostScriptRenderer.cpp"
#include "contexts/juce_LowLevelGraphicsSoftwareRenderer.cpp"
#include "images/juce_Image.cpp"
#include "images/juce_ImageCache.cpp"
#include "images/juce_ImageConvolutionKernel.cpp"
#include "images/juce_ImageFileFormat.cpp"
#include "image_formats/juce_GIFLoader.cpp"
#include "image_formats/juce_JPEGLoader.cpp"
#include "image_formats/juce_PNGLoader.cpp"
#include "fonts/juce_AttributedString.cpp"
#include "fonts/juce_Typeface.cpp"
#include "fonts/juce_CustomTypeface.cpp"
#include "fonts/juce_Font.cpp"
#include "fonts/juce_GlyphArrangement.cpp"
#include "fonts/juce_TextLayout.cpp"
#include "effects/juce_DropShadowEffect.cpp"
#include "effects/juce_GlowEffect.cpp"

//==============================================================================
#if JUCE_MAC || JUCE_IOS
 #include "../juce_core/native/juce_osx_ObjCHelpers.h"
 #include "native/juce_mac_CoreGraphicsHelpers.h"
 #include "native/juce_mac_Fonts.mm"
 #include "native/juce_mac_CoreGraphicsContext.mm"

#elif JUCE_WINDOWS
 #include "../juce_core/native/juce_win32_ComSmartPtr.h"
 #include "native/juce_win32_DirectWriteTypeface.cpp"
 #include "native/juce_win32_DirectWriteTypeLayout.cpp"
 #include "native/juce_win32_Fonts.cpp"
 #if JUCE_DIRECT2D
  #include "native/juce_win32_Direct2DGraphicsContext.cpp"
 #endif

#elif JUCE_LINUX
 #include "native/juce_linux_Fonts.cpp"

#elif JUCE_ANDROID
 #include "../juce_core/native/juce_android_JNIHelpers.h"
 #include "native/juce_android_GraphicsContext.cpp"
 #include "native/juce_android_Fonts.cpp"

#endif

}

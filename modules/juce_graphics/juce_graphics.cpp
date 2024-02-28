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

#ifdef JUCE_GRAPHICS_H_INCLUDED
 /* When you add this cpp file to your project, you mustn't include it in a file where you've
    already included any other headers - just put it inside a file on its own, possibly with your config
    flags preceding it, but don't include anything else. That also includes avoiding any automatic prefix
    header files that the compiler may be using.
 */
 #error "Incorrect use of JUCE cpp file"
#endif

#define JUCE_CORE_INCLUDE_OBJC_HELPERS 1
#define JUCE_CORE_INCLUDE_COM_SMART_PTR 1
#define JUCE_CORE_INCLUDE_JNI_HELPERS 1
#define JUCE_CORE_INCLUDE_NATIVE_HEADERS 1
#define JUCE_GRAPHICS_INCLUDE_COREGRAPHICS_HELPERS 1

#include "juce_graphics.h"

//==============================================================================
#if JUCE_MAC
 #import <QuartzCore/QuartzCore.h>
 #include <CoreText/CTFont.h>

#elif JUCE_WINDOWS
  // get rid of some warnings in Window's own headers
 JUCE_BEGIN_IGNORE_WARNINGS_MSVC (4458)

 #include <d2d1.h>
 #include <dwrite_3.h>

 #if JUCE_MINGW
  #include <malloc.h>
  #include <cstdio>
 #endif

 JUCE_END_IGNORE_WARNINGS_MSVC

#elif JUCE_IOS
 #import <QuartzCore/QuartzCore.h>
 #import <CoreText/CoreText.h>

#elif JUCE_LINUX || JUCE_BSD
 #ifndef JUCE_USE_FREETYPE
  #define JUCE_USE_FREETYPE 1
 #endif

 #ifndef JUCE_USE_FONTCONFIG
  #define JUCE_USE_FONTCONFIG 1
 #endif
#elif JUCE_ANDROID
 #include <android/font_matcher.h>
#endif

#if JUCE_USE_FREETYPE
 #include <ft2build.h>
 #include FT_FREETYPE_H
 #include FT_ADVANCES_H
#endif

#if JUCE_USE_FONTCONFIG
 #include <fontconfig/fontconfig.h>
#endif

#undef SIZEOF

#if (JUCE_MAC || JUCE_IOS) && USE_COREGRAPHICS_RENDERING && JUCE_USE_COREIMAGE_LOADER
 #define JUCE_USING_COREIMAGE_LOADER 1
#else
 #define JUCE_USING_COREIMAGE_LOADER 0
#endif

#if JUCE_USE_FREETYPE
 #include <juce_graphics/fonts/harfbuzz/hb-ft.h>
#endif

#if JUCE_WINDOWS
 #include <juce_graphics/fonts/harfbuzz/hb-directwrite.h>
#elif JUCE_MAC || JUCE_IOS
 #include <juce_graphics/fonts/harfbuzz/hb-coretext.h>
#endif

#include <juce_graphics/fonts/harfbuzz/hb-ot.h>

#if JUCE_UNIT_TESTS
 #include "fonts/juce_TypefaceTestData.cpp"
#endif

//==============================================================================
#include "fonts/juce_FunctionPointerDestructor.h"

#include "colour/juce_Colour.cpp"
#include "colour/juce_ColourGradient.cpp"
#include "colour/juce_Colours.cpp"
#include "colour/juce_FillType.cpp"
#include "geometry/juce_AffineTransform.cpp"
#include "geometry/juce_EdgeTable.cpp"
#include "geometry/juce_Path.cpp"
#include "geometry/juce_PathIterator.cpp"
#include "geometry/juce_PathStrokeType.cpp"
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
#include "fonts/juce_Font.cpp"
#include "fonts/juce_GlyphArrangement.cpp"
#include "fonts/juce_TextLayout.cpp"
#include "effects/juce_DropShadowEffect.cpp"
#include "effects/juce_GlowEffect.cpp"

#if JUCE_UNIT_TESTS
 #include "geometry/juce_Rectangle_test.cpp"
#endif

#if JUCE_USE_FREETYPE
 #include "fonts/juce_TypefaceFileCache.h"
 #include "native/juce_Fonts_freetype.cpp"
#endif

//==============================================================================
#if JUCE_MAC || JUCE_IOS
 #include "native/juce_Fonts_mac.mm"
 #include "native/juce_CoreGraphicsContext_mac.mm"
 #include "native/juce_IconHelpers_mac.cpp"

#elif JUCE_WINDOWS
 #include "native/juce_DirectWriteTypeface_windows.cpp"
 #include "native/juce_IconHelpers_windows.cpp"
 #if JUCE_DIRECT2D
  #include "native/juce_Direct2DGraphicsContext_windows.cpp"
 #endif

#elif JUCE_LINUX || JUCE_BSD
 #include "native/juce_Fonts_linux.cpp"
 #include "native/juce_IconHelpers_linux.cpp"

#elif JUCE_ANDROID
 #include "fonts/juce_TypefaceFileCache.h"
 #include "native/juce_GraphicsContext_android.cpp"
 #include "native/juce_Fonts_android.cpp"
 #include "native/juce_IconHelpers_android.cpp"

#endif

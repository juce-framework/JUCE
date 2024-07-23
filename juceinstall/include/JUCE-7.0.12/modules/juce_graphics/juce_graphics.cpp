/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

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

#elif JUCE_WINDOWS
  // get rid of some warnings in Window's own headers
 JUCE_BEGIN_IGNORE_WARNINGS_MSVC (4458)

 #if JUCE_MINGW && JUCE_USE_DIRECTWRITE
  #warning "DirectWrite not currently implemented with mingw..."
  #undef JUCE_USE_DIRECTWRITE
 #endif

 #if JUCE_USE_DIRECTWRITE || JUCE_DIRECT2D
  /*  This is a workaround for broken-by-default function definitions
      in the MinGW headers. If you're using a newer distribution of MinGW,
      then your headers may substitute the broken definitions with working definitions
      when this flag is enabled. Unfortunately, not all MinGW headers contain this
      workaround, so Direct2D remains disabled by default when building with MinGW.
  */
  #define WIDL_EXPLICIT_AGGREGATE_RETURNS 1

  /* If you hit a compile error trying to include these files, you may need to update
     your version of the Windows SDK to the latest one. The DirectWrite and Direct2D
     headers are in the version 7 SDKs.
  */
  #include <d2d1.h>
  #include <dwrite.h>
 #endif

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
#endif

#if JUCE_USE_FREETYPE
 #if JUCE_USE_FREETYPE_AMALGAMATED
  #include "native/freetype/FreeTypeAmalgam.h"
 #else
  #include <ft2build.h>
  #include FT_FREETYPE_H
 #endif
#endif

#undef SIZEOF

#if (JUCE_MAC || JUCE_IOS) && USE_COREGRAPHICS_RENDERING && JUCE_USE_COREIMAGE_LOADER
 #define JUCE_USING_COREIMAGE_LOADER 1
#else
 #define JUCE_USING_COREIMAGE_LOADER 0
#endif

//==============================================================================
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
#include "fonts/juce_CustomTypeface.cpp"
#include "fonts/juce_Font.cpp"
#include "fonts/juce_GlyphArrangement.cpp"
#include "fonts/juce_TextLayout.cpp"
#include "effects/juce_DropShadowEffect.cpp"
#include "effects/juce_GlowEffect.cpp"

#if JUCE_UNIT_TESTS
 #include "geometry/juce_Rectangle_test.cpp"
#endif

#if JUCE_USE_FREETYPE
 #include "native/juce_Fonts_freetype.cpp"
#endif

//==============================================================================
#if JUCE_MAC || JUCE_IOS
 #include "native/juce_Fonts_mac.mm"
 #include "native/juce_CoreGraphicsContext_mac.mm"
 #include "native/juce_IconHelpers_mac.cpp"

#elif JUCE_WINDOWS
 #include "native/juce_DirectWriteTypeface_windows.cpp"
 #include "native/juce_DirectWriteTypeLayout_windows.cpp"
 #include "native/juce_Fonts_windows.cpp"
 #include "native/juce_IconHelpers_windows.cpp"
 #if JUCE_DIRECT2D
  #include "native/juce_Direct2DGraphicsContext_windows.cpp"
 #endif

#elif JUCE_LINUX || JUCE_BSD
 #include "native/juce_Fonts_linux.cpp"
 #include "native/juce_IconHelpers_linux.cpp"

#elif JUCE_ANDROID
 #include "native/juce_GraphicsContext_android.cpp"
 #include "native/juce_Fonts_android.cpp"
 #include "native/juce_IconHelpers_android.cpp"

#endif

//==============================================================================
#if JUCE_USE_FREETYPE && JUCE_USE_FREETYPE_AMALGAMATED
 #undef PIXEL_MASK
 #undef ZLIB_VERSION
 #undef Z_ASCII
 #undef ZEXTERN
 #undef ZEXPORT

 extern "C"
 {
   #include "native/freetype/FreeTypeAmalgam.c"
 }
#endif

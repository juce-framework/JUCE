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
 #ifdef JUCE_MSVC
  #pragma warning (push)
  #pragma warning (disable : 4458)
 #endif

 #if JUCE_MINGW && JUCE_USE_DIRECTWRITE
  #warning "DirectWrite not currently implemented with mingw..."
  #undef JUCE_USE_DIRECTWRITE
 #endif

 #if JUCE_USE_DIRECTWRITE || JUCE_DIRECT2D
  /* If you hit a compile error trying to include these files, you may need to update
     your version of the Windows SDK to the latest one. The DirectWrite and Direct2D
     headers are in the version 7 SDKs.
  */
  #include <d2d1.h>
  #include <dwrite.h>
 #endif

 #if JUCE_MINGW
  #include <malloc.h>
 #endif

 #ifdef JUCE_MSVC
  #pragma warning (pop)
 #endif

#elif JUCE_IOS
 #import <QuartzCore/QuartzCore.h>
 #import <CoreText/CoreText.h>

 #if __IPHONE_OS_VERSION_MIN_REQUIRED < __IPHONE_3_2
  #error "JUCE no longer supports targets earlier than iOS 3.2"
 #endif

#elif JUCE_LINUX
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

#if JUCE_USE_FREETYPE
 #include "native/juce_freetype_Fonts.cpp"
#endif

//==============================================================================
#if JUCE_MAC || JUCE_IOS
 #include "native/juce_mac_Fonts.mm"
 #include "native/juce_mac_CoreGraphicsContext.mm"
 #include "native/juce_mac_IconHelpers.cpp"

#elif JUCE_WINDOWS
 #include "native/juce_win32_DirectWriteTypeface.cpp"
 #include "native/juce_win32_DirectWriteTypeLayout.cpp"
 #include "native/juce_win32_Fonts.cpp"
 #include "native/juce_win32_IconHelpers.cpp"
 #if JUCE_DIRECT2D
  #include "native/juce_win32_Direct2DGraphicsContext.cpp"
 #endif

#elif JUCE_LINUX
 #include "native/juce_linux_Fonts.cpp"
 #include "native/juce_linux_IconHelpers.cpp"

#elif JUCE_ANDROID
 #include "native/juce_android_GraphicsContext.cpp"
 #include "native/juce_android_Fonts.cpp"
 #include "native/juce_android_IconHelpers.cpp"

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

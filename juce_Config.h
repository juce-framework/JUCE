/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

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

#ifndef __JUCE_CONFIG_JUCEHEADER__
#define __JUCE_CONFIG_JUCEHEADER__

//==============================================================================
/*
    This file contains macros that enable/disable various JUCE features.
*/

//=============================================================================
/** The name of the namespace that all Juce classes and functions will be
    put inside. If this is not defined, no namespace will be used.
*/
#ifndef JUCE_NAMESPACE
  #define JUCE_NAMESPACE juce
#endif

//=============================================================================
/** JUCE_FORCE_DEBUG: Normally, JUCE_DEBUG is set to 1 or 0 based on compiler and
    project settings, but if you define this value, you can override this to force
    it to be true or false.
*/
#ifndef JUCE_FORCE_DEBUG
  //#define JUCE_FORCE_DEBUG 0
#endif

//=============================================================================
/** JUCE_LOG_ASSERTIONS: If this flag is enabled, the the jassert and jassertfalse
    macros will always use Logger::writeToLog() to write a message when an assertion happens.

    Enabling it will also leave this turned on in release builds. When it's disabled,
    however, the jassert and jassertfalse macros will not be compiled in a
    release build.

    @see jassert, jassertfalse, Logger
*/
#ifndef JUCE_LOG_ASSERTIONS
  #define JUCE_LOG_ASSERTIONS 0
#endif

//=============================================================================
/** JUCE_ASIO: Enables ASIO audio devices (MS Windows only).
    Turning this on means that you'll need to have the Steinberg ASIO SDK installed
    on your Windows build machine.

    See the comments in the ASIOAudioIODevice class's header file for more
    info about this.
*/
#ifndef JUCE_ASIO
  #define JUCE_ASIO 0
#endif

/** JUCE_WASAPI: Enables WASAPI audio devices (Windows Vista and above).
*/
#ifndef JUCE_WASAPI
  #define JUCE_WASAPI 0
#endif

/** JUCE_DIRECTSOUND: Enables DirectSound audio (MS Windows only).
*/
#ifndef JUCE_DIRECTSOUND
  #define JUCE_DIRECTSOUND 1
#endif

/** JUCE_DIRECTSHOW: Enables DirectShow media-streaming architecture (MS Windows only).
*/
#ifndef JUCE_DIRECTSHOW
  #define JUCE_DIRECTSHOW 0
#endif

/** JUCE_MEDIAFOUNDATION: Enables Media Foundation multimedia platform (Windows Vista and above).
*/
#ifndef JUCE_MEDIAFOUNDATION
  #define JUCE_MEDIAFOUNDATION 0
#endif

#if ! JUCE_WINDOWS
  #undef JUCE_DIRECTSHOW
  #undef JUCE_MEDIAFOUNDATION
#endif

/** JUCE_ALSA: Enables ALSA audio devices (Linux only). */
#ifndef JUCE_ALSA
  #define JUCE_ALSA 1
#endif

/** JUCE_JACK: Enables JACK audio devices (Linux only). */
#ifndef JUCE_JACK
  #define JUCE_JACK 0
#endif

//=============================================================================
/** JUCE_QUICKTIME: Enables the QuickTimeMovieComponent class (Mac and Windows).
    If you're building on Windows, you'll need to have the Apple QuickTime SDK
    installed, and its header files will need to be on your include path.
*/
#if ! (defined (JUCE_QUICKTIME) || JUCE_LINUX || JUCE_IOS || JUCE_ANDROID || (JUCE_WINDOWS && ! JUCE_MSVC))
  #define JUCE_QUICKTIME 0
#endif

#if (JUCE_IOS || JUCE_LINUX) && JUCE_QUICKTIME
  #undef JUCE_QUICKTIME
#endif

//=============================================================================
/** JUCE_OPENGL: Enables the OpenGLComponent class (available on all platforms).
    If you're not using OpenGL, you might want to turn this off to reduce your binary's size.
*/
#if ! (defined (JUCE_OPENGL) || JUCE_ANDROID)
  #define JUCE_OPENGL 1
#endif

/** JUCE_DIRECT2D: Enables the Windows 7 Direct2D renderer.
    If you're building on a platform older than Vista, you won't be able to compile with this feature.
*/
#ifndef JUCE_DIRECT2D
  #define JUCE_DIRECT2D 0
#endif

//=============================================================================
/** JUCE_USE_FLAC: Enables the FLAC audio codec classes (available on all platforms).
    If your app doesn't need to read FLAC files, you might want to disable this to
    reduce the size of your codebase and build time.
*/
#ifndef JUCE_USE_FLAC
  #define JUCE_USE_FLAC 1
#endif

/** JUCE_USE_OGGVORBIS: Enables the Ogg-Vorbis audio codec classes (available on all platforms).
    If your app doesn't need to read Ogg-Vorbis files, you might want to disable this to
    reduce the size of your codebase and build time.
*/
#ifndef JUCE_USE_OGGVORBIS
  #define JUCE_USE_OGGVORBIS 1
#endif

//=============================================================================
/** JUCE_USE_CDBURNER: Enables the audio CD reader code (Mac and Windows only).
    Unless you're using CD-burning, you should probably turn this flag off to
    reduce code size.
*/
#if (! defined (JUCE_USE_CDBURNER)) && ! (JUCE_WINDOWS && ! JUCE_MSVC)
  #define JUCE_USE_CDBURNER 1
#endif

/** JUCE_USE_CDREADER: Enables the audio CD reader code (Mac and Windows only).
    Unless you're using CD-reading, you should probably turn this flag off to
    reduce code size.
*/
#ifndef JUCE_USE_CDREADER
  #define JUCE_USE_CDREADER 1
#endif

//=============================================================================
/** JUCE_USE_CAMERA: Enables web-cam support using the CameraDevice class (Mac and Windows).
*/
#if (JUCE_QUICKTIME || JUCE_WINDOWS) && ! defined (JUCE_USE_CAMERA)
  #define JUCE_USE_CAMERA 0
#endif

//=============================================================================
/** JUCE_ENABLE_REPAINT_DEBUGGING: If this option is turned on, each area of the screen that
    gets repainted will flash in a random colour, so that you can check exactly how much and how
    often your components are being drawn.
*/
#ifndef JUCE_ENABLE_REPAINT_DEBUGGING
  #define JUCE_ENABLE_REPAINT_DEBUGGING 0
#endif

//=============================================================================
/** JUCE_USE_XINERAMA: Enables Xinerama multi-monitor support (Linux only).
    Unless you specifically want to disable this, it's best to leave this option turned on.
*/
#ifndef JUCE_USE_XINERAMA
  #define JUCE_USE_XINERAMA 1
#endif

/** JUCE_USE_XSHM: Enables X shared memory for faster rendering on Linux. This is best left
    turned on unless you have a good reason to disable it.
*/
#ifndef JUCE_USE_XSHM
  #define JUCE_USE_XSHM 1
#endif

/** JUCE_USE_XRENDER: Uses XRender to allow semi-transparent windowing on Linux.
*/
#ifndef JUCE_USE_XRENDER
  #define JUCE_USE_XRENDER 0
#endif

/** JUCE_USE_XCURSOR: Uses XCursor to allow ARGB cursor on Linux. This is best left turned on
    unless you have a good reason to disable it.
*/
#ifndef JUCE_USE_XCURSOR
  #define JUCE_USE_XCURSOR 1
#endif

//=============================================================================
/** JUCE_PLUGINHOST_VST: Enables the VST audio plugin hosting classes. This requires the
    Steinberg VST SDK to be installed on your machine, and should be left turned off unless
    you're building a plugin hosting app.

    @see VSTPluginFormat, AudioPluginFormat, AudioPluginFormatManager, JUCE_PLUGINHOST_AU
*/
#ifndef JUCE_PLUGINHOST_VST
  #define JUCE_PLUGINHOST_VST 0
#endif

/** JUCE_PLUGINHOST_AU: Enables the AudioUnit plugin hosting classes. This is Mac-only,
    of course, and should only be enabled if you're building a plugin hosting app.

    @see AudioUnitPluginFormat, AudioPluginFormat, AudioPluginFormatManager, JUCE_PLUGINHOST_VST
*/
#ifndef JUCE_PLUGINHOST_AU
  #define JUCE_PLUGINHOST_AU 0
#endif

//=============================================================================
/** JUCE_ONLY_BUILD_CORE_LIBRARY: Enabling this will avoid including any UI classes in the build.
    This should be enabled if you're writing a console application.
*/
#ifndef JUCE_ONLY_BUILD_CORE_LIBRARY
  #define JUCE_ONLY_BUILD_CORE_LIBRARY  0
#endif

/** JUCE_WEB_BROWSER: This lets you disable the WebBrowserComponent class (Mac and Windows).
    If you're not using any embedded web-pages, turning this off may reduce your code size.
*/
#ifndef JUCE_WEB_BROWSER
  #define JUCE_WEB_BROWSER 1
#endif


//=============================================================================
/** JUCE_SUPPORT_CARBON: Enabling this allows the Mac code to use old Carbon library functions.

    Carbon isn't required for a normal app, but may be needed by specialised classes like
    plugin-hosts, which support older APIs.
*/
#if ! (defined (JUCE_SUPPORT_CARBON) || defined (__LP64__))
  #define JUCE_SUPPORT_CARBON 1
#endif

//=============================================================================
/*  JUCE_INCLUDE_ZLIB_CODE: Can be used to disable Juce's embedded 3rd-party zlib code.
    You might need to tweak this if you're linking to an external zlib library in your app,
    but for normal apps, this option should be left alone.
*/
#ifndef JUCE_INCLUDE_ZLIB_CODE
  #define JUCE_INCLUDE_ZLIB_CODE        1
#endif

#ifndef JUCE_INCLUDE_FLAC_CODE
  #define JUCE_INCLUDE_FLAC_CODE        1
#endif

#ifndef JUCE_INCLUDE_OGGVORBIS_CODE
  #define JUCE_INCLUDE_OGGVORBIS_CODE   1
#endif

#ifndef JUCE_INCLUDE_PNGLIB_CODE
  #define JUCE_INCLUDE_PNGLIB_CODE      1
#endif

#ifndef JUCE_INCLUDE_JPEGLIB_CODE
  #define JUCE_INCLUDE_JPEGLIB_CODE     1
#endif

//=============================================================================
/** JUCE_CHECK_MEMORY_LEAKS: Enables a memory-leak check for certain objects when
    the app terminates. See the LeakedObjectDetector class and the JUCE_LEAK_DETECTOR
    macro for more details about enabling leak checking for specific classes.
*/
#if JUCE_DEBUG && ! defined (JUCE_CHECK_MEMORY_LEAKS)
  #define JUCE_CHECK_MEMORY_LEAKS 1
#endif

/** JUCE_CATCH_UNHANDLED_EXCEPTIONS: Turn on juce's internal catching of exceptions
    that are thrown by the message dispatch loop. With it enabled, any unhandled exceptions
    are passed to the JUCEApplication::unhandledException() callback for logging.
*/
#ifndef JUCE_CATCH_UNHANDLED_EXCEPTIONS
  #define JUCE_CATCH_UNHANDLED_EXCEPTIONS 1
#endif

//=============================================================================
// If only building the core classes, we can explicitly turn off some features to avoid including them:
#if JUCE_ONLY_BUILD_CORE_LIBRARY
  #undef  JUCE_QUICKTIME
  #define JUCE_QUICKTIME 0
  #undef  JUCE_OPENGL
  #define JUCE_OPENGL 0
  #undef  JUCE_USE_CDBURNER
  #define JUCE_USE_CDBURNER 0
  #undef  JUCE_USE_CDREADER
  #define JUCE_USE_CDREADER 0
  #undef  JUCE_WEB_BROWSER
  #define JUCE_WEB_BROWSER 0
  #undef  JUCE_PLUGINHOST_AU
  #define JUCE_PLUGINHOST_AU 0
  #undef  JUCE_PLUGINHOST_VST
  #define JUCE_PLUGINHOST_VST 0
#endif

#endif

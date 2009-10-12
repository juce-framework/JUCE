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
/** Normally, JUCE_DEBUG is set to 1 or 0 based on compiler and project settings,
    but if you define this value, you can override this can force it to be true or
    false.
*/
#ifndef JUCE_FORCE_DEBUG
  //#define JUCE_FORCE_DEBUG 1
#endif

//=============================================================================
/** If this flag is enabled, the the jassert and jassertfalse macros will
    always use Logger::writeToLog() to write a message when an assertion happens.

    Enabling it will also leave this turned on in release builds. When it's disabled,
    however, the jassert and jassertfalse macros will not be compiled in a
    release build.

    @see jassert, jassertfalse, Logger
*/
#ifndef JUCE_LOG_ASSERTIONS
//  #define JUCE_LOG_ASSERTIONS 1
#endif

//=============================================================================
/** Comment out this macro if you haven't got the Steinberg ASIO SDK, without
    which the ASIOAudioIODevice class can't be built. See the comments in the
    ASIOAudioIODevice class's header file for more info about this.

    (This only affects a Win32 build)
*/
#ifndef JUCE_ASIO
  #define JUCE_ASIO 1
#endif

/** Comment out this macro to disable the Windows WASAPI audio device type.
*/
#ifndef JUCE_WASAPI
//  #define JUCE_WASAPI 1
#endif

/** Comment out this macro to disable the Windows WASAPI audio device type.
*/
#ifndef JUCE_DIRECTSOUND
  #define JUCE_DIRECTSOUND 1
#endif

/** Comment out this macro to disable building of ALSA device support on Linux.
*/
#ifndef JUCE_ALSA
  #define JUCE_ALSA 1
#endif

//=============================================================================
/** Comment out this macro if you don't want to enable QuickTime or if you don't
    have the SDK installed.

    If this flag is not enabled, the QuickTimeMovieComponent and QuickTimeAudioFormat
    classes will be unavailable.

    On Windows, if you enable this, you'll need to have the QuickTime SDK
    installed, and its header files will need to be on your include path.
*/
#if ! (defined (JUCE_QUICKTIME) || JUCE_LINUX || JUCE_IPHONE || (JUCE_WINDOWS && ! JUCE_MSVC))
  #define JUCE_QUICKTIME 1
#endif


//=============================================================================
/** Comment out this macro if you don't want to enable OpenGL or if you don't
    have the appropriate headers and libraries available. If it's not enabled, the
    OpenGLComponent class will be unavailable.
*/
#ifndef JUCE_OPENGL
  #define JUCE_OPENGL 1
#endif

//=============================================================================
/** These flags enable the Ogg-Vorbis and Flac audio formats.

    If you're not going to need either of these formats, turn off the flags to
    avoid bloating your codebase with them.
*/
#ifndef JUCE_USE_FLAC
  #define JUCE_USE_FLAC 1
#endif

#ifndef JUCE_USE_OGGVORBIS
  #define JUCE_USE_OGGVORBIS 1
#endif

//=============================================================================
/** This flag lets you enable support for CD-burning. You might want to disable
    it to build without the MS SDK under windows.
*/
#if (! defined (JUCE_USE_CDBURNER)) && ! (JUCE_WINDOWS && ! JUCE_MSVC)
  #define JUCE_USE_CDBURNER 1
#endif

//=============================================================================
/** Enabling this provides support for cameras, using the CameraDevice class
*/
#if JUCE_QUICKTIME && ! defined (JUCE_USE_CAMERA)
//  #define JUCE_USE_CAMERA 1
#endif

//=============================================================================
/** Enabling this macro means that all regions that get repainted will have a coloured
    line drawn around them.

    This is handy if you're trying to optimise drawing, because it lets you easily see
    when anything is being repainted unnecessarily.
*/
#ifndef JUCE_ENABLE_REPAINT_DEBUGGING
//  #define JUCE_ENABLE_REPAINT_DEBUGGING 1
#endif

//=============================================================================
/** Enable this under Linux to use Xinerama for multi-monitor support.
*/
#ifndef JUCE_USE_XINERAMA
  #define JUCE_USE_XINERAMA 1
#endif

/** Enable this under Linux to use XShm for faster shared-memory rendering.
*/
#ifndef JUCE_USE_XSHM
  #define JUCE_USE_XSHM 1
#endif

//=============================================================================
/** Enabling this builds support for VST audio plugins.
    @see VSTPluginFormat, AudioPluginFormat, AudioPluginFormatManager, JUCE_PLUGINHOST_AU
*/
#ifndef JUCE_PLUGINHOST_VST
//  #define JUCE_PLUGINHOST_VST 1
#endif

/** Enabling this builds support for AudioUnit audio plugins.
    @see AudioUnitPluginFormat, AudioPluginFormat, AudioPluginFormatManager, JUCE_PLUGINHOST_VST
*/
#ifndef JUCE_PLUGINHOST_AU
//  #define JUCE_PLUGINHOST_AU 1
#endif

//=============================================================================
/** Enabling this will avoid including any UI code in the build. This is handy for
    writing command-line utilities, e.g. on linux boxes which don't have some
    of the UI libraries installed.
*/
#ifndef JUCE_ONLY_BUILD_CORE_LIBRARY
  //#define JUCE_ONLY_BUILD_CORE_LIBRARY  1
#endif

/** This lets you disable building of the WebBrowserComponent, if it's not required.
*/
#ifndef JUCE_WEB_BROWSER
  #define JUCE_WEB_BROWSER 1
#endif


//=============================================================================
/** Setting this allows the build to use old Carbon libraries that will be
    deprecated in newer versions of OSX. This is handy for some backwards-compatibility
    reasons.
*/
#ifndef JUCE_SUPPORT_CARBON
  #define JUCE_SUPPORT_CARBON 1
#endif

//=============================================================================
/*  These flags let you avoid the direct inclusion of some 3rd-party libs in the
    codebase - you might need to use this if you're linking to some of these libraries
    yourself.
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
/** Enable this to add extra memory-leak info to the new and delete operators.

    (Currently, this only affects Windows builds in debug mode).
*/
#ifndef JUCE_CHECK_MEMORY_LEAKS
  #define JUCE_CHECK_MEMORY_LEAKS 1
#endif

/** Enable this to turn on juce's internal catching of exceptions.

    Turning it off will avoid any exception catching. With it on, all exceptions
    are passed to the JUCEApplication::unhandledException() callback for logging.
*/
#ifndef JUCE_CATCH_UNHANDLED_EXCEPTIONS
  #define JUCE_CATCH_UNHANDLED_EXCEPTIONS 1
#endif

/** If this macro is set, the Juce String class will use unicode as its
    internal representation. If it isn't set, it'll use ANSI.
*/
#ifndef JUCE_STRINGS_ARE_UNICODE
  #define JUCE_STRINGS_ARE_UNICODE 1
#endif

//=============================================================================

#endif

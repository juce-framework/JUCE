/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

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
//  #define JUCE_ASIO 1
#endif

/** Comment out this macro to disable building of ALSA device support on Linux.
*/
#ifndef JUCE_ALSA
//  #define JUCE_ALSA 1
#endif

//=============================================================================
/** Comment out this macro if you don't want to enable QuickTime or if you don't
    have QuickTime installed. If it's not enabled, the QuickTimeWindow class will
    be unavailable.

    On Windows, if you enable this, you'll need to make sure the Apple Quicktime.dll
    file is found on your include path. By default the Quicktime installer will have
    put this in the "/Program Files/QuickTime" folder. Only QuickTime version 7 or later
    is currently supported.
*/
#if ! (defined (JUCE_QUICKTIME) || defined (LINUX) || (defined (_WIN32) && ! defined (_MSC_VER)))
//  #define JUCE_QUICKTIME 1
#endif

//=============================================================================
/** Comment out this macro if you don't want to enable OpenGL or if you don't
    have the appropriate headers and libraries available. If it's not enabled, the
    OpenGLComponent class will be unavailable.
*/
#ifndef JUCE_OPENGL
//  #define JUCE_OPENGL 1
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
/** Disabling this will avoid linking to any UI code. This is handy for
    writing command-line utilities, e.g. on linux boxes which don't have some
    of the UI libraries installed.

    (On mac and windows, this won't generally make much difference to the build).
*/
#ifndef JUCE_BUILD_GUI_CLASSES
  #define JUCE_BUILD_GUI_CLASSES 1
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

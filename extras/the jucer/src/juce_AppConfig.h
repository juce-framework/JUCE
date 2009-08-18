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

/*
    This file contains settings that you might want to explicitly apply to
    your Juce build.

    These flags enable or disable juce features - if you're linking to juce as
    a library, then to change them, you'd need to alter your juce_Config.h file and
    recompile the juce lib. But because we're using the amalgamated file, you can
    just include this file before including your juce_amalgamated.cpp file to
    have the same effect.

    If you leave any of these commented-out, they'll take on the default value
    assigned to them in juce_Config.h, so to force them on or off, just set them
    to an explicit 0 or 1 in here.
*/

//#define JUCE_ONLY_BUILD_CORE_LIBRARY   1
//#define JUCE_FORCE_DEBUG  1
//#define JUCE_LOG_ASSERTIONS  1
#define JUCE_ASIO  0
#define JUCE_ALSA  0
#define JUCE_QUICKTIME  0
#define JUCE_OPENGL  0
#define JUCE_USE_FLAC  0
#define JUCE_USE_OGGVORBIS  0
#define JUCE_USE_CDBURNER  0
//#define JUCE_ENABLE_REPAINT_DEBUGGING  1
//#define JUCE_USE_XINERAMA  1
//#define JUCE_USE_XSHM  1
#define JUCE_PLUGINHOST_VST  0
#define JUCE_PLUGINHOST_AU  0
//#define JUCE_CHECK_MEMORY_LEAKS  1
//#define JUCE_CATCH_UNHANDLED_EXCEPTIONS  1
//#define JUCE_STRINGS_ARE_UNICODE  1

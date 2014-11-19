/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

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

/*
    PLEASE NOTE! This file is just here to help transition old code to the newer
    modularised layout - but it will be removed at some point in the future, so
    you should update your projects to use the newer design as soon as possible.
*/
#ifdef _MSC_VER
 #pragma message ("The amalgamated files are now deprecated - please include juce modules directly, or preferably let the introjucer handle the inclusion of source code in your project.")
#else
 #warning "The amalgamated files are now deprecated - please include juce modules directly, or preferably let the introjucer handle the inclusion of source code in your project."
#endif

#ifdef __JUCE_JUCEHEADER__
 /* When you add the amalgamated cpp file to your project, you mustn't include it in
    a file where you've already included juce.h - just put it inside a file on its own,
    possibly with your config flags preceding it, but don't include anything else. */
 #error
#endif

#ifndef JUCE_BUILD_CORE
 #define JUCE_BUILD_CORE 1
#endif
#ifndef JUCE_BUILD_MISC
 #define JUCE_BUILD_MISC 1
#endif
#ifndef JUCE_BUILD_GUI
 #define JUCE_BUILD_GUI 1
#endif
#ifndef JUCE_BUILD_NATIVE
 #define JUCE_BUILD_NATIVE 1
#endif

#if JUCE_ONLY_BUILD_CORE_LIBRARY
 #undef JUCE_BUILD_MISC
 #undef JUCE_BUILD_GUI
 #undef JUCE_BUILD_NATIVE
#endif

#define JUCE_AMALGAMATED_INCLUDE 1

#if JUCE_BUILD_CORE
 #include "../modules/juce_core/juce_core.cpp"
#endif

#if JUCE_BUILD_MISC
 #include "../modules/juce_cryptography/juce_cryptography.cpp"
 #include "../modules/juce_data_structures/juce_data_structures.cpp"
 #include "../modules/juce_events/juce_events.cpp"
 #include "../modules/juce_graphics/juce_graphics.cpp"
#endif

#if JUCE_BUILD_NATIVE
 #include "../modules/juce_video/juce_video.cpp"
 #if JUCE_OPENGL
  #include "../modules/juce_opengl/juce_opengl.cpp"
 #endif
 #include "../modules/juce_audio_basics/juce_audio_basics.cpp"
 #include "../modules/juce_audio_formats/juce_audio_formats.cpp"
 #include "../modules/juce_audio_processors/juce_audio_processors.cpp"
 #include "../modules/juce_audio_devices/juce_audio_devices.cpp"
#endif

#if JUCE_BUILD_GUI
 #include "../modules/juce_gui_basics/juce_gui_basics.cpp"
 #include "../modules/juce_gui_extra/juce_gui_extra.cpp"
 #include "../modules/juce_audio_utils/juce_audio_utils.cpp"
#endif

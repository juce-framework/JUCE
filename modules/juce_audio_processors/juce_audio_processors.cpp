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

#ifdef JUCE_AUDIO_PROCESSORS_H_INCLUDED
 /* When you add this cpp file to your project, you mustn't include it in a file where you've
    already included any other headers - just put it inside a file on its own, possibly with your config
    flags preceding it, but don't include anything else. That also includes avoiding any automatic prefix
    header files that the compiler may be using.
 */
 #error "Incorrect use of JUCE cpp file"
#endif

#define JUCE_CORE_INCLUDE_NATIVE_HEADERS 1
#define JUCE_CORE_INCLUDE_OBJC_HELPERS 1
#define JUCE_GUI_BASICS_INCLUDE_XHEADERS 1
#define JUCE_GUI_BASICS_INCLUDE_SCOPED_THREAD_DPI_AWARENESS_SETTER 1
#define JUCE_GRAPHICS_INCLUDE_COREGRAPHICS_HELPERS 1
#define JUCE_VST3HEADERS_INCLUDE_HEADERS_ONLY 1

#include "juce_audio_processors.h"
#include <juce_gui_extra/juce_gui_extra.h>

//==============================================================================
#if JUCE_INTERNAL_HAS_VST && (JUCE_LINUX || JUCE_BSD)
 JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wvariadic-macros")
 #include <X11/Xlib.h>
 JUCE_END_IGNORE_WARNINGS_GCC_LIKE
 #include <X11/Xutil.h>
 #undef KeyPress
#endif

#if JUCE_INTERNAL_HAS_AU
 #include <AudioUnit/AudioUnit.h>
#endif

#include "format/juce_AudioPluginFormatManagerHelpers.cpp"
#include "processors/juce_AudioProcessorEditor.cpp"
#include "processors/juce_GenericAudioProcessorEditor.cpp"
#include "format_types/juce_VSTPluginFormat.cpp"
#include "format_types/juce_VST3PluginFormat.cpp"
#include "format_types/juce_AudioUnitPluginFormat.mm"
#include "scanning/juce_KnownPluginList.cpp"
#include "scanning/juce_PluginDirectoryScanner.cpp"
#include "scanning/juce_PluginListComponent.cpp"
#include "utilities/juce_ParameterAttachments.cpp"
#include "utilities/juce_AudioProcessorValueTreeState.cpp"
#include "utilities/juce_PluginHostType.cpp"

#include "format_types/juce_LV2PluginFormat.cpp"

#if JUCE_UNIT_TESTS
 #if JUCE_INTERNAL_HAS_VST3
  #include "format_types/juce_VST3PluginFormat_test.cpp"
 #endif

 #if JUCE_INTERNAL_HAS_AU
  #include "format_types/juce_AudioUnitPluginFormat_test.cpp"
 #endif

 #if JUCE_INTERNAL_HAS_LV2
  #include "format_types/juce_LV2PluginFormat_test.cpp"
 #endif
#endif

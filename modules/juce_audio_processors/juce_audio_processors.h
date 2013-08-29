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

#ifndef JUCE_AUDIO_PROCESSORS_H_INCLUDED
#define JUCE_AUDIO_PROCESSORS_H_INCLUDED

#include "../juce_gui_basics/juce_gui_basics.h"
#include "../juce_audio_basics/juce_audio_basics.h"


//=============================================================================
/** Config: JUCE_PLUGINHOST_VST
    Enables the VST audio plugin hosting classes. This requires the Steinberg VST SDK to be
    installed on your machine.

    @see VSTPluginFormat, AudioPluginFormat, AudioPluginFormatManager, JUCE_PLUGINHOST_AU
*/
#ifndef JUCE_PLUGINHOST_VST
 #define JUCE_PLUGINHOST_VST 0
#endif

/** Config: JUCE_PLUGINHOST_AU
    Enables the AudioUnit plugin hosting classes. This is Mac-only, of course.

    @see AudioUnitPluginFormat, AudioPluginFormat, AudioPluginFormatManager, JUCE_PLUGINHOST_VST
*/
#ifndef JUCE_PLUGINHOST_AU
 #define JUCE_PLUGINHOST_AU 0
#endif

#if ! (JUCE_PLUGINHOST_AU || JUCE_PLUGINHOST_VST)
// #error "You need to set either the JUCE_PLUGINHOST_AU anr/or JUCE_PLUGINHOST_VST flags if you're using this module!"
#endif

#if ! (defined (JUCE_SUPPORT_CARBON) || JUCE_64BIT)
 #define JUCE_SUPPORT_CARBON 1
#endif

//=============================================================================
//=============================================================================
namespace juce
{

class AudioProcessor;
#include "processors/juce_AudioPlayHead.h"
#include "processors/juce_AudioProcessorEditor.h"
#include "processors/juce_AudioProcessorListener.h"
#include "processors/juce_AudioProcessor.h"
#include "processors/juce_PluginDescription.h"
#include "processors/juce_AudioPluginInstance.h"
#include "processors/juce_AudioProcessorGraph.h"
#include "processors/juce_GenericAudioProcessorEditor.h"
#include "format/juce_AudioPluginFormat.h"
#include "format/juce_AudioPluginFormatManager.h"
#include "scanning/juce_KnownPluginList.h"
#include "format_types/juce_AudioUnitPluginFormat.h"
#include "format_types/juce_LADSPAPluginFormat.h"
#include "format_types/juce_VSTMidiEventList.h"
#include "format_types/juce_VSTPluginFormat.h"
#include "scanning/juce_PluginDirectoryScanner.h"
#include "scanning/juce_PluginListComponent.h"

}

#endif   // JUCE_AUDIO_PROCESSORS_H_INCLUDED

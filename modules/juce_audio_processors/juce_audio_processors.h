/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

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

#ifndef __JUCE_AUDIO_PROCESSORS_JUCEHEADER__
#define __JUCE_AUDIO_PROCESSORS_JUCEHEADER__

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
#include "../juce_core/system/juce_StandardHeader.h"

namespace juce
{

// START_AUTOINCLUDE processors, format, format_types, scanning
#ifndef __JUCE_AUDIOPLAYHEAD_JUCEHEADER__
 #include "processors/juce_AudioPlayHead.h"
#endif
#ifndef __JUCE_AUDIOPLUGININSTANCE_JUCEHEADER__
 #include "processors/juce_AudioPluginInstance.h"
#endif
#ifndef __JUCE_AUDIOPROCESSOR_JUCEHEADER__
 #include "processors/juce_AudioProcessor.h"
#endif
#ifndef __JUCE_AUDIOPROCESSOREDITOR_JUCEHEADER__
 #include "processors/juce_AudioProcessorEditor.h"
#endif
#ifndef __JUCE_AUDIOPROCESSORGRAPH_JUCEHEADER__
 #include "processors/juce_AudioProcessorGraph.h"
#endif
#ifndef __JUCE_AUDIOPROCESSORLISTENER_JUCEHEADER__
 #include "processors/juce_AudioProcessorListener.h"
#endif
#ifndef __JUCE_GENERICAUDIOPROCESSOREDITOR_JUCEHEADER__
 #include "processors/juce_GenericAudioProcessorEditor.h"
#endif
#ifndef __JUCE_PLUGINDESCRIPTION_JUCEHEADER__
 #include "processors/juce_PluginDescription.h"
#endif
#ifndef __JUCE_AUDIOPLUGINFORMAT_JUCEHEADER__
 #include "format/juce_AudioPluginFormat.h"
#endif
#ifndef __JUCE_AUDIOPLUGINFORMATMANAGER_JUCEHEADER__
 #include "format/juce_AudioPluginFormatManager.h"
#endif
#ifndef __JUCE_AUDIOUNITPLUGINFORMAT_JUCEHEADER__
 #include "format_types/juce_AudioUnitPluginFormat.h"
#endif
#ifndef __JUCE_DIRECTXPLUGINFORMAT_JUCEHEADER__
 #include "format_types/juce_DirectXPluginFormat.h"
#endif
#ifndef __JUCE_LADSPAPLUGINFORMAT_JUCEHEADER__
 #include "format_types/juce_LADSPAPluginFormat.h"
#endif
#include "format_types/juce_VSTMidiEventList.h"
#ifndef __JUCE_VSTPLUGINFORMAT_JUCEHEADER__
 #include "format_types/juce_VSTPluginFormat.h"
#endif
#ifndef __JUCE_KNOWNPLUGINLIST_JUCEHEADER__
 #include "scanning/juce_KnownPluginList.h"
#endif
#ifndef __JUCE_PLUGINDIRECTORYSCANNER_JUCEHEADER__
 #include "scanning/juce_PluginDirectoryScanner.h"
#endif
#ifndef __JUCE_PLUGINLISTCOMPONENT_JUCEHEADER__
 #include "scanning/juce_PluginListComponent.h"
#endif
// END_AUTOINCLUDE

}

#endif   // __JUCE_AUDIO_PROCESSORS_JUCEHEADER__

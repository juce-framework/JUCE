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


/*******************************************************************************
 The block below describes the properties of this module, and is read by
 the Projucer to automatically generate project code that uses it.
 For details about the syntax and how to create or use a module, see the
 JUCE Module Format.md file.


 BEGIN_JUCE_MODULE_DECLARATION

  ID:                 juce_audio_processors
  vendor:             juce
  version:            8.0.10
  name:               JUCE audio processor classes
  description:        Classes for loading and playing VST, AU, LADSPA, or internally-generated audio processors.
  website:            http://www.juce.com/juce
  license:            AGPLv3/Commercial
  minimumCppStandard: 17

  dependencies:       juce_gui_extra, juce_audio_basics
  OSXFrameworks:      CoreAudio CoreMIDI AudioToolbox
  iOSFrameworks:      AudioToolbox

 END_JUCE_MODULE_DECLARATION

*******************************************************************************/


#pragma once
#define JUCE_AUDIO_PROCESSORS_H_INCLUDED

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include <juce_audio_basics/juce_audio_basics.h>

//==============================================================================
/** Config: JUCE_PLUGINHOST_VST
    Enables the VST audio plugin hosting classes. You will need to have the VST2 SDK files in your header search paths. You can obtain the VST2 SDK files from on older version of the VST3 SDK.

    @see VSTPluginFormat, VST3PluginFormat, AudioPluginFormat, AudioPluginFormatManager, JUCE_PLUGINHOST_AU, JUCE_PLUGINHOST_VST3, JUCE_PLUGINHOST_LADSPA
*/
#ifndef JUCE_PLUGINHOST_VST
 #define JUCE_PLUGINHOST_VST 0
#endif

/** Config: JUCE_PLUGINHOST_VST3
    Enables the VST3 audio plugin hosting classes.

    @see VSTPluginFormat, VST3PluginFormat, AudioPluginFormat, AudioPluginFormatManager, JUCE_PLUGINHOST_VST, JUCE_PLUGINHOST_AU, JUCE_PLUGINHOST_LADSPA
*/
#ifndef JUCE_PLUGINHOST_VST3
 #define JUCE_PLUGINHOST_VST3 0
#endif

/** Config: JUCE_PLUGINHOST_AU
    Enables the AudioUnit plugin hosting classes. This is Mac-only, of course.

    @see AudioUnitPluginFormat, AudioPluginFormat, AudioPluginFormatManager, JUCE_PLUGINHOST_VST, JUCE_PLUGINHOST_VST3, JUCE_PLUGINHOST_LADSPA
*/
#ifndef JUCE_PLUGINHOST_AU
 #define JUCE_PLUGINHOST_AU 0
#endif

/** Config: JUCE_PLUGINHOST_LADSPA
    Enables the LADSPA plugin hosting classes. This is Linux-only, of course.

    @see LADSPAPluginFormat, AudioPluginFormat, AudioPluginFormatManager, JUCE_PLUGINHOST_VST, JUCE_PLUGINHOST_VST3, JUCE_PLUGINHOST_AU
 */
#ifndef JUCE_PLUGINHOST_LADSPA
 #define JUCE_PLUGINHOST_LADSPA 0
#endif

/** Config: JUCE_PLUGINHOST_LV2
    Enables the LV2 plugin hosting classes.
 */
#ifndef JUCE_PLUGINHOST_LV2
 #define JUCE_PLUGINHOST_LV2 0
#endif

/** Config: JUCE_PLUGINHOST_ARA
    Enables the ARA plugin extension hosting classes. You will need to download the ARA SDK and specify the
    path to it either in the Projucer, using juce_set_ara_sdk_path() in your CMake project file.

    The directory can be obtained by recursively cloning https://github.com/Celemony/ARA_SDK and checking out
    the tag releases/2.1.0.
*/
#ifndef JUCE_PLUGINHOST_ARA
 #define JUCE_PLUGINHOST_ARA 0
#endif

/** Config: JUCE_CUSTOM_VST3_SDK
    If enabled, the embedded VST3 SDK in JUCE will not be added to the project and instead you should
    add the path to your custom VST3 SDK to the project's header search paths. Most users shouldn't
    need to enable this and should just use the version of the SDK included with JUCE.
*/
#ifndef JUCE_CUSTOM_VST3_SDK
 #define JUCE_CUSTOM_VST3_SDK 0
#endif

#if ! (JUCE_PLUGINHOST_AU || JUCE_PLUGINHOST_VST || JUCE_PLUGINHOST_VST3 || JUCE_PLUGINHOST_LADSPA)
// #error "You need to set either the JUCE_PLUGINHOST_AU and/or JUCE_PLUGINHOST_VST and/or JUCE_PLUGINHOST_VST3 and/or JUCE_PLUGINHOST_LADSPA flags if you're using this module!"
#endif

#ifndef JUCE_SUPPORT_LEGACY_AUDIOPROCESSOR
 #define JUCE_SUPPORT_LEGACY_AUDIOPROCESSOR 1
#endif

//==============================================================================
#include "utilities/juce_AAXClientExtensions.h"
#include "utilities/juce_VST2ClientExtensions.h"
#include "utilities/juce_VST3Interface.h"
#include "utilities/juce_VST3ClientExtensions.h"
#include "format_types/juce_ARACommon.h"
#include "utilities/juce_ExtensionsVisitor.h"
#include "processors/juce_AudioProcessorParameter.h"
#include "processors/juce_HostedAudioProcessorParameter.h"
#include "processors/juce_AudioProcessorEditorHostContext.h"
#include "processors/juce_AudioProcessorEditor.h"
#include "processors/juce_AudioProcessorListener.h"
#include "processors/juce_AudioProcessorParameterGroup.h"
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
#include "format_types/juce_LV2PluginFormat.h"
#include "format_types/juce_VST3PluginFormat.h"
#include "format_types/juce_VSTMidiEventList.h"
#include "format_types/juce_VSTPluginFormat.h"
#include "format_types/juce_ARAHosting.h"
#include "scanning/juce_PluginDirectoryScanner.h"
#include "scanning/juce_PluginListComponent.h"
#include "utilities/juce_AudioProcessorParameterWithID.h"
#include "utilities/juce_RangedAudioParameter.h"
#include "utilities/juce_AudioParameterFloat.h"
#include "utilities/juce_AudioParameterInt.h"
#include "utilities/juce_AudioParameterBool.h"
#include "utilities/juce_AudioParameterChoice.h"
#include "utilities/juce_ParameterAttachments.h"
#include "utilities/juce_AudioProcessorValueTreeState.h"
#include "utilities/juce_PluginHostType.h"
#include "utilities/ARA/juce_ARADebug.h"
#include "utilities/ARA/juce_ARA_utils.h"

//==============================================================================
// These declarations are here to avoid missing-prototype warnings in user code.

// If you're implementing a plugin, you should supply a body for
// this function in your own code.
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter();

// If you are implementing an ARA enabled plugin, you need to
// implement this function somewhere in the codebase by returning
// SubclassOfARADocumentControllerSpecialisation::createARAFactory<SubclassOfARADocumentControllerSpecialisation>();
#if JucePlugin_Enable_ARA
 const ARA::ARAFactory* JUCE_CALLTYPE createARAFactory();
#endif

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

  ID:                 juce_audio_processors_headless
  vendor:             juce
  version:            8.0.12
  name:               JUCE audio processor classes without UI
  description:        Classes for loading and playing VST, AU, LADSPA, or internally-generated audio processors without UI.
  website:            http://www.juce.com/juce
  license:            AGPLv3/Commercial
  minimumCppStandard: 17

  dependencies:       juce_audio_basics juce_events
  OSXFrameworks:      CoreAudio CoreMIDI AudioToolbox
  iOSFrameworks:      AudioToolbox

 END_JUCE_MODULE_DECLARATION

*******************************************************************************/


#pragma once
#define JUCE_AUDIO_PROCESSORS_HEADLESS_H_INCLUDED

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_events/juce_events.h>

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

//==============================================================================
#include <juce_audio_processors_headless/format/juce_PluginFormatDefs.h>
#include <juce_audio_processors_headless/processors/juce_AudioProcessorListener.h>
#include <juce_audio_processors_headless/utilities/juce_AAXClientExtensions.h>
#include <juce_audio_processors_headless/utilities/juce_VST2ClientExtensions.h>
#include <juce_audio_processors_headless/utilities/juce_VST3Interface.h>
#include <juce_audio_processors_headless/utilities/juce_VST3ClientExtensions.h>
#include <juce_audio_processors_headless/format_types/juce_ARACommon.h>
#include <juce_audio_processors_headless/utilities/juce_ExtensionsVisitor.h>
#include <juce_audio_processors_headless/processors/juce_AudioProcessorParameter.h>
#include <juce_audio_processors_headless/processors/juce_HostedAudioProcessorParameter.h>
#include <juce_audio_processors_headless/processors/juce_AudioProcessorParameterGroup.h>
#include <juce_audio_processors_headless/processors/juce_AudioProcessor.h>
#include <juce_audio_processors_headless/processors/juce_PluginDescription.h>
#include <juce_audio_processors_headless/processors/juce_AudioPluginInstance.h>
#include <juce_audio_processors_headless/processors/juce_AudioProcessorGraph.h>
#include <juce_audio_processors_headless/format/juce_AudioPluginFormat.h>
#include <juce_audio_processors_headless/utilities/juce_AudioProcessorParameterWithID.h>
#include <juce_audio_processors_headless/utilities/juce_RangedAudioParameter.h>
#include <juce_audio_processors_headless/utilities/juce_AudioParameterFloat.h>
#include <juce_audio_processors_headless/utilities/juce_AudioParameterInt.h>
#include <juce_audio_processors_headless/utilities/juce_AudioParameterBool.h>
#include <juce_audio_processors_headless/utilities/juce_AudioParameterChoice.h>
#include <juce_audio_processors_headless/utilities/ARA/juce_ARADebug.h>
#include <juce_audio_processors_headless/utilities/ARA/juce_ARA_utils.h>
#include <juce_audio_processors_headless/format_types/juce_AudioUnitPluginFormatHeadless.h>
#include <juce_audio_processors_headless/format_types/juce_LADSPAPluginFormatHeadless.h>
#include <juce_audio_processors_headless/format_types/juce_LV2PluginFormatHeadless.h>
#include <juce_audio_processors_headless/format_types/juce_VST3PluginFormatHeadless.h>
#include <juce_audio_processors_headless/format_types/juce_VSTPluginFormatHeadless.h>
#include <juce_audio_processors_headless/format_types/juce_ARAHosting.h>
#include <juce_audio_processors_headless/format/juce_AudioPluginFormatManager.h>

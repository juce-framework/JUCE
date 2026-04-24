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
  version:            8.0.12
  name:               JUCE audio processor classes
  description:        Classes for loading and playing VST, AU, LADSPA, or internally-generated audio processors.
  website:            http://www.juce.com/juce
  license:            AGPLv3/Commercial
  minimumCppStandard: 17

  dependencies:       juce_gui_extra, juce_audio_processors_headless

 END_JUCE_MODULE_DECLARATION

*******************************************************************************/


#pragma once
#define JUCE_AUDIO_PROCESSORS_H_INCLUDED

#include <juce_audio_processors_headless/juce_audio_processors_headless.h>
#include <juce_gui_extra/juce_gui_extra.h>

//==============================================================================
#include "processors/juce_AudioProcessorEditorHostContext.h"
#include "processors/juce_AudioProcessorEditor.h"
#include "processors/juce_GenericAudioProcessorEditor.h"
#include "format/juce_AudioPluginFormatManagerHelpers.h"
#include "scanning/juce_KnownPluginList.h"
#include "format_types/juce_AudioUnitPluginFormat.h"
#include "format_types/juce_LADSPAPluginFormat.h"
#include "format_types/juce_LV2PluginFormat.h"
#include "format_types/juce_VST3PluginFormat.h"
#include "format_types/juce_VSTPluginFormat.h"
#include "scanning/juce_PluginDirectoryScanner.h"
#include "scanning/juce_PluginListComponent.h"
#include "utilities/juce_ParameterAttachments.h"
#include "utilities/juce_AudioProcessorValueTreeState.h"
#include "utilities/juce_PluginHostType.h"

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

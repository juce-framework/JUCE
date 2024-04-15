/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/


/*******************************************************************************
 The block below describes the properties of this module, and is read by
 the Projucer to automatically generate project code that uses it.
 For details about the syntax and how to create or use a module, see the
 JUCE Module Format.md file.


 BEGIN_JUCE_MODULE_DECLARATION

  ID:                 juce_midi_ci
  vendor:             juce
  version:            7.0.12
  name:               JUCE MIDI CI Classes
  description:        Classes facilitating communication via MIDI Capability Inquiry
  website:            http://www.juce.com/juce
  license:            GPL/Commercial
  minimumCppStandard: 17

  dependencies:       juce_audio_basics

 END_JUCE_MODULE_DECLARATION

*******************************************************************************/


#pragma once
#define JUCE_MIDI_CI_H_INCLUDED

#include <juce_audio_basics/juce_audio_basics.h>

#include <juce_midi_ci/ci/juce_CIFunctionBlock.h>
#include <juce_midi_ci/ci/juce_CIMuid.h>
#include <juce_midi_ci/ci/juce_CIEncoding.h>
#include <juce_midi_ci/ci/juce_CIEncodings.h>
#include <juce_midi_ci/ci/juce_CIMessages.h>
#include <juce_midi_ci/ci/juce_CIChannelAddress.h>
#include <juce_midi_ci/ci/juce_CIResponderOutput.h>
#include <juce_midi_ci/ci/juce_CIParser.h>
#include <juce_midi_ci/ci/juce_CISupportedAndActive.h>
#include <juce_midi_ci/ci/juce_CIResponderDelegate.h>
#include <juce_midi_ci/ci/juce_CIProfileStates.h>
#include <juce_midi_ci/ci/juce_CIProfileAtAddress.h>
#include <juce_midi_ci/ci/juce_CIProfileDelegate.h>
#include <juce_midi_ci/ci/juce_CIProfileHost.h>
#include <juce_midi_ci/ci/juce_CISubscription.h>
#include <juce_midi_ci/ci/juce_CIPropertyDelegate.h>
#include <juce_midi_ci/ci/juce_CIPropertyExchangeResult.h>
#include <juce_midi_ci/ci/juce_CIPropertyExchangeCache.h>
#include <juce_midi_ci/ci/juce_CIPropertyHost.h>
#include <juce_midi_ci/ci/juce_CIDeviceFeatures.h>
#include <juce_midi_ci/ci/juce_CIDeviceMessageHandler.h>
#include <juce_midi_ci/ci/juce_CIDeviceOptions.h>
#include <juce_midi_ci/ci/juce_CISubscriptionManager.h>
#include <juce_midi_ci/ci/juce_CIDeviceListener.h>
#include <juce_midi_ci/ci/juce_CIDevice.h>

namespace juce
{
    namespace ci = midi_ci;
}

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

  ID:                 juce_midi_ci
  vendor:             juce
  version:            8.0.2
  name:               JUCE MIDI CI Classes
  description:        Classes facilitating communication via MIDI Capability Inquiry
  website:            http://www.juce.com/juce
  license:            AGPLv3/Commercial
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

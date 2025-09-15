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

  ID:                 juce_audio_basics
  vendor:             juce
  version:            8.0.10
  name:               JUCE audio and MIDI data classes
  description:        Classes for audio buffer manipulation, midi message handling, synthesis, etc.
  website:            http://www.juce.com/juce
  license:            AGPLv3/Commercial
  minimumCppStandard: 17

  dependencies:       juce_core
  OSXFrameworks:      Accelerate
  iOSFrameworks:      Accelerate

 END_JUCE_MODULE_DECLARATION

*******************************************************************************/


#pragma once
#define JUCE_AUDIO_BASICS_H_INCLUDED

#include <juce_core/juce_core.h>

//==============================================================================
#undef Complex  // apparently some C libraries actually define these symbols (!)
#undef Factor

//==============================================================================
#ifndef JUCE_USE_SSE_INTRINSICS
 #define JUCE_USE_SSE_INTRINSICS 1
#endif

#if ! JUCE_INTEL
 #undef JUCE_USE_SSE_INTRINSICS
#endif

#if __ARM_NEON__ && ! (JUCE_USE_VDSP_FRAMEWORK || defined (JUCE_USE_ARM_NEON))
 #define JUCE_USE_ARM_NEON 1
#endif

#if TARGET_IPHONE_SIMULATOR
 #ifdef JUCE_USE_ARM_NEON
  #undef JUCE_USE_ARM_NEON
 #endif
 #define JUCE_USE_ARM_NEON 0
#endif

//==============================================================================
#include "buffers/juce_AudioDataConverters.h"
JUCE_BEGIN_IGNORE_WARNINGS_MSVC (4661)
#include "buffers/juce_FloatVectorOperations.h"
JUCE_END_IGNORE_WARNINGS_MSVC
#include "buffers/juce_AudioSampleBuffer.h"
#include "buffers/juce_AudioChannelSet.h"
#include "buffers/juce_AudioProcessLoadMeasurer.h"
#include "utilities/juce_Decibels.h"
#include "utilities/juce_IIRFilter.h"
#include "utilities/juce_GenericInterpolator.h"
#include "utilities/juce_Interpolators.h"
#include "utilities/juce_SmoothedValue.h"
#include "utilities/juce_Reverb.h"
#include "utilities/juce_ADSR.h"
#include "midi/juce_MidiMessage.h"
#include "midi/juce_MidiBuffer.h"
#include "midi/juce_MidiMessageSequence.h"
#include "midi/juce_MidiFile.h"
#include "midi/juce_MidiKeyboardState.h"
#include "midi/juce_MidiRPN.h"
#include "mpe/juce_MPEValue.h"
#include "mpe/juce_MPENote.h"
#include "mpe/juce_MPEZoneLayout.h"
#include "mpe/juce_MPEInstrument.h"
#include "mpe/juce_MPEMessages.h"
#include "mpe/juce_MPESynthesiserBase.h"
#include "mpe/juce_MPESynthesiserVoice.h"
#include "mpe/juce_MPESynthesiser.h"
#include "mpe/juce_MPEUtils.h"
#include "sources/juce_AudioSource.h"
#include "sources/juce_PositionableAudioSource.h"
#include "sources/juce_BufferingAudioSource.h"
#include "sources/juce_ChannelRemappingAudioSource.h"
#include "sources/juce_IIRFilterAudioSource.h"
#include "sources/juce_MemoryAudioSource.h"
#include "sources/juce_MixerAudioSource.h"
#include "sources/juce_ResamplingAudioSource.h"
#include "sources/juce_ReverbAudioSource.h"
#include "sources/juce_ToneGeneratorAudioSource.h"
#include "synthesisers/juce_Synthesiser.h"
#include "audio_play_head/juce_AudioPlayHead.h"
#include "utilities/juce_AudioWorkgroup.h"
#include "midi/ump/juce_UMPBytesOnGroup.h"
#include "midi/ump/juce_UMPDeviceInfo.h"

namespace juce
{
    namespace ump = universal_midi_packets;
}

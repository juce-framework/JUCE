/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2016 - ROLI Ltd.

   Permission is granted to use this software under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license/

   Permission to use, copy, modify, and/or distribute this software for any
   purpose with or without fee is hereby granted, provided that the above
   copyright notice and this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND ISC DISCLAIMS ALL WARRANTIES WITH REGARD
   TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
   FITNESS. IN NO EVENT SHALL ISC BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT,
   OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
   USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
   TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
   OF THIS SOFTWARE.

   -----------------------------------------------------------------------------

   To release a closed-source product which uses other parts of JUCE not
   licensed under the ISC terms, commercial licenses are available: visit
   www.juce.com for more information.

  ==============================================================================
*/

/*******************************************************************************
 The block below describes the properties of this module, and is read by
 the Projucer to automatically generate project code that uses it.
 For details about the syntax and how to create or use a module, see the
 JUCE Module Format.txt file.


 BEGIN_JUCE_MODULE_DECLARATION

  ID:               juce_audio_basics
  vendor:           juce
  version:          4.3.1
  name:             JUCE audio and MIDI data classes
  description:      Classes for audio buffer manipulation, midi message handling, synthesis, etc.
  website:          http://www.juce.com/juce
  license:          ISC

  dependencies:     juce_core
  OSXFrameworks:    Accelerate
  iOSFrameworks:    Accelerate

 END_JUCE_MODULE_DECLARATION

*******************************************************************************/


#ifndef JUCE_AUDIO_BASICS_H_INCLUDED
#define JUCE_AUDIO_BASICS_H_INCLUDED

#include <juce_core/juce_core.h>

namespace juce
{

#undef Complex  // apparently some C libraries actually define these symbols (!)
#undef Factor

#include "buffers/juce_AudioDataConverters.h"
#include "buffers/juce_FloatVectorOperations.h"
#include "buffers/juce_AudioSampleBuffer.h"
#include "buffers/juce_AudioChannelSet.h"
#include "effects/juce_Decibels.h"
#include "effects/juce_IIRFilter.h"
#include "effects/juce_LagrangeInterpolator.h"
#include "effects/juce_CatmullRomInterpolator.h"
#include "effects/juce_FFT.h"
#include "effects/juce_LinearSmoothedValue.h"
#include "effects/juce_Reverb.h"
#include "midi/juce_MidiMessage.h"
#include "midi/juce_MidiBuffer.h"
#include "midi/juce_MidiMessageSequence.h"
#include "midi/juce_MidiFile.h"
#include "midi/juce_MidiKeyboardState.h"
#include "midi/juce_MidiRPN.h"
#include "mpe/juce_MPEValue.h"
#include "mpe/juce_MPENote.h"
#include "mpe/juce_MPEZone.h"
#include "mpe/juce_MPEZoneLayout.h"
#include "mpe/juce_MPEInstrument.h"
#include "mpe/juce_MPEMessages.h"
#include "mpe/juce_MPESynthesiserBase.h"
#include "mpe/juce_MPESynthesiserVoice.h"
#include "mpe/juce_MPESynthesiser.h"
#include "sources/juce_AudioSource.h"
#include "sources/juce_PositionableAudioSource.h"
#include "sources/juce_BufferingAudioSource.h"
#include "sources/juce_ChannelRemappingAudioSource.h"
#include "sources/juce_IIRFilterAudioSource.h"
#include "sources/juce_MixerAudioSource.h"
#include "sources/juce_ResamplingAudioSource.h"
#include "sources/juce_ReverbAudioSource.h"
#include "sources/juce_ToneGeneratorAudioSource.h"
#include "synthesisers/juce_Synthesiser.h"

}

#endif   // JUCE_AUDIO_BASICS_H_INCLUDED

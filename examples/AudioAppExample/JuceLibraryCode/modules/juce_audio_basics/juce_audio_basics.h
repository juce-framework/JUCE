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

#ifndef JUCE_AUDIO_BASICS_H_INCLUDED
#define JUCE_AUDIO_BASICS_H_INCLUDED

#include "../juce_core/juce_core.h"

//=============================================================================
namespace juce
{

#include "buffers/juce_AudioDataConverters.h"
#include "buffers/juce_AudioSampleBuffer.h"
#include "buffers/juce_FloatVectorOperations.h"
#include "effects/juce_Decibels.h"
#include "effects/juce_IIRFilter.h"
#include "effects/juce_LagrangeInterpolator.h"
#include "effects/juce_Reverb.h"
#include "midi/juce_MidiMessage.h"
#include "midi/juce_MidiBuffer.h"
#include "midi/juce_MidiMessageSequence.h"
#include "midi/juce_MidiFile.h"
#include "midi/juce_MidiKeyboardState.h"
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

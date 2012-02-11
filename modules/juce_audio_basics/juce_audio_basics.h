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

#ifndef __JUCE_AUDIO_BASICS_JUCEHEADER__
#define __JUCE_AUDIO_BASICS_JUCEHEADER__

#include "../juce_core/juce_core.h"

//=============================================================================
namespace juce
{

// START_AUTOINCLUDE buffers, effects, midi, sources, synthesisers
#ifndef __JUCE_AUDIODATACONVERTERS_JUCEHEADER__
 #include "buffers/juce_AudioDataConverters.h"
#endif
#ifndef __JUCE_AUDIOSAMPLEBUFFER_JUCEHEADER__
 #include "buffers/juce_AudioSampleBuffer.h"
#endif
#ifndef __JUCE_DECIBELS_JUCEHEADER__
 #include "effects/juce_Decibels.h"
#endif
#ifndef __JUCE_IIRFILTER_JUCEHEADER__
 #include "effects/juce_IIRFilter.h"
#endif
#ifndef __JUCE_REVERB_JUCEHEADER__
 #include "effects/juce_Reverb.h"
#endif
#ifndef __JUCE_MIDIBUFFER_JUCEHEADER__
 #include "midi/juce_MidiBuffer.h"
#endif
#ifndef __JUCE_MIDIFILE_JUCEHEADER__
 #include "midi/juce_MidiFile.h"
#endif
#ifndef __JUCE_MIDIKEYBOARDSTATE_JUCEHEADER__
 #include "midi/juce_MidiKeyboardState.h"
#endif
#ifndef __JUCE_MIDIMESSAGE_JUCEHEADER__
 #include "midi/juce_MidiMessage.h"
#endif
#ifndef __JUCE_MIDIMESSAGESEQUENCE_JUCEHEADER__
 #include "midi/juce_MidiMessageSequence.h"
#endif
#ifndef __JUCE_AUDIOSOURCE_JUCEHEADER__
 #include "sources/juce_AudioSource.h"
#endif
#ifndef __JUCE_BUFFERINGAUDIOSOURCE_JUCEHEADER__
 #include "sources/juce_BufferingAudioSource.h"
#endif
#ifndef __JUCE_CHANNELREMAPPINGAUDIOSOURCE_JUCEHEADER__
 #include "sources/juce_ChannelRemappingAudioSource.h"
#endif
#ifndef __JUCE_IIRFILTERAUDIOSOURCE_JUCEHEADER__
 #include "sources/juce_IIRFilterAudioSource.h"
#endif
#ifndef __JUCE_MIXERAUDIOSOURCE_JUCEHEADER__
 #include "sources/juce_MixerAudioSource.h"
#endif
#ifndef __JUCE_POSITIONABLEAUDIOSOURCE_JUCEHEADER__
 #include "sources/juce_PositionableAudioSource.h"
#endif
#ifndef __JUCE_RESAMPLINGAUDIOSOURCE_JUCEHEADER__
 #include "sources/juce_ResamplingAudioSource.h"
#endif
#ifndef __JUCE_REVERBAUDIOSOURCE_JUCEHEADER__
 #include "sources/juce_ReverbAudioSource.h"
#endif
#ifndef __JUCE_TONEGENERATORAUDIOSOURCE_JUCEHEADER__
 #include "sources/juce_ToneGeneratorAudioSource.h"
#endif
#ifndef __JUCE_SYNTHESISER_JUCEHEADER__
 #include "synthesisers/juce_Synthesiser.h"
#endif
// END_AUTOINCLUDE

}

#endif   // __JUCE_AUDIO_BASICS_JUCEHEADER__

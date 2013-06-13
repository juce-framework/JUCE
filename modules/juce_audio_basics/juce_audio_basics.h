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
#ifndef __JUCE_FLOATVECTOROPERATIONS_JUCEHEADER__
 #include "buffers/juce_FloatVectorOperations.h"
#endif
#ifndef __JUCE_DECIBELS_JUCEHEADER__
 #include "effects/juce_Decibels.h"
#endif
#ifndef __JUCE_IIRFILTER_JUCEHEADER__
 #include "effects/juce_IIRFilter.h"
#endif
#ifndef __JUCE_LAGRANGEINTERPOLATOR_JUCEHEADER__
 #include "effects/juce_LagrangeInterpolator.h"
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

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

#if defined (__JUCE_AUDIO_BASICS_JUCEHEADER__) && ! JUCE_AMALGAMATED_INCLUDE
 /* When you add this cpp file to your project, you mustn't include it in a file where you've
    already included any other headers - just put it inside a file on its own, possibly with your config
    flags preceding it, but don't include anything else. That also includes avoiding any automatic prefix
    header files that the compiler may be using.
 */
 #error "Incorrect use of JUCE cpp file"
#endif

// Your project must contain an AppConfig.h file with your project-specific settings in it,
// and your header search path must make it accessible to the module's files.
#include "AppConfig.h"

#include "juce_audio_basics.h"

namespace juce
{

// START_AUTOINCLUDE buffers/*.cpp, effects/*.cpp, midi/*.cpp, sources/*.cpp, synthesisers/*.cpp
#include "buffers/juce_AudioDataConverters.cpp"
#include "buffers/juce_AudioSampleBuffer.cpp"
#include "effects/juce_IIRFilter.cpp"
#include "midi/juce_MidiBuffer.cpp"
#include "midi/juce_MidiFile.cpp"
#include "midi/juce_MidiKeyboardState.cpp"
#include "midi/juce_MidiMessage.cpp"
#include "midi/juce_MidiMessageSequence.cpp"
#include "sources/juce_BufferingAudioSource.cpp"
#include "sources/juce_ChannelRemappingAudioSource.cpp"
#include "sources/juce_IIRFilterAudioSource.cpp"
#include "sources/juce_MixerAudioSource.cpp"
#include "sources/juce_ResamplingAudioSource.cpp"
#include "sources/juce_ReverbAudioSource.cpp"
#include "sources/juce_ToneGeneratorAudioSource.cpp"
#include "synthesisers/juce_Synthesiser.cpp"
// END_AUTOINCLUDE

}

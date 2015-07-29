/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

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

#if defined (JUCE_AUDIO_BASICS_H_INCLUDED) && ! JUCE_AMALGAMATED_INCLUDE
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

#if JUCE_MINGW && ! defined (__SSE2__)
 #define JUCE_USE_SSE_INTRINSICS 0
#endif

#ifndef JUCE_USE_SSE_INTRINSICS
 #define JUCE_USE_SSE_INTRINSICS 1
#endif

#if ! JUCE_INTEL
 #undef JUCE_USE_SSE_INTRINSICS
#endif

#if JUCE_USE_SSE_INTRINSICS
 #include <emmintrin.h>
#endif

#ifndef JUCE_USE_VDSP_FRAMEWORK
 #define JUCE_USE_VDSP_FRAMEWORK 1
#endif

#if (JUCE_MAC || JUCE_IOS) && JUCE_USE_VDSP_FRAMEWORK
 #define Point CarbonDummyPointName // (workaround to avoid definition of "Point" by old Carbon headers)
 #include <Accelerate/Accelerate.h>
 #undef Point
#else
 #undef JUCE_USE_VDSP_FRAMEWORK
#endif

#if __ARM_NEON__ && ! (JUCE_USE_VDSP_FRAMEWORK || defined (JUCE_USE_ARM_NEON))
 #define JUCE_USE_ARM_NEON 1
#endif

#if JUCE_USE_ARM_NEON
 #include <arm_neon.h>
#endif

namespace juce
{

#include "buffers/juce_AudioDataConverters.cpp"
#include "buffers/juce_AudioSampleBuffer.cpp"
#include "buffers/juce_FloatVectorOperations.cpp"
#include "effects/juce_IIRFilter.cpp"
#include "effects/juce_LagrangeInterpolator.cpp"
#include "effects/juce_FFT.cpp"
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

}

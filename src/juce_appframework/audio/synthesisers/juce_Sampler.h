/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

  ==============================================================================
*/

#ifndef __JUCE_SAMPLER_JUCEHEADER__
#define __JUCE_SAMPLER_JUCEHEADER__

#include "../../../juce_core/containers/juce_BitArray.h"
#include "juce_Synthesiser.h"


//==============================================================================
/**
    A subclass of SynthesiserSound that represents a sampled audio clip.

    This is a pretty basic sampler, and just attempts to load the whole audio stream
    into memory.

    To use it, create a Synthesiser, add some SamplerVoice objects to it, then
    give it some SampledSound objects to play.

    @see SamplerVoice, Synthesiser, SynthesiserSound
*/
class JUCE_API  SamplerSound    : public SynthesiserSound
{
public:
    //==============================================================================
    /** Creates a sampled sound from an audio reader.

        This will attempt to load the audio from the source into memory and store
        it in this object.

        @param name         a name for the sample
        @param source       the audio to load. This object can be safely deleted by the
                            caller after this constructor returns
        @param midiNotes    the set of midi keys that this sound should be played on. This
                            is used by the SynthesiserSound::appliesToNote() method
        @param midiNoteForNormalPitch   the midi note at which the sample should be played
                                        with its natural rate. All other notes will be pitched
                                        up or down relative to this one
        @param attackTimeSecs   the attack (fade-in) time, in seconds
        @param releaseTimeSecs  the decay (fade-out) time, in seconds
        @param maxSampleLengthSeconds   a maximum length of audio to read from the audio
                                        source, in seconds
    */
    SamplerSound (const String& name,
                  AudioFormatReader& source,
                  const BitArray& midiNotes,
                  const int midiNoteForNormalPitch,
                  const double attackTimeSecs,
                  const double releaseTimeSecs,
                  const double maxSampleLengthSeconds);

    /** Destructor. */
    ~SamplerSound();

    //==============================================================================
    /** Returns the sample's name */
    const String& getName() const throw()                   { return name; }

    /** Returns the audio sample data.
        This could be 0 if there was a problem loading it.
    */
    AudioSampleBuffer* getAudioData() const throw()         { return data; }


    //==============================================================================
    bool appliesToNote (const int midiNoteNumber);
    bool appliesToChannel (const int midiChannel);


    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    friend class SamplerVoice;

    String name;
    AudioSampleBuffer* data;
    double sourceSampleRate;
    BitArray midiNotes;
    int length, attackSamples, releaseSamples;
    int midiRootNote;
};


//==============================================================================
/**
    A subclass of SynthesiserVoice that can play a SamplerSound.

    To use it, create a Synthesiser, add some SamplerVoice objects to it, then
    give it some SampledSound objects to play.

    @see SamplerSound, Synthesiser, SynthesiserVoice
*/
class JUCE_API  SamplerVoice    : public SynthesiserVoice
{
public:
    //==============================================================================
    /** Creates a SamplerVoice.
    */
    SamplerVoice();

    /** Destructor. */
    ~SamplerVoice();


    //==============================================================================
    bool canPlaySound (SynthesiserSound* sound);

    void startNote (const int midiNoteNumber,
                    const float velocity,
                    SynthesiserSound* sound,
                    const int currentPitchWheelPosition);

    void stopNote (const bool allowTailOff);

    void pitchWheelMoved (const int newValue);
    void controllerMoved (const int controllerNumber,
                          const int newValue);

    void renderNextBlock (AudioSampleBuffer& outputBuffer, int startSample, int numSamples);


    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    double pitchRatio;
    double sourceSamplePosition;
    float lgain, rgain, attackReleaseLevel, attackDelta, releaseDelta;
    bool isInAttack, isInRelease;
};


#endif   // __JUCE_SAMPLER_JUCEHEADER__

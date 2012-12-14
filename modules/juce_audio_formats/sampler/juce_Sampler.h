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

#ifndef __JUCE_SAMPLER_JUCEHEADER__
#define __JUCE_SAMPLER_JUCEHEADER__


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
                  const BigInteger& midiNotes,
                  int midiNoteForNormalPitch,
                  double attackTimeSecs,
                  double releaseTimeSecs,
                  double maxSampleLengthSeconds);

    /** Destructor. */
    ~SamplerSound();

    //==============================================================================
    /** Returns the sample's name */
    const String& getName() const                           { return name; }

    /** Returns the audio sample data.
        This could be 0 if there was a problem loading it.
    */
    AudioSampleBuffer* getAudioData() const                 { return data; }


    //==============================================================================
    bool appliesToNote (const int midiNoteNumber);
    bool appliesToChannel (const int midiChannel);


private:
    //==============================================================================
    friend class SamplerVoice;

    String name;
    ScopedPointer <AudioSampleBuffer> data;
    double sourceSampleRate;
    BigInteger midiNotes;
    int length, attackSamples, releaseSamples;
    int midiRootNote;

    JUCE_LEAK_DETECTOR (SamplerSound)
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


private:
    //==============================================================================
    double pitchRatio;
    double sourceSamplePosition;
    float lgain, rgain, attackReleaseLevel, attackDelta, releaseDelta;
    bool isInAttack, isInRelease;

    JUCE_LEAK_DETECTOR (SamplerVoice)
};


#endif   // __JUCE_SAMPLER_JUCEHEADER__

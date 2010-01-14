/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

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

#include "../../core/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_Sampler.h"
#include "../audio_file_formats/juce_AudioFormatReader.h"


//==============================================================================
SamplerSound::SamplerSound (const String& name_,
                            AudioFormatReader& source,
                            const BitArray& midiNotes_,
                            const int midiNoteForNormalPitch,
                            const double attackTimeSecs,
                            const double releaseTimeSecs,
                            const double maxSampleLengthSeconds)
    : name (name_),
      midiNotes (midiNotes_),
      midiRootNote (midiNoteForNormalPitch)
{
    sourceSampleRate = source.sampleRate;

    if (sourceSampleRate <= 0 || source.lengthInSamples <= 0)
    {
        length = 0;
        attackSamples = 0;
        releaseSamples = 0;
    }
    else
    {
        length = jmin ((int) source.lengthInSamples,
                       (int) (maxSampleLengthSeconds * sourceSampleRate));

        data = new AudioSampleBuffer (jmin (2, (int) source.numChannels), length + 4);

        data->readFromAudioReader (&source, 0, length + 4, 0, true, true);

        attackSamples = roundToInt (attackTimeSecs * sourceSampleRate);
        releaseSamples = roundToInt (releaseTimeSecs * sourceSampleRate);
    }
}

SamplerSound::~SamplerSound()
{
}

//==============================================================================
bool SamplerSound::appliesToNote (const int midiNoteNumber)
{
    return midiNotes [midiNoteNumber];
}

bool SamplerSound::appliesToChannel (const int /*midiChannel*/)
{
    return true;
}


//==============================================================================
SamplerVoice::SamplerVoice()
    : pitchRatio (0.0),
      sourceSamplePosition (0.0),
      lgain (0.0f),
      rgain (0.0f),
      isInAttack (false),
      isInRelease (false)
{
}

SamplerVoice::~SamplerVoice()
{
}

bool SamplerVoice::canPlaySound (SynthesiserSound* sound)
{
    return dynamic_cast <const SamplerSound*> (sound) != 0;
}

void SamplerVoice::startNote (const int midiNoteNumber,
                              const float velocity,
                              SynthesiserSound* s,
                              const int /*currentPitchWheelPosition*/)
{
    const SamplerSound* const sound = dynamic_cast <const SamplerSound*> (s);
    jassert (sound != 0); // this object can only play SamplerSounds!

    if (sound != 0)
    {
        const double targetFreq = MidiMessage::getMidiNoteInHertz (midiNoteNumber);
        const double naturalFreq = MidiMessage::getMidiNoteInHertz (sound->midiRootNote);

        pitchRatio = (targetFreq * sound->sourceSampleRate) / (naturalFreq * getSampleRate());

        sourceSamplePosition = 0.0;
        lgain = velocity;
        rgain = velocity;

        isInAttack = (sound->attackSamples > 0);
        isInRelease = false;

        if (isInAttack)
        {
            attackReleaseLevel = 0.0f;
            attackDelta = (float) (pitchRatio / sound->attackSamples);
        }
        else
        {
            attackReleaseLevel = 1.0f;
            attackDelta = 0.0f;
        }

        if (sound->releaseSamples > 0)
        {
            releaseDelta = (float) (-pitchRatio / sound->releaseSamples);
        }
        else
        {
            releaseDelta = 0.0f;
        }
    }
}

void SamplerVoice::stopNote (const bool allowTailOff)
{
    if (allowTailOff)
    {
        isInAttack = false;
        isInRelease = true;
    }
    else
    {
        clearCurrentNote();
    }
}

void SamplerVoice::pitchWheelMoved (const int /*newValue*/)
{
}

void SamplerVoice::controllerMoved (const int /*controllerNumber*/,
                                    const int /*newValue*/)
{
}

//==============================================================================
void SamplerVoice::renderNextBlock (AudioSampleBuffer& outputBuffer, int startSample, int numSamples)
{
    const SamplerSound* const playingSound = (SamplerSound*) (SynthesiserSound*) getCurrentlyPlayingSound();

    if (playingSound != 0)
    {
        const float* const inL = playingSound->data->getSampleData (0, 0);
        const float* const inR = playingSound->data->getNumChannels() > 1
                                    ? playingSound->data->getSampleData (1, 0) : 0;

        float* outL = outputBuffer.getSampleData (0, startSample);
        float* outR = outputBuffer.getNumChannels() > 1 ? outputBuffer.getSampleData (1, startSample) : 0;

        while (--numSamples >= 0)
        {
            const int pos = (int) sourceSamplePosition;
            const float alpha = (float) (sourceSamplePosition - pos);
            const float invAlpha = 1.0f - alpha;

            // just using a very simple linear interpolation here..
            float l = (inL [pos] * invAlpha + inL [pos + 1] * alpha);
            float r = (inR != 0) ? (inR [pos] * invAlpha + inR [pos + 1] * alpha)
                                 : l;

            l *= lgain;
            r *= rgain;

            if (isInAttack)
            {
                l *= attackReleaseLevel;
                r *= attackReleaseLevel;

                attackReleaseLevel += attackDelta;

                if (attackReleaseLevel >= 1.0f)
                {
                    attackReleaseLevel = 1.0f;
                    isInAttack = false;
                }
            }
            else if (isInRelease)
            {
                l *= attackReleaseLevel;
                r *= attackReleaseLevel;

                attackReleaseLevel += releaseDelta;

                if (attackReleaseLevel <= 0.0f)
                {
                    stopNote (false);
                    break;
                }
            }

            if (outR != 0)
            {
                *outL++ += l;
                *outR++ += r;
            }
            else
            {
                *outL++ += (l + r) * 0.5f;
            }

            sourceSamplePosition += pitchRatio;

            if (sourceSamplePosition > playingSound->length)
            {
                stopNote (false);
                break;
            }
        }
    }
}

END_JUCE_NAMESPACE

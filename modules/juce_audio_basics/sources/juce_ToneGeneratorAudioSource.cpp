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

ToneGeneratorAudioSource::ToneGeneratorAudioSource()
    : frequency (1000.0),
      sampleRate (44100.0),
      currentPhase (0.0),
      phasePerSample (0.0),
      amplitude (0.5f)
{
}

ToneGeneratorAudioSource::~ToneGeneratorAudioSource()
{
}

//==============================================================================
void ToneGeneratorAudioSource::setAmplitude (const float newAmplitude)
{
    amplitude = newAmplitude;
}

void ToneGeneratorAudioSource::setFrequency (const double newFrequencyHz)
{
    frequency = newFrequencyHz;
    phasePerSample = 0.0;
}

//==============================================================================
void ToneGeneratorAudioSource::prepareToPlay (int /*samplesPerBlockExpected*/, double rate)
{
    currentPhase = 0.0;
    phasePerSample = 0.0;
    sampleRate = rate;
}

void ToneGeneratorAudioSource::releaseResources()
{
}

void ToneGeneratorAudioSource::getNextAudioBlock (const AudioSourceChannelInfo& info)
{
    if (phasePerSample == 0.0)
        phasePerSample = double_Pi * 2.0 / (sampleRate / frequency);

    for (int i = 0; i < info.numSamples; ++i)
    {
        const float sample = amplitude * (float) std::sin (currentPhase);
        currentPhase += phasePerSample;

        for (int j = info.buffer->getNumChannels(); --j >= 0;)
            info.buffer->setSample (j, info.startSample + i, sample);
    }
}

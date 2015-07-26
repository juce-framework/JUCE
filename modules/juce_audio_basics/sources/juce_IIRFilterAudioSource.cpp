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

IIRFilterAudioSource::IIRFilterAudioSource (AudioSource* const inputSource,
                                            const bool deleteInputWhenDeleted)
    : input (inputSource, deleteInputWhenDeleted)
{
    jassert (inputSource != nullptr);

    for (int i = 2; --i >= 0;)
        iirFilters.add (new IIRFilter());
}

IIRFilterAudioSource::~IIRFilterAudioSource()  {}

//==============================================================================
void IIRFilterAudioSource::setCoefficients (const IIRCoefficients& newCoefficients)
{
    for (int i = iirFilters.size(); --i >= 0;)
        iirFilters.getUnchecked(i)->setCoefficients (newCoefficients);
}

void IIRFilterAudioSource::makeInactive()
{
    for (int i = iirFilters.size(); --i >= 0;)
        iirFilters.getUnchecked(i)->makeInactive();
}

//==============================================================================
void IIRFilterAudioSource::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    input->prepareToPlay (samplesPerBlockExpected, sampleRate);

    for (int i = iirFilters.size(); --i >= 0;)
        iirFilters.getUnchecked(i)->reset();
}

void IIRFilterAudioSource::releaseResources()
{
    input->releaseResources();
}

void IIRFilterAudioSource::getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill)
{
    input->getNextAudioBlock (bufferToFill);

    const int numChannels = bufferToFill.buffer->getNumChannels();

    while (numChannels > iirFilters.size())
        iirFilters.add (new IIRFilter (*iirFilters.getUnchecked (0)));

    for (int i = 0; i < numChannels; ++i)
        iirFilters.getUnchecked(i)
            ->processSamples (bufferToFill.buffer->getWritePointer (i, bufferToFill.startSample),
                              bufferToFill.numSamples);
}

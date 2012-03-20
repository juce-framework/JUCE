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
void IIRFilterAudioSource::setFilterParameters (const IIRFilter& newSettings)
{
    for (int i = iirFilters.size(); --i >= 0;)
        iirFilters.getUnchecked(i)->copyCoefficientsFrom (newSettings);
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
            ->processSamples (bufferToFill.buffer->getSampleData (i, bufferToFill.startSample),
                              bufferToFill.numSamples);
}

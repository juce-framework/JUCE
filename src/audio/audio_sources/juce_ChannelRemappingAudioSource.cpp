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

#include "juce_ChannelRemappingAudioSource.h"
#include "../../threads/juce_ScopedLock.h"


//==============================================================================
ChannelRemappingAudioSource::ChannelRemappingAudioSource (AudioSource* const source_,
                                                          const bool deleteSourceWhenDeleted_)
   : requiredNumberOfChannels (2),
     source (source_),
     deleteSourceWhenDeleted (deleteSourceWhenDeleted_),
     buffer (2, 16)
{
    remappedInfo.buffer = &buffer;
    remappedInfo.startSample = 0;
}

ChannelRemappingAudioSource::~ChannelRemappingAudioSource()
{
    if (deleteSourceWhenDeleted)
        delete source;
}

//==============================================================================
void ChannelRemappingAudioSource::setNumberOfChannelsToProduce (const int requiredNumberOfChannels_) throw()
{
    const ScopedLock sl (lock);
    requiredNumberOfChannels = requiredNumberOfChannels_;
}

void ChannelRemappingAudioSource::clearAllMappings() throw()
{
    const ScopedLock sl (lock);

    remappedInputs.clear();
    remappedOutputs.clear();
}

void ChannelRemappingAudioSource::setInputChannelMapping (const int destIndex, const int sourceIndex) throw()
{
    const ScopedLock sl (lock);

    while (remappedInputs.size() < destIndex)
        remappedInputs.add (-1);

    remappedInputs.set (destIndex, sourceIndex);
}

void ChannelRemappingAudioSource::setOutputChannelMapping (const int sourceIndex, const int destIndex) throw()
{
    const ScopedLock sl (lock);

    while (remappedOutputs.size() < sourceIndex)
        remappedOutputs.add (-1);

    remappedOutputs.set (sourceIndex, destIndex);
}

int ChannelRemappingAudioSource::getRemappedInputChannel (const int inputChannelIndex) const throw()
{
    const ScopedLock sl (lock);

    if (inputChannelIndex >= 0 && inputChannelIndex < remappedInputs.size())
        return remappedInputs.getUnchecked (inputChannelIndex);

    return -1;
}

int ChannelRemappingAudioSource::getRemappedOutputChannel (const int outputChannelIndex) const throw()
{
    const ScopedLock sl (lock);

    if (outputChannelIndex >= 0 && outputChannelIndex < remappedOutputs.size())
        return remappedOutputs .getUnchecked (outputChannelIndex);

    return -1;
}

//==============================================================================
void ChannelRemappingAudioSource::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    source->prepareToPlay (samplesPerBlockExpected, sampleRate);
}

void ChannelRemappingAudioSource::releaseResources()
{
    source->releaseResources();
}

void ChannelRemappingAudioSource::getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill)
{
    const ScopedLock sl (lock);

    buffer.setSize (requiredNumberOfChannels, bufferToFill.numSamples, false, false, true);

    const int numChans = bufferToFill.buffer->getNumChannels();

    int i;
    for (i = 0; i < buffer.getNumChannels(); ++i)
    {
        const int remappedChan = getRemappedInputChannel (i);

        if (remappedChan >= 0 && remappedChan < numChans)
        {
            buffer.copyFrom (i, 0, *bufferToFill.buffer,
                             remappedChan,
                             bufferToFill.startSample,
                             bufferToFill.numSamples);
        }
        else
        {
            buffer.clear (i, 0, bufferToFill.numSamples);
        }
    }

    remappedInfo.numSamples = bufferToFill.numSamples;

    source->getNextAudioBlock (remappedInfo);

    bufferToFill.clearActiveBufferRegion();

    for (i = 0; i < requiredNumberOfChannels; ++i)
    {
        const int remappedChan = getRemappedOutputChannel (i);

        if (remappedChan >= 0 && remappedChan < numChans)
        {
            bufferToFill.buffer->addFrom (remappedChan, bufferToFill.startSample,
                                          buffer, i, 0, bufferToFill.numSamples);

        }
    }
}

//==============================================================================
XmlElement* ChannelRemappingAudioSource::createXml() const throw()
{
    XmlElement* e = new XmlElement (T("MAPPINGS"));

    String ins, outs;
    int i;

    const ScopedLock sl (lock);

    for (i = 0; i < remappedInputs.size(); ++i)
        ins << remappedInputs.getUnchecked(i) << T(' ');

    for (i = 0; i < remappedOutputs.size(); ++i)
        outs << remappedOutputs.getUnchecked(i) << T(' ');

    e->setAttribute (T("inputs"), ins.trimEnd());
    e->setAttribute (T("outputs"), outs.trimEnd());

    return e;
}

void ChannelRemappingAudioSource::restoreFromXml (const XmlElement& e) throw()
{
    if (e.hasTagName (T("MAPPINGS")))
    {
        const ScopedLock sl (lock);

        clearAllMappings();

        StringArray ins, outs;
        ins.addTokens (e.getStringAttribute (T("inputs")), false);
        outs.addTokens (e.getStringAttribute (T("outputs")), false);

        int i;
        for (i = 0; i < ins.size(); ++i)
            remappedInputs.add (ins[i].getIntValue());

        for (i = 0; i < outs.size(); ++i)
            remappedOutputs.add (outs[i].getIntValue());
    }
}


END_JUCE_NAMESPACE

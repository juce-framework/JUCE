/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#include "IAAEffectProcessor.h"
#include "IAAEffectEditor.h"


IAAEffectProcessor::IAAEffectProcessor()
     : AudioProcessor (BusesProperties()
                       .withInput  ("Input",  AudioChannelSet::stereo(), true)
                       .withOutput ("Output", AudioChannelSet::stereo(), true)),
       parameters (*this, nullptr)
{
    parameters.createAndAddParameter ("gain",
                                      "Gain",
                                      String(),
                                      NormalisableRange<float> (0.0f, 1.0f),
                                      (float) (1.0 / 3.14),
                                      nullptr,
                                      nullptr);

    parameters.state = ValueTree (Identifier ("InterAppAudioEffect"));
}

IAAEffectProcessor::~IAAEffectProcessor()
{
}

//==============================================================================
const String IAAEffectProcessor::getName() const
{
    return JucePlugin_Name;
}

bool IAAEffectProcessor::acceptsMidi() const
{
    return false;
}

bool IAAEffectProcessor::producesMidi() const
{
    return false;
}

double IAAEffectProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int IAAEffectProcessor::getNumPrograms()
{
    return 1;
}

int IAAEffectProcessor::getCurrentProgram()
{
    return 0;
}

void IAAEffectProcessor::setCurrentProgram (int)
{
}

const String IAAEffectProcessor::getProgramName (int)
{
    return String();
}

void IAAEffectProcessor::changeProgramName (int, const String&)
{
}

//==============================================================================
void IAAEffectProcessor::prepareToPlay (double, int)
{
    previousGain = *parameters.getRawParameterValue ("gain");
}

void IAAEffectProcessor::releaseResources()
{
}

bool IAAEffectProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainInputChannelSet() != AudioChannelSet::stereo())
        return false;

    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;

    return true;
}

void IAAEffectProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer&)
{
    const float gain = *parameters.getRawParameterValue ("gain");

    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    auto numSamples = buffer.getNumSamples();

    for (int i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // Apply the gain to the samples using a ramp to avoid discontinuities in
    // the audio between processed buffers.
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        buffer.applyGainRamp (channel, 0, numSamples, previousGain, gain);
        auto newLevel = buffer.getMagnitude (channel, 0, numSamples);

        meterListeners.call ([=] (MeterListener& l) { l.handleNewMeterValue (channel, newLevel); });
    }

    previousGain = gain;

    // Now ask the host for the current time so we can store it to be displayed later.
    updateCurrentTimeInfoFromHost (lastPosInfo);
}

//==============================================================================
bool IAAEffectProcessor::hasEditor() const
{
    return true;
}

AudioProcessorEditor* IAAEffectProcessor::createEditor()
{
    return new IAAEffectEditor (*this, parameters);
}

//==============================================================================
void IAAEffectProcessor::getStateInformation (MemoryBlock& destData)
{
    auto xml = std::unique_ptr<XmlElement> (parameters.state.createXml());
    copyXmlToBinary (*xml, destData);
}

void IAAEffectProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    auto xmlState = std::unique_ptr<XmlElement> (getXmlFromBinary (data, sizeInBytes));

    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (parameters.state.getType()))
            parameters.state = ValueTree::fromXml (*xmlState);
}

bool IAAEffectProcessor::updateCurrentTimeInfoFromHost (AudioPlayHead::CurrentPositionInfo &posInfo)
{
    if (AudioPlayHead* ph = getPlayHead())
    {
        AudioPlayHead::CurrentPositionInfo newTime;

        if (ph->getCurrentPosition (newTime))
        {
            posInfo = newTime;  // Successfully got the current time from the host.
            return true;
        }
    }

    // If the host fails to provide the current time, we'll just reset our copy to a default.
    lastPosInfo.resetToDefault();

    return false;
}

//==============================================================================
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new IAAEffectProcessor();
}

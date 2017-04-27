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

#include "../JuceLibraryCode/JuceHeader.h"
#include "../../GenericEditor.h"

class NoiseGate  : public AudioProcessor
{
public:
    //==============================================================================
    //==============================================================================
    NoiseGate()
        : AudioProcessor (BusesProperties().withInput  ("Input",     AudioChannelSet::stereo())
                                             .withOutput ("Output",    AudioChannelSet::stereo())
                                             .withInput  ("Sidechain", AudioChannelSet::stereo()))
    {
        addParameter (threshold = new AudioParameterFloat ("threshold", "Threshold", 0.0f, 1.0f, 0.5f));
        addParameter (alpha  = new AudioParameterFloat ("alpha",  "Alpha",   0.0f, 1.0f, 0.8f));
    }

    ~NoiseGate() {}

    //==============================================================================
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override
    {
        // the sidechain can take any layout, the main bus needs to be the same on the input and output
        return (layouts.getMainInputChannelSet() == layouts.getMainOutputChannelSet() &&
                (! layouts.getMainInputChannelSet().isDisabled()));
    }

    //==============================================================================
    void prepareToPlay (double /*sampleRate*/, int /*maxBlockSize*/) override { lowPassCoeff = 0.0f; sampleCountDown = 0; }
    void releaseResources() override                                          {}

    void processBlock (AudioSampleBuffer& buffer, MidiBuffer&) override
    {
        AudioSampleBuffer mainInputOutput = getBusBuffer(buffer, true, 0);
        AudioSampleBuffer sideChainInput  = getBusBuffer(buffer, true, 1);

        float alphaCopy = *alpha;
        float thresholdCopy = *threshold;

        for (int j = 0; j < buffer.getNumSamples(); ++j)
        {
            float mixedSamples = 0.0f;
            for (int i = 0; i < sideChainInput.getNumChannels(); ++i)
                mixedSamples += sideChainInput.getReadPointer (i) [j];

            mixedSamples /= static_cast<float> (sideChainInput.getNumChannels());
            lowPassCoeff = (alphaCopy * lowPassCoeff) + ((1.0f - alphaCopy) * mixedSamples);

            if (lowPassCoeff >= thresholdCopy)
                sampleCountDown = (int) getSampleRate();

            // very in-effective way of doing this
            for (int i = 0; i < mainInputOutput.getNumChannels(); ++i)
                *mainInputOutput.getWritePointer (i, j) = sampleCountDown > 0 ? *mainInputOutput.getReadPointer (i, j) : 0.0f;

            if (sampleCountDown > 0)
                --sampleCountDown;
        }
    }

    //==============================================================================
    AudioProcessorEditor* createEditor() override            { return new GenericEditor (*this); }
    bool hasEditor() const override                          { return true; }
    const String getName() const override                    { return "NoiseGate"; }
    bool acceptsMidi() const override                        { return false; }
    bool producesMidi() const override                       { return false; }
    double getTailLengthSeconds() const override             { return 0.0; }
    int getNumPrograms() override                            { return 1; }
    int getCurrentProgram() override                         { return 0; }
    void setCurrentProgram (int) override                    {}
    const String getProgramName (int) override               { return ""; }
    void changeProgramName (int, const String&) override     {}
    bool isVST2() const noexcept                             { return (wrapperType == wrapperType_VST); }

    //==============================================================================
    void getStateInformation (MemoryBlock& destData) override
    {
        MemoryOutputStream stream (destData, true);

        stream.writeFloat (*threshold);
        stream.writeFloat (*alpha);
    }

    void setStateInformation (const void* data, int sizeInBytes) override
    {
        MemoryInputStream stream (data, static_cast<size_t> (sizeInBytes), false);

        threshold->setValueNotifyingHost (stream.readFloat());
        alpha->setValueNotifyingHost (stream.readFloat());
    }

    enum
    {
        kVST2MaxChannels = 8
    };

private:
    //==============================================================================
    AudioParameterFloat* threshold;
    AudioParameterFloat* alpha;
    int sampleCountDown;

    float lowPassCoeff;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NoiseGate)
};

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new NoiseGate();
}

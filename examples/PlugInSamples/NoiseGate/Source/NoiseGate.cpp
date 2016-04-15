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

#include "../JuceLibraryCode/JuceHeader.h"
#include "../../GenericEditor.h"

class NoiseGate  : public AudioProcessor
{
public:
    //==============================================================================
    NoiseGate()
    {
        addParameter (threshold = new AudioParameterFloat ("threshold", "Threshold", 0.0f, 1.0f, 0.5f));
        addParameter (alpha  = new AudioParameterFloat ("alpha",  "Alpha",   0.0f, 1.0f, 0.8f));

        // add single side-chain bus
        busArrangement.inputBuses.add (AudioProcessorBus ("Sidechain In",  AudioChannelSet::mono()));

        // To be compatible with all VST2 DAWs, it's best to pass through the sidechain
        if (isVST2())
            busArrangement.outputBuses.add (AudioProcessorBus ("Sidechain Out",  AudioChannelSet::mono()));
    }

    ~NoiseGate() {}

    //==============================================================================
    bool setPreferredBusArrangement (bool isInputBus, int busIndex, const AudioChannelSet& preferred) override
    {
        const bool isMainBus   = (busIndex == 0);
        const bool isSideChain = (busIndex == 1);

        const int numChannels = preferred.size();

        // do not allow disabling channels on main bus
        if (numChannels == 0 && isMainBus) return false;

        // VST2 does not natively support sidechains/aux buses.
        // But many DAWs treat the third input of a plug-in
        // as a sidechain. So limit the main bus to stereo!
        if (isVST2())
        {
            if (isMainBus && numChannels != 2) return false;

            // we only allow mono sidechains in VST-2
            if (isSideChain && numChannels != 1)
                return false;
        }

        // always have the same channel layout on both input and output on the main bus
        if (isMainBus && ! AudioProcessor::setPreferredBusArrangement (! isInputBus, busIndex, preferred))
            return false;

        return AudioProcessor::setPreferredBusArrangement (isInputBus, busIndex, preferred);
    }

    //==============================================================================
    void prepareToPlay (double /*sampleRate*/, int /*maxBlockSize*/) override { lowPassCoeff = 0.0f; sampleCountDown = 0; }
    void releaseResources() override                                          {}

    void processBlock (AudioSampleBuffer& buffer, MidiBuffer&) override
    {
        AudioSampleBuffer mainInputOutput = busArrangement.getBusBuffer (buffer, true, 0);
        AudioSampleBuffer sideChainInput  = busArrangement.getBusBuffer (buffer, true, 1);

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

        // VST-2 passes this through so clear the audio in this channel
        sideChainInput.clear();
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

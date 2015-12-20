/*
 ==============================================================================

 GainProcessor.cpp
 Created: 23 Nov 2015 3:08:33pm
 Author:  Fabian Renn

 ==============================================================================
 */

#include "../JuceLibraryCode/JuceHeader.h"
#include "../../GenericEditor.h"

//==============================================================================
/**
 */
class GainProcessor  : public AudioProcessor
{
public:

    //==============================================================================
    GainProcessor()
    {
        addParameter (gain = new AudioParameterFloat ("gain", "Gain", 0.0f, 1.0f, 0.5f));
    }

    ~GainProcessor() {}

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override {}
    void releaseResources() override {}

    void processBlock (AudioSampleBuffer& buffer, MidiBuffer&) override
    {
        buffer.applyGain (*gain);
    }

    //==============================================================================
    AudioProcessorEditor* createEditor() override { return new GenericEditor (*this); }
    bool hasEditor() const override               { return true;   }

    //==============================================================================
    const String getName() const override               { return "Gain PlugIn"; }
    bool acceptsMidi() const override                   { return false; }
    bool producesMidi() const override                  { return false; }
    bool silenceInProducesSilenceOut() const override   { return true; }
    double getTailLengthSeconds() const override        { return 0; }

    //==============================================================================
    int getNumPrograms() override                          { return 1; }
    int getCurrentProgram() override                       { return 0; }
    void setCurrentProgram (int) override                  {}
    const String getProgramName (int) override             { return String(); }
    void changeProgramName (int , const String& ) override { }

    //==============================================================================
    void getStateInformation (MemoryBlock& destData) override
    {
        MemoryOutputStream (destData, true).writeFloat (*gain);
    }

    void setStateInformation (const void* data, int sizeInBytes) override
    {
        gain->setValueNotifyingHost (MemoryInputStream (data, sizeInBytes, false).readFloat());
    }

    //==============================================================================
    bool setPreferredBusArrangement (bool isInputBus, int busIndex,
                                     const AudioChannelSet& preferred) override
    {
        const int numChannels = preferred.size();

        // do not allow disabling channels
        if (numChannels == 0) return false;

        // always have the same channel layout on both input and output on the main bus
        if (! AudioProcessor::setPreferredBusArrangement (! isInputBus, busIndex, preferred))
            return false;

        return AudioProcessor::setPreferredBusArrangement (isInputBus, busIndex, preferred);
    }

private:
    //==============================================================================
    AudioParameterFloat* gain;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GainProcessor)
};

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new GainProcessor();
}

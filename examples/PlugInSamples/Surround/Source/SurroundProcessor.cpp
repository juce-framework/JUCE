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
#include "SurroundEditor.h"

//==============================================================================
/**
 */
class SurroundProcessor  : public AudioProcessor,
                           public ChannelClickListener
{
public:
    SurroundProcessor() {}
    ~SurroundProcessor() {}

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override
    {
        channelClicked = 0;
        sampleOffset = static_cast<int> (std::ceil (sampleRate));

        ignoreUnused (samplesPerBlock);
    }

    void releaseResources() override {}

    void processBlock (AudioSampleBuffer& buffer, MidiBuffer&) override
    {
        buffer.clear();

        const int fillSamples = jmin (static_cast<int> (std::ceil (getSampleRate())) - sampleOffset,
                                      buffer.getNumSamples());

        float* const channelBuffer = buffer.getWritePointer (channelClicked);
        const float freq = (float) (440.0 / getSampleRate());

        for (int i = 0; i < fillSamples; ++i)
            channelBuffer[i] = std::sin (2.0f * float_Pi * freq * static_cast<float> (sampleOffset++));
    }

    //==============================================================================
    AudioProcessorEditor* createEditor() override { return new SurroundEditor (*this); }
    bool hasEditor() const override               { return true;   }

    //==============================================================================
    bool setPreferredBusArrangement (bool isInputBus, int busIndex,
                                     const AudioChannelSet& preferred) override
    {
        if  (   preferred == AudioChannelSet::mono()
             || preferred == AudioChannelSet::stereo()
             || preferred == AudioChannelSet::createLCR()
             || preferred == AudioChannelSet::createLCRS()
             || preferred == AudioChannelSet::quadraphonic()
             || preferred == AudioChannelSet::pentagonal()
             || preferred == AudioChannelSet::hexagonal()
             || preferred == AudioChannelSet::octagonal()
             || preferred == AudioChannelSet::ambisonic()
             || preferred == AudioChannelSet::create5point0()
             || preferred == AudioChannelSet::create5point1()
             || preferred == AudioChannelSet::create6point0()
             || preferred == AudioChannelSet::create6point1()
             || preferred == AudioChannelSet::create7point0()
             || preferred == AudioChannelSet::create7point1()
             || preferred == AudioChannelSet::createFront7point0()
             || preferred == AudioChannelSet::createFront7point1())
            return AudioProcessor::setPreferredBusArrangement (isInputBus, busIndex, preferred);

        return false;
    }

    //==============================================================================
    const String getName() const override               { return "Surround PlugIn"; }
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
    void getStateInformation (MemoryBlock&) override {}
    void setStateInformation (const void* , int) override {}

    void channelButtonClicked (int channelIndex) override
    {
        channelClicked = channelIndex;
        sampleOffset = 0;
    }

private:
    int channelClicked;
    int sampleOffset;
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SurroundProcessor)
};

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SurroundProcessor();
}

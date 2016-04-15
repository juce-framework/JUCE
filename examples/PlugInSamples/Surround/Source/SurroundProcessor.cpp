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

        const int numChannels = busArrangement.inputBuses.getReference(0).channels.size();
        channelActive.resize (numChannels);
        alphaCoeffs.resize (numChannels);
        reset();

        ignoreUnused (samplesPerBlock);
    }

    void releaseResources() override { reset(); }

    void processBlock (AudioSampleBuffer& buffer, MidiBuffer&) override
    {
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        {
            int& channelTime = channelActive.getReference (ch);
            float& alpha = alphaCoeffs.getReference (ch);

            for (int j = 0; j < buffer.getNumSamples(); ++j)
            {
                float sample = buffer.getReadPointer (ch)[j];
                alpha = (0.8f * alpha) + (0.2f * sample);

                if (fabsf (alpha) >= 0.1f)
                    channelTime = static_cast<int> (getSampleRate() / 2.0);
            }

            channelTime = jmax (0, channelTime - buffer.getNumSamples());
        }

        const int fillSamples = jmin (static_cast<int> (std::ceil (getSampleRate())) - sampleOffset,
                                      buffer.getNumSamples());

        float* const channelBuffer = buffer.getWritePointer (channelClicked);
        const float freq = (float) (440.0 / getSampleRate());

        for (int i = 0; i < fillSamples; ++i)
            channelBuffer[i] += std::sin (2.0f * float_Pi * freq * static_cast<float> (sampleOffset++));
    }

    //==============================================================================
    AudioProcessorEditor* createEditor() override { return new SurroundEditor (*this); }
    bool hasEditor() const override               { return true;   }

    //==============================================================================
    bool setPreferredBusArrangement (bool isInputBus, int busIndex,
                                     const AudioChannelSet& preferred) override
    {
        if  (! preferred.isDiscreteLayout())
        {
            if (! AudioProcessor::setPreferredBusArrangement (! isInputBus, busIndex, preferred))
                return false;

            return AudioProcessor::setPreferredBusArrangement (isInputBus, busIndex, preferred);
        }

        return false;
    }

    void reset() override
    {
        for (int i = 0; i < channelActive.size(); ++i)
            channelActive.getReference (i) = 0;
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

    bool isChannelActive (int channelIndex) override
    {
        return channelActive [channelIndex] > 0;
    }

private:
    Array<int> channelActive;
    Array<float> alphaCoeffs;
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

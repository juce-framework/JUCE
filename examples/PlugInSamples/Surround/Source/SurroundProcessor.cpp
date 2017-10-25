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
#include "SurroundEditor.h"

//==============================================================================
/**
 */
class SurroundProcessor  : public AudioProcessor,
                           public ChannelClickListener,
                           private AsyncUpdater
{
public:
    SurroundProcessor()
        : AudioProcessor(BusesProperties().withInput  ("Input",  AudioChannelSet::stereo())
                                          .withOutput ("Output", AudioChannelSet::stereo()))
    {}

    ~SurroundProcessor() {}

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override
    {
        channelClicked = 0;
        sampleOffset = static_cast<int> (std::ceil (sampleRate));

        const int numChannels = getChannelCountOfBus (true, 0);
        channelActive.resize (numChannels);
        alphaCoeffs.resize (numChannels);
        reset();

        triggerAsyncUpdate();

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

                if (std::abs (alpha) >= 0.1f)
                    channelTime = static_cast<int> (getSampleRate() / 2.0);
            }

            channelTime = jmax (0, channelTime - buffer.getNumSamples());
        }

        const int fillSamples = jmin (static_cast<int> (std::ceil (getSampleRate())) - sampleOffset,
                                      buffer.getNumSamples());

        if (isPositiveAndBelow (channelClicked, buffer.getNumChannels()))
        {
            float* const channelBuffer = buffer.getWritePointer (channelClicked);
            const float freq = (float) (440.0 / getSampleRate());

            for (int i = 0; i < fillSamples; ++i)
                channelBuffer[i] += std::sin (2.0f * float_Pi * freq * static_cast<float> (sampleOffset++));
        }
    }

    //==============================================================================
    AudioProcessorEditor* createEditor() override { return new SurroundEditor (*this); }
    bool hasEditor() const override               { return true;   }

    //==============================================================================
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override
    {
        return ((! layouts.getMainInputChannelSet() .isDiscreteLayout())
             && (! layouts.getMainOutputChannelSet().isDiscreteLayout())
             && (layouts.getMainInputChannelSet() == layouts.getMainOutputChannelSet())
             && (! layouts.getMainInputChannelSet().isDisabled()));
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

    void handleAsyncUpdate() override
    {
        if (AudioProcessorEditor* editor = getActiveEditor())
            if (SurroundEditor* surroundEditor = dynamic_cast<SurroundEditor*> (editor))
                surroundEditor->updateGUI();
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

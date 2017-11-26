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

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

#include <array>


// A simple Inter-App Audio plug-in with a gain control and some meters.
class IAAEffectProcessor  : public AudioProcessor
{
public:
    IAAEffectProcessor();
    ~IAAEffectProcessor();

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    void processBlock (AudioBuffer<float>&, MidiBuffer&) override;

    //==============================================================================
    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const String getProgramName (int index) override;
    void changeProgramName (int index, const String& newName) override;

    //==============================================================================
    void getStateInformation (MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==============================================================================
    bool updateCurrentTimeInfoFromHost (AudioPlayHead::CurrentPositionInfo&);

    // Allow an IAAAudioProcessorEditor to register as a listener to receive new
    // meter values directly from the audio thread.
    struct MeterListener
    {
        virtual ~MeterListener() {};

        virtual void handleNewMeterValue (int, float) = 0;
    };

    void addMeterListener    (MeterListener& listener) { meterListeners.add    (&listener); };
    void removeMeterListener (MeterListener& listener) { meterListeners.remove (&listener); };


private:
    //==============================================================================
    AudioProcessorValueTreeState parameters;
    float previousGain = 0.0;
    std::array <float, 2> meterValues = { { 0, 0 } };

    // This keeps a copy of the last set of timing info that was acquired during an
    // audio callback - the UI component will display this.
    AudioPlayHead::CurrentPositionInfo lastPosInfo;

    ListenerList<MeterListener> meterListeners;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (IAAEffectProcessor)
};

/*
  ==============================================================================

   This file is part of the JUCE examples.
   Copyright (c) 2017 - ROLI Ltd.

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES,
   WHETHER EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR
   PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

/*******************************************************************************
 The block below describes the properties of this PIP. A PIP is a short snippet
 of code that can be read by the Projucer and used to generate a JUCE project.

 BEGIN_JUCE_PIP_METADATA

 name:             OscillatorDemo
 version:          1.0.0
 vendor:           JUCE
 website:          http://juce.com
 description:      Oscillator demo using the DSP module.

 dependencies:     juce_audio_basics, juce_audio_devices, juce_audio_formats,
                   juce_audio_processors, juce_audio_utils, juce_core,
                   juce_data_structures, juce_dsp, juce_events, juce_graphics,
                   juce_gui_basics, juce_gui_extra
 exporters:        xcode_mac, vs2017, linux_make

 moduleFlags:      JUCE_STRICT_REFCOUNTEDPOINTER=1

 type:             Component
 mainClass:        OscillatorDemo

 useLocalCopy:     1

 END_JUCE_PIP_METADATA

*******************************************************************************/

#pragma once

#include "../Assets/DemoUtilities.h"
#include "../Assets/DSPDemos_Common.h"

using namespace dsp;

//==============================================================================
struct OscillatorDemoDSP
{
    void prepare (const ProcessSpec& spec)
    {
        gain.setGainDecibels (-6.0f);

        for (auto&& oscillator : oscillators)
        {
            oscillator.setFrequency (440.f);
            oscillator.prepare (spec);
        }

        updateParameters();

        tempBuffer = AudioBlock<float> (tempBufferMemory, spec.numChannels, spec.maximumBlockSize);
    }

    void process (const ProcessContextReplacing<float>& context)
    {
        tempBuffer.copy (context.getInputBlock());
        tempBuffer.multiply (static_cast<float> (fileMix));

        oscillators[currentOscillatorIdx].process (context);
        context.getOutputBlock().multiply (static_cast<float> (1.0 - fileMix));

        context.getOutputBlock().add (tempBuffer);

        gain.process (context);
    }

    void reset()
    {
        oscillators[currentOscillatorIdx].reset();
    }

    void updateParameters()
    {
        currentOscillatorIdx = jmin (numElementsInArray (oscillators),
                                     3 * (accuracy.getCurrentSelectedID() - 1) + (typeParam.getCurrentSelectedID() - 1));

        auto freq = static_cast<float> (freqParam.getCurrentValue());

        for (auto&& oscillator : oscillators)
            oscillator.setFrequency (freq);

        gain.setGainDecibels (static_cast<float> (gainParam.getCurrentValue()));

        fileMix = mixParam.getCurrentValue();
    }

    //==============================================================================
    Oscillator<float> oscillators[6] =
    {
        // No Approximation
        {[] (float x) { return std::sin (x); }},                   // sine
        {[] (float x) { return x / MathConstants<float>::pi; }},   // saw
        {[] (float x) { return x < 0.0f ? -1.0f : 1.0f; }},        // square

        // Approximated by a wave-table
        {[] (float x) { return std::sin (x); }, 100},                 // sine
        {[] (float x) { return x / MathConstants<float>::pi; }, 100}, // saw
        {[] (float x) { return x < 0.0f ? -1.0f : 1.0f; }, 100}       // square
    };

    int currentOscillatorIdx = 0;
    Gain<float> gain;

    ChoiceParameter typeParam { {"sine", "saw", "square"}, 1, "Type" };
    ChoiceParameter accuracy  { {"No Approximation", "Use Wavetable"}, 1, "Accuracy" };
    SliderParameter freqParam { { 20.0, 24000.0 }, 0.4, 440.0, "Frequency", "Hz" };
    SliderParameter gainParam { { -100.0, 20.0 }, 3.0, -20.0, "Gain", "dB" };
    SliderParameter mixParam  { { 0.0, 1.0 }, 1.0, 0.0, "File mix" };

    HeapBlock<char> tempBufferMemory;
    AudioBlock<float> tempBuffer;
    double fileMix;

    std::vector<DSPDemoParameterBase*> parameters { &typeParam, &accuracy, &freqParam, &gainParam, &mixParam };
};

struct OscillatorDemo    : public Component
{
    OscillatorDemo()
    {
        addAndMakeVisible (fileReaderComponent);
        setSize (750, 500);
    }

    void resized() override
    {
        fileReaderComponent.setBounds (getLocalBounds());
    }

    AudioFileReaderComponent<OscillatorDemoDSP> fileReaderComponent;
};

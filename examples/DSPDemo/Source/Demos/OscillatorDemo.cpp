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

#include "../DSPDemo.h"

//==============================================================================
// @@ START_DEMO
struct OscillatorDemo
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
// @@ END_DEMO

RegisterDSPDemo<OscillatorDemo> oscillatorDemo ("Oscillator", BinaryData::OscillatorDemo_cpp);

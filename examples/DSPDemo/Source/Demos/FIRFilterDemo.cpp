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
struct FIRFilterDemo
{
    void prepare (const ProcessSpec& spec)
    {
        sampleRate = spec.sampleRate;

        fir.state = FilterDesign<float>::designFIRLowpassWindowMethod (440.0f, sampleRate, 21,
                                                                       WindowingFunction<float>::blackman);
        fir.prepare (spec);
    }

    void process (const ProcessContextReplacing<float>& context)
    {
        fir.process (context);
    }

    void reset()
    {
        fir.reset();
    }

    void updateParameters()
    {
        if (sampleRate != 0)
        {
            auto cutoff = static_cast<float> (cutoffParam.getCurrentValue());
            auto windowingMethod = static_cast<WindowingFunction<float>::WindowingMethod> (typeParam.getCurrentSelectedID() - 1);

            *fir.state = *FilterDesign<float>::designFIRLowpassWindowMethod (cutoff, sampleRate, 21, windowingMethod);
        }
    }

    //==============================================================================
    ProcessorDuplicator<FIR::Filter<float>, FIR::Coefficients<float>> fir;

    double sampleRate = 0;

    SliderParameter cutoffParam { { 20.0, 20000.0 }, 0.4, 440.0f, "Cutoff", "Hz" };
    ChoiceParameter typeParam { { "Rectangular", "Triangular", "Hann", "Hamming", "Blackman", "Blackman-Harris", "Flat Top", "Kaiser" },
                                5, "Windowing Function" };

    std::vector<DSPDemoParameterBase*> parameters { &cutoffParam, &typeParam };
};
// @@ END_DEMO

RegisterDSPDemo<FIRFilterDemo> firDemo ("FIR Filter", BinaryData::FIRFilterDemo_cpp);

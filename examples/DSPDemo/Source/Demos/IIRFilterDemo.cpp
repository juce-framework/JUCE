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
struct IIRFilterDemo
{
    void prepare (const ProcessSpec& spec)
    {
        sampleRate = spec.sampleRate;

        iir.state = IIR::Coefficients<float>::makeLowPass (sampleRate, 440.0);
        iir.prepare (spec);
    }

    void process (const ProcessContextReplacing<float>& context)
    {
        iir.process (context);
    }

    void reset()
    {
        iir.reset();
    }

    void updateParameters()
    {
        if (sampleRate != 0)
        {
            auto cutoff = static_cast<float> (cutoffParam.getCurrentValue());
            auto qVal   = static_cast<float> (qParam.getCurrentValue());

            switch (typeParam.getCurrentSelectedID())
            {
                case 1:     *iir.state = *IIR::Coefficients<float>::makeLowPass  (sampleRate, cutoff, qVal); break;
                case 2:     *iir.state = *IIR::Coefficients<float>::makeHighPass (sampleRate, cutoff, qVal); break;
                case 3:     *iir.state = *IIR::Coefficients<float>::makeBandPass (sampleRate, cutoff, qVal); break;
                default:    break;
            }
        }
    }

    //==============================================================================
    ProcessorDuplicator<IIR::Filter<float>, IIR::Coefficients<float>> iir;

    ChoiceParameter typeParam { { "Low-pass", "High-pass", "Band-pass"}, 1, "Type" };
    SliderParameter cutoffParam { { 20.0, 20000.0 }, 0.5, 440.0f, "Cutoff", "Hz" };
    SliderParameter qParam { { 0.3, 20.0 }, 0.5, 1.0 / std::sqrt(2.0), "Q" };

    std::vector<DSPDemoParameterBase*> parameters { &typeParam, &cutoffParam, &qParam };
    double sampleRate = 0;
};
// @@ END_DEMO

RegisterDSPDemo<IIRFilterDemo> iirDemo ("IIR Filter", BinaryData::IIRFilterDemo_cpp);

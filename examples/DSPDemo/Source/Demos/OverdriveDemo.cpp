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
struct OverdriveDemo
{
    void prepare (const ProcessSpec& spec)
    {
        sampleRate = spec.sampleRate;

        auto& gainUp = overdrive.get<0>();
        gainUp.setGainDecibels (24);

        auto& bias = overdrive.get<1>();
        bias.setBias (0.4f);

        auto& wavShaper = overdrive.get<2>();
        wavShaper.functionToUse = std::tanh;

        auto& dcFilter = overdrive.get<3>();
        dcFilter.state = IIR::Coefficients<float>::makeHighPass (sampleRate, 5.0);

        auto& gainDown = overdrive.get<4>();
        gainDown.setGainDecibels (-18.0f);

        overdrive.prepare (spec);
    }

    void process (const ProcessContextReplacing<float>& context)
    {
        overdrive.process (context);
    }

    void reset()
    {
        overdrive.reset();
    }

    void updateParameters()
    {
        if (sampleRate != 0)
        {
            overdrive.get<0>().setGainDecibels (static_cast<float> (inGainParam.getCurrentValue()));
            overdrive.get<4>().setGainDecibels (static_cast<float> (outGainParam.getCurrentValue()));
        }
    }

    //==============================================================================
    using GainProcessor   = Gain<float>;
    using BiasProcessor   = Bias<float>;
    using DriveProcessor  = WaveShaper<float>;
    using DCFilter        = ProcessorDuplicator<IIR::Filter<float>,
                                                IIR::Coefficients<float>>;

    ProcessorChain<GainProcessor, BiasProcessor, DriveProcessor, DCFilter, GainProcessor> overdrive;

    SliderParameter inGainParam  { { -100.0, 60.0 }, 3, 24.0,  "Input Gain",  "dB" };
    SliderParameter outGainParam { { -100.0, 20.0 }, 3, -18.0, "Output Gain", "dB" };

    std::vector<DSPDemoParameterBase*> parameters { &inGainParam, &outGainParam };
    double sampleRate = 0;
};
// @@ END_DEMO

RegisterDSPDemo<OverdriveDemo> overdriveDemo ("Overdrive", BinaryData::OverdriveDemo_cpp);

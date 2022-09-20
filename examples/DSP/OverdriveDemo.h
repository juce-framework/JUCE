/*
  ==============================================================================

   This file is part of the JUCE examples.
   Copyright (c) 2022 - Raw Material Software Limited

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

 name:             OverdriveDemo
 version:          1.0.0
 vendor:           JUCE
 website:          http://juce.com
 description:      Overdrive demo using the DSP module.

 dependencies:     juce_audio_basics, juce_audio_devices, juce_audio_formats,
                   juce_audio_processors, juce_audio_utils, juce_core,
                   juce_data_structures, juce_dsp, juce_events, juce_graphics,
                   juce_gui_basics, juce_gui_extra
 exporters:        xcode_mac, vs2022, linux_make

 moduleFlags:      JUCE_STRICT_REFCOUNTEDPOINTER=1

 type:             Component
 mainClass:        OverdriveDemo

 useLocalCopy:     1

 END_JUCE_PIP_METADATA

*******************************************************************************/

#pragma once

#include "../Assets/DemoUtilities.h"
#include "../Assets/DSPDemos_Common.h"

using namespace dsp;

//==============================================================================
struct OverdriveDemoDSP
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
        if (sampleRate != 0.0)
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
    double sampleRate = 0.0;
};

struct OverdriveDemo    : public Component
{
    OverdriveDemo()
    {
        addAndMakeVisible (fileReaderComponent);
        setSize (750, 500);
    }

    void resized() override
    {
        fileReaderComponent.setBounds (getLocalBounds());
    }

    AudioFileReaderComponent<OverdriveDemoDSP> fileReaderComponent;
};

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

 name:             IIRFilterDemo
 version:          1.0.0
 vendor:           JUCE
 website:          http://juce.com
 description:      IIR filter demo using the DSP module.

 dependencies:     juce_audio_basics, juce_audio_devices, juce_audio_formats,
                   juce_audio_processors, juce_audio_utils, juce_core,
                   juce_data_structures, juce_dsp, juce_events, juce_graphics,
                   juce_gui_basics, juce_gui_extra
 exporters:        xcode_mac, vs2022, linux_make

 moduleFlags:      JUCE_STRICT_REFCOUNTEDPOINTER=1

 type:             Component
 mainClass:        IIRFilterDemo

 useLocalCopy:     1

 END_JUCE_PIP_METADATA

*******************************************************************************/

#pragma once

#include "../Assets/DemoUtilities.h"
#include "../Assets/DSPDemos_Common.h"

using namespace dsp;

//==============================================================================
struct IIRFilterDemoDSP
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
        if (! approximatelyEqual (sampleRate, 0.0))
        {
            auto cutoff = static_cast<float> (cutoffParam.getCurrentValue());
            auto qVal   = static_cast<float> (qParam.getCurrentValue());

            switch (typeParam.getCurrentSelectedID())
            {
                case 1:     *iir.state = IIR::ArrayCoefficients<float>::makeLowPass  (sampleRate, cutoff, qVal); break;
                case 2:     *iir.state = IIR::ArrayCoefficients<float>::makeHighPass (sampleRate, cutoff, qVal); break;
                case 3:     *iir.state = IIR::ArrayCoefficients<float>::makeBandPass (sampleRate, cutoff, qVal); break;
                default:    break;
            }
        }
    }

    //==============================================================================
    ProcessorDuplicator<IIR::Filter<float>, IIR::Coefficients<float>> iir;

    ChoiceParameter typeParam { { "Low-pass", "High-pass", "Band-pass" }, 1, "Type" };
    SliderParameter cutoffParam { { 20.0, 20000.0 }, 0.5, 440.0f, "Cutoff", "Hz" };
    SliderParameter qParam { { 0.3, 20.0 }, 0.5, 1.0 / std::sqrt(2.0), "Q" };

    std::vector<DSPDemoParameterBase*> parameters { &typeParam, &cutoffParam, &qParam };
    double sampleRate = 0.0;
};

struct IIRFilterDemo    : public Component
{
    IIRFilterDemo()
    {
        addAndMakeVisible (fileReaderComponent);
        setSize (750, 500);
    }

    void resized() override
    {
        fileReaderComponent.setBounds (getLocalBounds());
    }

    AudioFileReaderComponent<IIRFilterDemoDSP> fileReaderComponent;
};

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

 name:             FIRFilterDemo
 version:          1.0.0
 vendor:           JUCE
 website:          http://juce.com
 description:      FIR filter demo using the DSP module.

 dependencies:     juce_audio_basics, juce_audio_devices, juce_audio_formats,
                   juce_audio_processors, juce_audio_utils, juce_core,
                   juce_data_structures, juce_dsp, juce_events, juce_graphics,
                   juce_gui_basics, juce_gui_extra
 exporters:        xcode_mac, vs2022, linux_make

 moduleFlags:      JUCE_STRICT_REFCOUNTEDPOINTER=1

 type:             Component
 mainClass:        FIRFilterDemo

 useLocalCopy:     1

 END_JUCE_PIP_METADATA

*******************************************************************************/

#pragma once

#include "../Assets/DemoUtilities.h"
#include "../Assets/DSPDemos_Common.h"

using namespace dsp;

//==============================================================================
struct FIRFilterDemoDSP
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
        if (sampleRate != 0.0)
        {
            auto cutoff = static_cast<float> (cutoffParam.getCurrentValue());
            auto windowingMethod = static_cast<WindowingFunction<float>::WindowingMethod> (typeParam.getCurrentSelectedID() - 1);

            *fir.state = *FilterDesign<float>::designFIRLowpassWindowMethod (cutoff, sampleRate, 21, windowingMethod);
        }
    }

    //==============================================================================
    ProcessorDuplicator<FIR::Filter<float>, FIR::Coefficients<float>> fir;

    double sampleRate = 0.0;

    SliderParameter cutoffParam { { 20.0, 20000.0 }, 0.4, 440.0f, "Cutoff", "Hz" };
    ChoiceParameter typeParam { { "Rectangular", "Triangular", "Hann", "Hamming", "Blackman", "Blackman-Harris", "Flat Top", "Kaiser" },
                                5, "Windowing Function" };

    std::vector<DSPDemoParameterBase*> parameters { &cutoffParam, &typeParam };
};

struct FIRFilterDemo    : public Component
{
    FIRFilterDemo()
    {
        addAndMakeVisible (fileReaderComponent);
        setSize (750, 500);
    }

    void resized() override
    {
        fileReaderComponent.setBounds (getLocalBounds());
    }

    AudioFileReaderComponent<FIRFilterDemoDSP> fileReaderComponent;
};

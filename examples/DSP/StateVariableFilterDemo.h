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

 name:             StateVariableFilterDemo
 version:          1.0.0
 vendor:           JUCE
 website:          http://juce.com
 description:      State variable filter demo using the DSP module.

 dependencies:     juce_audio_basics, juce_audio_devices, juce_audio_formats,
                   juce_audio_processors, juce_audio_utils, juce_core,
                   juce_data_structures, juce_dsp, juce_events, juce_graphics,
                   juce_gui_basics, juce_gui_extra
 exporters:        xcode_mac, vs2017, linux_make

 moduleFlags:      JUCE_STRICT_REFCOUNTEDPOINTER=1

 type:             Component
 mainClass:        StateVariableFilterDemo

 useLocalCopy:     1

 END_JUCE_PIP_METADATA

*******************************************************************************/

#pragma once

#include "../Assets/DemoUtilities.h"
#include "../Assets/DSPDemos_Common.h"

using namespace dsp;

//==============================================================================
struct StateVariableFilterDemoDSP
{
    void prepare (const ProcessSpec& spec)
    {
        sampleRate = spec.sampleRate;

        filter.state = new StateVariableFilter::Parameters<float>;
        filter.prepare (spec);
    }

    void process (const ProcessContextReplacing<float>& context)
    {
        filter.process (context);
    }

    void reset()
    {
        filter.reset();
    }

    void updateParameters()
    {
        if (sampleRate != 0.0)
        {
            auto cutoff    = static_cast<float> (cutoffParam.getCurrentValue());
            auto resonance = static_cast<float> (qParam.getCurrentValue());
            auto type      = static_cast<StateVariableFilter::Parameters<float>::Type> (typeParam.getCurrentSelectedID() - 1);

            filter.state->type = type;
            filter.state->setCutOffFrequency (sampleRate, cutoff, resonance);
        }
    }

    //==============================================================================
    ProcessorDuplicator<StateVariableFilter::Filter<float>,
                        StateVariableFilter::Parameters<float>> filter;

    ChoiceParameter typeParam {{ "Low-pass", "Band-pass", "High-pass" }, 1, "Type" };
    SliderParameter cutoffParam {{ 20.0, 20000.0 }, 0.5, 440.0f, "Cutoff", "Hz" };
    SliderParameter qParam {{ 0.3, 20.0 }, 0.5, 1.0 / MathConstants<double>::sqrt2, "Resonance" };

    std::vector<DSPDemoParameterBase*> parameters { &typeParam, &cutoffParam, &qParam };
    double sampleRate = 0.0;
};

struct StateVariableFilterDemo    : public Component
{
    StateVariableFilterDemo()
    {
        addAndMakeVisible (fileReaderComponent);
        setSize (750, 500);
    }

    void resized() override
    {
        fileReaderComponent.setBounds (getLocalBounds());
    }

    AudioFileReaderComponent<StateVariableFilterDemoDSP> fileReaderComponent;
};

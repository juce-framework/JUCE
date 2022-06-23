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

 name:             WaveShaperTanhDemo
 version:          1.0.0
 vendor:           JUCE
 website:          http://juce.com
 description:      Wave shaper tanh demo using the DSP module.

 dependencies:     juce_audio_basics, juce_audio_devices, juce_audio_formats,
                   juce_audio_processors, juce_audio_utils, juce_core,
                   juce_data_structures, juce_dsp, juce_events, juce_graphics,
                   juce_gui_basics, juce_gui_extra
 exporters:        xcode_mac, vs2022, linux_make

 moduleFlags:      JUCE_STRICT_REFCOUNTEDPOINTER=1

 type:             Component
 mainClass:        WaveShaperTanhDemo

 useLocalCopy:     1

 END_JUCE_PIP_METADATA

*******************************************************************************/

#pragma once

#include "../Assets/DemoUtilities.h"
#include "../Assets/DSPDemos_Common.h"

using namespace dsp;

//==============================================================================
struct WaveShaperTanhDemoDSP
{
    void prepare (const ProcessSpec&) {}

    void process (const ProcessContextReplacing<float>& context)
    {
        shapers[currentShaperIdx].process (context);
    }

    void reset()
    {
        for (auto&& shaper : shapers)
            shaper.reset();
    }

    void updateParameters()
    {
        currentShaperIdx = jmin (numElementsInArray (shapers), accuracy.getCurrentSelectedID() - 1);
    }

    //==============================================================================
    WaveShaper<float> shapers[2] { { std::tanh }, { FastMathApproximations::tanh } };
    int currentShaperIdx = 0;

    ChoiceParameter accuracy {{ "No Approximation", "Use fast-math approximation" }, 1, "Accuracy" };
    std::vector<DSPDemoParameterBase*> parameters { &accuracy };  // no params for this demo
};

struct WaveShaperTanhDemo    : public Component
{
    WaveShaperTanhDemo()
    {
        addAndMakeVisible (fileReaderComponent);
        setSize (750, 500);
    }

    void resized() override
    {
        fileReaderComponent.setBounds (getLocalBounds());
    }

    AudioFileReaderComponent<WaveShaperTanhDemoDSP> fileReaderComponent;
};

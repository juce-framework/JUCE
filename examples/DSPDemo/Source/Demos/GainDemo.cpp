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
struct GainDemo
{
    void prepare (const ProcessSpec&)
    {
        gain.setGainDecibels (-6.0f);
    }

    void process (const ProcessContextReplacing<float>& context)
    {
        gain.process (context);
    }

    void reset()
    {
        gain.reset();
    }

    void updateParameters()
    {
        gain.setGainDecibels (static_cast<float> (gainParam.getCurrentValue()));
    }

    //==============================================================================
    Gain<float> gain;
    SliderParameter gainParam { { -100.0, 20.0 }, 3.0, -6.0, "Gain", "dB" };

    std::vector<DSPDemoParameterBase*> parameters { &gainParam };
};
// @@ END_DEMO

RegisterDSPDemo<GainDemo> gainDemo ("Gain", BinaryData::GainDemo_cpp);

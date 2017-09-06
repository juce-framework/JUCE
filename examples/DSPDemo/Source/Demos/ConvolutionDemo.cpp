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
struct ConvolutionDemo
{
    void prepare (const ProcessSpec& spec)
    {
        sampleRate = spec.sampleRate;
        convolution.prepare (spec);
        updateParameters();
    }

    void process (ProcessContextReplacing<float> context)
    {
        context.isBypassed = bypass;
        convolution.process (context);
    }

    void reset()
    {
        convolution.reset();
    }

    void updateParameters()
    {
        if (auto* cabinetTypeParameter = dynamic_cast<ChoiceParameter*> (parameters[0]))
        {
            if (cabinetTypeParameter->getCurrentSelectedID() == 1)
            {
                bypass = true;
            }
            else
            {
                bypass = false;

                auto maxSize = static_cast<size_t> (roundDoubleToInt (8192.0 * sampleRate / 44100.0));

                if (cabinetTypeParameter->getCurrentSelectedID() == 2)
                    convolution.loadImpulseResponse (BinaryData::guitar_amp_wav,
                                                     BinaryData::guitar_amp_wavSize,
                                                     false, true, maxSize);
                else
                    convolution.loadImpulseResponse (BinaryData::cassette_recorder_wav,
                                                     BinaryData::cassette_recorder_wavSize,
                                                     false, true, maxSize);
            }
        }
    }

    //==============================================================================
    double sampleRate = 0;
    bool bypass = false;

    Convolution convolution;

    ChoiceParameter cabinetParam { {"Bypass", "Guitar amplifier 8''", "Cassette recorder"}, 1, "Cabinet Type" };

    std::vector<DSPDemoParameterBase*> parameters { &cabinetParam };
};
// @@ END_DEMO

RegisterDSPDemo<ConvolutionDemo> convolutionDemo ("Convolution", BinaryData::ConvolutionDemo_cpp);

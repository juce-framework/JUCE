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

 name:             SIMDRegisterDemo
 version:          1.0.0
 vendor:           JUCE
 website:          http://juce.com
 description:      SIMD register demo using the DSP module.

 dependencies:     juce_audio_basics, juce_audio_devices, juce_audio_formats,
                   juce_audio_processors, juce_audio_utils, juce_core,
                   juce_data_structures, juce_dsp, juce_events, juce_graphics,
                   juce_gui_basics, juce_gui_extra
 exporters:        xcode_mac, vs2017, linux_make

 moduleFlags:      JUCE_STRICT_REFCOUNTEDPOINTER=1

 type:             Component
 mainClass:        SIMDRegisterDemo

 useLocalCopy:     1

 END_JUCE_PIP_METADATA

*******************************************************************************/

#pragma once

#include "../Assets/DemoUtilities.h"
#include "../Assets/DSPDemos_Common.h"

using namespace dsp;

//==============================================================================
struct SIMDRegisterDemoDSP
{
    void prepare (const ProcessSpec& spec)
    {
        sampleRate = spec.sampleRate;

        iirCoefficients = IIR::Coefficients<float>::makeLowPass (sampleRate, 440.0f);
        iir.reset (new IIR::Filter<SIMDRegister<float>> (iirCoefficients));

        interleaved = AudioBlock<SIMDRegister<float>> (interleavedBlockData, 1, spec.maximumBlockSize);
        zero        = AudioBlock<float> (zeroData, SIMDRegister<float>::size(), spec.maximumBlockSize);

        zero.clear();

        auto monoSpec = spec;
        monoSpec.numChannels = 1;
        iir->prepare (monoSpec);
    }

    void process (const ProcessContextReplacing<float>& context)
    {
        jassert (context.getInputBlock().getNumSamples()  == context.getOutputBlock().getNumSamples());
        jassert (context.getInputBlock().getNumChannels() == context.getOutputBlock().getNumChannels());

        auto& input  = context.getInputBlock();
        auto& output = context.getOutputBlock();
        auto n = input.getNumSamples();
        auto* inout = channelPointers.getData();


        for (size_t ch = 0; ch < SIMDRegister<float>::size(); ++ch)
            inout[ch] = (ch < input.getNumChannels() ? const_cast<float*> (input.getChannelPointer (ch)) : zero.getChannelPointer (ch));

        AudioDataConverters::interleaveSamples (inout, reinterpret_cast<float*> (interleaved.getChannelPointer (0)),
                                                static_cast<int> (n), static_cast<int> (SIMDRegister<float>::size()));


        iir->process (ProcessContextReplacing<SIMDRegister<float>> (interleaved));


        for (size_t ch = 0; ch < input.getNumChannels(); ++ch)
            inout[ch] = output.getChannelPointer (ch);

        AudioDataConverters::deinterleaveSamples (reinterpret_cast<float*> (interleaved.getChannelPointer (0)),
                                                  const_cast<float**> (inout),
                                                  static_cast<int> (n), static_cast<int> (SIMDRegister<float>::size()));
    }

    void reset()
    {
        iir.reset();
    }

    void updateParameters()
    {
        if (sampleRate != 0.0)
        {
            auto cutoff = static_cast<float> (cutoffParam.getCurrentValue());
            auto qVal   = static_cast<float> (qParam.getCurrentValue());

            switch (typeParam.getCurrentSelectedID())
            {
                case 1:   *iirCoefficients = *IIR::Coefficients<float>::makeLowPass  (sampleRate, cutoff, qVal); break;
                case 2:   *iirCoefficients = *IIR::Coefficients<float>::makeHighPass (sampleRate, cutoff, qVal); break;
                case 3:   *iirCoefficients = *IIR::Coefficients<float>::makeBandPass (sampleRate, cutoff, qVal); break;
                default:  break;
            }
        }
    }

    //==============================================================================
    IIR::Coefficients<float>::Ptr iirCoefficients;
    std::unique_ptr<IIR::Filter<SIMDRegister<float>>> iir;

    AudioBlock<SIMDRegister<float>> interleaved;
    AudioBlock<float> zero;

    HeapBlock<char> interleavedBlockData, zeroData;
    HeapBlock<const float*> channelPointers { SIMDRegister<float>::size() };

    ChoiceParameter typeParam { { "Low-pass", "High-pass", "Band-pass" }, 1, "Type" };
    SliderParameter cutoffParam { { 20.0, 20000.0 }, 0.5, 440.0f, "Cutoff", "Hz" };
    SliderParameter qParam { { 0.3, 20.0 }, 0.5, 0.7, "Q" };

    std::vector<DSPDemoParameterBase*> parameters { &typeParam, &cutoffParam, &qParam };
    double sampleRate = 0.0;
};

struct SIMDRegisterDemo    : public Component
{
    SIMDRegisterDemo()
    {
        addAndMakeVisible (fileReaderComponent);
        setSize (750, 500);
    }

    void resized() override
    {
        fileReaderComponent.setBounds (getLocalBounds());
    }

    AudioFileReaderComponent<SIMDRegisterDemoDSP> fileReaderComponent;
};

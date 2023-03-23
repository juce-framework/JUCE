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

 name:             SIMDRegisterDemo
 version:          1.0.0
 vendor:           JUCE
 website:          http://juce.com
 description:      SIMD register demo using the DSP module.

 dependencies:     juce_audio_basics, juce_audio_devices, juce_audio_formats,
                   juce_audio_processors, juce_audio_utils, juce_core,
                   juce_data_structures, juce_dsp, juce_events, juce_graphics,
                   juce_gui_basics, juce_gui_extra
 exporters:        xcode_mac, vs2022, linux_make

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

template <typename T>
static T* toBasePointer (SIMDRegister<T>* r) noexcept
{
    return reinterpret_cast<T*> (r);
}

constexpr auto registerSize = dsp::SIMDRegister<float>::size();

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

    template <typename SampleType>
    auto prepareChannelPointers (const AudioBlock<SampleType>& block)
    {
        std::array<SampleType*, registerSize> result {};

        for (size_t ch = 0; ch < result.size(); ++ch)
            result[ch] = (ch < block.getNumChannels() ? block.getChannelPointer (ch) : zero.getChannelPointer (ch));

        return result;
    }

    void process (const ProcessContextReplacing<float>& context)
    {
        jassert (context.getInputBlock().getNumSamples()  == context.getOutputBlock().getNumSamples());
        jassert (context.getInputBlock().getNumChannels() == context.getOutputBlock().getNumChannels());

        const auto& input  = context.getInputBlock();
        const auto numSamples = (int) input.getNumSamples();

        auto inChannels = prepareChannelPointers (input);

        using Format = AudioData::Format<AudioData::Float32, AudioData::NativeEndian>;

        AudioData::interleaveSamples (AudioData::NonInterleavedSource<Format> { inChannels.data(),                                 registerSize, },
                                      AudioData::InterleavedDest<Format>      { toBasePointer (interleaved.getChannelPointer (0)), registerSize },
                                      numSamples);

        iir->process (ProcessContextReplacing<SIMDRegister<float>> (interleaved));

        auto outChannels = prepareChannelPointers (context.getOutputBlock());

        AudioData::deinterleaveSamples (AudioData::InterleavedSource<Format>  { toBasePointer (interleaved.getChannelPointer (0)), registerSize },
                                        AudioData::NonInterleavedDest<Format> { outChannels.data(),                                registerSize },
                                        numSamples);
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
                case 1:   *iirCoefficients = IIR::ArrayCoefficients<float>::makeLowPass  (sampleRate, cutoff, qVal); break;
                case 2:   *iirCoefficients = IIR::ArrayCoefficients<float>::makeHighPass (sampleRate, cutoff, qVal); break;
                case 3:   *iirCoefficients = IIR::ArrayCoefficients<float>::makeBandPass (sampleRate, cutoff, qVal); break;
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

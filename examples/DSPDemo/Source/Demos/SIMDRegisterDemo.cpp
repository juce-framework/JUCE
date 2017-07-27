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

#if JUCE_USE_SIMD

//==============================================================================
// @@ START_DEMO
struct SIMDRegisterDemo
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
        if (sampleRate != 0)
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

    ChoiceParameter typeParam { { "Low-pass", "High-pass", "Band-pass"}, 1, "Type" };
    SliderParameter cutoffParam { { 20.0, 20000.0 }, 0.5, 440.0f, "Cutoff", "Hz" };
    SliderParameter qParam { { 0.3, 20.0 }, 0.5, 0.7, "Q" };

    std::vector<DSPDemoParameterBase*> parameters { &typeParam, &cutoffParam, &qParam };
    double sampleRate = 0;
};
// @@ END_DEMO

RegisterDSPDemo<SIMDRegisterDemo> simdDemo ("SIMD Filter", BinaryData::SIMDRegisterDemo_cpp);

#endif

/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{
namespace dsp
{

//==============================================================================
template <typename SampleType>
DryWetMixer<SampleType>::DryWetMixer()
    : DryWetMixer (0)
{
}

template <typename SampleType>
DryWetMixer<SampleType>::DryWetMixer (int maximumWetLatencyInSamples)
    : dryDelayLine (maximumWetLatencyInSamples)
{
    dryDelayLine.setDelay (0);

    update();
    reset();
}

//==============================================================================
template <typename SampleType>
void DryWetMixer<SampleType>::setMixingRule (MixingRule newRule)
{
    currentMixingRule = newRule;
    update();
}

template <typename SampleType>
void DryWetMixer<SampleType>::setWetMixProportion (SampleType newWetMixProportion)
{
    jassert (isPositiveAndNotGreaterThan (newWetMixProportion, 1.0));

    mix = jlimit (static_cast<SampleType> (0.0), static_cast<SampleType> (1.0), newWetMixProportion);
    update();
}

template <typename SampleType>
void DryWetMixer<SampleType>::setWetLatency (SampleType wetLatencySamples)
{
    dryDelayLine.setDelay (wetLatencySamples);
}

//==============================================================================
template <typename SampleType>
void DryWetMixer<SampleType>::prepare (const ProcessSpec& spec)
{
    jassert (spec.sampleRate > 0);
    jassert (spec.numChannels > 0);

    sampleRate = spec.sampleRate;

    dryDelayLine.prepare (spec);
    bufferDry.setSize ((int) spec.numChannels, (int) spec.maximumBlockSize, false, false, true);

    update();
    reset();
}

template <typename SampleType>
void DryWetMixer<SampleType>::reset()
{
    dryVolume.reset (sampleRate, 0.05);
    wetVolume.reset (sampleRate, 0.05);

    dryDelayLine.reset();
}

//==============================================================================
template <typename SampleType>
void DryWetMixer<SampleType>::pushDrySamples (const AudioBlock<const SampleType> drySamples)
{
    jassert (drySamples.getNumChannels() <= (size_t) bufferDry.getNumChannels());

    auto dryBlock = AudioBlock<SampleType> (bufferDry);
    dryBlock = dryBlock.getSubsetChannelBlock (0, drySamples.getNumChannels()).getSubBlock (0, drySamples.getNumSamples());

    auto context = ProcessContextNonReplacing<SampleType>(drySamples, dryBlock);
    dryDelayLine.process (context);
}

template <typename SampleType>
void DryWetMixer<SampleType>::mixWetSamples (AudioBlock<SampleType> inOutBlock)
{
    auto dryBlock = AudioBlock<SampleType> (bufferDry);
    dryBlock = dryBlock.getSubsetChannelBlock (0, inOutBlock.getNumChannels()).getSubBlock (0, inOutBlock.getNumSamples());

    dryBlock.multiplyBy (dryVolume);
    inOutBlock.multiplyBy (wetVolume);

    inOutBlock.add (dryBlock);
}

//==============================================================================
template <typename SampleType>
void DryWetMixer<SampleType>::update()
{
    SampleType dryValue, wetValue;

    switch (currentMixingRule)
    {
        case MixingRule::balanced:
            dryValue = static_cast<SampleType> (2.0) * jmin (static_cast<SampleType> (0.5), static_cast<SampleType> (1.0) - mix);
            wetValue = static_cast<SampleType> (2.0) * jmin (static_cast<SampleType> (0.5), mix);
            break;

        case MixingRule::linear:
            dryValue = static_cast<SampleType> (1.0) - mix;
            wetValue = mix;
            break;

        case MixingRule::sin3dB:
            dryValue = static_cast<SampleType> (std::sin (0.5 * MathConstants<double>::pi * (1.0 - mix)));
            wetValue = static_cast<SampleType> (std::sin (0.5 * MathConstants<double>::pi * mix));
            break;

        case MixingRule::sin4p5dB:
            dryValue = static_cast<SampleType> (std::pow (std::sin (0.5 * MathConstants<double>::pi * (1.0 - mix)), 1.5));
            wetValue = static_cast<SampleType> (std::pow (std::sin (0.5 * MathConstants<double>::pi * mix), 1.5));
            break;

        case MixingRule::sin6dB:
            dryValue = static_cast<SampleType> (std::pow (std::sin (0.5 * MathConstants<double>::pi * (1.0 - mix)), 2.0));
            wetValue = static_cast<SampleType> (std::pow (std::sin (0.5 * MathConstants<double>::pi * mix), 2.0));
            break;

        case MixingRule::squareRoot3dB:
            dryValue = std::sqrt (static_cast<SampleType> (1.0) - mix);
            wetValue = std::sqrt (mix);
            break;

        case MixingRule::squareRoot4p5dB:
            dryValue = static_cast<SampleType> (std::pow (std::sqrt (1.0 - mix), 1.5));
            wetValue = static_cast<SampleType> (std::pow (std::sqrt (mix), 1.5));
            break;

        default:
            dryValue = jmin (static_cast<SampleType> (0.5), static_cast<SampleType> (1.0) - mix);
            wetValue = jmin (static_cast<SampleType> (0.5), mix);
            break;
    }

    dryVolume.setTargetValue (dryValue);
    wetVolume.setTargetValue (wetValue);
}

//==============================================================================
template class DryWetMixer<float>;
template class DryWetMixer<double>;

} // namespace dsp
} // namespace juce

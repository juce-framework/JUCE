/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce::dsp
{

//==============================================================================
template <typename SampleType>
Compressor<SampleType>::Compressor()
{
    update();
}

//==============================================================================
template <typename SampleType>
void Compressor<SampleType>::setThreshold (SampleType newThreshold)
{
    thresholddB = newThreshold;
    update();
}

template <typename SampleType>
void Compressor<SampleType>::setRatio (SampleType newRatio)
{
    jassert (newRatio >= static_cast<SampleType> (1.0));

    ratio = newRatio;
    update();
}

template <typename SampleType>
void Compressor<SampleType>::setAttack (SampleType newAttack)
{
    attackTime = newAttack;
    update();
}

template <typename SampleType>
void Compressor<SampleType>::setRelease (SampleType newRelease)
{
    releaseTime = newRelease;
    update();
}

//==============================================================================
template <typename SampleType>
void Compressor<SampleType>::prepare (const ProcessSpec& spec)
{
    jassert (spec.sampleRate > 0);
    jassert (spec.numChannels > 0);

    sampleRate = spec.sampleRate;

    envelopeFilter.prepare (spec);

    update();
    reset();
}

template <typename SampleType>
void Compressor<SampleType>::reset()
{
    envelopeFilter.reset();
}

//==============================================================================
template <typename SampleType>
SampleType Compressor<SampleType>::processSample (int channel, SampleType inputValue)
{
    // Ballistics filter with peak rectifier
    auto env = envelopeFilter.processSample (channel, inputValue);

    // VCA
    auto gain = (env < threshold) ? static_cast<SampleType> (1.0)
                                  : std::pow (env * thresholdInverse, ratioInverse - static_cast<SampleType> (1.0));

    // Output
    return gain * inputValue;
}

template <typename SampleType>
void Compressor<SampleType>::update()
{
    threshold = Decibels::decibelsToGain (thresholddB, static_cast<SampleType> (-200.0));
    thresholdInverse = static_cast<SampleType> (1.0) / threshold;
    ratioInverse     = static_cast<SampleType> (1.0) / ratio;

    envelopeFilter.setAttackTime (attackTime);
    envelopeFilter.setReleaseTime (releaseTime);
}

//==============================================================================
template class Compressor<float>;
template class Compressor<double>;

} // namespace juce::dsp

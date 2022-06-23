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

namespace juce
{
namespace dsp
{

//==============================================================================
template <typename SampleType>
NoiseGate<SampleType>::NoiseGate()
{
    update();

    RMSFilter.setLevelCalculationType (BallisticsFilterLevelCalculationType::RMS);
    RMSFilter.setAttackTime  (static_cast<SampleType> (0.0));
    RMSFilter.setReleaseTime (static_cast<SampleType> (50.0));
}

template <typename SampleType>
void NoiseGate<SampleType>::setThreshold (SampleType newValue)
{
    thresholddB = newValue;
    update();
}

template <typename SampleType>
void NoiseGate<SampleType>::setRatio (SampleType newRatio)
{
    jassert (newRatio >= static_cast<SampleType> (1.0));

    ratio = newRatio;
    update();
}

template <typename SampleType>
void NoiseGate<SampleType>::setAttack (SampleType newAttack)
{
    attackTime = newAttack;
    update();
}

template <typename SampleType>
void NoiseGate<SampleType>::setRelease (SampleType newRelease)
{
    releaseTime = newRelease;
    update();
}

//==============================================================================
template <typename SampleType>
void NoiseGate<SampleType>::prepare (const ProcessSpec& spec)
{
    jassert (spec.sampleRate > 0);
    jassert (spec.numChannels > 0);

    sampleRate = spec.sampleRate;

    RMSFilter.prepare (spec);
    envelopeFilter.prepare (spec);

    update();
    reset();
}

template <typename SampleType>
void NoiseGate<SampleType>::reset()
{
    RMSFilter.reset();
    envelopeFilter.reset();
}

//==============================================================================
template <typename SampleType>
SampleType NoiseGate<SampleType>::processSample (int channel, SampleType sample)
{
    // RMS ballistics filter
    auto env = RMSFilter.processSample (channel, sample);

    // Ballistics filter
    env = envelopeFilter.processSample (channel, env);

    // VCA
    auto gain = (env > threshold) ? static_cast<SampleType> (1.0)
                                  : std::pow (env * thresholdInverse, currentRatio - static_cast<SampleType> (1.0));

    // Output
    return gain * sample;
}

template <typename SampleType>
void NoiseGate<SampleType>::update()
{
    threshold = Decibels::decibelsToGain (thresholddB, static_cast<SampleType> (-200.0));
    thresholdInverse = static_cast<SampleType> (1.0) / threshold;
    currentRatio = ratio;

    envelopeFilter.setAttackTime  (attackTime);
    envelopeFilter.setReleaseTime (releaseTime);
}

//==============================================================================
template class NoiseGate<float>;
template class NoiseGate<double>;

} // namespace dsp
} // namespace juce

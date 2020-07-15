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
Chorus<SampleType>::Chorus()
{
    auto oscFunction = [] (SampleType x) { return std::sin (x); };
    osc.initialise (oscFunction);

    dryWet.setMixingRule (DryWetMixingRule::linear);
}

template <typename SampleType>
void Chorus<SampleType>::setRate (SampleType newRateHz)
{
    jassert (isPositiveAndBelow (newRateHz, static_cast<SampleType> (100.0)));

    rate = newRateHz;
    update();
}

template <typename SampleType>
void Chorus<SampleType>::setDepth (SampleType newDepth)
{
    jassert (isPositiveAndNotGreaterThan (newDepth, static_cast<SampleType> (1.0)));

    depth = newDepth;
    update();
}

template <typename SampleType>
void Chorus<SampleType>::setCentreDelay (SampleType newDelayMs)
{
    jassert (isPositiveAndBelow (newDelayMs, static_cast<SampleType> (100.0)));

    centreDelay = jlimit (static_cast<SampleType> (1.0), static_cast<SampleType> (100.0), newDelayMs);
}

template <typename SampleType>
void Chorus<SampleType>::setFeedback (SampleType newFeedback)
{
    jassert (newFeedback >= static_cast<SampleType> (-1.0) && newFeedback <= static_cast<SampleType> (1.0));

    feedback = newFeedback;
    update();
}

template <typename SampleType>
void Chorus<SampleType>::setMix (SampleType newMix)
{
    jassert (isPositiveAndNotGreaterThan (newMix, static_cast<SampleType> (1.0)));

    mix = newMix;
    update();
}

//==============================================================================
template <typename SampleType>
void Chorus<SampleType>::prepare (const ProcessSpec& spec)
{
    jassert (spec.sampleRate > 0);
    jassert (spec.numChannels > 0);

    sampleRate = spec.sampleRate;

    delay.prepare (spec);

    dryWet.prepare (spec);
    feedbackVolume.resize (spec.numChannels);
    lastOutput.resize (spec.numChannels);

    osc.prepare (spec);
    bufferDelayTimes.setSize (1, (int) spec.maximumBlockSize, false, false, true);

    update();
    reset();
}

template <typename SampleType>
void Chorus<SampleType>::reset()
{
    std::fill (lastOutput.begin(), lastOutput.end(), static_cast<SampleType> (0));

    delay.reset();
    osc.reset();
    dryWet.reset();

    oscVolume.reset (sampleRate, 0.05);

    for (auto& vol : feedbackVolume)
        vol.reset (sampleRate, 0.05);
}

template <typename SampleType>
void Chorus<SampleType>::update()
{
    osc.setFrequency (rate);
    oscVolume.setTargetValue (depth * (SampleType) 0.5);
    dryWet.setWetMixProportion (mix);

    for (auto& vol : feedbackVolume)
        vol.setTargetValue (feedback);
}

//==============================================================================
template class Chorus<float>;
template class Chorus<double>;

} // namespace dsp
} // namespace juce

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
FirstOrderTPTFilter<SampleType>::FirstOrderTPTFilter()
{
    update();
}

//==============================================================================
template <typename SampleType>
void FirstOrderTPTFilter<SampleType>::setType (Type newValue)
{
    filterType = newValue;
}

template <typename SampleType>
void FirstOrderTPTFilter<SampleType>::setCutoffFrequency (SampleType newValue)
{
    jassert (isPositiveAndBelow (newValue, static_cast<SampleType> (sampleRate * 0.5)));

    cutoffFrequency = newValue;
    update();
}

//==============================================================================
template <typename SampleType>
void FirstOrderTPTFilter<SampleType>::prepare (const ProcessSpec& spec)
{
    jassert (spec.sampleRate > 0);
    jassert (spec.numChannels > 0);

    sampleRate = spec.sampleRate;
    s1.resize (spec.numChannels);

    reset();
}

template <typename SampleType>
void FirstOrderTPTFilter<SampleType>::reset()
{
    reset (static_cast<SampleType> (0));
}

template <typename SampleType>
void FirstOrderTPTFilter<SampleType>::reset (SampleType newValue)
{
    std::fill (s1.begin(), s1.end(), newValue);
}

//==============================================================================
template <typename SampleType>
SampleType FirstOrderTPTFilter<SampleType>::processSample (int channel, SampleType inputValue)
{
    auto& s = s1[(size_t) channel];

    auto v = G * (inputValue - s);
    auto y = v + s;
    s = y + v;

    switch (filterType)
    {
        case Type::lowpass:   return y;
        case Type::highpass:  return inputValue - y;
        case Type::allpass:   return 2 * y - inputValue;
        default:              break;
    }

    jassertfalse;
    return y;
}

template <typename SampleType>
void FirstOrderTPTFilter<SampleType>::snapToZero() noexcept
{
    for (auto& s : s1)
        util::snapToZero (s);
}

//==============================================================================
template <typename SampleType>
void FirstOrderTPTFilter<SampleType>::update()
{
    auto g = SampleType (std::tan (juce::MathConstants<double>::pi * cutoffFrequency / sampleRate));
    G = g / (1 + g);
}

//==============================================================================
template class FirstOrderTPTFilter<float>;
template class FirstOrderTPTFilter<double>;

} // namespace dsp
} // namespace juce

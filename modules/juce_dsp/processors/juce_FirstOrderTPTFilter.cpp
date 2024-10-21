/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

namespace juce::dsp
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

    update();
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

} // namespace juce::dsp

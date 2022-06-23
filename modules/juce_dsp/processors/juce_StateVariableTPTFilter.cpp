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
StateVariableTPTFilter<SampleType>::StateVariableTPTFilter()
{
    update();
}

template <typename SampleType>
void StateVariableTPTFilter<SampleType>::setType (Type newValue)
{
    filterType = newValue;
}

template <typename SampleType>
void StateVariableTPTFilter<SampleType>::setCutoffFrequency (SampleType newCutoffFrequencyHz)
{
    jassert (isPositiveAndBelow (newCutoffFrequencyHz, static_cast<SampleType> (sampleRate * 0.5)));

    cutoffFrequency = newCutoffFrequencyHz;
    update();
}

template <typename SampleType>
void StateVariableTPTFilter<SampleType>::setResonance (SampleType newResonance)
{
    jassert (newResonance > static_cast<SampleType> (0));

    resonance = newResonance;
    update();
}

//==============================================================================
template <typename SampleType>
void StateVariableTPTFilter<SampleType>::prepare (const ProcessSpec& spec)
{
    jassert (spec.sampleRate > 0);
    jassert (spec.numChannels > 0);

    sampleRate = spec.sampleRate;

    s1.resize (spec.numChannels);
    s2.resize (spec.numChannels);

    reset();
    update();
}

template <typename SampleType>
void StateVariableTPTFilter<SampleType>::reset()
{
    reset (static_cast<SampleType> (0));
}

template <typename SampleType>
void StateVariableTPTFilter<SampleType>::reset (SampleType newValue)
{
    for (auto v : { &s1, &s2 })
        std::fill (v->begin(), v->end(), newValue);
}

template <typename SampleType>
void StateVariableTPTFilter<SampleType>::snapToZero() noexcept
{
    for (auto v : { &s1, &s2 })
        for (auto& element : *v)
            util::snapToZero (element);
}

//==============================================================================
template <typename SampleType>
SampleType StateVariableTPTFilter<SampleType>::processSample (int channel, SampleType inputValue)
{
    auto& ls1 = s1[(size_t) channel];
    auto& ls2 = s2[(size_t) channel];

    auto yHP = h * (inputValue - ls1 * (g + R2) - ls2);

    auto yBP = yHP * g + ls1;
    ls1      = yHP * g + yBP;

    auto yLP = yBP * g + ls2;
    ls2      = yBP * g + yLP;

    switch (filterType)
    {
        case Type::lowpass:   return yLP;
        case Type::bandpass:  return yBP;
        case Type::highpass:  return yHP;
        default:              return yLP;
    }
}

//==============================================================================
template <typename SampleType>
void StateVariableTPTFilter<SampleType>::update()
{
    g  = static_cast<SampleType> (std::tan (juce::MathConstants<double>::pi * cutoffFrequency / sampleRate));
    R2 = static_cast<SampleType> (1.0 / resonance);
    h  = static_cast<SampleType> (1.0 / (1.0 + R2 * g + g * g));
}

//==============================================================================
template class StateVariableTPTFilter<float>;
template class StateVariableTPTFilter<double>;

} // namespace dsp
} // namespace juce

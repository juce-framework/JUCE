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
LinkwitzRileyFilter<SampleType>::LinkwitzRileyFilter()
{
    update();
}

//==============================================================================
template <typename SampleType>
void LinkwitzRileyFilter<SampleType>::setType (Type newType)
{
    filterType = newType;
}

template <typename SampleType>
void LinkwitzRileyFilter<SampleType>::setCutoffFrequency (SampleType newCutoffFrequencyHz)
{
    jassert (isPositiveAndBelow (newCutoffFrequencyHz, static_cast<SampleType> (sampleRate * 0.5)));

    cutoffFrequency = newCutoffFrequencyHz;
    update();
}

//==============================================================================
template <typename SampleType>
void LinkwitzRileyFilter<SampleType>::prepare (const ProcessSpec& spec)
{
    jassert (spec.sampleRate > 0);
    jassert (spec.numChannels > 0);

    sampleRate = spec.sampleRate;
    update();

    s1.resize (spec.numChannels);
    s2.resize (spec.numChannels);
    s3.resize (spec.numChannels);
    s4.resize (spec.numChannels);

    reset();
}

template <typename SampleType>
void LinkwitzRileyFilter<SampleType>::reset()
{
    for (auto s : { &s1, &s2, &s3, &s4 })
        std::fill (s->begin(), s->end(), static_cast<SampleType> (0));
}

template <typename SampleType>
void LinkwitzRileyFilter<SampleType>::snapToZero() noexcept
{
    for (auto s : { &s1, &s2, &s3, &s4 })
        for (auto& element : *s)
            util::snapToZero (element);
}

//==============================================================================
template <typename SampleType>
SampleType LinkwitzRileyFilter<SampleType>::processSample (int channel, SampleType inputValue)
{
    auto yH = (inputValue - (R2 + g) * s1[(size_t) channel] - s2[(size_t) channel]) * h;

    auto yB = g * yH + s1[(size_t) channel];
    s1[(size_t) channel] = g * yH + yB;

    auto yL = g * yB + s2[(size_t) channel];
    s2[(size_t) channel] = g * yB + yL;

    if (filterType == Type::allpass)
        return yL - R2 * yB + yH;

    auto yH2 = ((filterType == Type::lowpass ? yL : yH) - (R2 + g) * s3[(size_t) channel] - s4[(size_t) channel]) * h;

    auto yB2 = g * yH2 + s3[(size_t) channel];
    s3[(size_t) channel] = g * yH2 + yB2;

    auto yL2 = g * yB2 + s4[(size_t) channel];
    s4[(size_t) channel] = g * yB2 + yL2;

    return filterType == Type::lowpass ? yL2 : yH2;
}

template <typename SampleType>
void LinkwitzRileyFilter<SampleType>::processSample (int channel, SampleType inputValue, SampleType &outputLow, SampleType &outputHigh)
{
    auto yH = (inputValue - (R2 + g) * s1[(size_t) channel] - s2[(size_t) channel]) * h;

    auto yB = g * yH + s1[(size_t) channel];
    s1[(size_t) channel] = g * yH + yB;

    auto yL = g * yB + s2[(size_t) channel];
    s2[(size_t) channel] = g * yB + yL;

    auto yH2 = (yL - (R2 + g) * s3[(size_t) channel] - s4[(size_t) channel]) * h;

    auto yB2 = g * yH2 + s3[(size_t) channel];
    s3[(size_t) channel] = g * yH2 + yB2;

    auto yL2 = g * yB2 + s4[(size_t) channel];
    s4[(size_t) channel] = g * yB2 + yL2;

    outputLow = yL2;
    outputHigh = yL - R2 * yB + yH - yL2;
}

template <typename SampleType>
void LinkwitzRileyFilter<SampleType>::update()
{
    g  = (SampleType) std::tan (MathConstants<double>::pi * cutoffFrequency / sampleRate);
    R2 = (SampleType) std::sqrt (2.0);
    h  = (SampleType) (1.0 / (1.0 + R2 * g + g * g));
}

//==============================================================================
template class LinkwitzRileyFilter<float>;
template class LinkwitzRileyFilter<double>;

} // namespace juce::dsp

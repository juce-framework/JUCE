/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{
namespace SampleRateHelpers
{

static inline const std::vector<double>& getAllSampleRates()
{
    static auto sampleRates = []
    {
        std::vector<double> result;
        constexpr double baseRates[] = { 8000.0, 11025.0, 12000.0 };
        constexpr double maxRate = 768000.0;

        for (auto rate : baseRates)
            for (; rate <= maxRate; rate *= 2)
                result.insert (std::upper_bound (result.begin(), result.end(), rate),
                               rate);

        return result;
    }();

    return sampleRates;
}

} // namespace SampleRateHelpers
} // namespace juce

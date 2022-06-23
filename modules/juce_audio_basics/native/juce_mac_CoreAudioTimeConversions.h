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


// This file will be included directly by macOS/iOS-specific .cpps
#pragma once

#if ! DOXYGEN

#include <mach/mach_time.h>

namespace juce
{

struct CoreAudioTimeConversions
{
public:
    CoreAudioTimeConversions()
    {
        mach_timebase_info_data_t info{};
        mach_timebase_info (&info);
        numerator   = info.numer;
        denominator = info.denom;
    }

    uint64_t hostTimeToNanos (uint64_t hostTime) const
    {
        return multiplyByRatio (hostTime, numerator, denominator);
    }

    uint64_t nanosToHostTime (uint64_t nanos) const
    {
        return multiplyByRatio (nanos, denominator, numerator);
    }

private:
    // Adapted from CAHostTimeBase.h in the Core Audio Utility Classes
    static uint64_t multiplyByRatio (uint64_t toMultiply, uint64_t numerator, uint64_t denominator)
    {
       #if defined (__SIZEOF_INT128__)
        unsigned __int128
       #else
        long double
       #endif
            result = toMultiply;

        if (numerator != denominator)
        {
            result *= numerator;
            result /= denominator;
        }

        return (uint64_t) result;
    }

    uint64_t numerator = 0, denominator = 0;
};

} // namespace juce

#endif

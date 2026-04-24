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


// This file will be included directly by macOS/iOS-specific .cpps
#pragma once

/** @cond */

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

/** @endcond */

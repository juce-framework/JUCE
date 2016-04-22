/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

#ifndef JUCE_LINEARSMOOTHEDVALUE_H_INCLUDED
#define JUCE_LINEARSMOOTHEDVALUE_H_INCLUDED


//==============================================================================
/**
    Utility class for linearly smoothed values like volume etc. that should
    not change abruptly but as a linear ramp, to avoid audio glitches.
*/

//==============================================================================
template <typename FloatType>
class LinearSmoothedValue
{
public:
    /** Constructor. */
    LinearSmoothedValue() noexcept
        : currentValue (0), target (0), step (0), countdown (0), stepsToTarget (0)
    {
    }

    /** Constructor. */
    LinearSmoothedValue (FloatType initialValue) noexcept
        : currentValue (initialValue), target (initialValue), step (0), countdown (0), stepsToTarget (0)
    {
    }

    //==============================================================================
    /** Reset to a new sample rate and ramp length. */
    void reset (double sampleRate, double rampLengthInSeconds) noexcept
    {
        jassert (sampleRate > 0 && rampLengthInSeconds >= 0);
        stepsToTarget = (int) std::floor (rampLengthInSeconds * sampleRate);
        currentValue = target;
        countdown = 0;
    }

    /** Set a new target value. */
    void setValue (FloatType newValue) noexcept
    {
        if (target != newValue)
        {
            target = newValue;
            countdown = stepsToTarget;

            if (countdown <= 0)
                currentValue = target;
            else
                step = (target - currentValue) / (FloatType) countdown;
        }
    }

    /** Compute the next value. */
    FloatType getNextValue() noexcept
    {
        if (countdown <= 0)
            return target;

        --countdown;
        currentValue += step;
        return currentValue;
    }

    /** Returns true if the current value is currently being interpolated. */
    bool isSmoothing() const noexcept
    {
        return countdown > 0;
    }

    /** Returns the target value towards which the smoothed value is currently moving. */
    FloatType getTargetValue() const noexcept
    {
        return target;
    }

private:
    //==============================================================================
    FloatType currentValue, target, step;
    int countdown, stepsToTarget;
};


#endif   // JUCE_LINEARSMOOTHEDVALUE_H_INCLUDED

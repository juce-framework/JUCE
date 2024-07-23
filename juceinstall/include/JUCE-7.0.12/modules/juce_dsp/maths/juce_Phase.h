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

/**
    Represents an increasing phase value between 0 and 2*pi.

    This represents a value which can be incremented, and which wraps back to 0 when it
    goes past 2 * pi.

    @tags{DSP}
*/
template <typename Type>
struct Phase
{
    /** Resets the phase to 0. */
    void reset() noexcept               { phase = 0; }

    /** Returns the current value, and increments the phase by the given increment.
        The increment must be a positive value, it can't go backwards!
        The new value of the phase after calling this function will be (phase + increment) % (2 * pi).
    */
    Type advance (Type increment) noexcept
    {
        jassert (increment >= 0); // cannot run this value backwards!

        auto last = phase;
        auto next = last + increment;

        while (next >= MathConstants<Type>::twoPi)
            next -= MathConstants<Type>::twoPi;

        phase = next;
        return last;
    }

    Type phase = 0;
};

} // namespace juce::dsp

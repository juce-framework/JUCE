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

namespace juce
{

//==============================================================================
/**
    An object for storing and measuring durations for diagnostic purposes.

    This object is designed to have minimal performance impact so that it
    doesn't distort results and is safe to use even in real-time audio contexts.

    @tags{Core}
*/
class JUCE_API TimedDiagnostic
{
public:
    /** Returns the stored duration, converted to the requested time unit, as a
        numeric value.

        For example:
        - get<Milliseconds>() returns the stored duration expressed in milliseconds
        - get<Microseconds>() returns the stored duration expressed in microseconds
    */
    template <typename TimeUnit>
    auto get() const
    {
        return std::chrono::duration_cast<TimeUnit> (Seconds { value }).count();
    }

    /** Sets the stored duration from the given value.

        The provided duration may be expressed in any time unit, such as
        Seconds, Milliseconds, or Microseconds.
    */
    template <typename TimeUnit>
    void set (const TimeUnit& newValue)
    {
        value = std::chrono::duration_cast<Seconds> (newValue).count();
    }

    /** Creates a scoped timer that stores the elapsed time in this diagnostic.

        The returned object measures the time between its construction and
        destruction, and sets that elapsed duration to this TimedDiagnostic.
    */
    ScopedTimeMeasurement createTimer() &
    {
        return ScopedTimeMeasurement { value };
    }

    /** Returns true if this diagnostic does not currently store any measured
        time.
    */
    bool isEmpty() const
    {
        return exactlyEqual (value, 0.0);
    }

    /** Returns the sum of this diagnostic and another diagnostic. */
    TimedDiagnostic operator+ (TimedDiagnostic other) const
    {
        return TimedDiagnostic { *this } += other;
    }

    /** Returns the difference between this diagnostic and another diagnostic. */
    TimedDiagnostic operator- (TimedDiagnostic other) const
    {
        return TimedDiagnostic { *this } -= other;
    }

    /** Adds another diagnostic's stored duration to this one. */
    TimedDiagnostic& operator+= (TimedDiagnostic other)
    {
        value += other.value;
        return *this;
    }

    /** Subtracts another diagnostic's stored duration from this one. */
    TimedDiagnostic& operator-= (TimedDiagnostic other)
    {
        value -= other.value;
        return *this;
    }

    // Creating a timer from a temporary is a bad idea, since the reference to
    // 'value' is likely to dangle when the timer completes.
    ScopedTimeMeasurement createTimer() && = delete;

private:
    double value{};
};

} // namespace juce

/*
  ==============================================================================

   This file is part of the juce_core module of the JUCE library.
   Copyright (c) 2016 - ROLI Ltd.

   Permission to use, copy, modify, and/or distribute this software for any purpose with
   or without fee is hereby granted, provided that the above copyright notice and this
   permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD
   TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN
   NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
   DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
   IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
   CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

   ------------------------------------------------------------------------------

   NOTE! This permissive ISC license applies ONLY to files within the juce_core module!
   All other JUCE modules are covered by a dual GPL/commercial license, so if you are
   using any other modules, be sure to check that you also comply with their license.

   For more details, visit www.juce.com

  ==============================================================================
*/

#ifndef JUCE_STATISTICSACCUMULATOR_H_INCLUDED
#define JUCE_STATISTICSACCUMULATOR_H_INCLUDED


//==============================================================================
/**
    A class that measures various statistics about a series of floating point
    values that it is given.
*/
template <typename FloatType>
class StatisticsAccumulator
{
public:
    //==============================================================================
    /** Constructs a new StatisticsAccumulator. */
    StatisticsAccumulator() noexcept
        : count (0),
          minimum ( std::numeric_limits<FloatType>::infinity()),
          maximum (-std::numeric_limits<FloatType>::infinity())
    {}

    //==============================================================================
    /** Add a new value to the accumulator.
        This will update all running statistics accordingly.
    */
    void addValue (FloatType v) noexcept
    {
        jassert (juce_isfinite (v));

        sum += v;
        sumSquares += v * v;
        ++count;

        if (v > maximum) maximum = v;
        if (v < minimum) minimum = v;
    }

    /** Reset the accumulator.
        This will reset all currently saved statistcs.
    */
    void reset() noexcept               { *this = StatisticsAccumulator<FloatType>(); }

    //==============================================================================
    /** Returns the average (arithmetic mean) of all previously added values.
        If no values have been added yet, this will return zero.
    */
    FloatType getAverage() const noexcept
    {
        return count > 0 ? sum / (FloatType) count
                         : FloatType();
    }

    /** Returns the variance of all previously added values.
        If no values have been added yet, this will return zero.
    */
    FloatType getVariance() const noexcept
    {
        return count > 0 ? (sumSquares - sum * sum / (FloatType) count) / (FloatType) count
                         : FloatType();
    }

    /** Returns the standard deviation of all previously added values.
        If no values have been added yet, this will return zero.
    */
    FloatType getStandardDeviation() const noexcept
    {
        return std::sqrt (getVariance());
    }

    /** Returns the smallest of all previously added values.
        If no values have been added yet, this will return positive infinity.
    */
    FloatType getMinValue() const noexcept
    {
        return minimum;
    }

    /** Returns the largest of all previously added values.
        If no values have been added yet, this will return negative infinity.
    */
    FloatType getMaxValue() const noexcept
    {
        return maximum;
    }

    /** Returns how many values have been added to this accumulator. */
    size_t getCount() const noexcept
    {
        return count;
    }

private:
    //==============================================================================
    struct KahanSum
    {
        KahanSum() noexcept : sum(), error() {}
        operator FloatType() const noexcept             { return sum; }

        void JUCE_NO_ASSOCIATIVE_MATH_OPTIMISATIONS operator+= (FloatType value) noexcept
        {
            FloatType correctedValue = value - error;
            FloatType newSum = sum + correctedValue;
            error = (newSum - sum) - correctedValue;
            sum = newSum;
        }

        FloatType sum, error;
    };

    //==============================================================================
    size_t count;
    KahanSum sum, sumSquares;
    FloatType minimum, maximum;
};


#endif   // JUCE_STATISTICSACCUMULATOR_H_INCLUDED

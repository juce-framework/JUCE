/*
  ==============================================================================

   This file is part of the juce_core module of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

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

#ifndef JUCE_NORMALISABLERANGE_H_INCLUDED
#define JUCE_NORMALISABLERANGE_H_INCLUDED


//==============================================================================
/**
    Represents a mapping between an arbitrary range of values and a
    normalised 0->1 range.

    The properties of the mapping also include an optional snapping interval
    and skew-factor.

    @see Range
*/
template <typename ValueType>
class NormalisableRange
{
public:
    /** Creates a continuous range that performs a dummy mapping. */
    NormalisableRange() noexcept  : start(), end (1), interval(), skew (static_cast<ValueType> (1)), symmetricSkew (false) {}

    /** Creates a copy of another range. */
    NormalisableRange (const NormalisableRange& other) noexcept
        : start (other.start), end (other.end),
          interval (other.interval), skew (other.skew),
          symmetricSkew (other.symmetricSkew)
    {
        checkInvariants();
    }

    /** Creates a copy of another range. */
    NormalisableRange& operator= (const NormalisableRange& other) noexcept
    {
        start = other.start;
        end = other.end;
        interval = other.interval;
        skew = other.skew;
        symmetricSkew = other.symmetricSkew;
        checkInvariants();
        return *this;
    }

    /** Creates a NormalisableRange with a given range, interval and skew factor. */
    NormalisableRange (ValueType rangeStart,
                       ValueType rangeEnd,
                       ValueType intervalValue,
                       ValueType skewFactor,
                       bool useSymmetricSkew = false) noexcept
        : start (rangeStart), end (rangeEnd), interval (intervalValue),
          skew (skewFactor), symmetricSkew (useSymmetricSkew)
    {
        checkInvariants();
    }

    /** Creates a NormalisableRange with a given range and interval, but a dummy skew-factor. */
    NormalisableRange (ValueType rangeStart,
                       ValueType rangeEnd,
                       ValueType intervalValue) noexcept
        : start (rangeStart), end (rangeEnd), interval (intervalValue),
          skew (static_cast<ValueType> (1)), symmetricSkew (false)
    {
        checkInvariants();
    }

    /** Creates a NormalisableRange with a given range, continuous interval, but a dummy skew-factor. */
    NormalisableRange (ValueType rangeStart,
                       ValueType rangeEnd) noexcept
        : start (rangeStart), end (rangeEnd), interval(),
          skew (static_cast<ValueType> (1)), symmetricSkew (false)
    {
        checkInvariants();
    }

    /** Uses the properties of this mapping to convert a non-normalised value to
        its 0->1 representation.
    */
    ValueType convertTo0to1 (ValueType v) const noexcept
    {
        ValueType proportion = (v - start) / (end - start);

        if (skew == static_cast<ValueType> (1))
            return proportion;

        if (! symmetricSkew)
            return std::pow (proportion, skew);

        ValueType distanceFromMiddle = static_cast<ValueType> (2) * proportion - static_cast<ValueType> (1);

        return (static_cast<ValueType> (1) + std::pow (std::abs (distanceFromMiddle), skew)
                                           * (distanceFromMiddle < static_cast<ValueType> (0) ? static_cast<ValueType> (-1)
                                                                                              : static_cast<ValueType> (1)))
               / static_cast<ValueType> (2);
    }

    /** Uses the properties of this mapping to convert a normalised 0->1 value to
        its full-range representation.
    */
    ValueType convertFrom0to1 (ValueType proportion) const noexcept
    {
        if (! symmetricSkew)
        {
            if (skew != static_cast<ValueType> (1) && proportion > ValueType())
                proportion = std::exp (std::log (proportion) / skew);

                return start + (end - start) * proportion;
        }

        ValueType distanceFromMiddle = static_cast<ValueType> (2) * proportion - static_cast<ValueType> (1);

        if (skew != static_cast<ValueType> (1) && distanceFromMiddle != static_cast<ValueType> (0))
            distanceFromMiddle = std::exp (std::log (std::abs (distanceFromMiddle)) / skew)
                                 * (distanceFromMiddle < static_cast<ValueType> (0) ? static_cast<ValueType> (-1)
                                                                                    : static_cast<ValueType> (1));

        return start + (end - start) / static_cast<ValueType> (2) * (static_cast<ValueType> (1) + distanceFromMiddle);
    }

    /** Takes a non-normalised value and snaps it based on the interval property of
        this NormalisedRange. */
    ValueType snapToLegalValue (ValueType v) const noexcept
    {
        if (interval > ValueType())
            v = start + interval * std::floor ((v - start) / interval + static_cast<ValueType> (0.5));

        if (v <= start || end <= start)
            return start;

        if (v >= end)
            return end;

        return v;
    }

    Range<ValueType> getRange() const noexcept          { return Range<ValueType> (start, end); }

    /** The start of the non-normalised range. */
    ValueType start;

    /** The end of the non-normalised range. */
    ValueType end;

    /** The snapping interval that should be used (in non-normalised value). Use 0 for a continuous range. */
    ValueType interval;

    /** An optional skew factor that alters the way values are distribute across the range.

        The skew factor lets you skew the mapping logarithmically so that larger or smaller
        values are given a larger proportion of the available space.

        A factor of 1.0 has no skewing effect at all. If the factor is < 1.0, the lower end
        of the range will fill more of the slider's length; if the factor is > 1.0, the upper
        end of the range will be expanded.
    */
    ValueType skew;

    /** If true, the skew factor applies from the middle of the slider to each of its ends. */
    bool symmetricSkew;

private:
    void checkInvariants() const
    {
        jassert (end > start);
        jassert (interval >= ValueType());
        jassert (skew > ValueType());
    }
};


#endif   // JUCE_NORMALISABLERANGE_H_INCLUDED

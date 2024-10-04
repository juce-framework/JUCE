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

/**
    Helper functions for managing buffered readers.

    @tags{Audio}
*/
struct Reservoir
{
    /** Attempts to read the requested range from some kind of input stream,
        with intermediate buffering in a 'reservoir'.

        While there are still samples in the requested range left to read, this
        function will check whether the next part of the requested range is
        already loaded into the reservoir. If the range is available, then
        doBufferedRead will call readFromReservoir with the range that should
        be copied to the output buffer. If the range is not available,
        doBufferedRead will call fillReservoir to request that a new region is
        loaded into the reservoir. It will repeat these steps until either the
        entire requested region has been read, or the stream ends.

        This will return the range that could not be read successfully, if any.
        An empty range implies that the entire read was successful.

        Note that all ranges, including those provided as arguments to the
        callbacks, are relative to the original unbuffered input. That is, if
        getBufferedRange returns the range [200, 300), then readFromReservoir
        might be passed the range [250, 300) in order to copy the final 50
        samples from the reservoir.

        @param rangeToRead       the absolute position of the range that should
                                 be read
        @param getBufferedRange  a function void -> Range<Index> that returns
                                 the region currently held in the reservoir
        @param readFromReservoir a function Range<Index> -> void that can be
                                 used to copy samples from the region in the
                                 reservoir specified in the input range
        @param fillReservoir     a function Index -> void that is given a
                                 requested read location, and that should
                                 attempt to fill the reservoir starting at this
                                 location. After this function,
                                 getBufferedRange should return the new region
                                 contained in the managed buffer
    */
    template <typename Index, typename GetBufferedRange, typename ReadFromReservoir, typename FillReservoir>
    static Range<Index> doBufferedRead (Range<Index> rangeToRead,
                                        GetBufferedRange&& getBufferedRange,
                                        ReadFromReservoir&& readFromReservoir,
                                        FillReservoir&& fillReservoir)
    {
        while (! rangeToRead.isEmpty())
        {
            const auto bufferedRange = getBufferedRange();

            if (bufferedRange.contains (rangeToRead.getStart()))
            {
                const auto rangeToReadInBuffer = rangeToRead.getIntersectionWith (getBufferedRange());
                readFromReservoir (rangeToReadInBuffer);
                rangeToRead.setStart (rangeToReadInBuffer.getEnd());
            }
            else
            {
                fillReservoir (rangeToRead.getStart());

                const auto newRange = getBufferedRange();

                if (newRange.isEmpty() || ! newRange.contains (rangeToRead.getStart()))
                    break;
            }
        }

        return rangeToRead;
    }
};

} // namespace juce

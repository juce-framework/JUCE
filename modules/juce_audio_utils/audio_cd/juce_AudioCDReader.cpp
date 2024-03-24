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

#if JUCE_USE_CDREADER

int AudioCDReader::getNumTracks() const
{
    return trackStartSamples.size() - 1;
}

int AudioCDReader::getPositionOfTrackStart (int trackNum) const
{
    return trackStartSamples [trackNum];
}

const Array<int>& AudioCDReader::getTrackOffsets() const
{
    return trackStartSamples;
}

int AudioCDReader::getCDDBId()
{
    int checksum = 0;
    const int numTracks = getNumTracks();

    for (int i = 0; i < numTracks; ++i)
        for (int offset = (trackStartSamples.getUnchecked (i) + 88200) / 44100; offset > 0; offset /= 10)
            checksum += offset % 10;

    const int length = (trackStartSamples.getLast() - trackStartSamples.getFirst()) / 44100;

    // CCLLLLTT: checksum, length, tracks
    return ((checksum & 0xff) << 24) | (length << 8) | numTracks;
}

#endif

} // namespace juce

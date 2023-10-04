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

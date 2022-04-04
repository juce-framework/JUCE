/*
  ==============================================================================

   This file is part of the JUCE 7 technical preview.
   Copyright (c) 2022 - Raw Material Software Limited

   You may use this code under the terms of the GPL v3
   (see www.gnu.org/licenses).

   For the technical preview this file cannot be licensed commercially.

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
        for (int offset = (trackStartSamples.getUnchecked(i) + 88200) / 44100; offset > 0; offset /= 10)
            checksum += offset % 10;

    const int length = (trackStartSamples.getLast() - trackStartSamples.getFirst()) / 44100;

    // CCLLLLTT: checksum, length, tracks
    return ((checksum & 0xff) << 24) | (length << 8) | numTracks;
}

#endif

} // namespace juce

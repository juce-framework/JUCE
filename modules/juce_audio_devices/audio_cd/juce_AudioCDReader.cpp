/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

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

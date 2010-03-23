/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

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

// (This file gets included by juce_linux_NativeCode.cpp, rather than being
// compiled on its own).
#if JUCE_INCLUDED_FILE && JUCE_USE_CDREADER


//==============================================================================
AudioCDReader::AudioCDReader()
    : AudioFormatReader (0, "CD Audio")
{
}

const StringArray AudioCDReader::getAvailableCDNames()
{
    StringArray names;
    return names;
}

AudioCDReader* AudioCDReader::createReaderForCD (const int index)
{
    return 0;
}

AudioCDReader::~AudioCDReader()
{
}

void AudioCDReader::refreshTrackLengths()
{
}

bool AudioCDReader::readSamples (int** destSamples, int numDestChannels, int startOffsetInDestBuffer,
                                 int64 startSampleInFile, int numSamples)
{
    return false;
}

bool AudioCDReader::isCDStillPresent() const
{
    return false;
}

int AudioCDReader::getNumTracks() const
{
    return 0;
}

int AudioCDReader::getPositionOfTrackStart (int trackNum) const
{
    return 0;
}

bool AudioCDReader::isTrackAudio (int trackNum) const
{
    return false;
}

void AudioCDReader::enableIndexScanning (bool b)
{
}

int AudioCDReader::getLastIndex() const
{
    return 0;
}

const Array<int> AudioCDReader::findIndexesInTrack (const int trackNumber)
{
    return Array<int>();
}

int AudioCDReader::getCDDBId()
{
    return 0;
}

#endif

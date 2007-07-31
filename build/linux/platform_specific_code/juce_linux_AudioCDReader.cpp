/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

  ==============================================================================
*/

#include "../../../src/juce_core/basics/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "../../../src/juce_appframework/audio/audio_file_formats/juce_AudioCDReader.h"


//==============================================================================
AudioCDReader::AudioCDReader()
    : AudioFormatReader (0, T("CD Audio"))
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

bool AudioCDReader::read (int** destSamples,
                          int64 startSampleInFile,
                          int numSamples)
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

END_JUCE_NAMESPACE

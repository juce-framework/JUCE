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

#include "../../../juce_core/basics/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#if JUCE_MAC

//==============================================================================
// Mac version doesn't need any native code because it's all done with files..
// Windows + Linux versions are in the platform-dependent code sections.

#include "juce_AudioCDReader.h"
#include "juce_AiffAudioFormat.h"
#include "../../../juce_core/io/files/juce_FileInputStream.h"
#include "../../../juce_core/io/streams/juce_BufferedInputStream.h"


static void findCDs (OwnedArray<File>& cds)
{
    File volumes ("/Volumes");
    volumes.findChildFiles (cds, File::findDirectories, false);

    for (int i = cds.size(); --i >= 0;)
        if (! cds[i]->getChildFile (".TOC.plist").exists())
            cds.remove (i);
}

const StringArray AudioCDReader::getAvailableCDNames()
{
    OwnedArray<File> cds;
    findCDs (cds);

    StringArray names;

    for (int i = 0; i < cds.size(); ++i)
        names.add (cds[i]->getFileName());

    return names;
}

AudioCDReader* AudioCDReader::createReaderForCD (const int index)
{
    OwnedArray<File> cds;
    findCDs (cds);

    if (cds[index] != 0)
        return new AudioCDReader (*cds[index]);
    else
        return 0;
}

AudioCDReader::AudioCDReader (const File& volume)
   : AudioFormatReader (0, "CD Audio"),
     volumeDir (volume),
     currentReaderTrack (-1),
     reader (0)
{
     sampleRate = 44100.0;
     bitsPerSample = 16;
     numChannels = 2;
     usesFloatingPointData = false;

     refreshTrackLengths();
}

AudioCDReader::~AudioCDReader()
{
    if (reader != 0)
        delete reader;
}

static int getTrackNumber (const File& file)
{
    return file.getFileName()
               .initialSectionContainingOnly (T("0123456789"))
               .getIntValue();
}

int AudioCDReader::compareElements (const File* const first, const File* const second) throw()
{
    const int firstTrack  = getTrackNumber (*first);
    const int secondTrack = getTrackNumber (*second);

    jassert (firstTrack > 0 && secondTrack > 0);

    return firstTrack - secondTrack;
}

void AudioCDReader::refreshTrackLengths()
{
    tracks.clear();
    trackStartSamples.clear();
    volumeDir.findChildFiles (tracks, File::findFiles | File::ignoreHiddenFiles, false, T("*.aiff"));

    tracks.sort (*this);

    AiffAudioFormat format;
    int sample = 0;

    for (int i = 0; i < tracks.size(); ++i)
    {
        trackStartSamples.add (sample);

        FileInputStream* const in = tracks[i]->createInputStream();

        if (in != 0)
        {
            AudioFormatReader* const r = format.createReaderFor (in, true);

            if (r != 0)
            {
                sample += r->lengthInSamples;
                delete r;
            }
        }
    }

    trackStartSamples.add (sample);
    lengthInSamples = sample;
}

bool AudioCDReader::read (int** destSamples,
                          int64 startSampleInFile,
                          int numSamples)
{
    while (numSamples > 0)
    {
        int track = -1;

        for (int i = 0; i < trackStartSamples.size() - 1; ++i)
        {
            if (startSampleInFile < trackStartSamples.getUnchecked (i + 1))
            {
                track = i;
                break;
            }
        }

        if (track < 0)
            return false;

        if (track != currentReaderTrack)
        {
            deleteAndZero (reader);

            if (tracks [track] != 0)
            {
                FileInputStream* const in = tracks [track]->createInputStream();

                if (in != 0)
                {
                    BufferedInputStream* const bin = new BufferedInputStream (in, 65536, true);

                    AiffAudioFormat format;
                    reader = format.createReaderFor (bin, true);

                    if (reader == 0)
                        currentReaderTrack = -1;
                    else
                        currentReaderTrack = track;
                }
            }
        }

        if (reader == 0)
            return false;

        const int startPos = (int) (startSampleInFile - trackStartSamples.getUnchecked (track));
        const int numAvailable = (int) jmin ((int64) numSamples, reader->lengthInSamples - startPos);

        reader->read (destSamples, startPos, numAvailable);

        numSamples -= numAvailable;
        startSampleInFile += numAvailable;
    }

    return true;
}

bool AudioCDReader::isCDStillPresent() const
{
    return volumeDir.exists();
}

int AudioCDReader::getNumTracks() const
{
    return tracks.size();
}

int AudioCDReader::getPositionOfTrackStart (int trackNum) const
{
    return trackStartSamples [trackNum];
}

bool AudioCDReader::isTrackAudio (int trackNum) const
{
    return tracks [trackNum] != 0;
}

void AudioCDReader::enableIndexScanning (bool b)
{
    // any way to do this on a Mac??
}

int AudioCDReader::getLastIndex() const
{
    return 0;
}

const Array <int> AudioCDReader::findIndexesInTrack (const int trackNumber)
{
    return Array <int>();
}

int AudioCDReader::getCDDBId()
{
    return 0; //xxx
}

#endif

END_JUCE_NAMESPACE

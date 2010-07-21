/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-10 by Raw Material Software Ltd.

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

// (This file gets included by juce_mac_NativeCode.mm, rather than being
// compiled on its own).
#if JUCE_INCLUDED_FILE && JUCE_USE_CDREADER

//==============================================================================
static void juce_findCDs (Array<File>& cds)
{
    File volumes ("/Volumes");
    volumes.findChildFiles (cds, File::findDirectories, false);

    for (int i = cds.size(); --i >= 0;)
        if (! cds.getReference(i).getChildFile (".TOC.plist").exists())
            cds.remove (i);
}

const StringArray AudioCDReader::getAvailableCDNames()
{
    Array<File> cds;
    juce_findCDs (cds);

    StringArray names;

    for (int i = 0; i < cds.size(); ++i)
        names.add (cds.getReference(i).getFileName());

    return names;
}

AudioCDReader* AudioCDReader::createReaderForCD (const int index)
{
    Array<File> cds;
    juce_findCDs (cds);

    if (cds[index].exists())
        return new AudioCDReader (cds[index]);

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
}

void AudioCDReader::refreshTrackLengths()
{
    tracks.clear();
    trackStartSamples.clear();
    volumeDir.findChildFiles (tracks, File::findFiles | File::ignoreHiddenFiles, false, "*.aiff");

    struct CDTrackSorter
    {
        static int getCDTrackNumber (const File& file)
        {
            return file.getFileName()
                       .initialSectionContainingOnly ("0123456789")
                       .getIntValue();
        }

        static int compareElements (const File& first, const File& second)
        {
            const int firstTrack  = getCDTrackNumber (first);
            const int secondTrack = getCDTrackNumber (second);

            jassert (firstTrack > 0 && secondTrack > 0);

            return firstTrack - secondTrack;
        }
    };

    CDTrackSorter sorter;
    tracks.sort (sorter);

    AiffAudioFormat format;
    int sample = 0;

    for (int i = 0; i < tracks.size(); ++i)
    {
        trackStartSamples.add (sample);

        FileInputStream* const in = tracks.getReference(i).createInputStream();

        if (in != 0)
        {
            ScopedPointer <AudioFormatReader> r (format.createReaderFor (in, true));

            if (r != 0)
                sample += (int) r->lengthInSamples;
        }
    }

    trackStartSamples.add (sample);
    lengthInSamples = sample;
}

bool AudioCDReader::readSamples (int** destSamples, int numDestChannels, int startOffsetInDestBuffer,
                                 int64 startSampleInFile, int numSamples)
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
            reader = 0;

            FileInputStream* const in = tracks [track].createInputStream();

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

        if (reader == 0)
            return false;

        const int startPos = (int) (startSampleInFile - trackStartSamples.getUnchecked (track));
        const int numAvailable = (int) jmin ((int64) numSamples, reader->lengthInSamples - startPos);

        reader->readSamples (destSamples, numDestChannels, startOffsetInDestBuffer, startPos, numAvailable);

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
    return tracks [trackNum] != File::nonexistent;
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

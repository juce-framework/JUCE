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

#ifndef __JUCE_AUDIOCDREADER_JUCEHEADER__
#define __JUCE_AUDIOCDREADER_JUCEHEADER__

#include "juce_AudioFormatReader.h"
#include "../../../juce_core/text/juce_StringArray.h"
#if JUCE_MAC
#include "../../../juce_core/io/files/juce_File.h"
#endif

//==============================================================================
/**
    A type of AudioFormatReader that reads from an audio CD.

    One of these can be used to read a CD as if it's one big audio stream. Use the
    getPositionOfTrackStart() method to find where the individual tracks are
    within the stream.

    @see AudioFormatReader
*/
class JUCE_API  AudioCDReader  : public AudioFormatReader
{
public:
    //==============================================================================
    /** Returns a list of names of Audio CDs currently available for reading.

        If there's a CD drive but no CD in it, this might return an empty list, or
        possibly a device that can be opened but which has no tracks, depending
        on the platform.

        @see createReaderForCD
    */
    static const StringArray getAvailableCDNames();

    /** Tries to create an AudioFormatReader that can read from an Audio CD.

        @param index    the index of one of the available CDs - use getAvailableCDNames()
                        to find out how many there are.
        @returns        a new AudioCDReader object, or 0 if it couldn't be created. The
                        caller will be responsible for deleting the object returned.
    */
    static AudioCDReader* createReaderForCD (const int index);

    //==============================================================================
    /** Destructor. */
    ~AudioCDReader();

    /** Implementation of the AudioFormatReader method. */
    bool read (int** destSamples,
               int64 startSampleInFile,
               int numSamples);

    /** Checks whether the CD has been removed from the drive.
    */
    bool isCDStillPresent() const;

    /** Returns the total number of tracks (audio + data).
    */
    int getNumTracks() const;

    /** Finds the sample offset of the start of a track.

        @param trackNum     the track number, where 0 is the first track.
    */
    int getPositionOfTrackStart (int trackNum) const;

    /** Returns true if a given track is an audio track.

        @param trackNum     the track number, where 0 is the first track.
    */
    bool isTrackAudio (int trackNum) const;

    /** Refreshes the object's table of contents.

        If the disc has been ejected and a different one put in since this
        object was created, this will cause it to update its idea of how many tracks
        there are, etc.
    */
    void refreshTrackLengths();

    /** Enables scanning for indexes within tracks.

        @see getLastIndex
    */
    void enableIndexScanning (bool enabled);

    /** Returns the index number found during the last read() call.

        Index scanning is turned off by default - turn it on with enableIndexScanning().

        Then when the read() method is called, if it comes across an index within that
        block, the index number is stored and returned by this method.

        Some devices might not support indexes, of course.

        (If you don't know what CD indexes are, it's unlikely you'll ever need them).

        @see enableIndexScanning
    */
    int getLastIndex() const;

    /** Scans a track to find the position of any indexes within it.

        @param trackNumber  the track to look in, where 0 is the first track on the disc
        @returns    an array of sample positions of any index points found (not including
                    the index that marks the start of the track)
    */
    const Array <int> findIndexesInTrack (const int trackNumber);

    /** Returns the CDDB id number for the CD.

        It's not a great way of identifying a disc, but it's traditional.
    */
    int getCDDBId();

    /** Tries to eject the disk.

        Of course this might not be possible, if some other process is using it.
    */
    void ejectDisk();

    //==============================================================================
    juce_UseDebuggingNewOperator

private:

#if JUCE_MAC
    File volumeDir;
    OwnedArray<File> tracks;
    Array <int> trackStartSamples;
    int currentReaderTrack;
    AudioFormatReader* reader;
    AudioCDReader (const File& volume);
public:
    static int compareElements (const File* const, const File* const) throw();
private:

#elif JUCE_WIN32
    int numTracks;
    int trackStarts[100];
    bool audioTracks [100];
    void* handle;
    bool indexingEnabled;
    int lastIndex, firstFrameInBuffer, samplesInBuffer;
    MemoryBlock buffer;
    AudioCDReader (void* handle);
    int getIndexAt (int samplePos);

#elif JUCE_LINUX
    AudioCDReader();
#endif

    AudioCDReader (const AudioCDReader&);
    const AudioCDReader& operator= (const AudioCDReader&);
};

#endif   // __JUCE_AUDIOCDREADER_JUCEHEADER__

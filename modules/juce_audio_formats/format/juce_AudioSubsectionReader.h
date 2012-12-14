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

#ifndef __JUCE_AUDIOSUBSECTIONREADER_JUCEHEADER__
#define __JUCE_AUDIOSUBSECTIONREADER_JUCEHEADER__

#include "juce_AudioFormatReader.h"


//==============================================================================
/**
    This class is used to wrap an AudioFormatReader and only read from a
    subsection of the file.

    So if you have a reader which can read a 1000 sample file, you could wrap it
    in one of these to only access, e.g. samples 100 to 200, and any samples
    outside that will come back as 0. Accessing sample 0 from this reader will
    actually read the first sample from the other's subsection, which might
    be at a non-zero position.

    @see AudioFormatReader
*/
class JUCE_API  AudioSubsectionReader  : public AudioFormatReader
{
public:
    //==============================================================================
    /** Creates a AudioSubsectionReader for a given data source.

        @param sourceReader             the source reader from which we'll be taking data
        @param subsectionStartSample    the sample within the source reader which will be
                                        mapped onto sample 0 for this reader.
        @param subsectionLength         the number of samples from the source that will
                                        make up the subsection. If this reader is asked for
                                        any samples beyond this region, it will return zero.
        @param deleteSourceWhenDeleted  if true, the sourceReader object will be deleted when
                                        this object is deleted.
    */
    AudioSubsectionReader (AudioFormatReader* sourceReader,
                           int64 subsectionStartSample,
                           int64 subsectionLength,
                           bool deleteSourceWhenDeleted);

    /** Destructor. */
    ~AudioSubsectionReader();


    //==============================================================================
    bool readSamples (int** destSamples, int numDestChannels, int startOffsetInDestBuffer,
                      int64 startSampleInFile, int numSamples);

    void readMaxLevels (int64 startSample,
                        int64 numSamples,
                        float& lowestLeft,
                        float& highestLeft,
                        float& lowestRight,
                        float& highestRight);


private:
    //==============================================================================
    AudioFormatReader* const source;
    int64 startSample, length;
    const bool deleteSourceWhenDeleted;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioSubsectionReader)
};

#endif   // __JUCE_AUDIOSUBSECTIONREADER_JUCEHEADER__

/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

#ifndef JUCE_AUDIOSUBSECTIONREADER_H_INCLUDED
#define JUCE_AUDIOSUBSECTIONREADER_H_INCLUDED


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
    /** Creates an AudioSubsectionReader for a given data source.

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
                      int64 startSampleInFile, int numSamples) override;

    void readMaxLevels (int64 startSample, int64 numSamples,
                        Range<float>* results, int numChannelsToRead) override;


private:
    //==============================================================================
    AudioFormatReader* const source;
    int64 startSample, length;
    const bool deleteSourceWhenDeleted;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioSubsectionReader)
};

#endif   // JUCE_AUDIOSUBSECTIONREADER_H_INCLUDED

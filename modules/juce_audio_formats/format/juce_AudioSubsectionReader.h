/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

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

    @tags{Audio}
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
    ~AudioSubsectionReader() override;


    //==============================================================================
    bool readSamples (int* const* destSamples, int numDestChannels, int startOffsetInDestBuffer,
                      int64 startSampleInFile, int numSamples) override;

    void readMaxLevels (int64 startSample, int64 numSamples,
                        Range<float>* results, int numChannelsToRead) override;

    using AudioFormatReader::readMaxLevels;

private:
    //==============================================================================
    AudioFormatReader* const source;
    int64 startSample, length;
    const bool deleteSourceWhenDeleted;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioSubsectionReader)
};

} // namespace juce

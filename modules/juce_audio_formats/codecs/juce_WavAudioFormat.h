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

//==============================================================================
/**
    Reads and Writes WAV format audio files.

    @see AudioFormat
*/
class JUCE_API  WavAudioFormat  : public AudioFormat
{
public:
    //==============================================================================
    /** Creates a format object. */
    WavAudioFormat();

    /** Destructor. */
    ~WavAudioFormat();

    //==============================================================================
    /** Metadata property name used by wav readers and writers for adding
        a BWAV chunk to the file.

        @see AudioFormatReader::metadataValues, createWriterFor
    */
    static const char* const bwavDescription;

    /** Metadata property name used by wav readers and writers for adding
        a BWAV chunk to the file.

        @see AudioFormatReader::metadataValues, createWriterFor
    */
    static const char* const bwavOriginator;

    /** Metadata property name used by wav readers and writers for adding
        a BWAV chunk to the file.

        @see AudioFormatReader::metadataValues, createWriterFor
    */
    static const char* const bwavOriginatorRef;

    /** Metadata property name used by wav readers and writers for adding
        a BWAV chunk to the file.

        Date format is: yyyy-mm-dd

        @see AudioFormatReader::metadataValues, createWriterFor
    */
    static const char* const bwavOriginationDate;

    /** Metadata property name used by wav readers and writers for adding
        a BWAV chunk to the file.

        Time format is: hh-mm-ss

        @see AudioFormatReader::metadataValues, createWriterFor
    */
    static const char* const bwavOriginationTime;

    /** Metadata property name used by wav readers and writers for adding
        a BWAV chunk to the file.

        This is the number of samples from the start of an edit that the
        file is supposed to begin at. Seems like an obvious mistake to
        only allow a file to occur in an edit once, but that's the way
        it is..

        @see AudioFormatReader::metadataValues, createWriterFor
    */
    static const char* const bwavTimeReference;

    /** Metadata property name used by wav readers and writers for adding
        a BWAV chunk to the file.

        @see AudioFormatReader::metadataValues, createWriterFor
    */
    static const char* const bwavCodingHistory;

    /** Utility function to fill out the appropriate metadata for a BWAV file.

        This just makes it easier than using the property names directly, and it
        fills out the time and date in the right format.
    */
    static StringPairArray createBWAVMetadata (const String& description,
                                               const String& originator,
                                               const String& originatorRef,
                                               const Time& dateAndTime,
                                               const int64 timeReferenceSamples,
                                               const String& codingHistory);

    //==============================================================================
    Array<int> getPossibleSampleRates();
    Array<int> getPossibleBitDepths();
    bool canDoStereo();
    bool canDoMono();

    //==============================================================================
    AudioFormatReader* createReaderFor (InputStream* sourceStream,
                                        bool deleteStreamIfOpeningFails);

    MemoryMappedAudioFormatReader* createMemoryMappedReader (const File& file);

    AudioFormatWriter* createWriterFor (OutputStream* streamToWriteTo,
                                        double sampleRateToUse,
                                        unsigned int numberOfChannels,
                                        int bitsPerSample,
                                        const StringPairArray& metadataValues,
                                        int qualityOptionIndex);

    //==============================================================================
    /** Utility function to replace the metadata in a wav file with a new set of values.

        If possible, this cheats by overwriting just the metadata region of the file, rather
        than by copying the whole file again.
    */
    bool replaceMetadataInFile (const File& wavFile, const StringPairArray& newMetadata);


private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WavAudioFormat)
};

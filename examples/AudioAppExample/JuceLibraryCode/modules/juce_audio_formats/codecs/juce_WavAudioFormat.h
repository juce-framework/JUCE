/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

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
                                               const Time dateAndTime,
                                               const int64 timeReferenceSamples,
                                               const String& codingHistory);

    //==============================================================================
    /** Metadata property name used when reading a WAV file with an acid chunk. */
    static const char* const acidOneShot;
    /** Metadata property name used when reading a WAV file with an acid chunk. */
    static const char* const acidRootSet;
    /** Metadata property name used when reading a WAV file with an acid chunk. */
    static const char* const acidStretch;
    /** Metadata property name used when reading a WAV file with an acid chunk. */
    static const char* const acidDiskBased;
    /** Metadata property name used when reading a WAV file with an acid chunk. */
    static const char* const acidizerFlag;
    /** Metadata property name used when reading a WAV file with an acid chunk. */
    static const char* const acidRootNote;
    /** Metadata property name used when reading a WAV file with an acid chunk. */
    static const char* const acidBeats;
    /** Metadata property name used when reading a WAV file with an acid chunk. */
    static const char* const acidDenominator;
    /** Metadata property name used when reading a WAV file with an acid chunk. */
    static const char* const acidNumerator;
    /** Metadata property name used when reading a WAV file with an acid chunk. */
    static const char* const acidTempo;

    //==============================================================================
    /** Metadata property name used when reading an ISRC code from an AXML chunk. */
    static const char* const ISRC;

    /** Metadata property name used when reading a WAV file with a Tracktion chunk. */
    static const char* const tracktionLoopInfo;

    //==============================================================================
    Array<int> getPossibleSampleRates() override;
    Array<int> getPossibleBitDepths() override;
    bool canDoStereo() override;
    bool canDoMono() override;

    //==============================================================================
    AudioFormatReader* createReaderFor (InputStream* sourceStream,
                                        bool deleteStreamIfOpeningFails) override;

    MemoryMappedAudioFormatReader* createMemoryMappedReader (const File& file) override;

    AudioFormatWriter* createWriterFor (OutputStream* streamToWriteTo,
                                        double sampleRateToUse,
                                        unsigned int numberOfChannels,
                                        int bitsPerSample,
                                        const StringPairArray& metadataValues,
                                        int qualityOptionIndex) override;

    //==============================================================================
    /** Utility function to replace the metadata in a wav file with a new set of values.

        If possible, this cheats by overwriting just the metadata region of the file, rather
        than by copying the whole file again.
    */
    bool replaceMetadataInFile (const File& wavFile, const StringPairArray& newMetadata);


private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WavAudioFormat)
};

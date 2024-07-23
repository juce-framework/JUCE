/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

#if JUCE_USE_OGGVORBIS || DOXYGEN

//==============================================================================
/**
    Reads and writes the Ogg-Vorbis audio format.

    To compile this, you'll need to set the JUCE_USE_OGGVORBIS flag.

    @see AudioFormat,

    @tags{Audio}
*/
class JUCE_API  OggVorbisAudioFormat  : public AudioFormat
{
public:
    //==============================================================================
    OggVorbisAudioFormat();
    ~OggVorbisAudioFormat() override;

    //==============================================================================
    Array<int> getPossibleSampleRates() override;
    Array<int> getPossibleBitDepths() override;
    bool canDoStereo() override;
    bool canDoMono() override;
    bool isCompressed() override;
    StringArray getQualityOptions() override;

    //==============================================================================
    /** Tries to estimate the quality level of an ogg file based on its size.

        If it can't read the file for some reason, this will just return 1 (medium quality),
        otherwise it will return the approximate quality setting that would have been used
        to create the file.

        @see getQualityOptions
    */
    int estimateOggFileQuality (const File& source);

    //==============================================================================
    /** Metadata property name used by the Ogg writer - if you set a string for this
        value, it will be written into the ogg file as the name of the encoder app.

        @see createWriterFor
    */
    static const char* const encoderName;

    static const char* const id3title;          /**< Metadata key for setting an ID3 title. */
    static const char* const id3artist;         /**< Metadata key for setting an ID3 artist name. */
    static const char* const id3album;          /**< Metadata key for setting an ID3 album. */
    static const char* const id3comment;        /**< Metadata key for setting an ID3 comment. */
    static const char* const id3date;           /**< Metadata key for setting an ID3 date. */
    static const char* const id3genre;          /**< Metadata key for setting an ID3 genre. */
    static const char* const id3trackNumber;    /**< Metadata key for setting an ID3 track number. */

    //==============================================================================
    AudioFormatReader* createReaderFor (InputStream* sourceStream,
                                        bool deleteStreamIfOpeningFails) override;

    AudioFormatWriter* createWriterFor (OutputStream* streamToWriteTo,
                                        double sampleRateToUse,
                                        unsigned int numberOfChannels,
                                        int bitsPerSample,
                                        const StringPairArray& metadataValues,
                                        int qualityOptionIndex) override;
    using AudioFormat::createWriterFor;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OggVorbisAudioFormat)
};


#endif

} // namespace juce

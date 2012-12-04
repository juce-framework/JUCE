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

#if JUCE_USE_OGGVORBIS || defined (DOXYGEN)

//==============================================================================
/**
    Reads and writes the Ogg-Vorbis audio format.

    To compile this, you'll need to set the JUCE_USE_OGGVORBIS flag.

    @see AudioFormat,
*/
class JUCE_API  OggVorbisAudioFormat  : public AudioFormat
{
public:
    //==============================================================================
    OggVorbisAudioFormat();
    ~OggVorbisAudioFormat();

    //==============================================================================
    Array<int> getPossibleSampleRates();
    Array<int> getPossibleBitDepths();
    bool canDoStereo();
    bool canDoMono();
    bool isCompressed();
    StringArray getQualityOptions();

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
                                        bool deleteStreamIfOpeningFails);

    AudioFormatWriter* createWriterFor (OutputStream* streamToWriteTo,
                                        double sampleRateToUse,
                                        unsigned int numberOfChannels,
                                        int bitsPerSample,
                                        const StringPairArray& metadataValues,
                                        int qualityOptionIndex);

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OggVorbisAudioFormat)
};


#endif

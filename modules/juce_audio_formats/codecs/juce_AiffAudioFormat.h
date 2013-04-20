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
    Reads and Writes AIFF format audio files.

    @see AudioFormat
*/
class JUCE_API  AiffAudioFormat  : public AudioFormat
{
public:
    //==============================================================================
    /** Creates an format object. */
    AiffAudioFormat();

    /** Destructor. */
    ~AiffAudioFormat();

    //==============================================================================
    /** Metadata property name used when reading a aiff file with a basc chunk. */
    static const char* const appleOneShot;
    /** Metadata property name used when reading a aiff file with a basc chunk. */
    static const char* const appleRootSet;
    /** Metadata property name used when reading a aiff file with a basc chunk. */
    static const char* const appleRootNote;
    /** Metadata property name used when reading a aiff file with a basc chunk. */
    static const char* const appleBeats;
    /** Metadata property name used when reading a aiff file with a basc chunk. */
    static const char* const appleDenominator;
    /** Metadata property name used when reading a aiff file with a basc chunk. */
    static const char* const appleNumerator;
    /** Metadata property name used when reading a aiff file with a basc chunk. */
    static const char* const appleTag;
    /** Metadata property name used when reading a aiff file with a basc chunk. */
    static const char* const appleKey;

    //==============================================================================
    Array<int> getPossibleSampleRates();
    Array<int> getPossibleBitDepths();
    bool canDoStereo();
    bool canDoMono();

   #if JUCE_MAC
    bool canHandleFile (const File& fileToTest);
   #endif

    //==============================================================================
    AudioFormatReader* createReaderFor (InputStream* sourceStream,
                                        bool deleteStreamIfOpeningFails);

    MemoryMappedAudioFormatReader* createMemoryMappedReader (const File&);

    AudioFormatWriter* createWriterFor (OutputStream* streamToWriteTo,
                                        double sampleRateToUse,
                                        unsigned int numberOfChannels,
                                        int bitsPerSample,
                                        const StringPairArray& metadataValues,
                                        int qualityOptionIndex);


private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AiffAudioFormat)
};

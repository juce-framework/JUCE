/*
  ==============================================================================

   This file is part of the JUCE 6 technical preview.
   Copyright (c) 2020 - Raw Material Software Limited

   You may use this code under the terms of the GPL v3
   (see www.gnu.org/licenses).

   For this technical preview, this file is not subject to commercial licensing.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

#if JUCE_MAC || JUCE_IOS || DOXYGEN

//==============================================================================
/**
    OSX and iOS only - This uses the AudioToolbox framework to read any audio
    format that the system has a codec for.

    This should be able to understand formats such as mp3, m4a, etc.

    @see AudioFormat

    @tags{Audio}
*/
class JUCE_API  CoreAudioFormat     : public AudioFormat
{
public:
    //==============================================================================
    /** Creates a format object. */
    CoreAudioFormat();

    /** Destructor. */
    ~CoreAudioFormat() override;

    //==============================================================================
    /** Metadata property name used when reading a caf file with a MIDI chunk. */
    static const char* const midiDataBase64;
    /** Metadata property name used when reading a caf file with tempo information. */
    static const char* const tempo;
    /** Metadata property name used when reading a caf file time signature information. */
    static const char* const timeSig;
    /** Metadata property name used when reading a caf file time signature information. */
    static const char* const keySig;

    //==============================================================================
    Array<int> getPossibleSampleRates() override;
    Array<int> getPossibleBitDepths() override;
    bool canDoStereo() override;
    bool canDoMono() override;

    //==============================================================================
    AudioFormatReader* createReaderFor (InputStream*,
                                        bool deleteStreamIfOpeningFails) override;

    AudioFormatWriter* createWriterFor (OutputStream*,
                                        double sampleRateToUse,
                                        unsigned int numberOfChannels,
                                        int bitsPerSample,
                                        const StringPairArray& metadataValues,
                                        int qualityOptionIndex) override;
    using AudioFormat::createWriterFor;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CoreAudioFormat)
};

#endif

} // namespace juce

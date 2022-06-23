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
    /** File type hints. */
    enum class StreamKind
    {
        kNone,
        kAiff,
        kAifc,
        kWave,
        kSoundDesigner2,
        kNext,
        kMp3,
        kMp2,
        kMp1,
        kAc3,
        kAacAdts,
        kMpeg4,
        kM4a,
        kM4b,
        kCaf,
        k3gp,
        k3gp2,
        kAmr,
    };

    //==============================================================================
    /** Creates a format object. */
    CoreAudioFormat();

    /** Creates a format object and provides a hint as to the format of data
        to be read or written.
    */
    explicit CoreAudioFormat (StreamKind);

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
    StreamKind streamKind = StreamKind::kNone;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CoreAudioFormat)
};

#endif

} // namespace juce

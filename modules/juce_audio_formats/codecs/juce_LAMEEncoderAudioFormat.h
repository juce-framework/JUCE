/*
  ==============================================================================

   This file is part of the JUCE 7 technical preview.
   Copyright (c) 2022 - Raw Material Software Limited

   You may use this code under the terms of the GPL v3
   (see www.gnu.org/licenses).

   For the technical preview this file cannot be licensed commercially.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

#if JUCE_USE_LAME_AUDIO_FORMAT || DOXYGEN

//==============================================================================
/**
    An AudioFormat class which can use an installed version of the LAME mp3
    encoder to encode a file.

    This format can't read MP3s, it just writes them. Internally, the
    AudioFormatWriter object that is returned writes the incoming audio data
    to a temporary WAV file, and then when the writer is deleted, it invokes
    the LAME executable to convert the data to an MP3, whose data is then
    piped into the original OutputStream that was used when first creating
    the writer.

    @see AudioFormat

    @tags{Audio}
*/
class JUCE_API  LAMEEncoderAudioFormat    : public AudioFormat
{
public:
    /** Creates a LAMEEncoderAudioFormat that expects to find a working LAME
        executable at the location given.
    */
    LAMEEncoderAudioFormat (const File& lameExecutableToUse);
    ~LAMEEncoderAudioFormat();

    bool canHandleFile (const File&);
    Array<int> getPossibleSampleRates();
    Array<int> getPossibleBitDepths();
    bool canDoStereo();
    bool canDoMono();
    bool isCompressed();
    StringArray getQualityOptions();

    AudioFormatReader* createReaderFor (InputStream*, bool deleteStreamIfOpeningFails);

    AudioFormatWriter* createWriterFor (OutputStream*, double sampleRateToUse,
                                        unsigned int numberOfChannels, int bitsPerSample,
                                        const StringPairArray& metadataValues, int qualityOptionIndex);
    using AudioFormat::createWriterFor;

private:
    File lameApp;
    class Writer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LAMEEncoderAudioFormat)
};

#endif

} // namespace juce

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

#if JUCE_USE_LAME_AUDIO_FORMAT

class LAMEEncoderAudioFormat::Writer final : public AudioFormatWriter
{
public:
    Writer (OutputStream* destStream, const String& formatName,
            const File& appFile, int vbr, int cbr,
            double sampleRateIn, unsigned int numberOfChannels,
            int bitsPerSampleIn, const StringPairArray& metadata)
        : AudioFormatWriter (destStream, formatName, sampleRateIn,
                             numberOfChannels, (unsigned int) bitsPerSampleIn),
          vbrLevel (vbr), cbrBitrate (cbr)
    {
        WavAudioFormat wavFormat;

        if (auto out = tempWav.getFile().createOutputStream())
        {
            writer.reset (wavFormat.createWriterFor (out.release(), sampleRateIn, numChannels,
                                                     bitsPerSampleIn, metadata, 0));

            args.add (appFile.getFullPathName());

            args.add ("--quiet");

            if (cbrBitrate == 0)
            {
                args.add ("--vbr-new");
                args.add ("-V");
                args.add (String (vbrLevel));
            }
            else
            {
                args.add ("--cbr");
                args.add ("-b");
                args.add (String (cbrBitrate));
            }

            addMetadataArg (metadata, "id3title",       "--tt");
            addMetadataArg (metadata, "id3artist",      "--ta");
            addMetadataArg (metadata, "id3album",       "--tl");
            addMetadataArg (metadata, "id3comment",     "--tc");
            addMetadataArg (metadata, "id3date",        "--ty");
            addMetadataArg (metadata, "id3genre",       "--tg");
            addMetadataArg (metadata, "id3trackNumber", "--tn");
        }
    }

    void addMetadataArg (const StringPairArray& metadata, const char* key, const char* lameFlag)
    {
        auto value = metadata.getValue (key, {});

        if (value.isNotEmpty())
        {
            args.add (lameFlag);
            args.add (value);
        }
    }

    ~Writer()
    {
        if (writer != nullptr)
        {
            writer = nullptr;

            if (! convertToMP3())
                convertToMP3(); // try again
        }
    }

    bool write (const int** samplesToWrite, int numSamples)
    {
        return writer != nullptr && writer->write (samplesToWrite, numSamples);
    }

private:
    int vbrLevel, cbrBitrate;
    TemporaryFile tempWav { ".wav" };
    std::unique_ptr<AudioFormatWriter> writer;
    StringArray args;

    bool runLameChildProcess (const TemporaryFile& tempMP3, const StringArray& processArgs) const
    {
        ChildProcess cp;

        if (cp.start (processArgs))
        {
            [[maybe_unused]] auto childOutput = cp.readAllProcessOutput();
            DBG (childOutput);

            cp.waitForProcessToFinish (10000);
            return tempMP3.getFile().getSize() > 0;
        }

        return false;
    }

    bool convertToMP3() const
    {
        TemporaryFile tempMP3 (".mp3");

        StringArray args2 (args);
        args2.add (tempWav.getFile().getFullPathName());
        args2.add (tempMP3.getFile().getFullPathName());

        DBG (args2.joinIntoString (" "));

        if (runLameChildProcess (tempMP3, args2))
        {
            FileInputStream fis (tempMP3.getFile());

            if (fis.openedOk() && output->writeFromInputStream (fis, -1) > 0)
            {
                output->flush();
                return true;
            }
        }

        return false;
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Writer)
};

//==============================================================================
LAMEEncoderAudioFormat::LAMEEncoderAudioFormat (const File& lameApplication)
    : AudioFormat ("MP3 file", ".mp3"),
      lameApp (lameApplication)
{
}

LAMEEncoderAudioFormat::~LAMEEncoderAudioFormat()
{
}

bool LAMEEncoderAudioFormat::canHandleFile (const File&)
{
    return false;
}

Array<int> LAMEEncoderAudioFormat::getPossibleSampleRates()
{
    return { 32000, 44100, 48000 };
}

Array<int> LAMEEncoderAudioFormat::getPossibleBitDepths()
{
    return { 16 };
}

bool LAMEEncoderAudioFormat::canDoStereo()      { return true; }
bool LAMEEncoderAudioFormat::canDoMono()        { return true; }
bool LAMEEncoderAudioFormat::isCompressed()     { return true; }

StringArray LAMEEncoderAudioFormat::getQualityOptions()
{
    static const char* vbrOptions[] = { "VBR quality 0 (best)", "VBR quality 1", "VBR quality 2", "VBR quality 3",
                                        "VBR quality 4 (normal)", "VBR quality 5", "VBR quality 6", "VBR quality 7",
                                        "VBR quality 8", "VBR quality 9 (smallest)", nullptr };
    StringArray opts (vbrOptions);

    const int cbrRates[] = { 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320 };

    for (int i = 0; i < numElementsInArray (cbrRates); ++i)
        opts.add (String (cbrRates[i]) + " Kb/s CBR");

    return opts;
}

AudioFormatReader* LAMEEncoderAudioFormat::createReaderFor (InputStream*, const bool)
{
    return nullptr;
}

AudioFormatWriter* LAMEEncoderAudioFormat::createWriterFor (OutputStream* streamToWriteTo,
                                                            double sampleRateToUse,
                                                            unsigned int numberOfChannels,
                                                            int bitsPerSample,
                                                            const StringPairArray& metadataValues,
                                                            int qualityOptionIndex)
{
    if (streamToWriteTo == nullptr)
        return nullptr;

    int vbr = 4;
    int cbr = 0;

    const String qual (getQualityOptions() [qualityOptionIndex]);

    if (qual.contains ("VBR"))
        vbr = qual.retainCharacters ("0123456789").getIntValue();
    else
        cbr = qual.getIntValue();

    return new Writer (streamToWriteTo, getFormatName(), lameApp, vbr, cbr,
                       sampleRateToUse, numberOfChannels, bitsPerSample, metadataValues);
}

#endif

} // namespace juce

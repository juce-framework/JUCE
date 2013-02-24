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

#if JUCE_USE_LAME_AUDIO_FORMAT || defined (DOXYGEN)

//==============================================================================
/**
    An AudioFormat class which can use an installed version of the LAME mp3
    encoder to encode a file.

    This format can't read mp3s, it just writes them. Internally, the
    AudioFormatWriter object that is returned writes the incoming audio data
    to a temporary WAV file, and then when the writer is deleted, it invokes
    the LAME executable to convert the data to an MP3, whose data is then
    piped into the original OutputStream that was used when first creating
    the writer.

    @see AudioFormat
*/
class JUCE_API  LAMEEncoderAudioFormat    : public AudioFormat
{
public:
    /** Creates a LAMEEncoderAudioFormat that expects to find a working LAME
        executable at the location given.
    */
    LAMEEncoderAudioFormat (const File& lameApplicationToUse);
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

private:
    File lameApp;
    class Writer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LAMEEncoderAudioFormat)
};

#endif

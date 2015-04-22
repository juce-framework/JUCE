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

#if JUCE_MAC || JUCE_IOS || DOXYGEN

//==============================================================================
/**
    OSX and iOS only - This uses the AudioToolbox framework to read any audio
    format that the system has a codec for.

    This should be able to understand formats such as mp3, m4a, etc.

    @see AudioFormat
 */
class JUCE_API  CoreAudioFormat     : public AudioFormat
{
public:
    //==============================================================================
    /** Creates a format object. */
    CoreAudioFormat();

    /** Destructor. */
    ~CoreAudioFormat();

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

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CoreAudioFormat)
};

#endif

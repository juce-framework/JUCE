/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

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

#if JUCE_QUICKTIME

//==============================================================================
/**
    Uses QuickTime to read the audio track a movie or media file.

    As well as QuickTime movies, this should also manage to open other audio
    files that quicktime can understand, like mp3, m4a, etc.

    @see AudioFormat
*/
class JUCE_API  QuickTimeAudioFormat  : public AudioFormat
{
public:
    //==============================================================================
    /** Creates a format object. */
    QuickTimeAudioFormat();

    /** Destructor. */
    ~QuickTimeAudioFormat();

    //==============================================================================
    Array<int> getPossibleSampleRates();
    Array<int> getPossibleBitDepths();
    bool canDoStereo();
    bool canDoMono();

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
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (QuickTimeAudioFormat)
};


#endif

/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

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

#ifndef __JUCE_QUICKTIMEAUDIOFORMAT_JUCEHEADER__
#define __JUCE_QUICKTIMEAUDIOFORMAT_JUCEHEADER__

#include "juce_AudioFormat.h"
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
    const Array <int> getPossibleSampleRates();
    const Array <int> getPossibleBitDepths();
    bool canDoStereo();
    bool canDoMono();

    //==============================================================================
    AudioFormatReader* createReaderFor (InputStream* sourceStream,
                                        const bool deleteStreamIfOpeningFails);

    AudioFormatWriter* createWriterFor (OutputStream* streamToWriteTo,
                                        double sampleRateToUse,
                                        unsigned int numberOfChannels,
                                        int bitsPerSample,
                                        const StringPairArray& metadataValues,
                                        int qualityOptionIndex);


    //==============================================================================
    juce_UseDebuggingNewOperator
};


#endif
#endif   // __JUCE_QUICKTIMEAUDIOFORMAT_JUCEHEADER__

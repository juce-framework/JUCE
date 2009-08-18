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

#ifndef __JUCE_OGGVORBISAUDIOFORMAT_JUCEHEADER__
#define __JUCE_OGGVORBISAUDIOFORMAT_JUCEHEADER__

#include "juce_AudioFormat.h" // (must keep this outside the conditional define)

#if JUCE_USE_OGGVORBIS || defined (DOXYGEN)


//==============================================================================
/**
    Reads and writes the Ogg-Vorbis audio format.

    To compile this, you'll need to set the JUCE_USE_OGGVORBIS flag in juce_Config.h,
    and make sure your include search path and library search path are set up to find
    the Vorbis and Ogg header files and static libraries.

    @see AudioFormat,
*/
class JUCE_API  OggVorbisAudioFormat : public AudioFormat
{
public:
    //==============================================================================
    OggVorbisAudioFormat();
    ~OggVorbisAudioFormat();

    //==============================================================================
    const Array <int> getPossibleSampleRates();
    const Array <int> getPossibleBitDepths();
    bool canDoStereo();
    bool canDoMono();
    bool isCompressed();
    const StringArray getQualityOptions();

    //==============================================================================
    /** Tries to estimate the quality level of an ogg file based on its size.

        If it can't read the file for some reason, this will just return 1 (medium quality),
        otherwise it will return the approximate quality setting that would have been used
        to create the file.

        @see getQualityOptions
    */
    int estimateOggFileQuality (const File& source);


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
#endif   // __JUCE_OGGVORBISAUDIOFORMAT_JUCEHEADER__

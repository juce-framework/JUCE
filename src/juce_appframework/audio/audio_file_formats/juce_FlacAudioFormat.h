/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

  ==============================================================================
*/

#ifndef __JUCE_FLACAUDIOFORMAT_JUCEHEADER__
#define __JUCE_FLACAUDIOFORMAT_JUCEHEADER__

#if JUCE_USE_FLAC || defined (DOXYGEN)

#include "juce_AudioFormat.h"


//==============================================================================
/**
    Reads and writes the lossless-compression FLAC audio format.

    To compile this, you'll need to set the JUCE_USE_FLAC flag in juce_Config.h,
    and make sure your include search path and library search path are set up to find
    the FLAC header files and static libraries.

    @see AudioFormat
*/
class JUCE_API  FlacAudioFormat    : public AudioFormat
{
public:
    //==============================================================================
    FlacAudioFormat();
    ~FlacAudioFormat();

    //==============================================================================
    const Array <int> getPossibleSampleRates();
    const Array <int> getPossibleBitDepths();
    bool canDoStereo();
    bool canDoMono();
    bool isCompressed();

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
#endif   // __JUCE_FLACAUDIOFORMAT_JUCEHEADER__

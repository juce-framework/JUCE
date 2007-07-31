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

#ifndef __JUCE_WAVAUDIOFORMAT_JUCEHEADER__
#define __JUCE_WAVAUDIOFORMAT_JUCEHEADER__

#include "juce_AudioFormat.h"


//==============================================================================
/**
    Reads and Writes WAV format audio files.

    @see AudioFormat
*/
class JUCE_API  WavAudioFormat  : public AudioFormat
{
public:
    //==============================================================================
    /** Creates a format object. */
    WavAudioFormat();

    /** Destructor. */
    ~WavAudioFormat();

    //==============================================================================
    /** Metadata property name used by wav readers and writers for adding
        a BWAV chunk to the file.

        @see AudioFormatReader::metadataValues, createWriterFor
    */
    static const tchar* const bwavDescription;

    /** Metadata property name used by wav readers and writers for adding
        a BWAV chunk to the file.

        @see AudioFormatReader::metadataValues, createWriterFor
    */
    static const tchar* const bwavOriginator;

    /** Metadata property name used by wav readers and writers for adding
        a BWAV chunk to the file.

        @see AudioFormatReader::metadataValues, createWriterFor
    */
    static const tchar* const bwavOriginatorRef;

    /** Metadata property name used by wav readers and writers for adding
        a BWAV chunk to the file.

        Date format is: yyyy-mm-dd

        @see AudioFormatReader::metadataValues, createWriterFor
    */
    static const tchar* const bwavOriginationDate;

    /** Metadata property name used by wav readers and writers for adding
        a BWAV chunk to the file.

        Time format is: hh-mm-ss

        @see AudioFormatReader::metadataValues, createWriterFor
    */
    static const tchar* const bwavOriginationTime;

    /** Metadata property name used by wav readers and writers for adding
        a BWAV chunk to the file.

        This is the number of samples from the start of an edit that the
        file is supposed to begin at. Seems like an obvious mistake to
        only allow a file to occur in an edit once, but that's the way
        it is..

        @see AudioFormatReader::metadataValues, createWriterFor
    */
    static const tchar* const bwavTimeReference;

    /** Metadata property name used by wav readers and writers for adding
        a BWAV chunk to the file.

        This is a

        @see AudioFormatReader::metadataValues, createWriterFor
    */
    static const tchar* const bwavCodingHistory;

    /** Utility function to fill out the appropriate metadata for a BWAV file.

        This just makes it easier than using the property names directly, and it
        fills out the time and date in the right format.
    */
    static const StringPairArray createBWAVMetadata (const String& description,
                                                     const String& originator,
                                                     const String& originatorRef,
                                                     const Time& dateAndTime,
                                                     const int64 timeReferenceSamples,
                                                     const String& codingHistory);

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


#endif   // __JUCE_WAVAUDIOFORMAT_JUCEHEADER__

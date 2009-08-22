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

#ifndef __JUCE_AUDIOFORMATREADERSOURCE_JUCEHEADER__
#define __JUCE_AUDIOFORMATREADERSOURCE_JUCEHEADER__

#include "juce_PositionableAudioSource.h"
#include "../../threads/juce_Thread.h"
#include "../audio_file_formats/juce_AudioFormatReader.h"
#include "../dsp/juce_AudioSampleBuffer.h"


//==============================================================================
/**
    A type of AudioSource that will read from an AudioFormatReader.

    @see PositionableAudioSource, AudioTransportSource, BufferingAudioSource
*/
class JUCE_API  AudioFormatReaderSource  : public PositionableAudioSource
{
public:
    //==============================================================================
    /** Creates an AudioFormatReaderSource for a given reader.

        @param sourceReader                     the reader to use as the data source
        @param deleteReaderWhenThisIsDeleted    if true, the reader passed-in will be deleted
                                                when this object is deleted; if false it will be
                                                left up to the caller to manage its lifetime
    */
    AudioFormatReaderSource (AudioFormatReader* const sourceReader,
                             const bool deleteReaderWhenThisIsDeleted);

    /** Destructor. */
    ~AudioFormatReaderSource();

    //==============================================================================
    /** Toggles loop-mode.

        If set to true, it will continuously loop the input source. If false,
        it will just emit silence after the source has finished.

        @see isLooping
    */
    void setLooping (const bool shouldLoop) throw();

    /** Returns whether loop-mode is turned on or not. */
    bool isLooping() const                                      { return looping; }

    /** Returns the reader that's being used. */
    AudioFormatReader* getAudioFormatReader() const throw()     { return reader; }

    //==============================================================================
    /** Implementation of the AudioSource method. */
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate);

    /** Implementation of the AudioSource method. */
    void releaseResources();

    /** Implementation of the AudioSource method. */
    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill);

    //==============================================================================
    /** Implements the PositionableAudioSource method. */
    void setNextReadPosition (int newPosition);

    /** Implements the PositionableAudioSource method. */
    int getNextReadPosition() const;

    /** Implements the PositionableAudioSource method. */
    int getTotalLength() const;


    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    AudioFormatReader* reader;
    bool deleteReader;

    int volatile nextPlayPos;
    bool volatile looping;

    void readBufferSection (int start, int length, AudioSampleBuffer& buffer, int startSample);

    AudioFormatReaderSource (const AudioFormatReaderSource&);
    const AudioFormatReaderSource& operator= (const AudioFormatReaderSource&);
};




#endif   // __JUCE_AUDIOFORMATREADERSOURCE_JUCEHEADER__

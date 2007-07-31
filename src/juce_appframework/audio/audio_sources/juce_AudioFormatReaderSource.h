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

#ifndef __JUCE_AUDIOFORMATREADERSOURCE_JUCEHEADER__
#define __JUCE_AUDIOFORMATREADERSOURCE_JUCEHEADER__

#include "juce_PositionableAudioSource.h"
#include "../../../juce_core/threads/juce_Thread.h"
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

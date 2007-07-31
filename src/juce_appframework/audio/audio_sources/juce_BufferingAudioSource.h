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

#ifndef __JUCE_BUFFERINGAUDIOSOURCE_JUCEHEADER__
#define __JUCE_BUFFERINGAUDIOSOURCE_JUCEHEADER__

#include "juce_PositionableAudioSource.h"
#include "../../../juce_core/threads/juce_Thread.h"
#include "../dsp/juce_AudioSampleBuffer.h"


//==============================================================================
/**
    An AudioSource which takes another source as input, and buffers it using a thread.

    Create this as a wrapper around another thread, and it will read-ahead with
    a background thread to smooth out playback. You can either create one of these
    directly, or use it indirectly using an AudioTransportSource.

    @see PositionableAudioSource, AudioTransportSource
*/
class JUCE_API  BufferingAudioSource  : public PositionableAudioSource
{
public:
    //==============================================================================
    /** Creates a BufferingAudioSource.

        @param source                   the input source to read from
        @param deleteSourceWhenDeleted  if true, then the input source object will
                                        be deleted when this object is deleted
        @param numberOfSamplesToBuffer  the size of buffer to use for reading ahead
    */
    BufferingAudioSource (PositionableAudioSource* source,
                          const bool deleteSourceWhenDeleted,
                          int numberOfSamplesToBuffer);

    /** Destructor.

        The input source may be deleted depending on whether the deleteSourceWhenDeleted
        flag was set in the constructor.
    */
    ~BufferingAudioSource();

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
    int getTotalLength() const                  { return source->getTotalLength(); }

    /** Implements the PositionableAudioSource method. */
    bool isLooping() const                      { return source->isLooping(); }

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    //==============================================================================
    PositionableAudioSource* source;
    bool deleteSourceWhenDeleted;
    int numberOfSamplesToBuffer;
    AudioSampleBuffer buffer;
    CriticalSection bufferStartPosLock;
    int volatile bufferValidStart, bufferValidEnd, nextPlayPos;
    bool wasSourceLooping;
    double volatile sampleRate;

    friend class SharedBufferingAudioSourceThread;
    bool readNextBufferChunk();
    void readBufferSection (int start, int length, int bufferOffset);

    BufferingAudioSource (const BufferingAudioSource&);
    const BufferingAudioSource& operator= (const BufferingAudioSource&);
};


#endif   // __JUCE_BUFFERINGAUDIOSOURCE_JUCEHEADER__

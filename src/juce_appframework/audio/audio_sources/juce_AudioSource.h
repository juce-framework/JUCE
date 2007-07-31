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

#ifndef __JUCE_AUDIOSOURCE_JUCEHEADER__
#define __JUCE_AUDIOSOURCE_JUCEHEADER__

#include "../dsp/juce_AudioSampleBuffer.h"


//==============================================================================
/**
    Used by AudioSource::getNextAudioBlock().
*/
struct JUCE_API  AudioSourceChannelInfo
{
    /** The destination buffer to fill with audio data.

        When the AudioSource::getNextAudioBlock() method is called, the active section
        of this buffer should be filled with whatever output the source produces.

        Only the samples specified by the startSample and numSamples members of this structure
        should be affected by the call.

        The contents of the buffer when it is passed to the the AudioSource::getNextAudioBlock()
        method can be treated as the input if the source is performing some kind of filter operation,
        but should be cleared if this is not the case - the clearActiveBufferRegion() is
        a handy way of doing this.

        The number of channels in the buffer could be anything, so the AudioSource
        must cope with this in whatever way is appropriate for its function.
    */
    AudioSampleBuffer* buffer;

    /** The first sample in the buffer from which the callback is expected
        to write data. */
    int startSample;

    /** The number of samples in the buffer which the callback is expected to
        fill with data. */
    int numSamples;

    /** Convenient method to clear the buffer if the source is not producing any data. */
    void clearActiveBufferRegion() const
    {
        if (buffer != 0)
            buffer->clear (startSample, numSamples);
    }
};


//==============================================================================
/**
    Base class for objects that can produce a continuous stream of audio.

    @see AudioFormatReaderSource, ResamplingAudioSource
*/
class JUCE_API  AudioSource
{
protected:
    //==============================================================================
    /** Creates an AudioSource. */
    AudioSource() throw()       {}

public:
    /** Destructor. */
    virtual ~AudioSource()      {}

    //==============================================================================
    /** Tells the source to prepare for playing.

        The source can use this opportunity to initialise anything it needs to.

        @param samplesPerBlockExpected  the number of samples that the source
                                        will be expected to supply each time its
                                        getNextAudioBlock() method is called. This
                                        number may vary slightly, because it will be dependent
                                        on audio hardware callbacks, and these aren't
                                        guaranteed to always use a constant block size, so
                                        the source should be able to cope with small variations.
        @param sampleRate               the sample rate that the output will be used at - this
                                        is needed by sources such as tone generators.
        @see releaseResources, getNextAudioBlock
    */
    virtual void prepareToPlay (int samplesPerBlockExpected,
                                double sampleRate) = 0;

    /** Allows the source to release anything it no longer needs after playback has stopped.

        This will be called when the source is no longer going to have its getNextAudioBlock()
        method called, so it should release any spare memory, etc. that it might have
        allocated during the prepareToPlay() call.

        @see prepareToPlay, getNextAudioBlock
    */
    virtual void releaseResources() = 0;

    /** Called repeatedly to fetch subsequent blocks of audio data.

        After calling the prepareToPlay() method, this callback will be made each
        time the audio playback hardware (or whatever other destination the audio
        data is going to) needs another block of data.

        It will generally be called on a high-priority system thread, or possibly even
        an interrupt, so be careful not to do too much work here, as that will cause
        audio glitches!

        @see AudioSourceChannelInfo, prepareToPlay, releaseResources
    */
    virtual void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) = 0;
};


#endif   // __JUCE_AUDIOSOURCE_JUCEHEADER__

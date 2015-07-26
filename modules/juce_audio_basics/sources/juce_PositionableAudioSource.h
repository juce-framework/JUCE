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

#ifndef JUCE_POSITIONABLEAUDIOSOURCE_H_INCLUDED
#define JUCE_POSITIONABLEAUDIOSOURCE_H_INCLUDED


//==============================================================================
/**
    A type of AudioSource which can be repositioned.

    The basic AudioSource just streams continuously with no idea of a current
    time or length, so the PositionableAudioSource is used for a finite stream
    that has a current read position.

    @see AudioSource, AudioTransportSource
*/
class JUCE_API  PositionableAudioSource  : public AudioSource
{
protected:
    //==============================================================================
    /** Creates the PositionableAudioSource. */
    PositionableAudioSource() noexcept  {}

public:
    /** Destructor */
    ~PositionableAudioSource()          {}

    //==============================================================================
    /** Tells the stream to move to a new position.

        Calling this indicates that the next call to AudioSource::getNextAudioBlock()
        should return samples from this position.

        Note that this may be called on a different thread to getNextAudioBlock(),
        so the subclass should make sure it's synchronised.
    */
    virtual void setNextReadPosition (int64 newPosition) = 0;

    /** Returns the position from which the next block will be returned.

        @see setNextReadPosition
    */
    virtual int64 getNextReadPosition() const = 0;

    /** Returns the total length of the stream (in samples). */
    virtual int64 getTotalLength() const = 0;

    /** Returns true if this source is actually playing in a loop. */
    virtual bool isLooping() const = 0;

    /** Tells the source whether you'd like it to play in a loop. */
    virtual void setLooping (bool shouldLoop)       { (void) shouldLoop; }
};


#endif   // JUCE_POSITIONABLEAUDIOSOURCE_H_INCLUDED

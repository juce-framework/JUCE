/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

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

#ifndef __JUCE_POSITIONABLEAUDIOSOURCE_JUCEHEADER__
#define __JUCE_POSITIONABLEAUDIOSOURCE_JUCEHEADER__

#include "juce_AudioSource.h"


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


#endif   // __JUCE_POSITIONABLEAUDIOSOURCE_JUCEHEADER__

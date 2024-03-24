/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

//==============================================================================
/**
    A type of AudioSource which can be repositioned.

    The basic AudioSource just streams continuously with no idea of a current
    time or length, so the PositionableAudioSource is used for a finite stream
    that has a current read position.

    @see AudioSource, AudioTransportSource

    @tags{Audio}
*/
class JUCE_API  PositionableAudioSource  : public AudioSource
{
protected:
    //==============================================================================
    /** Creates the PositionableAudioSource. */
    PositionableAudioSource() = default;

public:
    /** Destructor */
    ~PositionableAudioSource() override = default;

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
    virtual void setLooping (bool shouldLoop);
};

} // namespace juce

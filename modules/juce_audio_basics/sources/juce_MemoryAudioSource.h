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
    An AudioSource which takes some float audio data as an input.

    @tags{Audio}
*/
class JUCE_API MemoryAudioSource   : public PositionableAudioSource
{
public:
    //==============================================================================
    /**  Creates a MemoryAudioSource by providing an audio buffer.

         If copyMemory is true then the buffer will be copied into an internal
         buffer which will be owned by the MemoryAudioSource. If copyMemory is
         false, then you must ensure that the lifetime of the audio buffer is
         at least as long as the MemoryAudioSource.
    */
    MemoryAudioSource (AudioBuffer<float>& audioBuffer, bool copyMemory, bool shouldLoop = false);

    //==============================================================================
    /** Implementation of the AudioSource method. */
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;

    /** Implementation of the AudioSource method. */
    void releaseResources() override;

    /** Implementation of the AudioSource method. */
    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override;

    //==============================================================================
    /** Implementation of the PositionableAudioSource method. */
    void setNextReadPosition (int64 newPosition) override;

    /** Implementation of the PositionableAudioSource method. */
    int64 getNextReadPosition() const override;

    /** Implementation of the PositionableAudioSource method. */
    int64 getTotalLength() const override;

    //==============================================================================
    /** Implementation of the PositionableAudioSource method. */
    bool isLooping() const override;

    /** Implementation of the PositionableAudioSource method. */
    void setLooping (bool shouldLoop) override;

private:
    //==============================================================================
    AudioBuffer<float> buffer;
    int position = 0;
    bool isCurrentlyLooping;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MemoryAudioSource)
};

} // namespace juce

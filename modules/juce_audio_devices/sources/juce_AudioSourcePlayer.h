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
    Wrapper class to continuously stream audio from an audio source to an
    AudioIODevice.

    This object acts as an AudioIODeviceCallback, so can be attached to an
    output device, and will stream audio from an AudioSource.

    @tags{Audio}
*/
class JUCE_API  AudioSourcePlayer  : public AudioIODeviceCallback
{
public:
    //==============================================================================
    /** Creates an empty AudioSourcePlayer. */
    AudioSourcePlayer();

    /** Destructor.

        Make sure this object isn't still being used by an AudioIODevice before
        deleting it!
    */
    ~AudioSourcePlayer() override;

    //==============================================================================
    /** Changes the current audio source to play from.

        If the source passed in is already being used, this method will do nothing.
        If the source is not null, its prepareToPlay() method will be called
        before it starts being used for playback.

        If there's another source currently playing, its releaseResources() method
        will be called after it has been swapped for the new one.

        @param newSource                the new source to use - this will NOT be deleted
                                        by this object when no longer needed, so it's the
                                        caller's responsibility to manage it.
    */
    void setSource (AudioSource* newSource);

    /** Returns the source that's playing.
        May return nullptr if there's no source.
    */
    AudioSource* getCurrentSource() const noexcept      { return source; }

    /** Sets a gain to apply to the audio data.
        @see getGain
    */
    void setGain (float newGain) noexcept;

    /** Returns the current gain.
        @see setGain
    */
    float getGain() const noexcept                      { return gain; }

    //==============================================================================
    /** Implementation of the AudioIODeviceCallbackWithContext method. */
    void audioDeviceIOCallbackWithContext (const float* const* inputChannelData,
                                           int totalNumInputChannels,
                                           float* const* outputChannelData,
                                           int totalNumOutputChannels,
                                           int numSamples,
                                           const AudioIODeviceCallbackContext& context) override;

    /** Implementation of the AudioIODeviceCallback method. */
    void audioDeviceAboutToStart (AudioIODevice* device) override;

    /** Implementation of the AudioIODeviceCallback method. */
    void audioDeviceStopped() override;

    /** An alternative method for initialising the source without an AudioIODevice. */
    void prepareToPlay (double sampleRate, int blockSize);

private:
    //==============================================================================
    CriticalSection readLock;
    AudioSource* source = nullptr;
    double sampleRate = 0;
    int bufferSize = 0;
    float* channels[128];
    float* outputChans[128];
    const float* inputChans[128];
    AudioBuffer<float> tempBuffer;
    float lastGain = 1.0f;
    std::atomic<float> gain { 1.0f };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioSourcePlayer)
};

} // namespace juce

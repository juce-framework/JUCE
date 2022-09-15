/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

//==============================================================================
/**
    A simple sound player that you can add to the AudioDeviceManager to play
    simple sounds.

    @see AudioProcessor, AudioProcessorGraph

    @tags{Audio}
*/
class JUCE_API  SoundPlayer   : public AudioIODeviceCallback
{
public:
    //==============================================================================
    SoundPlayer();

    /** Destructor. */
    ~SoundPlayer() override;

    //==============================================================================
    /** Plays a sound from a file. */
    void play (const File& file);

    /** Convenient method to play sound from a JUCE resource. */
    void play (const void* resourceData, size_t resourceSize);

    /** Plays the sound from an audio format reader.

        If deleteWhenFinished is true then the format reader will be
        automatically deleted once the sound has finished playing.
     */
    void play (AudioFormatReader* buffer, bool deleteWhenFinished = false);

    /** Plays the sound from a positionable audio source.

        This will output the sound coming from a positionable audio source.
        This gives you slightly more control over the sound playback compared
        to  the other playSound methods. For example, if you would like to
        stop the sound prematurely you can call this method with a
        TransportAudioSource and then call audioSource->stop. Note that,
        you must call audioSource->start to start the playback, if your
        audioSource is a TransportAudioSource.

        The audio device manager will not hold any references to this audio
        source once the audio source has stopped playing for any reason,
        for example when the sound has finished playing or when you have
        called audioSource->stop. Therefore, calling audioSource->start() on
        a finished audioSource will not restart the sound again. If this is
        desired simply call playSound with the same audioSource again.

        @param audioSource   the audio source to play
        @param deleteWhenFinished If this is true then the audio source will
                                  be deleted once the device manager has finished
                                  playing.
        @param sampleRateOfSource The sample rate of the source. If this is zero, JUCE
                                  will assume that the sample rate is the same as the
                                  audio output device.
     */
    void play (PositionableAudioSource* audioSource, bool deleteWhenFinished = false,
               double sampleRateOfSource = 0.0);

    /** Plays the sound from an audio sample buffer.

        This will output the sound contained in an audio sample buffer. If
        deleteWhenFinished is true then the audio sample buffer will be
        automatically deleted once the sound has finished playing.

        If playOnAllOutputChannels is true, then if there are more output channels
        than buffer channels, then the ones that are available will be re-used on
        multiple outputs so that something is sent to all output channels. If it
        is false, then the buffer will just be played on the first output channels.
     */
    void play (AudioBuffer<float>* buffer,
               bool deleteWhenFinished = false,
               bool playOnAllOutputChannels = false);

    /** Plays a beep through the current audio device.

        This is here to allow the audio setup UI panels to easily include a "test"
        button so that the user can check where the audio is coming from.
     */
    void playTestSound();

    //==============================================================================
    /** @internal */
    void audioDeviceIOCallbackWithContext (const float* const*, int, float* const*, int, int, const AudioIODeviceCallbackContext&) override;
    /** @internal */
    void audioDeviceAboutToStart (AudioIODevice*) override;
    /** @internal */
    void audioDeviceStopped() override;
    /** @internal */
    void audioDeviceError (const String& errorMessage) override;

private:
    //==============================================================================
    AudioFormatManager formatManager;
    AudioSourcePlayer player;
    MixerAudioSource mixer;
    OwnedArray<AudioSource> sources;

    //==============================================================================
    double sampleRate;
    int bufferSize;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SoundPlayer)
};

} // namespace juce

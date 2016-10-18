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

#ifndef JUCE_SOUNDPLAYER_H_INCLUDED
#define JUCE_SOUNDPLAYER_H_INCLUDED

//==============================================================================
/**
    A simple sound player that you can add to the AudioDeviceManager to play
    simple sounds.

    @see AudioProcessor, AudioProcessorGraph
*/
class JUCE_API  SoundPlayer             : public AudioIODeviceCallback
{
public:
    //==============================================================================
    SoundPlayer ();

    /** Destructor. */
    virtual ~SoundPlayer();

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
     */
    void play (PositionableAudioSource* audioSource, bool deleteWhenFinished = false);

    /** Plays the sound from an audio sample buffer.

        This will output the sound contained in an audio sample buffer. If
        deleteWhenFinished is true then the audio sample buffer will be
        automatically deleted once the sound has finished playing.

        If playOnAllOutputChannels is true, then if there are more output channels
        than buffer channels, then the ones that are available will be re-used on
        multiple outputs so that something is sent to all output channels. If it
        is false, then the buffer will just be played on the first output channels.
     */
    void play (AudioSampleBuffer* buffer,
               bool deleteWhenFinished = false,
               bool playOnAllOutputChannels = false);

    /** Plays a beep through the current audio device.

        This is here to allow the audio setup UI panels to easily include a "test"
        button so that the user can check where the audio is coming from.
     */
    void playTestSound();

    //==============================================================================
    /** @internal */
    void audioDeviceIOCallback (const float**, int, float**, int, int) override;
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


#endif   // JUCE_SOUNDPLAYER_H_INCLUDED

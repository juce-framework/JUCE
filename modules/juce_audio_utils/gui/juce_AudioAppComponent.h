/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

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

#ifndef JUCE_AUDIOAPPCOMPONENT_H_INCLUDED
#define JUCE_AUDIOAPPCOMPONENT_H_INCLUDED


//==============================================================================
/**
    A base class for writing audio apps that stream from the audio i/o devices.

    A subclass can inherit from this and implement just a few methods such as
    renderAudio(). The base class provides a basic AudioDeviceManager object
    and runs audio through the
*/
class AudioAppComponent   : public Component,
                            public AudioSource
{
public:
    AudioAppComponent();
    ~AudioAppComponent();

    /** A subclass should call this from their constructor, to set up the audio. */
    void setAudioChannels (int numInputChannels, int numOutputChannels);

    virtual void prepareToPlay (int samplesPerBlockExpected,
                                double sampleRate) = 0;

    /** Allows the source to release anything it no longer needs after playback has stopped.

        This will be called when the source is no longer going to have its getNextAudioBlock()
        method called, so it should release any spare memory, etc. that it might have
        allocated during the prepareToPlay() call.

        Note that there's no guarantee that prepareToPlay() will actually have been called before
        releaseResources(), and it may be called more than once in succession, so make sure your
        code is robust and doesn't make any assumptions about when it will be called.

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

    AudioDeviceManager deviceManager;

private:
    //==============================================================================
    AudioSourcePlayer audioSourcePlayer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioAppComponent)
};


#endif   // JUCE_AUDIOAPPCOMPONENT_H_INCLUDED

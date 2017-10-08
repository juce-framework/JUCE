/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

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
    A base class for writing audio apps that stream from the audio i/o devices.

    A subclass can inherit from this and implement just a few methods such as
    getNextAudioBlock(). The base class provides a basic AudioDeviceManager object
    and runs audio through the default output device.
*/
class JUCE_API AudioAppComponent   : public Component,
                                     public AudioSource
{
public:
    AudioAppComponent();
    ~AudioAppComponent();

    /** A subclass should call this from their constructor, to set up the audio. */
    void setAudioChannels (int numInputChannels, int numOutputChannels, const XmlElement* const storedSettings = nullptr);

    /** Tells the source to prepare for playing.

        An AudioSource has two states: prepared and unprepared.

        The prepareToPlay() method is guaranteed to be called at least once on an 'unpreprared'
        source to put it into a 'prepared' state before any calls will be made to getNextAudioBlock().
        This callback allows the source to initialise any resources it might need when playing.

        Once playback has finished, the releaseResources() method is called to put the stream
        back into an 'unprepared' state.

        Note that this method could be called more than once in succession without
        a matching call to releaseResources(), so make sure your code is robust and
        can handle that kind of situation.

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

    void shutdownAudio();


    AudioDeviceManager deviceManager;

private:
    //==============================================================================
    AudioSourcePlayer audioSourcePlayer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioAppComponent)
};

} // namespace juce

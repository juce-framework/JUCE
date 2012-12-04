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

#ifndef __JUCE_AUDIOSOURCEPLAYER_JUCEHEADER__
#define __JUCE_AUDIOSOURCEPLAYER_JUCEHEADER__


//==============================================================================
/**
    Wrapper class to continuously stream audio from an audio source to an
    AudioIODevice.

    This object acts as an AudioIODeviceCallback, so can be attached to an
    output device, and will stream audio from an AudioSource.
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
    virtual ~AudioSourcePlayer();

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

        May return 0 if there's no source.
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
    /** Implementation of the AudioIODeviceCallback method. */
    void audioDeviceIOCallback (const float** inputChannelData,
                                int totalNumInputChannels,
                                float** outputChannelData,
                                int totalNumOutputChannels,
                                int numSamples);

    /** Implementation of the AudioIODeviceCallback method. */
    void audioDeviceAboutToStart (AudioIODevice* device);

    /** Implementation of the AudioIODeviceCallback method. */
    void audioDeviceStopped();

    /** An alternative method for initialising the source without an AudioIODevice. */
    void prepareToPlay (double sampleRate, int blockSize);

private:
    //==============================================================================
    CriticalSection readLock;
    AudioSource* source;
    double sampleRate;
    int bufferSize;
    float* channels [128];
    float* outputChans [128];
    const float* inputChans [128];
    AudioSampleBuffer tempBuffer;
    float lastGain, gain;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioSourcePlayer)
};


#endif   // __JUCE_AUDIOSOURCEPLAYER_JUCEHEADER__

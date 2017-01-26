/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2016 - ROLI Ltd.

   Permission is granted to use this software under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license/

   Permission to use, copy, modify, and/or distribute this software for any
   purpose with or without fee is hereby granted, provided that the above
   copyright notice and this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND ISC DISCLAIMS ALL WARRANTIES WITH REGARD
   TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
   FITNESS. IN NO EVENT SHALL ISC BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT,
   OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
   USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
   TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
   OF THIS SOFTWARE.

   -----------------------------------------------------------------------------

   To release a closed-source product which uses other parts of JUCE not
   licensed under the ISC terms, commercial licenses are available: visit
   www.juce.com for more information.

  ==============================================================================
*/

#ifndef JUCE_MIXERAUDIOSOURCE_H_INCLUDED
#define JUCE_MIXERAUDIOSOURCE_H_INCLUDED


//==============================================================================
/**
    An AudioSource that mixes together the output of a set of other AudioSources.

    Input sources can be added and removed while the mixer is running as long as their
    prepareToPlay() and releaseResources() methods are called before and after adding
    them to the mixer.
*/
class JUCE_API  MixerAudioSource  : public AudioSource
{
public:
    //==============================================================================
    /** Creates a MixerAudioSource. */
    MixerAudioSource();

    /** Destructor. */
    ~MixerAudioSource();

    //==============================================================================
    /** Adds an input source to the mixer.

        If the mixer is running you'll need to make sure that the input source
        is ready to play by calling its prepareToPlay() method before adding it.
        If the mixer is stopped, then its input sources will be automatically
        prepared when the mixer's prepareToPlay() method is called.

        @param newInput             the source to add to the mixer
        @param deleteWhenRemoved    if true, then this source will be deleted when
                                    no longer needed by the mixer.
    */
    void addInputSource (AudioSource* newInput, bool deleteWhenRemoved);

    /** Removes an input source.
        If the source was added by calling addInputSource() with the deleteWhenRemoved
        flag set, it will be deleted by this method.
    */
    void removeInputSource (AudioSource* input);

    /** Removes all the input sources.
        Any sources which were added by calling addInputSource() with the deleteWhenRemoved
        flag set will be deleted by this method.
    */
    void removeAllInputs();

    //==============================================================================
    /** Implementation of the AudioSource method.
        This will call prepareToPlay() on all its input sources.
    */
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;

    /** Implementation of the AudioSource method.
        This will call releaseResources() on all its input sources.
    */
    void releaseResources() override;

    /** Implementation of the AudioSource method. */
    void getNextAudioBlock (const AudioSourceChannelInfo&) override;


private:
    //==============================================================================
    Array<AudioSource*> inputs;
    BigInteger inputsToDelete;
    CriticalSection lock;
    AudioSampleBuffer tempBuffer;
    double currentSampleRate;
    int bufferSizeExpected;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MixerAudioSource)
};


#endif   // JUCE_MIXERAUDIOSOURCE_H_INCLUDED

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

    /** A subclass should call this to set up the audio */
    void initialise (int numInputChannels, int numOutputChannels);

    AudioDeviceManager deviceManager;

private:
    //==============================================================================
    AudioSourcePlayer audioSourcePlayer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AnimatedAppComponent)
};


#endif   // JUCE_AUDIOAPPCOMPONENT_H_INCLUDED

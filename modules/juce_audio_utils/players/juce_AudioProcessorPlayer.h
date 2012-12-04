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

#ifndef __JUCE_AUDIOPROCESSORPLAYER_JUCEHEADER__
#define __JUCE_AUDIOPROCESSORPLAYER_JUCEHEADER__

#include "../../juce_audio_processors/processors/juce_AudioProcessor.h"


//==============================================================================
/**
    An AudioIODeviceCallback object which streams audio through an AudioProcessor.

    To use one of these, just make it the callback used by your AudioIODevice, and
    give it a processor to use by calling setProcessor().

    It's also a MidiInputCallback, so you can connect it to both an audio and midi
    input to send both streams through the processor.

    @see AudioProcessor, AudioProcessorGraph
*/
class JUCE_API  AudioProcessorPlayer    : public AudioIODeviceCallback,
                                          public MidiInputCallback
{
public:
    //==============================================================================
    AudioProcessorPlayer();

    /** Destructor. */
    virtual ~AudioProcessorPlayer();

    //==============================================================================
    /** Sets the processor that should be played.

        The processor that is passed in will not be deleted or owned by this object.
        To stop anything playing, pass in 0 to this method.
    */
    void setProcessor (AudioProcessor* processorToPlay);

    /** Returns the current audio processor that is being played.
    */
    AudioProcessor* getCurrentProcessor() const                     { return processor; }

    /** Returns a midi message collector that you can pass midi messages to if you
        want them to be injected into the midi stream that is being sent to the
        processor.
    */
    MidiMessageCollector& getMidiMessageCollector()                 { return messageCollector; }

    //==============================================================================
    /** @internal */
    void audioDeviceIOCallback (const float** inputChannelData,
                                int totalNumInputChannels,
                                float** outputChannelData,
                                int totalNumOutputChannels,
                                int numSamples);
    /** @internal */
    void audioDeviceAboutToStart (AudioIODevice* device);
    /** @internal */
    void audioDeviceStopped();
    /** @internal */
    void handleIncomingMidiMessage (MidiInput* source, const MidiMessage& message);
    /** @internal */
    void prepareToPlay (double sampleRate, int blockSize, int numChansIn, int numChansOut);

private:
    //==============================================================================
    AudioProcessor* processor;
    CriticalSection lock;
    double sampleRate;
    int blockSize;
    bool isPrepared;

    int numInputChans, numOutputChans;
    HeapBlock<float*> channels;
    AudioSampleBuffer tempBuffer;

    MidiBuffer incomingMidi;
    MidiMessageCollector messageCollector;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioProcessorPlayer)
};


#endif   // __JUCE_AUDIOPROCESSORPLAYER_JUCEHEADER__

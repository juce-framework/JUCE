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

#ifndef JUCE_AUDIOPROCESSORPLAYER_H_INCLUDED
#define JUCE_AUDIOPROCESSORPLAYER_H_INCLUDED

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
    AudioProcessorPlayer(bool doDoublePrecisionProcessing = false);

    /** Destructor. */
    virtual ~AudioProcessorPlayer();

    //==============================================================================
    /** Sets the processor that should be played.

        The processor that is passed in will not be deleted or owned by this object.
        To stop anything playing, pass a nullptr to this method.
    */
    void setProcessor (AudioProcessor* processorToPlay);

    /** Returns the current audio processor that is being played. */
    AudioProcessor* getCurrentProcessor() const noexcept            { return processor; }

    /** Returns a midi message collector that you can pass midi messages to if you
        want them to be injected into the midi stream that is being sent to the
        processor.
    */
    MidiMessageCollector& getMidiMessageCollector() noexcept        { return messageCollector; }

    /** Switch between double and single floating point precisions processing.
        The audio IO callbacks will still operate in single floating point
        precision, however, all internal processing including the
        AudioProcessor will be processed in double floating point precision if
        the AudioProcessor supports it (see
        AudioProcessor::supportsDoublePrecisionProcessing()).
        Otherwise, the processing will remain single precision irrespective of
        the parameter doublePrecision. */
    void setDoublePrecisionProcessing (bool doublePrecision);

    /** Returns true if this player processes internally processes the samples with
        double floating point precision. */
    inline bool getDoublePrecisionProcessing() { return isDoublePrecision; }

    //==============================================================================
    /** @internal */
    void audioDeviceIOCallback (const float**, int, float**, int, int) override;
    /** @internal */
    void audioDeviceAboutToStart (AudioIODevice*) override;
    /** @internal */
    void audioDeviceStopped() override;
    /** @internal */
    void handleIncomingMidiMessage (MidiInput*, const MidiMessage&) override;

private:
    //==============================================================================
    AudioProcessor* processor;
    CriticalSection lock;
    double sampleRate;
    int blockSize;
    bool isPrepared, isDoublePrecision;

    int numInputChans, numOutputChans;
    HeapBlock<float*> channels;
    AudioBuffer<float> tempBuffer;
    AudioBuffer<double> conversionBuffer;

    MidiBuffer incomingMidi;
    MidiMessageCollector messageCollector;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioProcessorPlayer)
};


#endif   // JUCE_AUDIOPROCESSORPLAYER_H_INCLUDED

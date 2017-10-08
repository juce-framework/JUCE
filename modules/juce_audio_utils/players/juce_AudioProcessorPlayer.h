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
    AudioProcessorPlayer (bool doDoublePrecisionProcessing = false);

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
    AudioProcessor* processor = nullptr;
    CriticalSection lock;
    double sampleRate = 0;
    int blockSize = 0;
    bool isPrepared = false, isDoublePrecision = false;

    int numInputChans = 0, numOutputChans = 0;
    HeapBlock<float*> channels;
    AudioBuffer<float> tempBuffer;
    AudioBuffer<double> conversionBuffer;

    MidiBuffer incomingMidi;
    MidiMessageCollector messageCollector;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioProcessorPlayer)
};

} // namespace juce

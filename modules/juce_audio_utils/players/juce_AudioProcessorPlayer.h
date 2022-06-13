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
    An AudioIODeviceCallback object which streams audio through an AudioProcessor.

    To use one of these, just make it the callback used by your AudioIODevice, and
    give it a processor to use by calling setProcessor().

    It's also a MidiInputCallback, so you can connect it to both an audio and midi
    input to send both streams through the processor. To set a MidiOutput for the processor,
    use the setMidiOutput() method.

    @see AudioProcessor, AudioProcessorGraph

    @tags{Audio}
*/
class JUCE_API  AudioProcessorPlayer    : public AudioIODeviceCallback,
                                          public MidiInputCallback
{
public:
    //==============================================================================
    AudioProcessorPlayer (bool doDoublePrecisionProcessing = false);

    /** Destructor. */
    ~AudioProcessorPlayer() override;

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

    /** Sets the MIDI output that should be used, if required.

        The MIDI output will not be deleted or owned by this object. If the MIDI output is
        deleted, pass a nullptr to this method.
    */
    void setMidiOutput (MidiOutput* midiOutputToUse);

    /** Switch between double and single floating point precisions processing.

        The audio IO callbacks will still operate in single floating point precision,
        however, all internal processing including the AudioProcessor will be processed in
        double floating point precision if the AudioProcessor supports it (see
        AudioProcessor::supportsDoublePrecisionProcessing()). Otherwise, the processing will
        remain single precision irrespective of the parameter doublePrecision.
    */
    void setDoublePrecisionProcessing (bool doublePrecision);

    /** Returns true if this player processes internally processes the samples with
        double floating point precision.
    */
    inline bool getDoublePrecisionProcessing() { return isDoublePrecision; }

    //==============================================================================
    /** @internal */
    void audioDeviceIOCallbackWithContext (const float**, int, float**, int, int, const AudioIODeviceCallbackContext&) override;
    /** @internal */
    void audioDeviceAboutToStart (AudioIODevice*) override;
    /** @internal */
    void audioDeviceStopped() override;
    /** @internal */
    void handleIncomingMidiMessage (MidiInput*, const MidiMessage&) override;

private:
    struct NumChannels
    {
        NumChannels() = default;
        NumChannels (int numIns, int numOuts) : ins (numIns), outs (numOuts) {}

        explicit NumChannels (const AudioProcessor::BusesLayout& layout)
            : ins (layout.getNumChannels (true, 0)), outs (layout.getNumChannels (false, 0)) {}

        AudioProcessor::BusesLayout toLayout() const
        {
            return { { AudioChannelSet::canonicalChannelSet (ins) },
                     { AudioChannelSet::canonicalChannelSet (outs) } };
        }

        int ins = 0, outs = 0;
    };

    //==============================================================================
    NumChannels findMostSuitableLayout (const AudioProcessor&) const;
    void resizeChannels();

    //==============================================================================
    AudioProcessor* processor = nullptr;
    CriticalSection lock;
    double sampleRate = 0;
    int blockSize = 0;
    bool isPrepared = false, isDoublePrecision = false;

    NumChannels deviceChannels, defaultProcessorChannels, actualProcessorChannels;
    std::vector<float*> channels;
    AudioBuffer<float> tempBuffer;
    AudioBuffer<double> conversionBuffer;

    MidiBuffer incomingMidi;
    MidiMessageCollector messageCollector;
    MidiOutput* midiOutput = nullptr;
    uint64_t sampleCount = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioProcessorPlayer)
};

} // namespace juce

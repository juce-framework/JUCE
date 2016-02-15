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

#include "../JuceLibraryCode/JuceHeader.h"
#include "../../GenericEditor.h"

//==============================================================================
/**
 */
class MultiOutSynth  : public AudioProcessor
{
public:
    enum
    {
        maxMidiChannel = 16,
        maxNumberOfVoices = 5
    };

    //==============================================================================
    MultiOutSynth()
    {
        // The base class constructor will already add a main stereo output bus
        // If you want to add your own main channel then simply call clear the
        // output buses (busArrangement.outputBuses.clear()) and then add your own

        // Add additional output buses but disable these by default
        for (int busNr = 1; busNr < maxMidiChannel; ++busNr)
            busArrangement.outputBuses.add (AudioProcessorBus (String ("Output #") += String (busNr + 1), AudioChannelSet::disabled()));


        // initialize other stuff (not related to buses)
        formatManager.registerBasicFormats();

        for (int midiChannel = 0; midiChannel < maxMidiChannel; ++midiChannel)
        {
            synth.add (new Synthesiser());

            for (int i = 0; i < maxNumberOfVoices; ++i)
                synth[midiChannel]->addVoice (new SamplerVoice());
        }

        loadNewSample (BinaryData::singing_ogg, BinaryData::singing_oggSize);
    }

    ~MultiOutSynth() {}

    //==============================================================================
    bool setPreferredBusArrangement (bool isInputBus, int busIndex,
                                     const AudioChannelSet& preferred) override
    {
        const int numChannels = preferred.size();
        const bool isMainBus = (busIndex == 0);

        // do not allow disabling the main output bus
        if (isMainBus && preferred.isDisabled()) return false;

        // only support mono or stereo (or disabling) buses
        if (numChannels > 2) return false;


        // pass the call on to the base class
        return AudioProcessor::setPreferredBusArrangement (isInputBus, busIndex, preferred);
    }

    //==============================================================================
    void prepareToPlay (double newSampleRate, int samplesPerBlock) override
    {
        ignoreUnused (samplesPerBlock);

        for (int midiChannel = 0; midiChannel < maxMidiChannel; ++midiChannel)
            synth[midiChannel]->setCurrentPlaybackSampleRate (newSampleRate);
    }

    void releaseResources() override {}

    void processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiBuffer) override
    {
        buffer.clear();

        for (int busNr = 0; busNr < maxMidiChannel; ++busNr)
        {
            MidiBuffer midiChannelBuffer = filterMidiMessagesForChannel (midiBuffer, busNr + 1);
            AudioSampleBuffer audioBusBuffer = busArrangement.getBusBuffer (buffer, false, busNr);

            synth [busNr]->renderNextBlock (audioBusBuffer, midiChannelBuffer, 0, audioBusBuffer.getNumSamples());
        }
    }

    //==============================================================================
    AudioProcessorEditor* createEditor() override { return new GenericEditor (*this); }
    bool hasEditor() const override               { return true;   }

    //==============================================================================
    const String getName() const override               { return "Gain PlugIn"; }
    bool acceptsMidi() const override                   { return false; }
    bool producesMidi() const override                  { return false; }
    double getTailLengthSeconds() const override        { return 0; }
    int getNumPrograms() override                          { return 1; }
    int getCurrentProgram() override                       { return 0; }
    void setCurrentProgram (int) override                  {}
    const String getProgramName (int) override             { return String(); }
    void changeProgramName (int , const String& ) override { }

    //==============================================================================
    void getStateInformation (MemoryBlock&) override {}
    void setStateInformation (const void*, int) override {}

private:
    //==============================================================================
    static MidiBuffer filterMidiMessagesForChannel (const MidiBuffer& input, int channel)
    {
        MidiMessage msg;
        int samplePosition;
        MidiBuffer output;

        for (MidiBuffer::Iterator it (input); it.getNextEvent (msg, samplePosition);)
            if (msg.getChannel() == channel) output.addEvent (msg, samplePosition);

        return output;
    }

    void loadNewSample (const void* data, int dataSize)
    {
        MemoryInputStream* soundBuffer = new MemoryInputStream (data, static_cast<std::size_t> (dataSize), false);
        ScopedPointer<AudioFormatReader> formatReader (formatManager.findFormatForFileExtension ("ogg")->createReaderFor (soundBuffer, true));

        BigInteger midiNotes;
        midiNotes.setRange (0, 126, true);
        SynthesiserSound::Ptr newSound = new SamplerSound ("Voice", *formatReader, midiNotes, 0x40, 0.0, 0.0, 10.0);

        for (int channel = 0; channel < maxMidiChannel; ++channel)
            synth[channel]->removeSound (0);

        sound = newSound;

        for (int channel = 0; channel < maxMidiChannel; ++channel)
            synth[channel]->addSound (sound);
    }

    //==============================================================================
    AudioFormatManager formatManager;
    OwnedArray<Synthesiser> synth;
    SynthesiserSound::Ptr sound;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MultiOutSynth)
};

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MultiOutSynth();
}

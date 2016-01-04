/*
 ==============================================================================

 Arpeggiator.cpp
 Created: 23 Nov 2015 3:08:33pm
 Author:  Fabian Renn

 ==============================================================================
 */

#include "../JuceLibraryCode/JuceHeader.h"
#include "../../GenericEditor.h"

//==============================================================================
/**
 */
class Arpeggiator  : public AudioProcessor
{
public:

    //==============================================================================
    Arpeggiator()
    {
        addParameter (speed = new AudioParameterFloat ("speed", "Arpeggiator Speed", 0.0, 1.0, 0.5));
    }

    ~Arpeggiator() {}

    //==============================================================================
    bool setPreferredBusArrangement (bool isInputBus, int busIndex,
                                     const AudioChannelSet& preferred) override
    {
        ignoreUnused (isInputBus, busIndex, preferred);

        // we don't support any audio buses
        return false;
    }

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override
    {
        ignoreUnused (samplesPerBlock);

        notes.clear();
        currentNote = 0;
        lastNoteValue = -1;
        time = 0.0;
        rate = static_cast<float> (sampleRate);
    }

    void releaseResources() override {}

    void processBlock (AudioSampleBuffer& buffer, MidiBuffer& midi) override
    {
        // the audio buffer in a midi effect will have zero channels!
        jassert (buffer.getNumChannels() == 0);

        // however we use the buffer to get timing information
        const int numSamples = buffer.getNumSamples();

        // get note duration
        const int noteDuration = static_cast<int> (std::ceil (rate * 0.25f * (0.1f + (1.0f - (*speed)))));

        MidiMessage msg;
        int ignore;

        for (MidiBuffer::Iterator it (midi); it.getNextEvent (msg, ignore);)
        {
            if      (msg.isNoteOn())  notes.add (msg.getNoteNumber());
            else if (msg.isNoteOff()) notes.removeValue (msg.getNoteNumber());
        }

        midi.clear();

        if ((time + numSamples) >= noteDuration)
        {
            const int offset = jmax (0, jmin ((int) (noteDuration - time), numSamples - 1));

            if (lastNoteValue > 0)
            {
                midi.addEvent (MidiMessage::noteOff (1, lastNoteValue), offset);
                lastNoteValue = -1;
            }

            if (notes.size() > 0)
            {
                currentNote = (currentNote + 1) % notes.size();
                lastNoteValue = notes[currentNote];
                midi.addEvent (MidiMessage::noteOn  (1, lastNoteValue, (uint8) 127), offset);
            }

        }

        time = (time + numSamples) % noteDuration;
    }

    //==============================================================================
    AudioProcessorEditor* createEditor() override { return new GenericEditor (*this); }
    bool hasEditor() const override               { return true;   }

    //==============================================================================
    const String getName() const override               { return "Arpeggiator"; }

    bool acceptsMidi() const override                   { return false; }
    bool producesMidi() const override                  { return false; }
    bool silenceInProducesSilenceOut() const override   { return true; }
    double getTailLengthSeconds() const override        { return 0; }

    //==============================================================================
    int getNumPrograms() override                          { return 1; }
    int getCurrentProgram() override                       { return 0; }
    void setCurrentProgram (int) override                  {}
    const String getProgramName (int) override             { return String(); }
    void changeProgramName (int , const String& ) override { }

    //==============================================================================
    void getStateInformation (MemoryBlock& destData) override
    {
        MemoryOutputStream (destData, true).writeFloat (*speed);
    }

    void setStateInformation (const void* data, int sizeInBytes) override
    {
        speed->setValueNotifyingHost (MemoryInputStream (data, static_cast<size_t> (sizeInBytes), false).readFloat());
    }

    //==============================================================================


private:
    //==============================================================================
    AudioParameterFloat* speed;
    int currentNote, lastNoteValue;
    int time;
    float rate;
    SortedSet<int> notes;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Arpeggiator)
};

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Arpeggiator();
}

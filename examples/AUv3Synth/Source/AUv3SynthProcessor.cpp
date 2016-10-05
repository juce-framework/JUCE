/*
  ==============================================================================

    Copyright (c) 2015 - ROLI Ltd.

  ==============================================================================
*/
#include "../JuceLibraryCode/JuceHeader.h"
#include "AUv3SynthEditor.h"

class AUv3SynthProcessor : public AudioProcessor
{
public:
    AUv3SynthProcessor ()
        : AudioProcessor (BusesProperties().withOutput ("Output", AudioChannelSet::stereo(), true)),
          currentRecording (1, 1), currentProgram (0)
    {
        // initialize parameters
        addParameter (isRecordingParam = new AudioParameterBool ("isRecording", "Is Recording", false));
        addParameter (roomSizeParam = new AudioParameterFloat ("roomSize", "Room Size", 0.0f, 1.0f, 0.5f));

        formatManager.registerBasicFormats();

        for (int i = 0; i < maxNumVoices; ++i)
            synth.addVoice (new SamplerVoice());

        loadNewSample (BinaryData::singing_ogg, BinaryData::singing_oggSize, "ogg");
    }

    //==============================================================================
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override
    {
        return (layouts.getMainInputChannels() == 2);
    }

    void prepareToPlay (double sampleRate, int estimatedMaxSizeOfBuffer) override
    {
        ignoreUnused (estimatedMaxSizeOfBuffer);

        lastSampleRate = sampleRate;

        currentRecording.setSize (1, static_cast<int> (std::ceil (kMaxDurationOfRecording * lastSampleRate)));
        samplesRecorded = 0;

        synth.setCurrentPlaybackSampleRate (lastSampleRate);
        reverb.setSampleRate (lastSampleRate);
    }

    void processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages) override
    {
        Reverb::Parameters reverbParameters;
        reverbParameters.roomSize = roomSizeParam->get();

        reverb.setParameters (reverbParameters);
        synth.renderNextBlock (buffer, midiMessages, 0, buffer.getNumSamples());

        if (getMainBusNumOutputChannels() == 1)
            reverb.processMono (buffer.getWritePointer (0), buffer.getNumSamples());
        else if (getMainBusNumOutputChannels() == 2)
            reverb.processStereo (buffer.getWritePointer (0), buffer.getWritePointer (1), buffer.getNumSamples());
    }

    //==============================================================================
    void releaseResources() override                                            { currentRecording.setSize (1, 1); }

    //==============================================================================
    bool acceptsMidi() const override                                           { return true; }
    bool producesMidi() const override                                          { return false; }
    bool silenceInProducesSilenceOut() const override                           { return false; }
    double getTailLengthSeconds() const override                                { return 0.0; }

    //==============================================================================
    AudioProcessorEditor* createEditor() override                               { return new AUv3SynthEditor (*this); }
    bool hasEditor() const override                                             { return true; }

    //==============================================================================
    const String getName() const override                                       { return "AUv3 Synth"; }
    int getNumPrograms() override                                               { return 4; }
    int getCurrentProgram() override                                            { return currentProgram; }
    void setCurrentProgram (int index) override                             { currentProgram = index; }
    const String getProgramName (int index) override
    {
        switch (index)
        {
            case 0:
                return "Piano";
            case 1:
                return "Singing";
            case 2:
                return "Pinched Balloon";
            case 3:
                return "Gazeebo";
        }

        return "<Unknown>";
    }

    //==============================================================================
    void changeProgramName (int /*index*/, const String& /*name*/) override     {}

    //==============================================================================
    void getStateInformation (MemoryBlock& destData) override
    {
        MemoryOutputStream stream (destData, true);

        stream.writeFloat (*isRecordingParam);
        stream.writeFloat (*roomSizeParam);
    }

    void setStateInformation (const void* data, int sizeInBytes) override
    {
        MemoryInputStream stream (data, static_cast<size_t> (sizeInBytes), false);

        isRecordingParam->setValueNotifyingHost (stream.readFloat());
        roomSizeParam->setValueNotifyingHost (stream.readFloat());

    }
private:
    //==============================================================================
    void loadNewSample (const void* data, int dataSize, const char* format)
    {
        MemoryInputStream* soundBuffer = new MemoryInputStream (data, static_cast<std::size_t> (dataSize), false);
        ScopedPointer<AudioFormatReader> formatReader (formatManager.findFormatForFileExtension (format)->createReaderFor (soundBuffer, true));

        BigInteger midiNotes;
        midiNotes.setRange (0, 126, true);
        SynthesiserSound::Ptr newSound = new SamplerSound ("Voice", *formatReader, midiNotes, 0x40, 0.0, 0.0, 10.0);
        synth.removeSound (0);
        sound = newSound;
        synth.addSound (sound);
    }

    void swapSamples()
    {
        MemoryBlock mb;
        MemoryOutputStream* stream = new MemoryOutputStream (mb, true);

        {
            ScopedPointer<AudioFormatWriter> writer (formatManager.findFormatForFileExtension ("wav")->createWriterFor (stream, lastSampleRate, 1, 16,
                                                                                                                        StringPairArray(), 0));
            writer->writeFromAudioSampleBuffer (currentRecording, 0, currentRecording.getNumSamples());
            writer->flush();
            stream->flush();
        }

        loadNewSample (mb.getData(), static_cast<int> (mb.getSize()), "wav");
    }

    //==============================================================================
    static constexpr int maxNumVoices = 5;
    static constexpr double kMaxDurationOfRecording = 1.0;

    //==============================================================================
    AudioFormatManager formatManager;

    int samplesRecorded;
    double lastSampleRate;
    AudioBuffer<float> currentRecording;

    Reverb reverb;
    Synthesiser synth;
    SynthesiserSound::Ptr sound;

    AudioParameterBool* isRecordingParam;
    AudioParameterFloat* roomSizeParam;

    int currentProgram;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AUv3SynthProcessor)
};

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AUv3SynthProcessor();
}

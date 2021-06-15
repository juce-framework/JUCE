/*
  ==============================================================================

   This file is part of the JUCE examples.
   Copyright (c) 2020 - Raw Material Software Limited

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES,
   WHETHER EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR
   PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

/*******************************************************************************
 The block below describes the properties of this PIP. A PIP is a short snippet
 of code that can be read by the Projucer and used to generate a JUCE project.

 BEGIN_JUCE_PIP_METADATA

 name:                  MultiOutSynthPlugin
 version:               1.0.0
 vendor:                JUCE
 website:               http://juce.com
 description:           Multi-out synthesiser audio plugin.

 dependencies:          juce_audio_basics, juce_audio_devices, juce_audio_formats,
                        juce_audio_plugin_client, juce_audio_processors,
                        juce_audio_utils, juce_core, juce_data_structures,
                        juce_events, juce_graphics, juce_gui_basics, juce_gui_extra
 exporters:             xcode_mac, vs2019

 moduleFlags:           JUCE_STRICT_REFCOUNTEDPOINTER=1

 type:                  AudioProcessor
 mainClass:             MultiOutSynth

 useLocalCopy:          1

 pluginCharacteristics: pluginIsSynth, pluginWantsMidiIn

 END_JUCE_PIP_METADATA

*******************************************************************************/

#pragma once

#include "../Assets/DemoUtilities.h"

//==============================================================================
class MultiOutSynth  : public AudioProcessor
{
public:
    enum
    {
        maxMidiChannel    = 16,
        maxNumberOfVoices = 5
    };

    //==============================================================================
    MultiOutSynth()
        : AudioProcessor (BusesProperties()
                          .withOutput ("Output #1",  AudioChannelSet::stereo(), true)
                          .withOutput ("Output #2",  AudioChannelSet::stereo(), false)
                          .withOutput ("Output #3",  AudioChannelSet::stereo(), false)
                          .withOutput ("Output #4",  AudioChannelSet::stereo(), false)
                          .withOutput ("Output #5",  AudioChannelSet::stereo(), false)
                          .withOutput ("Output #6",  AudioChannelSet::stereo(), false)
                          .withOutput ("Output #7",  AudioChannelSet::stereo(), false)
                          .withOutput ("Output #8",  AudioChannelSet::stereo(), false)
                          .withOutput ("Output #9",  AudioChannelSet::stereo(), false)
                          .withOutput ("Output #10", AudioChannelSet::stereo(), false)
                          .withOutput ("Output #11", AudioChannelSet::stereo(), false)
                          .withOutput ("Output #12", AudioChannelSet::stereo(), false)
                          .withOutput ("Output #13", AudioChannelSet::stereo(), false)
                          .withOutput ("Output #14", AudioChannelSet::stereo(), false)
                          .withOutput ("Output #15", AudioChannelSet::stereo(), false)
                          .withOutput ("Output #16", AudioChannelSet::stereo(), false))
    {
        // initialize other stuff (not related to buses)
        formatManager.registerBasicFormats();

        for (int midiChannel = 0; midiChannel < maxMidiChannel; ++midiChannel)
        {
            synth.add (new Synthesiser());

            for (int i = 0; i < maxNumberOfVoices; ++i)
                synth[midiChannel]->addVoice (new SamplerVoice());
        }

        loadNewSample (createAssetInputStream ("singing.ogg"), "ogg");
    }

    //==============================================================================
    bool canAddBus    (bool isInput) const override   { return ! isInput; }
    bool canRemoveBus (bool isInput) const override   { return ! isInput; }

    //==============================================================================
    void prepareToPlay (double newSampleRate, int samplesPerBlock) override
    {
        ignoreUnused (samplesPerBlock);

        for (auto* s : synth)
            s->setCurrentPlaybackSampleRate (newSampleRate);
    }

    void releaseResources() override {}

    void processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiBuffer) override
    {
        auto busCount = getBusCount (false);

        for (auto busNr = 0; busNr < busCount; ++busNr)
        {
            if (synth.size() <= busNr)
                continue;

            auto midiChannelBuffer = filterMidiMessagesForChannel (midiBuffer, busNr + 1);
            auto audioBusBuffer = getBusBuffer (buffer, false, busNr);

            synth [busNr]->renderNextBlock (audioBusBuffer, midiChannelBuffer, 0, audioBusBuffer.getNumSamples());
        }
    }

    using AudioProcessor::processBlock;

    //==============================================================================
    AudioProcessorEditor* createEditor() override          { return new GenericAudioProcessorEditor (*this); }
    bool hasEditor() const override                        { return true; }

    //==============================================================================
    const String getName() const override                  { return "Multi Out Synth PlugIn"; }
    bool acceptsMidi() const override                      { return false; }
    bool producesMidi() const override                     { return false; }
    double getTailLengthSeconds() const override           { return 0; }
    int getNumPrograms() override                          { return 1; }
    int getCurrentProgram() override                       { return 0; }
    void setCurrentProgram (int) override                  {}
    const String getProgramName (int) override             { return "None"; }
    void changeProgramName (int, const String&) override   {}

    bool isBusesLayoutSupported (const BusesLayout& layout) const override
    {
        for (const auto& bus : layout.outputBuses)
            if (bus != AudioChannelSet::stereo())
                return false;

        return layout.inputBuses.isEmpty() && 1 <= layout.outputBuses.size();
    }

    //==============================================================================
    void getStateInformation (MemoryBlock&) override {}
    void setStateInformation (const void*, int) override {}

private:
    //==============================================================================
    static MidiBuffer filterMidiMessagesForChannel (const MidiBuffer& input, int channel)
    {
        MidiBuffer output;

        for (const auto metadata : input)
        {
            const auto message = metadata.getMessage();

            if (message.getChannel() == channel)
                output.addEvent (message, metadata.samplePosition);
        }

        return output;
    }

    void loadNewSample (std::unique_ptr<InputStream> soundBuffer, const char* format)
    {
        std::unique_ptr<AudioFormatReader> formatReader (formatManager.findFormatForFileExtension (format)->createReaderFor (soundBuffer.release(), true));

        BigInteger midiNotes;
        midiNotes.setRange (0, 126, true);
        SynthesiserSound::Ptr newSound = new SamplerSound ("Voice", *formatReader, midiNotes, 0x40, 0.0, 0.0, 10.0);

        for (auto* s : synth)
            s->removeSound (0);

        sound = newSound;

        for (auto* s : synth)
            s->addSound (sound);
    }

    //==============================================================================
    AudioFormatManager formatManager;
    OwnedArray<Synthesiser> synth;
    SynthesiserSound::Ptr sound;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MultiOutSynth)
};

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


#pragma once


class MainComponent : public Component,
                      private AudioIODeviceCallback,
                      private MidiInputCallback,
                      private MPESetupComponent::Listener
{
public:
    //==============================================================================
    MainComponent()
        : audioSetupComp (audioDeviceManager, 0, 0, 0, 256, true, true, true, false),
          zoneLayoutComp (colourPicker),
          visualiserComp (colourPicker)
    {
        setSize (880, 720);
        audioDeviceManager.initialise (0, 2, 0, true, String(), 0);
        audioDeviceManager.addMidiInputCallback (String(), this);
        audioDeviceManager.addAudioCallback (this);

        addAndMakeVisible (audioSetupComp);
        addAndMakeVisible (MPESetupComp);
        addAndMakeVisible (zoneLayoutComp);
        addAndMakeVisible (visualiserViewport);

        visualiserViewport.setScrollBarsShown (false, true);
        visualiserViewport.setViewedComponent (&visualiserComp, false);
        visualiserViewport.setViewPositionProportionately (0.5, 0.0);

        MPESetupComp.addListener (&zoneLayoutComp);
        MPESetupComp.addListener (this);
        visualiserInstrument.addListener (&visualiserComp);

        synth.setVoiceStealingEnabled (false);
        for (int i = 0; i < 15; ++i)
            synth.addVoice (new MPEDemoSynthVoice);
    }

    ~MainComponent()
    {
        audioDeviceManager.removeMidiInputCallback (String(), this);
    }

    //==============================================================================
    void resized() override
    {
        const int visualiserCompWidth = 2800;
        const int visualiserCompHeight = 300;
        const int zoneLayoutCompHeight = 60;
        const float audioSetupCompRelativeWidth = 0.55f;

        Rectangle<int> r (getLocalBounds());

        visualiserViewport.setBounds (r.removeFromBottom (visualiserCompHeight));
        visualiserComp.setBounds (Rectangle<int> (visualiserCompWidth,
                                                  visualiserViewport.getHeight() - visualiserViewport.getScrollBarThickness()));

        zoneLayoutComp.setBounds (r.removeFromBottom (zoneLayoutCompHeight));
        audioSetupComp.setBounds (r.removeFromLeft (proportionOfWidth (audioSetupCompRelativeWidth)));
        MPESetupComp.setBounds (r);
    }

    //==============================================================================
    void audioDeviceIOCallback (const float** /*inputChannelData*/, int /*numInputChannels*/,
                                float** outputChannelData, int numOutputChannels,
                                int numSamples) override
    {
        AudioBuffer<float> buffer (outputChannelData, numOutputChannels, numSamples);
        buffer.clear();

        MidiBuffer incomingMidi;
        midiCollector.removeNextBlockOfMessages (incomingMidi, numSamples);
        synth.renderNextBlock (buffer, incomingMidi, 0, numSamples);
    }

    void audioDeviceAboutToStart (AudioIODevice* device) override
    {
        const double sampleRate = device->getCurrentSampleRate();
        midiCollector.reset (sampleRate);
        synth.setCurrentPlaybackSampleRate (sampleRate);
    }

    void audioDeviceStopped() override
    {
    }

private:
    //==============================================================================
    void handleIncomingMidiMessage (MidiInput* /*source*/,
                                    const MidiMessage& message) override
    {
        visualiserInstrument.processNextMidiEvent (message);
        midiCollector.addMessageToQueue (message);
    }

    //==============================================================================
    void zoneAdded (MPEZone newZone) override
    {
        MidiOutput* midiOutput = audioDeviceManager.getDefaultMidiOutput();
        if (midiOutput != nullptr)
            midiOutput->sendBlockOfMessagesNow (MPEMessages::addZone (newZone));

        zoneLayout.addZone (newZone);
        visualiserInstrument.setZoneLayout (zoneLayout);
        synth.setZoneLayout (zoneLayout);
        colourPicker.setZoneLayout (zoneLayout);
    }

    void allZonesCleared() override
    {
        MidiOutput* midiOutput = audioDeviceManager.getDefaultMidiOutput();
        if (midiOutput != nullptr)
            midiOutput->sendBlockOfMessagesNow (MPEMessages::clearAllZones());

        zoneLayout.clearAllZones();
        visualiserInstrument.setZoneLayout (zoneLayout);
        synth.setZoneLayout (zoneLayout);
        colourPicker.setZoneLayout (zoneLayout);
    }

    void legacyModeChanged (bool legacyModeShouldBeEnabled, int pitchbendRange, Range<int> channelRange) override
    {
        colourPicker.setLegacyModeEnabled (legacyModeShouldBeEnabled);

        if (legacyModeShouldBeEnabled)
        {
            synth.enableLegacyMode (pitchbendRange, channelRange);
            visualiserInstrument.enableLegacyMode (pitchbendRange, channelRange);
        }
        else
        {
            synth.setZoneLayout (zoneLayout);
            visualiserInstrument.setZoneLayout (zoneLayout);
        }
    }

    void voiceStealingEnabledChanged (bool voiceStealingEnabled) override
    {
        synth.setVoiceStealingEnabled (voiceStealingEnabled);
    }

    void numberOfVoicesChanged (int numberOfVoices) override
    {
        if (numberOfVoices < synth.getNumVoices())
            synth.reduceNumVoices (numberOfVoices);
        else
            while (synth.getNumVoices() < numberOfVoices)
                synth.addVoice (new MPEDemoSynthVoice);
    }

    //==============================================================================
    AudioDeviceManager audioDeviceManager;

    MPEZoneLayout zoneLayout;
    ZoneColourPicker colourPicker;

    AudioDeviceSelectorComponent audioSetupComp;
    MPESetupComponent MPESetupComp;
    ZoneLayoutComponent zoneLayoutComp;

    Visualiser visualiserComp;
    Viewport visualiserViewport;
    MPEInstrument visualiserInstrument;

    MPESynthesiser synth;
    MidiMessageCollector midiCollector;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};

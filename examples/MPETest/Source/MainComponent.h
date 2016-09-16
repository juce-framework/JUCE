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


#ifndef MAINCOMPONENT_H_INCLUDED
#define MAINCOMPONENT_H_INCLUDED


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
        setLookAndFeel (&lookAndFeel);
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
    LookAndFeel_V3 lookAndFeel;
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


#endif  // MAINCOMPONENT_H_INCLUDED

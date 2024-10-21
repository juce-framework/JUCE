/*
  ==============================================================================

   This file is part of the JUCE framework examples.
   Copyright (c) Raw Material Software Limited

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   to use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
   REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
   AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
   INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
   LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
   OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
   PERFORMANCE OF THIS SOFTWARE.

  ==============================================================================
*/

/*******************************************************************************
 The block below describes the properties of this PIP. A PIP is a short snippet
 of code that can be read by the Projucer and used to generate a JUCE project.

 BEGIN_JUCE_PIP_METADATA

 name:             MPEDemo
 version:          1.0.0
 vendor:           JUCE
 website:          http://juce.com
 description:      Simple MPE synthesiser application.

 dependencies:     juce_audio_basics, juce_audio_devices, juce_audio_formats,
                   juce_audio_processors, juce_audio_utils, juce_core,
                   juce_data_structures, juce_events, juce_graphics,
                   juce_gui_basics, juce_gui_extra
 exporters:        xcode_mac, vs2022, linux_make, androidstudio, xcode_iphone

 moduleFlags:      JUCE_STRICT_REFCOUNTEDPOINTER=1

 type:             Component
 mainClass:        MPEDemo

 useLocalCopy:     1

 END_JUCE_PIP_METADATA

*******************************************************************************/

#pragma once


//==============================================================================
class ZoneColourPicker
{
public:
    ZoneColourPicker() {}

    //==============================================================================
    Colour getColourForMidiChannel (int midiChannel) noexcept
    {
        if (legacyModeEnabled)
            return Colours::white;

        if (zoneLayout.getLowerZone().isUsingChannelAsMemberChannel (midiChannel))
            return getColourForZone (true);

        if (zoneLayout.getUpperZone().isUsingChannelAsMemberChannel (midiChannel))
            return getColourForZone (false);

        return Colours::transparentBlack;
    }

    //==============================================================================
    Colour getColourForZone (bool isLowerZone) const noexcept
    {
        if (legacyModeEnabled)
            return Colours::white;

        if (isLowerZone)
            return Colours::blue;

        return Colours::red;
    }

    //==============================================================================
    void setZoneLayout (MPEZoneLayout layout) noexcept          { zoneLayout = layout; }
    void setLegacyModeEnabled (bool shouldBeEnabled) noexcept   { legacyModeEnabled = shouldBeEnabled; }

private:
    //==============================================================================
    MPEZoneLayout zoneLayout;
    bool legacyModeEnabled = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ZoneColourPicker)
};

//==============================================================================
class MPESetupComponent final : public Component
{
public:
    //==============================================================================
    MPESetupComponent (MPEInstrument& instr)
        : instrument (instr)
    {
        addAndMakeVisible (isLowerZoneButton);
        isLowerZoneButton.setToggleState (true, NotificationType::dontSendNotification);

        initialiseComboBoxWithConsecutiveIntegers (memberChannels, memberChannelsLabel, 0, 16, defaultMemberChannels);
        initialiseComboBoxWithConsecutiveIntegers (masterPitchbendRange, masterPitchbendRangeLabel, 0, 96, defaultMasterPitchbendRange);
        initialiseComboBoxWithConsecutiveIntegers (notePitchbendRange, notePitchbendRangeLabel, 0, 96, defaultNotePitchbendRange);

        initialiseComboBoxWithConsecutiveIntegers (legacyStartChannel, legacyStartChannelLabel, 1, 16, 1, false);
        initialiseComboBoxWithConsecutiveIntegers (legacyEndChannel, legacyEndChannelLabel, 1, 16, 16, false);
        initialiseComboBoxWithConsecutiveIntegers (legacyPitchbendRange, legacyPitchbendRangeLabel, 0, 96, 2, false);

        addAndMakeVisible (setZoneButton);
        setZoneButton.onClick = [this] { setZoneButtonClicked(); };

        addAndMakeVisible (clearAllZonesButton);
        clearAllZonesButton.onClick = [this] { clearAllZonesButtonClicked(); };

        addAndMakeVisible (legacyModeEnabledToggle);
        legacyModeEnabledToggle.onClick = [this] { legacyModeEnabledToggleClicked(); };

        addAndMakeVisible (voiceStealingEnabledToggle);
        voiceStealingEnabledToggle.onClick = [this] { voiceStealingEnabledToggleClicked(); };

        initialiseComboBoxWithConsecutiveIntegers (numberOfVoices, numberOfVoicesLabel, 1, 20, 15);
    }

    //==============================================================================
    void resized() override
    {
        Rectangle<int> r (proportionOfWidth (0.65f), 15, proportionOfWidth (0.25f), 3000);
        auto h = 24;
        auto hspace = 6;
        auto hbigspace = 18;

        isLowerZoneButton.setBounds (r.removeFromTop (h));
        r.removeFromTop (hspace);
        memberChannels.setBounds (r.removeFromTop (h));
        r.removeFromTop (hspace);
        notePitchbendRange.setBounds (r.removeFromTop (h));
        r.removeFromTop (hspace);
        masterPitchbendRange.setBounds (r.removeFromTop (h));

        legacyStartChannel  .setBounds (isLowerZoneButton .getBounds());
        legacyEndChannel    .setBounds (memberChannels    .getBounds());
        legacyPitchbendRange.setBounds (notePitchbendRange.getBounds());

        r.removeFromTop (hbigspace);

        auto buttonLeft = proportionOfWidth (0.5f);

        setZoneButton.setBounds (r.removeFromTop (h).withLeft (buttonLeft));
        r.removeFromTop (hspace);
        clearAllZonesButton.setBounds (r.removeFromTop (h).withLeft (buttonLeft));

        r.removeFromTop (hbigspace);

        auto toggleLeft = proportionOfWidth (0.25f);

        legacyModeEnabledToggle.setBounds (r.removeFromTop (h).withLeft (toggleLeft));
        r.removeFromTop (hspace);
        voiceStealingEnabledToggle.setBounds (r.removeFromTop (h).withLeft (toggleLeft));
        r.removeFromTop (hspace);
        numberOfVoices.setBounds (r.removeFromTop (h));
    }

    //==============================================================================
    bool isVoiceStealingEnabled() const  { return voiceStealingEnabledToggle.getToggleState(); }
    int getNumVoices() const             { return numberOfVoices.getText().getIntValue(); }

    std::function<void()> onSynthParametersChange;

private:
    //==============================================================================
    void initialiseComboBoxWithConsecutiveIntegers (ComboBox& comboBox, Label& labelToAttach,
                                                    int firstValue, int numValues, int valueToSelect,
                                                    bool makeVisible = true)
    {
        for (auto i = 0; i < numValues; ++i)
            comboBox.addItem (String (i + firstValue), i + 1);

        comboBox.setSelectedId (valueToSelect - firstValue + 1);
        labelToAttach.attachToComponent (&comboBox, true);

        if (makeVisible)
            addAndMakeVisible (comboBox);
        else
            addChildComponent (comboBox);

        if (&comboBox == &numberOfVoices)
            comboBox.onChange = [this] { numberOfVoicesChanged(); };
        else if (&comboBox == &legacyPitchbendRange)
            comboBox.onChange = [this] { if (legacyModeEnabledToggle.getToggleState()) legacyModePitchbendRangeChanged(); };
        else if (&comboBox == &legacyStartChannel || &comboBox == &legacyEndChannel)
            comboBox.onChange = [this] { if (legacyModeEnabledToggle.getToggleState()) legacyModeChannelRangeChanged(); };
    }

    //==============================================================================
    void setZoneButtonClicked()
    {
        auto isLowerZone = isLowerZoneButton.getToggleState();
        auto numMemberChannels = memberChannels.getText().getIntValue();
        auto perNotePb = notePitchbendRange.getText().getIntValue();
        auto masterPb = masterPitchbendRange.getText().getIntValue();

        auto zoneLayout = instrument.getZoneLayout();

        if (isLowerZone)
            zoneLayout.setLowerZone (numMemberChannels, perNotePb, masterPb);
        else
            zoneLayout.setUpperZone (numMemberChannels, perNotePb, masterPb);

        instrument.setZoneLayout (zoneLayout);
    }

    void clearAllZonesButtonClicked()
    {
        instrument.setZoneLayout ({});
    }

    void legacyModeEnabledToggleClicked()
    {
        auto legacyModeEnabled = legacyModeEnabledToggle.getToggleState();

        isLowerZoneButton   .setVisible (! legacyModeEnabled);
        memberChannels      .setVisible (! legacyModeEnabled);
        notePitchbendRange  .setVisible (! legacyModeEnabled);
        masterPitchbendRange.setVisible (! legacyModeEnabled);
        setZoneButton       .setVisible (! legacyModeEnabled);
        clearAllZonesButton .setVisible (! legacyModeEnabled);

        legacyStartChannel  .setVisible (legacyModeEnabled);
        legacyEndChannel    .setVisible (legacyModeEnabled);
        legacyPitchbendRange.setVisible (legacyModeEnabled);

        if (legacyModeEnabled)
        {
            if (areLegacyModeParametersValid())
            {
                instrument.enableLegacyMode();

                instrument.setLegacyModeChannelRange   (getLegacyModeChannelRange());
                instrument.setLegacyModePitchbendRange (getLegacyModePitchbendRange());
            }
            else
            {
                handleInvalidLegacyModeParameters();
            }
        }
        else
        {
            instrument.setZoneLayout ({ MPEZone (MPEZone::Type::lower, 15) });
        }
    }

    //==============================================================================
    void legacyModePitchbendRangeChanged()
    {
        jassert (legacyModeEnabledToggle.getToggleState() == true);

        instrument.setLegacyModePitchbendRange (getLegacyModePitchbendRange());
    }

    void legacyModeChannelRangeChanged()
    {
        jassert (legacyModeEnabledToggle.getToggleState() == true);

        if (areLegacyModeParametersValid())
            instrument.setLegacyModeChannelRange (getLegacyModeChannelRange());
        else
            handleInvalidLegacyModeParameters();
    }

    bool areLegacyModeParametersValid() const
    {
        return legacyStartChannel.getText().getIntValue() <= legacyEndChannel.getText().getIntValue();
    }

    void handleInvalidLegacyModeParameters()
    {
        auto options = MessageBoxOptions::makeOptionsOk (MessageBoxIconType::WarningIcon,
                                                         "Invalid legacy mode channel layout",
                                                         "Cannot set legacy mode start/end channel:\n"
                                                         "The end channel must not be less than the start channel!",
                                                         "Got it");
        messageBox = AlertWindow::showScopedAsync (options, nullptr);
    }

    Range<int> getLegacyModeChannelRange() const
    {
        return { legacyStartChannel.getText().getIntValue(),
                 legacyEndChannel.getText().getIntValue() + 1 };
    }

    int getLegacyModePitchbendRange() const
    {
        return legacyPitchbendRange.getText().getIntValue();
    }

    //==============================================================================
    void voiceStealingEnabledToggleClicked()
    {
        jassert (onSynthParametersChange != nullptr);
        onSynthParametersChange();
    }

    void numberOfVoicesChanged()
    {
        jassert (onSynthParametersChange != nullptr);
        onSynthParametersChange();
    }

    //==============================================================================
    MPEInstrument& instrument;

    ComboBox memberChannels, masterPitchbendRange, notePitchbendRange;

    ToggleButton isLowerZoneButton  { "Lower zone" };

    Label memberChannelsLabel       { {}, "Nr. of member channels:" };
    Label masterPitchbendRangeLabel { {}, "Master pitchbend range (semitones):" };
    Label notePitchbendRangeLabel   { {}, "Note pitchbend range (semitones):" };

    TextButton setZoneButton        { "Set zone" };
    TextButton clearAllZonesButton  { "Clear all zones" };

    ComboBox legacyStartChannel, legacyEndChannel, legacyPitchbendRange;

    Label legacyStartChannelLabel   { {}, "First channel:" };
    Label legacyEndChannelLabel     { {}, "Last channel:" };
    Label legacyPitchbendRangeLabel { {}, "Pitchbend range (semitones):"};

    ToggleButton legacyModeEnabledToggle    { "Enable Legacy Mode" };
    ToggleButton voiceStealingEnabledToggle { "Enable synth voice stealing" };

    ComboBox numberOfVoices;
    Label numberOfVoicesLabel { {}, "Number of synth voices"};

    ScopedMessageBox messageBox;

    static constexpr int defaultMemberChannels       = 15,
                         defaultMasterPitchbendRange = 2,
                         defaultNotePitchbendRange   = 48;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MPESetupComponent)
};

//==============================================================================
class ZoneLayoutComponent final : public Component,
                                  private MPEInstrument::Listener
{
public:
    //==============================================================================
    ZoneLayoutComponent (MPEInstrument& instr, ZoneColourPicker& zoneColourPicker)
        : instrument (instr),
          colourPicker (zoneColourPicker)
    {
        instrument.addListener (this);
    }

    ~ZoneLayoutComponent() override
    {
        instrument.removeListener (this);
    }

    //==============================================================================
    void paint (Graphics& g) override
    {
        paintBackground (g);

        if (instrument.isLegacyModeEnabled())
            paintLegacyMode (g);
        else
            paintZones (g);
    }

private:
    //==============================================================================
    void zoneLayoutChanged() override
    {
        repaint();
    }

    //==============================================================================
    void paintBackground (Graphics& g)
    {
        g.setColour (Colours::black);
        auto channelWidth = getChannelRectangleWidth();

        for (auto i = 0; i < numMidiChannels; ++i)
        {
            auto x = float (i) * channelWidth;
            Rectangle<int> channelArea ((int) x, 0, (int) channelWidth, getHeight());

            g.drawLine ({ x, 0.0f, x, float (getHeight()) });
            g.drawText (String (i + 1), channelArea.reduced (4, 4), Justification::topLeft, false);
        }
    }

    //==============================================================================
    void paintZones (Graphics& g)
    {
        auto channelWidth = getChannelRectangleWidth();

        auto zoneLayout = instrument.getZoneLayout();

        Array<MPEZoneLayout::Zone> activeZones;
        if (zoneLayout.getLowerZone().isActive())  activeZones.add (zoneLayout.getLowerZone());
        if (zoneLayout.getUpperZone().isActive())  activeZones.add (zoneLayout.getUpperZone());

        for (auto zone : activeZones)
        {
            auto zoneColour = colourPicker.getColourForZone (zone.isLowerZone());

            auto xPos = zone.isLowerZone() ? 0 : zone.getLastMemberChannel() - 1;

            Rectangle<int> zoneRect { int (channelWidth * (float) xPos), 20,
                                      int (channelWidth * (float) (zone.numMemberChannels + 1)), getHeight() - 20 };

            g.setColour (zoneColour);
            g.drawRect (zoneRect, 3);

            auto masterRect = zone.isLowerZone() ? zoneRect.removeFromLeft ((int) channelWidth) : zoneRect.removeFromRight ((int) channelWidth);

            g.setColour (zoneColour.withAlpha (0.3f));
            g.fillRect (masterRect);

            g.setColour (zoneColour.contrasting());
            g.drawText ("<>" + String (zone.masterPitchbendRange),  masterRect.reduced (4), Justification::top,    false);
            g.drawText ("<>" + String (zone.perNotePitchbendRange), masterRect.reduced (4), Justification::bottom, false);
        }
    }

    //==============================================================================
    void paintLegacyMode (Graphics& g)
    {
        auto channelRange = instrument.getLegacyModeChannelRange();
        auto startChannel = channelRange.getStart() - 1;
        auto numChannels  = channelRange.getEnd() - startChannel - 1;

        Rectangle<int> zoneRect (int (getChannelRectangleWidth() * (float) startChannel), 0,
                                 int (getChannelRectangleWidth() * (float) numChannels), getHeight());

        zoneRect.removeFromTop (20);

        g.setColour (Colours::white);
        g.drawRect (zoneRect, 3);
        g.drawText ("LGCY", zoneRect.reduced (4, 4), Justification::topLeft, false);
        g.drawText ("<>" + String (instrument.getLegacyModePitchbendRange()), zoneRect.reduced (4, 4), Justification::bottomLeft, false);
    }

    //==============================================================================
    float getChannelRectangleWidth() const noexcept
    {
        return (float) getWidth() / (float) numMidiChannels;
    }

    //==============================================================================
    static constexpr int numMidiChannels = 16;

    MPEInstrument& instrument;
    ZoneColourPicker& colourPicker;
};

//==============================================================================
class MPEDemoSynthVoice final : public MPESynthesiserVoice
{
public:
    //==============================================================================
    MPEDemoSynthVoice() {}

    //==============================================================================
    void noteStarted() override
    {
        jassert (currentlyPlayingNote.isValid());
        jassert (currentlyPlayingNote.keyState == MPENote::keyDown
                 || currentlyPlayingNote.keyState == MPENote::keyDownAndSustained);

        level    .setTargetValue (currentlyPlayingNote.pressure.asUnsignedFloat());
        frequency.setTargetValue (currentlyPlayingNote.getFrequencyInHertz());
        timbre   .setTargetValue (currentlyPlayingNote.timbre.asUnsignedFloat());

        phase = 0.0;
        auto cyclesPerSample = frequency.getNextValue() / currentSampleRate;
        phaseDelta = MathConstants<double>::twoPi * cyclesPerSample;

        tailOff = 0.0;
    }

    void noteStopped (bool allowTailOff) override
    {
        jassert (currentlyPlayingNote.keyState == MPENote::off);

        if (allowTailOff)
        {
            // start a tail-off by setting this flag. The render callback will pick up on
            // this and do a fade out, calling clearCurrentNote() when it's finished.

            if (approximatelyEqual (tailOff, 0.0)) // we only need to begin a tail-off if it's not already doing so - the
                                                   // stopNote method could be called more than once.
                tailOff = 1.0;
        }
        else
        {
            // we're being told to stop playing immediately, so reset everything..
            clearCurrentNote();
            phaseDelta = 0.0;
        }
    }

    void notePressureChanged() override
    {
        level.setTargetValue (currentlyPlayingNote.pressure.asUnsignedFloat());
    }

    void notePitchbendChanged() override
    {
        frequency.setTargetValue (currentlyPlayingNote.getFrequencyInHertz());
    }

    void noteTimbreChanged() override
    {
        timbre.setTargetValue (currentlyPlayingNote.timbre.asUnsignedFloat());
    }

    void noteKeyStateChanged() override {}

    void setCurrentSampleRate (double newRate) override
    {
        if (! approximatelyEqual (currentSampleRate, newRate))
        {
            noteStopped (false);
            currentSampleRate = newRate;

            level    .reset (currentSampleRate, smoothingLengthInSeconds);
            timbre   .reset (currentSampleRate, smoothingLengthInSeconds);
            frequency.reset (currentSampleRate, smoothingLengthInSeconds);
        }
    }

    //==============================================================================
    virtual void renderNextBlock (AudioBuffer<float>& outputBuffer,
                                  int startSample,
                                  int numSamples) override
    {
        if (! approximatelyEqual (phaseDelta, 0.0))
        {
            if (tailOff > 0.0)
            {
                while (--numSamples >= 0)
                {
                    auto currentSample = getNextSample() * (float) tailOff;

                    for (auto i = outputBuffer.getNumChannels(); --i >= 0;)
                        outputBuffer.addSample (i, startSample, currentSample);

                    ++startSample;

                    tailOff *= 0.99;

                    if (tailOff <= 0.005)
                    {
                        clearCurrentNote();

                        phaseDelta = 0.0;
                        break;
                    }
                }
            }
            else
            {
                while (--numSamples >= 0)
                {
                    auto currentSample = getNextSample();

                    for (auto i = outputBuffer.getNumChannels(); --i >= 0;)
                        outputBuffer.addSample (i, startSample, currentSample);

                    ++startSample;
                }
            }
        }
    }

    using MPESynthesiserVoice::renderNextBlock;

private:
    //==============================================================================
    float getNextSample() noexcept
    {
        auto levelDb = (level.getNextValue() - 1.0) * maxLevelDb;
        auto amplitude = pow (10.0f, 0.05f * levelDb) * maxLevel;

        // timbre is used to blend between a sine and a square.
        auto f1 = std::sin (phase);
        auto f2 = copysign (1.0, f1);
        auto a2 = timbre.getNextValue();
        auto a1 = 1.0 - a2;

        auto nextSample = float (amplitude * ((a1 * f1) + (a2 * f2)));

        auto cyclesPerSample = frequency.getNextValue() / currentSampleRate;
        phaseDelta = MathConstants<double>::twoPi * cyclesPerSample;
        phase = std::fmod (phase + phaseDelta, MathConstants<double>::twoPi);

        return nextSample;
    }

    //==============================================================================
    SmoothedValue<double> level, timbre, frequency;

    double phase      = 0.0;
    double phaseDelta = 0.0;
    double tailOff    = 0.0;

    const double maxLevel   = 0.05;
    const double maxLevelDb = 31.0;
    const double smoothingLengthInSeconds = 0.01;
};

//==============================================================================
class MPEDemo final : public Component,
                      private AudioIODeviceCallback,
                      private MidiInputCallback,
                      private MPEInstrument::Listener
{
public:
    //==============================================================================
    MPEDemo()
    {
       #ifndef JUCE_DEMO_RUNNER
        audioDeviceManager.initialise (0, 2, nullptr, true, {}, nullptr);
       #endif

        audioDeviceManager.addMidiInputDeviceCallback ({}, this);
        audioDeviceManager.addAudioCallback (this);

        addAndMakeVisible (audioSetupComp);
        addAndMakeVisible (mpeSetupComp);
        addAndMakeVisible (zoneLayoutComp);
        addAndMakeVisible (keyboardComponent);

        synth.setVoiceStealingEnabled (false);
        for (auto i = 0; i < 15; ++i)
            synth.addVoice (new MPEDemoSynthVoice());

        mpeSetupComp.onSynthParametersChange = [this]
        {
            synth.setVoiceStealingEnabled (mpeSetupComp.isVoiceStealingEnabled());

            auto numVoices = mpeSetupComp.getNumVoices();

            if (numVoices < synth.getNumVoices())
            {
                synth.reduceNumVoices (numVoices);
            }
            else
            {
                while (synth.getNumVoices() < numVoices)
                    synth.addVoice (new MPEDemoSynthVoice());
            }
        };

        instrument.addListener (this);

        setSize (880, 720);
    }

    ~MPEDemo() override
    {
        audioDeviceManager.removeMidiInputDeviceCallback ({}, this);
        audioDeviceManager.removeAudioCallback (this);
    }

    //==============================================================================
    void resized() override
    {
        auto zoneLayoutCompHeight = 60;
        auto audioSetupCompRelativeWidth = 0.55f;

        auto r = getLocalBounds();

        keyboardComponent.setBounds (r.removeFromBottom (150));
        r.reduce (10, 10);

        zoneLayoutComp.setBounds (r.removeFromBottom (zoneLayoutCompHeight));
        audioSetupComp.setBounds (r.removeFromLeft (proportionOfWidth (audioSetupCompRelativeWidth)));
        mpeSetupComp  .setBounds (r);
    }

    //==============================================================================
    void audioDeviceIOCallbackWithContext (const float* const* inputChannelData, int numInputChannels,
                                           float* const* outputChannelData, int numOutputChannels,
                                           int numSamples, const AudioIODeviceCallbackContext& context) override
    {
        ignoreUnused (inputChannelData, numInputChannels, context);

        AudioBuffer<float> buffer (outputChannelData, numOutputChannels, numSamples);
        buffer.clear();

        MidiBuffer incomingMidi;
        midiCollector.removeNextBlockOfMessages (incomingMidi, numSamples);
        synth.renderNextBlock (buffer, incomingMidi, 0, numSamples);
    }

    void audioDeviceAboutToStart (AudioIODevice* device) override
    {
        auto sampleRate = device->getCurrentSampleRate();
        midiCollector.reset (sampleRate);
        synth.setCurrentPlaybackSampleRate (sampleRate);
    }

    void audioDeviceStopped() override {}

private:
    //==============================================================================
    void handleIncomingMidiMessage (MidiInput* /*source*/,
                                    const MidiMessage& message) override
    {
        instrument.processNextMidiEvent (message);
        midiCollector.addMessageToQueue (message);
    }

    //==============================================================================
    void zoneLayoutChanged() override
    {
        if (instrument.isLegacyModeEnabled())
        {
            colourPicker.setLegacyModeEnabled (true);

            synth.enableLegacyMode (instrument.getLegacyModePitchbendRange(),
                                    instrument.getLegacyModeChannelRange());
        }
        else
        {
            colourPicker.setLegacyModeEnabled (false);

            auto zoneLayout = instrument.getZoneLayout();

            if (auto* midiOutput = audioDeviceManager.getDefaultMidiOutput())
                midiOutput->sendBlockOfMessagesNow (MPEMessages::setZoneLayout (zoneLayout));

            synth.setZoneLayout (zoneLayout);
            colourPicker.setZoneLayout (zoneLayout);
        }
    }

    //==============================================================================
    // if this PIP is running inside the demo runner, we'll use the shared device manager instead
   #ifndef JUCE_DEMO_RUNNER
    AudioDeviceManager audioDeviceManager;
   #else
    AudioDeviceManager& audioDeviceManager { getSharedAudioDeviceManager (0, 2) };
   #endif

    AudioDeviceSelectorComponent audioSetupComp { audioDeviceManager, 0, 0, 0, 256, true, true, true, false };
    MidiMessageCollector midiCollector;

    MPEInstrument instrument  { MPEZone (MPEZone::Type::lower, 15) };

    ZoneColourPicker colourPicker;
    MPESetupComponent mpeSetupComp      { instrument };
    ZoneLayoutComponent zoneLayoutComp  { instrument, colourPicker};

    MPESynthesiser synth                   { instrument };
    MPEKeyboardComponent keyboardComponent { instrument, MPEKeyboardComponent::horizontalKeyboard };

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MPEDemo)
};

/*
  ==============================================================================

   This file is part of the JUCE examples.
   Copyright (c) 2017 - ROLI Ltd.

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

 name:             MPEDemo
 version:          1.0.0
 vendor:           JUCE
 website:          http://juce.com
 description:      Simple MPE synthesiser application.

 dependencies:     juce_audio_basics, juce_audio_devices, juce_audio_formats,
                   juce_audio_processors, juce_audio_utils, juce_core,
                   juce_data_structures, juce_events, juce_graphics,
                   juce_gui_basics, juce_gui_extra
 exporters:        xcode_mac, vs2017, linux_make

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
class NoteComponent : public Component
{
public:
    NoteComponent (const MPENote& n, Colour colourToUse)
        : note (n), colour (colourToUse)
    {}

    //==============================================================================
    void update (const MPENote& newNote, Point<float> newCentre)
    {
        note = newNote;
        centre = newCentre;

        setBounds (getSquareAroundCentre (jmax (getNoteOnRadius(), getNoteOffRadius(), getPressureRadius()))
                     .getUnion (getTextRectangle())
                     .getSmallestIntegerContainer()
                     .expanded (3));

        repaint();
    }

    //==============================================================================
    void paint (Graphics& g) override
    {
        if (note.keyState == MPENote::keyDown || note.keyState == MPENote::keyDownAndSustained)
            drawPressedNoteCircle (g, colour);
        else if (note.keyState == MPENote::sustained)
            drawSustainedNoteCircle (g, colour);
        else
            return;

        drawNoteLabel (g, colour);
    }

    //==============================================================================
    MPENote note;
    Colour colour;
    Point<float> centre;

private:
    //==============================================================================
    void drawPressedNoteCircle (Graphics& g, Colour zoneColour)
    {
        g.setColour (zoneColour.withAlpha (0.3f));
        g.fillEllipse (translateToLocalBounds (getSquareAroundCentre (getNoteOnRadius())));
        g.setColour (zoneColour);
        g.drawEllipse (translateToLocalBounds (getSquareAroundCentre (getPressureRadius())), 2.0f);
    }

    //==============================================================================
    void drawSustainedNoteCircle (Graphics& g, Colour zoneColour)
    {
        g.setColour (zoneColour);
        Path circle, dashedCircle;
        circle.addEllipse (translateToLocalBounds (getSquareAroundCentre (getNoteOffRadius())));
        float dashLengths[] = { 3.0f, 3.0f };
        PathStrokeType (2.0, PathStrokeType::mitered).createDashedStroke (dashedCircle, circle, dashLengths, 2);
        g.fillPath (dashedCircle);
    }

    //==============================================================================
    void drawNoteLabel (Graphics& g, Colour /**zoneColour*/)
    {
        auto textBounds = translateToLocalBounds (getTextRectangle()).getSmallestIntegerContainer();

        g.drawText ("+", textBounds, Justification::centred);
        g.drawText (MidiMessage::getMidiNoteName (note.initialNote, true, true, 3), textBounds, Justification::centredBottom);
        g.setFont (Font (22.0f, Font::bold));
        g.drawText (String (note.midiChannel), textBounds, Justification::centredTop);
    }

    //==============================================================================
    Rectangle<float> getSquareAroundCentre (float radius) const noexcept
    {
        return Rectangle<float> (radius * 2.0f, radius * 2.0f).withCentre (centre);
    }

    Rectangle<float> translateToLocalBounds (Rectangle<float> r) const noexcept
    {
        return r - getPosition().toFloat();
    }

    Rectangle<float> getTextRectangle() const noexcept
    {
        return Rectangle<float> (30.0f, 50.0f).withCentre (centre);
    }

    float getNoteOnRadius()   const noexcept   { return note.noteOnVelocity .asUnsignedFloat() * maxNoteRadius; }
    float getNoteOffRadius()  const noexcept   { return note.noteOffVelocity.asUnsignedFloat() * maxNoteRadius; }
    float getPressureRadius() const noexcept   { return note.pressure       .asUnsignedFloat() * maxNoteRadius; }

    const float maxNoteRadius = 100.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NoteComponent)
};

//==============================================================================
class Visualiser : public Component,
                   public MPEInstrument::Listener,
                   private AsyncUpdater
{
public:
    //==============================================================================
    Visualiser (ZoneColourPicker& zoneColourPicker)
        : colourPicker (zoneColourPicker)
    {}

    //==============================================================================
    void paint (Graphics& g) override
    {
        g.fillAll (Colours::black);

        auto noteDistance = float (getWidth()) / 128;
        for (auto i = 0; i < 128; ++i)
        {
            auto x = noteDistance * i;
            auto noteHeight = int (MidiMessage::isMidiNoteBlack (i) ? 0.7 * getHeight() : getHeight());

            g.setColour (MidiMessage::isMidiNoteBlack (i) ? Colours::white : Colours::grey);
            g.drawLine (x, 0.0f, x, (float) noteHeight);

            if (i > 0 && i % 12 == 0)
            {
                g.setColour (Colours::grey);
                auto octaveNumber = (i / 12) - 2;
                g.drawText ("C" + String (octaveNumber), (int) x - 15, getHeight() - 30, 30, 30, Justification::centredBottom);
            }
        }
    }

    //==============================================================================
    void noteAdded (MPENote newNote) override
    {
        const ScopedLock sl (lock);
        activeNotes.add (newNote);
        triggerAsyncUpdate();
    }

    void notePressureChanged  (MPENote note) override { noteChanged (note); }
    void notePitchbendChanged (MPENote note) override { noteChanged (note); }
    void noteTimbreChanged    (MPENote note) override { noteChanged (note); }
    void noteKeyStateChanged  (MPENote note) override { noteChanged (note); }

    void noteChanged (MPENote changedNote)
    {
        const ScopedLock sl (lock);

        for (auto& note : activeNotes)
            if (note.noteID == changedNote.noteID)
                note = changedNote;

        triggerAsyncUpdate();
    }

    void noteReleased (MPENote finishedNote) override
    {
        const ScopedLock sl (lock);

        for (auto i = activeNotes.size(); --i >= 0;)
            if (activeNotes.getReference(i).noteID == finishedNote.noteID)
                activeNotes.remove (i);

        triggerAsyncUpdate();
    }


private:
    //==============================================================================
    MPENote* findActiveNote (int noteID) const noexcept
    {
        for (auto& note : activeNotes)
            if (note.noteID == noteID)
                return &note;

        return nullptr;
    }

    NoteComponent* findNoteComponent (int noteID) const noexcept
    {
        for (auto& noteComp : noteComponents)
            if (noteComp->note.noteID == noteID)
                return noteComp;

        return nullptr;
    }

    //==============================================================================
    void handleAsyncUpdate() override
    {
        const ScopedLock sl (lock);

        for (auto i = noteComponents.size(); --i >= 0;)
            if (findActiveNote (noteComponents.getUnchecked(i)->note.noteID) == nullptr)
                noteComponents.remove (i);

        for (auto& note : activeNotes)
            if (findNoteComponent (note.noteID) == nullptr)
                addAndMakeVisible (noteComponents.add (new NoteComponent (note, colourPicker.getColourForMidiChannel(note.midiChannel))));

        for (auto& noteComp : noteComponents)
            if (auto* noteInfo = findActiveNote (noteComp->note.noteID))
                noteComp->update (*noteInfo, getCentrePositionForNote (*noteInfo));
    }

    //==============================================================================
    Point<float> getCentrePositionForNote (MPENote note) const
    {
        auto n = float (note.initialNote) + float (note.totalPitchbendInSemitones);
        auto x = getWidth() * n / 128;
        auto y = getHeight() * (1 - note.timbre.asUnsignedFloat());

        return { x, y };
    }

    //==============================================================================
    OwnedArray<NoteComponent> noteComponents;
    CriticalSection lock;
    Array<MPENote> activeNotes;
    ZoneColourPicker& colourPicker;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Visualiser)
};

//==============================================================================
class MPESetupComponent : public Component,
                          public ChangeBroadcaster
{
public:
    //==============================================================================
    class Listener
    {
    public:
        virtual ~Listener() {}
        virtual void zoneChanged (bool isLower, int numMemberChans, int perNotePb, int masterPb) = 0;
        virtual void allZonesCleared() = 0;
        virtual void legacyModeChanged (bool legacyModeEnabled, int pitchbendRange, Range<int> channelRange) = 0;
        virtual void voiceStealingEnabledChanged (bool voiceStealingEnabled) = 0;
        virtual void numberOfVoicesChanged (int numberOfVoices) = 0;
    };

    void addListener (Listener* listenerToAdd)         { listeners.add (listenerToAdd); }
    void removeListener (Listener* listenerToRemove)   { listeners.remove (listenerToRemove); }

    //==============================================================================
    MPESetupComponent()
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

        if (isLowerZone)
            zoneLayout.setLowerZone (numMemberChannels, perNotePb, masterPb);
        else
            zoneLayout.setUpperZone (numMemberChannels, perNotePb, masterPb);

        listeners.call ([&] (Listener& l) { l.zoneChanged (isLowerZone, numMemberChannels, perNotePb, masterPb); });
    }

    //==============================================================================
    void clearAllZonesButtonClicked()
    {
        zoneLayout.clearAllZones();
        listeners.call ([] (Listener& l) { l.allZonesCleared(); });
    }

    //==============================================================================
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

        if (areLegacyModeParametersValid())
        {
            listeners.call ([&] (Listener& l) { l.legacyModeChanged (legacyModeEnabledToggle.getToggleState(),
                                                                     legacyPitchbendRange.getText().getIntValue(),
                                                                     getLegacyModeChannelRange()); });
        }
        else
        {
            handleInvalidLegacyModeParameters();
        }
    }

    //==============================================================================
    void voiceStealingEnabledToggleClicked()
    {
        auto newState = voiceStealingEnabledToggle.getToggleState();
        listeners.call ([=] (Listener& l) { l.voiceStealingEnabledChanged (newState); });
    }

    //==============================================================================
    void numberOfVoicesChanged()
    {
        listeners.call ([this] (Listener& l) { l.numberOfVoicesChanged (numberOfVoices.getText().getIntValue()); });
    }

    void legacyModePitchbendRangeChanged()
    {
        jassert (legacyModeEnabledToggle.getToggleState() == true);

        listeners.call ([this] (Listener& l) { l.legacyModeChanged (true,
                                                                    legacyPitchbendRange.getText().getIntValue(),
                                                                    getLegacyModeChannelRange()); });
    }

    void legacyModeChannelRangeChanged()
    {
        jassert (legacyModeEnabledToggle.getToggleState() == true);

        if (areLegacyModeParametersValid())
        {
            listeners.call ([this] (Listener& l) { l.legacyModeChanged (true,
                                                                        legacyPitchbendRange.getText().getIntValue(),
                                                                        getLegacyModeChannelRange()); });
        }
        else
        {
            handleInvalidLegacyModeParameters();
        }
    }

    //==============================================================================
    bool areLegacyModeParametersValid() const
    {
        return legacyStartChannel.getText().getIntValue() <= legacyEndChannel.getText().getIntValue();
    }

    void handleInvalidLegacyModeParameters() const
    {
        AlertWindow::showMessageBoxAsync (AlertWindow::WarningIcon,
                                          "Invalid legacy mode channel layout",
                                          "Cannot set legacy mode start/end channel:\n"
                                          "The end channel must not be less than the start channel!",
                                          "Got it");
    }

    //==============================================================================
    Range<int> getLegacyModeChannelRange() const
    {
        return { legacyStartChannel.getText().getIntValue(),
                 legacyEndChannel.getText().getIntValue() + 1 };
    }

    //==============================================================================
    MPEZoneLayout zoneLayout;

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

    ListenerList<Listener> listeners;

    const int defaultMemberChannels       = 15,
              defaultMasterPitchbendRange = 2,
              defaultNotePitchbendRange   = 48;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MPESetupComponent)
};

//==============================================================================
class ZoneLayoutComponent : public Component,
                            public MPESetupComponent::Listener
{
public:
    //==============================================================================
    ZoneLayoutComponent (const ZoneColourPicker& zoneColourPicker)
        : colourPicker (zoneColourPicker)
    {}

    //==============================================================================
    void paint (Graphics& g) override
    {
        paintBackground (g);

        if (legacyModeEnabled)
            paintLegacyMode (g);
        else
            paintZones (g);
    }

    //==============================================================================
    void zoneChanged (bool isLowerZone, int numMemberChannels,
                      int perNotePitchbendRange, int masterPitchbendRange) override
    {
        if (isLowerZone)
            zoneLayout.setLowerZone (numMemberChannels, perNotePitchbendRange, masterPitchbendRange);
        else
            zoneLayout.setUpperZone (numMemberChannels, perNotePitchbendRange, masterPitchbendRange);

        repaint();
    }

    void allZonesCleared() override
    {
        zoneLayout.clearAllZones();
        repaint();
    }

    void legacyModeChanged (bool legacyModeShouldBeEnabled, int pitchbendRange, Range<int> channelRange) override
    {
        legacyModeEnabled = legacyModeShouldBeEnabled;
        legacyModePitchbendRange = pitchbendRange;
        legacyModeChannelRange = channelRange;

        repaint();
    }

    void voiceStealingEnabledChanged (bool) override   { /* not interested in this change */ }
    void numberOfVoicesChanged (int) override          { /* not interested in this change */ }

private:
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

        Array<MPEZoneLayout::Zone> activeZones;
        if (zoneLayout.getLowerZone().isActive())  activeZones.add (zoneLayout.getLowerZone());
        if (zoneLayout.getUpperZone().isActive())  activeZones.add (zoneLayout.getUpperZone());

        for (auto zone : activeZones)
        {
            auto zoneColour = colourPicker.getColourForZone (zone.isLowerZone());

            auto xPos = zone.isLowerZone() ? 0 : zone.getLastMemberChannel() - 1;

            Rectangle<int> zoneRect { int (channelWidth * (xPos)), 20,
                                      int (channelWidth * (zone.numMemberChannels + 1)), getHeight() - 20 };

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
        auto startChannel = legacyModeChannelRange.getStart() - 1;
        auto numChannels  = legacyModeChannelRange.getEnd() - startChannel - 1;


        Rectangle<int> zoneRect (int (getChannelRectangleWidth() * startChannel), 0,
                                 int (getChannelRectangleWidth() * numChannels), getHeight());

        zoneRect.removeFromTop (20);

        g.setColour (Colours::white);
        g.drawRect (zoneRect, 3);
        g.drawText ("LGCY", zoneRect.reduced (4, 4), Justification::topLeft, false);
        g.drawText ("<>" + String (legacyModePitchbendRange), zoneRect.reduced (4, 4), Justification::bottomLeft, false);
    }

    //==============================================================================
    float getChannelRectangleWidth() const noexcept
    {
        return float (getWidth()) / numMidiChannels;
    }

    //==============================================================================
    MPEZoneLayout zoneLayout;
    const ZoneColourPicker& colourPicker;

    bool legacyModeEnabled = false;
    int legacyModePitchbendRange = 48;
    Range<int> legacyModeChannelRange = { 1, 17 };
    const int numMidiChannels = 16;
};

//==============================================================================
class MPEDemoSynthVoice : public MPESynthesiserVoice
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

        level.setValue (currentlyPlayingNote.pressure.asUnsignedFloat());
        frequency.setValue (currentlyPlayingNote.getFrequencyInHertz());
        timbre.setValue (currentlyPlayingNote.timbre.asUnsignedFloat());

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

            if (tailOff == 0.0) // we only need to begin a tail-off if it's not already doing so - the
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
        level.setValue (currentlyPlayingNote.pressure.asUnsignedFloat());
    }

    void notePitchbendChanged() override
    {
        frequency.setValue (currentlyPlayingNote.getFrequencyInHertz());
    }

    void noteTimbreChanged() override
    {
        timbre.setValue (currentlyPlayingNote.timbre.asUnsignedFloat());
    }

    void noteKeyStateChanged() override {}

    void setCurrentSampleRate (double newRate) override
    {
        if (currentSampleRate != newRate)
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
        if (phaseDelta != 0.0)
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
    LinearSmoothedValue<double> level, timbre, frequency;

    double phase      = 0.0;
    double phaseDelta = 0.0;
    double tailOff    = 0.0;

    const double maxLevel   = 0.05;
    const double maxLevelDb = 31.0;
    const double smoothingLengthInSeconds = 0.01;
};

//==============================================================================
class MPEDemo : public Component,
                private AudioIODeviceCallback,
                private MidiInputCallback,
                private MPESetupComponent::Listener
{
public:
    //==============================================================================
    MPEDemo()
        : audioSetupComp (audioDeviceManager, 0, 0, 0, 256, true, true, true, false),
          zoneLayoutComp (colourPicker),
          visualiserComp (colourPicker)
    {
       #ifndef JUCE_DEMO_RUNNER
        audioDeviceManager.initialise (0, 2, 0, true, {}, 0);
       #endif

        audioDeviceManager.addMidiInputCallback ({}, this);
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
        for (auto i = 0; i < 15; ++i)
            synth.addVoice (new MPEDemoSynthVoice());

        setSize (880, 720);
    }

    ~MPEDemo()
    {
        audioDeviceManager.removeMidiInputCallback ({}, this);
        audioDeviceManager.removeAudioCallback (this);
    }

    //==============================================================================
    void resized() override
    {
        auto visualiserCompWidth  = 2800;
        auto visualiserCompHeight = 300;
        auto zoneLayoutCompHeight = 60;
        auto audioSetupCompRelativeWidth = 0.55f;

        auto r = getLocalBounds();

        visualiserViewport.setBounds (r.removeFromBottom (visualiserCompHeight));
        visualiserComp    .setBounds ({ visualiserCompWidth,
                                        visualiserViewport.getHeight() - visualiserViewport.getScrollBarThickness() });

        zoneLayoutComp.setBounds (r.removeFromBottom (zoneLayoutCompHeight));
        audioSetupComp.setBounds (r.removeFromLeft (proportionOfWidth (audioSetupCompRelativeWidth)));
        MPESetupComp  .setBounds (r);
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
        visualiserInstrument.processNextMidiEvent (message);
        midiCollector.addMessageToQueue (message);
    }

    //==============================================================================
    void zoneChanged (bool isLowerZone, int numMemberChannels,
                      int perNotePitchbendRange, int masterPitchbendRange) override
    {
        auto* midiOutput = audioDeviceManager.getDefaultMidiOutput();
        if (midiOutput != nullptr)
        {
            if (isLowerZone)
                midiOutput->sendBlockOfMessagesNow (MPEMessages::setLowerZone (numMemberChannels, perNotePitchbendRange, masterPitchbendRange));
            else
                midiOutput->sendBlockOfMessagesNow (MPEMessages::setUpperZone (numMemberChannels, perNotePitchbendRange, masterPitchbendRange));
        }

        if (isLowerZone)
            zoneLayout.setLowerZone (numMemberChannels, perNotePitchbendRange, masterPitchbendRange);
        else
            zoneLayout.setUpperZone (numMemberChannels, perNotePitchbendRange, masterPitchbendRange);

        visualiserInstrument.setZoneLayout (zoneLayout);
        synth.setZoneLayout (zoneLayout);
        colourPicker.setZoneLayout (zoneLayout);
    }

    void allZonesCleared() override
    {
        auto* midiOutput = audioDeviceManager.getDefaultMidiOutput();
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
                synth.addVoice (new MPEDemoSynthVoice());
    }

    //==============================================================================
    // if this PIP is running inside the demo runner, we'll use the shared device manager instead
   #ifndef JUCE_DEMO_RUNNER
    AudioDeviceManager audioDeviceManager;
   #else
    AudioDeviceManager& audioDeviceManager { getSharedAudioDeviceManager (0, 2) };
   #endif

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

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MPEDemo)
};

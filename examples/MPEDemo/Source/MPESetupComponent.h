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


class MPESetupComponent : public Component,
                          public ChangeBroadcaster,
                          private Button::Listener,
                          private ComboBox::Listener
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

        initialiseButton (setZoneButton);
        initialiseButton (clearAllZonesButton);
        initialiseButton (legacyModeEnabledToggle);
        initialiseButton (voiceStealingEnabledToggle);

        initialiseComboBoxWithConsecutiveIntegers (numberOfVoices, numberOfVoicesLabel, 1, 20, 15);

        numberOfVoices.addListener (this);
    }

    //==============================================================================
    void resized() override
    {
        Rectangle<int> r (proportionOfWidth (0.65f), 15, proportionOfWidth (0.25f), 3000);
        const int h = 24;
        const int hspace = 6;
        const int hbigspace = 18;

        isLowerZoneButton.setBounds (r.removeFromTop (h));
        r.removeFromTop (hspace);
        memberChannels.setBounds (r.removeFromTop (h));
        r.removeFromTop (hspace);
        notePitchbendRange.setBounds (r.removeFromTop (h));
        r.removeFromTop (hspace);
        masterPitchbendRange.setBounds (r.removeFromTop (h));

        legacyStartChannel.setBounds (isLowerZoneButton.getBounds());
        legacyEndChannel.setBounds (memberChannels.getBounds());
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
        for (int i = 0; i < numValues; ++i)
            comboBox.addItem (String (i + firstValue), i + 1);

        comboBox.setSelectedId (valueToSelect - firstValue + 1);
        labelToAttach.attachToComponent (&comboBox, true);

        if (makeVisible)
            addAndMakeVisible (comboBox);
        else
            addChildComponent (comboBox);

        comboBox.addListener (this);
    }

    //==============================================================================
    void initialiseButton (Button& button)
    {
        addAndMakeVisible (button);
        button.addListener (this);
    }

    //==============================================================================
    void buttonClicked (Button* button) override
    {
        if      (button == &setZoneButton)               setZoneButtonClicked();
        else if (button == &clearAllZonesButton)         clearAllZonesButtonClicked();
        else if (button == &legacyModeEnabledToggle)     legacyModeEnabledToggleClicked();
        else if (button == &voiceStealingEnabledToggle)  voiceStealingEnabledToggleClicked();
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

        isLowerZoneButton.setVisible    (! legacyModeEnabled);
        memberChannels.setVisible       (! legacyModeEnabled);
        notePitchbendRange.setVisible   (! legacyModeEnabled);
        masterPitchbendRange.setVisible (! legacyModeEnabled);
        setZoneButton.setVisible        (! legacyModeEnabled);
        clearAllZonesButton.setVisible  (! legacyModeEnabled);

        legacyStartChannel.setVisible   (legacyModeEnabled);
        legacyEndChannel.setVisible     (legacyModeEnabled);
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
    void comboBoxChanged (ComboBox* comboBoxThatHasChanged) override
    {
        if (comboBoxThatHasChanged == &numberOfVoices)
        {
            numberOfVoicesChanged();
        }
        else if (legacyModeEnabledToggle.getToggleState())
        {
            if (comboBoxThatHasChanged == &legacyPitchbendRange)
                legacyModePitchbendRangeChanged();
            else if (comboBoxThatHasChanged == &legacyStartChannel || comboBoxThatHasChanged == &legacyEndChannel)
                legacyModeChannelRangeChanged();
        }
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

    const int defaultMasterChannel = 1, defaultMemberChannels = 15,
              defaultMasterPitchbendRange = 2, defaultNotePitchbendRange = 48;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MPESetupComponent)

};

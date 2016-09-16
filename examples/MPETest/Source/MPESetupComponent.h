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


#ifndef MPESETUPCOMPONENT_H_INCLUDED
#define MPESETUPCOMPONENT_H_INCLUDED


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
        virtual void zoneAdded (MPEZone newZone) = 0;
        virtual void allZonesCleared() = 0;
        virtual void legacyModeChanged (bool legacyModeEnabled, int pitchbendRange, Range<int> channelRange) = 0;
        virtual void voiceStealingEnabledChanged (bool voiceStealingEnabled) = 0;
        virtual void numberOfVoicesChanged (int numberOfVoices) = 0;
    };

    void addListener (Listener* listenerToAdd)         { listeners.add (listenerToAdd); }
    void removeListener (Listener* listenerToRemove)   { listeners.remove (listenerToRemove); }

    //==============================================================================
    MPESetupComponent()
        : masterChannelLabel (String(), "Master channel:"),
          noteChannelsLabel (String(), "Nr. of note channels:"),
          masterPitchbendRangeLabel (String(), "Master pitchbend range (semitones):"),
          notePitchbendRangeLabel (String(), "Note pitchbend range (semitones):"),
          addZoneButton ("Add this zone"),
          clearAllZonesButton ("Clear all zones"),
          legacyStartChannelLabel (String(), "First channel:"),
          legacyEndChannelLabel (String(), "Last channel:"),
          legacyPitchbendRangeLabel (String(), "Pitchbend range (semitones):"),
          legacyModeEnabledToggle ("Enable Legacy Mode"),
          voiceStealingEnabledToggle ("Enable synth voice stealing"),
          numberOfVoicesLabel (String(), "Number of synth voices")
    {

        initialiseComboBoxWithConsecutiveIntegers (masterChannel, masterChannelLabel, 1, 15, defaultMasterChannel);
        initialiseComboBoxWithConsecutiveIntegers (noteChannels, noteChannelsLabel, 1, 15, defaultNoteChannels);
        initialiseComboBoxWithConsecutiveIntegers (masterPitchbendRange, masterPitchbendRangeLabel, 0, 96, defaultMasterPitchbendRange);
        initialiseComboBoxWithConsecutiveIntegers (notePitchbendRange, notePitchbendRangeLabel, 0, 96, defaultNotePitchbendRange);

        initialiseComboBoxWithConsecutiveIntegers (legacyStartChannel, legacyStartChannelLabel, 1, 16, 1, false);
        initialiseComboBoxWithConsecutiveIntegers (legacyEndChannel, legacyEndChannelLabel, 1, 16, 16, false);
        initialiseComboBoxWithConsecutiveIntegers (legacyPitchbendRange, legacyPitchbendRangeLabel, 0, 96, 2, false);

        initialiseButton (addZoneButton);
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

        masterChannel.setBounds (r.removeFromTop (h));
        r.removeFromTop (hspace);
        noteChannels.setBounds (r.removeFromTop (h));
        r.removeFromTop (hspace);
        notePitchbendRange.setBounds (r.removeFromTop (h));
        r.removeFromTop (hspace);
        masterPitchbendRange.setBounds (r.removeFromTop (h));

        legacyStartChannel.setBounds (masterChannel.getBounds());
        legacyEndChannel.setBounds (noteChannels.getBounds());
        legacyPitchbendRange.setBounds (notePitchbendRange.getBounds());

        r.removeFromTop (hbigspace);

        int buttonLeft = proportionOfWidth (0.5f);

        addZoneButton.setBounds (r.removeFromTop (h).withLeft (buttonLeft));
        r.removeFromTop (hspace);
        clearAllZonesButton.setBounds (r.removeFromTop (h).withLeft (buttonLeft));

        r.removeFromTop (hbigspace);

        int toggleLeft = proportionOfWidth (0.25f);

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
        if (button == &addZoneButton)
            addZoneButtonClicked();
        else if (button == &clearAllZonesButton)
            clearAllZonesButtonClicked();
        else if (button == &legacyModeEnabledToggle)
            legacyModeEnabledToggleClicked();
        else if (button == &voiceStealingEnabledToggle)
            voiceStealingEnabledToggleClicked();
    }

    //==============================================================================
    void addZoneButtonClicked()
    {
        if (areMPEParametersValid())
        {
            MPEZone newZone (masterChannel.getText().getIntValue(),
                                        noteChannels.getText().getIntValue(),
                                        notePitchbendRange.getText().getIntValue(),
                                        masterPitchbendRange.getText().getIntValue());

            zoneLayout.addZone (newZone);
            listeners.call (&MPESetupComponent::Listener::zoneAdded, newZone);
        }
        else
        {
            handleInvalidMPEParameters();
        }
    }

    //==============================================================================
    void clearAllZonesButtonClicked()
    {
        zoneLayout.clearAllZones();
        listeners.call (&MPESetupComponent::Listener::allZonesCleared);
    }

    //==============================================================================
    void legacyModeEnabledToggleClicked()
    {
        bool legacyModeEnabled = legacyModeEnabledToggle.getToggleState();

        masterChannel.setVisible (! legacyModeEnabled);
        noteChannels.setVisible (! legacyModeEnabled);
        notePitchbendRange.setVisible (! legacyModeEnabled);
        masterPitchbendRange.setVisible (! legacyModeEnabled);
        addZoneButton.setVisible (! legacyModeEnabled);
        clearAllZonesButton.setVisible (! legacyModeEnabled);

        legacyStartChannel.setVisible (legacyModeEnabled);
        legacyEndChannel.setVisible (legacyModeEnabled);
        legacyPitchbendRange.setVisible (legacyModeEnabled);

        if (areLegacyModeParametersValid())
        {
            listeners.call (&MPESetupComponent::Listener::legacyModeChanged,
                            legacyModeEnabledToggle.getToggleState(),
                            legacyPitchbendRange.getText().getIntValue(),
                            getLegacyModeChannelRange());
        }
        else
        {
            handleInvalidLegacyModeParameters();
        }
    }

    //==============================================================================
    void voiceStealingEnabledToggleClicked()
    {
        listeners.call (&MPESetupComponent::Listener::voiceStealingEnabledChanged,
                        voiceStealingEnabledToggle.getToggleState());
    }

    //==============================================================================
    void comboBoxChanged (ComboBox* comboBoxThatHasChanged) override
    {
        if (comboBoxThatHasChanged == &numberOfVoices)
        {
            numberOfVoicesChanged();
        }
        else if (legacyModeEnabledToggle.getToggleState() == true)
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
        listeners.call (&MPESetupComponent::Listener::numberOfVoicesChanged,
                        numberOfVoices.getText().getIntValue());
    }

    void legacyModePitchbendRangeChanged()
    {
        jassert (legacyModeEnabledToggle.getToggleState() == true);

        listeners.call (&MPESetupComponent::Listener::legacyModeChanged, true,
                        legacyPitchbendRange.getText().getIntValue(),
                        getLegacyModeChannelRange());
    }

    void legacyModeChannelRangeChanged()
    {
        jassert (legacyModeEnabledToggle.getToggleState() == true);

        if (areLegacyModeParametersValid())
        {
            listeners.call (&MPESetupComponent::Listener::legacyModeChanged, true,
                            legacyPitchbendRange.getText().getIntValue(),
                            getLegacyModeChannelRange());
        }
        else
        {
            handleInvalidLegacyModeParameters();
        }
    }

    //==============================================================================
    bool areMPEParametersValid() const
    {
        int maxPossibleNumNoteChannels = 16 - masterChannel.getText().getIntValue();
        return noteChannels.getText().getIntValue() <= maxPossibleNumNoteChannels;
    }

    void handleInvalidMPEParameters() const
    {
        AlertWindow::showMessageBoxAsync (AlertWindow::WarningIcon,
                                          "Invalid zone layout",
                                          "Cannot create MPE zone:\n"
                                          "Invalid zone parameters selected!",
                                          "Got it");
    }

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
        return Range<int> (legacyStartChannel.getText().getIntValue(),
                           legacyEndChannel.getText().getIntValue() + 1);
    }

    //==============================================================================
    MPEZoneLayout zoneLayout;

    ComboBox masterChannel, noteChannels, masterPitchbendRange, notePitchbendRange;
    Label masterChannelLabel, noteChannelsLabel, masterPitchbendRangeLabel, notePitchbendRangeLabel;
    TextButton addZoneButton, clearAllZonesButton;

    ComboBox legacyStartChannel, legacyEndChannel, legacyPitchbendRange;
    Label legacyStartChannelLabel, legacyEndChannelLabel, legacyPitchbendRangeLabel;

    ToggleButton legacyModeEnabledToggle, voiceStealingEnabledToggle;
    ComboBox numberOfVoices;
    Label numberOfVoicesLabel;

    ListenerList<Listener> listeners;

    const int defaultMasterChannel = 1, defaultNoteChannels = 15,
              defaultMasterPitchbendRange = 2, defaultNotePitchbendRange = 48;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MPESetupComponent)

};


#endif  // MPESETUPCOMPONENT_H_INCLUDED

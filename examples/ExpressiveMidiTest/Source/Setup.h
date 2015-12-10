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


#ifndef SETUP_H_INCLUDED
#define SETUP_H_INCLUDED

//==============================================================================
struct Utilities
{
    static Colour getZoneColour (int index) noexcept
    {
        return Colours::red; // TIMUR TODO: use different colours for different zones!
    }

};

//==============================================================================
class ExpressiveMidiSetupComponent : public Component,
                                     public ChangeBroadcaster,
                                     private Button::Listener
{
public:
    //==========================================================================
    ExpressiveMidiSetupComponent()
        : masterChannelLabel (String::empty, "Master channel:"),
          noteChannelsLabel (String::empty, "Nr. of note channels:"),
          masterPitchbendRangeLabel (String::empty, "Master pitchbend range (semitones):"),
          notePitchbendRangeLabel (String::empty, "Note pitchbend range (semitones):"),
          addZoneButton ("Add this zone"),
          clearAllZonesButton ("Clear all zones")
    {

        initialiseComboBoxWithConsecutiveIntegers (masterChannel, masterChannelLabel, 1, 15, defaultMasterChannel);
        initialiseComboBoxWithConsecutiveIntegers (noteChannels, noteChannelsLabel, 1, 15, defaultNoteChannels);
        initialiseComboBoxWithConsecutiveIntegers (masterPitchbendRange, masterPitchbendRangeLabel, 0, 96, defaultMasterPitchbendRange);
        initialiseComboBoxWithConsecutiveIntegers (notePitchbendRange, notePitchbendRangeLabel, 0, 96, defaultNotePitchbendRange);

        initialiseTextButton (addZoneButton);
        initialiseTextButton (clearAllZonesButton);
    }

    //==========================================================================
    void resized() override
    {
        Rectangle<int> r (proportionOfWidth (0.65f), 15, proportionOfWidth (0.25f), 3000);
        const int h = 24;
        const int space = h / 4;

        masterChannel.setBounds (r.removeFromTop (h));
        r.removeFromTop (space);
        noteChannels.setBounds (r.removeFromTop (h));
        r.removeFromTop (space);
        masterPitchbendRange.setBounds (r.removeFromTop (h));
        r.removeFromTop (space);
        notePitchbendRange.setBounds (r.removeFromTop (h));

        r.removeFromTop (18);
        r.setLeft (proportionOfWidth (0.5f));

        addZoneButton.setBounds (r.removeFromTop (h));
        r.removeFromTop (space);
        clearAllZonesButton.setBounds (r.removeFromTop (h));
    }

    //==========================================================================
    class Listener
    {
    public:
        virtual ~Listener() {}
        virtual void expressiveMidiZoneLayoutChanged (ExpressiveMidiZoneLayout newLayout) = 0;
    };

    void addListener (Listener* listenerToAdd)
    {
        listeners.add (listenerToAdd);

        if (zoneLayout.getNumZones() > 0)
            // make the new listener immediately aware of current zone layout
            listenerToAdd->expressiveMidiZoneLayoutChanged (zoneLayout);
    }

    void removeListener (Listener* listenerToRemove)
    {
        listeners.remove (listenerToRemove);
    }

private:
    //==========================================================================
    void initialiseComboBoxWithConsecutiveIntegers (ComboBox& comboBox, Label& labelToAttach, int firstValue, int numValues, int valueToSelect)
    {
        addAndMakeVisible (comboBox);

        for (int i = 0; i < numValues; ++i)
            comboBox.addItem (String (i + firstValue), i + 1);

        comboBox.setSelectedId (valueToSelect - firstValue + 1);
        labelToAttach.attachToComponent (&comboBox, true);
    }

    //==========================================================================
    void initialiseTextButton (TextButton& button)
    {
        addAndMakeVisible (button);
        button.addListener (this);
    }

    //==========================================================================
    void buttonClicked (Button* button) override
    {
        if (button == &addZoneButton)
            addZoneButtonClicked();
        else if (button == &clearAllZonesButton)
            clearAllZonesButtonClicked();
    }

    //==========================================================================
    void addZoneButtonClicked()
    {
        if (selectedZoneParametersValid())
        {
            ExpressiveMidiZone newZone (masterChannel.getText().getIntValue(),
                                        noteChannels.getText().getIntValue(),
                                        notePitchbendRange.getText().getIntValue(),
                                        masterPitchbendRange.getText().getIntValue());

            zoneLayout.addZone (newZone);
            listeners.call (&Listener::expressiveMidiZoneLayoutChanged, zoneLayout);
        }
        else
        {
            handleInvalidNrOfNoteChannels();
        }
    }

    //==========================================================================
    void clearAllZonesButtonClicked()
    {
        zoneLayout.clearAllZones();
        listeners.call (&Listener::expressiveMidiZoneLayoutChanged, zoneLayout);
    }

    //==========================================================================
    bool selectedZoneParametersValid() const
    {
        int maxPossibleNumNoteChannels = 16 - masterChannel.getText().getIntValue();
        return noteChannels.getText().getIntValue() <= maxPossibleNumNoteChannels;
    }

    //==========================================================================
    void handleInvalidNrOfNoteChannels() const
    {
        AlertWindow::showMessageBoxAsync (AlertWindow::WarningIcon,
                                          "Invalid zone layout",
                                          "Cannot create Expressive MIDI zone:\n"
                                          "Invalid zone parameters selected!",
                                          "Got it");
    }

    //==========================================================================
    ExpressiveMidiZoneLayout zoneLayout;
    ComboBox masterChannel, noteChannels, masterPitchbendRange, notePitchbendRange;
    Label masterChannelLabel, noteChannelsLabel, masterPitchbendRangeLabel, notePitchbendRangeLabel;
    TextButton addZoneButton, clearAllZonesButton;
    ListenerList<Listener> listeners;

    const int defaultMasterChannel = 1, defaultNoteChannels = 15,
              defaultMasterPitchbendRange = 2, defaultNotePitchbendRange = 48;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ExpressiveMidiSetupComponent)

};


//==============================================================================
class ZoneLayoutComponent : public Component,
                            public ExpressiveMidiSetupComponent::Listener
{
public:
    //==========================================================================
    void paint (Graphics& g) override
    {
        g.setColour (Colours::black);
        float channelWidth = getChannelRectangleWidth();

        for (int i = 0; i < numMidiChannels; ++i)
        {
            float x = float (i) * channelWidth;
            Rectangle<int> channelArea (x, 0.0f, channelWidth, getHeight());

            Line<float> line (x, 0.0f, x, float (getHeight()));
            g.drawLine (line);
            g.drawText (String (i + 1), channelArea.reduced (4, 4), Justification::topLeft, false);
        }

        paintZones (g);
    }

    //==========================================================================
    void paintZones (Graphics& g)
    {
        float channelWidth = getChannelRectangleWidth();

        for (int i = 0; i < zoneLayout.getNumZones(); ++i)
        {
            ExpressiveMidiZone zone = zoneLayout.getZone (i);
            Rectangle<int> zoneRect (getChannelRectangleWidth() * (zone.getMasterChannel() - 1), 0,
                                     getChannelRectangleWidth() * (zone.getNumNoteChannels() + 1), getHeight());
            zoneRect.removeFromTop (20);

            g.setColour (Utilities::getZoneColour (i).withAlpha (0.3f));
            g.fillRect (zoneRect.withWidth (channelWidth));

            g.setColour (Utilities::getZoneColour (i));
            g.drawRect (zoneRect, 3);
            g.drawText ("<>" + String (zone.getPerNotePitchbendRange()), zoneRect.withTrimmedLeft (channelWidth).reduced (4, 4), Justification::bottomLeft, false);

            g.setColour (Colours::black);
            g.drawText ("ZONE " + String (i + 1), zoneRect.reduced (4, 4), Justification::topLeft, false);
            g.drawText ("<>" + String (zone.getMasterPitchbendRange()), zoneRect.reduced (4, 4), Justification::bottomLeft, false);
        }
    }

    //==========================================================================
    void expressiveMidiZoneLayoutChanged (ExpressiveMidiZoneLayout newLayout) override
    {
        zoneLayout = newLayout;
        repaint();
    }

private:
    //==========================================================================
    float getChannelRectangleWidth() const noexcept
    {
        return float (getWidth()) / numMidiChannels;
    }

    //==========================================================================
    ExpressiveMidiZoneLayout zoneLayout;
    const int numMidiChannels = 16;
};


#endif  // SETUP_H_INCLUDED

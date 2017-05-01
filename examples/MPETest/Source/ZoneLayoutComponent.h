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
    void zoneAdded (MPEZone newZone) override
    {
        zoneLayout.addZone (newZone);
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
        float channelWidth = getChannelRectangleWidth();

        for (int i = 0; i < numMidiChannels; ++i)
        {
            float x = float (i) * channelWidth;
            Rectangle<int> channelArea ((int) x, 0, (int) channelWidth, getHeight());

            Line<float> line (x, 0.0f, x, float (getHeight()));
            g.drawLine (line);
            g.drawText (String (i + 1), channelArea.reduced (4, 4), Justification::topLeft, false);
        }
    }

    //==============================================================================
    void paintZones (Graphics& g)
    {
        float channelWidth = getChannelRectangleWidth();

        for (int i = 0; i < zoneLayout.getNumZones(); ++i)
        {
            MPEZone zone = *zoneLayout.getZoneByIndex (i);
            Colour zoneColour = colourPicker.getColourForZoneIndex (i);

            Rectangle<int> zoneRect (int (getChannelRectangleWidth() * (zone.getMasterChannel() - 1)), 0,
                                     int (getChannelRectangleWidth() * (zone.getNumNoteChannels() + 1)), getHeight());
            zoneRect.removeFromTop (20);

            g.setColour (zoneColour.withAlpha (0.3f));
            g.fillRect (zoneRect.withWidth ((int) channelWidth));

            g.setColour (zoneColour);
            g.drawRect (zoneRect, 3);
            g.drawText ("<>" + String (zone.getPerNotePitchbendRange()), zoneRect.withTrimmedLeft ((int) channelWidth).reduced (4, 4), Justification::bottomLeft, false);

            g.setColour (Colours::black);
            g.drawText ("ZONE " + String (i + 1), zoneRect.reduced (4, 4), Justification::topLeft, false);
            g.drawText ("<>" + String (zone.getMasterPitchbendRange()), zoneRect.reduced (4, 4), Justification::bottomLeft, false);
        }
    }

    //==============================================================================
    void paintLegacyMode (Graphics& g)
    {
        int startChannel = legacyModeChannelRange.getStart() - 1;
        int numChannels = legacyModeChannelRange.getEnd() - startChannel - 1;


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

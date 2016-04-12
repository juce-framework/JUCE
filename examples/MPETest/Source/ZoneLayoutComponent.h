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


#ifndef ZONELAYOUTCOMPONENT_H_INCLUDED
#define ZONELAYOUTCOMPONENT_H_INCLUDED


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


#endif  // ZONELAYOUTCOMPONENT_H_INCLUDED

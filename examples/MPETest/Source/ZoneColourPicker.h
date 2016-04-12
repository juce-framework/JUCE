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


#ifndef ZONECOLOURPICKER_H_INCLUDED
#define ZONECOLOURPICKER_H_INCLUDED


class ZoneColourPicker
{
public:
    //==============================================================================
    ZoneColourPicker()
        : legacyModeEnabled (false)
    {
    }

    //==============================================================================
    Colour getColourForMidiChannel (int midiChannel) const noexcept
    {
        if (legacyModeEnabled)
            return Colours::white;

        if (zoneLayout.getNumZones() == 0)
            return Colours::transparentBlack;

        MPEZone* zone = zoneLayout.getZoneByChannel (midiChannel);

        if (zone == nullptr)
            return Colours::transparentBlack;

        return getColourForZoneIndex (std::distance (zoneLayout.getZoneByIndex (0), zone));

    }

    //==============================================================================
    Colour getColourForZoneIndex (int zoneIndex) const noexcept
    {
        if (legacyModeEnabled)
            return Colours::white;

        if (zoneIndex >= zoneLayout.getNumZones())
            return Colours::transparentBlack;

        static const std::array<Colour, 8> colours = {
            Colours::red,
            Colours::yellow,
            Colours::blue,
            Colours::magenta,
            Colours::limegreen,
            Colours::cyan,
            Colours::orange,
            Colours::salmon
        };

        return colours[zoneIndex % colours.size()];
    }

    //==============================================================================
    void setZoneLayout (MPEZoneLayout layout) noexcept       { zoneLayout = layout; }
    void setLegacyModeEnabled (bool shouldBeEnabled) noexcept  { legacyModeEnabled = shouldBeEnabled; }

private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ZoneColourPicker)

    MPEZoneLayout zoneLayout;
    bool legacyModeEnabled;
};


#endif  // ZONECOLOURPICKER_H_INCLUDED

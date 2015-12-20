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
    //==========================================================================
    ZoneColourPicker()
        : omniModeEnabled (false)
    {
    }

    //==========================================================================
    Colour getColourForMidiChannel (int midiChannel) const noexcept
    {
        if (omniModeEnabled)
            return Colours::white;

        if (zoneLayout.getNumZones() == 0)
            return Colours::transparentBlack;

        MPEZone* zone = zoneLayout.getZoneByChannel (midiChannel);

        if (zone == nullptr)
            return Colours::transparentBlack;

        return getColourForZoneIndex (std::distance (zoneLayout.getZoneByIndex (0), zone));

    }

    //==========================================================================
    Colour getColourForZoneIndex (int zoneIndex) const noexcept
    {
        if (omniModeEnabled)
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

    //==========================================================================
    void setZoneLayout (MPEZoneLayout layout) noexcept       { zoneLayout = layout; }
    void setOmniModeEnabled (bool shouldBeEnabled) noexcept  { omniModeEnabled = shouldBeEnabled; }

private:
    //==========================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ZoneColourPicker)

    MPEZoneLayout zoneLayout;
    bool omniModeEnabled;
};


#endif  // ZONECOLOURPICKER_H_INCLUDED

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

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


//==============================================================================
struct IconButton    : public Button
{
    IconButton (String name, const Path* p)
        : Button (name),
          icon (p, Colours::transparentBlack)
    {
        lookAndFeelChanged();
        setTooltip (name);
    }

    void paintButton (Graphics& g, bool isMouseOverButton, bool isButtonDown) override
    {
        auto alpha = 1.0f;
        if (! isEnabled())
        {
            isMouseOverButton = false;
            isButtonDown = false;

            alpha = 0.2f;
        }

        auto backgroundColour = isIDEButton ? Colours::white
                                            : isUserButton ? findColour (userButtonBackgroundColourId)
                                                           : findColour (defaultButtonBackgroundColourId);

        backgroundColour = isButtonDown ? backgroundColour.darker (0.5f)
                                        : isMouseOverButton ? backgroundColour.darker (0.2f)
                                                            : backgroundColour;

        auto bounds = getLocalBounds().toFloat();

        if (isButtonDown)
            bounds.reduce (2, 2);

        Path ellipse;
        ellipse.addEllipse (bounds);
        g.reduceClipRegion(ellipse);

        g.setColour (backgroundColour.withAlpha (alpha));
        g.fillAll();

        if (iconImage != Image())
        {
            if (isIDEButton)
                bounds.reduce (7, 7);

            g.setOpacity (alpha);
            g.drawImage (iconImage, bounds, RectanglePlacement::fillDestination, false);
        }
        else
        {
            icon.withColour (findColour (defaultIconColourId).withAlpha (alpha)).draw (g, bounds.reduced (2, 2), false);
        }
    }

    Icon icon;
    Image iconImage;

    bool isIDEButton = false;
    bool isUserButton = false;
};

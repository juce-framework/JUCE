/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#pragma once


//==============================================================================
class IconButton  : public Button
{
public:
    IconButton (String buttonName, Image imageToDisplay)
        : Button (buttonName),
          iconImage (imageToDisplay)
    {
        setTooltip (buttonName);
    }

    IconButton (String buttonName, Path pathToDisplay)
        : Button (buttonName),
          iconPath (pathToDisplay),
          iconImage (createImageFromPath (iconPath))
    {
        setTooltip (buttonName);
    }

    void setImage (Image newImage)
    {
        iconImage = newImage;
        repaint();
    }

    void setPath (Path newPath)
    {
        iconImage = createImageFromPath (newPath);
        repaint();
    }

    void setBackgroundColour (Colour backgroundColourToUse)
    {
        backgroundColour = backgroundColourToUse;
        usingNonDefaultBackgroundColour = true;
    }

    void setIconInset (int newIconInset)
    {
        iconInset = newIconInset;
        repaint();
    }

    void paintButton (Graphics& g, bool isMouseOverButton, bool isButtonDown) override
    {
        float alpha = 1.0f;

        if (! isEnabled())
        {
            isMouseOverButton = false;
            isButtonDown = false;

            alpha = 0.2f;
        }

        auto fill = isButtonDown ? backgroundColour.darker (0.5f)
                                 : isMouseOverButton ? backgroundColour.darker (0.2f)
                                                     : backgroundColour;

        auto bounds = getLocalBounds();

        if (isButtonDown)
            bounds.reduce (2, 2);

        Path ellipse;
        ellipse.addEllipse (bounds.toFloat());
        g.reduceClipRegion (ellipse);

        g.setColour (fill.withAlpha (alpha));
        g.fillAll();

        g.setOpacity (alpha);
        g.drawImage (iconImage, bounds.reduced (iconInset).toFloat(), RectanglePlacement::fillDestination, false);
    }

private:
    void lookAndFeelChanged() override
    {
        if (! usingNonDefaultBackgroundColour)
            backgroundColour = findColour (defaultButtonBackgroundColourId);

        if (iconPath != Path())
            iconImage = createImageFromPath (iconPath);

        repaint();
    }

    Image createImageFromPath (Path path)
    {
        Image image (Image::ARGB, 250, 250, true);
        Graphics g (image);

        g.setColour (findColour (defaultIconColourId));

        g.fillPath (path, RectanglePlacement (RectanglePlacement::centred)
                            .getTransformToFit (path.getBounds(), image.getBounds().toFloat()));

        return image;
    }

    Path iconPath;
    Image iconImage;
    Colour backgroundColour { findColour (defaultButtonBackgroundColourId) };
    bool usingNonDefaultBackgroundColour = false;
    int iconInset = 2;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (IconButton)
};

/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

#pragma once


//==============================================================================
class IconButton final : public Button
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

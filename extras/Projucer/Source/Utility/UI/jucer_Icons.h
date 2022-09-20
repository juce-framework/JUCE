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
struct Icon
{
    Icon() = default;

    Icon (const Path& pathToUse, Colour pathColour)
        : path (pathToUse),
          colour (pathColour)
    {
    }

    void draw (Graphics& g, const juce::Rectangle<float>& area, bool isCrossedOut) const
    {
        if (! path.isEmpty())
        {
            g.setColour (colour);

            const RectanglePlacement placement (RectanglePlacement::centred | RectanglePlacement::onlyReduceInSize);
            g.fillPath (path, placement.getTransformToFit (path.getBounds(), area));

            if (isCrossedOut)
            {
                g.setColour (Colours::red.withAlpha (0.8f));
                g.drawLine ((float) area.getX(), area.getY() + area.getHeight() * 0.2f,
                            (float) area.getRight(), area.getY() + area.getHeight() * 0.8f, 3.0f);
            }
        }
    }

    Icon withContrastingColourTo (Colour background) const
    {
        return Icon (path, background.contrasting (colour, 0.6f));
    }

    Icon withColour (Colour newColour)
    {
        return Icon (path, newColour);
    }

    Path path;
    Colour colour;
};

//==============================================================================
class Icons
{
public:
    Icons();

    Path imageDoc, config, graph, info, warning, user, closedFolder, exporter, fileExplorer, file,
         modules, openFolder, settings, singleModule, plus, android, codeBlocks,
         linux, xcode, visualStudio;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Icons)
};

const Icons& getIcons();

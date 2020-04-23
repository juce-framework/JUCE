/*
  ==============================================================================

   This file is part of the JUCE 6 technical preview.
   Copyright (c) 2020 - Raw Material Software Limited

   You may use this code under the terms of the GPL v3
   (see www.gnu.org/licenses).

   For this technical preview, this file is not subject to commercial licensing.

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

    Path folder, document, imageDoc, config, juceLogo, graph, jigsaw, info, warning, bug,
         code, box, mainJuceLogo, user, closedFolder, exporter, fileExplorer, file, buildTab,
         modules, openFolder, play, settings, singleModule, edit, plus, android, codeBlocks,
         linux, xcode, visualStudio, clion;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Icons)
};

#ifndef BUILDING_JUCE_COMPILEENGINE
 const Icons& getIcons();
#endif

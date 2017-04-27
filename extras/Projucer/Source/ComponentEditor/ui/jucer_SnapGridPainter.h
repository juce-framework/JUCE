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

#include "../jucer_JucerDocument.h"
#include "../jucer_PaintRoutine.h"


//==============================================================================
class SnapGridPainter
{
public:
    SnapGridPainter()
        : snapGridSize (-1), snapShown (false)
    {
    }

    bool updateFromDesign (JucerDocument& design)
    {
        if (snapGridSize != design.getSnappingGridSize()
             || snapShown != (design.isSnapShown() && design.isSnapActive (false)))
        {
            snapGridSize = design.getSnappingGridSize();
            snapShown = design.isSnapShown() && design.isSnapActive (false);
            return true;
        }

        return false;
    }

    void draw (Graphics& g, PaintRoutine* backgroundGraphics)
    {
        if (snapShown && snapGridSize > 2)
        {
            Colour col (Colours::black);

            if (backgroundGraphics != nullptr)
                col = backgroundGraphics->getBackgroundColour().contrasting();

            const Rectangle<int> clip (g.getClipBounds());

            RectangleList<float> gridLines;

            for (int x = clip.getX() - (clip.getX() % snapGridSize); x < clip.getRight(); x += snapGridSize)
                gridLines.addWithoutMerging (Rectangle<float> ((float) x, 0.0f, 1.0f, (float) clip.getBottom()));

            for (int y = clip.getY() - (clip.getY() % snapGridSize); y < clip.getBottom(); y += snapGridSize)
                gridLines.addWithoutMerging (Rectangle<float> (0.0f, (float) y, (float) clip.getRight(), 1.0f));

            g.setColour (col.withAlpha (0.1f));
            g.fillRectList (gridLines);
        }
    }

private:
    int snapGridSize;
    bool snapShown;
};

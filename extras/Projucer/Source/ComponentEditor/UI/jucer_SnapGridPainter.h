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

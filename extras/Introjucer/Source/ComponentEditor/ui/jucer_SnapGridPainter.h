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

#ifndef JUCER_SNAPGRIDPAINTER_H_INCLUDED
#define JUCER_SNAPGRIDPAINTER_H_INCLUDED

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


#endif   // JUCER_SNAPGRIDPAINTER_H_INCLUDED

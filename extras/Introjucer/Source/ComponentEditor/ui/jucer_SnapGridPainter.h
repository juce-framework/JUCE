/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

#ifndef __JUCER_SNAPGRIDPAINTER_JUCEHEADER__
#define __JUCER_SNAPGRIDPAINTER_JUCEHEADER__

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

            backgroundFill = Image();
            return true;
        }

        return false;
    }

    void updateColour()
    {
        backgroundFill = Image();
    }

    void draw (Graphics& g, PaintRoutine* backgroundGraphics)
    {
        if (backgroundFill.isNull() && snapShown)
        {
            backgroundFill = Image (Image::ARGB, snapGridSize, snapGridSize, true);

            Graphics g2 (backgroundFill);

            Colour col (Colours::black);

            if (backgroundGraphics != nullptr)
                col = backgroundGraphics->getBackgroundColour().contrasting();

            if (snapGridSize > 2)
            {
                g2.setColour (col.withAlpha (0.1f));
                g2.drawRect (0, 0, snapGridSize + 1, snapGridSize + 1);
            }
        }

        if (backgroundFill.isValid())
        {
            g.setTiledImageFill (backgroundFill, 0, 0, 1.0f);
            g.fillAll();
        }
    }

private:
    int snapGridSize;
    bool snapShown;
    Image backgroundFill;
};


#endif   // __JUCER_SNAPGRIDPAINTER_JUCEHEADER__

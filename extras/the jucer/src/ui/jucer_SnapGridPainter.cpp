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

#include "../jucer_Headers.h"
#include "jucer_SnapGridPainter.h"


//==============================================================================
SnapGridPainter::SnapGridPainter()
    : snapGridSize (-1),
      snapShown (false),
      backgroundFill (0)
{
}

SnapGridPainter::~SnapGridPainter()
{
}

bool SnapGridPainter::updateFromDesign (JucerDocument& design)
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

void SnapGridPainter::updateColour()
{
    backgroundFill = Image();
}

void SnapGridPainter::draw (Graphics& g, PaintRoutine* backgroundGraphics)
{
    if (backgroundFill.isNull() && snapShown)
    {
        backgroundFill = Image (Image::ARGB, snapGridSize, snapGridSize, true);

        Graphics g (backgroundFill);

        Colour col (Colours::black);

        if (backgroundGraphics != 0)
            col = backgroundGraphics->getBackgroundColour().contrasting();

        if (snapGridSize > 2)
        {
            g.setColour (col.withAlpha (0.1f));
            g.drawRect (0, 0, snapGridSize + 1, snapGridSize + 1);
        }

        g.setColour (col.withAlpha (0.35f));
        g.setPixel (0, 0);
    }

    if (backgroundFill.isValid())
    {
        g.setTiledImageFill (backgroundFill, 0, 0, 1.0f);
        g.fillAll();
    }
}

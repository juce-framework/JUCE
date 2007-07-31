/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

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
    delete backgroundFill;
}

bool SnapGridPainter::updateFromDesign (JucerDocument& design)
{
    if (snapGridSize != design.getSnappingGridSize()
         || snapShown != (design.isSnapShown() && design.isSnapActive (false)))
    {
        snapGridSize = design.getSnappingGridSize();
        snapShown = design.isSnapShown() && design.isSnapActive (false);

        deleteAndZero (backgroundFill);
        return true;
    }

    return false;
}

void SnapGridPainter::updateColour()
{
    deleteAndZero (backgroundFill);
}

void SnapGridPainter::draw (Graphics& g, PaintRoutine* backgroundGraphics)
{
    if (backgroundFill == 0 && snapShown)
    {
        backgroundFill = new Image (Image::ARGB, snapGridSize, snapGridSize, true);

        Graphics g (*backgroundFill);

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

    if (backgroundFill != 0)
    {
        ImageBrush ib (backgroundFill, 0, 0, 1.0f);
        g.setBrush (&ib);
        g.fillAll();
    }
}

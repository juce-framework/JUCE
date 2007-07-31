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

#ifndef __JUCER_SNAPGRIDPAINTER_JUCEHEADER__
#define __JUCER_SNAPGRIDPAINTER_JUCEHEADER__

#include "../model/jucer_JucerDocument.h"
#include "../model/jucer_PaintRoutine.h"


//==============================================================================
/**
*/
class SnapGridPainter
{
public:
    //==============================================================================
    SnapGridPainter();
    ~SnapGridPainter();

    //==============================================================================
    void draw (Graphics& g, PaintRoutine* backgroundGraphics);

    bool updateFromDesign (JucerDocument& design);
    void updateColour();

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    int snapGridSize;
    bool snapShown;
    Image* backgroundFill;
};


#endif   // __JUCER_SNAPGRIDPAINTER_JUCEHEADER__

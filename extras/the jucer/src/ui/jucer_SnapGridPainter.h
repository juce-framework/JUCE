/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

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

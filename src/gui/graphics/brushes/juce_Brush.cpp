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

#include "../../../core/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE


#include "juce_Brush.h"
#include "../contexts/juce_Graphics.h"
#include "../contexts/juce_EdgeTable.h"


//==============================================================================
Brush::Brush() throw()
{
}

Brush::~Brush() throw()
{
}

void Brush::paintVerticalLine (LowLevelGraphicsContext& context,
                               int x, float y1, float y2) throw()
{
    Path p;
    p.addRectangle ((float) x, y1, 1.0f, y2 - y1);
    paintPath (context, p, AffineTransform::identity);
}

void Brush::paintHorizontalLine (LowLevelGraphicsContext& context,
                                 int y, float x1, float x2) throw()
{
    Path p;
    p.addRectangle (x1, (float) y, x2 - x1, 1.0f);
    paintPath (context, p, AffineTransform::identity);
}

void Brush::paintLine (LowLevelGraphicsContext& context,
                       float x1, float y1, float x2, float y2) throw()
{
    Path p;
    p.addLineSegment (x1, y1, x2, y2, 1.0f);
    paintPath (context, p, AffineTransform::identity);
}


END_JUCE_NAMESPACE

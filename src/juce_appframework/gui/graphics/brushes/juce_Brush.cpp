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

#include "../../../../juce_core/basics/juce_StandardHeader.h"

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

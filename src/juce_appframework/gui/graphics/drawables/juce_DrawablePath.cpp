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

#include "../../../../juce_core/basics/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE


#include "juce_DrawablePath.h"
#include "../brushes/juce_SolidColourBrush.h"


//==============================================================================
DrawablePath::DrawablePath()
    : fillBrush (new SolidColourBrush (Colours::black)),
      strokeBrush (0),
      strokeType (0.0f)
{
}

DrawablePath::~DrawablePath()
{
    delete fillBrush;
    delete strokeBrush;
}

//==============================================================================
void DrawablePath::setPath (const Path& newPath)
{
    path = newPath;
    updateOutline();
}

void DrawablePath::setSolidFill (const Colour& newColour)
{
    delete fillBrush;
    fillBrush = new SolidColourBrush (newColour);
}

void DrawablePath::setFillBrush (const Brush& newBrush)
{
    delete fillBrush;
    fillBrush = newBrush.createCopy();
}

void DrawablePath::setOutline (const float thickness, const Colour& colour)
{
    strokeType = PathStrokeType (thickness);
    delete strokeBrush;
    strokeBrush = new SolidColourBrush (colour);
    updateOutline();
}

void DrawablePath::setOutline (const PathStrokeType& strokeType_, const Brush& newStrokeBrush)
{
    strokeType = strokeType_;
    delete strokeBrush;
    strokeBrush = newStrokeBrush.createCopy();
    updateOutline();
}


//==============================================================================
void DrawablePath::draw (Graphics& g, const AffineTransform& transform) const
{
    const Colour oldColour (g.getCurrentColour()); // save this so we can restore it later
    const float currentOpacity = oldColour.getFloatAlpha();

    {
        Brush* const tempBrush = fillBrush->createCopy();
        tempBrush->applyTransform (transform);
        tempBrush->multiplyOpacity (currentOpacity);

        g.setBrush (tempBrush);
        g.fillPath (path, transform);

        delete tempBrush;
    }

    if (strokeBrush != 0 && strokeType.getStrokeThickness() > 0.0f)
    {
        Brush* const tempBrush = strokeBrush->createCopy();
        tempBrush->applyTransform (transform);
        tempBrush->multiplyOpacity (currentOpacity);

        g.setBrush (tempBrush);
        g.fillPath (outline, transform);

        delete tempBrush;
    }

    g.setColour (oldColour);
}

void DrawablePath::updateOutline()
{
    outline.clear();
    strokeType.createStrokedPath (outline, path, AffineTransform::identity, 4.0f);
}

void DrawablePath::getBounds (float& x, float& y, float& width, float& height) const
{
    if (strokeType.getStrokeThickness() > 0.0f)
        outline.getBounds (x, y, width, height);
    else
        path.getBounds (x, y, width, height);
}

bool DrawablePath::hitTest (float x, float y) const
{
    return path.contains (x, y)
        || outline.contains (x, y);
}

Drawable* DrawablePath::createCopy() const
{
    DrawablePath* const dp = new DrawablePath();

    dp->path = path;
    dp->setFillBrush (*fillBrush);

    if (strokeBrush != 0)
        dp->setOutline (strokeType, *strokeBrush);

    return dp;
}


END_JUCE_NAMESPACE

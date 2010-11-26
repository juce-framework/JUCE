/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-10 by Raw Material Software Ltd.

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

#include "juce_DrawableRectangle.h"
#include "juce_DrawableComposite.h"

//==============================================================================
DrawableRectangle::DrawableRectangle()
{
}

DrawableRectangle::DrawableRectangle (const DrawableRectangle& other)
    : DrawableShape (other)
{
}

DrawableRectangle::~DrawableRectangle()
{
}

Drawable* DrawableRectangle::createCopy() const
{
    return new DrawableRectangle (*this);
}

//==============================================================================
void DrawableRectangle::setRectangle (const RelativeParallelogram& newBounds)
{
    bounds = newBounds;
    pathChanged();
    strokeChanged();
}

void DrawableRectangle::setCornerSize (const RelativePoint& newSize)
{
    cornerSize = newSize;
    pathChanged();
    strokeChanged();
}

//==============================================================================
bool DrawableRectangle::rebuildPath (Path& path) const
{
    Point<float> points[3];
    bounds.resolveThreePoints (points, getParent());

    const float w = Line<float> (points[0], points[1]).getLength();
    const float h = Line<float> (points[0], points[2]).getLength();

    const float cornerSizeX = (float) cornerSize.x.resolve (getParent());
    const float cornerSizeY = (float) cornerSize.y.resolve (getParent());

    path.clear();

    if (cornerSizeX > 0 && cornerSizeY > 0)
        path.addRoundedRectangle (0, 0, w, h, cornerSizeX, cornerSizeY);
    else
        path.addRectangle (0, 0, w, h);

    path.applyTransform (AffineTransform::fromTargetPoints (0, 0, points[0].getX(), points[0].getY(),
                                                            w, 0, points[1].getX(), points[1].getY(),
                                                            0, h, points[2].getX(), points[2].getY()));
    return true;
}

const AffineTransform DrawableRectangle::calculateTransform() const
{
    Point<float> resolved[3];
    bounds.resolveThreePoints (resolved, getParent());

    return AffineTransform::fromTargetPoints (resolved[0].getX(), resolved[0].getY(),
                                              resolved[1].getX(), resolved[1].getY(),
                                              resolved[2].getX(), resolved[2].getY());
}

//==============================================================================
const Identifier DrawableRectangle::valueTreeType ("Rectangle");
const Identifier DrawableRectangle::ValueTreeWrapper::topLeft ("topLeft");
const Identifier DrawableRectangle::ValueTreeWrapper::topRight ("topRight");
const Identifier DrawableRectangle::ValueTreeWrapper::bottomLeft ("bottomLeft");
const Identifier DrawableRectangle::ValueTreeWrapper::cornerSize ("cornerSize");

//==============================================================================
DrawableRectangle::ValueTreeWrapper::ValueTreeWrapper (const ValueTree& state_)
    : FillAndStrokeState (state_)
{
    jassert (state.hasType (valueTreeType));
}

const RelativeParallelogram DrawableRectangle::ValueTreeWrapper::getRectangle() const
{
    return RelativeParallelogram (state.getProperty (topLeft, "0, 0"),
                                  state.getProperty (topRight, "100, 0"),
                                  state.getProperty (bottomLeft, "0, 100"));
}

void DrawableRectangle::ValueTreeWrapper::setRectangle (const RelativeParallelogram& newBounds, UndoManager* undoManager)
{
    state.setProperty (topLeft, newBounds.topLeft.toString(), undoManager);
    state.setProperty (topRight, newBounds.topRight.toString(), undoManager);
    state.setProperty (bottomLeft, newBounds.bottomLeft.toString(), undoManager);
}

void DrawableRectangle::ValueTreeWrapper::setCornerSize (const RelativePoint& newSize, UndoManager* undoManager)
{
    state.setProperty (cornerSize, newSize.toString(), undoManager);
}

const RelativePoint DrawableRectangle::ValueTreeWrapper::getCornerSize() const
{
    return RelativePoint (state [cornerSize]);
}

Value DrawableRectangle::ValueTreeWrapper::getCornerSizeValue (UndoManager* undoManager) const
{
    return state.getPropertyAsValue (cornerSize, undoManager);
}

//==============================================================================
void DrawableRectangle::refreshFromValueTree (const ValueTree& tree, ImageProvider* imageProvider)
{
    ValueTreeWrapper v (tree);
    setName (v.getID());

    if (refreshFillTypes (v, getParent(), imageProvider))
        repaint();

    RelativeParallelogram newBounds (v.getRectangle());

    const PathStrokeType newStroke (v.getStrokeType());
    const RelativePoint newCornerSize (v.getCornerSize());

    if (strokeType != newStroke || newBounds != bounds || newCornerSize != cornerSize)
    {
        repaint();
        bounds = newBounds;
        strokeType = newStroke;
        cornerSize = newCornerSize;
        pathChanged();
    }
}

const ValueTree DrawableRectangle::createValueTree (ImageProvider* imageProvider) const
{
    ValueTree tree (valueTreeType);
    ValueTreeWrapper v (tree);

    v.setID (getName(), 0);
    writeTo (v, imageProvider, 0);
    v.setRectangle (bounds, 0);
    v.setCornerSize (cornerSize, 0);

    return tree;
}

END_JUCE_NAMESPACE

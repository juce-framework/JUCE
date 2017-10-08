/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

DrawableRectangle::DrawableRectangle()
{
}

DrawableRectangle::DrawableRectangle (const DrawableRectangle& other)
    : DrawableShape (other),
      bounds (other.bounds),
      cornerSize (other.cornerSize)
{
    rebuildPath();
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
    if (bounds != newBounds)
    {
        bounds = newBounds;
        rebuildPath();
    }
}

void DrawableRectangle::setCornerSize (const RelativePoint& newSize)
{
    if (cornerSize != newSize)
    {
        cornerSize = newSize;
        rebuildPath();
    }
}

void DrawableRectangle::rebuildPath()
{
    if (bounds.isDynamic() || cornerSize.isDynamic())
    {
        Drawable::Positioner<DrawableRectangle>* const p = new Drawable::Positioner<DrawableRectangle> (*this);
        setPositioner (p);
        p->apply();
    }
    else
    {
        setPositioner (nullptr);
        recalculateCoordinates (nullptr);
    }
}

bool DrawableRectangle::registerCoordinates (RelativeCoordinatePositionerBase& pos)
{
    bool ok = pos.addPoint (bounds.topLeft);
    ok = pos.addPoint (bounds.topRight) && ok;
    ok = pos.addPoint (bounds.bottomLeft) && ok;
    return pos.addPoint (cornerSize) && ok;
}

void DrawableRectangle::recalculateCoordinates (Expression::Scope* scope)
{
    Point<float> points[3];
    bounds.resolveThreePoints (points, scope);

    const float cornerSizeX = (float) cornerSize.x.resolve (scope);
    const float cornerSizeY = (float) cornerSize.y.resolve (scope);

    const float w = Line<float> (points[0], points[1]).getLength();
    const float h = Line<float> (points[0], points[2]).getLength();

    Path newPath;

    if (cornerSizeX > 0 && cornerSizeY > 0)
        newPath.addRoundedRectangle (0, 0, w, h, cornerSizeX, cornerSizeY);
    else
        newPath.addRectangle (0, 0, w, h);

    newPath.applyTransform (AffineTransform::fromTargetPoints (0, 0, points[0].x, points[0].y,
                                                               w, 0, points[1].x, points[1].y,
                                                               0, h, points[2].x, points[2].y));

    if (path != newPath)
    {
        path.swapWithPath (newPath);
        pathChanged();
    }
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

RelativeParallelogram DrawableRectangle::ValueTreeWrapper::getRectangle() const
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

RelativePoint DrawableRectangle::ValueTreeWrapper::getCornerSize() const
{
    return RelativePoint (state [cornerSize]);
}

Value DrawableRectangle::ValueTreeWrapper::getCornerSizeValue (UndoManager* undoManager)
{
    return state.getPropertyAsValue (cornerSize, undoManager);
}

//==============================================================================
void DrawableRectangle::refreshFromValueTree (const ValueTree& tree, ComponentBuilder& builder)
{
    ValueTreeWrapper v (tree);
    setComponentID (v.getID());

    refreshFillTypes (v, builder.getImageProvider());
    setStrokeType (v.getStrokeType());
    setRectangle (v.getRectangle());
    setCornerSize (v.getCornerSize());
}

ValueTree DrawableRectangle::createValueTree (ComponentBuilder::ImageProvider* imageProvider) const
{
    ValueTree tree (valueTreeType);
    ValueTreeWrapper v (tree);

    v.setID (getComponentID());
    writeTo (v, imageProvider, nullptr);
    v.setRectangle (bounds, nullptr);
    v.setCornerSize (cornerSize, nullptr);

    return tree;
}

} // namespace juce

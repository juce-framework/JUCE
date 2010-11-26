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

#include "juce_DrawablePath.h"
#include "juce_DrawableComposite.h"


//==============================================================================
DrawablePath::DrawablePath()
{
}

DrawablePath::DrawablePath (const DrawablePath& other)
    : DrawableShape (other)
{
    if (other.relativePath != 0)
        relativePath = new RelativePointPath (*other.relativePath);
    else
        setPath (other.path);
}

DrawablePath::~DrawablePath()
{
}

Drawable* DrawablePath::createCopy() const
{
    return new DrawablePath (*this);
}

//==============================================================================
void DrawablePath::setPath (const Path& newPath)
{
    path = newPath;
    pathChanged();
}

const Path& DrawablePath::getPath() const
{
    return path;
}

const Path& DrawablePath::getStrokePath() const
{
    return strokePath;
}

bool DrawablePath::rebuildPath (Path& path) const
{
    if (relativePath != 0)
    {
        path.clear();
        relativePath->createPath (path, getParent());
        return true;
    }

    return false;
}

//==============================================================================
const Identifier DrawablePath::valueTreeType ("Path");

const Identifier DrawablePath::ValueTreeWrapper::nonZeroWinding ("nonZeroWinding");
const Identifier DrawablePath::ValueTreeWrapper::point1 ("p1");
const Identifier DrawablePath::ValueTreeWrapper::point2 ("p2");
const Identifier DrawablePath::ValueTreeWrapper::point3 ("p3");

//==============================================================================
DrawablePath::ValueTreeWrapper::ValueTreeWrapper (const ValueTree& state_)
    : FillAndStrokeState (state_)
{
    jassert (state.hasType (valueTreeType));
}

ValueTree DrawablePath::ValueTreeWrapper::getPathState()
{
    return state.getOrCreateChildWithName (path, 0);
}

bool DrawablePath::ValueTreeWrapper::usesNonZeroWinding() const
{
    return state [nonZeroWinding];
}

void DrawablePath::ValueTreeWrapper::setUsesNonZeroWinding (bool b, UndoManager* undoManager)
{
    state.setProperty (nonZeroWinding, b, undoManager);
}

//==============================================================================
const Identifier DrawablePath::ValueTreeWrapper::Element::mode ("mode");
const Identifier DrawablePath::ValueTreeWrapper::Element::startSubPathElement ("Move");
const Identifier DrawablePath::ValueTreeWrapper::Element::closeSubPathElement ("Close");
const Identifier DrawablePath::ValueTreeWrapper::Element::lineToElement ("Line");
const Identifier DrawablePath::ValueTreeWrapper::Element::quadraticToElement ("Quad");
const Identifier DrawablePath::ValueTreeWrapper::Element::cubicToElement ("Cubic");

const char* DrawablePath::ValueTreeWrapper::Element::cornerMode = "corner";
const char* DrawablePath::ValueTreeWrapper::Element::roundedMode = "round";
const char* DrawablePath::ValueTreeWrapper::Element::symmetricMode = "symm";

DrawablePath::ValueTreeWrapper::Element::Element (const ValueTree& state_)
    : state (state_)
{
}

DrawablePath::ValueTreeWrapper::Element::~Element()
{
}

DrawablePath::ValueTreeWrapper DrawablePath::ValueTreeWrapper::Element::getParent() const
{
    return ValueTreeWrapper (state.getParent().getParent());
}

DrawablePath::ValueTreeWrapper::Element DrawablePath::ValueTreeWrapper::Element::getPreviousElement() const
{
    return Element (state.getSibling (-1));
}

int DrawablePath::ValueTreeWrapper::Element::getNumControlPoints() const throw()
{
    const Identifier i (state.getType());
    if (i == startSubPathElement || i == lineToElement) return 1;
    if (i == quadraticToElement) return 2;
    if (i == cubicToElement) return 3;
    return 0;
}

const RelativePoint DrawablePath::ValueTreeWrapper::Element::getControlPoint (const int index) const
{
    jassert (index >= 0 && index < getNumControlPoints());
    return RelativePoint (state [index == 0 ? point1 : (index == 1 ? point2 : point3)].toString());
}

Value DrawablePath::ValueTreeWrapper::Element::getControlPointValue (int index, UndoManager* undoManager) const
{
    jassert (index >= 0 && index < getNumControlPoints());
    return state.getPropertyAsValue (index == 0 ? point1 : (index == 1 ? point2 : point3), undoManager);
}

void DrawablePath::ValueTreeWrapper::Element::setControlPoint (const int index, const RelativePoint& point, UndoManager* undoManager)
{
    jassert (index >= 0 && index < getNumControlPoints());
    state.setProperty (index == 0 ? point1 : (index == 1 ? point2 : point3), point.toString(), undoManager);
}

const RelativePoint DrawablePath::ValueTreeWrapper::Element::getStartPoint() const
{
    const Identifier i (state.getType());

    if (i == startSubPathElement)
        return getControlPoint (0);

    jassert (i == lineToElement || i == quadraticToElement || i == cubicToElement || i == closeSubPathElement);

    return getPreviousElement().getEndPoint();
}

const RelativePoint DrawablePath::ValueTreeWrapper::Element::getEndPoint() const
{
    const Identifier i (state.getType());
    if (i == startSubPathElement || i == lineToElement)  return getControlPoint (0);
    if (i == quadraticToElement)                         return getControlPoint (1);
    if (i == cubicToElement)                             return getControlPoint (2);

    jassert (i == closeSubPathElement);
    return RelativePoint();
}

float DrawablePath::ValueTreeWrapper::Element::getLength (Expression::EvaluationContext* nameFinder) const
{
    const Identifier i (state.getType());

    if (i == lineToElement || i == closeSubPathElement)
        return getEndPoint().resolve (nameFinder).getDistanceFrom (getStartPoint().resolve (nameFinder));

    if (i == cubicToElement)
    {
        Path p;
        p.startNewSubPath (getStartPoint().resolve (nameFinder));
        p.cubicTo (getControlPoint (0).resolve (nameFinder), getControlPoint (1).resolve (nameFinder), getControlPoint (2).resolve (nameFinder));
        return p.getLength();
    }

    if (i == quadraticToElement)
    {
        Path p;
        p.startNewSubPath (getStartPoint().resolve (nameFinder));
        p.quadraticTo (getControlPoint (0).resolve (nameFinder), getControlPoint (1).resolve (nameFinder));
        return p.getLength();
    }

    jassert (i == startSubPathElement);
    return 0;
}

const String DrawablePath::ValueTreeWrapper::Element::getModeOfEndPoint() const
{
    return state [mode].toString();
}

void DrawablePath::ValueTreeWrapper::Element::setModeOfEndPoint (const String& newMode, UndoManager* undoManager)
{
    if (state.hasType (cubicToElement))
        state.setProperty (mode, newMode, undoManager);
}

void DrawablePath::ValueTreeWrapper::Element::convertToLine (UndoManager* undoManager)
{
    const Identifier i (state.getType());

    if (i == quadraticToElement || i == cubicToElement)
    {
        ValueTree newState (lineToElement);
        Element e (newState);
        e.setControlPoint (0, getEndPoint(), undoManager);
        state = newState;
    }
}

void DrawablePath::ValueTreeWrapper::Element::convertToCubic (Expression::EvaluationContext* nameFinder, UndoManager* undoManager)
{
    const Identifier i (state.getType());

    if (i == lineToElement || i == quadraticToElement)
    {
        ValueTree newState (cubicToElement);
        Element e (newState);

        const RelativePoint start (getStartPoint());
        const RelativePoint end (getEndPoint());
        const Point<float> startResolved (start.resolve (nameFinder));
        const Point<float> endResolved (end.resolve (nameFinder));
        e.setControlPoint (0, startResolved + (endResolved - startResolved) * 0.3f, undoManager);
        e.setControlPoint (1, startResolved + (endResolved - startResolved) * 0.7f, undoManager);
        e.setControlPoint (2, end, undoManager);

        state = newState;
    }
}

void DrawablePath::ValueTreeWrapper::Element::convertToPathBreak (UndoManager* undoManager)
{
    const Identifier i (state.getType());

    if (i != startSubPathElement)
    {
        ValueTree newState (startSubPathElement);
        Element e (newState);
        e.setControlPoint (0, getEndPoint(), undoManager);
        state = newState;
    }
}

namespace DrawablePathHelpers
{
    const Point<float> findCubicSubdivisionPoint (float proportion, const Point<float> points[4])
    {
        const Point<float> mid1 (points[0] + (points[1] - points[0]) * proportion),
                           mid2 (points[1] + (points[2] - points[1]) * proportion),
                           mid3 (points[2] + (points[3] - points[2]) * proportion);

        const Point<float> newCp1 (mid1 + (mid2 - mid1) * proportion),
                           newCp2 (mid2 + (mid3 - mid2) * proportion);

        return newCp1 + (newCp2 - newCp1) * proportion;
    }

    const Point<float> findQuadraticSubdivisionPoint (float proportion, const Point<float> points[3])
    {
        const Point<float> mid1 (points[0] + (points[1] - points[0]) * proportion),
                           mid2 (points[1] + (points[2] - points[1]) * proportion);

        return mid1 + (mid2 - mid1) * proportion;
    }
}

float DrawablePath::ValueTreeWrapper::Element::findProportionAlongLine (const Point<float>& targetPoint, Expression::EvaluationContext* nameFinder) const
{
    using namespace DrawablePathHelpers;
    const Identifier i (state.getType());
    float bestProp = 0;

    if (i == cubicToElement)
    {
        RelativePoint rp1 (getStartPoint()), rp2 (getControlPoint (0)), rp3 (getControlPoint (1)), rp4 (getEndPoint());

        const Point<float> points[] = { rp1.resolve (nameFinder), rp2.resolve (nameFinder), rp3.resolve (nameFinder), rp4.resolve (nameFinder) };

        float bestDistance = std::numeric_limits<float>::max();

        for (int i = 110; --i >= 0;)
        {
            float prop = i > 10 ? ((i - 10) / 100.0f) : (bestProp + ((i - 5) / 1000.0f));
            const Point<float> centre (findCubicSubdivisionPoint (prop, points));
            const float distance = centre.getDistanceFrom (targetPoint);

            if (distance < bestDistance)
            {
                bestProp = prop;
                bestDistance = distance;
            }
        }
    }
    else if (i == quadraticToElement)
    {
        RelativePoint rp1 (getStartPoint()), rp2 (getControlPoint (0)), rp3 (getEndPoint());
        const Point<float> points[] = { rp1.resolve (nameFinder), rp2.resolve (nameFinder), rp3.resolve (nameFinder) };

        float bestDistance = std::numeric_limits<float>::max();

        for (int i = 110; --i >= 0;)
        {
            float prop = i > 10 ? ((i - 10) / 100.0f) : (bestProp + ((i - 5) / 1000.0f));
            const Point<float> centre (findQuadraticSubdivisionPoint ((float) prop, points));
            const float distance = centre.getDistanceFrom (targetPoint);

            if (distance < bestDistance)
            {
                bestProp = prop;
                bestDistance = distance;
            }
        }
    }
    else if (i == lineToElement)
    {
        RelativePoint rp1 (getStartPoint()), rp2 (getEndPoint());
        const Line<float> line (rp1.resolve (nameFinder), rp2.resolve (nameFinder));
        bestProp = line.findNearestProportionalPositionTo (targetPoint);
    }

    return bestProp;
}

ValueTree DrawablePath::ValueTreeWrapper::Element::insertPoint (const Point<float>& targetPoint, Expression::EvaluationContext* nameFinder, UndoManager* undoManager)
{
    ValueTree newTree;
    const Identifier i (state.getType());

    if (i == cubicToElement)
    {
        float bestProp = findProportionAlongLine (targetPoint, nameFinder);

        RelativePoint rp1 (getStartPoint()), rp2 (getControlPoint (0)), rp3 (getControlPoint (1)), rp4 (getEndPoint());
        const Point<float> points[] = { rp1.resolve (nameFinder), rp2.resolve (nameFinder), rp3.resolve (nameFinder), rp4.resolve (nameFinder) };

        const Point<float> mid1 (points[0] + (points[1] - points[0]) * bestProp),
                           mid2 (points[1] + (points[2] - points[1]) * bestProp),
                           mid3 (points[2] + (points[3] - points[2]) * bestProp);

        const Point<float> newCp1 (mid1 + (mid2 - mid1) * bestProp),
                           newCp2 (mid2 + (mid3 - mid2) * bestProp);

        const Point<float> newCentre (newCp1 + (newCp2 - newCp1) * bestProp);

        setControlPoint (0, mid1, undoManager);
        setControlPoint (1, newCp1, undoManager);
        setControlPoint (2, newCentre, undoManager);
        setModeOfEndPoint (roundedMode, undoManager);

        Element newElement (newTree = ValueTree (cubicToElement));
        newElement.setControlPoint (0, newCp2, 0);
        newElement.setControlPoint (1, mid3, 0);
        newElement.setControlPoint (2, rp4, 0);

        state.getParent().addChild (newTree, state.getParent().indexOf (state) + 1, undoManager);
    }
    else if (i == quadraticToElement)
    {
        float bestProp = findProportionAlongLine (targetPoint, nameFinder);

        RelativePoint rp1 (getStartPoint()), rp2 (getControlPoint (0)), rp3 (getEndPoint());
        const Point<float> points[] = { rp1.resolve (nameFinder), rp2.resolve (nameFinder), rp3.resolve (nameFinder) };

        const Point<float> mid1 (points[0] + (points[1] - points[0]) * bestProp),
                           mid2 (points[1] + (points[2] - points[1]) * bestProp);

        const Point<float> newCentre (mid1 + (mid2 - mid1) * bestProp);

        setControlPoint (0, mid1, undoManager);
        setControlPoint (1, newCentre, undoManager);
        setModeOfEndPoint (roundedMode, undoManager);

        Element newElement (newTree = ValueTree (quadraticToElement));
        newElement.setControlPoint (0, mid2, 0);
        newElement.setControlPoint (1, rp3, 0);

        state.getParent().addChild (newTree, state.getParent().indexOf (state) + 1, undoManager);
    }
    else if (i == lineToElement)
    {
        RelativePoint rp1 (getStartPoint()), rp2 (getEndPoint());
        const Line<float> line (rp1.resolve (nameFinder), rp2.resolve (nameFinder));
        const Point<float> newPoint (line.findNearestPointTo (targetPoint));

        setControlPoint (0, newPoint, undoManager);

        Element newElement (newTree = ValueTree (lineToElement));
        newElement.setControlPoint (0, rp2, 0);

        state.getParent().addChild (newTree, state.getParent().indexOf (state) + 1, undoManager);
    }
    else if (i == closeSubPathElement)
    {
    }

    return newTree;
}

void DrawablePath::ValueTreeWrapper::Element::removePoint (UndoManager* undoManager)
{
    state.getParent().removeChild (state, undoManager);
}

//==============================================================================
void DrawablePath::refreshFromValueTree (const ValueTree& tree, ImageProvider* imageProvider)
{
    ValueTreeWrapper v (tree);
    setName (v.getID());

    if (refreshFillTypes (v, getParent(), imageProvider))
        repaint();

    ScopedPointer<RelativePointPath> newRelativePath (new RelativePointPath (tree));

    Path newPath;
    newRelativePath->createPath (newPath, getParent());

    if (! newRelativePath->containsAnyDynamicPoints())
        newRelativePath = 0;

    const PathStrokeType newStroke (v.getStrokeType());
    if (strokeType != newStroke || path != newPath)
    {
        path.swapWithPath (newPath);
        strokeType = newStroke;
        pathChanged();
    }

    relativePath = newRelativePath;
}

const ValueTree DrawablePath::createValueTree (ImageProvider* imageProvider) const
{
    ValueTree tree (valueTreeType);
    ValueTreeWrapper v (tree);

    v.setID (getName(), 0);
    writeTo (v, imageProvider, 0);

    if (relativePath != 0)
    {
        relativePath->writeTo (tree, 0);
    }
    else
    {
        RelativePointPath rp (path);
        rp.writeTo (tree, 0);
    }

    return tree;
}


END_JUCE_NAMESPACE

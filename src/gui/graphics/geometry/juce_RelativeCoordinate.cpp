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

#include "juce_RelativeCoordinate.h"
#include "../drawables/juce_DrawablePath.h"
#include "../../../io/streams/juce_MemoryOutputStream.h"


//==============================================================================
namespace RelativeCoordinateHelpers
{
    void skipComma (const juce_wchar* const s, int& i)
    {
        while (CharacterFunctions::isWhitespace (s[i]))
            ++i;

        if (s[i] == ',')
            ++i;
    }
}

//==============================================================================
const String RelativeCoordinate::Strings::parent ("parent");
const String RelativeCoordinate::Strings::left ("left");
const String RelativeCoordinate::Strings::right ("right");
const String RelativeCoordinate::Strings::top ("top");
const String RelativeCoordinate::Strings::bottom ("bottom");
const String RelativeCoordinate::Strings::parentLeft ("parent.left");
const String RelativeCoordinate::Strings::parentTop ("parent.top");
const String RelativeCoordinate::Strings::parentRight ("parent.right");
const String RelativeCoordinate::Strings::parentBottom ("parent.bottom");

//==============================================================================
RelativeCoordinate::RelativeCoordinate()
{
}

RelativeCoordinate::RelativeCoordinate (const Expression& term_)
    : term (term_)
{
}

RelativeCoordinate::RelativeCoordinate (const RelativeCoordinate& other)
    : term (other.term)
{
}

RelativeCoordinate& RelativeCoordinate::operator= (const RelativeCoordinate& other)
{
    term = other.term;
    return *this;
}

RelativeCoordinate::RelativeCoordinate (const double absoluteDistanceFromOrigin)
    : term (absoluteDistanceFromOrigin)
{
}

RelativeCoordinate::RelativeCoordinate (const String& s)
{
    try
    {
        term = Expression (s);
    }
    catch (...)
    {}
}

RelativeCoordinate::~RelativeCoordinate()
{
}

bool RelativeCoordinate::operator== (const RelativeCoordinate& other) const throw()
{
    return term.toString() == other.term.toString();
}

bool RelativeCoordinate::operator!= (const RelativeCoordinate& other) const throw()
{
    return ! operator== (other);
}

double RelativeCoordinate::resolve (const Expression::EvaluationContext* context) const
{
    try
    {
        if (context != 0)
            return term.evaluate (*context);
        else
            return term.evaluate();
    }
    catch (...)
    {}

    return 0.0;
}

bool RelativeCoordinate::isRecursive (const Expression::EvaluationContext* context) const
{
    try
    {
        if (context != 0)
            term.evaluate (*context);
        else
            term.evaluate();
    }
    catch (...)
    {
        return true;
    }

    return false;
}

void RelativeCoordinate::moveToAbsolute (double newPos, const Expression::EvaluationContext* context)
{
    try
    {
        if (context != 0)
        {
            term = term.adjustedToGiveNewResult (newPos, *context);
        }
        else
        {
            Expression::EvaluationContext defaultContext;
            term = term.adjustedToGiveNewResult (newPos, defaultContext);
        }
    }
    catch (...)
    {}
}

bool RelativeCoordinate::references (const String& coordName, const Expression::EvaluationContext* context) const
{
    try
    {
        return term.referencesSymbol (coordName, context);
    }
    catch (...)
    {}

    return false;
}

bool RelativeCoordinate::isDynamic() const
{
    return term.usesAnySymbols();
}

const String RelativeCoordinate::toString() const
{
    return term.toString();
}

void RelativeCoordinate::renameSymbolIfUsed (const String& oldName, const String& newName)
{
    jassert (newName.isNotEmpty() && newName.toLowerCase().containsOnly ("abcdefghijklmnopqrstuvwxyz0123456789_"));

    if (term.referencesSymbol (oldName, 0))
        term = term.withRenamedSymbol (oldName, newName);
}

//==============================================================================
RelativePoint::RelativePoint()
{
}

RelativePoint::RelativePoint (const Point<float>& absolutePoint)
    : x (absolutePoint.getX()), y (absolutePoint.getY())
{
}

RelativePoint::RelativePoint (const float x_, const float y_)
    : x (x_), y (y_)
{
}

RelativePoint::RelativePoint (const RelativeCoordinate& x_, const RelativeCoordinate& y_)
    : x (x_), y (y_)
{
}

RelativePoint::RelativePoint (const String& s)
{
    int i = 0;
    x = RelativeCoordinate (Expression::parse (s, i));
    RelativeCoordinateHelpers::skipComma (s, i);
    y = RelativeCoordinate (Expression::parse (s, i));
}

bool RelativePoint::operator== (const RelativePoint& other) const throw()
{
    return x == other.x && y == other.y;
}

bool RelativePoint::operator!= (const RelativePoint& other) const throw()
{
    return ! operator== (other);
}

const Point<float> RelativePoint::resolve (const Expression::EvaluationContext* context) const
{
    return Point<float> ((float) x.resolve (context),
                         (float) y.resolve (context));
}

void RelativePoint::moveToAbsolute (const Point<float>& newPos, const Expression::EvaluationContext* context)
{
    x.moveToAbsolute (newPos.getX(), context);
    y.moveToAbsolute (newPos.getY(), context);
}

const String RelativePoint::toString() const
{
    return x.toString() + ", " + y.toString();
}

void RelativePoint::renameSymbolIfUsed (const String& oldName, const String& newName)
{
    x.renameSymbolIfUsed (oldName, newName);
    y.renameSymbolIfUsed (oldName, newName);
}

bool RelativePoint::isDynamic() const
{
    return x.isDynamic() || y.isDynamic();
}


//==============================================================================
RelativeRectangle::RelativeRectangle()
{
}

RelativeRectangle::RelativeRectangle (const RelativeCoordinate& left_, const RelativeCoordinate& right_,
                                      const RelativeCoordinate& top_, const RelativeCoordinate& bottom_)
    : left (left_), right (right_), top (top_), bottom (bottom_)
{
}

RelativeRectangle::RelativeRectangle (const Rectangle<float>& rect, const String& componentName)
    : left (rect.getX()),
      right (Expression::symbol (componentName + "." + RelativeCoordinate::Strings::left) + Expression ((double) rect.getWidth())),
      top (rect.getY()),
      bottom (Expression::symbol (componentName + "." + RelativeCoordinate::Strings::top) + Expression ((double) rect.getHeight()))
{
}

RelativeRectangle::RelativeRectangle (const String& s)
{
    int i = 0;
    left = RelativeCoordinate (Expression::parse (s, i));
    RelativeCoordinateHelpers::skipComma (s, i);
    top = RelativeCoordinate (Expression::parse (s, i));
    RelativeCoordinateHelpers::skipComma (s, i);
    right = RelativeCoordinate (Expression::parse (s, i));
    RelativeCoordinateHelpers::skipComma (s, i);
    bottom = RelativeCoordinate (Expression::parse (s, i));
}

bool RelativeRectangle::operator== (const RelativeRectangle& other) const throw()
{
    return left == other.left && top == other.top && right == other.right && bottom == other.bottom;
}

bool RelativeRectangle::operator!= (const RelativeRectangle& other) const throw()
{
    return ! operator== (other);
}

const Rectangle<float> RelativeRectangle::resolve (const Expression::EvaluationContext* context) const
{
    const double l = left.resolve (context);
    const double r = right.resolve (context);
    const double t = top.resolve (context);
    const double b = bottom.resolve (context);

    return Rectangle<float> ((float) l, (float) t, (float) (r - l), (float) (b - t));
}

void RelativeRectangle::moveToAbsolute (const Rectangle<float>& newPos, const Expression::EvaluationContext* context)
{
    left.moveToAbsolute (newPos.getX(), context);
    right.moveToAbsolute (newPos.getRight(), context);
    top.moveToAbsolute (newPos.getY(), context);
    bottom.moveToAbsolute (newPos.getBottom(), context);
}

const String RelativeRectangle::toString() const
{
    return left.toString() + ", " + top.toString() + ", " + right.toString() + ", " + bottom.toString();
}

void RelativeRectangle::renameSymbolIfUsed (const String& oldName, const String& newName)
{
    left.renameSymbolIfUsed (oldName, newName);
    right.renameSymbolIfUsed (oldName, newName);
    top.renameSymbolIfUsed (oldName, newName);
    bottom.renameSymbolIfUsed (oldName, newName);
}


//==============================================================================
RelativePointPath::RelativePointPath()
    : usesNonZeroWinding (true),
      containsDynamicPoints (false)
{
}

RelativePointPath::RelativePointPath (const RelativePointPath& other)
    : usesNonZeroWinding (true),
      containsDynamicPoints (false)
{
    ValueTree state (DrawablePath::valueTreeType);
    other.writeTo (state, 0);
    parse (state);
}

RelativePointPath::RelativePointPath (const ValueTree& drawable)
    : usesNonZeroWinding (true),
      containsDynamicPoints (false)
{
    parse (drawable);
}

RelativePointPath::RelativePointPath (const Path& path)
{
    usesNonZeroWinding = path.isUsingNonZeroWinding();

    Path::Iterator i (path);

    while (i.next())
    {
        switch (i.elementType)
        {
            case Path::Iterator::startNewSubPath:   elements.add (new StartSubPath (RelativePoint (i.x1, i.y1))); break;
            case Path::Iterator::lineTo:            elements.add (new LineTo (RelativePoint (i.x1, i.y1))); break;
            case Path::Iterator::quadraticTo:       elements.add (new QuadraticTo (RelativePoint (i.x1, i.y1), RelativePoint (i.x2, i.y2))); break;
            case Path::Iterator::cubicTo:           elements.add (new CubicTo (RelativePoint (i.x1, i.y1), RelativePoint (i.x2, i.y2), RelativePoint (i.x3, i.y3))); break;
            case Path::Iterator::closePath:         elements.add (new CloseSubPath()); break;
            default:                                jassertfalse; break;
        }
    }
}

void RelativePointPath::writeTo (ValueTree state, UndoManager* undoManager) const
{
    DrawablePath::ValueTreeWrapper wrapper (state);
    wrapper.setUsesNonZeroWinding (usesNonZeroWinding, undoManager);

    ValueTree pathTree (wrapper.getPathState());
    pathTree.removeAllChildren (undoManager);

    for (int i = 0; i < elements.size(); ++i)
        pathTree.addChild (elements.getUnchecked(i)->createTree(), -1, undoManager);
}

void RelativePointPath::parse (const ValueTree& state)
{
    DrawablePath::ValueTreeWrapper wrapper (state);
    usesNonZeroWinding = wrapper.usesNonZeroWinding();
    RelativePoint points[3];

    const ValueTree pathTree (wrapper.getPathState());
    const int num = pathTree.getNumChildren();
    for (int i = 0; i < num; ++i)
    {
        const DrawablePath::ValueTreeWrapper::Element e (pathTree.getChild(i));

        const int numCps = e.getNumControlPoints();
        for (int j = 0; j < numCps; ++j)
        {
            points[j] = e.getControlPoint (j);
            containsDynamicPoints = containsDynamicPoints || points[j].isDynamic();
        }

        const Identifier type (e.getType());

        if (type == DrawablePath::ValueTreeWrapper::Element::startSubPathElement)
            elements.add (new StartSubPath (points[0]));
        else if (type == DrawablePath::ValueTreeWrapper::Element::closeSubPathElement)
            elements.add (new CloseSubPath());
        else if (type == DrawablePath::ValueTreeWrapper::Element::lineToElement)
            elements.add (new LineTo (points[0]));
        else if (type == DrawablePath::ValueTreeWrapper::Element::quadraticToElement)
            elements.add (new QuadraticTo (points[0], points[1]));
        else if (type == DrawablePath::ValueTreeWrapper::Element::cubicToElement)
            elements.add (new CubicTo (points[0], points[1], points[2]));
        else
            jassertfalse;
    }
}

RelativePointPath::~RelativePointPath()
{
}

void RelativePointPath::swapWith (RelativePointPath& other) throw()
{
    elements.swapWithArray (other.elements);
    swapVariables (usesNonZeroWinding, other.usesNonZeroWinding);
}

void RelativePointPath::createPath (Path& path, Expression::EvaluationContext* coordFinder)
{
    for (int i = 0; i < elements.size(); ++i)
        elements.getUnchecked(i)->addToPath (path, coordFinder);
}

bool RelativePointPath::containsAnyDynamicPoints() const
{
    return containsDynamicPoints;
}

//==============================================================================
RelativePointPath::ElementBase::ElementBase (const ElementType type_) : type (type_)
{
}

//==============================================================================
RelativePointPath::StartSubPath::StartSubPath (const RelativePoint& pos)
    : ElementBase (startSubPathElement), startPos (pos)
{
}

const ValueTree RelativePointPath::StartSubPath::createTree() const
{
    ValueTree v (DrawablePath::ValueTreeWrapper::Element::startSubPathElement);
    v.setProperty (DrawablePath::ValueTreeWrapper::point1, startPos.toString(), 0);
    return v;
}

void RelativePointPath::StartSubPath::addToPath (Path& path, Expression::EvaluationContext* coordFinder) const
{
    path.startNewSubPath (startPos.resolve (coordFinder));
}

RelativePoint* RelativePointPath::StartSubPath::getControlPoints (int& numPoints)
{
    numPoints = 1;
    return &startPos;
}

//==============================================================================
RelativePointPath::CloseSubPath::CloseSubPath()
    : ElementBase (closeSubPathElement)
{
}

const ValueTree RelativePointPath::CloseSubPath::createTree() const
{
    return ValueTree (DrawablePath::ValueTreeWrapper::Element::closeSubPathElement);
}

void RelativePointPath::CloseSubPath::addToPath (Path& path, Expression::EvaluationContext*) const
{
    path.closeSubPath();
}

RelativePoint* RelativePointPath::CloseSubPath::getControlPoints (int& numPoints)
{
    numPoints = 0;
    return 0;
}

//==============================================================================
RelativePointPath::LineTo::LineTo (const RelativePoint& endPoint_)
    : ElementBase (lineToElement), endPoint (endPoint_)
{
}

const ValueTree RelativePointPath::LineTo::createTree() const
{
    ValueTree v (DrawablePath::ValueTreeWrapper::Element::lineToElement);
    v.setProperty (DrawablePath::ValueTreeWrapper::point1, endPoint.toString(), 0);
    return v;
}

void RelativePointPath::LineTo::addToPath (Path& path, Expression::EvaluationContext* coordFinder) const
{
    path.lineTo (endPoint.resolve (coordFinder));
}

RelativePoint* RelativePointPath::LineTo::getControlPoints (int& numPoints)
{
    numPoints = 1;
    return &endPoint;
}

//==============================================================================
RelativePointPath::QuadraticTo::QuadraticTo (const RelativePoint& controlPoint, const RelativePoint& endPoint)
    : ElementBase (quadraticToElement)
{
    controlPoints[0] = controlPoint;
    controlPoints[1] = endPoint;
}

const ValueTree RelativePointPath::QuadraticTo::createTree() const
{
    ValueTree v (DrawablePath::ValueTreeWrapper::Element::quadraticToElement);
    v.setProperty (DrawablePath::ValueTreeWrapper::point1, controlPoints[0].toString(), 0);
    v.setProperty (DrawablePath::ValueTreeWrapper::point2, controlPoints[1].toString(), 0);
    return v;
}

void RelativePointPath::QuadraticTo::addToPath (Path& path, Expression::EvaluationContext* coordFinder) const
{
    path.quadraticTo (controlPoints[0].resolve (coordFinder),
                      controlPoints[1].resolve (coordFinder));
}

RelativePoint* RelativePointPath::QuadraticTo::getControlPoints (int& numPoints)
{
    numPoints = 2;
    return controlPoints;
}

//==============================================================================
RelativePointPath::CubicTo::CubicTo (const RelativePoint& controlPoint1, const RelativePoint& controlPoint2, const RelativePoint& endPoint)
    : ElementBase (cubicToElement)
{
    controlPoints[0] = controlPoint1;
    controlPoints[1] = controlPoint2;
    controlPoints[2] = endPoint;
}

const ValueTree RelativePointPath::CubicTo::createTree() const
{
    ValueTree v (DrawablePath::ValueTreeWrapper::Element::cubicToElement);
    v.setProperty (DrawablePath::ValueTreeWrapper::point1, controlPoints[0].toString(), 0);
    v.setProperty (DrawablePath::ValueTreeWrapper::point2, controlPoints[1].toString(), 0);
    v.setProperty (DrawablePath::ValueTreeWrapper::point3, controlPoints[2].toString(), 0);
    return v;
}

void RelativePointPath::CubicTo::addToPath (Path& path, Expression::EvaluationContext* coordFinder) const
{
    path.cubicTo (controlPoints[0].resolve (coordFinder),
                  controlPoints[1].resolve (coordFinder),
                  controlPoints[2].resolve (coordFinder));
}

RelativePoint* RelativePointPath::CubicTo::getControlPoints (int& numPoints)
{
    numPoints = 3;
    return controlPoints;
}


//==============================================================================
RelativeParallelogram::RelativeParallelogram()
{
}

RelativeParallelogram::RelativeParallelogram (const Rectangle<float>& r)
    : topLeft (r.getTopLeft()), topRight (r.getTopRight()), bottomLeft (r.getBottomLeft())
{
}

RelativeParallelogram::RelativeParallelogram (const RelativePoint& topLeft_, const RelativePoint& topRight_, const RelativePoint& bottomLeft_)
    : topLeft (topLeft_), topRight (topRight_), bottomLeft (bottomLeft_)
{
}

RelativeParallelogram::RelativeParallelogram (const String& topLeft_, const String& topRight_, const String& bottomLeft_)
    : topLeft (topLeft_), topRight (topRight_), bottomLeft (bottomLeft_)
{
}

RelativeParallelogram::~RelativeParallelogram()
{
}

void RelativeParallelogram::resolveThreePoints (Point<float>* points, Expression::EvaluationContext* const coordFinder) const
{
    points[0] = topLeft.resolve (coordFinder);
    points[1] = topRight.resolve (coordFinder);
    points[2] = bottomLeft.resolve (coordFinder);
}

void RelativeParallelogram::resolveFourCorners (Point<float>* points, Expression::EvaluationContext* const coordFinder) const
{
    resolveThreePoints (points, coordFinder);
    points[3] = points[1] + (points[2] - points[0]);
}

const Rectangle<float> RelativeParallelogram::getBounds (Expression::EvaluationContext* const coordFinder) const
{
    Point<float> points[4];
    resolveFourCorners (points, coordFinder);
    return Rectangle<float>::findAreaContainingPoints (points, 4);
}

void RelativeParallelogram::getPath (Path& path, Expression::EvaluationContext* const coordFinder) const
{
    Point<float> points[4];
    resolveFourCorners (points, coordFinder);

    path.startNewSubPath (points[0]);
    path.lineTo (points[1]);
    path.lineTo (points[3]);
    path.lineTo (points[2]);
    path.closeSubPath();
}

const AffineTransform RelativeParallelogram::resetToPerpendicular (Expression::EvaluationContext* const coordFinder)
{
    Point<float> corners[3];
    resolveThreePoints (corners, coordFinder);

    const Line<float> top (corners[0], corners[1]);
    const Line<float> left (corners[0], corners[2]);
    const Point<float> newTopRight (corners[0] + Point<float> (top.getLength(), 0.0f));
    const Point<float> newBottomLeft (corners[0] + Point<float> (0.0f, left.getLength()));

    topRight.moveToAbsolute (newTopRight, coordFinder);
    bottomLeft.moveToAbsolute (newBottomLeft, coordFinder);

    return AffineTransform::fromTargetPoints (corners[0].getX(), corners[0].getY(), corners[0].getX(), corners[0].getY(),
                                              corners[1].getX(), corners[1].getY(), newTopRight.getX(), newTopRight.getY(),
                                              corners[2].getX(), corners[2].getY(), newBottomLeft.getX(), newBottomLeft.getY());
}

bool RelativeParallelogram::operator== (const RelativeParallelogram& other) const throw()
{
    return topLeft == other.topLeft && topRight == other.topRight && bottomLeft == other.bottomLeft;
}

bool RelativeParallelogram::operator!= (const RelativeParallelogram& other) const throw()
{
    return ! operator== (other);
}

const Point<float> RelativeParallelogram::getInternalCoordForPoint (const Point<float>* const corners, Point<float> target) throw()
{
    const Point<float> tr (corners[1] - corners[0]);
    const Point<float> bl (corners[2] - corners[0]);
    target -= corners[0];

    return Point<float> (Line<float> (Point<float>(), tr).getIntersection (Line<float> (target, target - bl)).getDistanceFromOrigin(),
                         Line<float> (Point<float>(), bl).getIntersection (Line<float> (target, target - tr)).getDistanceFromOrigin());
}

const Point<float> RelativeParallelogram::getPointForInternalCoord (const Point<float>* const corners, const Point<float>& point) throw()
{
    return corners[0]
            + Line<float> (Point<float>(), corners[1] - corners[0]).getPointAlongLine (point.getX())
            + Line<float> (Point<float>(), corners[2] - corners[0]).getPointAlongLine (point.getY());
}


END_JUCE_NAMESPACE

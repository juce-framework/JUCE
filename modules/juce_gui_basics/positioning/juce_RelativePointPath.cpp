/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

RelativePointPath::RelativePointPath()
    : usesNonZeroWinding (true),
      containsDynamicPoints (false)
{
}

RelativePointPath::RelativePointPath (const RelativePointPath& other)
    : usesNonZeroWinding (true),
      containsDynamicPoints (false)
{
    for (int i = 0; i < other.elements.size(); ++i)
        elements.add (other.elements.getUnchecked (i)->clone());
}

RelativePointPath::RelativePointPath (const Path& path)
    : usesNonZeroWinding (path.isUsingNonZeroWinding()),
      containsDynamicPoints (false)
{
    for (Path::Iterator i (path); i.next();)
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

RelativePointPath::~RelativePointPath()
{
}

bool RelativePointPath::operator== (const RelativePointPath& other) const noexcept
{
    if (elements.size() != other.elements.size()
         || usesNonZeroWinding != other.usesNonZeroWinding
         || containsDynamicPoints != other.containsDynamicPoints)
        return false;

    for (int i = 0; i < elements.size(); ++i)
    {
        ElementBase* const e1 = elements.getUnchecked (i);
        ElementBase* const e2 = other.elements.getUnchecked (i);

        if (e1->type != e2->type)
            return false;

        int numPoints1, numPoints2;
        const RelativePoint* const points1 = e1->getControlPoints (numPoints1);
        const RelativePoint* const points2 = e2->getControlPoints (numPoints2);

        jassert (numPoints1 == numPoints2);

        for (int j = numPoints1; --j >= 0;)
            if (points1[j] != points2[j])
                return false;
    }

    return true;
}

bool RelativePointPath::operator!= (const RelativePointPath& other) const noexcept
{
    return ! operator== (other);
}

void RelativePointPath::swapWith (RelativePointPath& other) noexcept
{
    elements.swapWith (other.elements);
    std::swap (usesNonZeroWinding, other.usesNonZeroWinding);
    std::swap (containsDynamicPoints, other.containsDynamicPoints);
}

void RelativePointPath::createPath (Path& path, Expression::Scope* scope) const
{
    for (int i = 0; i < elements.size(); ++i)
        elements.getUnchecked (i)->addToPath (path, scope);
}

bool RelativePointPath::containsAnyDynamicPoints() const
{
    return containsDynamicPoints;
}

void RelativePointPath::addElement (ElementBase* newElement)
{
    if (newElement != nullptr)
    {
        elements.add (newElement);
        containsDynamicPoints = containsDynamicPoints || newElement->isDynamic();
    }
}

//==============================================================================
RelativePointPath::ElementBase::ElementBase (const ElementType type_) : type (type_)
{
}

bool RelativePointPath::ElementBase::isDynamic()
{
    int numPoints;
    const RelativePoint* const points = getControlPoints (numPoints);

    for (int i = numPoints; --i >= 0;)
        if (points[i].isDynamic())
            return true;

    return false;
}

//==============================================================================
RelativePointPath::StartSubPath::StartSubPath (const RelativePoint& pos)
    : ElementBase (startSubPathElement), startPos (pos)
{
}

void RelativePointPath::StartSubPath::addToPath (Path& path, Expression::Scope* scope) const
{
    path.startNewSubPath (startPos.resolve (scope));
}

RelativePoint* RelativePointPath::StartSubPath::getControlPoints (int& numPoints)
{
    numPoints = 1;
    return &startPos;
}

RelativePointPath::ElementBase* RelativePointPath::StartSubPath::clone() const
{
    return new StartSubPath (startPos);
}

//==============================================================================
RelativePointPath::CloseSubPath::CloseSubPath()
    : ElementBase (closeSubPathElement)
{
}

void RelativePointPath::CloseSubPath::addToPath (Path& path, Expression::Scope*) const
{
    path.closeSubPath();
}

RelativePoint* RelativePointPath::CloseSubPath::getControlPoints (int& numPoints)
{
    numPoints = 0;
    return nullptr;
}

RelativePointPath::ElementBase* RelativePointPath::CloseSubPath::clone() const
{
    return new CloseSubPath();
}

//==============================================================================
RelativePointPath::LineTo::LineTo (const RelativePoint& endPoint_)
    : ElementBase (lineToElement), endPoint (endPoint_)
{
}

void RelativePointPath::LineTo::addToPath (Path& path, Expression::Scope* scope) const
{
    path.lineTo (endPoint.resolve (scope));
}

RelativePoint* RelativePointPath::LineTo::getControlPoints (int& numPoints)
{
    numPoints = 1;
    return &endPoint;
}

RelativePointPath::ElementBase* RelativePointPath::LineTo::clone() const
{
    return new LineTo (endPoint);
}

//==============================================================================
RelativePointPath::QuadraticTo::QuadraticTo (const RelativePoint& controlPoint, const RelativePoint& endPoint)
    : ElementBase (quadraticToElement)
{
    controlPoints[0] = controlPoint;
    controlPoints[1] = endPoint;
}

void RelativePointPath::QuadraticTo::addToPath (Path& path, Expression::Scope* scope) const
{
    path.quadraticTo (controlPoints[0].resolve (scope),
                      controlPoints[1].resolve (scope));
}

RelativePoint* RelativePointPath::QuadraticTo::getControlPoints (int& numPoints)
{
    numPoints = 2;
    return controlPoints;
}

RelativePointPath::ElementBase* RelativePointPath::QuadraticTo::clone() const
{
    return new QuadraticTo (controlPoints[0], controlPoints[1]);
}


//==============================================================================
RelativePointPath::CubicTo::CubicTo (const RelativePoint& controlPoint1, const RelativePoint& controlPoint2, const RelativePoint& endPoint)
    : ElementBase (cubicToElement)
{
    controlPoints[0] = controlPoint1;
    controlPoints[1] = controlPoint2;
    controlPoints[2] = endPoint;
}

void RelativePointPath::CubicTo::addToPath (Path& path, Expression::Scope* scope) const
{
    path.cubicTo (controlPoints[0].resolve (scope),
                  controlPoints[1].resolve (scope),
                  controlPoints[2].resolve (scope));
}

RelativePoint* RelativePointPath::CubicTo::getControlPoints (int& numPoints)
{
    numPoints = 3;
    return controlPoints;
}

RelativePointPath::ElementBase* RelativePointPath::CubicTo::clone() const
{
    return new CubicTo (controlPoints[0], controlPoints[1], controlPoints[2]);
}

} // namespace juce

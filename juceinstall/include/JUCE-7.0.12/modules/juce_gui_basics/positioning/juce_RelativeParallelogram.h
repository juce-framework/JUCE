/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

//==============================================================================
/**
    A parallelogram defined by three RelativePoint positions.

    @see RelativePoint, RelativeCoordinate

    @tags{GUI}
*/
class JUCE_API  RelativeParallelogram
{
public:
    //==============================================================================
    RelativeParallelogram();
    RelativeParallelogram (const Rectangle<float>& simpleRectangle);
    RelativeParallelogram (const RelativePoint& topLeft, const RelativePoint& topRight, const RelativePoint& bottomLeft);
    RelativeParallelogram (const String& topLeft, const String& topRight, const String& bottomLeft);
    ~RelativeParallelogram();

    //==============================================================================
    void resolveThreePoints (Point<float>* points, Expression::Scope* scope) const;
    void resolveFourCorners (Point<float>* points, Expression::Scope* scope) const;
    const Rectangle<float> getBounds (Expression::Scope* scope) const;
    void getPath (Path& path, Expression::Scope* scope) const;
    AffineTransform resetToPerpendicular (Expression::Scope* scope);
    bool isDynamic() const;

    bool operator== (const RelativeParallelogram&) const noexcept;
    bool operator!= (const RelativeParallelogram&) const noexcept;

    static Point<float> getInternalCoordForPoint (const Point<float>* parallelogramCorners, Point<float> point) noexcept;
    static Point<float> getPointForInternalCoord (const Point<float>* parallelogramCorners, Point<float> internalPoint) noexcept;
    static Rectangle<float> getBoundingBox (const Point<float>* parallelogramCorners) noexcept;

    //==============================================================================
    RelativePoint topLeft, topRight, bottomLeft;
};

} // namespace juce

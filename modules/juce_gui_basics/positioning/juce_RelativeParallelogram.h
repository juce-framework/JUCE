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

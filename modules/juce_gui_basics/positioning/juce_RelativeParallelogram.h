/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

#ifndef JUCE_RELATIVEPARALLELOGRAM_H_INCLUDED
#define JUCE_RELATIVEPARALLELOGRAM_H_INCLUDED


//==============================================================================
/**
    A parallelogram defined by three RelativePoint positions.

    @see RelativePoint, RelativeCoordinate
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


#endif   // JUCE_RELATIVEPARALLELOGRAM_H_INCLUDED

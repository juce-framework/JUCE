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

#ifndef __JUCE_RELATIVEPARALLELOGRAM_JUCEHEADER__
#define __JUCE_RELATIVEPARALLELOGRAM_JUCEHEADER__

#include "juce_RelativePoint.h"


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
    void resolveThreePoints (Point<float>* points, Expression::EvaluationContext* coordFinder) const;
    void resolveFourCorners (Point<float>* points, Expression::EvaluationContext* coordFinder) const;
    const Rectangle<float> getBounds (Expression::EvaluationContext* coordFinder) const;
    void getPath (Path& path, Expression::EvaluationContext* coordFinder) const;
    const AffineTransform resetToPerpendicular (Expression::EvaluationContext* coordFinder);
    bool isDynamic() const;

    bool operator== (const RelativeParallelogram& other) const throw();
    bool operator!= (const RelativeParallelogram& other) const throw();

    static const Point<float> getInternalCoordForPoint (const Point<float>* parallelogramCorners, Point<float> point) throw();
    static const Point<float> getPointForInternalCoord (const Point<float>* parallelogramCorners, const Point<float>& internalPoint) throw();
    static const Rectangle<float> getBoundingBox (const Point<float>* parallelogramCorners) throw();

    //==============================================================================
    RelativePoint topLeft, topRight, bottomLeft;
};


#endif   // __JUCE_RELATIVEPARALLELOGRAM_JUCEHEADER__

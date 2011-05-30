/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

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

#ifndef __JUCE_RELATIVEPOINT_JUCEHEADER__
#define __JUCE_RELATIVEPOINT_JUCEHEADER__

#include "juce_RelativeCoordinate.h"


//==============================================================================
/**
    An X-Y position stored as a pair of RelativeCoordinate values.

    @see RelativeCoordinate, RelativeRectangle
*/
class JUCE_API  RelativePoint
{
public:
    /** Creates a point at the origin. */
    RelativePoint();

    /** Creates an absolute point, relative to the origin. */
    RelativePoint (const Point<float>& absolutePoint);

    /** Creates an absolute point, relative to the origin. */
    RelativePoint (float absoluteX, float absoluteY);

    /** Creates an absolute point from two coordinates. */
    RelativePoint (const RelativeCoordinate& x, const RelativeCoordinate& y);

    /** Creates a point from a stringified representation.
        The string must contain a pair of coordinates, separated by space or a comma. The syntax for the coordinate
        strings is explained in the RelativeCoordinate class.
        @see toString
    */
    RelativePoint (const String& stringVersion);

    bool operator== (const RelativePoint& other) const noexcept;
    bool operator!= (const RelativePoint& other) const noexcept;

    /** Calculates the absolute position of this point.

        You'll need to provide a suitable Expression::Scope for looking up any coordinates that may
        be needed to calculate the result.
    */
    const Point<float> resolve (const Expression::Scope* evaluationContext) const;

    /** Changes the values of this point's coordinates to make it resolve to the specified position.

        Calling this will leave any anchor points unchanged, but will set any absolute
        or relative positions to whatever values are necessary to make the resultant position
        match the position that is provided.
    */
    void moveToAbsolute (const Point<float>& newPos, const Expression::Scope* evaluationContext);

    /** Returns a string which represents this point.
        This returns a comma-separated pair of coordinates. For details of the string syntax used by the
        coordinates, see the RelativeCoordinate constructor notes.
        The string that is returned can be passed to the RelativePoint constructor to recreate the point.
    */
    String toString() const;

    /** Returns true if this point depends on any other coordinates for its position. */
    bool isDynamic() const;

    // The actual X and Y coords...
    RelativeCoordinate x, y;
};


#endif   // __JUCE_RELATIVEPOINT_JUCEHEADER__

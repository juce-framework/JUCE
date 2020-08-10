/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
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
    An X-Y position stored as a pair of RelativeCoordinate values.

    @see RelativeCoordinate, RelativeRectangle

    @tags{GUI}
*/
class JUCE_API  RelativePoint
{
public:
    /** Creates a point at the origin. */
    RelativePoint();

    /** Creates an absolute point, relative to the origin. */
    RelativePoint (Point<float> absolutePoint);

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

    bool operator== (const RelativePoint&) const noexcept;
    bool operator!= (const RelativePoint&) const noexcept;

    /** Calculates the absolute position of this point.

        You'll need to provide a suitable Expression::Scope for looking up any coordinates that may
        be needed to calculate the result.
    */
    Point<float> resolve (const Expression::Scope* evaluationContext) const;

    /** Changes the values of this point's coordinates to make it resolve to the specified position.

        Calling this will leave any anchor points unchanged, but will set any absolute
        or relative positions to whatever values are necessary to make the resultant position
        match the position that is provided.
    */
    void moveToAbsolute (Point<float> newPos, const Expression::Scope* evaluationContext);

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

} // namespace juce

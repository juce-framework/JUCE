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
    A rectangle stored as a set of RelativeCoordinate values.

    The rectangle's top, left, bottom and right edge positions are each stored as a RelativeCoordinate.

    @see RelativeCoordinate, RelativePoint

    @tags{GUI}
*/
class JUCE_API  RelativeRectangle
{
public:
    //==============================================================================
    /** Creates a zero-size rectangle at the origin. */
    RelativeRectangle();

    /** Creates an absolute rectangle, relative to the origin. */
    explicit RelativeRectangle (const Rectangle<float>& rect);

    /** Creates a rectangle from four coordinates. */
    RelativeRectangle (const RelativeCoordinate& left, const RelativeCoordinate& right,
                       const RelativeCoordinate& top, const RelativeCoordinate& bottom);

    /** Creates a rectangle from a stringified representation.
        The string must contain a sequence of 4 coordinates, separated by commas, in the order
        left, top, right, bottom. The syntax for the coordinate strings is explained in the
        RelativeCoordinate class.
        @see toString
    */
    explicit RelativeRectangle (const String& stringVersion);

    bool operator== (const RelativeRectangle&) const noexcept;
    bool operator!= (const RelativeRectangle&) const noexcept;

    //==============================================================================
    /** Calculates the absolute position of this rectangle.

        You'll need to provide a suitable Expression::Scope for looking up any coordinates that may
        be needed to calculate the result.
    */
    const Rectangle<float> resolve (const Expression::Scope* scope) const;

    /** Changes the values of this rectangle's coordinates to make it resolve to the specified position.

        Calling this will leave any anchor points unchanged, but will set any absolute
        or relative positions to whatever values are necessary to make the resultant position
        match the position that is provided.
    */
    void moveToAbsolute (const Rectangle<float>& newPos, const Expression::Scope* scope);

    /** Returns true if this rectangle depends on any external symbols for its position.
        Coordinates that refer to symbols based on "this" are assumed not to be dynamic.
    */
    bool isDynamic() const;

    /** Returns a string which represents this point.
        This returns a comma-separated list of coordinates, in the order left, top, right, bottom.
        If you're using this to position a Component, then see the notes for
        Component::setBounds (const RelativeRectangle&) for details of the syntax used.
        The string that is returned can be passed to the RelativeRectangle constructor to recreate the rectangle.
    */
    String toString() const;

    /** Renames a symbol if it is used by any of the coordinates.
        This calls Expression::withRenamedSymbol() on the rectangle's coordinates.
    */
    void renameSymbol (const Expression::Symbol& oldSymbol, const String& newName, const Expression::Scope& scope);

    /** Creates and sets an appropriate Component::Positioner object for the given component, which will
        keep it positioned with this rectangle.
    */
    void applyToComponent (Component& component) const;

    //==============================================================================
    // The actual rectangle coords...
    RelativeCoordinate left, right, top, bottom;
};

} // namespace juce

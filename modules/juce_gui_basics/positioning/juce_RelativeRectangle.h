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

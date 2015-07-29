/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

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

#ifndef JUCE_RELATIVERECTANGLE_H_INCLUDED
#define JUCE_RELATIVERECTANGLE_H_INCLUDED


//==============================================================================
/**
    A rectangle stored as a set of RelativeCoordinate values.

    The rectangle's top, left, bottom and right edge positions are each stored as a RelativeCoordinate.

    @see RelativeCoordinate, RelativePoint
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


#endif   // JUCE_RELATIVERECTANGLE_H_INCLUDED

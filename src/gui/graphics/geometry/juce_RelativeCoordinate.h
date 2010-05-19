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

#ifndef __JUCE_RELATIVECOORDINATE_JUCEHEADER__
#define __JUCE_RELATIVECOORDINATE_JUCEHEADER__

#include "juce_Rectangle.h"


//==============================================================================
/**
    Expresses a coordinate as an absolute or proportional distance from other
    named coordinates.

    A RelativeCoordinate represents a position as either:
        - an absolute distance from the origin
        - an absolute distance from another named RelativeCoordinate
        - a proportion of the distance between two other named RelativeCoordinates

    Of course, the coordinates that this one is relative to may themselves be relative
    to other coordinates, so complex arrangements can be built up (as long as you're careful
    not to create recursive loops!)

    Rather than keeping pointers to the coordinates that this one is dependent on, it
    stores their names, and when resolving this coordinate to an absolute value, a
    NamedCoordinateFinder class is required to retrieve these coordinates by name.

    @see RelativePoint, RelativeRectangle
*/
class JUCE_API  RelativeCoordinate
{
public:
    //==============================================================================
    /** Creates a zero coordinate. */
    RelativeCoordinate();

    /** Creates an absolute position from the parent origin on either the X or Y axis.

        @param absoluteDistanceFromOrigin   the distance from the origin
        @param isHorizontal                 this must be true if this is an X coordinate, or false if it's on the Y axis.
    */
    RelativeCoordinate (double absoluteDistanceFromOrigin, bool isHorizontal);

    /** Creates an absolute position relative to a named coordinate.

        @param absoluteDistanceFromAnchor   the distance to add to the named anchor point
        @param anchorPoint      the name of the coordinate from which this one will be relative. See the constructor
                                notes for a description of the syntax for coordinate names.
    */
    RelativeCoordinate (double absoluteDistanceFromAnchor, const String& anchorPoint);

    /** Creates a relative position between two named points.


        @param relativeProportionBetweenAnchors     a value between 0 and 1 indicating this coordinate's relative position
                                between anchorPoint1 and anchorPoint2.
        @param anchorPoint1     the name of the first coordinate from which this one will be relative. See the constructor
                                notes for a description of the syntax for coordinate names.
        @param anchorPoint2     the name of the first coordinate from which this one will be relative. See the constructor
                                notes for a description of the syntax for coordinate names.
    */
    RelativeCoordinate (double relativeProportionBetweenAnchors, const String& anchorPoint1, const String& anchorPoint2);

    /** Recreates a coordinate from a string description.

        The string can be in one of the following formats:
            - "123"                         = 123 pixels from parent origin (this is equivalent to "parent.left + 123"
                                                or "parent.top + 123", depending on which axis the coordinate is using)
            - "anchor"                      = the same position as the coordinate named "anchor"
            - "anchor + 123"                = the coordinate named "anchor" + 123 pixels
            - "anchor - 123"                = the coordinate named "anchor" - 123 pixels
            - "50%"                         = 50% of the distance between the coordinate space's top-left origin and its extent
                                                (this is equivalent to "50% * parent.left -> parent.right" or "50% * parent.top -> parent.bottom")
            - "50% * anchor"                = 50% of the distance between the coordinate space's origin and the coordinate named "anchor"
                                                (this is equivalent to "50% * parent.left -> anchor" or "50% * parent.top -> anchor")
            - "50% * anchor1 -> anchor2"    = 50% of the distance between the coordinate "anchor1" and the coordinate "anchor2"

        An anchor name can either be just a single identifier (letters, digits and underscores only - no spaces),
        e.g. "marker1", or it can be a two-part name in the form "objectName.edge". For example "parent.left" is
        the origin, or "myComponent.top" is the top edge of a component called "myComponent". The exact names that
        will be recognised are dependent on the NamedCoordinateFinder that you provide for looking them up, but
        "parent.left", "parent.top", "parent.right" and "parent.bottom" are always available, representing the
        extents of the target coordinate space.

        @param stringVersion    the string to parse
        @param isHorizontal     this must be true if this is an X coordinate, or false if it's on the Y axis.

        @see toString
    */
    RelativeCoordinate (const String& stringVersion, bool isHorizontal);

    /** Destructor. */
    ~RelativeCoordinate();

    //==============================================================================
    /**
        Provides an interface for looking up the position of a named anchor when resolving a RelativeCoordinate.

        When using RelativeCoordinates, to resolve their names you need to provide a subclass of this which
        can retrieve a coordinate by name.
    */
    class JUCE_API  NamedCoordinateFinder
    {
    public:
        /** Destructor. */
        virtual ~NamedCoordinateFinder() {}

        /** Returns the coordinate for a given name.

            The objectName parameter will be the first section of the name, and the edge the name of the second part.
            E.g. for "parent.right", objectName would be "parent" and edge would be "right". If the name
            has no dot, the edge parameter will be an empty string.

            This method must be able to resolve "parent.left", "parent.top", "parent.right" and "parent.bottom", as
            well as any other objects that your application uses.
        */
        virtual const RelativeCoordinate findNamedCoordinate (const String& objectName, const String& edge) const = 0;
    };

    //==============================================================================
    /** Calculates the absolute position of this coordinate.

        You'll need to provide a suitable NamedCoordinateFinder for looking up any coordinates that may
        be needed to calculate the result.
    */
    double resolve (const NamedCoordinateFinder& nameFinder) const;

    /** Returns true if this coordinate uses the specified coord name at any level in its evaluation.
        This will recursively check any coordinates upon which this one depends.
    */
    bool references (const String& coordName, const NamedCoordinateFinder& nameFinder) const;

    /** Returns true if there's a recursive loop when trying to resolve this coordinate's position. */
    bool isRecursive (const NamedCoordinateFinder& nameFinder) const;

    //==============================================================================
    /** Changes the value of this coord to make it resolve to the specified position.

        Calling this will leave the anchor points unchanged, but will set this coordinate's absolute
        or relative position to whatever value is necessary to make its resultant position
        match the position that is provided.
    */
    void moveToAbsolute (double absoluteTargetPosition, const NamedCoordinateFinder& nameFinder);

    /** Returns true if the coordinate is calculated as a proportion of the distance between two other points.
        @see toggleProportionality
    */
    bool isProportional() const throw()                     { return anchor2.isNotEmpty(); }

    /** Toggles the coordinate between using a proportional or absolute position.
        Note that calling this will reset the names of any anchor points, and just make the
        coordinate relative to the parent origin and parent size.
    */
    void toggleProportionality (const NamedCoordinateFinder& nameFinder, bool isHorizontal);

    /** Returns a value that can be edited to set this coordinate's position.
        The meaning of this number depends on the coordinate's mode. If the coordinate is
        proportional, the number will be a percentage between 0 and 100. If the
        coordinate is absolute, then it will be the number of pixels from its anchor point.
        @see setEditableNumber
     */
    const double getEditableNumber() const;

    /** Sets the value that controls this coordinate's position.
        The meaning of this number depends on the coordinate's mode. If the coordinate is
        proportional, the number must be a percentage between 0 and 100. If the
        coordinate is absolute, then it indicates the number of pixels from its anchor point.
        @see setEditableNumber
    */
    void setEditableNumber (const double newValue);

    //==============================================================================
    /** Returns the name of the first anchor point from which this coordinate is relative.
    */
    const String getAnchorName1() const                     { return anchor1; }

    /** Returns the name of the second anchor point from which this coordinate is relative.
        The second anchor is only valid if the coordinate is in proportional mode.
    */
    const String getAnchorName2() const                     { return anchor2; }

    /** Returns the first anchor point as a coordinate. */
    const RelativeCoordinate getAnchorCoordinate1() const;

    /** Returns the first anchor point as a coordinate.
        The second anchor is only valid if the coordinate is in proportional mode.
    */
    const RelativeCoordinate getAnchorCoordinate2() const;

    /** Changes the first anchor point, keeping the resultant position of this coordinate in
        the same place it was previously.
    */
    void changeAnchor1 (const String& newAnchor, const NamedCoordinateFinder& nameFinder);

    /** Changes the second anchor point, keeping the resultant position of this coordinate in
        the same place it was previously.
    */
    void changeAnchor2 (const String& newAnchor, const NamedCoordinateFinder& nameFinder);

    /** Tells the coordinate that an object is changing its name or being deleted.

        If either of this coordinates anchor points match this name, they will be replaced.
        If the newName string is empty, it indicates that the object is being removed, so if
        this coordinate was using it, the coordinate is changed to be relative to the origin
        instead.
    */
    void renameAnchorIfUsed (const String& oldName, const String& newName,
                             const NamedCoordinateFinder& nameFinder);

    //==============================================================================
    /** Returns a string which represents this coordinate.
        For details of the string syntax, see the constructor notes.
    */
    const String toString() const;

    //==============================================================================
    /** A set of static strings that are commonly used by the RelativeCoordinate class.

        As well as avoiding using string literals in your code, using these preset values
        has the advantage that all instances of the same string will share the same, reference-counted
        String object, so if you have thousands of points which all refer to the same
        anchor points, this can save a significant amount of memory allocation.
    */
    struct Strings
    {
        static const String parent;         /**< "parent" */
        static const String left;           /**< "left" */
        static const String right;          /**< "right" */
        static const String top;            /**< "top" */
        static const String bottom;         /**< "bottom" */
        static const String parentLeft;     /**< "parent.left" */
        static const String parentTop;      /**< "parent.top" */
        static const String parentRight;    /**< "parent.right" */
        static const String parentBottom;   /**< "parent.bottom" */
    };

private:
    //==============================================================================
    String anchor1, anchor2;
    double value;

    double resolve (const NamedCoordinateFinder& nameFinder, int recursionCounter) const;
    static double resolveAnchor (const String& anchorName, const NamedCoordinateFinder& nameFinder, int recursionCounter);
};


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

    /** Creates a point from a stringified representation.
        The string must contain a pair of coordinates, separated by a comma. The syntax for the coordinate
        strings is explained in the RelativeCoordinate class.
        @see toString
    */
    RelativePoint (const String& stringVersion);

    /** Calculates the absolute position of this point.

        You'll need to provide a suitable NamedCoordinateFinder for looking up any coordinates that may
        be needed to calculate the result.
    */
    const Point<float> resolve (const RelativeCoordinate::NamedCoordinateFinder& nameFinder) const;

    /** Changes the values of this point's coordinates to make it resolve to the specified position.

        Calling this will leave any anchor points unchanged, but will set any absolute
        or relative positions to whatever values are necessary to make the resultant position
        match the position that is provided.
    */
    void moveToAbsolute (const Point<float>& newPos, const RelativeCoordinate::NamedCoordinateFinder& nameFinder);

    /** Returns a string which represents this point.
        This returns a comma-separated pair of coordinates. For details of the string syntax used by the
        coordinates, see the RelativeCoordinate constructor notes.
        The string that is returned can be passed to the RelativePoint constructor to recreate the point.
    */
    const String toString() const;

    /** Tells the point that an object is changing its name or being deleted.
        This calls RelativeCoordinate::renameAnchorIfUsed() on its X and Y coordinates.
    */
    void renameAnchorIfUsed (const String& oldName, const String& newName,
                             const RelativeCoordinate::NamedCoordinateFinder& nameFinder);

    // The actual X and Y coords...
    RelativeCoordinate x, y;
};


//==============================================================================
/**
    An rectangle stored as a set of RelativeCoordinate values.

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
    explicit RelativeRectangle (const Rectangle<float>& rect, const String& componentName);

    /** Creates a rectangle from a stringified representation.
        The string must contain a sequence of 4 coordinates, separated by commas, in the order
        left, top, right, bottom. The syntax for the coordinate strings is explained in the
        RelativeCoordinate class.
        @see toString
    */
    explicit RelativeRectangle (const String& stringVersion);

    //==============================================================================
    /** Calculates the absolute position of this rectangle.

        You'll need to provide a suitable NamedCoordinateFinder for looking up any coordinates that may
        be needed to calculate the result.
    */
    const Rectangle<float> resolve (const RelativeCoordinate::NamedCoordinateFinder& nameFinder) const;

    /** Changes the values of this rectangle's coordinates to make it resolve to the specified position.

        Calling this will leave any anchor points unchanged, but will set any absolute
        or relative positions to whatever values are necessary to make the resultant position
        match the position that is provided.
    */
    void moveToAbsolute (const Rectangle<float>& newPos, const RelativeCoordinate::NamedCoordinateFinder& nameFinder);

    /** Returns a string which represents this point.
        This returns a comma-separated list of coordinates, in the order left, top, right, bottom. For details of
        the string syntax used by the coordinates, see the RelativeCoordinate constructor notes.
        The string that is returned can be passed to the RelativeRectangle constructor to recreate the rectangle.
    */
    const String toString() const;

    /** Tells the rectangle that an object is changing its name or being deleted.
        This calls RelativeCoordinate::renameAnchorIfUsed() on the rectangle's coordinates.
    */
    void renameAnchorIfUsed (const String& oldName, const String& newName,
                             const RelativeCoordinate::NamedCoordinateFinder& nameFinder);

    // The actual rectangle coords...
    RelativeCoordinate left, right, top, bottom;
};


#endif   // __JUCE_RELATIVECOORDINATE_JUCEHEADER__

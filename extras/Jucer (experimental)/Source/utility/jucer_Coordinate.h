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

#ifndef __JUCER_COORDINATE_H_EF56ACFA__
#define __JUCER_COORDINATE_H_EF56ACFA__

#include "../jucer_Headers.h"


//==============================================================================
/**
    Describes a coordinate's, either as an absolute position, or relative to
    other named positions.
*/
class Coordinate
{
public:
    //==============================================================================
    /** Creates a zero coordinate. */
    Coordinate();

    /** Recreates a coordinate from its stringified version. */
    Coordinate (const String& stringVersion, bool isHorizontal);

    /** Creates an absolute position from the parent origin. */
    Coordinate (double absoluteDistanceFromOrigin, bool isHorizontal);

    /** Creates an absolute position relative to a named anchor. */
    Coordinate (double absoluteDistanceFromAnchor, const String& anchorPoint);

    /** Creates a relative position between two named points. */
    Coordinate (double relativeProportionBetweenAnchors, const String& anchorPoint1, const String& anchorPoint2);

    /** Destructor. */
    ~Coordinate();

    //==============================================================================
    /**
        Provides an interface for looking up the position of a named anchor.
    */
    class NamedCoordinateFinder
    {
    public:
        virtual ~NamedCoordinateFinder() {}
        virtual const Coordinate findNamedCoordinate (const String& objectName, const String& edge) const = 0;
    };

    //==============================================================================
    /** Calculates the absolute position of this co-ordinate. */
    double resolve (const NamedCoordinateFinder& nameSource) const;

    /** Returns true if this co-ordinate is expressed in terms of the specified coord at any
        level in its evaluation. */
    bool references (const String& coordName, const NamedCoordinateFinder& nameSource) const;

    //==============================================================================
    /** Changes the value of this coord to make it resolve to the specified position. */
    void moveToAbsolute (double newPos, const NamedCoordinateFinder& nameSource);

    /** */
    bool isProportional() const throw()                     { return anchor2.isNotEmpty(); }

    /** */
    void toggleProportionality (const NamedCoordinateFinder& nameSource, bool isHorizontal);

    /** */
    const double getEditableNumber() const;

    /** */
    void setEditableNumber (const double newValue);

    //==============================================================================
    /** */
    const String getAnchorName1() const                     { return anchor1; }

    /** */
    const String getAnchorName2() const                     { return anchor2; }

    /** */
    const Coordinate getAnchorCoordinate1() const;

    /** */
    const Coordinate getAnchorCoordinate2() const;

    /** */
    void changeAnchor1 (const String& newAnchor, const NamedCoordinateFinder& nameSource);

    /** */
    void changeAnchor2 (const String& newAnchor, const NamedCoordinateFinder& nameSource);

    /** Tells the coord that an anchor is changing its name.
        If the new name is empty, it removes the anchor.
    */
    void renameAnchorIfUsed (const String& oldName, const String& newName, const NamedCoordinateFinder& nameSource);

    //==============================================================================
    /*
        Position string formats:
            123                         = absolute pixels from parent origin
            anchor
            anchor + 123
            anchor - 123
            50%                         = percentage between parent origin and parent extent
            50% * anchor                = percentage between parent origin and anchor
            50% * anchor1 -> anchor2    = percentage between two named points

        where an anchor name can be:
            "parent.top", "parent.left", "parent.bottom", "parent.right"
            "objectName.top", "objectName.left", "objectName.bottom", "objectName.right"
            "markerName"
    */
    const String toString() const;

    //==============================================================================
    struct Strings
    {
        static const String parent;
        static const String left;
        static const String right;
        static const String top;
        static const String bottom;
        static const String originX;
        static const String originY;
        static const String extentX;
        static const String extentY;
    };

    //==============================================================================
    struct RecursiveCoordinateException  : public std::runtime_error
    {
        RecursiveCoordinateException();
    };

private:
    //==============================================================================
    String anchor1, anchor2;
    double value;

    double resolve (const NamedCoordinateFinder& nameSource, int recursionCounter) const;
    double resolveAnchor (const String& name, const NamedCoordinateFinder& nameSource, int recursionCounter) const;
    const String getOriginAnchorName (bool isHorizontal) const throw();
    const String getExtentAnchorName (bool isHorizontal) const throw();
    const Coordinate lookUpName (const String& name, const NamedCoordinateFinder& nameSource) const;
    static bool isOrigin (const String& name);
    static const String getObjectName (const String& fullName);
    static const String getEdgeName (const String& fullName);
};

//==============================================================================
class CoordinatePair
{
public:
    CoordinatePair();
    CoordinatePair (const Point<float>& absolutePoint);
    CoordinatePair (const String& stringVersion);

    const Point<float> resolve (const Coordinate::NamedCoordinateFinder& nameSource) const;
    void moveToAbsolute (const Point<float>& newPos, const Coordinate::NamedCoordinateFinder& nameSource);
    const String toString() const;

    // Tells the coord that an anchor is changing its name.
    void renameAnchorIfUsed (const String& oldName, const String& newName, const Coordinate::NamedCoordinateFinder& nameSource);

    Coordinate x, y;
};

//==============================================================================
/**
    Describes a rectangle as a set of Coordinate values.
*/
class RectangleCoordinates
{
public:
    //==============================================================================
    RectangleCoordinates();
    explicit RectangleCoordinates (const Rectangle<float>& rect, const String& componentName);
    explicit RectangleCoordinates (const String& stringVersion);

    //==============================================================================
    const Rectangle<int> resolve (const Coordinate::NamedCoordinateFinder& nameSource) const;
    void moveToAbsolute (const Rectangle<float>& newPos, const Coordinate::NamedCoordinateFinder& nameSource);
    const String toString() const;

    // Tells the coord that an anchor is changing its name.
    void renameAnchorIfUsed (const String& oldName, const String& newName, const Coordinate::NamedCoordinateFinder& nameSource);

    Coordinate left, right, top, bottom;
};


//==============================================================================
/**
*/
class ComponentAutoLayoutManager    : public ComponentListener,
                                      public Coordinate::NamedCoordinateFinder,
                                      public AsyncUpdater
{
public:
    //==============================================================================
    /**
    */
    ComponentAutoLayoutManager (Component* parentComponent);

    /** Destructor. */
    ~ComponentAutoLayoutManager();

    //==============================================================================
    /**
    */
    void setMarker (const String& name, const Coordinate& coord);

    /**
    */
    void setComponentBounds (Component* component, const String& componentName, const RectangleCoordinates& bounds);

    /**
    */
    void applyLayout();

    //==============================================================================
    /** @internal */
    const Coordinate findNamedCoordinate (const String& objectName, const String& edge) const;
    /** @internal */
    void componentMovedOrResized (Component& component, bool wasMoved, bool wasResized);
    /** @internal */
    void componentBeingDeleted (Component& component);
    /** @internal */
    void handleAsyncUpdate();

    juce_UseDebuggingNewOperator

private:
    //==============================================================================
    struct ComponentPosition
    {
        ComponentPosition (Component* component, const String& name, const RectangleCoordinates& coords);

        Component* component;
        String name;
        RectangleCoordinates coords;
    };

    struct MarkerPosition
    {
        MarkerPosition (const String& name, const Coordinate& coord);

        String markerName;
        Coordinate position;
    };

    Component* parent;
    OwnedArray <ComponentPosition> components;
    OwnedArray <MarkerPosition> markers;

    ComponentAutoLayoutManager (const ComponentAutoLayoutManager&);
    ComponentAutoLayoutManager& operator= (const ComponentAutoLayoutManager&);
};


#endif  // __JUCER_COORDINATE_H_EF56ACFA__

/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

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
    Holds a co-ordinate along the x or y axis, expressed either as an absolute
    position, or relative to other named marker positions.
*/
class Coordinate
{
public:
    //==============================================================================
    /** Creates a zero coordinate. */
    explicit Coordinate (bool isHorizontal);

    /** Recreates a coordinate from its stringified version. */
    Coordinate (const String& stringVersion, bool isHorizontal);

    /** Creates an absolute position from the parent origin. */
    Coordinate (double absoluteDistanceFromOrigin, bool isHorizontal);

    /** Creates an absolute position relative to a named marker. */
    Coordinate (double absolutePosition, const String& relativeToMarker, bool isHorizontal);

    /** Creates a relative position between two named markers. */
    Coordinate (double relativePosition, const String& marker1, const String& marker2, bool isHorizontal);

    /** Destructor. */
    ~Coordinate();

    //==============================================================================
    /**
        Provides an interface for looking up the position of a named marker.
    */
    class MarkerResolver
    {
    public:
        virtual ~MarkerResolver() {}
        virtual const Coordinate findMarker (const String& name, bool isHorizontal) = 0;
    };

    /** Calculates the absolute position of this co-ordinate. */
    double resolve (MarkerResolver& markerResolver) const;

    /** Returns true if this co-ordinate is expressed in terms of markers that form a recursive loop. */
    bool isRecursive (MarkerResolver& markerResolver) const;

    /** Changes the value of this marker to make it resolve to the specified position. */
    void moveToAbsolute (double newPos, MarkerResolver& markerResolver);

    const Coordinate getAnchorPoint1() const;
    const Coordinate getAnchorPoint2() const;

    const double getEditableValue() const;
    void setEditableValue (const double newValue);

    bool isProportional() const throw()                 { return isProportion; }
    void toggleProportionality (MarkerResolver& markerResolver);

    const String getAnchor1() const                     { return checkName (anchor1); }
    void changeAnchor1 (const String& newMarkerName, MarkerResolver& markerResolver);

    const String getAnchor2() const                     { return checkName (anchor2); }
    void changeAnchor2 (const String& newMarkerName, MarkerResolver& markerResolver);

    //==============================================================================
    /*
        Position string formats:
            123                         = absolute pixels from parent origin
            marker
            marker + 123
            marker - 123
            50%                         = percentage between parent origin and parent extent
            50% * marker                = percentage between parent origin and marker
            50% * marker1 -> marker2    = percentage between two markers

        standard marker names:
            "origin"                            = parent origin
            "size"                              = parent right or bottom
            "top", "left", "bottom", "right"    = refer to the component's own left, right, top and bottom.
    */
    const String toString() const;

    //==============================================================================
    static const char* parentLeftMarkerName;
    static const char* parentRightMarkerName;
    static const char* parentTopMarkerName;
    static const char* parentBottomMarkerName;

private:
    //==============================================================================
    String anchor1, anchor2;
    double value;
    bool isProportion, isHorizontal;

    double resolve (MarkerResolver& markerResolver, int recursionCounter) const;
    double getPosition (const String& name, MarkerResolver& markerResolver, int recursionCounter) const;
    const String checkName (const String& name) const;
    const String getOriginMarkerName() const;
    const String getExtentMarkerName() const;
    static bool isOrigin (const String& name);
    static void skipWhitespace (const String& s, int& i);
    static const String readMarkerName (const String& s, int& i);
    static double readNumber (const String& s, int& i);
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
    explicit RectangleCoordinates (const Rectangle<int>& rect);
    explicit RectangleCoordinates (const String& stringVersion);

    //==============================================================================
    const Rectangle<int> resolve (Coordinate::MarkerResolver& markerResolver) const;
    bool isRecursive (Coordinate::MarkerResolver& markerResolver) const;
    void moveToAbsolute (const Rectangle<int>& newPos, Coordinate::MarkerResolver& markerResolver);
    const String toString() const;

    Coordinate left, right, top, bottom;
};


#endif  // __JUCER_COORDINATE_H_EF56ACFA__

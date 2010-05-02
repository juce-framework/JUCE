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

#ifndef __JUCER_MARKERLISTBASE_H_E0091A88__
#define __JUCER_MARKERLISTBASE_H_E0091A88__


//==============================================================================
class MarkerListBase    : public Coordinate::MarkerResolver
{
public:
    MarkerListBase (const ValueTree& group_, bool isX_)   : group (group_), isX (isX_) {}
    virtual ~MarkerListBase() {}

    static const String getId (const ValueTree& markerState)                    { return markerState [getIdProperty()]; }
    ValueTree& getGroup()                                                       { return group; }
    int size() const                                                            { return group.getNumChildren(); }
    ValueTree getMarker (int index) const                                       { return group.getChild (index); }
    ValueTree getMarkerNamed (const String& name) const                         { return group.getChildWithProperty (getMarkerNameProperty(), name); }
    bool contains (const ValueTree& markerState) const                          { return markerState.isAChildOf (group); }
    const Coordinate getCoordinate (const ValueTree& markerState) const         { return Coordinate (markerState [getMarkerNameProperty()].toString(), isX); }
    const String getName (const ValueTree& markerState) const                   { return markerState [getMarkerNameProperty()].toString(); }
    Value getNameAsValue (const ValueTree& markerState) const                   { return markerState.getPropertyAsValue (getMarkerNameProperty(), getUndoManager()); }
    void setCoordinate (ValueTree& markerState, const Coordinate& newCoord)     { markerState.setProperty (getMarkerNameProperty(), newCoord.toString(), getUndoManager()); }

    void createMarker (const String& name, int position)
    {
        ValueTree marker (getMarkerTag());
        marker.setProperty (getMarkerNameProperty(), getNonexistentMarkerName (name), 0);
        marker.setProperty (getMarkerPosProperty(), Coordinate (position, isX).toString(), 0);
        marker.setProperty (getIdProperty(), createAlphaNumericUID(), 0);
        group.addChild (marker, -1, getUndoManager());
    }

    void deleteMarker (ValueTree& markerState)                                  { group.removeChild (markerState, getUndoManager()); }

    bool createProperties (Array <PropertyComponent*>& props, const String& itemId);

    //==============================================================================
    static const char* getMarkerTag()           { return "MARKER"; }
    static const char* getIdProperty()          { return "id"; }
    static const char* getMarkerNameProperty()  { return "name"; }
    static const char* getMarkerPosProperty()   { return "position"; }

protected:
    virtual UndoManager* getUndoManager() const = 0;
    virtual const String getNonexistentMarkerName (const String& name) = 0;

    ValueTree group;
    const bool isX;
};


#endif  // __JUCER_MARKERLISTBASE_H_E0091A88__

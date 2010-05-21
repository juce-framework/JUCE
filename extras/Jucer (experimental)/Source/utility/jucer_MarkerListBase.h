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

#include "jucer_CoordinatePropertyComponent.h"


//==============================================================================
class MarkerListBase    : public RelativeCoordinate::NamedCoordinateFinder
{
public:
    MarkerListBase (const ValueTree& group_, bool isX_)   : group (group_), isX (isX_) {}
    virtual ~MarkerListBase() {}

    bool isHorizontal() const                                                   { return isX; }
    static const String getId (const ValueTree& markerState)                    { return markerState [getIdProperty()]; }
    ValueTree& getGroup()                                                       { return group; }
    int size() const                                                            { return group.getNumChildren(); }
    ValueTree getMarker (int index) const                                       { return group.getChild (index); }
    ValueTree getMarkerNamed (const String& name) const                         { return group.getChildWithProperty (getMarkerNameProperty(), name); }
    bool contains (const ValueTree& markerState) const                          { return markerState.isAChildOf (group); }

    const String getName (const ValueTree& markerState) const                   { return markerState [getMarkerNameProperty()].toString(); }
    Value getNameAsValue (const ValueTree& markerState) const                   { return markerState.getPropertyAsValue (getMarkerNameProperty(), getUndoManager()); }

    const RelativeCoordinate getCoordinate (const ValueTree& markerState) const         { return RelativeCoordinate (markerState [getMarkerPosProperty()].toString(), isX); }
    void setCoordinate (ValueTree& markerState, const RelativeCoordinate& newCoord)     { markerState.setProperty (getMarkerPosProperty(), newCoord.toString(), getUndoManager()); }

    void renameAnchorInMarkers (const String& oldName, const String& newName)
    {
        for (int i = size(); --i >= 0;)
        {
            ValueTree v (getMarker (i));
            RelativeCoordinate coord (getCoordinate (v));
            coord.renameAnchorIfUsed (oldName, newName, this);
            setCoordinate (v, coord);
        }
    }

    void createMarker (const String& name, int position)
    {
        ValueTree marker (getMarkerTag());
        marker.setProperty (getMarkerNameProperty(), getNonexistentMarkerName (name), 0);
        marker.setProperty (getMarkerPosProperty(), RelativeCoordinate (position, isX).toString(), 0);
        marker.setProperty (getIdProperty(), createAlphaNumericUID(), 0);
        group.addChild (marker, -1, getUndoManager());
    }

    void deleteMarker (ValueTree& markerState)
    {
        renameAnchor (getName (markerState), String::empty);
        group.removeChild (markerState, getUndoManager());
    }

    //==============================================================================
    virtual UndoManager* getUndoManager() const = 0;
    virtual const String getNonexistentMarkerName (const String& name) = 0;
    virtual void renameAnchor (const String& oldName, const String& newName) = 0;
    virtual void addMarkerMenuItems (const ValueTree& markerState, const RelativeCoordinate& coord, PopupMenu& menu, bool isAnchor1) = 0;
    virtual const String getChosenMarkerMenuItem (const RelativeCoordinate& coord, int itemId) const = 0;

    //==============================================================================
    static const Identifier getMarkerTag()           { static Identifier i ("MARKER"); return i; }
    static const Identifier getIdProperty()          { return Ids::id_;  }
    static const Identifier getMarkerNameProperty()  { return Ids::name;  }
    static const Identifier getMarkerPosProperty()   { return Ids::position;  }

    //==============================================================================
    class MarkerNameValueSource   : public Value::ValueSource,
                                    public Value::Listener
    {
    public:
        MarkerNameValueSource (MarkerListBase* markerList_, const Value& value)
           : sourceValue (value),
             markerList (markerList_)
        {
            sourceValue.addListener (this);
        }

        ~MarkerNameValueSource() {}

        void valueChanged (Value&)      { sendChangeMessage (true); }
        const var getValue() const      { return sourceValue.toString(); }

        void setValue (const var& newValue)
        {
            if (newValue == sourceValue)
                return;

            const String name (markerList->getNonexistentMarkerName (newValue));

            if (sourceValue != name)
            {
                markerList->renameAnchor (sourceValue.toString(), name);
                sourceValue = name;
            }
        }

    private:
        Value sourceValue;
        MarkerListBase* markerList;

        MarkerNameValueSource (const MarkerNameValueSource&);
        const MarkerNameValueSource& operator= (const MarkerNameValueSource&);
    };

    //==============================================================================
    class PositionPropertyComponent  : public CoordinatePropertyComponent
    {
    public:
        //==============================================================================
        PositionPropertyComponent (NamedCoordinateFinder* nameSource_, MarkerListBase& markerList_,
                                   const String& name, const ValueTree& markerState_,
                                   const Value& coordValue_)
            : CoordinatePropertyComponent (nameSource_, name, coordValue_, markerList_.isHorizontal()),
              markerList (markerList_),
              markerState (markerState_)
        {
        }

        ~PositionPropertyComponent()
        {
        }

        const String pickMarker (TextButton* button, const String& currentMarker, bool isAnchor1)
        {
            RelativeCoordinate coord (getCoordinate());

            PopupMenu m;
            markerList.addMarkerMenuItems (markerState, coord, m, isAnchor1);

            const int r = m.showAt (button);

            if (r > 0)
                return markerList.getChosenMarkerMenuItem (coord, r);

            return String::empty;
        }

    private:
        MarkerListBase& markerList;
        ValueTree markerState;
    };

    //==============================================================================
protected:
    ValueTree group;
    const bool isX;

private:
    MarkerListBase (const MarkerListBase&);
    MarkerListBase& operator= (const MarkerListBase&);
};


#endif  // __JUCER_MARKERLISTBASE_H_E0091A88__

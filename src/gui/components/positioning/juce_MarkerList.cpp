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

#include "../../../core/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_MarkerList.h"
#include "../juce_Component.h"

//==============================================================================
MarkerList::MarkerList()
{
}

MarkerList::MarkerList (const MarkerList& other)
{
    operator= (other);
}

MarkerList& MarkerList::operator= (const MarkerList& other)
{
    if (other != *this)
    {
        markers.clear();
        markers.addCopiesOf (other.markers);
        markersHaveChanged();
    }

    return *this;
}

MarkerList::~MarkerList()
{
    listeners.call (&MarkerList::Listener::markerListBeingDeleted, this);
}

bool MarkerList::operator== (const MarkerList& other) const throw()
{
    if (other.markers.size() != markers.size())
        return false;

    for (int i = markers.size(); --i >= 0;)
    {
        const Marker* const m1 = markers.getUnchecked(i);
        jassert (m1 != 0);

        const Marker* const m2 = other.getMarker (m1->name);

        if (m2 == 0 || *m1 != *m2)
            return false;
    }

    return true;
}

bool MarkerList::operator!= (const MarkerList& other) const throw()
{
    return ! operator== (other);
}

//==============================================================================
int MarkerList::getNumMarkers() const throw()
{
    return markers.size();
}

const MarkerList::Marker* MarkerList::getMarker (const int index) const throw()
{
    return markers [index];
}

const MarkerList::Marker* MarkerList::getMarker (const String& name) const throw()
{
    for (int i = 0; i < markers.size(); ++i)
    {
        const Marker* const m = markers.getUnchecked(i);

        if (m->name == name)
            return m;
    }

    return 0;
}

void MarkerList::setMarker (const String& name, const RelativeCoordinate& position)
{
    Marker* const m = const_cast <Marker*> (getMarker (name));

    if (m != 0)
    {
        if (m->position != position)
        {
            m->position = position;
            markersHaveChanged();
        }

        return;
    }

    markers.add (new Marker (name, position));
    markersHaveChanged();
}

void MarkerList::removeMarker (const int index)
{
    if (isPositiveAndBelow (index, markers.size()))
    {
        markers.remove (index);
        markersHaveChanged();
    }
}

void MarkerList::removeMarker (const String& name)
{
    for (int i = 0; i < markers.size(); ++i)
    {
        const Marker* const m = markers.getUnchecked(i);

        if (m->name == name)
        {
            markers.remove (i);
            markersHaveChanged();
        }
    }
}

void MarkerList::markersHaveChanged()
{
    listeners.call (&MarkerList::Listener::markersChanged, this);
}

void MarkerList::Listener::markerListBeingDeleted (MarkerList*)
{
}

void MarkerList::addListener (Listener* listener)
{
    listeners.add (listener);
}

void MarkerList::removeListener (Listener* listener)
{
    listeners.remove (listener);
}

//==============================================================================
MarkerList::Marker::Marker (const Marker& other)
    : name (other.name), position (other.position)
{
}

MarkerList::Marker::Marker (const String& name_, const RelativeCoordinate& position_)
    : name (name_), position (position_)
{
}

bool MarkerList::Marker::operator== (const Marker& other) const throw()
{
    return name == other.name && position == other.position;
}

bool MarkerList::Marker::operator!= (const Marker& other) const throw()
{
    return ! operator== (other);
}

//==============================================================================
const Identifier MarkerList::ValueTreeWrapper::markerTag ("Marker");
const Identifier MarkerList::ValueTreeWrapper::nameProperty ("name");
const Identifier MarkerList::ValueTreeWrapper::posProperty ("position");

MarkerList::ValueTreeWrapper::ValueTreeWrapper (const ValueTree& state_)
    : state (state_)
{
}

int MarkerList::ValueTreeWrapper::getNumMarkers() const
{
    return state.getNumChildren();
}

const ValueTree MarkerList::ValueTreeWrapper::getMarkerState (int index) const
{
    return state.getChild (index);
}

const ValueTree MarkerList::ValueTreeWrapper::getMarkerState (const String& name) const
{
    return state.getChildWithProperty (nameProperty, name);
}

bool MarkerList::ValueTreeWrapper::containsMarker (const ValueTree& marker) const
{
    return marker.isAChildOf (state);
}

const MarkerList::Marker MarkerList::ValueTreeWrapper::getMarker (const ValueTree& marker) const
{
    jassert (containsMarker (marker));

    return MarkerList::Marker (marker [nameProperty], RelativeCoordinate (marker [posProperty].toString()));
}

void MarkerList::ValueTreeWrapper::setMarker (const MarkerList::Marker& m, UndoManager* undoManager)
{
    ValueTree marker (state.getChildWithProperty (nameProperty, m.name));

    if (marker.isValid())
    {
        marker.setProperty (posProperty, m.position.toString(), undoManager);
    }
    else
    {
        marker = ValueTree (markerTag);
        marker.setProperty (nameProperty, m.name, 0);
        marker.setProperty (posProperty, m.position.toString(), 0);
        state.addChild (marker, -1, undoManager);
    }
}

void MarkerList::ValueTreeWrapper::removeMarker (const ValueTree& marker, UndoManager* undoManager)
{
    state.removeChild (marker, undoManager);
}

//==============================================================================
class MarkerListEvaluator   : public Expression::EvaluationContext
{
public:
    MarkerListEvaluator (const MarkerList& markerList_, Component* const parentComponent_)
        : markerList (markerList_), parentComponent (parentComponent_)
    {
    }

    const Expression getSymbolValue (const String& objectName, const String& member) const
    {
        if (member.isEmpty())
        {
            const MarkerList::Marker* const marker = markerList.getMarker (objectName);

            if (marker != 0)
                return Expression (marker->position.resolve (this));
        }
        else if (parentComponent != 0 && objectName == RelativeCoordinate::Strings::parent)
        {
            if (member == RelativeCoordinate::Strings::right)  return Expression ((double) parentComponent->getWidth());
            if (member == RelativeCoordinate::Strings::bottom) return Expression ((double) parentComponent->getHeight());
        }

        return Expression::EvaluationContext::getSymbolValue (objectName, member);
    }

private:
    const MarkerList& markerList;
    Component* parentComponent;

    JUCE_DECLARE_NON_COPYABLE (MarkerListEvaluator);
};

double MarkerList::getMarkerPosition (const Marker& marker, Component* const parentComponent) const
{
    MarkerListEvaluator context (*this, parentComponent);
    return marker.position.resolve (&context);
}

//==============================================================================
void MarkerList::ValueTreeWrapper::applyTo (MarkerList& markerList)
{
    const int numMarkers = getNumMarkers();

    StringArray updatedMarkers;

    int i;
    for (i = 0; i < numMarkers; ++i)
    {
        const ValueTree marker (state.getChild (i));
        const String name (marker [nameProperty].toString());
        markerList.setMarker (name, RelativeCoordinate (marker [posProperty].toString()));
        updatedMarkers.add (name);
    }

    for (i = markerList.getNumMarkers(); --i >= 0;)
        if (! updatedMarkers.contains (markerList.getMarker (i)->name))
            markerList.removeMarker (i);
}

void MarkerList::ValueTreeWrapper::readFrom (const MarkerList& markerList, UndoManager* undoManager)
{
    state.removeAllChildren (undoManager);

    for (int i = 0; i < markerList.getNumMarkers(); ++i)
        setMarker (*markerList.getMarker(i), undoManager);
}

END_JUCE_NAMESPACE

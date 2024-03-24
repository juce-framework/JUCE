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
    listeners.call ([this] (Listener& l) { l.markerListBeingDeleted (this); });
}

bool MarkerList::operator== (const MarkerList& other) const noexcept
{
    if (other.markers.size() != markers.size())
        return false;

    for (int i = markers.size(); --i >= 0;)
    {
        const Marker* const m1 = markers.getUnchecked (i);
        jassert (m1 != nullptr);

        const Marker* const m2 = other.getMarker (m1->name);

        if (m2 == nullptr || *m1 != *m2)
            return false;
    }

    return true;
}

bool MarkerList::operator!= (const MarkerList& other) const noexcept
{
    return ! operator== (other);
}

//==============================================================================
int MarkerList::getNumMarkers() const noexcept
{
    return markers.size();
}

const MarkerList::Marker* MarkerList::getMarker (const int index) const noexcept
{
    return markers [index];
}

const MarkerList::Marker* MarkerList::getMarker (const String& name) const noexcept
{
    return getMarkerByName (name);
}

MarkerList::Marker* MarkerList::getMarkerByName (const String& name) const noexcept
{
    for (int i = 0; i < markers.size(); ++i)
    {
        Marker* const m = markers.getUnchecked (i);

        if (m->name == name)
            return m;
    }

    return nullptr;
}

void MarkerList::setMarker (const String& name, const RelativeCoordinate& position)
{
    if (Marker* const m = getMarkerByName (name))
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
        const Marker* const m = markers.getUnchecked (i);

        if (m->name == name)
        {
            markers.remove (i);
            markersHaveChanged();
        }
    }
}

void MarkerList::markersHaveChanged()
{
    listeners.call ([this] (Listener& l) { l.markersChanged (this); });
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

bool MarkerList::Marker::operator== (const Marker& other) const noexcept
{
    return name == other.name && position == other.position;
}

bool MarkerList::Marker::operator!= (const Marker& other) const noexcept
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

ValueTree MarkerList::ValueTreeWrapper::getMarkerState (int index) const
{
    return state.getChild (index);
}

ValueTree MarkerList::ValueTreeWrapper::getMarkerState (const String& name) const
{
    return state.getChildWithProperty (nameProperty, name);
}

bool MarkerList::ValueTreeWrapper::containsMarker (const ValueTree& marker) const
{
    return marker.isAChildOf (state);
}

MarkerList::Marker MarkerList::ValueTreeWrapper::getMarker (const ValueTree& marker) const
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
        marker.setProperty (nameProperty, m.name, nullptr);
        marker.setProperty (posProperty, m.position.toString(), nullptr);
        state.appendChild (marker, undoManager);
    }
}

void MarkerList::ValueTreeWrapper::removeMarker (const ValueTree& marker, UndoManager* undoManager)
{
    state.removeChild (marker, undoManager);
}

double MarkerList::getMarkerPosition (const Marker& marker, Component* parentComponent) const
{
    if (parentComponent == nullptr)
        return marker.position.resolve (nullptr);

    RelativeCoordinatePositionerBase::ComponentScope scope (*parentComponent);
    return marker.position.resolve (&scope);
}

//==============================================================================
void MarkerList::ValueTreeWrapper::applyTo (MarkerList& markerList)
{
    const int numMarkers = getNumMarkers();

    StringArray updatedMarkers;

    for (int i = 0; i < numMarkers; ++i)
    {
        const ValueTree marker (state.getChild (i));
        const String name (marker [nameProperty].toString());
        markerList.setMarker (name, RelativeCoordinate (marker [posProperty].toString()));
        updatedMarkers.add (name);
    }

    for (int i = markerList.getNumMarkers(); --i >= 0;)
        if (! updatedMarkers.contains (markerList.getMarker (i)->name))
            markerList.removeMarker (i);
}

void MarkerList::ValueTreeWrapper::readFrom (const MarkerList& markerList, UndoManager* undoManager)
{
    state.removeAllChildren (undoManager);

    for (int i = 0; i < markerList.getNumMarkers(); ++i)
        setMarker (*markerList.getMarker (i), undoManager);
}

} // namespace juce

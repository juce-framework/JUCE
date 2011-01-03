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

#include "juce_RelativeCoordinatePositioner.h"


//==============================================================================
RelativeCoordinatePositionerBase::RelativeCoordinatePositionerBase (Component& component_)
    : Component::Positioner (component_), registeredOk (false)
{
}

RelativeCoordinatePositionerBase::~RelativeCoordinatePositionerBase()
{
    unregisterListeners();
}

const Expression RelativeCoordinatePositionerBase::getSymbolValue (const String& objectName, const String& member) const
{
    jassert (objectName.isNotEmpty());

    if (member.isNotEmpty())
    {
        const Component* comp = getSourceComponent (objectName);

        if (comp == 0)
        {
            if (objectName == RelativeCoordinate::Strings::parent)
                comp = getComponent().getParentComponent();
            else if (objectName == RelativeCoordinate::Strings::this_ || objectName == getComponent().getComponentID())
                comp = &getComponent();
        }

        if (comp != 0)
        {
            if (member == RelativeCoordinate::Strings::left)   return xToExpression (comp, 0);
            if (member == RelativeCoordinate::Strings::right)  return xToExpression (comp, comp->getWidth());
            if (member == RelativeCoordinate::Strings::top)    return yToExpression (comp, 0);
            if (member == RelativeCoordinate::Strings::bottom) return yToExpression (comp, comp->getHeight());
        }
    }

    for (int i = sourceMarkerLists.size(); --i >= 0;)
    {
        MarkerList* const markerList = sourceMarkerLists.getUnchecked(i);
        const MarkerList::Marker* const marker = markerList->getMarker (objectName);

        if (marker != 0)
            return marker->position.getExpression();
    }

    return Expression::EvaluationContext::getSymbolValue (objectName, member);
}

void RelativeCoordinatePositionerBase::componentMovedOrResized (Component&, bool /*wasMoved*/, bool /*wasResized*/)
{
    apply();
}

void RelativeCoordinatePositionerBase::componentParentHierarchyChanged (Component&)
{
    apply();
}

void RelativeCoordinatePositionerBase::componentBeingDeleted (Component& component)
{
    jassert (sourceComponents.contains (&component));
    sourceComponents.removeValue (&component);
}

void RelativeCoordinatePositionerBase::markersChanged (MarkerList*)
{
    apply();
}

void RelativeCoordinatePositionerBase::markerListBeingDeleted (MarkerList* markerList)
{
    jassert (sourceMarkerLists.contains (markerList));
    sourceMarkerLists.removeValue (markerList);
}

void RelativeCoordinatePositionerBase::apply()
{
    if (! registeredOk)
    {
        unregisterListeners();
        registeredOk = registerCoordinates();
    }

    applyToComponentBounds();
}

bool RelativeCoordinatePositionerBase::addCoordinate (const RelativeCoordinate& coord)
{
    return registerListeners (coord.getExpression());
}

bool RelativeCoordinatePositionerBase::registerListeners (const Expression& e)
{
    bool ok = true;

    if (e.getType() == Expression::symbolType)
    {
        String objectName, memberName;
        e.getSymbolParts (objectName, memberName);

        if (memberName.isNotEmpty())
            ok = registerComponent (objectName) && ok;
        else
            ok = registerMarker (objectName) && ok;
    }
    else
    {
        for (int i = e.getNumInputs(); --i >= 0;)
            ok = registerListeners (e.getInput (i)) && ok;
    }

    return ok;
}

bool RelativeCoordinatePositionerBase::registerComponent (const String& componentID)
{
    Component* comp = findComponent (componentID);

    if (comp == 0)
    {
        if (componentID == RelativeCoordinate::Strings::parent)
            comp = getComponent().getParentComponent();
        else if (componentID == RelativeCoordinate::Strings::this_ || componentID == getComponent().getComponentID())
            comp = &getComponent();
    }

    if (comp != 0)
    {
        if (comp != &getComponent())
            registerComponentListener (comp);

        return true;
    }
    else
    {
        // The component we want doesn't exist, so watch the parent in case the hierarchy changes and it appears later..
        Component* const parent = getComponent().getParentComponent();

        if (parent != 0)
            registerComponentListener (parent);
        else
            registerComponentListener (&getComponent());

        return false;
    }
}

bool RelativeCoordinatePositionerBase::registerMarker (const String markerName)
{
    Component* const parent = getComponent().getParentComponent();

    if (parent != 0)
    {
        MarkerList* list = parent->getMarkers (true);

        if (list == 0 || list->getMarker (markerName) == 0)
            list = parent->getMarkers (false);

        if (list != 0 && list->getMarker (markerName) != 0)
        {
            registerMarkerListListener (list);
            return true;
        }
        else
        {
            // The marker we want doesn't exist, so watch all lists in case they change and the marker appears later..
            registerMarkerListListener (parent->getMarkers (true));
            registerMarkerListListener (parent->getMarkers (false));
        }
    }

    return false;
}

void RelativeCoordinatePositionerBase::registerComponentListener (Component* const comp)
{
    if (comp != 0 && ! sourceComponents.contains (comp))
    {
        comp->addComponentListener (this);
        sourceComponents.add (comp);
    }
}

void RelativeCoordinatePositionerBase::registerMarkerListListener (MarkerList* const list)
{
    if (list != 0 && ! sourceMarkerLists.contains (list))
    {
        list->addListener (this);
        sourceMarkerLists.add (list);
    }
}

void RelativeCoordinatePositionerBase::unregisterListeners()
{
    int i;
    for (i = sourceComponents.size(); --i >= 0;)
        sourceComponents.getUnchecked(i)->removeComponentListener (this);

    for (i = sourceMarkerLists.size(); --i >= 0;)
        sourceMarkerLists.getUnchecked(i)->removeListener (this);

    sourceComponents.clear();
    sourceMarkerLists.clear();
}

Component* RelativeCoordinatePositionerBase::findComponent (const String& componentID) const
{
    Component* const parent = getComponent().getParentComponent();

    if (parent != 0)
    {
        for (int i = parent->getNumChildComponents(); --i >= 0;)
        {
            Component* const c = parent->getChildComponent(i);

            if (c->getComponentID() == componentID)
                return c;
        }
    }

    return 0;
}

Component* RelativeCoordinatePositionerBase::getSourceComponent (const String& objectName) const
{
    for (int i = sourceComponents.size(); --i >= 0;)
    {
        Component* const comp = sourceComponents.getUnchecked(i);

        if (comp->getComponentID() == objectName)
            return comp;
    }

    return 0;
}

const Expression RelativeCoordinatePositionerBase::xToExpression (const Component* const source, const int x) const
{
    return Expression ((double) (getComponent().getLocalPoint (source, Point<int> (x, 0)).getX() + getComponent().getX()));
}

const Expression RelativeCoordinatePositionerBase::yToExpression (const Component* const source, const int y) const
{
    return Expression ((double) (getComponent().getLocalPoint (source, Point<int> (0, y)).getY() + getComponent().getY()));
}

END_JUCE_NAMESPACE

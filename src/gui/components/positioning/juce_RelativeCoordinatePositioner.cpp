/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

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
RelativeCoordinatePositionerBase::ComponentScope::ComponentScope (Component& component_)
    : component (component_)
{
}

Expression RelativeCoordinatePositionerBase::ComponentScope::getSymbolValue (const String& symbol) const
{
    switch (RelativeCoordinate::StandardStrings::getTypeOf (symbol))
    {
        case RelativeCoordinate::StandardStrings::x:
        case RelativeCoordinate::StandardStrings::left:   return Expression ((double) component.getX());
        case RelativeCoordinate::StandardStrings::y:
        case RelativeCoordinate::StandardStrings::top:    return Expression ((double) component.getY());
        case RelativeCoordinate::StandardStrings::width:  return Expression ((double) component.getWidth());
        case RelativeCoordinate::StandardStrings::height: return Expression ((double) component.getHeight());
        case RelativeCoordinate::StandardStrings::right:  return Expression ((double) component.getRight());
        case RelativeCoordinate::StandardStrings::bottom: return Expression ((double) component.getBottom());
        default: break;
    }

    MarkerList* list;
    const MarkerList::Marker* const marker = findMarker (symbol, list);

    if (marker != nullptr)
        return marker->position.getExpression();

    return Expression::Scope::getSymbolValue (symbol);
}

void RelativeCoordinatePositionerBase::ComponentScope::visitRelativeScope (const String& scopeName, Visitor& visitor) const
{
    Component* targetComp = nullptr;

    if (scopeName == RelativeCoordinate::Strings::parent)
        targetComp = component.getParentComponent();
    else
        targetComp = findSiblingComponent (scopeName);

    if (targetComp != nullptr)
        visitor.visit (ComponentScope (*targetComp));
    else
        Expression::Scope::visitRelativeScope (scopeName, visitor);
}

String RelativeCoordinatePositionerBase::ComponentScope::getScopeUID() const
{
    return String::toHexString ((int) (pointer_sized_int) (void*) &component);
}

Component* RelativeCoordinatePositionerBase::ComponentScope::findSiblingComponent (const String& componentID) const
{
    Component* const parent = component.getParentComponent();

    if (parent != nullptr)
    {
        for (int i = parent->getNumChildComponents(); --i >= 0;)
        {
            Component* const c = parent->getChildComponent(i);

            if (c->getComponentID() == componentID)
                return c;
        }
    }

    return nullptr;
}

const MarkerList::Marker* RelativeCoordinatePositionerBase::ComponentScope::findMarker (const String& name, MarkerList*& list) const
{
    const MarkerList::Marker* marker = nullptr;

    Component* const parent = component.getParentComponent();

    if (parent != nullptr)
    {
        list = parent->getMarkers (true);
        if (list != nullptr)
            marker = list->getMarker (name);

        if (marker == nullptr)
        {
            list = parent->getMarkers (false);

            if (list != nullptr)
                marker = list->getMarker (name);
        }
    }

    return marker;
}

//==============================================================================
class RelativeCoordinatePositionerBase::DependencyFinderScope  : public ComponentScope
{
public:
    DependencyFinderScope (Component& component_, RelativeCoordinatePositionerBase& positioner_, bool& ok_)
        : ComponentScope (component_), positioner (positioner_), ok (ok_)
    {
    }

    Expression getSymbolValue (const String& symbol) const
    {
        if (symbol == RelativeCoordinate::Strings::left || symbol == RelativeCoordinate::Strings::x
             || symbol == RelativeCoordinate::Strings::width || symbol == RelativeCoordinate::Strings::right
             || symbol == RelativeCoordinate::Strings::top || symbol == RelativeCoordinate::Strings::y
             || symbol == RelativeCoordinate::Strings::height || symbol == RelativeCoordinate::Strings::bottom)
        {
            positioner.registerComponentListener (component);
        }
        else
        {
            MarkerList* list;
            const MarkerList::Marker* const marker = findMarker (symbol, list);

            if (marker != nullptr)
            {
                positioner.registerMarkerListListener (list);
            }
            else
            {
                // The marker we want doesn't exist, so watch all lists in case they change and the marker appears later..
                positioner.registerMarkerListListener (component.getMarkers (true));
                positioner.registerMarkerListListener (component.getMarkers (false));
                ok = false;
            }
        }

        return ComponentScope::getSymbolValue (symbol);
    }

    void visitRelativeScope (const String& scopeName, Visitor& visitor) const
    {
        Component* targetComp = nullptr;

        if (scopeName == RelativeCoordinate::Strings::parent)
            targetComp = component.getParentComponent();
        else
            targetComp = findSiblingComponent (scopeName);

        if (targetComp != nullptr)
        {
            visitor.visit (DependencyFinderScope (*targetComp, positioner, ok));
        }
        else
        {
            // The named component doesn't exist, so we'll watch the parent for changes in case it appears later..
            Component* const parent = component.getParentComponent();
            if (parent != nullptr)
                positioner.registerComponentListener (*parent);

            positioner.registerComponentListener (component);
            ok = false;
        }
    }

private:
    RelativeCoordinatePositionerBase& positioner;
    bool& ok;

    JUCE_DECLARE_NON_COPYABLE (DependencyFinderScope);
};

//==============================================================================
RelativeCoordinatePositionerBase::RelativeCoordinatePositionerBase (Component& component_)
    : Component::Positioner (component_), registeredOk (false)
{
}

RelativeCoordinatePositionerBase::~RelativeCoordinatePositionerBase()
{
    unregisterListeners();
}

void RelativeCoordinatePositionerBase::componentMovedOrResized (Component&, bool /*wasMoved*/, bool /*wasResized*/)
{
    apply();
}

void RelativeCoordinatePositionerBase::componentParentHierarchyChanged (Component&)
{
    apply();
}

void RelativeCoordinatePositionerBase::componentChildrenChanged (Component& changed)
{
    if (getComponent().getParentComponent() == &changed && ! registeredOk)
        apply();
}

void RelativeCoordinatePositionerBase::componentBeingDeleted (Component& comp)
{
    jassert (sourceComponents.contains (&comp));
    sourceComponents.removeValue (&comp);
    registeredOk = false;
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
    bool ok = true;
    DependencyFinderScope finderScope (getComponent(), *this, ok);
    coord.getExpression().evaluate (finderScope);
    return ok;
}

bool RelativeCoordinatePositionerBase::addPoint (const RelativePoint& point)
{
    const bool ok = addCoordinate (point.x);
    return addCoordinate (point.y) && ok;
}

void RelativeCoordinatePositionerBase::registerComponentListener (Component& comp)
{
    if (! sourceComponents.contains (&comp))
    {
        comp.addComponentListener (this);
        sourceComponents.add (&comp);
    }
}

void RelativeCoordinatePositionerBase::registerMarkerListListener (MarkerList* const list)
{
    if (list != nullptr && ! sourceMarkerLists.contains (list))
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


END_JUCE_NAMESPACE

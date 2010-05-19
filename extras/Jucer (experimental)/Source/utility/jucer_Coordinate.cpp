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

#include "jucer_Coordinate.h"


//==============================================================================
ComponentAutoLayoutManager::ComponentAutoLayoutManager (Component* parentComponent)
    : parent (parentComponent)
{
    parent->addComponentListener (this);
}

ComponentAutoLayoutManager::~ComponentAutoLayoutManager()
{
    parent->removeComponentListener (this);

    for (int i = components.size(); --i >= 0;)
        components.getUnchecked(i)->component->removeComponentListener (this);
}

void ComponentAutoLayoutManager::setMarker (const String& name, const RelativeCoordinate& coord)
{
    for (int i = markers.size(); --i >= 0;)
    {
        MarkerPosition* m = markers.getUnchecked(i);
        if (m->markerName == name)
        {
            m->position = coord;
            applyLayout();
            return;
        }
    }

    markers.add (new MarkerPosition (name, coord));
    applyLayout();
}

void ComponentAutoLayoutManager::setComponentBounds (Component* comp, const String& name, const RelativeRectangle& coords)
{
    jassert (comp != 0);

    // All the components that this layout manages must be inside the parent component..
    jassert (parent->isParentOf (comp));

    for (int i = components.size(); --i >= 0;)
    {
        ComponentPosition* c = components.getUnchecked(i);
        if (c->component == comp)
        {
            c->name = name;
            c->coords = coords;
            triggerAsyncUpdate();
            return;
        }
    }

    components.add (new ComponentPosition (comp, name, coords));
    comp->addComponentListener (this);
    triggerAsyncUpdate();
}

void ComponentAutoLayoutManager::applyLayout()
{
    for (int i = components.size(); --i >= 0;)
    {
        ComponentPosition* c = components.getUnchecked(i);

        // All the components that this layout manages must be inside the parent component..
        jassert (parent->isParentOf (c->component));

        c->component->setBounds (c->coords.resolve (*this).getSmallestIntegerContainer());
    }
}

const RelativeCoordinate ComponentAutoLayoutManager::findNamedCoordinate (const String& objectName, const String& edge) const
{
    if (objectName == RelativeCoordinate::Strings::parent)
    {
        if (edge == RelativeCoordinate::Strings::right)     return RelativeCoordinate ((double) parent->getWidth(), true);
        if (edge == RelativeCoordinate::Strings::bottom)    return RelativeCoordinate ((double) parent->getHeight(), false);
    }

    if (objectName.isNotEmpty() && edge.isNotEmpty())
    {
        for (int i = components.size(); --i >= 0;)
        {
            ComponentPosition* c = components.getUnchecked(i);

            if (c->name == objectName)
            {
                if (edge == RelativeCoordinate::Strings::left)   return c->coords.left;
                if (edge == RelativeCoordinate::Strings::right)  return c->coords.right;
                if (edge == RelativeCoordinate::Strings::top)    return c->coords.top;
                if (edge == RelativeCoordinate::Strings::bottom) return c->coords.bottom;
            }
        }
    }

    for (int i = markers.size(); --i >= 0;)
    {
        MarkerPosition* m = markers.getUnchecked(i);

        if (m->markerName == objectName)
            return m->position;
    }

    return RelativeCoordinate();
}

void ComponentAutoLayoutManager::componentMovedOrResized (Component& component, bool wasMoved, bool wasResized)
{
    triggerAsyncUpdate();

    if (parent == &component)
        handleUpdateNowIfNeeded();
}

void ComponentAutoLayoutManager::componentBeingDeleted (Component& component)
{
    for (int i = components.size(); --i >= 0;)
    {
        ComponentPosition* c = components.getUnchecked(i);
        if (c->component == &component)
        {
            components.remove (i);
            break;
        }
    }
}

void ComponentAutoLayoutManager::handleAsyncUpdate()
{
    applyLayout();
}

ComponentAutoLayoutManager::MarkerPosition::MarkerPosition (const String& name, const RelativeCoordinate& coord)
    : markerName (name), position (coord)
{
}

ComponentAutoLayoutManager::ComponentPosition::ComponentPosition (Component* component_, const String& name_, const RelativeRectangle& coords_)
    : component (component_), name (name_), coords (coords_)
{
}

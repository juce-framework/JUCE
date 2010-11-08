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

#include "juce_ComponentMovementWatcher.h"


//==============================================================================
ComponentMovementWatcher::ComponentMovementWatcher (Component* const component_)
    : component (component_),
      lastPeer (0),
      reentrant (false)
{
    jassert (component != 0); // can't use this with a null pointer..

    component->addComponentListener (this);

    registerWithParentComps();
}

ComponentMovementWatcher::~ComponentMovementWatcher()
{
    component->removeComponentListener (this);

    unregister();
}

//==============================================================================
void ComponentMovementWatcher::componentParentHierarchyChanged (Component&)
{
    // agh! don't delete the target component without deleting this object first!
    jassert (component != 0);

    if (! reentrant)
    {
        reentrant = true;

        ComponentPeer* const peer = component->getPeer();

        if (peer != lastPeer)
        {
            componentPeerChanged();

            if (component == 0)
                return;

            lastPeer = peer;
        }

        unregister();
        registerWithParentComps();

        reentrant = false;

        componentMovedOrResized (*component, true, true);
    }
}

void ComponentMovementWatcher::componentMovedOrResized (Component&, bool wasMoved, bool wasResized)
{
    // agh! don't delete the target component without deleting this object first!
    jassert (component != 0);

    if (wasMoved)
    {
        const Point<int> pos (component->getTopLevelComponent()->getLocalPoint (component, Point<int>()));

        wasMoved = lastBounds.getPosition() != pos;
        lastBounds.setPosition (pos);
    }

    wasResized = (lastBounds.getWidth() != component->getWidth() || lastBounds.getHeight() != component->getHeight());
    lastBounds.setSize (component->getWidth(), component->getHeight());

    if (wasMoved || wasResized)
        componentMovedOrResized (wasMoved, wasResized);
}

void ComponentMovementWatcher::registerWithParentComps()
{
    Component* p = component->getParentComponent();

    while (p != 0)
    {
        p->addComponentListener (this);
        registeredParentComps.add (p);
        p = p->getParentComponent();
    }
}

void ComponentMovementWatcher::unregister()
{
    for (int i = registeredParentComps.size(); --i >= 0;)
        registeredParentComps.getUnchecked(i)->removeComponentListener (this);

    registeredParentComps.clear();
}


END_JUCE_NAMESPACE

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

#ifdef JUCE_DEBUG
    deletionWatcher = new ComponentDeletionWatcher (component_);
#endif

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
#ifdef JUCE_DEBUG
    // agh! don't delete the target component without deleting this object first!
    jassert (! deletionWatcher->hasBeenDeleted());
#endif

    if (! reentrant)
    {
        reentrant = true;

        ComponentPeer* const peer = component->getPeer();

        if (peer != lastPeer)
        {
            ComponentDeletionWatcher watcher (component);

            componentPeerChanged();

            if (watcher.hasBeenDeleted())
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
#ifdef JUCE_DEBUG
    // agh! don't delete the target component without deleting this object first!
    jassert (! deletionWatcher->hasBeenDeleted());
#endif

    if (wasMoved)
    {
        int x = 0, y = 0;
        component->relativePositionToOtherComponent (component->getTopLevelComponent(), x, y);

        wasMoved = (lastX != x || lastY != y);
        lastX = x;
        lastY = y;
    }

    wasResized = (lastWidth != component->getWidth() || lastHeight != component->getHeight());
    lastWidth = component->getWidth();
    lastHeight = component->getHeight();

    if (wasMoved || wasResized)
        componentMovedOrResized (wasMoved, wasResized);
}

void ComponentMovementWatcher::registerWithParentComps() throw()
{
    Component* p = component->getParentComponent();

    while (p != 0)
    {
        p->addComponentListener (this);
        registeredParentComps.add (p);
        p = p->getParentComponent();
    }
}

void ComponentMovementWatcher::unregister() throw()
{
    for (int i = registeredParentComps.size(); --i >= 0;)
        ((Component*) registeredParentComps.getUnchecked(i))->removeComponentListener (this);

    registeredParentComps.clear();
}


END_JUCE_NAMESPACE

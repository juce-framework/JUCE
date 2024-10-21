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

ComponentMovementWatcher::ComponentMovementWatcher (Component* const comp)
    : component (comp),
      wasShowing (comp->isShowing())
{
    jassert (component != nullptr); // can't use this with a null pointer..

    component->addComponentListener (this);
    registerWithParentComps();
}

ComponentMovementWatcher::~ComponentMovementWatcher()
{
    if (component != nullptr)
        component->removeComponentListener (this);

    unregister();
}

//==============================================================================
void ComponentMovementWatcher::componentParentHierarchyChanged (Component&)
{
    if (component != nullptr && ! reentrant)
    {
        const ScopedValueSetter<bool> setter (reentrant, true);

        auto* peer = component->getPeer();
        auto peerID = peer != nullptr ? peer->getUniqueID() : 0;

        if (peerID != lastPeerID)
        {
            componentPeerChanged();

            if (component == nullptr)
                return;

            lastPeerID = peerID;
        }

        unregister();
        registerWithParentComps();

        componentMovedOrResized (*component, true, true);

        if (component != nullptr)
            componentVisibilityChanged (*component);
    }
}

void ComponentMovementWatcher::componentMovedOrResized (Component&, bool wasMoved, bool wasResized)
{
    if (component != nullptr)
    {
        if (wasMoved)
        {
            Point<int> newPos;
            auto* top = component->getTopLevelComponent();

            if (top != component)
                newPos = top->getLocalPoint (component, Point<int>());
            else
                newPos = top->getPosition();

            wasMoved = lastBounds.getPosition() != newPos;
            lastBounds.setPosition (newPos);
        }

        wasResized = (lastBounds.getWidth() != component->getWidth() || lastBounds.getHeight() != component->getHeight());
        lastBounds.setSize (component->getWidth(), component->getHeight());

        if (wasMoved || wasResized)
            componentMovedOrResized (wasMoved, wasResized);
    }
}

void ComponentMovementWatcher::componentBeingDeleted (Component& comp)
{
    registeredParentComps.removeFirstMatchingValue (&comp);

    if (component == &comp)
        unregister();
}

void ComponentMovementWatcher::componentVisibilityChanged (Component&)
{
    if (component != nullptr)
    {
        const bool isShowingNow = component->isShowing();

        if (wasShowing != isShowingNow)
        {
            wasShowing = isShowingNow;
            componentVisibilityChanged();
        }
    }
}

void ComponentMovementWatcher::registerWithParentComps()
{
    for (auto* p = component->getParentComponent(); p != nullptr; p = p->getParentComponent())
    {
        p->addComponentListener (this);
        registeredParentComps.add (p);
    }
}

void ComponentMovementWatcher::unregister()
{
    for (auto* c : registeredParentComps)
        c->removeComponentListener (this);

    registeredParentComps.clear();
}

} // namespace juce

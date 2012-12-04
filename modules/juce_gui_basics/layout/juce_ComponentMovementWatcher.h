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

#ifndef __JUCE_COMPONENTMOVEMENTWATCHER_JUCEHEADER__
#define __JUCE_COMPONENTMOVEMENTWATCHER_JUCEHEADER__

#include "../components/juce_Component.h"


//==============================================================================
/** An object that watches for any movement of a component or any of its parent components.

    This makes it easy to check when a component is moved relative to its top-level
    peer window. The normal Component::moved() method is only called when a component
    moves relative to its immediate parent, and sometimes you want to know if any of
    components higher up the tree have moved (which of course will affect the overall
    position of all their sub-components).

    It also includes a callback that lets you know when the top-level peer is changed.

    This class is used by specialised components like WebBrowserComponent or QuickTimeComponent
    because they need to keep their custom windows in the right place and respond to
    changes in the peer.
*/
class JUCE_API  ComponentMovementWatcher    : public ComponentListener
{
public:
    //==============================================================================
    /** Creates a ComponentMovementWatcher to watch a given target component. */
    ComponentMovementWatcher (Component* component);

    /** Destructor. */
    ~ComponentMovementWatcher();

    //==============================================================================
    /** This callback happens when the component that is being watched is moved
        relative to its top-level peer window, or when it is resized. */
    virtual void componentMovedOrResized (bool wasMoved, bool wasResized) = 0;

    /** This callback happens when the component's top-level peer is changed. */
    virtual void componentPeerChanged() = 0;

    /** This callback happens when the component's visibility state changes, possibly due to
        one of its parents being made visible or invisible.
    */
    virtual void componentVisibilityChanged() = 0;

    /** Returns the component that's being watched. */
    Component* getComponent() const noexcept         { return component; }

    //==============================================================================
    /** @internal */
    void componentParentHierarchyChanged (Component& component);
    /** @internal */
    void componentMovedOrResized (Component& component, bool wasMoved, bool wasResized);
    /** @internal */
    void componentBeingDeleted (Component& component);
    /** @internal */
    void componentVisibilityChanged (Component& component);

private:
    //==============================================================================
    WeakReference<Component> component;
    uint32 lastPeerID;
    Array <Component*> registeredParentComps;
    bool reentrant, wasShowing;
    Rectangle<int> lastBounds;

    void unregister();
    void registerWithParentComps();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ComponentMovementWatcher)
};


#endif   // __JUCE_COMPONENTMOVEMENTWATCHER_JUCEHEADER__

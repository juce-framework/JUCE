/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

#ifndef JUCE_COMPONENTMOVEMENTWATCHER_H_INCLUDED
#define JUCE_COMPONENTMOVEMENTWATCHER_H_INCLUDED


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
    void componentParentHierarchyChanged (Component&) override;
    /** @internal */
    void componentMovedOrResized (Component&, bool wasMoved, bool wasResized) override;
    /** @internal */
    void componentBeingDeleted (Component&) override;
    /** @internal */
    void componentVisibilityChanged (Component&) override;

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


#endif   // JUCE_COMPONENTMOVEMENTWATCHER_H_INCLUDED

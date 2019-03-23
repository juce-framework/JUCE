/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

//==============================================================================
/** An object that watches for any movement of a component or any of its parent components.

    This makes it easy to check when a component is moved relative to its top-level
    peer window. The normal Component::moved() method is only called when a component
    moves relative to its immediate parent, and sometimes you want to know if any of
    components higher up the tree have moved (which of course will affect the overall
    position of all their sub-components).

    It also includes a callback that lets you know when the top-level peer is changed.

    This class is used by specialised components like WebBrowserComponent
    because they need to keep their custom windows in the right place and respond to
    changes in the peer.

    @tags{GUI}
*/
class JUCE_API  ComponentMovementWatcher    : public ComponentListener
{
public:
    //==============================================================================
    /** Creates a ComponentMovementWatcher to watch a given target component. */
    ComponentMovementWatcher (Component* componentToWatch);

    /** Destructor. */
    ~ComponentMovementWatcher() override;

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
    Component* getComponent() const noexcept         { return component.get(); }

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
    uint32 lastPeerID = 0;
    Array<Component*> registeredParentComps;
    bool reentrant = false, wasShowing;
    Rectangle<int> lastBounds;

    void unregister();
    void registerWithParentComps();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ComponentMovementWatcher)
};

} // namespace juce

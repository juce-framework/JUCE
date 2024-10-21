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

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
/**
    Gets informed about changes to a component's hierarchy or position.

    To monitor a component for changes, register a subclass of ComponentListener
    with the component using Component::addComponentListener().

    Be sure to deregister listeners before you delete them!

    @see Component::addComponentListener, Component::removeComponentListener

    @tags{GUI}
*/
class JUCE_API  ComponentListener
{
public:
    /** Destructor. */
    virtual ~ComponentListener() = default;

    /** Called when the component's position or size changes.

        @param component    the component that was moved or resized
        @param wasMoved     true if the component's top-left corner has just moved
        @param wasResized   true if the component's width or height has just changed
        @see Component::setBounds, Component::resized, Component::moved
    */
    virtual void componentMovedOrResized (Component& component,
                                          bool wasMoved,
                                          bool wasResized);

    /** Called when the component is brought to the top of the z-order.

        @param component    the component that was moved
        @see Component::toFront, Component::broughtToFront
    */
    virtual void componentBroughtToFront (Component& component);

    /** Called when the component is made visible or invisible.

        @param component    the component that changed
        @see Component::setVisible
    */
    virtual void componentVisibilityChanged (Component& component);

    /** Called when the component has children added or removed, or their z-order
        changes.

        @param component    the component whose children have changed
        @see Component::childrenChanged, Component::addChildComponent,
             Component::removeChildComponent
    */
    virtual void componentChildrenChanged (Component& component);

    /** Called to indicate that the component's parents have changed.

        When a component is added or removed from its parent, all of its children
        will produce this notification (recursively - so all children of its
        children will also be called as well).

        @param component    the component that this listener is registered with
        @see Component::parentHierarchyChanged
    */
    virtual void componentParentHierarchyChanged (Component& component);

    /** Called when the component's name is changed.

        @param component    the component that had its name changed
        @see Component::setName, Component::getName
    */
    virtual void componentNameChanged (Component& component);

    /** Called when the component is in the process of being deleted.

        This callback is made from inside the destructor, so be very, very cautious
        about what you do in here.

        In particular, bear in mind that it's the Component base class's destructor that calls
        this - so if the object that's being deleted is a subclass of Component, then the
        subclass layers of the object will already have been destructed when it gets to this
        point!

        @param component    the component that was deleted
    */
    virtual void componentBeingDeleted (Component& component);

    /* Called when the component's enablement is changed.

       @param component    the component that had its enablement changed
       @see Component::setEnabled, Component::isEnabled, Component::enablementChanged
    */
    virtual void componentEnablementChanged (Component& component);
};

} // namespace juce

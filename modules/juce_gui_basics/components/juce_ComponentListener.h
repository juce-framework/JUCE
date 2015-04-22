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

#ifndef JUCE_COMPONENTLISTENER_H_INCLUDED
#define JUCE_COMPONENTLISTENER_H_INCLUDED


//==============================================================================
/**
    Gets informed about changes to a component's hierarchy or position.

    To monitor a component for changes, register a subclass of ComponentListener
    with the component using Component::addComponentListener().

    Be sure to deregister listeners before you delete them!

    @see Component::addComponentListener, Component::removeComponentListener
*/
class JUCE_API  ComponentListener
{
public:
    /** Destructor. */
    virtual ~ComponentListener()  {}

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
    */
    virtual void componentBeingDeleted (Component& component);
};


#endif   // JUCE_COMPONENTLISTENER_H_INCLUDED

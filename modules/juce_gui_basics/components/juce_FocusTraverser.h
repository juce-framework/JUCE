/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

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
/**
    Controls the order in which focus moves between components.

    The algorithm used by this class to work out the order of traversal is as
    follows:
    - Only visible and enabled components are considered focusable.
    - If two components both have an explicit focus order specified then the
      one with the lowest number comes first (see the
      Component::setExplicitFocusOrder() method).
    - Any component with an explicit focus order greater than 0 comes before ones
      that don't have an order specified.
    - Components with their 'always on top' flag set come before those without.
    - Any unspecified components are traversed in a left-to-right, then
      top-to-bottom order.

    If you need focus traversal in a more customised way you can create a
    ComponentTraverser subclass that uses your own algorithm and return it
    from Component::createFocusTraverser().

    @see ComponentTraverser, Component::createFocusTraverser

    @tags{GUI}
*/
class JUCE_API  FocusTraverser  : public ComponentTraverser
{
public:
    /** Destructor. */
    ~FocusTraverser() override = default;

    /** Returns the component that should receive focus by default within the given
        parent component.

        The default implementation will just return the foremost visible and enabled
        child component, and will return nullptr if there is no suitable component.
    */
    Component* getDefaultComponent (Component* parentComponent) override;

    /** Returns the component that should be given focus after the specified one when
        moving "forwards".

        The default implementation will return the next visible and enabled component
        which is to the right of or below this one, and will return nullptr if there
        is no suitable component.
    */
    Component* getNextComponent (Component* current) override;

    /** Returns the component that should be given focus after the specified one when
        moving "backwards".

        The default implementation will return the previous visible and enabled component
        which is to the left of or above this one, and will return nullptr if there
        is no suitable component.
    */
    Component* getPreviousComponent (Component* current) override;

    /** Returns all of the components that can receive focus within the given parent
        component in traversal order.

        The default implementation will return all visible and enabled child components.
    */
    std::vector<Component*> getAllComponents (Component* parentComponent) override;
};

} // namespace juce

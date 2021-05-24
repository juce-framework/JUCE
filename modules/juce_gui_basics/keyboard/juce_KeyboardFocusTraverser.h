/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
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
    Controls the order in which keyboard focus moves between components.

    The default behaviour of this class uses a FocusTraverser object internally to
    determine the default/next/previous component until it finds one which wants
    keyboard focus, as set by the Component::setWantsKeyboardFocus() method.

    If you need keyboard focus traversal in a more customised way, you can create
    a subclass of ComponentTraverser that uses your own algorithm, and use
    Component::createKeyboardFocusTraverser() to create it.

    @see FocusTraverser, ComponentTraverser, Component::createKeyboardFocusTraverser

    @tags{GUI}
*/
class JUCE_API  KeyboardFocusTraverser  : public ComponentTraverser
{
public:
    /** Destructor. */
    ~KeyboardFocusTraverser() override = default;

    /** Returns the component that should receive keyboard focus by default within the
        given parent component.

        The default implementation will return the foremost focusable component (as
        determined by FocusTraverser) that also wants keyboard focus, or nullptr if
        there is no suitable component.
    */
    Component* getDefaultComponent (Component* parentComponent) override;

    /** Returns the component that should be given keyboard focus after the specified
        one when moving "forwards".

        The default implementation will return the next focusable component (as
        determined by FocusTraverser) that also wants keyboard focus, or nullptr if
        there is no suitable component.
    */
    Component* getNextComponent (Component* current) override;

    /** Returns the component that should be given keyboard focus after the specified
        one when moving "backwards".

        The default implementation will return the previous focusable component (as
        determined by FocusTraverser) that also wants keyboard focus, or nullptr if
        there is no suitable component.
    */
    Component* getPreviousComponent (Component* current) override;

    /** Returns all of the components that can receive keyboard focus within the given
        parent component in traversal order.

        The default implementation will return all focusable child components (as
        determined by FocusTraverser) that also wants keyboard focus.
    */
    std::vector<Component*> getAllComponents (Component* parentComponent) override;
};

} // namespace juce

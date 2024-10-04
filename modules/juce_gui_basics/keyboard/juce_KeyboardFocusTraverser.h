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

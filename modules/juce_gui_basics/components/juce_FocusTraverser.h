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

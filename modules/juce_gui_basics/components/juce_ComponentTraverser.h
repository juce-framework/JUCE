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
    Base class for traversing components.

    If you need custom focus or keyboard focus traversal for a component you can
    create a subclass of ComponentTraverser and return it from
    Component::createFocusTraverser() or Component::createKeyboardFocusTraverser().

    @see Component::createFocusTraverser, Component::createKeyboardFocusTraverser

    @tags{GUI}
*/
class JUCE_API  ComponentTraverser
{
public:
    /** Destructor. */
    virtual ~ComponentTraverser() = default;

    /** Returns the component that should be used as the traversal entry point
        within the given parent component.

        This must return nullptr if there is no default component.
    */
    virtual Component* getDefaultComponent (Component* parentComponent) = 0;

    /** Returns the component that comes after the specified one when moving "forwards".

        This must return nullptr if there is no next component.
    */
    virtual Component* getNextComponent (Component* current) = 0;

    /** Returns the component that comes after the specified one when moving "backwards".

        This must return nullptr if there is no previous component.
    */
    virtual Component* getPreviousComponent (Component* current) = 0;

    /** Returns all of the traversable components within the given parent component in
        traversal order.
    */
    virtual std::vector<Component*> getAllComponents (Component* parentComponent) = 0;
};

} // namespace juce

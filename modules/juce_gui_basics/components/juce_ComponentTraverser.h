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

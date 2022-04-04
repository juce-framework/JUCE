/*
  ==============================================================================

   This file is part of the JUCE 7 technical preview.
   Copyright (c) 2022 - Raw Material Software Limited

   You may use this code under the terms of the GPL v3
   (see www.gnu.org/licenses).

   For the technical preview this file cannot be licensed commercially.

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

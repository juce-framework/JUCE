/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

#ifndef __JUCE_KEYBOARDFOCUSTRAVERSER_JUCEHEADER__
#define __JUCE_KEYBOARDFOCUSTRAVERSER_JUCEHEADER__

class Component;


//==============================================================================
/**
    Controls the order in which focus moves between components.

    The default algorithm used by this class to work out the order of traversal
    is as follows:
    - if two components both have an explicit focus order specified, then the
      one with the lowest number comes first (see the Component::setExplicitFocusOrder()
      method).
    - any component with an explicit focus order greater than 0 comes before ones
      that don't have an order specified.
    - any unspecified components are traversed in a left-to-right, then top-to-bottom
      order.

    If you need traversal in a more customised way, you can create a subclass
    of KeyboardFocusTraverser that uses your own algorithm, and use
    Component::createFocusTraverser() to create it.

    @see Component::setExplicitFocusOrder, Component::createFocusTraverser
*/
class JUCE_API  KeyboardFocusTraverser
{
public:
    KeyboardFocusTraverser();

    /** Destructor. */
    virtual ~KeyboardFocusTraverser();

    /** Returns the component that should be given focus after the specified one
        when moving "forwards".

        The default implementation will return the next component which is to the
        right of or below this one.

        This may return 0 if there's no suitable candidate.
    */
    virtual Component* getNextComponent (Component* current);

    /** Returns the component that should be given focus after the specified one
        when moving "backwards".

        The default implementation will return the next component which is to the
        left of or above this one.

        This may return 0 if there's no suitable candidate.
    */
    virtual Component* getPreviousComponent (Component* current);

    /** Returns the component that should receive focus be default within the given
        parent component.

        The default implementation will just return the foremost child component that
        wants focus.

        This may return 0 if there's no suitable candidate.
    */
    virtual Component* getDefaultComponent (Component* parentComponent);
};


#endif   // __JUCE_KEYBOARDFOCUSTRAVERSER_JUCEHEADER__

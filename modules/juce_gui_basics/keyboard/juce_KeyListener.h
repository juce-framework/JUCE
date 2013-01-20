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

#ifndef __JUCE_KEYLISTENER_JUCEHEADER__
#define __JUCE_KEYLISTENER_JUCEHEADER__

#include "juce_KeyPress.h"
class Component;


//==============================================================================
/**
    Receives callbacks when keys are pressed.

    You can add a key listener to a component to be informed when that component
    gets key events. See the Component::addListener method for more details.

    @see KeyPress, Component::addKeyListener, KeyPressMappingSet
*/
class JUCE_API  KeyListener
{
public:
    /** Destructor. */
    virtual ~KeyListener()  {}

    //==============================================================================
    /** Called to indicate that a key has been pressed.

        If your implementation returns true, then the key event is considered to have
        been consumed, and will not be passed on to any other components. If it returns
        false, then the key will be passed to other components that might want to use it.

        @param key                      the keystroke, including modifier keys
        @param originatingComponent     the component that received the key event
        @see keyStateChanged, Component::keyPressed
    */
    virtual bool keyPressed (const KeyPress& key,
                             Component* originatingComponent) = 0;

    /** Called when any key is pressed or released.

        When this is called, classes that might be interested in
        the state of one or more keys can use KeyPress::isKeyCurrentlyDown() to
        check whether their key has changed.

        If your implementation returns true, then the key event is considered to have
        been consumed, and will not be passed on to any other components. If it returns
        false, then the key will be passed to other components that might want to use it.

        @param originatingComponent     the component that received the key event
        @param isKeyDown                true if a key is being pressed, false if one is being released
        @see KeyPress, Component::keyStateChanged
    */
    virtual bool keyStateChanged (bool isKeyDown, Component* originatingComponent);
};


#endif   // __JUCE_KEYLISTENER_JUCEHEADER__

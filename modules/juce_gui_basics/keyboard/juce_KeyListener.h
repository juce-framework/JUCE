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
    Receives callbacks when keys are pressed.

    You can add a key listener to a component to be informed when that component
    gets key events. See the Component::addListener method for more details.

    @see KeyPress, Component::addKeyListener, KeyPressMappingSet

    @tags{GUI}
*/
class JUCE_API  KeyListener
{
public:
    /** Destructor. */
    virtual ~KeyListener() = default;

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

} // namespace juce

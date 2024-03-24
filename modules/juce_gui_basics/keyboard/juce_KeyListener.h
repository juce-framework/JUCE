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
    Receives callbacks when keys are pressed.

    You can add a key listener to a component to be informed when that component
    gets key events. See the Component::addKeyListener method for more details.

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

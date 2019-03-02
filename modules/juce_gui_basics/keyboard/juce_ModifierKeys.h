/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

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
    Represents the state of the mouse buttons and modifier keys.

    This is used both by mouse events and by KeyPress objects to describe
    the state of keys such as shift, control, alt, etc.

    @see KeyPress, MouseEvent::mods

    @tags{GUI}
*/
class JUCE_API  ModifierKeys
{
public:
    //==============================================================================
    /** Creates a ModifierKeys object with no flags set. */
    ModifierKeys() = default;

    /** Creates a ModifierKeys object from a raw set of flags.

        @param flags to represent the keys that are down
        @see    shiftModifier, ctrlModifier, altModifier, leftButtonModifier,
                rightButtonModifier, commandModifier, popupMenuClickModifier
    */
    ModifierKeys (int flags) noexcept;

    /** Creates a copy of another object. */
    ModifierKeys (const ModifierKeys&) = default;

    /** Copies this object from another one. */
    ModifierKeys& operator= (const ModifierKeys&) = default;

    //==============================================================================
    /** Checks whether the 'command' key flag is set (or 'ctrl' on Windows/Linux).

        This is a platform-agnostic way of checking for the operating system's
        preferred command-key modifier - so on the Mac it tests for the Apple key, on
        Windows/Linux, it's actually checking for the CTRL key.
    */
    inline bool isCommandDown() const noexcept          { return testFlags (commandModifier); }

    /** Checks whether the user is trying to launch a pop-up menu.

        This checks for platform-specific modifiers that might indicate that the user
        is following the operating system's normal method of showing a pop-up menu.

        So on Windows/Linux, this method is really testing for a right-click.
        On the Mac, it tests for either the CTRL key being down, or a right-click.
    */
    inline bool isPopupMenu() const noexcept            { return testFlags (popupMenuClickModifier); }

    /** Checks whether the flag is set for the left mouse-button. */
    inline bool isLeftButtonDown() const noexcept       { return testFlags (leftButtonModifier); }

    /** Checks whether the flag is set for the right mouse-button.

        Note that for detecting popup-menu clicks, you should be using isPopupMenu() instead, as
        this is platform-independent (and makes your code more explanatory too).
    */
    inline bool isRightButtonDown() const noexcept      { return testFlags (rightButtonModifier); }

    inline bool isMiddleButtonDown() const noexcept     { return testFlags (middleButtonModifier); }

    /** Tests for any of the mouse-button flags. */
    inline bool isAnyMouseButtonDown() const noexcept   { return testFlags (allMouseButtonModifiers); }

    /** Tests for any of the modifier key flags. */
    inline bool isAnyModifierKeyDown() const noexcept   { return testFlags ((shiftModifier | ctrlModifier | altModifier | commandModifier)); }

    /** Checks whether the shift key's flag is set. */
    inline bool isShiftDown() const noexcept            { return testFlags (shiftModifier); }

    /** Checks whether the CTRL key's flag is set.

        Remember that it's better to use the platform-agnostic routines to test for command-key and
        popup-menu modifiers.

        @see isCommandDown, isPopupMenu
    */
    inline bool isCtrlDown() const noexcept             { return testFlags (ctrlModifier); }

    /** Checks whether the ALT key's flag is set. */
    inline bool isAltDown() const noexcept              { return testFlags (altModifier); }

    //==============================================================================
    /** Flags that represent the different keys. */
    enum Flags
    {
        /** Indicates no modifier keys. */
        noModifiers                             = 0,

        /** Shift key flag. */
        shiftModifier                           = 1,

        /** CTRL key flag. */
        ctrlModifier                            = 2,

        /** ALT key flag. */
        altModifier                             = 4,

        /** Left mouse button flag. */
        leftButtonModifier                      = 16,

        /** Right mouse button flag. */
        rightButtonModifier                     = 32,

        /** Middle mouse button flag. */
        middleButtonModifier                    = 64,

       #if JUCE_MAC
        /** Command key flag - on windows this is the same as the CTRL key flag. */
        commandModifier                         = 8,

        /** Popup menu flag - on windows this is the same as rightButtonModifier, on the
            Mac it's the same as (rightButtonModifier | ctrlModifier). */
        popupMenuClickModifier                  = rightButtonModifier | ctrlModifier,
       #else
        /** Command key flag - on windows this is the same as the CTRL key flag. */
        commandModifier                         = ctrlModifier,

        /** Popup menu flag - on windows this is the same as rightButtonModifier, on the
            Mac it's the same as (rightButtonModifier | ctrlModifier). */
        popupMenuClickModifier                  = rightButtonModifier,
       #endif

        /** Represents a combination of all the shift, alt, ctrl and command key modifiers. */
        allKeyboardModifiers                    = shiftModifier | ctrlModifier | altModifier | commandModifier,

        /** Represents a combination of all the mouse buttons at once. */
        allMouseButtonModifiers                 = leftButtonModifier | rightButtonModifier | middleButtonModifier,

        /** Represents a combination of all the alt, ctrl and command key modifiers. */
        ctrlAltCommandModifiers                 = ctrlModifier | altModifier | commandModifier
    };

    //==============================================================================
    /** Returns a copy of only the mouse-button flags */
    ModifierKeys withOnlyMouseButtons() const noexcept                  { return ModifierKeys (flags & allMouseButtonModifiers); }

    /** Returns a copy of only the non-mouse flags */
    ModifierKeys withoutMouseButtons() const noexcept                   { return ModifierKeys (flags & ~allMouseButtonModifiers); }

    bool operator== (const ModifierKeys other) const noexcept           { return flags == other.flags; }
    bool operator!= (const ModifierKeys other) const noexcept           { return flags != other.flags; }

    //==============================================================================
    /** Returns the raw flags for direct testing. */
    inline int getRawFlags() const noexcept                             { return flags; }

    ModifierKeys withoutFlags (int rawFlagsToClear) const noexcept      { return ModifierKeys (flags & ~rawFlagsToClear); }
    ModifierKeys withFlags (int rawFlagsToSet) const noexcept           { return ModifierKeys (flags | rawFlagsToSet); }

    /** Tests a combination of flags and returns true if any of them are set. */
    bool testFlags (int flagsToTest) const noexcept                     { return (flags & flagsToTest) != 0; }

    /** Returns the total number of mouse buttons that are down. */
    int getNumMouseButtonsDown() const noexcept;

    //==============================================================================
    /** This object represents the last-known state of the keyboard and mouse buttons. */
    static ModifierKeys currentModifiers;

    /** Creates a ModifierKeys object to represent the last-known state of the
        keyboard and mouse buttons.

        This method is here for backwards compatibility and there's no need to call it anymore,
        you should use the public currentModifiers member directly.
     */
    static ModifierKeys getCurrentModifiers() noexcept                  { return currentModifiers; }

    /** Creates a ModifierKeys object to represent the current state of the
        keyboard and mouse buttons.

        This method is here for backwards compatibility and you should call ComponentPeer::getCurrentModifiersRealtime()
        instead (which is what this method now does).
    */
    static ModifierKeys getCurrentModifiersRealtime() noexcept;

private:
    int flags = 0;
};

} // namespace juce

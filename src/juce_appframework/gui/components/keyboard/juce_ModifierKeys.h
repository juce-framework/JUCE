/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330, 
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

  ==============================================================================
*/

#ifndef __JUCE_MODIFIERKEYS_JUCEHEADER__
#define __JUCE_MODIFIERKEYS_JUCEHEADER__


//==============================================================================
/**
    Represents the state of the mouse buttons and modifier keys.

    This is used both by mouse events and by KeyPress objects to describe
    the state of keys such as shift, control, alt, etc.

    @see KeyPress, MouseEvent::mods
*/
class JUCE_API  ModifierKeys
{
public:
    //==============================================================================
    /** Creates a ModifierKeys object from a raw set of flags.

        @param flags to represent the keys that are down
        @see    shiftModifier, ctrlModifier, altModifier, leftButtonModifier,
                rightButtonModifier, commandModifier, popupMenuClickModifier
    */
    ModifierKeys (const int flags = 0) throw();

    /** Creates a copy of another object. */
    ModifierKeys (const ModifierKeys& other) throw();

    /** Copies this object from another one. */
    const ModifierKeys& operator= (const ModifierKeys& other) throw();

    //==============================================================================
    /** Checks whether the 'command' key flag is set (or 'ctrl' on Windows/Linux).

        This is a platform-agnostic way of checking for the operating system's
        preferred command-key modifier - so on the Mac it tests for the Apple key, on
        Windows/Linux, it's actually checking for the CTRL key.
    */
    inline bool isCommandDown() const throw()           { return (flags & commandModifier) != 0; }

    /** Checks whether the user is trying to launch a pop-up menu.

        This checks for platform-specific modifiers that might indicate that the user
        is following the operating system's normal method of showing a pop-up menu.

        So on Windows/Linux, this method is really testing for a right-click.
        On the Mac, it tests for either the CTRL key being down, or a right-click.
    */
    inline bool isPopupMenu() const throw()             { return (flags & popupMenuClickModifier) != 0; }

    /** Checks whether the flag is set for the left mouse-button. */
    inline bool isLeftButtonDown() const throw()        { return (flags & leftButtonModifier) != 0; }

    /** Checks whether the flag is set for the right mouse-button.

        Note that for detecting popup-menu clicks, you should be using isPopupMenu() instead, as
        this is platform-independent (and makes your code more explanatory too).
    */
    inline bool isRightButtonDown() const throw()       { return (flags & rightButtonModifier) != 0; }

    inline bool isMiddleButtonDown() const throw()      { return (flags & middleButtonModifier) != 0; }

    /** Tests for any of the mouse-button flags. */
    inline bool isAnyMouseButtonDown() const throw()    { return (flags & allMouseButtonModifiers) != 0; }

    /** Tests for any of the modifier key flags. */
    inline bool isAnyModifierKeyDown() const throw()    { return (flags & (shiftModifier | ctrlModifier | altModifier | commandModifier)) != 0; }

    /** Checks whether the shift key's flag is set. */
    inline bool isShiftDown() const throw()             { return (flags & shiftModifier) != 0; }

    /** Checks whether the CTRL key's flag is set.

        Remember that it's better to use the platform-agnostic routines to test for command-key and
        popup-menu modifiers.

        @see isCommandDown, isPopupMenu
    */
    inline bool isCtrlDown() const throw()              { return (flags & ctrlModifier) != 0; }

    /** Checks whether the shift key's flag is set. */
    inline bool isAltDown() const throw()               { return (flags & altModifier) != 0; }

    //==============================================================================
    // modifier bitmasks

    /** Shift key flag. */
    static const int shiftModifier;
    /** CTRL key flag. */
    static const int ctrlModifier;
    /** ALT key flag. */
    static const int altModifier;
    /** Left mouse button flag. */
    static const int leftButtonModifier;
    /** Right mouse button flag. */
    static const int rightButtonModifier;
    /** Middle mouse button flag. */
    static const int middleButtonModifier;
    /** Command key flag - on windows this is the same as the CTRL key flag. */
    static const int commandModifier;

    /** Popup menu flag - on windows this is the same as rightButtonModifier, on the
        Mac it's the same as (rightButtonModifier | ctrlModifier). */
    static const int popupMenuClickModifier;

    /** Represents a combination of all the shift, alt, ctrl and command key modifiers. */
    static const int allKeyboardModifiers;

    /** Represents a combination of all the mouse buttons at once. */
    static const int allMouseButtonModifiers;

    //==============================================================================
    /** Returns the raw flags for direct testing. */
    inline int getRawFlags() const throw()                          { return flags; }

    /** Tests a combination of flags and returns true if any of them are set. */
    inline bool testFlags (const int flagsToTest) const throw()     { return (flags & flagsToTest) != 0; }

    //==============================================================================
    /** Creates a ModifierKeys object to represent the last-known state of the
        keyboard and mouse buttons.

        @see getCurrentModifiersRealtime
    */
    static const ModifierKeys getCurrentModifiers() throw();

    /** Creates a ModifierKeys object to represent the current state of the
        keyboard and mouse buttons.

        This isn't often needed and isn't recommended, but will actively check all the
        mouse and key states rather than just returning their last-known state like
        getCurrentModifiers() does.

        This is only needed in special circumstances for up-to-date modifier information
        at times when the app's event loop isn't running normally.
    */
    static const ModifierKeys getCurrentModifiersRealtime();


private:
    //==============================================================================
    int flags;

    static int currentModifierFlags;

    friend class ComponentPeer;
    static void updateCurrentModifiers();
};


#endif   // __JUCE_MODIFIERKEYS_JUCEHEADER__

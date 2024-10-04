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
    Represents a key press, including any modifier keys that are needed.

    E.g. a KeyPress might represent CTRL+C, SHIFT+ALT+H, Spacebar, Escape, etc.

    @see Component, KeyListener, KeyPressMappingSet, Button::addShortcut

    @tags{GUI}
*/
class JUCE_API  KeyPress
{
public:
    //==============================================================================
    /** Creates an (invalid) KeyPress.

        @see isValid
    */
    KeyPress() = default;

    /** Destructor. */
    ~KeyPress() = default;

    /** Creates a KeyPress for a key and some modifiers.

        e.g.
        CTRL+C would be: KeyPress ('c', ModifierKeys::ctrlModifier, 0)
        SHIFT+Escape would be: KeyPress (KeyPress::escapeKey, ModifierKeys::shiftModifier, 0)

        @param keyCode      a code that represents the key - this value must be
                            one of special constants listed in this class, or an
                            8-bit character code such as a letter (case is ignored),
                            digit or a simple key like "," or ".". Note that this
                            isn't the same as the textCharacter parameter, so for example
                            a keyCode of 'a' and a shift-key modifier should have a
                            textCharacter value of 'A'.
        @param modifiers    the modifiers to associate with the keystroke
        @param textCharacter    the character that would be printed if someone typed
                            this keypress into a text editor. This value may be
                            null if the keypress is a non-printing character
        @see getKeyCode, isKeyCode, getModifiers
    */
    KeyPress (int keyCode,
              ModifierKeys modifiers,
              juce_wchar textCharacter) noexcept;

    /** Creates a keypress with a keyCode but no modifiers or text character. */
    explicit KeyPress (int keyCode) noexcept;

    /** Creates a copy of another KeyPress. */
    KeyPress (const KeyPress&) = default;

    /** Copies this KeyPress from another one. */
    KeyPress& operator= (const KeyPress&) = default;

    /** Compares two KeyPress objects. */
    bool operator== (const KeyPress& other) const noexcept;

    /** Compares two KeyPress objects. */
    bool operator!= (const KeyPress& other) const noexcept;

    /** Returns true if this keypress is for the given keycode without any modifiers. */
    bool operator== (int keyCode) const noexcept;

    /** Returns true if this keypress is not for the given keycode without any modifiers. */
    bool operator!= (int keyCode) const noexcept;

    //==============================================================================
    /** Returns true if this is a valid KeyPress.

        A null keypress can be created by the default constructor, in case it's
        needed.
    */
    bool isValid() const noexcept                               { return keyCode != 0; }

    /** Returns the key code itself.

        This will either be one of the special constants defined in this class,
        or an 8-bit character code.
    */
    int getKeyCode() const noexcept                             { return keyCode; }

    /** Returns the key modifiers.

        @see ModifierKeys
    */
    ModifierKeys getModifiers() const noexcept                  { return mods; }

    /** Returns the character that is associated with this keypress.

        This is the character that you'd expect to see printed if you press this
        keypress in a text editor or similar component.
    */
    juce_wchar getTextCharacter() const noexcept                { return textCharacter; }

    /** Checks whether the KeyPress's key is the same as the one provided, without checking
        the modifiers.

        The values for key codes can either be one of the special constants defined in
        this class, or an 8-bit character code.

        @see getKeyCode
    */
    bool isKeyCode (int keyCodeToCompare) const noexcept        { return keyCode == keyCodeToCompare; }

    //==============================================================================
    /** Converts a textual key description to a KeyPress.

        This attempts to decode a textual version of a keypress, e.g. "ctrl + c" or "spacebar".

        This isn't designed to cope with any kind of input, but should be given the
        strings that are created by the getTextDescription() method.

        If the string can't be parsed, the object returned will be invalid.

        @see getTextDescription
    */
    static KeyPress createFromDescription (const String& textVersion);

    /** Creates a textual description of the key combination.

        e.g. "ctrl + c" or "delete".

        To store a keypress in a file, use this method, along with createFromDescription()
        to retrieve it later.
    */
    String getTextDescription() const;

    /** Creates a textual description of the key combination, using unicode icon symbols if possible.

        On OSX, this uses the Apple symbols for command, option, shift, etc, instead of the textual
        modifier key descriptions that are returned by getTextDescription()
    */
    String getTextDescriptionWithIcons() const;

    //==============================================================================
    /** Checks whether the user is currently holding down the keys that make up this
        KeyPress.

        Note that this will return false if any extra modifier keys are
        down - e.g. if the keypress is CTRL+X and the user is actually holding CTRL+ALT+x
        then it will be false.
    */
    bool isCurrentlyDown() const;

    /** Checks whether a particular key is held down, irrespective of modifiers.

        The values for key codes can either be one of the special constants defined in
        this class, or an 8-bit character code.
    */
    static bool isKeyCurrentlyDown (int keyCode);

    //==============================================================================
    // Key codes
    //
    // Note that the actual values of these are platform-specific and may change
    // without warning, so don't store them anywhere as constants. For persisting/retrieving
    // KeyPress objects, use getTextDescription() and createFromDescription() instead.
    //

    static const int spaceKey;      /**< key-code for the space bar */
    static const int escapeKey;     /**< key-code for the escape key */
    static const int returnKey;     /**< key-code for the return key*/
    static const int tabKey;        /**< key-code for the tab key*/

    static const int deleteKey;     /**< key-code for the delete key (not backspace) */
    static const int backspaceKey;  /**< key-code for the backspace key */
    static const int insertKey;     /**< key-code for the insert key */

    static const int upKey;         /**< key-code for the cursor-up key */
    static const int downKey;       /**< key-code for the cursor-down key */
    static const int leftKey;       /**< key-code for the cursor-left key */
    static const int rightKey;      /**< key-code for the cursor-right key */
    static const int pageUpKey;     /**< key-code for the page-up key */
    static const int pageDownKey;   /**< key-code for the page-down key */
    static const int homeKey;       /**< key-code for the home key */
    static const int endKey;        /**< key-code for the end key */

    static const int F1Key;         /**< key-code for the F1 key */
    static const int F2Key;         /**< key-code for the F2 key */
    static const int F3Key;         /**< key-code for the F3 key */
    static const int F4Key;         /**< key-code for the F4 key */
    static const int F5Key;         /**< key-code for the F5 key */
    static const int F6Key;         /**< key-code for the F6 key */
    static const int F7Key;         /**< key-code for the F7 key */
    static const int F8Key;         /**< key-code for the F8 key */
    static const int F9Key;         /**< key-code for the F9 key */
    static const int F10Key;        /**< key-code for the F10 key */
    static const int F11Key;        /**< key-code for the F11 key */
    static const int F12Key;        /**< key-code for the F12 key */
    static const int F13Key;        /**< key-code for the F13 key */
    static const int F14Key;        /**< key-code for the F14 key */
    static const int F15Key;        /**< key-code for the F15 key */
    static const int F16Key;        /**< key-code for the F16 key */
    static const int F17Key;        /**< key-code for the F17 key */
    static const int F18Key;        /**< key-code for the F18 key */
    static const int F19Key;        /**< key-code for the F19 key */
    static const int F20Key;        /**< key-code for the F20 key */
    static const int F21Key;        /**< key-code for the F21 key */
    static const int F22Key;        /**< key-code for the F22 key */
    static const int F23Key;        /**< key-code for the F23 key */
    static const int F24Key;        /**< key-code for the F24 key */
    static const int F25Key;        /**< key-code for the F25 key */
    static const int F26Key;        /**< key-code for the F26 key */
    static const int F27Key;        /**< key-code for the F27 key */
    static const int F28Key;        /**< key-code for the F28 key */
    static const int F29Key;        /**< key-code for the F29 key */
    static const int F30Key;        /**< key-code for the F30 key */
    static const int F31Key;        /**< key-code for the F31 key */
    static const int F32Key;        /**< key-code for the F32 key */
    static const int F33Key;        /**< key-code for the F33 key */
    static const int F34Key;        /**< key-code for the F34 key */
    static const int F35Key;        /**< key-code for the F35 key */

    static const int numberPad0;     /**< key-code for the 0 on the numeric keypad. */
    static const int numberPad1;     /**< key-code for the 1 on the numeric keypad. */
    static const int numberPad2;     /**< key-code for the 2 on the numeric keypad. */
    static const int numberPad3;     /**< key-code for the 3 on the numeric keypad. */
    static const int numberPad4;     /**< key-code for the 4 on the numeric keypad. */
    static const int numberPad5;     /**< key-code for the 5 on the numeric keypad. */
    static const int numberPad6;     /**< key-code for the 6 on the numeric keypad. */
    static const int numberPad7;     /**< key-code for the 7 on the numeric keypad. */
    static const int numberPad8;     /**< key-code for the 8 on the numeric keypad. */
    static const int numberPad9;     /**< key-code for the 9 on the numeric keypad. */

    static const int numberPadAdd;            /**< key-code for the add sign on the numeric keypad. */
    static const int numberPadSubtract;       /**< key-code for the subtract sign on the numeric keypad. */
    static const int numberPadMultiply;       /**< key-code for the multiply sign on the numeric keypad. */
    static const int numberPadDivide;         /**< key-code for the divide sign on the numeric keypad. */
    static const int numberPadSeparator;      /**< key-code for the comma on the numeric keypad. */
    static const int numberPadDecimalPoint;   /**< key-code for the decimal point sign on the numeric keypad. */
    static const int numberPadEquals;         /**< key-code for the equals key on the numeric keypad. */
    static const int numberPadDelete;         /**< key-code for the delete key on the numeric keypad. */

    static const int playKey;        /**< key-code for a multimedia 'play' key, (not all keyboards will have one) */
    static const int stopKey;        /**< key-code for a multimedia 'stop' key, (not all keyboards will have one) */
    static const int fastForwardKey; /**< key-code for a multimedia 'fast-forward' key, (not all keyboards will have one) */
    static const int rewindKey;      /**< key-code for a multimedia 'rewind' key, (not all keyboards will have one) */

private:
    //==============================================================================
    int keyCode = 0;
    ModifierKeys mods;
    juce_wchar textCharacter = 0;

    JUCE_LEAK_DETECTOR (KeyPress)
};

} // namespace juce

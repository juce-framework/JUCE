/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

KeyPress::KeyPress() noexcept
    : keyCode (0), textCharacter (0)
{
}

KeyPress::KeyPress (int code, ModifierKeys m, juce_wchar textChar) noexcept
    : keyCode (code), mods (m), textCharacter (textChar)
{
}

KeyPress::KeyPress (const int code) noexcept
    : keyCode (code), textCharacter (0)
{
}

KeyPress::KeyPress (const KeyPress& other) noexcept
    : keyCode (other.keyCode), mods (other.mods),
      textCharacter (other.textCharacter)
{
}

KeyPress& KeyPress::operator= (const KeyPress& other) noexcept
{
    keyCode = other.keyCode;
    mods = other.mods;
    textCharacter = other.textCharacter;

    return *this;
}

bool KeyPress::operator== (int otherKeyCode) const noexcept
{
    return keyCode == otherKeyCode && ! mods.isAnyModifierKeyDown();
}

bool KeyPress::operator== (const KeyPress& other) const noexcept
{
    return mods.getRawFlags() == other.mods.getRawFlags()
            && (textCharacter == other.textCharacter
                 || textCharacter == 0
                 || other.textCharacter == 0)
            && (keyCode == other.keyCode
                 || (keyCode < 256
                      && other.keyCode < 256
                      && CharacterFunctions::toLowerCase ((juce_wchar) keyCode)
                           == CharacterFunctions::toLowerCase ((juce_wchar) other.keyCode)));
}

bool KeyPress::operator!= (const KeyPress& other) const noexcept
{
    return ! operator== (other);
}

bool KeyPress::operator!= (int otherKeyCode) const noexcept
{
    return ! operator== (otherKeyCode);
}

bool KeyPress::isCurrentlyDown() const
{
    return isKeyCurrentlyDown (keyCode)
            && (ModifierKeys::getCurrentModifiers().getRawFlags() & ModifierKeys::allKeyboardModifiers)
                  == (mods.getRawFlags() & ModifierKeys::allKeyboardModifiers);
}

//==============================================================================
namespace KeyPressHelpers
{
    struct KeyNameAndCode
    {
        const char* name;
        int code;
    };

    const KeyNameAndCode translations[] =
    {
        { "spacebar",       KeyPress::spaceKey },
        { "return",         KeyPress::returnKey },
        { "escape",         KeyPress::escapeKey },
        { "backspace",      KeyPress::backspaceKey },
        { "cursor left",    KeyPress::leftKey },
        { "cursor right",   KeyPress::rightKey },
        { "cursor up",      KeyPress::upKey },
        { "cursor down",    KeyPress::downKey },
        { "page up",        KeyPress::pageUpKey },
        { "page down",      KeyPress::pageDownKey },
        { "home",           KeyPress::homeKey },
        { "end",            KeyPress::endKey },
        { "delete",         KeyPress::deleteKey },
        { "insert",         KeyPress::insertKey },
        { "tab",            KeyPress::tabKey },
        { "play",           KeyPress::playKey },
        { "stop",           KeyPress::stopKey },
        { "fast forward",   KeyPress::fastForwardKey },
        { "rewind",         KeyPress::rewindKey }
    };

    struct ModifierDescription
    {
        const char* name;
        int flag;
    };

    static const ModifierDescription modifierNames[] =
    {
        { "ctrl",      ModifierKeys::ctrlModifier },
        { "control",   ModifierKeys::ctrlModifier },
        { "ctl",       ModifierKeys::ctrlModifier },
        { "shift",     ModifierKeys::shiftModifier },
        { "shft",      ModifierKeys::shiftModifier },
        { "alt",       ModifierKeys::altModifier },
        { "option",    ModifierKeys::altModifier },
        { "command",   ModifierKeys::commandModifier },
        { "cmd",       ModifierKeys::commandModifier }
    };

    static const char* numberPadPrefix() noexcept      { return "numpad "; }

    static int getNumpadKeyCode (const String& desc)
    {
        if (desc.containsIgnoreCase (numberPadPrefix()))
        {
            const juce_wchar lastChar = desc.trimEnd().getLastCharacter();

            switch (lastChar)
            {
                case '0': case '1': case '2': case '3': case '4':
                case '5': case '6': case '7': case '8': case '9':
                    return (int) (KeyPress::numberPad0 + lastChar - '0');

                case '+':   return KeyPress::numberPadAdd;
                case '-':   return KeyPress::numberPadSubtract;
                case '*':   return KeyPress::numberPadMultiply;
                case '/':   return KeyPress::numberPadDivide;
                case '.':   return KeyPress::numberPadDecimalPoint;
                case '=':   return KeyPress::numberPadEquals;

                default:    break;
            }

            if (desc.endsWith ("separator"))  return KeyPress::numberPadSeparator;
            if (desc.endsWith ("delete"))     return KeyPress::numberPadDelete;
        }

        return 0;
    }

   #if JUCE_MAC
    struct OSXSymbolReplacement
    {
        const char* text;
        juce_wchar symbol;
    };

    const OSXSymbolReplacement osxSymbols[] =
    {
        { "shift + ",     0x21e7 },
        { "command + ",   0x2318 },
        { "option + ",    0x2325 },
        { "ctrl + ",      0x2303 },
        { "return",       0x21b5 },
        { "cursor left",  0x2190 },
        { "cursor right", 0x2192 },
        { "cursor up",    0x2191 },
        { "cursor down",  0x2193 },
        { "backspace",    0x232b },
        { "delete",       0x2326 },
        { "spacebar",     0x2423 }
    };
   #endif
}

//==============================================================================
KeyPress KeyPress::createFromDescription (const String& desc)
{
    int modifiers = 0;

    for (int i = 0; i < numElementsInArray (KeyPressHelpers::modifierNames); ++i)
        if (desc.containsWholeWordIgnoreCase (KeyPressHelpers::modifierNames[i].name))
            modifiers |= KeyPressHelpers::modifierNames[i].flag;

    int key = 0;

    for (int i = 0; i < numElementsInArray (KeyPressHelpers::translations); ++i)
    {
        if (desc.containsWholeWordIgnoreCase (String (KeyPressHelpers::translations[i].name)))
        {
            key = KeyPressHelpers::translations[i].code;
            break;
        }
    }

    if (key == 0)
        key = KeyPressHelpers::getNumpadKeyCode (desc);

    if (key == 0)
    {
        // see if it's a function key..
        if (! desc.containsChar ('#')) // avoid mistaking hex-codes like "#f1"
            for (int i = 1; i <= 12; ++i)
                if (desc.containsWholeWordIgnoreCase ("f" + String (i)))
                    key = F1Key + i - 1;

        if (key == 0)
        {
            // give up and use the hex code..
            const int hexCode = desc.fromFirstOccurrenceOf ("#", false, false)
                                    .retainCharacters ("0123456789abcdefABCDEF")
                                    .getHexValue32();

            if (hexCode > 0)
                key = hexCode;
            else
                key = (int) CharacterFunctions::toUpperCase (desc.getLastCharacter());
        }
    }

    return KeyPress (key, ModifierKeys (modifiers), 0);
}

String KeyPress::getTextDescription() const
{
    String desc;

    if (keyCode > 0)
    {
        // some keyboard layouts use a shift-key to get the slash, but in those cases, we
        // want to store it as being a slash, not shift+whatever.
        if (textCharacter == '/')
            return "/";

        if (mods.isCtrlDown())      desc << "ctrl + ";
        if (mods.isShiftDown())     desc << "shift + ";

       #if JUCE_MAC
        if (mods.isAltDown())       desc << "option + ";
        if (mods.isCommandDown())   desc << "command + ";
       #else
        if (mods.isAltDown())       desc << "alt + ";
       #endif

        for (int i = 0; i < numElementsInArray (KeyPressHelpers::translations); ++i)
            if (keyCode == KeyPressHelpers::translations[i].code)
                return desc + KeyPressHelpers::translations[i].name;

        if (keyCode >= F1Key && keyCode <= F16Key)                  desc << 'F' << (1 + keyCode - F1Key);
        else if (keyCode >= numberPad0 && keyCode <= numberPad9)    desc << KeyPressHelpers::numberPadPrefix() << (keyCode - numberPad0);
        else if (keyCode >= 33 && keyCode < 176)        desc += CharacterFunctions::toUpperCase ((juce_wchar) keyCode);
        else if (keyCode == numberPadAdd)               desc << KeyPressHelpers::numberPadPrefix() << '+';
        else if (keyCode == numberPadSubtract)          desc << KeyPressHelpers::numberPadPrefix() << '-';
        else if (keyCode == numberPadMultiply)          desc << KeyPressHelpers::numberPadPrefix() << '*';
        else if (keyCode == numberPadDivide)            desc << KeyPressHelpers::numberPadPrefix() << '/';
        else if (keyCode == numberPadSeparator)         desc << KeyPressHelpers::numberPadPrefix() << "separator";
        else if (keyCode == numberPadDecimalPoint)      desc << KeyPressHelpers::numberPadPrefix() << '.';
        else if (keyCode == numberPadDelete)            desc << KeyPressHelpers::numberPadPrefix() << "delete";
        else                                            desc << '#' << String::toHexString (keyCode);
    }

    return desc;
}

String KeyPress::getTextDescriptionWithIcons() const
{
   #if JUCE_MAC
    String s (getTextDescription());

    for (int i = 0; i < numElementsInArray (KeyPressHelpers::osxSymbols); ++i)
        s = s.replace (KeyPressHelpers::osxSymbols[i].text,
                       String::charToString (KeyPressHelpers::osxSymbols[i].symbol));

    return s;
   #else
    return getTextDescription();
   #endif
}

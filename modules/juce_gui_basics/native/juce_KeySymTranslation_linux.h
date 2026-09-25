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

   JUCE End User Licence Agreement: https://juce.com/legal/juce-9-licence/
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

struct KeySymTranslation
{
    KeySymTranslation() = delete;

    // The public KeyPress constants and both Linux backends build extended-key codes from this.
    static constexpr int extendedKeyModifier = 0x10000000;

    // Mirrors <xkbcommon/xkbcommon-keysyms.h>, numerically identical to the X11 XK_ values.
    // Shared spelling so neither Linux backend needs the X11 or xkb headers to translate a key.
    enum : uint32_t
    {
        keyAsterisk   = 0x2a,
        keyPlus       = 0x2b,
        keySlash      = 0x2f,
        key0          = 0x30,
        key1          = 0x31,
        key2          = 0x32,
        key3          = 0x33,
        key4          = 0x34,
        key5          = 0x35,
        key6          = 0x36,
        key7          = 0x37,
        key8          = 0x38,
        key9          = 0x39,
        keyHyphen     = 0xad,
        keyISOLeftTab = 0xfe20,
        keyBackSpace  = 0xff08,
        keyTab        = 0xff09,
        keyReturn     = 0xff0d,
        keyScrollLock = 0xff14,
        keyEscape     = 0xff1b,
        keyHome       = 0xff50,
        keyLeft       = 0xff51,
        keyUp         = 0xff52,
        keyRight      = 0xff53,
        keyDown       = 0xff54,
        keyPageUp     = 0xff55,
        keyPageDown   = 0xff56,
        keyEnd        = 0xff57,
        keyInsert     = 0xff63,
        keyNumLock    = 0xff7f,
        keyKPEnter    = 0xff8d,
        keyKPHome     = 0xff95,
        keyKPLeft     = 0xff96,
        keyKPUp       = 0xff97,
        keyKPRight    = 0xff98,
        keyKPDown     = 0xff99,
        keyKPPageUp   = 0xff9a,
        keyKPPageDown = 0xff9b,
        keyKPEnd      = 0xff9c,
        keyKPInsert   = 0xff9e,
        keyKPDelete   = 0xff9f,
        keyKPMultiply = 0xffaa,
        keyKPAdd      = 0xffab,
        keyKPSubtract = 0xffad,
        keyKPDivide   = 0xffaf,
        keyKP0        = 0xffb0,
        keyKP1        = 0xffb1,
        keyKP2        = 0xffb2,
        keyKP3        = 0xffb3,
        keyKP4        = 0xffb4,
        keyKP5        = 0xffb5,
        keyKP6        = 0xffb6,
        keyKP7        = 0xffb7,
        keyKP8        = 0xffb8,
        keyKP9        = 0xffb9,
        keyF1         = 0xffbe,
        keyF35        = 0xffe0,
        keyShiftL     = 0xffe1,
        keyShiftR     = 0xffe2,
        keyControlL   = 0xffe3,
        keyControlR   = 0xffe4,
        keyCapsLock   = 0xffe5,
        keyAltL       = 0xffe9,
        keyAltR       = 0xffea,
        keyDelete     = 0xffff
    };

    struct KeyPressTranslation
    {
        int keyCode;
        bool keyPressed;
    };

    // hasCharacter is true when the key event also produced text (X11 utf8[0], Wayland utf32).
    static KeyPressTranslation translateKeySymToKeyPress (uint32_t sym, int initialKeyCode, bool hasCharacter)
    {
        auto keyCode = initialKeyCode;
        auto keyPressed = false;

        if ((sym & 0xff00) == 0xff00 || keyCode == (int) keyISOLeftTab)
        {
            switch (sym)  // Translate keypad
            {
                case keyKPAdd:      keyCode = keyPlus;     break;
                case keyKPSubtract: keyCode = keyHyphen;   break;
                case keyKPDivide:   keyCode = keySlash;    break;
                case keyKPMultiply: keyCode = keyAsterisk; break;
                case keyKPEnter:    keyCode = keyReturn;   break;
                case keyKPInsert:   keyCode = keyInsert;   break;
                case keyDelete:
                case keyKPDelete:   keyCode = keyDelete;   break;
                case keyKPLeft:     keyCode = keyLeft;     break;
                case keyKPRight:    keyCode = keyRight;    break;
                case keyKPUp:       keyCode = keyUp;       break;
                case keyKPDown:     keyCode = keyDown;     break;
                case keyKPHome:     keyCode = keyHome;     break;
                case keyKPEnd:      keyCode = keyEnd;      break;
                case keyKPPageDown: keyCode = keyPageDown; break;
                case keyKPPageUp:   keyCode = keyPageUp;   break;

                case keyKP0:        keyCode = key0;        break;
                case keyKP1:        keyCode = key1;        break;
                case keyKP2:        keyCode = key2;        break;
                case keyKP3:        keyCode = key3;        break;
                case keyKP4:        keyCode = key4;        break;
                case keyKP5:        keyCode = key5;        break;
                case keyKP6:        keyCode = key6;        break;
                case keyKP7:        keyCode = key7;        break;
                case keyKP8:        keyCode = key8;        break;
                case keyKP9:        keyCode = key9;        break;

                default:            break;
            }

            switch (keyCode)
            {
                case keyLeft:
                case keyRight:
                case keyUp:
                case keyDown:
                case keyPageUp:
                case keyPageDown:
                case keyEnd:
                case keyHome:
                case keyDelete:
                case keyInsert:
                    keyPressed = true;
                    keyCode = (keyCode & 0xff) | extendedKeyModifier;
                    break;

                case keyTab:
                case keyReturn:
                case keyEscape:
                case keyBackSpace:
                    keyPressed = true;
                    keyCode &= 0xff;
                    break;

                case keyISOLeftTab:
                    keyPressed = true;
                    keyCode = keyTab & 0xff;
                    break;

                default:
                    if (sym >= keyF1 && sym <= keyF35)
                    {
                        keyPressed = true;
                        keyCode = (int) ((sym & 0xff) | extendedKeyModifier);
                    }
                    break;
            }
        }

        if (hasCharacter || ((sym & 0xff00) == 0 && sym >= 8))
            keyPressed = true;

        return { keyCode, keyPressed };
    }

    // Folds a KeyPress code back to the keysym that key-state queries compare against, the reverse
    // of the mapping above. Mirrors X11 isKeyCurrentlyDown behaviour.
    static constexpr uint32_t keySymForKeyPressCode (int keyCode)
    {
        if ((keyCode & extendedKeyModifier) != 0)
            return 0xff00u | (uint32_t) (keyCode & 0xff);

        const auto sym = (uint32_t) keyCode;

        if (sym == (keyTab & 0xffu) || sym == (keyReturn & 0xffu)
            || sym == (keyEscape & 0xffu) || sym == (keyBackSpace & 0xffu))
            return 0xff00u | sym;

        return sym;
    }
};

} // namespace juce

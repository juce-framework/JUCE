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
/** This class is used to invoke a range of text-editor navigation methods on
    an object, based upon a keypress event.

    It's currently used internally by the TextEditor and CodeEditorComponent.

    @tags{GUI}
*/
template <class CallbackClass>
struct TextEditorKeyMapper
{
    /** Checks the keypress and invokes one of a range of navigation functions that
        the target class must implement, based on the key event.
    */
    static bool invokeKeyFunction (CallbackClass& target, const KeyPress& key)
    {
        auto mods = key.getModifiers();

        const bool isShiftDown   = mods.isShiftDown();
        const bool ctrlOrAltDown = mods.isCtrlDown() || mods.isAltDown();

        int numCtrlAltCommandKeys = 0;
        if (mods.isCtrlDown())    ++numCtrlAltCommandKeys;
        if (mods.isAltDown())     ++numCtrlAltCommandKeys;

        if (key == KeyPress (KeyPress::downKey, ModifierKeys::ctrlModifier, 0) && target.scrollUp())   return true;
        if (key == KeyPress (KeyPress::upKey,   ModifierKeys::ctrlModifier, 0) && target.scrollDown()) return true;

       #if JUCE_MAC
        if (mods.isCommandDown() && ! ctrlOrAltDown)
        {
            if (key.isKeyCode (KeyPress::upKey))        return target.moveCaretToTop (isShiftDown);
            if (key.isKeyCode (KeyPress::downKey))      return target.moveCaretToEnd (isShiftDown);
            if (key.isKeyCode (KeyPress::leftKey))      return target.moveCaretToStartOfLine (isShiftDown);
            if (key.isKeyCode (KeyPress::rightKey))     return target.moveCaretToEndOfLine   (isShiftDown);
        }

        if (mods.isCommandDown())
            ++numCtrlAltCommandKeys;
       #endif

        if (numCtrlAltCommandKeys < 2)
        {
            if (key.isKeyCode (KeyPress::leftKey))  return target.moveCaretLeft  (ctrlOrAltDown, isShiftDown);
            if (key.isKeyCode (KeyPress::rightKey)) return target.moveCaretRight (ctrlOrAltDown, isShiftDown);

            if (key.isKeyCode (KeyPress::homeKey))  return ctrlOrAltDown ? target.moveCaretToTop         (isShiftDown)
                                                                         : target.moveCaretToStartOfLine (isShiftDown);
            if (key.isKeyCode (KeyPress::endKey))   return ctrlOrAltDown ? target.moveCaretToEnd         (isShiftDown)
                                                                         : target.moveCaretToEndOfLine   (isShiftDown);
        }

        if (numCtrlAltCommandKeys == 0)
        {
            if (key.isKeyCode (KeyPress::upKey))        return target.moveCaretUp   (isShiftDown);
            if (key.isKeyCode (KeyPress::downKey))      return target.moveCaretDown (isShiftDown);

            if (key.isKeyCode (KeyPress::pageUpKey))    return target.pageUp   (isShiftDown);
            if (key.isKeyCode (KeyPress::pageDownKey))  return target.pageDown (isShiftDown);
        }

        if (key == KeyPress ('c', ModifierKeys::commandModifier, 0)
              || key == KeyPress (KeyPress::insertKey, ModifierKeys::ctrlModifier, 0))
            return target.copyToClipboard();

        if (key == KeyPress ('x', ModifierKeys::commandModifier, 0)
              || key == KeyPress (KeyPress::deleteKey, ModifierKeys::shiftModifier, 0))
            return target.cutToClipboard();

        if (key == KeyPress ('v', ModifierKeys::commandModifier, 0)
              || key == KeyPress (KeyPress::insertKey, ModifierKeys::shiftModifier, 0))
            return target.pasteFromClipboard();

        // NB: checking for delete must happen after the earlier check for shift + delete
        if (numCtrlAltCommandKeys < 2)
        {
            if (key.isKeyCode (KeyPress::backspaceKey)) return target.deleteBackwards (ctrlOrAltDown);
            if (key.isKeyCode (KeyPress::deleteKey))    return target.deleteForwards  (ctrlOrAltDown);
        }

        if (key == KeyPress ('a', ModifierKeys::commandModifier, 0))
            return target.selectAll();

        if (key == KeyPress ('z', ModifierKeys::commandModifier, 0))
            return target.undo();

        if (key == KeyPress ('y', ModifierKeys::commandModifier, 0)
             || key == KeyPress ('z', ModifierKeys::commandModifier | ModifierKeys::shiftModifier, 0))
            return target.redo();

        return false;
    }
};

} // namespace juce

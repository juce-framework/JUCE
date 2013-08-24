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

#ifndef JUCE_TEXTEDITORKEYMAPPER_H_INCLUDED
#define JUCE_TEXTEDITORKEYMAPPER_H_INCLUDED


//==============================================================================
/** This class is used to invoke a range of text-editor navigation methods on
    an object, based upon a keypress event.

    It's currently used internally by the TextEditor and CodeEditorComponent.
*/
template <class CallbackClass>
struct TextEditorKeyMapper
{
    /** Checks the keypress and invokes one of a range of navigation functions that
        the target class must implement, based on the key event.
    */
    static bool invokeKeyFunction (CallbackClass& target, const KeyPress& key)
    {
        const ModifierKeys& mods = key.getModifiers();

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

        if (numCtrlAltCommandKeys < 2)
        {
            if (key.isKeyCode (KeyPress::backspaceKey)) return target.deleteBackwards (ctrlOrAltDown);
            if (key.isKeyCode (KeyPress::deleteKey))    return target.deleteForwards  (ctrlOrAltDown);
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


#endif   // JUCE_TEXTEDITORKEYMAPPER_H_INCLUDED

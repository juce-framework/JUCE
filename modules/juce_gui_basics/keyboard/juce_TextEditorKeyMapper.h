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

#ifndef __JUCE_TEXTEDITORKEYMAPPER_JUCEHEADER__
#define __JUCE_TEXTEDITORKEYMAPPER_JUCEHEADER__

#include "juce_KeyPress.h"


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
        const bool isShiftDown   = key.getModifiers().isShiftDown();
        const bool ctrlOrAltDown = key.getModifiers().isCtrlDown() || key.getModifiers().isAltDown();

        if (key == KeyPress (KeyPress::downKey, ModifierKeys::ctrlModifier, 0) && target.scrollUp())   return true;
        if (key == KeyPress (KeyPress::upKey,   ModifierKeys::ctrlModifier, 0) && target.scrollDown()) return true;

       #if JUCE_MAC
        if (key.getModifiers().isCommandDown())
        {
            if (key.isKeyCode (KeyPress::upKey))        return target.moveCaretToTop (isShiftDown);
            if (key.isKeyCode (KeyPress::downKey))      return target.moveCaretToEnd (isShiftDown);
            if (key.isKeyCode (KeyPress::leftKey))      return target.moveCaretToStartOfLine (isShiftDown);
            if (key.isKeyCode (KeyPress::rightKey))     return target.moveCaretToEndOfLine (isShiftDown);
        }
       #endif

        if (key.isKeyCode (KeyPress::upKey))        return target.moveCaretUp (isShiftDown);
        if (key.isKeyCode (KeyPress::downKey))      return target.moveCaretDown (isShiftDown);
        if (key.isKeyCode (KeyPress::leftKey))      return target.moveCaretLeft (ctrlOrAltDown, isShiftDown);
        if (key.isKeyCode (KeyPress::rightKey))     return target.moveCaretRight (ctrlOrAltDown, isShiftDown);
        if (key.isKeyCode (KeyPress::pageUpKey))    return target.pageUp (isShiftDown);
        if (key.isKeyCode (KeyPress::pageDownKey))  return target.pageDown (isShiftDown);

        if (key.isKeyCode (KeyPress::homeKey))  return ctrlOrAltDown ? target.moveCaretToTop (isShiftDown)
                                                                     : target.moveCaretToStartOfLine (isShiftDown);
        if (key.isKeyCode (KeyPress::endKey))   return ctrlOrAltDown ? target.moveCaretToEnd (isShiftDown)
                                                                     : target.moveCaretToEndOfLine (isShiftDown);

        if (key == KeyPress ('c', ModifierKeys::commandModifier, 0)
              || key == KeyPress (KeyPress::insertKey, ModifierKeys::ctrlModifier, 0))
            return target.copyToClipboard();

        if (key == KeyPress ('x', ModifierKeys::commandModifier, 0)
              || key == KeyPress (KeyPress::deleteKey, ModifierKeys::shiftModifier, 0))
            return target.cutToClipboard();

        if (key == KeyPress ('v', ModifierKeys::commandModifier, 0)
              || key == KeyPress (KeyPress::insertKey, ModifierKeys::shiftModifier, 0))
            return target.pasteFromClipboard();

        if (key.isKeyCode (KeyPress::backspaceKey))     return target.deleteBackwards (ctrlOrAltDown);
        if (key.isKeyCode (KeyPress::deleteKey))        return target.deleteForwards (ctrlOrAltDown);

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


#endif   // __JUCE_TEXTEDITORKEYMAPPER_JUCEHEADER__

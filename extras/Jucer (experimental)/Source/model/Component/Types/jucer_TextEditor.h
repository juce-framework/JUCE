/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-10 by Raw Material Software Ltd.

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

#ifdef ADD_TO_LIST
  ADD_TO_LIST (TextEditorHandler);
#else

#include "../jucer_ComponentDocument.h"


//==============================================================================
class TextEditorHandler  : public ComponentTypeHelper<TextEditor>
{
public:
    TextEditorHandler() : ComponentTypeHelper<TextEditor> ("TextEditor", "TextEditor", "TEXTEDITOR", "textEditor")
    {
        addEditableColour (TextEditor::backgroundColourId, "Background", "backgroundColour");
        addEditableColour (TextEditor::textColourId, "Text", "textColour");
        addEditableColour (TextEditor::highlightColourId, "Highlight", "highlightColour");
        addEditableColour (TextEditor::highlightedTextColourId, "Highlighted Text", "highlightedTextColour");
        addEditableColour (TextEditor::caretColourId, "Caret", "caretColour");
        addEditableColour (TextEditor::outlineColourId, "Outline", "outlineColour");
        addEditableColour (TextEditor::focusedOutlineColourId, "Outline (focused)", "focusedOutlineColour");
        addEditableColour (TextEditor::shadowColourId, "Shadow", "shadowColour");
    }

    ~TextEditorHandler()  {}

    Component* createComponent()                { return new TextEditor(); }
    const Rectangle<int> getDefaultSize()       { return Rectangle<int> (0, 0, 180, 24); }

    void initialiseNew (ComponentTypeInstance& item)
    {
        item.set ("text", "Text Editor Content");
        item.set ("readOnly", false);
        item.set ("scrollbarsShown", true);
        item.set ("caretVisible", true);
        item.set ("popupMenuEnabled", true);
        item.set ("mode", 1);
    }

    void update (ComponentTypeInstance& item, TextEditor* comp)
    {
        comp->setReadOnly (item ["readOnly"]);
        comp->setScrollbarsShown (item ["scrollbarsShown"]);
        comp->setCaretVisible (item ["caretVisible"]);
        comp->setPopupMenuEnabled (item ["popupMenuEnabled"]);
        int mode = item ["mode"];
        comp->setMultiLine (mode > 1, true);
        comp->setReturnKeyStartsNewLine (mode != 3);
        comp->setText (item ["text"].toString());
    }

    void createProperties (ComponentTypeInstance& item, Array <PropertyComponent*>& props)
    {
        item.addTooltipProperty (props);
        item.addFocusOrderProperty (props);

        props.add (new TextPropertyComponent (item.getValue ("text"), "Text", 16384, true));
        props.getLast()->setTooltip ("The editor's initial content.");

        const char* const modes[] = { "Single-Line", "Multi-Line (Return key starts new line)", "Multi-Line (Return key disabled)", 0 };
        props.add (new ChoicePropertyComponent (item.getValue ("mode"), "Mode", StringArray (modes)));

        props.add (new BooleanPropertyComponent (item.getValue ("readOnly"), "Read-Only", "Read-Only"));
        props.add (new BooleanPropertyComponent (item.getValue ("scrollbarsShown"), "Scrollbars", "Scrollbars Shown"));
        props.add (new BooleanPropertyComponent (item.getValue ("caretVisible"), "Caret", "Caret Visible"));
        props.add (new BooleanPropertyComponent (item.getValue ("popupMenuEnabled"), "Popup Menu", "Popup Menu Enabled"));

        addEditableColourProperties (item, props);
    }

    void createCode (ComponentTypeInstance& item, CodeGenerator& code)
    {
        code.constructorCode += item.createConstructorStatement (String::empty);
    }
};

#endif

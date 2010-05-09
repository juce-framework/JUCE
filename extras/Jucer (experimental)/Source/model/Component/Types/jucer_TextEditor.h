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
    TextEditorHandler() : ComponentTypeHelper<TextEditor> ("TextEditor", "TEXTEDITOR", "textEditor")
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

    void initialiseNew (ComponentDocument& document, ValueTree& state)
    {
        state.setProperty ("text", "Text Editor Content", 0);
        state.setProperty ("readOnly", false, 0);
        state.setProperty ("scrollbarsShown", true, 0);
        state.setProperty ("caretVisible", true, 0);
        state.setProperty ("popupMenuEnabled", true, 0);
        state.setProperty ("mode", 1, 0);
    }

    void update (ComponentDocument& document, TextEditor* comp, const ValueTree& state)
    {
        comp->setReadOnly (state["readOnly"]);
        comp->setScrollbarsShown (state ["scrollbarsShown"]);
        comp->setCaretVisible (state ["caretVisible"]);
        comp->setPopupMenuEnabled (state ["popupMenuEnabled"]);
        int mode = state ["mode"];
        comp->setMultiLine (mode > 1, true);
        comp->setReturnKeyStartsNewLine (mode != 3);
        comp->setText (state ["text"].toString());
    }

    void createProperties (ComponentDocument& document, ValueTree& state, Array <PropertyComponent*>& props)
    {
        addTooltipProperty (document, state, props);
        addFocusOrderProperty (document, state, props);

        props.add (new TextPropertyComponent (getValue ("text", state, document), "Text", 16384, true));
        props.getLast()->setTooltip ("The editor's initial content.");

        const char* const modes[] = { "Single-Line", "Multi-Line (Return key starts new line)", "Multi-Line (Return key disabled)", 0 };
        props.add (new ChoicePropertyComponent (getValue ("mode", state, document), "Mode", StringArray (modes)));

        props.add (new BooleanPropertyComponent (getValue ("readOnly", state, document), "Read-Only", "Read-Only"));
        props.add (new BooleanPropertyComponent (getValue ("scrollbarsShown", state, document), "Scrollbars", "Scrollbars Shown"));
        props.add (new BooleanPropertyComponent (getValue ("caretVisible", state, document), "Caret", "Caret Visible"));
        props.add (new BooleanPropertyComponent (getValue ("popupMenuEnabled", state, document), "Popup Menu", "Popup Menu Enabled"));

        addEditableColourProperties (document, state, props);
    }
};

#endif

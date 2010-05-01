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

    void update (ComponentDocument& document, TextEditor* comp, const ValueTree& state)
    {
        comp->setText (state ["text"].toString());
    }

    void initialiseNew (ComponentDocument& document, ValueTree& state)
    {
        state.setProperty ("text", "Text Editor Content", 0);
    }

    void createProperties (ComponentDocument& document, ValueTree& state, Array <PropertyComponent*>& props)
    {
        addTooltipProperty (document, state, props);
        addFocusOrderProperty (document, state, props);

        props.add (new TextPropertyComponent (getValue ("text", state, document), "Text", 16384, true));
        props.getLast()->setTooltip ("The editor's initial content.");

        addEditableColourProperties (document, state, props);
    }
};

#endif

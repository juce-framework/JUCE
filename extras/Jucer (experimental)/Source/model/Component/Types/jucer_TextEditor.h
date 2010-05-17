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
        TextEditor defaultComp;

        item.set (Ids::text, "Text Editor Content");
        item.set (Ids::readOnly, false);
        item.set (Ids::scrollbarsShown, true);
        item.set (Ids::caretVisible, true);
        item.set (Ids::popupMenuEnabled, true);
        item.set (Ids::mode, 1);
        item.set (Ids::font, defaultComp.getFont().toString());
    }

    void update (ComponentTypeInstance& item, TextEditor* comp)
    {
        comp->setReadOnly (item [Ids::readOnly]);
        comp->setScrollbarsShown (item [Ids::scrollbarsShown]);
        comp->setCaretVisible (item [Ids::caretVisible]);
        comp->setPopupMenuEnabled (item [Ids::popupMenuEnabled]);
        int mode = item [Ids::mode];
        comp->setMultiLine (mode > 1, true);
        comp->setReturnKeyStartsNewLine (mode != 3);

        const Font font (Font::fromString (item [Ids::font]));
        if (comp->getFont() != font)
            comp->applyFontToAllText (font);

        comp->setText (item [Ids::text].toString());
    }

    void createProperties (ComponentTypeInstance& item, Array <PropertyComponent*>& props)
    {
        item.addTooltipProperty (props);
        item.addFocusOrderProperty (props);

        props.add (new TextPropertyComponent (item.getValue (Ids::text), "Text", 16384, true));
        props.getLast()->setTooltip ("The editor's initial content.");

        const char* const modes[] = { "Single-Line", "Multi-Line (Return key starts new line)", "Multi-Line (Return key disabled)", 0 };
        const int values[] = { 1, 2, 3, 0 };
        props.add (new ChoicePropertyComponent (item.getValue (Ids::mode), "Mode", StringArray (modes), Array<var> (values)));

        props.add (new BooleanPropertyComponent (item.getValue (Ids::readOnly), "Read-Only", "Read-Only"));
        props.add (new BooleanPropertyComponent (item.getValue (Ids::scrollbarsShown), "Scrollbars", "Scrollbars Shown"));
        props.add (new BooleanPropertyComponent (item.getValue (Ids::caretVisible), "Caret", "Caret Visible"));
        props.add (new BooleanPropertyComponent (item.getValue (Ids::popupMenuEnabled), "Popup Menu", "Popup Menu Enabled"));

        item.addFontProperties (props, Ids::font);

        addEditableColourProperties (item, props);
    }

    void createCode (ComponentTypeInstance& item, CodeGenerator& code)
    {
        TextEditor defaultComp;

        code.constructorCode
            << item.createConstructorStatement (String::empty);

        if (defaultComp.isReadOnly() != (bool) item [Ids::readOnly])
            code.constructorCode << item.getMemberName() << "->setReadOnly (" << CodeHelpers::boolLiteral (item [Ids::readOnly]) << ");" << newLine;

        if (defaultComp.areScrollbarsShown() != (bool) item [Ids::scrollbarsShown])
            code.constructorCode << item.getMemberName() << "->setScrollbarsShown (" << CodeHelpers::boolLiteral (item [Ids::scrollbarsShown]) << ");" << newLine;

        if (defaultComp.isCaretVisible() != (bool) item [Ids::caretVisible])
            code.constructorCode << item.getMemberName() << "->setCaretVisible (" << CodeHelpers::boolLiteral (item [Ids::caretVisible]) << ");" << newLine;

        if (defaultComp.isPopupMenuEnabled() != (bool) item [Ids::popupMenuEnabled])
            code.constructorCode << item.getMemberName() << "->setPopupMenuEnabled (" << CodeHelpers::boolLiteral (item [Ids::popupMenuEnabled]) << ");" << newLine;

        int mode = item [Ids::mode];
        if (defaultComp.isMultiLine() != (mode > 1))
            code.constructorCode << item.getMemberName() << "->setMultiLine (" << CodeHelpers::boolLiteral (mode > 1) << ", true);" << newLine;

        if (defaultComp.getReturnKeyStartsNewLine() != (mode != 3))
            code.constructorCode << item.getMemberName() << "->setReturnKeyStartsNewLine (" << CodeHelpers::boolLiteral (mode != 3) << ");" << newLine;

        const Font font (Font::fromString (item [Ids::font]));
        if (font != defaultComp.getFont())
            code.constructorCode << item.getMemberName() << "->setFont (" << CodeHelpers::fontToCode (font) << ");" << newLine;

        code.constructorCode << item.getMemberName() << "->setText (" << CodeHelpers::stringLiteral (item [Ids::text]) << ");" << newLine;
    }
};

#endif

/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

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

class TextEditorHandler  : public ComponentTypeHandler
{
public:
    TextEditorHandler()
        : ComponentTypeHandler ("Text Editor", "TextEditor", typeid (TextEditor), 150, 24)
    {
        registerColour (TextEditor::textColourId, "text", "textcol");
        registerColour (TextEditor::backgroundColourId, "background", "bkgcol");
        registerColour (TextEditor::highlightColourId, "highlight", "hilitecol");
        registerColour (TextEditor::outlineColourId, "outline", "outlinecol");
        registerColour (TextEditor::shadowColourId, "shadow", "shadowcol");
        registerColour (CaretComponent::caretColourId, "caret", "caretcol");
    }

    Component* createNewComponent (JucerDocument*)
    {
        return new TextEditor ("new text editor");
    }

    XmlElement* createXmlFor (Component* comp, const ComponentLayout* layout)
    {
        XmlElement* e = ComponentTypeHandler::createXmlFor (comp, layout);
        TextEditor* te = (TextEditor*) comp;

        e->setAttribute ("initialText", comp->getProperties() ["initialText"].toString());

        e->setAttribute ("multiline", te->isMultiLine());
        e->setAttribute ("retKeyStartsLine", te->getReturnKeyStartsNewLine());
        e->setAttribute ("readonly", te->isReadOnly());
        e->setAttribute ("scrollbars", te->areScrollbarsShown());
        e->setAttribute ("caret", te->isCaretVisible());
        e->setAttribute ("popupmenu", te->isPopupMenuEnabled());

        return e;
    }

    bool restoreFromXml (const XmlElement& xml, Component* comp, const ComponentLayout* layout)
    {
        if (! ComponentTypeHandler::restoreFromXml (xml, comp, layout))
            return false;

        TextEditor* te = (TextEditor*) comp;
        TextEditor defaultEditor;

        te->setMultiLine (xml.getBoolAttribute ("multiline", defaultEditor.isMultiLine()));
        te->setReturnKeyStartsNewLine (xml.getBoolAttribute ("retKeyStartsLine", defaultEditor.getReturnKeyStartsNewLine()));
        te->setReadOnly (xml.getBoolAttribute ("readonly", defaultEditor.isReadOnly()));
        te->setScrollbarsShown (xml.getBoolAttribute ("scrollbars", defaultEditor.areScrollbarsShown()));
        te->setCaretVisible (xml.getBoolAttribute ("caret", defaultEditor.isCaretVisible()));
        te->setPopupMenuEnabled (xml.getBoolAttribute ("popupmenu", defaultEditor.isPopupMenuEnabled()));

        const String initialText (xml.getStringAttribute ("initialText"));
        te->setText (initialText, false);
        te->getProperties().set ("initialText", initialText);
        return true;
    }

    void getEditableProperties (Component* component, JucerDocument& document, Array<PropertyComponent*>& props)
    {
        ComponentTypeHandler::getEditableProperties (component, document, props);

        TextEditor* const t = dynamic_cast<TextEditor*> (component);
        jassert (t != nullptr);

        props.add (new TextEditorInitialTextProperty (t, document));
        props.add (new TextEditorMultiLineProperty (t, document));
        props.add (new TextEditorReadOnlyProperty (t, document));
        props.add (new TextEditorScrollbarsProperty (t, document));
        props.add (new TextEditorCaretProperty (t, document));
        props.add (new TextEditorPopupMenuProperty (t, document));

        addColourProperties (t, document, props);
    }

    String getCreationParameters (GeneratedCode&, Component* component)
    {
        return quotedString (component->getName(), false);
    }

    void fillInCreationCode (GeneratedCode& code, Component* component, const String& memberVariableName)
    {
        ComponentTypeHandler::fillInCreationCode (code, component, memberVariableName);

        TextEditor* const te = dynamic_cast<TextEditor*> (component);
        jassert (te != 0);

        String s;
        s << memberVariableName << "->setMultiLine (" << CodeHelpers::boolLiteral (te->isMultiLine()) << ");\n"
          << memberVariableName << "->setReturnKeyStartsNewLine (" << CodeHelpers::boolLiteral (te->getReturnKeyStartsNewLine()) << ");\n"
          << memberVariableName << "->setReadOnly (" << CodeHelpers::boolLiteral (te->isReadOnly()) << ");\n"
          << memberVariableName << "->setScrollbarsShown (" << CodeHelpers::boolLiteral (te->areScrollbarsShown()) << ");\n"
          << memberVariableName << "->setCaretVisible (" << CodeHelpers::boolLiteral (te->isCaretVisible()) << ");\n"
          << memberVariableName << "->setPopupMenuEnabled (" << CodeHelpers::boolLiteral (te->isPopupMenuEnabled()) << ");\n"
          << getColourIntialisationCode (component, memberVariableName)
          << memberVariableName << "->setText (" << quotedString (te->getProperties() ["initialText"].toString(), code.shouldUseTransMacro()) << ");\n\n";

        code.constructorCode += s;
    }

private:
    //==============================================================================
    class TextEditorMultiLineProperty  : public ComponentChoiceProperty <TextEditor>
    {
    public:
        TextEditorMultiLineProperty (TextEditor* comp, JucerDocument& doc)
            : ComponentChoiceProperty <TextEditor> ("mode", comp, doc)
        {
            choices.add ("single line");
            choices.add ("multi-line, return key starts new line");
            choices.add ("multi-line, return key disabled");
        }

        void setIndex (int newIndex)
        {
            document.perform (new TextEditorMultilineChangeAction (component, *document.getComponentLayout(), newIndex),
                              "Change TextEditor multiline mode");
        }

        int getIndex() const
        {
            return component->isMultiLine() ? (component->getReturnKeyStartsNewLine() ? 1 : 2) : 0;
        }

    private:
        class TextEditorMultilineChangeAction  : public ComponentUndoableAction <TextEditor>
        {
        public:
            TextEditorMultilineChangeAction (TextEditor* const comp, ComponentLayout& l, const int newState_)
                : ComponentUndoableAction <TextEditor> (comp, l),
                  newState (newState_)
            {
                oldState = comp->isMultiLine() ? (comp->getReturnKeyStartsNewLine() ? 1 : 2) : 0;
            }

            bool perform()
            {
                showCorrectTab();
                getComponent()->setMultiLine (newState > 0);
                getComponent()->setReturnKeyStartsNewLine (newState == 1);
                changed();
                return true;
            }

            bool undo()
            {
                showCorrectTab();
                getComponent()->setMultiLine (oldState > 0);
                getComponent()->setReturnKeyStartsNewLine (oldState == 1);
                changed();
                return true;
            }

            int newState, oldState;
        };
    };

    //==============================================================================
    class TextEditorReadOnlyProperty  : public ComponentBooleanProperty <TextEditor>
    {
    public:
        TextEditorReadOnlyProperty (TextEditor* comp, JucerDocument& doc)
            : ComponentBooleanProperty <TextEditor> ("editable", "Editable", "Editable", comp, doc)
        {
        }

        void setState (bool newState)
        {
            document.perform (new TextEditorReadonlyChangeAction (component, *document.getComponentLayout(), ! newState),
                              "Change TextEditor read-only mode");
        }

        bool getState() const        { return ! component->isReadOnly(); }

    private:
        class TextEditorReadonlyChangeAction  : public ComponentUndoableAction <TextEditor>
        {
        public:
            TextEditorReadonlyChangeAction (TextEditor* const comp, ComponentLayout& l, const bool newState_)
                : ComponentUndoableAction <TextEditor> (comp, l),
                  newState (newState_)
            {
                oldState = comp->isReadOnly();
            }

            bool perform()
            {
                showCorrectTab();
                getComponent()->setReadOnly (newState);
                changed();
                return true;
            }

            bool undo()
            {
                showCorrectTab();
                getComponent()->setReadOnly (oldState);
                changed();
                return true;
            }

            bool newState, oldState;
        };
    };

    //==============================================================================
    class TextEditorScrollbarsProperty  : public ComponentBooleanProperty <TextEditor>
    {
    public:
        TextEditorScrollbarsProperty (TextEditor* comp, JucerDocument& doc)
            : ComponentBooleanProperty <TextEditor> ("scrollbars", "Scrollbars enabled", "Scrollbars enabled", comp, doc)
        {
        }

        void setState (bool newState)
        {
            document.perform (new TextEditorScrollbarChangeAction (component, *document.getComponentLayout(), newState),
                              "Change TextEditor scrollbars");
        }

        bool getState() const        { return component->areScrollbarsShown(); }

    private:
        class TextEditorScrollbarChangeAction  : public ComponentUndoableAction <TextEditor>
        {
        public:
            TextEditorScrollbarChangeAction (TextEditor* const comp, ComponentLayout& l, const bool newState_)
                : ComponentUndoableAction <TextEditor> (comp, l),
                  newState (newState_)
            {
                oldState = comp->areScrollbarsShown();
            }

            bool perform()
            {
                showCorrectTab();
                getComponent()->setScrollbarsShown (newState);
                changed();
                return true;
            }

            bool undo()
            {
                showCorrectTab();
                getComponent()->setScrollbarsShown (oldState);
                changed();
                return true;
            }

            bool newState, oldState;
        };
    };

    //==============================================================================
    class TextEditorCaretProperty  : public ComponentBooleanProperty <TextEditor>
    {
    public:
        TextEditorCaretProperty (TextEditor* comp, JucerDocument& doc)
            : ComponentBooleanProperty <TextEditor> ("caret", "Caret visible", "Caret visible", comp, doc)
        {
        }

        void setState (bool newState)
        {
            document.perform (new TextEditorCaretChangeAction (component, *document.getComponentLayout(), newState),
                              "Change TextEditor caret");
        }

        bool getState() const        { return component->isCaretVisible(); }

    private:
        class TextEditorCaretChangeAction  : public ComponentUndoableAction <TextEditor>
        {
        public:
            TextEditorCaretChangeAction (TextEditor* const comp, ComponentLayout& l, const bool newState_)
                : ComponentUndoableAction <TextEditor> (comp, l),
                  newState (newState_)
            {
                oldState = comp->isCaretVisible();
            }

            bool perform()
            {
                showCorrectTab();
                getComponent()->setCaretVisible (newState);
                changed();
                return true;
            }

            bool undo()
            {
                showCorrectTab();
                getComponent()->setCaretVisible (oldState);
                changed();
                return true;
            }

            bool newState, oldState;
        };
    };

    //==============================================================================
    class TextEditorPopupMenuProperty  : public ComponentBooleanProperty <TextEditor>
    {
    public:
        TextEditorPopupMenuProperty (TextEditor* comp, JucerDocument& doc)
            : ComponentBooleanProperty <TextEditor> ("popup menu", "Popup menu enabled", "Popup menu enabled", comp, doc)
        {
        }

        void setState (bool newState)
        {
            document.perform (new TextEditorPopupMenuChangeAction (component, *document.getComponentLayout(), newState),
                              "Change TextEditor popup menu");
        }

        bool getState() const        { return component->isPopupMenuEnabled(); }

    private:
        class TextEditorPopupMenuChangeAction  : public ComponentUndoableAction <TextEditor>
        {
        public:
            TextEditorPopupMenuChangeAction (TextEditor* const comp, ComponentLayout& l, const bool newState_)
                : ComponentUndoableAction <TextEditor> (comp, l),
                  newState (newState_)
            {
                oldState = comp->isPopupMenuEnabled();
            }

            bool perform()
            {
                showCorrectTab();
                getComponent()->setPopupMenuEnabled (newState);
                changed();
                return true;
            }

            bool undo()
            {
                showCorrectTab();
                getComponent()->setPopupMenuEnabled (oldState);
                changed();
                return true;
            }

            bool newState, oldState;
        };
    };

    //==============================================================================
    class TextEditorInitialTextProperty  : public ComponentTextProperty <TextEditor>
    {
    public:
        TextEditorInitialTextProperty (TextEditor* comp, JucerDocument& doc)
            : ComponentTextProperty <TextEditor> ("initial text", 10000, true, comp, doc)
        {}

        void setText (const String& newText) override
        {
            document.perform (new TextEditorInitialTextChangeAction (component, *document.getComponentLayout(), newText),
                              "Change TextEditor initial text");
        }

        String getText() const override
        {
            return component->getProperties() ["initialText"];
        }

    private:
        class TextEditorInitialTextChangeAction  : public ComponentUndoableAction <TextEditor>
        {
        public:
            TextEditorInitialTextChangeAction (TextEditor* const comp, ComponentLayout& l, const String& newState_)
                : ComponentUndoableAction <TextEditor> (comp, l),
                  newState (newState_)
            {
                oldState = comp->getProperties() ["initialText"];
            }

            bool perform()
            {
                showCorrectTab();
                getComponent()->setText (newState, false);
                getComponent()->getProperties().set ("initialText", newState);
                changed();
                return true;
            }

            bool undo()
            {
                showCorrectTab();
                getComponent()->setText (oldState, false);
                getComponent()->getProperties().set ("initialText", oldState);
                changed();
                return true;
            }

            String newState, oldState;
        };
    };
};

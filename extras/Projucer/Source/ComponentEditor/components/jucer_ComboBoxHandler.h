/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

class ComboBoxHandler  : public ComponentTypeHandler
{
public:
    ComboBoxHandler()
        : ComponentTypeHandler ("Combo Box", "ComboBox", typeid (ComboBox), 150, 24)
    {}

    Component* createNewComponent (JucerDocument*) override
    {
        return new ComboBox ("new combo box");
    }

    XmlElement* createXmlFor (Component* comp, const ComponentLayout* layout) override
    {
        ComboBox* const c = dynamic_cast<ComboBox*> (comp);
        jassert (c != nullptr);

        XmlElement* e = ComponentTypeHandler::createXmlFor (comp, layout);

        e->setAttribute ("editable", c->isTextEditable());
        e->setAttribute ("layout", c->getJustificationType().getFlags());
        e->setAttribute ("items", c->getProperties() ["items"].toString());
        e->setAttribute ("textWhenNonSelected", c->getTextWhenNothingSelected());
        e->setAttribute ("textWhenNoItems", c->getTextWhenNoChoicesAvailable());

        return e;
    }

    bool restoreFromXml (const XmlElement& xml, Component* comp, const ComponentLayout* layout) override
    {
        if (! ComponentTypeHandler::restoreFromXml (xml, comp, layout))
            return false;

        ComboBox defaultBox;

        ComboBox* const c = dynamic_cast<ComboBox*> (comp);
        jassert (c != nullptr);

        c->setEditableText (xml.getBoolAttribute ("editable", defaultBox.isTextEditable()));
        c->setJustificationType (Justification (xml.getIntAttribute ("layout", defaultBox.getJustificationType().getFlags())));
        c->getProperties().set ("items", xml.getStringAttribute ("items", String()));
        c->setTextWhenNothingSelected (xml.getStringAttribute ("textWhenNonSelected", defaultBox.getTextWhenNothingSelected()));
        c->setTextWhenNoChoicesAvailable (xml.getStringAttribute ("textWhenNoItems", defaultBox.getTextWhenNoChoicesAvailable()));

        updateItems (c);

        return true;
    }

    void getEditableProperties (Component* component, JucerDocument& document,
                                Array<PropertyComponent*>& props, bool multipleSelected) override
    {
        ComponentTypeHandler::getEditableProperties (component, document, props, multipleSelected);

        if (multipleSelected)
            return;

        if (auto* c = dynamic_cast<ComboBox*> (component))
        {
            props.add (new ComboItemsProperty (c, document));
            props.add (new ComboEditableProperty (c, document));
            props.add (new ComboJustificationProperty (c, document));
            props.add (new ComboTextWhenNoneSelectedProperty (c, document));
            props.add (new ComboTextWhenNoItemsProperty (c, document));
        }
    }

    String getCreationParameters (GeneratedCode&, Component* component) override
    {
        return quotedString (component->getName(), false);
    }

    void fillInCreationCode (GeneratedCode& code, Component* component, const String& memberVariableName) override
    {
        ComponentTypeHandler::fillInCreationCode (code, component, memberVariableName);

        ComboBox* const c = dynamic_cast<ComboBox*> (component);
        jassert (c != nullptr);

        String s;
        s << memberVariableName << "->setEditableText (" << CodeHelpers::boolLiteral (c->isTextEditable()) << ");\n"
          << memberVariableName << "->setJustificationType (" << CodeHelpers::justificationToCode (c->getJustificationType()) << ");\n"
          << memberVariableName << "->setTextWhenNothingSelected (" << quotedString (c->getTextWhenNothingSelected(), code.shouldUseTransMacro()) << ");\n"
          << memberVariableName << "->setTextWhenNoChoicesAvailable (" << quotedString (c->getTextWhenNoChoicesAvailable(), code.shouldUseTransMacro()) << ");\n";

        StringArray lines;
        lines.addLines (c->getProperties() ["items"].toString());
        int itemId = 1;

        for (int i = 0; i < lines.size(); ++i)
        {
            if (lines[i].trim().isEmpty())
                s << memberVariableName << "->addSeparator();\n";
            else
                s << memberVariableName << "->addItem ("
                  << quotedString (lines[i], code.shouldUseTransMacro()) << ", " << itemId++ << ");\n";
        }

        if (needsCallback (component))
            s << memberVariableName << "->addListener (this);\n";

        s << '\n';

        code.constructorCode += s;
    }

    void fillInGeneratedCode (Component* component, GeneratedCode& code) override
    {
        ComponentTypeHandler::fillInGeneratedCode (component, code);

        if (needsCallback (component))
        {
            String& callback = code.getCallbackCode ("public ComboBox::Listener",
                                                     "void",
                                                     "comboBoxChanged (ComboBox* comboBoxThatHasChanged)",
                                                     true);

            if (callback.trim().isNotEmpty())
                callback << "else ";

            const String memberVariableName (code.document->getComponentLayout()->getComponentMemberVariableName (component));
            const String userCodeComment ("UserComboBoxCode_" + memberVariableName);

            callback
                << "if (comboBoxThatHasChanged == " << memberVariableName
                << ")\n{\n    //[" << userCodeComment << "] -- add your combo box handling code here..\n    //[/" << userCodeComment << "]\n}\n";
        }
    }

    static void updateItems (ComboBox* c)
    {
        StringArray lines;
        lines.addLines (c->getProperties() ["items"].toString());

        c->clear();
        int itemId = 1;

        for (int i = 0; i < lines.size(); ++i)
        {
            if (lines[i].trim().isEmpty())
                c->addSeparator();
            else
                c->addItem (lines[i], itemId++);
        }
    }

    static bool needsCallback (Component*)
    {
        return true; // xxx should be configurable
    }

private:
    class ComboEditableProperty  : public ComponentBooleanProperty <ComboBox>
    {
    public:
        ComboEditableProperty (ComboBox* comp, JucerDocument& doc)
            : ComponentBooleanProperty <ComboBox> ("editable", "Text is editable", "Text is editable", comp, doc)
        {
        }

        void setState (bool newState)
        {
            document.perform (new ComboEditableChangeAction (component, *document.getComponentLayout(), newState),
                              "Change combo box editability");
        }

        bool getState() const
        {
            return component->isTextEditable();
        }

    private:
        class ComboEditableChangeAction  : public ComponentUndoableAction <ComboBox>
        {
        public:
            ComboEditableChangeAction (ComboBox* const comp, ComponentLayout& l, const bool newState_)
                : ComponentUndoableAction <ComboBox> (comp, l),
                  newState (newState_)
            {
                oldState = comp->isTextEditable();
            }

            bool perform()
            {
                showCorrectTab();
                getComponent()->setEditableText (newState);
                changed();
                return true;
            }

            bool undo()
            {
                showCorrectTab();
                getComponent()->setEditableText (oldState);
                changed();
                return true;
            }

            bool newState, oldState;
        };
    };

    //==============================================================================
    class ComboJustificationProperty  : public JustificationProperty
    {
    public:
        ComboJustificationProperty (ComboBox* comp, JucerDocument& doc)
            : JustificationProperty ("text layout", false),
              component (comp),
              document (doc)
        {
        }

        void setJustification (Justification newJustification)
        {
            document.perform (new ComboJustifyChangeAction (component, *document.getComponentLayout(), newJustification),
                              "Change combo box justification");
        }

        Justification getJustification() const        { return component->getJustificationType(); }

    private:
        ComboBox* const component;
        JucerDocument& document;

        class ComboJustifyChangeAction  : public ComponentUndoableAction <ComboBox>
        {
        public:
            ComboJustifyChangeAction (ComboBox* const comp, ComponentLayout& l, Justification newState_)
                : ComponentUndoableAction <ComboBox> (comp, l),
                  newState (newState_),
                  oldState (comp->getJustificationType())
            {
            }

            bool perform()
            {
                showCorrectTab();
                getComponent()->setJustificationType (newState);
                changed();
                return true;
            }

            bool undo()
            {
                showCorrectTab();
                getComponent()->setJustificationType (oldState);
                changed();
                return true;
            }

            Justification newState, oldState;
        };
    };

    //==============================================================================
    class ComboItemsProperty  : public ComponentTextProperty <ComboBox>
    {
    public:
        ComboItemsProperty (ComboBox* comp, JucerDocument& doc)
            : ComponentTextProperty <ComboBox> ("items", 10000, true, comp, doc)
        {}

        void setText (const String& newText) override
        {
            document.perform (new ComboItemsChangeAction (component, *document.getComponentLayout(), newText),
                              "Change combo box items");
        }

        String getText() const override
        {
            return component->getProperties() ["items"];
        }

    private:
        class ComboItemsChangeAction  : public ComponentUndoableAction <ComboBox>
        {
        public:
            ComboItemsChangeAction (ComboBox* const comp, ComponentLayout& l, const String& newState_)
                : ComponentUndoableAction <ComboBox> (comp, l),
                  newState (newState_)
            {
                oldState = comp->getProperties() ["items"];
            }

            bool perform()
            {
                showCorrectTab();
                getComponent()->getProperties().set ("items", newState);
                ComboBoxHandler::updateItems (getComponent());
                changed();
                return true;
            }

            bool undo()
            {
                showCorrectTab();
                getComponent()->getProperties().set ("items", oldState);
                ComboBoxHandler::updateItems (getComponent());
                changed();
                return true;
            }

            String newState, oldState;
        };
    };

    //==============================================================================
    class ComboTextWhenNoneSelectedProperty  : public ComponentTextProperty <ComboBox>
    {
    public:
        ComboTextWhenNoneSelectedProperty (ComboBox* comp, JucerDocument& doc)
            : ComponentTextProperty <ComboBox> ("text when none selected", 200, false, comp, doc)
        {}

        void setText (const String& newText) override
        {
            document.perform (new ComboNonSelTextChangeAction (component, *document.getComponentLayout(), newText),
                              "Change combo box text when nothing selected");
        }

        String getText() const override
        {
            return component->getTextWhenNothingSelected();
        }

    private:
        class ComboNonSelTextChangeAction  : public ComponentUndoableAction <ComboBox>
        {
        public:
            ComboNonSelTextChangeAction (ComboBox* const comp, ComponentLayout& l, const String& newState_)
                : ComponentUndoableAction <ComboBox> (comp, l),
                  newState (newState_)
            {
                oldState = comp->getTextWhenNothingSelected();
            }

            bool perform()
            {
                showCorrectTab();
                getComponent()->setTextWhenNothingSelected (newState);
                changed();
                return true;
            }

            bool undo()
            {
                showCorrectTab();
                getComponent()->setTextWhenNothingSelected (oldState);
                changed();
                return true;
            }

            String newState, oldState;
        };
    };

    //==============================================================================
    class ComboTextWhenNoItemsProperty  : public ComponentTextProperty <ComboBox>
    {
    public:
        ComboTextWhenNoItemsProperty (ComboBox* comp, JucerDocument& doc)
            : ComponentTextProperty <ComboBox> ("text when no items", 200, false, comp, doc)
        {}

        void setText (const String& newText) override
        {
            document.perform (new ComboNoItemTextChangeAction (component, *document.getComponentLayout(), newText),
                              "Change combo box 'no items' text");
        }

        String getText() const override
        {
            return component->getTextWhenNoChoicesAvailable();
        }

    private:
        class ComboNoItemTextChangeAction  : public ComponentUndoableAction <ComboBox>
        {
        public:
            ComboNoItemTextChangeAction (ComboBox* const comp, ComponentLayout& l, const String& newState_)
                : ComponentUndoableAction <ComboBox> (comp, l),
                  newState (newState_)
            {
                oldState = comp->getTextWhenNoChoicesAvailable();
            }

            bool perform()
            {
                showCorrectTab();
                getComponent()->setTextWhenNoChoicesAvailable (newState);
                changed();
                return true;
            }

            bool undo()
            {
                showCorrectTab();
                getComponent()->setTextWhenNoChoicesAvailable (oldState);
                changed();
                return true;
            }

            String newState, oldState;
        };
    };
};

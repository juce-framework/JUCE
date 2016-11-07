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

class ComboBoxHandler  : public ComponentTypeHandler
{
public:
    ComboBoxHandler()
        : ComponentTypeHandler ("Combo Box", "ComboBox", typeid (ComboBox), 150, 24)
    {}

    Component* createNewComponent (JucerDocument*)
    {
        return new ComboBox ("new combo box");
    }

    XmlElement* createXmlFor (Component* comp, const ComponentLayout* layout)
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

    bool restoreFromXml (const XmlElement& xml, Component* comp, const ComponentLayout* layout)
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

    void getEditableProperties (Component* component, JucerDocument& document, Array<PropertyComponent*>& props)
    {
        ComponentTypeHandler::getEditableProperties (component, document, props);

        ComboBox* const c = dynamic_cast<ComboBox*> (component);
        jassert (c != nullptr);

        props.add (new ComboItemsProperty (c, document));
        props.add (new ComboEditableProperty (c, document));
        props.add (new ComboJustificationProperty (c, document));
        props.add (new ComboTextWhenNoneSelectedProperty (c, document));
        props.add (new ComboTextWhenNoItemsProperty (c, document));
    }

    String getCreationParameters (GeneratedCode&, Component* component)
    {
        return quotedString (component->getName(), false);
    }

    void fillInCreationCode (GeneratedCode& code, Component* component, const String& memberVariableName)
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

    void fillInGeneratedCode (Component* component, GeneratedCode& code)
    {
        ComponentTypeHandler::fillInGeneratedCode (component, code);

        if (needsCallback (component))
        {
            String& callback = code.getCallbackCode ("public ComboBoxListener",
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

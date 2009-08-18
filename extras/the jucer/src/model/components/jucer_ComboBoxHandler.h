/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

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

#ifndef __JUCER_COMBOBOXHANDLER_JUCEHEADER__
#define __JUCER_COMBOBOXHANDLER_JUCEHEADER__


//==============================================================================
/**
*/
class ComboBoxHandler  : public ComponentTypeHandler
{
public:
    //==============================================================================
    ComboBoxHandler()
        : ComponentTypeHandler ("Combo Box", "ComboBox", typeid (ComboBox), 150, 24)
    {}

    //==============================================================================
    Component* createNewComponent (JucerDocument*)
    {
        return new ComboBox (T("new combo box"));
    }

    XmlElement* createXmlFor (Component* comp, const ComponentLayout* layout)
    {
        ComboBox* const c = dynamic_cast <ComboBox*> (comp);
        jassert (c != 0);

        XmlElement* e = ComponentTypeHandler::createXmlFor (comp, layout);

        e->setAttribute (T("editable"), c->isTextEditable());
        e->setAttribute (T("layout"), c->getJustificationType().getFlags());
        e->setAttribute (T("items"), c->getComponentProperty (T("items"), false));
        e->setAttribute (T("textWhenNonSelected"), c->getTextWhenNothingSelected());
        e->setAttribute (T("textWhenNoItems"), c->getTextWhenNoChoicesAvailable());

        return e;
    }

    bool restoreFromXml (const XmlElement& xml, Component* comp, const ComponentLayout* layout)
    {
        if (! ComponentTypeHandler::restoreFromXml (xml, comp, layout))
            return false;

        ComboBox defaultBox (String::empty);

        ComboBox* const c = dynamic_cast <ComboBox*> (comp);
        jassert (c != 0);

        c->setEditableText (xml.getBoolAttribute (T("editable"), defaultBox.isTextEditable()));
        c->setJustificationType (Justification (xml.getIntAttribute (T("layout"), defaultBox.getJustificationType().getFlags())));
        c->setComponentProperty (T("items"), xml.getStringAttribute (T("items"), String::empty));
        c->setTextWhenNothingSelected (xml.getStringAttribute (T("textWhenNonSelected"), defaultBox.getTextWhenNothingSelected()));
        c->setTextWhenNoChoicesAvailable (xml.getStringAttribute (T("textWhenNoItems"), defaultBox.getTextWhenNoChoicesAvailable()));

        updateItems (c);

        return true;
    }

    void getEditableProperties (Component* component, JucerDocument& document, Array <PropertyComponent*>& properties)
    {
        ComponentTypeHandler::getEditableProperties (component, document, properties);

        ComboBox* const c = dynamic_cast <ComboBox*> (component);
        jassert (c != 0);

        properties.add (new ComboItemsProperty (c, document));
        properties.add (new ComboEditableProperty (c, document));
        properties.add (new ComboJustificationProperty (c, document));
        properties.add (new ComboTextWhenNoneSelectedProperty (c, document));
        properties.add (new ComboTextWhenNoItemsProperty (c, document));
    }

    const String getCreationParameters (Component* component)
    {
        return quotedString (component->getName());
    }

    void fillInCreationCode (GeneratedCode& code, Component* component, const String& memberVariableName)
    {
        ComponentTypeHandler::fillInCreationCode (code, component, memberVariableName);

        ComboBox* const c = dynamic_cast <ComboBox*> (component);
        jassert (c != 0);

        String s;
        s << memberVariableName << "->setEditableText (" << boolToString (c->isTextEditable()) << ");\n"
          << memberVariableName << "->setJustificationType (" << justificationToCode (c->getJustificationType()) << ");\n"
          << memberVariableName << "->setTextWhenNothingSelected (" << quotedString (c->getTextWhenNothingSelected()) << ");\n"
          << memberVariableName << "->setTextWhenNoChoicesAvailable (" << quotedString (c->getTextWhenNoChoicesAvailable()) << ");\n";

        StringArray lines;
        lines.addLines (c->getComponentProperty (T("items"), false));
        int itemId = 1;

        for (int i = 0; i < lines.size(); ++i)
        {
            if (lines[i].trim().isEmpty())
                s << memberVariableName << "->addSeparator();\n";
            else
                s << memberVariableName << "->addItem ("
                  << quotedString (lines[i]) << ", " << itemId++ << ");\n";
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
            String& callback = code.getCallbackCode (T("public ComboBoxListener"),
                                                     T("void"),
                                                     T("comboBoxChanged (ComboBox* comboBoxThatHasChanged)"),
                                                     true);

            if (callback.trim().isNotEmpty())
                callback << "else ";

            const String memberVariableName (code.document->getComponentLayout()->getComponentMemberVariableName (component));
            const String userCodeComment (T("UserComboBoxCode_") + memberVariableName);

            callback
                << "if (comboBoxThatHasChanged == " << memberVariableName
                << ")\n{\n    //[" << userCodeComment << "] -- add your combo box handling code here..\n    //[/" << userCodeComment << "]\n}\n";
        }
    }

    static void updateItems (ComboBox* c)
    {
        StringArray lines;
        lines.addLines (c->getComponentProperty (T("items"), false));

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

    static bool needsCallback (Component* slider)
    {
        return true; // xxx should be configurable
    }

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    //==============================================================================
    class ComboEditableProperty  : public ComponentBooleanProperty <ComboBox>
    {
    public:
        ComboEditableProperty (ComboBox* component_, JucerDocument& document_)
            : ComponentBooleanProperty <ComboBox> (T("editable"), T("Text is editable"), T("Text is editable"), component_, document_)
        {
        }

        //==============================================================================
        void setState (const bool newState)
        {
            document.perform (new ComboEditableChangeAction (component, *document.getComponentLayout(), newState),
                              T("Change combo box editability"));
        }

        bool getState() const
        {
            return component->isTextEditable();
        }

    private:
        class ComboEditableChangeAction  : public ComponentUndoableAction <ComboBox>
        {
        public:
            ComboEditableChangeAction (ComboBox* const comp, ComponentLayout& layout, const bool newState_)
                : ComponentUndoableAction <ComboBox> (comp, layout),
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
        ComboJustificationProperty (ComboBox* component_, JucerDocument& document_)
            : JustificationProperty (T("text layout"), false),
              component (component_),
              document (document_)
        {
        }

        //==============================================================================
        void setJustification (const Justification& newJustification)
        {
            document.perform (new ComboJustifyChangeAction (component, *document.getComponentLayout(), newJustification),
                              T("Change combo box justification"));
        }

        const Justification getJustification() const        { return component->getJustificationType(); }

    private:
        ComboBox* const component;
        JucerDocument& document;

        class ComboJustifyChangeAction  : public ComponentUndoableAction <ComboBox>
        {
        public:
            ComboJustifyChangeAction (ComboBox* const comp, ComponentLayout& layout, const Justification& newState_)
                : ComponentUndoableAction <ComboBox> (comp, layout),
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
        ComboItemsProperty (ComboBox* component_, JucerDocument& document_)
            : ComponentTextProperty <ComboBox> (T("items"), 10000, true, component_, document_)
        {}

        //==============================================================================
        void setText (const String& newText)
        {
            document.perform (new ComboItemsChangeAction (component, *document.getComponentLayout(), newText),
                              T("Change combo box items"));
        }

        const String getText() const
        {
            return component->getComponentProperty (T("items"), false);
        }

    private:
        class ComboItemsChangeAction  : public ComponentUndoableAction <ComboBox>
        {
        public:
            ComboItemsChangeAction (ComboBox* const comp, ComponentLayout& layout, const String& newState_)
                : ComponentUndoableAction <ComboBox> (comp, layout),
                  newState (newState_)
            {
                oldState = comp->getComponentProperty (T("items"), false);
            }

            bool perform()
            {
                showCorrectTab();
                getComponent()->setComponentProperty (T("items"), newState);
                ComboBoxHandler::updateItems (getComponent());
                changed();
                return true;
            }

            bool undo()
            {
                showCorrectTab();
                getComponent()->setComponentProperty (T("items"), oldState);
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
        ComboTextWhenNoneSelectedProperty (ComboBox* component_, JucerDocument& document_)
            : ComponentTextProperty <ComboBox> (T("text when none selected"), 200, false, component_, document_)
        {}

        //==============================================================================
        void setText (const String& newText)
        {
            document.perform (new ComboNonSelTextChangeAction (component, *document.getComponentLayout(), newText),
                              T("Change combo box text when nothing selected"));
        }

        const String getText() const
        {
            return component->getTextWhenNothingSelected();
        }

    private:
        class ComboNonSelTextChangeAction  : public ComponentUndoableAction <ComboBox>
        {
        public:
            ComboNonSelTextChangeAction (ComboBox* const comp, ComponentLayout& layout, const String& newState_)
                : ComponentUndoableAction <ComboBox> (comp, layout),
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
        ComboTextWhenNoItemsProperty (ComboBox* component_, JucerDocument& document_)
            : ComponentTextProperty <ComboBox> (T("text when no items"), 200, false, component_, document_)
        {}

        //==============================================================================
        void setText (const String& newText)
        {
            document.perform (new ComboNoItemTextChangeAction (component, *document.getComponentLayout(), newText),
                              T("Change combo box 'no items' text"));
        }

        const String getText() const
        {
            return component->getTextWhenNoChoicesAvailable();
        }

    private:
        class ComboNoItemTextChangeAction  : public ComponentUndoableAction <ComboBox>
        {
        public:
            ComboNoItemTextChangeAction (ComboBox* const comp, ComponentLayout& layout, const String& newState_)
                : ComponentUndoableAction <ComboBox> (comp, layout),
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


#endif   // __JUCER_COMBOBOXHANDLER_JUCEHEADER__

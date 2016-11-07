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

class GroupComponentHandler  : public ComponentTypeHandler
{
public:
    GroupComponentHandler()
        : ComponentTypeHandler ("Group Box", "GroupComponent", typeid (GroupComponent), 200, 150)
    {
        registerColour (GroupComponent::outlineColourId, "outline", "outlinecol");
        registerColour (GroupComponent::textColourId, "text", "textcol");
    }

    Component* createNewComponent (JucerDocument*)
    {
        return new GroupComponent ("new group", "group");
    }

    XmlElement* createXmlFor (Component* comp, const ComponentLayout* layout)
    {
        GroupComponent* const g = (GroupComponent*) comp;

        XmlElement* e = ComponentTypeHandler::createXmlFor (comp, layout);
        e->setAttribute ("title", g->getText());

        GroupComponent defaultComp;

        if (g->getTextLabelPosition().getFlags() != defaultComp.getTextLabelPosition().getFlags())
            e->setAttribute ("textpos", g->getTextLabelPosition().getFlags());

        return e;
    }

    bool restoreFromXml (const XmlElement& xml, Component* comp, const ComponentLayout* layout)
    {
        GroupComponent* const g = (GroupComponent*) comp;

        if (! ComponentTypeHandler::restoreFromXml (xml, comp, layout))
            return false;

        g->setText (xml.getStringAttribute ("title", g->getText()));
        g->setTextLabelPosition (Justification (xml.getIntAttribute ("textpos", g->getTextLabelPosition().getFlags())));

        return true;
    }

    String getCreationParameters (GeneratedCode& code, Component* component)
    {
        GroupComponent* g = dynamic_cast<GroupComponent*> (component);

        return quotedString (component->getName(), false)
                + ",\n"
                + quotedString (g->getText(), code.shouldUseTransMacro());
    }

    void fillInCreationCode (GeneratedCode& code, Component* component, const String& memberVariableName)
    {
        ComponentTypeHandler::fillInCreationCode (code, component, memberVariableName);

        GroupComponent* const g = dynamic_cast<GroupComponent*> (component);

        String s;

        GroupComponent defaultComp;

        if (g->getTextLabelPosition().getFlags() != defaultComp.getTextLabelPosition().getFlags())
        {
            s << memberVariableName << "->setTextLabelPosition ("
              << CodeHelpers::justificationToCode (g->getTextLabelPosition())
              << ");\n";
        }

        s << getColourIntialisationCode (component, memberVariableName)
          << '\n';

        code.constructorCode += s;
    }

    void getEditableProperties (Component* component, JucerDocument& document, Array<PropertyComponent*>& props)
    {
        ComponentTypeHandler::getEditableProperties (component, document, props);

        props.add (new GroupTitleProperty ((GroupComponent*) component, document));
        props.add (new GroupJustificationProperty ((GroupComponent*) component, document));

        addColourProperties (component, document, props);
    }

private:
    //==============================================================================
    class GroupTitleProperty   : public ComponentTextProperty <GroupComponent>
    {
    public:
        GroupTitleProperty (GroupComponent* comp, JucerDocument& doc)
            : ComponentTextProperty <GroupComponent> ("text", 200, false, comp, doc)
        {}

        void setText (const String& newText) override
        {
            document.perform (new GroupTitleChangeAction (component, *document.getComponentLayout(), newText),
                              "Change group title");
        }

        String getText() const override
        {
            return component->getText();
        }

    private:
        class GroupTitleChangeAction  : public ComponentUndoableAction <GroupComponent>
        {
        public:
            GroupTitleChangeAction (GroupComponent* const comp, ComponentLayout& l, const String& newName_)
                : ComponentUndoableAction <GroupComponent> (comp, l),
                  newName (newName_)
            {
                oldName = comp->getText();
            }

            bool perform()
            {
                showCorrectTab();
                getComponent()->setText (newName);
                changed();
                return true;
            }

            bool undo()
            {
                showCorrectTab();
                getComponent()->setText (oldName);
                changed();
                return true;
            }

            String newName, oldName;
        };
    };

    //==============================================================================
    class GroupJustificationProperty  : public JustificationProperty,
                                        public ChangeListener
    {
    public:
        GroupJustificationProperty (GroupComponent* const group_, JucerDocument& doc)
            : JustificationProperty ("layout", true),
              group (group_),
              document (doc)
        {
            document.addChangeListener (this);
        }

        ~GroupJustificationProperty()
        {
            document.removeChangeListener (this);
        }

        void setJustification (Justification newJustification)
        {
            document.perform (new GroupJustifyChangeAction (group, *document.getComponentLayout(), newJustification),
                              "Change text label position");
        }

        Justification getJustification() const
        {
            return group->getTextLabelPosition();
        }

        void changeListenerCallback (ChangeBroadcaster*)     { refresh(); }

    private:
        GroupComponent* const group;
        JucerDocument& document;

        class GroupJustifyChangeAction  : public ComponentUndoableAction <GroupComponent>
        {
        public:
            GroupJustifyChangeAction (GroupComponent* const comp, ComponentLayout& l, Justification newState_)
                : ComponentUndoableAction <GroupComponent> (comp, l),
                  newState (newState_),
                  oldState (comp->getTextLabelPosition())
            {
            }

            bool perform()
            {
                showCorrectTab();
                getComponent()->setTextLabelPosition (newState);
                changed();
                return true;
            }

            bool undo()
            {
                showCorrectTab();
                getComponent()->setTextLabelPosition (oldState);
                changed();
                return true;
            }

            Justification newState, oldState;
        };
    };
};

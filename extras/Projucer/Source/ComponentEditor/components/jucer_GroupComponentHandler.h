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

class GroupComponentHandler  : public ComponentTypeHandler
{
public:
    GroupComponentHandler()
        : ComponentTypeHandler ("Group Box", "GroupComponent", typeid (GroupComponent), 200, 150)
    {
        registerColour (GroupComponent::outlineColourId, "outline", "outlinecol");
        registerColour (GroupComponent::textColourId, "text", "textcol");
    }

    Component* createNewComponent (JucerDocument*) override
    {
        return new GroupComponent ("new group", "group");
    }

    XmlElement* createXmlFor (Component* comp, const ComponentLayout* layout) override
    {
        GroupComponent* const g = (GroupComponent*) comp;

        XmlElement* e = ComponentTypeHandler::createXmlFor (comp, layout);
        e->setAttribute ("title", g->getText());

        GroupComponent defaultComp;

        if (g->getTextLabelPosition().getFlags() != defaultComp.getTextLabelPosition().getFlags())
            e->setAttribute ("textpos", g->getTextLabelPosition().getFlags());

        return e;
    }

    bool restoreFromXml (const XmlElement& xml, Component* comp, const ComponentLayout* layout) override
    {
        GroupComponent* const g = (GroupComponent*) comp;

        if (! ComponentTypeHandler::restoreFromXml (xml, comp, layout))
            return false;

        g->setText (xml.getStringAttribute ("title", g->getText()));
        g->setTextLabelPosition (Justification (xml.getIntAttribute ("textpos", g->getTextLabelPosition().getFlags())));

        return true;
    }

    String getCreationParameters (GeneratedCode& code, Component* component) override
    {
        GroupComponent* g = dynamic_cast<GroupComponent*> (component);

        return quotedString (component->getName(), false)
                + ",\n"
                + quotedString (g->getText(), code.shouldUseTransMacro());
    }

    void fillInCreationCode (GeneratedCode& code, Component* component, const String& memberVariableName) override
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

    void getEditableProperties (Component* component, JucerDocument& document,
                                Array<PropertyComponent*>& props, bool multipleSelected) override
    {
        ComponentTypeHandler::getEditableProperties (component, document, props, multipleSelected);

        if (multipleSelected)
            return;

        if (auto* gc = dynamic_cast<GroupComponent*> (component))
        {
            props.add (new GroupTitleProperty (gc, document));
            props.add (new GroupJustificationProperty (gc, document));
        }

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

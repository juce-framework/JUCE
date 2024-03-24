/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

#pragma once


//==============================================================================
class GroupComponentHandler  : public ComponentTypeHandler
{
public:
    GroupComponentHandler()
        : ComponentTypeHandler ("Group Box", "juce::GroupComponent", typeid (GroupComponent), 200, 150)
    {
        registerColour (juce::GroupComponent::outlineColourId, "outline", "outlinecol");
        registerColour (juce::GroupComponent::textColourId, "text", "textcol");
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

            bool perform() override
            {
                showCorrectTab();
                getComponent()->setText (newName);
                changed();
                return true;
            }

            bool undo() override
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
                                        private ChangeListener
    {
    public:
        GroupJustificationProperty (GroupComponent* const group_, JucerDocument& doc)
            : JustificationProperty ("layout", true),
              group (group_),
              document (doc)
        {
            document.addChangeListener (this);
        }

        ~GroupJustificationProperty() override
        {
            document.removeChangeListener (this);
        }

        void setJustification (Justification newJustification) override
        {
            document.perform (new GroupJustifyChangeAction (group, *document.getComponentLayout(), newJustification),
                              "Change text label position");
        }

        Justification getJustification() const override
        {
            return group->getTextLabelPosition();
        }

    private:
        void changeListenerCallback (ChangeBroadcaster*) override { refresh(); }

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

            bool perform() override
            {
                showCorrectTab();
                getComponent()->setTextLabelPosition (newState);
                changed();
                return true;
            }

            bool undo() override
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

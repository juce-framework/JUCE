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

#ifndef __JUCER_GROUPCOMPONENTHANDLER_JUCEHEADER__
#define __JUCER_GROUPCOMPONENTHANDLER_JUCEHEADER__


//==============================================================================
/**
*/
class GroupComponentHandler  : public ComponentTypeHandler
{
public:
    //==============================================================================
    GroupComponentHandler()
        : ComponentTypeHandler ("Group Box", "GroupComponent", typeid (GroupComponent), 200, 150)
    {
        registerColour (GroupComponent::outlineColourId, "outline", "outlinecol");
        registerColour (GroupComponent::textColourId, "text", "textcol");
    }

    //==============================================================================
    Component* createNewComponent (JucerDocument*)
    {
        return new GroupComponent (T("new group"), T("group"));
    }

    XmlElement* createXmlFor (Component* comp, const ComponentLayout* layout)
    {
        GroupComponent* const g = (GroupComponent*) comp;

        XmlElement* e = ComponentTypeHandler::createXmlFor (comp, layout);
        e->setAttribute (T("title"), g->getText());

        GroupComponent defaultComp (String::empty, String::empty);

        if (g->getTextLabelPosition().getFlags() != defaultComp.getTextLabelPosition().getFlags())
            e->setAttribute (T("textpos"), g->getTextLabelPosition().getFlags());

        return e;
    }

    bool restoreFromXml (const XmlElement& xml, Component* comp, const ComponentLayout* layout)
    {
        GroupComponent* const g = (GroupComponent*) comp;

        if (! ComponentTypeHandler::restoreFromXml (xml, comp, layout))
            return false;

        g->setText (xml.getStringAttribute (T("title"), g->getText()));
        g->setTextLabelPosition (Justification (xml.getIntAttribute (T("textpos"), g->getTextLabelPosition().getFlags())));

        return true;
    }

    const String getCreationParameters (Component* component)
    {
        GroupComponent* g = dynamic_cast <GroupComponent*> (component);

        return quotedString (component->getName())
                + ",\n"
                + quotedString (g->getText());
    }

    void fillInCreationCode (GeneratedCode& code, Component* component, const String& memberVariableName)
    {
        ComponentTypeHandler::fillInCreationCode (code, component, memberVariableName);

        GroupComponent* const g = dynamic_cast <GroupComponent*> (component);

        String s;

        GroupComponent defaultComp (String::empty, String::empty);

        if (g->getTextLabelPosition().getFlags() != defaultComp.getTextLabelPosition().getFlags())
        {
            s << memberVariableName << "->setTextLabelPosition ("
              << justificationToCode (g->getTextLabelPosition())
              << ");\n";
        }

        s << getColourIntialisationCode (component, memberVariableName)
          << T('\n');

        code.constructorCode += s;
    }

    void getEditableProperties (Component* component, JucerDocument& document, Array <PropertyComponent*>& properties)
    {
        ComponentTypeHandler::getEditableProperties (component, document, properties);

        properties.add (new GroupTitleProperty ((GroupComponent*) component, document));
        properties.add (new GroupJustificationProperty ((GroupComponent*) component, document));

        addColourProperties (component, document, properties);
    }

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    //==============================================================================
    class GroupTitleProperty   : public ComponentTextProperty <GroupComponent>
    {
    public:
        GroupTitleProperty (GroupComponent* component_, JucerDocument& document_)
            : ComponentTextProperty <GroupComponent> (T("text"), 200, false, component_, document_)
        {}

        //==============================================================================
        void setText (const String& newText)
        {
            document.perform (new GroupTitleChangeAction (component, *document.getComponentLayout(), newText),
                              T("Change group title"));
        }

        const String getText() const
        {
            return component->getText();
        }

    private:
        class GroupTitleChangeAction  : public ComponentUndoableAction <GroupComponent>
        {
        public:
            GroupTitleChangeAction (GroupComponent* const comp, ComponentLayout& layout, const String& newName_)
                : ComponentUndoableAction <GroupComponent> (comp, layout),
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
        GroupJustificationProperty (GroupComponent* const group_, JucerDocument& document_)
            : JustificationProperty (T("layout"), true),
              group (group_),
              document (document_)
        {
            document.addChangeListener (this);
        }

        ~GroupJustificationProperty()
        {
            document.removeChangeListener (this);
        }

        void setJustification (const Justification& newJustification)
        {
            document.perform (new GroupJustifyChangeAction (group, *document.getComponentLayout(), newJustification),
                              T("Change text label position"));
        }

        const Justification getJustification() const
        {
            return group->getTextLabelPosition();
        }

        void changeListenerCallback (void*)     { refresh(); }

    private:
        GroupComponent* const group;
        JucerDocument& document;

        class GroupJustifyChangeAction  : public ComponentUndoableAction <GroupComponent>
        {
        public:
            GroupJustifyChangeAction (GroupComponent* const comp, ComponentLayout& layout, const Justification& newState_)
                : ComponentUndoableAction <GroupComponent> (comp, layout),
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


#endif   // __JUCER_GROUPCOMPONENTHANDLER_JUCEHEADER__

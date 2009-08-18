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

#ifndef __JUCER_BUTTONHANDLER_JUCEHEADER__
#define __JUCER_BUTTONHANDLER_JUCEHEADER__

#include "../../properties/jucer_ComponentChoiceProperty.h"
#include "../../properties/jucer_ComponentTextProperty.h"
#include "../../properties/jucer_ComponentBooleanProperty.h"
#include "jucer_ComponentUndoableAction.h"


//==============================================================================
/**
*/
class ButtonHandler  : public ComponentTypeHandler
{
public:
    //==============================================================================
    ButtonHandler (const String& typeDescription_,
                   const String& className_,
                   const std::type_info& componentClass,
                   const int defaultWidth_,
                   const int defaultHeight_)
        : ComponentTypeHandler (typeDescription_, className_, componentClass,
                                defaultWidth_, defaultHeight_)
    {}

    //==============================================================================
    void getEditableProperties (Component* component, JucerDocument& document, Array <PropertyComponent*>& properties)
    {
        ComponentTypeHandler::getEditableProperties (component, document, properties);

        Button* const b = dynamic_cast <Button*> (component);

        properties.add (new ButtonTextProperty (b, document));

        properties.add (new ButtonCallbackProperty (b, document));

        properties.add (new ButtonRadioGroupProperty (b, document));

        properties.add (new ButtonConnectedEdgeProperty (T("connected left"), Button::ConnectedOnLeft, b, document));
        properties.add (new ButtonConnectedEdgeProperty (T("connected right"), Button::ConnectedOnRight, b, document));
        properties.add (new ButtonConnectedEdgeProperty (T("connected top"), Button::ConnectedOnTop, b, document));
        properties.add (new ButtonConnectedEdgeProperty (T("connected bottom"), Button::ConnectedOnBottom, b, document));
    }

    XmlElement* createXmlFor (Component* comp, const ComponentLayout* layout)
    {
        Button* const b = dynamic_cast <Button*> (comp);

        XmlElement* e = ComponentTypeHandler::createXmlFor (comp, layout);
        e->setAttribute (T("buttonText"), b->getButtonText());
        e->setAttribute (T("connectedEdges"), b->getConnectedEdgeFlags());
        e->setAttribute (T("needsCallback"), needsButtonListener (b));
        e->setAttribute (T("radioGroupId"), b->getRadioGroupId());

        return e;
    }

    bool restoreFromXml (const XmlElement& xml, Component* comp, const ComponentLayout* layout)
    {
        Button* const b = dynamic_cast <Button*> (comp);

        if (! ComponentTypeHandler::restoreFromXml (xml, comp, layout))
            return false;

        b->setButtonText (xml.getStringAttribute (T("buttonText"), b->getButtonText()));
        b->setConnectedEdges (xml.getIntAttribute (T("connectedEdges"), 0));
        setNeedsButtonListener (b, xml.getBoolAttribute (T("needsCallback"), true));
        b->setRadioGroupId (xml.getIntAttribute (T("radioGroupId"), 0));

        return true;
    }

    const String getCreationParameters (Component* component)
    {
        return quotedString (component->getName());
    }

    void fillInCreationCode (GeneratedCode& code, Component* component, const String& memberVariableName)
    {
        ComponentTypeHandler::fillInCreationCode (code, component, memberVariableName);

        Button* const b = dynamic_cast <Button*> (component);

        if (b->getButtonText() != b->getName())
        {
            code.constructorCode
              << memberVariableName << "->setButtonText ("
              << quotedString (b->getButtonText()) << ");\n";
        }

        if (b->getConnectedEdgeFlags() != 0)
        {
            StringArray flags;

            if (b->isConnectedOnLeft())
                flags.add (T("Button::ConnectedOnLeft"));

            if (b->isConnectedOnRight())
                flags.add (T("Button::ConnectedOnRight"));

            if (b->isConnectedOnTop())
                flags.add (T("Button::ConnectedOnTop"));

            if (b->isConnectedOnBottom())
                flags.add (T("Button::ConnectedOnBottom"));

            String s;
            s << memberVariableName << "->setConnectedEdges ("
              << flags.joinIntoString (T(" | ")) << ");\n";

            code.constructorCode += s;
        }

        if (b->getRadioGroupId() != 0)
            code.constructorCode << memberVariableName << "->setRadioGroupId ("
                                 << b->getRadioGroupId() << ");\n";

        if (needsButtonListener (component))
            code.constructorCode << memberVariableName << "->addButtonListener (this);\n";
    }

    void fillInGeneratedCode (Component* component, GeneratedCode& code)
    {
        ComponentTypeHandler::fillInGeneratedCode (component, code);

        if (needsButtonListener (component))
        {
            String& callback = code.getCallbackCode (T("public ButtonListener"),
                                                     T("void"),
                                                     T("buttonClicked (Button* buttonThatWasClicked)"),
                                                     true);

            if (callback.isNotEmpty())
                callback << "else ";

            const String memberVariableName (code.document->getComponentLayout()->getComponentMemberVariableName (component));
            const String userCodeComment (T("UserButtonCode_") + memberVariableName);

            callback
                << "if (buttonThatWasClicked == " << memberVariableName
                << ")\n{\n    //[" << userCodeComment << "] -- add your button handler code here..\n    //[/" << userCodeComment << "]\n}\n";
        }
    }

    static bool needsButtonListener (Component* button)
    {
        return button->getComponentPropertyBool (T("generateListenerCallback"), false, true);
    }

    static void setNeedsButtonListener (Component* button, const bool shouldDoCallback)
    {
        button->setComponentProperty (T("generateListenerCallback"), shouldDoCallback);
    }

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    //==============================================================================
    class ButtonTextProperty  : public ComponentTextProperty <Button>
    {
    public:
        ButtonTextProperty (Button* button_, JucerDocument& document_)
            : ComponentTextProperty <Button> (T("text"), 100, false, button_, document_)
        {
        }

        void setText (const String& newText)
        {
            document.perform (new ButtonTextChangeAction (component, *document.getComponentLayout(), newText),
                              T("Change button text"));
        }

        const String getText() const
        {
            return component->getButtonText();
        }

    private:
        class ButtonTextChangeAction  : public ComponentUndoableAction <Button>
        {
        public:
            ButtonTextChangeAction (Button* const comp, ComponentLayout& layout, const String& newName_)
                : ComponentUndoableAction <Button> (comp, layout),
                  newName (newName_)
            {
                oldName = comp->getButtonText();
            }

            bool perform()
            {
                showCorrectTab();
                getComponent()->setButtonText (newName);
                changed();
                return true;
            }

            bool undo()
            {
                showCorrectTab();
                getComponent()->setButtonText (oldName);
                changed();
                return true;
            }

            String newName, oldName;
        };
    };

    class ButtonCallbackProperty    : public ComponentBooleanProperty <Button>
    {
    public:
        ButtonCallbackProperty (Button* button, JucerDocument& document)
            : ComponentBooleanProperty <Button> (T("callback"), T("Generate ButtonListener"), T("Generate ButtonListener"), button, document)
        {
        }

        void setState (const bool newState)
        {
            document.perform (new ButtonCallbackChangeAction (component, *document.getComponentLayout(), newState),
                              T("Change button callback"));
        }

        bool getState() const       { return needsButtonListener (component); }

    private:
        class ButtonCallbackChangeAction  : public ComponentUndoableAction <Button>
        {
        public:
            ButtonCallbackChangeAction (Button* const comp, ComponentLayout& layout, const bool newState_)
                : ComponentUndoableAction <Button> (comp, layout),
                  newState (newState_)
            {
                oldState = needsButtonListener (comp);
            }

            bool perform()
            {
                showCorrectTab();
                setNeedsButtonListener (getComponent(), newState);
                changed();
                return true;
            }

            bool undo()
            {
                showCorrectTab();
                setNeedsButtonListener (getComponent(), oldState);
                changed();
                return true;
            }

            bool newState, oldState;
        };
    };

    class ButtonRadioGroupProperty  : public ComponentTextProperty <Button>
    {
    public:
        ButtonRadioGroupProperty (Button* const button_, JucerDocument& document_)
            : ComponentTextProperty <Button> (T("radio group"), 10, false, button_, document_)
        {
        }

        void setText (const String& newText)
        {
            document.perform (new ButtonRadioGroupChangeAction (component, *document.getComponentLayout(), newText.getIntValue()),
                              T("Change radio group ID"));
        }

        const String getText() const
        {
            return String (component->getRadioGroupId());
        }

    private:
        class ButtonRadioGroupChangeAction  : public ComponentUndoableAction <Button>
        {
        public:
            ButtonRadioGroupChangeAction (Button* const comp, ComponentLayout& layout, const int newId_)
                : ComponentUndoableAction <Button> (comp, layout),
                  newId (newId_)
            {
                oldId = comp->getRadioGroupId();
            }

            bool perform()
            {
                showCorrectTab();
                getComponent()->setRadioGroupId (newId);
                changed();
                return true;
            }

            bool undo()
            {
                showCorrectTab();
                getComponent()->setRadioGroupId (oldId);
                changed();
                return true;
            }

            int newId, oldId;
        };
    };

    class ButtonConnectedEdgeProperty   : public ComponentBooleanProperty <Button>
    {
    public:
        ButtonConnectedEdgeProperty (const String& name, const int flag_,
                                     Button* button, JucerDocument& document)
            : ComponentBooleanProperty <Button> (name, T("Connected"), T("Connected"), button, document),
              flag (flag_)
        {
        }

        void setState (const bool newState)
        {
            document.perform (new ButtonConnectedChangeAction (component, *document.getComponentLayout(), flag, newState),
                              T("Change button connected edges"));
        }

        bool getState() const
        {
            return (component->getConnectedEdgeFlags() & flag) != 0;
        }

    private:
        const int flag;

        class ButtonConnectedChangeAction  : public ComponentUndoableAction <Button>
        {
        public:
            ButtonConnectedChangeAction (Button* const comp, ComponentLayout& layout, const int flag_, const bool newState_)
                : ComponentUndoableAction <Button> (comp, layout),
                  flag (flag_),
                  newState (newState_)
            {
                oldState = ((comp->getConnectedEdgeFlags() & flag) != 0);
            }

            bool perform()
            {
                showCorrectTab();

                if (newState)
                    getComponent()->setConnectedEdges (getComponent()->getConnectedEdgeFlags() | flag);
                else
                    getComponent()->setConnectedEdges (getComponent()->getConnectedEdgeFlags() & ~flag);

                changed();
                return true;
            }

            bool undo()
            {
                showCorrectTab();

                if (oldState)
                    getComponent()->setConnectedEdges (getComponent()->getConnectedEdgeFlags() | flag);
                else
                    getComponent()->setConnectedEdges (getComponent()->getConnectedEdgeFlags() & ~flag);

                changed();
                return true;
            }

            const int flag;
            bool newState, oldState;
        };
    };
};


#endif   // __JUCER_BUTTONHANDLER_JUCEHEADER__

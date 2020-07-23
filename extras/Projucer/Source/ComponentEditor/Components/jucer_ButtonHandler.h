/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#pragma once


//==============================================================================
class ButtonHandler  : public ComponentTypeHandler
{
public:
    ButtonHandler (const String& typeDescription_,
                   const String& className_,
                   const std::type_info& componentClass,
                   const int defaultWidth_,
                   const int defaultHeight_)
        : ComponentTypeHandler (typeDescription_, className_, componentClass,
                                defaultWidth_, defaultHeight_)
    {}

    void getEditableProperties (Component* component, JucerDocument& document,
                                Array<PropertyComponent*>& props, bool multipleSelected) override
    {
        ComponentTypeHandler::getEditableProperties (component, document, props, multipleSelected);

        if (multipleSelected)
            return;

        if (auto* b = dynamic_cast<Button*> (component))
        {
            props.add (new ButtonTextProperty (b, document));

            props.add (new ButtonCallbackProperty (b, document));

            props.add (new ButtonRadioGroupProperty (b, document));

            props.add (new ButtonConnectedEdgeProperty ("connected left", Button::ConnectedOnLeft, b, document));
            props.add (new ButtonConnectedEdgeProperty ("connected right", Button::ConnectedOnRight, b, document));
            props.add (new ButtonConnectedEdgeProperty ("connected top", Button::ConnectedOnTop, b, document));
            props.add (new ButtonConnectedEdgeProperty ("connected bottom", Button::ConnectedOnBottom, b, document));
        }
    }

    XmlElement* createXmlFor (Component* comp, const ComponentLayout* layout) override
    {
        Button* const b = dynamic_cast<Button*> (comp);

        XmlElement* e = ComponentTypeHandler::createXmlFor (comp, layout);
        e->setAttribute ("buttonText", b->getButtonText());
        e->setAttribute ("connectedEdges", b->getConnectedEdgeFlags());
        e->setAttribute ("needsCallback", needsButtonListener (b));
        e->setAttribute ("radioGroupId", b->getRadioGroupId());

        return e;
    }

    bool restoreFromXml (const XmlElement& xml, Component* comp, const ComponentLayout* layout) override
    {
        Button* const b = dynamic_cast<Button*> (comp);

        if (! ComponentTypeHandler::restoreFromXml (xml, comp, layout))
            return false;

        b->setButtonText (xml.getStringAttribute ("buttonText", b->getButtonText()));
        b->setConnectedEdges (xml.getIntAttribute ("connectedEdges", 0));
        setNeedsButtonListener (b, xml.getBoolAttribute ("needsCallback", true));
        b->setRadioGroupId (xml.getIntAttribute ("radioGroupId", 0));

        return true;
    }

    String getCreationParameters (GeneratedCode&, Component* component) override
    {
        return quotedString (component->getName(), false);
    }

    void fillInCreationCode (GeneratedCode& code, Component* component, const String& memberVariableName) override
    {
        ComponentTypeHandler::fillInCreationCode (code, component, memberVariableName);

        Button* const b = dynamic_cast<Button*> (component);

        if (b->getButtonText() != b->getName())
        {
            code.constructorCode
              << memberVariableName << "->setButtonText ("
              << quotedString (b->getButtonText(), code.shouldUseTransMacro()) << ");\n";
        }

        if (b->getConnectedEdgeFlags() != 0)
        {
            StringArray flags;

            if (b->isConnectedOnLeft())
                flags.add ("juce::Button::ConnectedOnLeft");

            if (b->isConnectedOnRight())
                flags.add ("juce::Button::ConnectedOnRight");

            if (b->isConnectedOnTop())
                flags.add ("juce::Button::ConnectedOnTop");

            if (b->isConnectedOnBottom())
                flags.add ("juce::Button::ConnectedOnBottom");

            String s;
            s << memberVariableName << "->setConnectedEdges ("
              << flags.joinIntoString (" | ") << ");\n";

            code.constructorCode += s;
        }

        if (b->getRadioGroupId() != 0)
            code.constructorCode << memberVariableName << "->setRadioGroupId ("
                                 << b->getRadioGroupId() << ");\n";

        if (needsButtonListener (component))
            code.constructorCode << memberVariableName << "->addListener (this);\n";
    }

    void fillInGeneratedCode (Component* component, GeneratedCode& code) override
    {
        ComponentTypeHandler::fillInGeneratedCode (component, code);

        if (needsButtonListener (component))
        {
            String& callback = code.getCallbackCode ("public juce::Button::Listener",
                                                     "void",
                                                     "buttonClicked (juce::Button* buttonThatWasClicked)",
                                                     true);

            if (callback.isNotEmpty())
                callback << "else ";

            const String memberVariableName (code.document->getComponentLayout()->getComponentMemberVariableName (component));
            const String userCodeComment ("UserButtonCode_" + memberVariableName);

            callback
                << "if (buttonThatWasClicked == " << memberVariableName << ".get())\n"
                << "{\n    //[" << userCodeComment << "] -- add your button handler code here..\n    //[/" << userCodeComment << "]\n}\n";
        }
    }

    static bool needsButtonListener (Component* button)
    {
        return button->getProperties().getWithDefault ("generateListenerCallback", true);
    }

    static void setNeedsButtonListener (Component* button, const bool shouldDoCallback)
    {
        button->getProperties().set ("generateListenerCallback", shouldDoCallback);
    }

private:
    //==============================================================================
    class ButtonTextProperty  : public ComponentTextProperty <Button>
    {
    public:
        ButtonTextProperty (Button* button_, JucerDocument& doc)
            : ComponentTextProperty <Button> ("text", 100, false, button_, doc)
        {
        }

        void setText (const String& newText) override
        {
            document.perform (new ButtonTextChangeAction (component, *document.getComponentLayout(), newText),
                              "Change button text");
        }

        String getText() const override
        {
            return component->getButtonText();
        }

    private:
        class ButtonTextChangeAction  : public ComponentUndoableAction <Button>
        {
        public:
            ButtonTextChangeAction (Button* const comp, ComponentLayout& l, const String& newName_)
                : ComponentUndoableAction <Button> (comp, l),
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
        ButtonCallbackProperty (Button* b, JucerDocument& doc)
            : ComponentBooleanProperty <Button> ("callback", "Generate ButtonListener", "Generate ButtonListener", b, doc)
        {
        }

        void setState (bool newState)
        {
            document.perform (new ButtonCallbackChangeAction (component, *document.getComponentLayout(), newState),
                              "Change button callback");
        }

        bool getState() const       { return needsButtonListener (component); }

    private:
        class ButtonCallbackChangeAction  : public ComponentUndoableAction <Button>
        {
        public:
            ButtonCallbackChangeAction (Button* const comp, ComponentLayout& l, const bool newState_)
                : ComponentUndoableAction <Button> (comp, l),
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
        ButtonRadioGroupProperty (Button* const button_, JucerDocument& doc)
            : ComponentTextProperty <Button> ("radio group", 10, false, button_, doc)
        {
        }

        void setText (const String& newText) override
        {
            document.perform (new ButtonRadioGroupChangeAction (component, *document.getComponentLayout(), newText.getIntValue()),
                              "Change radio group ID");
        }

        String getText() const override
        {
            return String (component->getRadioGroupId());
        }

    private:
        class ButtonRadioGroupChangeAction  : public ComponentUndoableAction <Button>
        {
        public:
            ButtonRadioGroupChangeAction (Button* const comp, ComponentLayout& l, const int newId_)
                : ComponentUndoableAction <Button> (comp, l),
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
                                     Button* b, JucerDocument& doc)
            : ComponentBooleanProperty <Button> (name, "Connected", "Connected", b, doc),
              flag (flag_)
        {
        }

        void setState (bool newState)
        {
            document.perform (new ButtonConnectedChangeAction (component, *document.getComponentLayout(), flag, newState),
                              "Change button connected edges");
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
            ButtonConnectedChangeAction (Button* const comp, ComponentLayout& l, const int flag_, const bool newState_)
                : ComponentUndoableAction <Button> (comp, l),
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

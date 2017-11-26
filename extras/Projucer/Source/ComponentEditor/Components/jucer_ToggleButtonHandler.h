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

#pragma once


//==============================================================================
class ToggleButtonHandler  : public ButtonHandler
{
public:
    ToggleButtonHandler()
        : ButtonHandler ("Toggle Button", "ToggleButton", typeid (ToggleButton), 150, 24)
    {
        registerColour (ToggleButton::textColourId, "text colour", "txtcol");
    }

    Component* createNewComponent (JucerDocument*) override
    {
        return new ToggleButton ("new toggle button");
    }

    void getEditableProperties (Component* component, JucerDocument& document,
                                Array<PropertyComponent*>& props, bool multipleSelected) override
    {
        ButtonHandler::getEditableProperties (component, document, props, multipleSelected);

        if (multipleSelected)
            return;

        if (auto* tb = dynamic_cast<ToggleButton*> (component))
            props.add (new ToggleButtonStateProperty (tb, document));

        addColourProperties (component, document, props);
    }

    XmlElement* createXmlFor (Component* comp, const ComponentLayout* layout) override
    {
        ToggleButton* tb = (ToggleButton*) comp;

        XmlElement* e = ButtonHandler::createXmlFor (comp, layout);
        e->setAttribute ("state", tb->getToggleState());

        return e;
    }

    bool restoreFromXml (const XmlElement& xml, Component* comp, const ComponentLayout* layout) override
    {
        ToggleButton* const tb = (ToggleButton*) comp;

        if (! ButtonHandler::restoreFromXml (xml, comp, layout))
            return false;

        tb->setToggleState (xml.getBoolAttribute ("state", false), dontSendNotification);
        return true;
    }

    void fillInCreationCode (GeneratedCode& code, Component* component, const String& memberVariableName) override
    {
        ButtonHandler::fillInCreationCode (code, component, memberVariableName);

        ToggleButton* const tb = dynamic_cast<ToggleButton*> (component);

        String s;

        if (tb->getToggleState())
            s << memberVariableName << "->setToggleState (true, dontSendNotification);\n";

        s << getColourIntialisationCode (component, memberVariableName)
          << '\n';

        code.constructorCode += s;
    }

private:
    class ToggleButtonStateProperty : public ComponentBooleanProperty <ToggleButton>
    {
    public:
        ToggleButtonStateProperty (ToggleButton* button_, JucerDocument& doc)
            : ComponentBooleanProperty <ToggleButton> ("initial state", "on", "off", button_, doc)
        {
        }

        void setState (bool newState)
        {
            document.perform (new ToggleStateChangeAction (component, *document.getComponentLayout(), newState),
                              "Change ToggleButton state");
        }

        bool getState() const
        {
            return component->getToggleState();
        }

    private:
        class ToggleStateChangeAction  : public ComponentUndoableAction <ToggleButton>
        {
        public:
            ToggleStateChangeAction (ToggleButton* const comp, ComponentLayout& l, const bool newState_)
                : ComponentUndoableAction <ToggleButton> (comp, l),
                  newState (newState_)
            {
                oldState = comp->getToggleState();
            }

            bool perform()
            {
                showCorrectTab();
                getComponent()->setToggleState (newState, dontSendNotification);
                changed();
                return true;
            }

            bool undo()
            {
                showCorrectTab();
                getComponent()->setToggleState (oldState, dontSendNotification);
                changed();
                return true;
            }

            bool newState, oldState;
        };
    };
};

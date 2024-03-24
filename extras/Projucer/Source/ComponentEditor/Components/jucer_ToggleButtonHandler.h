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
class ToggleButtonHandler  : public ButtonHandler
{
public:
    ToggleButtonHandler()
        : ButtonHandler ("Toggle Button", "juce::ToggleButton", typeid (ToggleButton), 150, 24)
    {
        registerColour (juce::ToggleButton::textColourId, "text colour", "txtcol");
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
            s << memberVariableName << "->setToggleState (true, juce::dontSendNotification);\n";

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

        void setState (bool newState) override
        {
            document.perform (new ToggleStateChangeAction (component, *document.getComponentLayout(), newState),
                              "Change ToggleButton state");
        }

        bool getState() const override
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

            bool perform() override
            {
                showCorrectTab();
                getComponent()->setToggleState (newState, dontSendNotification);
                changed();
                return true;
            }

            bool undo() override
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

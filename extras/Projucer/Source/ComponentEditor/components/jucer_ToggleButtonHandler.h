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

class ToggleButtonHandler  : public ButtonHandler
{
public:
    ToggleButtonHandler()
        : ButtonHandler ("Toggle Button", "ToggleButton", typeid (ToggleButton), 150, 24)
    {
        registerColour (ToggleButton::textColourId, "text colour", "txtcol");
    }

    Component* createNewComponent (JucerDocument*)
    {
        return new ToggleButton ("new toggle button");
    }

    void getEditableProperties (Component* component, JucerDocument& document, Array<PropertyComponent*>& props)
    {
        ButtonHandler::getEditableProperties (component, document, props);

        props.add (new ToggleButtonStateProperty ((ToggleButton*) component, document));

        addColourProperties (component, document, props);
    }

    XmlElement* createXmlFor (Component* comp, const ComponentLayout* layout)
    {
        ToggleButton* tb = (ToggleButton*) comp;

        XmlElement* e = ButtonHandler::createXmlFor (comp, layout);
        e->setAttribute ("state", tb->getToggleState());

        return e;
    }

    bool restoreFromXml (const XmlElement& xml, Component* comp, const ComponentLayout* layout)
    {
        ToggleButton* const tb = (ToggleButton*) comp;

        if (! ButtonHandler::restoreFromXml (xml, comp, layout))
            return false;

        tb->setToggleState (xml.getBoolAttribute ("state", false), dontSendNotification);
        return true;
    }

    void fillInCreationCode (GeneratedCode& code, Component* component, const String& memberVariableName)
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

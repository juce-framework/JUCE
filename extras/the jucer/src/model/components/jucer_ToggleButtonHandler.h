/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

  ==============================================================================
*/

#ifndef __JUCER_TOGGLEBUTTONHANDLER_JUCEHEADER__
#define __JUCER_TOGGLEBUTTONHANDLER_JUCEHEADER__

#include "jucer_ButtonHandler.h"
#include "../../properties/jucer_ComponentChoiceProperty.h"


//==============================================================================
/**
*/
class ToggleButtonHandler  : public ButtonHandler
{
public:
    //==============================================================================
    ToggleButtonHandler()
        : ButtonHandler ("Toggle Button", "ToggleButton", typeid (ToggleButton), 150, 24)
    {
        registerColour (ToggleButton::textColourId, "text colour", "txtcol");
    }

    //==============================================================================
    Component* createNewComponent (JucerDocument*)
    {
        return new ToggleButton (T("new toggle button"));
    }

    void getEditableProperties (Component* component, JucerDocument& document, Array <PropertyComponent*>& properties)
    {
        ButtonHandler::getEditableProperties (component, document, properties);

        properties.add (new ToggleButtonStateProperty ((ToggleButton*) component, document));

        addColourProperties (component, document, properties);
    }

    XmlElement* createXmlFor (Component* comp, const ComponentLayout* layout)
    {
        ToggleButton* tb = (ToggleButton*) comp;

        XmlElement* e = ButtonHandler::createXmlFor (comp, layout);
        e->setAttribute (T("state"), tb->getToggleState());

        return e;
    }

    bool restoreFromXml (const XmlElement& xml, Component* comp, const ComponentLayout* layout)
    {
        ToggleButton* const tb = (ToggleButton*) comp;

        if (! ButtonHandler::restoreFromXml (xml, comp, layout))
            return false;

        tb->setToggleState (xml.getBoolAttribute (T("state"), false), false);
        return true;
    }

    void fillInCreationCode (GeneratedCode& code, Component* component, const String& memberVariableName)
    {
        ButtonHandler::fillInCreationCode (code, component, memberVariableName);

        ToggleButton* const tb = dynamic_cast <ToggleButton*> (component);

        String s;

        if (tb->getToggleState())
            s << memberVariableName << "->setToggleState (true, false);\n";

        s << getColourIntialisationCode (component, memberVariableName)
          << '\n';

        code.constructorCode += s;
    }

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    class ToggleButtonStateProperty : public ComponentBooleanProperty <ToggleButton>
    {
    public:
        ToggleButtonStateProperty (ToggleButton* button_, JucerDocument& document_)
            : ComponentBooleanProperty <ToggleButton> (T("initial state"), T("on"), T("off"), button_, document_)
        {
        }

        void setState (const bool newState)
        {
            document.perform (new ToggleStateChangeAction (component, *document.getComponentLayout(), newState),
                              T("Change ToggleButton state"));
        }

        bool getState() const
        {
            return component->getToggleState();
        }

    private:
        class ToggleStateChangeAction  : public ComponentUndoableAction <ToggleButton>
        {
        public:
            ToggleStateChangeAction (ToggleButton* const comp, ComponentLayout& layout, const bool newState_)
                : ComponentUndoableAction <ToggleButton> (comp, layout),
                  newState (newState_)
            {
                oldState = comp->getToggleState();
            }

            bool perform()
            {
                showCorrectTab();
                getComponent()->setToggleState (newState, false);
                changed();
                return true;
            }

            bool undo()
            {
                showCorrectTab();
                getComponent()->setToggleState (oldState, false);
                changed();
                return true;
            }

            bool newState, oldState;
        };
    };
};


#endif   // __JUCER_TOGGLEBUTTONHANDLER_JUCEHEADER__

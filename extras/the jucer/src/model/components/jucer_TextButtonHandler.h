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

#ifndef __JUCER_TEXTBUTTONHANDLER_JUCEHEADER__
#define __JUCER_TEXTBUTTONHANDLER_JUCEHEADER__

#include "jucer_ButtonHandler.h"
#include "../../properties/jucer_ComponentColourProperty.h"


//==============================================================================
/**
*/
class TextButtonHandler  : public ButtonHandler
{
public:
    //==============================================================================
    TextButtonHandler()
        : ButtonHandler ("Text Button", "TextButton", typeid (TextButton), 150, 24)
    {
        registerColour (TextButton::buttonColourId, "background (normal)", "bgColOff");
        registerColour (TextButton::buttonOnColourId, "background (on)", "bgColOn");
        registerColour (TextButton::textColourId, "text colour", "textCol");
    }

    //==============================================================================
    Component* createNewComponent (JucerDocument*)
    {
        return new TextButton (T("new button"), String::empty);
    }

    void getEditableProperties (Component* component, JucerDocument& document, Array <PropertyComponent*>& properties)
    {
        ButtonHandler::getEditableProperties (component, document, properties);

        addColourProperties (component, document, properties);
    }

    XmlElement* createXmlFor (Component* comp, const ComponentLayout* layout)
    {
        XmlElement* e = ButtonHandler::createXmlFor (comp, layout);

        //TextButton* tb = (TextButton*) comp;

        return e;
    }

    bool restoreFromXml (const XmlElement& xml, Component* comp, const ComponentLayout* layout)
    {
        if (! ButtonHandler::restoreFromXml (xml, comp, layout))
            return false;

        //TextButton* tb = (TextButton*) comp;

        return true;
    }

    void fillInCreationCode (GeneratedCode& code, Component* component, const String& memberVariableName)
    {
        ButtonHandler::fillInCreationCode (code, component, memberVariableName);

        //TextButton* const tb = dynamic_cast <TextButton*> (component);
        //TextButton defaultButton (String::empty);

        String s;

        s << getColourIntialisationCode (component, memberVariableName)
          << T('\n');

        code.constructorCode += s;
    }

    //==============================================================================
    juce_UseDebuggingNewOperator

private:

};


#endif   // __JUCER_TEXTBUTTONHANDLER_JUCEHEADER__

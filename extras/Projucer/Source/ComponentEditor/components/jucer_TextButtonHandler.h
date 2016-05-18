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

class TextButtonHandler  : public ButtonHandler
{
public:
    TextButtonHandler()
        : ButtonHandler ("Text Button", "TextButton", typeid (TextButton), 150, 24)
    {
        registerColour (TextButton::buttonColourId, "background (normal)", "bgColOff");
        registerColour (TextButton::buttonOnColourId, "background (on)", "bgColOn");
        registerColour (TextButton::textColourOffId, "text colour (normal)", "textCol");
        registerColour (TextButton::textColourOnId, "text colour (on)", "textColOn");
    }

    Component* createNewComponent (JucerDocument*)
    {
        return new TextButton ("new button", String());
    }

    void getEditableProperties (Component* component, JucerDocument& document, Array<PropertyComponent*>& props)
    {
        ButtonHandler::getEditableProperties (component, document, props);
        addColourProperties (component, document, props);
    }

    XmlElement* createXmlFor (Component* comp, const ComponentLayout* layout)
    {
        return ButtonHandler::createXmlFor (comp, layout);
    }

    bool restoreFromXml (const XmlElement& xml, Component* comp, const ComponentLayout* layout)
    {
        return ButtonHandler::restoreFromXml (xml, comp, layout);
    }

    void fillInCreationCode (GeneratedCode& code, Component* component, const String& memberVariableName)
    {
        ButtonHandler::fillInCreationCode (code, component, memberVariableName);

        String s;

        s << getColourIntialisationCode (component, memberVariableName)
          << '\n';

        code.constructorCode += s;
    }
};

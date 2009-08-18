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

#ifndef __JUCER_HYPERLINKBUTTONHANDLER_JUCEHEADER__
#define __JUCER_HYPERLINKBUTTONHANDLER_JUCEHEADER__

#include "jucer_ButtonHandler.h"


//==============================================================================
/**
*/
class HyperlinkButtonHandler  : public ButtonHandler
{
public:
    //==============================================================================
    HyperlinkButtonHandler()
        : ButtonHandler ("Hyperlink Button", "HyperlinkButton", typeid (HyperlinkButton), 150, 24)
    {
        registerColour (HyperlinkButton::textColourId, "text", "textCol");
    }

    //==============================================================================
    Component* createNewComponent (JucerDocument*)
    {
        HyperlinkButton* hb = new HyperlinkButton (T("new hyperlink"), URL (T("http://www.rawmaterialsoftware.com/juce")));

        setNeedsButtonListener (hb, false);
        return hb;
    }

    void getEditableProperties (Component* component, JucerDocument& document, Array <PropertyComponent*>& properties)
    {
        HyperlinkButton* const hb = (HyperlinkButton*) component;

        ButtonHandler::getEditableProperties (component, document, properties);

        properties.add (new HyperlinkURLProperty (hb, document));

        addColourProperties (component, document, properties);
    }

    XmlElement* createXmlFor (Component* comp, const ComponentLayout* layout)
    {
        HyperlinkButton* const hb = (HyperlinkButton*) comp;

        XmlElement* const e = ButtonHandler::createXmlFor (comp, layout);

        e->setAttribute (T("url"), hb->getURL().toString (false));

        return e;
    }

    bool restoreFromXml (const XmlElement& xml, Component* comp, const ComponentLayout* layout)
    {
        HyperlinkButton* const hb = (HyperlinkButton*) comp;

        if (! ButtonHandler::restoreFromXml (xml, comp, layout))
            return false;

        hb->setURL (URL (xml.getStringAttribute (T("url"), hb->getURL().toString (false))));

        return true;
    }

    const String getCreationParameters (Component* comp)
    {
        HyperlinkButton* const hb = dynamic_cast <HyperlinkButton*> (comp);

        return quotedString (hb->getButtonText())
                + ",\nURL ("
                + quotedString (hb->getURL().toString (false))
                + ")";
    }

    void fillInCreationCode (GeneratedCode& code, Component* component, const String& memberVariableName)
    {
        ButtonHandler::fillInCreationCode (code, component, memberVariableName);

        code.constructorCode << getColourIntialisationCode (component, memberVariableName)
                             << T('\n');
    }

    juce_UseDebuggingNewOperator

private:
    //==============================================================================
    class HyperlinkURLProperty  : public ComponentTextProperty <HyperlinkButton>
    {
    public:
        HyperlinkURLProperty (HyperlinkButton* component_, JucerDocument& document_)
            : ComponentTextProperty <HyperlinkButton> (T("URL"), 512, false, component_, document_)
        {}

        //==============================================================================
        void setText (const String& newText)
        {
            document.perform (new HyperlinkURLChangeAction (component, *document.getComponentLayout(), URL (newText)),
                              T("Change hyperlink URL"));
        }

        const String getText() const
        {
            return component->getURL().toString (false);
        }

    private:
        class HyperlinkURLChangeAction  : public ComponentUndoableAction <HyperlinkButton>
        {
        public:
            HyperlinkURLChangeAction (HyperlinkButton* const comp, ComponentLayout& layout, const URL& newState_)
                : ComponentUndoableAction <HyperlinkButton> (comp, layout),
                  newState (newState_)
            {
                oldState = comp->getURL();
            }

            bool perform()
            {
                showCorrectTab();
                getComponent()->setURL (newState);
                changed();
                return true;
            }

            bool undo()
            {
                showCorrectTab();
                getComponent()->setURL (oldState);
                changed();
                return true;
            }

            URL newState, oldState;
        };
    };

};


#endif   // __JUCER_HYPERLINKBUTTONHANDLER_JUCEHEADER__

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

class HyperlinkButtonHandler  : public ButtonHandler
{
public:
    HyperlinkButtonHandler()
        : ButtonHandler ("Hyperlink Button", "HyperlinkButton", typeid (HyperlinkButton), 150, 24)
    {
        registerColour (HyperlinkButton::textColourId, "text", "textCol");
    }

    Component* createNewComponent (JucerDocument*)
    {
        HyperlinkButton* hb = new HyperlinkButton ("new hyperlink", URL ("http://www.juce.com"));

        setNeedsButtonListener (hb, false);
        return hb;
    }

    void getEditableProperties (Component* component, JucerDocument& document, Array<PropertyComponent*>& props)
    {
        HyperlinkButton* const hb = (HyperlinkButton*) component;
        ButtonHandler::getEditableProperties (component, document, props);
        props.add (new HyperlinkURLProperty (hb, document));
        addColourProperties (component, document, props);
    }

    XmlElement* createXmlFor (Component* comp, const ComponentLayout* layout)
    {
        HyperlinkButton* const hb = (HyperlinkButton*) comp;
        XmlElement* const e = ButtonHandler::createXmlFor (comp, layout);
        e->setAttribute ("url", hb->getURL().toString (false));
        return e;
    }

    bool restoreFromXml (const XmlElement& xml, Component* comp, const ComponentLayout* layout)
    {
        HyperlinkButton* const hb = (HyperlinkButton*) comp;

        if (! ButtonHandler::restoreFromXml (xml, comp, layout))
            return false;

        hb->setURL (URL (xml.getStringAttribute ("url", hb->getURL().toString (false))));

        return true;
    }

    String getCreationParameters (GeneratedCode& code, Component* comp)
    {
        HyperlinkButton* const hb = dynamic_cast<HyperlinkButton*> (comp);

        return quotedString (hb->getButtonText(), code.shouldUseTransMacro())
                + ",\nURL ("
                + quotedString (hb->getURL().toString (false), false)
                + ")";
    }

    void fillInCreationCode (GeneratedCode& code, Component* component, const String& memberVariableName)
    {
        ButtonHandler::fillInCreationCode (code, component, memberVariableName);

        code.constructorCode << getColourIntialisationCode (component, memberVariableName)
                             << '\n';
    }

private:
    //==============================================================================
    class HyperlinkURLProperty  : public ComponentTextProperty <HyperlinkButton>
    {
    public:
        HyperlinkURLProperty (HyperlinkButton* comp, JucerDocument& doc)
            : ComponentTextProperty <HyperlinkButton> ("URL", 512, false, comp, doc)
        {}

        void setText (const String& newText) override
        {
            document.perform (new HyperlinkURLChangeAction (component, *document.getComponentLayout(), URL (newText)),
                              "Change hyperlink URL");
        }

        String getText() const override
        {
            return component->getURL().toString (false);
        }

    private:
        class HyperlinkURLChangeAction  : public ComponentUndoableAction <HyperlinkButton>
        {
        public:
            HyperlinkURLChangeAction (HyperlinkButton* const comp, ComponentLayout& l, const URL& newState_)
                : ComponentUndoableAction <HyperlinkButton> (comp, l),
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

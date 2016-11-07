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

class LabelHandler  : public ComponentTypeHandler
{
public:
    LabelHandler()
        : ComponentTypeHandler ("Label", "Label", typeid (Label), 150, 24)
    {
        registerColour (Label::backgroundColourId, "background", "bkgCol");
        registerColour (Label::textColourId, "text", "textCol");
        registerColour (Label::outlineColourId, "outline", "outlineCol");
        registerColour (TextEditor::textColourId, "editor text", "edTextCol");
        registerColour (TextEditor::backgroundColourId, "editor bkg", "edBkgCol");
        registerColour (TextEditor::highlightColourId, "highlight", "hiliteCol");
    }

    Component* createNewComponent (JucerDocument*)
    {
        return new Label ("new label", "label text");
    }

    XmlElement* createXmlFor (Component* comp, const ComponentLayout* layout)
    {
        Label* const l = dynamic_cast<Label*> (comp);

        XmlElement* e = ComponentTypeHandler::createXmlFor (comp, layout);
        e->setAttribute ("labelText", l->getText());

        e->setAttribute ("editableSingleClick", l->isEditableOnSingleClick());
        e->setAttribute ("editableDoubleClick", l->isEditableOnDoubleClick());
        e->setAttribute ("focusDiscardsChanges", l->doesLossOfFocusDiscardChanges());

        e->setAttribute ("fontname", l->getProperties().getWithDefault ("typefaceName", FontPropertyComponent::getDefaultFont()).toString());
        e->setAttribute ("fontsize", roundToInt (l->getFont().getHeight() * 100.0) / 100.0);
        e->setAttribute ("bold", l->getFont().isBold());
        e->setAttribute ("italic", l->getFont().isItalic());
        e->setAttribute ("justification", l->getJustificationType().getFlags());

        return e;
    }

    bool restoreFromXml (const XmlElement& xml, Component* comp, const ComponentLayout* layout)
    {
        Label* const l = dynamic_cast<Label*> (comp);

        if (! ComponentTypeHandler::restoreFromXml (xml, comp, layout))
            return false;

        Label defaultLabel;

        Font font;
        font.setHeight ((float) xml.getDoubleAttribute ("fontsize", 15.0));
        font.setBold (xml.getBoolAttribute ("bold", false));
        font.setItalic (xml.getBoolAttribute ("italic", false));
        l->setFont (font);

        l->getProperties().set ("typefaceName", xml.getStringAttribute ("fontname", FontPropertyComponent::getDefaultFont()));
        updateLabelFont (l);

        l->setJustificationType (Justification (xml.getIntAttribute ("justification", Justification::centred)));

        l->setText (xml.getStringAttribute ("labelText", "Label Text"), dontSendNotification);

        l->setEditable (xml.getBoolAttribute ("editableSingleClick", defaultLabel.isEditableOnSingleClick()),
                        xml.getBoolAttribute ("editableDoubleClick", defaultLabel.isEditableOnDoubleClick()),
                        xml.getBoolAttribute ("focusDiscardsChanges", defaultLabel.doesLossOfFocusDiscardChanges()));

        return true;
    }

    static void updateLabelFont (Label* label)
    {
        Font f (label->getFont());
        f = FontPropertyComponent::applyNameToFont (label->getProperties().getWithDefault ("typefaceName", FontPropertyComponent::getDefaultFont()), f);
        label->setFont (f);
    }

    String getCreationParameters (GeneratedCode& code, Component* component)
    {
        Label* const l = dynamic_cast<Label*> (component);

        return quotedString (component->getName(), false)
                 + ",\n"
                 + quotedString (l->getText(), code.shouldUseTransMacro());
    }

    void fillInCreationCode (GeneratedCode& code, Component* component, const String& memberVariableName)
    {
        ComponentTypeHandler::fillInCreationCode (code, component, memberVariableName);

        Label* const l = dynamic_cast<Label*> (component);

        String s;

        s << memberVariableName << "->setFont ("
          << FontPropertyComponent::getCompleteFontCode (l->getFont(), l->getProperties().getWithDefault ("typefaceName", FontPropertyComponent::getDefaultFont()))
          << ");\n"
          << memberVariableName << "->setJustificationType ("
          << CodeHelpers::justificationToCode (l->getJustificationType())
          << ");\n"
          << memberVariableName << "->setEditable ("
          << CodeHelpers::boolLiteral (l->isEditableOnSingleClick()) << ", "
          << CodeHelpers::boolLiteral (l->isEditableOnDoubleClick()) << ", "
          << CodeHelpers::boolLiteral (l->doesLossOfFocusDiscardChanges()) << ");\n"
          << getColourIntialisationCode (component, memberVariableName);

        if (needsCallback (component))
            s << memberVariableName << "->addListener (this);\n";

        s << '\n';

        code.constructorCode += s;
    }

    void fillInGeneratedCode (Component* component, GeneratedCode& code)
    {
        ComponentTypeHandler::fillInGeneratedCode (component, code);

        if (needsCallback (component))
        {
            String& callback = code.getCallbackCode ("public LabelListener",
                                                     "void",
                                                     "labelTextChanged (Label* labelThatHasChanged)",
                                                     true);

            if (callback.trim().isNotEmpty())
                callback << "else ";

            const String memberVariableName (code.document->getComponentLayout()->getComponentMemberVariableName (component));
            const String userCodeComment ("UserLabelCode_" + memberVariableName);

            callback
                << "if (labelThatHasChanged == " << memberVariableName
                << ")\n{\n    //[" << userCodeComment << "] -- add your label text handling code here..\n    //[/" << userCodeComment << "]\n}\n";
        }
    }

    void getEditableProperties (Component* component, JucerDocument& document, Array<PropertyComponent*>& props)
    {
        ComponentTypeHandler::getEditableProperties (component, document, props);

        Label* const l = dynamic_cast<Label*> (component);
        props.add (new LabelTextProperty (l, document));

        props.add (new LabelJustificationProperty (l, document));
        props.add (new FontNameProperty (l, document));
        props.add (new FontSizeProperty (l, document));
        props.add (new FontStyleProperty (l, document));

        addColourProperties (component, document, props);

        props.add (new LabelEditableProperty (l, document));

        if (l->isEditableOnDoubleClick() || l->isEditableOnSingleClick())
            props.add (new LabelLossOfFocusProperty (l, document));
    }

    static bool needsCallback (Component* label)
    {
        return ((Label*) label)->isEditableOnSingleClick()
                 || ((Label*) label)->isEditableOnDoubleClick(); // xxx should be configurable
    }

private:

    //==============================================================================
    class LabelTextProperty  : public ComponentTextProperty <Label>
    {
    public:
        LabelTextProperty (Label* comp, JucerDocument& doc)
            : ComponentTextProperty <Label> ("text", 10000, true, comp, doc)
        {}

        void setText (const String& newText) override
        {
            document.perform (new LabelTextChangeAction (component, *document.getComponentLayout(), newText),
                              "Change Label text");
        }

        String getText() const override
        {
            return component->getText();
        }

    private:
        class LabelTextChangeAction  : public ComponentUndoableAction <Label>
        {
        public:
            LabelTextChangeAction (Label* const comp, ComponentLayout& l, const String& newState_)
                : ComponentUndoableAction <Label> (comp, l),
                  newState (newState_)
            {
                oldState = comp->getText();
            }

            bool perform()
            {
                showCorrectTab();
                getComponent()->setText (newState, dontSendNotification);
                changed();
                return true;
            }

            bool undo()
            {
                showCorrectTab();
                getComponent()->setText (oldState, dontSendNotification);
                changed();
                return true;
            }

            String newState, oldState;
        };
    };

    //==============================================================================
    class LabelEditableProperty   : public ComponentChoiceProperty <Label>
    {
    public:
        LabelEditableProperty (Label* comp, JucerDocument& doc)
           : ComponentChoiceProperty <Label> ("editing", comp, doc)
        {
            choices.add ("read-only");
            choices.add ("edit on single-click");
            choices.add ("edit on double-click");
        }

        void setIndex (int newIndex)
        {
            document.perform (new LabelEditableChangeAction (component, *document.getComponentLayout(), newIndex),
                              "Change Label editability");
        }

        int getIndex() const
        {
            return component->isEditableOnSingleClick()
                    ? 1
                    : (component->isEditableOnDoubleClick() ? 2 : 0);
        }

    private:
        class LabelEditableChangeAction  : public ComponentUndoableAction <Label>
        {
        public:
            LabelEditableChangeAction (Label* const comp, ComponentLayout& l, const int newState_)
                : ComponentUndoableAction <Label> (comp, l),
                  newState (newState_)
            {
                oldState = comp->isEditableOnSingleClick()
                            ? 1
                            : (comp->isEditableOnDoubleClick() ? 2 : 0);
            }

            bool perform()
            {
                showCorrectTab();
                getComponent()->setEditable (newState == 1, newState >= 1, getComponent()->doesLossOfFocusDiscardChanges());
                changed();
                layout.getSelectedSet().changed();
                return true;
            }

            bool undo()
            {
                showCorrectTab();
                getComponent()->setEditable (oldState == 1, oldState >= 1, getComponent()->doesLossOfFocusDiscardChanges());
                changed();
                layout.getSelectedSet().changed();
                return true;
            }

            int newState, oldState;
        };
    };

    //==============================================================================
    class LabelLossOfFocusProperty   : public ComponentChoiceProperty <Label>
    {
    public:
        LabelLossOfFocusProperty (Label* comp, JucerDocument& doc)
           : ComponentChoiceProperty <Label> ("focus", comp, doc)
        {
            choices.add ("loss of focus discards changes");
            choices.add ("loss of focus commits changes");
        }

        void setIndex (int newIndex)
        {
            document.perform (new LabelFocusLossChangeAction (component, *document.getComponentLayout(), newIndex == 0),
                              "Change Label focus behaviour");
        }

        int getIndex() const
        {
            return component->doesLossOfFocusDiscardChanges() ? 0 : 1;
        }

    private:
        class LabelFocusLossChangeAction  : public ComponentUndoableAction <Label>
        {
        public:
            LabelFocusLossChangeAction (Label* const comp, ComponentLayout& l, const bool newState_)
                : ComponentUndoableAction <Label> (comp, l),
                  newState (newState_)
            {
                oldState = comp->doesLossOfFocusDiscardChanges();
            }

            bool perform()
            {
                showCorrectTab();
                getComponent()->setEditable (getComponent()->isEditableOnSingleClick(),
                                             getComponent()->isEditableOnDoubleClick(),
                                             newState);
                changed();
                return true;
            }

            bool undo()
            {
                showCorrectTab();
                getComponent()->setEditable (getComponent()->isEditableOnSingleClick(),
                                             getComponent()->isEditableOnDoubleClick(),
                                             oldState);
                changed();
                return true;
            }

            bool newState, oldState;
        };
    };

    //==============================================================================
    class LabelJustificationProperty  : public JustificationProperty,
                                        public ChangeListener
    {
    public:
        LabelJustificationProperty (Label* const label_, JucerDocument& doc)
            : JustificationProperty ("layout", false),
              label (label_),
              document (doc)
        {
            document.addChangeListener (this);
        }

        ~LabelJustificationProperty()
        {
            document.removeChangeListener (this);
        }

        void setJustification (Justification newJustification)
        {
            document.perform (new LabelJustifyChangeAction (label, *document.getComponentLayout(), newJustification),
                              "Change Label justification");
        }

        Justification getJustification() const
        {
            return label->getJustificationType();
        }

        void changeListenerCallback (ChangeBroadcaster*)     { refresh(); }

    private:
        Label* const label;
        JucerDocument& document;

        class LabelJustifyChangeAction  : public ComponentUndoableAction <Label>
        {
        public:
            LabelJustifyChangeAction (Label* const comp, ComponentLayout& l, Justification newState_)
                : ComponentUndoableAction <Label> (comp, l),
                  newState (newState_),
                  oldState (comp->getJustificationType())
            {
            }

            bool perform()
            {
                showCorrectTab();
                getComponent()->setJustificationType (newState);
                changed();
                return true;
            }

            bool undo()
            {
                showCorrectTab();
                getComponent()->setJustificationType (oldState);
                changed();
                return true;
            }

            Justification newState, oldState;
        };
    };

    //==============================================================================
    class FontNameProperty  : public FontPropertyComponent,
                              public ChangeListener
    {
    public:
        FontNameProperty (Label* const label_, JucerDocument& doc)
            : FontPropertyComponent ("font"),
              label (label_),
              document (doc)
        {
            document.addChangeListener (this);
        }

        ~FontNameProperty()
        {
            document.removeChangeListener (this);
        }

        void setTypefaceName (const String& newFontName)
        {
            document.perform (new FontNameChangeAction (label, *document.getComponentLayout(), newFontName),
                              "Change Label typeface");
        }

        String getTypefaceName() const
        {
            return label->getProperties().getWithDefault ("typefaceName", FontPropertyComponent::getDefaultFont());
        }

        void changeListenerCallback (ChangeBroadcaster*)                     { refresh(); }

    private:
        Label* const label;
        JucerDocument& document;

        class FontNameChangeAction  : public ComponentUndoableAction <Label>
        {
        public:
            FontNameChangeAction (Label* const comp, ComponentLayout& l, const String& newState_)
                : ComponentUndoableAction <Label> (comp, l),
                  newState (newState_)
            {
                oldState = comp->getProperties().getWithDefault ("typefaceName", FontPropertyComponent::getDefaultFont());
            }

            bool perform()
            {
                showCorrectTab();
                getComponent()->getProperties().set ("typefaceName", newState);
                LabelHandler::updateLabelFont (getComponent());
                changed();
                return true;
            }

            bool undo()
            {
                showCorrectTab();
                getComponent()->getProperties().set ("typefaceName", oldState);
                LabelHandler::updateLabelFont (getComponent());
                changed();
                return true;
            }

            String newState, oldState;
        };
    };

    //==============================================================================
    class FontSizeProperty  : public SliderPropertyComponent,
                              public ChangeListener
    {
    public:
        FontSizeProperty (Label* const label_, JucerDocument& doc)
            : SliderPropertyComponent ("size", 1.0, 250.0, 0.1, 0.3),
              label (label_),
              document (doc)
        {
            document.addChangeListener (this);
        }

        ~FontSizeProperty()
        {
            document.removeChangeListener (this);
        }

        void setValue (double newValue)
        {
            document.getUndoManager().undoCurrentTransactionOnly();

            document.perform (new FontSizeChangeAction (label, *document.getComponentLayout(), (float) newValue),
                              "Change Label font size");
        }

        double getValue() const
        {
            return label->getFont().getHeight();
        }

        void changeListenerCallback (ChangeBroadcaster*)     { refresh(); }

    private:
        Label* const label;
        JucerDocument& document;

        class FontSizeChangeAction  : public ComponentUndoableAction <Label>
        {
        public:
            FontSizeChangeAction (Label* const comp, ComponentLayout& l, const float newState_)
                : ComponentUndoableAction <Label> (comp, l),
                  newState (newState_)
            {
                oldState = comp->getFont().getHeight();
            }

            bool perform()
            {
                showCorrectTab();
                Font f (getComponent()->getFont());
                f.setHeight ((float) newState);
                getComponent()->setFont (f);
                changed();
                return true;
            }

            bool undo()
            {
                showCorrectTab();
                Font f (getComponent()->getFont());
                f.setHeight ((float) oldState);
                getComponent()->setFont (f);
                changed();
                return true;
            }

            float newState, oldState;
        };
    };

    //==============================================================================
    class FontStyleProperty  : public ChoicePropertyComponent,
                               public ChangeListener
    {
    public:
        FontStyleProperty (Label* const label_, JucerDocument& doc)
            : ChoicePropertyComponent ("style"),
              label (label_),
              document (doc)
        {
            document.addChangeListener (this);

            choices.add ("normal");
            choices.add ("bold");
            choices.add ("italic");
            choices.add ("bold + italic");
        }

        ~FontStyleProperty()
        {
            document.removeChangeListener (this);
        }

        void setIndex (int newIndex)
        {
            Font f (label->getFont());

            f.setBold (newIndex == 1 || newIndex == 3);
            f.setItalic (newIndex == 2 || newIndex == 3);

            document.perform (new FontStyleChangeAction (label, *document.getComponentLayout(), f),
                              "Change Label font style");
        }

        int getIndex() const
        {
            if (label->getFont().isBold() && label->getFont().isItalic())
                return 3;
            else if (label->getFont().isBold())
                return 1;
            else if (label->getFont().isItalic())
                return 2;

            return 0;
        }

        void changeListenerCallback (ChangeBroadcaster*)     { refresh(); }

    private:
        Label* const label;
        JucerDocument& document;

        class FontStyleChangeAction  : public ComponentUndoableAction <Label>
        {
        public:
            FontStyleChangeAction (Label* const comp, ComponentLayout& l, const Font& newState_)
                : ComponentUndoableAction <Label> (comp, l),
                  newState (newState_)
            {
                oldState = comp->getFont();
            }

            bool perform()
            {
                showCorrectTab();
                getComponent()->setFont (newState);
                changed();
                return true;
            }

            bool undo()
            {
                showCorrectTab();
                getComponent()->setFont (oldState);
                changed();
                return true;
            }

            Font newState, oldState;
        };
    };
};

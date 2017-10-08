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

    Component* createNewComponent (JucerDocument*) override
    {
        return new Label ("new label", "label text");
    }

    XmlElement* createXmlFor (Component* comp, const ComponentLayout* layout) override
    {
        Label* const l = dynamic_cast<Label*> (comp);

        XmlElement* e = ComponentTypeHandler::createXmlFor (comp, layout);
        e->setAttribute ("labelText", l->getText());

        e->setAttribute ("editableSingleClick", l->isEditableOnSingleClick());
        e->setAttribute ("editableDoubleClick", l->isEditableOnDoubleClick());
        e->setAttribute ("focusDiscardsChanges", l->doesLossOfFocusDiscardChanges());

        e->setAttribute ("fontname", l->getProperties().getWithDefault ("typefaceName", FontPropertyComponent::getDefaultFont()).toString());
        e->setAttribute ("fontsize", roundToInt (l->getFont().getHeight() * 100.0) / 100.0);
        e->setAttribute ("kerning", roundToInt (l->getFont().getExtraKerningFactor() * 1000.0) / 1000.0);
        e->setAttribute ("bold", l->getFont().isBold());
        e->setAttribute ("italic", l->getFont().isItalic());
        e->setAttribute ("justification", l->getJustificationType().getFlags());
        if (l->getFont().getTypefaceStyle() != "Regular")
        {
            e->setAttribute ("typefaceStyle", l->getFont().getTypefaceStyle());
        }

        return e;
    }

    bool restoreFromXml (const XmlElement& xml, Component* comp, const ComponentLayout* layout) override
    {
        Label* const l = dynamic_cast<Label*> (comp);

        if (! ComponentTypeHandler::restoreFromXml (xml, comp, layout))
            return false;

        Label defaultLabel;

        Font font;
        font.setHeight ((float) xml.getDoubleAttribute ("fontsize", 15.0));
        font.setBold (xml.getBoolAttribute ("bold", false));
        font.setItalic (xml.getBoolAttribute ("italic", false));
        font.setExtraKerningFactor ((float) xml.getDoubleAttribute ("kerning", 0.0));
        auto fontStyle = xml.getStringAttribute ("typefaceStyle");
        if (! fontStyle.isEmpty())
            font.setTypefaceStyle (fontStyle);

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

    String getCreationParameters (GeneratedCode& code, Component* component) override
    {
        Label* const l = dynamic_cast<Label*> (component);

        return quotedString (component->getName(), false)
                 + ",\n"
                 + quotedString (l->getText(), code.shouldUseTransMacro());
    }

    void fillInCreationCode (GeneratedCode& code, Component* component, const String& memberVariableName) override
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

    void fillInGeneratedCode (Component* component, GeneratedCode& code) override
    {
        ComponentTypeHandler::fillInGeneratedCode (component, code);

        if (needsCallback (component))
        {
            String& callback = code.getCallbackCode ("public Label::Listener",
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

    void getEditableProperties (Component* component, JucerDocument& document,
                                Array<PropertyComponent*>& props, bool multipleSelected) override
    {
        ComponentTypeHandler::getEditableProperties (component, document, props, multipleSelected);

        if (multipleSelected)
            return;

        if (auto* const l = dynamic_cast<Label*> (component))
        {
            props.add (new LabelTextProperty          (l, document));
            props.add (new LabelJustificationProperty (l, document));
            props.add (new FontNameProperty           (l, document));
            props.add (new FontStyleProperty          (l, document));
            props.add (new FontSizeProperty           (l, document));
            props.add (new FontKerningProperty        (l, document));

            props.add (new LabelEditableProperty (l, document));

            if (l->isEditableOnDoubleClick() || l->isEditableOnSingleClick())
                props.add (new LabelLossOfFocusProperty (l, document));
        }

        addColourProperties (component, document, props);
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

            updateStylesList (label->getFont());
        }

        ~FontStyleProperty()
        {
            document.removeChangeListener (this);
        }

        void updateStylesList (const Font& newFont)
        {
            if (getNumChildComponents() > 0)
            {
                if (auto cb = dynamic_cast<ComboBox*> (getChildComponent (0)))
                    cb->clear();

                getChildComponent (0)->setVisible (false);
                removeAllChildren();
            }

            choices.clear();

            choices.add ("Regular");
            choices.add ("Bold");
            choices.add ("Italic");
            choices.add ("Bold Italic");

            choices.mergeArray (newFont.getAvailableStyles());
            refresh();
        }

        void setIndex (int newIndex)
        {
            Font f (label->getFont());

            if (f.getAvailableStyles().contains (choices[newIndex]))
            {
                f.setBold   (false);
                f.setItalic (false);
                f.setTypefaceStyle (choices[newIndex]);
            }
            else
            {
                f.setTypefaceStyle ("Regular");
                f.setBold   (newIndex == 1 || newIndex == 3);
                f.setItalic (newIndex == 2 || newIndex == 3);
            }

            document.perform (new FontStyleChangeAction (label, *document.getComponentLayout(), f),
                              "Change Label font style");
        }

        int getIndex() const
        {
            auto f = label->getFont();

            const auto typefaceIndex = choices.indexOf (f.getTypefaceStyle());
            if (typefaceIndex == -1)
            {
                if (f.isBold() && f.isItalic())
                    return 3;
                else if (f.isBold())
                    return 1;
                else if (f.isItalic())
                    return 2;

                return 0;
            }

            return typefaceIndex;
        }

        void changeListenerCallback (ChangeBroadcaster*)
        {
            updateStylesList (label->getFont());
        }

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

    //==============================================================================
    class FontKerningProperty  : public SliderPropertyComponent,
                                 public ChangeListener
    {
    public:
        FontKerningProperty (Label* const label_, JucerDocument& doc)
            : SliderPropertyComponent ("kerning", -0.5, 0.5, 0.001),
              label (label_),
              document (doc)
        {
            document.addChangeListener (this);
        }

        ~FontKerningProperty()
        {
            document.removeChangeListener (this);
        }

        void setValue (double newValue)
        {
            document.getUndoManager().undoCurrentTransactionOnly();

            document.perform (new FontKerningChangeAction (label, *document.getComponentLayout(), (float) newValue),
                              "Change Label font kerning");
        }

        double getValue() const
        {
            return label->getFont().getExtraKerningFactor();
        }

        void changeListenerCallback (ChangeBroadcaster*)
        {
            refresh();
        }

    private:
        Label* const label;
        JucerDocument& document;

        class FontKerningChangeAction  : public ComponentUndoableAction <Label>
        {
        public:
            FontKerningChangeAction (Label* const comp, ComponentLayout& l, const float newState_)
                : ComponentUndoableAction <Label> (comp, l),
                  newState (newState_)
            {
                oldState = comp->getFont().getExtraKerningFactor();
            }

            bool perform()
            {
                showCorrectTab();
                Font f (getComponent()->getFont());
                f.setExtraKerningFactor ((float) newState);
                getComponent()->setFont (f);
                changed();
                return true;
            }

            bool undo()
            {
                showCorrectTab();
                Font f (getComponent()->getFont());
                f.setExtraKerningFactor ((float) oldState);
                getComponent()->setFont (f);
                changed();
                return true;
            }

            float newState, oldState;
        };
    };
};

/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

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

#ifndef __JUCER_LABELHANDLER_JUCEHEADER__
#define __JUCER_LABELHANDLER_JUCEHEADER__

#include "../../properties/jucer_JustificationProperty.h"
#include "../../properties/jucer_FontPropertyComponent.h"
#include "../../properties/jucer_ComponentBooleanProperty.h"


//==============================================================================
/**
*/
class LabelHandler  : public ComponentTypeHandler
{
public:
    //==============================================================================
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

    //==============================================================================
    Component* createNewComponent (JucerDocument*)
    {
        return new Label ("new label", "label text");
    }

    XmlElement* createXmlFor (Component* comp, const ComponentLayout* layout)
    {
        Label* const l = dynamic_cast <Label*> (comp);

        XmlElement* e = ComponentTypeHandler::createXmlFor (comp, layout);
        e->setAttribute ("labelText", l->getText());

        e->setAttribute ("editableSingleClick", l->isEditableOnSingleClick());
        e->setAttribute ("editableDoubleClick", l->isEditableOnDoubleClick());
        e->setAttribute ("focusDiscardsChanges", l->doesLossOfFocusDiscardChanges());

        e->setAttribute ("fontname", l->getProperties().getWithDefault ("typefaceName", FontPropertyComponent::defaultFont).toString());
        e->setAttribute ("fontsize", roundToInt (l->getFont().getHeight() * 100.0) / 100.0);
        e->setAttribute ("bold", l->getFont().isBold());
        e->setAttribute ("italic", l->getFont().isItalic());
        e->setAttribute ("justification", l->getJustificationType().getFlags());

        return e;
    }

    bool restoreFromXml (const XmlElement& xml, Component* comp, const ComponentLayout* layout)
    {
        Label* const l = dynamic_cast <Label*> (comp);

        if (! ComponentTypeHandler::restoreFromXml (xml, comp, layout))
            return false;

        Label defaultLabel (String::empty, String::empty);

        Font font;
        font.setHeight ((float) xml.getDoubleAttribute ("fontsize", 15.0));
        font.setBold (xml.getBoolAttribute ("bold", false));
        font.setItalic (xml.getBoolAttribute ("italic", false));
        l->setFont (font);

        l->getProperties().set ("typefaceName", xml.getStringAttribute ("fontname", FontPropertyComponent::defaultFont));
        updateLabelFont (l);

        l->setJustificationType (Justification (xml.getIntAttribute ("justification", Justification::centred)));

        l->setText (xml.getStringAttribute ("labelText", "Label Text"), false);

        l->setEditable (xml.getBoolAttribute ("editableSingleClick", defaultLabel.isEditableOnSingleClick()),
                        xml.getBoolAttribute ("editableDoubleClick", defaultLabel.isEditableOnDoubleClick()),
                        xml.getBoolAttribute ("focusDiscardsChanges", defaultLabel.doesLossOfFocusDiscardChanges()));

        return true;
    }

    static void updateLabelFont (Label* label)
    {
        Font f (label->getFont());
        f = FontPropertyComponent::applyNameToFont (label->getProperties().getWithDefault ("typefaceName", FontPropertyComponent::defaultFont), f);
        label->setFont (f);
    }

    const String getCreationParameters (Component* component)
    {
        Label* const l = dynamic_cast <Label*> (component);

        return quotedString (component->getName())
                 + ",\n"
                 + quotedString (l->getText());
    }

    void fillInCreationCode (GeneratedCode& code, Component* component, const String& memberVariableName)
    {
        ComponentTypeHandler::fillInCreationCode (code, component, memberVariableName);

        Label* const l = dynamic_cast <Label*> (component);

        String s;

        s << memberVariableName << "->setFont ("
          << FontPropertyComponent::getCompleteFontCode (l->getFont(), l->getProperties().getWithDefault ("typefaceName", FontPropertyComponent::defaultFont))
          << ");\n"
          << memberVariableName << "->setJustificationType ("
          << justificationToCode (l->getJustificationType())
          << ");\n"
          << memberVariableName << "->setEditable ("
          << boolToString (l->isEditableOnSingleClick()) << ", "
          << boolToString (l->isEditableOnDoubleClick()) << ", "
          << boolToString (l->doesLossOfFocusDiscardChanges()) << ");\n"
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

    void getEditableProperties (Component* component, JucerDocument& document, Array <PropertyComponent*>& properties)
    {
        ComponentTypeHandler::getEditableProperties (component, document, properties);

        Label* const l = dynamic_cast <Label*> (component);
        properties.add (new LabelTextProperty (l, document));

        properties.add (new LabelJustificationProperty (l, document));
        properties.add (new FontNameProperty (l, document));
        properties.add (new FontSizeProperty (l, document));
        properties.add (new FontStyleProperty (l, document));

        addColourProperties (component, document, properties);

        properties.add (new LabelEditableProperty (l, document));

        if (l->isEditableOnDoubleClick() || l->isEditableOnSingleClick())
            properties.add (new LabelLossOfFocusProperty (l, document));
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
        LabelTextProperty (Label* component_, JucerDocument& document_)
            : ComponentTextProperty <Label> ("text", 10000, true, component_, document_)
        {}

        void setText (const String& newText)
        {
            document.perform (new LabelTextChangeAction (component, *document.getComponentLayout(), newText),
                              "Change Label text");
        }

        String getText() const
        {
            return component->getText();
        }

    private:
        class LabelTextChangeAction  : public ComponentUndoableAction <Label>
        {
        public:
            LabelTextChangeAction (Label* const comp, ComponentLayout& layout, const String& newState_)
                : ComponentUndoableAction <Label> (comp, layout),
                  newState (newState_)
            {
                oldState = comp->getText();
            }

            bool perform()
            {
                showCorrectTab();
                getComponent()->setText (newState, false);
                changed();
                return true;
            }

            bool undo()
            {
                showCorrectTab();
                getComponent()->setText (oldState, false);
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
        LabelEditableProperty (Label* component_,
                               JucerDocument& document_)
           : ComponentChoiceProperty <Label> ("editing", component_, document_)
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
            LabelEditableChangeAction (Label* const comp, ComponentLayout& layout, const int newState_)
                : ComponentUndoableAction <Label> (comp, layout),
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
        LabelLossOfFocusProperty (Label* component_, JucerDocument& document_)
           : ComponentChoiceProperty <Label> ("focus", component_, document_)
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
            LabelFocusLossChangeAction (Label* const comp, ComponentLayout& layout, const bool newState_)
                : ComponentUndoableAction <Label> (comp, layout),
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
        LabelJustificationProperty (Label* const label_, JucerDocument& document_)
            : JustificationProperty ("layout", false),
              label (label_),
              document (document_)
        {
            document.addChangeListener (this);
        }

        ~LabelJustificationProperty()
        {
            document.removeChangeListener (this);
        }

        void setJustification (const Justification& newJustification)
        {
            document.perform (new LabelJustifyChangeAction (label, *document.getComponentLayout(), newJustification),
                              "Change Label justification");
        }

        const Justification getJustification() const
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
            LabelJustifyChangeAction (Label* const comp, ComponentLayout& layout, const Justification& newState_)
                : ComponentUndoableAction <Label> (comp, layout),
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
        FontNameProperty (Label* const label_, JucerDocument& document_)
            : FontPropertyComponent ("font"),
              label (label_),
              document (document_)
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
            return label->getProperties().getWithDefault ("typefaceName", FontPropertyComponent::defaultFont);
        }

        void changeListenerCallback (ChangeBroadcaster*)                     { refresh(); }

    private:
        Label* const label;
        JucerDocument& document;

        class FontNameChangeAction  : public ComponentUndoableAction <Label>
        {
        public:
            FontNameChangeAction (Label* const comp, ComponentLayout& layout, const String& newState_)
                : ComponentUndoableAction <Label> (comp, layout),
                  newState (newState_)
            {
                oldState = comp->getProperties().getWithDefault ("typefaceName", FontPropertyComponent::defaultFont);
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
        FontSizeProperty (Label* const label_, JucerDocument& document_)
            : SliderPropertyComponent ("size", 1.0, 250.0, 0.1, 0.3),
              label (label_),
              document (document_)
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
            FontSizeChangeAction (Label* const comp, ComponentLayout& layout, const float newState_)
                : ComponentUndoableAction <Label> (comp, layout),
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
        FontStyleProperty (Label* const label_, JucerDocument& document_)
            : ChoicePropertyComponent ("style"),
              label (label_),
              document (document_)
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
            FontStyleChangeAction (Label* const comp, ComponentLayout& layout, const Font& newState_)
                : ComponentUndoableAction <Label> (comp, layout),
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


#endif   // __JUCER_LABELHANDLER_JUCEHEADER__

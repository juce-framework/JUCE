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

#ifndef __JUCER_SLIDERHANDLER_JUCEHEADER__
#define __JUCER_SLIDERHANDLER_JUCEHEADER__

//==============================================================================
/**
*/
class SliderHandler  : public ComponentTypeHandler
{
public:
    //==============================================================================
    SliderHandler()
        : ComponentTypeHandler ("Slider", "Slider", typeid (Slider), 150, 24)
    {
        registerColour (Slider::backgroundColourId, "background", "bkgcol");
        registerColour (Slider::thumbColourId, "thumb", "thumbcol");
        registerColour (Slider::trackColourId, "track", "trackcol");
        registerColour (Slider::rotarySliderFillColourId, "rotary fill", "rotarysliderfill");
        registerColour (Slider::rotarySliderOutlineColourId, "rotary outln", "rotaryslideroutline");
        registerColour (Slider::textBoxTextColourId, "textbox text", "textboxtext");
        registerColour (Slider::textBoxBackgroundColourId, "textbox bkgd", "textboxbkgd");
        registerColour (Slider::textBoxHighlightColourId, "textbox highlt", "textboxhighlight");
        registerColour (Slider::textBoxOutlineColourId, "textbox outln", "textboxoutline");
    }

    //==============================================================================
    Component* createNewComponent (JucerDocument*)
    {
        return new Slider (T("new slider"));
    }

    //==============================================================================
    XmlElement* createXmlFor (Component* comp, const ComponentLayout* layout)
    {
        XmlElement* e = ComponentTypeHandler::createXmlFor (comp, layout);

        Slider* const s = dynamic_cast <Slider*> (comp);
        e->setAttribute (T("min"), s->getMinimum());
        e->setAttribute (T("max"), s->getMaximum());
        e->setAttribute (T("int"), s->getInterval());
        e->setAttribute (T("style"), sliderStyleToString (s->getSliderStyle()));
        e->setAttribute (T("textBoxPos"), textBoxPosToString (s->getTextBoxPosition()));
        e->setAttribute (T("textBoxEditable"), s->isTextBoxEditable());
        e->setAttribute (T("textBoxWidth"), s->getTextBoxWidth());
        e->setAttribute (T("textBoxHeight"), s->getTextBoxHeight());
        e->setAttribute (T("skewFactor"), s->getSkewFactor());

        return e;
    }

    bool restoreFromXml (const XmlElement& xml, Component* comp, const ComponentLayout* layout)
    {
        if (! ComponentTypeHandler::restoreFromXml (xml, comp, layout))
            return false;

        Slider* const s = dynamic_cast <Slider*> (comp);

        s->setRange (xml.getDoubleAttribute (T("min"), 0.0),
                     xml.getDoubleAttribute (T("max"), 10.0),
                     xml.getDoubleAttribute (T("int"), 0.0));

        s->setSliderStyle (sliderStringToStyle (xml.getStringAttribute (T("style"), T("LinearHorizontal"))));

        s->setTextBoxStyle (stringToTextBoxPos (xml.getStringAttribute (T("textBoxPos"), T("TextBoxLeft"))),
                            ! xml.getBoolAttribute (T("textBoxEditable"), true),
                            xml.getIntAttribute (T("textBoxWidth"), 80),
                            xml.getIntAttribute (T("textBoxHeight"), 20));

        s->setSkewFactor (xml.getDoubleAttribute (T("skewFactor"), 1.0));

        return true;
    }

    //==============================================================================
    const String getCreationParameters (Component* component)
    {
        return quotedString (component->getName());
    }

    void fillInCreationCode (GeneratedCode& code, Component* component, const String& memberVariableName)
    {
        ComponentTypeHandler::fillInCreationCode (code, component, memberVariableName);

        Slider* const s = dynamic_cast <Slider*> (component);

        String r;
        r << memberVariableName << "->setRange ("
          << s->getMinimum() << ", " << s->getMaximum() << ", " << s->getInterval()
          << ");\n"
          << memberVariableName << "->setSliderStyle (Slider::"
          << sliderStyleToString (s->getSliderStyle()) << ");\n"
          << memberVariableName << "->setTextBoxStyle (Slider::"
          << textBoxPosToString (s->getTextBoxPosition())
          << ", " << boolToString (! s->isTextBoxEditable())
          << ", " << s->getTextBoxWidth() << ", " << s->getTextBoxHeight() << ");\n"
          << getColourIntialisationCode (component, memberVariableName);

        if (needsCallback (component))
            r << memberVariableName << "->addListener (this);\n";

        if (s->getSkewFactor() != 1.0)
            r << memberVariableName << "->setSkewFactor (" << s->getSkewFactor() << ");\n";

        r << T('\n');
        code.constructorCode += r;
    }

    void fillInGeneratedCode (Component* component, GeneratedCode& code)
    {
        ComponentTypeHandler::fillInGeneratedCode (component, code);

        if (needsCallback (component))
        {
            String& callback = code.getCallbackCode (T("public SliderListener"),
                                                     T("void"),
                                                     T("sliderValueChanged (Slider* sliderThatWasMoved)"),
                                                     true);

            if (callback.isNotEmpty())
                callback << "else ";

            const String memberVariableName (code.document->getComponentLayout()->getComponentMemberVariableName (component));
            const String userCodeComment (T("UserSliderCode_") + memberVariableName);

            callback
                << "if (sliderThatWasMoved == " << memberVariableName
                << ")\n{\n    //[" << userCodeComment << "] -- add your slider handling code here..\n    //[/" << userCodeComment << "]\n}\n";
        }
    }

    //==============================================================================
    void getEditableProperties (Component* component, JucerDocument& document, Array <PropertyComponent*>& properties)
    {
        ComponentTypeHandler::getEditableProperties (component, document, properties);

        Slider* s = dynamic_cast <Slider*> (component);
        jassert (s != 0);

        properties.add (new SliderRangeProperty (s, document, T("minimum"), 0));
        properties.add (new SliderRangeProperty (s, document, T("maximum"), 1));
        properties.add (new SliderRangeProperty (s, document, T("interval"), 2));
        properties.add (new SliderTypeProperty (s, document));
        properties.add (new SliderTextboxProperty (s, document));
        properties.add (new SliderTextboxEditableProperty (s, document));
        properties.add (new SliderTextboxSizeProperty (s, document, true));
        properties.add (new SliderTextboxSizeProperty (s, document, false));
        properties.add (new SliderSkewProperty (s, document));

        addColourProperties (component, document, properties);
    }

    static bool needsCallback (Component* slider)
    {
        return true; //xxx should be a property
    }

    juce_UseDebuggingNewOperator

private:
    //==============================================================================
    class SliderTypeProperty  : public ComponentChoiceProperty <Slider>
    {
    public:
        SliderTypeProperty (Slider* slider, JucerDocument& document)
            : ComponentChoiceProperty <Slider> (T("type"), slider, document)
        {
            choices.add ("Linear Horizontal");
            choices.add ("Linear Vertical");
            choices.add ("Linear Bar");
            choices.add ("Rotary");
            choices.add ("Rotary HorizontalDrag");
            choices.add ("Rotary VerticalDrag");
            choices.add ("Inc/Dec Buttons");
            choices.add ("Two Value Horizontal");
            choices.add ("Two Value Vertical");
            choices.add ("Three Value Horizontal");
            choices.add ("Three Value Vertical");
        }

        void setIndex (const int newIndex)
        {
            const Slider::SliderStyle types[] = { Slider::LinearHorizontal,
                                                  Slider::LinearVertical,
                                                  Slider::LinearBar,
                                                  Slider::Rotary,
                                                  Slider::RotaryHorizontalDrag,
                                                  Slider::RotaryVerticalDrag,
                                                  Slider::IncDecButtons,
                                                  Slider::TwoValueHorizontal,
                                                  Slider::TwoValueVertical,
                                                  Slider::ThreeValueHorizontal,
                                                  Slider::ThreeValueVertical };

            if (newIndex >= 0 && newIndex < numElementsInArray (types))
            {
                document.perform (new SliderTypeChangeAction (component, *document.getComponentLayout(), types [newIndex]),
                                  T("Change Slider style"));
            }
        }

        int getIndex() const
        {
            const Slider::SliderStyle types[] = { Slider::LinearHorizontal,
                                                  Slider::LinearVertical,
                                                  Slider::LinearBar,
                                                  Slider::Rotary,
                                                  Slider::RotaryHorizontalDrag,
                                                  Slider::RotaryVerticalDrag,
                                                  Slider::IncDecButtons,
                                                  Slider::TwoValueHorizontal,
                                                  Slider::TwoValueVertical,
                                                  Slider::ThreeValueHorizontal,
                                                  Slider::ThreeValueVertical };

            for (int i = 0; i < numElementsInArray (types); ++i)
                if (types [i] == dynamic_cast <Slider*> (component)->getSliderStyle())
                    return i;

            return -1;
        }

    private:
        class SliderTypeChangeAction  : public ComponentUndoableAction <Slider>
        {
        public:
            SliderTypeChangeAction (Slider* const comp, ComponentLayout& layout, const Slider::SliderStyle newState_)
                : ComponentUndoableAction <Slider> (comp, layout),
                  newState (newState_)
            {
                oldState = comp->getSliderStyle();
            }

            bool perform()
            {
                showCorrectTab();
                getComponent()->setSliderStyle (newState);
                changed();
                return true;
            }

            bool undo()
            {
                showCorrectTab();
                getComponent()->setSliderStyle (oldState);
                changed();
                return true;
            }

            Slider::SliderStyle newState, oldState;
        };
    };

    //==============================================================================
    class SliderTextboxProperty  : public ComponentChoiceProperty <Slider>
    {
    public:
        SliderTextboxProperty (Slider* slider, JucerDocument& document)
            : ComponentChoiceProperty <Slider> (T("text position"), slider, document)
        {
            choices.add ("No text box");
            choices.add ("Text box on left");
            choices.add ("Text box on right");
            choices.add ("Text box above");
            choices.add ("Text box below");
        }

        void setIndex (const int newIndex)
        {
            const Slider::TextEntryBoxPosition types[] = { Slider::NoTextBox,
                                                           Slider::TextBoxLeft,
                                                           Slider::TextBoxRight,
                                                           Slider::TextBoxAbove,
                                                           Slider::TextBoxBelow };

            if (newIndex >= 0 && newIndex < numElementsInArray (types))
            {
                document.perform (new SliderTextBoxChangeAction (component, *document.getComponentLayout(), types [newIndex]),
                                  T("Change Slider textbox"));
            }
        }

        int getIndex() const
        {
            const Slider::TextEntryBoxPosition types[] = { Slider::NoTextBox,
                                                           Slider::TextBoxLeft,
                                                           Slider::TextBoxRight,
                                                           Slider::TextBoxAbove,
                                                           Slider::TextBoxBelow };

            for (int i = 0; i < numElementsInArray (types); ++i)
                if (types [i] == component->getTextBoxPosition())
                    return i;

            return -1;
        }

    private:
        class SliderTextBoxChangeAction  : public ComponentUndoableAction <Slider>
        {
        public:
            SliderTextBoxChangeAction (Slider* const comp, ComponentLayout& layout, const Slider::TextEntryBoxPosition newState_)
                : ComponentUndoableAction <Slider> (comp, layout),
                  newState (newState_)
            {
                oldState = comp->getTextBoxPosition();
            }

            bool perform()
            {
                showCorrectTab();
                getComponent()->setTextBoxStyle (newState,
                                                 ! getComponent()->isTextBoxEditable(),
                                                 getComponent()->getTextBoxWidth(),
                                                 getComponent()->getTextBoxHeight());
                changed();
                return true;
            }

            bool undo()
            {
                showCorrectTab();
                getComponent()->setTextBoxStyle (oldState,
                                                 ! getComponent()->isTextBoxEditable(),
                                                 getComponent()->getTextBoxWidth(),
                                                 getComponent()->getTextBoxHeight());
                changed();
                return true;
            }

            Slider::TextEntryBoxPosition newState, oldState;
        };
    };

    //==============================================================================
    class SliderTextboxEditableProperty  : public ComponentBooleanProperty <Slider>
    {
    public:
        SliderTextboxEditableProperty (Slider* slider, JucerDocument& document)
            : ComponentBooleanProperty <Slider> (T("text box mode"), T("Editable"), T("Editable"), slider, document)
        {
        }

        void setState (const bool newState)
        {
            document.perform (new SliderEditableChangeAction (component, *document.getComponentLayout(), newState),
                              T("Change Slider editability"));
        }

        bool getState() const
        {
            return component->isTextBoxEditable();
        }

    private:
        class SliderEditableChangeAction  : public ComponentUndoableAction <Slider>
        {
        public:
            SliderEditableChangeAction (Slider* const comp, ComponentLayout& layout, const bool newState_)
                : ComponentUndoableAction <Slider> (comp, layout),
                  newState (newState_)
            {
                oldState = comp->isTextBoxEditable();
            }

            bool perform()
            {
                showCorrectTab();
                getComponent()->setTextBoxIsEditable (newState);
                changed();
                return true;
            }

            bool undo()
            {
                showCorrectTab();
                getComponent()->setTextBoxIsEditable (oldState);
                changed();
                return true;
            }

            bool newState, oldState;
        };
    };

    //==============================================================================
    class SliderTextboxSizeProperty  : public ComponentTextProperty <Slider>
    {
    public:
        SliderTextboxSizeProperty (Slider* slider, JucerDocument& document, const bool isWidth_)
            : ComponentTextProperty <Slider> (isWidth_ ? T("text box width") : T("text box height"),
                                              12, false, slider, document),
              isWidth (isWidth_)
        {
        }

        void setText (const String& newText)
        {
            document.perform (new SliderBoxSizeChangeAction (component, *document.getComponentLayout(), isWidth, newText.getIntValue()),
                              T("Change Slider textbox size"));
        }

        const String getText() const
        {
            return String (isWidth ? component->getTextBoxWidth()
                                   : component->getTextBoxHeight());
        }

    private:
        const bool isWidth;

        class SliderBoxSizeChangeAction  : public ComponentUndoableAction <Slider>
        {
        public:
            SliderBoxSizeChangeAction (Slider* const comp, ComponentLayout& layout, const bool isWidth_, int newSize_)
                : ComponentUndoableAction <Slider> (comp, layout),
                  isWidth (isWidth_),
                  newSize (newSize_)
            {
                oldSize = isWidth ? comp->getTextBoxWidth()
                                  : comp->getTextBoxHeight();
            }

            bool perform()
            {
                showCorrectTab();

                if (isWidth)
                    getComponent()->setTextBoxStyle (getComponent()->getTextBoxPosition(),
                                                     ! getComponent()->isTextBoxEditable(),
                                                     newSize,
                                                     getComponent()->getTextBoxHeight());
                else
                    getComponent()->setTextBoxStyle (getComponent()->getTextBoxPosition(),
                                                     ! getComponent()->isTextBoxEditable(),
                                                     getComponent()->getTextBoxWidth(),
                                                     newSize);
                changed();
                return true;
            }

            bool undo()
            {
                showCorrectTab();

                if (isWidth)
                    getComponent()->setTextBoxStyle (getComponent()->getTextBoxPosition(),
                                                     ! getComponent()->isTextBoxEditable(),
                                                     oldSize,
                                                     getComponent()->getTextBoxHeight());
                else
                    getComponent()->setTextBoxStyle (getComponent()->getTextBoxPosition(),
                                                     ! getComponent()->isTextBoxEditable(),
                                                     getComponent()->getTextBoxWidth(),
                                                     oldSize);
                changed();
                return true;
            }

            bool isWidth;
            int newSize, oldSize;
        };
    };

    //==============================================================================
    class SliderRangeProperty  : public ComponentTextProperty <Slider>
    {
    public:
        SliderRangeProperty (Slider* slider, JucerDocument& document,
                             const String& name, const int rangeParam_)
            : ComponentTextProperty <Slider> (name, 15, false, slider, document),
              rangeParam (rangeParam_)
        {
        }

        void setText (const String& newText)
        {
            double state [3];
            state [0] = component->getMinimum();
            state [1] = component->getMaximum();
            state [2] = component->getInterval();

            state [rangeParam] = newText.getDoubleValue();

            document.perform (new SliderRangeChangeAction (component, *document.getComponentLayout(), state),
                              T("Change Slider range"));
        }

        const String getText() const
        {
            Slider* s = dynamic_cast <Slider*> (component);
            jassert (s != 0);

            switch (rangeParam)
            {
            case 0:
                return String (s->getMinimum());

            case 1:
                return String (s->getMaximum());

            case 2:
                return String (s->getInterval());

            default:
                jassertfalse
                break;
            }

            return String::empty;
        }

    private:
        const int rangeParam;

        class SliderRangeChangeAction  : public ComponentUndoableAction <Slider>
        {
        public:
            SliderRangeChangeAction (Slider* const comp, ComponentLayout& layout, const double newState_[3])
                : ComponentUndoableAction <Slider> (comp, layout)
            {
                newState [0] = newState_ [0];
                newState [1] = newState_ [1];
                newState [2] = newState_ [2];

                oldState [0] = comp->getMinimum();
                oldState [1] = comp->getMaximum();
                oldState [2] = comp->getInterval();
            }

            bool perform()
            {
                showCorrectTab();
                getComponent()->setRange (newState[0], newState[1], newState[2]);
                changed();
                return true;
            }

            bool undo()
            {
                showCorrectTab();
                getComponent()->setRange (oldState[0], oldState[1], oldState[2]);
                changed();
                return true;
            }

            double newState[3], oldState[3];
        };
    };

    //==============================================================================
    class SliderSkewProperty  : public ComponentTextProperty <Slider>
    {
    public:
        SliderSkewProperty (Slider* slider, JucerDocument& document)
            : ComponentTextProperty <Slider> (T("skew factor"), 12, false, slider, document)
        {
        }

        void setText (const String& newText)
        {
            const double skew = jlimit (0.001, 1000.0, newText.getDoubleValue());

            document.perform (new SliderSkewChangeAction (component, *document.getComponentLayout(), skew),
                              T("Change Slider skew"));
        }

        const String getText() const
        {
            Slider* s = dynamic_cast <Slider*> (component);
            jassert (s != 0);

            return String (s->getSkewFactor());
        }

    private:
        class SliderSkewChangeAction  : public ComponentUndoableAction <Slider>
        {
        public:
            SliderSkewChangeAction (Slider* const comp, ComponentLayout& layout, const double newValue_)
                : ComponentUndoableAction <Slider> (comp, layout)
            {
                newValue = newValue_;
                oldValue = comp->getSkewFactor();
            }

            bool perform()
            {
                showCorrectTab();
                getComponent()->setSkewFactor (newValue);
                changed();
                return true;
            }

            bool undo()
            {
                showCorrectTab();
                getComponent()->setSkewFactor (oldValue);
                changed();
                return true;
            }

            double newValue, oldValue;
        };
    };

    //==============================================================================
    static const String sliderStyleToString (Slider::SliderStyle style)
    {
        switch (style)
        {
        case Slider::LinearHorizontal:
            return T("LinearHorizontal");
        case Slider::LinearVertical:
            return T("LinearVertical");
        case Slider::LinearBar:
            return T("LinearBar");
        case Slider::Rotary:
            return T("Rotary");
        case Slider::RotaryHorizontalDrag:
            return T("RotaryHorizontalDrag");
        case Slider::RotaryVerticalDrag:
            return T("RotaryVerticalDrag");
        case Slider::IncDecButtons:
            return T("IncDecButtons");
        case Slider::TwoValueHorizontal:
            return T("TwoValueHorizontal");
        case Slider::TwoValueVertical:
            return T("TwoValueVertical");
        case Slider::ThreeValueHorizontal:
            return T("ThreeValueHorizontal");
        case Slider::ThreeValueVertical:
            return T("ThreeValueVertical");

        default:
            jassertfalse
            break;
        }

        return String::empty;
    }

    static Slider::SliderStyle sliderStringToStyle (const String& s)
    {
        if (s == T("LinearHorizontal"))
            return Slider::LinearHorizontal;
        else if (s == T("LinearVertical"))
            return Slider::LinearVertical;
        else if (s == T("LinearBar"))
            return Slider::LinearBar;
        else if (s == T("Rotary"))
            return Slider::Rotary;
        else if (s == T("RotaryHorizontalDrag"))
            return Slider::RotaryHorizontalDrag;
        else if (s == T("RotaryVerticalDrag"))
            return Slider::RotaryVerticalDrag;
        else if (s == T("IncDecButtons"))
            return Slider::IncDecButtons;
        else if (s.startsWithIgnoreCase (T("TwoValueHoriz")))
            return Slider::TwoValueHorizontal;
        else if (s.startsWithIgnoreCase (T("TwoValueVert")))
            return Slider::TwoValueVertical;
        else if (s.startsWithIgnoreCase (T("ThreeValueHoriz")))
            return Slider::ThreeValueHorizontal;
        else if (s.startsWithIgnoreCase (T("ThreeValueVert")))
            return Slider::ThreeValueVertical;

        jassertfalse
        return Slider::LinearHorizontal;
    }

    static const String textBoxPosToString (const Slider::TextEntryBoxPosition pos)
    {
        switch (pos)
        {
        case Slider::NoTextBox:
            return T("NoTextBox");
        case Slider::TextBoxLeft:
            return T("TextBoxLeft");
        case Slider::TextBoxRight:
            return T("TextBoxRight");
        case Slider::TextBoxAbove:
            return T("TextBoxAbove");
        case Slider::TextBoxBelow:
            return T("TextBoxBelow");
        default:
            jassertfalse
            break;
        }

        return String::empty;
    }

    static const Slider::TextEntryBoxPosition stringToTextBoxPos (const String& s)
    {
        if (s == T("NoTextBox"))
            return Slider::NoTextBox;
        else if (s == T("TextBoxLeft"))
            return Slider::TextBoxLeft;
        else if (s == T("TextBoxRight"))
            return Slider::TextBoxRight;
        else if (s == T("TextBoxAbove"))
            return Slider::TextBoxAbove;
        else if (s == T("TextBoxBelow"))
            return Slider::TextBoxBelow;

        jassertfalse
        return Slider::TextBoxLeft;
    }
};


#endif   // __JUCER_SLIDERHANDLER_JUCEHEADER__

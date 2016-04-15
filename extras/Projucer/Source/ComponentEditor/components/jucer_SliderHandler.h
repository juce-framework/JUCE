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

static const Slider::SliderStyle sliderStyleTypes[] =
{
    Slider::LinearHorizontal,
    Slider::LinearVertical,
    Slider::LinearBar,
    Slider::LinearBarVertical,
    Slider::Rotary,
    Slider::RotaryHorizontalDrag,
    Slider::RotaryVerticalDrag,
    Slider::RotaryHorizontalVerticalDrag,
    Slider::IncDecButtons,
    Slider::TwoValueHorizontal,
    Slider::TwoValueVertical,
    Slider::ThreeValueHorizontal,
    Slider::ThreeValueVertical
};

static const Slider::TextEntryBoxPosition sliderTextBoxPositions[] =
{
    Slider::NoTextBox,
    Slider::TextBoxLeft,
    Slider::TextBoxRight,
    Slider::TextBoxAbove,
    Slider::TextBoxBelow
};


struct SliderHandler  : public ComponentTypeHandler
{
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

    Component* createNewComponent (JucerDocument*) override
    {
        return new Slider ("new slider");
    }

    XmlElement* createXmlFor (Component* comp, const ComponentLayout* layout) override
    {
        XmlElement* e = ComponentTypeHandler::createXmlFor (comp, layout);

        Slider* s = dynamic_cast<Slider*> (comp);
        e->setAttribute ("min", s->getMinimum());
        e->setAttribute ("max", s->getMaximum());
        e->setAttribute ("int", s->getInterval());
        e->setAttribute ("style", sliderStyleToString (s->getSliderStyle()));
        e->setAttribute ("textBoxPos", textBoxPosToString (s->getTextBoxPosition()));
        e->setAttribute ("textBoxEditable", s->isTextBoxEditable());
        e->setAttribute ("textBoxWidth", s->getTextBoxWidth());
        e->setAttribute ("textBoxHeight", s->getTextBoxHeight());
        e->setAttribute ("skewFactor", s->getSkewFactor());
        e->setAttribute ("needsCallback", needsSliderListener (s));

        return e;
    }

    bool restoreFromXml (const XmlElement& xml, Component* comp, const ComponentLayout* layout) override
    {
        if (! ComponentTypeHandler::restoreFromXml (xml, comp, layout))
            return false;

        Slider* const s = dynamic_cast<Slider*> (comp);

        s->setRange (xml.getDoubleAttribute ("min", 0.0),
                     xml.getDoubleAttribute ("max", 10.0),
                     xml.getDoubleAttribute ("int", 0.0));

        s->setSliderStyle (sliderStringToStyle (xml.getStringAttribute ("style", "LinearHorizontal")));

        s->setTextBoxStyle (stringToTextBoxPos (xml.getStringAttribute ("textBoxPos", "TextBoxLeft")),
                            ! xml.getBoolAttribute ("textBoxEditable", true),
                            xml.getIntAttribute ("textBoxWidth", 80),
                            xml.getIntAttribute ("textBoxHeight", 20));

        s->setSkewFactor (xml.getDoubleAttribute ("skewFactor", 1.0));

        setNeedsSliderListener (s, xml.getBoolAttribute ("needsCallback", true));

        return true;
    }

    String getCreationParameters (GeneratedCode&, Component* component) override
    {
        return quotedString (component->getName(), false);
    }

    void fillInCreationCode (GeneratedCode& code, Component* component, const String& memberVariableName) override
    {
        ComponentTypeHandler::fillInCreationCode (code, component, memberVariableName);

        Slider* const s = dynamic_cast<Slider*> (component);

        String r;
        r << memberVariableName << "->setRange ("
          << s->getMinimum() << ", " << s->getMaximum() << ", " << s->getInterval()
          << ");\n"
          << memberVariableName << "->setSliderStyle (Slider::"
          << sliderStyleToString (s->getSliderStyle()) << ");\n"
          << memberVariableName << "->setTextBoxStyle (Slider::"
          << textBoxPosToString (s->getTextBoxPosition())
          << ", " << CodeHelpers::boolLiteral (! s->isTextBoxEditable())
          << ", " << s->getTextBoxWidth() << ", " << s->getTextBoxHeight() << ");\n"
          << getColourIntialisationCode (component, memberVariableName);

        if (needsSliderListener (component))
            r << memberVariableName << "->addListener (this);\n";

        if (s->getSkewFactor() != 1.0)
            r << memberVariableName << "->setSkewFactor (" << s->getSkewFactor() << ");\n";

        r << '\n';
        code.constructorCode += r;
    }

    void fillInGeneratedCode (Component* component, GeneratedCode& code) override
    {
        ComponentTypeHandler::fillInGeneratedCode (component, code);

        if (needsSliderListener (component))
        {
            String& callback = code.getCallbackCode ("public SliderListener",
                                                     "void",
                                                     "sliderValueChanged (Slider* sliderThatWasMoved)",
                                                     true);

            if (callback.isNotEmpty())
                callback << "else ";

            const String memberVariableName (code.document->getComponentLayout()->getComponentMemberVariableName (component));
            const String userCodeComment ("UserSliderCode_" + memberVariableName);

            callback
                << "if (sliderThatWasMoved == " << memberVariableName
                << ")\n{\n    //[" << userCodeComment << "] -- add your slider handling code here..\n    //[/" << userCodeComment << "]\n}\n";
        }
    }

    void getEditableProperties (Component* component, JucerDocument& document, Array<PropertyComponent*>& props) override
    {
        ComponentTypeHandler::getEditableProperties (component, document, props);

        Slider* s = dynamic_cast<Slider*> (component);
        jassert (s != 0);

        props.add (new SliderRangeProperty (s, document, "minimum", 0));
        props.add (new SliderRangeProperty (s, document, "maximum", 1));
        props.add (new SliderRangeProperty (s, document, "interval", 2));
        props.add (new SliderTypeProperty (s, document));
        props.add (new SliderTextboxProperty (s, document));
        props.add (new SliderTextboxEditableProperty (s, document));
        props.add (new SliderTextboxSizeProperty (s, document, true));
        props.add (new SliderTextboxSizeProperty (s, document, false));
        props.add (new SliderSkewProperty (s, document));
        props.add (new SliderCallbackProperty (s, document));

        addColourProperties (component, document, props);
    }

    static bool needsSliderListener (Component* slider)
    {
        return slider->getProperties().getWithDefault ("generateListenerCallback", true);
    }

    static void setNeedsSliderListener (Component* slider, bool shouldDoCallback)
    {
        slider->getProperties().set ("generateListenerCallback", shouldDoCallback);
    }

private:
    //==============================================================================
    struct SliderTypeProperty  : public ComponentChoiceProperty<Slider>
    {
        SliderTypeProperty (Slider* slider, JucerDocument& doc)
            : ComponentChoiceProperty<Slider> ("type", slider, doc)
        {
            choices.add ("Linear Horizontal");
            choices.add ("Linear Vertical");
            choices.add ("Linear Bar Horizontal");
            choices.add ("Linear Bar Vertical");
            choices.add ("Rotary");
            choices.add ("Rotary HorizontalDrag");
            choices.add ("Rotary VerticalDrag");
            choices.add ("Rotary HorizontalVerticalDrag");
            choices.add ("Inc/Dec Buttons");
            choices.add ("Two Value Horizontal");
            choices.add ("Two Value Vertical");
            choices.add ("Three Value Horizontal");
            choices.add ("Three Value Vertical");
        }

        void setIndex (int newIndex) override
        {
            if (newIndex >= 0 && newIndex < numElementsInArray (sliderStyleTypes))
                document.perform (new SliderTypeChangeAction (component, *document.getComponentLayout(),
                                                              sliderStyleTypes[newIndex]),
                                  "Change Slider style");
        }

        int getIndex() const override
        {
            for (int i = 0; i < numElementsInArray (sliderStyleTypes); ++i)
                if (sliderStyleTypes[i] == dynamic_cast<Slider*> (component)->getSliderStyle())
                    return i;

            return -1;
        }

    private:
        struct SliderTypeChangeAction  : public ComponentUndoableAction<Slider>
        {
            SliderTypeChangeAction (Slider* comp, ComponentLayout& l, Slider::SliderStyle newState_)
                : ComponentUndoableAction<Slider> (comp, l),
                  newState (newState_)
            {
                oldState = comp->getSliderStyle();
            }

            bool perform() override
            {
                showCorrectTab();
                getComponent()->setSliderStyle (newState);
                changed();
                return true;
            }

            bool undo() override
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
    struct SliderTextboxProperty  : public ComponentChoiceProperty<Slider>
    {
        SliderTextboxProperty (Slider* slider, JucerDocument& doc)
            : ComponentChoiceProperty<Slider> ("text position", slider, doc)
        {
            choices.add ("No text box");
            choices.add ("Text box on left");
            choices.add ("Text box on right");
            choices.add ("Text box above");
            choices.add ("Text box below");
        }

        void setIndex (int newIndex) override
        {
            if (newIndex >= 0 && newIndex < numElementsInArray (sliderTextBoxPositions))
                document.perform (new SliderTextBoxChangeAction (component, *document.getComponentLayout(),
                                                                 sliderTextBoxPositions[newIndex]),
                                  "Change Slider textbox");
        }

        int getIndex() const override
        {
            for (int i = 0; i < numElementsInArray (sliderTextBoxPositions); ++i)
                if (sliderTextBoxPositions[i] == component->getTextBoxPosition())
                    return i;

            return -1;
        }

    private:
        struct SliderTextBoxChangeAction  : public ComponentUndoableAction<Slider>
        {
            SliderTextBoxChangeAction (Slider* comp, ComponentLayout& l, Slider::TextEntryBoxPosition newState_)
                : ComponentUndoableAction<Slider> (comp, l),
                  newState (newState_)
            {
                oldState = comp->getTextBoxPosition();
            }

            bool perform() override
            {
                showCorrectTab();
                getComponent()->setTextBoxStyle (newState,
                                                 ! getComponent()->isTextBoxEditable(),
                                                 getComponent()->getTextBoxWidth(),
                                                 getComponent()->getTextBoxHeight());
                changed();
                return true;
            }

            bool undo() override
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
    struct SliderTextboxEditableProperty  : public ComponentBooleanProperty<Slider>
    {
        SliderTextboxEditableProperty (Slider* slider, JucerDocument& doc)
            : ComponentBooleanProperty<Slider> ("text box mode", "Editable", "Editable", slider, doc)
        {
        }

        void setState (bool newState) override
        {
            document.perform (new SliderEditableChangeAction (component, *document.getComponentLayout(), newState),
                              "Change Slider editability");
        }

        bool getState() const override
        {
            return component->isTextBoxEditable();
        }

    private:
        struct SliderEditableChangeAction  : public ComponentUndoableAction<Slider>
        {
            SliderEditableChangeAction (Slider* const comp, ComponentLayout& l, bool newState_)
                : ComponentUndoableAction<Slider> (comp, l),
                  newState (newState_)
            {
                oldState = comp->isTextBoxEditable();
            }

            bool perform() override
            {
                showCorrectTab();
                getComponent()->setTextBoxIsEditable (newState);
                changed();
                return true;
            }

            bool undo() override
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
    struct SliderCallbackProperty  : public ComponentBooleanProperty<Slider>
    {
        SliderCallbackProperty (Slider* s, JucerDocument& doc)
            : ComponentBooleanProperty<Slider> ("callback", "Generate SliderListener",
                                                "Generate SliderListener", s, doc)
        {
        }

        void setState (bool newState) override
        {
            document.perform (new SliderCallbackChangeAction (component, *document.getComponentLayout(), newState),
                              "Change button callback");
        }

        bool getState() const override       { return needsSliderListener (component); }

        struct SliderCallbackChangeAction  : public ComponentUndoableAction<Slider>
        {
            SliderCallbackChangeAction (Slider* comp, ComponentLayout& l, bool newState_)
                : ComponentUndoableAction<Slider> (comp, l),
                  newState (newState_)
            {
                oldState = needsSliderListener (comp);
            }

            bool perform() override
            {
                showCorrectTab();
                setNeedsSliderListener (getComponent(), newState);
                changed();
                return true;
            }

            bool undo() override
            {
                showCorrectTab();
                setNeedsSliderListener (getComponent(), oldState);
                changed();
                return true;
            }

            bool newState, oldState;
        };
    };

    //==============================================================================
    struct SliderTextboxSizeProperty  : public ComponentTextProperty<Slider>
    {
        SliderTextboxSizeProperty (Slider* slider, JucerDocument& doc, bool isWidth_)
            : ComponentTextProperty<Slider> (isWidth_ ? "text box width" : "text box height",
                                             12, false, slider, doc),
              isWidth (isWidth_)
        {
        }

        void setText (const String& newText) override
        {
            document.perform (new SliderBoxSizeChangeAction (component, *document.getComponentLayout(), isWidth, newText.getIntValue()),
                              "Change Slider textbox size");
        }

        String getText() const override
        {
            return String (isWidth ? component->getTextBoxWidth()
                                   : component->getTextBoxHeight());
        }

    private:
        const bool isWidth;

        struct SliderBoxSizeChangeAction  : public ComponentUndoableAction<Slider>
        {
            SliderBoxSizeChangeAction (Slider* const comp, ComponentLayout& l, bool isWidth_, int newSize_)
                : ComponentUndoableAction<Slider> (comp, l),
                  isWidth (isWidth_),
                  newSize (newSize_)
            {
                oldSize = isWidth ? comp->getTextBoxWidth()
                                  : comp->getTextBoxHeight();
            }

            bool perform() override
            {
                showCorrectTab();
                Slider& c = *getComponent();

                if (isWidth)
                    c.setTextBoxStyle (c.getTextBoxPosition(),
                                       ! c.isTextBoxEditable(),
                                       newSize,
                                       c.getTextBoxHeight());
                else
                    c.setTextBoxStyle (c.getTextBoxPosition(),
                                       ! c.isTextBoxEditable(),
                                       c.getTextBoxWidth(),
                                       newSize);
                changed();
                return true;
            }

            bool undo() override
            {
                showCorrectTab();
                Slider& c = *getComponent();

                if (isWidth)
                    c.setTextBoxStyle (c.getTextBoxPosition(),
                                       ! c.isTextBoxEditable(),
                                       oldSize,
                                       c.getTextBoxHeight());
                else
                    c.setTextBoxStyle (c.getTextBoxPosition(),
                                       ! c.isTextBoxEditable(),
                                       c.getTextBoxWidth(),
                                       oldSize);
                changed();
                return true;
            }

            bool isWidth;
            int newSize, oldSize;
        };
    };

    //==============================================================================
    struct SliderRangeProperty  : public ComponentTextProperty<Slider>
    {
        SliderRangeProperty (Slider* slider, JucerDocument& doc,
                             const String& name, int rangeParam_)
            : ComponentTextProperty<Slider> (name, 15, false, slider, doc),
              rangeParam (rangeParam_)
        {
        }

        void setText (const String& newText) override
        {
            double state [3];
            state [0] = component->getMinimum();
            state [1] = component->getMaximum();
            state [2] = component->getInterval();

            state [rangeParam] = newText.getDoubleValue();

            document.perform (new SliderRangeChangeAction (component, *document.getComponentLayout(), state),
                              "Change Slider range");
        }

        String getText() const override
        {
            Slider* s = dynamic_cast<Slider*> (component);
            jassert (s != nullptr);

            switch (rangeParam)
            {
                case 0:     return String (s->getMinimum());
                case 1:     return String (s->getMaximum());
                case 2:     return String (s->getInterval());
                default:    jassertfalse; break;
            }

            return String();
        }

    private:
        const int rangeParam;

        struct SliderRangeChangeAction  : public ComponentUndoableAction<Slider>
        {
            SliderRangeChangeAction (Slider* comp, ComponentLayout& l, const double newState_[3])
                : ComponentUndoableAction<Slider> (comp, l)
            {
                newState[0] = newState_ [0];
                newState[1] = newState_ [1];
                newState[2] = newState_ [2];

                oldState[0] = comp->getMinimum();
                oldState[1] = comp->getMaximum();
                oldState[2] = comp->getInterval();
            }

            bool perform() override
            {
                showCorrectTab();
                getComponent()->setRange (newState[0], newState[1], newState[2]);
                changed();
                return true;
            }

            bool undo() override
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
    struct SliderSkewProperty  : public ComponentTextProperty<Slider>
    {
        SliderSkewProperty (Slider* slider, JucerDocument& doc)
            : ComponentTextProperty<Slider> ("skew factor", 12, false, slider, doc)
        {
        }

        void setText (const String& newText) override
        {
            const double skew = jlimit (0.001, 1000.0, newText.getDoubleValue());

            document.perform (new SliderSkewChangeAction (component, *document.getComponentLayout(), skew),
                              "Change Slider skew");
        }

        String getText() const override
        {
            Slider* s = dynamic_cast<Slider*> (component);
            jassert (s != 0);

            return String (s->getSkewFactor());
        }

        struct SliderSkewChangeAction  : public ComponentUndoableAction<Slider>
        {
            SliderSkewChangeAction (Slider* comp, ComponentLayout& l, double newValue_)
                : ComponentUndoableAction<Slider> (comp, l)
            {
                newValue = newValue_;
                oldValue = comp->getSkewFactor();
            }

            bool perform() override
            {
                showCorrectTab();
                getComponent()->setSkewFactor (newValue);
                changed();
                return true;
            }

            bool undo() override
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
    static String sliderStyleToString (Slider::SliderStyle style)
    {
        switch (style)
        {
            case Slider::LinearHorizontal:              return "LinearHorizontal";
            case Slider::LinearVertical:                return "LinearVertical";
            case Slider::LinearBar:                     return "LinearBar";
            case Slider::LinearBarVertical:             return "LinearBarVertical";
            case Slider::Rotary:                        return "Rotary";
            case Slider::RotaryHorizontalDrag:          return "RotaryHorizontalDrag";
            case Slider::RotaryVerticalDrag:            return "RotaryVerticalDrag";
            case Slider::RotaryHorizontalVerticalDrag:  return "RotaryHorizontalVerticalDrag";
            case Slider::IncDecButtons:                 return "IncDecButtons";
            case Slider::TwoValueHorizontal:            return "TwoValueHorizontal";
            case Slider::TwoValueVertical:              return "TwoValueVertical";
            case Slider::ThreeValueHorizontal:          return "ThreeValueHorizontal";
            case Slider::ThreeValueVertical:            return "ThreeValueVertical";
            default:                                    jassertfalse; break;
        }

        return String();
    }

    static Slider::SliderStyle sliderStringToStyle (const String& s)
    {
        for (int i = 0; i < numElementsInArray (sliderStyleTypes); ++i)
            if (s == sliderStyleToString (sliderStyleTypes[i]))
                return sliderStyleTypes[i];

        jassertfalse;
        return Slider::LinearHorizontal;
    }

    static String textBoxPosToString (const Slider::TextEntryBoxPosition pos)
    {
        switch (pos)
        {
            case Slider::NoTextBox:     return "NoTextBox";
            case Slider::TextBoxLeft:   return "TextBoxLeft";
            case Slider::TextBoxRight:  return "TextBoxRight";
            case Slider::TextBoxAbove:  return "TextBoxAbove";
            case Slider::TextBoxBelow:  return "TextBoxBelow";
            default:                    jassertfalse; break;
        }

        return String();
    }

    static Slider::TextEntryBoxPosition stringToTextBoxPos (const String& s)
    {
        for (int i = 0; i < numElementsInArray (sliderTextBoxPositions); ++i)
            if (s == textBoxPosToString (sliderTextBoxPositions[i]))
                return sliderTextBoxPositions[i];

        jassertfalse;
        return Slider::TextBoxLeft;
    }
};

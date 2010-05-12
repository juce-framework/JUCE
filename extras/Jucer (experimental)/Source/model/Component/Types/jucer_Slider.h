/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-10 by Raw Material Software Ltd.

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

#ifdef ADD_TO_LIST
  ADD_TO_LIST (SliderHandler);
#else

#include "../jucer_ComponentDocument.h"


//==============================================================================
class SliderHandler  : public ComponentTypeHelper<Slider>
{
public:
    SliderHandler() : ComponentTypeHelper<Slider> ("Slider", "Slider", "SLIDER", "slider")
    {
        addEditableColour (Slider::backgroundColourId, "Background", "backgroundColour");
        addEditableColour (Slider::thumbColourId, "Thumb", "thumbColour");
        addEditableColour (Slider::trackColourId, "Track", "trackColour");
        addEditableColour (Slider::rotarySliderFillColourId, "Rotary Fill", "rotaryFillColour");
        addEditableColour (Slider::rotarySliderOutlineColourId, "Rotary Outline", "rotaryOutlineColour");
        addEditableColour (Slider::textBoxTextColourId, "Text", "textColour");
        addEditableColour (Slider::textBoxBackgroundColourId, "Text Background", "textBackgroundColour");
        addEditableColour (Slider::textBoxHighlightColourId, "Text Highlight", "textHighlightColour");
        addEditableColour (Slider::textBoxOutlineColourId, "Textbox Outline", "textboxOutlineColour");
    }

    ~SliderHandler()  {}

    Component* createComponent()                { return new Slider(); }
    const Rectangle<int> getDefaultSize()       { return Rectangle<int> (0, 0, 200, 24); }

    void initialiseNew (ComponentTypeInstance& item)
    {
        item.set ("min", 0);
        item.set ("max", 100);
        item.set ("interval", 1);
        item.set ("type", 1 + Slider::LinearHorizontal);
        item.set ("textBoxPos", 2);
        item.set ("editable", true);
        item.set ("textBoxWidth", 80);
        item.set ("textBoxHeight", 20);
        item.set ("skew", 1);
    }

    void update (ComponentTypeInstance& item, Slider* comp)
    {
        comp->setRange ((double) item ["min"], (double) item ["max"], (double) item ["interval"]);
        comp->setSliderStyle ((Slider::SliderStyle) ((int) item ["type"] - 1));
        comp->setTextBoxStyle ((Slider::TextEntryBoxPosition) ((int) item ["textBoxPos"] - 1),
                               ! (bool) item ["editable"],
                               (int) item ["textBoxWidth"], (int) item ["textBoxHeight"]);
        comp->setSkewFactor ((double) item ["skew"]);
    }

    void createProperties (ComponentTypeInstance& item, Array <PropertyComponent*>& props)
    {
        item.addTooltipProperty (props);
        item.addFocusOrderProperty (props);

        props.add (new TextPropertyComponent (Value (new NumericValueSource<double> (item.getValue ("min"))), "Minimum", 16, false));
        props.add (new TextPropertyComponent (Value (new NumericValueSource<double> (item.getValue ("max"))), "Maximum", 16, false));
        props.add (new TextPropertyComponent (Value (new NumericValueSource<double> (item.getValue ("interval"))), "Interval", 16, false));

        const char* const types[] = { "LinearHorizontal", "LinearVertical", "LinearBar", "Rotary", "RotaryHorizontalDrag", "RotaryVerticalDrag",
                                      "IncDecButtons", "TwoValueHorizontal", "TwoValueVertical", "ThreeValueHorizontal", "ThreeValueVertical", 0 };
        props.add (new ChoicePropertyComponent (item.getValue ("type"), "Type", StringArray (types)));

        const char* const textBoxPositions[] = { "NoTextBox", "TextBoxLeft", "TextBoxRight", "TextBoxAbove", "TextBoxBelow", 0 };
        props.add (new ChoicePropertyComponent (item.getValue ("textBoxPos"), "Text Box", StringArray (textBoxPositions)));

        props.add (new BooleanPropertyComponent (item.getValue ("editable"), "Editable", "Value can be edited"));
        props.add (new TextPropertyComponent (Value (new NumericValueSource<int> (item.getValue ("textBoxWidth"))), "Text Box Width", 8, false));
        props.add (new TextPropertyComponent (Value (new NumericValueSource<int> (item.getValue ("textBoxHeight"))), "Text Box Height", 8, false));

        props.add (new TextPropertyComponent (Value (new NumericValueSource<double> (item.getValue ("skew"))), "Skew Factor", 16, false));
        addEditableColourProperties (item, props);
    }

    void createCode (ComponentTypeInstance& item, CodeGenerator& code)
    {
        code.constructorCode << item.createConstructorStatement (CodeFormatting::stringLiteral (item.getComponentName()));
    }
};

#endif

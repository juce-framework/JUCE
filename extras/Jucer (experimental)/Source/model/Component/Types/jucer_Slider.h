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
    SliderHandler() : ComponentTypeHelper<Slider> ("Slider", "SLIDER", "slider")
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

    Component* createComponent()                { return new Slider (String::empty); }
    const Rectangle<int> getDefaultSize()       { return Rectangle<int> (0, 0, 200, 24); }

    void initialiseNew (ComponentDocument& document, ValueTree& state)
    {
        state.setProperty ("min", 0, 0);
        state.setProperty ("max", 100, 0);
        state.setProperty ("interval", 1, 0);
        state.setProperty ("type", 1 + Slider::LinearHorizontal, 0);
        state.setProperty ("textBoxPos", 2, 0);
        state.setProperty ("editable", true, 0);
        state.setProperty ("textBoxWidth", 80, 0);
        state.setProperty ("textBoxHeight", 20, 0);
        state.setProperty ("skew", 1, 0);
    }

    void update (ComponentDocument& document, Slider* comp, const ValueTree& state)
    {
        comp->setRange ((double) state ["min"], (double) state ["max"], (double) state ["interval"]);
        comp->setSliderStyle ((Slider::SliderStyle) ((int) state ["type"] - 1));
        comp->setTextBoxStyle ((Slider::TextEntryBoxPosition) ((int) state ["textBoxPos"] - 1),
                               ! (bool) state ["editable"],
                               (int) state ["textBoxWidth"], (int) state ["textBoxHeight"]);
        comp->setSkewFactor ((double) state ["skew"]);
    }

    void createProperties (ComponentDocument& document, ValueTree& state, Array <PropertyComponent*>& props)
    {
        addTooltipProperty (document, state, props);
        addFocusOrderProperty (document, state, props);

        props.add (new TextPropertyComponent (Value (new NumericValueSource<double> (getValue ("min", state, document))), "Minimum", 16, false));
        props.add (new TextPropertyComponent (Value (new NumericValueSource<double> (getValue ("max", state, document))), "Maximum", 16, false));
        props.add (new TextPropertyComponent (Value (new NumericValueSource<double> (getValue ("interval", state, document))), "Interval", 16, false));

        const char* const types[] = { "LinearHorizontal", "LinearVertical", "LinearBar", "Rotary", "RotaryHorizontalDrag", "RotaryVerticalDrag",
                                      "IncDecButtons", "TwoValueHorizontal", "TwoValueVertical", "ThreeValueHorizontal", "ThreeValueVertical", 0 };
        props.add (new ChoicePropertyComponent (state.getPropertyAsValue ("type", document.getUndoManager()), "Type", StringArray (types)));

        const char* const textBoxPositions[] = { "NoTextBox", "TextBoxLeft", "TextBoxRight", "TextBoxAbove", "TextBoxBelow", 0 };
        props.add (new ChoicePropertyComponent (state.getPropertyAsValue ("textBoxPos", document.getUndoManager()), "Text Box", StringArray (textBoxPositions)));

        props.add (new BooleanPropertyComponent (getValue ("editable", state, document), "Editable", "Value can be edited"));
        props.add (new TextPropertyComponent (Value (new NumericValueSource<int> (getValue ("textBoxWidth", state, document))), "Text Box Width", 8, false));
        props.add (new TextPropertyComponent (Value (new NumericValueSource<int> (getValue ("textBoxHeight", state, document))), "Text Box Height", 8, false));

        props.add (new TextPropertyComponent (Value (new NumericValueSource<double> (getValue ("skew", state, document))), "Skew Factor", 16, false));
        addEditableColourProperties (document, state, props);
    }
};

#endif

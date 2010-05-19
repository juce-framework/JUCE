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
        item.set (Ids::type, Slider::LinearHorizontal);
        item.set (Ids::min, 0);
        item.set (Ids::max, 100);
        item.set (Ids::interval, 1);
        item.set (Ids::textBoxPos, 2);
        item.set (Ids::editable, true);
        item.set (Ids::textBoxWidth, 80);
        item.set (Ids::textBoxHeight, 20);
        item.set (Ids::skew, 1);
    }

    void update (ComponentTypeInstance& item, Slider* comp)
    {
        comp->setRange ((double) item [Ids::min], (double) item [Ids::max], (double) item [Ids::interval]);
        comp->setSliderStyle ((Slider::SliderStyle) (int) item [Ids::type]);
        comp->setTextBoxStyle ((Slider::TextEntryBoxPosition) (int) item [Ids::textBoxPos],
                               ! (bool) item [Ids::editable],
                               (int) item [Ids::textBoxWidth], (int) item [Ids::textBoxHeight]);
        comp->setSkewFactor ((double) item [Ids::skew]);
    }

    void createProperties (ComponentTypeInstance& item, Array <PropertyComponent*>& props)
    {
        item.addTooltipProperty (props);
        item.addFocusOrderProperty (props);

        props.add (new TextPropertyComponent (Value (new NumericValueSource<double> (item.getValue (Ids::min))), "Minimum", 16, false));
        props.add (new TextPropertyComponent (Value (new NumericValueSource<double> (item.getValue (Ids::max))), "Maximum", 16, false));
        props.add (new TextPropertyComponent (Value (new NumericValueSource<double> (item.getValue (Ids::interval))), "Interval", 16, false));

        const char* const types[] = { "LinearHorizontal", "LinearVertical", "LinearBar", "Rotary", "RotaryHorizontalDrag", "RotaryVerticalDrag",
                                      "IncDecButtons", "TwoValueHorizontal", "TwoValueVertical", "ThreeValueHorizontal", "ThreeValueVertical", 0 };
        const int typeValues[] = { (int) Slider::LinearHorizontal, (int) Slider::LinearVertical, (int) Slider::LinearBar, (int) Slider::Rotary,
                                   (int) Slider::RotaryHorizontalDrag, (int) Slider::RotaryVerticalDrag, (int) Slider::IncDecButtons, (int) Slider::TwoValueHorizontal,
                                   (int) Slider::TwoValueVertical, (int) Slider::ThreeValueHorizontal, (int) Slider::ThreeValueVertical };
        props.add (new ChoicePropertyComponent (item.getValue (Ids::type), "Type", StringArray (types), Array<var> (typeValues, numElementsInArray (typeValues))));

        const char* const textBoxPositions[] = { "NoTextBox", "TextBoxLeft", "TextBoxRight", "TextBoxAbove", "TextBoxBelow", 0 };
        const int positionValues[] = { (int) Slider::NoTextBox, (int) Slider::TextBoxLeft, (int) Slider::TextBoxRight, (int) Slider::TextBoxAbove, (int) Slider::TextBoxBelow };
        props.add (new ChoicePropertyComponent (item.getValue (Ids::textBoxPos), "Text Box", StringArray (textBoxPositions), Array<var> (positionValues, numElementsInArray (positionValues))));

        props.add (new BooleanPropertyComponent (item.getValue (Ids::editable), "Editable", "Value can be edited"));
        props.add (new TextPropertyComponent (Value (new NumericValueSource<int> (item.getValue (Ids::textBoxWidth))), "Text Box Width", 8, false));
        props.add (new TextPropertyComponent (Value (new NumericValueSource<int> (item.getValue (Ids::textBoxHeight))), "Text Box Height", 8, false));

        props.add (new TextPropertyComponent (Value (new NumericValueSource<double> (item.getValue (Ids::skew))), "Skew Factor", 16, false));
        addEditableColourProperties (item, props);
    }

    static const char* sliderTypeString (int type) throw()
    {
        switch (type)
        {
            case Slider::LinearHorizontal:      return "LinearHorizontal";
            case Slider::LinearVertical:        return "LinearVertical";
            case Slider::LinearBar:             return "LinearBar";
            case Slider::Rotary:                return "Rotary";
            case Slider::RotaryHorizontalDrag:  return "RotaryHorizontalDrag";
            case Slider::RotaryVerticalDrag:    return "RotaryVerticalDrag";
            case Slider::IncDecButtons:         return "IncDecButtons";
            case Slider::TwoValueHorizontal:    return "TwoValueHorizontal";
            case Slider::TwoValueVertical:      return "TwoValueVertical";
            case Slider::ThreeValueHorizontal:  return "ThreeValueHorizontal";
            case Slider::ThreeValueVertical:    return "ThreeValueVertical";
            default:                            jassertfalse; break;
        }

        return "";
    }

    static const char* sliderTextBoxString (int type) throw()
    {
        switch (type)
        {
            case Slider::NoTextBox:     return "NoTextBox";
            case Slider::TextBoxLeft:   return "TextBoxLeft";
            case Slider::TextBoxRight:  return "TextBoxRight";
            case Slider::TextBoxAbove:  return "TextBoxAbove";
            case Slider::TextBoxBelow:  return "TextBoxBelow";
            default:                    jassertfalse; break;
        }

        return "";
    }

    void createCode (ComponentTypeInstance& item, CodeGenerator& code)
    {
        code.constructorCode
            << item.createConstructorStatement (CodeHelpers::stringLiteral (item.getComponentName()))
            << item.getMemberName() << "->setRange (" << CodeHelpers::doubleLiteral (item [Ids::min])
                                    << ", " << CodeHelpers::doubleLiteral (item [Ids::max])
                                    << ", " << CodeHelpers::doubleLiteral (item [Ids::interval])
                                    << ");" << newLine
            << item.getMemberName() << "->setSliderStyle (Slider::" << sliderTypeString ((int) item [Ids::type]) << ");" << newLine
            << item.getMemberName() << "->setTextBoxStyle (Slider::" << sliderTextBoxString ((int) item [Ids::type])
                                    << ", " << CodeHelpers::boolLiteral (! (bool) item [Ids::editable])
                                    << ", " << String ((int) item [Ids::textBoxWidth])
                                    << ", " << String ((int) item [Ids::textBoxHeight])
                                    << ");" << newLine;

        double skew = (double) item [Ids::skew];
        if (skew != 1.0 && skew != 0)
            code.constructorCode << item.getMemberName() << "->setSkewFactor (" << CodeHelpers::doubleLiteral (skew) << ");" << newLine;
    }
};

#endif

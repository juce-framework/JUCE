/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

SliderPropertyComponent::SliderPropertyComponent (const String& name,
                                                  const double rangeMin,
                                                  const double rangeMax,
                                                  const double interval,
                                                  const double skewFactor,
                                                  bool symmetricSkew)
    : PropertyComponent (name)
{
    addAndMakeVisible (slider);

    slider.setRange (rangeMin, rangeMax, interval);
    slider.setSkewFactor (skewFactor, symmetricSkew);
    slider.setSliderStyle (Slider::LinearBar);

    slider.onValueChange = [this]
    {
        if (getValue() != slider.getValue())
            setValue (slider.getValue());
    };
}

SliderPropertyComponent::SliderPropertyComponent (const Value& valueToControl,
                                                  const String& name,
                                                  const double rangeMin,
                                                  const double rangeMax,
                                                  const double interval,
                                                  const double skewFactor,
                                                  bool symmetricSkew)
    : PropertyComponent (name)
{
    addAndMakeVisible (slider);

    slider.setRange (rangeMin, rangeMax, interval);
    slider.setSkewFactor (skewFactor, symmetricSkew);
    slider.setSliderStyle (Slider::LinearBar);

    slider.getValueObject().referTo (valueToControl);
}

SliderPropertyComponent::~SliderPropertyComponent()
{
}

void SliderPropertyComponent::setValue (const double /*newValue*/)
{
}

double SliderPropertyComponent::getValue() const
{
    return slider.getValue();
}

void SliderPropertyComponent::refresh()
{
    slider.setValue (getValue(), dontSendNotification);
}

} // namespace juce

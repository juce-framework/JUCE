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

#include "../../../core/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_SliderPropertyComponent.h"


//==============================================================================
SliderPropertyComponent::SliderPropertyComponent (const String& name,
                                                  const double rangeMin,
                                                  const double rangeMax,
                                                  const double interval,
                                                  const double skewFactor)
    : PropertyComponent (name)
{
    addAndMakeVisible (slider = new Slider (name));

    slider->setRange (rangeMin, rangeMax, interval);
    slider->setSkewFactor (skewFactor);
    slider->setSliderStyle (Slider::LinearBar);

    slider->addListener (this);
}

SliderPropertyComponent::SliderPropertyComponent (Value& valueToControl,
                                                  const String& name,
                                                  const double rangeMin,
                                                  const double rangeMax,
                                                  const double interval,
                                                  const double skewFactor)
    : PropertyComponent (name)
{
    addAndMakeVisible (slider = new Slider (name));

    slider->setRange (rangeMin, rangeMax, interval);
    slider->setSkewFactor (skewFactor);
    slider->setSliderStyle (Slider::LinearBar);

    slider->getValueObject().referTo (valueToControl);
}

SliderPropertyComponent::~SliderPropertyComponent()
{
    deleteAllChildren();
}

void SliderPropertyComponent::setValue (const double /*newValue*/)
{
}

const double SliderPropertyComponent::getValue() const
{
    return slider->getValue();
}

void SliderPropertyComponent::refresh()
{
    slider->setValue (getValue(), false);
}

void SliderPropertyComponent::sliderValueChanged (Slider*)
{
    if (getValue() != slider->getValue())
        setValue (slider->getValue());
}


END_JUCE_NAMESPACE

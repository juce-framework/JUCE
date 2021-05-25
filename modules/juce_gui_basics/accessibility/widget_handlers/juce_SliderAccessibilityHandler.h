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

/** Basic accessible interface for a Slider.

    @tags{Accessibility}
*/
class JUCE_API  SliderAccessibilityHandler  : public AccessibilityHandler
{
public:
    explicit SliderAccessibilityHandler (Slider& sliderToWrap)
        : AccessibilityHandler (sliderToWrap,
                                AccessibilityRole::slider,
                                {},
                                { std::make_unique<SliderValueInterface> (sliderToWrap) })
    {
    }

private:
    class SliderValueInterface  : public AccessibilityValueInterface
    {
    public:
        explicit SliderValueInterface (Slider& sliderToWrap)
            : slider (sliderToWrap)
        {
        }

        bool isReadOnly() const override  { return false; }

        double getCurrentValue() const override
        {
            return slider.isTwoValue() ? slider.getMaxValue() : slider.getValue();
        }

        void setValue (double newValue) override
        {
            if (slider.isTwoValue())
                slider.setMaxValue (newValue, sendNotification);
            else
                slider.setValue (newValue, sendNotification);
        }

        String getCurrentValueAsString() const override
        {
            return slider.getTextFromValue (getCurrentValue());
        }

        void setValueAsString (const String& newValue) override
        {
            setValue (slider.getValueFromText (newValue));
        }

        AccessibleValueRange getRange() const override
        {
            return { { slider.getMinimum(), slider.getMaximum() },
                     getStepSize() };
        }

    private:
        double getStepSize() const
        {
            auto interval = slider.getInterval();

            return interval != 0.0 ? interval
                                   : slider.proportionOfLengthToValue (0.01);
        }

        Slider& slider;
    };

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SliderAccessibilityHandler)
};

} // namespace juce

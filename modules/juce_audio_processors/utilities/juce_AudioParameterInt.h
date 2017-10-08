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

namespace juce
{

/**
    Provides a class of AudioProcessorParameter that can be used as an
    integer value with a given range.

    @see AudioParameterFloat, AudioParameterBool, AudioParameterChoice
*/
class JUCE_API  AudioParameterInt  : public AudioProcessorParameterWithID
{
public:
    /** Creates an AudioParameterInt with an ID, name, and range.
        Note that the min and max range values are inclusive.
        On creation, its value is set to the default value.
    */
    AudioParameterInt (const String& parameterID, const String& name,
                       int minValue, int maxValue,
                       int defaultValue,
                       const String& label = String());

    /** Destructor. */
    ~AudioParameterInt();

    /** Returns the parameter's current value as an integer. */
    int get() const noexcept                    { return roundToInt (value); }
    /** Returns the parameter's current value as an integer. */
    operator int() const noexcept               { return get(); }

    /** Changes the parameter's current value to a new integer.
        The value passed in will be snapped to the permitted range before being used.
    */
    AudioParameterInt& operator= (int newValue);

    /** Returns the parameter's range. */
    Range<int> getRange() const noexcept        { return Range<int> (minValue, maxValue); }


private:
    //==============================================================================
    const int minValue, maxValue, rangeOfValues;
    float value, defaultValue;

    float getValue() const override;
    void setValue (float newValue) override;
    float getDefaultValue() const override;
    int getNumSteps() const override;
    String getText (float, int) const override;
    float getValueForText (const String&) const override;

    int limitRange (int) const noexcept;
    float convertTo0to1 (int) const noexcept;
    int convertFrom0to1 (float) const noexcept;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioParameterInt)
};

} // namespace juce

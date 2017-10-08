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
    A subclass of AudioProcessorParameter that provides an easy way to create a
    parameter which maps onto a given NormalisableRange.

    @see AudioParameterInt, AudioParameterBool, AudioParameterChoice
*/
class JUCE_API  AudioParameterFloat  : public AudioProcessorParameterWithID
{
public:
    /** Creates a AudioParameterFloat with an ID, name, and range.
        On creation, its value is set to the default value.
    */
    AudioParameterFloat (const String& parameterID, const String& name,
                         NormalisableRange<float> normalisableRange,
                         float defaultValue,
                         const String& label = String(),
                         Category category = AudioProcessorParameter::genericParameter);

    /** Creates a AudioParameterFloat with an ID, name, and range.
        On creation, its value is set to the default value.
        For control over skew factors, you can use the other
        constructor and provide a NormalisableRange.
    */
    AudioParameterFloat (String parameterID, String name,
                         float minValue,
                         float maxValue,
                         float defaultValue);

    /** Destructor. */
    ~AudioParameterFloat();

    /** Returns the parameter's current value. */
    float get() const noexcept                  { return value; }
    /** Returns the parameter's current value. */
    operator float() const noexcept             { return value; }

    /** Changes the parameter's current value. */
    AudioParameterFloat& operator= (float newValue);

    /** Provides access to the parameter's range. */
    NormalisableRange<float> range;


private:
    //==============================================================================
    float value, defaultValue;

    float getValue() const override;
    void setValue (float newValue) override;
    float getDefaultValue() const override;
    int getNumSteps() const override;
    String getText (float, int) const override;
    float getValueForText (const String&) const override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioParameterFloat)
};

} // namespace juce

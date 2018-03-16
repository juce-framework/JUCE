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

    @tags{Audio}
*/
class JUCE_API  AudioParameterFloat  : public AudioProcessorParameterWithID
{
public:
    /** Creates a AudioParameterFloat with the specified parameters.

        @param parameterID         The parameter ID to use
        @param name                The parameter name to use
        @param normalisableRange   The NormalisableRange to use
        @param defaultValue        The non-normalised default value
        @param label               An optional label for the parameter's value
        @param category            An optional parameter category
        @param stringFromValue     An optional lambda function that converts a non-normalised
                                   value to a string with a maximum length. This may
                                   be used by hosts to display the parameter's value.
        @param valueFromString     An optional lambda function that parses a string and
                                   converts it into a non-normalised value. Some hosts use
                                   this to allow users to type in parameter values.
    */
    AudioParameterFloat (const String& parameterID, const String& name,
                         NormalisableRange<float> normalisableRange,
                         float defaultValue,
                         const String& label = String(),
                         Category category = AudioProcessorParameter::genericParameter,
                         std::function<String (float value, int maximumStringLength)> stringFromValue = nullptr,
                         std::function<float (const String& text)> valueFromString = nullptr);

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

protected:
    /** Override this method if you are interested in receiving callbacks
        when the parameter value changes.
    */
    virtual void valueChanged (float newValue);

private:
    //==============================================================================
    float getValue() const override;
    void setValue (float newValue) override;
    float getDefaultValue() const override;
    int getNumSteps() const override;
    String getText (float, int) const override;
    float getValueForText (const String&) const override;

    float value;
    const float defaultValue;
    std::function<String (float, int)> stringFromValueFunction;
    std::function<float (const String&)> valueFromStringFunction;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioParameterFloat)
};

} // namespace juce

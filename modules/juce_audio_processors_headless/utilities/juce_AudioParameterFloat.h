/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

/** Properties of an AudioParameterFloat.

    @see AudioParameterFloat(), RangedAudioParameterAttributes()

    @tags{Audio}
*/
class AudioParameterFloatAttributes : public RangedAudioParameterAttributes<AudioParameterFloatAttributes, float> {};

//==============================================================================
/**
    A subclass of AudioProcessorParameter that provides an easy way to create a
    parameter which maps onto a given NormalisableRange.

    @see AudioParameterInt, AudioParameterBool, AudioParameterChoice

    @tags{Audio}
*/
class JUCE_API  AudioParameterFloat  : public RangedAudioParameter
{
public:
    /** Creates a AudioParameterFloat with the specified parameters.

        Note that the attributes argument is optional and only needs to be
        supplied if you want to change options from their default values.

        Example usage:
        @code
        auto attributes = AudioParameterFloatAttributes().withStringFromValueFunction ([] (auto x, auto) { return String (x * 100); })
                                                         .withLabel ("%");
        auto param = std::make_unique<AudioParameterFloat> ("paramID", "Parameter Name", NormalisableRange<float>(), 0.5f, attributes);
        @endcode

        @param parameterID         The parameter ID to use
        @param parameterName       The parameter name to use
        @param normalisableRange   The NormalisableRange to use
        @param defaultValue        The non-normalised default value
        @param attributes          Optional characteristics
    */
    AudioParameterFloat (const ParameterID& parameterID,
                         const String& parameterName,
                         NormalisableRange<float> normalisableRange,
                         float defaultValue,
                         const AudioParameterFloatAttributes& attributes = {});

    /** Creates a AudioParameterFloat with the specified parameters.

        @param parameterID         The parameter ID to use
        @param parameterName       The parameter name to use
        @param normalisableRange   The NormalisableRange to use
        @param defaultValue        The non-normalised default value
        @param parameterLabel      An optional label for the parameter's value
        @param parameterCategory   An optional parameter category
        @param stringFromValue     An optional lambda function that converts a non-normalised
                                   value to a string with a maximum length. This may
                                   be used by hosts to display the parameter's value.
        @param valueFromString     An optional lambda function that parses a string and
                                   converts it into a non-normalised value. Some hosts use
                                   this to allow users to type in parameter values.
    */
    [[deprecated ("Prefer the signature taking an Attributes argument")]]
    AudioParameterFloat (const ParameterID& parameterID,
                         const String& parameterName,
                         NormalisableRange<float> normalisableRange,
                         float defaultValue,
                         const String& parameterLabel,
                         Category parameterCategory = AudioProcessorParameter::genericParameter,
                         std::function<String (float value, int maximumStringLength)> stringFromValue = nullptr,
                         std::function<float (const String& text)> valueFromString = nullptr)
        : AudioParameterFloat (parameterID,
                               parameterName,
                               std::move (normalisableRange),
                               defaultValue,
                               AudioParameterFloatAttributes().withLabel (parameterLabel)
                                                              .withCategory (parameterCategory)
                                                              .withStringFromValueFunction (std::move (stringFromValue))
                                                              .withValueFromStringFunction (std::move (valueFromString)))
    {
    }

    /** Creates a AudioParameterFloat with an ID, name, and range.
        On creation, its value is set to the default value.
        For control over skew factors, you can use the other
        constructor and provide a NormalisableRange.
    */
    AudioParameterFloat (const ParameterID& parameterID,
                         const String& parameterName,
                         float minValue,
                         float maxValue,
                         float defaultValue);

    /** Destructor. */
    ~AudioParameterFloat() override;

    /** Returns the parameter's current value. */
    float get() const noexcept                  { return value; }

    /** Returns the parameter's current value. */
    operator float() const noexcept             { return value; }

    /** Changes the parameter's current value. */
    AudioParameterFloat& operator= (float newValue);

    /** Returns the range of values that the parameter can take. */
    const NormalisableRange<float>& getNormalisableRange() const override   { return range; }

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

    std::atomic<float> value;
    const float valueDefault;
    std::function<String (float, int)> stringFromValueFunction;
    std::function<float (const String&)> valueFromStringFunction;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioParameterFloat)
};

} // namespace juce

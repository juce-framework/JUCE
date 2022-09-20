/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
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

/** Properties of an AudioParameterBool.

    @see AudioParameterBool(), RangedAudioParameterAttributes()
*/
class AudioParameterBoolAttributes : public RangedAudioParameterAttributes<AudioParameterBoolAttributes, bool> {};

//==============================================================================
/**
    Provides a class of AudioProcessorParameter that can be used as a boolean value.

    @see AudioParameterFloat, AudioParameterInt, AudioParameterChoice

    @tags{Audio}
*/
class JUCE_API AudioParameterBool  : public RangedAudioParameter
{
public:
    /** Creates a AudioParameterBool with the specified parameters.

        Note that the attributes argument is optional and only needs to be
        supplied if you want to change options from their default values.

        Example usage:
        @code
        auto attributes = AudioParameterBoolAttributes().withStringFromValueFunction ([] (auto x, auto) { return x ? "On" : "Off"; })
                                                        .withLabel ("enabled");
        auto param = std::make_unique<AudioParameterBool> ("paramID", "Parameter Name", false, attributes);
        @endcode

        @param parameterID         The parameter ID to use
        @param parameterName       The parameter name to use
        @param defaultValue        The default value
        @param attributes          Optional characteristics
    */
    AudioParameterBool (const ParameterID& parameterID,
                        const String& parameterName,
                        bool defaultValue,
                        const AudioParameterBoolAttributes& attributes = {});

    /** Creates a AudioParameterBool with the specified parameters.

        @param parameterID         The parameter ID to use
        @param parameterName       The parameter name to use
        @param defaultValue        The default value
        @param parameterLabel      An optional label for the parameter's value
        @param stringFromBool      An optional lambda function that converts a bool
                                   value to a string with a maximum length. This may
                                   be used by hosts to display the parameter's value.
        @param boolFromString      An optional lambda function that parses a string and
                                   converts it into a bool value. Some hosts use this
                                   to allow users to type in parameter values.
    */
    [[deprecated ("Prefer the signature taking an Attributes argument")]]
    AudioParameterBool (const ParameterID& parameterID,
                        const String& parameterName,
                        bool defaultValue,
                        const String& parameterLabel,
                        std::function<String (bool value, int maximumStringLength)> stringFromBool = nullptr,
                        std::function<bool (const String& text)> boolFromString = nullptr)
        : AudioParameterBool (parameterID,
                              parameterName,
                              defaultValue,
                              AudioParameterBoolAttributes().withLabel (parameterLabel)
                                                            .withStringFromValueFunction (std::move (stringFromBool))
                                                            .withValueFromStringFunction (std::move (boolFromString)))
    {
    }

    /** Destructor. */
    ~AudioParameterBool() override;

    /** Returns the parameter's current boolean value. */
    bool get() const noexcept          { return value >= 0.5f; }

    /** Returns the parameter's current boolean value. */
    operator bool() const noexcept     { return get(); }

    /** Changes the parameter's current value to a new boolean. */
    AudioParameterBool& operator= (bool newValue);

    /** Returns the range of values that the parameter can take. */
    const NormalisableRange<float>& getNormalisableRange() const override   { return range; }

protected:
    /** Override this method if you are interested in receiving callbacks
        when the parameter value changes.
    */
    virtual void valueChanged (bool newValue);

private:
    //==============================================================================
    float getValue() const override;
    void setValue (float newValue) override;
    float getDefaultValue() const override;
    int getNumSteps() const override;
    bool isDiscrete() const override;
    bool isBoolean() const override;
    String getText (float, int) const override;
    float getValueForText (const String&) const override;

    const NormalisableRange<float> range { 0.0f, 1.0f, 1.0f };
    std::atomic<float> value;
    const float valueDefault;
    std::function<String (bool, int)> stringFromBoolFunction;
    std::function<bool (const String&)> boolFromStringFunction;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioParameterBool)
};

} // namespace juce

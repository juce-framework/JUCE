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
    Provides a class of AudioProcessorParameter that can be used as a boolean value.

    @see AudioParameterFloat, AudioParameterInt, AudioParameterChoice

    @tags{Audio}
*/
class JUCE_API AudioParameterBool  : public RangedAudioParameter
{
public:
    /** Creates a AudioParameterBool with the specified parameters.

        @param parameterID         The parameter ID to use
        @param name                The parameter name to use
        @param defaultValue        The default value
        @param label               An optional label for the parameter's value
        @param stringFromBool      An optional lambda function that converts a bool
                                   value to a string with a maximum length. This may
                                   be used by hosts to display the parameter's value.
        @param boolFromString      An optional lambda function that parses a string and
                                   converts it into a bool value. Some hosts use this
                                   to allow users to type in parameter values.
    */
    AudioParameterBool (const String& parameterID, const String& name, bool defaultValue,
                        const String& label = String(),
                        std::function<String (bool value, int maximumStringLength)> stringFromBool = nullptr,
                        std::function<bool (const String& text)> boolFromString = nullptr);

    /** Destructor. */
    ~AudioParameterBool();

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
    float value;
    const float defaultValue;
    std::function<String (bool, int)> stringFromBoolFunction;
    std::function<bool (const String&)> boolFromStringFunction;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioParameterBool)
};

} // namespace juce

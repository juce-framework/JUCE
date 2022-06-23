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

/** Properties of an AudioParameterInt.

    @see AudioParameterInt(), RangedAudioParameterAttributes()
*/
class AudioParameterIntAttributes : public RangedAudioParameterAttributes<AudioParameterIntAttributes, int> {};

//==============================================================================
/**
    Provides a class of AudioProcessorParameter that can be used as an
    integer value with a given range.

    @see AudioParameterFloat, AudioParameterBool, AudioParameterChoice

    @tags{Audio}
*/
class JUCE_API  AudioParameterInt  : public RangedAudioParameter
{
public:
    /** Creates a AudioParameterInt with the specified parameters.

        Note that the attributes argument is optional and only needs to be
        supplied if you want to change options from their default values.

        Example usage:
        @code
        auto attributes = AudioParameterIntAttributes().withStringFromValueFunction ([] (auto x, auto) { return String (x); })
                                                       .withLabel ("things");
        auto param = std::make_unique<AudioParameterInt> ("paramID", "Parameter Name", 0, 100, 50, attributes);
        @endcode

        @param parameterID         The parameter ID to use
        @param parameterName       The parameter name to use
        @param minValue            The minimum parameter value
        @param maxValue            The maximum parameter value
        @param defaultValue        The default value
        @param attributes          Optional characteristics
    */
    AudioParameterInt (const ParameterID& parameterID,
                       const String& parameterName,
                       int minValue,
                       int maxValue,
                       int defaultValue,
                       const AudioParameterIntAttributes& attributes = {});

    /** Creates a AudioParameterInt with the specified parameters.

        @param parameterID         The parameter ID to use
        @param parameterName       The parameter name to use
        @param minValue            The minimum parameter value
        @param maxValue            The maximum parameter value
        @param defaultValueIn      The default value
        @param parameterLabel      An optional label for the parameter's value
        @param stringFromInt       An optional lambda function that converts a int
                                   value to a string with a maximum length. This may
                                   be used by hosts to display the parameter's value.
        @param intFromString       An optional lambda function that parses a string
                                   and converts it into an int. Some hosts use this
                                   to allow users to type in parameter values.
    */
    [[deprecated ("Prefer the signature taking an Attributes argument")]]
    AudioParameterInt (const ParameterID& parameterID,
                       const String& parameterName,
                       int minValue,
                       int maxValue,
                       int defaultValueIn,
                       const String& parameterLabel,
                       std::function<String (int value, int maximumStringLength)> stringFromInt = nullptr,
                       std::function<int (const String& text)> intFromString = nullptr)
        : AudioParameterInt (parameterID,
                             parameterName,
                             minValue,
                             maxValue,
                             defaultValueIn,
                             AudioParameterIntAttributes().withLabel (parameterLabel)
                                                          .withStringFromValueFunction (std::move (stringFromInt))
                                                          .withValueFromStringFunction (std::move (intFromString)))
    {
    }

    /** Destructor. */
    ~AudioParameterInt() override;

    /** Returns the parameter's current value as an integer. */
    int get() const noexcept                    { return roundToInt (value.load()); }

    /** Returns the parameter's current value as an integer. */
    operator int() const noexcept               { return get(); }

    /** Changes the parameter's current value to a new integer.
        The value passed in will be snapped to the permitted range before being used.
    */
    AudioParameterInt& operator= (int newValue);

    /** Returns the parameter's range. */
    Range<int> getRange() const noexcept        { return { (int) getNormalisableRange().start, (int) getNormalisableRange().end }; }

    /** Returns the range of values that the parameter can take. */
    const NormalisableRange<float>& getNormalisableRange() const override   { return range; }

protected:
    /** Override this method if you are interested in receiving callbacks
        when the parameter value changes.
    */
    virtual void valueChanged (int newValue);

private:
    //==============================================================================
    float getValue() const override;
    void setValue (float newValue) override;
    float getDefaultValue() const override;
    int getNumSteps() const override;
    String getText (float, int) const override;
    float getValueForText (const String&) const override;

    const NormalisableRange<float> range;
    std::atomic<float> value;
    const float defaultValue;
    std::function<String (int, int)> stringFromIntFunction;
    std::function<int (const String&)> intFromStringFunction;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioParameterInt)
};

} // namespace juce

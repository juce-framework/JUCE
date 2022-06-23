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

/**
    @internal

    Holds common attributes of audio parameters.

    CRTP is used here because we want the Attributes types for each parameter
    (Float, Bool, Choice, Int) to be distinct and extensible in the future.
    i.e. the identifiers AudioParameterFloatAttributes and RangedAudioParameterAttributes<float>
    should not be interchangable because we might need to add float-specific attributes in
    the future. Users should not refer directly to RangedAudioParameterAttributes.
*/
template <typename Derived, typename Value>
class RangedAudioParameterAttributes
{
    using This = RangedAudioParameterAttributes;

public:
    using Category = AudioProcessorParameter::Category;

    using StringFromValue = std::function<String (Value, int)>;
    using ValueFromString = std::function<Value (const String&)>;

    /** An optional lambda function that converts a non-normalised value to a string with a maximum length. This may be used by hosts to display the parameter's value. */
    JUCE_NODISCARD auto withStringFromValueFunction (StringFromValue x)                       const { return withMember (asDerived(), &Derived::stringFromValue, std::move (x)); }

    /** An optional lambda function that parses a string and converts it into a non-normalised value. Some hosts use this to allow users to type in parameter values. */
    JUCE_NODISCARD auto withValueFromStringFunction (ValueFromString x)                       const { return withMember (asDerived(), &Derived::valueFromString, std::move (x)); }

    /** See AudioProcessorParameterWithIDAttributes::withLabel() */
    JUCE_NODISCARD auto withLabel (String x)                                                  const { return withMember (asDerived(), &Derived::attributes, attributes.withLabel (std::move (x))); }

    /** See AudioProcessorParameterWithIDAttributes::withCategory() */
    JUCE_NODISCARD auto withCategory (Category x)                                             const { return withMember (asDerived(), &Derived::attributes, attributes.withCategory (std::move (x))); }

    /** See AudioProcessorParameter::isMetaParameter() */
    JUCE_NODISCARD auto withMeta (bool x)                                                     const { return withMember (asDerived(), &Derived::attributes, attributes.withMeta (std::move (x))); }

    /** See AudioProcessorParameter::isAutomatable() */
    JUCE_NODISCARD auto withAutomatable (bool x)                                              const { return withMember (asDerived(), &Derived::attributes, attributes.withAutomatable (std::move (x))); }

    /** See AudioProcessorParameter::isOrientationInverted() */
    JUCE_NODISCARD auto withInverted (bool x)                                                 const { return withMember (asDerived(), &Derived::attributes, attributes.withInverted (std::move (x))); }

    /** An optional lambda function that converts a non-normalised value to a string with a maximum length. This may be used by hosts to display the parameter's value. */
    JUCE_NODISCARD const auto& getStringFromValueFunction()                                   const { return stringFromValue; }

    /** An optional lambda function that parses a string and converts it into a non-normalised value. Some hosts use this to allow users to type in parameter values. */
    JUCE_NODISCARD const auto& getValueFromStringFunction()                                   const { return valueFromString; }

    /** Gets attributes that would also apply to an AudioProcessorParameterWithID */
    JUCE_NODISCARD const auto& getAudioProcessorParameterWithIDAttributes()                   const { return attributes; }

private:
    auto& asDerived() const { return *static_cast<const Derived*> (this); }

    AudioProcessorParameterWithIDAttributes attributes;
    StringFromValue stringFromValue;
    ValueFromString valueFromString;
};

//==============================================================================
/**
    This abstract base class is used by some AudioProcessorParameter helper classes.

    @see AudioParameterFloat, AudioParameterInt, AudioParameterBool, AudioParameterChoice

    @tags{Audio}
*/
class JUCE_API RangedAudioParameter   : public AudioProcessorParameterWithID
{
public:
    using AudioProcessorParameterWithID::AudioProcessorParameterWithID;

    /** Returns the range of values that the parameter can take. */
    virtual const NormalisableRange<float>& getNormalisableRange() const = 0;

    /** Returns the number of steps for this parameter based on the normalisable range's interval.
        If you are using lambda functions to define the normalisable range's snapping behaviour
        then you should override this function so that it returns the number of snapping points.
    */
    int getNumSteps() const override;

    /** Normalises and snaps a value based on the normalisable range. */
    float convertTo0to1 (float v) const noexcept;

    /** Denormalises and snaps a value based on the normalisable range. */
    float convertFrom0to1 (float v) const noexcept;
};

} // namespace juce

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

/**
    @internal

    Holds common attributes of audio parameters.

    CRTP is used here because we want the Attributes types for each parameter
    (Float, Bool, Choice, Int) to be distinct and extensible in the future.
    i.e. the identifiers AudioParameterFloatAttributes and RangedAudioParameterAttributes<float>
    should not be interchangable because we might need to add float-specific attributes in
    the future. Users should not refer directly to RangedAudioParameterAttributes.

    @tags{Audio}
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
    [[nodiscard]] auto withStringFromValueFunction (StringFromValue x)                       const { return withMember (asDerived(), &Derived::stringFromValue, std::move (x)); }

    /** An optional lambda function that parses a string and converts it into a non-normalised value. Some hosts use this to allow users to type in parameter values. */
    [[nodiscard]] auto withValueFromStringFunction (ValueFromString x)                       const { return withMember (asDerived(), &Derived::valueFromString, std::move (x)); }

    /** See AudioProcessorParameterWithIDAttributes::withLabel() */
    [[nodiscard]] auto withLabel (String x)                                                  const { return withMember (asDerived(), &Derived::attributes, attributes.withLabel (std::move (x))); }

    /** See AudioProcessorParameterWithIDAttributes::withCategory() */
    [[nodiscard]] auto withCategory (Category x)                                             const { return withMember (asDerived(), &Derived::attributes, attributes.withCategory (std::move (x))); }

    /** See AudioProcessorParameter::isMetaParameter() */
    [[nodiscard]] auto withMeta (bool x)                                                     const { return withMember (asDerived(), &Derived::attributes, attributes.withMeta (std::move (x))); }

    /** See AudioProcessorParameter::isAutomatable() */
    [[nodiscard]] auto withAutomatable (bool x)                                              const { return withMember (asDerived(), &Derived::attributes, attributes.withAutomatable (std::move (x))); }

    /** See AudioProcessorParameter::isOrientationInverted() */
    [[nodiscard]] auto withInverted (bool x)                                                 const { return withMember (asDerived(), &Derived::attributes, attributes.withInverted (std::move (x))); }

    /** An optional lambda function that converts a non-normalised value to a string with a maximum length. This may be used by hosts to display the parameter's value. */
    [[nodiscard]] const auto& getStringFromValueFunction()                                   const { return stringFromValue; }

    /** An optional lambda function that parses a string and converts it into a non-normalised value. Some hosts use this to allow users to type in parameter values. */
    [[nodiscard]] const auto& getValueFromStringFunction()                                   const { return valueFromString; }

    /** Gets attributes that would also apply to an AudioProcessorParameterWithID */
    [[nodiscard]] const auto& getAudioProcessorParameterWithIDAttributes()                   const { return attributes; }

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

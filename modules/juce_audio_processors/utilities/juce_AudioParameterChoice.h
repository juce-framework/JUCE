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

/** Properties of an AudioParameterChoice.

    @see AudioParameterChoice(), RangedAudioParameterAttributes()

    @tags{Audio}
*/
class AudioParameterChoiceAttributes : public RangedAudioParameterAttributes<AudioParameterChoiceAttributes, int> {};

//==============================================================================
/**
    Provides a class of AudioProcessorParameter that can be used to select
    an indexed, named choice from a list.

    @see AudioParameterFloat, AudioParameterInt, AudioParameterBool

    @tags{Audio}
*/
class JUCE_API  AudioParameterChoice  : public RangedAudioParameter
{
public:
    /** Creates a AudioParameterChoice with the specified parameters.

        Note that the attributes argument is optional and only needs to be
        supplied if you want to change options from their default values.

        Example usage:
        @code
        auto attributes = AudioParameterChoiceAttributes().withLabel ("selected");
        auto param = std::make_unique<AudioParameterChoice> ("paramID", "Parameter Name", StringArray { "a", "b", "c" }, 0, attributes);
        @endcode

        @param parameterID         The parameter ID to use
        @param parameterName       The parameter name to use
        @param choices             The set of choices to use
        @param defaultItemIndex    The index of the default choice
        @param attributes          Optional characteristics
    */
    AudioParameterChoice (const ParameterID& parameterID,
                          const String& parameterName,
                          const StringArray& choices,
                          int defaultItemIndex,
                          const AudioParameterChoiceAttributes& attributes = {});

    /** Creates a AudioParameterChoice with the specified parameters.

        @param parameterID         The parameter ID to use
        @param parameterName       The parameter name to use
        @param choicesToUse        The set of choices to use
        @param defaultItemIndex    The index of the default choice
        @param parameterLabel      An optional label for the parameter's value
        @param stringFromIndex     An optional lambda function that converts a choice
                                   index to a string with a maximum length. This may
                                   be used by hosts to display the parameter's value.
        @param indexFromString     An optional lambda function that parses a string and
                                   converts it into a choice index. Some hosts use this
                                   to allow users to type in parameter values.
    */
    [[deprecated ("Prefer the signature taking an Attributes argument")]]
    AudioParameterChoice (const ParameterID& parameterID,
                          const String& parameterName,
                          const StringArray& choicesToUse,
                          int defaultItemIndex,
                          const String& parameterLabel,
                          std::function<String (int index, int maximumStringLength)> stringFromIndex = nullptr,
                          std::function<int (const String& text)> indexFromString = nullptr)
        : AudioParameterChoice (parameterID,
                                parameterName,
                                choicesToUse,
                                defaultItemIndex,
                                AudioParameterChoiceAttributes().withLabel (parameterLabel)
                                                                .withStringFromValueFunction (std::move (stringFromIndex))
                                                                .withValueFromStringFunction (std::move (indexFromString)))
    {
    }

    /** Destructor. */
    ~AudioParameterChoice() override;

    /** Returns the current index of the selected item. */
    int getIndex() const noexcept                   { return roundToInt (value.load()); }

    /** Returns the current index of the selected item. */
    operator int() const noexcept                   { return getIndex(); }

    /** Returns the name of the currently selected item. */
    String getCurrentChoiceName() const noexcept    { return choices[getIndex()]; }

    /** Returns the name of the currently selected item. */
    operator String() const noexcept                { return getCurrentChoiceName(); }

    /** Changes the selected item to a new index. */
    AudioParameterChoice& operator= (int newValue);

    /** Returns the range of values that the parameter can take. */
    const NormalisableRange<float>& getNormalisableRange() const override   { return range; }

    /** Provides access to the list of choices that this parameter is working with. */
    const StringArray choices;

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
    bool isDiscrete() const override;
    String getText (float, int) const override;
    float getValueForText (const String&) const override;

    const NormalisableRange<float> range;
    std::atomic<float> value;
    const float defaultValue;
    std::function<String (int, int)> stringFromIndexFunction;
    std::function<int (const String&)> indexFromStringFunction;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioParameterChoice)
};

} // namespace juce

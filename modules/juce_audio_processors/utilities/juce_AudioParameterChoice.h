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
    Provides a class of AudioProcessorParameter that can be used to select
    an indexed, named choice from a list.

    @see AudioParameterFloat, AudioParameterInt, AudioParameterBool

    @tags{Audio}
*/
class JUCE_API  AudioParameterChoice  : public RangedAudioParameter
{
public:
    /** Creates a AudioParameterChoice with the specified parameters.

        @param parameterID         The parameter ID to use
        @param name                The parameter name to use
        @param choices             The set of choices to use
        @param defaultItemIndex    The index of the default choice
        @param label               An optional label for the parameter's value
        @param stringFromIndex     An optional lambda function that converts a choice
                                   index to a string with a maximum length. This may
                                   be used by hosts to display the parameter's value.
        @param indexFromString     An optional lambda function that parses a string and
                                   converts it into a choice index. Some hosts use this
                                   to allow users to type in parameter values.
    */
    AudioParameterChoice (const String& parameterID, const String& name,
                          const StringArray& choices,
                          int defaultItemIndex,
                          const String& label = String(),
                          std::function<String (int index, int maximumStringLength)> stringFromIndex = nullptr,
                          std::function<int (const String& text)> indexFromString = nullptr);

    /** Destructor. */
    ~AudioParameterChoice() override;

    /** Returns the current index of the selected item. */
    int getIndex() const noexcept                   { return roundToInt (value); }

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
    float value;
    const float defaultValue;
    std::function<String (int, int)> stringFromIndexFunction;
    std::function<int (const String&)> indexFromStringFunction;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioParameterChoice)
};

} // namespace juce

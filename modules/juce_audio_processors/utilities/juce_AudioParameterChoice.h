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
*/
class JUCE_API  AudioParameterChoice  : public AudioProcessorParameterWithID
{
public:
    /** Creates a AudioParameterChoice with an ID, name, and list of items.
        On creation, its value is set to the default index.
    */
    AudioParameterChoice (const String& parameterID, const String& name,
                          const StringArray& choices,
                          int defaultItemIndex,
                          const String& label = String());

    /** Destructor. */
    ~AudioParameterChoice();

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

    /** Provides access to the list of choices that this parameter is working with. */
    const StringArray choices;


private:
    //==============================================================================
    float value, defaultValue;

    float getValue() const override;
    void setValue (float newValue) override;
    float getDefaultValue() const override;
    int getNumSteps() const override;
    bool isDiscrete() const override;
    String getText (float, int) const override;
    float getValueForText (const String&) const override;

    int limitRange (int) const noexcept;
    float convertTo0to1 (int) const noexcept;
    int convertFrom0to1 (float) const noexcept;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioParameterChoice)
};

} // namespace juce

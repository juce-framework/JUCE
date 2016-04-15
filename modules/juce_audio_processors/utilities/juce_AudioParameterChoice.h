/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

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
    AudioParameterChoice (String parameterID, String name,
                          const StringArray& choices,
                          int defaultItemIndex);

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
    String getText (float, int) const override;
    float getValueForText (const String&) const override;

    int limitRange (int) const noexcept;
    float convertTo0to1 (int) const noexcept;
    int convertFrom0to1 (float) const noexcept;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioParameterChoice)
};

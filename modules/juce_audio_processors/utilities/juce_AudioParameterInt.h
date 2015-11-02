/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

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
    Provides a class of AudioProcessorParameter that can be used as an
    integer value with a given range.

    @see AudioParameterFloat, AudioParameterBool, AudioParameterChoice
*/
class JUCE_API  AudioParameterInt  : public AudioProcessorParameterWithID
{
public:
    /** Creates an AudioParameterInt with an ID, name, and range.
        Note that the min and max range values are inclusive.
        On creation, its value is set to the default value.
    */
    AudioParameterInt (String parameterID, String name,
                       int minValue, int maxValue,
                       int defaultValue);

    /** Destructor. */
    ~AudioParameterInt();

    /** Returns the parameter's current value as an integer. */
    int get() const noexcept                    { return roundToInt (value); }
    /** Returns the parameter's current value as an integer. */
    operator int() const noexcept               { return get(); }

    /** Changes the parameter's current value to a new integer.
        The value passed in will be snapped to the permitted range before being used.
    */
    AudioParameterInt& operator= (int newValue);

    /** Returns the parameter's range. */
    Range<int> getRange() const noexcept        { return Range<int> (minValue, maxValue); }


private:
    //==============================================================================
    int minValue, maxValue;
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

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioParameterInt)
};

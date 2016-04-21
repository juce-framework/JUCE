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
    A subclass of AudioProcessorParameter that provides an easy way to create a
    parameter which maps onto a given NormalisableRange.

    @see AudioParameterInt, AudioParameterBool, AudioParameterChoice
*/
class JUCE_API  AudioParameterFloat  : public AudioProcessorParameterWithID
{
public:
    /** Creates a AudioParameterFloat with an ID, name, and range.
        On creation, its value is set to the default value.
    */
    AudioParameterFloat (String parameterID, String name,
                         NormalisableRange<float> normalisableRange,
                         float defaultValue);

    /** Creates a AudioParameterFloat with an ID, name, and range.
        On creation, its value is set to the default value.
        For control over skew factors, you can use the other
        constructor and provide a NormalisableRange.
    */
    AudioParameterFloat (String parameterID, String name,
                         float minValue,
                         float maxValue,
                         float defaultValue);

    /** Destructor. */
    ~AudioParameterFloat();

    /** Returns the parameter's current value. */
    float get() const noexcept                  { return value; }
    /** Returns the parameter's current value. */
    operator float() const noexcept             { return value; }

    /** Changes the parameter's current value. */
    AudioParameterFloat& operator= (float newValue);

    /** Provides access to the parameter's range. */
    NormalisableRange<float> range;


private:
    //==============================================================================
    float value, defaultValue;

    float getValue() const override;
    void setValue (float newValue) override;
    float getDefaultValue() const override;
    int getNumSteps() const override;
    String getText (float, int) const override;
    float getValueForText (const String&) const override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioParameterFloat)
};

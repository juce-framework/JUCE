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
    Provides a class of AudioProcessorParameter that can be used as a boolean value.

    @see AudioParameterFloat, AudioParameterInt, AudioParameterChoice
*/
class JUCE_API AudioParameterBool  : public AudioProcessorParameterWithID
{
public:
    /** Creates a AudioParameterBool with an ID and name.
        On creation, its value is set to the default value.
    */
    AudioParameterBool (String parameterID, String name, bool defaultValue);

    /** Destructor. */
    ~AudioParameterBool();

    /** Returns the parameter's current boolean value. */
    bool get() const noexcept          { return value >= 0.5f; }
    /** Returns the parameter's current boolean value. */
    operator bool() const noexcept     { return get(); }

    /** Changes the parameter's current value to a new boolean. */
    AudioParameterBool& operator= (bool newValue);


private:
    //==============================================================================
    float value, defaultValue;

    float getValue() const override;
    void setValue (float newValue) override;
    float getDefaultValue() const override;
    int getNumSteps() const override;
    String getText (float, int) const override;
    float getValueForText (const String&) const override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioParameterBool)
};

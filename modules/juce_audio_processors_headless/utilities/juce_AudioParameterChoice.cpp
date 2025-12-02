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

AudioParameterChoice::AudioParameterChoice (const ParameterID& idToUse,
                                            const String& nameToUse,
                                            const StringArray& c,
                                            int def,
                                            const AudioParameterChoiceAttributes& attributes)
   : RangedAudioParameter (idToUse, nameToUse, attributes.getAudioProcessorParameterWithIDAttributes()),
     choices (c),
     range ([this]
            {
                NormalisableRange<float> rangeWithInterval { 0.0f, (float) choices.size() - 1.0f,
                                                             [] (float, float end, float v) { return jlimit (0.0f, end, v * end); },
                                                             [] (float, float end, float v) { return jlimit (0.0f, 1.0f, v / end); },
                                                             [] (float start, float end, float v) { return (float) roundToInt (juce::jlimit (start, end, v)); } };
                rangeWithInterval.interval = 1.0f;
                return rangeWithInterval;
            }()),
     value ((float) def),
     defaultValue (convertTo0to1 ((float) def)),
     stringFromIndexFunction (attributes.getStringFromValueFunction() != nullptr
                                  ? attributes.getStringFromValueFunction()
                                  : [this] (int index, int) { return choices [index]; }),
     indexFromStringFunction (attributes.getValueFromStringFunction() != nullptr
                                  ? attributes.getValueFromStringFunction()
                                  : [this] (const String& text) { return choices.indexOf (text); })
{
    jassert (choices.size() > 1); // you must supply an actual set of items to choose from!
}

AudioParameterChoice::~AudioParameterChoice()
{
    #if __cpp_lib_atomic_is_always_lock_free
     static_assert (std::atomic<float>::is_always_lock_free,
                    "AudioParameterChoice requires a lock-free std::atomic<float>");
    #endif
}

float AudioParameterChoice::getValue() const                             { return convertTo0to1 (value); }
void AudioParameterChoice::setValue (float newValue)                     { value = convertFrom0to1 (newValue); valueChanged (getIndex()); }
float AudioParameterChoice::getDefaultValue() const                      { return defaultValue; }
int AudioParameterChoice::getNumSteps() const                            { return choices.size(); }
bool AudioParameterChoice::isDiscrete() const                            { return true; }
float AudioParameterChoice::getValueForText (const String& text) const   { return convertTo0to1 ((float) indexFromStringFunction (text)); }
String AudioParameterChoice::getText (float v, int length) const         { return stringFromIndexFunction ((int) convertFrom0to1 (v), length); }
void AudioParameterChoice::valueChanged (int)                            {}

AudioParameterChoice& AudioParameterChoice::operator= (int newValue)
{
    if (getIndex() != newValue)
        setValueNotifyingHost (convertTo0to1 ((float) newValue));

    return *this;
}


//==============================================================================
//==============================================================================
#if JUCE_UNIT_TESTS

struct AudioParameterChoiceTests final : public UnitTest
{
    AudioParameterChoiceTests()
        : UnitTest ("AudioParameterChoice", UnitTestCategories::audioProcessorParameters)
    {}

    void runTest() override
    {
        beginTest ("Three options switches at the correct points");
        {
            AudioParameterChoice choice ({}, {}, { "a", "b", "c" }, {});

            choice.setValueNotifyingHost (0.0f);
            expectEquals (choice.getIndex(), 0);

            choice.setValueNotifyingHost (0.2f);
            expectEquals (choice.getIndex(), 0);

            choice.setValueNotifyingHost (0.3f);
            expectEquals (choice.getIndex(), 1);

            choice.setValueNotifyingHost (0.7f);
            expectEquals (choice.getIndex(), 1);

            choice.setValueNotifyingHost (0.8f);
            expectEquals (choice.getIndex(), 2);

            choice.setValueNotifyingHost (1.0f);
            expectEquals (choice.getIndex(), 2);
        }

        beginTest ("Out-of-bounds input");
        {
            AudioParameterChoice choiceParam ({}, {}, { "a", "b", "c" }, {});

            choiceParam.setValueNotifyingHost (-0.5f);
            expectEquals (choiceParam.getIndex(), 0);

            choiceParam.setValueNotifyingHost (1.5f);
            expectEquals (choiceParam.getIndex(), 2);
        }
    }

};

static AudioParameterChoiceTests audioParameterChoiceTests;

#endif

} // namespace juce

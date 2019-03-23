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

// This file contains the implementations of the various AudioParameter[XYZ] classes..


AudioProcessorParameterWithID::AudioProcessorParameterWithID (const String& idToUse,
                                                              const String& nameToUse,
                                                              const String& labelToUse,
                                                              AudioProcessorParameter::Category categoryToUse)
    : paramID (idToUse), name (nameToUse), label (labelToUse), category (categoryToUse) {}
AudioProcessorParameterWithID::~AudioProcessorParameterWithID() {}

String AudioProcessorParameterWithID::getName (int maximumStringLength) const        { return name.substring (0, maximumStringLength); }
String AudioProcessorParameterWithID::getLabel() const                               { return label; }
AudioProcessorParameter::Category AudioProcessorParameterWithID::getCategory() const { return category; }


//==============================================================================
AudioParameterFloat::AudioParameterFloat (const String& idToUse, const String& nameToUse,
                                          NormalisableRange<float> r, float def,
                                          const String& labelToUse, Category categoryToUse,
                                          std::function<String (float, int)> stringFromValue,
                                          std::function<float (const String&)> valueFromString)
   : RangedAudioParameter (idToUse, nameToUse, labelToUse, categoryToUse),
     range (r), value (def), defaultValue (def),
     stringFromValueFunction (stringFromValue),
     valueFromStringFunction (valueFromString)
{
    if (stringFromValueFunction == nullptr)
    {
        auto numDecimalPlacesToDisplay = [this]
        {
            int numDecimalPlaces = 7;

            if (range.interval != 0.0f)
            {
                if (approximatelyEqual (std::abs (range.interval - (int) range.interval), 0.0f))
                    return 0;

                auto v = std::abs (roundToInt (range.interval * pow (10, numDecimalPlaces)));

                while ((v % 10) == 0 && numDecimalPlaces > 0)
                {
                    --numDecimalPlaces;
                    v /= 10;
                }
            }

            return numDecimalPlaces;
        }();

        stringFromValueFunction = [numDecimalPlacesToDisplay] (float v, int length)
        {
            String asText (v, numDecimalPlacesToDisplay);
            return length > 0 ? asText.substring (0, length) : asText;
        };
    }

    if (valueFromStringFunction == nullptr)
        valueFromStringFunction = [] (const String& text) { return text.getFloatValue(); };
}

AudioParameterFloat::AudioParameterFloat (String pid, String nm, float minValue, float maxValue, float def)
   : AudioParameterFloat (pid, nm, { minValue, maxValue, 0.01f }, def)
{
}

AudioParameterFloat::~AudioParameterFloat() {}

float AudioParameterFloat::getValue() const                              { return convertTo0to1 (value); }
void AudioParameterFloat::setValue (float newValue)                      { value = convertFrom0to1 (newValue); valueChanged (get()); }
float AudioParameterFloat::getDefaultValue() const                       { return convertTo0to1 (defaultValue); }
int AudioParameterFloat::getNumSteps() const                             { return AudioProcessorParameterWithID::getNumSteps(); }
String AudioParameterFloat::getText (float v, int length) const          { return stringFromValueFunction (convertFrom0to1 (v), length); }
float AudioParameterFloat::getValueForText (const String& text) const    { return convertTo0to1 (valueFromStringFunction (text)); }
void AudioParameterFloat::valueChanged (float)                           {}

AudioParameterFloat& AudioParameterFloat::operator= (float newValue)
{
    if (value != newValue)
        setValueNotifyingHost (convertTo0to1 (newValue));

    return *this;
}


//==============================================================================
AudioParameterInt::AudioParameterInt (const String& idToUse, const String& nameToUse,
                                      int minValue, int maxValue, int def,
                                      const String& labelToUse,
                                      std::function<String (int, int)> stringFromInt,
                                      std::function<int (const String&)> intFromString)
   : RangedAudioParameter (idToUse, nameToUse, labelToUse),
     range ((float) minValue, (float) maxValue,
            [](float start, float end, float v) { return jlimit (start, end, v * (end - start) + start); },
            [](float start, float end, float v) { return jlimit (0.0f, 1.0f, (v - start) / (end - start)); },
            [](float start, float end, float v) { return (float) roundToInt (juce::jlimit (start, end, v)); }),
     value ((float) def),
     defaultValue (convertTo0to1 ((float) def)),
     stringFromIntFunction (stringFromInt),
     intFromStringFunction (intFromString)
{
    jassert (minValue < maxValue); // must have a non-zero range of values!

    if (stringFromIntFunction == nullptr)
        stringFromIntFunction = [] (int v, int) { return String (v); };

    if (intFromStringFunction == nullptr)
        intFromStringFunction = [] (const String& text) { return text.getIntValue(); };
}

AudioParameterInt::~AudioParameterInt() {}

float AudioParameterInt::getValue() const                                { return convertTo0to1 (value); }
void AudioParameterInt::setValue (float newValue)                        { value = convertFrom0to1 (newValue); valueChanged (get()); }
float AudioParameterInt::getDefaultValue() const                         { return defaultValue; }
int AudioParameterInt::getNumSteps() const                               { return ((int) getNormalisableRange().getRange().getLength()) + 1; }
float AudioParameterInt::getValueForText (const String& text) const      { return convertTo0to1 ((float) intFromStringFunction (text)); }
String AudioParameterInt::getText (float v, int length) const            { return stringFromIntFunction ((int) convertFrom0to1 (v), length); }
void AudioParameterInt::valueChanged (int)                               {}

AudioParameterInt& AudioParameterInt::operator= (int newValue)
{
    if (get() != newValue)
        setValueNotifyingHost (convertTo0to1 ((float) newValue));

    return *this;
}


//==============================================================================
AudioParameterBool::AudioParameterBool (const String& idToUse, const String& nameToUse,
                                        bool def, const String& labelToUse,
                                        std::function<String (bool, int)> stringFromBool,
                                        std::function<bool (const String&)> boolFromString)
   : RangedAudioParameter (idToUse, nameToUse, labelToUse),
     value (def ? 1.0f : 0.0f),
     defaultValue (value),
     stringFromBoolFunction (stringFromBool),
     boolFromStringFunction (boolFromString)
{
    if (stringFromBoolFunction == nullptr)
        stringFromBoolFunction = [] (bool v, int) { return v ? TRANS("On") : TRANS("Off"); };

    if (boolFromStringFunction == nullptr)
    {
        StringArray onStrings;
        onStrings.add (TRANS("on"));
        onStrings.add (TRANS("yes"));
        onStrings.add (TRANS("true"));

        StringArray offStrings;
        offStrings.add (TRANS("off"));
        offStrings.add (TRANS("no"));
        offStrings.add (TRANS("false"));

        boolFromStringFunction = [onStrings, offStrings] (const String& text)
        {
            String lowercaseText (text.toLowerCase());

            for (auto& testText : onStrings)
                if (lowercaseText == testText)
                    return true;

            for (auto& testText : offStrings)
                if (lowercaseText == testText)
                    return false;

            return text.getIntValue() != 0;
        };
    }
}

AudioParameterBool::~AudioParameterBool() {}

float AudioParameterBool::getValue() const                               { return value; }
void AudioParameterBool::setValue (float newValue)                       { value = newValue; valueChanged (get()); }
float AudioParameterBool::getDefaultValue() const                        { return defaultValue; }
int AudioParameterBool::getNumSteps() const                              { return 2; }
bool AudioParameterBool::isDiscrete() const                              { return true; }
bool AudioParameterBool::isBoolean() const                               { return true; }
void AudioParameterBool::valueChanged (bool)                             {}

float AudioParameterBool::getValueForText (const String& text) const
{
    return boolFromStringFunction (text) ? 1.0f : 0.0f;
}

String AudioParameterBool::getText (float v, int maximumLength) const
{
    return stringFromBoolFunction (v >= 0.5f, maximumLength);
}

AudioParameterBool& AudioParameterBool::operator= (bool newValue)
{
    if (get() != newValue)
        setValueNotifyingHost (newValue ? 1.0f : 0.0f);

    return *this;
}


//==============================================================================
AudioParameterChoice::AudioParameterChoice (const String& idToUse, const String& nameToUse,
                                            const StringArray& c, int def, const String& labelToUse,
                                            std::function<String (int, int)> stringFromIndex,
                                            std::function<int (const String&)> indexFromString)
   : RangedAudioParameter (idToUse, nameToUse, labelToUse), choices (c),
     range (0.0f, choices.size() - 1.0f,
            [](float, float end, float v) { return jlimit (0.0f, end, v * end); },
            [](float, float end, float v) { return jlimit (0.0f, 1.0f, v / end); },
            [](float start, float end, float v) { return (float) roundToInt (juce::jlimit (start, end, v)); }),
     value ((float) def),
     defaultValue (convertTo0to1 ((float) def)),
     stringFromIndexFunction (stringFromIndex),
     indexFromStringFunction (indexFromString)
{
    jassert (choices.size() > 0); // you must supply an actual set of items to choose from!

    if (stringFromIndexFunction == nullptr)
        stringFromIndexFunction = [this] (int index, int) { return choices [index]; };

    if (indexFromStringFunction == nullptr)
        indexFromStringFunction = [this] (const String& text) { return choices.indexOf (text); };
}

AudioParameterChoice::~AudioParameterChoice() {}

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

#if JUCE_UNIT_TESTS

static struct SpecialisedAudioParameterTests final   : public UnitTest
{
    SpecialisedAudioParameterTests() : UnitTest ("Specialised Audio Parameters", "AudioProcessor parameters") {}

    void runTest() override
    {
        beginTest ("A choice parameter with three options switches at the correct points");
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

        beginTest ("An int parameter with three options switches at the correct points");
        {
            AudioParameterInt intParam ({}, {}, 1, 3, 1);

            intParam.setValueNotifyingHost (0.0f);
            expectEquals (intParam.get(), 1);

            intParam.setValueNotifyingHost (0.2f);
            expectEquals (intParam.get(), 1);

            intParam.setValueNotifyingHost (0.3f);
            expectEquals (intParam.get(), 2);

            intParam.setValueNotifyingHost (0.7f);
            expectEquals (intParam.get(), 2);

            intParam.setValueNotifyingHost (0.8f);
            expectEquals (intParam.get(), 3);

            intParam.setValueNotifyingHost (1.0f);
            expectEquals (intParam.get(), 3);
        }


        beginTest ("Choice parameters handle out-of-bounds input");
        {
            AudioParameterChoice choiceParam ({}, {}, { "a", "b", "c" }, {});

            choiceParam.setValueNotifyingHost (-0.5f);
            expectEquals (choiceParam.getIndex(), 0);

            choiceParam.setValueNotifyingHost (1.5f);
            expectEquals (choiceParam.getIndex(), 2);
        }

        beginTest ("Int parameters handle out-of-bounds input");
        {
            AudioParameterInt intParam ({}, {}, -1, 2, 0);

            intParam.setValueNotifyingHost (-0.5f);
            expectEquals (intParam.get(), -1);

            intParam.setValueNotifyingHost (1.5f);
            expectEquals (intParam.get(), 2);

            intParam = -5;
            expectEquals (intParam.get(), -1);

            intParam = 5;
            expectEquals (intParam.get(), 2);
        }
    }
} specialisedAudioParameterTests;

#endif

} // namespace juce

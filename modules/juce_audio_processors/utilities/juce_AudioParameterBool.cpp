/*
  ==============================================================================

   This file is part of the JUCE 6 technical preview.
   Copyright (c) 2020 - Raw Material Software Limited

   You may use this code under the terms of the GPL v3
   (see www.gnu.org/licenses).

   For this technical preview, this file is not subject to commercial licensing.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

AudioParameterBool::AudioParameterBool (const String& idToUse, const String& nameToUse,
                                        bool def, const String& labelToUse,
                                        std::function<String(bool, int)> stringFromBool,
                                        std::function<bool(const String&)> boolFromString)
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

AudioParameterBool::~AudioParameterBool()
{
    #if __cpp_lib_atomic_is_always_lock_free
     static_assert (std::atomic<float>::is_always_lock_free,
                    "AudioParameterBool requires a lock-free std::atomic<float>");
    #endif
}

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

} // namespace juce

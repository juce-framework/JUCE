/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

AudioParameterBool::AudioParameterBool (const ParameterID& idToUse,
                                        const String& nameToUse,
                                        bool def,
                                        const AudioParameterBoolAttributes& attributes)
    : RangedAudioParameter (idToUse, nameToUse, attributes.getAudioProcessorParameterWithIDAttributes()),
      value (def ? 1.0f : 0.0f),
      valueDefault (def),
      stringFromBoolFunction (attributes.getStringFromValueFunction() != nullptr
                                  ? attributes.getStringFromValueFunction()
                                  : [] (bool v, int) { return v ? TRANS("On") : TRANS("Off"); }),
      boolFromStringFunction (attributes.getValueFromStringFunction() != nullptr
                                  ? attributes.getValueFromStringFunction()
                                  : [] (const String& text)
                                    {
                                        static const StringArray onStrings { TRANS ("on"), TRANS ("yes"), TRANS ("true") };
                                        static const StringArray offStrings { TRANS ("off"), TRANS ("no"), TRANS ("false") };

                                        String lowercaseText (text.toLowerCase());

                                        for (auto& testText : onStrings)
                                            if (lowercaseText == testText)
                                                return true;

                                        for (auto& testText : offStrings)
                                            if (lowercaseText == testText)
                                                return false;

                                        return text.getIntValue() != 0;
                                    })
{
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
float AudioParameterBool::getDefaultValue() const                        { return valueDefault; }
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

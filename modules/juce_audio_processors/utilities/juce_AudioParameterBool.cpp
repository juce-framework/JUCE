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

AudioParameterBool::AudioParameterBool (const ParameterID& idToUse,
                                        const String& nameToUse,
                                        bool def,
                                        const AudioParameterBoolAttributes& attributes)
    : RangedAudioParameter (idToUse, nameToUse, attributes.getAudioProcessorParameterWithIDAttributes()),
      value (def ? 1.0f : 0.0f),
      valueDefault (def),
      stringFromBoolFunction (attributes.getStringFromValueFunction() != nullptr
                                  ? attributes.getStringFromValueFunction()
                                  : [] (bool v, int) { return v ? TRANS ("On") : TRANS ("Off"); }),
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

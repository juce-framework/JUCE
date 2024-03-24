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

AudioParameterFloat::AudioParameterFloat (const ParameterID& idToUse,
                                          const String& nameToUse,
                                          NormalisableRange<float> r,
                                          float def,
                                          const AudioParameterFloatAttributes& attributes)
    : RangedAudioParameter (idToUse, nameToUse, attributes.getAudioProcessorParameterWithIDAttributes()),
      range (r),
      value (def),
      valueDefault (def),
      stringFromValueFunction (attributes.getStringFromValueFunction()),
      valueFromStringFunction (attributes.getValueFromStringFunction())
{
    if (stringFromValueFunction == nullptr)
    {
        auto numDecimalPlacesToDisplay = [this]
        {
            int numDecimalPlaces = 7;

            if (! approximatelyEqual (range.interval, 0.0f))
            {
                if (approximatelyEqual (std::abs (range.interval - std::floor (range.interval)), 0.0f))
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

AudioParameterFloat::AudioParameterFloat (const ParameterID& pid, const String& nm, float minValue, float maxValue, float def)
   : AudioParameterFloat (pid, nm, { minValue, maxValue, 0.01f }, def)
{
}

AudioParameterFloat::~AudioParameterFloat()
{
    #if __cpp_lib_atomic_is_always_lock_free
     static_assert (std::atomic<float>::is_always_lock_free,
                    "AudioParameterFloat requires a lock-free std::atomic<float>");
    #endif
}

float AudioParameterFloat::getValue() const                              { return convertTo0to1 (value); }
void AudioParameterFloat::setValue (float newValue)                      { value = convertFrom0to1 (newValue); valueChanged (get()); }
float AudioParameterFloat::getDefaultValue() const                       { return convertTo0to1 (valueDefault); }
int AudioParameterFloat::getNumSteps() const                             { return AudioProcessorParameterWithID::getNumSteps(); }
String AudioParameterFloat::getText (float v, int length) const          { return stringFromValueFunction (convertFrom0to1 (v), length); }
float AudioParameterFloat::getValueForText (const String& text) const    { return convertTo0to1 (valueFromStringFunction (text)); }
void AudioParameterFloat::valueChanged (float)                           {}

AudioParameterFloat& AudioParameterFloat::operator= (float newValue)
{
    if (! approximatelyEqual ((float) value, newValue))
        setValueNotifyingHost (convertTo0to1 (newValue));

    return *this;
}

} // namespace juce

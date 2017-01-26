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
                                          const String& labelToUse, Category categoryToUse)
   : AudioProcessorParameterWithID (idToUse, nameToUse, labelToUse, categoryToUse),
     range (r), value (def), defaultValue (def)
{
}

AudioParameterFloat::AudioParameterFloat (String pid, String nm, float minValue, float maxValue, float def)
   : AudioProcessorParameterWithID (pid, nm), range (minValue, maxValue), value (def), defaultValue (def)
{
}

AudioParameterFloat::~AudioParameterFloat() {}

float AudioParameterFloat::getValue() const                              { return range.convertTo0to1 (value); }
void AudioParameterFloat::setValue (float newValue)                      { value = range.convertFrom0to1 (newValue); }
float AudioParameterFloat::getDefaultValue() const                       { return range.convertTo0to1 (defaultValue); }
int AudioParameterFloat::getNumSteps() const                             { return AudioProcessorParameterWithID::getNumSteps(); }
float AudioParameterFloat::getValueForText (const String& text) const    { return range.convertTo0to1 (text.getFloatValue()); }

String AudioParameterFloat::getText (float v, int length) const
{
    String asText (range.convertFrom0to1 (v), 2);
    return length > 0 ? asText.substring (0, length) : asText;
}

AudioParameterFloat& AudioParameterFloat::operator= (float newValue)
{
    if (value != newValue)
        setValueNotifyingHost (range.convertTo0to1 (newValue));

    return *this;
}

//==============================================================================
AudioParameterInt::AudioParameterInt (const String& idToUse, const String& nameToUse,
                                      int mn, int mx, int def,
                                      const String& labelToUse)
   : AudioProcessorParameterWithID (idToUse, nameToUse, labelToUse),
     minValue (mn), maxValue (mx),
     value ((float) def),
     defaultValue (convertTo0to1 (def))
{
    jassert (minValue < maxValue); // must have a non-zero range of values!
}

AudioParameterInt::~AudioParameterInt() {}

int AudioParameterInt::limitRange (int v) const noexcept                 { return jlimit (minValue, maxValue, v); }
float AudioParameterInt::convertTo0to1 (int v) const noexcept            { return (limitRange (v) - minValue) / (float) (maxValue - minValue); }
int AudioParameterInt::convertFrom0to1 (float v) const noexcept          { return limitRange (roundToInt ((v * (float) (maxValue - minValue)) + minValue)); }

float AudioParameterInt::getValue() const                                { return convertTo0to1 (roundToInt (value)); }
void AudioParameterInt::setValue (float newValue)                        { value = (float) convertFrom0to1 (newValue); }
float AudioParameterInt::getDefaultValue() const                         { return defaultValue; }
int AudioParameterInt::getNumSteps() const                               { return AudioProcessorParameterWithID::getNumSteps(); }
float AudioParameterInt::getValueForText (const String& text) const      { return convertTo0to1 (text.getIntValue()); }
String AudioParameterInt::getText (float v, int /*length*/) const        { return String (convertFrom0to1 (v)); }

AudioParameterInt& AudioParameterInt::operator= (int newValue)
{
    if (get() != newValue)
        setValueNotifyingHost (convertTo0to1 (newValue));

    return *this;
}


//==============================================================================
AudioParameterBool::AudioParameterBool (const String& idToUse, const String& nameToUse,
                                        bool def, const String& labelToUse)
   : AudioProcessorParameterWithID (idToUse, nameToUse, labelToUse),
     value (def ? 1.0f : 0.0f),
     defaultValue (value)
{
}

AudioParameterBool::~AudioParameterBool() {}

float AudioParameterBool::getValue() const                               { return value; }
void AudioParameterBool::setValue (float newValue)                       { value = newValue; }
float AudioParameterBool::getDefaultValue() const                        { return defaultValue; }
int AudioParameterBool::getNumSteps() const                              { return 2; }
float AudioParameterBool::getValueForText (const String& text) const     { return text.getIntValue() != 0 ? 1.0f : 0.0f; }
String AudioParameterBool::getText (float v, int /*length*/) const       { return String ((int) (v > 0.5f ? 1 : 0)); }

AudioParameterBool& AudioParameterBool::operator= (bool newValue)
{
    if (get() != newValue)
        setValueNotifyingHost (newValue ? 1.0f : 0.0f);

    return *this;
}


//==============================================================================
AudioParameterChoice::AudioParameterChoice (const String& idToUse, const String& nameToUse,
                                            const StringArray& c, int def, const String& labelToUse)
   : AudioProcessorParameterWithID (idToUse, nameToUse, labelToUse), choices (c),
     value ((float) def),
     defaultValue (convertTo0to1 (def))
{
    jassert (choices.size() > 0); // you must supply an actual set of items to choose from!
}

AudioParameterChoice::~AudioParameterChoice() {}

int AudioParameterChoice::limitRange (int v) const noexcept              { return jlimit (0, choices.size() - 1, v); }
float AudioParameterChoice::convertTo0to1 (int v) const noexcept         { return jlimit (0.0f, 1.0f, (v + 0.5f) / (float) choices.size()); }
int AudioParameterChoice::convertFrom0to1 (float v) const noexcept       { return limitRange ((int) (v * (float) choices.size())); }

float AudioParameterChoice::getValue() const                             { return convertTo0to1 (roundToInt (value)); }
void AudioParameterChoice::setValue (float newValue)                     { value = (float) convertFrom0to1 (newValue); }
float AudioParameterChoice::getDefaultValue() const                      { return defaultValue; }
int AudioParameterChoice::getNumSteps() const                            { return choices.size(); }
float AudioParameterChoice::getValueForText (const String& text) const   { return convertTo0to1 (choices.indexOf (text)); }
String AudioParameterChoice::getText (float v, int /*length*/) const     { return choices [convertFrom0to1 (v)]; }

AudioParameterChoice& AudioParameterChoice::operator= (int newValue)
{
    if (getIndex() != newValue)
        setValueNotifyingHost (convertTo0to1 (newValue));

    return *this;
}

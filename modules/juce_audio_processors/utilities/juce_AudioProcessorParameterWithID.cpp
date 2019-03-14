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

AudioProcessorParameterWithID::AudioProcessorParameterWithID (const String& idToUse,
                                                              const String& nameToUse,
                                                              const String& labelToUse,
                                                              AudioProcessorParameter::Category categoryToUse)
    : paramID (idToUse), name (nameToUse), label (labelToUse), category (categoryToUse) {}
AudioProcessorParameterWithID::~AudioProcessorParameterWithID() {}

String AudioProcessorParameterWithID::getName (int maximumStringLength) const        { return name.substring (0, maximumStringLength); }
String AudioProcessorParameterWithID::getLabel() const                               { return label; }
AudioProcessorParameter::Category AudioProcessorParameterWithID::getCategory() const { return category; }

} // namespace juce

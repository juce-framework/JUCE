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

AudioProcessorParameterWithID::AudioProcessorParameterWithID (const ParameterID& idToUse,
                                                              const String& nameToUse,
                                                              const AudioProcessorParameterWithIDAttributes& attributes)
    : HostedAudioProcessorParameter (idToUse.getVersionHint()),
      paramID (idToUse.getParamID()),
      name (nameToUse),
      label (attributes.getLabel()),
      category (attributes.getCategory()),
      meta (attributes.getMeta()),
      automatable (attributes.getAutomatable()),
      inverted (attributes.getInverted())
{
}

String AudioProcessorParameterWithID::getName (int maximumStringLength) const        { return name.substring (0, maximumStringLength); }
String AudioProcessorParameterWithID::getLabel() const                               { return label; }
AudioProcessorParameter::Category AudioProcessorParameterWithID::getCategory() const { return category; }

} // namespace juce

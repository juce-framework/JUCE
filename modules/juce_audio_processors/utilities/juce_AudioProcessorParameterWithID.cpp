/*
  ==============================================================================

   This file is part of the JUCE 7 technical preview.
   Copyright (c) 2022 - Raw Material Software Limited

   You may use this code under the terms of the GPL v3
   (see www.gnu.org/licenses).

   For the technical preview this file cannot be licensed commercially.

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

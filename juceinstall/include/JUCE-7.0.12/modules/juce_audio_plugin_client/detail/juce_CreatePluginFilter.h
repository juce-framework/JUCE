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

#pragma once

namespace juce
{

inline std::unique_ptr<AudioProcessor> createPluginFilterOfType (AudioProcessor::WrapperType type)
{
    PluginHostType::jucePlugInClientCurrentWrapperType = type;
    AudioProcessor::setTypeOfNextNewPlugin (type);
    auto pluginInstance = rawToUniquePtr (::createPluginFilter());
    AudioProcessor::setTypeOfNextNewPlugin (AudioProcessor::wrapperType_Undefined);

    // your createPluginFilter() method must return an object!
    jassert (pluginInstance != nullptr && pluginInstance->wrapperType == type);

   #if JucePlugin_Enable_ARA
    jassert (dynamic_cast<juce::AudioProcessorARAExtension*> (pluginInstance.get()) != nullptr);
   #endif

    return pluginInstance;
}

} // namespace juce

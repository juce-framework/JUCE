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

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

/** Somewhere in the codebase of your plugin, you need to implement this function
    and make it return a new instance of the filter subclass that you're building.
*/
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter();

namespace juce
{

inline AudioProcessor* JUCE_API JUCE_CALLTYPE createPluginFilterOfType (AudioProcessor::WrapperType type)
{
    AudioProcessor::setTypeOfNextNewPlugin (type);
    AudioProcessor* const pluginInstance = ::createPluginFilter();
    AudioProcessor::setTypeOfNextNewPlugin (AudioProcessor::wrapperType_Undefined);

    // your createPluginFilter() method must return an object!
    jassert (pluginInstance != nullptr && pluginInstance->wrapperType == type);

    return pluginInstance;
}

} // namespace juce

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

#include <juce_audio_plugin_client/juce_audio_plugin_client.h>
#include "juce_CreatePluginFilter.h"

namespace juce
{
    #define Component juce::Component

   #if JUCE_MAC
    #define Point juce::Point
    void repostCurrentNSEvent();
   #endif

    //==============================================================================
    inline const PluginHostType& getHostType()
    {
        static PluginHostType hostType;
        return hostType;
    }
}

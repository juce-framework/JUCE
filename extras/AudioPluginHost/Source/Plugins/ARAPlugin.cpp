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

#include "ARAPlugin.h"

#if JUCE_PLUGINHOST_ARA && (JUCE_MAC || JUCE_WINDOWS)

const Identifier ARAPluginInstanceWrapper::ARATestHost::Context::xmlRootTag         { "ARATestHostContext" };
const Identifier ARAPluginInstanceWrapper::ARATestHost::Context::xmlAudioFileAttrib { "AudioFile" };

#endif

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

#include <juce_core/system/juce_TargetPlatform.h>

#if JucePlugin_Build_Standalone

#if ! JUCE_MODULE_AVAILABLE_juce_audio_utils
 #error To compile AudioUnitv3 and/or Standalone plug-ins, you need to add the juce_audio_utils and juce_audio_devices modules!
#endif

#include "Standalone/juce_StandaloneFilterApp.cpp"

#if JUCE_USE_CUSTOM_PLUGIN_STANDALONE_APP
 extern juce::JUCEApplicationBase* juce_CreateApplication();

 #if JUCE_IOS
  extern void* juce_GetIOSCustomDelegateClass();
 #endif

#else
 JUCE_CREATE_APPLICATION_DEFINE(juce::StandaloneFilterApp)
#endif

JUCE_MAIN_FUNCTION_DEFINITION

#endif

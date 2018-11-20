/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

/*******************************************************************************
 The block below describes the properties of this module, and is read by
 the Projucer to automatically generate project code that uses it.
 For details about the syntax and how to create or use a module, see the
 JUCE Module Format.txt file.


 BEGIN_JUCE_MODULE_DECLARATION

  ID:               juce_blocks_basics
  vendor:           juce
  version:          5.3.2
  name:             Provides low-level control over ROLI BLOCKS devices
  description:      JUCE wrapper for low-level control over ROLI BLOCKS devices.
  website:          http://developer.roli.com
  license:          ISC

  dependencies:     juce_events juce_audio_devices

 END_JUCE_MODULE_DECLARATION

*******************************************************************************/


#pragma once

//==============================================================================
#include <juce_events/juce_events.h>
#include <juce_audio_devices/juce_audio_devices.h>

#if ! JUCE_HAS_CONSTEXPR
 #ifndef JUCE_DEMO_RUNNER
  #error "The juce_blocks_basics module requires a compiler that supports constexpr"
 #endif
#else

namespace juce
{
    class TouchSurface;
    class LEDGrid;
    class LEDRow;
    class StatusLight;
    class LightRing;
    class ControlButton;
}

#include "blocks/juce_Block.h"
#include "blocks/juce_TouchSurface.h"
#include "blocks/juce_LEDGrid.h"
#include "blocks/juce_LEDRow.h"
#include "blocks/juce_ControlButton.h"
#include "blocks/juce_TouchList.h"
#include "blocks/juce_StatusLight.h"
#include "topology/juce_Topology.h"
#include "topology/juce_TopologySource.h"
#include "topology/juce_PhysicalTopologySource.h"
#include "topology/juce_RuleBasedTopologySource.h"
#include "visualisers/juce_DrumPadLEDProgram.h"
#include "visualisers/juce_BitmapLEDProgram.h"

namespace juce
{
 #include "littlefoot/juce_LittleFootRunner.h"
 #include "littlefoot/juce_LittleFootCompiler.h"
 #include "littlefoot/juce_LittleFootRemoteHeap.h"
}

#endif

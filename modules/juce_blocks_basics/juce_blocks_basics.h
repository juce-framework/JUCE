/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2016 - ROLI Ltd.

   Permission is granted to use this software under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license/

   Permission to use, copy, modify, and/or distribute this software for any
   purpose with or without fee is hereby granted, provided that the above
   copyright notice and this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND ISC DISCLAIMS ALL WARRANTIES WITH REGARD
   TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
   FITNESS. IN NO EVENT SHALL ISC BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT,
   OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
   USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
   TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
   OF THIS SOFTWARE.

   -----------------------------------------------------------------------------

   To release a closed-source product which uses other parts of JUCE not
   licensed under the ISC terms, commercial licenses are available: visit
   www.juce.com for more information.

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
  version:          4.3.0
  name:             Provides low-level control over ROLI BLOCKS devices
  description:      JUCE wrapper for low-level control over ROLI BLOCKS devices.
  website:          http://developer.roli.com
  license:          ISC

  dependencies:     juce_events juce_audio_devices

 END_JUCE_MODULE_DECLARATION

*******************************************************************************/


#ifndef JUCE_BLOCKS_BASICS_H_INCLUDED
#define JUCE_BLOCKS_BASICS_H_INCLUDED

//==============================================================================
#include <juce_events/juce_events.h>
#include <juce_audio_devices/juce_audio_devices.h>

namespace juce
{
  class TouchSurface;
  class LEDGrid;
  class LEDRow;
  class StatusLight;
  class LightRing;
  class ControlButton;

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
  #include "littlefoot/juce_LittleFootRunner.h"
  #include "littlefoot/juce_LittleFootCompiler.h"
  #include "littlefoot/juce_LittleFootRemoteHeap.h"
  #include "visualisers/juce_DrumPadLEDProgram.h"
  #include "visualisers/juce_BitmapLEDProgram.h"
}


#endif   // JUCE_BLOCKS_BASICS_H_INCLUDED

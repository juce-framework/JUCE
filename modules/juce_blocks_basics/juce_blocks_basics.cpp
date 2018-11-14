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

#include "juce_blocks_basics.h"

#if ! JUCE_HAS_CONSTEXPR
 #ifndef JUCE_DEMO_RUNNER
  #error "The juce_blocks_basics module requires a compiler that supports constexpr"
 #endif
#else

namespace juce
{
 #include "littlefoot/juce_LittleFootRemoteHeap.h"
}

#include "protocol/juce_BitPackingUtilities.h"
#include "protocol/juce_BlocksProtocolDefinitions.h"
#include "protocol/juce_HostPacketDecoder.h"
#include "protocol/juce_HostPacketBuilder.h"
#include "protocol/juce_BlockModels.h"

#include "blocks/juce_BlockConfigManager.h"
#include "blocks/juce_Block.cpp"
#include "topology/juce_PhysicalTopologySource.cpp"
#include "topology/juce_RuleBasedTopologySource.cpp"
#include "visualisers/juce_DrumPadLEDProgram.cpp"
#include "visualisers/juce_BitmapLEDProgram.cpp"

#endif

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

#include "BlockFinder.h"

using namespace juce;

BlockFinder::BlockFinder()
{
    // Register to receive topologyChanged() callbacks from pts.
    pts.addListener (this);
}

void BlockFinder::topologyChanged()
{
    // We have a new topology, so find out what it isand store it in a local
    // variable.
    BlockTopology currentTopology = pts.getCurrentTopology();
    Logger::writeToLog ("\nNew BLOCKS topology.");

    // The blocks member of a BlockTopology contains an array of blocks. Here we
    // loop over them and print some information.
    Logger::writeToLog (String ("Detected ") + String (currentTopology.blocks.size()) + " blocks:");
    for (auto& block : currentTopology.blocks)
    {
        Logger::writeToLog ("");
        Logger::writeToLog (String("    Description:   ") + block->getDeviceDescription());
        Logger::writeToLog (String("    Battery level: ") + String (block->getBatteryLevel()));
        Logger::writeToLog (String("    UID:           ") + String (block->uid));
        Logger::writeToLog (String("    Serial number: ") + block->serialNumber);
    }
}

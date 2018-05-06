/*
  ==============================================================================

   This file is part of the JUCE examples.
   Copyright (c) 2017 - ROLI Ltd.

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES,
   WHETHER EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR
   PURPOSE, ARE DISCLAIMED.

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
    auto currentTopology = pts.getCurrentTopology();
    Logger::writeToLog ("\nNew BLOCKS topology.");

    // The blocks member of a BlockTopology contains an array of blocks. Here we
    // loop over them and print some information.
    Logger::writeToLog ("Detected " + String (currentTopology.blocks.size()) + " blocks:");

    for (auto& block : currentTopology.blocks)
    {
        Logger::writeToLog ("");
        Logger::writeToLog ("    Description:   " + block->getDeviceDescription());
        Logger::writeToLog ("    Battery level: " + String (block->getBatteryLevel()));
        Logger::writeToLog ("    UID:           " + String (block->uid));
        Logger::writeToLog ("    Serial number: " + block->serialNumber);
    }
}

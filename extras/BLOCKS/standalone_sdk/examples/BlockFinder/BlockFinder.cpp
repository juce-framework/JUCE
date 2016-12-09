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

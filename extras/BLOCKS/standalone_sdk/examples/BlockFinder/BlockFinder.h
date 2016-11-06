#ifndef BLOCKFINDER_INCLUDED
#define BLOCKFINDER_INCLUDED

#include <BlocksHeader.h>

// Monitors a PhysicalTopologySource for changes to the connected BLOCKS and
// prints some information about the BLOCKS that are available.
class BlockFinder : private juce::TopologySource::Listener
{
public:
    // Register as a listener to the PhysicalTopologySource, so that we receive
    // callbacks in topologyChanged().
    BlockFinder();

private:
    // Called by the PhysicalTopologySource when the BLOCKS topology changes.
    void topologyChanged() override;

    // The PhysicalTopologySource member variable which reports BLOCKS changes.
    juce::PhysicalTopologySource pts;
};

#endif // BLOCKFINDER_INCLUDED

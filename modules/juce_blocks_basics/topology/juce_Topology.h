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

namespace juce
{

/** Describes a physical connection between two ports of two block devices.

    @tags{Blocks}
*/
struct BlockDeviceConnection
{
    Block::UID device1, device2;
    Block::ConnectionPort connectionPortOnDevice1, connectionPortOnDevice2;

    bool operator== (const BlockDeviceConnection&) const noexcept;
    bool operator!= (const BlockDeviceConnection&) const noexcept;
};


/** Describes a set of blocks and the connections between them.

    @tags{Blocks}
*/
struct BlockTopology
{
    Block::Array blocks;
    Array<BlockDeviceConnection> connections;

    bool operator== (const BlockTopology&) const noexcept;
    bool operator!= (const BlockTopology&) const noexcept;

    Block::Ptr getBlockWithUID (Block::UID deviceID)
    {
        for (auto&& block : blocks)
            if (block->uid == deviceID)
                return block;

        return {};
    }

    const Block::Ptr getBlockWithUID (Block::UID deviceID) const
    {
        for (auto&& block : blocks)
            if (block->uid == deviceID)
                return block;

        return {};
    }

    Block::Array getDirectlyConnectedBlocks (Block::UID blockUID) const
    {
        Block::Array connectedBlocks;

        for (const auto& connection : connections)
        {
            auto connectedDeviceUID = Block::UID { 0 };

            if (connection.device1 == blockUID)
                connectedDeviceUID = connection.device2;

            else if (connection.device2 == blockUID)
                connectedDeviceUID = connection.device1;

            if (connectedDeviceUID != Block::UID { 0 })
                if (auto newBlock = getBlockWithUID (connectedDeviceUID))
                    connectedBlocks.addIfNotAlreadyThere (newBlock);
        }

        return connectedBlocks;
    }

    Array<BlockDeviceConnection> getConnectionsBetweenBlocks (Block::UID uid1, Block::UID uid2) const
    {
        Array<BlockDeviceConnection> blockConnections;

        for (const auto& connection : connections)
        {
            if ((connection.device1 == uid1 && connection.device2 == uid2)
                || (connection.device1 == uid2 && connection.device2 == uid1))
            {
                blockConnections.add (connection);
            }
        }

        return blockConnections;
    }

    int getNumberOfConnectionsToBlock (Block::UID uid) const
    {
        int connectionCount = 0;

        for (const auto& connection : connections)
            if (connection.device1 == uid || connection.device2 == uid)
                ++connectionCount;

        return connectionCount;
    }
};

} // namespace juce

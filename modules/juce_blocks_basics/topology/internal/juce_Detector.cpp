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

//==============================================================================
/** This is the main singleton object that keeps track of connected blocks */
struct Detector   : public ReferenceCountedObject,
                    private Timer,
                    private AsyncUpdater
{
    using BlockImpl = BlockImplementation<Detector>;

    Detector()  : defaultDetector (new MIDIDeviceDetector()), deviceDetector (*defaultDetector)
    {
        startTimer (10);
    }

    Detector (PhysicalTopologySource::DeviceDetector& dd)  : deviceDetector (dd)
    {
        startTimer (10);
    }

    ~Detector()
    {
        jassert (activeTopologySources.isEmpty());
    }

    using Ptr = ReferenceCountedObjectPtr<Detector>;

    static Detector::Ptr getDefaultDetector()
    {
        auto& d = getDefaultDetectorPointer();

        if (d == nullptr)
            d = new Detector();

        return d;
    }

    static Detector::Ptr& getDefaultDetectorPointer()
    {
        static Detector::Ptr defaultDetector;
        return defaultDetector;
    }

    void detach (PhysicalTopologySource* pts)
    {
        activeTopologySources.removeAllInstancesOf (pts);

        if (activeTopologySources.isEmpty())
        {
            for (auto& b : currentTopology.blocks)
                if (auto bi = BlockImpl::getFrom (b))
                    bi->sendCommandMessage (BlocksProtocol::endAPIMode);

            currentTopology = {};

            auto& d = getDefaultDetectorPointer();

            if (d != nullptr && d->getReferenceCount() == 2)
                getDefaultDetectorPointer() = nullptr;
        }
    }

    bool isConnected (Block::UID deviceID) const noexcept
    {
        JUCE_ASSERT_MESSAGE_MANAGER_IS_LOCKED // This method must only be called from the message thread!

        for (auto&& b : currentTopology.blocks)
            if (b->uid == deviceID)
                return true;

        return false;
    }

    void handleDeviceAdded (const DeviceInfo& info)
    {
        JUCE_ASSERT_MESSAGE_MANAGER_IS_LOCKED

        const auto blockWasRemoved = containsBlockWithUID (blocksToRemove, info.uid);
        const auto knownBlock = std::find_if (previouslySeenBlocks.begin(), previouslySeenBlocks.end(),
                                              [uid = info.uid] (Block::Ptr block) { return uid == block->uid; });

        Block::Ptr block;

        if (knownBlock != previouslySeenBlocks.end())
        {
            block = *knownBlock;

            if (auto* blockImpl = BlockImpl::getFrom (*block))
            {
                blockImpl->markReconnected (info);
                previouslySeenBlocks.removeObject (block);
            }
        }
        else
        {
            block = new BlockImpl (*this, info);
        }

        currentTopology.blocks.addIfNotAlreadyThere (block);

        if (blockWasRemoved)
        {
            blocksToUpdate.addIfNotAlreadyThere (block);
            blocksToAdd.removeObject (block);
        }
        else
        {
            blocksToAdd.addIfNotAlreadyThere (block);
            blocksToUpdate.removeObject (block);
        }

        blocksToRemove.removeObject (block);

        triggerAsyncUpdate();
    }

    void handleDeviceRemoved (const DeviceInfo& info)
    {
        JUCE_ASSERT_MESSAGE_MANAGER_IS_LOCKED

        const auto blockIt = std::find_if (currentTopology.blocks.begin(), currentTopology.blocks.end(),
                                           [uid = info.uid] (Block::Ptr block) { return uid == block->uid; });

        if (blockIt != currentTopology.blocks.end())
        {
            const auto block = *blockIt;

            if (auto blockImpl = BlockImpl::getFrom (block))
                blockImpl->markDisconnected();

            currentTopology.blocks.removeObject (block);
            previouslySeenBlocks.addIfNotAlreadyThere (block);

            blocksToRemove.addIfNotAlreadyThere (block);
            blocksToUpdate.removeObject (block);
            blocksToAdd.removeObject (block);
            triggerAsyncUpdate();
        }
    }

    void handleConnectionsChanged()
    {
        JUCE_ASSERT_MESSAGE_MANAGER_IS_LOCKED
        triggerAsyncUpdate();
    }

    void handleDeviceUpdated (const DeviceInfo& info)
    {
        if (containsBlockWithUID (blocksToRemove, info.uid))
            return;

        const auto blockIt = std::find_if (currentTopology.blocks.begin(), currentTopology.blocks.end(),
                                           [uid = info.uid] (Block::Ptr block) { return uid == block->uid; });

        if (blockIt != currentTopology.blocks.end())
        {
            const auto block = *blockIt;

            if (auto blockImpl = BlockImpl::getFrom (block))
                blockImpl->markReconnected (info);

            if (! containsBlockWithUID (blocksToAdd, info.uid))
            {
                blocksToUpdate.addIfNotAlreadyThere (block);
                triggerAsyncUpdate();
            }
        }
    }

    void handleBatteryChargingChanged (Block::UID deviceID, const BlocksProtocol::BatteryCharging isCharging)
    {
        if (auto block = currentTopology.getBlockWithUID (deviceID))
            if (auto blockImpl = BlockImpl::getFrom (*block))
                blockImpl->batteryCharging = isCharging;
    }

    void handleBatteryLevelChanged (Block::UID deviceID, const BlocksProtocol::BatteryLevel batteryLevel)
    {
        if (auto block = currentTopology.getBlockWithUID (deviceID))
            if (auto blockImpl = BlockImpl::getFrom (*block))
                blockImpl->batteryLevel = batteryLevel;
    }

    void handleIndexChanged (Block::UID deviceID, const BlocksProtocol::TopologyIndex index)
    {
        if (auto block = currentTopology.getBlockWithUID (deviceID))
            if (auto blockImpl = BlockImpl::getFrom (*block))
                blockImpl->topologyIndex = index;
    }

    void notifyBlockIsRestarting (Block::UID deviceID)
    {
        for (auto& group : connectedDeviceGroups)
            group->handleBlockRestarting (deviceID);
    }

    Array<Block::UID> getDnaDependentDeviceUIDs (Block::UID uid)
    {
        JUCE_ASSERT_MESSAGE_THREAD

        Array<Block::UID> dependentDeviceUIDs;

        if (auto block = getBlockImplementationWithUID (uid))
        {
            if (auto master = getBlockImplementationWithUID (block->masterUID))
            {
                auto graph = BlockGraph (currentTopology, [uid] (Block::Ptr b) { return b->uid != uid; });
                const auto pathWithoutBlock = graph.getTraversalPathFromMaster (master);

                for (const auto b : currentTopology.blocks)
                {
                    if (b->uid != uid && ! pathWithoutBlock.contains (b))
                    {
                        TOPOLOGY_LOG ( "Dependent device: " + b->name);
                        dependentDeviceUIDs.add (b->uid);
                    }
                }
            }
        }

        return dependentDeviceUIDs;
    }

    void handleSharedDataACK (Block::UID deviceID, uint32 packetCounter) const
    {
        JUCE_ASSERT_MESSAGE_MANAGER_IS_LOCKED

        if (auto* bi = getBlockImplementationWithUID (deviceID))
            bi->handleSharedDataACK (packetCounter);
    }

    void handleFirmwareUpdateACK (Block::UID deviceID, uint8 resultCode, uint32 resultDetail)
    {
        if (auto* bi = getBlockImplementationWithUID (deviceID))
            bi->handleFirmwareUpdateACK (resultCode, resultDetail);
    }

    void handleConfigUpdateMessage (Block::UID deviceID, int32 item, int32 value, int32 min, int32 max)
    {
        if (auto* bi = getBlockImplementationWithUID (deviceID))
            bi->handleConfigUpdateMessage (item, value, min, max);
    }

    void notifyBlockOfConfigChange (BlockImpl& bi, uint32 item)
    {
        if (auto configChangedCallback = bi.configChangedCallback)
        {
            if (item >= bi.getMaxConfigIndex())
                configChangedCallback (bi, {}, item);
            else
                configChangedCallback (bi, bi.getLocalConfigMetaData (item), item);
        }
    }

    void handleConfigSetMessage (Block::UID deviceID, int32 item, int32 value)
    {
        if (auto* bi = getBlockImplementationWithUID (deviceID))
        {
            bi->handleConfigSetMessage (item, value);
            notifyBlockOfConfigChange (*bi, uint32 (item));
        }
    }

    void handleConfigFactorySyncEndMessage (Block::UID deviceID)
    {
        if (auto* bi = getBlockImplementationWithUID (deviceID))
            notifyBlockOfConfigChange (*bi, bi->getMaxConfigIndex());
    }

    void handleConfigFactorySyncResetMessage (Block::UID deviceID)
    {
        if (auto* bi = getBlockImplementationWithUID (deviceID))
            bi->resetConfigListActiveStatus();
    }

    void handleLogMessage (Block::UID deviceID, const String& message) const
    {
        JUCE_ASSERT_MESSAGE_MANAGER_IS_LOCKED

        if (auto* bi = getBlockImplementationWithUID (deviceID))
            bi->handleLogMessage (message);
    }

    void handleButtonChange (Block::UID deviceID, Block::Timestamp timestamp, uint32 buttonIndex, bool isDown) const
    {
        JUCE_ASSERT_MESSAGE_MANAGER_IS_LOCKED

        if (auto* bi = getBlockImplementationWithUID (deviceID))
        {
            bi->pingFromDevice();

            if (isPositiveAndBelow (buttonIndex, bi->getButtons().size()))
                if (auto* cbi = dynamic_cast<BlockImpl::ControlButtonImplementation*> (bi->getButtons().getUnchecked (int (buttonIndex))))
                    cbi->broadcastButtonChange (timestamp, bi->modelData.buttons[(int) buttonIndex].type, isDown);
        }
    }

    void handleTouchChange (Block::UID deviceID, const TouchSurface::Touch& touchEvent)
    {
        JUCE_ASSERT_MESSAGE_MANAGER_IS_LOCKED

        auto block = currentTopology.getBlockWithUID (deviceID);
        if (block != nullptr)
        {
            if (auto* surface = dynamic_cast<BlockImpl::TouchSurfaceImplementation*> (block->getTouchSurface()))
            {
                TouchSurface::Touch scaledEvent (touchEvent);

                scaledEvent.x      *= block->getWidth();
                scaledEvent.y      *= block->getHeight();
                scaledEvent.startX *= block->getWidth();
                scaledEvent.startY *= block->getHeight();

                surface->broadcastTouchChange (scaledEvent);
            }
        }
    }

    void cancelAllActiveTouches() noexcept
    {
        for (auto& block : currentTopology.blocks)
            if (auto* surface = block->getTouchSurface())
                surface->cancelAllActiveTouches();
                }

    void handleCustomMessage (Block::UID deviceID, Block::Timestamp timestamp, const int32* data)
    {
        if (auto* bi = getBlockImplementationWithUID (deviceID))
            bi->handleCustomMessage (timestamp, data);
    }

    //==============================================================================
    template <typename PacketBuilder>
    bool sendMessageToDevice (Block::UID deviceID, const PacketBuilder& builder) const
    {
        for (auto* c : connectedDeviceGroups)
            if (c->contains (deviceID))
                return c->sendMessageToDevice (builder);

        return false;
    }

    static Detector* getFrom (Block& b) noexcept
    {
        if (auto* bi = BlockImpl::getFrom (b))
            return (bi->detector);

        jassertfalse;
        return nullptr;
    }

    PhysicalTopologySource::DeviceConnection* getDeviceConnectionFor (const Block& b)
    {
        for (const auto& d : connectedDeviceGroups)
        {
            if (d->contains (b.uid))
                return d->getDeviceConnection();
        }

        return nullptr;
    }

    const PhysicalTopologySource::DeviceConnection* getDeviceConnectionFor (const Block& b) const
    {
        for (const auto& d : connectedDeviceGroups)
        {
            if (d->contains (b.uid))
                return d->getDeviceConnection();
        }

        return nullptr;
    }

    std::unique_ptr<MIDIDeviceDetector> defaultDetector;
    PhysicalTopologySource::DeviceDetector& deviceDetector;

    Array<PhysicalTopologySource*> activeTopologySources;

    BlockTopology currentTopology;

private:
    Block::Array previouslySeenBlocks, blocksToAdd, blocksToRemove, blocksToUpdate;

    void timerCallback() override
    {
        startTimer (1500);

        auto detectedDevices = deviceDetector.scanForDevices();

        handleDevicesRemoved (detectedDevices);
        handleDevicesAdded (detectedDevices);
    }

    bool containsBlockWithUID (const Block::Array& blocks, Block::UID uid)
    {
        for (const auto block : blocks)
            if (block->uid == uid)
                return true;

        return false;
    }

    void handleDevicesRemoved (const StringArray& detectedDevices)
    {
        for (int i = connectedDeviceGroups.size(); --i >= 0;)
            if (! connectedDeviceGroups.getUnchecked(i)->isStillConnected (detectedDevices))
                connectedDeviceGroups.remove (i);
    }

    void handleDevicesAdded (const StringArray& detectedDevices)
    {
        for (const auto& devName : detectedDevices)
        {
            if (! hasDeviceFor (devName))
            {
                if (auto d = deviceDetector.openDevice (detectedDevices.indexOf (devName)))
                {
                    connectedDeviceGroups.add (new ConnectedDeviceGroup<Detector> (*this, devName, d));
                }
            }
        }
    }

    bool hasDeviceFor (const String& devName) const
    {
        for (auto d : connectedDeviceGroups)
            if (d->deviceName == devName)
                return true;

        return false;
    }

    BlockImpl* getBlockImplementationWithUID (Block::UID deviceID) const noexcept
    {
        if (auto block = currentTopology.getBlockWithUID (deviceID))
            return BlockImpl::getFrom (*block);

            return nullptr;
    }

    OwnedArray<ConnectedDeviceGroup<Detector>> connectedDeviceGroups;

    //==============================================================================
    /** This is a friend of the BlocksImplementation that will scan and set the
        physical positions of the blocks.

        Returns an array of blocks that were updated.
    */
    struct BlocksLayoutTraverser
    {
        static Block::Array updateBlocks (const BlockTopology& topology)
        {
            Block::Array updated;
            Array<Block::UID> visited;

            for (auto& block : topology.blocks)
            {
                if (block->isMasterBlock() && ! visited.contains (block->uid))
                {
                    if (auto* bi = BlockImpl::getFrom (block))
                    {
                        if (bi->rotation != 0 || bi->position.first != 0 || bi->position.second != 0)
                        {
                            bi->rotation = 0;
                            bi->position = {};
                            updated.add (block);
                        }
                    }

                    layoutNeighbours (*block, topology, visited, updated);
                }
            }

            return updated;
        }

    private:
        // returns the distance from corner clockwise
        static int getUnitForIndex (Block::Ptr block, Block::ConnectionPort::DeviceEdge edge, int index)
        {
            if (block->getType() == Block::seaboardBlock)
            {
                if (edge == Block::ConnectionPort::DeviceEdge::north)
                {
                    if (index == 0) return 1;
                    if (index == 1) return 4;
                }
                else if (edge != Block::ConnectionPort::DeviceEdge::south)
                {
                    return 1;
                }
            }

            if (edge == Block::ConnectionPort::DeviceEdge::south)
                return block->getWidth() - (index + 1);

            if (edge == Block::ConnectionPort::DeviceEdge::west)
                return block->getHeight() - (index + 1);

            return index;
        }

        // returns how often north needs to rotate by 90 degrees
        static int getRotationForEdge (Block::ConnectionPort::DeviceEdge edge)
        {
            switch (edge)
            {
                case Block::ConnectionPort::DeviceEdge::north:  return 0;
                case Block::ConnectionPort::DeviceEdge::east:   return 1;
                case Block::ConnectionPort::DeviceEdge::south:  return 2;
                case Block::ConnectionPort::DeviceEdge::west:   return 3;
            }

            jassertfalse;
            return 0;
        }

        static void layoutNeighbours (const Block::Ptr block,
                                      const BlockTopology& topology,
                                      Array<Block::UID>& visited,
                                      Block::Array& updated)
        {
            visited.add (block->uid);

            for (auto& connection : topology.connections)
            {
                if ((connection.device1 == block->uid && ! visited.contains (connection.device2))
                    || (connection.device2 == block->uid && ! visited.contains (connection.device1)))
                {
                    const auto theirUid = connection.device1 == block->uid ? connection.device2 : connection.device1;
                    const auto neighbourPtr = topology.getBlockWithUID (theirUid);

                    if (auto* neighbour = dynamic_cast<BlockImpl*> (neighbourPtr.get()))
                    {
                        const auto  myBounds    = block->getBlockAreaWithinLayout();
                        const auto& myPort      = connection.device1 == block->uid ? connection.connectionPortOnDevice1 : connection.connectionPortOnDevice2;
                        const auto& theirPort   = connection.device1 == block->uid ? connection.connectionPortOnDevice2 : connection.connectionPortOnDevice1;
                        const auto  myOffset    = getUnitForIndex (block, myPort.edge, myPort.index);
                        const auto  theirOffset = getUnitForIndex (neighbourPtr, theirPort.edge, theirPort.index);

                        {
                            const auto neighbourRotation = (2 + block->getRotation()
                                                            + getRotationForEdge (myPort.edge)
                                                            - getRotationForEdge (theirPort.edge)) % 4;

                            if (neighbour->rotation != neighbourRotation)
                            {
                                neighbour->rotation = neighbourRotation;
                                updated.addIfNotAlreadyThere (neighbourPtr);
                            }
                        }

                        std::pair<int, int> delta;
                        const auto theirBounds = neighbour->getBlockAreaWithinLayout();

                        switch ((block->getRotation() + getRotationForEdge (myPort.edge)) % 4)
                        {
                            case 0: // over me
                                delta = { myOffset - (theirBounds.width - (theirOffset + 1)), -theirBounds.height };
                                break;
                            case 1: // right of me
                                delta = { myBounds.width, myOffset - (theirBounds.height - (theirOffset + 1)) };
                                break;
                            case 2: // under me
                                delta = { (myBounds.width - (myOffset + 1)) - theirOffset, myBounds.height };
                                break;
                            case 3: // left of me
                                delta = { -theirBounds.width, (myBounds.height - (myOffset + 1)) - theirOffset };
                                break;
                        }

                        {
                            const auto neighbourX = myBounds.x + delta.first;
                            const auto neighbourY = myBounds.y + delta.second;

                            if (neighbour->position.first != neighbourX
                                || neighbour->position.second != neighbourY)
                            {
                                neighbour->position.first = neighbourX;
                                neighbour->position.second = neighbourY;

                                updated.addIfNotAlreadyThere (neighbourPtr);
                            }
                        }

                        layoutNeighbours (neighbourPtr, topology, visited, updated);
                    }
                }
            }
        }
    };

    //==============================================================================
   #if DUMP_TOPOLOGY
    static String idToSerialNum (const BlockTopology& topology, Block::UID uid)
    {
        for (auto* b : topology.blocks)
            if (b->uid == uid)
                return b->serialNumber;

        return "???";
    }

    static String portEdgeToString (Block::ConnectionPort port)
    {
        switch (port.edge)
        {
            case Block::ConnectionPort::DeviceEdge::north: return "north";
            case Block::ConnectionPort::DeviceEdge::south: return "south";
            case Block::ConnectionPort::DeviceEdge::east:  return "east";
            case Block::ConnectionPort::DeviceEdge::west:  return "west";
        }

        return {};
    }

    static String portToString (Block::ConnectionPort port)
    {
        return portEdgeToString (port) + "_" + String (port.index);
    }

    static void dumpTopology (const BlockTopology& topology)
    {
        MemoryOutputStream m;

        m << "=============================================================================" << newLine
        << "Topology:  " << topology.blocks.size() << " device(s)" << newLine
        << newLine;

        int index = 0;

        for (auto block : topology.blocks)
        {
            m << "Device " << index++ << (block->isMasterBlock() ? ":  (MASTER)" : ":") << newLine;

            m << "  Description: " << block->getDeviceDescription() << newLine
            << "  Serial: " << block->serialNumber << newLine;

            if (auto bi = BlockImplementation<Detector>::getFrom (*block))
                m << "  Short address: " << (int) bi->getDeviceIndex() << newLine;

            m << "  Battery level: " + String (roundToInt (100.0f * block->getBatteryLevel())) + "%" << newLine
            << "  Battery charging: " + String (block->isBatteryCharging() ? "y" : "n") << newLine
            << "  Width: " << block->getWidth() << newLine
            << "  Height: " << block->getHeight() << newLine
            << "  Millimeters per unit: " << block->getMillimetersPerUnit() << newLine
            << newLine;
        }

        for (auto& connection : topology.connections)
        {
            m << idToSerialNum (topology, connection.device1)
            << ":" << portToString (connection.connectionPortOnDevice1)
            << "  <->  "
            << idToSerialNum (topology, connection.device2)
            << ":" << portToString (connection.connectionPortOnDevice2) << newLine;
        }

        m << "=============================================================================" << newLine;

        Logger::outputDebugString (m.toString());
    }
   #endif

    //==============================================================================
    void updateBlockPositions()
    {
        const auto updated = BlocksLayoutTraverser::updateBlocks (currentTopology);

        for (const auto block : updated)
        {
            if (containsBlockWithUID (blocksToAdd, block->uid) || containsBlockWithUID (blocksToRemove, block->uid))
                continue;

            blocksToUpdate.addIfNotAlreadyThere (block);
        }
    }

    void updateBlockConnections()
    {
        currentTopology.connections.clearQuick();

        for (auto d : connectedDeviceGroups)
            currentTopology.connections.addArray (d->getCurrentDeviceConnections());
    }

    void handleAsyncUpdate() override
    {
        updateBlockConnections();
        updateBlockPositions();

        for (auto* d : activeTopologySources)
        {
            for (const auto block : blocksToAdd)
                d->listeners.call ([&block] (TopologySource::Listener& l) { l.blockAdded (block); });

            for (const auto block : blocksToRemove)
                d->listeners.call ([&block] (TopologySource::Listener& l) { l.blockRemoved (block); });

            for (const auto block : blocksToUpdate)
                d->listeners.call ([&block] (TopologySource::Listener& l) { l.blockUpdated (block); });
        }

        const auto topologyChanged = blocksToAdd.size() > 0 || blocksToRemove.size() > 0 || blocksToUpdate.size() > 0;

        if (topologyChanged)
        {
           #if DUMP_TOPOLOGY
            dumpTopology (currentTopology);
           #endif

            for (auto* d : activeTopologySources)
                d->listeners.call ([] (TopologySource::Listener& l) { l.topologyChanged(); });
        }

        blocksToUpdate.clear();
        blocksToAdd.clear();
        blocksToRemove.clear();

        static const int maxBlocksToSave = 100;

        if (previouslySeenBlocks.size() > maxBlocksToSave)
            previouslySeenBlocks.removeRange (0, 2 * (previouslySeenBlocks.size() - maxBlocksToSave));
    }

    //==============================================================================
    JUCE_DECLARE_WEAK_REFERENCEABLE (Detector)
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Detector)
};

} // namespace juce

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

namespace
{
    static bool containsBlockWithUID (const Block::Array& blocks, Block::UID uid) noexcept
    {
        for (auto&& block : blocks)
            if (block->uid == uid)
                return true;

        return false;
    }

    static bool versionNumberChanged (const DeviceInfo& device, juce::String version) noexcept
    {
        auto deviceVersion = device.version.asString();
        return deviceVersion != version && deviceVersion.isNotEmpty();
    }

    static void setVersionNumberForBlock (const DeviceInfo& deviceInfo, Block& block) noexcept
    {
        jassert (deviceInfo.uid == block.uid);
        block.versionNumber = deviceInfo.version.asString();
    }

    static void setNameForBlock (const DeviceInfo& deviceInfo, Block& block)
    {
        jassert (deviceInfo.uid == block.uid);
        block.name = deviceInfo.name.asString();
    }
}

//==============================================================================
/** This is the main singleton object that keeps track of connected blocks */
struct Detector   : public juce::ReferenceCountedObject,
                    private juce::Timer
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

    using Ptr = juce::ReferenceCountedObjectPtr<Detector>;

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
                if (auto bi = BlockImpl::getFrom (*b))
                    bi->sendCommandMessage (BlocksProtocol::endAPIMode);

            currentTopology = {};
            lastTopology = {};

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

    const BlocksProtocol::DeviceStatus* getLastStatus (Block::UID deviceID) const noexcept
    {
        for (auto d : connectedDeviceGroups)
            if (auto status = d->getLastStatus (deviceID))
                return status;

        return nullptr;
    }

    void handleTopologyChange()
    {
        JUCE_ASSERT_MESSAGE_MANAGER_IS_LOCKED

        {
            juce::Array<DeviceInfo> newDeviceInfo;
            juce::Array<BlockDeviceConnection> newDeviceConnections;

            for (auto d : connectedDeviceGroups)
            {
                newDeviceInfo.addArray (d->getCurrentDeviceInfo());
                newDeviceConnections.addArray (d->getCurrentDeviceConnections());
            }

            for (int i = currentTopology.blocks.size(); --i >= 0;)
            {
                auto currentBlock = currentTopology.blocks.getUnchecked (i);

                auto newDeviceIter = std::find_if (newDeviceInfo.begin(), newDeviceInfo.end(),
                                                   [&] (DeviceInfo& info) { return info.uid == currentBlock->uid; });

                auto* blockImpl = BlockImpl::getFrom (*currentBlock);

                if (newDeviceIter == newDeviceInfo.end())
                {
                    if (blockImpl != nullptr)
                        blockImpl->markDisconnected();

                    disconnectedBlocks.addIfNotAlreadyThere (currentTopology.blocks.removeAndReturn (i).get());
                }
                else
                {
                    if (blockImpl != nullptr && blockImpl->wasPowerCycled())
                    {
                        blockImpl->resetPowerCycleFlag();
                        blockImpl->markReconnected (*newDeviceIter);
                    }

                    updateCurrentBlockInfo (currentBlock, *newDeviceIter);
                }
            }

            static const int maxBlocksToSave = 100;

            if (disconnectedBlocks.size() > maxBlocksToSave)
                disconnectedBlocks.removeRange (0, 2 * (disconnectedBlocks.size() - maxBlocksToSave));

            for (auto& info : newDeviceInfo)
                if (info.serial.isValid() && ! containsBlockWithUID (currentTopology.blocks, getBlockUIDFromSerialNumber (info.serial)))
                    addBlock (info);

            currentTopology.connections.swapWith (newDeviceConnections);
        }

        broadcastTopology();
    }

    void notifyBlockIsRestarting (Block::UID deviceID)
    {
        for (auto& group : connectedDeviceGroups)
            group->notifyBlockIsRestarting (deviceID);
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
    int getIndexFromDeviceID (Block::UID deviceID) const noexcept
    {
        for (auto* c : connectedDeviceGroups)
        {
            auto index = c->getIndexFromDeviceID (deviceID);

            if (index >= 0)
                return index;
        }

        return -1;
    }

    template <typename PacketBuilder>
    bool sendMessageToDevice (Block::UID deviceID, const PacketBuilder& builder) const
    {
        for (auto* c : connectedDeviceGroups)
            if (c->getIndexFromDeviceID (deviceID) >= 0)
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
            for (const auto& info : d->getCurrentDeviceInfo())
            {
                if (info.uid == b.uid)
                    return d->getDeviceConnection();
            }
        }

        return nullptr;
    }

    const PhysicalTopologySource::DeviceConnection* getDeviceConnectionFor (const Block& b) const
    {
        for (const auto& d : connectedDeviceGroups)
        {
            for (const auto& info : d->getCurrentDeviceInfo())
            {
                if (info.uid == b.uid)
                    return d->getDeviceConnection();
            }
        }

        return nullptr;
    }

    std::unique_ptr<MIDIDeviceDetector> defaultDetector;
    PhysicalTopologySource::DeviceDetector& deviceDetector;

    juce::Array<PhysicalTopologySource*> activeTopologySources;

    BlockTopology currentTopology, lastTopology;
    juce::ReferenceCountedArray<Block, CriticalSection> disconnectedBlocks;

private:
    void timerCallback() override
    {
        startTimer (1500);

        auto detectedDevices = deviceDetector.scanForDevices();

        handleDevicesRemoved (detectedDevices);
        handleDevicesAdded (detectedDevices);
    }

    void handleDevicesRemoved (const juce::StringArray& detectedDevices)
    {
        bool anyDevicesRemoved = false;

        for (int i = connectedDeviceGroups.size(); --i >= 0;)
        {
            if (! connectedDeviceGroups.getUnchecked(i)->isStillConnected (detectedDevices))
            {
                connectedDeviceGroups.remove (i);
                anyDevicesRemoved = true;
            }
        }

        if (anyDevicesRemoved)
            handleTopologyChange();
    }

    void handleDevicesAdded (const juce::StringArray& detectedDevices)
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

    bool hasDeviceFor (const juce::String& devName) const
    {
        for (auto d : connectedDeviceGroups)
            if (d->deviceName == devName)
                return true;

        return false;
    }

    void addBlock (DeviceInfo info)
    {
        if (! reactivateBlockIfKnown (info))
            addNewBlock (info);
    }

    bool reactivateBlockIfKnown (DeviceInfo info)
    {
        const auto uid = getBlockUIDFromSerialNumber (info.serial);

        for (int i = disconnectedBlocks.size(); --i >= 0;)
        {
            if (uid != disconnectedBlocks.getUnchecked (i)->uid)
                continue;

            auto block = disconnectedBlocks.removeAndReturn (i);

            if (auto* blockImpl = BlockImpl::getFrom (*block))
            {
                blockImpl->markReconnected (info);
                currentTopology.blocks.add (block);
                return true;
            }
        }

        return false;
    }

    void addNewBlock (DeviceInfo info)
    {
        currentTopology.blocks.add (new BlockImpl (info.serial, *this, info.version,
                                                   info.name, info.isMaster));
    }

    void updateCurrentBlockInfo (Block::Ptr blockToUpdate, DeviceInfo& updatedInfo)
    {
        jassert (updatedInfo.uid == blockToUpdate->uid);

        if (versionNumberChanged (updatedInfo, blockToUpdate->versionNumber))
            setVersionNumberForBlock (updatedInfo, *blockToUpdate);

        if (updatedInfo.name.isValid())
            setNameForBlock (updatedInfo, *blockToUpdate);

        if (updatedInfo.isMaster != blockToUpdate->isMasterBlock())
            BlockImpl::getFrom (*blockToUpdate)->setToMaster (updatedInfo.isMaster);
    }

    BlockImpl* getBlockImplementationWithUID (Block::UID deviceID) const noexcept
    {
        if (auto&& block = currentTopology.getBlockWithUID (deviceID))
            return BlockImpl::getFrom (*block);

            return nullptr;
    }

    juce::OwnedArray<ConnectedDeviceGroup<Detector>> connectedDeviceGroups;

    //==============================================================================
    /** This is a friend of the BlocksImplementation that will scan and set the
     physical positions of the blocks */
    struct BlocksTraverser
    {
        void traverseBlockArray (const BlockTopology& topology)
        {
            juce::Array<Block::UID> visited;

            for (auto& block : topology.blocks)
            {
                if (block->isMasterBlock() && ! visited.contains (block->uid))
                {
                    if (auto* bi = dynamic_cast<BlockImpl*> (block))
                    {
                        bi->masterUID = {};
                        bi->position = {};
                        bi->rotation = 0;
                    }

                    layoutNeighbours (*block, topology, block->uid, visited);
                }
            }
        }

        // returns the distance from corner clockwise
        int getUnitForIndex (Block::Ptr block, Block::ConnectionPort::DeviceEdge edge, int index)
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
        int getRotationForEdge (Block::ConnectionPort::DeviceEdge edge)
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

        void layoutNeighbours (Block::Ptr block, const BlockTopology& topology,
                               Block::UID masterUid, juce::Array<Block::UID>& visited)
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

                        neighbour->masterUID = masterUid;
                        neighbour->rotation = (2 + block->getRotation()
                                               + getRotationForEdge (myPort.edge)
                                               - getRotationForEdge (theirPort.edge)) % 4;

                        Point<int> delta;
                        const auto theirBounds = neighbour->getBlockAreaWithinLayout();

                        switch ((block->getRotation() + getRotationForEdge (myPort.edge)) % 4)
                        {
                            case 0: // over me
                                delta = { myOffset - (theirBounds.getWidth() - (theirOffset + 1)), -theirBounds.getHeight() };
                                break;
                            case 1: // right of me
                                delta = { myBounds.getWidth(), myOffset - (theirBounds.getHeight() - (theirOffset + 1)) };
                                break;
                            case 2: // under me
                                delta = { (myBounds.getWidth() - (myOffset + 1)) - theirOffset, myBounds.getHeight() };
                                break;
                            case 3: // left of me
                                delta = { -theirBounds.getWidth(), (myBounds.getHeight() - (myOffset + 1)) - theirOffset };
                                break;
                        }

                        neighbour->position = myBounds.getPosition() + delta;
                    }

                    layoutNeighbours (neighbourPtr, topology, masterUid, visited);
                }
            }
        }
    };

    //==============================================================================
   #if DUMP_TOPOLOGY
    static juce::String idToSerialNum (const BlockTopology& topology, Block::UID uid)
    {
        for (auto* b : topology.blocks)
            if (b->uid == uid)
                return b->serialNumber;

        return "???";
    }

    static juce::String portEdgeToString (Block::ConnectionPort port)
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

    static juce::String portToString (Block::ConnectionPort port)
    {
        return portEdgeToString (port) + "_" + juce::String (port.index);
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

            m << "  Battery level: " + juce::String (juce::roundToInt (100.0f * block->getBatteryLevel())) + "%" << newLine
            << "  Battery charging: " + juce::String (block->isBatteryCharging() ? "y" : "n") << newLine
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
    void broadcastTopology()
    {
        if (currentTopology != lastTopology)
        {
            lastTopology = currentTopology;

            BlocksTraverser traverser;
            traverser.traverseBlockArray (currentTopology);

            for (auto* d : activeTopologySources)
                d->listeners.call ([] (TopologySource::Listener& l) { l.topologyChanged(); });

           #if DUMP_TOPOLOGY
            dumpTopology (lastTopology);
           #endif
        }
    }

    JUCE_DECLARE_WEAK_REFERENCEABLE (Detector)
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Detector)
};

} // namespace juce

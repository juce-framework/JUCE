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
    static Block::Timestamp deviceTimestampToHost (uint32 timestamp) noexcept
    {
        return static_cast<Block::Timestamp> (timestamp);
    }

    template <typename V>
    static int removeUnusedBlocksFromMap (std::map<Block::UID, V>& mapToClean, const juce::Array<DeviceInfo>& currentDevices)
    {
        int numRemoved = 0;

        for (auto iter = mapToClean.begin(); iter != mapToClean.end();)
        {
            bool found = false;

            for (auto& info : currentDevices)
            {
                if (info.uid == iter->first)
                {
                    found = true;
                    break;
                }
            }

            if (found)
            {
                ++iter;
            }
            else
            {
                mapToClean.erase (iter++);
                ++numRemoved;
            }
        }

        return numRemoved;
    }

    /* Returns true new element added to map or value of existing key changed */
    template <typename V>
    static bool insertOrAssign (std::map<Block::UID, V>& map, const Block::UID& key, const V& value)
    {
        const auto result = map.emplace (std::make_pair (key, value));

        if (! result.second)
        {
            if (result.first->second == value)
                return false;

            result.first->second = value;
        }

        return true;
    }
}

template <typename Detector>
struct ConnectedDeviceGroup  : private juce::AsyncUpdater,
                               private juce::Timer
{
    //==============================================================================
    ConnectedDeviceGroup (Detector& d, const juce::String& name, PhysicalTopologySource::DeviceConnection* connection)
        : detector (d), deviceName (name), deviceConnection (connection)
    {


        if (auto midiDeviceConnection = static_cast<MIDIDeviceConnection*> (deviceConnection.get()))
        {
            depreciatedVersionReader = std::make_unique<DepreciatedVersionReader> (*midiDeviceConnection);

            juce::ScopedLock lock (midiDeviceConnection->criticalSecton);
            setMidiMessageCallback();
        }
        else
        {
            setMidiMessageCallback();
        }

        startTimer (200);
        sendTopologyRequest();
    }

    bool isStillConnected (const juce::StringArray& detectedDevices) const noexcept
    {
        return detectedDevices.contains (deviceName) && ! failedToGetTopology();
    }

    int getIndexFromDeviceID (Block::UID uid) const noexcept
    {
        for (auto& d : currentDeviceInfo)
            if (d.uid == uid)
                return d.index;

        return -1;
    }

    const BlocksProtocol::DeviceStatus* getLastStatus (Block::UID deviceID) const noexcept
    {
        for (auto&& status : currentTopologyDevices)
            if (getBlockUIDFromSerialNumber (status.serialNumber) == deviceID)
                return &status;

        return nullptr;
    }

    void notifyBlockIsRestarting (Block::UID deviceID)
    {
        forceApiDisconnected (deviceID);
    }

    //==============================================================================
    // The following methods will be called by the HostPacketDecoder:
    void beginTopology (int numDevices, int numConnections)
    {
        incomingTopologyDevices.clearQuick();
        incomingTopologyDevices.ensureStorageAllocated (numDevices);
        incomingTopologyConnections.clearQuick();
        incomingTopologyConnections.ensureStorageAllocated (numConnections);
    }

    void extendTopology (int numDevices, int numConnections)
    {
        incomingTopologyDevices.ensureStorageAllocated (incomingTopologyDevices.size() + numDevices);
        incomingTopologyConnections.ensureStorageAllocated (incomingTopologyConnections.size() + numConnections);
    }

    void handleTopologyDevice (BlocksProtocol::DeviceStatus status)
    {
        incomingTopologyDevices.add (status);
    }

    void handleTopologyConnection (BlocksProtocol::DeviceConnection connection)
    {
        incomingTopologyConnections.add (connection);
    }

    void endTopology()
    {
        lastTopologyReceiveTime = juce::Time::getCurrentTime();

        if (incomingTopologyDevices.isEmpty())
        {
            jassertfalse;
            return;
        }

        currentTopologyDevices.swapWith (incomingTopologyDevices);
        currentTopologyConnections.swapWith (incomingTopologyConnections);

        incomingTopologyDevices.clearQuick();
        incomingTopologyConnections.clearQuick();

        refreshCurrentDeviceInfo();
        refreshCurrentDeviceConnections();

        removeUnusedBlocksFromMap (versionNumbers, currentDeviceInfo);
        removeUnusedBlocksFromMap (blockNames, currentDeviceInfo);

        removePingForDisconnectedBlocks();

        detector.handleTopologyChange();
    }

    void handleVersion (BlocksProtocol::DeviceVersion version)
    {
        const auto uid = getDeviceIDFromIndex (version.index);

        if (uid == Block::UID() || version.version.length <= 1)
            return;

        setVersion (uid, version.version);
    }

    void handleName (BlocksProtocol::DeviceName name)
    {
        const auto uid = getDeviceIDFromIndex (name.index);

        if (uid == Block::UID() || name.name.length <= 1)
            return;

        if (insertOrAssign (blockNames, uid, name.name))
        {
            refreshCurrentDeviceInfo();
            detector.handleTopologyChange();
        }
    }

    void handleControlButtonUpDown (BlocksProtocol::TopologyIndex deviceIndex, uint32 timestamp,
                                    BlocksProtocol::ControlButtonID buttonID, bool isDown)
    {
        if (auto deviceID = getDeviceIDFromMessageIndex (deviceIndex))
            detector.handleButtonChange (deviceID, deviceTimestampToHost (timestamp), buttonID.get(), isDown);
    }

    void handleCustomMessage (BlocksProtocol::TopologyIndex deviceIndex, uint32 timestamp, const int32* data)
    {
        if (auto deviceID = getDeviceIDFromMessageIndex (deviceIndex))
            detector.handleCustomMessage (deviceID, deviceTimestampToHost (timestamp), data);
    }

    void handleTouchChange (BlocksProtocol::TopologyIndex deviceIndex,
                            uint32 timestamp,
                            BlocksProtocol::TouchIndex touchIndex,
                            BlocksProtocol::TouchPosition position,
                            BlocksProtocol::TouchVelocity velocity,
                            bool isStart, bool isEnd)
    {
        if (auto deviceID = getDeviceIDFromMessageIndex (deviceIndex))
        {
            TouchSurface::Touch touch;

            touch.index             = (int) touchIndex.get();
            touch.x                 = position.x.toUnipolarFloat();
            touch.y                 = position.y.toUnipolarFloat();
            touch.z                 = position.z.toUnipolarFloat();
            touch.xVelocity         = velocity.vx.toBipolarFloat();
            touch.yVelocity         = velocity.vy.toBipolarFloat();
            touch.zVelocity         = velocity.vz.toBipolarFloat();
            touch.eventTimestamp    = deviceTimestampToHost (timestamp);
            touch.isTouchStart      = isStart;
            touch.isTouchEnd        = isEnd;
            touch.blockUID          = deviceID;

            setTouchStartPosition (touch);

            detector.handleTouchChange (deviceID, touch);
        }
    }

    void setTouchStartPosition (TouchSurface::Touch& touch)
    {
        auto& startPos = touchStartPositions.getValue (touch);

        if (touch.isTouchStart)
            startPos = { touch.x, touch.y };

        touch.startX = startPos.x;
        touch.startY = startPos.y;
    }

    void handlePacketACK (BlocksProtocol::TopologyIndex deviceIndex,
                          BlocksProtocol::PacketCounter counter)
    {
        if (auto deviceID = getDeviceIDFromMessageIndex (deviceIndex))
        {
            detector.handleSharedDataACK (deviceID, counter);
            updateApiPing (deviceID);
        }
    }

    void handleFirmwareUpdateACK (BlocksProtocol::TopologyIndex deviceIndex,
                                  BlocksProtocol::FirmwareUpdateACKCode resultCode,
                                  BlocksProtocol::FirmwareUpdateACKDetail resultDetail)
    {
        if (auto deviceID = getDeviceIDFromMessageIndex (deviceIndex))
        {
            detector.handleFirmwareUpdateACK (deviceID, (uint8) resultCode.get(), (uint32) resultDetail.get());
            updateApiPing (deviceID);
        }
    }

    void handleConfigUpdateMessage (BlocksProtocol::TopologyIndex deviceIndex,
                                    int32 item, int32 value, int32 min, int32 max)
    {
        if (auto deviceID = getDeviceIDFromMessageIndex (deviceIndex))
            detector.handleConfigUpdateMessage (deviceID, item, value, min, max);
    }

    void handleConfigSetMessage (BlocksProtocol::TopologyIndex deviceIndex,
                                 int32 item, int32 value)
    {
        if (auto deviceID = getDeviceIDFromMessageIndex (deviceIndex))
            detector.handleConfigSetMessage (deviceID, item, value);
    }

    void handleConfigFactorySyncEndMessage (BlocksProtocol::TopologyIndex deviceIndex)
    {
        if (auto deviceID = getDeviceIDFromMessageIndex (deviceIndex))
            detector.handleConfigFactorySyncEndMessage (deviceID);
    }

    void handleConfigFactorySyncResetMessage (BlocksProtocol::TopologyIndex deviceIndex)
    {
        if (auto deviceID = getDeviceIDFromMessageIndex (deviceIndex))
            detector.handleConfigFactorySyncResetMessage (deviceID);
    }

    void handleLogMessage (BlocksProtocol::TopologyIndex deviceIndex, const String& message)
    {
        if (auto deviceID = getDeviceIDFromMessageIndex (deviceIndex))
            detector.handleLogMessage (deviceID, message);
    }

    //==============================================================================
    template <typename PacketBuilder>
    bool sendMessageToDevice (const PacketBuilder& builder) const
    {
        if (deviceConnection->sendMessageToDevice (builder.getData(), (size_t) builder.size()))
        {
           #if DUMP_BANDWIDTH_STATS
            registerBytesOut (builder.size());
           #endif
            return true;
        }

        return false;
    }

    PhysicalTopologySource::DeviceConnection* getDeviceConnection()
    {
        return deviceConnection.get();
    }

    juce::Array<DeviceInfo> getCurrentDeviceInfo()
    {
        auto blocks = currentDeviceInfo;
        blocks.removeIf ([this] (DeviceInfo& info) { return ! isApiConnected (info.uid); });
        return blocks;
    }

    juce::Array<BlockDeviceConnection> getCurrentDeviceConnections()
    {
        auto connections = currentDeviceConnections;
        connections.removeIf ([this] (BlockDeviceConnection& c) { return ! isApiConnected (c.device1) || ! isApiConnected (c.device2); });
        return connections;
    }

    Detector& detector;
    juce::String deviceName;

    static constexpr double pingTimeoutSeconds = 6.0;

private:
    //==============================================================================
    juce::Array<DeviceInfo> currentDeviceInfo;
    juce::Array<BlockDeviceConnection> currentDeviceConnections;
    std::unique_ptr<PhysicalTopologySource::DeviceConnection> deviceConnection;

    juce::Array<BlocksProtocol::DeviceStatus> incomingTopologyDevices, currentTopologyDevices;
    juce::Array<BlocksProtocol::DeviceConnection> incomingTopologyConnections, currentTopologyConnections;

    juce::CriticalSection incomingPacketLock;
    juce::Array<juce::MemoryBlock> incomingPackets;

    std::map<Block::UID, BlocksProtocol::VersionNumber> versionNumbers;
    std::map<Block::UID, BlocksProtocol::BlockName> blockNames;

    std::unique_ptr<DepreciatedVersionReader> depreciatedVersionReader;

    struct TouchStart { float x, y; };
    TouchList<TouchStart> touchStartPositions;

    Block::UID masterBlock = 0;

    //==============================================================================
    void setMidiMessageCallback()
    {
        deviceConnection->handleMessageFromDevice = [this] (const void* data, size_t dataSize)
        {
            this->handleIncomingMessage (data, dataSize);
        };
    }

    //==============================================================================
    juce::Time lastTopologyRequestTime, lastTopologyReceiveTime;
    int numTopologyRequestsSent = 0;

    void scheduleNewTopologyRequest()
    {
        numTopologyRequestsSent = 0;
        lastTopologyReceiveTime = juce::Time();
        lastTopologyRequestTime = juce::Time::getCurrentTime();
    }

    void sendTopologyRequest()
    {
        ++numTopologyRequestsSent;
        lastTopologyRequestTime = juce::Time::getCurrentTime();
        sendCommandMessage (0, BlocksProtocol::requestTopologyMessage);
    }

    void timerCallback() override
    {
        const auto now = juce::Time::getCurrentTime();

        if ((now > lastTopologyReceiveTime + juce::RelativeTime::seconds (30.0))
            && now > lastTopologyRequestTime + juce::RelativeTime::seconds (1.0)
            && numTopologyRequestsSent < 4)
            sendTopologyRequest();

        checkApiTimeouts (now);
        startApiModeOnConnectedBlocks();
        requestMasterBlockVersionIfNeeded();
    }

    bool failedToGetTopology() const noexcept
    {
        return numTopologyRequestsSent >= 4 && lastTopologyReceiveTime == juce::Time();
    }

    bool sendCommandMessage (BlocksProtocol::TopologyIndex deviceIndex, uint32 commandID) const
    {
        BlocksProtocol::HostPacketBuilder<64> p;
        p.writePacketSysexHeaderBytes (deviceIndex);
        p.deviceControlMessage (commandID);
        p.writePacketSysexFooter();
        return sendMessageToDevice (p);
    }

    //==============================================================================
    void requestMasterBlockVersionIfNeeded()
    {
        if (depreciatedVersionReader == nullptr)
            return;

        const auto masterVersion = depreciatedVersionReader->getVersionNumber();

        if (masterVersion.isNotEmpty())
            setVersion (masterBlock, masterVersion);
    }

    void setVersion (const Block::UID uid, const BlocksProtocol::VersionNumber versionNumber)
    {
        if (uid == masterBlock)
            depreciatedVersionReader.reset();

        if (insertOrAssign (versionNumbers, uid, versionNumber))
        {
            refreshCurrentDeviceInfo();
            detector.handleTopologyChange();
        }
    }

    //==============================================================================
    struct BlockPingTime
    {
        Block::UID blockUID;
        juce::Time lastPing;
        juce::Time connected;
    };

    juce::Array<BlockPingTime> blockPings;

    void updateApiPing (Block::UID uid)
    {
        const auto now = juce::Time::getCurrentTime();

        if (auto* ping = getPing (uid))
        {
            LOG_PING ("Ping: " << uid << " " << now.formatted ("%Mm %Ss"));
            ping->lastPing = now;
        }
        else
        {
            LOG_CONNECTIVITY ("API Connected " << uid);
            blockPings.add ({ uid, now, now });
            detector.handleTopologyChange();
        }
    }

    BlockPingTime* getPing (Block::UID uid)
    {
        for (auto& ping : blockPings)
            if (uid == ping.blockUID)
                return &ping;

        return nullptr;
    }

    void removeDeviceInfo (Block::UID uid)
    {
        currentDeviceInfo.removeIf ([uid] (DeviceInfo& info) { return uid == info.uid; });
    }

    bool isApiConnected (Block::UID uid)
    {
        return getPing (uid) != nullptr;
    }

    void forceApiDisconnected (Block::UID uid)
    {
        if (isApiConnected (uid))
        {
            // Clear all known API connections and broadcast an empty topology,
            // as DNA blocks connected to the restarting block may be offline.
            LOG_CONNECTIVITY ("API Disconnected " << uid << ", re-probing topology");
            currentDeviceInfo.clearQuick();
            blockPings.clearQuick();
            blockNames.clear();
            versionNumbers.clear();
            detector.handleTopologyChange();
            scheduleNewTopologyRequest();
        }
    }

    void checkApiTimeouts (juce::Time now)
    {
        const auto timedOut = [this, now] (BlockPingTime& ping)
        {
            if (ping.lastPing >= now - juce::RelativeTime::seconds (pingTimeoutSeconds))
                return false;

            LOG_CONNECTIVITY ("Ping timeout: " << ping.blockUID);
            removeDeviceInfo (ping.blockUID);
            return true;
        };

        if (blockPings.removeIf (timedOut) > 0)
        {
            scheduleNewTopologyRequest();
            detector.handleTopologyChange();
        }
    }

    /* Returns true is ping was removed */
    void removePingForDisconnectedBlocks()
    {
        const auto removed = [this] (auto& ping)
        {
            for (auto& info : currentDeviceInfo)
                if (info.uid == ping.blockUID)
                    return false;

            LOG_CONNECTIVITY ("API Disconnected by topology update " << ping.blockUID);
            return true;
        };

        blockPings.removeIf (removed);
    }

    void startApiModeOnConnectedBlocks()
    {
        for (auto& info : currentDeviceInfo)
        {
            if (! isApiConnected (info.uid))
            {
                LOG_CONNECTIVITY ("API Try " << info.uid);
                sendCommandMessage (info.index, BlocksProtocol::endAPIMode);
                sendCommandMessage (info.index, BlocksProtocol::beginAPIMode);
            }
        }
    }

    //==============================================================================
    void checkVersionNumberTimeouts()
    {
        for (const auto& device : currentDeviceInfo)
        {
            const auto version = versionNumbers.find (device.uid);

            if (version == versionNumbers.end())
            {
                auto* ping = getPing (device.uid);

            }
        }
    }

    //==============================================================================
    Block::UID getDeviceIDFromIndex (BlocksProtocol::TopologyIndex index) const noexcept
    {
        for (auto& d : currentDeviceInfo)
            if (d.index == index)
                return d.uid;

        return {};
    }

    Block::UID getDeviceIDFromMessageIndex (BlocksProtocol::TopologyIndex index) noexcept
    {
        const auto uid = getDeviceIDFromIndex (index);

        // re-request topology if we get an event from an unknown block
        if (uid == Block::UID())
            scheduleNewTopologyRequest();

        return uid;
    }

    //==============================================================================
    void handleIncomingMessage (const void* data, size_t dataSize)
    {
        juce::MemoryBlock mb (data, dataSize);

        {
            const juce::ScopedLock sl (incomingPacketLock);
            incomingPackets.add (std::move (mb));
        }

        triggerAsyncUpdate();

       #if DUMP_BANDWIDTH_STATS
        registerBytesIn ((int) dataSize);
       #endif
    }

    void handleAsyncUpdate() override
    {
        juce::Array<juce::MemoryBlock> packets;
        packets.ensureStorageAllocated (32);

        {
            const juce::ScopedLock sl (incomingPacketLock);
            incomingPackets.swapWith (packets);
        }

        for (auto& packet : packets)
        {
            auto data = static_cast<const uint8*> (packet.getData());

            BlocksProtocol::HostPacketDecoder<ConnectedDeviceGroup>
            ::processNextPacket (*this, *data, data + 1, (int) packet.getSize() - 1);
        }
    }

    //==============================================================================
    BlocksProtocol::VersionNumber getVersionNumber (Block::UID uid)
    {
        const auto version = versionNumbers.find (uid);
        return version == versionNumbers.end() ? BlocksProtocol::VersionNumber() : version->second;
    }

    BlocksProtocol::BlockName getName (Block::UID uid)
    {
        const auto name = blockNames.find (uid);
        return name == blockNames.end() ? BlocksProtocol::BlockName() : name->second;
    }

    //==============================================================================
    const DeviceInfo* getDeviceInfoFromUID (Block::UID uid) const noexcept
    {
        for (auto& d : currentDeviceInfo)
            if (d.uid == uid)
                return &d;

        return nullptr;
    }

    Block::ConnectionPort convertConnectionPort (Block::UID uid, BlocksProtocol::ConnectorPort p) noexcept
    {
        if (auto* info = getDeviceInfoFromUID (uid))
            return BlocksProtocol::BlockDataSheet (info->serial).convertPortIndexToConnectorPort (p);

            jassertfalse;
        return { Block::ConnectionPort::DeviceEdge::north, 0 };
    }

    //==============================================================================
    void refreshCurrentDeviceInfo()
    {
        currentDeviceInfo.clearQuick();

        for (auto& device : currentTopologyDevices)
        {
            const auto uid = getBlockUIDFromSerialNumber (device.serialNumber);
            const auto version = getVersionNumber (uid);
            const auto name = getName (uid);

            // For backwards compatibility we assume the first device we see in a group is the master and won't change
            if (masterBlock == 0)
                masterBlock = uid;

            currentDeviceInfo.add ({ uid,
                                     device.index,
                                     device.serialNumber,
                                     version,
                                     name,
                                     masterBlock == uid });
       }
    }

    void refreshCurrentDeviceConnections()
    {
        currentDeviceConnections.clearQuick();

        for (auto&& c : currentTopologyConnections)
        {
            BlockDeviceConnection dc;
            dc.device1 = getDeviceIDFromIndex (c.device1);
            dc.device2 = getDeviceIDFromIndex (c.device2);

            if (dc.device1 <= 0 || dc.device2 <= 0)
                continue;

            dc.connectionPortOnDevice1 = convertConnectionPort (dc.device1, c.port1);
            dc.connectionPortOnDevice2 = convertConnectionPort (dc.device2, c.port2);

            currentDeviceConnections.add (dc);
        }
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ConnectedDeviceGroup)
};

} // namespace juce

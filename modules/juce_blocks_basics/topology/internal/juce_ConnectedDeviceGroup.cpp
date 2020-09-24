/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

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
}

template <typename Detector>
struct ConnectedDeviceGroup  : private AsyncUpdater,
                               private Timer
{
    //==============================================================================
    ConnectedDeviceGroup (Detector& d, const String& name, PhysicalTopologySource::DeviceConnection* connection)
        : detector (d), deviceName (name), deviceConnection (connection)
    {
        if (auto midiDeviceConnection = static_cast<MIDIDeviceConnection*> (deviceConnection.get()))
        {
            ScopedLock lock (midiDeviceConnection->criticalSecton);
            setMidiMessageCallback();
        }
        else
        {
            setMidiMessageCallback();
        }

        initialiseSerialReader();

        startTimer (200);
        sendTopologyRequest();
    }

    ~ConnectedDeviceGroup() override
    {
        for (const auto& device : currentDeviceInfo)
            detector.handleDeviceRemoved (device);
    }

    bool isStillConnected (const StringArray& detectedDevices) const noexcept
    {
        return detectedDevices.contains (deviceName) && ! failedToGetTopology();
    }

    bool contains (Block::UID uid)
    {
        return getIndexFromDeviceID (uid) >= 0;
    }

    void handleBlockRestarting (Block::UID deviceID)
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
        lastTopologyReceiveTime = Time::getCurrentTime();

        if (incomingTopologyDevices.isEmpty()
            || incomingTopologyConnections.size() < incomingTopologyDevices.size() - 1)
        {
            LOG_CONNECTIVITY ("Invalid topology or device list received.");
            LOG_CONNECTIVITY ("Device size : " << incomingTopologyDevices.size());
            LOG_CONNECTIVITY ("Connections size : " << incomingTopologyConnections.size());
            scheduleNewTopologyRequest();
            return;
        }

        LOG_CONNECTIVITY ("Valid topology received");

        updateCurrentDeviceList();
        updateCurrentDeviceConnections();
    }

    void handleVersion (BlocksProtocol::DeviceVersion version)
    {
        setVersion (version.index, version.version);
    }

    void handleName (BlocksProtocol::DeviceName name)
    {
        if (name.name.length <= 1)
            return;

        if (const auto info = getDeviceInfoFromIndex (name.index))
        {
            if (info->name == name.name)
                return;

            info->name = name.name;
            detector.handleDeviceUpdated (*info);
        }
    }

    void handleControlButtonUpDown (BlocksProtocol::TopologyIndex deviceIndex, uint32 timestamp,
                                    BlocksProtocol::ControlButtonID buttonID, bool isDown)
    {
        if (auto deviceID = getDeviceIDFromIndex (deviceIndex))
            detector.handleButtonChange (deviceID, deviceTimestampToHost (timestamp), buttonID.get(), isDown);
    }

    void handleCustomMessage (BlocksProtocol::TopologyIndex deviceIndex, uint32 timestamp, const int32* data)
    {
        if (auto deviceID = getDeviceIDFromIndex (deviceIndex))
            detector.handleCustomMessage (deviceID, deviceTimestampToHost (timestamp), data);
    }

    void handleTouchChange (BlocksProtocol::TopologyIndex deviceIndex,
                            uint32 timestamp,
                            BlocksProtocol::TouchIndex touchIndex,
                            BlocksProtocol::TouchPosition position,
                            BlocksProtocol::TouchVelocity velocity,
                            bool isStart, bool isEnd)
    {
        if (auto deviceID = getDeviceIDFromIndex (deviceIndex))
        {
            TouchSurface::Touch touch;

            touch.index             = (int) touchIndex.get();
            touch.x                 = (float) position.x.toUnipolarFloat();
            touch.y                 = (float) position.y.toUnipolarFloat();
            touch.z                 = (float) position.z.toUnipolarFloat();
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
        if (auto deviceID = getDeviceIDFromIndex (deviceIndex))
        {
            detector.handleSharedDataACK (deviceID, counter);
            updateApiPing (deviceID);
        }
    }

    void handleFirmwareUpdateACK (BlocksProtocol::TopologyIndex deviceIndex,
                                  BlocksProtocol::FirmwareUpdateACKCode resultCode,
                                  BlocksProtocol::FirmwareUpdateACKDetail resultDetail)
    {
        if (auto deviceID = getDeviceIDFromIndex (deviceIndex))
        {
            detector.handleFirmwareUpdateACK (deviceID, (uint8) resultCode.get(), (uint32) resultDetail.get());
            updateApiPing (deviceID);
        }
    }

    void handleConfigUpdateMessage (BlocksProtocol::TopologyIndex deviceIndex,
                                    int32 item, int32 value, int32 min, int32 max)
    {
        if (auto deviceID = getDeviceIDFromIndex (deviceIndex))
            detector.handleConfigUpdateMessage (deviceID, item, value, min, max);
    }

    void handleConfigSetMessage (BlocksProtocol::TopologyIndex deviceIndex,
                                 int32 item, int32 value)
    {
        if (auto deviceID = getDeviceIDFromIndex (deviceIndex))
            detector.handleConfigSetMessage (deviceID, item, value);
    }

    void handleConfigFactorySyncEndMessage (BlocksProtocol::TopologyIndex deviceIndex)
    {
        if (auto deviceID = getDeviceIDFromIndex (deviceIndex))
            detector.handleConfigFactorySyncEndMessage (deviceID);
    }

    void handleConfigFactorySyncResetMessage (BlocksProtocol::TopologyIndex deviceIndex)
    {
        if (auto deviceID = getDeviceIDFromIndex (deviceIndex))
            detector.handleConfigFactorySyncResetMessage (deviceID);
    }

    void handleLogMessage (BlocksProtocol::TopologyIndex deviceIndex, const String& message)
    {
        if (auto deviceID = getDeviceIDFromIndex (deviceIndex))
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

    Array<BlockDeviceConnection> getCurrentDeviceConnections()
    {
        Array<BlockDeviceConnection> connections;

        for (const auto& connection : currentDeviceConnections)
            if (isApiConnected (getDeviceIDFromIndex (connection.device1)) && isApiConnected (getDeviceIDFromIndex (connection.device2)))
                connections.add (getBlockDeviceConnection (connection));

        return connections;
    }

    Detector& detector;
    String deviceName;

    static constexpr double pingTimeoutSeconds = 6.0;

private:
    //==============================================================================
    Array<DeviceInfo> currentDeviceInfo;
    Array<BlocksProtocol::DeviceStatus> incomingTopologyDevices;
    Array<BlocksProtocol::DeviceConnection> incomingTopologyConnections, currentDeviceConnections;

    std::unique_ptr<PhysicalTopologySource::DeviceConnection> deviceConnection;

    CriticalSection incomingPacketLock;
    Array<MemoryBlock> incomingPackets;

    std::unique_ptr<DepreciatedVersionReader> depreciatedVersionReader;
    std::unique_ptr<BlockSerialReader> masterSerialReader;

    struct TouchStart { float x, y; };
    TouchList<TouchStart> touchStartPositions;

    static constexpr Block::UID invalidUid = 0;
    Block::UID masterBlockUid = invalidUid;

    //==============================================================================
    void timerCallback() override
    {
        const auto now = Time::getCurrentTime();

        if ((now > lastTopologyReceiveTime + RelativeTime::seconds (30.0))
            && now > lastTopologyRequestTime + RelativeTime::seconds (1.0)
            && numTopologyRequestsSent < 4)
            sendTopologyRequest();

        checkApiTimeouts (now);
        startApiModeOnConnectedBlocks();
        checkMasterBlockVersion();
        checkMasterSerial();
    }

    //==============================================================================
    void setMidiMessageCallback()
    {
        deviceConnection->handleMessageFromDevice = [this] (const void* data, size_t dataSize)
        {
            this->handleIncomingMessage (data, dataSize);
        };
    }

    void handleIncomingMessage (const void* data, size_t dataSize)
    {
        MemoryBlock mb (data, dataSize);

        {
            const ScopedLock sl (incomingPacketLock);
            incomingPackets.add (std::move (mb));
        }

        triggerAsyncUpdate();

       #if DUMP_BANDWIDTH_STATS
        registerBytesIn ((int) dataSize);
       #endif
    }

    void handleAsyncUpdate() override
    {
        Array<MemoryBlock> packets;
        packets.ensureStorageAllocated (32);

        {
            const ScopedLock sl (incomingPacketLock);
            incomingPackets.swapWith (packets);
        }

        for (auto& packet : packets)
        {
            auto data = static_cast<const uint8*> (packet.getData());

            BlocksProtocol::HostPacketDecoder<ConnectedDeviceGroup>
            ::processNextPacket (*this, *data, data + 1, (int) packet.getSize() - 1);
        }
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
    Time lastTopologyRequestTime, lastTopologyReceiveTime;
    int numTopologyRequestsSent = 0;

    void scheduleNewTopologyRequest()
    {
        LOG_CONNECTIVITY ("Topology Request Scheduled");
        numTopologyRequestsSent = 0;
        lastTopologyReceiveTime = Time();
        lastTopologyRequestTime = Time::getCurrentTime();
    }

    void sendTopologyRequest()
    {
        ++numTopologyRequestsSent;
        lastTopologyRequestTime = Time::getCurrentTime();
        sendCommandMessage (0, BlocksProtocol::requestTopologyMessage);
    }

    bool failedToGetTopology() const noexcept
    {
        return numTopologyRequestsSent >= 4 && lastTopologyReceiveTime == Time();
    }

    //==============================================================================
    void checkMasterBlockVersion()
    {
        if (depreciatedVersionReader == nullptr)
            return;

        const auto masterVersion = depreciatedVersionReader->getVersionNumber();

        if (masterVersion.isNotEmpty())
        {
            const auto masterIndex = getIndexFromDeviceID (masterBlockUid);

            if (masterIndex >= 0)
                setVersion (BlocksProtocol::TopologyIndex (masterIndex), masterVersion);
            else
                jassertfalse;
        }
    }

    void setVersion (const BlocksProtocol::TopologyIndex index, const BlocksProtocol::VersionNumber versionNumber)
    {
        if (versionNumber.length <= 1)
            return;

        if (const auto info = getDeviceInfoFromIndex (index))
        {
            if (info->version == versionNumber)
                return;

            if (info->uid == masterBlockUid)
                depreciatedVersionReader.reset();

            info->version = versionNumber;
            detector.handleDeviceUpdated (*info);
        }
    }

    //==============================================================================
    void checkMasterSerial()
    {
        if (masterSerialReader == nullptr)
            initialiseSerialReader();

        if (masterSerialReader == nullptr)
            return;

        if (masterBlockUid != invalidUid && masterSerialReader->hasSerial())
        {
            auto uid = getBlockUIDFromSerialNumber (masterSerialReader->getSerial());

            if (uid != masterBlockUid)
                updateMasterUid (uid);
        }
    }

    void updateMasterUid (const Block::UID newMasterUid)
    {
        LOG_CONNECTIVITY ("Updating master from " + String (masterBlockUid) + " to " + String (newMasterUid));

        masterBlockUid = newMasterUid;

        Array<DeviceInfo> devicesToUpdate;

        for (auto& info : currentDeviceInfo)
        {
            if (info.masterUid != masterBlockUid)
            {
                info.masterUid = masterBlockUid;

                info.isMaster = info.uid == masterBlockUid;

                devicesToUpdate.add (info);
            }
        }

        detector.handleDevicesUpdated (devicesToUpdate);
    }

    Block::UID determineMasterBlockUid (Array<BlocksProtocol::DeviceStatus> devices)
    {
        if (masterSerialReader != nullptr && masterSerialReader->hasSerial())
        {
            auto foundSerial = masterSerialReader->getSerial();
            for (const auto& device : incomingTopologyDevices)
            {
                if (device.serialNumber.asString() == foundSerial)
                {
                    LOG_CONNECTIVITY ("Found master from serial " + foundSerial);
                    return getBlockUIDFromSerialNumber (foundSerial);
                }
            }
        }

        if (devices.size() > 0)
        {
            LOG_CONNECTIVITY ("Found master from first device " + devices[0].serialNumber.asString());
            return getBlockUIDFromSerialNumber (incomingTopologyDevices[0].serialNumber);
        }

        jassertfalse;
        return invalidUid;
    }

    //==============================================================================
    struct BlockPingTime
    {
        Block::UID blockUID;
        Time lastPing;
        Time connected;
    };

    Array<BlockPingTime> blockPings;

    BlockPingTime* getPing (Block::UID uid)
    {
        for (auto& ping : blockPings)
            if (uid == ping.blockUID)
                return &ping;

        return nullptr;
    }

    void removePing (Block::UID uid)
    {
        const auto remove = [uid] (const BlockPingTime& ping)
        {
            if (uid == ping.blockUID)
            {
                LOG_CONNECTIVITY ("API Disconnected by topology update " << ping.blockUID);
                return true;
            }

            return false;
        };

        blockPings.removeIf (remove);
    }

    void updateApiPing (Block::UID uid)
    {
        const auto now = Time::getCurrentTime();

        if (auto* ping = getPing (uid))
        {
            LOG_PING ("Ping: " << uid << " " << now.formatted ("%Mm %Ss"));
            ping->lastPing = now;
        }
        else
        {
            LOG_CONNECTIVITY ("API Connected " << uid);
            blockPings.add ({ uid, now, now });

            if (const auto info = getDeviceInfoFromUID (uid))
                detector.handleDeviceAdded (*info);
        }
    }

    bool isApiConnected (Block::UID uid)
    {
        return getPing (uid) != nullptr;
    }

    void forceApiDisconnected (Block::UID uid)
    {
        for (auto dependentUID : detector.getDnaDependentDeviceUIDs (uid))
            removeDevice (dependentUID);

        removeDevice (uid);

        if (uid == masterBlockUid)
        {
            masterBlockUid = invalidUid;
            masterSerialReader.reset();
        }

        scheduleNewTopologyRequest();
    }

    void checkApiTimeouts (Time now)
    {
        Array<Block::UID> toRemove;

        for (const auto& ping : blockPings)
        {
            if (ping.lastPing < now - RelativeTime::seconds (pingTimeoutSeconds))
            {
                LOG_CONNECTIVITY ("Ping timeout: " << ping.blockUID);
                toRemove.add (ping.blockUID);
                scheduleNewTopologyRequest();
            }
        }

        for (const auto& uid : toRemove)
            removeDevice (uid);
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
    Block::UID getDeviceIDFromIndex (BlocksProtocol::TopologyIndex index) noexcept
    {
        for (const auto& device : currentDeviceInfo)
            if (device.index == index)
                return device.uid;

        scheduleNewTopologyRequest();
        return {};
    }

    int getIndexFromDeviceID (Block::UID uid) const noexcept
    {
        for (auto& d : currentDeviceInfo)
            if (d.uid == uid)
                return d.index;

        return -1;
    }

    DeviceInfo* getDeviceInfoFromUID (Block::UID uid) noexcept
    {
        for (auto& d : currentDeviceInfo)
            if (d.uid == uid)
                return &d;

        return nullptr;
    }

    DeviceInfo* getDeviceInfoFromIndex (BlocksProtocol::TopologyIndex index) noexcept
    {
        for (auto& d : currentDeviceInfo)
            if (d.index == index)
                return &d;

        return nullptr;
    }

    void removeDeviceInfo (Block::UID uid)
    {
        currentDeviceInfo.removeIf ([uid] (const DeviceInfo& info) { return info.uid == uid; });
    }

    const DeviceStatus* getIncomingDeviceStatus (BlockSerialNumber serialNumber) const
    {
        for (auto& device : incomingTopologyDevices)
            if (device.serialNumber == serialNumber)
                return &device;

        return nullptr;
    }

    //==============================================================================
    void removeDevice (Block::UID uid)
    {
        LOG_CONNECTIVITY ("Removing device: " << uid);

        if (const auto info = getDeviceInfoFromUID (uid))
            detector.handleDeviceRemoved (*info);

        removeDeviceInfo (uid);
        removePing (uid);
    }

    void updateCurrentDeviceList()
    {
        Array<Block::UID> toRemove;

        //Update known devices
        for (int i = currentDeviceInfo.size(); --i >= 0; )
        {
            auto& currentDevice = currentDeviceInfo.getReference (i);

            if (const auto newStatus = getIncomingDeviceStatus (currentDevice.serial))
            {
                if (currentDevice.index != newStatus->index)
                {
                    currentDevice.index = newStatus->index;
                    detector.handleIndexChanged (currentDevice.uid, currentDevice.index);
                }

                if (currentDevice.batteryCharging != newStatus->batteryCharging)
                {
                    currentDevice.batteryCharging = newStatus->batteryCharging;
                    detector.handleBatteryChargingChanged (currentDevice.uid, currentDevice.batteryCharging);
                }

                if (currentDevice.batteryLevel != newStatus->batteryLevel)
                {
                    currentDevice.batteryLevel = newStatus->batteryLevel;
                    detector.handleBatteryLevelChanged (currentDevice.uid, currentDevice.batteryLevel);
                }
            }
            else
            {
                toRemove.add (currentDevice.uid);
            }
        }

        for (const auto& uid : toRemove)
            removeDevice (uid);

        if (masterBlockUid == invalidUid)
        {
            masterBlockUid = determineMasterBlockUid (incomingTopologyDevices);
            initialiseVersionReader();
        }

        //Add new devices
        for (const auto& device : incomingTopologyDevices)
        {
            const auto uid = getBlockUIDFromSerialNumber (device.serialNumber);

            if (getDeviceInfoFromUID (uid) == nullptr)
            {
                currentDeviceInfo.add ({ uid,
                                         device.index,
                                         device.serialNumber,
                                         BlocksProtocol::VersionNumber(),
                                         BlocksProtocol::BlockName(),
                                         device.batteryLevel,
                                         device.batteryCharging,
                                         masterBlockUid });
            }
        }
    }

    //==============================================================================
    Block::ConnectionPort convertConnectionPort (Block::UID uid, BlocksProtocol::ConnectorPort p) noexcept
    {
        if (auto* info = getDeviceInfoFromUID (uid))
            return BlocksProtocol::BlockDataSheet (info->serial).convertPortIndexToConnectorPort (p);

        jassertfalse;
        return { Block::ConnectionPort::DeviceEdge::north, 0 };
    }

    BlockDeviceConnection getBlockDeviceConnection (const BlocksProtocol::DeviceConnection& connection)
    {
        BlockDeviceConnection dc;

        dc.device1 = getDeviceIDFromIndex (connection.device1);
        dc.device2 = getDeviceIDFromIndex (connection.device2);

        if (dc.device1 <= 0 || dc.device2 <= 0)
            jassertfalse;

        dc.connectionPortOnDevice1 = convertConnectionPort (dc.device1, connection.port1);
        dc.connectionPortOnDevice2 = convertConnectionPort (dc.device2, connection.port2);

        return dc;
    }

    void updateCurrentDeviceConnections()
    {
        currentDeviceConnections.clearQuick();
        currentDeviceConnections.swapWith (incomingTopologyConnections);

        detector.handleConnectionsChanged();
    }

    void initialiseVersionReader()
    {
        if (auto midiDeviceConnection = static_cast<MIDIDeviceConnection*> (deviceConnection.get()))
            depreciatedVersionReader = std::make_unique<DepreciatedVersionReader> (*midiDeviceConnection);
    }

    void initialiseSerialReader()
    {
        if (auto midiDeviceConnection = static_cast<MIDIDeviceConnection*> (deviceConnection.get()))
            masterSerialReader = std::make_unique<BlockSerialReader> (*midiDeviceConnection);
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ConnectedDeviceGroup)
};

} // namespace juce

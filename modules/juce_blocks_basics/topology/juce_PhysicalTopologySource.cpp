/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2016 - ROLI Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

#define JUCE_ASSERT_MESSAGE_MANAGER_IS_LOCKED \
    jassert (juce::MessageManager::getInstance()->currentThreadHasLockedMessageManager());

#if DUMP_BANDWIDTH_STATS
namespace
{
    struct PortIOStats
    {
        PortIOStats (const char* nm) : name (nm) {}

        const char* const name;
        int byteCount           = 0;
        int messageCount        = 0;
        int bytesPerSec         = 0;
        int largestMessageBytes = 0;
        int lastMessageBytes    = 0;

        void update (double elapsedSec)
        {
            if (byteCount > 0)
            {
                bytesPerSec = (int) (byteCount / elapsedSec);
                byteCount = 0;
                juce::Logger::writeToLog (getString());
            }
        }

        juce::String getString() const
        {
            return juce::String (name) + ": "
                    + "count="    + juce::String (messageCount).paddedRight (' ', 7)
                    + "rate="     + (juce::String (bytesPerSec / 1024.0f, 1) + " Kb/sec").paddedRight (' ', 11)
                    + "largest="  + (juce::String (largestMessageBytes) + " bytes").paddedRight (' ', 11)
                    + "last="     + (juce::String (lastMessageBytes) + " bytes").paddedRight (' ', 11);
        }

        void registerMessage (int numBytes) noexcept
        {
            byteCount += numBytes;
            ++messageCount;
            lastMessageBytes = numBytes;
            largestMessageBytes = juce::jmax (largestMessageBytes, numBytes);
        }
    };

    static PortIOStats inputStats { "Input" }, outputStats { "Output" };
    static uint32 startTime = 0;

    static inline void resetOnSecondBoundary()
    {
        auto now = juce::Time::getMillisecondCounter();
        double elapsedSec = (now - startTime) / 1000.0;

        if (elapsedSec >= 1.0)
        {
            inputStats.update (elapsedSec);
            outputStats.update (elapsedSec);
            startTime = now;
        }
    }

    static inline void registerBytesOut (int numBytes)
    {
        outputStats.registerMessage (numBytes);
        resetOnSecondBoundary();
    }

    static inline void registerBytesIn (int numBytes)
    {
        inputStats.registerMessage (numBytes);
        resetOnSecondBoundary();
    }
}

juce::String getMidiIOStats()
{
    return inputStats.getString() + "   " + outputStats.getString();
}
#endif


//==============================================================================
struct PhysicalTopologySource::Internal
{
    struct Detector;
    struct BlockImplementation;
    struct ControlButtonImplementation;
    struct RotaryDialImplementation;
    struct TouchSurfaceImplementation;
    struct LEDGridImplementation;
    struct LEDRowImplementation;

    //==============================================================================
    struct MIDIDeviceConnection  : public DeviceConnection,
                                   public juce::MidiInputCallback
    {
        MIDIDeviceConnection() {}

        ~MIDIDeviceConnection()
        {
            JUCE_ASSERT_MESSAGE_MANAGER_IS_LOCKED

            listeners.call (&Listener::connectionBeingDeleted, *this);

            midiInput->stop();
        }

        struct Listener
        {
            virtual ~Listener() {}

            virtual void handleIncomingMidiMessage (const juce::MidiMessage& message) = 0;
            virtual void connectionBeingDeleted (const MIDIDeviceConnection&) = 0;
        };

        void addListener (Listener* l)
        {
            listeners.add (l);
        }

        void removeListener (Listener* l)
        {
            listeners.remove (l);
        }

        bool sendMessageToDevice (const void* data, size_t dataSize) override
        {
            JUCE_ASSERT_MESSAGE_MANAGER_IS_LOCKED // This method must only be called from the message thread!

            jassert (dataSize > sizeof (BlocksProtocol::roliSysexHeader) + 2);
            jassert (memcmp (data, BlocksProtocol::roliSysexHeader, sizeof (BlocksProtocol::roliSysexHeader)) == 0);
            jassert (static_cast<const uint8*> (data)[dataSize - 1] == 0xf7);

            if (midiOutput != nullptr)
            {
                midiOutput->sendMessageNow (juce::MidiMessage (data, (int) dataSize));
                return true;
            }

            return false;
        }

        void handleIncomingMidiMessage (juce::MidiInput*, const juce::MidiMessage& message) override
        {
            const auto data = message.getRawData();
            const int dataSize = message.getRawDataSize();
            const int bodySize = dataSize - (int) (sizeof (BlocksProtocol::roliSysexHeader) + 1);

            if (bodySize > 0 && memcmp (data, BlocksProtocol::roliSysexHeader, sizeof (BlocksProtocol::roliSysexHeader)) == 0)
                if (handleMessageFromDevice != nullptr)
                    handleMessageFromDevice (data + sizeof (BlocksProtocol::roliSysexHeader), (size_t) bodySize);

            listeners.call (&Listener::handleIncomingMidiMessage, message);
        }

        std::unique_ptr<juce::MidiInput> midiInput;
        std::unique_ptr<juce::MidiOutput> midiOutput;

    private:
        juce::ListenerList<Listener> listeners;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MIDIDeviceConnection)
    };

    struct MIDIDeviceDetector  : public DeviceDetector
    {
        MIDIDeviceDetector() {}

        juce::StringArray scanForDevices() override
        {
            juce::StringArray result;

            for (auto& pair : findDevices())
                result.add (pair.inputName + " & " + pair.outputName);

            return result;
        }

        DeviceConnection* openDevice (int index) override
        {
            auto pair = findDevices()[index];

            if (pair.inputIndex >= 0 && pair.outputIndex >= 0)
            {
                std::unique_ptr<MIDIDeviceConnection> dev (new MIDIDeviceConnection());

                dev->midiInput.reset (juce::MidiInput::openDevice (pair.inputIndex, dev.get()));
                dev->midiOutput.reset (juce::MidiOutput::openDevice (pair.outputIndex));

                if (dev->midiInput != nullptr)
                {
                    dev->midiInput->start();
                    return dev.release();
                }
            }

            return nullptr;
        }

        static bool isBlocksMidiDeviceName (const juce::String& name)
        {
            return name.indexOf (" BLOCK") > 0 || name.indexOf (" Block") > 0;
        }

        struct MidiInputOutputPair
        {
            juce::String outputName, inputName;
            int outputIndex = -1, inputIndex = -1;
        };

        static juce::Array<MidiInputOutputPair> findDevices()
        {
            juce::Array<MidiInputOutputPair> result;

            auto midiInputs  = juce::MidiInput::getDevices();
            auto midiOutputs = juce::MidiOutput::getDevices();

            for (int j = 0; j < midiInputs.size(); ++j)
            {
                if (isBlocksMidiDeviceName (midiInputs[j]))
                {
                    MidiInputOutputPair pair;
                    pair.inputName = midiInputs[j];
                    pair.inputIndex = j;

                    for (int i = 0; i < midiOutputs.size(); ++i)
                    {
                        if (midiOutputs[i].trim() == pair.inputName.trim())
                        {
                            pair.outputName = midiOutputs[i];
                            pair.outputIndex = i;
                            break;
                        }
                    }

                    result.add (pair);
                }
            }

            return result;
        }

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MIDIDeviceDetector)
    };

    //==============================================================================
    struct DeviceInfo
    {
        Block::UID uid;
        BlocksProtocol::TopologyIndex index;
        BlocksProtocol::BlockSerialNumber serial;
        bool isMaster;
    };

    static Block::Timestamp deviceTimestampToHost (uint32 timestamp) noexcept
    {
        return static_cast<Block::Timestamp> (timestamp);
    }

    static juce::Array<DeviceInfo> getArrayOfDeviceInfo (const juce::Array<BlocksProtocol::DeviceStatus>& devices)
    {
        juce::Array<DeviceInfo> result;
        bool isFirst = true;

        for (auto& device : devices)
        {
            result.add ({ getBlockUIDFromSerialNumber (device.serialNumber),
                          device.index,
                          device.serialNumber,
                          isFirst });

            isFirst = false;
        }

        return result;
    }

    static bool containsBlockWithUID (const juce::Array<DeviceInfo>& devices, Block::UID uid) noexcept
    {
        for (auto&& d : devices)
            if (d.uid == uid)
                return true;

        return false;
    }

    static bool containsBlockWithUID (const Block::Array& blocks, Block::UID uid) noexcept
    {
        for (auto&& block : blocks)
            if (block->uid == uid)
                return true;

        return false;
    }

    //==============================================================================
    struct ConnectedDeviceGroup  : private juce::AsyncUpdater,
                                   private juce::Timer
    {
        ConnectedDeviceGroup (Detector& d, const juce::String& name, DeviceConnection* connection)
            : detector (d), deviceName (name), deviceConnection (connection)
        {
            lastGlobalPingTime = juce::Time::getCurrentTime();

            deviceConnection->handleMessageFromDevice = [this] (const void* data, size_t dataSize)
            {
                this->handleIncomingMessage (data, dataSize);
            };

            startTimer (200);
            sendTopologyRequest();
        }

        bool isStillConnected (const juce::StringArray& detectedDevices) const noexcept
        {
            return detectedDevices.contains (deviceName)
                    && ! failedToGetTopology()
                    && lastGlobalPingTime > juce::Time::getCurrentTime() - juce::RelativeTime::seconds (pingTimeoutSeconds);
        }

        Block::UID getDeviceIDFromIndex (BlocksProtocol::TopologyIndex index) const noexcept
        {
            for (auto& d : currentDeviceInfo)
                if (d.index == index)
                    return d.uid;

            return {};
        }

        int getIndexFromDeviceID (Block::UID uid) const noexcept
        {
            for (auto& d : currentDeviceInfo)
                if (d.uid == uid)
                    return d.index;

            return -1;
        }

        DeviceInfo* getDeviceInfoFromUID (Block::UID uid) const noexcept
        {
            for (auto& d : currentDeviceInfo)
                if (d.uid == uid)
                    return &d;

            return nullptr;
        }

        const BlocksProtocol::DeviceStatus* getLastStatus (Block::UID deviceID) const noexcept
        {
            for (auto&& status : currentTopologyDevices)
                if (getBlockUIDFromSerialNumber (status.serialNumber) == deviceID)
                    return &status;

            return nullptr;
        }

        //==============================================================================
        juce::Time lastTopologyRequestTime, lastTopologyReceiveTime;
        int numTopologyRequestsSent = 0;

        void sendTopologyRequest()
        {
            ++numTopologyRequestsSent;
            lastTopologyRequestTime = juce::Time::getCurrentTime();
            sendCommandMessage (0, BlocksProtocol::requestTopologyMessage);
        }

        void scheduleNewTopologyRequest()
        {
            numTopologyRequestsSent = 0;
            lastTopologyReceiveTime = juce::Time();
        }

        bool failedToGetTopology() const noexcept
        {
            return numTopologyRequestsSent > 4 && lastTopologyReceiveTime == juce::Time();
        }

        bool hasAnyBlockStoppedPinging() const noexcept
        {
            auto now = juce::Time::getCurrentTime();

            for (auto& ping : blockPings)
                if (ping.lastPing < now - juce::RelativeTime::seconds (pingTimeoutSeconds))
                    return true;

            return false;
        }

        void timerCallback() override
        {
            auto now = juce::Time::getCurrentTime();

            if ((now > lastTopologyReceiveTime + juce::RelativeTime::seconds (30.0) || hasAnyBlockStoppedPinging())
                  && now > lastTopologyRequestTime + juce::RelativeTime::seconds (1.0)
                  && numTopologyRequestsSent < 4)
                sendTopologyRequest();
        }

        //==============================================================================
        // The following methods will be called by the DeviceToHostPacketDecoder:

        void beginTopology (int numDevices, int numConnections)
        {
            incomingTopologyDevices.clearQuick();
            incomingTopologyDevices.ensureStorageAllocated (numDevices);
            incomingTopologyConnections.clearQuick();
            incomingTopologyConnections.ensureStorageAllocated (numConnections);
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
            currentDeviceInfo = getArrayOfDeviceInfo (incomingTopologyDevices);
            currentDeviceConnections = getArrayOfConnections (incomingTopologyConnections);
            currentTopologyDevices = incomingTopologyDevices;
            currentTopologyConnections = incomingTopologyConnections;
            detector.handleTopologyChange();

            lastTopologyReceiveTime = juce::Time::getCurrentTime();
            blockPings.clear();
        }

        void handleControlButtonUpDown (BlocksProtocol::TopologyIndex deviceIndex, uint32 timestamp,
                                        BlocksProtocol::ControlButtonID buttonID, bool isDown)
        {
            if (auto deviceID = getDeviceIDFromMessageIndex (deviceIndex))
                detector.handleButtonChange (deviceID, deviceTimestampToHost (timestamp), buttonID.get(), isDown);
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

        void handlePacketACK (BlocksProtocol::TopologyIndex deviceIndex, BlocksProtocol::PacketCounter counter)
        {
            if (auto deviceID = getDeviceIDFromMessageIndex (deviceIndex))
                detector.handleSharedDataACK (deviceID, counter);
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

        bool sendCommandMessage (BlocksProtocol::TopologyIndex deviceIndex, uint32 commandID) const
        {
            BlocksProtocol::HostPacketBuilder<64> p;
            p.writePacketSysexHeaderBytes (deviceIndex);
            p.deviceControlMessage (commandID);
            p.writePacketSysexFooter();
            return sendMessageToDevice (p);
        }

        bool broadcastCommandMessage (uint32 commandID) const
        {
            return sendCommandMessage (BlocksProtocol::topologyIndexForBroadcast, commandID);
        }

        DeviceConnection* getDeviceConnection()
        {
            return deviceConnection.get();
        }

        Detector& detector;
        juce::String deviceName;

        juce::Array<DeviceInfo> currentDeviceInfo;
        juce::Array<BlockDeviceConnection> currentDeviceConnections;

        static constexpr double pingTimeoutSeconds = 6.0;

    private:
        //==============================================================================
        std::unique_ptr<DeviceConnection> deviceConnection;

        juce::Array<BlocksProtocol::DeviceStatus> incomingTopologyDevices, currentTopologyDevices;
        juce::Array<BlocksProtocol::DeviceConnection> incomingTopologyConnections, currentTopologyConnections;

        juce::CriticalSection incomingPacketLock;
        juce::Array<juce::MemoryBlock> incomingPackets;

        struct TouchStart
        {
            float x, y;
        };

        TouchList<TouchStart> touchStartPositions;

        juce::Time lastGlobalPingTime;

        struct BlockPingTime
        {
            Block::UID blockUID;
            juce::Time lastPing;
        };

        juce::Array<BlockPingTime> blockPings;

        Block::UID getDeviceIDFromMessageIndex (BlocksProtocol::TopologyIndex index) noexcept
        {
            auto uid = getDeviceIDFromIndex (index);

            if (uid == Block::UID())
            {
                scheduleNewTopologyRequest(); // force a re-request of the topology when we
                                              // get an event from a block that we don't know about
            }
            else
            {
                auto now = juce::Time::getCurrentTime();

                for (auto& ping : blockPings)
                {
                    if (ping.blockUID == uid)
                    {
                        ping.lastPing = now;
                        return uid;
                    }
                }

                blockPings.add ({ uid, now });
            }

            return uid;
        }

        juce::Array<BlockDeviceConnection> getArrayOfConnections (const juce::Array<BlocksProtocol::DeviceConnection>& connections)
        {
            juce::Array<BlockDeviceConnection> result;

            for (auto&& c : connections)
            {
                BlockDeviceConnection dc;
                dc.device1 = getDeviceIDFromIndex (c.device1);
                dc.device2 = getDeviceIDFromIndex (c.device2);
                dc.connectionPortOnDevice1 = convertConnectionPort (dc.device1, c.port1);
                dc.connectionPortOnDevice2 = convertConnectionPort (dc.device2, c.port2);

                result.add (dc);
            }

            return result;
        }

        Block::ConnectionPort convertConnectionPort (Block::UID uid, BlocksProtocol::ConnectorPort p) noexcept
        {
            if (auto* info = getDeviceInfoFromUID (uid))
                return BlocksProtocol::BlockDataSheet (info->serial).convertPortIndexToConnectorPort (p);

            jassertfalse;
            return { Block::ConnectionPort::DeviceEdge::north, 0 };
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
                lastGlobalPingTime = juce::Time::getCurrentTime();
                auto data = static_cast<const uint8*> (packet.getData());

                BlocksProtocol::HostPacketDecoder<ConnectedDeviceGroup>
                    ::processNextPacket (*this, *data, data + 1, (int) packet.getSize() - 1);
            }
        }

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ConnectedDeviceGroup)
    };

    //==============================================================================
    /** This is the main singleton object that keeps track of connected blocks */
    struct Detector   : public juce::ReferenceCountedObject,
                        private juce::Timer
    {
        Detector()  : defaultDetector (new MIDIDeviceDetector()), deviceDetector (*defaultDetector)
        {
            startTimer (10);
        }

        Detector (DeviceDetector& dd)  : deviceDetector (dd)
        {
            startTimer (10);
        }

        ~Detector()
        {
            jassert (activeTopologySources.isEmpty());
            jassert (activeControlButtons.isEmpty());
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
                    if (auto bi = BlockImplementation::getFrom (*b))
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
                    newDeviceInfo.addArray (d->currentDeviceInfo);
                    newDeviceConnections.addArray (d->currentDeviceConnections);
                }

                for (int i = currentTopology.blocks.size(); --i >= 0;)
                {
                    auto block = currentTopology.blocks.getUnchecked(i);

                    if (! containsBlockWithUID (newDeviceInfo, block->uid))
                    {
                        if (auto bi = BlockImplementation::getFrom (*block))
                            bi->invalidate();

                        currentTopology.blocks.remove (i);
                    }
                }

                for (auto& info : newDeviceInfo)
                    if (info.serial.isValid())
                        if (! containsBlockWithUID (currentTopology.blocks, getBlockUIDFromSerialNumber (info.serial)))
                            currentTopology.blocks.add (new BlockImplementation (info.serial, *this, info.isMaster));

                currentTopology.connections.swapWith (newDeviceConnections);
            }

            for (auto d : activeTopologySources)
                d->listeners.call (&TopologySource::Listener::topologyChanged);

           #if DUMP_TOPOLOGY
            dumpTopology (currentTopology);
           #endif
        }

        void handleSharedDataACK (Block::UID deviceID, uint32 packetCounter) const
        {
            JUCE_ASSERT_MESSAGE_MANAGER_IS_LOCKED

            for (auto&& b : currentTopology.blocks)
                if (b->uid == deviceID)
                    if (auto bi = BlockImplementation::getFrom (*b))
                        bi->handleSharedDataACK (packetCounter);
        }

        void handleButtonChange (Block::UID deviceID, Block::Timestamp timestamp, uint32 buttonIndex, bool isDown) const
        {
            JUCE_ASSERT_MESSAGE_MANAGER_IS_LOCKED

            for (auto b : activeControlButtons)
            {
                if (b->block.uid == deviceID)
                {
                    if (auto bi = BlockImplementation::getFrom (b->block))
                    {
                        bi->pingFromDevice();

                        if (buttonIndex < (uint32) bi->modelData.buttons.size())
                            b->broadcastButtonChange (timestamp, bi->modelData.buttons[(int) buttonIndex].type, isDown);
                    }
                }
            }
        }

        void handleTouchChange (Block::UID deviceID, const TouchSurface::Touch& touchEvent)
        {
            JUCE_ASSERT_MESSAGE_MANAGER_IS_LOCKED

            for (auto t : activeTouchSurfaces)
            {
                if (t->block.uid == deviceID)
                {
                    TouchSurface::Touch scaledEvent (touchEvent);

                    scaledEvent.x      *= t->block.getWidth();
                    scaledEvent.y      *= t->block.getHeight();
                    scaledEvent.startX *= t->block.getWidth();
                    scaledEvent.startY *= t->block.getHeight();

                    t->broadcastTouchChange (scaledEvent);
                }
            }
        }

        void cancelAllActiveTouches() noexcept
        {
            for (auto surface : activeTouchSurfaces)
                surface->cancelAllActiveTouches();
        }

        //==============================================================================
        int getIndexFromDeviceID (Block::UID deviceID) const noexcept
        {
            for (auto c : connectedDeviceGroups)
            {
                const int index = c->getIndexFromDeviceID (deviceID);

                if (index >= 0)
                    return index;
            }

            return -1;
        }

        template <typename PacketBuilder>
        bool sendMessageToDevice (Block::UID deviceID, const PacketBuilder& builder) const
        {
            for (auto c : connectedDeviceGroups)
                if (c->getIndexFromDeviceID (deviceID) >= 0)
                    return c->sendMessageToDevice (builder);

            return false;
        }

        static Detector* getFrom (Block& b) noexcept
        {
            if (auto bi = BlockImplementation::getFrom (b))
                return &(bi->detector);

            jassertfalse;
            return nullptr;
        }

        DeviceConnection* getDeviceConnectionFor (const Block& b)
        {
            for (const auto& d : connectedDeviceGroups)
            {
                for (const auto& info : d->currentDeviceInfo)
                {
                    if (info.uid == b.uid)
                        return d->getDeviceConnection();
                }
            }

            return nullptr;
        }

        std::unique_ptr<MIDIDeviceDetector> defaultDetector;
        DeviceDetector& deviceDetector;

        juce::Array<PhysicalTopologySource*> activeTopologySources;
        juce::Array<ControlButtonImplementation*> activeControlButtons;
        juce::Array<TouchSurfaceImplementation*> activeTouchSurfaces;

        BlockTopology currentTopology;

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
            bool anyDevicesAdded = false;

            for (const auto& devName : detectedDevices)
            {
                if (! hasDeviceFor (devName))
                {
                    if (auto d = deviceDetector.openDevice (detectedDevices.indexOf (devName)))
                    {
                        connectedDeviceGroups.add (new ConnectedDeviceGroup (*this, devName, d));
                        anyDevicesAdded = true;
                    }
                }
            }

            if (anyDevicesAdded)
                handleTopologyChange();
        }

        bool hasDeviceFor (const juce::String& devName) const
        {
            for (auto d : connectedDeviceGroups)
                if (d->deviceName == devName)
                    return true;

            return false;
        }

        juce::OwnedArray<ConnectedDeviceGroup> connectedDeviceGroups;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Detector)
    };

    //==============================================================================
    struct BlockImplementation  : public Block,
                                  private MIDIDeviceConnection::Listener
    {
        BlockImplementation (const BlocksProtocol::BlockSerialNumber& serial, Detector& detectorToUse, bool master)
            : Block (juce::String ((const char*) serial.serial, sizeof (serial.serial))), modelData (serial),
              remoteHeap (modelData.programAndHeapSize), detector (detectorToUse), isMaster (master)
        {
            sendCommandMessage (BlocksProtocol::beginAPIMode);

            if (modelData.hasTouchSurface)
                touchSurface.reset (new TouchSurfaceImplementation (*this));

            int i = 0;
            for (auto b : modelData.buttons)
                controlButtons.add (new ControlButtonImplementation (*this, i++, b));

            if (modelData.lightGridWidth > 0 && modelData.lightGridHeight > 0)
                ledGrid.reset (new LEDGridImplementation (*this));

            for (auto s : modelData.statusLEDs)
                statusLights.add (new StatusLightImplementation (*this, s));

            if (modelData.numLEDRowLEDs > 0)
                ledRow.reset (new LEDRowImplementation (*this));

            listenerToMidiConnection = dynamic_cast<MIDIDeviceConnection*> (detector.getDeviceConnectionFor (*this));
            if (listenerToMidiConnection != nullptr)
                listenerToMidiConnection->addListener (this);
        }

        ~BlockImplementation()
        {
            if (listenerToMidiConnection != nullptr)
                listenerToMidiConnection->removeListener (this);
        }

        void invalidate()
        {
            isStillConnected = false;
        }

        Type getType() const override                                   { return modelData.apiType; }
        juce::String getDeviceDescription() const override              { return modelData.description; }
        int getWidth() const override                                   { return modelData.widthUnits; }
        int getHeight() const override                                  { return modelData.heightUnits; }
        float getMillimetersPerUnit() const override                    { return 47.0f; }
        bool isHardwareBlock() const override                           { return true; }
        juce::Array<Block::ConnectionPort> getPorts() const override    { return modelData.ports; }
        bool isConnected() const override                               { return isStillConnected && detector.isConnected (uid); }
        bool isMasterBlock() const override                             { return isMaster; }

        TouchSurface* getTouchSurface() const override                  { return touchSurface.get(); }
        LEDGrid* getLEDGrid() const override                            { return ledGrid.get(); }
        LEDRow* getLEDRow() const override                              { return ledRow.get(); }

        juce::Array<ControlButton*> getButtons() const override
        {
            juce::Array<ControlButton*> result;
            result.addArray (controlButtons);
            return result;
        }

        juce::Array<StatusLight*> getStatusLights() const override
        {
            juce::Array<StatusLight*> result;
            result.addArray (statusLights);
            return result;
        }

        float getBatteryLevel() const override
        {
            if (auto status = detector.getLastStatus (uid))
                return status->batteryLevel.toUnipolarFloat();

            return 0.0f;
        }

        bool isBatteryCharging() const override
        {
            if (auto status = detector.getLastStatus (uid))
                return status->batteryCharging.get() != 0;

            return false;
        }

        bool supportsGraphics() const override
        {
            return false;
        }

        int getDeviceIndex() const noexcept
        {
            return isConnected() ? detector.getIndexFromDeviceID (uid) : -1;
        }

        template <typename PacketBuilder>
        bool sendMessageToDevice (const PacketBuilder& builder)
        {
            lastMessageSendTime = juce::Time::getCurrentTime();
            return detector.sendMessageToDevice (uid, builder);
        }

        bool sendCommandMessage (uint32 commandID)
        {
            int index = getDeviceIndex();

            if (index < 0)
                return false;

            BlocksProtocol::HostPacketBuilder<64> p;
            p.writePacketSysexHeaderBytes ((BlocksProtocol::TopologyIndex) index);
            p.deviceControlMessage (commandID);
            p.writePacketSysexFooter();

            return sendMessageToDevice (p);
        }

        static BlockImplementation* getFrom (Block& b) noexcept
        {
            if (auto bi = dynamic_cast<BlockImplementation*> (&b))
                return bi;

            jassertfalse;
            return nullptr;
        }

        bool isControlBlock() const
        {
            auto type = getType();

            return type == Block::Type::liveBlock
                || type == Block::Type::loopBlock
                || type == Block::Type::developerControlBlock;
        }

        //==============================================================================
        void clearProgramAndData()
        {
            programSize = 0;
            remoteHeap.clear();
        }

        void setProgram (const void* compiledCode, size_t codeSize)
        {
            clearProgramAndData();
            setDataBytes (0, compiledCode, codeSize);
            programSize = (uint32) codeSize;
        }

        void setDataByte (size_t offset, uint8 value)
        {
            remoteHeap.setByte (programSize + offset, value);
        }

        void setDataBytes (size_t offset, const void* newData, size_t num)
        {
            remoteHeap.setBytes (programSize + offset, static_cast<const uint8*> (newData), num);
        }

        void setDataBits (uint32 startBit, uint32 numBits, uint32 value)
        {
            remoteHeap.setBits (programSize * 8 + startBit, numBits, value);
        }

        uint8 getDataByte (size_t offset)
        {
            return remoteHeap.getByte (programSize + offset);
        }

        void sendProgramEvent (const LEDGrid::ProgramEventMessage& message)
        {
            static_assert (sizeof (LEDGrid::ProgramEventMessage::values) == 4 * BlocksProtocol::numProgramMessageInts,
                           "Need to keep the internal and external messages structures the same");

            if (remoteHeap.isProgramLoaded())
            {
                auto index = getDeviceIndex();

                if (index >= 0)
                {
                    BlocksProtocol::HostPacketBuilder<128> p;
                    p.writePacketSysexHeaderBytes ((BlocksProtocol::TopologyIndex) index);

                    if (p.addProgramEventMessage (message.values))
                    {
                        p.writePacketSysexFooter();
                        sendMessageToDevice (p);
                    }
                }
                else
                {
                    jassertfalse;
                }
            }
        }

        void handleSharedDataACK (uint32 packetCounter) noexcept
        {
            pingFromDevice();
            remoteHeap.handleACKFromDevice (*this, packetCounter);
        }

        void pingFromDevice()
        {
            lastMessageReceiveTime = juce::Time::getCurrentTime();
        }

        void addDataInputPortListener (DataInputPortListener* listener) override
        {
            Block::addDataInputPortListener (listener);

            if (auto midiInput = getMidiInput())
                midiInput->start();
        }

        void sendMessage (const void* message, size_t messageSize) override
        {
            if (auto midiOutput = getMidiOutput())
                midiOutput->sendMessageNow ({ message, (int) messageSize });
        }

        void handleTimerTick()
        {
            if (++resetMessagesSent < 3)
            {
                if (resetMessagesSent == 1)
                    sendCommandMessage (BlocksProtocol::endAPIMode);

                sendCommandMessage (BlocksProtocol::beginAPIMode);
                return;
            }

            if (ledGrid != nullptr)
                if (auto renderer = ledGrid->getRenderer())
                    renderer->renderLEDGrid (*ledGrid);

            remoteHeap.sendChanges (*this);

            if (lastMessageSendTime < juce::Time::getCurrentTime() - juce::RelativeTime::milliseconds (pingIntervalMs))
                sendCommandMessage (BlocksProtocol::ping);
        }

        //==============================================================================
        std::unique_ptr<TouchSurface> touchSurface;
        juce::OwnedArray<ControlButton> controlButtons;
        std::unique_ptr<LEDGridImplementation> ledGrid;
        std::unique_ptr<LEDRowImplementation> ledRow;
        juce::OwnedArray<StatusLight> statusLights;

        BlocksProtocol::BlockDataSheet modelData;

        MIDIDeviceConnection* listenerToMidiConnection = nullptr;

        static constexpr int pingIntervalMs = 400;

        static constexpr uint32 maxBlockSize = BlocksProtocol::padBlockProgramAndHeapSize;
        static constexpr uint32 maxPacketCounter = BlocksProtocol::PacketCounter::maxValue;
        static constexpr uint32 maxPacketSize = 200;

        using PacketBuilder = BlocksProtocol::HostPacketBuilder<maxPacketSize>;

        using RemoteHeapType = littlefoot::LittleFootRemoteHeap<BlockImplementation>;
        RemoteHeapType remoteHeap;

        uint32 programSize = 0;

        Detector& detector;
        juce::Time lastMessageSendTime, lastMessageReceiveTime;

    private:
        uint32 resetMessagesSent = 0;
        bool isStillConnected = true;
        bool isMaster = false;

        const juce::MidiInput* getMidiInput() const
        {
            if (auto c = dynamic_cast<MIDIDeviceConnection*> (detector.getDeviceConnectionFor (*this)))
                return c->midiInput.get();

            jassertfalse;
            return nullptr;
        }

        juce::MidiInput* getMidiInput()
        {
            return const_cast<juce::MidiInput*> (static_cast<const BlockImplementation&>(*this).getMidiInput());
        }

        const juce::MidiOutput* getMidiOutput() const
        {
            if (auto c = dynamic_cast<MIDIDeviceConnection*> (detector.getDeviceConnectionFor (*this)))
                return c->midiOutput.get();

            jassertfalse;
            return nullptr;
        }

        juce::MidiOutput* getMidiOutput()
        {
            return const_cast<juce::MidiOutput*> (static_cast<const BlockImplementation&>(*this).getMidiOutput());
        }

        void handleIncomingMidiMessage (const juce::MidiMessage& message) override
        {
            dataInputPortListeners.call (&Block::DataInputPortListener::handleIncomingDataPortMessage,
                                         *this, message.getRawData(), (size_t) message.getRawDataSize());
        }

        void connectionBeingDeleted (const MIDIDeviceConnection& c) override
        {
            jassert (listenerToMidiConnection == &c);
            juce::ignoreUnused (c);
            listenerToMidiConnection->removeListener (this);
            listenerToMidiConnection = nullptr;
        }

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BlockImplementation)
    };

    //==============================================================================
    struct LEDRowImplementation  : public LEDRow
    {
        LEDRowImplementation (BlockImplementation& b) : LEDRow (b), blockImpl (b)
        {
            loadProgramOntoBlock();
        }

        /*  Data format:

            0:  10 x 5-6-5 bits for button LED RGBs
            20: 15 x 5-6-5 bits for LED row colours
            50:  1 x 5-6-5 bits for LED row overlay colour
        */
        static constexpr uint32 totalDataSize = 256;

        //==============================================================================
        void setButtonColour (uint32 index, LEDColour colour)
        {
            if (index < 10)
                write565Colour (16 * index, colour);
        }

        int getNumLEDs() const override
        {
            return blockImpl.modelData.numLEDRowLEDs;
        }

        void setLEDColour (int index, LEDColour colour) override
        {
            if ((uint32) index < 15u)
                write565Colour (20 * 8 + 16 * (uint32) index, colour);
        }

        void setOverlayColour (LEDColour colour) override
        {
            write565Colour (50 * 8, colour);
        }

        void resetOverlayColour() override
        {
            write565Colour (50 * 8, {});
        }

    private:
        void loadProgramOntoBlock()
        {
            littlefoot::Compiler compiler;
            compiler.addNativeFunctions (PhysicalTopologySource::getStandardLittleFootFunctions());

            auto err = compiler.compile (getLittleFootProgram(), totalDataSize);

            if (err.failed())
            {
                DBG (err.getErrorMessage());
                jassertfalse;
                return;
            }

            blockImpl.setProgram (compiler.compiledObjectCode.begin(), (size_t) compiler.compiledObjectCode.size());
        }

        void write565Colour (uint32 bitIndex, LEDColour colour)
        {
            blockImpl.setDataBits (bitIndex,      5, colour.getRed()   >> 3);
            blockImpl.setDataBits (bitIndex + 5,  6, colour.getGreen() >> 2);
            blockImpl.setDataBits (bitIndex + 11, 5, colour.getBlue()  >> 3);
        }

        static const char* getLittleFootProgram() noexcept
        {
            return R"littlefoot(

            int getColour (int bitIndex)
            {
                return makeARGB (255,
                                 getHeapBits (bitIndex,      5) << 3,
                                 getHeapBits (bitIndex + 5,  6) << 2,
                                 getHeapBits (bitIndex + 11, 5) << 3);
            }

            int getButtonColour (int index)
            {
                return getColour (16 * index);
            }

            int getLEDColour (int index)
            {
                if (getHeapInt (50))
                    return getColour (50 * 8);

                return getColour (20 * 8 + 16 * index);
            }

            void repaint()
            {
                for (int x = 0; x < 15; ++x)
                    setLED (x, 0, getLEDColour (x));

                for (int i = 0; i < 10; ++i)
                    setLED (i, 1, getButtonColour (i));
            }

            void handleMessage (int p1, int p2) {}

            )littlefoot";
        }

        BlockImplementation& blockImpl;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LEDRowImplementation)
    };

    //==============================================================================
    struct TouchSurfaceImplementation  : public TouchSurface,
                                         private juce::Timer
    {
        TouchSurfaceImplementation (BlockImplementation& b)  : TouchSurface (b), blockImpl (b)
        {
            if (auto det = Detector::getFrom (block))
                det->activeTouchSurfaces.add (this);

            startTimer (500);
        }

        ~TouchSurfaceImplementation()
        {
            if (auto det = Detector::getFrom (block))
                det->activeTouchSurfaces.removeFirstMatchingValue (this);
        }

        void broadcastTouchChange (const TouchSurface::Touch& touchEvent)
        {
            auto& status = touches.getValue (touchEvent);

            // Fake a touch end if we receive a duplicate touch-start with no preceding touch-end (ie: comms error)
            if (touchEvent.isTouchStart && status.isActive)
                killTouch (touchEvent, status, juce::Time::getMillisecondCounter());

            // Fake a touch start if we receive an unexpected event with no matching start event. (ie: comms error)
            if (! touchEvent.isTouchStart && ! status.isActive)
            {
                TouchSurface::Touch t (touchEvent);
                t.isTouchStart = true;
                t.isTouchEnd = false;

                if (t.zVelocity <= 0)  t.zVelocity = status.lastStrikePressure;
                if (t.zVelocity <= 0)  t.zVelocity = t.z;
                if (t.zVelocity <= 0)  t.zVelocity = 0.9f;

                listeners.call (&TouchSurface::Listener::touchChanged, *this, t);
            }

            // Normal handling:
            status.lastEventTime = juce::Time::getMillisecondCounter();
            status.isActive = ! touchEvent.isTouchEnd;

            if (touchEvent.isTouchStart)
                status.lastStrikePressure = touchEvent.zVelocity;

            listeners.call (&TouchSurface::Listener::touchChanged, *this, touchEvent);
        }

        void timerCallback() override
        {
            // Find touches that seem to have become stuck, and fake a touch-end for them..
            static const uint32 touchTimeOutMs = 40;

            for (auto& t : touches)
            {
                auto& status = t.value;
                auto now = juce::Time::getMillisecondCounter();

                if (status.isActive && now > status.lastEventTime + touchTimeOutMs)
                    killTouch (t.touch, status, now);
            }
        }

        struct TouchStatus
        {
            uint32 lastEventTime = 0;
            float lastStrikePressure = 0;
            bool isActive = false;
        };

        void killTouch (const TouchSurface::Touch& touch, TouchStatus& status, uint32 timeStamp) noexcept
        {
            jassert (status.isActive);

            TouchSurface::Touch killTouch (touch);

            killTouch.z                 = 0;
            killTouch.xVelocity         = 0;
            killTouch.yVelocity         = 0;
            killTouch.zVelocity         = -1.0f;
            killTouch.eventTimestamp    = timeStamp;
            killTouch.isTouchStart      = false;
            killTouch.isTouchEnd        = true;

            listeners.call (&TouchSurface::Listener::touchChanged, *this, killTouch);

            status.isActive = false;
        }

        void cancelAllActiveTouches() noexcept override
        {
            const auto now = juce::Time::getMillisecondCounter();

            for (auto& t : touches)
                if (t.value.isActive)
                    killTouch (t.touch, t.value, now);

            touches.clear();
        }

        BlockImplementation& blockImpl;
        TouchList<TouchStatus> touches;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TouchSurfaceImplementation)
    };

    //==============================================================================
    struct ControlButtonImplementation  : public ControlButton
    {
        ControlButtonImplementation (BlockImplementation& b, int index, BlocksProtocol::BlockDataSheet::ButtonInfo info)
            : ControlButton (b), blockImpl (b), buttonInfo (info), buttonIndex (index)
        {
            if (auto det = Detector::getFrom (block))
                det->activeControlButtons.add (this);
        }

        ~ControlButtonImplementation()
        {
            if (auto det = Detector::getFrom (block))
                det->activeControlButtons.removeFirstMatchingValue (this);
        }

        ButtonFunction getType() const override         { return buttonInfo.type; }
        juce::String getName() const override           { return BlocksProtocol::getButtonNameForFunction (buttonInfo.type); }
        float getPositionX() const override             { return buttonInfo.x; }
        float getPositionY() const override             { return buttonInfo.y; }

        bool hasLight() const override                  { return blockImpl.isControlBlock(); }

        bool setLightColour (LEDColour colour) override
        {
            if (hasLight())
            {
                if (auto row = blockImpl.ledRow.get())
                {
                    row->setButtonColour ((uint32) buttonIndex, colour);
                    return true;
                }
            }

            return false;
        }

        void broadcastButtonChange (Block::Timestamp timestamp, ControlButton::ButtonFunction button, bool isDown)
        {
            if (button == buttonInfo.type)
            {
                if (wasDown == isDown)
                    sendButtonChangeToListeners (timestamp, ! isDown);

                sendButtonChangeToListeners (timestamp, isDown);
                wasDown = isDown;
            }
        }

        void sendButtonChangeToListeners (Block::Timestamp timestamp, bool isDown)
        {
            if (isDown)
                listeners.call (&ControlButton::Listener::buttonPressed, *this, timestamp);
            else
                listeners.call (&ControlButton::Listener::buttonReleased, *this, timestamp);
        }

        BlockImplementation& blockImpl;
        BlocksProtocol::BlockDataSheet::ButtonInfo buttonInfo;
        int buttonIndex;
        bool wasDown = false;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ControlButtonImplementation)
    };


    //==============================================================================
    struct StatusLightImplementation  : public StatusLight
    {
        StatusLightImplementation (Block& b, BlocksProtocol::BlockDataSheet::StatusLEDInfo i)  : StatusLight (b), info (i)
        {
        }

        juce::String getName() const override               { return info.name; }

        bool setColour (LEDColour newColour) override
        {
            // XXX TODO!
            juce::ignoreUnused (newColour);
            return false;
        }

        BlocksProtocol::BlockDataSheet::StatusLEDInfo info;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StatusLightImplementation)
    };

    //==============================================================================
    struct LEDGridImplementation  : public LEDGrid
    {
        LEDGridImplementation (BlockImplementation& b)  : LEDGrid (b), blockImpl (b)
        {
        }

        int getNumColumns() const override      { return blockImpl.modelData.lightGridWidth; }
        int getNumRows() const override         { return blockImpl.modelData.lightGridHeight; }

        juce::Result setProgram (Program* newProgram) override
        {
            if (program.get() != newProgram)
            {
                program.reset (newProgram);

                if (program != nullptr)
                {
                    littlefoot::Compiler compiler;
                    compiler.addNativeFunctions (PhysicalTopologySource::getStandardLittleFootFunctions());

                    auto err = compiler.compile (newProgram->getLittleFootProgram(), newProgram->getHeapSize());

                    if (err.failed())
                        return err;

                    DBG ("Compiled littlefoot program, size = " << (int) compiler.compiledObjectCode.size() << " bytes");

                    blockImpl.setProgram (compiler.compiledObjectCode.begin(), (size_t) compiler.compiledObjectCode.size());
                }
                else
                {
                    blockImpl.clearProgramAndData();
                }
            }
            else
            {
                jassertfalse;
            }

            return juce::Result::ok();
        }

        Program* getProgram() const override                                        { return program.get(); }

        void sendProgramEvent (const ProgramEventMessage& m) override               { blockImpl.sendProgramEvent (m); }
        void setDataByte (size_t offset, uint8 value) override                      { blockImpl.setDataByte (offset, value); }
        void setDataBytes (size_t offset, const void* data, size_t num) override    { blockImpl.setDataBytes (offset, data, num); }
        void setDataBits (uint32 startBit, uint32 numBits, uint32 value) override   { blockImpl.setDataBits (startBit, numBits, value); }
        uint8 getDataByte (size_t offset) override                                  { return blockImpl.getDataByte (offset); }

        BlockImplementation& blockImpl;
        std::unique_ptr<Program> program;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LEDGridImplementation)
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

            if (auto bi = BlockImplementation::getFrom (*block))
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
};

//==============================================================================
struct PhysicalTopologySource::DetectorHolder  : private juce::Timer
{
    DetectorHolder (PhysicalTopologySource& pts)
        : topologySource (pts),
          detector (Internal::Detector::getDefaultDetector())
    {
        startTimerHz (30);
    }

    DetectorHolder (PhysicalTopologySource& pts, DeviceDetector& dd)
        : topologySource (pts),
          detector (new Internal::Detector (dd))
    {
        startTimerHz (30);
    }

    void timerCallback() override
    {
        if (! topologySource.hasOwnServiceTimer())
            handleTimerTick();
    }

    void handleTimerTick()
    {
        for (auto& b : detector->currentTopology.blocks)
            if (auto bi = Internal::BlockImplementation::getFrom (*b))
                bi->handleTimerTick();
    }

    PhysicalTopologySource& topologySource;
    Internal::Detector::Ptr detector;
};

//==============================================================================
PhysicalTopologySource::PhysicalTopologySource()
    : detector (new DetectorHolder (*this))
{
    detector->detector->activeTopologySources.add (this);
}

PhysicalTopologySource::PhysicalTopologySource (DeviceDetector& detectorToUse)
    : detector (new DetectorHolder (*this, detectorToUse))
{
    detector->detector->activeTopologySources.add (this);
}

PhysicalTopologySource::~PhysicalTopologySource()
{
    detector->detector->detach (this);
    detector = nullptr;
}

BlockTopology PhysicalTopologySource::getCurrentTopology() const
{
    JUCE_ASSERT_MESSAGE_MANAGER_IS_LOCKED // This method must only be called from the message thread!

    return detector->detector->currentTopology;
}

void PhysicalTopologySource::cancelAllActiveTouches() noexcept
{
    detector->detector->cancelAllActiveTouches();
}

bool PhysicalTopologySource::hasOwnServiceTimer() const     { return false; }
void PhysicalTopologySource::handleTimerTick()              { detector->handleTimerTick(); }

PhysicalTopologySource::DeviceConnection::DeviceConnection() {}
PhysicalTopologySource::DeviceConnection::~DeviceConnection() {}

PhysicalTopologySource::DeviceDetector::DeviceDetector() {}
PhysicalTopologySource::DeviceDetector::~DeviceDetector() {}

const char* const* PhysicalTopologySource::getStandardLittleFootFunctions() noexcept
{
    return BlocksProtocol::ledProgramLittleFootFunctions;
}

static bool blocksMatch (const Block::Array& list1, const Block::Array& list2) noexcept
{
    if (list1.size() != list2.size())
        return false;

    for (auto&& b : list1)
        if (! list2.contains (b))
            return false;

    return true;
}

bool BlockTopology::operator== (const BlockTopology& other) const noexcept
{
    return connections == other.connections && blocksMatch (blocks, other.blocks);
}

bool BlockTopology::operator!= (const BlockTopology& other) const noexcept
{
    return ! operator== (other);
}

bool BlockDeviceConnection::operator== (const BlockDeviceConnection& other) const noexcept
{
    return (device1 == other.device1 && device2 == other.device2
             && connectionPortOnDevice1 == other.connectionPortOnDevice1
             && connectionPortOnDevice2 == other.connectionPortOnDevice2)
        || (device1 == other.device2 && device2 == other.device1
             && connectionPortOnDevice1 == other.connectionPortOnDevice2
             && connectionPortOnDevice2 == other.connectionPortOnDevice1);
}

bool BlockDeviceConnection::operator!= (const BlockDeviceConnection& other) const noexcept
{
    return ! operator== (other);
}

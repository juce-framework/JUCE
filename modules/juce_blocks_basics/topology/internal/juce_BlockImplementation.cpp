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

template <typename Detector>
struct BlockImplementation  : public Block,
                              private MIDIDeviceConnection::Listener,
                              private Timer
{
public:
    struct ControlButtonImplementation;
    struct TouchSurfaceImsplementation;
    struct LEDGridImplementation;
    struct LEDRowImplementation;

    BlockImplementation (const BlocksProtocol::BlockSerialNumber& serial,
                         Detector& detectorToUse,
                         BlocksProtocol::VersionNumber version,
                         BlocksProtocol::BlockName blockName,
                         bool isMasterBlock)
        : Block (juce::String ((const char*) serial.serial,   sizeof (serial.serial)),
                 juce::String ((const char*) version.data, version.length),
                 juce::String ((const char*) blockName.data,  blockName.length)),
          modelData (serial),
          remoteHeap (modelData.programAndHeapSize),
          detector (&detectorToUse),
          isMaster (isMasterBlock)
    {
        if (modelData.hasTouchSurface)
            touchSurface.reset (new TouchSurfaceImplementation (*this));

        int i = 0;

        for (auto&& b : modelData.buttons)
            controlButtons.add (new ControlButtonImplementation (*this, i++, b));

        if (modelData.lightGridWidth > 0 && modelData.lightGridHeight > 0)
            ledGrid.reset (new LEDGridImplementation (*this));

        for (auto&& s : modelData.statusLEDs)
            statusLights.add (new StatusLightImplementation (*this, s));

        updateMidiConnectionListener();
    }

    ~BlockImplementation()
    {
        if (listenerToMidiConnection != nullptr)
        {
            config.setDeviceComms (nullptr);
            listenerToMidiConnection->removeListener (this);
        }
    }

    void markDisconnected()
    {
        if (auto surface = dynamic_cast<TouchSurfaceImplementation*> (touchSurface.get()))
            surface->disableTouchSurface();
    }

    void markReconnected (const DeviceInfo& deviceInfo)
    {
        versionNumber = asString (deviceInfo.version);
        name = asString (deviceInfo.name);
        isMaster = deviceInfo.isMaster;

        setProgram (nullptr);
        remoteHeap.resetDeviceStateToUnknown();

        if (auto surface = dynamic_cast<TouchSurfaceImplementation*> (touchSurface.get()))
            surface->activateTouchSurface();

        updateMidiConnectionListener();
    }

    void setToMaster (bool shouldBeMaster)
    {
        isMaster = shouldBeMaster;
    }

    void updateMidiConnectionListener()
    {
        if (detector == nullptr)
            return;

        listenerToMidiConnection = dynamic_cast<MIDIDeviceConnection*> (detector->getDeviceConnectionFor (*this));

        if (listenerToMidiConnection != nullptr)
            listenerToMidiConnection->addListener (this);

        config.setDeviceComms (listenerToMidiConnection);
    }

    Type getType() const override                                   { return modelData.apiType; }
    juce::String getDeviceDescription() const override              { return modelData.description; }
    int getWidth() const override                                   { return modelData.widthUnits; }
    int getHeight() const override                                  { return modelData.heightUnits; }
    float getMillimetersPerUnit() const override                    { return 47.0f; }
    bool isHardwareBlock() const override                           { return true; }
    juce::Array<Block::ConnectionPort> getPorts() const override    { return modelData.ports; }
    bool isConnected() const override                               { return detector && detector->isConnected (uid); }
    bool isMasterBlock() const override                             { return isMaster; }
    Block::UID getConnectedMasterUID() const override               { return masterUID; }
    int getRotation() const override                                { return rotation; }

    Rectangle<int> getBlockAreaWithinLayout() const override
    {
        if (rotation % 2 == 0)
            return { position.getX(), position.getY(), modelData.widthUnits, modelData.heightUnits };

        return { position.getX(), position.getY(), modelData.heightUnits, modelData.widthUnits };
    }

    TouchSurface* getTouchSurface() const override                  { return touchSurface.get(); }
    LEDGrid* getLEDGrid() const override                            { return ledGrid.get(); }

    LEDRow* getLEDRow() override
    {
        if (ledRow == nullptr && modelData.numLEDRowLEDs > 0)
            ledRow.reset (new LEDRowImplementation (*this));

        return ledRow.get();
    }

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
        if (detector == nullptr)
            return 0.0f;

        if (auto status = detector->getLastStatus (uid))
            return status->batteryLevel.toUnipolarFloat();

        return 0.0f;
    }

    bool isBatteryCharging() const override
    {
        if (detector == nullptr)
            return false;

        if (auto status = detector->getLastStatus (uid))
            return status->batteryCharging.get() != 0;

        return false;
    }

    bool supportsGraphics() const override
    {
        return false;
    }

    int getDeviceIndex() const noexcept
    {
        if (detector == nullptr)
            return -1;

        return isConnected() ? detector->getIndexFromDeviceID (uid) : -1;
    }

    template <typename PacketBuilder>
    bool sendMessageToDevice (const PacketBuilder& builder)
    {
        if (detector != nullptr)
        {
            lastMessageSendTime = juce::Time::getCurrentTime();
            return detector->sendMessageToDevice (uid, builder);
        }

        return false;
    }

    bool sendCommandMessage (uint32 commandID)
    {
        return buildAndSendPacket<64> ([commandID] (BlocksProtocol::HostPacketBuilder<64>& p)
                                       { return p.deviceControlMessage (commandID); });
    }

    void handleCustomMessage (Block::Timestamp, const int32* data)
    {
        ProgramEventMessage m;

        for (uint32 i = 0; i < BlocksProtocol::numProgramMessageInts; ++i)
            m.values[i] = data[i];

        programEventListeners.call ([&] (ProgramEventListener& l) { l.handleProgramEvent (*this, m); });
    }

    static BlockImplementation* getFrom (Block& b) noexcept
    {
        jassert (dynamic_cast<BlockImplementation*> (&b) != nullptr);
        return dynamic_cast<BlockImplementation*> (&b);
    }

    //==============================================================================
    std::function<void(const String&)> logger;

    void setLogger (std::function<void(const String&)> newLogger) override
    {
        logger = newLogger;
    }

    void handleLogMessage (const String& message) const
    {
        if (logger != nullptr)
            logger (message);
    }

    //==============================================================================
    juce::Result setProgram (Program* newProgram) override
    {
        if (newProgram != nullptr && program.get() == newProgram)
        {
            jassertfalse;
            return juce::Result::ok();
        }

        stopTimer();

        {
            std::unique_ptr<Program> p (newProgram);

            if (program != nullptr
                && newProgram != nullptr
                && program->getLittleFootProgram() == newProgram->getLittleFootProgram())
                return juce::Result::ok();

            std::swap (program, p);
        }

        programSize = 0;
        isProgramLoaded = shouldSaveProgramAsDefault = false;

        if (program == nullptr)
        {
            remoteHeap.clear();
            return juce::Result::ok();
        }

        littlefoot::Compiler compiler;
        compiler.addNativeFunctions (PhysicalTopologySource::getStandardLittleFootFunctions());

        const auto err = compiler.compile (program->getLittleFootProgram(), 512, program->getSearchPaths());

        if (err.failed())
            return err;

        DBG ("Compiled littlefoot program, space needed: "
             << (int) compiler.getCompiledProgram().getTotalSpaceNeeded() << " bytes");

        if (compiler.getCompiledProgram().getTotalSpaceNeeded() > getMemorySize())
            return Result::fail ("Program too large!");

        const auto size = (size_t) compiler.compiledObjectCode.size();
        programSize = (uint32) size;

        remoteHeap.resetDataRangeToUnknown (0, remoteHeap.blockSize);
        remoteHeap.clear();
        remoteHeap.sendChanges (*this, true);

        remoteHeap.resetDataRangeToUnknown (0, (uint32) size);
        remoteHeap.setBytes (0, compiler.compiledObjectCode.begin(), size);
        remoteHeap.sendChanges (*this, true);

        this->resetConfigListActiveStatus();

        if (auto changeCallback = this->configChangedCallback)
            changeCallback (*this, {}, this->getMaxConfigIndex());

        startTimer (20);

        return juce::Result::ok();
    }

    Program* getProgram() const override                                        { return program.get(); }

    void sendProgramEvent (const ProgramEventMessage& message) override
    {
        static_assert (sizeof (ProgramEventMessage::values) == 4 * BlocksProtocol::numProgramMessageInts,
                       "Need to keep the internal and external messages structures the same");

        if (remoteHeap.isProgramLoaded())
        {
            buildAndSendPacket<128> ([&message] (BlocksProtocol::HostPacketBuilder<128>& p)
                                     { return p.addProgramEventMessage (message.values); });
        }
    }

    void timerCallback() override
    {
        if (remoteHeap.isFullySynced() && remoteHeap.isProgramLoaded())
        {
            isProgramLoaded = true;
            stopTimer();

            if (shouldSaveProgramAsDefault)
                doSaveProgramAsDefault();

            if (programLoadedCallback != nullptr)
                programLoadedCallback (*this);
        }
        else
        {
            startTimer (100);
        }
    }

    void saveProgramAsDefault() override
    {
        shouldSaveProgramAsDefault = true;

        if (! isTimerRunning() && isProgramLoaded)
            doSaveProgramAsDefault();
    }

    uint32 getMemorySize() override
    {
        return modelData.programAndHeapSize;
    }

    uint32 getHeapMemorySize() override
    {
        jassert (isPositiveAndNotGreaterThan (programSize, modelData.programAndHeapSize));
        return modelData.programAndHeapSize - programSize;
    }

    void setDataByte (size_t offset, uint8 value) override
    {
        remoteHeap.setByte (programSize + offset, value);
    }

    void setDataBytes (size_t offset, const void* newData, size_t num) override
    {
        remoteHeap.setBytes (programSize + offset, static_cast<const uint8*> (newData), num);
    }

    void setDataBits (uint32 startBit, uint32 numBits, uint32 value) override
    {
        remoteHeap.setBits (programSize * 8 + startBit, numBits, value);
    }

    uint8 getDataByte (size_t offset) override
    {
        return remoteHeap.getByte (programSize + offset);
    }

    void handleSharedDataACK (uint32 packetCounter) noexcept
    {
        pingFromDevice();
        remoteHeap.handleACKFromDevice (*this, packetCounter);
    }

    bool sendFirmwareUpdatePacket (const uint8* data, uint8 size, std::function<void (uint8, uint32)> callback) override
    {
        firmwarePacketAckCallback = {};

        if (buildAndSendPacket<256> ([data, size] (BlocksProtocol::HostPacketBuilder<256>& p)
                                     { return p.addFirmwareUpdatePacket (data, size); }))
        {
            firmwarePacketAckCallback = callback;
            return true;
        }

        return false;
    }

    void handleFirmwareUpdateACK (uint8 resultCode, uint32 resultDetail)
    {
        if (firmwarePacketAckCallback != nullptr)
        {
            firmwarePacketAckCallback (resultCode, resultDetail);
            firmwarePacketAckCallback = {};
        }
    }

    void handleConfigUpdateMessage (int32 item, int32 value, int32 min, int32 max)
    {
        config.handleConfigUpdateMessage (item, value, min, max);
    }

    void handleConfigSetMessage(int32 item, int32 value)
    {
        config.handleConfigSetMessage (item, value);
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
        if (ledGrid != nullptr)
            if (auto renderer = ledGrid->getRenderer())
                renderer->renderLEDGrid (*ledGrid);

        remoteHeap.sendChanges (*this, false);

        if (lastMessageSendTime < juce::Time::getCurrentTime() - juce::RelativeTime::milliseconds (pingIntervalMs))
            sendCommandMessage (BlocksProtocol::ping);
    }

    //==============================================================================
    int32 getLocalConfigValue (uint32 item) override
    {
        initialiseDeviceIndexAndConnection();
        return config.getItemValue ((BlocksProtocol::ConfigItemId) item);
    }

    void setLocalConfigValue (uint32 item, int32 value) override
    {
        initialiseDeviceIndexAndConnection();
        config.setItemValue ((BlocksProtocol::ConfigItemId) item, value);
    }

    void setLocalConfigRange (uint32 item, int32 min, int32 max) override
    {
        initialiseDeviceIndexAndConnection();
        config.setItemMin ((BlocksProtocol::ConfigItemId) item, min);
        config.setItemMax ((BlocksProtocol::ConfigItemId) item, max);
    }

    void setLocalConfigItemActive (uint32 item, bool isActive) override
    {
        initialiseDeviceIndexAndConnection();
        config.setItemActive ((BlocksProtocol::ConfigItemId) item, isActive);
    }

    bool isLocalConfigItemActive (uint32 item) override
    {
        initialiseDeviceIndexAndConnection();
        return config.getItemActive ((BlocksProtocol::ConfigItemId) item);
    }

    uint32 getMaxConfigIndex() override
    {
        return uint32 (BlocksProtocol::maxConfigIndex);
    }

    bool isValidUserConfigIndex (uint32 item) override
    {
        return item >= (uint32) BlocksProtocol::ConfigItemId::user0
        && item < (uint32) (BlocksProtocol::ConfigItemId::user0 + numberOfUserConfigs);
    }

    ConfigMetaData getLocalConfigMetaData (uint32 item) override
    {
        initialiseDeviceIndexAndConnection();
        return config.getMetaData ((BlocksProtocol::ConfigItemId) item);
    }

    void requestFactoryConfigSync() override
    {
        initialiseDeviceIndexAndConnection();
        config.requestFactoryConfigSync();
    }

    void resetConfigListActiveStatus() override
    {
        config.resetConfigListActiveStatus();
    }

    void setConfigChangedCallback (std::function<void(Block&, const ConfigMetaData&, uint32)> configChanged) override
    {
        configChangedCallback = std::move (configChanged);
    }

    void setProgramLoadedCallback (std::function<void(Block&)> programLoaded) override
    {
        programLoadedCallback = std::move (programLoaded);
    }

    bool setName (const juce::String& newName) override
    {
        return buildAndSendPacket<128> ([&newName] (BlocksProtocol::HostPacketBuilder<128>& p)
                                        { return p.addSetBlockName (newName); });
    }

    void factoryReset() override
    {
        buildAndSendPacket<32> ([] (BlocksProtocol::HostPacketBuilder<32>& p)
                                { return p.addFactoryReset(); });
    }

    void blockReset() override
    {
        if (buildAndSendPacket<32> ([] (BlocksProtocol::HostPacketBuilder<32>& p)
                                    { return p.addBlockReset(); }))
        {
            hasBeenPowerCycled = true;

            if (detector != nullptr)
                detector->notifyBlockIsRestarting (uid);
        }
    }

    bool wasPowerCycled() const { return hasBeenPowerCycled; }
    void resetPowerCycleFlag()  { hasBeenPowerCycled = false; }

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

    WeakReference<Detector> detector;
    juce::Time lastMessageSendTime, lastMessageReceiveTime;

    BlockConfigManager config;
    std::function<void(Block&, const ConfigMetaData&, uint32)> configChangedCallback;

    std::function<void(Block&)> programLoadedCallback;

private:
    std::unique_ptr<Program> program;
    uint32 programSize = 0;

    std::function<void(uint8, uint32)> firmwarePacketAckCallback;

    bool isMaster = false;
    Block::UID masterUID = {};

    Point<int> position;
    int rotation = 0;
    friend Detector;

    bool isProgramLoaded = false;
    bool shouldSaveProgramAsDefault = false;
    bool hasBeenPowerCycled = false;

    void initialiseDeviceIndexAndConnection()
    {
        config.setDeviceIndex ((TopologyIndex) getDeviceIndex());
        config.setDeviceComms (listenerToMidiConnection);
    }

    const juce::MidiInput* getMidiInput() const
    {
        if (detector != nullptr)
            if (auto c = dynamic_cast<const MIDIDeviceConnection*> (detector->getDeviceConnectionFor (*this)))
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
        if (detector != nullptr)
            if (auto c = dynamic_cast<const MIDIDeviceConnection*> (detector->getDeviceConnectionFor (*this)))
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
        dataInputPortListeners.call ([&] (DataInputPortListener& l) { l.handleIncomingDataPortMessage (*this, message.getRawData(),
                                                                                                       (size_t) message.getRawDataSize()); });
    }

    void connectionBeingDeleted (const MIDIDeviceConnection& c) override
    {
        jassert (listenerToMidiConnection == &c);
        juce::ignoreUnused (c);
        listenerToMidiConnection->removeListener (this);
        listenerToMidiConnection = nullptr;
        config.setDeviceComms (nullptr);
    }

    void doSaveProgramAsDefault()
    {
        sendCommandMessage (BlocksProtocol::saveProgramAsDefault);
    }

    template<int packetBytes, typename PacketBuilderFn>
    bool buildAndSendPacket (PacketBuilderFn buildFn)
    {
        auto index = getDeviceIndex();

        if (index < 0)
        {
            jassertfalse;
            return false;
        }

        BlocksProtocol::HostPacketBuilder<packetBytes> p;
        p.writePacketSysexHeaderBytes ((BlocksProtocol::TopologyIndex) index);

        if (! buildFn (p))
            return false;

        p.writePacketSysexFooter();
        return sendMessageToDevice (p);
    }

public:
    //==============================================================================
    struct TouchSurfaceImplementation  : public TouchSurface,
                                         private juce::Timer
    {
        TouchSurfaceImplementation (BlockImplementation& b)  : TouchSurface (b), blockImpl (b)
        {
            activateTouchSurface();
        }

        ~TouchSurfaceImplementation()
        {
            disableTouchSurface();
        }

        void activateTouchSurface()
        {
            startTimer (500);
        }

        void disableTouchSurface()
        {
            stopTimer();
        }

        int getNumberOfKeywaves() const noexcept override
        {
            return blockImpl.modelData.numKeywaves;
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

                listeners.call ([&] (TouchSurface::Listener& l) { l.touchChanged (*this, t); });
            }

            // Normal handling:
            status.lastEventTime = juce::Time::getMillisecondCounter();
            status.isActive = ! touchEvent.isTouchEnd;

            if (touchEvent.isTouchStart)
                status.lastStrikePressure = touchEvent.zVelocity;

            listeners.call ([&] (TouchSurface::Listener& l) { l.touchChanged (*this, touchEvent); });
        }

        void timerCallback() override
        {
            // Find touches that seem to have become stuck, and fake a touch-end for them..
            static const uint32 touchTimeOutMs = 500;

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

            listeners.call ([&] (TouchSurface::Listener& l) { l.touchChanged (*this, killTouch); });

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
        }

        ~ControlButtonImplementation()
        {
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
                listeners.call ([&] (ControlButton::Listener& l) { l.buttonPressed (*this, timestamp); });
            else
                listeners.call ([&] (ControlButton::Listener& l) { l.buttonReleased (*this, timestamp); });
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

        BlockImplementation& blockImpl;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LEDGridImplementation)
    };

    //==============================================================================
    struct LEDRowImplementation  : public LEDRow,
                                   private Timer
    {
        LEDRowImplementation (BlockImplementation& b) : LEDRow (b)
        {
            startTimer (300);
        }

        void setButtonColour (uint32 index, LEDColour colour)
        {
            if (index < 10)
            {
                colours[index] = colour;
                flush();
            }
        }

        int getNumLEDs() const override
        {
            return static_cast<const BlockImplementation&> (block).modelData.numLEDRowLEDs;
        }

        void setLEDColour (int index, LEDColour colour) override
        {
            if ((uint32) index < 15u)
            {
                colours[10 + index] = colour;
                flush();
            }
        }

        void setOverlayColour (LEDColour colour) override
        {
            colours[25] = colour;
            flush();
        }

        void resetOverlayColour() override
        {
            setOverlayColour ({});
        }

    private:
        LEDColour colours[26];

        void timerCallback() override
        {
            stopTimer();
            loadProgramOntoBlock();
            flush();
        }

        void loadProgramOntoBlock()
        {
            if (block.getProgram() == nullptr)
            {
                auto err = block.setProgram (new DefaultLEDGridProgram (block));

                if (err.failed())
                {
                    DBG (err.getErrorMessage());
                    jassertfalse;
                }
            }
        }

        void flush()
        {
            if (block.getProgram() != nullptr)
                for (uint32 i = 0; i < (uint32) numElementsInArray (colours); ++i)
                    write565Colour (16 * i, colours[i]);
        }

        void write565Colour (uint32 bitIndex, LEDColour colour)
        {
            block.setDataBits (bitIndex,      5, colour.getRed()   >> 3);
            block.setDataBits (bitIndex + 5,  6, colour.getGreen() >> 2);
            block.setDataBits (bitIndex + 11, 5, colour.getBlue()  >> 3);
        }

        struct DefaultLEDGridProgram  : public Block::Program
        {
            DefaultLEDGridProgram (Block& b) : Block::Program (b) {}

            juce::String getLittleFootProgram() override
            {
                /*  Data format:

                 0:  10 x 5-6-5 bits for button LED RGBs
                 20: 15 x 5-6-5 bits for LED row colours
                 50:  1 x 5-6-5 bits for LED row overlay colour
                 */
                return R"littlefoot(

                #heapsize: 128

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
                        fillPixel (getLEDColour (x), x, 0);

                    for (int i = 0; i < 10; ++i)
                        fillPixel (getButtonColour (i), i, 1);
                }

                void handleMessage (int p1, int p2) {}

                )littlefoot";
            }
        };

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LEDRowImplementation)
    };

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BlockImplementation)
};

} // namespace juce

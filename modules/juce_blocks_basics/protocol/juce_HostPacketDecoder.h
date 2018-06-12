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
namespace BlocksProtocol
{

/**
    Parses data packets from a BLOCKS device, and translates them into callbacks
    on a handler object

    @tags{Blocks}
*/
template <typename Handler>
struct HostPacketDecoder
{
    static void processNextPacket (Handler& handler, TopologyIndex deviceIndex, const void* data, int size)
    {
        if (Packed7BitArrayReader::checksumIsOK (static_cast<const uint8*> (data), (uint32) size))
        {
            Packed7BitArrayReader reader (data, size - 1);

            if (reader.getRemainingBits() < (int) PacketTimestamp::bits)
            {
                jassertfalse; // not a valid message..
                return;
            }

            auto packetTimestamp = reader.read<PacketTimestamp>();
            deviceIndex &= 63; // top bit is used as a direction indicator

            bool topologyChanged = false;

            for (;;)
            {
                auto nextMessageType = getMessageType (reader);

                if (nextMessageType == 0)
                    break;

                topologyChanged |= messageIncludesTopologyChange (nextMessageType);

                if (! processNextMessage (handler, reader, (MessageFromDevice) nextMessageType, deviceIndex, packetTimestamp))
                    break;
            }

            if (topologyChanged)
                handler.notifyDetectorTopologyChanged();
        }
    }

    static uint32 getMessageType (Packed7BitArrayReader& reader)
    {
        if (reader.getRemainingBits() < MessageType::bits)
            return 0;

        return reader.read<MessageType>().get();
    }

    static bool messageIncludesTopologyChange (uint32 messageType)
    {
        switch ((MessageFromDevice) messageType)
        {
            case MessageFromDevice::deviceTopology:
            case MessageFromDevice::deviceTopologyExtend:
            case MessageFromDevice::deviceTopologyEnd:
            case MessageFromDevice::deviceVersion:
            case MessageFromDevice::deviceName:
                return true;

            default:
                return false;
        }
    }

    static bool processNextMessage (Handler& handler, Packed7BitArrayReader& reader,
                                    MessageFromDevice messageType, TopologyIndex deviceIndex,
                                    PacketTimestamp packetTimestamp)
    {
        switch (messageType)
        {
            case MessageFromDevice::deviceTopology:           return handleTopology (handler, reader, true);
            case MessageFromDevice::deviceTopologyExtend:     return handleTopology (handler, reader, false);
            case MessageFromDevice::deviceTopologyEnd:        return handleTopologyEnd (handler, reader);
            case MessageFromDevice::deviceVersion:            return handleVersion (handler, reader);
            case MessageFromDevice::deviceName:               return handleName (handler, reader);
            case MessageFromDevice::touchStart:               return handleTouch (handler, reader, deviceIndex, packetTimestamp, true, false);
            case MessageFromDevice::touchMove:                return handleTouch (handler, reader, deviceIndex, packetTimestamp, false, false);
            case MessageFromDevice::touchEnd:                 return handleTouch (handler, reader, deviceIndex, packetTimestamp, false, true);
            case MessageFromDevice::touchStartWithVelocity:   return handleTouchWithVelocity (handler, reader, deviceIndex, packetTimestamp, true, false);
            case MessageFromDevice::touchMoveWithVelocity:    return handleTouchWithVelocity (handler, reader, deviceIndex, packetTimestamp, false, false);
            case MessageFromDevice::touchEndWithVelocity:     return handleTouchWithVelocity (handler, reader, deviceIndex, packetTimestamp, false, true);
            case MessageFromDevice::controlButtonDown:        return handleButtonDownOrUp (handler, reader, deviceIndex, packetTimestamp, true);
            case MessageFromDevice::controlButtonUp:          return handleButtonDownOrUp (handler, reader, deviceIndex, packetTimestamp, false);
            case MessageFromDevice::programEventMessage:      return handleCustomMessage (handler, reader, deviceIndex, packetTimestamp);
            case MessageFromDevice::packetACK:                return handlePacketACK (handler, reader, deviceIndex);
            case MessageFromDevice::firmwareUpdateACK:        return handleFirmwareUpdateACK (handler, reader, deviceIndex);
            case MessageFromDevice::configMessage:            return handleConfigMessage (handler, reader, deviceIndex);
            case MessageFromDevice::logMessage:               return handleLogMessage (handler, reader, deviceIndex);

            default:
                jassertfalse; // got an invalid message type, could be a corrupt packet, or a
                              // message type that the host doesn't expect to get
                return false;
        }
    }

    static bool handleTopology (Handler& handler, Packed7BitArrayReader& reader, bool newTopology)
    {
        if (reader.getRemainingBits() < DeviceCount::bits + ConnectionCount::bits)
        {
            jassertfalse; // not enough data available for this message type!
            return false;
        }

        auto deviceProtocolVersion = reader.read<ProtocolVersion>();

        if (deviceProtocolVersion > currentProtocolVersion)
        {
            jassertfalse;
            return false;
        }

        const uint32 numDevices     = reader.read<DeviceCount>();
        const uint32 numConnections = reader.read<ConnectionCount>();

        if ((uint32) reader.getRemainingBits() < numDevices * BitSizes::topologyDeviceInfo
                                                    + numConnections * BitSizes::topologyConnectionInfo)
        {
            jassertfalse; // not enough data available for this message type!
            return false;
        }

        if (newTopology)
            handler.beginTopology ((int) numDevices, (int) numConnections);
        else
            handler.extendTopology ((int) numDevices, (int) numConnections);

        for (uint32 i = 0; i < numDevices; ++i)
            handleTopologyDevice (handler, reader);

        for (uint32 i = 0; i < numConnections; ++i)
            handleTopologyConnection (handler, reader);

        // Packet must be last in topology, otherwise wait for topology end message
        if (numDevices < maxBlocksInTopologyPacket && numConnections < maxConnectionsInTopologyPacket)
            handler.endTopology();

        return true;
    }

    static bool handleTopologyEnd (Handler& handler, Packed7BitArrayReader& reader)
    {
        auto deviceProtocolVersion = reader.read<ProtocolVersion>();

        if (deviceProtocolVersion > currentProtocolVersion)
        {
            jassertfalse;
            return false;
        }

        handler.endTopology();
        return true;
    }

    static void handleTopologyDevice (Handler& handler, Packed7BitArrayReader& reader)
    {
        DeviceStatus status;

        for (uint32 i = 0; i < sizeof (BlockSerialNumber); ++i)
            status.serialNumber.serial[i] = (uint8) reader.readBits (7);

        status.index            = (TopologyIndex) reader.readBits (topologyIndexBits);
        status.batteryLevel     = reader.read<BatteryLevel>();
        status.batteryCharging  = reader.read<BatteryCharging>();

        handler.handleTopologyDevice (status);
    }

    static void handleTopologyConnection (Handler& handler, Packed7BitArrayReader& reader)
    {
        DeviceConnection connection;

        connection.device1 = (uint8) reader.readBits (topologyIndexBits);
        connection.port1   = reader.read<ConnectorPort>();
        connection.device2 = (uint8) reader.readBits (topologyIndexBits);
        connection.port2   = reader.read<ConnectorPort>();

        handler.handleTopologyConnection (connection);
    }

    static bool handleVersion (Handler& handler, Packed7BitArrayReader& reader)
    {
        DeviceVersion version;

        version.index = (TopologyIndex) reader.readBits (topologyIndexBits);
        version.version.length = (uint8) reader.readBits (7);

        for (uint32 i = 0; i < version.version.length; ++i)
            version.version.version[i] = (uint8) reader.readBits (7);

        handler.handleVersion (version);
        return true;
    }

    static bool handleName (Handler& handler, Packed7BitArrayReader& reader)
    {
        DeviceName name;

        name.index = (TopologyIndex) reader.readBits (topologyIndexBits);
        name.name.length = (uint8) reader.readBits (7);

        for (uint32 i = 0; i < name.name.length; ++i)
            name.name.name[i] = (uint8) reader.readBits (7);

        handler.handleName (name);
        return true;
    }

    static bool handleTouch (Handler& handler, Packed7BitArrayReader& reader, TopologyIndex deviceIndex,
                             PacketTimestamp packetTimestamp, bool isStart, bool isEnd)
    {
        if (reader.getRemainingBits() < BitSizes::touchMessage - MessageType::bits)
        {
            jassertfalse; // not enough data available for this message type!
            return false;
        }

        auto timeOffset  = reader.read<PacketTimestampOffset>();
        auto touchIndex  = reader.read<TouchIndex>();
        auto x           = reader.read<TouchPosition::Xcoord>();
        auto y           = reader.read<TouchPosition::Ycoord>();
        auto z           = reader.read<TouchPosition::Zcoord>();

        handleTouch (handler, deviceIndex, packetTimestamp.get() + timeOffset.get(),
                     touchIndex, { x, y, z }, { 0, 0, 0 }, isStart, isEnd);
        return true;
    }

    static bool handleTouchWithVelocity (Handler& handler, Packed7BitArrayReader& reader, TopologyIndex deviceIndex,
                                         PacketTimestamp packetTimestamp, bool isStart, bool isEnd)
    {
        if (reader.getRemainingBits() < BitSizes::touchMessageWithVelocity - MessageType::bits)
        {
            jassertfalse; // not enough data available for this message type!
            return false;
        }

        auto timeOffset  = reader.read<PacketTimestampOffset>();
        auto touchIndex  = reader.read<TouchIndex>();
        auto x           = reader.read<TouchPosition::Xcoord>();
        auto y           = reader.read<TouchPosition::Ycoord>();
        auto z           = reader.read<TouchPosition::Zcoord>();
        auto vx          = reader.read<TouchVelocity::VXcoord>();
        auto vy          = reader.read<TouchVelocity::VYcoord>();
        auto vz          = reader.read<TouchVelocity::VZcoord>();

        handleTouch (handler, deviceIndex, packetTimestamp.get() + timeOffset.get(),
                     touchIndex, { x, y, z }, { vx, vy, vz }, isStart, isEnd);
        return true;
    }

    static void handleTouch (Handler& handler, TopologyIndex deviceIndex, uint32 timestamp, TouchIndex touchIndex,
                             TouchPosition position, TouchVelocity velocity, bool isStart, bool isEnd)
    {
        handler.handleTouchChange (deviceIndex, timestamp, touchIndex, position, velocity, isStart, isEnd);
    }

    static bool handleButtonDownOrUp (Handler& handler, Packed7BitArrayReader& reader, TopologyIndex deviceIndex,
                                      PacketTimestamp packetTimestamp, bool isDown)
    {
        if (reader.getRemainingBits() < BitSizes::controlButtonMessage - MessageType::bits)
        {
            jassertfalse; // not enough data available for this message type!
            return false;
        }

        auto timeOffset  = reader.read<PacketTimestampOffset>();
        auto buttonID    = reader.read<ControlButtonID>();

        handler.handleControlButtonUpDown (deviceIndex, packetTimestamp.get() + timeOffset.get(), buttonID, isDown);
        return true;
    }

    static bool handleCustomMessage (Handler& handler, Packed7BitArrayReader& reader,
                                     TopologyIndex deviceIndex, PacketTimestamp packetTimestamp)
    {
        if (reader.getRemainingBits() < BitSizes::programEventMessage - MessageType::bits)
        {
            jassertfalse; // not enough data available for this message type!
            return false;
        }

        int32 data[numProgramMessageInts] = {};

        for (uint32 i = 0; i < numProgramMessageInts; ++i)
            data[i] = (int32) reader.read<IntegerWithBitSize<32>>().get();

        handler.handleCustomMessage (deviceIndex, packetTimestamp.get(), data);
        return true;
    }

    static bool handlePacketACK (Handler& handler, Packed7BitArrayReader& reader, TopologyIndex deviceIndex)
    {
        if (reader.getRemainingBits() < BitSizes::packetACK - MessageType::bits)
        {
            jassertfalse; // not enough data available for this message type!
            return false;
        }

        handler.handlePacketACK (deviceIndex, reader.read<PacketCounter>());
        return true;
    }

    static bool handleFirmwareUpdateACK (Handler& handler, Packed7BitArrayReader& reader, TopologyIndex deviceIndex)
    {
        if (reader.getRemainingBits() < FirmwareUpdateACKCode::bits)
        {
            jassertfalse; // not enough data available for this message type!
            return false;
        }

        auto ackCode   = reader.read<FirmwareUpdateACKCode>();
        auto ackDetail = reader.read<FirmwareUpdateACKDetail>();

        handler.handleFirmwareUpdateACK (deviceIndex, ackCode, ackDetail);
        return true;
    }

    static bool handleConfigMessage (Handler& handler, Packed7BitArrayReader& reader, TopologyIndex deviceIndex)
    {
        ConfigCommand type = reader.read<ConfigCommand>().get();

        if (type == updateConfig)
        {
            auto item  = (int32) reader.read<IntegerWithBitSize<8>>().get();
            auto value = (int32) reader.read<IntegerWithBitSize<32>>().get();
            auto min   = (int32) reader.read<IntegerWithBitSize<32>>().get();
            auto max   = (int32) reader.read<IntegerWithBitSize<32>>().get();

            handler.handleConfigUpdateMessage (deviceIndex, item, value, min, max);
            return true;
        }

        if (type == setConfig)
        {
            auto item  = (int32) reader.read<IntegerWithBitSize<8>>().get();
            auto value = (int32) reader.read<IntegerWithBitSize<32>>().get();

            handler.handleConfigSetMessage (deviceIndex, item, value);
            return true;
        }

        if (type == factorySyncEnd)
        {
            handler.handleConfigFactorySyncEndMessage (deviceIndex);
        }

        return true;
    }

    static bool handleLogMessage (Handler& handler, Packed7BitArrayReader& reader, TopologyIndex deviceIndex)
    {
        String message;

        while (reader.getRemainingBits() >= 7)
        {
            uint32 c = reader.read<IntegerWithBitSize<7>>();
            message << (char) c;
        }

        handler.handleLogMessage (deviceIndex, message);
        return true;
    }
};

} // namespace BlocksProtocol
} // namespace juce

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


/**
    Parses data packets from a BLOCKS device, and translates them into callbacks
    on a handler object
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

            while (processNextMessage (handler, reader, deviceIndex, packetTimestamp))
            {}
        }
    }

    static bool processNextMessage (Handler& handler, Packed7BitArrayReader& reader,
                                    TopologyIndex deviceIndex, PacketTimestamp packetTimestamp)
    {
        if (reader.getRemainingBits() < MessageType::bits)
            return false;

        auto messageType = reader.read<MessageType>().get();

        if (messageType == 0)
            return false;

        switch ((MessageFromDevice) messageType)
        {
            case MessageFromDevice::deviceTopology:           return handleTopology (handler, reader);
            case MessageFromDevice::touchStart:               return handleTouch (handler, reader, deviceIndex, packetTimestamp, true, false);
            case MessageFromDevice::touchMove:                return handleTouch (handler, reader, deviceIndex, packetTimestamp, false, false);
            case MessageFromDevice::touchEnd:                 return handleTouch (handler, reader, deviceIndex, packetTimestamp, false, true);
            case MessageFromDevice::touchStartWithVelocity:   return handleTouchWithVelocity (handler, reader, deviceIndex, packetTimestamp, true, false);
            case MessageFromDevice::touchMoveWithVelocity:    return handleTouchWithVelocity (handler, reader, deviceIndex, packetTimestamp, false, false);
            case MessageFromDevice::touchEndWithVelocity:     return handleTouchWithVelocity (handler, reader, deviceIndex, packetTimestamp, false, true);
            case MessageFromDevice::controlButtonDown:        return handleButtonDownOrUp (handler, reader, deviceIndex, packetTimestamp, true);
            case MessageFromDevice::controlButtonUp:          return handleButtonDownOrUp (handler, reader, deviceIndex, packetTimestamp, false);
            case MessageFromDevice::packetACK:                return handlePacketACK (handler, reader, deviceIndex);

            default:
                jassertfalse; // got an invalid message type, could be a corrupt packet, or a
                              // message type that the host doesn't expect to get
                return false;
        }
    }

    static bool handleTopology (Handler& handler, Packed7BitArrayReader& reader)
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

        handler.beginTopology ((int) numDevices, (int) numConnections);

        for (uint32 i = 0; i < numDevices; ++i)
            handleTopologyDevice (handler, reader);

        for (uint32 i = 0; i < numConnections; ++i)
            handleTopologyConnection (handler, reader);

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
};

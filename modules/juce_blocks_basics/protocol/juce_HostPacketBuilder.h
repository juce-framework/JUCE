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
namespace BlocksProtocol
{

/**
    Helper class for constructing a packet for sending to a BLOCKS device

    @tags{Blocks}
*/
template <int maxPacketBytes>
struct HostPacketBuilder
{
    HostPacketBuilder() noexcept {}
    HostPacketBuilder (const HostPacketBuilder&) = delete;
    HostPacketBuilder (HostPacketBuilder&&) = default;

    const void* getData() const noexcept        { return data.getData(); }
    int size() const noexcept                   { return data.size(); }

    //==============================================================================
    void writePacketSysexHeaderBytes (TopologyIndex deviceIndex) noexcept
    {
        static_assert (maxPacketBytes > 10, "Not enough bytes for a sensible message!");

        jassert ((deviceIndex & 64) == 0);

        data.writeHeaderSysexBytes (deviceIndex);
    }

    void writePacketSysexFooter() noexcept
    {
        data.writePacketSysexFooter();
    }

    //==============================================================================
    bool deviceControlMessage (DeviceCommand command) noexcept
    {
        if (! data.hasCapacity ((int) MessageType::bits + (int) DeviceCommand::bits))
            return false;

        writeMessageType (MessageFromHost::deviceCommandMessage);
        data << command;
        return true;
    }

    //==============================================================================
    bool beginDataChanges (PacketIndex packetIndex) noexcept
    {
        if (! data.hasCapacity ((int) MessageType::bits + (int) PacketIndex::bits + (int) DataChangeCommand::bits))
            return false;

        writeMessageType (MessageFromHost::sharedDataChange);
        data << packetIndex;
        return true;
    }

    bool endDataChanges (bool isLastChange) noexcept
    {
        if (! data.hasCapacity (DataChangeCommand::bits))
            return false;

        data << DataChangeCommand ((uint32) isLastChange ? endOfChanges : endOfPacket);
        return true;
    }

    bool skipBytes (int numToSkip) noexcept
    {
        if (numToSkip <= 0)
            return true;

        auto state = data.getState();

        while (numToSkip > ByteCountMany::maxValue)
        {
            if (! skipBytes (ByteCountMany::maxValue))
            {
                data.restore (state);
                return false;
            }

            numToSkip -= ByteCountMany::maxValue;
        }

        if (numToSkip > ByteCountFew::maxValue)
        {
            if (! data.hasCapacity (DataChangeCommand::bits * 2 + ByteCountMany::bits))
            {
                data.restore (state);
                return false;
            }

            data << DataChangeCommand ((uint32) skipBytesMany) << ByteCountMany ((uint32) numToSkip);
            return true;
        }

        if (! data.hasCapacity (DataChangeCommand::bits * 2 + ByteCountFew::bits))
        {
            data.restore (state);
            return false;
        }

        data << DataChangeCommand ((uint32) skipBytesFew) << ByteCountFew ((uint32) numToSkip);
        return true;
    }

    bool setMultipleBytes (const uint8* values, int num) noexcept
    {
        if (num <= 0)
            return true;

        if (! data.hasCapacity (DataChangeCommand::bits * 2 + num * (1 + ByteValue::bits)))
            return false;

        data << DataChangeCommand ((uint32) setSequenceOfBytes);

        for (int i = 0; i < num; ++i)
            data << ByteValue ((uint32) values[i])
                 << ByteSequenceContinues (i < num - 1 ? 1 : 0);

        return true;
    }

    bool setMultipleBytes (uint8 value, uint8 lastValue, int num) noexcept
    {
        if (num <= 0)
            return true;

        if (num == 1)
            return setMultipleBytes (&value, 1); // (this is a more compact message)

        auto state = data.getState();

        if (num > ByteCountMany::maxValue)
        {
            if (! setMultipleBytes (value, lastValue, ByteCountMany::maxValue))
            {
                data.restore (state);
                return false;
            }

            return setMultipleBytes (value, lastValue, num - ByteCountMany::maxValue);
        }

        if (num > ByteCountFew::maxValue)
        {
            if (! data.hasCapacity (DataChangeCommand::bits * 2 + ByteCountMany::bits + ByteValue::bits))
            {
                data.restore (state);
                return false;
            }

            data << DataChangeCommand ((uint32) setManyBytesWithValue)
                 << ByteCountMany ((uint32) num)
                 << ByteValue ((uint32) value);

            return true;
        }

        if (value == lastValue)
        {
            if (! data.hasCapacity (DataChangeCommand::bits * 2 + ByteCountFew::bits))
            {
                data.restore (state);
                return false;
            }

            data << DataChangeCommand ((uint32) setFewBytesWithLastValue) << ByteCountFew ((uint32) num);
            return true;
        }

        if (! data.hasCapacity (DataChangeCommand::bits * 2 + ByteCountFew::bits + ByteValue::bits))
        {
            data.restore (state);
            return false;
        }

        data << DataChangeCommand ((uint32) setFewBytesWithValue) << ByteCountFew ((uint32) num)
             << ByteValue ((uint32) value);

        return true;
    }

    bool addProgramEventMessage (const int32* messageData)
    {
        if (! data.hasCapacity (BitSizes::programEventMessage))
            return false;

        writeMessageType (MessageFromHost::programEventMessage);

        for (uint32 i = 0; i < numProgramMessageInts; ++i)
            data << IntegerWithBitSize<32> ((uint32) messageData[i]);

        return true;
    }

    bool addFirmwareUpdatePacket (const uint8* packetData, uint8 size)
    {
        if (! data.hasCapacity (MessageType::bits + FirmwareUpdatePacketSize::bits + 7 * size))
            return false;

        writeMessageType (MessageFromHost::firmwareUpdatePacket);
        data << FirmwareUpdatePacketSize (size);

        for (uint8 i = 0; i < size; ++i)
            data << IntegerWithBitSize<7> ((uint32) packetData[i]);

        return true;
    }

    //==============================================================================
    bool addConfigSetMessage (int32 item, int32 value)
    {
        if (! data.hasCapacity (BitSizes::configSetMessage))
            return false;

        writeMessageType(MessageFromHost::configMessage);
        ConfigCommand type = ConfigCommands::setConfig;
        data << type << IntegerWithBitSize<8> ((uint32) item) << IntegerWithBitSize<32>((uint32) value);
        return true;
    }

    bool addRequestMessage (int32 item)
    {
        if (! data.hasCapacity (BitSizes::configSetMessage))
            return false;

        writeMessageType(MessageFromHost::configMessage);
        ConfigCommand type = ConfigCommands::requestConfig;
        data << type << IntegerWithBitSize<32> (0) << IntegerWithBitSize<8> ((uint32) item);
        return true;
    }

    bool addRequestFactorySyncMessage()
    {
        if (! data.hasCapacity ((int) MessageType::bits + (int) ConfigCommand::bits))
            return false;

        writeMessageType (MessageFromHost::configMessage);
        ConfigCommand type = ConfigCommands::requestFactorySync;
        data << type;
        return true;
    }

    bool addRequestUserSyncMessage()
    {
        if (! data.hasCapacity ((int) MessageType::bits + (int) ConfigCommand::bits))
            return false;

        writeMessageType (MessageFromHost::configMessage);
        ConfigCommand type = ConfigCommands::requestUserSync;
        data << type;
        return true;
    }

    //==============================================================================
    bool addFactoryReset()
    {
        if (! data.hasCapacity (MessageType::bits))
            return false;

        writeMessageType (MessageFromHost::factoryReset);
        return true;
    }

    bool addBlockReset()
    {
        if (! data.hasCapacity (MessageType::bits))
            return false;

        writeMessageType (MessageFromHost::blockReset);
        return true;
    }

    bool addSetBlockName (const String& name)
    {
        if (name.length() > 32 || ! data.hasCapacity (MessageType::bits + 7 + (7 * name.length())))
            return false;

        writeMessageType (MessageFromHost::setName);

        data << IntegerWithBitSize<7> ((uint32) name.length());

        for (auto i = 0; i < name.length(); ++i)
            data << IntegerWithBitSize<7> ((uint32) name.toRawUTF8()[i]);

        data << IntegerWithBitSize<7> (0);

        return true;
    }

    //==============================================================================
private:
    Packed7BitArrayBuilder<maxPacketBytes> data;

    void writeMessageType (MessageFromHost type) noexcept
    {
        data << MessageType ((uint32) type);
    }
};

} // namespace BlocksProtocol
} // namespace juce

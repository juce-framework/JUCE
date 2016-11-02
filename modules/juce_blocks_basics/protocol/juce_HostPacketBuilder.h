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
    Helper class for constructing a packet for sending to a BLOCKS device
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
        if (! data.hasCapacity (MessageType::bits + DeviceCommand::bits))
            return false;

        writeMessageType (MessageFromHost::deviceCommandMessage);
        data << command;
        return true;
    }

    //==============================================================================
    bool beginDataChanges (PacketIndex packetIndex) noexcept
    {
        if (! data.hasCapacity (MessageType::bits + PacketIndex::bits + DataChangeCommand::bits))
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

    //==============================================================================
private:
    Packed7BitArrayBuilder<maxPacketBytes> data;

    void writeMessageType (MessageFromHost type) noexcept
    {
        data << MessageType ((uint32) type);
    }
};

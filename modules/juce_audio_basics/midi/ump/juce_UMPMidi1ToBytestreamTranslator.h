/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

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

#ifndef DOXYGEN

namespace juce
{
namespace universal_midi_packets
{

/**
    Parses a raw stream of uint32_t holding a series of Universal MIDI Packets using
    the MIDI 1.0 Protocol, converting to plain (non-UMP) MidiMessages.

    @tags{Audio}
*/
class Midi1ToBytestreamTranslator
{
public:
    /** Ensures that there is room in the internal buffer for a sysex message of at least
        `initialBufferSize` bytes.
    */
    explicit Midi1ToBytestreamTranslator (int initialBufferSize)
    {
        pendingSysExData.reserve (size_t (initialBufferSize));
    }

    /** Clears the concatenator. */
    void reset()
    {
        pendingSysExData.clear();
        pendingSysExTime = 0.0;
    }

    /** Converts a Universal MIDI Packet using the MIDI 1.0 Protocol to
        an equivalent MidiMessage. Accumulates SysEx packets into a single
        MidiMessage, as appropriate.

        @param packet       a packet which is using the MIDI 1.0 Protocol.
        @param time         the timestamp to be applied to these messages.
        @param callback     a callback which will be called with each converted MidiMessage.
    */
    template <typename MessageCallback>
    void dispatch (const View& packet, double time, MessageCallback&& callback)
    {
        const auto firstWord = *packet.data();

        if (! pendingSysExData.empty() && shouldPacketTerminateSysExEarly (firstWord))
            pendingSysExData.clear();

        switch (packet.size())
        {
            case 1:
            {
                // Utility messages don't translate to bytestream format
                if (Utils::getMessageType (firstWord) != 0x00)
                    callback (fromUmp (PacketX1 { firstWord }, time));

                break;
            }

            case 2:
            {
                if (Utils::getMessageType (firstWord) == 0x3)
                    processSysEx (PacketX2 { packet[0], packet[1] }, time, callback);

                break;
            }

            case 3:  // no 3-word packets in the current spec
            case 4:  // no 4-word packets translate to bytestream format
            default:
                break;
        }
    }

    /** Converts from a Universal MIDI Packet to MIDI 1 bytestream format.

        This is only capable of converting a single Universal MIDI Packet to
        an equivalent bytestream MIDI message. This function cannot understand
        multi-packet messages, like SysEx7 messages.

        To convert multi-packet messages, use `Midi1ToBytestreamTranslator`
        to convert from a UMP MIDI 1.0 stream, or `ToBytestreamDispatcher`
        to convert from both MIDI 2.0 and MIDI 1.0.
    */
    static MidiMessage fromUmp (const PacketX1& m, double time = 0)
    {
        const auto word = m.front();
        jassert (Utils::getNumWordsForMessageType (word) == 1);

        const std::array<uint8_t, 3> bytes { { uint8_t ((word >> 0x10) & 0xff),
                                               uint8_t ((word >> 0x08) & 0xff),
                                               uint8_t ((word >> 0x00) & 0xff) } };
        const auto numBytes = MidiMessage::getMessageLengthFromFirstByte (bytes.front());
        return MidiMessage (bytes.data(), numBytes, time);
    }

private:
    template <typename MessageCallback>
    void processSysEx (const PacketX2& packet,
                       double time,
                       MessageCallback&& callback)
    {
        switch (getSysEx7Kind (packet[0]))
        {
            case SysEx7::Kind::complete:
                startSysExMessage (time);
                pushBytes (packet);
                terminateSysExMessage (callback);
                break;

            case SysEx7::Kind::begin:
                startSysExMessage (time);
                pushBytes (packet);
                break;

            case SysEx7::Kind::continuation:
                if (pendingSysExData.empty())
                    break;

                pushBytes (packet);
                break;

            case SysEx7::Kind::end:
                if (pendingSysExData.empty())
                    break;

                pushBytes (packet);
                terminateSysExMessage (callback);
                break;
        }
    }

    void pushBytes (const PacketX2& packet)
    {
        const auto bytes = SysEx7::getDataBytes (packet);
        pendingSysExData.insert (pendingSysExData.end(),
                                 bytes.data.begin(),
                                 bytes.data.begin() + bytes.size);
    }

    void startSysExMessage (double time)
    {
        pendingSysExTime = time;
        pendingSysExData.push_back (0xf0);
    }

    template <typename MessageCallback>
    void terminateSysExMessage (MessageCallback&& callback)
    {
        pendingSysExData.push_back (0xf7);
        callback (MidiMessage (pendingSysExData.data(),
                               int (pendingSysExData.size()),
                               pendingSysExTime));
        pendingSysExData.clear();
    }

    static bool shouldPacketTerminateSysExEarly (uint32_t firstWord)
    {
        return ! (isSysExContinuation (firstWord)
                  || isSystemRealTime (firstWord)
                  || isJROrNOP (firstWord));
    }

    static SysEx7::Kind getSysEx7Kind (uint32_t word)
    {
        return SysEx7::Kind ((word >> 0x14) & 0xf);
    }

    static bool isJROrNOP (uint32_t word)
    {
        return Utils::getMessageType (word) == 0x0;
    }

    static bool isSysExContinuation (uint32_t word)
    {
        if (Utils::getMessageType (word) != 0x3)
            return false;

        const auto kind = getSysEx7Kind (word);
        return kind == SysEx7::Kind::continuation || kind == SysEx7::Kind::end;
    }

    static bool isSystemRealTime (uint32_t word)
    {
        return Utils::getMessageType (word) == 0x1 && ((word >> 0x10) & 0xff) >= 0xf8;
    }

    std::vector<uint8_t> pendingSysExData;

    double pendingSysExTime = 0.0;
};

}
}

#endif

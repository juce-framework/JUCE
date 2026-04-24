/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

/** @cond */
namespace juce::universal_midi_packets
{

/**
    Extracts from a series of Universal MIDI Packets the bytes that are also meaningful in the
    bytestream MIDI 1.0 format.
*/
class SingleGroupMidi1ToBytestreamExtractor
{
public:
    void reset()
    {
        sysexInProgress = false;
    }

    /** Converts a Universal MIDI Packet using the MIDI 1.0 Protocol to
        an equivalent MidiMessage. If the packet doesn't convert to a single bytestream message
        (as may be the case for long sysex7 data), then the the callback will be passed just the
        sysex bytes in the current packet. To reconstruct the entire sysex message, the caller
        can bytes that are marked as ongoingSysex, and process the full message once the callback
        receives bytes that are marked as lastSysex.

        @param packet       a packet which is using the MIDI 1.0 Protocol.
        @param time         the timestamp to be applied to these messages.
        @param callback     a callback that will be called with each converted MidiMessage.
    */
    template <typename MessageCallback>
    void dispatch (const View& packet, double time, MessageCallback&& callback)
    {
        const auto firstWord = *packet.data();

        if (sysexInProgress && shouldPacketTerminateSysExEarly (firstWord))
        {
            // unexpected end of last sysex
            callback (SysexExtractorCallbackKind::lastSysex, Span<const std::byte>());
            sysexInProgress = false;
        }

        switch (packet.size())
        {
            case 1:
            {
                // Utility messages don't translate to bytestream format
                if (Utils::getMessageType (firstWord) != Utils::MessageKind::utility)
                {
                    const auto converted = fromUmp (PacketX1 { firstWord }, time);
                    callback (SysexExtractorCallbackKind::notSysex,
                              Span (unalignedPointerCast<const std::byte*> (converted.getRawData()),
                                    (size_t) converted.getRawDataSize()));
                }

                break;
            }

            case 2:
            {
                if (Utils::getMessageType (firstWord) == Utils::MessageKind::sysex7)
                    processSysEx (PacketX2 { packet[0], packet[1] }, callback);

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

        const std::array<std::byte, 3> bytes { { std::byte ((word >> 0x10) & 0xff),
                                                 std::byte ((word >> 0x08) & 0xff),
                                                 std::byte ((word >> 0x00) & 0xff) } };
        const auto numBytes = MidiMessage::getMessageLengthFromFirstByte ((uint8_t) bytes.front());
        return MidiMessage (bytes.data(), numBytes, time);
    }

private:
    template <typename MessageCallback>
    void processSysEx (const PacketX2& packet, MessageCallback&& callback)
    {
        const std::array<std::byte, 1> initial { std::byte { 0xf0 } }, final { std::byte { 0xf7 } };
        std::array<std::byte, 8> storage{};
        size_t validBytes = 0;

        const auto pushBytes = [&] (const Span<const std::byte> b)
        {
            std::copy (b.begin(), b.end(), storage.data() + validBytes);
            validBytes += b.size();
        };

        const auto pushPacket = [&] (const PacketX2& p)
        {
            const auto newBytes = SysEx7::getDataBytes (p);
            pushBytes (Span<const std::byte> (newBytes.data.data(), newBytes.size));
        };

        const auto kind = getSysEx7Kind (packet[0]);

        if (   (  sysexInProgress && (kind == SysEx7::Kind::begin || kind == SysEx7::Kind::complete))
            || (! sysexInProgress && (kind == SysEx7::Kind::continuation || kind == SysEx7::Kind::end)))
        {
            // Malformed SysEx, drop progress and return
            callback (SysexExtractorCallbackKind::lastSysex, Span<const std::byte>());
            sysexInProgress = false;
            return;
        }

        switch (kind)
        {
            case SysEx7::Kind::complete:
                pushBytes (Span (initial));
                pushPacket (packet);
                pushBytes (Span (final));
                break;

            case SysEx7::Kind::begin:
                pushBytes (Span (initial));
                pushPacket (packet);
                break;

            case SysEx7::Kind::continuation:
                pushPacket (packet);
                break;

            case SysEx7::Kind::end:
                pushPacket (packet);
                pushBytes (Span (final));
                break;
        }

        sysexInProgress = sysexInProgress ? (kind == SysEx7::Kind::continuation)
                                          : (kind == SysEx7::Kind::begin);
        const auto callbackKind = sysexInProgress ? SysexExtractorCallbackKind::ongoingSysex
                                                  : SysexExtractorCallbackKind::lastSysex;
        callback (callbackKind, Span (storage.data(), validBytes));
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
        return Utils::getMessageType (word) == Utils::MessageKind::utility;
    }

    static bool isSysExContinuation (uint32_t word)
    {
        if (Utils::getMessageType (word) != Utils::MessageKind::sysex7)
            return false;

        const auto kind = getSysEx7Kind (word);
        return kind == SysEx7::Kind::continuation || kind == SysEx7::Kind::end;
    }

    static bool isSystemRealTime (uint32_t word)
    {
        return Utils::getMessageType (word) == Utils::MessageKind::commonRealtime && ((word >> 0x10) & 0xff) >= 0xf8;
    }

    bool sysexInProgress = false;
};

/**
    Parses a raw stream of uint32_t holding a series of Universal MIDI Packets using
    the MIDI 1.0 Protocol, converting to plain (non-UMP) MidiMessages.

    @tags{Audio}
*/
class SingleGroupMidi1ToBytestreamTranslator
{
public:
    /** Ensures that there is room in the internal buffer for a sysex message of at least
        initialBufferSize bytes.
    */
    explicit SingleGroupMidi1ToBytestreamTranslator (int initialBufferSize)
    {
        pendingSysExData.reserve (size_t (initialBufferSize));
    }

    /** Clears the concatenator. */
    void reset()
    {
        extractor.reset();
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
        extractor.dispatch (packet, time, [&] (SysexExtractorCallbackKind kind, Span<const std::byte> bytes)
        {
            switch (kind)
            {
                case SysexExtractorCallbackKind::notSysex:
                    callback (BytesOnGroup { 0, bytes }, time);
                    return;

                case SysexExtractorCallbackKind::ongoingSysex:
                {
                    if (pendingSysExData.empty())
                        pendingSysExTime = time;

                    pendingSysExData.insert (pendingSysExData.end(), bytes.begin(), bytes.end());
                    return;
                }

                case SysexExtractorCallbackKind::lastSysex:
                {
                    pendingSysExData.insert (pendingSysExData.end(), bytes.begin(), bytes.end());

                    if (pendingSysExData.empty())
                        return;

                    // If this is not true, then the sysex message was truncated somehow and we
                    // probably shouldn't allow it to propagate
                    if (pendingSysExData.back() == std::byte { 0xf7 })
                        callback (BytesOnGroup { 0, Span<const std::byte> (pendingSysExData) }, pendingSysExTime);

                    pendingSysExData.clear();

                    return;
                }
            }
        });
    }

private:
    SingleGroupMidi1ToBytestreamExtractor extractor;
    std::vector<std::byte> pendingSysExData;
    double pendingSysExTime = 0.0;
};

} // namespace juce::universal_midi_packets
/** @endcond */

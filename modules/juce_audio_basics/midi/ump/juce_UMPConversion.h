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

/** Represents a MIDI message on bytestream transport that happened at a particular time.

    Unlike MidiMessage, BytestreamMidiView is non-owning.
*/
struct BytestreamMidiView
{
    constexpr BytestreamMidiView (Span<const std::byte> bytesIn, double timestampIn)
        : bytes (bytesIn), timestamp (timestampIn) {}

    /** Creates a view over the provided message.

        Note that the argument is a pointer, not a reference, in order to avoid taking a reference
        to a temporary.
    */
    explicit BytestreamMidiView (const MidiMessage* msg)
        : bytes (msg->asSpan()),
          timestamp (msg->getTimeStamp()) {}

    explicit BytestreamMidiView (const MidiMessageMetadata msg)
        : bytes (msg.asSpan()),
          timestamp (msg.samplePosition) {}

    MidiMessage getMessage() const
    {
        return MidiMessage (bytes.data(), (int) bytes.size(), timestamp);
    }

    MidiMessageMetadata getMidiMessageMetadata() const
    {
        return MidiMessageMetadata { reinterpret_cast<const uint8*> (bytes.data()),
                                     (int) bytes.size(),
                                     (int) timestamp };
    }

    Span<const std::byte> bytes;
    double timestamp = 0.0;
};

//==============================================================================
/**
    Functions to assist conversion of UMP messages to/from other formats,
    especially older 'bytestream' formatted MidiMessages.

    @tags{Audio}
*/
struct Conversion
{
    /** Converts 7-bit data (the most significant bit of each byte must be unset) to a series of
        Universal MIDI Packets.
    */
    template <typename PacketCallbackFunction>
    static void umpFrom7BitData (BytesOnGroup msg, PacketCallbackFunction&& callback)
    {
        // If this is hit, non-7-bit data was supplied.
        // Maybe you forgot to trim the leading/trailing bytes that delimit a bytestream SysEx message.
        jassert (std::all_of (msg.bytes.begin(), msg.bytes.end(), [] (std::byte b) { return (b & std::byte { 0x80 }) == std::byte{}; }));

        Factory::splitIntoPackets (msg.bytes, 6, [&] (SysEx7::Kind kind, Span<const std::byte> bytesThisTime)
        {
            const auto packet = Factory::Detail::makeSysEx (msg.group, kind, bytesThisTime);
            callback (View (packet.data()));
        });
    }

    /** Converts from a MIDI 1 bytestream to MIDI 1 on Universal MIDI Packets.

        @param bytes    the bytes in a single well-formed bytestream MIDI message
        @param callback a function that accepts a single View argument. This may be called several
                        times for each invocation of toMidi1 if the bytestream message converts
                        to multiple Universal MIDI Packets.
    */
    template <typename PacketCallbackFunction>
    static void toMidi1 (const BytesOnGroup& groupBytes, PacketCallbackFunction&& callback)
    {
        const auto size = groupBytes.bytes.size();

        if (size <= 0)
            return;

        const auto* data = groupBytes.bytes.data();
        const auto firstByte = data[0];

        if (firstByte != std::byte { 0xf0 })
        {
            const auto mask = [size]() -> uint32_t
            {
                switch (size)
                {
                    case 0: return 0xff000000;
                    case 1: return 0xffff0000;
                    case 2: return 0xffffff00;
                    case 3: return 0xffffffff;
                }

                // This function can only handle a single bytestream MIDI message at a time!
                jassertfalse;
                return 0x00000000;
            }();

            const auto extraByte = ((((firstByte & std::byte { 0xf0 }) == std::byte { 0xf0 }) ? std::byte { 0x1 } : std::byte { 0x2 }) << 0x4);
            const std::byte group { (uint8_t) (groupBytes.group & 0xf) };
            const PacketX1 packet { mask & Utils::bytesToWord (extraByte | group, data[0], data[1], data[2]) };
            callback (View (packet.data()));
            return;
        }

        umpFrom7BitData ({ groupBytes.group, Span (data + 1, size - 2) }, std::forward<PacketCallbackFunction> (callback));
    }

    /** Widens a 7-bit MIDI 1.0 value to a 8-bit MIDI 2.0 value. */
    static uint8_t scaleTo8 (uint8_t word7Bit)
    {
        const auto shifted = (uint8_t) (word7Bit << 0x1);
        const auto repeat = (uint8_t) (word7Bit & 0x3f);
        const auto mask = (uint8_t) (word7Bit <= 0x40 ? 0x0 : 0xff);
        return (uint8_t) (shifted | ((repeat >> 5) & mask));
    }

    /** Widens a 7-bit MIDI 1.0 value to a 16-bit MIDI 2.0 value. */
    static uint16_t scaleTo16 (uint8_t word7Bit)
    {
        const auto shifted = (uint16_t) (word7Bit << 0x9);
        const auto repeat = (uint16_t) (word7Bit & 0x3f);
        const auto mask = (uint16_t) (word7Bit <= 0x40 ? 0x0 : 0xffff);
        return (uint16_t) (shifted | (((repeat << 3) | (repeat >> 3)) & mask));
    }

    /** Widens a 14-bit MIDI 1.0 value to a 16-bit MIDI 2.0 value. */
    static uint16_t scaleTo16 (uint16_t word14Bit)
    {
        const auto shifted = (uint16_t) (word14Bit << 0x2);
        const auto repeat = (uint16_t) (word14Bit & 0x1fff);
        const auto mask = (uint16_t) (word14Bit <= 0x2000 ? 0x0 : 0xffff);
        return (uint16_t) (shifted | ((repeat >> 11) & mask));
    }

    /** Widens a 7-bit MIDI 1.0 value to a 32-bit MIDI 2.0 value. */
    static uint32_t scaleTo32 (uint8_t word7Bit)
    {
        const auto shifted = (uint32_t) (word7Bit << 0x19);
        const auto repeat = (uint32_t) (word7Bit & 0x3f);
        const auto mask = (uint32_t) (word7Bit <= 0x40 ? 0x0 : 0xffffffff);
        return (uint32_t) (shifted | (((repeat << 19)
                                     | (repeat << 13)
                                     | (repeat << 7)
                                     | (repeat << 1)
                                     | (repeat >> 5)) & mask));
    }

    /** Widens a 14-bit MIDI 1.0 value to a 32-bit MIDI 2.0 value. */
    static uint32_t scaleTo32 (uint16_t word14Bit)
    {
        const auto shifted = (uint32_t) (word14Bit << 0x12);
        const auto repeat = (uint32_t) (word14Bit & 0x1fff);
        const auto mask = (uint32_t) (word14Bit <= 0x2000 ? 0x0 : 0xffffffff);
        return (uint32_t) (shifted | (((repeat << 5) | (repeat >> 8)) & mask));
    }

    /** Narrows a 16-bit MIDI 2.0 value to a 7-bit MIDI 1.0 value. */
    static uint8_t scaleTo7 (uint8_t word8Bit) { return (uint8_t) (word8Bit >> 1); }

    /** Narrows a 16-bit MIDI 2.0 value to a 7-bit MIDI 1.0 value. */
    static uint8_t scaleTo7 (uint16_t word16Bit) { return (uint8_t) (word16Bit >> 9); }

    /** Narrows a 32-bit MIDI 2.0 value to a 7-bit MIDI 1.0 value. */
    static uint8_t scaleTo7 (uint32_t word32Bit) { return (uint8_t) (word32Bit >> 25); }

    /** Narrows a 32-bit MIDI 2.0 value to a 14-bit MIDI 1.0 value. */
    static uint16_t scaleTo14 (uint16_t word16Bit) { return (uint16_t) (word16Bit >> 2); }

    /** Narrows a 32-bit MIDI 2.0 value to a 14-bit MIDI 1.0 value. */
    static uint16_t scaleTo14 (uint32_t word32Bit) { return (uint16_t) (word32Bit >> 18); }

    /** Converts UMP messages which may include MIDI 2.0 channel voice messages into
        equivalent MIDI 1.0 messages (still in UMP format).

        `callback` is a function that accepts a single View argument and will be
        called with each converted packet.

        Note that not all MIDI 2.0 messages have MIDI 1.0 equivalents, so such
        messages will be ignored.
    */
    template <typename Callback>
    static void midi2ToMidi1DefaultTranslation (const View& v, Callback&& callback)
    {
        const auto firstWord = v[0];

        if (Utils::getMessageType (firstWord) != Utils::MessageKind::channelVoice2)
        {
            callback (v);
            return;
        }

        const auto status = Utils::getStatus (firstWord);
        const auto typeAndGroup = ((std::byte { 0x2 } << 0x4) | std::byte { Utils::getGroup (firstWord) });

        switch ((uint8_t) status)
        {
            case 0x8:   // note off
            case 0x9:   // note on
            case 0xa:   // poly pressure
            case 0xb:   // control change
            {
                const auto statusAndChannel = std::byte ((firstWord >> 0x10) & 0xff);
                const auto byte2 = std::byte ((firstWord >> 0x08) & 0xff);
                const auto byte3 = std::byte { scaleTo7 (v[1]) };

                // If this is a note-on, and the scaled byte is 0,
                // the scaled velocity should be 1 instead of 0
                const auto needsCorrection = status == std::byte { 0x9 } && byte3 == std::byte { 0 };
                const auto correctedByte = needsCorrection ? std::byte { 1 } : byte3;

                const auto shouldIgnore = status == std::byte { 0xb } && [&]
                {
                    switch (uint8_t (byte2))
                    {
                        case 0:
                        case 6:
                        case 32:
                        case 38:
                        case 98:
                        case 99:
                        case 100:
                        case 101:
                            return true;
                    }

                    return false;
                }();

                if (shouldIgnore)
                    return;

                const PacketX1 packet { Utils::bytesToWord (typeAndGroup,
                                                            statusAndChannel,
                                                            byte2,
                                                            correctedByte) };
                callback (View (packet.data()));
                return;
            }

            case 0xd: // channel pressure
            {
                const auto statusAndChannel = std::byte ((firstWord >> 0x10) & 0xff);
                const auto byte2 = std::byte { scaleTo7 (v[1]) };

                const PacketX1 packet { Utils::bytesToWord (typeAndGroup,
                                                            statusAndChannel,
                                                            byte2,
                                                            std::byte { 0 }) };
                callback (View (packet.data()));
                return;
            }

            case 0x2:   // rpn
            case 0x3:   // nrpn
            {
                const auto ccX = status == std::byte { 0x2 } ? std::byte { 101 } : std::byte { 99 };
                const auto ccY = status == std::byte { 0x2 } ? std::byte { 100 } : std::byte { 98 };
                const auto statusAndChannel = std::byte ((0xb << 0x4) | Utils::getChannel (firstWord));
                const auto data = scaleTo14 (v[1]);

                const PacketX1 packets[]
                {
                    PacketX1 { Utils::bytesToWord (typeAndGroup, statusAndChannel, ccX,              std::byte ((firstWord >> 0x8) & 0x7f)) },
                    PacketX1 { Utils::bytesToWord (typeAndGroup, statusAndChannel, ccY,              std::byte ((firstWord >> 0x0) & 0x7f)) },
                    PacketX1 { Utils::bytesToWord (typeAndGroup, statusAndChannel, std::byte { 6 },  std::byte ((data >> 0x7) & 0x7f)) },
                    PacketX1 { Utils::bytesToWord (typeAndGroup, statusAndChannel, std::byte { 38 }, std::byte ((data >> 0x0) & 0x7f)) },
                };

                for (const auto& packet : packets)
                    callback (View (packet.data()));

                return;
            }

            case 0xc: // program change / bank select
            {
                if (firstWord & 1)
                {
                    const auto statusAndChannel = std::byte ((0xb << 0x4) | Utils::getChannel (firstWord));
                    const auto secondWord = v[1];

                    const PacketX1 packets[]
                    {
                        PacketX1 { Utils::bytesToWord (typeAndGroup, statusAndChannel, std::byte { 0 },  std::byte ((secondWord >> 0x8) & 0x7f)) },
                        PacketX1 { Utils::bytesToWord (typeAndGroup, statusAndChannel, std::byte { 32 }, std::byte ((secondWord >> 0x0) & 0x7f)) },
                    };

                    for (const auto& packet : packets)
                        callback (View (packet.data()));
                }

                const auto statusAndChannel = std::byte ((0xc << 0x4) | Utils::getChannel (firstWord));
                const PacketX1 packet { Utils::bytesToWord (typeAndGroup,
                                                            statusAndChannel,
                                                            std::byte ((v[1] >> 0x18) & 0x7f),
                                                            std::byte { 0 }) };
                callback (View (packet.data()));
                return;
            }

            case 0xe: // pitch bend
            {
                const auto data = scaleTo14 (v[1]);
                const auto statusAndChannel = std::byte ((firstWord >> 0x10) & 0xff);
                const PacketX1 packet { Utils::bytesToWord (typeAndGroup,
                                                            statusAndChannel,
                                                            std::byte (data & 0x7f),
                                                            std::byte ((data >> 7) & 0x7f)) };
                callback (View (packet.data()));
                return;
            }

            default: // other message types do not translate
                return;
        }
    }
};

} // namespace juce::universal_midi_packets
/** @endcond */

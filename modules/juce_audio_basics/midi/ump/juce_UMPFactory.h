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
    Holds the data from a stream configuration notification message, with strong types.

    @tags{Audio}
*/
class StreamConfiguration
{
public:
    [[nodiscard]] StreamConfiguration withProtocol (PacketProtocol p) const { return withFlag (isMidi2, p == PacketProtocol::MIDI_2_0); }
    [[nodiscard]] StreamConfiguration withTransmitTimestamp (bool b)  const { return withFlag (transmitTimestamp, b); }
    [[nodiscard]] StreamConfiguration withReceiveTimestamp  (bool b)  const { return withFlag (receiveTimestamp,  b); }

    /** The protocol in use by the endpoint. This protocol will be used for sending and receiving messages. */
    [[nodiscard]] PacketProtocol getProtocol()                    const { return getFlag (isMidi2) ? PacketProtocol::MIDI_2_0 : PacketProtocol::MIDI_1_0; }
    /** True if this endpoint intends to send JR timestamps. */
    [[nodiscard]] bool getTransmitTimestamp()                     const { return getFlag (transmitTimestamp); }
    /** True if this endpoint expects to receive JR timestamps. */
    [[nodiscard]] bool getReceiveTimestamp()                      const { return getFlag (receiveTimestamp); }

    bool operator== (const StreamConfiguration& other) const { return options == other.options; }
    bool operator!= (const StreamConfiguration& other) const { return options != other.options; }

private:
    enum Flags
    {
        isMidi2                 = 1 << 0,
        transmitTimestamp       = 1 << 1,
        receiveTimestamp        = 1 << 2,
    };

    StreamConfiguration withFlag (Flags f, bool value) const
    {
        return withMember (*this, &StreamConfiguration::options, value ? (options | f) : (options & ~f));
    }

    bool getFlag (Flags f) const
    {
        return (options & f) != 0;
    }

    int options = 0;
};

/**
    Holds the data from an endpoint info notification message, with strong types.

    @tags{Audio}
*/
class EndpointInfo
{
    auto tie() const { return std::tie (versionMajor, versionMinor, numFunctionBlocks, flags); }

public:
    [[nodiscard]] EndpointInfo withVersion (uint8_t major, uint8_t minor) const
    {
        return withMember (withMember (*this, &EndpointInfo::versionMinor, minor), &EndpointInfo::versionMajor, major);
    }

    [[nodiscard]] EndpointInfo withNumFunctionBlocks (uint8_t x) const
    {
        return withMember (*this, &EndpointInfo::numFunctionBlocks, x);
    }

    [[nodiscard]] EndpointInfo withStaticFunctionBlocks  (bool b) const       { return withFlag (staticFunctionBlocks, b); }
    [[nodiscard]] EndpointInfo withMidi1Support          (bool b) const       { return withFlag (supportsMidi1, b); }
    [[nodiscard]] EndpointInfo withMidi2Support          (bool b) const       { return withFlag (supportsMidi2, b); }
    [[nodiscard]] EndpointInfo withReceiveJRSupport      (bool b) const       { return withFlag (supportsReceiveJR, b); }
    [[nodiscard]] EndpointInfo withTransmitJRSupport     (bool b) const       { return withFlag (supportsTransmitJR, b); }

    /** The major version byte. */
    [[nodiscard]] uint8_t getVersionMajor()       const { return versionMajor; }
    /** The minor version byte. */
    [[nodiscard]] uint8_t getVersionMinor()       const { return versionMinor; }
    /** The number of function blocks declared on this endpoint. */
    [[nodiscard]] uint8_t getNumFunctionBlocks()  const { return numFunctionBlocks; }
    /** True if the function block configuration cannot change. */
    [[nodiscard]] bool hasStaticFunctionBlocks()  const { return getFlag (staticFunctionBlocks); }
    /** True if this endpoint is capable of supporting the MIDI 1.0 protocol. */
    [[nodiscard]] bool hasMidi1Support()          const { return getFlag (supportsMidi1); }
    /** True if this endpoint is capable of supporting the MIDI 2.0 protocol. */
    [[nodiscard]] bool hasMidi2Support()          const { return getFlag (supportsMidi2); }
    /** True if this endpoint is capable of receiving JR timestamps. */
    [[nodiscard]] bool hasReceiveJRSupport()      const { return getFlag (supportsReceiveJR); }
    /** True if this endpoint is capable of transmitting JR timestamps. */
    [[nodiscard]] bool hasTransmitJRSupport()     const { return getFlag (supportsTransmitJR); }

    bool operator== (const EndpointInfo& other) const { return tie() == other.tie(); }
    bool operator!= (const EndpointInfo& other) const { return tie() != other.tie(); }

private:
    enum Flags
    {
        staticFunctionBlocks    = 1 << 0,
        supportsMidi1           = 1 << 1,
        supportsMidi2           = 1 << 2,
        supportsReceiveJR       = 1 << 3,
        supportsTransmitJR      = 1 << 4,
    };

    EndpointInfo withFlag (Flags f, bool value) const
    {
        return withMember (*this, &EndpointInfo::flags, (uint8_t) (value ? (flags | f) : (flags & ~f)));
    }

    bool getFlag (Flags f) const
    {
        return (flags & f) != 0;
    }

    uint8_t versionMajor, versionMinor, numFunctionBlocks, flags;
};

/** Directions that can apply to a Function Block or Group Terminal Block. */
enum class BlockDirection : uint8_t
{
    unknown       = 0b00, ///< Block direction is unknown or undeclared
    receiver      = 0b01, ///< Block is a receiver of messages
    sender        = 0b10, ///< Block is a sender of messages
    bidirectional = 0b11, ///< Block both sends and receives messages
};

/** UI hints that can apply to a Function Block or Group Terminal Block. */
enum class BlockUiHint : uint8_t
{
    unknown       = 0b00, ///< Block direction is unknown or undeclared
    receiver      = 0b01, ///< Block is a receiver of messages
    sender        = 0b10, ///< Block is a sender of messages
    bidirectional = 0b11, ///< Block both sends and receives messages
};

/** Describes how a MIDI 1.0 port maps to a given Block, if applicable. */
enum class BlockMIDI1ProxyKind : uint8_t
{
    inapplicable          = 0b00, ///< Block does not represent a MIDI 1.0 port
    unrestrictedBandwidth = 0b01, ///< Block represents a MIDI 1.0 port and can handle high bandwidth
    restrictedBandwidth   = 0b10, ///< Block represents a MIDI 1.0 port that requires restricted bandwidth
};

/**
    Holds the data from a function block info notification message, with strong types.

    @tags{Audio}
*/
class BlockInfo
{
public:
    [[nodiscard]] BlockInfo withEnabled (bool x) const { return withMember (*this, &BlockInfo::enabled, x); }
    [[nodiscard]] BlockInfo withUiHint (BlockUiHint x) const { return withMember (*this, &BlockInfo::flags, replaceBits<4, 2> (flags, (uint8_t) x)); }
    [[nodiscard]] BlockInfo withMIDI1ProxyKind (BlockMIDI1ProxyKind x) const { return withMember (*this, &BlockInfo::flags, replaceBits<2, 2> (flags, (uint8_t) x)); }
    [[nodiscard]] BlockInfo withDirection (BlockDirection x) const { return withMember (*this, &BlockInfo::flags, replaceBits<0, 2> (flags, (uint8_t) x)); }
    [[nodiscard]] BlockInfo withFirstGroup (uint8_t x) const { return withMember (*this, &BlockInfo::firstGroup, x); }
    [[nodiscard]] BlockInfo withNumGroups (uint8_t x) const { return withMember (*this, &BlockInfo::numGroups, x); }
    [[nodiscard]] BlockInfo withCiVersion (uint8_t x) const { return withMember (*this, &BlockInfo::ciVersion, x); }
    [[nodiscard]] BlockInfo withMaxSysex8Streams (uint8_t x) const { return withMember (*this, &BlockInfo::numSysex8Streams, x); }

    /** True if the block is enabled/active, false otherwise. */
    bool isEnabled() const { return enabled; }
    /** The directionality of the block, for display to the user. */
    BlockUiHint getUiHint() const { return (BlockUiHint) getBits<4, 2> (flags); }
    /** The kind of MIDI 1.0 proxy represented by this block, if any. */
    BlockMIDI1ProxyKind getMIDI1ProxyKind() const { return (BlockMIDI1ProxyKind) getBits<2, 2> (flags); }
    /** The actual directionality of the block. */
    BlockDirection getDirection() const { return (BlockDirection) getBits<0, 2> (flags); }
    /** The zero-based index of the first group in the block. */
    uint8_t getFirstGroup() const { return firstGroup; }
    /** The number of groups contained in the block, must be one or greater. */
    uint8_t getNumGroups() const { return numGroups; }
    /** The CI version supported by this block. Implies a bidirectional block. */
    uint8_t getCiVersion() const { return ciVersion; }
    /** The number of simultaneous SysEx8 streams supported on this block. */
    uint8_t getMaxSysex8Streams() const { return numSysex8Streams; }

    bool operator== (const BlockInfo& other) const
    {
        const auto tie = [] (auto& x)
        {
            return std::tuple (x.enabled, x.flags, x.firstGroup, x.numGroups, x.ciVersion, x.numSysex8Streams);
        };
        return tie (*this) == tie (other);
    }

    bool operator!= (const BlockInfo& other) const
    {
        return ! operator== (other);
    }

private:
    template <auto position, auto numBits, typename Value>
    static Value replaceBits (Value value, Value replacement)
    {
        constexpr auto mask = ((Value) 1 << numBits) - 1;
        const auto maskedValue = value & ~(mask << position);
        return (Value) (maskedValue | (replacement << position));
    }

    template <auto position, auto numBits, typename Value>
    static Value getBits (Value value)
    {
        constexpr auto mask = ((Value) 1 << numBits) - 1;
        return (value >> position) & mask;
    }

    uint8_t enabled{};
    uint8_t flags{};
    uint8_t firstGroup{};
    uint8_t numGroups{};
    uint8_t ciVersion{};
    uint8_t numSysex8Streams{};
};

/**
    This struct holds functions that can be used to create different kinds
    of Universal MIDI Packet.

    @tags{Audio}
*/
struct Factory
{
    template <typename Callback>
    static void splitIntoPackets (Span<const std::byte> bytes, size_t bytesPerPacket, Callback&& callback)
    {
        const auto numPackets = (bytes.size() / bytesPerPacket) + ((bytes.size() % bytesPerPacket) != 0);
        auto* dataOffset = bytes.data();

        if (numPackets <= 1)
        {
            callback (SysEx7::Kind::complete, bytes);
            return;
        }

        for (auto i = static_cast<ssize_t> (bytes.size()); i > 0; i -= (ssize_t) bytesPerPacket, dataOffset += bytesPerPacket)
        {
            const auto kind = [&]
            {
                if (i == (ssize_t) bytes.size())
                    return SysEx7::Kind::begin;

                if (i <= (ssize_t) bytesPerPacket)
                    return SysEx7::Kind::end;

                return SysEx7::Kind::continuation;
            }();

            const auto bytesNow = std::min ((ssize_t) bytesPerPacket, i);
            callback (kind, Span (dataOffset, (size_t) bytesNow));
        }
    }

    /** @internal */
    struct Detail
    {
        static PacketX1 makeSystem()   { return PacketX1{}.withMessageType (Utils::MessageKind::commonRealtime); }
        static PacketX1 makeV1()       { return PacketX1{}.withMessageType (Utils::MessageKind::channelVoice1); }
        static PacketX2 makeV2()       { return PacketX2{}.withMessageType (Utils::MessageKind::channelVoice2); }
        static PacketX4 makeStream()   { return PacketX4{}.withMessageType (Utils::MessageKind::stream); }

        static PacketX2 makeSysEx (uint8_t group, SysEx7::Kind status, Span<const std::byte> data)
        {
            jassert (data.size() <= 6);

            std::array<std::byte, 8> bytes { {} };
            bytes[0] = std::byte (0x3 << 0x4) | std::byte (group);
            bytes[1] = std::byte ((uint8_t) status << 0x4) | std::byte (data.size());

            std::memcpy (bytes.data() + 2, data.data(), data.size());

            std::array<uint32_t, 2> words;

            for (const auto [index, word] : enumerate (words))
                word = ByteOrder::bigEndianInt (bytes.data() + 4 * index);

            return PacketX2 { words };
        }

        static PacketX4 makeSysEx8 (uint8_t group,
                                    std::byte status,
                                    uint8_t dataStart,
                                    Span<const std::byte> data)
        {
            jassert (data.size() <= (size_t) (16 - dataStart));

            std::array<std::byte, 16> bytes{{}};
            bytes[0] = std::byte (0x5 << 0x4) | std::byte (group);
            bytes[1] = std::byte (status << 0x4) | std::byte (data.size());

            std::memcpy (bytes.data() + dataStart, data.data(), data.size());

            std::array<uint32_t, 4> words;

            for (const auto [index, word] : enumerate (words))
                word = ByteOrder::bigEndianInt (bytes.data() + 4 * index);

            return PacketX4 { words };
        }

        static PacketX4 makePacketX4 (Span<const std::byte> header,
                                      Span<const std::byte> data)
        {
            jassert (data.size() <= 14);

            std::array<std::byte, 16> bytes{{}};
            std::copy (header.begin(), header.end(), bytes.begin());
            std::copy (data.begin(), data.end(), std::next (bytes.begin(), (ptrdiff_t) header.size()));

            std::array<uint32_t, 4> words{};

            size_t index = 0;

            for (auto& word : words)
                word = ByteOrder::bigEndianInt (bytes.data() + 4 * index++);

            return PacketX4 { words };
        }

        static PacketX4 makeStreamSubpacket (std::byte status,
                                             SysEx7::Kind kind,
                                             Span<const std::byte> data)
        {
            jassert (data.size() <= 14);
            const std::byte header[] { std::byte (0xf0) | std::byte ((uint8_t) kind << 2), status, };
            return makePacketX4 (header, data);
        }

        static PacketX4 makeStreamConfiguration (StreamConfiguration options)
        {
            return Detail::makeStream().withU8<0x2> (options.getProtocol() == PacketProtocol::MIDI_2_0 ? 0x2 : 0x1)
                                       .withU8<0x3> ((options.getReceiveTimestamp() ? 0x2 : 0x0) | (options.getTransmitTimestamp() ? 0x1 : 0x0));
        }

    };

    static PacketX1 makeNoop (uint8_t group)
    {
        return PacketX1{}.withGroup (group);
    }

    static PacketX1 makeJRClock (uint8_t group, uint16_t time)
    {
        return PacketX1 { time }.withStatus (std::byte { 1 }).withGroup (group);
    }

    static PacketX1 makeJRTimestamp (uint8_t group, uint16_t time)
    {
        return PacketX1 { time }.withStatus (std::byte { 2 }).withGroup (group);
    }

    static PacketX1 makeTimeCode (uint8_t group, uint8_t code)
    {
        return Detail::makeSystem().withGroup (group)
                                   .withU8<1> (0xf1)
                                   .withU8<2> (code & 0x7f);
    }

    static PacketX1 makeSongPositionPointer (uint8_t group, uint16_t pos)
    {
        return Detail::makeSystem().withGroup (group)
                                   .withU8<1> (0xf2)
                                   .withU8<2> (pos & 0x7f)
                                   .withU8<3> ((pos >> 7) & 0x7f);
    }

    static PacketX1 makeSongSelect (uint8_t group, uint8_t song)
    {
        return Detail::makeSystem().withGroup (group)
                                   .withU8<1> (0xf3)
                                   .withU8<2> (song & 0x7f);
    }

    static PacketX1 makeTuneRequest (uint8_t group)
    {
        return Detail::makeSystem().withGroup (group)
                                   .withU8<1> (0xf6);
    }

    static PacketX1 makeTimingClock (uint8_t group)
    {
        return Detail::makeSystem().withGroup (group)
                                   .withU8<1> (0xf8);
    }

    static PacketX1 makeStart (uint8_t group)
    {
        return Detail::makeSystem().withGroup (group)
                                   .withU8<1> (0xfa);
    }

    static PacketX1 makeContinue (uint8_t group)
    {
        return Detail::makeSystem().withGroup (group)
                                   .withU8<1> (0xfb);
    }

    static PacketX1 makeStop (uint8_t group)
    {
        return Detail::makeSystem().withGroup (group)
                                   .withU8<1> (0xfc);
    }

    static PacketX1 makeActiveSensing (uint8_t group)
    {
        return Detail::makeSystem().withGroup (group)
                                   .withU8<1> (0xfe);
    }

    static PacketX1 makeReset (uint8_t group)
    {
        return Detail::makeSystem().withGroup (group)
                                   .withU8<1> (0xff);
    }

    static PacketX1 makeNoteOffV1 (uint8_t group,
                                   uint8_t channel,
                                   uint8_t note,
                                   uint8_t velocity)
    {
        return Detail::makeV1().withGroup (group)
                               .withStatus (std::byte { 0x8 })
                               .withChannel (channel)
                               .withU8<2> (note & 0x7f)
                               .withU8<3> (velocity & 0x7f);
    }

    static PacketX1 makeNoteOnV1 (uint8_t group,
                                  uint8_t channel,
                                  uint8_t note,
                                  uint8_t velocity)
    {
        return Detail::makeV1().withGroup (group)
                               .withStatus (std::byte { 0x9 })
                               .withChannel (channel)
                               .withU8<2> (note & 0x7f)
                               .withU8<3> (velocity & 0x7f);
    }

    static PacketX1 makePolyPressureV1 (uint8_t group,
                                        uint8_t channel,
                                        uint8_t note,
                                        uint8_t pressure)
    {
        return Detail::makeV1().withGroup (group)
                               .withStatus (std::byte { 0xa })
                               .withChannel (channel)
                               .withU8<2> (note & 0x7f)
                               .withU8<3> (pressure & 0x7f);
    }

    static PacketX1 makeControlChangeV1 (uint8_t group,
                                         uint8_t channel,
                                         uint8_t controller,
                                         uint8_t value)
    {
        return Detail::makeV1().withGroup (group)
                               .withStatus (std::byte { 0xb })
                               .withChannel (channel)
                               .withU8<2> (controller & 0x7f)
                               .withU8<3> (value & 0x7f);
    }

    static PacketX1 makeProgramChangeV1 (uint8_t group,
                                         uint8_t channel,
                                         uint8_t program)
    {
        return Detail::makeV1().withGroup (group)
                               .withStatus (std::byte { 0xc })
                               .withChannel (channel)
                               .withU8<2> (program & 0x7f);
    }

    static PacketX1 makeChannelPressureV1 (uint8_t group,
                                           uint8_t channel,
                                           uint8_t pressure)
    {
        return Detail::makeV1().withGroup (group)
                               .withStatus (std::byte { 0xd })
                               .withChannel (channel)
                               .withU8<2> (pressure & 0x7f);
    }

    static PacketX1 makePitchBend (uint8_t group,
                                   uint8_t channel,
                                   uint16_t pitchbend)
    {
        return Detail::makeV1().withGroup (group)
                               .withStatus (std::byte { 0xe })
                               .withChannel (channel)
                               .withU8<2> (pitchbend & 0x7f)
                               .withU8<3> ((pitchbend >> 7) & 0x7f);
    }

    static PacketX2 makeSysExIn1Packet (uint8_t group,
                                        Span<const std::byte> data)
    {
        return Detail::makeSysEx (group, SysEx7::Kind::complete, data);
    }

    static PacketX2 makeSysExStart (uint8_t group,
                                    Span<const std::byte> data)
    {
        return Detail::makeSysEx (group, SysEx7::Kind::begin, data);
    }

    static PacketX2 makeSysExContinue (uint8_t group,
                                       Span<const std::byte> data)
    {
        return Detail::makeSysEx (group, SysEx7::Kind::continuation, data);
    }

    static PacketX2 makeSysExEnd (uint8_t group,
                                  Span<const std::byte> data)
    {
        return Detail::makeSysEx (group, SysEx7::Kind::end, data);
    }

    static PacketX2 makeRegisteredPerNoteControllerV2 (uint8_t group,
                                                       uint8_t channel,
                                                       uint8_t note,
                                                       uint8_t controller,
                                                       uint32_t data)
    {
        return Detail::makeV2().withGroup (group)
                               .withStatus (std::byte { 0x0 })
                               .withChannel (channel)
                               .withU8<2> (note & 0x7f)
                               .withU8<3> (controller & 0x7f)
                               .withU32<1> (data);
    }

    static PacketX2 makeAssignablePerNoteControllerV2 (uint8_t group,
                                                       uint8_t channel,
                                                       uint8_t note,
                                                       uint8_t controller,
                                                       uint32_t data)
    {
        return Detail::makeV2().withGroup (group)
                               .withStatus (std::byte { 0x1 })
                               .withChannel (channel)
                               .withU8<2> (note & 0x7f)
                               .withU8<3> (controller & 0x7f)
                               .withU32<1> (data);
    }

    static PacketX2 makeRegisteredControllerV2 (uint8_t group,
                                                uint8_t channel,
                                                uint8_t bank,
                                                uint8_t index,
                                                uint32_t data)
    {
        return Detail::makeV2().withGroup (group)
                               .withStatus (std::byte { 0x2 })
                               .withChannel (channel)
                               .withU8<2> (bank & 0x7f)
                               .withU8<3> (index & 0x7f)
                               .withU32<1> (data);
    }

    static PacketX2 makeAssignableControllerV2 (uint8_t group,
                                                uint8_t channel,
                                                uint8_t bank,
                                                uint8_t index,
                                                uint32_t data)
    {
        return Detail::makeV2().withGroup (group)
                               .withStatus (std::byte { 0x3 })
                               .withChannel (channel)
                               .withU8<2> (bank & 0x7f)
                               .withU8<3> (index & 0x7f)
                               .withU32<1> (data);
    }

    static PacketX2 makeRelativeRegisteredControllerV2 (uint8_t group,
                                                        uint8_t channel,
                                                        uint8_t bank,
                                                        uint8_t index,
                                                        uint32_t data)
    {
        return Detail::makeV2().withGroup (group)
                               .withStatus (std::byte { 0x4 })
                               .withChannel (channel)
                               .withU8<2> (bank & 0x7f)
                               .withU8<3> (index & 0x7f)
                               .withU32<1> (data);
    }

    static PacketX2 makeRelativeAssignableControllerV2 (uint8_t group,
                                                        uint8_t channel,
                                                        uint8_t bank,
                                                        uint8_t index,
                                                        uint32_t data)
    {
        return Detail::makeV2().withGroup (group)
                               .withStatus (std::byte { 0x5 })
                               .withChannel (channel)
                               .withU8<2> (bank & 0x7f)
                               .withU8<3> (index & 0x7f)
                               .withU32<1> (data);
    }

    static PacketX2 makePerNotePitchBendV2 (uint8_t group,
                                            uint8_t channel,
                                            uint8_t note,
                                            uint32_t data)
    {
        return Detail::makeV2().withGroup (group)
                               .withStatus (std::byte { 0x6 })
                               .withChannel (channel)
                               .withU8<2> (note & 0x7f)
                               .withU32<1> (data);
    }

    enum class NoteAttributeKind : uint8_t
    {
        none            = 0x00,
        manufacturer    = 0x01,
        profile         = 0x02,
        pitch7_9        = 0x03
    };

    static PacketX2 makeNoteOffV2 (uint8_t group,
                                   uint8_t channel,
                                   uint8_t note,
                                   NoteAttributeKind attribute,
                                   uint16_t velocity,
                                   uint16_t attributeValue)
    {
        return Detail::makeV2().withGroup (group)
                               .withStatus (std::byte { 0x8 })
                               .withChannel (channel)
                               .withU8<2> (note & 0x7f)
                               .withU8<3> ((uint8_t) attribute)
                               .withU16<2> (velocity)
                               .withU16<3> (attributeValue);
    }

    static PacketX2 makeNoteOnV2 (uint8_t group,
                                  uint8_t channel,
                                  uint8_t note,
                                  NoteAttributeKind attribute,
                                  uint16_t velocity,
                                  uint16_t attributeValue)
    {
        return Detail::makeV2().withGroup (group)
                               .withStatus (std::byte { 0x9 })
                               .withChannel (channel)
                               .withU8<2> (note & 0x7f)
                               .withU8<3> ((uint8_t) attribute)
                               .withU16<2> (velocity)
                               .withU16<3> (attributeValue);
    }

    static PacketX2 makePolyPressureV2 (uint8_t group,
                                        uint8_t channel,
                                        uint8_t note,
                                        uint32_t data)
    {
        return Detail::makeV2().withGroup (group)
                               .withStatus (std::byte { 0xa })
                               .withChannel (channel)
                               .withU8<2> (note & 0x7f)
                               .withU32<1> (data);
    }

    static PacketX2 makeControlChangeV2 (uint8_t group,
                                         uint8_t channel,
                                         uint8_t controller,
                                         uint32_t data)
    {
        return Detail::makeV2().withGroup (group)
                               .withStatus (std::byte { 0xb })
                               .withChannel (channel)
                               .withU8<2> (controller & 0x7f)
                               .withU32<1> (data);
    }

    static PacketX2 makeProgramChangeV2 (uint8_t group,
                                         uint8_t channel,
                                         uint8_t optionFlags,
                                         uint8_t program,
                                         uint8_t bankMsb,
                                         uint8_t bankLsb)
    {
        return Detail::makeV2().withGroup (group)
                               .withStatus (std::byte { 0xc })
                               .withChannel (channel)
                               .withU8<3> (optionFlags)
                               .withU8<4> (program)
                               .withU8<6> (bankMsb)
                               .withU8<7> (bankLsb);
    }

    static PacketX2 makeChannelPressureV2 (uint8_t group,
                                           uint8_t channel,
                                           uint32_t data)
    {
        return Detail::makeV2().withGroup (group)
                               .withStatus (std::byte { 0xd })
                               .withChannel (channel)
                               .withU32<1> (data);
    }

    static PacketX2 makePitchBendV2 (uint8_t group,
                                     uint8_t channel,
                                     uint32_t data)
    {
        return Detail::makeV2().withGroup (group)
                               .withStatus (std::byte { 0xe })
                               .withChannel (channel)
                               .withU32<1> (data);
    }

    static PacketX2 makePerNoteManagementV2 (uint8_t group,
                                             uint8_t channel,
                                             uint8_t note,
                                             std::byte optionFlags)
    {
        return Detail::makeV2().withGroup (group)
                               .withStatus (std::byte { 0xf })
                               .withChannel (channel)
                               .withU8<2> (note)
                               .withU8<3> (uint8_t (optionFlags));
    }


    static PacketX4 makeSysEx8in1Packet (uint8_t group,
                                         uint8_t streamId,
                                         Span<const std::byte> data)
    {
        return Detail::makeSysEx8 (group, std::byte { 0x0 }, 3, data).withU8<2> (streamId);
    }

    static PacketX4 makeSysEx8Start (uint8_t group,
                                     uint8_t streamId,
                                     Span<const std::byte> data)
    {
        return Detail::makeSysEx8 (group, std::byte { 0x1 }, 3, data).withU8<2> (streamId);
    }

    static PacketX4 makeSysEx8Continue (uint8_t group,
                                        uint8_t streamId,
                                        Span<const std::byte> data)
    {
        return Detail::makeSysEx8 (group, std::byte { 0x2 }, 3, data).withU8<2> (streamId);
    }

    static PacketX4 makeSysEx8End (uint8_t group,
                                   uint8_t streamId,
                                   Span<const std::byte> data)
    {
        return Detail::makeSysEx8 (group, std::byte { 0x3 }, 3, data).withU8<2> (streamId);
    }

    static PacketX4 makeEndpointDiscovery (uint8_t versionMajor,
                                           uint8_t versionMinor,
                                           std::byte filterBitmap)
    {
        return Detail::makeStream().withU8<2> (versionMajor)
                                   .withU8<3> (versionMinor)
                                   .withU8<7> ((uint8_t) filterBitmap);
    }

    static PacketX4 makeEndpointInfoNotification (const EndpointInfo& info)
    {
        return Detail::makeStream().withU8<1> (1)
                                   .withU8<2> (info.getVersionMajor())
                                   .withU8<3> (info.getVersionMinor())
                                   .withU8<4> (info.getNumFunctionBlocks() | (info.hasStaticFunctionBlocks() ? 0x80 : 0x00))
                                   .withU8<6> ((info.hasMidi1Support() ? 0x1 : 0x0) | (info.hasMidi2Support() ? 0x2 : 0x0))
                                   .withU8<7> ((info.hasTransmitJRSupport() ? 0x1 : 0x0) | (info.hasReceiveJRSupport() ? 0x2 : 0x0));
    }

    static PacketX4 makeFunctionBlockDiscovery (uint8_t block, std::byte filterBitmap)
    {
        return Detail::makeStream().withU8<1> (0x10)
                                   .withU8<2> (block)
                                   .withU8<3> ((uint8_t) filterBitmap);
    }

    static PacketX4 makeDeviceIdentityNotification (DeviceInfo info)
    {
        return Detail::makeStream().withU8<0x1> (2)
                                   .withU8<0x5> ((uint8_t) info.manufacturer[0])
                                   .withU8<0x6> ((uint8_t) info.manufacturer[1])
                                   .withU8<0x7> ((uint8_t) info.manufacturer[2])
                                   .withU8<0x8> ((uint8_t) info.family[0])
                                   .withU8<0x9> ((uint8_t) info.family[1])
                                   .withU8<0xa> ((uint8_t) info.modelNumber[0])
                                   .withU8<0xb> ((uint8_t) info.modelNumber[1])
                                   .withU8<0xc> ((uint8_t) info.revision[0])
                                   .withU8<0xd> ((uint8_t) info.revision[1])
                                   .withU8<0xe> ((uint8_t) info.revision[2])
                                   .withU8<0xf> ((uint8_t) info.revision[3]);
    }

    template <typename Fn>
    static bool makeEndpointNameNotification (const String& bytes, Fn&& fn)
    {
        constexpr auto maxSize = 98;

        if (maxSize <= bytes.getNumBytesAsUTF8())
            return false;

        const Span byteSpan { reinterpret_cast<const std::byte*> (bytes.toRawUTF8()), bytes.getNumBytesAsUTF8() };

        splitIntoPackets (byteSpan, 14, [&] (SysEx7::Kind kind, Span<const std::byte> bytesThisTime)
        {
            const auto packet = Detail::makeStreamSubpacket (std::byte (3), kind, bytesThisTime);
            fn (View (packet.data()));
        });

        return true;
    }

    template <typename Fn>
    static bool makeProductInstanceIdNotification (const String& bytes, Fn&& fn)
    {
        constexpr auto maxSize = 42;

        if (maxSize < bytes.getNumBytesAsUTF8())
            return false;

        const Span byteSpan { reinterpret_cast<const std::byte*> (bytes.toRawUTF8()), bytes.getNumBytesAsUTF8() };

        splitIntoPackets (byteSpan, 14, [&] (SysEx7::Kind kind, Span<const std::byte> bytesThisTime)
        {
            const auto packet = Detail::makeStreamSubpacket (std::byte (4), kind, bytesThisTime);
            fn (View (packet.data()));
        });

        return true;
    }

    template <typename Fn>
    static bool makeFunctionBlockNameNotification (uint8_t index, const String& bytes, Fn&& fn)
    {
        constexpr auto maxSize = 91;

        if (maxSize < bytes.getNumBytesAsUTF8())
            return false;

        const Span byteSpan { reinterpret_cast<const std::byte*> (bytes.toRawUTF8()), bytes.getNumBytesAsUTF8() };

        splitIntoPackets (byteSpan, 13, [&] (SysEx7::Kind kind, Span<const std::byte> bytesThisTime)
        {
            const std::byte header[] { std::byte (0xf0) | std::byte ((uint8_t) kind << 2), std::byte (0x12), std::byte (index) };
            fn (View (Detail::makePacketX4 (header, bytesThisTime).data()));
        });

        return true;
    }

    static PacketX4 makeFunctionBlockInfoNotification (uint8_t index, const BlockInfo& info)
    {
        const auto flags = ((uint8_t) info.getDirection() << 0)
                         | ((uint8_t) info.getMIDI1ProxyKind() << 2)
                         | ((uint8_t) info.getUiHint() << 4);
        return Detail::makeStream().withU8<0x1> (0x11)
                                   .withU8<0x2> ((uint8_t) (index | (info.isEnabled() << 7)))
                                   .withU8<0x3> ((uint8_t) flags)
                                   .withU8<0x4> (info.getFirstGroup())
                                   .withU8<0x5> (info.getNumGroups())
                                   .withU8<0x6> (info.getCiVersion())
                                   .withU8<0x7> (info.getMaxSysex8Streams());
    }

    static PacketX4 makeStreamConfigurationRequest (StreamConfiguration options)
    {
        return Detail::makeStreamConfiguration (options).withU8<0x1> (5);
    }

    static PacketX4 makeStreamConfigurationNotification (StreamConfiguration options)
    {
        return Detail::makeStreamConfiguration (options).withU8<0x1> (6);
    }
};

} // namespace juce::universal_midi_packets
/** @endcond */

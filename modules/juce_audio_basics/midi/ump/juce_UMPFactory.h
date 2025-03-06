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

#ifndef DOXYGEN

namespace juce::universal_midi_packets
{

/**
    This struct holds functions that can be used to create different kinds
    of Universal MIDI Packet.

    @tags{Audio}
*/
struct Factory
{
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
};

} // namespace juce::universal_midi_packets

#endif

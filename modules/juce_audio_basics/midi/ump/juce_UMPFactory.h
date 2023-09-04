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
    This struct holds functions that can be used to create different kinds
    of Universal MIDI Packet.

    @tags{Audio}
*/
struct Factory
{
    /** @internal */
    struct Detail
    {
        static PacketX1 makeSystem()   { return PacketX1{}.withMessageType (1); }
        static PacketX1 makeV1()       { return PacketX1{}.withMessageType (2); }
        static PacketX2 makeV2()       { return PacketX2{}.withMessageType (4); }

        static PacketX2 makeSysEx (uint8_t group,
                                   uint8_t status,
                                   uint8_t numBytes,
                                   const std::byte* data)
        {
            jassert (numBytes <= 6);

            std::array<uint8_t, 8> bytes{{}};
            bytes[0] = (0x3 << 0x4) | group;
            bytes[1] = (uint8_t) (status << 0x4) | numBytes;

            std::memcpy (bytes.data() + 2, data, numBytes);

            std::array<uint32_t, 2> words;

            size_t index = 0;

            for (auto& word : words)
                word = ByteOrder::bigEndianInt (bytes.data() + 4 * index++);

            return PacketX2 { words };
        }

        static PacketX4 makeSysEx8 (uint8_t group,
                                    uint8_t status,
                                    uint8_t numBytes,
                                    uint8_t dataStart,
                                    const uint8_t* data)
        {
            jassert (numBytes <= 16 - dataStart);

            std::array<uint8_t, 16> bytes{{}};
            bytes[0] = (0x5 << 0x4) | group;
            bytes[1] = (uint8_t) (status << 0x4) | numBytes;

            std::memcpy (bytes.data() + dataStart, data, numBytes);

            std::array<uint32_t, 4> words;

            size_t index = 0;

            for (auto& word : words)
                word = ByteOrder::bigEndianInt (bytes.data() + 4 * index++);

            return PacketX4 { words };
        }
    };

    static PacketX1 makeNoop (uint8_t group)
    {
        return PacketX1{}.withGroup (group);
    }

    static PacketX1 makeJRClock (uint8_t group, uint16_t time)
    {
        return PacketX1 { time }.withStatus (1).withGroup (group);
    }

    static PacketX1 makeJRTimestamp (uint8_t group, uint16_t time)
    {
        return PacketX1 { time }.withStatus (2).withGroup (group);
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
                               .withStatus (0x8)
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
                               .withStatus (0x9)
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
                               .withStatus (0xa)
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
                               .withStatus (0xb)
                               .withChannel (channel)
                               .withU8<2> (controller & 0x7f)
                               .withU8<3> (value & 0x7f);
    }

    static PacketX1 makeProgramChangeV1 (uint8_t group,
                                         uint8_t channel,
                                         uint8_t program)
    {
        return Detail::makeV1().withGroup (group)
                               .withStatus (0xc)
                               .withChannel (channel)
                               .withU8<2> (program & 0x7f);
    }

    static PacketX1 makeChannelPressureV1 (uint8_t group,
                                           uint8_t channel,
                                           uint8_t pressure)
    {
        return Detail::makeV1().withGroup (group)
                               .withStatus (0xd)
                               .withChannel (channel)
                               .withU8<2> (pressure & 0x7f);
    }

    static PacketX1 makePitchBend (uint8_t group,
                                   uint8_t channel,
                                   uint16_t pitchbend)
    {
        return Detail::makeV1().withGroup (group)
                               .withStatus (0xe)
                               .withChannel (channel)
                               .withU8<2> (pitchbend & 0x7f)
                               .withU8<3> ((pitchbend >> 7) & 0x7f);
    }

    static PacketX2 makeSysExIn1Packet (uint8_t group,
                                        uint8_t numBytes,
                                        const std::byte* data)
    {
        return Detail::makeSysEx (group, 0x0, numBytes, data);
    }

    static PacketX2 makeSysExStart (uint8_t group,
                                    uint8_t numBytes,
                                    const std::byte* data)
    {
        return Detail::makeSysEx (group, 0x1, numBytes, data);
    }

    static PacketX2 makeSysExContinue (uint8_t group,
                                       uint8_t numBytes,
                                       const std::byte* data)
    {
        return Detail::makeSysEx (group, 0x2, numBytes, data);
    }

    static PacketX2 makeSysExEnd (uint8_t group,
                                  uint8_t numBytes,
                                  const std::byte* data)
    {
        return Detail::makeSysEx (group, 0x3, numBytes, data);
    }

    static PacketX2 makeRegisteredPerNoteControllerV2 (uint8_t group,
                                                       uint8_t channel,
                                                       uint8_t note,
                                                       uint8_t controller,
                                                       uint32_t data)
    {
        return Detail::makeV2().withGroup (group)
                               .withStatus (0x0)
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
                               .withStatus (0x1)
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
                               .withStatus (0x2)
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
                               .withStatus (0x3)
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
                               .withStatus (0x4)
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
                               .withStatus (0x5)
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
                               .withStatus (0x6)
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
                               .withStatus (0x8)
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
                               .withStatus (0x9)
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
                               .withStatus (0xa)
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
                               .withStatus (0xb)
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
                               .withStatus (0xc)
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
                               .withStatus (0xd)
                               .withChannel (channel)
                               .withU32<1> (data);
    }

    static PacketX2 makePitchBendV2 (uint8_t group,
                                     uint8_t channel,
                                     uint32_t data)
    {
        return Detail::makeV2().withGroup (group)
                               .withStatus (0xe)
                               .withChannel (channel)
                               .withU32<1> (data);
    }

    static PacketX2 makePerNoteManagementV2 (uint8_t group,
                                             uint8_t channel,
                                             uint8_t note,
                                             uint8_t optionFlags)
    {
        return Detail::makeV2().withGroup (group)
                               .withStatus (0xf)
                               .withChannel (channel)
                               .withU8<2> (note)
                               .withU8<3> (optionFlags);
    }


    static PacketX4 makeSysEx8in1Packet (uint8_t group,
                                         uint8_t numBytes,
                                         uint8_t streamId,
                                         const uint8_t* data)
    {
        return Detail::makeSysEx8 (group, 0x0, numBytes, 3, data).withU8<2> (streamId);
    }

    static PacketX4 makeSysEx8Start (uint8_t group,
                                     uint8_t numBytes,
                                     uint8_t streamId,
                                     const uint8_t* data)
    {
        return Detail::makeSysEx8 (group, 0x1, numBytes, 3, data).withU8<2> (streamId);
    }

    static PacketX4 makeSysEx8Continue (uint8_t group,
                                        uint8_t numBytes,
                                        uint8_t streamId,
                                        const uint8_t* data)
    {
        return Detail::makeSysEx8 (group, 0x2, numBytes, 3, data).withU8<2> (streamId);
    }

    static PacketX4 makeSysEx8End (uint8_t group,
                                   uint8_t numBytes,
                                   uint8_t streamId,
                                   const uint8_t* data)
    {
        return Detail::makeSysEx8 (group, 0x3, numBytes, 3, data).withU8<2> (streamId);
    }

    static PacketX4 makeMixedDataSetHeader (uint8_t group,
                                            uint8_t dataSetId,
                                            const uint8_t* data)
    {
        return Detail::makeSysEx8 (group, 0x8, 14, 2, data).withChannel (dataSetId);
    }

    static PacketX4 makeDataSetPayload (uint8_t group,
                                        uint8_t dataSetId,
                                        const uint8_t* data)
    {
        return Detail::makeSysEx8 (group, 0x9, 14, 2, data).withChannel (dataSetId);
    }
};

}
}

#endif

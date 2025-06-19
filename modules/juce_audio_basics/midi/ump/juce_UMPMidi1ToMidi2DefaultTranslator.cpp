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

namespace juce::universal_midi_packets
{

PacketX2 Midi1ToMidi2DefaultTranslator::processNoteOnOrOff (const HelperValues helpers)
{
    const auto velocity = helpers.byte2;
    const auto needsConversion = (helpers.byte0 & std::byte { 0xf0 }) == std::byte { 0x90 } && velocity == std::byte { 0 };
    const auto firstByte = needsConversion ? (std::byte { 0x80 } | (helpers.byte0 & std::byte { 0xf }))
                                           : helpers.byte0;

    return PacketX2
    {
        Utils::bytesToWord (helpers.typeAndGroup, firstByte, helpers.byte1, std::byte { 0 }),
        (uint32_t) (Conversion::scaleTo16 (uint8_t (velocity)) << 0x10)
    };
}

PacketX2 Midi1ToMidi2DefaultTranslator::processPolyPressure (const HelperValues helpers)
{
    return PacketX2
    {
        Utils::bytesToWord (helpers.typeAndGroup, helpers.byte0, helpers.byte1, std::byte { 0 }),
        Conversion::scaleTo32 (uint8_t (helpers.byte2))
    };
}

bool Midi1ToMidi2DefaultTranslator::processControlChange (const HelperValues helpers,
                                                          PacketX2& packet)
{
    const auto statusAndChannel = helpers.byte0;
    const auto cc               = uint8_t (helpers.byte1);

    const auto shouldAccumulate = [&]
    {
        switch (cc)
        {
            case 6:
            case 38:
            case 98:
            case 99:
            case 100:
            case 101:
                return true;
        }

        return false;
    }();

    const auto group   = (uint8_t) (helpers.typeAndGroup & std::byte { 0xf });
    const auto channel = (uint8_t) (statusAndChannel & std::byte { 0xf });
    const auto byte    = helpers.byte2;

    if (shouldAccumulate)
    {
        auto& accumulator = groupAccumulators[group][channel];

        if (accumulator.addByte (cc, byte))
        {
            const auto& bytes = accumulator.getBytes();
            const auto bank   = bytes[0];
            const auto index  = bytes[1];
            const auto msb    = bytes[2];
            const auto lsb    = bytes[3];

            const auto value = uint16_t ((uint16_t (msb & std::byte { 0x7f }) << 7) | uint16_t (lsb & std::byte { 0x7f }));

            const auto newStatus = accumulator.getKind() == PnKind::nrpn ? std::byte { 0x3 } : std::byte { 0x2 };

            packet = PacketX2
            {
                Utils::bytesToWord (helpers.typeAndGroup, std::byte ((newStatus << 0x4) | std::byte { channel }), bank, index),
                Conversion::scaleTo32 (value)
            };
            return true;
        }

        return false;
    }

    if (cc == 0)
    {
        groupBanks[group][channel].setMsb (byte);
        return false;
    }

    if (cc == 32)
    {
        groupBanks[group][channel].setLsb (byte);
        return false;
    }

    packet = PacketX2
    {
        Utils::bytesToWord (helpers.typeAndGroup, statusAndChannel, std::byte { cc }, std::byte { 0 }),
        Conversion::scaleTo32 (uint8_t (helpers.byte2))
    };
    return true;
}

PacketX2 Midi1ToMidi2DefaultTranslator::processProgramChange (const HelperValues helpers) const
{
    const auto group   = (uint8_t) (helpers.typeAndGroup & std::byte { 0xf });
    const auto channel = (uint8_t) (helpers.byte0 & std::byte { 0xf });
    const auto bank    = groupBanks[group][channel];
    const auto valid   = bank.isValid();

    return PacketX2
    {
        Utils::bytesToWord (helpers.typeAndGroup,
                            helpers.byte0,
                            std::byte { 0 },
                            valid ? std::byte { 1 } : std::byte { 0 }),
        Utils::bytesToWord (helpers.byte1,
                            std::byte { 0 },
                            valid ? std::byte { bank.getMsb() } : std::byte { 0 },
                            valid ? std::byte { bank.getLsb() } : std::byte { 0 })
    };
}

PacketX2 Midi1ToMidi2DefaultTranslator::processChannelPressure (const HelperValues helpers)
{
    return PacketX2
    {
        Utils::bytesToWord (helpers.typeAndGroup, helpers.byte0, std::byte { 0 }, std::byte { 0 }),
        Conversion::scaleTo32 (uint8_t (helpers.byte1))
    };
}

PacketX2 Midi1ToMidi2DefaultTranslator::processPitchBend (const HelperValues helpers)
{
    const auto lsb   = helpers.byte1;
    const auto msb   = helpers.byte2;
    const auto value = uint16_t (uint16_t (msb) << 7 | uint16_t (lsb));

    return PacketX2
    {
        Utils::bytesToWord (helpers.typeAndGroup, helpers.byte0, std::byte { 0 }, std::byte { 0 }),
        Conversion::scaleTo32 (value)
    };
}

bool Midi1ToMidi2DefaultTranslator::PnAccumulator::addByte (uint8_t cc, std::byte byte)
{
    const auto isStart = cc == 99 || cc == 101;

    if (isStart)
    {
        kind = cc == 99 ? PnKind::nrpn : PnKind::rpn;
        index = 0;
    }

    bytes[index] = byte;

    const auto shouldContinue = [&]
    {
        switch (index)
        {
            case 0: return isStart;
            case 1: return kind == PnKind::nrpn ? cc == 98 : cc == 100;
            case 2: return cc == 6;
            case 3: return cc == 38;
        }

        return false;
    }();

    index = shouldContinue ? index + 1 : 0;

    if (index != bytes.size())
        return false;

    index = 0;
    return true;
}

} // namespace juce::universal_midi_packets

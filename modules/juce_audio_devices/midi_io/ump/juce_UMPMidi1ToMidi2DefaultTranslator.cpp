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
namespace universal_midi_packets
{

PacketX2 Midi1ToMidi2DefaultTranslator::processNoteOnOrOff (const HelperValues helpers)
{
    const auto velocity = helpers.byte2;
    const auto needsConversion = (helpers.byte0 >> 0x4) == 0x9 && velocity == 0;
    const auto firstByte = needsConversion ? (uint8_t) ((0x8 << 0x4) | (helpers.byte0 & 0xf))
                                           : helpers.byte0;

    return PacketX2
    {
        Utils::bytesToWord (helpers.typeAndGroup, firstByte, helpers.byte1, 0),
        (uint32_t) (Conversion::scaleTo16 (velocity) << 0x10)
    };
}

PacketX2 Midi1ToMidi2DefaultTranslator::processPolyPressure (const HelperValues helpers)
{
    return PacketX2
    {
        Utils::bytesToWord (helpers.typeAndGroup, helpers.byte0, helpers.byte1, 0),
        Conversion::scaleTo32 (helpers.byte2)
    };
}

bool Midi1ToMidi2DefaultTranslator::processControlChange (const HelperValues helpers,
                                                          PacketX2& packet)
{
    const auto statusAndChannel = helpers.byte0;
    const auto cc               = helpers.byte1;

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

    const auto group   = (uint8_t) (helpers.typeAndGroup & 0xf);
    const auto channel = (uint8_t) (statusAndChannel & 0xf);
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

            const auto value = (uint16_t) (((msb & 0x7f) << 7) | (lsb & 0x7f));

            const auto newStatus = (uint8_t) (accumulator.getKind() == PnKind::nrpn ? 0x3 : 0x2);

            packet = PacketX2
            {
                Utils::bytesToWord (helpers.typeAndGroup, (uint8_t) ((newStatus << 0x4) | channel), bank, index),
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
        Utils::bytesToWord (helpers.typeAndGroup, statusAndChannel, cc, 0),
        Conversion::scaleTo32 (helpers.byte2)
    };
    return true;
}

PacketX2 Midi1ToMidi2DefaultTranslator::processProgramChange (const HelperValues helpers) const
{
    const auto group   = (uint8_t) (helpers.typeAndGroup & 0xf);
    const auto channel = (uint8_t) (helpers.byte0 & 0xf);
    const auto bank    = groupBanks[group][channel];
    const auto valid   = bank.isValid();

    return PacketX2
    {
        Utils::bytesToWord (helpers.typeAndGroup, helpers.byte0, 0, valid ? 1 : 0),
        Utils::bytesToWord (helpers.byte1, 0, valid ? bank.getMsb() : 0, valid ? bank.getLsb() : 0)
    };
}

PacketX2 Midi1ToMidi2DefaultTranslator::processChannelPressure (const HelperValues helpers)
{
    return PacketX2
    {
        Utils::bytesToWord (helpers.typeAndGroup, helpers.byte0, 0, 0),
        Conversion::scaleTo32 (helpers.byte1)
    };
}

PacketX2 Midi1ToMidi2DefaultTranslator::processPitchBend (const HelperValues helpers)
{
    const auto lsb   = helpers.byte1;
    const auto msb   = helpers.byte2;
    const auto value = (uint16_t) (msb << 7 | lsb);

    return PacketX2
    {
        Utils::bytesToWord (helpers.typeAndGroup, helpers.byte0, 0, 0),
        Conversion::scaleTo32 (value)
    };
}

bool Midi1ToMidi2DefaultTranslator::PnAccumulator::addByte (uint8_t cc, uint8_t byte)
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

}
}

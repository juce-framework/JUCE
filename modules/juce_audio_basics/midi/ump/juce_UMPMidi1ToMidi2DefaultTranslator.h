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
    Translates a series of MIDI 1 Universal MIDI Packets to corresponding MIDI 2
    packets.

    @tags{Audio}
*/
class Midi1ToMidi2DefaultTranslator
{
public:
    Midi1ToMidi2DefaultTranslator() = default;

    /** Converts MIDI 1 Universal MIDI Packets to corresponding MIDI 2 packets,
        calling `callback` with each converted packet.

        In some cases (such as RPN/NRPN messages) multiple MIDI 1 packets will
        convert to a single MIDI 2 packet. In these cases, the translator will
        accumulate the full message internally, and send a single callback with
        the completed message, once all the individual MIDI 1 packets have been
        processed.
    */
    template <typename PacketCallback>
    void dispatch (const View& v, PacketCallback&& callback)
    {
        const auto firstWord = v[0];
        const auto messageType = Utils::getMessageType (firstWord);

        if (messageType != 0x2)
        {
            callback (v);
            return;
        }

        const HelperValues helperValues
        {
            std::byte ((0x4 << 0x4) | Utils::getGroup (firstWord)),
            std::byte ((firstWord >> 0x10) & 0xff),
            std::byte ((firstWord >> 0x08) & 0x7f),
            std::byte ((firstWord >> 0x00) & 0x7f),
        };

        switch (Utils::getStatus (firstWord))
        {
            case 0x8:
            case 0x9:
            {
                const auto packet = processNoteOnOrOff (helperValues);
                callback (View (packet.data()));
                return;
            }

            case 0xa:
            {
                const auto packet = processPolyPressure (helperValues);
                callback (View (packet.data()));
                return;
            }

            case 0xb:
            {
                PacketX2 packet;

                if (processControlChange (helperValues, packet))
                    callback (View (packet.data()));

                return;
            }

            case 0xc:
            {
                const auto packet = processProgramChange (helperValues);
                callback (View (packet.data()));
                return;
            }

            case 0xd:
            {
                const auto packet = processChannelPressure (helperValues);
                callback (View (packet.data()));
                return;
            }

            case 0xe:
            {
                const auto packet = processPitchBend (helperValues);
                callback (View (packet.data()));
                return;
            }
        }
    }

    void reset()
    {
        groupAccumulators = {};
        groupBanks = {};
    }

private:
    enum class PnKind { nrpn, rpn };

    struct HelperValues
    {
        std::byte typeAndGroup;
        std::byte byte0;
        std::byte byte1;
        std::byte byte2;
    };

    static PacketX2 processNoteOnOrOff (HelperValues helpers);
    static PacketX2 processPolyPressure (HelperValues helpers);

    bool processControlChange (HelperValues helpers, PacketX2& packet);

    PacketX2 processProgramChange (HelperValues helpers) const;

    static PacketX2 processChannelPressure (HelperValues helpers);
    static PacketX2 processPitchBend (HelperValues helpers);

    class PnAccumulator
    {
    public:
        bool addByte (uint8_t cc, std::byte byte);

        const std::array<std::byte, 4>& getBytes() const noexcept { return bytes; }
        PnKind getKind() const noexcept { return kind; }

    private:
        std::array<std::byte, 4> bytes;
        uint8_t index = 0;
        PnKind kind = PnKind::nrpn;
    };

    class Bank
    {
    public:
        bool isValid() const noexcept { return ! (msb & 0x80); }

        uint8_t getMsb() const noexcept { return msb & 0x7f; }
        uint8_t getLsb() const noexcept { return lsb & 0x7f; }

        void setMsb (uint8_t i) noexcept { msb = i & 0x7f; }
        void setLsb (uint8_t i) noexcept { msb &= 0x7f; lsb = i & 0x7f; }

    private:
        // We use the top bit to indicate whether this bank is valid.
        // After reading the spec, it's not clear how we should determine whether
        // there are valid values, so we'll just assume that the bank is valid
        // once either the lsb or msb have been written.
        uint8_t msb = 0x80;
        uint8_t lsb = 0x00;
    };

    using ChannelAccumulators = std::array<PnAccumulator, 16>;
    std::array<ChannelAccumulators, 16> groupAccumulators;

    using ChannelBanks = std::array<Bank, 16>;
    std::array<ChannelBanks, 16> groupBanks;
};

} // namespace juce::universal_midi_packets
/** @endcond */

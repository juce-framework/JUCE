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
    Helpful types and functions for interacting with Universal MIDI Packets.

    @tags{Audio}
*/
struct Utils
{
    /** Joins 4 bytes into a single 32-bit word. */
    static constexpr uint32_t bytesToWord (std::byte a, std::byte b, std::byte c, std::byte d)
    {
        return uint32_t (a) << 0x18
             | uint32_t (b) << 0x10
             | uint32_t (c) << 0x08
             | uint32_t (d) << 0x00;
    }

    /** Returns the expected number of 32-bit words in a Universal MIDI Packet, given
        the first word of the packet.

        The result will be between 1 and 4 inclusive.
        A result of 1 means that the word is itself a complete packet.
    */
    static uint32_t getNumWordsForMessageType (uint32_t);

    /**
        Helper functions for setting/getting 4-bit ranges inside a 32-bit word.
    */
    template <size_t Index>
    struct U4
    {
        static constexpr uint32_t shift = (uint32_t) 0x1c - Index * 4;

        static constexpr uint32_t set (uint32_t word, uint8_t value)
        {
            return (word & ~((uint32_t) 0xf << shift)) | (uint32_t) ((value & 0xf) << shift);
        }

        static constexpr uint8_t get (uint32_t word)
        {
            return (uint8_t) ((word >> shift) & 0xf);
        }
    };

    /**
        Helper functions for setting/getting 8-bit ranges inside a 32-bit word.
    */
    template <size_t Index>
    struct U8
    {
        static constexpr uint32_t shift = (uint32_t) 0x18 - Index * 8;

        static constexpr uint32_t set (uint32_t word, uint8_t value)
        {
            return (word & ~((uint32_t) 0xff << shift)) | (uint32_t) (value << shift);
        }

        static constexpr uint8_t get (uint32_t word)
        {
            return (uint8_t) ((word >> shift) & 0xff);
        }
    };

    /**
        Helper functions for setting/getting 16-bit ranges inside a 32-bit word.
    */
    template <size_t Index>
    struct U16
    {
        static constexpr uint32_t shift = (uint32_t) 0x10 - Index * 16;

        static constexpr uint32_t set (uint32_t word, uint16_t value)
        {
            return (word & ~((uint32_t) 0xffff << shift)) | (uint32_t) (value << shift);
        }

        static constexpr uint16_t get (uint32_t word)
        {
            return (uint16_t) ((word >> shift) & 0xffff);
        }
    };

    static constexpr uint8_t getMessageType (uint32_t w) noexcept { return U4<0>::get (w); }
    static constexpr uint8_t getGroup       (uint32_t w) noexcept { return U4<1>::get (w); }
    static constexpr uint8_t getStatus      (uint32_t w) noexcept { return U4<2>::get (w); }
    static constexpr uint8_t getChannel     (uint32_t w) noexcept { return U4<3>::get (w); }
};

} // namespace juce::universal_midi_packets
/** @endcond */

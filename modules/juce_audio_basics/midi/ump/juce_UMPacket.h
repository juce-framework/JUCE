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

namespace juce::universal_midi_packets
{

/**
    Holds a single Universal MIDI Packet.

    @tags{Audio}
*/
template <size_t numWords>
class Packet
{
public:
    Packet() = default;

    template <size_t w = numWords, std::enable_if_t<w == 1, int> = 0>
    Packet (uint32_t a)
        : contents { { a } }
    {
        jassert (Utils::getNumWordsForMessageType (a) == 1);
    }

    template <size_t w = numWords, std::enable_if_t<w == 2, int> = 0>
    Packet (uint32_t a, uint32_t b)
        : contents { { a, b } }
    {
        jassert (Utils::getNumWordsForMessageType (a) == 2);
    }

    template <size_t w = numWords, std::enable_if_t<w == 3, int> = 0>
    Packet (uint32_t a, uint32_t b, uint32_t c)
        : contents { { a, b, c } }
    {
        jassert (Utils::getNumWordsForMessageType (a) == 3);
    }

    template <size_t w = numWords, std::enable_if_t<w == 4, int> = 0>
    Packet (uint32_t a, uint32_t b, uint32_t c, uint32_t d)
        : contents { { a, b, c, d } }
    {
        jassert (Utils::getNumWordsForMessageType (a) == 4);
    }

    template <size_t w, std::enable_if_t<w == numWords, int> = 0>
    explicit Packet (const std::array<uint32_t, w>& fullPacket)
        : contents (fullPacket)
    {
        jassert (Utils::getNumWordsForMessageType (fullPacket.front()) == numWords);
    }

    Packet withMessageType (uint8_t type) const noexcept
    {
        return withU4<0> (type);
    }

    Packet withGroup (uint8_t group) const noexcept
    {
        return withU4<1> (group);
    }

    Packet withStatus (uint8_t status) const noexcept
    {
        return withU4<2> (status);
    }

    Packet withChannel (uint8_t channel) const noexcept
    {
        return withU4<3> (channel);
    }

    uint8_t getMessageType() const noexcept { return getU4<0>(); }

    uint8_t getGroup() const noexcept { return getU4<1>(); }

    uint8_t getStatus() const noexcept { return getU4<2>(); }

    uint8_t getChannel() const noexcept { return getU4<3>(); }

    template <size_t index>
    Packet withU4 (uint8_t value) const noexcept
    {
        constexpr auto word = index / 8;
        auto copy = *this;
        std::get<word> (copy.contents) = Utils::U4<index % 8>::set (copy.template getU32<word>(), value);
        return copy;
    }

    template <size_t index>
    Packet withU8 (uint8_t value) const noexcept
    {
        constexpr auto word = index / 4;
        auto copy = *this;
        std::get<word> (copy.contents) = Utils::U8<index % 4>::set (copy.template getU32<word>(), value);
        return copy;
    }

    template <size_t index>
    Packet withU16 (uint16_t value) const noexcept
    {
        constexpr auto word = index / 2;
        auto copy = *this;
        std::get<word> (copy.contents) = Utils::U16<index % 2>::set (copy.template getU32<word>(), value);
        return copy;
    }

    template <size_t index>
    Packet withU32 (uint32_t value) const noexcept
    {
        auto copy = *this;
        std::get<index> (copy.contents) = value;
        return copy;
    }

    template <size_t index>
    uint8_t getU4() const noexcept
    {
        return Utils::U4<index % 8>::get (this->template getU32<index / 8>());
    }

    template <size_t index>
    uint8_t getU8() const noexcept
    {
        return Utils::U8<index % 4>::get (this->template getU32<index / 4>());
    }

    template <size_t index>
    uint16_t getU16() const noexcept
    {
        return Utils::U16<index % 2>::get (this->template getU32<index / 2>());
    }

    template <size_t index>
    uint32_t getU32() const noexcept
    {
        return std::get<index> (contents);
    }

    //==============================================================================
    using Contents = std::array<uint32_t, numWords>;

    using const_iterator    = typename Contents::const_iterator;

    const_iterator  begin()                   const noexcept { return contents.begin(); }
    const_iterator cbegin()                   const noexcept { return contents.begin(); }

    const_iterator  end()                     const noexcept { return contents.end(); }
    const_iterator cend()                     const noexcept { return contents.end(); }

    const uint32_t* data()                    const noexcept { return contents.data(); }

    const uint32_t& front()                   const noexcept { return contents.front(); }
    const uint32_t& back()                    const noexcept { return contents.back(); }

    const uint32_t& operator[] (size_t index) const noexcept { return contents[index]; }

private:
    Contents contents { {} };
};

using PacketX1 = Packet<1>;
using PacketX2 = Packet<2>;
using PacketX3 = Packet<3>;
using PacketX4 = Packet<4>;

} // namespace juce::universal_midi_packets

#endif

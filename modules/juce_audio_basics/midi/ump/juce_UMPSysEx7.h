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
    This struct acts as a single-file namespace for Universal MIDI Packet
    functionality related to 7-bit SysEx.

    @tags{Audio}
*/
struct SysEx7
{
    /** Returns the number of 64-bit packets required to hold a series of
        SysEx bytes.

        The number passed to this function should exclude the leading/trailing
        SysEx bytes used in an old midi bytestream, as these are not required
        when using Universal MIDI Packets.
    */
    static uint32_t getNumPacketsRequiredForDataSize (uint32_t);

    /** The different kinds of UMP SysEx-7 message. */
    enum class Kind : uint8_t
    {
        /** The whole message fits in a single 2-word packet. */
        complete     = 0,

        /** The packet begins a SysEx message that will continue in subsequent packets. */
        begin        = 1,

        /** The packet is a continuation of an ongoing SysEx message. */
        continuation = 2,

        /** The packet terminates an ongoing SysEx message. */
        end          = 3
    };

    /** Holds the bytes from a single SysEx-7 packet. */
    struct PacketBytes
    {
        std::array<uint8_t, 6> data;
        uint8_t size;
    };

    /** Extracts the data bytes from a 64-bit data message. */
    static PacketBytes getDataBytes (const PacketX2& packet);
};

}
}

#endif

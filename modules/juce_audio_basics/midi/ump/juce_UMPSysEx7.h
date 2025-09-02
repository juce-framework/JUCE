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
        std::array<std::byte, 6> data;
        uint8_t size;
    };

    /** Extracts the data bytes from a 64-bit data message. */
    static PacketBytes getDataBytes (const PacketX2& packet);
};

} // namespace juce::universal_midi_packets
/** @endcond */

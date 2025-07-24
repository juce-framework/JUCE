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

/** Kinds of MIDI message transport.
*/
enum class Transport : uint8_t
{
    bytestream, ///< A stream of variable-length messages. Suitable for MIDI 1.0.
    ump,        ///< A stream of 32-bit words. Suitable for MIDI-1UP and MIDI 2.0.
};

/** The kinds of MIDI protocol that can be formatted into Universal MIDI Packets.
*/
enum class PacketProtocol : uint8_t
{
    MIDI_1_0,
    MIDI_2_0,
};

/** All kinds of MIDI protocol understood by JUCE.
*/
enum class MidiProtocol : uint8_t
{
    bytestream,
    UMP_MIDI_1_0,
    UMP_MIDI_2_0,
};

} // namespace juce::universal_midi_packets
/** @endcond */

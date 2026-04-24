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

String StringUtils::getHexString (const View& view)
{
    auto result = String::toHexString (*view.data());

    std::for_each (std::next (view.begin()), view.end(), [&] (uint32_t word)
    {
        result << ' ' << String::toHexString (word);
    });

    return result;
}

static String getUtilityMessageDescription (const PacketX1& p)
{
    const auto word = p.front();

    switch ((uint8_t) Utils::getStatus (word))
    {
        case 0x0: return "NOOP";
        case 0x1: return "JR Clock " + String::toHexString ((word >> 0x10) & 0xff);
        case 0x2: return "JR Timestamp " + String::toHexString ((word >> 0x10) & 0xff);
    }

    jassertfalse;
    return {};
}

static String getPacketDescription (const PacketX1& p)
{
    switch (Utils::getMessageType (p.front()))
    {
        case Utils::MessageKind::utility:           return "Utility: " + getUtilityMessageDescription (p);
        case Utils::MessageKind::commonRealtime:    return "System: " + SingleGroupMidi1ToBytestreamExtractor::fromUmp (p).getDescription();
        case Utils::MessageKind::channelVoice1:     return "MIDI 1.0 Channel Voice: " + SingleGroupMidi1ToBytestreamExtractor::fromUmp (p).getDescription();

        case Utils::MessageKind::channelVoice2:
        case Utils::MessageKind::sysex7:
        case Utils::MessageKind::sysex8:
        case Utils::MessageKind::stream:
            break;
    }

    jassertfalse;
    return {};
}

static String getData64MessageDescription (const PacketX2& p)
{
    const auto bytes = SysEx7::getDataBytes (p);

    String byteString;

    std::for_each (bytes.data.data(), bytes.data.data() + bytes.size, [&] (std::byte byte)
    {
        byteString << String::toHexString ((uint8_t) byte);
    });

    switch (SysEx7::Kind (Utils::getStatus (p.front())))
    {
        case SysEx7::Kind::complete:     return "Full: " + byteString;
        case SysEx7::Kind::begin:        return "Start: " + byteString;
        case SysEx7::Kind::continuation: return "Continue: " + byteString;
        case SysEx7::Kind::end:          return "End: " + byteString;
    }

    jassertfalse;
    return {};
}

static String getMidi2ChannelVoiceDescription (const PacketX2& p)
{
    const auto getNoteName = [&] { return MidiMessage::getMidiNoteName ((p[0] >> 0x8) & 0xff, true, true, 3); };
    const auto getByte3 = [&] { return String::toHexString ((p[0] >> 0x8) & 0xff); };
    const auto getByte4 = [&] { return String::toHexString ((p[0] >> 0x0) & 0xff); };

    const auto getVelocity = [&] { return String::toHexString ((p[1] >> 0x10) & 0xffff); };
    const auto getAttribute = [&] { return String::toHexString ((p[1] >> 0x00) & 0xffff); };

    const auto getChannel = [&] { return String::toHexString (p.getChannel()); };

    switch ((uint8_t) Utils::getStatus (p[0]))
    {
        case 0x0: return "Registered Per-Note Controller " + getByte4()
                         + " Channel " + getChannel()
                         + " Note " + getNoteName()
                         + " Data " + String::toHexString (p[1]);
        case 0x1: return "Assignable Per-Note Controller " + getByte4()
                         + " Channel " + getChannel()
                         + " Note " + getNoteName()
                         + " Data " + String::toHexString (p[1]);
        case 0x2: return "Registered Controller Bank " + getByte3()
                         + " Channel " + getChannel()
                         + " Index " + getByte4()
                         + " Data " + String::toHexString (p[1]);
        case 0x3: return "Assignable Controller Bank " + getByte3()
                         + " Channel " + getChannel()
                         + " Index " + getByte4()
                         + " Data " + String::toHexString (p[1]);
        case 0x4: return "Relative Registered Controller Bank " + getByte3()
                         + " Channel " + getChannel()
                         + " Index " + getByte4()
                         + " Data " + String::toHexString (p[1]);
        case 0x5: return "Relative Assignable Controller Bank " + getByte3()
                         + " Channel " + getChannel()
                         + " Index " + getByte4()
                         + " Data " + String::toHexString (p[1]);
        case 0x6: return "Per-Note Pitch Bend Note " + getNoteName()
                         + " Channel " + getChannel()
                         + " Data " + String::toHexString (p[1]);
        case 0x8: return "Note Off " + getNoteName()
                         + " Channel " + getChannel()
                         + " Attribute Type " + getByte4()
                         + " Velocity " + getVelocity()
                         + " Attribute Data " + getAttribute();
        case 0x9: return "Note On " + getNoteName()
                         + " Channel " + getChannel()
                         + " Attribute Type " + getByte4()
                         + " Velocity " + getVelocity()
                         + " Attribute Data " + getAttribute();
        case 0xa: return "Poly Pressure Note " + getNoteName()
                         + " Channel " + getChannel()
                         + " Data " + String::toHexString (p[1]);
        case 0xb: return "Control Change " + getByte3()
                         + " Channel " + getChannel()
                         + " Data " + String::toHexString (p[1]);
        case 0xc: return "Program Change Options " + getByte4()
                         + " Channel " + getChannel()
                         + " Program " + String::toHexString ((p[1] >> 0x18) & 0xff)
                         + " Bank MSB " + String::toHexString ((p[1] >> 0x08) & 0xff)
                         + " Bank LSB " + String::toHexString ((p[1] >> 0x00) & 0xff);
        case 0xd: return "Channel Pressure " + String::toHexString (p[1])
                         + " Channel " + getChannel();
        case 0xe: return "Pitch Bend " + String::toHexString (p[1])
                         + " Channel " + getChannel();
        case 0xf: return "Per-Note Management Note " + getNoteName()
                         + " Channel " + getChannel()
                         + " Options " + getByte4();
    }

    jassertfalse;
    return {};
}

static String getPacketDescription (const PacketX2& p)
{
    switch (Utils::getMessageType (p.front()))
    {
        case Utils::MessageKind::sysex7:        return "Data 64-Bit: " + getData64MessageDescription (p);
        case Utils::MessageKind::channelVoice2: return "MIDI 2.0 Channel Voice: " + getMidi2ChannelVoiceDescription (p);

        case Utils::MessageKind::utility:
        case Utils::MessageKind::channelVoice1:
        case Utils::MessageKind::sysex8:
        case Utils::MessageKind::commonRealtime:
        case Utils::MessageKind::stream:
            break;
    }

    jassertfalse;
    return {};
}

static String getPacketDescription (const PacketX3&)
{
    jassertfalse;
    return {};
}

static String getPacketDescription (const PacketX4& p)
{
    switch (Utils::getMessageType (p[0]))
    {
        case Utils::MessageKind::sysex8: return "Data 128 Bit";         // TODO
        case Utils::MessageKind::stream: return "Stream Configuration"; // TODO

        case Utils::MessageKind::utility:
        case Utils::MessageKind::sysex7:
        case Utils::MessageKind::channelVoice1:
        case Utils::MessageKind::channelVoice2:
        case Utils::MessageKind::commonRealtime:
            break;
    }

    jassertfalse;
    return {};
}

String StringUtils::getDescription (const View& v)
{
    const auto group = (String ("Group ") + String (Utils::getGroup (*v.data()))) + ' ';

    switch (v.size())
    {
        case 1:
            return group + getPacketDescription (PacketX1 { v[0] });

        case 2:
            return group + getPacketDescription (PacketX2 { v[0], v[1] });

        case 3:
            return group + getPacketDescription (PacketX3 { v[0], v[1], v[2] });

        case 4:
            return group + getPacketDescription (PacketX4 { v[0], v[1], v[2], v[3] });
    }

    jassertfalse;
    return {};
}

} // namespace juce::universal_midi_packets

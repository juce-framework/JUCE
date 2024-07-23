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

namespace juce::universal_midi_packets
{

/**
    Holds MIDI device info that may be required by certain UMP messages and
    MIDI-CI messages.

    @tags{Audio}
*/
struct DeviceInfo
{
    std::array<std::byte, 3> manufacturer;  ///< LSB first
    std::array<std::byte, 2> family;        ///< LSB first
    std::array<std::byte, 2> modelNumber;   ///< LSB first
    std::array<std::byte, 4> revision;

private:
    auto tie() const { return std::tie (manufacturer, family, modelNumber, revision); }

public:
    bool operator== (const DeviceInfo& other) const { return tie() == other.tie(); }
    bool operator!= (const DeviceInfo& other) const { return tie() != other.tie(); }

    static constexpr auto marshallingVersion = std::nullopt;

    template <typename Archive, typename This>
    static auto serialise (Archive& archive, This& t)
    {
        return archive (named ("manufacturer", t.manufacturer),
                        named ("family", t.family),
                        named ("modelNumber", t.modelNumber),
                        named ("revision", t.revision));
    }
};

} // namespace juce::universal_midi_packets

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

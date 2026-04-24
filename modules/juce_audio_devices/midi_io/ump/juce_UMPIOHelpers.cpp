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

struct EndpointAndStaticInfo
{
    Endpoint endpoint;
    StaticDeviceInfo info;
    EndpointId id;
};

class IOHelpers
{
public:
    IOHelpers() = delete;

    static Block makeLegacyBlock (bool isInput)
    {
        const auto direction = isInput ? BlockDirection::receiver : BlockDirection::sender;
        const auto hint = isInput ? BlockUiHint::receiver : BlockUiHint::sender;
        return Block{}.withName ("Legacy MIDI 1.0")
                      .withEnabled (true)
                      .withFirstGroup (0)
                      .withNumGroups (1)
                      .withMIDI1ProxyKind (BlockMIDI1ProxyKind::unrestrictedBandwidth)
                      .withDirection (direction)
                      .withUiHint (hint);
    }

    static EndpointAndStaticInfo makeProxyEndpoint (const MidiDeviceInfo& info, BlockDirection direction)
    {
        jassert (direction != BlockDirection::unknown);

        const auto hint = std::invoke ([&]
        {
            switch (direction)
            {
                case BlockDirection::bidirectional: return BlockUiHint::bidirectional;
                case BlockDirection::sender: return BlockUiHint::sender;
                case BlockDirection::receiver: return BlockUiHint::receiver;
                case BlockDirection::unknown: break;
            }

            return BlockUiHint::unknown;
        });

        const auto block = Block{}.withDirection (direction)
                                  .withUiHint (hint)
                                  .withEnabled (true)
                                  .withFirstGroup (0)
                                  .withNumGroups (1)
                                  .withMIDI1ProxyKind (BlockMIDI1ProxyKind::unrestrictedBandwidth);
        const auto id = std::invoke ([&]
        {
            switch (direction)
            {
                case BlockDirection::bidirectional:
                    return EndpointId::makeSrcDst (info.identifier, info.identifier);
                case BlockDirection::receiver:
                    return EndpointId::make (IOKind::dst, info.identifier);
                case BlockDirection::sender:
                    return EndpointId::make (IOKind::src, info.identifier);
                case BlockDirection::unknown: break;
            }

            return EndpointId{};
        });

        const auto srcId = direction != BlockDirection::receiver ? info.identifier : "";
        const auto dstId = direction != BlockDirection::sender ? info.identifier : "";

        std::array<String, 16> srcIds { srcId };
        std::array<String, 16> dstIds { dstId };

        const auto baseEndpoint = Endpoint{}.withName (info.name)
                                            .withProtocol (PacketProtocol::MIDI_1_0)
                                            .withUMPVersion (1, 1)
                                            .withMidi1Support (true)
                                            .withStaticBlocks (true)
                                            .withBlocks (std::array { block });

        const auto staticInfo = StaticDeviceInfo{}.withLegacyIdentifiersSrc (srcIds)
                                                  .withLegacyIdentifiersDst (dstIds)
                                                  .withHasSource (direction == BlockDirection::sender)
                                                  .withHasDestination (direction == BlockDirection::receiver)
                                                  .withName (info.name)
                                                  .withTransport (Transport::bytestream);

        return { baseEndpoint, staticInfo, id };
    }
};

}

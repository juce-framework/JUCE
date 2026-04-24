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

//==============================================================================
/**
    Represents a single MIDI endpoint, which may have up to one input and up to one output.

    An Endpoint object just holds a snapshot of the physical endpoint's last known state at the
    point when the Endpoint instance was created.
    Instead of storing Endpoint instances, it's a better idea to store an EndpointId, and to call
    Endpoints::getEndpoint() to get an up-to-date snapshot.

    To connect to an endpoint, use Session::connectInput() or Session::connectOutput() to create
    a connection in the context of a particular session.

    @tags{Audio}
*/
class Endpoint
{
public:
    Endpoint() = default;

    [[nodiscard]] Endpoint withName (String x) const { return withMember (*this, &Endpoint::name, x); }
    [[nodiscard]] Endpoint withProtocol (std::optional<PacketProtocol> x) const { return withMember (*this, &Endpoint::streamConfig, StreamConfigFlags ((streamConfig & ~maskProtocol) | asFlags (x))); }
    [[nodiscard]] Endpoint withDeviceInfo (DeviceInfo x) const { return withMember (*this, &Endpoint::deviceInfo, std::move (x)); }
    [[nodiscard]] Endpoint withProductInstanceId (String x) const { return withMember (*this, &Endpoint::productInstanceId, std::move (x)); }
    [[nodiscard]] Endpoint withUMPVersion (uint8_t major, uint8_t minor) const { return withMember (*this, &Endpoint::endpointInfo, endpointInfo.withVersion (major, minor)); }
    [[nodiscard]] Endpoint withStaticBlocks (bool x = true) const { return withMember (*this, &Endpoint::endpointInfo, endpointInfo.withStaticFunctionBlocks (x)); }
    [[nodiscard]] Endpoint withMidi1Support (bool x = true) const { return withMember (*this, &Endpoint::endpointInfo, endpointInfo.withMidi1Support (x)); }
    [[nodiscard]] Endpoint withMidi2Support (bool x = true) const { return withMember (*this, &Endpoint::endpointInfo, endpointInfo.withMidi2Support (x)); }
    [[nodiscard]] Endpoint withReceiveJRSupport (bool x = true) const { return withMember (*this, &Endpoint::endpointInfo, endpointInfo.withReceiveJRSupport (x)); }
    [[nodiscard]] Endpoint withTransmitJRSupport (bool x = true) const { return withMember (*this, &Endpoint::endpointInfo, endpointInfo.withTransmitJRSupport (x)); }

    [[nodiscard]] Endpoint withReceiveJREnabled (bool x = true) const { return withMember (*this, &Endpoint::streamConfig, StreamConfigFlags (x ? (streamConfig | maskRxjr) : (streamConfig & ~maskRxjr))); }
    [[nodiscard]] Endpoint withTransmitJREnabled (bool x = true) const { return withMember (*this, &Endpoint::streamConfig, StreamConfigFlags (x ? (streamConfig | maskTxjr) : (streamConfig & ~maskTxjr))); }

    /** The block index is used to uniquely identify the block, so be sure to always declare
        blocks in a consistent order.
    */
    [[nodiscard]] Endpoint withBlocks (Span<const Block> x) const
    {
        auto result = withNumBlocks (x.size());
        std::copy (x.begin(), x.end(), result.blocks.begin());
        return result;
    }

    /** The number of blocks on this endpoint. */
    [[nodiscard]] Endpoint withNumBlocks (size_t x) const
    {
        jassert (x <= blocks.size());
        auto result = *this;
        result.endpointInfo = result.endpointInfo.withNumFunctionBlocks ((uint8_t) std::min (blocks.size(), x));
        return result;
    }

    /** Returns the name of this endpoint. */
    String getName() const { return name; }

    /** Returns properties of the device that owns the endpoint. */
    DeviceInfo getDeviceInfo() const
    {
        return deviceInfo;
    }

    /** Returns the product instance ID if available, or an empty string otherwise.
        The product instance ID should match the device serial number, and should be unique per
        manufacturer, family, and model.

        This ID can be used to distinguish between separate devices that have the same DeviceInfo.
        It can also be used to determine whether separate endpoints are associated with the same
        device.
    */
    String getProductInstanceId() const
    {
        return productInstanceId;
    }

    /** The protocol that the endpoint currently expects to send and receive; endpoints are allowed
        to switch protocols, so this won't always return the same value.
        May return nullopt if the protocol is unknown, perhaps because negotiation has not taken place.
    */
    std::optional<PacketProtocol> getProtocol() const
    {
        switch (streamConfig & maskProtocol)
        {
            case 1: return PacketProtocol::MIDI_1_0;
            case 2: return PacketProtocol::MIDI_2_0;
        }

        return {};
    }

    uint8_t getUMPVersionMajor()    const { return endpointInfo.getVersionMajor(); }
    uint8_t getUMPVersionMinor()    const { return endpointInfo.getVersionMinor(); }
    bool hasStaticBlocks()          const { return endpointInfo.hasStaticFunctionBlocks(); }
    bool hasMidi1Support()          const { return endpointInfo.hasMidi1Support(); }
    bool hasMidi2Support()          const { return endpointInfo.hasMidi2Support(); }
    bool hasReceiveJRSupport()      const { return endpointInfo.hasReceiveJRSupport(); }
    bool hasTransmitJRSupport()     const { return endpointInfo.hasTransmitJRSupport(); }

    bool isReceiveJREnabled()       const { return streamConfig & maskRxjr; }
    bool isTransmitJREnabled()      const { return streamConfig & maskTxjr; }

    /** There can be a maximum of 32 blocks.
        This may return an empty span if the endpoint has neither function blocks nor group terminal
        blocks.
    */
    Span<const Block> getBlocks() const&
    {
        return { blocks.data(), endpointInfo.getNumFunctionBlocks() };
    }

    /** Returns a mutable view over the blocks in this endpoint. */
    Span<Block> getBlocks() &
    {
        return { blocks.data(), endpointInfo.getNumFunctionBlocks() };
    }

    // These are deleted because it's probably not a good idea to create a span over a temporary
    Span<const Block> getBlocks() const&& = delete;
    Span<Block> getBlocks() && = delete;

private:
    enum StreamConfigFlags : uint16_t
    {
        maskProtocol = 0x00ff,
        maskTxjr     = 0x0100,
        maskRxjr     = 0x0200,
    };

    static StreamConfigFlags asFlags (std::optional<PacketProtocol> p)
    {
        if (! p.has_value())
            return {};

        switch (*p)
        {
            case PacketProtocol::MIDI_1_0: return StreamConfigFlags (1);
            case PacketProtocol::MIDI_2_0: return StreamConfigFlags (2);
        }

        return {};
    }

    std::array<Block, 32> blocks;
    String name;
    String productInstanceId;
    EndpointInfo endpointInfo;
    DeviceInfo deviceInfo;
    StreamConfigFlags streamConfig{};
};

}

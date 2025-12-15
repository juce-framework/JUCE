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

namespace juce::midi_ci
{

//==============================================================================
/**
    Byte values representing different addresses within a group.

    @tags{Audio}
*/
enum class ChannelInGroup : uint8_t
{
    channel0 = 0x0,
    channel1 = 0x1,
    channel2 = 0x2,
    channel3 = 0x3,
    channel4 = 0x4,
    channel5 = 0x5,
    channel6 = 0x6,
    channel7 = 0x7,
    channel8 = 0x8,
    channel9 = 0x9,
    channelA = 0xA,
    channelB = 0xB,
    channelC = 0xC,
    channelD = 0xD,
    channelE = 0xE,
    channelF = 0xF,
    wholeGroup = 0x7e, ///< Refers to all channels in the UMP group
    wholeBlock = 0x7f, ///< Refers to all channels in the function block that contains the UMP group
};

/**
    Utility functions for working with the ChannelInGroup enum.

    @tags{Audio}
*/
struct ChannelInGroupUtils
{
    ChannelInGroupUtils() = delete;

    /** Converts a ChannelInGroup to a descriptive string. */
    static String toString (ChannelInGroup c)
    {
        if (c == ChannelInGroup::wholeGroup)
            return "Group";

        if (c == ChannelInGroup::wholeBlock)
            return "Function Block";

        const auto underlying = (std::underlying_type_t<ChannelInGroup>) c;
        return "Channel " + String (underlying + 1);
    }
};

using Profile = std::array<std::byte, 5>;

//==============================================================================
/**
    Namespace containing structs representing different kinds of MIDI-CI message.

    @tags{Audio}
*/
namespace Message
{
    /** Wraps a span, providing equality operators that compare the span
        contents elementwise.

        @tags{Audio}
    */
    template <typename T>
    struct ComparableRange
    {
        T& data;

        bool operator== (const ComparableRange& other) const
        {
            return std::equal (data.begin(), data.end(), other.data.begin(), other.data.end());
        }

        bool operator!= (const ComparableRange& other) const { return ! operator== (other); }
    };

    template <typename T> static constexpr auto makeComparableRange (      T& t) { return ComparableRange<      T> { t }; }
    template <typename T> static constexpr auto makeComparableRange (const T& t) { return ComparableRange<const T> { t }; }

    //==============================================================================
    /**
        Holds fields that can be found at the beginning of every MIDI CI message.

        @tags{Audio}
    */
    struct Header
    {
        ChannelInGroup deviceID{};
        std::byte category{};
        std::byte version{};
        MUID source = MUID::makeUnchecked (0);
        MUID destination = MUID::makeUnchecked (0);

        auto tie() const;

        bool operator== (const Header& x) const;
        bool operator!= (const Header& x) const;
    };

    /**
        Groups together a CI message header, and some number of trailing bytes.

        @tags{Audio}
    */
    struct Generic
    {
        Header header;
        Span<const std::byte> data;
    };

    //==============================================================================
    /** See the MIDI-CI specification.

        @tags{Audio}
    */
    struct DiscoveryResponse
    {
        ump::DeviceInfo device;
        std::byte capabilities{};
        uint32_t maximumSysexSize{};
        std::byte outputPathID{};       /**< Only valid if the message header specifies version 0x02 or greater. */
        std::byte functionBlock{};      /**< Only valid if the message header specifies version 0x02 or greater. */

        auto tie() const;

        bool operator== (const DiscoveryResponse& x) const;
        bool operator!= (const DiscoveryResponse& x) const;
    };

    /** See the MIDI-CI specification.

        @tags{Audio}
    */
    struct Discovery
    {
        ump::DeviceInfo device;
        std::byte capabilities{};
        uint32_t maximumSysexSize{};
        std::byte outputPathID{};       /**< Only valid if the message header specifies version 0x02 or greater. */

        auto tie() const;

        bool operator== (const Discovery& x) const;
        bool operator!= (const Discovery& x) const;
    };

    /** See the MIDI-CI specification.

        @tags{Audio}
    */
    struct EndpointInquiryResponse
    {
        std::byte status;
        Span<const std::byte> data;

        auto tie() const;

        bool operator== (const EndpointInquiryResponse& x) const;
        bool operator!= (const EndpointInquiryResponse& x) const;
    };

    /** See the MIDI-CI specification.

        @tags{Audio}
    */
    struct EndpointInquiry
    {
        std::byte status;

        auto tie() const;

        bool operator== (const EndpointInquiry& x) const;
        bool operator!= (const EndpointInquiry& x) const;
    };

    /** See the MIDI-CI specification.

        @tags{Audio}
    */
    struct InvalidateMUID
    {
        MUID target = MUID::makeUnchecked (0);

        auto tie() const;

        bool operator== (const InvalidateMUID& x) const;
        bool operator!= (const InvalidateMUID& x) const;
    };

    /** See the MIDI-CI specification.

        @tags{Audio}
    */
    struct ACK
    {
        std::byte originalCategory{};
        std::byte statusCode{};
        std::byte statusData{};
        std::array<std::byte, 5> details{};
        Span<const std::byte> messageText{};

        /** Convenience function that returns the message's text as a String. */
        String getMessageTextAsString() const
        {
            return Encodings::stringFrom7BitText (messageText);
        }

        auto tie() const;

        bool operator== (const ACK& x) const;
        bool operator!= (const ACK& x) const;
    };

    /** See the MIDI-CI specification.

        @tags{Audio}
    */
    struct NAK
    {
        std::byte originalCategory{};        /**< Only valid if the message header specifies version 0x02 or greater. */
        std::byte statusCode{};              /**< Only valid if the message header specifies version 0x02 or greater. */
        std::byte statusData{};              /**< Only valid if the message header specifies version 0x02 or greater. */
        std::array<std::byte, 5> details{};  /**< Only valid if the message header specifies version 0x02 or greater. */
        Span<const std::byte> messageText{}; /**< Only valid if the message header specifies version 0x02 or greater. */

        /** Convenience function that returns the message's text as a String. */
        String getMessageTextAsString() const
        {
            return Encodings::stringFrom7BitText (messageText);
        }

        auto tie() const;

        bool operator== (const NAK& x) const;
        bool operator!= (const NAK& x) const;
    };

    /** See the MIDI-CI specification.

        @tags{Audio}
    */
    struct ProfileInquiryResponse
    {
        Span<const Profile> enabledProfiles;
        Span<const Profile> disabledProfiles;

        auto tie() const;

        bool operator== (const ProfileInquiryResponse& x) const;
        bool operator!= (const ProfileInquiryResponse& x) const;
    };

    /** See the MIDI-CI specification.

        @tags{Audio}
    */
    struct ProfileInquiry
    {
        auto tie() const;

        bool operator== (const ProfileInquiry& x) const;
        bool operator!= (const ProfileInquiry& x) const;
    };

    /** See the MIDI-CI specification.

        @tags{Audio}
    */
    struct ProfileAdded
    {
        Profile profile{};

        auto tie() const;

        bool operator== (const ProfileAdded& x) const;
        bool operator!= (const ProfileAdded& x) const;
    };

    /** See the MIDI-CI specification.

        @tags{Audio}
    */
    struct ProfileRemoved
    {
        Profile profile{};

        auto tie() const;

        bool operator== (const ProfileRemoved& x) const;
        bool operator!= (const ProfileRemoved& x) const;
    };

    /** See the MIDI-CI specification.

        @tags{Audio}
    */
    struct ProfileDetailsResponse
    {
        Profile profile{};
        std::byte target{};
        Span<const std::byte> data;

        auto tie() const;

        bool operator== (const ProfileDetailsResponse& x) const;
        bool operator!= (const ProfileDetailsResponse& x) const;
    };

    /** See the MIDI-CI specification.

        @tags{Audio}
    */
    struct ProfileDetails
    {
        Profile profile{};
        std::byte target{};

        auto tie() const;

        bool operator== (const ProfileDetails& x) const;
        bool operator!= (const ProfileDetails& x) const;
    };

    /** See the MIDI-CI specification.

        @tags{Audio}
    */
    struct ProfileOn
    {
        Profile profile{};
        uint16_t numChannels{}; /**< Only valid if the message header specifies version 0x02 or greater. */

        auto tie() const;

        bool operator== (const ProfileOn& x) const;
        bool operator!= (const ProfileOn& x) const;
    };

    /** See the MIDI-CI specification.

        @tags{Audio}
    */
    struct ProfileOff
    {
        Profile profile{};

        auto tie() const;

        bool operator== (const ProfileOff& x) const;
        bool operator!= (const ProfileOff& x) const;
    };

    /** See the MIDI-CI specification.

        @tags{Audio}
    */
    struct ProfileEnabledReport
    {
        Profile profile{};
        uint16_t numChannels{}; /**< Only valid if the message header specifies version 0x02 or greater. */

        auto tie() const;

        bool operator== (const ProfileEnabledReport& x) const;
        bool operator!= (const ProfileEnabledReport& x) const;
    };

    /** See the MIDI-CI specification.

        @tags{Audio}
    */
    struct ProfileDisabledReport
    {
        Profile profile{};
        uint16_t numChannels{}; /**< Only valid if the message header specifies version 0x02 or greater. */

        auto tie() const;

        bool operator== (const ProfileDisabledReport& x) const;
        bool operator!= (const ProfileDisabledReport& x) const;
    };

    /** See the MIDI-CI specification.

        @tags{Audio}
    */
    struct ProfileSpecificData
    {
        Profile profile{};
        Span<const std::byte> data;

        auto tie() const;

        bool operator== (const ProfileSpecificData& x) const;
        bool operator!= (const ProfileSpecificData& x) const;
    };

    /** See the MIDI-CI specification.

        @tags{Audio}
    */
    struct PropertyExchangeCapabilitiesResponse
    {
        std::byte numSimultaneousRequestsSupported{};
        std::byte majorVersion{}; /**< Only valid if the message header specifies version 0x02 or greater. */
        std::byte minorVersion{}; /**< Only valid if the message header specifies version 0x02 or greater. */

        auto tie() const;

        bool operator== (const PropertyExchangeCapabilitiesResponse& x) const;
        bool operator!= (const PropertyExchangeCapabilitiesResponse& x) const;
    };

    /** See the MIDI-CI specification.

        @tags{Audio}
    */
    struct PropertyExchangeCapabilities
    {
        std::byte numSimultaneousRequestsSupported{};
        std::byte majorVersion{}; /**< Only valid if the message header specifies version 0x02 or greater. */
        std::byte minorVersion{}; /**< Only valid if the message header specifies version 0x02 or greater. */

        auto tie() const;

        bool operator== (const PropertyExchangeCapabilities& x) const;
        bool operator!= (const PropertyExchangeCapabilities& x) const;
    };

    /** A property-exchange message that has no payload, and must therefore
        be contained in a single chunk.

        @tags{Audio}
    */
    struct StaticSizePropertyExchange
    {
        std::byte requestID{};
        Span<const std::byte> header;

        auto tie() const;
    };

    /** A property-exchange message that may form part of a multi-chunk
        message sequence.

        @tags{Audio}
    */
    struct DynamicSizePropertyExchange
    {
        std::byte requestID{};
        Span<const std::byte> header;
        uint16_t totalNumChunks{};
        uint16_t thisChunkNum{};
        Span<const std::byte> data;

        auto tie() const;
    };

    /** See the MIDI-CI specification.

        @tags{Audio}
    */
    struct PropertyGetDataResponse : public DynamicSizePropertyExchange
    {
        bool operator== (const PropertyGetDataResponse& x) const;
        bool operator!= (const PropertyGetDataResponse& x) const;
    };

    /** See the MIDI-CI specification.

        @tags{Audio}
    */
    struct PropertyGetData : public StaticSizePropertyExchange
    {
        bool operator== (const PropertyGetData& x) const;
        bool operator!= (const PropertyGetData& x) const;
    };

    /** See the MIDI-CI specification.

        @tags{Audio}
    */
    struct PropertySetDataResponse : public StaticSizePropertyExchange
    {
        bool operator== (const PropertySetDataResponse& x) const;
        bool operator!= (const PropertySetDataResponse& x) const;
    };

    /** See the MIDI-CI specification.

        @tags{Audio}
    */
    struct PropertySetData : public DynamicSizePropertyExchange
    {
        bool operator== (const PropertySetData& x) const;
        bool operator!= (const PropertySetData& x) const;
    };

    /** See the MIDI-CI specification.

        @tags{Audio}
    */
    struct PropertySubscribeResponse : public DynamicSizePropertyExchange
    {
        bool operator== (const PropertySubscribeResponse& x) const;
        bool operator!= (const PropertySubscribeResponse& x) const;
    };

    /** See the MIDI-CI specification.

        @tags{Audio}
    */
    struct PropertySubscribe : public DynamicSizePropertyExchange
    {
        bool operator== (const PropertySubscribe& x) const;
        bool operator!= (const PropertySubscribe& x) const;
    };

    /** See the MIDI-CI specification.

        @tags{Audio}
    */
    struct PropertyNotify : public DynamicSizePropertyExchange
    {
        bool operator== (const PropertyNotify& x) const;
        bool operator!= (const PropertyNotify& x) const;
    };

    /** See the MIDI-CI specification.

        @tags{Audio}
    */
    struct ProcessInquiryResponse
    {
        std::byte supportedFeatures{};

        auto tie() const;

        bool operator== (const ProcessInquiryResponse& x) const;
        bool operator!= (const ProcessInquiryResponse& x) const;
    };

    /** See the MIDI-CI specification.

        @tags{Audio}
    */
    struct ProcessInquiry
    {
        auto tie() const;

        bool operator== (const ProcessInquiry& x) const;
        bool operator!= (const ProcessInquiry& x) const;
    };

    /** See the MIDI-CI specification.

        @tags{Audio}
    */
    struct ProcessMidiMessageReportResponse
    {
        std::byte messageDataControl{};
        std::byte requestedMessages{};
        std::byte channelControllerMessages{};
        std::byte noteDataMessages{};

        auto tie() const;

        bool operator== (const ProcessMidiMessageReportResponse& x) const;
        bool operator!= (const ProcessMidiMessageReportResponse& x) const;
    };

    /** See the MIDI-CI specification.

        @tags{Audio}
    */
    struct ProcessMidiMessageReport
    {
        std::byte messageDataControl{};
        std::byte requestedMessages{};
        std::byte channelControllerMessages{};
        std::byte noteDataMessages{};

        auto tie() const;

        bool operator== (const ProcessMidiMessageReport& x) const;
        bool operator!= (const ProcessMidiMessageReport& x) const;
    };

    /** See the MIDI-CI specification.

        @tags{Audio}
    */
    struct ProcessEndMidiMessageReport
    {
        auto tie() const;

        bool operator== (const ProcessEndMidiMessageReport& x) const;
        bool operator!= (const ProcessEndMidiMessageReport& x) const;
    };

    /**
        A message with a header and optional body.

        The body may be set to std::monostate to indicate some kind of failure, such as a malformed
        incoming message.

        @tags{Audio}
    */
    struct Parsed
    {
        using Body = std::variant<std::monostate,
                                  Discovery,
                                  DiscoveryResponse,
                                  InvalidateMUID,
                                  EndpointInquiry,
                                  EndpointInquiryResponse,
                                  ACK,
                                  NAK,
                                  ProfileInquiry,
                                  ProfileInquiryResponse,
                                  ProfileAdded,
                                  ProfileRemoved,
                                  ProfileDetails,
                                  ProfileDetailsResponse,
                                  ProfileOn,
                                  ProfileOff,
                                  ProfileEnabledReport,
                                  ProfileDisabledReport,
                                  ProfileSpecificData,
                                  PropertyExchangeCapabilities,
                                  PropertyExchangeCapabilitiesResponse,
                                  PropertyGetData,
                                  PropertyGetDataResponse,
                                  PropertySetData,
                                  PropertySetDataResponse,
                                  PropertySubscribe,
                                  PropertySubscribeResponse,
                                  PropertyNotify,
                                  ProcessInquiry,
                                  ProcessInquiryResponse,
                                  ProcessMidiMessageReport,
                                  ProcessMidiMessageReportResponse,
                                  ProcessEndMidiMessageReport>;

        Header header;
        Body body;

        bool operator== (const Parsed& other) const;
        bool operator!= (const Parsed& other) const;
    };
}

} // namespace juce::midi_ci

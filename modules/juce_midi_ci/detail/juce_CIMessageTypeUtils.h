/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce::midi_ci::detail::MessageTypeUtils
{

//==============================================================================
/*
    An interface used for types that want to operate on parsed MIDI-CI messages.

    @tags{Audio}
*/
struct MessageVisitor
{
    MessageVisitor() = default;
    MessageVisitor (const MessageVisitor&) = default;
    MessageVisitor (MessageVisitor&&) = default;
    MessageVisitor& operator= (const MessageVisitor&) = default;
    MessageVisitor& operator= (MessageVisitor&&) = default;
    virtual ~MessageVisitor() = default;

    virtual void visit (const std::monostate&)                                  const {}
    virtual void visit (const Message::Discovery&)                              const {}
    virtual void visit (const Message::EndpointInquiry&)                        const {}
    virtual void visit (const Message::ProfileInquiry&)                         const {}
    virtual void visit (const Message::ProfileDetails&)                         const {}
    virtual void visit (const Message::PropertyExchangeCapabilities&)           const {}
    virtual void visit (const Message::PropertyGetData&)                        const {}
    virtual void visit (const Message::PropertySetData&)                        const {}
    virtual void visit (const Message::PropertySubscribe&)                      const {}
    virtual void visit (const Message::ProcessInquiry&)                         const {}
    virtual void visit (const Message::ProcessMidiMessageReport&)               const {}
    virtual void visit (const Message::DiscoveryResponse&)                      const {}
    virtual void visit (const Message::EndpointInquiryResponse&)                const {}
    virtual void visit (const Message::InvalidateMUID&)                         const {}
    virtual void visit (const Message::ACK&)                                    const {}
    virtual void visit (const Message::NAK&)                                    const {}
    virtual void visit (const Message::ProfileInquiryResponse&)                 const {}
    virtual void visit (const Message::ProfileAdded&)                           const {}
    virtual void visit (const Message::ProfileRemoved&)                         const {}
    virtual void visit (const Message::ProfileDetailsResponse&)                 const {}
    virtual void visit (const Message::ProfileOn&)                              const {}
    virtual void visit (const Message::ProfileOff&)                             const {}
    virtual void visit (const Message::ProfileEnabledReport&)                   const {}
    virtual void visit (const Message::ProfileDisabledReport&)                  const {}
    virtual void visit (const Message::ProfileSpecificData&)                    const {}
    virtual void visit (const Message::PropertyExchangeCapabilitiesResponse&)   const {}
    virtual void visit (const Message::PropertyGetDataResponse&)                const {}
    virtual void visit (const Message::PropertySetDataResponse&)                const {}
    virtual void visit (const Message::PropertySubscribeResponse&)              const {}
    virtual void visit (const Message::PropertyNotify&)                         const {}
    virtual void visit (const Message::ProcessInquiryResponse&)                 const {}
    virtual void visit (const Message::ProcessMidiMessageReportResponse&)       const {}
    virtual void visit (const Message::ProcessEndMidiMessageReport&)            const {}
};

using ParseFn = Message::Parsed::Body (*) (Message::Generic, Parser::Status* status);
using VisitFn = void (*) (const Message::Parsed&, const MessageVisitor&);

/*  These return the Universal System Exclusive Sub-ID#2 for a particular message type. */
template <typename Specific>
static constexpr auto getParserFor (std::in_place_type_t<Specific>)
{
    return [] (Message::Generic message, Parser::Status* status) -> Message::Parsed::Body
    {
        // Parse messages using the version specified in the header of the message
        if (Specific parsed; Marshalling::Reader { message.data, static_cast<uint8_t> (message.header.version) } (parsed))
        {
            return parsed;
        }

        if (status != nullptr)
            *status = Parser::Status::malformed;

        return std::monostate{};
    };
}

template <typename Specific>
static constexpr auto getVisitorFor (std::in_place_type_t<Specific>)
{
    return [] (const Message::Parsed& parsed, const MessageVisitor& visitor)
    {
        if (auto* body = std::get_if<Specific> (&parsed.body))
            visitor.visit (*body);
    };
}

template <typename... Ts>
struct LookupTables
{
    constexpr LookupTables()
    {
        for (auto& x : parsers)
        {
            x = [] (Message::Generic, Parser::Status* status) -> Message::Parsed::Body
            {
                if (status != nullptr)
                    *status = Parser::Status::unrecognisedMessage;

                return std::monostate{};
            };
        }

        for (auto& x : visitors)
        {
            x = [] (const Message::Parsed&, const MessageVisitor& visitor)
            {
                visitor.visit (std::monostate{});
            };
        }

        (registerTag (std::in_place_type<Ts>), ...);
    }

    template <typename T>
    constexpr void registerTag (std::in_place_type_t<T> tag)
    {
        constexpr auto category = MessageMeta::Meta<T>::subID2;
        parsers[uint8_t (category)] = getParserFor (tag);
        visitors[uint8_t (category)] = getVisitorFor (tag);
    }

    ParseFn parsers[std::numeric_limits<uint8_t>::max()]{};
    VisitFn visitors[std::numeric_limits<uint8_t>::max()]{};
};

template <typename Body>
static void send (BufferOutput& output, uint8_t group, const Message::Header& header, const Body& body)
{
    output.getOutputBuffer().clear();
    Marshalling::Writer { output.getOutputBuffer() } (header, body);
    output.send (group);
}

template <typename Body>
static void send (BufferOutput& output, uint8_t group, MUID targetMuid, ChannelInGroup cig, const Body& body)
{
    Message::Header header
    {
        cig,
        MessageMeta::Meta<Body>::subID2,
        MessageMeta::implementationVersion,
        output.getMuid(),
        targetMuid,
    };

    send (output, group, header, body);
}


template <typename Body>
static void send (ResponderOutput& output, const Body& body)
{
    send (output, output.getIncomingGroup(), output.getReplyHeader (MessageMeta::Meta<Body>::subID2), body);
}

static void sendNAK (ResponderOutput& output, std::byte statusCode)
{
    const auto header = output.getReplyHeader (MessageMeta::Meta<Message::NAK>::subID2);
    const Message::NAK body { output.getIncomingHeader().category,
                              statusCode,
                              std::byte { 0x00 },
                              {},   // No additional details
                              {} }; // No message text
    send (output, output.getIncomingGroup(), header, body);
}

class BaseCaseDelegate : public ResponderDelegate
{
public:
    bool tryRespond (ResponderOutput& output, const Message::Parsed&) override
    {
        sendNAK (output, {});
        return true;
    }
};

static constexpr auto getTables()
{
    return LookupTables<Message::Discovery,
                        Message::DiscoveryResponse,
                        Message::InvalidateMUID,
                        Message::EndpointInquiry,
                        Message::EndpointInquiryResponse,
                        Message::ACK,
                        Message::NAK,
                        Message::ProfileInquiry,
                        Message::ProfileInquiryResponse,
                        Message::ProfileAdded,
                        Message::ProfileRemoved,
                        Message::ProfileDetails,
                        Message::ProfileDetailsResponse,
                        Message::ProfileOn,
                        Message::ProfileOff,
                        Message::ProfileEnabledReport,
                        Message::ProfileDisabledReport,
                        Message::ProfileSpecificData,
                        Message::PropertyExchangeCapabilities,
                        Message::PropertyExchangeCapabilitiesResponse,
                        Message::PropertyGetData,
                        Message::PropertyGetDataResponse,
                        Message::PropertySetData,
                        Message::PropertySetDataResponse,
                        Message::PropertySubscribe,
                        Message::PropertySubscribeResponse,
                        Message::PropertyNotify,
                        Message::ProcessInquiry,
                        Message::ProcessInquiryResponse,
                        Message::ProcessMidiMessageReport,
                        Message::ProcessMidiMessageReportResponse,
                        Message::ProcessEndMidiMessageReport>{};

}

static void visit (const Message::Parsed& msg, const MessageVisitor& visitor)
{
    constexpr auto tables = getTables();
    const auto fn = tables.visitors[(uint8_t) msg.header.category];
    fn (msg, visitor);
}

} // namespace juce::midi_ci::detail::MessageTypeUtils

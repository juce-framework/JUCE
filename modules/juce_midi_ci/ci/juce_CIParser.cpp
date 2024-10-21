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

std::optional<Message::Parsed> Parser::parse (Span<const std::byte> message, Status* status)
{
    const auto setStatus = [&] (Status s)
    {
        if (status != nullptr)
            *status = s;
    };

    setStatus (Status::noError);

    Message::Generic generic;

    if (! detail::Marshalling::Reader { message } (generic))
    {
        // Got a full sysex message, but it didn't contain a well-formed header.
        setStatus (Status::malformed);
        return {};
    }

    if ((generic.header.version & std::byte { 0x70 }) != std::byte{})
    {
        setStatus (Status::reservedVersion);
        return Message::Parsed { generic.header, std::monostate{} };
    }

    const auto index = (uint8_t) generic.header.category;
    constexpr auto tables = detail::MessageTypeUtils::getTables();
    const auto processFunction = tables.parsers[index];
    return Message::Parsed { generic.header, processFunction (generic, status) };
}

std::optional<Message::Parsed> Parser::parse (const MUID ourMUID,
                                              Span<const std::byte> message,
                                              Status* status)
{
    const auto setStatus = [&] (Status s)
    {
        if (status != nullptr)
            *status = s;
    };

    setStatus (Status::noError);

    if (const auto parsed = parse (message, status))
    {
        if (parsed->header.destination != MUID::getBroadcast() && parsed->header.destination != ourMUID)
            setStatus (Status::mismatchedMUID);
        else if (parsed->header.source == ourMUID)
            setStatus (Status::collidingMUID);
        else if ((parsed->header.version & std::byte { 0x70 }) != std::byte{})
            setStatus (Status::reservedVersion);

        return parsed;
    }

    return {};
}

class DescriptionVisitor : public detail::MessageTypeUtils::MessageVisitor
{
public:
    DescriptionVisitor (const Message::Parsed* m, String* str) : msg (m), result (str) {}

    void visit (const std::monostate&)                                        const override {}
    void visit (const Message::Discovery& body)                               const override { visitImpl (body); }
    void visit (const Message::DiscoveryResponse& body)                       const override { visitImpl (body); }
    void visit (const Message::InvalidateMUID& body)                          const override { visitImpl (body); }
    void visit (const Message::EndpointInquiry& body)                         const override { visitImpl (body); }
    void visit (const Message::EndpointInquiryResponse& body)                 const override { visitImpl (body); }
    void visit (const Message::ACK& body)                                     const override { visitImpl (body); }
    void visit (const Message::NAK& body)                                     const override { visitImpl (body); }
    void visit (const Message::ProfileInquiry& body)                          const override { visitImpl (body); }
    void visit (const Message::ProfileInquiryResponse& body)                  const override { visitImpl (body); }
    void visit (const Message::ProfileAdded& body)                            const override { visitImpl (body); }
    void visit (const Message::ProfileRemoved& body)                          const override { visitImpl (body); }
    void visit (const Message::ProfileDetails& body)                          const override { visitImpl (body); }
    void visit (const Message::ProfileDetailsResponse& body)                  const override { visitImpl (body); }
    void visit (const Message::ProfileOn& body)                               const override { visitImpl (body); }
    void visit (const Message::ProfileOff& body)                              const override { visitImpl (body); }
    void visit (const Message::ProfileEnabledReport& body)                    const override { visitImpl (body); }
    void visit (const Message::ProfileDisabledReport& body)                   const override { visitImpl (body); }
    void visit (const Message::ProfileSpecificData& body)                     const override { visitImpl (body); }
    void visit (const Message::PropertyExchangeCapabilities& body)            const override { visitImpl (body); }
    void visit (const Message::PropertyExchangeCapabilitiesResponse& body)    const override { visitImpl (body); }
    void visit (const Message::PropertyGetData& body)                         const override { visitImpl (body); }
    void visit (const Message::PropertyGetDataResponse& body)                 const override { visitImpl (body); }
    void visit (const Message::PropertySetData& body)                         const override { visitImpl (body); }
    void visit (const Message::PropertySetDataResponse& body)                 const override { visitImpl (body); }
    void visit (const Message::PropertySubscribe& body)                       const override { visitImpl (body); }
    void visit (const Message::PropertySubscribeResponse& body)               const override { visitImpl (body); }
    void visit (const Message::PropertyNotify& body)                          const override { visitImpl (body); }
    void visit (const Message::ProcessInquiry& body)                          const override { visitImpl (body); }
    void visit (const Message::ProcessInquiryResponse& body)                  const override { visitImpl (body); }
    void visit (const Message::ProcessMidiMessageReport& body)                const override { visitImpl (body); }
    void visit (const Message::ProcessMidiMessageReportResponse& body)        const override { visitImpl (body); }
    void visit (const Message::ProcessEndMidiMessageReport& body)             const override { visitImpl (body); }

private:
    static const char* getDescription (const Message::Discovery&)                            { return "Discovery"; }
    static const char* getDescription (const Message::DiscoveryResponse&)                    { return "Discovery Response"; }
    static const char* getDescription (const Message::InvalidateMUID&)                       { return "Invalidate MUID"; }
    static const char* getDescription (const Message::EndpointInquiry&)                      { return "Endpoint"; }
    static const char* getDescription (const Message::EndpointInquiryResponse&)              { return "Endpoint Response"; }
    static const char* getDescription (const Message::ACK&)                                  { return "ACK"; }
    static const char* getDescription (const Message::NAK&)                                  { return "NAK"; }
    static const char* getDescription (const Message::ProfileInquiry&)                       { return "Profile Inquiry"; }
    static const char* getDescription (const Message::ProfileInquiryResponse&)               { return "Profile Inquiry Response"; }
    static const char* getDescription (const Message::ProfileAdded&)                         { return "Profile Added"; }
    static const char* getDescription (const Message::ProfileRemoved&)                       { return "Profile Removed"; }
    static const char* getDescription (const Message::ProfileDetails&)                       { return "Profile Details"; }
    static const char* getDescription (const Message::ProfileDetailsResponse&)               { return "Profile Details Response"; }
    static const char* getDescription (const Message::ProfileOn&)                            { return "Profile On"; }
    static const char* getDescription (const Message::ProfileOff&)                           { return "Profile Off"; }
    static const char* getDescription (const Message::ProfileEnabledReport&)                 { return "Profile Enabled Report"; }
    static const char* getDescription (const Message::ProfileDisabledReport&)                { return "Profile Disabled Report"; }
    static const char* getDescription (const Message::ProfileSpecificData&)                  { return "Profile Specific Data"; }
    static const char* getDescription (const Message::PropertyExchangeCapabilities&)         { return "Property Exchange Capabilities"; }
    static const char* getDescription (const Message::PropertyExchangeCapabilitiesResponse&) { return "Property Exchange Capabilities Response"; }
    static const char* getDescription (const Message::PropertyGetData&)                      { return "Property Get Data"; }
    static const char* getDescription (const Message::PropertyGetDataResponse&)              { return "Property Get Data Response"; }
    static const char* getDescription (const Message::PropertySetData&)                      { return "Property Set Data"; }
    static const char* getDescription (const Message::PropertySetDataResponse&)              { return "Property Set Data Response"; }
    static const char* getDescription (const Message::PropertySubscribe&)                    { return "Property Subscribe"; }
    static const char* getDescription (const Message::PropertySubscribeResponse&)            { return "Property Subscribe Response"; }
    static const char* getDescription (const Message::PropertyNotify&)                       { return "Property Notify"; }
    static const char* getDescription (const Message::ProcessInquiry&)                       { return "Process Inquiry"; }
    static const char* getDescription (const Message::ProcessInquiryResponse&)               { return "Process Inquiry Response"; }
    static const char* getDescription (const Message::ProcessMidiMessageReport&)             { return "Process Midi Message Report"; }
    static const char* getDescription (const Message::ProcessMidiMessageReportResponse&)     { return "Process Midi Message Report Response"; }
    static const char* getDescription (const Message::ProcessEndMidiMessageReport&)          { return "Process End Midi Message Report"; }

    template <typename Body>
    void visitImpl (const Body& body) const
    {
        const auto opts = ToVarOptions{}.withExplicitVersion ((int) msg->header.version)
                                        .withVersionIncluded (false);
        auto json = ToVar::convert (body, opts);

        if (auto* obj = json->getDynamicObject(); obj != nullptr && obj->hasProperty ("header"))
        {
            const auto header = obj->getProperty ("header");
            const auto bytes = [&]() -> std::vector<std::byte>
            {
                const auto* arr = header.getArray();

                if (arr == nullptr)
                    return {};

                std::vector<std::byte> vec;
                vec.reserve ((size_t) arr->size());

                for (const auto& i : *arr)
                    vec.push_back ((std::byte) (int) i);

                return vec;
            }();

            obj->setProperty ("header", Encodings::jsonFrom7BitText (bytes));
        }

        if (json.has_value())
            *result = String (getDescription (body)) + ": " + JSON::toString (*json, JSON::FormatOptions{}.withSpacing (JSON::Spacing::none));
    }

    const Message::Parsed* msg = nullptr;
    String* result = nullptr;
};

String Parser::getMessageDescription (const Message::Parsed& message)
{
    String result { "!! Unrecognised !!" };
    detail::MessageTypeUtils::visit (message, DescriptionVisitor { &message, &result });
    return result;
}

//==============================================================================
//==============================================================================
#if JUCE_UNIT_TESTS

class ParserTests : public UnitTest
{
public:
    ParserTests() : UnitTest ("Parser", UnitTestCategories::midi) {}

    void runTest() override
    {
        auto random = getRandom();

        beginTest ("Sending an empty message does nothing");
        {
            const auto parsed = Parser::parse (MUID::makeRandom (random), {});
            expect (parsed == std::nullopt);
        }

        beginTest ("Sending a garbage message does nothing");
        {
            const std::vector<std::byte> bytes (128, std::byte { 0x70 });
            const auto parsed = Parser::parse (MUID::makeRandom (random), bytes);
            expect (parsed == std::nullopt);
        }

        beginTest ("Sending a message with truncated body produces a malformed status");
        {
            constexpr auto version1 = 0x01;
            const auto truncatedV1 = makeByteArray (0x7e,
                    /* to function block                 */ 0x7f,
                    /* midi CI                           */ 0x0d,
                    /* discovery message                 */ 0x70,
                    /* version                           */ version1,
                    /* source MUID                       */ 0x01,
                    /* ...                               */ 0x02,
                    /* ...                               */ 0x03,
                    /* ...                               */ 0x04,
                    /* broadcast MUID                    */ 0x7f,
                    /* ...                               */ 0x7f,
                    /* ...                               */ 0x7f,
                    /* ...                               */ 0x7f,
                    /* manufacturer                      */ 0x10,
                    /* ...                               */ 0x11,
                    /* ...                               */ 0x12,
                    /* family                            */ 0x20,
                    /* ...                               */ 0x21,
                    /* model                             */ 0x30,
                    /* ...                               */ 0x31,
                    /* revision                          */ 0x40,
                    /* ...                               */ 0x41,
                    /* ...                               */ 0x42,
                    /* ...                               */ 0x43,
                    /* CI category supported             */ 0x7f,
                    /* max sysex size                    */ 0x7f,
                    /* ...                               */ 0x7f,
                    /* ...                               */ 0x7f);
            /* Missing final byte for a version 1 message */
            Parser::Status status{};
            const auto parsedV1 = Parser::parse (MUID::makeRandom (random), truncatedV1, &status);

            expect (status == Parser::Status::malformed);
            expect (parsedV1 == Message::Parsed { Message::Header { ChannelInGroup::wholeBlock,
                                                                    std::byte { 0x70 },
                                                                    std::byte { version1 },
                                                                    MUID::makeUnchecked (0x80c101),
                                                                    MUID::getBroadcast() },
                                                  std::monostate{} });

            constexpr auto version2 = 0x02;
            const auto truncatedV2 = makeByteArray (0x7e,
                    /* to function block                 */ 0x7f,
                    /* midi CI                           */ 0x0d,
                    /* discovery message                 */ 0x70,
                    /* version                           */ version2,
                    /* source MUID                       */ 0x01,
                    /* ...                               */ 0x02,
                    /* ...                               */ 0x03,
                    /* ...                               */ 0x04,
                    /* broadcast MUID                    */ 0x7f,
                    /* ...                               */ 0x7f,
                    /* ...                               */ 0x7f,
                    /* ...                               */ 0x7f,
                    /* manufacturer                      */ 0x10,
                    /* ...                               */ 0x11,
                    /* ...                               */ 0x12,
                    /* family                            */ 0x20,
                    /* ...                               */ 0x21,
                    /* model                             */ 0x30,
                    /* ...                               */ 0x31,
                    /* revision                          */ 0x40,
                    /* ...                               */ 0x41,
                    /* ...                               */ 0x42,
                    /* ...                               */ 0x43,
                    /* CI category supported             */ 0x7f,
                    /* max sysex size                    */ 0x7f,
                    /* ...                               */ 0x7f,
                    /* ...                               */ 0x7f,
                    /* ...                               */ 0x7f);
            /* Missing final byte for a version 2 message */
            const auto parsedV2 = Parser::parse (MUID::makeRandom (random), truncatedV2);

            expect (status == Parser::Status::malformed);
            expect (parsedV2 == Message::Parsed { Message::Header { ChannelInGroup::wholeBlock,
                                                                    std::byte { 0x70 },
                                                                    std::byte { version2 },
                                                                    MUID::makeUnchecked (0x80c101),
                                                                    MUID::getBroadcast() },
                                                  std::monostate{} });
        }

        const auto getExpectedDiscoveryInput = [] (uint8_t version, uint8_t outputPathID)
        {
            return Message::Parsed { Message::Header { ChannelInGroup::wholeBlock,
                                                       std::byte { 0x70 },
                                                       std::byte { version },
                                                       MUID::makeUnchecked (0x80c101),
                                                       MUID::getBroadcast() },
                                     Message::Discovery { { { std::byte { 0x10 }, std::byte { 0x11 }, std::byte { 0x12 } },
                                                            { std::byte { 0x20 }, std::byte { 0x21 } },
                                                            { std::byte { 0x30 }, std::byte { 0x31 } },
                                                            { std::byte { 0x40 }, std::byte { 0x41 }, std::byte { 0x42 }, std::byte { 0x43 } } },
                                                          std::byte { 0x7f },
                                                          0xfffffff,
                                                          std::byte { outputPathID } } };
        };

        beginTest ("Sending a V1 discovery message notifies the input listener");
        {
            const auto initialMUID = MUID::makeRandom (random);
            constexpr uint8_t version = 0x01;

            const auto bytes = makeByteArray (0x7e,
                    /* to function block           */ 0x7f,
                    /* midi CI                     */ 0x0d,
                    /* discovery message           */ 0x70,
                    /* version                     */ version,
                    /* source MUID                 */ 0x01,
                    /* ...                         */ 0x02,
                    /* ...                         */ 0x03,
                    /* ...                         */ 0x04,
                    /* broadcast MUID              */ 0x7f,
                    /* ...                         */ 0x7f,
                    /* ...                         */ 0x7f,
                    /* ...                         */ 0x7f,
                    /* manufacturer                */ 0x10,
                    /* ...                         */ 0x11,
                    /* ...                         */ 0x12,
                    /* family                      */ 0x20,
                    /* ...                         */ 0x21,
                    /* model                       */ 0x30,
                    /* ...                         */ 0x31,
                    /* revision                    */ 0x40,
                    /* ...                         */ 0x41,
                    /* ...                         */ 0x42,
                    /* ...                         */ 0x43,
                    /* CI category supported       */ 0x7f,
                    /* max sysex size              */ 0x7f,
                    /* ...                         */ 0x7f,
                    /* ...                         */ 0x7f,
                    /* ...                         */ 0x7f);
            const auto parsed = Parser::parse (initialMUID, bytes);

            expect (parsed == getExpectedDiscoveryInput (version, 0));
        }

        beginTest ("Sending a V2 discovery message notifies the input listener");
        {
            constexpr uint8_t outputPathID = 5;
            const auto initialMUID = MUID::makeRandom (random);
            constexpr uint8_t version = 0x02;

            const auto bytes = makeByteArray (0x7e,
                    /* to function block           */ 0x7f,
                    /* midi CI                     */ 0x0d,
                    /* discovery message           */ 0x70,
                    /* version                     */ version,
                    /* source MUID                 */ 0x01,
                    /* ...                         */ 0x02,
                    /* ...                         */ 0x03,
                    /* ...                         */ 0x04,
                    /* broadcast MUID              */ 0x7f,
                    /* ...                         */ 0x7f,
                    /* ...                         */ 0x7f,
                    /* ...                         */ 0x7f,
                    /* manufacturer                */ 0x10,
                    /* ...                         */ 0x11,
                    /* ...                         */ 0x12,
                    /* family                      */ 0x20,
                    /* ...                         */ 0x21,
                    /* model                       */ 0x30,
                    /* ...                         */ 0x31,
                    /* revision                    */ 0x40,
                    /* ...                         */ 0x41,
                    /* ...                         */ 0x42,
                    /* ...                         */ 0x43,
                    /* CI category supported       */ 0x7f,
                    /* max sysex size              */ 0x7f,
                    /* ...                         */ 0x7f,
                    /* ...                         */ 0x7f,
                    /* ...                         */ 0x7f,
                    /* output path ID              */ outputPathID);
            const auto parsed = Parser::parse (initialMUID, bytes);

            expect (parsed == getExpectedDiscoveryInput (version, outputPathID));
        }

        beginTest ("Sending a discovery message with a future version notifies the input listener and ignores trailing fields");
        {
            constexpr uint8_t outputPathID = 10;
            const auto initialMUID = MUID::makeRandom (random);
            constexpr auto version = (uint8_t) detail::MessageMeta::implementationVersion + 1;

            const auto bytes = makeByteArray (0x7e,
                    /* to function block           */ 0x7f,
                    /* midi CI                     */ 0x0d,
                    /* discovery message           */ 0x70,
                    /* version                     */ version,
                    /* source MUID                 */ 0x01,
                    /* ...                         */ 0x02,
                    /* ...                         */ 0x03,
                    /* ...                         */ 0x04,
                    /* broadcast MUID              */ 0x7f,
                    /* ...                         */ 0x7f,
                    /* ...                         */ 0x7f,
                    /* ...                         */ 0x7f,
                    /* manufacturer                */ 0x10,
                    /* ...                         */ 0x11,
                    /* ...                         */ 0x12,
                    /* family                      */ 0x20,
                    /* ...                         */ 0x21,
                    /* model                       */ 0x30,
                    /* ...                         */ 0x31,
                    /* revision                    */ 0x40,
                    /* ...                         */ 0x41,
                    /* ...                         */ 0x42,
                    /* ...                         */ 0x43,
                    /* CI category supported       */ 0x7f,
                    /* max sysex size              */ 0x7f,
                    /* ...                         */ 0x7f,
                    /* ...                         */ 0x7f,
                    /* ...                         */ 0x7f,
                    /* output path ID              */ outputPathID,
                    /* extra bytes                 */ 0x00,
                    /* ...                         */ 0x00,
                    /* ...                         */ 0x00,
                    /* ...                         */ 0x00);
            const auto parsed = Parser::parse (initialMUID, bytes);

            expect (parsed == getExpectedDiscoveryInput (version, outputPathID));
        }
    }

private:
    template <typename... Ts>
    static std::array<std::byte, sizeof... (Ts)> makeByteArray (Ts&&... ts)
    {
        jassert (((0 <= (int) ts && (int) ts <= std::numeric_limits<uint8_t>::max()) && ...));
        return { std::byte (ts)... };
    }
};

static ParserTests parserTests;

#endif

} // namespace juce::midi_ci

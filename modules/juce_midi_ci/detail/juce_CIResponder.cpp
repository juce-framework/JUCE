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


namespace juce::midi_ci::detail
{

Parser::Status Responder::processCompleteMessage (BufferOutput& output,
                                                  ump::BytesOnGroup message,
                                                  Span<ResponderDelegate* const> listeners)
{
    auto status = Parser::Status::noError;
    const auto parsed = Parser::parse (output.getMuid(), message.bytes, &status);

    if (! parsed.has_value())
        return Parser::Status::malformed;

    class Output : public ResponderOutput
    {
    public:
        Output (BufferOutput& o, Message::Header h, uint8_t g)
            : innerOutput (o), header (h), group (g) {}

        MUID getMuid() const override { return innerOutput.getMuid(); }
        Message::Header getIncomingHeader() const override { return header; }
        uint8_t getIncomingGroup() const override { return group; }
        std::vector<std::byte>& getOutputBuffer() override { return innerOutput.getOutputBuffer(); }
        void send (uint8_t g) override { innerOutput.send (g); }

    private:
        BufferOutput& innerOutput;
        Message::Header header;
        uint8_t group{};
    };

    Output responderOutput { output, parsed->header, message.group };

    if (status != Parser::Status::noError)
    {
        switch (status)
        {
            case Parser::Status::collidingMUID:
            {
                const Message::Header header { ChannelInGroup::wholeBlock,
                                               MessageMeta::Meta<Message::InvalidateMUID>::subID2,
                                               MessageMeta::implementationVersion,
                                               output.getMuid(),
                                               MUID::getBroadcast() };
                const Message::InvalidateMUID body { output.getMuid() };
                MessageTypeUtils::send (responderOutput, responderOutput.getIncomingGroup(), header, body);
                break;
            }

            case Parser::Status::unrecognisedMessage:
                MessageTypeUtils::sendNAK (responderOutput, std::byte { 0x01 });
                break;

            case Parser::Status::reservedVersion:
                MessageTypeUtils::sendNAK (responderOutput, std::byte { 0x02 });
                break;

            case Parser::Status::malformed:
                MessageTypeUtils::sendNAK (responderOutput, std::byte { 0x41 });
                break;

            case Parser::Status::mismatchedMUID:
            case Parser::Status::noError:
                break;
        }

        return status;
    }

    for (auto* listener : listeners)
        if (listener != nullptr && listener->tryRespond (responderOutput, *parsed))
            return Parser::Status::noError;

    MessageTypeUtils::BaseCaseDelegate base;

    if (base.tryRespond (responderOutput, *parsed))
        return Parser::Status::noError;

    return Parser::Status::unrecognisedMessage;
}

//==============================================================================
//==============================================================================
#if JUCE_UNIT_TESTS

class ResponderTests : public UnitTest
{
public:
    ResponderTests() : UnitTest ("Responder", UnitTestCategories::midi) {}

    void runTest() override
    {
        auto random = getRandom();
        std::vector<std::byte> outgoing;

        const auto makeOutput = [&]
        {
            struct Output : public BufferOutput
            {
                Output (Random& r, std::vector<std::byte>& b)
                    : muid (MUID::makeRandom (r)), buf (b) {}

                MUID getMuid() const override { return muid; }
                std::vector<std::byte>& getOutputBuffer() override { return buf; }
                void send (uint8_t) override { sent.push_back (buf); }

                MUID muid;
                std::vector<std::byte>& buf;
                std::vector<std::vector<std::byte>> sent;
            };

            return Output { random, outgoing };
        };

        beginTest ("An endpoint message with a matching MUID provokes an endpoint response");
        {
            constexpr auto version = MessageMeta::implementationVersion;

            auto output = makeOutput();
            const auto initialMUID = output.getMuid();

            const auto bytes = makeByteArray (0x7e,
            /* to function block           */ 0x7f,
            /* midi CI                     */ 0x0d,
            /* endpoint message            */ 0x72,
            /* version                     */ version,
            /* source MUID                 */ 0x01,
            /* ...                         */ 0x02,
            /* ...                         */ 0x03,
            /* ...                         */ 0x04,
            /* destination MUID            */ (initialMUID.get() >> 0x00) & 0x7f,
            /* ...                         */ (initialMUID.get() >> 0x07) & 0x7f,
            /* ...                         */ (initialMUID.get() >> 0x0e) & 0x7f,
            /* ...                         */ (initialMUID.get() >> 0x15) & 0x7f,
            /* status, product instance ID */ 0x00);

            const Message::Parsed expectedInput { Message::Header { ChannelInGroup::wholeBlock,
                                                                    std::byte { 0x72 },
                                                                    version,
                                                                    MUID::makeUnchecked (0x80c101),
                                                                    initialMUID },
                                                  Message::EndpointInquiry { std::byte { 0x00 } } };
            EndpointResponderListener listener;
            processCompleteMessage (output, { 0, bytes }, listener);
            expect (listener == SilentResponderListener (expectedInput));

            const auto expectedOutputBytes = makeByteArray (0x7e,
            /* to function block                         */ 0x7f,
            /* midi CI                                   */ 0x0d,
            /* endpoint reply                            */ 0x73,
            /* version                                   */ version,
            /* source MUID                               */ (initialMUID.get() >> 0x00) & 0x7f,
            /* ...                                       */ (initialMUID.get() >> 0x07) & 0x7f,
            /* ...                                       */ (initialMUID.get() >> 0x0e) & 0x7f,
            /* ...                                       */ (initialMUID.get() >> 0x15) & 0x7f,
            /* destination MUID                          */ 0x01,
            /* ...                                       */ 0x02,
            /* ...                                       */ 0x03,
            /* ...                                       */ 0x04,
            /* status                                    */ 0x00,
            /* 16-bit length of following data           */ 0x04,
            /* ...                                       */ 0x00,
            /* info                                      */ 0x00,
            /* ...                                       */ 0x00,
            /* ...                                       */ 0x00,
            /* ...                                       */ 0x00);

            expect (rangesEqual (output.sent.front(), expectedOutputBytes));
        }

        beginTest ("An endpoint message directed at a different MUID does not provoke a response");
        {
            const auto destMUID = MUID::makeRandom (random);
            constexpr auto version = MessageMeta::implementationVersion;

            const auto bytes = makeByteArray (0x7e,
            /* to function block           */ 0x7f,
            /* midi CI                     */ 0x0d,
            /* endpoint message            */ 0x72,
            /* version                     */ version,
            /* source MUID                 */ 0x01,
            /* ...                         */ 0x02,
            /* ...                         */ 0x03,
            /* ...                         */ 0x04,
            /* destination MUID            */ (destMUID.get() >> 0x00) & 0x7f,
            /* ...                         */ (destMUID.get() >> 0x07) & 0x7f,
            /* ...                         */ (destMUID.get() >> 0x0e) & 0x7f,
            /* ...                         */ (destMUID.get() >> 0x15) & 0x7f,
            /* status, product instance ID */ 0x00);

            auto output = makeOutput();
            EndpointResponderListener listener;
            processCompleteMessage (output, { 0, bytes }, listener);
            expect (listener == SilentResponderListener());
            expect (output.sent.empty());
        }

        beginTest ("If the listener fails to compose an endpoint response, a NAK is emitted");
        {
            auto output = makeOutput();
            const auto initialMUID = output.getMuid();

            SilentResponderListener listener;
            constexpr auto version = MessageMeta::implementationVersion;

            const auto bytes = makeByteArray (0x7e,
            /* to function block           */ 0x7f,
            /* midi CI                     */ 0x0d,
            /* endpoint message            */ 0x72,
            /* version                     */ version,
            /* source MUID                 */ 0x01,
            /* ...                         */ 0x02,
            /* ...                         */ 0x03,
            /* ...                         */ 0x04,
            /* destination MUID            */ (initialMUID.get() >> 0x00) & 0x7f,
            /* ...                         */ (initialMUID.get() >> 0x07) & 0x7f,
            /* ...                         */ (initialMUID.get() >> 0x0e) & 0x7f,
            /* ...                         */ (initialMUID.get() >> 0x15) & 0x7f,
            /* status, product instance ID */ 0x00);

            processCompleteMessage (output, { 0, bytes }, listener);

            const auto expectedOutputBytes = makeByteArray (0x7e,
            /* to function block                         */ 0x7f,
            /* midi CI                                   */ 0x0d,
            /* nak                                       */ 0x7f,
            /* version                                   */ version,
            /* source MUID                               */ (initialMUID.get() >> 0x00) & 0x7f,
            /* ...                                       */ (initialMUID.get() >> 0x07) & 0x7f,
            /* ...                                       */ (initialMUID.get() >> 0x0e) & 0x7f,
            /* ...                                       */ (initialMUID.get() >> 0x15) & 0x7f,
            /* destination MUID                          */ 0x01,
            /* ...                                       */ 0x02,
            /* ...                                       */ 0x03,
            /* ...                                       */ 0x04,
            /* original transaction sub-id #2            */ 0x72,
            /* nak status code                           */ 0x00,
            /* nak status data                           */ 0x00,
            /* details                                   */ 0x00,
            /* ...                                       */ 0x00,
            /* ...                                       */ 0x00,
            /* ...                                       */ 0x00,
            /* ...                                       */ 0x00,
            /* message text length                       */ 0x00,
            /* ...                                       */ 0x00);
            expect (rangesEqual (output.sent.front(), expectedOutputBytes));
        }

        beginTest ("If a message is sent with reserved bits set in the Message Format Version, a NAK is emitted");
        {
            auto output = makeOutput();
            const auto initialMUID = output.getMuid();

            const auto bytes = makeByteArray (0x7e,
            /* to function block           */ 0x7f,
            /* midi CI                     */ 0x0d,
            /* endpoint message            */ 0x72,
            /* version,  reserved bit set  */ 0x12,
            /* source MUID                 */ 0x01,
            /* ...                         */ 0x02,
            /* ...                         */ 0x03,
            /* ...                         */ 0x04,
            /* destination MUID            */ (initialMUID.get() >> 0x00) & 0x7f,
            /* ...                         */ (initialMUID.get() >> 0x07) & 0x7f,
            /* ...                         */ (initialMUID.get() >> 0x0e) & 0x7f,
            /* ...                         */ (initialMUID.get() >> 0x15) & 0x7f,
            /* status, product instance ID */ 0x00);
            SilentResponderListener listener;
            processCompleteMessage (output, { 0, bytes }, listener);

            expect (listener == SilentResponderListener{});

            const auto expectedOutputBytes = makeByteArray (0x7e,
            /* to function block                         */ 0x7f,
            /* midi CI                                   */ 0x0d,
            /* nak                                       */ 0x7f,
            /* version                                   */ MessageMeta::implementationVersion,
            /* source MUID                               */ (initialMUID.get() >> 0x00) & 0x7f,
            /* ...                                       */ (initialMUID.get() >> 0x07) & 0x7f,
            /* ...                                       */ (initialMUID.get() >> 0x0e) & 0x7f,
            /* ...                                       */ (initialMUID.get() >> 0x15) & 0x7f,
            /* destination MUID                          */ 0x01,
            /* ...                                       */ 0x02,
            /* ...                                       */ 0x03,
            /* ...                                       */ 0x04,
            /* original transaction sub-id #2            */ 0x72,
            /* nak status code                           */ 0x02,
            /* nak status data                           */ 0x00,
            /* details                                   */ 0x00,
            /* ...                                       */ 0x00,
            /* ...                                       */ 0x00,
            /* ...                                       */ 0x00,
            /* ...                                       */ 0x00,
            /* message text length                       */ 0x00,
            /* ...                                       */ 0x00);

            expect (rangesEqual (output.sent.front(), expectedOutputBytes));
        }

        beginTest ("If the message body is malformed, a NAK with a status of 0x41 is emitted");
        {
            const auto sourceMUID = MUID::makeRandom (random);

            Message::Header header;
            header.deviceID = ChannelInGroup::wholeBlock;
            header.category = std::byte { 0x7e };
            header.version = MessageMeta::implementationVersion;
            header.source = sourceMUID;
            header.destination = MUID::getBroadcast();

            Message::InvalidateMUID invalidate;
            invalidate.target = MUID::makeRandom (random);

            std::vector<std::byte> message;
            Marshalling::Writer { message } (header, invalidate);

            // Remove a byte from the end of the message
            message.pop_back();

            auto output = makeOutput();
            const auto ourMUID = output.getMuid();

            SilentResponderListener listener;
            processCompleteMessage (output, { 0, message }, listener);

            const auto expectedOutputBytes = makeByteArray (0x7e,
            /* to function block                         */ 0x7f,
            /* midi CI                                   */ 0x0d,
            /* nak                                       */ 0x7f,
            /* version                                   */ MessageMeta::implementationVersion,
            /* source MUID                               */ (ourMUID.get() >> 0x00) & 0x7f,
            /* ...                                       */ (ourMUID.get() >> 0x07) & 0x7f,
            /* ...                                       */ (ourMUID.get() >> 0x0e) & 0x7f,
            /* ...                                       */ (ourMUID.get() >> 0x15) & 0x7f,
            /* destination MUID                          */ (sourceMUID.get() >> 0x00) & 0x7f,
            /* ...                                       */ (sourceMUID.get() >> 0x07) & 0x7f,
            /* ...                                       */ (sourceMUID.get() >> 0x0e) & 0x7f,
            /* ...                                       */ (sourceMUID.get() >> 0x15) & 0x7f,
            /* original transaction sub-id #2            */ 0x7e,
            /* nak status code                           */ 0x41,
            /* nak status data                           */ 0x00,
            /* details                                   */ 0x00,
            /* ...                                       */ 0x00,
            /* ...                                       */ 0x00,
            /* ...                                       */ 0x00,
            /* ...                                       */ 0x00,
            /* message text length                       */ 0x00,
            /* ...                                       */ 0x00);
            expect (rangesEqual (output.sent.front(), expectedOutputBytes));
        }

        beginTest ("If an unrecognised message is received, a NAK with a status of 0x01 is emitted");
        {
            const auto sourceMUID = MUID::makeRandom (random);

            Message::Header header;
            header.deviceID = ChannelInGroup::wholeBlock;
            header.category = std::byte { 0x50 }; // reserved
            header.version = MessageMeta::implementationVersion;
            header.source = sourceMUID;
            header.destination = MUID::getBroadcast();

            std::vector<std::byte> message;
            Marshalling::Writer { message } (header);
            message.emplace_back();

            auto output = makeOutput();
            const auto ourMUID = output.getMuid();

            SilentResponderListener listener;
            processCompleteMessage (output, { 0, message }, listener);

            const auto expectedOutputBytes = makeByteArray (0x7e,
            /* to function block                         */ 0x7f,
            /* midi CI                                   */ 0x0d,
            /* nak                                       */ 0x7f,
            /* version                                   */ MessageMeta::implementationVersion,
            /* source MUID                               */ (ourMUID.get() >> 0x00) & 0x7f,
            /* ...                                       */ (ourMUID.get() >> 0x07) & 0x7f,
            /* ...                                       */ (ourMUID.get() >> 0x0e) & 0x7f,
            /* ...                                       */ (ourMUID.get() >> 0x15) & 0x7f,
            /* destination MUID                          */ (sourceMUID.get() >> 0x00) & 0x7f,
            /* ...                                       */ (sourceMUID.get() >> 0x07) & 0x7f,
            /* ...                                       */ (sourceMUID.get() >> 0x0e) & 0x7f,
            /* ...                                       */ (sourceMUID.get() >> 0x15) & 0x7f,
            /* original transaction sub-id #2            */ 0x50,
            /* nak status code                           */ 0x01,
            /* nak status data                           */ 0x00,
            /* details                                   */ 0x00,
            /* ...                                       */ 0x00,
            /* ...                                       */ 0x00,
            /* ...                                       */ 0x00,
            /* ...                                       */ 0x00,
            /* message text length                       */ 0x00,
            /* ...                                       */ 0x00);
            expect (rangesEqual (output.sent.front(), expectedOutputBytes));
        }
    }

private:
    template <typename... Ts>
    static std::array<std::byte, sizeof... (Ts)> makeByteArray (Ts&&... ts)
    {
        jassert (((0 <= (int) ts && (int) ts <= std::numeric_limits<uint8_t>::max()) && ...));
        return { std::byte (ts)... };
    }

    struct SilentResponderListener : public ResponderDelegate
    {
        SilentResponderListener() = default;
        explicit SilentResponderListener (const Message::Parsed& p) : parsed (p) {}

        bool tryRespond (ResponderOutput&, const Message::Parsed& p) override
        {
            parsed = p;
            return false;
        }

        // Returning false indicates that the message was not handled
        bool operator== (const SilentResponderListener& other) const { return parsed == other.parsed; }
        bool operator!= (const SilentResponderListener& other) const { return ! operator== (other); }

        std::optional<Message::Parsed> parsed;
    };

    struct EndpointResponderListener : public SilentResponderListener
    {
        bool tryRespond (ResponderOutput& output, const Message::Parsed& message) override
        {
            parsed = message;

            if (std::holds_alternative<Message::EndpointInquiry> (message.body))
            {
                std::array<std::byte, 4> data{};
                Message::EndpointInquiryResponse response;
                response.status = std::byte{};
                response.data = data;

                MessageTypeUtils::send (output, output.getIncomingGroup(), output.getReplyHeader (std::byte { 0x73 }), response);
                return true;
            }

            return SilentResponderListener::tryRespond (output, message);

        }

        using SilentResponderListener::operator==, SilentResponderListener::operator!=;
    };

    struct OutputCallback
    {
        void operator() (Span<const std::byte> bytes)
        {
            output = std::vector<std::byte> (bytes.begin(), bytes.end());
        }

        std::vector<std::byte> output;
    };

    template <typename A, typename B>
    static bool rangesEqual (A&& a, B&& b)
    {
        using std::begin, std::end;
        return std::equal (begin (a), end (a), begin (b), end (b));
    }


    static Parser::Status processCompleteMessage (BufferOutput& output,
                                                  ump::BytesOnGroup message,
                                                  ResponderDelegate& listener)
    {
        ResponderDelegate* const listeners[] { &listener };
        return Responder::processCompleteMessage (output, message, listeners);
    }
};

static ResponderTests responderTests;

#endif

} // namespace juce::midi_ci::detail

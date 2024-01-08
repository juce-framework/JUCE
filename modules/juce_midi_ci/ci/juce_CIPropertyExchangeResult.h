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

namespace juce::midi_ci
{

/**
    Contains data returned by a responder in response to a request.

    PropertyExchangeResult::kind indicates whether the transaction resulted in
    a well-formed message; however, it's possible that the message is a
    well-formed message indicating an error in the responder, so it's important
    to check the 'status' field of the header before attempting to do anything
    with the payload.

    @tags{Audio}
*/
class PropertyExchangeResult
{
public:
    enum class Error
    {
        partial,                  ///< Got a response, but the responder terminated it before
                                  ///< sending a well-formed message.

        notify,                   ///< Got a notify message terminating the transaction.
        tooManyTransactions,      ///< Unable to send the request because doing so would
                                  ///< exceed the number of simultaneous inquiries that were declared.
                                  ///< @see PropertyDelegate::getNumSimultaneousRequestsSupported().
    };

    /** Creates a result denoting an error state. */
    explicit PropertyExchangeResult (Error errorIn)
        : PropertyExchangeResult (errorIn, {}, {}) {}

    /** Creates a result denoting a successful transmission. */
    PropertyExchangeResult (var headerIn, Span<const std::byte> bodyIn)
        : PropertyExchangeResult (std::nullopt, headerIn, bodyIn) {}

    /** Returns the result kind, either nullopt for a successful transmission, or
        an error code if something went wrong.
    */
    std::optional<Error> getError() const { return error; }

    /** Parses the header as a subscription header.

        This may only be called for messages of kind 'full'.
    */
    PropertySubscriptionHeader getHeaderAsSubscriptionHeader() const
    {
        jassert (header != var());
        return PropertySubscriptionHeader::parseCondensed (header);
    }

    /** Parses the header as a request header.

        This may only be called for messages of kind 'full'.
    */
    PropertyRequestHeader getHeaderAsRequestHeader() const
    {
        jassert (header != var());
        return PropertyRequestHeader::parseCondensed (header);
    }

    /** Parses the header as a reply header.

        This may only be called for messages of kind 'full'.
    */
    PropertyReplyHeader getHeaderAsReplyHeader() const
    {
        jassert (header != var());
        return PropertyReplyHeader::parseCondensed (header);
    }

    /** When getKind returns 'full', this is the message payload.

        Note that this is not stored internally; if you need to keep this data
        around and reference it in the future, you should copy it into a
        vector or some other suitable container.
    */
    Span<const std::byte> getBody() const { return body; }

private:
    PropertyExchangeResult (std::optional<Error> errorIn, var headerIn, Span<const std::byte> bodyIn)
        : error (errorIn), header (headerIn), body (bodyIn) {}

    std::optional<Error> error;
    var header;
    Span<const std::byte> body;
};

} // namespace juce::midi_ci

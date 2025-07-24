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

#define JUCE_SUBSCRIPTION_COMMANDS X(start) X(partial) X(full) X(notify) X(end)

/**
    Kinds of command that may be sent as part of a subscription update.

    Check the Property Exchange specification to find the meaning of the
    different kinds.

    @tags{Audio}
*/
enum class PropertySubscriptionCommand
{
   #define X(str) str,
    JUCE_SUBSCRIPTION_COMMANDS
   #undef X
};

/**
    Functions to use with PropertySubscriptionCommand.

    @tags{Audio}
*/
struct PropertySubscriptionCommandUtils
{
    PropertySubscriptionCommandUtils() = delete;

    /** Converts a command to a human-readable string. */
    static const char* toString (PropertySubscriptionCommand x)
    {
        switch (x)
        {
           #define X(str) case PropertySubscriptionCommand::str: return #str;
            JUCE_SUBSCRIPTION_COMMANDS
           #undef X
        }

        return nullptr;
    }

    /** Converts a command string from a property exchange JSON header to
        an PropertySubscriptionCommand.
    */
    static std::optional<PropertySubscriptionCommand> toCommand (const char* str)
    {
       #define X(name) if (std::string_view (str) == std::string_view (#name)) return PropertySubscriptionCommand::name;
        JUCE_SUBSCRIPTION_COMMANDS
       #undef X

        return {};
    }
};

#undef JUCE_SUBSCRIPTION_COMMANDS

/**
    A struct containing data members that correspond to common fields in a
    property subscription header.

    Check the Property Exchange specification to find the meaning of the
    different fields.

    @tags{Audio}
*/
struct PropertySubscriptionHeader
{
    String resource;
    String resId;
    Encoding mutualEncoding = Encoding::ascii;
    String mediaType = "application/json";
    PropertySubscriptionCommand command { -1 };
    String subscribeId;
    std::map<Identifier, var> extended;

    /** Converts a JSON object to a PropertyRequestHeader.

        Unspecified fields will use their default values.
    */
    static PropertySubscriptionHeader parseCondensed (const var&);

    /** Converts a PropertySubscriptionHeader to a JSON object suitable for use as
        a MIDI-CI message header after conversion to 7-bit ASCII.
    */
    var toVarCondensed() const;
};

/**
    Contains information about the pagination of a request.

    Check the Property Exchange specification to find the meaning of the
    different fields.

    @tags{Audio}
*/
struct Pagination
{
    int offset = 0;
    int limit = 1;
};

/**
    A struct containing data members that correspond to common fields in a
    property request header.

    Check the Property Exchange specification to find the meaning of the
    different fields.

    @tags{Audio}
*/
struct PropertyRequestHeader
{
    String resource;
    String resId;
    Encoding mutualEncoding = Encoding::ascii;
    String mediaType = "application/json";
    bool setPartial = false;
    std::optional<Pagination> pagination;
    std::map<Identifier, var> extended;

    /** Converts a JSON object to a PropertyRequestHeader.

        Unspecified fields will use their default values.
    */
    static PropertyRequestHeader parseCondensed (const var&);

    /** Converts a PropertyRequestHeader to a JSON object suitable for use as
        a MIDI-CI message header after conversion to 7-bit ASCII.
    */
    var toVarCondensed() const;
};

/**
    Bundles together a property request header and a data payload.

    @tags{Audio}
*/
struct PropertyRequestData
{
    PropertyRequestHeader header;
    Span<const std::byte> body;
};

/**
    A struct containing data members that correspond to common fields in a
    reply to a property exchange request.

    Check the Property Exchange specification to find the meaning of the
    different fields.

    For extended attributes that don't correspond to any of the defined data
    members, use the 'extended' map.

    @tags{Audio}
*/
struct PropertyReplyHeader
{
    int status = 200;
    String message;
    Encoding mutualEncoding = Encoding::ascii;
    int cacheTime = 0;
    String mediaType = "application/json";
    std::map<Identifier, var> extended;

    /** Converts a JSON object to a PropertyReplyHeader.

        Unspecified fields will use their default values.
    */
    static PropertyReplyHeader parseCondensed (const var&);

    /** Converts a PropertyReplyHeader to a JSON object suitable for use as
        a MIDI-CI message header after conversion to 7-bit ASCII.
    */
    var toVarCondensed() const;
};

/**
    Bundles together a property reply header and a data payload.

    @tags{Audio}
*/
struct PropertyReplyData
{
    PropertyReplyHeader header;
    std::vector<std::byte> body;
};

/**
    An interface with methods that can be overridden to customise how a Device
    implementing properties responds to property inquiries.

    @tags{Audio}
*/
struct PropertyDelegate
{
    PropertyDelegate() = default;
    virtual ~PropertyDelegate() = default;
    PropertyDelegate (const PropertyDelegate&) = default;
    PropertyDelegate (PropertyDelegate&&) = default;
    PropertyDelegate& operator= (const PropertyDelegate&) = default;
    PropertyDelegate& operator= (PropertyDelegate&&) = default;

    /** Returns the max number of simultaneous property exchange messages that can be processed. */
    virtual uint8_t getNumSimultaneousRequestsSupported() const { return 127; }

    /** Returns a header/body containing the requested data.
        To report an error, you can return a failure status code in the header and leave the body empty.
    */
    virtual PropertyReplyData propertyGetDataRequested (MUID, const PropertyRequestHeader&) = 0;

    /** Returns a header that describes the result of the set operation. */
    virtual PropertyReplyHeader propertySetDataRequested (MUID, const PropertyRequestData&) = 0;

    /** Returns true to allow the subscription, or false otherwise. */
    virtual bool subscriptionStartRequested (MUID, const PropertySubscriptionHeader&) = 0;

    /** Called with the corresponding subscription token after a subscription has started. */
    virtual void subscriptionDidStart (MUID, const String& subId, const PropertySubscriptionHeader&) = 0;

    /** Called when a device requests for an ongoing subscription to end. */
    virtual void subscriptionWillEnd (MUID, const Subscription& sub) = 0;
};

} // namespace juce::midi_ci

/** @cond */
namespace juce
{

template <>
struct SerialisationTraits<midi_ci::PropertySubscriptionCommand>
{
    static constexpr auto marshallingVersion = std::nullopt;

    template <typename Archive>
    void load (Archive& archive, midi_ci::PropertySubscriptionCommand& t)
    {
        String command;
        archive (command);
        t = midi_ci::PropertySubscriptionCommandUtils::toCommand (command.toRawUTF8()).value_or (midi_ci::PropertySubscriptionCommand{});
    }

    template <typename Archive>
    void save (Archive& archive, const midi_ci::PropertySubscriptionCommand& t)
    {
        archive (midi_ci::PropertySubscriptionCommandUtils::toString (t));
    }
};

} // namespace juce
/** @endcond */

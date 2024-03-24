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

struct PropertyDelegateDetail
{
    /*
        Note: We don't use ToVar and FromVar here, because we want to omit fields that are using
        their default values.
    */

    template <typename Target>
    static Target parseTargetHeader (const var& v,
                                     const std::map<Identifier, void (*) (Target&, const var&)>& parsers)
    {
        Target target;

        if (auto* obj = v.getDynamicObject())
        {
            for (const auto& pair : obj->getProperties())
            {
                const auto parserIter = parsers.find (pair.name);

                if (parserIter != parsers.end())
                    parserIter->second (target, pair.value);
                else
                    target.extended[pair.name] = pair.value;
            }
        }

        return target;
    }

    static auto getParsersForPropertyReplyHeader()
    {
        using Target = PropertyReplyHeader;
        std::map<Identifier, void (*) (Target& header, const var& v)> map;

        map.emplace ("status",         [] (Target& header, const var& v) { header.status      = v; });
        map.emplace ("message",        [] (Target& header, const var& v) { header.message     = v; });
        map.emplace ("cacheTime",      [] (Target& header, const var& v) { header.cacheTime   = v; });
        map.emplace ("mediaType",      [] (Target& header, const var& v) { header.mediaType   = v; });
        map.emplace ("mutualEncoding", [] (Target& header, const var& v)
        {
            header.mutualEncoding = EncodingUtils::toEncoding (v.toString().toRawUTF8()).value_or (Encoding::ascii);
        });

        return map;
    }

    template <typename Target>
    static auto getParsersForGenericPropertyRequestHeader()
    {
        std::map<Identifier, void (*) (Target& header, const var& v)> map;

        map.emplace ("resource",       [] (Target& header, const var& v) { header.resource    = v; });
        map.emplace ("resId",          [] (Target& header, const var& v) { header.resId       = v; });
        map.emplace ("mediaType",      [] (Target& header, const var& v) { header.mediaType   = v; });
        map.emplace ("mutualEncoding", [] (Target& header, const var& v)
        {
            header.mutualEncoding = EncodingUtils::toEncoding (v.toString().toRawUTF8()).value_or (Encoding::ascii);
        });

        return map;
    }

    static auto getParsersForPropertyRequestHeader()
    {
        auto map = getParsersForGenericPropertyRequestHeader<PropertyRequestHeader>();
        map.emplace ("setPartial", [] (PropertyRequestHeader& header, const var& v) { header.setPartial = v; });
        map.emplace ("offset",     [] (PropertyRequestHeader& header, const var& v)
        {
            if (! header.pagination.has_value())
                header.pagination = Pagination{};

            header.pagination->offset = v;
        });
        map.emplace ("limit",      [] (PropertyRequestHeader& header, const var& v)
        {
            if (! header.pagination.has_value())
                header.pagination = Pagination{};

            header.pagination->limit = v;
        });
        return map;
    }

    static auto getParsersForPropertySubscriptionHeader()
    {
        auto map = getParsersForGenericPropertyRequestHeader<PropertySubscriptionHeader>();

        map.emplace ("subscribeId", [] (PropertySubscriptionHeader& header, const var& v) { header.subscribeId = v; });
        map.emplace ("command",     [] (PropertySubscriptionHeader& header, const var& v)
        {
            header.command = [&]
            {
                if (v == "start")
                    return PropertySubscriptionCommand::start;

                if (v == "partial")
                    return PropertySubscriptionCommand::partial;

                if (v == "full")
                    return PropertySubscriptionCommand::full;

                if (v == "notify")
                    return PropertySubscriptionCommand::notify;

                if (v == "end")
                    return PropertySubscriptionCommand::end;

                return PropertySubscriptionCommand::notify;
            }();
        });

        return map;
    }

    static auto getSetPartial (const PropertySubscriptionHeader&) { return false; }
    static auto getSetPartial (const PropertyRequestHeader& h) { return h.setPartial; }
    static auto getSetPartial (const PropertyReplyHeader&) { return false; }

    static auto getPagination (const PropertySubscriptionHeader&) { return std::optional<Pagination>{}; }
    static auto getPagination (const PropertyRequestHeader& h) { return h.pagination; }
    static auto getPagination (const PropertyReplyHeader&) { return std::optional<Pagination>{}; }

    static auto getCacheTime (const PropertySubscriptionHeader&) { return 0; }
    static auto getCacheTime (const PropertyRequestHeader&) { return 0; }
    static auto getCacheTime (const PropertyReplyHeader& h) { return h.cacheTime; }

    static auto getMessage (const PropertySubscriptionHeader&) { return String{}; }
    static auto getMessage (const PropertyRequestHeader&) { return String{}; }
    static auto getMessage (const PropertyReplyHeader& h) { return h.message; }

    static auto getResource (const PropertySubscriptionHeader& h) { return h.resource; }
    static auto getResource (const PropertyRequestHeader& h) { return h.resource; }
    static auto getResource (const PropertyReplyHeader&) { return String{}; }

    static auto getResId (const PropertySubscriptionHeader& h) { return h.resId; }
    static auto getResId (const PropertyRequestHeader& h) { return h.resId; }
    static auto getResId (const PropertyReplyHeader&) { return String{}; }

    static auto getCommand (const PropertySubscriptionHeader& h) { return h.command; }
    static auto getCommand (const PropertyRequestHeader&) { return PropertySubscriptionCommand{}; }
    static auto getCommand (const PropertyReplyHeader&) { return PropertySubscriptionCommand{}; }

    static auto getSubscribeId (const PropertySubscriptionHeader& h) { return h.subscribeId; }
    static auto getSubscribeId (const PropertyRequestHeader&) { return String{}; }
    static auto getSubscribeId (const PropertyReplyHeader&) { return String{}; }

    static std::optional<int> getStatus (const PropertySubscriptionHeader&) { return {}; }
    static std::optional<int> getStatus (const PropertyRequestHeader&) { return {}; }
    static std::optional<int> getStatus (const PropertyReplyHeader& h) { return h.status; }

    template <typename T>
    static auto toFieldsFromHeader (const T& t)
    {
        auto fields = t.extended;

        // Status shall always be included if it is present in the header
        if (const auto status = getStatus (t))
            fields["status"] = *status;

        if (getResource (t) != getResource (T()))
            fields["resource"] = getResource (t);

        if (getCommand (t) != getCommand (T()))
            fields["command"] = PropertySubscriptionCommandUtils::toString (getCommand (t));

        if (getSubscribeId (t) != getSubscribeId (T()))
            fields["subscribeId"] = getSubscribeId (t);

        if (getResId (t) != getResId (T()))
            fields["resId"] = getResId (t);

        if (t.mutualEncoding != T().mutualEncoding)
            fields["mutualEncoding"] = EncodingUtils::toString (t.mutualEncoding);

        if (t.mediaType != T().mediaType)
            fields["mediaType"] = t.mediaType;

        if (getSetPartial (t))
            fields["setPartial"] = true;

        if (getCacheTime (t) != getCacheTime (T()))
            fields["cacheTime"] = getCacheTime (t);

        if (getMessage (t) != getMessage (T()))
            fields["message"] = getMessage (t);

        if (const auto pagination = getPagination (t))
        {
            fields["offset"] = pagination->offset;
            fields["limit"]  = pagination->limit;
        }

        return fields;
    }
};

//==============================================================================
PropertySubscriptionHeader PropertySubscriptionHeader::parseCondensed (const var& v)
{
    return PropertyDelegateDetail::parseTargetHeader (v, PropertyDelegateDetail::getParsersForPropertySubscriptionHeader());
}

var PropertySubscriptionHeader::toVarCondensed() const
{
    return JSONUtils::makeObjectWithKeyFirst (PropertyDelegateDetail::toFieldsFromHeader (*this), "command");
}

PropertyRequestHeader PropertyRequestHeader::parseCondensed (const var& v)
{
    return PropertyDelegateDetail::parseTargetHeader (v, PropertyDelegateDetail::getParsersForPropertyRequestHeader());
}

var PropertyRequestHeader::toVarCondensed() const
{
    return JSONUtils::makeObjectWithKeyFirst (PropertyDelegateDetail::toFieldsFromHeader (*this), "resource");
}

PropertyReplyHeader PropertyReplyHeader::parseCondensed (const var& v)
{
    return PropertyDelegateDetail::parseTargetHeader (v, PropertyDelegateDetail::getParsersForPropertyReplyHeader());
}

var PropertyReplyHeader::toVarCondensed() const
{
    return JSONUtils::makeObjectWithKeyFirst (PropertyDelegateDetail::toFieldsFromHeader (*this), "status");
}

} // namespace juce::midi_ci

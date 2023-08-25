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

class PropertyHost::Visitor : public detail::MessageTypeUtils::MessageVisitor
{
public:
    Visitor (PropertyHost* h, ResponderOutput* o, bool* b)
        : host (h), output (o), handled (b) {}

    void visit (const Message::PropertyExchangeCapabilities& body)  const override { visitImpl (body); }
    void visit (const Message::PropertyGetData& body)               const override { visitImpl (body); }
    void visit (const Message::PropertySetData& body)               const override { visitImpl (body); }
    void visit (const Message::PropertySubscribe& body)             const override { visitImpl (body); }
    void visit (const Message::PropertyNotify& body)                const override { visitImpl (body); }
    using MessageVisitor::visit;

private:
    template <typename Body>
    void visitImpl (const Body& body) const { *handled = messageReceived (body); }

    bool messageReceived (const Message::PropertyExchangeCapabilities&) const
    {
        detail::MessageTypeUtils::send (*output, Message::PropertyExchangeCapabilitiesResponse { std::byte { host->delegate.getNumSimultaneousRequestsSupported() },
                                                                                                 {},
                                                                                                 {} });
        return true;
    }

    bool messageReceived (const Message::PropertyGetData& data) const
    {
        // This should always be a single message, so no need to accumulate chunks
        const auto reply = host->delegate.propertyGetDataRequested (output->getIncomingHeader().source,
                                                                    PropertyRequestHeader::parseCondensed (Encodings::jsonFrom7BitText (data.header)));

        const auto encoded = Encodings::tryEncode (reply.body, reply.header.mutualEncoding);

        if (! encoded.has_value())
        {
            // If this is hit, the data that was supplied isn't valid for the encoding that was specified
            jassertfalse;
            return false;
        }

        detail::PropertyHostUtils::send (*output,
                                         output->getIncomingGroup(),
                                         detail::MessageMeta::Meta<Message::PropertyGetDataResponse>::subID2,
                                         output->getIncomingHeader().source,
                                         data.requestID,
                                         Encodings::jsonTo7BitText (reply.header.toVarCondensed()),
                                         *encoded,
                                         host->cacheProvider.getMaxSysexSizeForMuid (output->getIncomingHeader().source));

        return true;
    }

    bool messageReceived (const Message::PropertySetData& data) const
    {
        auto* caches = host->cacheProvider.getCacheForMuidAsResponder (output->getIncomingHeader().source);

        if (caches == nullptr)
            return false;

        const auto source = output->getIncomingHeader().source;
        const auto dest = output->getIncomingHeader().destination;
        const auto group = output->getIncomingGroup();
        const auto request = data.requestID;
        caches->primeCache (host->delegate.getNumSimultaneousRequestsSupported(), [this, source, dest, group, request] (const PropertyExchangeResult& result)
        {
            const auto send = [&] (const PropertyReplyHeader& header)
            {
                detail::MessageTypeUtils::send (host->output,
                                                group,
                                                Message::Header { ChannelInGroup::wholeBlock,
                                                                  detail::MessageMeta::Meta<Message::PropertySetDataResponse>::subID2,
                                                                  detail::MessageMeta::implementationVersion,
                                                                  dest,
                                                                  source },
                                                Message::PropertySetDataResponse { { request, Encodings::jsonTo7BitText (header.toVarCondensed()) } });
            };

            if (result.getError() == PropertyExchangeResult::Error::tooManyTransactions)
            {
                PropertyReplyHeader header;
                header.status = 343;
                header.message = TRANS ("The device has initiated too many simultaneous requests");
                send (header);
                return;
            }

            if (result.getError().has_value())
            {
                PropertyReplyHeader header;
                header.status = 400;
                header.message = TRANS ("Request was incomplete");
                send (header);
                return;
            }

            send (host->delegate.propertySetDataRequested (source, { result.getHeaderAsRequestHeader(), result.getBody() }));
        }, request);

        caches->addChunk (data.requestID, data);

        return true;
    }

    bool messageReceived (const Message::PropertySubscribe& data) const
    {
        auto* caches = host->cacheProvider.getCacheForMuidAsResponder (output->getIncomingHeader().source);

        if (caches == nullptr)
            return false;

        if (data.header.empty() || data.thisChunkNum != 1 || data.totalNumChunks != 1)
            return false;

        const auto subHeader = PropertySubscriptionHeader::parseCondensed (Encodings::jsonFrom7BitText (data.header));
        const auto tryNotifyInitiator = subHeader.command == PropertySubscriptionCommand::start
                                        || subHeader.command == PropertySubscriptionCommand::end;

        if (! tryNotifyInitiator)
            return false;

        const auto source = output->getIncomingHeader().source;

        const auto sendResponse = [&] (const PropertyReplyHeader& header)
        {
            detail::PropertyHostUtils::send (*output,
                                             output->getIncomingGroup(),
                                             detail::MessageMeta::Meta<Message::PropertySubscribeResponse>::subID2,
                                             source,
                                             data.requestID,
                                             Encodings::jsonTo7BitText (header.toVarCondensed()),
                                             {},
                                             host->cacheProvider.getMaxSysexSizeForMuid (source));
        };

        if (subHeader.command == PropertySubscriptionCommand::start)
        {
            if (host->delegate.subscriptionStartRequested (source, subHeader))
            {
                auto& currentSubscribeIds = host->registry[source];
                const auto newToken = findUnusedSubscribeId (currentSubscribeIds);
                [[maybe_unused]] const auto pair = currentSubscribeIds.emplace (newToken, subHeader.resource);
                jassert (pair.second);
                const auto subscribeId = subscribeIdFromUid (newToken);
                host->delegate.subscriptionDidStart (source, subscribeId, subHeader);

                PropertyReplyHeader header;
                header.extended["subscribeId"] = subscribeId;
                sendResponse (header);
            }
            else
            {
                PropertyReplyHeader header;
                header.status = 405;
                sendResponse (header);
            }

            return true;
        }

        if (subHeader.command == PropertySubscriptionCommand::end)
        {
            const auto token = uidFromSubscribeId (subHeader.subscribeId);
            auto& currentSubscribeIds = host->registry[source];
            const auto iter = currentSubscribeIds.find (token);

            if (iter != currentSubscribeIds.end())
            {
                host->delegate.subscriptionWillEnd (source, { subHeader.subscribeId, iter->second });
                currentSubscribeIds.erase (iter);

                sendResponse ({});
                return true;
            }

            return false;
        }

        return false;
    }

    bool messageReceived (const Message::PropertyNotify& n) const
    {
        const auto m = output->getIncomingHeader().source;

        if (auto* it = host->cacheProvider.getCacheForMuidAsResponder (m))
            it->notify (n.requestID, n.header);

        if (auto* it = host->cacheProvider.getCacheForMuidAsInitiator (m))
            it->notify (n.requestID, n.header);

        return true;
    }

    PropertyHost* host = nullptr;
    ResponderOutput* output = nullptr;
    bool* handled = nullptr;
};

//==============================================================================
std::set<Subscription> PropertyHost::findSubscriptionsForDevice (MUID device) const
{
    const auto iter = registry.find (device);

    if (iter == registry.end())
        return {};

    std::set<Subscription> result;

    for (const auto& [subId, resource] : iter->second)
    {
        [[maybe_unused]] const auto pair = result.insert ({ subscribeIdFromUid (subId), resource });
        jassert (pair.second);
    }

    return result;
}

int PropertyHost::countOngoingTransactions() const
{
    const auto muids = cacheProvider.getDiscoveredMuids();

    return std::accumulate (muids.begin(), muids.end(), 0, [&] (auto acc, const auto& m)
    {
        if (auto* cache = cacheProvider.getCacheForMuidAsResponder (m))
            return acc + cache->countOngoingTransactions();

        return acc;
    });
}

bool PropertyHost::tryRespond (ResponderOutput& responderOutput, const Message::Parsed& message)
{
    bool result = false;
    detail::MessageTypeUtils::visit (message, Visitor { this, &responderOutput, &result });
    return result;
}

ErasedScopeGuard PropertyHost::sendSubscriptionUpdate (MUID device,
                                                       const PropertySubscriptionHeader& header,
                                                       Span<const std::byte> body,
                                                       std::function<void (const PropertyExchangeResult&)> cb)
{
    const auto deviceIter = registry.find (device);

    if (deviceIter == registry.end())
    {
        // That device doesn't have any active subscriptions
        jassertfalse;
        return {};
    }

    const auto uid = uidFromSubscribeId (header.subscribeId);
    const auto subIter = deviceIter->second.find (uid);

    if (subIter == deviceIter->second.end())
    {
        // That subscribeId isn't currently in use by that device
        jassertfalse;
        return {};
    }

    const auto resource = subIter->second;

    if (header.resource != resource)
    {
        // That subscribeId corresponds to a different resource
        jassertfalse;
        return {};
    }

    if (header.command == PropertySubscriptionCommand::start)
    {
        // This function is intended to update ongoing subscriptions. To start a new subscription,
        // use CIDevice.
        jassertfalse;
        return {};
    }

    auto* caches = cacheProvider.getCacheForMuidAsInitiator (device);

    if (caches == nullptr)
        return {};

    const auto terminator = detail::PropertyHostUtils::getTerminator (output, functionBlock, device);
    auto wrappedCallback = [&]() -> std::function<void (const PropertyExchangeResult&)>
    {
        if (header.command != PropertySubscriptionCommand::end)
            return cb;

        return [this, device, uid, resource, cb] (const PropertyExchangeResult& result)
        {
            if (! result.getError().has_value())
            {
                delegate.subscriptionWillEnd (device, { subscribeIdFromUid (uid), resource });
                registry[device].erase (uid);
            }

            NullCheckedInvocation::invoke (cb, result);
        };
    }();

    const auto encoded = Encodings::tryEncode (body, header.mutualEncoding);

    if (! encoded.has_value())
    {
        NullCheckedInvocation::invoke (wrappedCallback, PropertyExchangeResult { PropertyExchangeResult::Error::invalidPayload });
        return {};
    }

    auto primed = caches->primeCache (delegate.getNumSimultaneousRequestsSupported(),
                                      std::move (wrappedCallback),
                                      std::move (terminator));

    if (! primed.isValid())
        return {};

    detail::PropertyHostUtils::send (output,
                                     functionBlock.firstGroup,
                                     detail::MessageMeta::Meta<Message::PropertySubscribe>::subID2,
                                     device,
                                     primed.id,
                                     Encodings::jsonTo7BitText (header.toVarCondensed()),
                                     *encoded,
                                     cacheProvider.getMaxSysexSizeForMuid (device));

    return std::move (primed.token);
}

void PropertyHost::terminateSubscription (MUID device, const String& subscribeId)
{
    const auto deviceIter = registry.find (device);

    if (deviceIter == registry.end())
    {
        // That device doesn't have any active subscriptions
        jassertfalse;
        return;
    }

    const auto uid = uidFromSubscribeId (subscribeId);
    const auto subIter = deviceIter->second.find (uid);

    if (subIter == deviceIter->second.end())
    {
        // That subscribeId isn't currently in use by that device
        jassertfalse;
        return;
    }

    PropertySubscriptionHeader header;
    header.command = PropertySubscriptionCommand::end;
    header.subscribeId = subscribeId;
    header.resource = subIter->second;

    sendSubscriptionUpdate (device, header, {}, nullptr).release();
}

PropertyHost::SubscriptionToken PropertyHost::uidFromSubscribeId (String id)
{
    try
    {
        // from_chars would be better once we no longer need to support older macOS
        return { (size_t) std::stoull (id.toStdString(), {}, 36) };
    }
    catch (...) {}

    jassertfalse;
    return {};
}

String PropertyHost::subscribeIdFromUid (SubscriptionToken uid)
{
    const auto str = std::to_string (uid.uid);
    jassert (str.size() <= 8);
    return str;
}

PropertyHost::SubscriptionToken PropertyHost::findUnusedSubscribeId (const std::map<SubscriptionToken, String>& used)
{
    return ! used.empty() ? SubscriptionToken { std::prev (used.end())->first.uid + 1 } : SubscriptionToken { 0 };
}

} // namespace juce::midi_ci

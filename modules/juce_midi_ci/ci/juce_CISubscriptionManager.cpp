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

struct RequestRetryQueueEntry
{
    PropertySubscriptionHeader msg;
    Token64 key{};           ///< A unique identifier for this message
    bool inFlight = false;   ///< True if the message has been sent and we're waiting for a reply, false otherwise
};

/*
    A queue to store pending property exchange messages.

    A property exchange message may fail to send because the initiator doesn't have enough vacant
    property exchange IDs.
    Similarly, if the responder doesn't have enough vacant IDs, then it may tell us to retry the
    request.

    We store messages that we're planning to send, and mark them as in-flight once we've attempted
    to send them.
    We always try to send the first not-in-flight message in the queue.
    If the responder informs us that the message was actioned, or there was an unrecoverable error,
    then we can remove the message from the queue. We can also remove the message if the user
    decides that the message is no longer important.
    Otherwise, if the message wasn't sent successfully, we leave the message at its current
    position in the queue, and mark it as not-in-flight again.
*/
class RequestRetryQueue
{
    using Entry = RequestRetryQueueEntry;

private:
    auto getIter (Token64 k)
    {
        const auto iter = std::lower_bound (entries.begin(),
                                            entries.end(),
                                            (uint64_t) k,
                                            [] (const Entry& e, uint64_t v) { return (uint64_t) e.key < v; });
        return iter != entries.end() && iter->key == k ? iter : entries.end();
    }

public:
    /*  Add a new message at the end of the queue, and return the entry for that message. */
    Entry* add (PropertySubscriptionHeader msg)
    {
        const auto key = ++lastKey;

        entries.push_back (Entry { std::move (msg), Token64 { key }, false });
        return &entries.back();
    }

    /*  Erase the entry for a given key. */
    std::optional<Entry> erase (Token64 k)
    {
        const auto iter = getIter (k);

        if (iter == entries.end())
            return {};

        auto result = std::move (*iter);
        entries.erase (iter);
        return result;
    }

    /*  Find the next entry that should be sent, and return it after marking it as in-flight. */
    const Entry* markNextInFlight()
    {
        const auto iter = std::find_if (entries.begin(), entries.end(), [] (const Entry& e) { return ! e.inFlight; });

        if (iter == entries.end())
            return nullptr;

        iter->inFlight = true;
        return &*iter;
    }

    void markNotInFlight (Token64 k)
    {
        const auto iter = getIter (k);

        if (iter != entries.end())
            iter->inFlight = false;
    }

private:
    std::vector<Entry> entries;
    uint64_t lastKey = 0;
};

/**
    Info about a particular subscription.

    You can think of this as a subscription agreement as identified by a subscribeId, but this
    also holds state that is necessary to negotiate the subscribeId.
*/
struct SubscriptionState
{
    // If we're waiting to send this subscription request, this is monostate
    // If the request has been sent, but we haven't received a reply, this is the id of the request
    // If the subscription started successfully, this is the subscribeId for the subscription
    std::variant<std::monostate, Token64, String> state;
    String resource;
};

/**
    Info about all the subscriptions requested of a particular device/MUID.
    This keeps track of the order in which subscription requests are made, so that requests can
    be re-tried in order if the initial sending of a request fails.
*/
class DeviceSubscriptionStates
{
public:
    Token64 postToQueue (const PropertySubscriptionHeader& header)
    {
        return queue.add (header)->key;
    }

    Token64 beginSubscription (const PropertySubscriptionHeader& header)
    {
        jassert (header.command == PropertySubscriptionCommand::start);

        auto headerCopy = header;
        headerCopy.command = PropertySubscriptionCommand::start;

        const auto key = postToQueue (headerCopy);
        stateForSubscription[key].resource = headerCopy.resource;

        return key;
    }

    std::optional<SubscriptionState> endSubscription (Token64 key)
    {
        queue.erase (key);

        const auto iter = stateForSubscription.find (key);

        if (iter == stateForSubscription.end())
            return {};

        auto subInfo = iter->second;
        stateForSubscription.erase (iter);

        return { std::move (subInfo) };
    }

    std::vector<Token64> endSubscription (String subscribeId)
    {
        std::vector<Token64> ended;

        for (auto it = stateForSubscription.begin(); it != stateForSubscription.end();)
        {
            if (const auto* id = std::get_if<String> (&it->second.state))
            {
                if (*id == subscribeId)
                {
                    ended.push_back (it->first);
                    queue.erase (it->first);
                    it = stateForSubscription.erase (it);
                    continue;
                }
            }

            ++it;
        }

        return ended;
    }

    void endAll()
    {
        for (auto& item : stateForSubscription)
            queue.erase (item.first);

        stateForSubscription.clear();
    }

    void resetKey (Token64 key)
    {
        const auto iter = stateForSubscription.find (key);

        if (iter != stateForSubscription.end())
            iter->second.state = std::monostate{};

        queue.markNotInFlight (key);
    }

    void setRequestIdForKey (Token64 key, Token64 request)
    {
        const auto iter = stateForSubscription.find (key);

        if (iter != stateForSubscription.end())
            iter->second.state = request;
    }

    void setSubscribeIdForKey (Token64 key, String subscribeId)
    {
        const auto iter = stateForSubscription.find (key);

        if (iter != stateForSubscription.end())
            iter->second.state = subscribeId;

        queue.erase (key);
    }

    auto* markNextInFlight()
    {
        return queue.markNextInFlight();
    }

    std::optional<SubscriptionState> getInfoForSubscriptionKey (Token64 key) const
    {
        const auto iter = stateForSubscription.find (key);

        if (iter != stateForSubscription.end())
            return iter->second;

        return {};
    }

    auto begin() const { return stateForSubscription.begin(); }
    auto end() const { return stateForSubscription.end(); }

private:
    RequestRetryQueue queue;
    std::map<Token64, SubscriptionState> stateForSubscription;
};

class SubscriptionManager::Impl : public std::enable_shared_from_this<Impl>,
                                  private DeviceListener
{
public:
    explicit Impl (SubscriptionManagerDelegate& d)
        : delegate (d) {}

    SubscriptionKey beginSubscription (MUID m, const PropertySubscriptionHeader& header)
    {
        const auto key = infoForMuid[m].beginSubscription (header);
        sendPendingMessages();
        return SubscriptionKey { m, key };
    }

    void endSubscription (SubscriptionKey key)
    {
        const auto iter = infoForMuid.find (key.getMuid());

        if (iter == infoForMuid.end())
            return;

        const auto ended = iter->second.endSubscription (key.getKey());

        if (! ended.has_value())
            return;

        if (auto* request = std::get_if<Token64> (&ended->state))
        {
            delegate.abortPropertyRequest ({ key.getMuid(), *request });
        }
        else if (auto* subscribeId = std::get_if<String> (&ended->state))
        {
            PropertySubscriptionHeader header;
            header.command = PropertySubscriptionCommand::end;
            header.subscribeId = *subscribeId;
            iter->second.postToQueue (header);
            sendPendingMessages();
        }
    }

    void endSubscriptionFromResponder (MUID m, String sub)
    {
        const auto iter = infoForMuid.find (m);

        if (iter != infoForMuid.end())
            for (const auto& ended : iter->second.endSubscription (sub))
                delegate.propertySubscriptionChanged ({ m, ended }, std::nullopt);
    }

    void endSubscriptionsFromResponder (MUID m)
    {
        const auto iter = infoForMuid.find (m);

        if (iter == infoForMuid.end())
            return;

        std::vector<Token64> tokens;
        std::transform (iter->second.begin(),
                        iter->second.end(),
                        std::back_inserter (tokens),
                        [] (const auto& p) { return p.first; });

        iter->second.endAll();

        for (const auto& ended : tokens)
            delegate.propertySubscriptionChanged ({ m, ended }, std::nullopt);
    }

    std::vector<SubscriptionKey> getOngoingSubscriptions() const
    {
        std::vector<SubscriptionKey> result;

        for (const auto& pair : infoForMuid)
            for (const auto& info : pair.second)
                result.emplace_back (pair.first, info.first);

        return result;
    }

    std::optional<SubscriptionState> getInfoForSubscriptionKey (SubscriptionKey key) const
    {
        const auto iter = infoForMuid.find (key.getMuid());

        if (iter != infoForMuid.end())
            return iter->second.getInfoForSubscriptionKey (key.getKey());

        return {};
    }

    bool sendPendingMessages()
    {
        // Note: not using any_of here because we don't want the early-exit behaviour
        bool result = true;

        for (auto& pair : infoForMuid)
            result &= sendPendingMessages (pair.first, pair.second);

        return result;
    }

private:
    void handleReply (SubscriptionKey subscriptionKey, PropertySubscriptionCommand command, const PropertyExchangeResult& r)
    {
        const auto iter = infoForMuid.find (subscriptionKey.getMuid());

        if (iter == infoForMuid.end())
            return;

        auto& second = iter->second;

        if (const auto error = r.getError())
        {
            // If the responder requested a retry, keep the message in the queue so that
            // it can be re-sent
            if (*error == PropertyExchangeResult::Error::tooManyTransactions)
            {
                second.resetKey (subscriptionKey.getKey());
                return;
            }

            // We tried to begin or end a subscription, but the responder said no!
            // If the responder declined to start a subscription, we can just
            // mark the subscription as ended.
            // If the responder declined to end a subscription, that's a bit trickier.
            // Hopefully this won't happen in practice, because all the options to resolve are pretty bad:
            // - One option is to ignore the failure. The remote device can carry on sending us updates.
            //   This might be a bit dangerous if we repeatedly subscribe and then fail to unsubscribe, as this
            //   would result in lots of redundant subscription messages that could clog the connection.
            // - Another option is to store the subscription-end request and to attempt to send it again later.
            //   This also has the potential to clog up the connection, depending on how frequently we attempt
            //   to re-send failed messages. Given that unsubscribing has already failed once, there's no
            //   guarantee that any future attempts will succeed, so we might end up in a loop, sending the
            //   same message over and over.
            // On balance, I think the former option is best for now. If this ends up being an issue in
            // practice, perhaps we could add a mechanism to do exponential back-off, but that would
            // add complexity that isn't necessarily required.
            jassert (*error != PropertyExchangeResult::Error::notify);

            // If we failed to begin a subscription, then the subscription never started,
            // and we should remove it from the set of ongoing subscriptions.
            second.endSubscription (subscriptionKey.getKey());

            // We only need to alert the delegate if the subscription failed to start.
            // If the subscription fails to end, we'll treat the subscription as ended anyway.
            if (command == PropertySubscriptionCommand::start)
                delegate.propertySubscriptionChanged (subscriptionKey, std::nullopt);

            return;
        }

        if (command == PropertySubscriptionCommand::start)
        {
            second.setSubscribeIdForKey (subscriptionKey.getKey(), r.getHeaderAsSubscriptionHeader().subscribeId);
            delegate.propertySubscriptionChanged (subscriptionKey, r.getHeaderAsSubscriptionHeader().subscribeId);
        }
    }

    bool sendPendingMessages (MUID m, DeviceSubscriptionStates& info)
    {
        while (auto* entry = info.markNextInFlight())
        {
            const auto requestKind = entry->msg.command;
            const SubscriptionKey subscriptionKey { m, entry->key };

            auto cb = [weak = weak_from_this(), requestKind, subscriptionKey] (const PropertyExchangeResult& r)
            {
                if (const auto locked = weak.lock())
                    locked->handleReply (subscriptionKey, requestKind, r);
            };

            if (const auto request = delegate.sendPropertySubscribe (m, entry->msg, std::move (cb)))
            {
                if (entry->msg.command == PropertySubscriptionCommand::start)
                    info.setRequestIdForKey (entry->key, request->getKey());
            }
            else
            {
                // Couldn't find a valid ID to use, so we must have exhausted all message slots.
                // There's no point trying to send the rest of the messages that are queued for this
                // MUID, so give up. It's probably a good idea to try again in a bit.
                info.resetKey (entry->key);
                return false;
            }
        }

        return true;
    }

    SubscriptionManagerDelegate& delegate;
    std::map<MUID, DeviceSubscriptionStates> infoForMuid;
};

//==============================================================================
SubscriptionManager::SubscriptionManager (SubscriptionManagerDelegate& delegate)
    : pimpl (std::make_shared<Impl> (delegate)) {}

SubscriptionKey SubscriptionManager::beginSubscription (MUID m, const PropertySubscriptionHeader& header)
{
    return pimpl->beginSubscription (m, header);
}

void SubscriptionManager::endSubscription (SubscriptionKey key)
{
    pimpl->endSubscription (key);
}

void SubscriptionManager::endSubscriptionFromResponder (MUID m, String sub)
{
    pimpl->endSubscriptionFromResponder (m, sub);
}

void SubscriptionManager::endSubscriptionsFromResponder (MUID m)
{
    pimpl->endSubscriptionsFromResponder (m);
}

std::vector<SubscriptionKey> SubscriptionManager::getOngoingSubscriptions() const
{
    return pimpl->getOngoingSubscriptions();
}

std::optional<String> SubscriptionManager::getSubscribeIdForKey (SubscriptionKey key) const
{
    if (const auto info = pimpl->getInfoForSubscriptionKey (key))
        if (const auto* subscribeId = std::get_if<String> (&info->state))
            return *subscribeId;

    return {};
}

std::optional<String> SubscriptionManager::getResourceForKey (SubscriptionKey key) const
{
    if (const auto info = pimpl->getInfoForSubscriptionKey (key))
        return info->resource;

    return {};
}

bool SubscriptionManager::sendPendingMessages()
{
    return pimpl->sendPendingMessages();
}

//==============================================================================
//==============================================================================
#if JUCE_UNIT_TESTS

class SubscriptionTests : public UnitTest
{
public:
    SubscriptionTests() : UnitTest ("Subscription", UnitTestCategories::midi) {}

    void runTest() override
    {
        auto random = getRandom();

        class Delegate : public SubscriptionManagerDelegate
        {
        public:
            std::optional<RequestKey> sendPropertySubscribe (MUID m,
                                                             const PropertySubscriptionHeader&,
                                                             std::function<void (const PropertyExchangeResult&)> cb) override
            {
                ++sendCount;

                if (! sendShouldSucceed)
                    return {};

                const RequestKey key { m, Token64 { ++lastKey } };
                callbacks[key] = std::move (cb);

                return key;
            }

            void abortPropertyRequest (RequestKey k) override
            {
                ++abortCount;
                callbacks.erase (k);
            }

            void propertySubscriptionChanged (SubscriptionKey, const std::optional<String>&) override
            {
                subChanged = true;
            }

            void setSendShouldSucceed (bool b) { sendShouldSucceed = b; }

            void sendResult (RequestKey key, const PropertyExchangeResult& r)
            {
                const auto iter = callbacks.find (key);

                if (iter != callbacks.end())
                    NullCheckedInvocation::invoke (iter->second, r);

                callbacks.erase (key);
            }

            std::vector<RequestKey> getOngoingRequests() const
            {
                std::vector<RequestKey> result;
                result.reserve (callbacks.size());
                std::transform (callbacks.begin(), callbacks.end(), std::back_inserter (result), [] (const auto& p) { return p.first; });
                return result;
            }

            uint64_t getAndClearSendCount()  { return std::exchange (sendCount,  0); }
            uint64_t getAndClearAbortCount() { return std::exchange (abortCount, 0); }
            bool getAndClearSubChanged()     { return std::exchange (subChanged, false); }

        private:
            std::map<RequestKey, std::function<void (const PropertyExchangeResult&)>> callbacks;
            uint64_t sendCount = 0, abortCount = 0;
            uint64_t lastKey = 0;
            bool sendShouldSucceed = true, subChanged = false;
        };

        Delegate delegate;
        SubscriptionManager manager { delegate };

        const auto inquiryMUID = MUID::makeRandom (random);

        beginTest ("Beginning a subscription and ending it before the remote device replies aborts the request");
        {
            PropertySubscriptionHeader header;
            header.command = PropertySubscriptionCommand::start;
            header.resource = "X-CustomProp";

            const auto a = manager.beginSubscription (inquiryMUID, header);

            expect (manager.getOngoingSubscriptions() == std::vector { a });
            expect (delegate.getAndClearSendCount() == 1);

            // Sending a subscription request uses a request slot
            expect (delegate.getOngoingRequests().size() == 1);
            const auto request = delegate.getOngoingRequests().back();

            // subscription id is empty until the responder confirms the subscription
            expect (manager.getResourceForKey (a) == header.resource);

            manager.endSubscription (a);

            expect (delegate.getOngoingRequests().empty());
            expect (delegate.getAndClearAbortCount() == 1);
            expect (manager.getOngoingSubscriptions().empty());

            const auto successHeader = []
            {
                auto ptr = std::make_unique<DynamicObject>();
                ptr->setProperty ("status", 200);
                ptr->setProperty ("subscribeId", "anId");
                return var { ptr.release() };
            }();

            delegate.sendResult (request, PropertyExchangeResult { successHeader, {} });

            // Already ended, the confirmation shouldn't do anything
            expect (! delegate.getAndClearSubChanged());
            expect (delegate.getOngoingRequests().empty());
            expect (delegate.getAndClearSendCount() == 0);
            expect (delegate.getAndClearAbortCount() == 0);

            // There shouldn't be any queued messages.
            expect (manager.sendPendingMessages());
            expect (! delegate.getAndClearSubChanged());
            expect (delegate.getOngoingRequests().empty());
            expect (delegate.getAndClearSendCount() == 0);
            expect (delegate.getAndClearAbortCount() == 0);
        }

        beginTest ("Starting a new subscription while the device is waiting for a previous subscription to be confirmed queues further requests");
        {
            PropertySubscriptionHeader header;
            header.command = PropertySubscriptionCommand::start;
            header.resource = "X-CustomProp";

            delegate.setSendShouldSucceed (false);
            const auto a = manager.beginSubscription (inquiryMUID, header);
            expect (delegate.getAndClearSendCount() == 1);

            expect (! manager.sendPendingMessages());
            expect (delegate.getAndClearSendCount() == 1);

            delegate.setSendShouldSucceed (true);
            expect (manager.sendPendingMessages());
            expect (delegate.getAndClearSendCount() == 1);

            delegate.setSendShouldSucceed (false);
            const auto b = manager.beginSubscription (inquiryMUID, header);
            const auto c = manager.beginSubscription (inquiryMUID, header);

            expect (manager.getOngoingSubscriptions() == std::vector { a, b, c });
            expect (delegate.getOngoingRequests().size() == 1);

            // subscription id is empty until the responder confirms the subscription
            expect (manager.getResourceForKey (a) == header.resource);
            expect (manager.getResourceForKey (b) == header.resource);
            expect (manager.getResourceForKey (c) == header.resource);

            expect (delegate.getAndClearSendCount() == 2);
            expect (delegate.getAndClearAbortCount() == 0);

            delegate.setSendShouldSucceed (true);

            // The device has sent a subscription start for a, but not for c,
            // so it should send a notify to end subscription a, but shouldn't emit any
            // messages related to subscription c.
            manager.endSubscription (a);
            manager.endSubscription (c);

            expect (manager.getOngoingSubscriptions() == std::vector { b });

            expect (delegate.getOngoingRequests().empty());
            expect (delegate.getAndClearSendCount() == 0);
            expect (delegate.getAndClearAbortCount() == 1);

            // There should still be requests related to subscription b pending
            expect (manager.sendPendingMessages());
            expect (delegate.getOngoingRequests().size() == 1);
            expect (delegate.getAndClearSendCount() == 1);
            expect (delegate.getAndClearAbortCount() == 0);

            // Now, we should send a terminate request for subscription b
            manager.endSubscription (b);

            expect (delegate.getOngoingRequests().empty());
            expect (delegate.getAndClearSendCount() == 0);
            expect (delegate.getAndClearAbortCount() == 1);

            // The manager never received any replies, so it shouldn't have notified listeners about
            // changed subscriptions
            expect (! delegate.getAndClearSubChanged());
        }

        beginTest ("If the device receives a retry or notify in response to a subscription start request, the subscription is retried or terminated as necessary");
        {
            PropertySubscriptionHeader header;
            header.command = PropertySubscriptionCommand::start;
            header.resource = "X-CustomProp";

            const auto a = manager.beginSubscription (inquiryMUID, header);

            expect (manager.getOngoingSubscriptions() == std::vector { a });
            expect (delegate.getOngoingRequests().size() == 1);
            expect (delegate.getAndClearSendCount() == 1);
            expect (delegate.getAndClearAbortCount() == 0);

            delegate.sendResult (delegate.getOngoingRequests().back(), PropertyExchangeResult { PropertyExchangeResult::Error::tooManyTransactions });

            // The subscription is still active from the perspective of the manager, but the
            // first request is over and should be retried
            expect (manager.getOngoingSubscriptions() == std::vector { a });
            expect (! delegate.getAndClearSubChanged());
            expect (delegate.getOngoingRequests().empty());
            expect (delegate.getAndClearSendCount() == 0);
            expect (delegate.getAndClearAbortCount() == 0);

            expect (manager.sendPendingMessages());

            expect (manager.getOngoingSubscriptions() == std::vector { a });
            expect (delegate.getOngoingRequests().size() == 1);
            expect (delegate.getAndClearSendCount() == 1);
            expect (delegate.getAndClearAbortCount() == 0);

            delegate.sendResult (delegate.getOngoingRequests().back(), PropertyExchangeResult { PropertyExchangeResult::Error::notify });

            // The request was terminated by the responder, so the delegate should get a sub-changed message
            expect (delegate.getAndClearSubChanged());
            expect (manager.getOngoingSubscriptions().empty());
            expect (delegate.getOngoingRequests().empty());
            expect (delegate.getAndClearSendCount() == 0);
            expect (delegate.getAndClearAbortCount() == 0);

            expect (manager.sendPendingMessages());
            expect (! delegate.getAndClearSubChanged());
            expect (manager.getOngoingSubscriptions().empty());
            expect (delegate.getOngoingRequests().empty());
            expect (delegate.getAndClearSendCount() == 0);
            expect (delegate.getAndClearAbortCount() == 0);
        }

        beginTest ("If the device receives a retry or notify in response to a subscription end request, the subscription is retried as necessary");
        {
            PropertySubscriptionHeader header;
            header.command = PropertySubscriptionCommand::start;
            header.resource = "X-CustomProp";

            const auto a = manager.beginSubscription (inquiryMUID, header);

            expect (manager.getOngoingSubscriptions() == std::vector { a });
            expect (manager.getResourceForKey (a) == header.resource);
            expect (! manager.getSubscribeIdForKey (a).has_value());

            const auto subscriptionResponseHeader = []
            {
                auto ptr = std::make_unique<DynamicObject>();
                ptr->setProperty ("status", 200);
                ptr->setProperty ("subscribeId", "newId");
                return ptr.release();
            }();

            // Accept the subscription
            delegate.sendResult (delegate.getOngoingRequests().back(), PropertyExchangeResult { subscriptionResponseHeader, {} });

            // The subscription is still active from the perspective of the device, but the
            // request is over and should be retried
            expect (manager.getOngoingSubscriptions() == std::vector { a });
            expect (delegate.getAndClearSubChanged());
            // Now that the subscription was accepted, the subscription id should be non-empty
            expect (manager.getResourceForKey (a) == header.resource);
            expect (manager.getSubscribeIdForKey (a) == "newId");
            expect (delegate.getOngoingRequests().empty());
            expect (delegate.getAndClearSendCount() == 1);
            expect (delegate.getAndClearAbortCount() == 0);

            manager.endSubscription (a);

            expect (manager.getOngoingSubscriptions().empty());
            expect (! delegate.getAndClearSubChanged());
            expect (delegate.getOngoingRequests().size() == 1);
            expect (delegate.getAndClearSendCount() == 1);
            expect (delegate.getAndClearAbortCount() == 0);

            // The responder is busy, can't process the subscription end
            delegate.sendResult (delegate.getOngoingRequests().back(), PropertyExchangeResult { PropertyExchangeResult::Error::tooManyTransactions });

            expect (manager.getOngoingSubscriptions().empty());
            expect (delegate.getOngoingRequests().empty());
            expect (delegate.getAndClearSendCount() == 0);
            expect (delegate.getAndClearAbortCount() == 0);

            expect (manager.sendPendingMessages());
            expect (manager.getOngoingSubscriptions().empty());
            expect (delegate.getOngoingRequests().size() == 1);
            expect (delegate.getAndClearSendCount() == 1);
            expect (delegate.getAndClearAbortCount() == 0);

            // The responder told us to immediately terminate our request to end the subscription!
            // It's unclear how this should behave, so we'll just ignore the failure and assume
            // the subscription is really over.
            delegate.sendResult (delegate.getOngoingRequests().back(), PropertyExchangeResult { PropertyExchangeResult::Error::notify });

            expect (manager.getOngoingSubscriptions().empty());
            expect (delegate.getOngoingRequests().empty());
            expect (delegate.getAndClearSendCount() == 0);
            expect (delegate.getAndClearAbortCount() == 0);

            expect (manager.sendPendingMessages());
            expect (delegate.getAndClearSendCount() == 0);
            expect (delegate.getAndClearAbortCount() == 0);

            expect (! delegate.getAndClearSubChanged());
        }
    }
};

static SubscriptionTests subscriptionTests;

#endif

} // namespace juce::midi_ci

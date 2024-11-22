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

class Device::Impl : private SubscriptionManagerDelegate
{
    template <typename This>
    static auto getProfileHostImpl (This& t) { return t.profileHost.has_value() ? &*t.profileHost : nullptr; }

    template <typename This>
    static auto getPropertyHostImpl (This& t) { return t.propertyHost.has_value() ? &*t.propertyHost : nullptr; }

public:
    explicit Impl (const Options& opt)
        : options (getValidated (opt)),
          muid (getReallyRandomMuid())
    {
        if (options.getFeatures().isProfileConfigurationSupported())
            profileHost.emplace (options.getFunctionBlock(), profileDelegate, concreteBufferOutput);

        if (options.getFeatures().isPropertyExchangeSupported())
            propertyHost.emplace (options.getFunctionBlock(), propertyDelegate, concreteBufferOutput, cacheProvider);

        outgoing.reserve (options.getMaxSysExSize());
    }

    ~Impl() override
    {
        if (concreteBufferOutput.hasSentMuid())
        {
            detail::MessageTypeUtils::send (concreteBufferOutput,
                                            options.getFunctionBlock().firstGroup,
                                            MUID::getBroadcast(),
                                            ChannelInGroup::wholeBlock,
                                            Message::InvalidateMUID { muid });
        }
    }

    void sendDiscovery()
    {
        {
            const auto aboutToRemove = std::move (discovered);

            for (const auto& pair : aboutToRemove)
                listeners.call ([&] (auto& l) { l.deviceRemoved (pair.first); });
        }

        const Message::Header header
        {
            ChannelInGroup::wholeBlock,
            detail::MessageMeta::Meta<Message::Discovery>::subID2,
            detail::MessageMeta::implementationVersion,
            muid,
            MUID::getBroadcast(),
        };

        jassert (options.getOutputs().size() < 128);

        for (size_t i = 0; i < options.getOutputs().size(); ++i)
        {
            const Message::Discovery discovery
            {
                options.getDeviceInfo(),
                options.getFeatures().getSupportedCapabilities(),
                uint32_t (options.getMaxSysExSize()),
                std::byte (i % 128),
            };

            outgoing.clear();
            detail::Marshalling::Writer { outgoing } (header, discovery);
            options.getOutputs()[i]->processMessage ({ options.getFunctionBlock().firstGroup, outgoing });
        }
    }

    void sendEndpointInquiry (MUID destination, Message::EndpointInquiry endpoint)
    {
        detail::MessageTypeUtils::send (concreteBufferOutput,
                                        options.getFunctionBlock().firstGroup,
                                        destination,
                                        ChannelInGroup::wholeBlock,
                                        endpoint);
    }

    void sendProfileInquiry (MUID receiver, ChannelInGroup address)
    {
        if (! supportsProfiles (receiver))
            return;

        detail::MessageTypeUtils::send (concreteBufferOutput,
                                        options.getFunctionBlock().firstGroup,
                                        receiver,
                                        address,
                                        Message::ProfileInquiry{});
    }

    void sendProfileDetailsInquiry (MUID receiver, ChannelInGroup address, Profile profile, std::byte target)
    {
        if (! supportsProfiles (receiver))
            return;

        detail::MessageTypeUtils::send (concreteBufferOutput,
                                        options.getFunctionBlock().firstGroup,
                                        receiver,
                                        address,
                                        Message::ProfileDetails { profile, target });
    }

    void sendProfileSpecificData (MUID receiver, ChannelInGroup address, Profile profile, Span<const std::byte> data)
    {
        if (! supportsProfiles (receiver))
            return;

        detail::MessageTypeUtils::send (concreteBufferOutput,
                                        options.getFunctionBlock().firstGroup,
                                        receiver,
                                        address,
                                        Message::ProfileSpecificData { profile, data });
    }

    void sendProfileEnablement (MUID m, ChannelInGroup address, Profile profile, int numChannels)
    {
        if (! supportsProfiles (m))
            return;

        // There are only 256 channels on a UMP endpoint, so requesting more probably doesn't make sense!
        jassert (numChannels <= 256);

        if (numChannels > 0)
        {
            const auto channelsToSend = address == ChannelInGroup::wholeBlock || address == ChannelInGroup::wholeGroup
                                      ? 0
                                      : numChannels;

            detail::MessageTypeUtils::send (concreteBufferOutput,
                                            options.getFunctionBlock().firstGroup,
                                            m,
                                            address,
                                            Message::ProfileOn { profile, (uint16_t) channelsToSend });
        }
        else
        {
            detail::MessageTypeUtils::send (concreteBufferOutput,
                                            options.getFunctionBlock().firstGroup,
                                            m,
                                            address,
                                            Message::ProfileOff { profile });
        }
    }

    void sendPropertyCapabilitiesInquiry (MUID m)
    {
        if (! supportsProperties (m))
            return;

        detail::MessageTypeUtils::send (concreteBufferOutput,
                                        options.getFunctionBlock().firstGroup,
                                        m,
                                        ChannelInGroup::wholeBlock,
                                        Message::PropertyExchangeCapabilities { std::byte { propertyDelegate.getNumSimultaneousRequestsSupported() }, {}, {} });
    }

    std::optional<RequestKey> sendPropertyGetInquiry (MUID m,
                                                      const PropertyRequestHeader& header,
                                                      std::function<void (const PropertyExchangeResult&)> onResult)
    {
        const auto iter = discovered.find (m);

        if (iter == discovered.end() || ! Features { iter->second.discovery.capabilities }.isPropertyExchangeSupported())
            return {};

        const auto primed = iter->second.initiatorPropertyCaches.primeCache (propertyDelegate.getNumSimultaneousRequestsSupported(),
                                                                             std::move (onResult));

        if (! primed.has_value())
            return {};

        const auto id = iter->second.initiatorPropertyCaches.getRequestIdForToken (*primed);
        jassert (id.has_value());

        detail::MessageTypeUtils::send (concreteBufferOutput,
                                        options.getFunctionBlock().firstGroup,
                                        m,
                                        ChannelInGroup::wholeBlock,
                                        Message::PropertyGetData { { id->asByte(), Encodings::jsonTo7BitText (header.toVarCondensed()) } });

        return RequestKey { m, *primed };
    }

    std::optional<RequestKey> sendPropertySetInquiry (MUID m,
                                                      const PropertyRequestHeader& header,
                                                      Span<const std::byte> body,
                                                      std::function<void (const PropertyExchangeResult&)> onResult)
    {
        const auto encoded = Encodings::tryEncode (body, header.mutualEncoding);

        if (! encoded.has_value())
            return {};

        const auto iter = discovered.find (m);

        if (iter == discovered.end() || ! Features { iter->second.discovery.capabilities }.isPropertyExchangeSupported())
            return {};

        const auto primed = iter->second.initiatorPropertyCaches.primeCache (propertyDelegate.getNumSimultaneousRequestsSupported(),
                                                                             std::move (onResult));

        if (! primed.has_value())
            return {};

        const auto id = iter->second.initiatorPropertyCaches.getRequestIdForToken (*primed);
        jassert (id.has_value());

        detail::PropertyHostUtils::send (concreteBufferOutput,
                                         options.getFunctionBlock().firstGroup,
                                         detail::MessageMeta::Meta<Message::PropertySetData>::subID2,
                                         m,
                                         id->asByte(),
                                         Encodings::jsonTo7BitText (header.toVarCondensed()),
                                         *encoded,
                                         cacheProvider.getMaxSysexSizeForMuid (m));

        return RequestKey { m, *primed };
    }

    void abortPropertyRequest (RequestKey k) override
    {
        const auto iter = discovered.find (k.getMuid());

        if (iter == discovered.end())
            return;

        const auto id = iter->second.initiatorPropertyCaches.getRequestIdForToken (k.getKey());

        if (! id.has_value() || ! iter->second.initiatorPropertyCaches.terminate (k.getKey()))
            return;

        const Message::Header notifyHeader
        {
            ChannelInGroup::wholeBlock,
            detail::MessageMeta::Meta<Message::PropertyNotify>::subID2,
            detail::MessageMeta::implementationVersion,
            muid,
            k.getMuid(),
        };

        const auto jsonHeader = Encodings::jsonTo7BitText (JSONUtils::makeObjectWithKeyFirst ({ { "status", 144 } }, "status"));
        detail::MessageTypeUtils::send (concreteBufferOutput,
                                        options.getFunctionBlock().firstGroup,
                                        notifyHeader,
                                        Message::PropertyNotify { { id->asByte(), jsonHeader, 1, 1, {} } });
    }

    std::optional<RequestID> getIdForRequestKey (RequestKey key) const
    {
        const auto iter = discovered.find (key.getMuid());

        if (iter == discovered.end())
            return {};

        return iter->second.initiatorPropertyCaches.getRequestIdForToken (key.getKey());
    }

    std::vector<RequestKey> getOngoingRequests() const
    {
        std::vector<RequestKey> result;

        for (auto& i : discovered)
            for (const auto& token : i.second.initiatorPropertyCaches.getOngoingTransactions())
                result.emplace_back (i.first, token);

        return result;
    }

    SubscriptionKey beginSubscription (MUID m, const PropertySubscriptionHeader& header)
    {
        return subscriptionManager.beginSubscription (m, header);
    }

    void endSubscription (SubscriptionKey key)
    {
        subscriptionManager.endSubscription (key);
    }

    std::vector<SubscriptionKey> getOngoingSubscriptions() const
    {
        return subscriptionManager.getOngoingSubscriptions();
    }

    std::optional<String> getSubscribeIdForKey (SubscriptionKey key) const
    {
        return subscriptionManager.getSubscribeIdForKey (key);
    }

    std::optional<String> getResourceForKey (SubscriptionKey key) const
    {
        return subscriptionManager.getResourceForKey (key);
    }

    bool sendPendingMessages()
    {
        return subscriptionManager.sendPendingMessages();
    }

    void processMessage (ump::BytesOnGroup msg)
    {
        // Queried before the property host to unconditionally register capabilities of property exchange hosts.
        FirstListener firstListener { this };
        LastListener lastListener { this };

        ResponderDelegate* const l[] { &firstListener,
                                       getProfileHostImpl (*this),
                                       getPropertyHostImpl (*this),
                                       &lastListener };

        const auto status = detail::Responder::processCompleteMessage (concreteBufferOutput, msg, l);

        if (status == Parser::Status::collidingMUID)
        {
            muid = getReallyRandomMuid();
            concreteBufferOutput.resetSentMuid();
            sendDiscovery();
        }
    }

    void addListener (Listener& l)
    {
        listeners.add (&l);
    }

    void removeListener (Listener& l)
    {
        listeners.remove (&l);
    }

    std::vector<MUID> getDiscoveredMuids() const
    {
        std::vector<MUID> result (discovered.size(), MUID::makeUnchecked (0));
        std::transform (discovered.begin(), discovered.end(), result.begin(), [] (const auto& p) { return p.first; });
        return result;
    }

    std::optional<Message::Discovery> getDiscoveryInfoForMuid (MUID m) const
    {
        const auto iter = discovered.find (m);
        return iter != discovered.end()
             ? std::optional<Message::Discovery> (iter->second.discovery)
             : std::nullopt;
    }

    std::optional<int> getNumPropertyExchangeRequestsSupportedForMuid (MUID m) const
    {
        const auto iter = discovered.find (m);
        return iter != discovered.end()
             ? std::optional<int> ((int) iter->second.propertyExchangeResponse->numSimultaneousRequestsSupported)
             : std::nullopt;
    }

    const ChannelProfileStates* getProfileStateForMuid (MUID m, ChannelAddress address) const
    {
        const auto iter = discovered.find (m);
        return iter != discovered.end() ? iter->second.profileStates.getStateForDestination (address) : nullptr;
    }

    var getResourceListForMuid (MUID x) const
    {
        const auto iter = discovered.find (x);
        return iter != discovered.end() ? iter->second.resourceList : var();
    }

    var getDeviceInfoForMuid (MUID x) const
    {
        const auto iter = discovered.find (x);
        return iter != discovered.end() ? iter->second.deviceInfo : var();
    }

    var getChannelListForMuid (MUID x) const
    {
        const auto iter = discovered.find (x);
        return iter != discovered.end() ? iter->second.channelList : var();
    }

    MUID getMuid() const { return muid; }

    Options getOptions() const { return options; }

    ProfileHost* getProfileHost() { return getProfileHostImpl (*this); }
    const ProfileHost* getProfileHost() const { return getProfileHostImpl (*this); }

    PropertyHost* getPropertyHost() { return getPropertyHostImpl (*this); }
    const PropertyHost* getPropertyHost() const { return getPropertyHostImpl (*this); }

private:
    class FirstListener : public ResponderDelegate
    {
    public:
        explicit FirstListener (Impl* d) : device (d) {}

        bool tryRespond (ResponderOutput& output, const Message::Parsed& message) override
        {
            detail::MessageTypeUtils::visit (message, Visitor { device, &output });
            return false;
        }

    private:
        class Visitor : public detail::MessageTypeUtils::MessageVisitor
        {
        public:
            Visitor (Impl* d, ResponderOutput* o)
                : device (d), output (o) {}

            void visit (const Message::PropertyExchangeCapabilities& caps)         const override { visitImpl (caps); }
            void visit (const Message::PropertyExchangeCapabilitiesResponse& caps) const override { visitImpl (caps); }
            using MessageVisitor::visit;

        private:
            template <typename Body>
            void visitImpl (const Body& t) const
            {
                const auto responderMUID = output->getIncomingHeader().source;
                const auto iter = device->discovered.find (responderMUID);

                if (iter == device->discovered.end())
                    return;

                iter->second.propertyExchangeResponse = Message::PropertyExchangeCapabilitiesResponse { t.numSimultaneousRequestsSupported,
                                                                                                        t.majorVersion,
                                                                                                        t.minorVersion };
            }

            Impl* device = nullptr;
            ResponderOutput* output = nullptr;
        };

        Impl* device = nullptr;
    };

    class LastListener : public ResponderDelegate
    {
    public:
        explicit LastListener (Impl* d) : device (d) {}

        bool tryRespond (ResponderOutput& output, const Message::Parsed& message) override
        {
            bool result = false;
            detail::MessageTypeUtils::visit (message, Visitor { device, &output, &result });
            return result;
        }

    private:
        class Visitor : public detail::MessageTypeUtils::MessageVisitor
        {
        public:
            Visitor (Impl* d, ResponderOutput* o, bool* b)
                : device (d), output (o), handled (b) {}

            void visit (const Message::Discovery& x)                            const override { visitImpl (x); }
            void visit (const Message::DiscoveryResponse& x)                    const override { visitImpl (x); }
            void visit (const Message::InvalidateMUID& x)                       const override { visitImpl (x); }
            void visit (const Message::EndpointInquiry& x)                      const override { visitImpl (x); }
            void visit (const Message::EndpointInquiryResponse& x)              const override { visitImpl (x); }
            void visit (const Message::NAK& x)                                  const override { visitImpl (x); }
            void visit (const Message::ProfileInquiryResponse& x)               const override { visitImpl (x); }
            void visit (const Message::ProfileAdded& x)                         const override { visitImpl (x); }
            void visit (const Message::ProfileRemoved& x)                       const override { visitImpl (x); }
            void visit (const Message::ProfileEnabledReport& x)                 const override { visitImpl (x); }
            void visit (const Message::ProfileDisabledReport& x)                const override { visitImpl (x); }
            void visit (const Message::ProfileDetailsResponse& x)               const override { visitImpl (x); }
            void visit (const Message::ProfileSpecificData& x)                  const override { visitImpl (x); }
            void visit (const Message::PropertyExchangeCapabilitiesResponse& x) const override { visitImpl (x); }
            void visit (const Message::PropertyGetDataResponse& x)              const override { visitImpl (x); }
            void visit (const Message::PropertySetDataResponse& x)              const override { visitImpl (x); }
            void visit (const Message::PropertySubscribe& x)                    const override { visitImpl (x); }
            void visit (const Message::PropertySubscribeResponse& x)            const override { visitImpl (x); }
            void visit (const Message::PropertyNotify& x)                       const override { visitImpl (x); }
            using MessageVisitor::visit;

        private:
            template <typename Body>
            void visitImpl (const Body& body) const { *handled = messageReceived (body); }

            bool messageReceived (const Message::Discovery& body) const
            {
                const auto replyPath = uint8_t (output->getIncomingHeader().version) >= 0x02 ? body.outputPathID : std::byte { 0x00 };

                detail::MessageTypeUtils::send (*output, Message::DiscoveryResponse
                {
                    device->options.getDeviceInfo(),
                    device->options.getFeatures().getSupportedCapabilities(),
                    uint32_t (device->options.getMaxSysExSize()),
                    replyPath,
                    device->options.getFunctionBlock().identifier,
                });

                // TODO(reuk) rather than sending a new discovery inquiry, we should store the details from the incoming message
                const auto iter = device->discovered.find (output->getIncomingHeader().source);

                if (iter == device->discovered.end())
                {
                    const auto initiator = output->getIncomingHeader().source;
                    device->discovered.emplace (initiator, Discovered { body });
                    device->listeners.call ([&] (auto& l) { l.deviceAdded (initiator); });
                    device->sendEndpointInquiry (initiator, Message::EndpointInquiry { std::byte{} });
                }

                return true;
            }

            bool messageReceived (const Message::DiscoveryResponse& response) const
            {
                const auto responderMUID = output->getIncomingHeader().source;
                const auto iter = device->discovered.find (responderMUID);

                if (iter != device->discovered.end())
                {
                    device->discovered.erase (iter);
                    device->listeners.call ([&] (auto& l) { l.deviceRemoved (responderMUID); });

                    const Message::Header header
                    {
                        ChannelInGroup::wholeBlock,
                        detail::MessageMeta::Meta<Message::InvalidateMUID>::subID2,
                        detail::MessageMeta::implementationVersion,
                        device->muid,
                        MUID::getBroadcast(),
                    };

                    detail::MessageTypeUtils::send (*output, output->getIncomingGroup(), header, Message::InvalidateMUID { responderMUID });
                }
                else
                {
                    const Message::Discovery discovery { response.device,
                                                         response.capabilities,
                                                         response.maximumSysexSize,
                                                         response.outputPathID };
                    device->discovered.emplace (responderMUID, Discovered { discovery });
                    device->listeners.call ([&] (auto& l) { l.deviceAdded (responderMUID); });
                    device->sendEndpointInquiry (output->getIncomingHeader().source, Message::EndpointInquiry { std::byte{} });
                }

                return true;
            }

            bool messageReceived (const Message::InvalidateMUID& invalidate) const
            {
                const auto targetMuid = invalidate.target;
                const auto iter = device->discovered.find (targetMuid);

                if (iter != device->discovered.end())
                {
                    device->subscriptionManager.endSubscriptionsFromResponder (targetMuid);
                    device->discovered.erase (iter);
                    device->listeners.call ([&] (auto& l) { l.deviceRemoved (targetMuid); });
                }

                if (invalidate.target != device->muid)
                    return false;

                device->muid = getReallyRandomMuid();
                device->concreteBufferOutput.resetSentMuid();
                device->sendDiscovery();

                return true;
            }

            bool messageReceived (const Message::EndpointInquiry& endpoint) const
            {
                // Only status 0 is defined at time of writing
                if (endpoint.status == std::byte{})
                {
                    const auto& id = device->options.getProductInstanceId();
                    const auto length = std::distance (id.begin(), std::find (id.begin(), id.end(), 0));

                    if (length <= 0)
                        return false;

                    Message::EndpointInquiryResponse response;
                    response.status = endpoint.status;
                    response.data = Span<const std::byte> (reinterpret_cast<const std::byte*> (id.data()), (size_t) length);
                    detail::MessageTypeUtils::send (*output, response);
                    return true;
                }

                return false;
            }

            bool messageReceived (const Message::EndpointInquiryResponse& endpoint) const
            {
                const auto responderMUID = output->getIncomingHeader().source;
                const auto iter = device->discovered.find (responderMUID);

                if (iter == device->discovered.end())
                    return false; // Got an endpoint response for a device we haven't discovered

                device->listeners.call ([&] (auto& l) { l.endpointReceived (responderMUID, endpoint); });
                return true;
            }

            bool messageReceived (const Message::NAK& nak) const
            {
                const auto responderMUID = output->getIncomingHeader().source;
                device->listeners.call ([&] (auto& l) { l.messageNotAcknowledged (responderMUID, nak); });
                return true;
            }

            bool messageReceived (const Message::ProfileInquiryResponse& response) const
            {
                const auto responderMUID = output->getIncomingHeader().source;
                const auto iter = device->discovered.find (responderMUID);

                if (iter == device->discovered.end())
                    return false;

                const auto destination = output->getIncomingHeader().deviceID;
                auto* state = iter->second.profileStates.getStateForDestination (output->getChannelAddress());

                if (state == nullptr)
                    return false;

                ChannelProfileStates newState;

                for (auto& enabled : response.enabledProfiles)
                    newState.set (enabled, { 1, 1 });

                for (auto& disabled : response.disabledProfiles)
                    newState.set (disabled, { 1, 0 });

                *state = newState;
                device->listeners.call ([&] (auto& l) { l.profileStateReceived (responderMUID, destination); });

                return true;
            }

            bool messageReceived (const Message::ProfileAdded& added) const
            {
                const auto responderMUID = output->getIncomingHeader().source;
                const auto iter = device->discovered.find (responderMUID);

                if (iter == device->discovered.end())
                    return false;

                const auto address = output->getChannelAddress();
                auto* state = iter->second.profileStates.getStateForDestination (address);

                if (state == nullptr)
                    return false;

                state->set (added.profile, { 1, 0 });
                device->listeners.call ([&] (auto& l) { l.profilePresenceChanged (responderMUID, address.getChannel(), added.profile, true); });

                return true;
            }

            bool messageReceived (const Message::ProfileRemoved& removed) const
            {
                const auto responderMUID = output->getIncomingHeader().source;
                const auto iter = device->discovered.find (responderMUID);

                if (iter == device->discovered.end())
                    return false;

                const auto address = output->getChannelAddress();
                auto* state = iter->second.profileStates.getStateForDestination (address);

                if (state == nullptr)
                    return false;

                state->erase (removed.profile);
                device->listeners.call ([&] (auto& l) { l.profilePresenceChanged (responderMUID, address.getChannel(), removed.profile, false); });

                return true;
            }

            bool messageReceived (const Message::ProfileEnabledReport& x) const
            {
                const auto responderMUID = output->getIncomingHeader().source;
                const auto iter = device->discovered.find (responderMUID);

                if (iter == device->discovered.end())
                    return false;

                const auto address = output->getChannelAddress();
                auto* state = iter->second.profileStates.getStateForDestination (address);

                if (state == nullptr)
                    return false;

                const auto numChannels = jmax ((uint16_t) 1, x.numChannels);

                state->set (x.profile, { state->get (x.profile).supported, numChannels });
                device->listeners.call ([&] (auto& l) { l.profileEnablementChanged (responderMUID, address.getChannel(), x.profile, numChannels); });

                return true;
            }

            bool messageReceived (const Message::ProfileDisabledReport& x) const
            {
                const auto responderMUID = output->getIncomingHeader().source;
                const auto iter = device->discovered.find (responderMUID);

                if (iter == device->discovered.end())
                    return false;

                const auto address = output->getChannelAddress();
                auto* state = iter->second.profileStates.getStateForDestination (address);

                if (state == nullptr)
                    return false;

                state->set (x.profile, { state->get (x.profile).supported, 0 });
                device->listeners.call ([&] (auto& l) { l.profileEnablementChanged (responderMUID, address.getChannel(), x.profile, 0); });

                return true;
            }

            bool messageReceived (const Message::ProfileDetailsResponse& response) const
            {
                const auto responderMUID = output->getIncomingHeader().source;
                const auto destination = output->getIncomingHeader().deviceID;
                device->listeners.call ([&] (auto& l) { l.profileDetailsReceived (responderMUID, destination, response.profile, response.target, response.data); });

                return true;
            }

            bool messageReceived (const Message::ProfileSpecificData& data) const
            {
                const auto responderMUID = output->getIncomingHeader().source;
                const auto destination = output->getIncomingHeader().deviceID;
                device->listeners.call ([&] (auto& l) { l.profileSpecificDataReceived (responderMUID, destination, data.profile, data.data); });

                return true;
            }

            bool messageReceived (const Message::PropertyExchangeCapabilitiesResponse&) const
            {
                const auto source = output->getIncomingHeader().source;
                const auto iter = device->discovered.find (source);

                constexpr auto hasResource = [] (var obj, auto resource)
                {
                    if (auto* array = obj.getArray())
                        for (const auto& item : *array)
                            if (item.isObject() && item.getProperty ("resource", {}) == var (resource))
                                return true;

                    return false;
                };

                const auto onResourceListReceived = [this, iter, source, hasResource] (const PropertyExchangeResult& result)
                {
                    const auto validateResponse = [] (const PropertyExchangeResult& r)
                    {
                        const auto parsed = r.getHeaderAsReplyHeader();
                        return ! r.getError().has_value()
                               && parsed.mediaType == PropertySubscriptionHeader().mediaType
                               && parsed.status == 200;
                    };

                    const auto allDone = [this, source]
                    {
                        device->listeners.call ([source] (auto& l) { l.propertyExchangeCapabilitiesReceived (source); });
                    };

                    if (! validateResponse (result))
                    {
                        jassertfalse;
                        allDone();
                        return;
                    }

                    const auto bodyAsObj = Encodings::jsonFrom7BitText (result.getBody());
                    iter->second.resourceList = bodyAsObj;

                    const auto onChannelListReceived = [iter, allDone, validateResponse] (const PropertyExchangeResult& r)
                    {
                        if (validateResponse (r))
                            iter->second.channelList = Encodings::jsonFrom7BitText (r.getBody());

                        allDone();
                        return;
                    };

                    const auto getChannelList = [this, bodyAsObj, source, allDone, hasResource, onChannelListReceived]
                    {
                        if (hasResource (bodyAsObj, "ChannelList"))
                        {
                            PropertyRequestHeader header;
                            header.resource = "ChannelList";
                            device->sendPropertyGetInquiry (source, header, onChannelListReceived);
                            return;
                        }

                        allDone();
                        return;
                    };

                    if (hasResource (bodyAsObj, "DeviceInfo"))
                    {
                        PropertyRequestHeader header;
                        header.resource = "DeviceInfo";
                        device->sendPropertyGetInquiry (source,
                                                        header,
                                                        [iter, getChannelList, validateResponse] (const PropertyExchangeResult& r)
                                                        {
                                                            if (validateResponse (r))
                                                                iter->second.deviceInfo = Encodings::jsonFrom7BitText (r.getBody());

                                                            getChannelList();
                                                        });
                        return;
                    }

                    return getChannelList();
                };

                PropertyRequestHeader header;
                header.resource = "ResourceList";
                device->sendPropertyGetInquiry (source, header, onResourceListReceived);

                return true;
            }

            bool handlePropertyDataResponse (const Message::DynamicSizePropertyExchange& response) const
            {
                const auto responderMUID = output->getIncomingHeader().source;
                const auto iter = device->discovered.find (responderMUID);

                if (iter == device->discovered.end())
                    return false;

                const auto request = RequestID::create (response.requestID);

                if (! request.has_value())
                    return false;

                iter->second.initiatorPropertyCaches.addChunk (*request, response);

                return true;
            }

            bool messageReceived (const Message::PropertyGetDataResponse& response) const
            {
                handlePropertyDataResponse (response);
                return true;
            }

            bool messageReceived (const Message::PropertySetDataResponse& response) const
            {
                handlePropertyDataResponse (Message::DynamicSizePropertyExchange { response.requestID,
                                                                                   response.header,
                                                                                   1,
                                                                                   1,
                                                                                   {} });
                return true;
            }

            bool messageReceived (const Message::PropertySubscribe& subscription) const
            {
                const auto responderMUID = output->getIncomingHeader().source;
                const auto iter = device->discovered.find (responderMUID);

                if (iter == device->discovered.end())
                    return false;

                const auto request = subscription.requestID;
                const auto source = output->getIncomingHeader().source;

                const auto jsonHeader = Encodings::jsonFrom7BitText (subscription.header);
                const auto typedHeader = PropertySubscriptionHeader::parseCondensed (jsonHeader);
                const auto subscribeId = typedHeader.subscribeId;

                const auto callback = [this, request, source, subscribeId] (const PropertyExchangeResult& result)
                {
                    if (result.getError().has_value())
                        return;

                    PropertySubscriptionData data;

                    data.header = result.getHeaderAsSubscriptionHeader();
                    data.body = result.getBody();

                    if (data.header.command == PropertySubscriptionCommand::end)
                        device->subscriptionManager.endSubscriptionFromResponder (source, subscribeId);

                    if (data.header.command != PropertySubscriptionCommand::start)
                        device->listeners.call ([source, &data] (auto& l) { l.propertySubscriptionDataReceived (source, data); });

                    PropertyReplyHeader header;
                    const auto headerBytes = Encodings::jsonTo7BitText (header.toVarCondensed());

                    detail::MessageTypeUtils::send (device->concreteBufferOutput,
                                                    device->options.getFunctionBlock().firstGroup,
                                                    source,
                                                    ChannelInGroup::wholeBlock,
                                                    Message::PropertySubscribeResponse { { request, headerBytes, 1, 1, {} } });
                };

                const auto requestID = RequestID::create (subscription.requestID);

                if (! requestID.has_value())
                    return false;

                // Subscription events may be sent at any time by the responder, so there may not be
                // an existing transaction ID for new subscription messages.
                iter->second.responderPropertyCaches.primeCache (device->propertyDelegate.getNumSimultaneousRequestsSupported(),
                                                                 callback,
                                                                 *requestID);

                iter->second.responderPropertyCaches.addChunk (*requestID, subscription);

                return true;
            }

            bool messageReceived (const Message::PropertySubscribeResponse& response) const
            {
                handlePropertyDataResponse (response);
                return true;
            }

            bool messageReceived (const Message::PropertyNotify& notify) const
            {
                const auto responderMUID = output->getIncomingHeader().source;
                const auto iter = device->discovered.find (responderMUID);

                if (iter == device->discovered.end())
                    return false;

                const auto requestID = RequestID::create (notify.requestID);

                if (! requestID.has_value())
                    return false;

                iter->second.initiatorPropertyCaches.notify (*requestID, notify.header);
                iter->second.responderPropertyCaches.notify (*requestID, notify.header);

                return true;
            }

            Impl* device = nullptr;
            ResponderOutput* output = nullptr;
            bool* handled = nullptr;
        };

        Impl* device = nullptr;
    };

    struct Discovered
    {
        explicit Discovered (Message::Discovery r) : discovery (r) {}

        Message::Discovery discovery;
        std::optional<Message::PropertyExchangeCapabilitiesResponse> propertyExchangeResponse;
        BlockProfileStates profileStates;
        InitiatorPropertyExchangeCache initiatorPropertyCaches;
        ResponderPropertyExchangeCache responderPropertyCaches;
        var resourceList, deviceInfo, channelList;
    };

    class ConcreteBufferOutput : public BufferOutput
    {
    public:
        explicit ConcreteBufferOutput (Impl& d) : device (d) {}

        MUID getMuid() const override { return device.muid; }
        std::vector<std::byte>& getOutputBuffer() override { return device.outgoing; }

        void send (uint8_t group) override
        {
            sentMuid = true;

            for (auto* o : device.options.getOutputs())
                o->processMessage ({ group, getOutputBuffer() });
        }

        bool hasSentMuid() const { return sentMuid; }
        void resetSentMuid() { sentMuid = false; }

    private:
        Impl& device;
        bool sentMuid = false;
    };

    class CacheProviderImpl : public CacheProvider
    {
    public:
        explicit CacheProviderImpl (Impl& d) : device (d) {}

        std::set<MUID> getDiscoveredMuids() const override
        {
            std::set<MUID> result;

            for (const auto& d : device.discovered)
                result.insert (d.first);

            return result;
        }

        InitiatorPropertyExchangeCache* getCacheForMuidAsInitiator (MUID m) override
        {
            const auto iter = device.discovered.find (m);
            return iter != device.discovered.end() ? &iter->second.initiatorPropertyCaches : nullptr;
        }

        ResponderPropertyExchangeCache* getCacheForMuidAsResponder (MUID m) override
        {
            const auto iter = device.discovered.find (m);
            return iter != device.discovered.end() ? &iter->second.responderPropertyCaches : nullptr;
        }

        int getMaxSysexSizeForMuid (MUID m) const override
        {
            constexpr auto defaultResult = 1 << 16;

            const auto iter = device.discovered.find (m);
            return iter != device.discovered.end() ? jmin (defaultResult, (int) iter->second.discovery.maximumSysexSize) : defaultResult;
        }

    private:
        Impl& device;
    };

    class ProfileDelegateImpl : public ProfileDelegate
    {
    public:
        explicit ProfileDelegateImpl (Impl& d) : device (d) {}

        void profileEnablementRequested (MUID x, ProfileAtAddress profileAtAddress, int numChannels, bool enabled) override
        {
            if (auto* d = device.options.getProfileDelegate())
                return d->profileEnablementRequested (x, profileAtAddress, numChannels, enabled);

            if (! device.profileHost.has_value())
                return;

            device.profileHost->setProfileEnablement (profileAtAddress, enabled ? jmax (1, numChannels) : 0);
        }

    private:
        Impl& device;
    };

    class PropertyDelegateImpl : public PropertyDelegate
    {
    public:
        explicit PropertyDelegateImpl (Impl& d) : device (d) {}

        uint8_t getNumSimultaneousRequestsSupported() const override
        {
            if (auto* d = device.options.getPropertyDelegate())
                return d->getNumSimultaneousRequestsSupported();

            return 127;
        }

        PropertyReplyData propertyGetDataRequested (MUID m, const PropertyRequestHeader& header) override
        {
            if (auto* d = device.options.getPropertyDelegate())
                return d->propertyGetDataRequested (m, header);

            PropertyReplyData result;
            result.header.status = 404; // Resource not found, do not retry
            result.header.message = TRANS ("Handling for \"Inquiry: Get Property Data\" is not implemented.");
            return result;
        }

        PropertyReplyHeader propertySetDataRequested (MUID m, const PropertyRequestData& data) override
        {
            if (auto* d = device.options.getPropertyDelegate())
                return d->propertySetDataRequested (m, data);

            PropertyReplyHeader result;
            result.status = 404; // Resource not found, do not retry
            result.message = TRANS ("Handling for \"Inquiry: Set Property Data\" is not implemented.");
            return result;
        }

        bool subscriptionStartRequested (MUID m, const PropertySubscriptionHeader& data) override
        {
            if (auto* d = device.options.getPropertyDelegate())
                return d->subscriptionStartRequested (m, data);

            return false;
        }

        void subscriptionDidStart (MUID m, const String& id, const PropertySubscriptionHeader& data) override
        {
            if (auto* d = device.options.getPropertyDelegate())
                d->subscriptionDidStart (m, id, data);
        }

        void subscriptionWillEnd (MUID m, const ci::Subscription& subscription) override
        {
            if (auto* d = device.options.getPropertyDelegate())
                d->subscriptionWillEnd (m, subscription);
        }

    private:
        Impl& device;
    };

    std::optional<RequestKey> sendPropertySubscribe (MUID m,
                                                     const PropertySubscriptionHeader& header,
                                                     std::function<void (const PropertyExchangeResult&)> onResult) override
    {
        const auto iter = discovered.find (m);

        if (iter == discovered.end())
            return {};

        const auto primed = iter->second.initiatorPropertyCaches.primeCache (propertyDelegate.getNumSimultaneousRequestsSupported(),
                                                                             std::move (onResult));

        if (! primed.has_value())
            return {};

        const auto id = iter->second.initiatorPropertyCaches.getRequestIdForToken (*primed);
        jassert (id.has_value());

        detail::PropertyHostUtils::send (concreteBufferOutput,
                                         options.getFunctionBlock().firstGroup,
                                         detail::MessageMeta::Meta<Message::PropertySubscribe>::subID2,
                                         m,
                                         id->asByte(),
                                         Encodings::jsonTo7BitText (header.toVarCondensed()),
                                         {},
                                         cacheProvider.getMaxSysexSizeForMuid (m));

        return RequestKey (m, *primed);
    }

    void propertySubscriptionChanged (SubscriptionKey key, const std::optional<String>& subscribeId) override
    {
        listeners.call ([&] (auto& l) { l.propertySubscriptionChanged (key, subscribeId); });
    }

    static MUID getReallyRandomMuid()
    {
        Random random;
        return MUID::makeRandom (random);
    }

    static DeviceOptions getValidated (DeviceOptions opt)
    {
        opt = opt.withMaxSysExSize (jmax ((size_t) 128, opt.getMaxSysExSize()));

        if (opt.getFeatures().isPropertyExchangeSupported())
            opt = opt.withMaxSysExSize (jmax ((size_t) 512, opt.getMaxSysExSize()));

        opt = opt.withFeatures (opt.getFeatures().withProcessInquirySupported (false));

        // You'll need to provide some outputs if you want the device to talk to the outside world!
        jassert (! opt.getOutputs().empty());

        return opt;
    }

    template <typename Member>
    bool supportsFlag (MUID m, Member member) const
    {
        const auto iter = discovered.find (m);
        return iter != discovered.end() && (Features (iter->second.discovery.capabilities).*member)();
    }

    bool supportsProfiles (MUID m) const
    {
        return supportsFlag (m, &Features::isProfileConfigurationSupported);
    }

    bool supportsProperties (MUID m) const
    {
        return supportsFlag (m, &Features::isPropertyExchangeSupported);
    }

    DeviceOptions options;
    MUID muid;
    std::vector<std::byte> outgoing;
    std::map<MUID, Discovered> discovered;
    SubscriptionManager subscriptionManager { *this };
    ListenerList<Listener> listeners;
    ConcreteBufferOutput concreteBufferOutput { *this };
    CacheProviderImpl cacheProvider { *this };
    ProfileDelegateImpl profileDelegate { *this };
    PropertyDelegateImpl propertyDelegate { *this };
    std::optional<ProfileHost> profileHost;
    std::optional<PropertyHost> propertyHost;
};

//==============================================================================
Device::Device (const Options& opt) : pimpl (std::make_unique<Impl> (opt)) {}
Device::~Device() = default;
Device::Device (Device&&) noexcept = default;
Device& Device::operator= (Device&&) noexcept = default;

void Device::processMessage (ump::BytesOnGroup msg) { pimpl->processMessage (msg); }
void Device::sendDiscovery() { pimpl->sendDiscovery(); }
void Device::sendEndpointInquiry (MUID destination, Message::EndpointInquiry endpoint) { pimpl->sendEndpointInquiry (destination, endpoint); }
void Device::sendProfileInquiry (MUID destination, ChannelInGroup address) { pimpl->sendProfileInquiry (destination, address); }
void Device::sendProfileDetailsInquiry (MUID destination, ChannelInGroup address, Profile profile, std::byte target)
{
    pimpl->sendProfileDetailsInquiry (destination, address, profile, target);
}
void Device::sendProfileSpecificData (MUID destination, ChannelInGroup address, Profile profile, Span<const std::byte> data)
{
    pimpl->sendProfileSpecificData (destination, address, profile, data);
}
void Device::sendProfileEnablement (MUID destination, ChannelInGroup address, Profile profile, int numChannels)
{
    pimpl->sendProfileEnablement (destination, address, profile, numChannels);
}
void Device::sendPropertyCapabilitiesInquiry (MUID destination)
{
    pimpl->sendPropertyCapabilitiesInquiry (destination);
}

std::optional<RequestKey> Device::sendPropertyGetInquiry (MUID m,
                                                          const PropertyRequestHeader& header,
                                                          std::function<void (const PropertyExchangeResult&)> onResult)
{
    return pimpl->sendPropertyGetInquiry (m, header, std::move (onResult));
}

std::optional<RequestKey> Device::sendPropertySetInquiry (MUID m,
                                                          const PropertyRequestHeader& header,
                                                          Span<const std::byte> body,
                                                          std::function<void (const PropertyExchangeResult&)> onResult)
{
    return pimpl->sendPropertySetInquiry (m, header, body, std::move (onResult));
}
void Device::abortPropertyRequest (RequestKey key) { pimpl->abortPropertyRequest (key); }
std::optional<RequestID> Device::getIdForRequestKey (RequestKey key) const { return pimpl->getIdForRequestKey (key); }
std::vector<RequestKey> Device::getOngoingRequests() const { return pimpl->getOngoingRequests(); }

SubscriptionKey Device::beginSubscription (MUID m, const PropertySubscriptionHeader& header) { return pimpl->beginSubscription (m, header); }
void Device::endSubscription (SubscriptionKey key) { pimpl->endSubscription (key); }
std::vector<SubscriptionKey> Device::getOngoingSubscriptions() const { return pimpl->getOngoingSubscriptions(); }
std::optional<String> Device::getSubscribeIdForKey (SubscriptionKey key) const { return pimpl->getSubscribeIdForKey (key); }
std::optional<String> Device::getResourceForKey (SubscriptionKey key) const { return pimpl->getResourceForKey (key); }
bool Device::sendPendingMessages() { return pimpl->sendPendingMessages(); }

void Device::addListener (Listener& l) { pimpl->addListener (l); }
void Device::removeListener (Listener& l) { pimpl->removeListener (l); }
MUID Device::getMuid() const { return pimpl->getMuid(); }
DeviceOptions Device::getOptions() const { return pimpl->getOptions(); }
std::vector<MUID> Device::getDiscoveredMuids() const { return pimpl->getDiscoveredMuids(); }
const ProfileHost* Device::getProfileHost() const { return pimpl->getProfileHost(); }
ProfileHost* Device::getProfileHost() { return pimpl->getProfileHost(); }
const PropertyHost* Device::getPropertyHost() const { return pimpl->getPropertyHost(); }
PropertyHost* Device::getPropertyHost() { return pimpl->getPropertyHost(); }
std::optional<Message::Discovery> Device::getDiscoveryInfoForMuid (MUID m) const { return pimpl->getDiscoveryInfoForMuid (m); }
const ChannelProfileStates* Device::getProfileStateForMuid (MUID m, ChannelAddress address) const { return pimpl->getProfileStateForMuid (m, address); }
std::optional<int> Device::getNumPropertyExchangeRequestsSupportedForMuid (MUID m) const
{
    return pimpl->getNumPropertyExchangeRequestsSupportedForMuid (m);
}
var Device::getResourceListForMuid (MUID x) const { return pimpl->getResourceListForMuid (x); }
var Device::getDeviceInfoForMuid (MUID x) const { return pimpl->getDeviceInfoForMuid (x); }
var Device::getChannelListForMuid (MUID x) const { return pimpl->getChannelListForMuid (x); }

//==============================================================================
//==============================================================================
#if JUCE_UNIT_TESTS

class DeviceTests : public UnitTest
{
public:
    DeviceTests() : UnitTest ("Device", UnitTestCategories::midi) {}

    void runTest() override
    {
        auto random = getRandom();

        struct GroupOutput
        {
            uint8_t group;
            std::vector<std::byte> bytes;

            bool operator== (const GroupOutput& other) const
            {
                const auto tie = [] (const auto& x) { return std::tie (x.group, x.bytes); };
                return tie (*this) == tie (other);
            }

            bool operator!= (const GroupOutput& other) const { return ! operator== (other); }
        };

        struct Output : public DeviceMessageHandler
        {
            void processMessage (ump::BytesOnGroup msg) override
            {
                messages.push_back ({ msg.group, std::vector<std::byte> (msg.bytes.begin(), msg.bytes.end()) });
            }

            std::vector<GroupOutput> messages;
        };

        const ump::DeviceInfo deviceInfo { { std::byte { 0x01 }, std::byte { 0x02 }, std::byte { 0x03 } },
                                           { std::byte { 0x11 }, std::byte { 0x12 } },
                                           { std::byte { 0x21 }, std::byte { 0x22 } },
                                           { std::byte { 0x31 }, std::byte { 0x32 }, std::byte { 0x33 }, std::byte { 0x34 } } };

        FunctionBlock functionBlock;

        beginTest ("When receiving Discovery from a MUID that matches the Device MUID, reply with InvalidateMUID and initiate discovery");
        {
            Output output;
            const auto options = DeviceOptions().withOutputs ({ &output })
                                                .withFunctionBlock (functionBlock)
                                                .withDeviceInfo (deviceInfo)
                                                .withMaxSysExSize (512);
            Device device { options };

            const auto commonMUID = device.getMuid();

            device.processMessage ({ 0, getMessageBytes ({ ChannelInGroup::wholeBlock,
                                                           detail::MessageMeta::Meta<Message::Discovery>::subID2,
                                                           detail::MessageMeta::implementationVersion,
                                                           commonMUID,
                                                           MUID::getBroadcast() },
                                                         Message::Discovery { ump::DeviceInfo { { std::byte { 0x05 }, std::byte { 0x06 }, std::byte { 0x07 } },
                                                                                                { std::byte { 0x15 }, std::byte { 0x16 } },
                                                                                                { std::byte { 0x25 }, std::byte { 0x26 } },
                                                                                                { std::byte { 0x35 }, std::byte { 0x36 }, std::byte { 0x37 }, std::byte { 0x38 } } },
                                                                              std::byte{},
                                                                              1024,
                                                                              std::byte{} }) });

            expect (device.getMuid() != commonMUID);
            const std::vector<GroupOutput> responses
            {
                { 0, getMessageBytes ({ ChannelInGroup::wholeBlock,
                                        detail::MessageMeta::Meta<Message::InvalidateMUID>::subID2,
                                        detail::MessageMeta::implementationVersion,
                                        commonMUID,
                                        MUID::getBroadcast() },
                                      Message::InvalidateMUID { commonMUID }) },
                { 0, getMessageBytes ({ ChannelInGroup::wholeBlock,
                                        detail::MessageMeta::Meta<Message::Discovery>::subID2,
                                        detail::MessageMeta::implementationVersion,
                                        device.getMuid(),
                                        MUID::getBroadcast() },
                                      Message::Discovery { deviceInfo, std::byte{}, 512, std::byte{} }) },
            };
            expect (output.messages == responses);
        }

        beginTest ("When receiving Discovery from a MUID that does not match the Device MUID, reply with DiscoveryResponse and EndpointInquiry");
        {
            Output output;
            const auto options = DeviceOptions().withOutputs ({ &output })
                                                .withFunctionBlock (functionBlock)
                                                .withDeviceInfo (deviceInfo)
                                                .withMaxSysExSize (512);
            Device device { options };

            const auto responderMUID = device.getMuid();
            const auto initiatorMUID = MUID::makeRandom (random);

            device.processMessage ({ 0, getMessageBytes ({ ChannelInGroup::wholeBlock,
                                                           detail::MessageMeta::Meta<Message::Discovery>::subID2,
                                                           detail::MessageMeta::implementationVersion,
                                                           initiatorMUID,
                                                           MUID::getBroadcast() },
                                                         Message::Discovery { ump::DeviceInfo { { std::byte { 0x05 }, std::byte { 0x06 }, std::byte { 0x07 } },
                                                                                                { std::byte { 0x15 }, std::byte { 0x16 } },
                                                                                                { std::byte { 0x25 }, std::byte { 0x26 } },
                                                                                                { std::byte { 0x35 }, std::byte { 0x36 }, std::byte { 0x37 }, std::byte { 0x38 } } },
                                                                              std::byte{},
                                                                              1024,
                                                                              std::byte{} }) });

            expect (device.getMuid() == responderMUID);
            const std::vector<GroupOutput> responses
            {
                { 0, getMessageBytes ({ ChannelInGroup::wholeBlock,
                                        detail::MessageMeta::Meta<Message::DiscoveryResponse>::subID2,
                                        detail::MessageMeta::implementationVersion,
                                        responderMUID,
                                        initiatorMUID },
                                      Message::DiscoveryResponse { deviceInfo, std::byte{}, 512, std::byte{}, std::byte { 0x7f } }) },
                { 0, getMessageBytes ({ ChannelInGroup::wholeBlock,
                                        detail::MessageMeta::Meta<Message::EndpointInquiry>::subID2,
                                        detail::MessageMeta::implementationVersion,
                                        responderMUID,
                                        initiatorMUID },
                                      Message::EndpointInquiry { std::byte{} }) },
            };
            expect (output.messages == responses);
        }

        beginTest ("Sending a V1 discovery message notifies the listener");
        {
            Output output;
            const auto options = DeviceOptions().withOutputs ({ &output })
                                                .withFunctionBlock (functionBlock)
                                                .withDeviceInfo (deviceInfo)
                                                .withMaxSysExSize (512);
            Device device { options };

            const auto responderMUID = device.getMuid();
            const auto initiatorMUID = MUID::makeRandom (random);
            constexpr uint8_t version = 0x01;

            auto bytes = getMessageBytes ({ ChannelInGroup::wholeBlock,
                                            detail::MessageMeta::Meta<Message::Discovery>::subID2,
                                            std::byte { version },
                                            initiatorMUID,
                                            MUID::getBroadcast() },
                                          Message::Discovery { ump::DeviceInfo { { std::byte { 0x05 }, std::byte { 0x06 }, std::byte { 0x07 } },
                                                                                 { std::byte { 0x15 }, std::byte { 0x16 } },
                                                                                 { std::byte { 0x25 }, std::byte { 0x26 } },
                                                                                 { std::byte { 0x35 }, std::byte { 0x36 }, std::byte { 0x37 }, std::byte { 0x38 } } },
                                                               std::byte{},
                                                               1024,
                                                               std::byte{} });

            // V1 message doesn't have an output path
            bytes.pop_back();
            device.processMessage ({ 0, bytes });

            expect (device.getMuid() == responderMUID);
            const std::vector<GroupOutput> responses
            {
                { 0, getMessageBytes ({ ChannelInGroup::wholeBlock,
                                        detail::MessageMeta::Meta<Message::DiscoveryResponse>::subID2,
                                        detail::MessageMeta::implementationVersion,
                                        responderMUID,
                                        initiatorMUID },
                                      Message::DiscoveryResponse { deviceInfo, std::byte{}, 512, std::byte{}, std::byte { 0x7f } }) },
                { 0, getMessageBytes ({ ChannelInGroup::wholeBlock,
                                        detail::MessageMeta::Meta<Message::EndpointInquiry>::subID2,
                                        detail::MessageMeta::implementationVersion,
                                        responderMUID,
                                        initiatorMUID },
                                      Message::EndpointInquiry { std::byte{} }) },
            };
            expect (output.messages == responses);
        }

        beginTest ("Sending a V2 discovery message notifies the input listener");
        {
            constexpr std::byte outputPathID { 5 };
            const auto initiatorMUID = MUID::makeRandom (random);
            constexpr std::byte version { 0x02 };

            Output output;
            const auto options = DeviceOptions().withOutputs ({ &output })
                                                .withFunctionBlock (functionBlock)
                                                .withDeviceInfo (deviceInfo)
                                                .withMaxSysExSize (512);
            Device device { options };

            const auto responderMUID = device.getMuid();

            device.processMessage ({ 0, getMessageBytes ({ ChannelInGroup::wholeBlock,
                                                           detail::MessageMeta::Meta<Message::Discovery>::subID2,
                                                           version,
                                                           initiatorMUID,
                                                           MUID::getBroadcast() },
                                                         Message::Discovery { ump::DeviceInfo { { std::byte { 0x05 }, std::byte { 0x06 }, std::byte { 0x07 } },
                                                                                                { std::byte { 0x15 }, std::byte { 0x16 } },
                                                                                                { std::byte { 0x25 }, std::byte { 0x26 } },
                                                                                                { std::byte { 0x35 }, std::byte { 0x36 }, std::byte { 0x37 }, std::byte { 0x38 } } },
                                                                              std::byte{},
                                                                              1024,
                                                                              outputPathID }) });

            expect (device.getMuid() == responderMUID);
            const std::vector<GroupOutput> responses
            {
                { 0, getMessageBytes ({ ChannelInGroup::wholeBlock,
                                        detail::MessageMeta::Meta<Message::DiscoveryResponse>::subID2,
                                        detail::MessageMeta::implementationVersion,
                                        responderMUID,
                                        initiatorMUID },
                                      Message::DiscoveryResponse { deviceInfo, std::byte{}, 512, outputPathID, std::byte { 0x7f } }) },
                { 0, getMessageBytes ({ ChannelInGroup::wholeBlock,
                                        detail::MessageMeta::Meta<Message::EndpointInquiry>::subID2,
                                        detail::MessageMeta::implementationVersion,
                                        responderMUID,
                                        initiatorMUID },
                                      Message::EndpointInquiry { std::byte{} }) },
            };
            expect (output.messages == responses);
        }

        beginTest ("Sending a discovery message with a future version notifies the input listener and ignores trailing fields");
        {
            constexpr std::byte outputPathID { 10 };
            const auto initiatorMUID = MUID::makeRandom (random);
            constexpr std::byte version { 0x03 };

            Output output;
            const auto options = DeviceOptions().withOutputs ({ &output })
                                                .withFunctionBlock (functionBlock)
                                                .withDeviceInfo (deviceInfo)
                                                .withMaxSysExSize (512);
            Device device { options };

            const auto responderMUID = device.getMuid();

            auto bytes = getMessageBytes ({ ChannelInGroup::wholeBlock,
                                            detail::MessageMeta::Meta<Message::Discovery>::subID2,
                                            version,
                                            initiatorMUID,
                                            MUID::getBroadcast() },
                                          Message::Discovery { ump::DeviceInfo { { std::byte { 0x05 }, std::byte { 0x06 }, std::byte { 0x07 } },
                                                                                 { std::byte { 0x15 }, std::byte { 0x16 } },
                                                                                 { std::byte { 0x25 }, std::byte { 0x26 } },
                                                                                 { std::byte { 0x35 }, std::byte { 0x36 }, std::byte { 0x37 }, std::byte { 0x38 } } },
                                                               std::byte{},
                                                               1024,
                                                               outputPathID });

            // Future versions might have more trailing bytes
            bytes.insert (bytes.end(), { std::byte{}, std::byte{} });
            device.processMessage ({ 0, bytes });

            expect (device.getMuid() == responderMUID);
            const std::vector<GroupOutput> responses
            {
                { 0, getMessageBytes ({ ChannelInGroup::wholeBlock,
                                        detail::MessageMeta::Meta<Message::DiscoveryResponse>::subID2,
                                        detail::MessageMeta::implementationVersion,
                                        responderMUID,
                                        initiatorMUID },
                                      Message::DiscoveryResponse { deviceInfo, std::byte{}, 512, outputPathID, std::byte { 0x7f } }) },
                { 0, getMessageBytes ({ ChannelInGroup::wholeBlock,
                                        detail::MessageMeta::Meta<Message::EndpointInquiry>::subID2,
                                        detail::MessageMeta::implementationVersion,
                                        responderMUID,
                                        initiatorMUID },
                                      Message::EndpointInquiry { std::byte{} }) },
            };
            expect (output.messages == responses);
        }

        beginTest ("When receiving an InvalidateMUID that matches the Device MUID, initiate discovery using a new MUID");
        {
            Output output;
            const auto options = DeviceOptions().withOutputs ({ &output })
                                                .withFunctionBlock (functionBlock)
                                                .withDeviceInfo (deviceInfo)
                                                .withMaxSysExSize (512);
            Device device { options };

            const auto deviceMUID = device.getMuid();

            device.processMessage ({ 0, getMessageBytes ({ ChannelInGroup::wholeBlock,
                                                           detail::MessageMeta::Meta<Message::InvalidateMUID>::subID2,
                                                           detail::MessageMeta::implementationVersion,
                                                           MUID::makeRandom (random),
                                                           MUID::getBroadcast() },
                                                         Message::InvalidateMUID { deviceMUID }) });

            expect (device.getMuid() != deviceMUID);

            expect (Parser::parse (MUID::makeRandom (random), output.messages.front().bytes) == Message::Parsed { { ChannelInGroup::wholeBlock,
                                                                                                                    detail::MessageMeta::Meta<Message::Discovery>::subID2,
                                                                                                                    detail::MessageMeta::implementationVersion,
                                                                                                                    device.getMuid(),
                                                                                                                    MUID::getBroadcast() },
                                                                                                                  Message::Discovery { deviceInfo,
                                                                                                                                       {},
                                                                                                                                       512,
                                                                                                                                       {} } });
        }

        struct Listener : public DeviceListener
        {
            void deviceAdded   (MUID x) override { added  .push_back (x); }
            void deviceRemoved (MUID x) override { removed.push_back (x); }

            std::vector<MUID> added, removed;
        };

        beginTest ("When receiving a DiscoveryResponse, update the set of known devices, notify outputs, and request endpoint info");
        {
            Listener delegate;
            Output output;
            const auto options = DeviceOptions().withOutputs ({ &output })
                                                .withFunctionBlock (functionBlock)
                                                .withDeviceInfo (deviceInfo)
                                                .withMaxSysExSize (512);
            Device device { options };
            device.addListener (delegate);

            expect (device.getDiscoveredMuids().empty());

            const auto deviceMUID = device.getMuid();
            const auto responderMUID = MUID::makeRandom (random);

            device.processMessage ({ 0, getMessageBytes ({ ChannelInGroup::wholeBlock,
                                                           detail::MessageMeta::Meta<Message::DiscoveryResponse>::subID2,
                                                           detail::MessageMeta::implementationVersion,
                                                           responderMUID,
                                                           deviceMUID },
                                                         Message::DiscoveryResponse { deviceInfo, std::byte{}, 512, std::byte{}, std::byte { 0x7f } }) });

            expect (device.getDiscoveredMuids() == std::vector { responderMUID });
            expect (delegate.added == std::vector { responderMUID });

            std::vector<GroupOutput> responses
            {
                { 0, getMessageBytes ({ ChannelInGroup::wholeBlock,
                                        detail::MessageMeta::Meta<Message::EndpointInquiry>::subID2,
                                        detail::MessageMeta::implementationVersion,
                                        deviceMUID,
                                        responderMUID },
                                      Message::EndpointInquiry { std::byte{} }) },
            };
            expect (output.messages == responses);

            beginTest ("When receiving a DiscoveryResponse with a MUID that matches a known device, invalidate that MUID");
            {
                device.processMessage ({ 0, getMessageBytes ({ ChannelInGroup::wholeBlock,
                                                               detail::MessageMeta::Meta<Message::DiscoveryResponse>::subID2,
                                                               detail::MessageMeta::implementationVersion,
                                                               responderMUID,
                                                               deviceMUID },
                                                             Message::DiscoveryResponse { deviceInfo, std::byte{}, 512, std::byte{}, std::byte { 0x7f } }) });

                expect (device.getDiscoveredMuids().empty());
                expect (delegate.removed == std::vector { responderMUID });

                responses.push_back ({ 0, getMessageBytes ({ ChannelInGroup::wholeBlock,
                                                             detail::MessageMeta::Meta<Message::InvalidateMUID>::subID2,
                                                             detail::MessageMeta::implementationVersion,
                                                             deviceMUID,
                                                             MUID::getBroadcast() },
                                                           Message::InvalidateMUID { responderMUID }) });
                expect (output.messages == responses);
            }
        }

        beginTest ("After receiving an EndpointResponse, the listener is notified");
        {
            static constexpr std::byte dataBytes[] { std::byte { 0x01 }, std::byte { 0x7f }, std::byte { 0x41 } };

            struct EndpointListener : public DeviceListener
            {
                EndpointListener (UnitTest& t, Device& d) : test (t), device (d) {}

                void endpointReceived (MUID, Message::EndpointInquiryResponse) override { called = true; }

                UnitTest& test;
                Device& device;
                bool called = false;
            };

            Output output;
            const auto options = DeviceOptions().withOutputs ({ &output })
                                                .withFunctionBlock (functionBlock)
                                                .withDeviceInfo (deviceInfo)
                                                .withMaxSysExSize (512);
            Device device { options };

            EndpointListener delegate { *this, device };
            device.addListener (delegate);

            const auto responderMUID = MUID::makeRandom (random);
            const auto deviceMUID = device.getMuid();

            device.processMessage ({ 0, getMessageBytes ({ ChannelInGroup::wholeBlock,
                                                           detail::MessageMeta::Meta<Message::DiscoveryResponse>::subID2,
                                                           detail::MessageMeta::implementationVersion,
                                                           responderMUID,
                                                           deviceMUID },
                                                         Message::DiscoveryResponse { deviceInfo, std::byte{}, 512, std::byte{}, std::byte { 0x7f } }) });

            expect (! delegate.called);

            device.processMessage ({ 0, getMessageBytes ({ ChannelInGroup::wholeBlock,
                                                           detail::MessageMeta::Meta<Message::EndpointInquiryResponse>::subID2,
                                                           detail::MessageMeta::implementationVersion,
                                                           responderMUID,
                                                           deviceMUID },
                                                         Message::EndpointInquiryResponse { std::byte{}, dataBytes }) });

            expect (delegate.called);
        }

        beginTest ("If a device has not previously acted as a responder, modifying profiles does not emit events");
        {
            Output output;

            const auto options = DeviceOptions().withOutputs ({ &output })
                                                .withFunctionBlock (functionBlock)
                                                .withDeviceInfo (deviceInfo)
                                                .withMaxSysExSize (512)
                                                .withFeatures (DeviceFeatures{}.withProfileConfigurationSupported (true));
            Device device { options };

            expect (device.getProfileHost() != nullptr);

            const Profile profile { std::byte { 0x01 },
                                    std::byte { 0x02 },
                                    std::byte { 0x03 },
                                    std::byte { 0x04 },
                                    std::byte { 0x05 } };

            device.getProfileHost()->addProfile ({ profile, ChannelAddress{}.withChannel (ChannelInGroup::wholeBlock) });

            expect (output.messages.empty());

            beginTest ("The device reports profiles accurately");
            {
                const auto inquiryMUID = MUID::makeRandom (random);
                device.processMessage ({ 0, getMessageBytes ({ ChannelInGroup::wholeBlock,
                                                               detail::MessageMeta::Meta<Message::ProfileInquiry>::subID2,
                                                               detail::MessageMeta::implementationVersion,
                                                               inquiryMUID,
                                                               device.getMuid() },
                                                             Message::ProfileInquiry{}) });

                const Profile disabledProfiles[] { profile };
                expect (output.messages.size() == 1);
                expect (output.messages.back().bytes == getMessageBytes ({ ChannelInGroup::wholeBlock,
                                                                           detail::MessageMeta::Meta<Message::ProfileInquiryResponse>::subID2,
                                                                           detail::MessageMeta::implementationVersion,
                                                                           device.getMuid(),
                                                                           inquiryMUID },
                                                                         Message::ProfileInquiryResponse { {}, disabledProfiles }));
            }

            beginTest ("If a device has previously acted as a responder to profile inquiry, then modifying profiles emits events");
            {
                device.getProfileHost()->setProfileEnablement ({ profile, ChannelAddress{}.withChannel (ChannelInGroup::wholeBlock) }, 1);

                expect (output.messages.size() == 2);
                expect (output.messages.back().bytes == getMessageBytes ({ ChannelInGroup::wholeBlock,
                                                                           detail::MessageMeta::Meta<Message::ProfileEnabledReport>::subID2,
                                                                           detail::MessageMeta::implementationVersion,
                                                                           device.getMuid(),
                                                                           MUID::getBroadcast() },
                                                                         Message::ProfileEnabledReport { profile, 0 }));
            }
        }

        beginTest ("If a device receives a details inquiry message addressed to an unsupported profile, a NAK with a code of 0x04 is emitted");
        {
            Output output;
            const auto options = DeviceOptions().withOutputs ({ &output })
                                                .withFunctionBlock (functionBlock)
                                                .withDeviceInfo (deviceInfo)
                                                .withMaxSysExSize (512)
                                                .withFeatures (DeviceFeatures{}.withProfileConfigurationSupported (true));
            Device device { options };

            expect (device.getProfileHost() != nullptr);

            const auto inquiryMUID = MUID::makeRandom (random);

            const Profile profile { std::byte { 0x01 },
                                    std::byte { 0x02 },
                                    std::byte { 0x03 },
                                    std::byte { 0x04 },
                                    std::byte { 0x05 } };
            device.processMessage ({ 0, getMessageBytes ({ ChannelInGroup::wholeBlock,
                                                           detail::MessageMeta::Meta<Message::ProfileDetails>::subID2,
                                                           detail::MessageMeta::implementationVersion,
                                                           inquiryMUID,
                                                           device.getMuid() },
                                                         Message::ProfileDetails { profile, std::byte { 0x02 } }) });

            expect (output.messages.size() == 1);
            expect (output.messages.back().bytes == getMessageBytes ({ ChannelInGroup::wholeBlock,
                                                                       detail::MessageMeta::Meta<Message::NAK>::subID2,
                                                                       detail::MessageMeta::implementationVersion,
                                                                       device.getMuid(),
                                                                       inquiryMUID },
                                                                     Message::NAK { detail::MessageMeta::Meta<Message::ProfileDetails>::subID2,
                                                                                    std::byte { 0x04 },
                                                                                    {},
                                                                                    {},
                                                                                    {} }));
        }

        beginTest ("If a device receives a set profile on and enables the profile, profile enabled report is emitted");
        {
            // Note: if there's no explicit profile delegate, the device will toggle profiles as requested.
            Output output;
            const auto options = DeviceOptions().withOutputs ({ &output })
                                                .withFunctionBlock (functionBlock)
                                                .withDeviceInfo (deviceInfo)
                                                .withMaxSysExSize (512)
                                                .withFeatures (DeviceFeatures{}.withProfileConfigurationSupported (true));
            Device device { options };
            expect (device.getProfileHost() != nullptr);

            const Profile profile { std::byte { 0x01 },
                                    std::byte { 0x02 },
                                    std::byte { 0x03 },
                                    std::byte { 0x04 },
                                    std::byte { 0x05 } };

            device.getProfileHost()->addProfile ({ profile, ChannelAddress{}.withChannel (ChannelInGroup::wholeBlock) });

            const auto inquiryMUID = MUID::makeRandom (random);

            device.processMessage ({ 0, getMessageBytes ({ ChannelInGroup::wholeBlock,
                                                           detail::MessageMeta::Meta<Message::ProfileOn>::subID2,
                                                           detail::MessageMeta::implementationVersion,
                                                           inquiryMUID,
                                                           device.getMuid() },
                                                         Message::ProfileOn { profile, 0 }) });

            expect (output.messages.size() == 1);
            expect (output.messages.back().bytes == getMessageBytes ({ ChannelInGroup::wholeBlock,
                                                                       detail::MessageMeta::Meta<Message::ProfileEnabledReport>::subID2,
                                                                       detail::MessageMeta::implementationVersion,
                                                                       device.getMuid(),
                                                                       MUID::getBroadcast() },
                                                                     Message::ProfileEnabledReport { profile, 0 }));
        }

        struct DoNothingProfileDelegate : public ProfileDelegate
        {
            void profileEnablementRequested (MUID, ProfileAtAddress, int, bool) override {}
        };

        beginTest ("If a device receives a set profile on but then doesn't enable the profile, profile disabled report is emitted");
        {
            DoNothingProfileDelegate delegate;
            Output output;
            const auto options = DeviceOptions().withOutputs ({ &output })
                                                .withFunctionBlock (functionBlock)
                                                .withDeviceInfo (deviceInfo)
                                                .withMaxSysExSize (512)
                                                .withFeatures (DeviceFeatures{}.withProfileConfigurationSupported (true))
                                                .withProfileDelegate (&delegate);
            Device device { options };

            expect (device.getProfileHost() != nullptr);

            const Profile profile { std::byte { 0x01 },
                                    std::byte { 0x02 },
                                    std::byte { 0x03 },
                                    std::byte { 0x04 },
                                    std::byte { 0x05 } };

            device.getProfileHost()->addProfile ({ profile, ChannelAddress{}.withChannel (ChannelInGroup::wholeBlock) });

            const auto inquiryMUID = MUID::makeRandom (random);

            device.processMessage ({ 0, getMessageBytes ({ ChannelInGroup::wholeBlock,
                                                           detail::MessageMeta::Meta<Message::ProfileOn>::subID2,
                                                           detail::MessageMeta::implementationVersion,
                                                           inquiryMUID,
                                                           device.getMuid() },
                                                         Message::ProfileOn { profile, 1 }) });

            expect (output.messages.size() == 1);
            expect (output.messages.back().bytes == getMessageBytes ({ ChannelInGroup::wholeBlock,
                                                                       detail::MessageMeta::Meta<Message::ProfileDisabledReport>::subID2,
                                                                       detail::MessageMeta::implementationVersion,
                                                                       device.getMuid(),
                                                                       MUID::getBroadcast() },
                                                                     Message::ProfileDisabledReport { profile, {} }));
        }

        beginTest ("If a device receives a set profile on for an unsupported profile, NAK is emitted");
        {
            Output output;
            const auto options = DeviceOptions().withOutputs ({ &output })
                                                .withFunctionBlock (functionBlock)
                                                .withDeviceInfo (deviceInfo)
                                                .withMaxSysExSize (512)
                                                .withFeatures (DeviceFeatures{}.withProfileConfigurationSupported (true));
            Device device { options };

            expect (device.getProfileHost() != nullptr);

            const Profile profile { std::byte { 0x01 },
                                    std::byte { 0x02 },
                                    std::byte { 0x03 },
                                    std::byte { 0x04 },
                                    std::byte { 0x05 } };

            const auto inquiryMUID = MUID::makeRandom (random);

            device.processMessage ({ 0, getMessageBytes ({ ChannelInGroup::wholeBlock,
                                                           detail::MessageMeta::Meta<Message::ProfileOn>::subID2,
                                                           detail::MessageMeta::implementationVersion,
                                                           inquiryMUID,
                                                           device.getMuid() },
                                                         Message::ProfileOn { profile, 1 }) });

            expect (output.messages.size() == 1);
            expect (output.messages.back().bytes == getMessageBytes ({ ChannelInGroup::wholeBlock,
                                                                       detail::MessageMeta::Meta<Message::NAK>::subID2,
                                                                       detail::MessageMeta::implementationVersion,
                                                                       device.getMuid(),
                                                                       inquiryMUID },
                                                                     Message::NAK { detail::MessageMeta::Meta<Message::ProfileOn>::subID2,
                                                                                    {},
                                                                                    {},
                                                                                    {},
                                                                                    {} }));
        }

        beginTest ("If a device receives a set profile off and disables the profile, profile disabled report is emitted");
        {
            // Note: if there's no explicit profile delegate, the device will toggle profiles as requested.
            Output output;
            const auto options = DeviceOptions().withOutputs ({ &output })
                                                .withFunctionBlock (functionBlock)
                                                .withDeviceInfo (deviceInfo)
                                                .withMaxSysExSize (512)
                                                .withFeatures (DeviceFeatures{}.withProfileConfigurationSupported (true));
            Device device { options };

            expect (device.getProfileHost() != nullptr);

            const Profile profile { std::byte { 0x01 },
                                    std::byte { 0x02 },
                                    std::byte { 0x03 },
                                    std::byte { 0x04 },
                                    std::byte { 0x05 } };

            device.getProfileHost()->addProfile ({ profile, ChannelAddress{}.withChannel (ChannelInGroup::wholeBlock) });
            device.getProfileHost()->setProfileEnablement ({ profile, ChannelAddress{}.withChannel (ChannelInGroup::wholeBlock) }, 0);

            const auto inquiryMUID = MUID::makeRandom (random);

            device.processMessage ({ 0, getMessageBytes ({ ChannelInGroup::wholeBlock,
                                                           detail::MessageMeta::Meta<Message::ProfileOff>::subID2,
                                                           detail::MessageMeta::implementationVersion,
                                                           inquiryMUID,
                                                           device.getMuid() },
                                                         Message::ProfileOff { profile }) });

            expect (output.messages.size() == 1);
            expect (output.messages.back().bytes == getMessageBytes ({ ChannelInGroup::wholeBlock,
                                                                       detail::MessageMeta::Meta<Message::ProfileDisabledReport>::subID2,
                                                                       detail::MessageMeta::implementationVersion,
                                                                       device.getMuid(),
                                                                       MUID::getBroadcast() },
                                                                     Message::ProfileDisabledReport { profile, {} }));
        }

        beginTest ("If a device receives a set profile off but then doesn't disable the profile, profile enabled report is emitted");
        {
            Output output;
            DoNothingProfileDelegate delegate;
            const auto options = DeviceOptions().withOutputs ({ &output })
                                                .withFunctionBlock (functionBlock)
                                                .withDeviceInfo (deviceInfo)
                                                .withMaxSysExSize (512)
                                                .withFeatures (DeviceFeatures{}.withProfileConfigurationSupported (true))
                                                .withProfileDelegate (&delegate);
            Device device { options };

            expect (device.getProfileHost() != nullptr);

            const Profile profile { std::byte { 0x01 },
                                    std::byte { 0x02 },
                                    std::byte { 0x03 },
                                    std::byte { 0x04 },
                                    std::byte { 0x05 } };

            device.getProfileHost()->addProfile ({ profile, ChannelAddress{}.withChannel (ChannelInGroup::wholeBlock) });
            device.getProfileHost()->setProfileEnablement ({ profile, ChannelAddress{}.withChannel (ChannelInGroup::wholeBlock) }, 1);

            const auto inquiryMUID = MUID::makeRandom (random);

            device.processMessage ({ 0, getMessageBytes ({ ChannelInGroup::wholeBlock,
                                                           detail::MessageMeta::Meta<Message::ProfileOff>::subID2,
                                                           detail::MessageMeta::implementationVersion,
                                                           inquiryMUID,
                                                           device.getMuid() },
                                                         Message::ProfileOff { profile }) });

            expect (output.messages.size() == 1);
            expect (output.messages.back().bytes == getMessageBytes ({ ChannelInGroup::wholeBlock,
                                                                       detail::MessageMeta::Meta<Message::ProfileEnabledReport>::subID2,
                                                                       detail::MessageMeta::implementationVersion,
                                                                       device.getMuid(),
                                                                       MUID::getBroadcast() },
                                                                     Message::ProfileEnabledReport { profile, 0 }));
        }

        beginTest ("If a device receives a set profile off for an unsupported profile, NAK is emitted");
        {
            Output output;
            const auto options = DeviceOptions().withOutputs ({ &output })
                                                .withFunctionBlock (functionBlock)
                                                .withDeviceInfo (deviceInfo)
                                                .withMaxSysExSize (512)
                                                .withFeatures (DeviceFeatures{}.withProfileConfigurationSupported (true));
            Device device { options };

            expect (device.getProfileHost() != nullptr);

            const Profile profile { std::byte { 0x01 },
                                    std::byte { 0x02 },
                                    std::byte { 0x03 },
                                    std::byte { 0x04 },
                                    std::byte { 0x05 } };

            const auto inquiryMUID = MUID::makeRandom (random);

            device.processMessage ({ 0, getMessageBytes ({ ChannelInGroup::wholeBlock,
                                                           detail::MessageMeta::Meta<Message::ProfileOff>::subID2,
                                                           detail::MessageMeta::implementationVersion,
                                                           inquiryMUID,
                                                           device.getMuid() },
                                                         Message::ProfileOff { profile }) });

            expect (output.messages.size() == 1);
            expect (output.messages.back().bytes == getMessageBytes ({ ChannelInGroup::wholeBlock,
                                                                       detail::MessageMeta::Meta<Message::NAK>::subID2,
                                                                       detail::MessageMeta::implementationVersion,
                                                                       device.getMuid(),
                                                                       inquiryMUID },
                                                                     Message::NAK { detail::MessageMeta::Meta<Message::ProfileOff>::subID2,
                                                                                    {},
                                                                                    {},
                                                                                    {},
                                                                                    {} }));
        }

        const FunctionBlock realBlock { std::byte{}, 0, 3 };

        beginTest ("If a device receives a profile inquiry addressed to a channel, that channel's profiles are emitted");
        {
            Output output;
            const auto options = DeviceOptions().withOutputs ({ &output })
                                                .withFunctionBlock (realBlock)
                                                .withDeviceInfo (deviceInfo)
                                                .withMaxSysExSize (512)
                                                .withFeatures (DeviceFeatures{}.withProfileConfigurationSupported (true));
            Device device { options };

            auto& profileHost = *device.getProfileHost();

            Profile channel0Profile { std::byte { 0x01 } };
            Profile channel1Profile { std::byte { 0x02 } };

            profileHost.addProfile ({ channel0Profile, ChannelAddress{}.withChannel (ChannelInGroup::channel0) });
            profileHost.addProfile ({ channel1Profile, ChannelAddress{}.withChannel (ChannelInGroup::channel1) });

            const auto inquiryMUID = MUID::makeRandom (random);

            device.processMessage ({ 0, getMessageBytes ({ ChannelInGroup::channel0,
                                                           detail::MessageMeta::Meta<Message::ProfileInquiry>::subID2,
                                                           detail::MessageMeta::implementationVersion,
                                                           inquiryMUID,
                                                           device.getMuid() },
                                                         Message::ProfileInquiry{}) });

            const Profile channel0Profiles[] { channel0Profile };
            const Profile channel1Profiles[] { channel1Profile };

            expect (output.messages.size() == 1);
            expect (output.messages.back().bytes == getMessageBytes ({ ChannelInGroup::channel0,
                                                                       detail::MessageMeta::Meta<Message::ProfileInquiryResponse>::subID2,
                                                                       detail::MessageMeta::implementationVersion,
                                                                       device.getMuid(),
                                                                       inquiryMUID },
                                                                     Message::ProfileInquiryResponse { {}, channel0Profiles }));

            device.processMessage ({ 0, getMessageBytes ({ ChannelInGroup::channel2,
                                                           detail::MessageMeta::Meta<Message::ProfileInquiry>::subID2,
                                                           detail::MessageMeta::implementationVersion,
                                                           inquiryMUID,
                                                           device.getMuid() },
                                                         Message::ProfileInquiry{}) });

            expect (output.messages.back().bytes == getMessageBytes ({ ChannelInGroup::channel2,
                                                                       detail::MessageMeta::Meta<Message::ProfileInquiryResponse>::subID2,
                                                                       detail::MessageMeta::implementationVersion,
                                                                       device.getMuid(),
                                                                       inquiryMUID },
                                                                     Message::ProfileInquiryResponse { {}, {} }));

            Profile group0Profile { std::byte { 0x05 } };
            Profile group1Profile { std::byte { 0x06 } };
            const Profile group0Profiles[] { group0Profile };
            const Profile group1Profiles[] { group1Profile };

            beginTest ("If a device receives a profile inquiry addressed to a group, that group's profiles are emitted");
            {
                profileHost.addProfile ({ group0Profile, ChannelAddress{}.withGroup (0).withChannel (ChannelInGroup::wholeGroup) });
                profileHost.addProfile ({ group1Profile, ChannelAddress{}.withGroup (1).withChannel (ChannelInGroup::wholeGroup) });

                device.processMessage ({ 0, getMessageBytes ({ ChannelInGroup::wholeGroup,
                                                               detail::MessageMeta::Meta<Message::ProfileInquiry>::subID2,
                                                               detail::MessageMeta::implementationVersion,
                                                               inquiryMUID,
                                                               device.getMuid() },
                                                             Message::ProfileInquiry{}) });

                expect (output.messages.back().bytes == getMessageBytes ({ ChannelInGroup::wholeGroup,
                                                                           detail::MessageMeta::Meta<Message::ProfileInquiryResponse>::subID2,
                                                                           detail::MessageMeta::implementationVersion,
                                                                           device.getMuid(),
                                                                           inquiryMUID },
                                                                         Message::ProfileInquiryResponse { {}, group0Profiles }));

                device.processMessage ({ 2, getMessageBytes ({ ChannelInGroup::wholeGroup,
                                                               detail::MessageMeta::Meta<Message::ProfileInquiry>::subID2,
                                                               detail::MessageMeta::implementationVersion,
                                                               inquiryMUID,
                                                               device.getMuid() },
                                                             Message::ProfileInquiry{}) });

                expect (output.messages.back().bytes == getMessageBytes ({ ChannelInGroup::wholeGroup,
                                                                           detail::MessageMeta::Meta<Message::ProfileInquiryResponse>::subID2,
                                                                           detail::MessageMeta::implementationVersion,
                                                                           device.getMuid(),
                                                                           inquiryMUID },
                                                                         Message::ProfileInquiryResponse { {}, {} }));
            }

            beginTest ("If a device receives a profile inquiry addressed to a block, the profiles for member channels, then member groups, then the block are emitted");
            {
                Profile blockProfile { std::byte { 0x0a } };

                profileHost.addProfile ({ blockProfile, ChannelAddress{}.withChannel (ChannelInGroup::wholeBlock) });

                output.messages.clear();

                device.processMessage ({ 1, getMessageBytes ({ ChannelInGroup::wholeBlock,
                                                               detail::MessageMeta::Meta<Message::ProfileInquiry>::subID2,
                                                               detail::MessageMeta::implementationVersion,
                                                               inquiryMUID,
                                                               device.getMuid() },
                                                             Message::ProfileInquiry{}) });

                const Profile blockProfiles[] { blockProfile };

                expect (output.messages == std::vector<GroupOutput> { { 0, getMessageBytes ({ ChannelInGroup::channel0,
                                                                                              detail::MessageMeta::Meta<Message::ProfileInquiryResponse>::subID2,
                                                                                              detail::MessageMeta::implementationVersion,
                                                                                              device.getMuid(),
                                                                                              inquiryMUID },
                                                                                            Message::ProfileInquiryResponse { {}, channel0Profiles }) },
                                                                      { 0, getMessageBytes ({ ChannelInGroup::channel1,
                                                                                              detail::MessageMeta::Meta<Message::ProfileInquiryResponse>::subID2,
                                                                                              detail::MessageMeta::implementationVersion,
                                                                                              device.getMuid(),
                                                                                              inquiryMUID },
                                                                                            Message::ProfileInquiryResponse { {}, channel1Profiles }) },
                                                                      { 0, getMessageBytes ({ ChannelInGroup::wholeGroup,
                                                                                              detail::MessageMeta::Meta<Message::ProfileInquiryResponse>::subID2,
                                                                                              detail::MessageMeta::implementationVersion,
                                                                                              device.getMuid(),
                                                                                              inquiryMUID },
                                                                                            Message::ProfileInquiryResponse { {}, group0Profiles }) },
                                                                      { 1, getMessageBytes ({ ChannelInGroup::wholeGroup,
                                                                                              detail::MessageMeta::Meta<Message::ProfileInquiryResponse>::subID2,
                                                                                              detail::MessageMeta::implementationVersion,
                                                                                              device.getMuid(),
                                                                                              inquiryMUID },
                                                                                            Message::ProfileInquiryResponse { {}, group1Profiles }) },
                                                                      { 1, getMessageBytes ({ ChannelInGroup::wholeBlock,
                                                                                              detail::MessageMeta::Meta<Message::ProfileInquiryResponse>::subID2,
                                                                                              detail::MessageMeta::implementationVersion,
                                                                                              device.getMuid(),
                                                                                              inquiryMUID },
                                                                                            Message::ProfileInquiryResponse { {}, blockProfiles }) } });
            }
        }

        // Property exchange
        {
            const auto inquiryMUID = MUID::makeRandom (random);

            struct Delegate : public PropertyDelegate
            {
                uint8_t getNumSimultaneousRequestsSupported() const override { return 1; }
                PropertyReplyData propertyGetDataRequested (MUID, const PropertyRequestHeader&) override { return {}; }
                PropertyReplyHeader propertySetDataRequested (MUID, const PropertyRequestData&) override { return {}; }
                bool subscriptionStartRequested (MUID, const PropertySubscriptionHeader&) override { return true; }
                void subscriptionDidStart (MUID, const String&, const PropertySubscriptionHeader&) override {}
                void subscriptionWillEnd (MUID, const Subscription&) override {}
            };

            Delegate delegate;
            Output output;
            const auto options = DeviceOptions().withOutputs ({ &output })
                                                .withFunctionBlock (realBlock)
                                                .withDeviceInfo (deviceInfo)
                                                .withMaxSysExSize (512)
                                                .withFeatures (DeviceFeatures{}.withPropertyExchangeSupported (true))
                                                .withPropertyDelegate (&delegate);
            Device device { options };

            device.processMessage ({ 0, getMessageBytes ({ ChannelInGroup::wholeBlock,
                                                           detail::MessageMeta::Meta<Message::Discovery>::subID2,
                                                           detail::MessageMeta::implementationVersion,
                                                           inquiryMUID,
                                                           MUID::getBroadcast() },
                                                         Message::Discovery { {}, DeviceFeatures{}.withPropertyExchangeSupported (true).getSupportedCapabilities(), 512, {} }) });

            expect (output.messages.size() == 2);
            output.messages.clear();

            beginTest ("If a device receives too many concurrent property exchange requests, it responds with a retry status code.");
            {
                auto obj = std::make_unique<DynamicObject>();
                obj->setProperty ("resource", "X-CustomProp");
                const auto header = Encodings::jsonTo7BitText (obj.release());

                for (const auto& requestID : { std::byte { 0 }, std::byte { 1 } })
                {
                    device.processMessage ({ 0, getMessageBytes ({ ChannelInGroup::wholeBlock,
                                                                   detail::MessageMeta::Meta<Message::PropertySetData>::subID2,
                                                                   detail::MessageMeta::implementationVersion,
                                                                   inquiryMUID,
                                                                   device.getMuid() },
                                                                 Message::PropertySetData { { requestID, header, 0, 1, {} } }) });
                }

                expect (output.messages.size() == 1);
                const auto parsed = Parser::parse (output.messages.back().bytes);

                expect (parsed.has_value());
                expect (parsed->header == Message::Header { ChannelInGroup::wholeBlock,
                                                            detail::MessageMeta::Meta<Message::PropertySetDataResponse>::subID2,
                                                            detail::MessageMeta::implementationVersion,
                                                            device.getMuid(),
                                                            inquiryMUID });

                auto* body = std::get_if<Message::PropertySetDataResponse> (&parsed->body);
                expect (body != nullptr);
                expect (body->requestID == std::byte { 1 });
                auto replyHeader = Encodings::jsonFrom7BitText (body->header);
                expect (replyHeader.getProperty ("status", "") == var (343));
            }

            // Terminate ongoing message
            device.processMessage ({ 0, getMessageBytes ({ ChannelInGroup::wholeBlock,
                                                           detail::MessageMeta::Meta<Message::PropertySetData>::subID2,
                                                           detail::MessageMeta::implementationVersion,
                                                           inquiryMUID,
                                                           device.getMuid() },
                                                           Message::PropertySetData { { {}, {}, 0, 0, {} } }) });
            output.messages.clear();

            beginTest ("If a device receives an unexpectedly-terminated request, it responds with an error status code.");
            {
                auto obj = std::make_unique<DynamicObject>();
                obj->setProperty ("resource", "X-CustomProp");
                const auto header = Encodings::jsonTo7BitText (obj.release());
                const std::byte requestID { 3 };

                device.processMessage ({ 0, getMessageBytes ({ ChannelInGroup::wholeBlock,
                                                               detail::MessageMeta::Meta<Message::PropertySetData>::subID2,
                                                               detail::MessageMeta::implementationVersion,
                                                               inquiryMUID,
                                                               device.getMuid() },
                                                             Message::PropertySetData { { requestID, header, 2, 1, {} } }) });

                device.processMessage ({ 0, getMessageBytes ({ ChannelInGroup::wholeBlock,
                                                               detail::MessageMeta::Meta<Message::PropertySetData>::subID2,
                                                               detail::MessageMeta::implementationVersion,
                                                               inquiryMUID,
                                                               device.getMuid() },
                                                             Message::PropertySetData { { requestID, header, 2, 0, {} } }) });

                expect (output.messages.size() == 1);
                const auto parsed = Parser::parse (output.messages.back().bytes);

                expect (parsed.has_value());
                expect (parsed->header == Message::Header { ChannelInGroup::wholeBlock,
                                                            detail::MessageMeta::Meta<Message::PropertySetDataResponse>::subID2,
                                                            detail::MessageMeta::implementationVersion,
                                                            device.getMuid(),
                                                            inquiryMUID });

                auto* body = std::get_if<Message::PropertySetDataResponse> (&parsed->body);
                expect (body != nullptr);
                expect (body->requestID == requestID);
                auto replyHeader = Encodings::jsonFrom7BitText (body->header);
                expect (replyHeader.getProperty ("status", "") == var (400));
            }

            output.messages.clear();

            const auto makeStatusHeader = [] (int status)
            {
                auto ptr = std::make_unique<DynamicObject>();
                ptr->setProperty ("status", status);
                return Encodings::jsonTo7BitText (ptr.release());
            };

            const auto successHeader = makeStatusHeader (200);
            const auto retryHeader = makeStatusHeader (343);
            const auto cancelHeader = makeStatusHeader (144);

            // Common rules for PE section 10: There is no reply message associated with any Notify message.
            beginTest ("If a request is terminated via notify, the device does not respond");
            {
                auto obj = std::make_unique<DynamicObject>();
                obj->setProperty ("resource", "X-CustomProp");
                const auto header = Encodings::jsonTo7BitText (obj.release());
                const std::byte requestID { 100 };

                device.processMessage ({ 0, getMessageBytes ({ ChannelInGroup::wholeBlock,
                                                               detail::MessageMeta::Meta<Message::PropertySetData>::subID2,
                                                               detail::MessageMeta::implementationVersion,
                                                               inquiryMUID,
                                                               device.getMuid() },
                                                             Message::PropertySetData { { requestID, header, 2, 1, {} } }) });

                expect (device.getPropertyHost()->countOngoingTransactions() == 1);

                device.processMessage ({ 0, getMessageBytes ({ ChannelInGroup::wholeBlock,
                                                               detail::MessageMeta::Meta<Message::PropertyNotify>::subID2,
                                                               detail::MessageMeta::implementationVersion,
                                                               inquiryMUID,
                                                               device.getMuid() },
                                                             Message::PropertyNotify { { requestID, cancelHeader, 1, 1, {} } }) });

                expect (device.getPropertyHost()->countOngoingTransactions() == 0);

                expect (output.messages.empty());
            }

            beginTest ("Sending too many property requests simultaneously fails");
            {
                PropertyRequestHeader header;
                header.resource = "X-CustomProp";
                const auto a = device.sendPropertyGetInquiry (inquiryMUID, header, [] (const PropertyExchangeResult&) {});

                expect (a.has_value());
                expect (device.getOngoingRequests() == std::vector { *a });

                // Our device only supports 1 simultaneous request, so this should fail to send
                const auto b = device.sendPropertyGetInquiry (inquiryMUID, header, [] (const PropertyExchangeResult&) {});
                expect (! b.has_value());
                expect (device.getOngoingRequests() == std::vector { *a });

                // Reply to the first request
                device.processMessage ({ 0, getMessageBytes ({ ChannelInGroup::wholeBlock,
                                                               detail::MessageMeta::Meta<Message::PropertyGetDataResponse>::subID2,
                                                               detail::MessageMeta::implementationVersion,
                                                               inquiryMUID,
                                                               device.getMuid() },
                                                             Message::PropertyGetDataResponse { { device.getIdForRequestKey (*a)->asByte(), successHeader, 1, 1, {} } }) });

                // Now that a response to the first request has been received, there should be no
                // requests in progress.
                expect (device.getOngoingRequests().empty());
            }

            output.messages.clear();

            beginTest ("Aborting a property request sends a property notify");
            {
                PropertyRequestHeader header;
                header.resource = "X-CustomProp";

                bool callbackCalled = false;
                const auto a = device.sendPropertyGetInquiry (inquiryMUID, header, [&] (const PropertyExchangeResult&)
                {
                    callbackCalled = true;
                });

                expect (a.has_value());
                expect (device.getOngoingRequests() == std::vector { *a });
                expect (! callbackCalled);

                const auto requestID = device.getIdForRequestKey (*a);
                device.abortPropertyRequest (*a);

                expect (device.getOngoingRequests().empty());
                expect (! callbackCalled);

                expect (output.messages.size() == 2);

                const auto inquiry = Parser::parse (output.messages.front().bytes);
                expect (inquiry.has_value());
                expect (inquiry->header == Message::Header { ChannelInGroup::wholeBlock,
                                                             detail::MessageMeta::Meta<Message::PropertyGetData>::subID2,
                                                             detail::MessageMeta::implementationVersion,
                                                             device.getMuid(),
                                                             inquiryMUID });

                const auto notify = Parser::parse (output.messages.back().bytes);
                expect (notify.has_value());
                expect (notify->header == Message::Header { ChannelInGroup::wholeBlock,
                                                            detail::MessageMeta::Meta<Message::PropertyNotify>::subID2,
                                                            detail::MessageMeta::implementationVersion,
                                                            device.getMuid(),
                                                            inquiryMUID });

                auto* body = std::get_if<Message::PropertyNotify> (&notify->body);
                expect (body != nullptr);
                expect (body->requestID == requestID->asByte());
                expect (body->thisChunkNum == 1);
                expect (body->totalNumChunks == 1);

                const auto replyHeader = Encodings::jsonFrom7BitText (body->header);
                expect (replyHeader.getProperty ("status", "") == var (144));
            }

            output.messages.clear();

            beginTest ("Aborting a completed property request does nothing");
            {
                PropertyRequestHeader header;
                header.resource = "X-CustomProp";

                const auto a = device.sendPropertyGetInquiry (inquiryMUID, header, [&] (const PropertyExchangeResult&) {});

                expect (a.has_value());
                expect (device.getOngoingRequests() == std::vector { *a });

                // Reply to the get data request
                device.processMessage ({ 0, getMessageBytes ({ ChannelInGroup::wholeBlock,
                                                               detail::MessageMeta::Meta<Message::PropertyGetDataResponse>::subID2,
                                                               detail::MessageMeta::implementationVersion,
                                                               inquiryMUID,
                                                               device.getMuid() },
                                                             Message::PropertyGetDataResponse { { device.getIdForRequestKey (*a)->asByte(), successHeader, 1, 1, {} } }) });

                // After replying, there should be no ongoing requests
                expect (device.getOngoingRequests().empty());
                expect (output.messages.size() == 1);

                // This request has already finished
                device.abortPropertyRequest (*a);

                expect (device.getOngoingRequests().empty());
                expect (output.messages.size() == 1);
            }

            output.messages.clear();

            beginTest ("Beginning a subscription and ending it before the remote device replies causes a property notify to be sent");
            {
                PropertySubscriptionHeader header;
                header.command = PropertySubscriptionCommand::start;
                header.resource = "X-CustomProp";

                const auto a = device.beginSubscription (inquiryMUID, header);

                expect (device.getOngoingSubscriptions() == std::vector { a });
                // Sending a subscription request uses a request slot
                expect (device.getOngoingRequests().size() == 1);

                // subscription id is empty until the responder confirms the subscription
                expect (! device.getSubscribeIdForKey (a).has_value());
                expect (device.getResourceForKey (a) == header.resource);

                expect (output.messages.size() == 1);

                {
                    const auto parsed = Parser::parse (output.messages.back().bytes);
                    expect (parsed.has_value());
                    expect (parsed->header == Message::Header { ChannelInGroup::wholeBlock,
                                                                detail::MessageMeta::Meta<Message::PropertySubscribe>::subID2,
                                                                detail::MessageMeta::implementationVersion,
                                                                device.getMuid(),
                                                                inquiryMUID });
                    auto* body = std::get_if<Message::PropertySubscribe> (&parsed->body);
                    expect (body != nullptr);
                    const auto bodyHeader = Encodings::jsonFrom7BitText (body->header);
                    expect (bodyHeader.getProperty ("command", "") == var ("start"));
                }

                output.messages.clear();

                const auto requestID = device.getIdForRequestKey (device.getOngoingRequests().back());

                device.endSubscription (a);

                expect (output.messages.size() == 1);

                {
                    const auto parsed = Parser::parse (output.messages.back().bytes);
                    expect (parsed.has_value());
                    expect (parsed->header == Message::Header { ChannelInGroup::wholeBlock,
                                                                detail::MessageMeta::Meta<Message::PropertyNotify>::subID2,
                                                                detail::MessageMeta::implementationVersion,
                                                                device.getMuid(),
                                                                inquiryMUID });
                    auto* body = std::get_if<Message::PropertyNotify> (&parsed->body);
                    expect (body != nullptr);
                    const auto bodyHeader = Encodings::jsonFrom7BitText (body->header);
                    expect (bodyHeader.getProperty ("status", "") == var (144));
                }

                expect (device.getOngoingSubscriptions().empty());
                // The start request is no longer in progress because it was terminated by the notify
                expect (device.getOngoingRequests().empty());

                device.processMessage ({ 0, getMessageBytes ({ ChannelInGroup::wholeBlock,
                                                               detail::MessageMeta::Meta<Message::PropertySubscribeResponse>::subID2,
                                                               detail::MessageMeta::implementationVersion,
                                                               inquiryMUID,
                                                               device.getMuid() },
                                                             Message::PropertySubscribeResponse { { requestID->asByte(), successHeader, 1, 1, {} } }) });

                expect (device.getOngoingRequests().empty());

                output.messages.clear();

                // There shouldn't be any queued messages.
                device.sendPendingMessages();

                expect (output.messages.empty());
                expect (device.getOngoingRequests().empty());
            }

            output.messages.clear();

            beginTest ("Starting a new subscription while the device is waiting for a previous subscription to be confirmed queues further requests");
            {
                PropertySubscriptionHeader header;
                header.command = PropertySubscriptionCommand::start;
                header.resource = "X-CustomProp";

                const auto a = device.beginSubscription (inquiryMUID, header);
                const auto b = device.beginSubscription (inquiryMUID, header);
                const auto c = device.beginSubscription (inquiryMUID, header);

                expect (device.getOngoingSubscriptions() == std::vector { a, b, c });
                expect (device.getOngoingRequests().size() == 1);

                // subscription id is empty until the responder confirms the subscription
                expect (device.getResourceForKey (a) == header.resource);
                expect (device.getResourceForKey (b) == header.resource);
                expect (device.getResourceForKey (c) == header.resource);

                expect (output.messages.size() == 1);

                // The device has sent a subscription start for a, but not for c,
                // so it should send a notify to end subscription a, but shouldn't emit any
                // messages related to subscription c.
                device.endSubscription (a);
                device.endSubscription (c);

                expect (device.getOngoingSubscriptions() == std::vector { b });

                expect (output.messages.size() == 2);
                expect (device.getOngoingRequests().empty());

                // There should still be requests related to subscription b pending
                device.sendPendingMessages();

                expect (output.messages.size() == 3);
                expect (device.getOngoingRequests().size() == 1);

                // Now, we should send a terminate request for subscription b
                device.endSubscription (b);

                expect (device.getOngoingSubscriptions().empty());

                expect (output.messages.size() == 4);
                expect (device.getOngoingRequests().empty());
            }

            output.messages.clear();

            beginTest ("If the device receives a retry or notify in response to a subscription start request, the subscription is retried or terminated as necessary");
            {
                PropertySubscriptionHeader header;
                header.command = PropertySubscriptionCommand::start;
                header.resource = "X-CustomProp";

                const auto a = device.beginSubscription (inquiryMUID, header);

                expect (device.getOngoingSubscriptions() == std::vector { a });
                expect (device.getOngoingRequests().size() == 1);
                expect (output.messages.size() == 1);

                const auto request0 = device.getOngoingRequests().back();

                device.processMessage ({ 0, getMessageBytes ({ ChannelInGroup::wholeBlock,
                                                               detail::MessageMeta::Meta<Message::PropertySubscribeResponse>::subID2,
                                                               detail::MessageMeta::implementationVersion,
                                                               inquiryMUID,
                                                               device.getMuid() },
                                                             Message::PropertySubscribeResponse { { device.getIdForRequestKey (request0)->asByte(), retryHeader, 1, 1, {} } }) });

                // The subscription is still active from the perspective of the device, but the
                // first request is over and should be retried
                expect (device.getOngoingSubscriptions() == std::vector { a });
                expect (device.getOngoingRequests().empty());
                expect (output.messages.size() == 1);

                device.sendPendingMessages();

                expect (device.getOngoingSubscriptions() == std::vector { a });
                expect (device.getOngoingRequests().size() == 1);
                expect (output.messages.size() == 2);

                const auto request1 = device.getOngoingRequests().back();

                device.processMessage ({ 0, getMessageBytes ({ ChannelInGroup::wholeBlock,
                                                               detail::MessageMeta::Meta<Message::PropertyNotify>::subID2,
                                                               detail::MessageMeta::implementationVersion,
                                                               inquiryMUID,
                                                               device.getMuid() },
                                                             Message::PropertyNotify { { device.getIdForRequestKey (request1)->asByte(), cancelHeader, 1, 1, {} } }) });

                expect (device.getOngoingSubscriptions().empty());
                expect (device.getOngoingRequests().empty());
                expect (output.messages.size() == 2);
            }

            beginTest ("If the device receives a retry or notify in response to a subscription end request, the subscription is retried as necessary");
            {
                PropertySubscriptionHeader header;
                header.command = PropertySubscriptionCommand::start;
                header.resource = "X-CustomProp";

                const auto a = device.beginSubscription (inquiryMUID, header);

                expect (device.getOngoingSubscriptions() == std::vector { a });
                expect (device.getResourceForKey (a) == header.resource);

                const auto subscriptionResponseHeader = Encodings::jsonTo7BitText ([]
                                                                                   {
                                                                                       auto ptr = std::make_unique<DynamicObject>();
                                                                                       ptr->setProperty ("status", 200);
                                                                                       ptr->setProperty ("subscribeId", "newId");
                                                                                       return ptr.release();
                                                                                   }());

                // Accept the subscription
                device.processMessage ({ 0, getMessageBytes ({ ChannelInGroup::wholeBlock,
                                                               detail::MessageMeta::Meta<Message::PropertySubscribeResponse>::subID2,
                                                               detail::MessageMeta::implementationVersion,
                                                               inquiryMUID,
                                                               device.getMuid() },
                                                             Message::PropertySubscribeResponse { { device.getIdForRequestKey (device.getOngoingRequests().back())->asByte(), subscriptionResponseHeader, 1, 1, {} } }) });

                // The subscription is still active from the perspective of the device, but the
                // request is over and should be retried
                expect (device.getOngoingSubscriptions() == std::vector { a });
                // Now that the subscription was accepted, the subscription id should be non-empty
                expect (device.getResourceForKey (a) == header.resource);
                expect (device.getSubscribeIdForKey (a) == "newId");
                expect (device.getOngoingRequests().empty());

                device.endSubscription (a);

                expect (device.getOngoingSubscriptions().empty());
                expect (device.getOngoingRequests().size() == 1);

                // The responder is busy, can't process the subscription end
                device.processMessage ({ 0, getMessageBytes ({ ChannelInGroup::wholeBlock,
                                                               detail::MessageMeta::Meta<Message::PropertySubscribeResponse>::subID2,
                                                               detail::MessageMeta::implementationVersion,
                                                               inquiryMUID,
                                                               device.getMuid() },
                                                             Message::PropertySubscribeResponse { { device.getIdForRequestKey (device.getOngoingRequests().back())->asByte(), retryHeader, 1, 1, {} } }) });

                expect (device.getOngoingSubscriptions().empty());
                expect (device.getOngoingRequests().empty());

                device.sendPendingMessages();

                expect (device.getOngoingSubscriptions().empty());
                expect (device.getOngoingRequests().size() == 1);

                // The responder told us to immediately terminate our request to end the subscription!
                // It's unclear how this should behave, so we'll just ignore the failure and assume
                // the subscription is really over.
                device.processMessage ({ 0, getMessageBytes ({ ChannelInGroup::wholeBlock,
                                                               detail::MessageMeta::Meta<Message::PropertyNotify>::subID2,
                                                               detail::MessageMeta::implementationVersion,
                                                               inquiryMUID,
                                                               device.getMuid() },
                                                             Message::PropertyNotify { { device.getIdForRequestKey (device.getOngoingRequests().back())->asByte(), cancelHeader, 1, 1, {} } }) });

                expect (device.getOngoingSubscriptions().empty());
                expect (device.getOngoingRequests().empty());

                output.messages.clear();

                device.sendPendingMessages();

                expect (device.getOngoingSubscriptions().empty());
                expect (device.getOngoingRequests().empty());
                expect (output.messages.empty());
            }

            output.messages.clear();

            const auto startResponseHeader = [&]
            {
                auto ptr = std::make_unique<DynamicObject>();
                ptr->setProperty ("status", 200);
                ptr->setProperty ("subscribeId", "newId");
                return Encodings::jsonTo7BitText (ptr.release());
            }();

            beginTest ("The responder can terminate a subscription");
            {
                PropertySubscriptionHeader header;
                header.command = PropertySubscriptionCommand::start;
                header.resource = "X-CustomProp";

                const auto a = device.beginSubscription (inquiryMUID, header);

                expect (device.getOngoingRequests().size() == 1);
                expect (device.getOngoingSubscriptions().size() == 1);
                expect (device.getResourceForKey (a) == "X-CustomProp");

                device.processMessage ({ 0, getMessageBytes ({ ChannelInGroup::wholeBlock,
                                                               detail::MessageMeta::Meta<Message::PropertySubscribeResponse>::subID2,
                                                               detail::MessageMeta::implementationVersion,
                                                               inquiryMUID,
                                                               device.getMuid() },
                                                             Message::PropertySubscribeResponse { { device.getIdForRequestKey (device.getOngoingRequests().back())->asByte(),
                                                                                                    startResponseHeader,
                                                                                                    1,
                                                                                                    1,
                                                                                                    {} } }) });

                expect (device.getOngoingRequests().empty());
                expect (device.getOngoingSubscriptions().size() == 1);
                expect (output.messages.size() == 1);

                output.messages.clear();

                expect (device.getResourceForKey (a) == "X-CustomProp");
                expect (device.getSubscribeIdForKey (a) == "newId");

                const auto endRequestHeader = [&]
                {
                    auto ptr = std::make_unique<DynamicObject>();
                    ptr->setProperty ("command", "end");
                    ptr->setProperty ("subscribeId", "newId");
                    return Encodings::jsonTo7BitText (ptr.release());
                }();

                device.processMessage ({ 0, getMessageBytes ({ ChannelInGroup::wholeBlock,
                                                               detail::MessageMeta::Meta<Message::PropertySubscribe>::subID2,
                                                               detail::MessageMeta::implementationVersion,
                                                               inquiryMUID,
                                                               device.getMuid() },
                                                             Message::PropertySubscribe { { std::byte { 0x42 }, endRequestHeader, 1, 1, {} } }) });

                expect (device.getOngoingRequests().empty());
                expect (device.getOngoingSubscriptions().empty());
                expect (output.messages.size() == 1);

                {
                    const auto parsed = Parser::parse (output.messages.back().bytes);
                    expect (parsed.has_value());
                    expect (parsed->header == Message::Header { ChannelInGroup::wholeBlock,
                                                                detail::MessageMeta::Meta<Message::PropertySubscribeResponse>::subID2,
                                                                detail::MessageMeta::implementationVersion,
                                                                device.getMuid(),
                                                                inquiryMUID });
                    auto* body = std::get_if<Message::PropertySubscribeResponse> (&parsed->body);
                    expect (body != nullptr);
                    const auto bodyHeader = Encodings::jsonFrom7BitText (body->header);
                    expect (bodyHeader.getProperty ("status", "") == var (200));
                }
            }

            beginTest ("Invalidating a MUID clears subscriptions to that MUID");
            {
                PropertySubscriptionHeader header;
                header.command = PropertySubscriptionCommand::start;
                header.resource = "X-CustomProp";

                const auto a = device.beginSubscription (inquiryMUID, header);

                device.processMessage ({ 0, getMessageBytes ({ ChannelInGroup::wholeBlock,
                                                               detail::MessageMeta::Meta<Message::PropertySubscribeResponse>::subID2,
                                                               detail::MessageMeta::implementationVersion,
                                                               inquiryMUID,
                                                               device.getMuid() },
                                                             Message::PropertySubscribeResponse { { device.getIdForRequestKey (device.getOngoingRequests().back())->asByte(),
                                                                                                    startResponseHeader,
                                                                                                    1,
                                                                                                    1,
                                                                                                    {} } }) });

                expect (device.getOngoingSubscriptions() == std::vector { a });

                device.processMessage ({ 0, getMessageBytes ({ ChannelInGroup::wholeBlock,
                                                               detail::MessageMeta::Meta<Message::InvalidateMUID>::subID2,
                                                               detail::MessageMeta::implementationVersion,
                                                               inquiryMUID,
                                                               MUID::getBroadcast() },
                                                             Message::InvalidateMUID { inquiryMUID }) });

                expect (device.getOngoingSubscriptions().empty());
            }

            beginTest ("Disconnecting and then connecting with the same MUID doesn't reuse SubscribeKeys");
            {
                expect (device.getDiscoveredMuids().empty());

                device.processMessage ({ 0, getMessageBytes ({ ChannelInGroup::wholeBlock,
                                                               detail::MessageMeta::Meta<Message::Discovery>::subID2,
                                                               detail::MessageMeta::implementationVersion,
                                                               inquiryMUID,
                                                               MUID::getBroadcast() },
                                                             Message::Discovery { {}, DeviceFeatures{}.withPropertyExchangeSupported (true).getSupportedCapabilities(), 512, {} }) });

                expect (device.getDiscoveredMuids().size() == 1);

                PropertySubscriptionHeader header;
                header.command = PropertySubscriptionCommand::start;
                header.resource = "X-CustomProp";

                const auto subscription = device.beginSubscription (inquiryMUID, header);

                device.processMessage ({ 0, getMessageBytes ({ ChannelInGroup::wholeBlock,
                                                               detail::MessageMeta::Meta<Message::PropertySubscribeResponse>::subID2,
                                                               detail::MessageMeta::implementationVersion,
                                                               inquiryMUID,
                                                               device.getMuid() },
                                                             Message::PropertySubscribeResponse { { device.getIdForRequestKey (device.getOngoingRequests().back())->asByte(),
                                                                                                    startResponseHeader,
                                                                                                    1,
                                                                                                    1,
                                                                                                    {} } }) });

                expect (device.getSubscribeIdForKey (subscription) == "newId");
                expect (device.getResourceForKey (subscription) == "X-CustomProp");

                device.processMessage ({ 0, getMessageBytes ({ ChannelInGroup::wholeBlock,
                                                               detail::MessageMeta::Meta<Message::InvalidateMUID>::subID2,
                                                               detail::MessageMeta::implementationVersion,
                                                               inquiryMUID,
                                                               MUID::getBroadcast() },
                                                             Message::InvalidateMUID { inquiryMUID }) });

                expect (device.getDiscoveredMuids().empty());

                device.processMessage ({ 0, getMessageBytes ({ ChannelInGroup::wholeBlock,
                                                               detail::MessageMeta::Meta<Message::Discovery>::subID2,
                                                               detail::MessageMeta::implementationVersion,
                                                               inquiryMUID,
                                                               MUID::getBroadcast() },
                                                             Message::Discovery { {}, DeviceFeatures{}.withPropertyExchangeSupported (true).getSupportedCapabilities(), 512, {} }) });

                expect (device.getDiscoveredMuids().size() == 1);

                const auto newSubscription = device.beginSubscription (inquiryMUID, header);

                expect (subscription != newSubscription);
                expect (device.getOngoingSubscriptions() == std::vector { newSubscription });
            }
        }
    }

private:
    template <typename Msg>
    static std::vector<std::byte> getMessageBytes (const Message::Header& header, const Msg& body)
    {
        std::vector<std::byte> bytes;
        detail::Marshalling::Writer { bytes } (header, body);
        return bytes;
    }
};

static DeviceTests deviceTests;

#endif

} // namespace juce::midi_ci

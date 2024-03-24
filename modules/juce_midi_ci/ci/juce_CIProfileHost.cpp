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

class ProfileHost::Visitor : public detail::MessageTypeUtils::MessageVisitor
{
public:
    Visitor (ProfileHost* h, ResponderOutput* o, bool* b)
        : host (h), output (o), handled (b) {}

    void visit (const Message::ProfileInquiry& body) const override { visitImpl (body); }
    void visit (const Message::ProfileDetails& body) const override { visitImpl (body); }
    void visit (const Message::ProfileOn& body)      const override { visitImpl (body); }
    void visit (const Message::ProfileOff& body)     const override { visitImpl (body); }
    using MessageVisitor::visit;

    static auto getNumChannels (Message::Header header, Message::ProfileOn p)
    {
        return (uint8_t) header.version >= 2 ? p.numChannels : 1;
    }

    static auto getNumChannels (Message::Header, Message::ProfileOff)  { return 0; }

private:
    template <typename Body>
    void visitImpl (const Body& body) const { *handled = messageReceived (body); }

    bool messageReceived (const Message::ProfileInquiry&) const
    {
        host->isResponder = true;

        if ((uint8_t) output->getIncomingHeader().deviceID < 16
            || output->getIncomingHeader().deviceID == ChannelInGroup::wholeGroup)
        {
            if (const auto* state = host->getProfileStates().groupStates[output->getIncomingGroup()].getStateForDestination (output->getIncomingHeader().deviceID))
            {
                const auto active   = state->getActive();
                const auto inactive = state->getInactive();
                detail::MessageTypeUtils::send (*output, Message::ProfileInquiryResponse { active, inactive });
            }
        }
        else if (output->getIncomingHeader().deviceID == ChannelInGroup::wholeBlock)
        {
            auto header = output->getReplyHeader (detail::MessageMeta::Meta<Message::ProfileInquiryResponse>::subID2);

            const auto sendIfNonEmpty = [&] (const auto group, const auto& state)
            {
                if (! state.empty())
                {
                    const auto active   = state.getActive();
                    const auto inactive = state.getInactive();
                    detail::MessageTypeUtils::send (*output, (uint8_t) group, header, Message::ProfileInquiryResponse { active, inactive });
                }
            };

            for (auto groupNum = 0; groupNum < host->functionBlock.numGroups; ++groupNum)
            {
                const auto group = host->functionBlock.firstGroup + groupNum;
                const auto& groupState = host->getProfileStates().groupStates[(size_t) group];

                for (size_t channel = 0; channel < groupState.channelStates.size(); ++channel)
                {
                    header.deviceID = ChannelInGroup (channel);
                    sendIfNonEmpty (group, groupState.channelStates[channel]);
                }
            }

            header.deviceID = ChannelInGroup::wholeGroup;

            for (auto i = 0; i < host->functionBlock.numGroups; ++i)
            {
                const auto group = host->functionBlock.firstGroup + i;
                const auto& groupState = host->getProfileStates().groupStates[(size_t) group];
                sendIfNonEmpty (group, groupState.groupState);
            }

            // Always send the block response to indicate that no further replies will follow
            header.deviceID = ChannelInGroup::wholeBlock;
            const auto state = host->getProfileStates().blockState;
            const auto active   = state.getActive();
            const auto inactive = state.getInactive();
            detail::MessageTypeUtils::send (*output, output->getIncomingGroup(), header, Message::ProfileInquiryResponse { active, inactive });
        }

        return true;
    }

    bool messageReceived (const Message::ProfileDetails& body) const
    {
        if (body.target == std::byte{})
        {
            const auto address = ChannelAddress{}.withGroup (output->getIncomingGroup())
                                                 .withChannel (output->getIncomingHeader().deviceID);
            const ProfileAtAddress profileAtAddress { body.profile, address };
            const auto state = host->getState (profileAtAddress);
            std::vector<std::byte> extraData;
            detail::Marshalling::Writer { extraData } (state.active, state.supported);
            detail::MessageTypeUtils::send (*output, Message::ProfileDetailsResponse { body.profile, body.target, extraData });
        }
        else
        {
            detail::MessageTypeUtils::sendNAK (*output, std::byte { 0x04 });
        }

        return true;
    }

    template <typename Body>
    bool profileEnablementReceived (const Body& request) const
    {
        const auto destination = ChannelAddress{}.withGroup (output->getIncomingGroup())
                                                 .withChannel (output->getIncomingHeader().deviceID);
        if (auto* state = host->states.getStateForDestination (destination))
        {
            const auto previousState = state->get (request.profile);

            if (previousState.isSupported())
            {
                const ProfileAtAddress profileAtAddress { request.profile, destination };

                {
                    const ScopedValueSetter scope { host->currentEnablementMessage,
                                                    std::optional<ProfileAtAddress> (profileAtAddress) };
                    host->delegate.profileEnablementRequested (output->getIncomingHeader().source,
                                                               profileAtAddress,
                                                               getNumChannels (output->getIncomingHeader(), request),
                                                               std::is_same_v<Message::ProfileOn, Body>);
                }

                const auto currentState = host->getState (profileAtAddress);

                const auto sendResponse = [&] (auto response)
                {
                    const Message::Header header
                    {
                        profileAtAddress.address.getChannel(),
                        detail::MessageMeta::Meta<decltype (response)>::subID2,
                        detail::MessageMeta::implementationVersion,
                        output->getMuid(),
                        MUID::getBroadcast(),
                    };

                    detail::MessageTypeUtils::send (*output, profileAtAddress.address.getGroup(), header, response);
                };

                const auto numIndividualChannels = (std::is_same_v<Message::ProfileOn, Body> ? currentState : previousState).active;

                const auto numChannelsToSend = destination.isSingleChannel()
                                             ? numIndividualChannels
                                             : uint16_t{};

                if (currentState.isActive())
                    sendResponse (Message::ProfileEnabledReport { profileAtAddress.profile, numChannelsToSend });
                else
                    sendResponse (Message::ProfileDisabledReport { profileAtAddress.profile, numChannelsToSend });

                host->isResponder = true;
                return true;
            }
        }

        detail::MessageTypeUtils::sendNAK (*output, {});
        return true;
    }

    bool messageReceived (const Message::ProfileOn& request) const
    {
        return profileEnablementReceived (request);
    }

    bool messageReceived (const Message::ProfileOff& request) const
    {
        return profileEnablementReceived (request);
    }

    ProfileHost* host = nullptr;
    ResponderOutput* output = nullptr;
    bool* handled = nullptr;
};

void ProfileHost::setProfileEnablement (ProfileAtAddress profileAtAddress, int numChannels)
{
    if (numChannels > 0)
        enableProfileImpl (profileAtAddress, numChannels);
    else
        disableProfileImpl (profileAtAddress);
}

void ProfileHost::addProfile (ProfileAtAddress profileAtAddress, int maxNumChannels)
{
    auto* state = states.getStateForDestination (profileAtAddress.address);

    if (state == nullptr || state->get (profileAtAddress.profile).isSupported())
        return;

    // There are only 256 channels on a UMP endpoint, so requesting more probably doesn't make sense!
    jassert (maxNumChannels <= 256);

    state->set (profileAtAddress.profile, { (uint16_t) jmax (1, maxNumChannels), 0 });

    if (! isResponder || profileAtAddress == currentEnablementMessage)
        return;

    const Message::Header header
    {
        profileAtAddress.address.getChannel(),
        detail::MessageMeta::Meta<Message::ProfileAdded>::subID2,
        detail::MessageMeta::implementationVersion,
        output.getMuid(),
        MUID::getBroadcast(),
    };

    detail::MessageTypeUtils::send (output,
                                    profileAtAddress.address.getGroup(),
                                    header,
                                    Message::ProfileAdded { profileAtAddress.profile });
}

void ProfileHost::removeProfile (ProfileAtAddress profileAtAddress)
{
    auto* state = states.getStateForDestination (profileAtAddress.address);

    if (state == nullptr)
        return;

    setProfileEnablement (profileAtAddress, 0);

    if (! state->get (profileAtAddress.profile).isSupported())
        return;

    state->erase (profileAtAddress.profile);

    if (! isResponder || profileAtAddress == currentEnablementMessage)
        return;

    const Message::Header header
    {
        profileAtAddress.address.getChannel(),
        detail::MessageMeta::Meta<Message::ProfileRemoved>::subID2,
        detail::MessageMeta::implementationVersion,
        output.getMuid(),
        MUID::getBroadcast(),
    };

    detail::MessageTypeUtils::send (output,
                                    profileAtAddress.address.getGroup(),
                                    header,
                                    Message::ProfileRemoved { profileAtAddress.profile });
}

void ProfileHost::enableProfileImpl (ProfileAtAddress profileAtAddress, int numChannels)
{
    auto* state = states.getStateForDestination (profileAtAddress.address);

    if (state == nullptr)
        return;

    const auto old = state->get (profileAtAddress.profile);

    if (! old.isSupported())
        return;

    // There are only 256 channels on a UMP endpoint, so requesting more probably doesn't make sense!
    jassert (numChannels <= 256);

    const auto enabledChannels = jmax ((uint16_t) 1, jmin (old.supported, (uint16_t) numChannels));
    state->set (profileAtAddress.profile, { old.supported, enabledChannels });

    if (! isResponder || profileAtAddress == currentEnablementMessage)
        return;

    const Message::Header header
    {
        profileAtAddress.address.getChannel(),
        detail::MessageMeta::Meta<Message::ProfileEnabledReport>::subID2,
        detail::MessageMeta::implementationVersion,
        output.getMuid(),
        MUID::getBroadcast(),
    };

    const auto numChannelsToSend = profileAtAddress.address.isSingleChannel() ? enabledChannels : uint16_t{};

    detail::MessageTypeUtils::send (output,
                                    profileAtAddress.address.getGroup(),
                                    header,
                                    Message::ProfileEnabledReport { profileAtAddress.profile, numChannelsToSend });
}

void ProfileHost::disableProfileImpl (ProfileAtAddress profileAtAddress)
{
    auto* state = states.getStateForDestination (profileAtAddress.address);

    if (state == nullptr)
        return;

    const auto old = state->get (profileAtAddress.profile);

    if (! old.isActive())
        return;

    state->set (profileAtAddress.profile, { old.supported, 0 });

    if (! isResponder || profileAtAddress == currentEnablementMessage)
        return;

    const Message::Header header
    {
        profileAtAddress.address.getChannel(),
        detail::MessageMeta::Meta<Message::ProfileDisabledReport>::subID2,
        detail::MessageMeta::implementationVersion,
        output.getMuid(),
        MUID::getBroadcast(),
    };

    const auto numChannelsToSend = profileAtAddress.address.isSingleChannel() ? old.active : uint16_t{};

    detail::MessageTypeUtils::send (output,
                                    profileAtAddress.address.getGroup(),
                                    header,
                                    Message::ProfileDisabledReport { profileAtAddress.profile, numChannelsToSend });
}

bool ProfileHost::tryRespond (ResponderOutput& responderOutput, const Message::Parsed& message)
{
    bool result = false;
    detail::MessageTypeUtils::visit (message, Visitor { this, &responderOutput, &result });
    return result;
}

} // namespace juce::midi_ci

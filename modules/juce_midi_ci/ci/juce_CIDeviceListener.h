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

/**
    Contains information relating to a subscription update. Check the header's
    subscription kind to find out whether the payload is a full update, a
    partial update, or empty (as is the case for a notification or
    subscription-end request).

    @tags{Audio}
*/
struct PropertySubscriptionData
{
    PropertySubscriptionHeader header;
    Span<const std::byte> body;
};

//==============================================================================
/**
    An interface that receives callbacks when certain messages are received by a Device.

    @tags{Audio}
*/
struct DeviceListener
{
    DeviceListener() = default;
    virtual ~DeviceListener() = default;
    DeviceListener (const DeviceListener&) = default;
    DeviceListener (DeviceListener&&) = default;
    DeviceListener& operator= (const DeviceListener&) = default;
    DeviceListener& operator= (DeviceListener&&) = default;

    //==============================================================================
    /** Called to indicate that a device with the provided MUID was discovered.
        To find out more about the device, use Device::getDiscoveryInfoForMuid().
    */
    virtual void deviceAdded   ([[maybe_unused]] MUID x) {}

    /** Called to indicate that a device's MUID was invalidated.
        If you were previously storing your own information about this device, you should forget
        that information here.
    */
    virtual void deviceRemoved ([[maybe_unused]] MUID x) {}

    /** Called to indicate that endpoint information was received for the given device.
        See the MIDI-CI spec for an explanation of the different status codes.
    */
    virtual void endpointReceived ([[maybe_unused]] MUID x,
                                   [[maybe_unused]] Message::EndpointInquiryResponse response) {}


    /** Called to indicate that a NAK message was received.
        This is useful e.g. to display a diagnostic to the user, or to cache the failed request
        details and retry the request at a later date.

        The message field of the NAK is 7-bit text. You can convert it to a string using
        Encodings::stringFrom7BitText().
    */
    virtual void messageNotAcknowledged ([[maybe_unused]] MUID x,
                                         [[maybe_unused]] Message::NAK) {}

    //==============================================================================
    /** Called to indicate that another device reported its enabled and disabled profiles on a
        particular channel.

        @see Device::getProfileStateForMuid()
    */
    virtual void profileStateReceived ([[maybe_unused]] MUID x,
                                       [[maybe_unused]] ChannelInGroup destination) {}

    /** Called to indicate that a profile was added or removed. */
    virtual void profilePresenceChanged ([[maybe_unused]] MUID x,
                                         [[maybe_unused]] ChannelInGroup destination,
                                         [[maybe_unused]] Profile profile,
                                         [[maybe_unused]] bool exists) {}

    /** Called to indicate that a profile was enabled or disabled.
        A channel count of 0 indicates that the profile was disabled.
    */
    virtual void profileEnablementChanged ([[maybe_unused]] MUID x,
                                           [[maybe_unused]] ChannelInGroup destination,
                                           [[maybe_unused]] Profile profile,
                                           [[maybe_unused]] int numChannels) {}

    /** Called to indicate that details about a profile were received. */
    virtual void profileDetailsReceived ([[maybe_unused]] MUID x,
                                         [[maybe_unused]] ChannelInGroup destination,
                                         [[maybe_unused]] Profile profile,
                                         [[maybe_unused]] std::byte target,
                                         [[maybe_unused]] Span<const std::byte> data) {}

    /** Called to indicate that data for a profile were received.

        Note that this function may be called either when a remote device attempts to send data to
        one of the local Device's profiles, or when a profile on a remote device produces some data.

        Each profile will specify its own mechanism for distinguishing between the two cases if
        necessary.
    */
    virtual void profileSpecificDataReceived ([[maybe_unused]] MUID x,
                                              [[maybe_unused]] ChannelInGroup destination,
                                              [[maybe_unused]] Profile profile,
                                              [[maybe_unused]] Span<const std::byte> data) {}

    //==============================================================================
    /** Called to indicate that another device reported its property exchange capabilities.

        @see Device::getPropertyExchangeCapabilitiesResponseForMuid()
    */
    virtual void propertyExchangeCapabilitiesReceived ([[maybe_unused]] MUID x) {}

    /** Called to indicate that a subscription update was received.
        This only receives messages with responder commands (partial, full, notify, end).

        To start a subscription, use Device::sendPropertySubscriptionStart().
    */
    virtual void propertySubscriptionDataReceived ([[maybe_unused]] MUID x,
                                                   [[maybe_unused]] const PropertySubscriptionData& data) {}

    /** Called when a remote device updates a subscription by accepting or terminating it.

        If the subscription was accepted, the subscribeId will be non-null. Otherwise, a null
        subscribeId indicates that the subscription was terminated.
    */
    virtual void propertySubscriptionChanged ([[maybe_unused]] SubscriptionKey subscription,
                                              [[maybe_unused]] const std::optional<String>& subscribeId) {}
};

} // namespace juce::midi_ci

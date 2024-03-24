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

//==============================================================================
/**
    Instances of this type are responsible for parsing and interpreting incoming
    MIDI-CI messages, and for sending MIDI-CI messages to other devices.

    Each Device can act both as a target for messages, and as a source of
    messages intended to inspect/configure other devices.

    The member functions of Device are generally used to inspect other
    devices. Member functions starting with 'send' are used to send or request
    information from other devices; registered DeviceListeners will be notified
    when the Device receives a response, and then member functions named
    matching 'get.*ForMuid' can be used to retrieve the result of the inquiry.

    If the Device does not have local profiles or properties, then responses
    to all incoming messages will be generated automatically using the
    information supplied during construction.

    If the Device has profiles or properties, then you should implement a
    ProfileDelegate and/or a PropertyDelegate as appropriate, and pass this
    delegate during construction. Each Delegate will receive callbacks when a
    remote device makes a request of the local device, such as
    enabling/disabling a profile, or setting/getting property data.

    Sometimes the local device must send notifications when
    updating its profile or property state, for example when profiles are
    added, or when a subscribed property is changed. Methods to send these
    notifications are found on the ProfileHost and PropertyHost classes.

    @tags{Audio}
*/
class Device : public DeviceMessageHandler
{
public:
    using Features          = DeviceFeatures;
    using Listener          = DeviceListener;
    using Options           = DeviceOptions;

    /** Constructs a device using the provided options. */
    explicit Device (const Options& opt);

    Device (Device&&) noexcept;
    Device& operator= (Device&&) noexcept;

    JUCE_DECLARE_NON_COPYABLE (Device)

    /** Destructor, sends a message to invalidate this device's MUID. */
    ~Device() override;

    //==============================================================================
    /** To be called with any message that should be processed by the device.
        This should only be passed complete CI messages - you might find the Extractor
        class useful for parsing a stream of Universal MIDI Packets and extracting the
        CI messages.
        Note that this function does *not* synchronise with any other member function of this
        class. This means that you must not call this directly from the MIDI input thread if there's
        any chance of other member functions being called on the same instance simultaneously from
        other threads.
        It's probably easiest to send all messages onto the main thread and to limit interactions
        with the Device to that thread.
    */
    void processMessage (ump::BytesOnGroup) override;

    //==============================================================================
    /** Sends an inquiry message.

        You can use DeviceListener::deviceAdded to be notified when new devices are discovered.

        This will clear the internal cache of discovered devices, and repopulate it as discovery
        response messages are received.
    */
    void sendDiscovery();

    /** Sends an endpoint inquiry message.

        Check the MIDI-CI spec for an explanation of the different endpoint message status codes.

        Received responses will be sent to DeviceListener::endpointReceived. Responses are not
        cached by the Device; if you need to cache endpoint responses, you can keep your own
        map of MUID->response, update it in endpointReceived, and remove entries in
        DeviceListener::deviceRemoved.
    */
    void sendEndpointInquiry (MUID destination, Message::EndpointInquiry endpoint);

    //==============================================================================
    /** Sends a profile inquiry to a particular device.

        DeviceListener::profileStateReceived will be called when the device replies.
    */
    void sendProfileInquiry (MUID muid, ChannelInGroup address);

    /** Sends a profile details inquiry to a particular device.

        DeviceListener::profileDetailsReceived will be called when the device replies.
    */
    void sendProfileDetailsInquiry (MUID muid, ChannelInGroup address, Profile profile, std::byte target);

    /** Sends profile data to a particular device. */
    void sendProfileSpecificData (MUID muid, ChannelInGroup address, Profile profile, Span<const std::byte>);

    /** Sets a profile on or off. Pass 0 or less to disable the profile, or a positive number to enable it.

        This also goes for group/block profiles. If the request is addressed to a group/block, then
        a positive number will cause a "profile on" message to be sent, and a non-positive number
        will cause a "profile off" message to be sent. The channel count of the sent message will
        always be zero for messages addressed to groups/blocks.regardless of the value of the
        numChannels argument.
    */
    void sendProfileEnablement (MUID muid, ChannelInGroup address, Profile profile, int numChannels);

    //==============================================================================
    /** Sends a property inquiry to a particular device.
        If the device supports properties, this will also automatically request the ResourceList
        property, and then the ChannelList and DeviceInfo properties if they are present in the
        ResourceList.
    */
    void sendPropertyCapabilitiesInquiry (MUID destination);

    /** Initiates an inquiry to fetch a property from a particular device.

        @param m         the MUID of the device to query
        @param header    specifies the resource to query, along with format/encoding options
        @param onResult  called when the transaction completes; not called if the transaction fails to start
        @returns         a key uniquely identifying this request, if the transaction begins successfully, or nullopt otherwise
    */
    std::optional<RequestKey> sendPropertyGetInquiry (MUID m,
                                                      const PropertyRequestHeader& header,
                                                      std::function<void (const PropertyExchangeResult&)> onResult);

    /** Initiates an inquiry to set a property on a particular device.

        @param m         the MUID of the device to query
        @param header    specifies the resource to query, along with format/encoding options
        @param body      the unencoded body content of the message
        @param onResult  called when the transaction completes; not called if the transaction fails to start
        @returns         a key uniquely identifying this request, if the transaction begins successfully, or nullopt otherwise
    */
    std::optional<RequestKey> sendPropertySetInquiry (MUID m,
                                                      const PropertyRequestHeader& header,
                                                      Span<const std::byte> body,
                                                      std::function<void (const PropertyExchangeResult&)> onResult);

    /** Cancels a request started with sendPropertyGetInquiry() or sendPropertySetInquiry().

        This sends a property notify message indicating that the responder no longer needs to
        process the initial request.
    */
    void abortPropertyRequest (RequestKey);

    /** Returns the request id corresponding to a particular request.

        If the request could not be found (it never started, or already finished), then this
        returns nullopt.
    */
    std::optional<RequestID> getIdForRequestKey (RequestKey) const;

    /** Returns all the ongoing requests. */
    std::vector<RequestKey> getOngoingRequests() const;

    /** Attempts to begin a subscription with the provided attributes.

        Once the subscription is no longer required, cancel it by passing the SubscriptionKey to endSubscription().
    */
    SubscriptionKey beginSubscription (MUID m, const PropertySubscriptionHeader& header);

    /** Ends a previously-started subscription. */
    void endSubscription (SubscriptionKey);

    /** Returns all the subscriptions that have been initiated by this device. */
    std::vector<SubscriptionKey> getOngoingSubscriptions() const;

    /** If the provided subscription has started successfully, this returns the subscribeId assigned
        to the subscription by the remote device.
    */
    std::optional<String> getSubscribeIdForKey (SubscriptionKey key) const;

    /** If the provided subscription has not been cancelled, this returns the name of the
        subscribed resource.
    */
    std::optional<String> getResourceForKey (SubscriptionKey key) const;

    /** Sends any cached messages that need retrying.

        @returns true if there are no more messages to send, or false otherwise
    */
    bool sendPendingMessages();

    //==============================================================================
    /** Adds a listener that will be notified when particular events occur.

        Check the members of the Listener class to see the kinds of events that are reported.
        To receive notifications through Listener::propertySubscriptionReceived(), you must
        first request a subscription using sendPropertySubscriptionStart().

        @see Listener, removeListener()
    */
    void addListener (Listener& l);

    /** Removes a listener that was previously added with addListener(). */
    void removeListener (Listener& l);

    //==============================================================================
    /** Returns the MUID currently associated with this device.

        This may change, e.g. if another device reports that it shares the same MUID.
    */
    MUID getMuid() const;

    /** Returns the configuration of this device. */
    Options getOptions() const;

    /** Returns a list of all MUIDs that have been discovered by this device. */
    std::vector<MUID> getDiscoveredMuids() const;

    /** If you set withProfileConfigurationSupported when constructing this device, this will return
        a pointer to an object that can be used to query the states of the profiles for this device.
    */
    const ProfileHost*  getProfileHost()  const;

    /** If you set withProfileConfigurationSupported when constructing this device, this will return
        a pointer to an object that can be used to modify the states of the profiles for this device.
    */
          ProfileHost*  getProfileHost();

    /** If you set withPropertyExchangeSupported when constructing this device, this will return
        a pointer to an object that can be used to query the states of the properties for this device.
    */
    const PropertyHost* getPropertyHost() const;

    /** If you set withPropertyExchangeSupported when constructing this device, this will return
        a pointer to an object that can be used to modify the states of the properties for this device.
    */
          PropertyHost* getPropertyHost();

    //==============================================================================
    /** Returns basic attributes about another device that was discovered.

        If there's no record of the provided device, this will return nullopt.
    */
    std::optional<Message::Discovery> getDiscoveryInfoForMuid (MUID m) const;

    /** Returns the states of the profiles on a particular channel of a device.

        If the state is unknown, returns nullptr.

        Devices don't report profile capabilities unless asked; you can request capabilities
        using inquireProfile().
    */
    const ChannelProfileStates* getProfileStateForMuid (MUID m, ChannelAddress address) const;

    /** Returns the number of simultaneous property exchange requests supported by a particular
        device.

        If there's no record of this device's property capabilities (including the case where
        the device doesn't support property exchange at all) this will return nullopt.

        Devices don't report property capabilities unless asked; you can request capabilities
        using inquirePropertyCapabilities().
    */
    std::optional<int> getNumPropertyExchangeRequestsSupportedForMuid (MUID m) const;

    /** After DeviceListener::propertyExchangeCapabilitiesReceived() has been received for a
        particular device, this function will return that device's ResourceList if available, or
        a null var otherwise.
    */
    var getResourceListForMuid (MUID x) const;

    /** After DeviceListener::propertyExchangeCapabilitiesReceived() has been received for a
        particular device, this function will return that device's DeviceInfo if available, or
        a null var otherwise.
    */
    var getDeviceInfoForMuid (MUID x) const;

    /** After DeviceListener::propertyExchangeCapabilitiesReceived() has been received for a
        particular device, this function will return that device's ChannelList if available, or
        a null var otherwise.
    */
    var getChannelListForMuid (MUID x) const;

private:
    class Impl;
    std::unique_ptr<Impl> pimpl;
};

} // namespace juce::midi_ci

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
    */
    void sendProfileEnablement (MUID muid, ChannelInGroup address, Profile profile, int numChannels);

    //==============================================================================
    /** Sends a property inquiry to a particular device.
        If the device supports properties, this will also automatically request the ResourceList
        property, and then the ChannelList and DeviceInfo properties if they are present in the
        ResourceList.
    */
    void sendPropertyCapabilitiesInquiry (MUID destination);

    /** Sends an inquiry to get a property value from another device, invoking a callback once
        the full transaction has completed.

        @param destination      the device whose property will be set
        @param header           information about the property data that will be sent
        @param onResult         this will be called once the result of the transaction is known.
                                If the transaction cannot start for some reason (e.g. the request is
                                malformed, or there are too many simultaneous requests) then the
                                function will be called immediately. Otherwise, the function will be
                                called once the destination device has confirmed receipt of the
                                inquiry.
        @return                 a token bounding the lifetime of the request.
                                If you need to terminate the transaction before it has completed,
                                you can call reset() on this token, or cause its destructor to run.
    */
    ErasedScopeGuard sendPropertyGetInquiry (MUID destination,
                                             const PropertyRequestHeader& header,
                                             std::function<void (const PropertyExchangeResult&)> onResult);

    /** Sends an inquiry to set a property value on another device, invoking a callback once
        the full transaction has completed.

        @param destination      the device whose property will be set
        @param header           information about the property data that will be sent
        @param body             the property data payload to send.
                                If the header specifies 'ascii' encoding, then you are responsible
                                for ensuring that no byte of the payload data has its most
                                significant bit set. Sending the message will fail if this is not
                                the case. Otherwise, if another encoding is specified then the
                                payload data may contain any byte values. You should not attempt to
                                encode the data yourself; the payload will be automatically encoded
                                before being sent.
        @param onResult         this will be called once the result of the transaction is known.
                                If the transaction cannot start for some reason (e.g. the
                                destination does not support property exchange, the request is
                                malformed, or there are too many simultaneous requests) then the
                                function will be called immediately. Otherwise, the function will be
                                called once the destination device has confirmed receipt of the
                                inquiry.
    */
    void sendPropertySetInquiry (MUID destination,
                                 const PropertyRequestHeader& header,
                                 Span<const std::byte> body,
                                 std::function<void (const PropertyExchangeResult&)> onResult);

    /** Sends an inquiry to start a subscription to a property on a device.
        The provided callback will be called to indicate whether starting the subscription
        succeeded or failed.
        When the remote device indicates that its property value has changed,
        DeviceListener::propertySubscriptionReceived will be called with information about the
        update.
    */
    void sendPropertySubscriptionStart (MUID,
                                        const PropertySubscriptionHeader& header,
                                        std::function<void (const PropertyExchangeResult&)>);

    /** Sends an inquiry to end a subscription to a property on a device.
        The provided callback will be called to indicate whether the subscriber acknowledged
        receipt of the message.
        Note that the remote device may also choose to terminate the subscription of its own
        accord - in this case, the end request will be sent to
        DeviceListener::propertySubscriptionReceived.
    */
    void sendPropertySubscriptionEnd (MUID,
                                      const String& subscribeId,
                                      std::function<void (const PropertyExchangeResult&)>);

    /** Returns all of the subscriptions that we have requested from another device.

        Does *not* include subscriptions that other devices have requested from us.
    */
    std::vector<Subscription> getOngoingSubscriptionsForMuid (MUID m) const;

    /** Returns the number of transactions initiated by us that are yet to receive complete replies.

        Does *not* include the count of unfinished requests addressed to us by other devices.

        @see PropertyHost::countOngoingTransactions()
    */
    int countOngoingPropertyTransactions() const;

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

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

namespace juce
{

// Annoyingly, the macOS 15.2 SDK still sets MAC_OS_X_VERSION_MAX_ALLOWED to MAC_OS_VERSION_14_0
// so we can't use this macro to check for availability of very recent symbols.
#if __has_include ("CoreMIDI/MIDIUMPMutableFunctionBlock.h")
    #define JUCE_COREMIDI_UMP_ENDPOINT_CAN_BE_BUILT 1
#else
    #define JUCE_COREMIDI_UMP_ENDPOINT_CAN_BE_BUILT 0
#endif

struct CoreMidiHelpers
{
    #ifndef JUCE_LOG_COREMIDI_ERRORS
     #define JUCE_LOG_COREMIDI_ERRORS 1
    #endif

    static ump::Transport getPlatformTransport()
    {
        if (@available (macOS 11, iOS 14, *))
            return ump::Transport::ump;

        return ump::Transport::bytestream;
    }

    static constexpr MIDIProtocolID convertToPacketProtocol (ump::PacketProtocol p)
    {
        return p == ump::PacketProtocol::MIDI_2_0 ? kMIDIProtocol_2_0
                                                  : kMIDIProtocol_1_0;
    }

    static constexpr auto int14ToBytes (uint16_t x)
    {
        return std::array<std::byte, 2> { std::byte (x & 0x7f), std::byte ((x >> 7) & 0x7f) };
    }

    static constexpr auto bytesToInt14 (Span<const std::byte, 2> bytes)
    {
        return UInt16 ((UInt16 (bytes[0]) & 0x7f) | ((UInt16 (bytes[1]) & 0x7f) << 7));
    }

   #if JUCE_COREMIDI_UMP_ENDPOINT_CAN_BE_BUILT
    API_AVAILABLE (macos (15), ios (18))
    static ump::EndpointId getIdForNativeEndpoint (const MIDIUMPEndpoint* endpoint)
    {
        const auto src = getConnectedEndpointInfo (endpoint.MIDISource);
        const auto dst = getConnectedEndpointInfo (endpoint.MIDIDestination);
        return ump::EndpointId::makeSrcDst (src.identifier, dst.identifier);
    }
   #endif

    static ump::EndpointId convertToVirtualId (ump::EndpointId e)
    {
        const auto addPrefix = [] (auto str) { return str.isNotEmpty() ? "VIRTUAL" + str : ""; };
        return ump::EndpointId::makeSrcDst (addPrefix (e.src), addPrefix (e.dst));
    }

    static MIDIEntityRef findEntity (MIDIEndpointRef e)
    {
        if (e == MIDIEndpointRef{})
            return {};

        MIDIEntityRef result{};
        MIDIEndpointGetEntity (e, &result);
        return result;
    }

    static MIDIDeviceRef findDevice (MIDIEntityRef e)
    {
        if (e == MIDIEntityRef{})
            return {};

        MIDIDeviceRef result{};
        MIDIEntityGetDevice (e, &result);
        return result;
    }

    static String getStringProperty (MIDIObjectRef obj, const CFStringRef propertyName)
    {
        CFObjectHolder<CFStringRef> result;

        if (MIDIObjectGetStringProperty (obj, propertyName, &result.object) == noErr)
            return String::fromCFString (result.object);

        return {};
    }

    static String findName (MIDIObjectRef obj)
    {
        return getStringProperty (obj, kMIDIPropertyName);
    }

    static String findManufacturer (MIDIObjectRef obj)
    {
        return getStringProperty (obj, kMIDIPropertyManufacturer);
    }

    static String findProduct (MIDIObjectRef obj)
    {
        return getStringProperty (obj, kMIDIPropertyModel);
    }

    static bool checkError (OSStatus err, [[maybe_unused]] int lineNum)
    {
        if (err == noErr)
            return true;

       #if JUCE_LOG_COREMIDI_ERRORS
        Logger::writeToLog ("CoreMIDI error, line " + String (lineNum) + " - " + String::toHexString ((int) err));
       #endif

        return false;
    }

    static void logNSError ([[maybe_unused]] NSError* e, [[maybe_unused]] int lineNum)
    {
       #if JUCE_LOG_COREMIDI_ERRORS
        const auto errorText = std::invoke ([&]
        {
            if (e == nullptr)
                return String();

            return " - " + nsStringToJuce (e.localizedDescription)
                 + " - " + nsStringToJuce (e.localizedFailureReason);
        });

        Logger::writeToLog ("CoreMIDI error, line " + String (lineNum) + errorText);
       #endif
    }

    #pragma push_macro ("JUCE_CHECK_ERROR")
    #define JUCE_CHECK_ERROR(a) checkError (a, __LINE__)

    enum class ImplementationStrategy
    {
        onlyNew,
        both,
        onlyOld
    };

    #if JUCE_MAC_API_VERSION_MIN_REQUIRED_AT_LEAST (11, 0) || JUCE_IOS_API_VERSION_MIN_REQUIRED_AT_LEAST (14, 0)
     #define JUCE_HAS_OLD_COREMIDI_API 0
     static constexpr auto implementationStrategy = ImplementationStrategy::onlyNew;
    #else
     #define JUCE_HAS_OLD_COREMIDI_API 1
     static constexpr auto implementationStrategy = ImplementationStrategy::both;
    #endif

    template <typename Resource, typename Destructor>
    class ScopedMidiResource
    {
    public:
        ScopedMidiResource() = default;

        explicit ScopedMidiResource (Resource r) : contents (r, {}) {}

        ~ScopedMidiResource() noexcept
        {
            auto ref = std::get<0> (contents);

            if (ref != 0)
                std::get<1> (contents) (ref);
        }

        ScopedMidiResource (const ScopedMidiResource& other) = delete;
        ScopedMidiResource& operator= (const ScopedMidiResource& other) = delete;

        ScopedMidiResource (ScopedMidiResource&& other) noexcept { swap (other); }

        ScopedMidiResource& operator= (ScopedMidiResource&& other) noexcept
        {
            swap (other);
            return *this;
        }

        void swap (ScopedMidiResource& other) noexcept { std::swap (other.contents, contents); }

        Resource operator*() const noexcept { return std::get<0> (contents); }

        Resource release() noexcept
        {
            auto old = std::get<0> (contents);
            std::get<0> (contents) = 0;
            return old;
        }

    private:
        std::tuple<Resource, Destructor> contents { {}, {} };
    };

    struct PortRefDestructor
    {
        void operator() (MIDIPortRef p) const noexcept
        {
            JUCE_CHECK_ERROR (MIDIPortDispose (p));
        }
    };

    using ScopedPortRef = ScopedMidiResource<MIDIPortRef, PortRefDestructor>;

    struct EndpointRefDestructor
    {
        void operator() (MIDIEndpointRef p) const noexcept
        {
            JUCE_CHECK_ERROR (MIDIEndpointDispose (p));
        }
    };

    using ScopedEndpointRef = ScopedMidiResource<MIDIEndpointRef, EndpointRefDestructor>;

    //==============================================================================
    class MidiPortAndEndpoint
    {
    public:
        MidiPortAndEndpoint() = default;

        MidiPortAndEndpoint (ScopedPortRef p, ScopedEndpointRef ep) noexcept
            : port (std::move (p)), endpoint (std::move (ep)) {}

        MidiPortAndEndpoint (MidiPortAndEndpoint&&) noexcept = default;
        MidiPortAndEndpoint& operator= (MidiPortAndEndpoint&&) noexcept = default;

        ~MidiPortAndEndpoint() noexcept
        {
            // if port != 0, it means we didn't create the endpoint, so it's not safe to delete it
            if (*port != 0)
                endpoint.release();
        }

        MIDIPortRef getPort() const { return *port; }
        MIDIEndpointRef getEndpoint() const { return *endpoint; }

        void start() const
        {
            JUCE_CHECK_ERROR (MIDIPortConnectSource (*port, *endpoint, nullptr));
        }

    private:
        ScopedPortRef port;
        ScopedEndpointRef endpoint;
    };

    static MidiDeviceInfo getMidiObjectInfo (MIDIObjectRef entity)
    {
        const auto name = findName (entity);
        const auto identifier = std::invoke ([&]
        {
            SInt32 objectID = 0;

            if (JUCE_CHECK_ERROR (MIDIObjectGetIntegerProperty (entity, kMIDIPropertyUniqueID, &objectID)))
                return String (objectID);

            CFObjectHolder<CFStringRef> str;

            if (JUCE_CHECK_ERROR (MIDIObjectGetStringProperty (entity, kMIDIPropertyUniqueID, &str.object)))
                return String::fromCFString (str.object);

            return String{};
        });

        return MidiDeviceInfo { name, identifier };
    }

    static MidiDeviceInfo getEndpointInfo (MIDIEndpointRef endpoint, bool isExternal)
    {
        const auto entity = findEntity (endpoint);

        // probably virtual
        if (entity == MIDIEntityRef{})
            return getMidiObjectInfo (endpoint);

        auto result = getMidiObjectInfo (endpoint);

        // endpoint is empty - try the entity
        if (result == MidiDeviceInfo())
            result = getMidiObjectInfo (entity);

        // now consider the device
        const auto device = findDevice (entity);

        if (device == MIDIDeviceRef{})
            return result;

        const auto info = getMidiObjectInfo (device);

        if (info == MidiDeviceInfo())
            return result;

        // if an external device has only one entity, throw away
        // the endpoint name and just use the device name
        if (isExternal && MIDIDeviceGetNumberOfEntities (device) < 2)
        {
            result.name = info.name;
            return result;
        }

        if (! result.name.startsWithIgnoreCase (info.name))
        {
            // prepend the device name and identifier to the entity's
            return { (info.name + " " + result.name).trimEnd(),
                     info.identifier + " " + result.identifier };
        }

        return result;
    }

    static MidiDeviceInfo getConnectedEndpointInfo (MIDIEndpointRef endpoint)
    {
        MidiDeviceInfo result;

        // Does the endpoint have connections?
        CFObjectHolder<CFDataRef> connections;

        MIDIObjectGetDataProperty (endpoint, kMIDIPropertyConnectionUniqueID, &connections.object);

        if (connections.object != nullptr)
        {
            const auto numConnections = ((int) CFDataGetLength (connections.object)) / (int) sizeof (MIDIUniqueID);

            if (numConnections > 0)
            {
                auto* pid = reinterpret_cast<const SInt32*> (CFDataGetBytePtr (connections.object));

                for (int i = 0; i < numConnections; ++i, ++pid)
                {
                    const auto id = (MIDIUniqueID) ByteOrder::swapIfLittleEndian ((uint32) *pid);
                    MIDIObjectRef connObject;
                    MIDIObjectType connObjectType;
                    const auto err = MIDIObjectFindByUniqueID (id, &connObject, &connObjectType);

                    if (err == noErr)
                    {
                        const auto deviceInfo = connObjectType == kMIDIObjectType_ExternalSource
                                                || connObjectType == kMIDIObjectType_ExternalDestination
                                                // Connected to an external device's endpoint (10.3 and later).
                                                ? getEndpointInfo (static_cast<MIDIEndpointRef> (connObject), true)
                                                // Connected to an external device (10.2) (or something else, catch-all)
                                                : getMidiObjectInfo (connObject);

                        if (deviceInfo != MidiDeviceInfo{})
                        {
                            result.name += (result.name.isNotEmpty() ? ", " : "") + deviceInfo.name;
                            result.identifier += (result.identifier.isNotEmpty() ? ", " : "") + deviceInfo.identifier;
                        }
                    }
                }
            }
        }

        // Here, either the endpoint had no connections, or we failed to obtain names for them.
        if (result == MidiDeviceInfo{})
            return getEndpointInfo (endpoint, false);

        return result;
    }

    class EndpointInfo
    {
    public:
        ump::Endpoint endpoint;
        ump::StaticDeviceInfo staticInfo;
        MIDIEndpointRef src{};
        MIDIEndpointRef dst{};

        static EndpointInfo withEndpoint (ump::Endpoint ep,
                                          ump::IOKind kind,
                                          MIDIEndpointRef s,
                                          Span<const String, 16> ids)
        {
            return kind == ump::IOKind::src ? withSrc (ep, s, ids) : withDst (ep, s, ids);
        }

        static EndpointInfo withSrcDst (ump::Endpoint ep,
                                        MIDIEndpointRef s,
                                        Span<const String, 16> srcIds,
                                        MIDIEndpointRef d,
                                        Span<const String, 16> dstIds)
        {
            return { ep, s, srcIds, d, dstIds };
        }

        static EndpointInfo withSrc (ump::Endpoint ep, MIDIEndpointRef s, Span<const String, 16> srcIds)
        {
            return { ep, s, srcIds, {}, std::array<String, 16>{} };
        }

        static EndpointInfo withDst (ump::Endpoint ep, MIDIEndpointRef d, Span<const String, 16> dstIds)
        {
            return { ep, {}, std::array<String, 16>{}, d, dstIds };
        }

        MIDIEndpointRef get (ump::IOKind k) const
        {
            return k == ump::IOKind::src ? src : dst;
        }

    private:
        EndpointInfo() = default;
        EndpointInfo (ump::Endpoint ep,
                      MIDIEndpointRef s,
                      Span<const String, 16> srcIds,
                      MIDIEndpointRef d,
                      Span<const String, 16> dstIds)
            : endpoint (ep),
              staticInfo (ump::StaticDeviceInfo{}.withName (endpoint.getName())
                                                 .withManufacturer (findManufacturer (s != MIDIEndpointRef{} ? s : d))
                                                 .withTransport (getPlatformTransport())
                                                 .withProduct (findProduct (s != MIDIEndpointRef{} ? s : d))
                                                 .withHasSource (s != MIDIEndpointRef{})
                                                 .withHasDestination (s != MIDIEndpointRef{})
                                                 .withLegacyIdentifiersSrc (srcIds)
                                                 .withLegacyIdentifiersDst (dstIds)),
              src (s),
              dst (d) {}
    };

    struct EndpointRemovalListener
    {
        virtual ~EndpointRemovalListener() = default;
        virtual void endpointRemoved (MIDIEndpointRef) = 0;
    };

    /*
        This is used to implement the global Endpoints object.
        As well as providing access to cached ump::Endpoint instances,
        it also stores a mapping of ump::EndpointId to MIDIEndpointRef, which is used to open
        devices by their ID.
    */
    class SharedEndpointsImplNative
    {
    public:
        MIDIClientRef getClient() const { return client; }

        void getEndpoints (std::vector<ump::EndpointId>& x) const
        {
            endpoints.getEndpoints (x);
        }

        std::optional<ump::Endpoint> getEndpoint (const ump::EndpointId& x) const
        {
            if (const auto endpointInfo = getCachedInfo (x))
                return endpointInfo->endpoint;

            return {};
        }

        std::optional<ump::StaticDeviceInfo> getStaticDeviceInfo (const ump::EndpointId& x) const
        {
            if (const auto endpointInfo = getCachedInfo (x))
                return endpointInfo->staticInfo;

            return {};
        }

        MIDIEndpointRef getNativeEndpointForId (ump::IOKind kind, const ump::EndpointId& x) const
        {
            return endpoints.getNativeEndpointForId (kind, x);
        }

        void addEndpointRemovalListener (EndpointRemovalListener& l)
        {
            endpointRemovalListeners.add (&l);
        }

        void removeEndpointRemovalListener (EndpointRemovalListener& l)
        {
            endpointRemovalListeners.remove (&l);
        }

       #if JUCE_COREMIDI_UMP_ENDPOINT_CAN_BE_BUILT
        API_AVAILABLE (macos (15.0), ios (18.0))
        void addVirtualEndpoint (const ump::EndpointId& privateId, MIDIUMPEndpoint* endpoint)
        {
            virtualEndpoints[privateId] = endpoint;
        }

        void removeVirtualEndpoint (const ump::EndpointId& privateId)
        {
            virtualEndpoints.erase (privateId);
        }
       #endif

        void addLegacyVirtualPort (const ump::EndpointId& privateId, EndpointInfo ep)
        {
            legacyPorts.insert_or_assign (privateId, std::move (ep));
        }

        void removeLegacyVirtualPort (const ump::EndpointId& privateId)
        {
            legacyPorts.erase (privateId);
        }

        static std::shared_ptr<SharedEndpointsImplNative> make (ump::EndpointsListener& listener)
        {
            JUCE_ASSERT_MESSAGE_THREAD

            enableSimulatorMidiSession();

            const auto name = ump::Endpoints::Impl::getGlobalMidiClientName();
            CFUniquePtr<CFStringRef> cfName (name.toCFString());
            MIDIClientRef clientRef{};

            if (! JUCE_CHECK_ERROR (MIDIClientCreate (cfName.get(), systemChangeCallback, nullptr, &clientRef)))
                return nullptr;

            const std::shared_ptr<SharedEndpointsImplNative> result { new SharedEndpointsImplNative (clientRef, listener) };
            Listeners::get().add (result);
            return result;
        }

    private:
        std::optional<EndpointInfo> getCachedInfo (const ump::EndpointId& x) const
        {
           #if JUCE_COREMIDI_UMP_ENDPOINT_CAN_BE_BUILT
            if (@available (macOS 15, iOS 18, *))
                if (const auto iter = virtualEndpoints.find (x); iter != virtualEndpoints.end())
                    return getInfoForEndpoint (iter->second);
           #endif

            if (const auto iter = legacyPorts.find (x); iter != legacyPorts.end())
                return iter->second;

            return endpoints.getEndpointInfo (x);
        }

        class Listeners : private AsyncUpdater
        {
        public:
            ~Listeners() override
            {
                cancelPendingUpdate();
            }

            void add (std::weak_ptr<SharedEndpointsImplNative> ptr)
            {
                const std::scoped_lock lock { mutex };
                listeners.push_back (ptr);
            }

            void notify (std::optional<MIDIEndpointRef> removed)
            {
                {
                    const std::scoped_lock lock { mutex };
                    updateQueue.push_back (removed);
                }

                triggerMainThreadUpdate();
            }

            static Listeners& get()
            {
                static Listeners listeners;
                return listeners;
            }

        private:
            Listeners() = default;

            void triggerMainThreadUpdate()
            {
                if (MessageManager::getInstance()->isThisTheMessageThread())
                    handleAsyncUpdate();
                else
                    triggerAsyncUpdate();
            }

            void handleAsyncUpdate() override
            {
                const auto [listenersCopy, updatesCopy] = std::invoke ([&]
                {
                    const std::scoped_lock lock { mutex };
                    const ScopeGuard scope { [this] { updateQueue.clear(); } };
                    return std::tuple (listeners, updateQueue);
                });

                for (const auto& u : updatesCopy)
                    for (const auto& l : listenersCopy)
                        if (const auto locked = l.lock())
                            locked->notify (u);

                const std::scoped_lock lock { mutex };
                listeners.erase (std::remove_if (listeners.begin(), listeners.end(), [] (auto& l) { return ! l.lock(); }),
                                 listeners.end());
            }

            std::mutex mutex;
            std::vector<std::weak_ptr<SharedEndpointsImplNative>> listeners;

            // Queue of update messages to send, nullopt indicates that the event was not a removal
            std::vector<std::optional<MIDIEndpointRef>> updateQueue;
        };

        SharedEndpointsImplNative (MIDIClientRef c, ump::EndpointsListener& l)
            : client (c), listener (l)
        {
            if (@available (macOS 15.0, iOS 18.0, *))
                observers.emplace (*this);
        }

        static void enableSimulatorMidiSession()
        {
           #if TARGET_OS_SIMULATOR
            static bool hasEnabledNetworkSession = false;

            if (! hasEnabledNetworkSession)
            {
                MIDINetworkSession* session = [MIDINetworkSession defaultSession];
                session.enabled = YES;
                session.connectionPolicy = MIDINetworkConnectionPolicy_Anyone;

                hasEnabledNetworkSession = true;
            }
           #endif
        }

        static void systemChangeCallback (const MIDINotification* notification, void*)
        {
            if (notification == nullptr)
                return;

            switch (notification->messageID)
            {
                case kMIDIMsgObjectRemoved:
                {
                    const auto& addRemove = *reinterpret_cast<const MIDIObjectAddRemoveNotification*> (notification);

                    if (addRemove.childType == kMIDIObjectType_Source || addRemove.childType == kMIDIObjectType_Destination)
                        Listeners::get().notify (addRemove.child);

                    break;
                }

                case kMIDIMsgObjectAdded:
                case kMIDIMsgPropertyChanged:
                case kMIDIMsgThruConnectionsChanged:
                case kMIDIMsgIOError:
                case kMIDIMsgSerialPortOwnerChanged:
                    Listeners::get().notify (std::nullopt);
                    break;

                case kMIDIMsgSetupChanged:
               #if JUCE_IOS
                case kMIDIMsgInternalStart:
               #endif

                default:
                    break;
            }
        }

        template <typename Numeric, size_t N>
        static auto copyToByteArray (const Numeric (&arr)[N])
        {
            std::array<std::byte, N> result;
            std::transform (std::begin (arr), std::end (arr), std::begin (result), [] (auto x) { return std::byte (x); });
            return result;
        }

        template <typename Fn>
        static void forEachEndpointInDirection (ump::IOKind dir, Fn&& callback)
        {
            const auto end = dir == ump::IOKind::src
                           ? MIDIGetNumberOfSources()
                           : MIDIGetNumberOfDestinations();

            const auto get = dir == ump::IOKind::src
                           ? MIDIGetSource
                           : MIDIGetDestination;

            for (auto i = decltype (end){}; i < end; ++i)
            {
                const MIDIEndpointRef ref = get (i);

                if (ref == MIDIEndpointRef{})
                    continue;

                callback (ref);
            }
        }

        static std::array<String, 16> getGroupIdentifiers (MIDIDeviceRef dev, ump::IOKind dir)
        {
            std::array<String, 16> result;

            forEachEndpointInDevice (dev, dir, [&] (auto endpoint)
            {
                const auto identifier = getConnectedEndpointInfo (endpoint).identifier;
                const auto bitmap = getUMPActiveGroupBitmap (endpoint);

                if (! bitmap.has_value())
                {
                    result[0] = identifier;
                    return;
                }

                if (countNumberOfBits ((uint32) *bitmap) > 1)
                    return;

                const auto index = log2 (*bitmap);

                if (index >= result.size())
                {
                    // This shouldn't happen, there's a max of 16 groups!
                    jassertfalse;
                    return;
                }

                result[(size_t) index] = identifier;
            });

            return result;
        }

        static size_t getSyntheticBlocks (MIDIDeviceRef dev, Span<ump::Block, 32> blocks)
        {
            size_t result = 0;

            for (const auto dir : ump::ioKinds)
            {
                forEachEndpointInDevice (dev, dir, [&] (auto endpoint)
                {
                    const auto bitmap = getUMPActiveGroupBitmap (endpoint);

                    if (! bitmap.has_value())
                        return;

                    const auto numGroups = countNumberOfBits ((uint32) *bitmap);

                    if (numGroups == 0)
                        return;

                    const auto index = log2 (*bitmap);

                    if (result >= blocks.size())
                    {
                        // This shouldn't happen, there's a max of 32 blocks!
                        jassertfalse;
                        return;
                    }

                    blocks[result++] = ump::Block{}.withDirection (dir == ump::IOKind::src ? ump::BlockDirection::sender : ump::BlockDirection::receiver)
                                                   .withUiHint (dir == ump::IOKind::src ? ump::BlockUiHint::sender : ump::BlockUiHint::receiver)
                                                   .withEnabled (true)
                                                   .withFirstGroup ((uint8_t) index)
                                                   .withNumGroups ((uint8_t) numGroups)
                                                   .withMaxSysex8Streams (0)
                                                   .withMIDI1ProxyKind (ump::BlockMIDI1ProxyKind::inapplicable)
                                                   .withName (findName (endpoint));
                });
            }

            return result;
        }

       #if JUCE_COREMIDI_UMP_ENDPOINT_CAN_BE_BUILT
        API_AVAILABLE (macos (15.0), ios (18.0))
        static EndpointInfo getInfoForEndpoint (MIDIUMPEndpoint* endpoint)
        {
            jassert (endpoint != nullptr);

            const auto protocol = endpoint.MIDIProtocol == kMIDIProtocol_2_0
                                ? ump::PacketProtocol::MIDI_2_0
                                : ump::PacketProtocol::MIDI_1_0;

            const ump::DeviceInfo deviceInfo
            {
                copyToByteArray (endpoint.deviceInfo.manufacturerID.sysExIDByte),
                int14ToBytes (endpoint.deviceInfo.family),
                int14ToBytes (endpoint.deviceInfo.modelNumber),
                copyToByteArray (endpoint.deviceInfo.revisionLevel.revisionLevel),
            };

            std::array<ump::Block, 32> blocks;
            size_t blockCount = 0;

            for (MIDIUMPFunctionBlock* block in endpoint.functionBlocks)
            {
                const auto fb = ump::Block{}.withName (nsStringToJuce (block.name))
                                            .withFirstGroup (block.firstGroup)
                                            .withNumGroups (block.totalGroupsSpanned)
                                            .withMaxSysex8Streams (block.maxSysEx8Streams)
                                            .withDirection (ump::BlockDirection { (uint8_t) block.direction })
                                            .withUiHint (ump::BlockUiHint { (uint8_t) block.UIHint })
                                            .withMIDI1ProxyKind (ump::BlockMIDI1ProxyKind { (uint8_t) block.MIDI1Info })
                                            .withEnabled (block.isEnabled);

                blocks[blockCount++] = fb;
            }

            const auto srcManufacturer = findManufacturer (endpoint.MIDISource);
            const auto dstManufacturer = findManufacturer (endpoint.MIDIDestination);

            const auto item = ump::Endpoint{}.withName (nsStringToJuce (endpoint.name))
                                             .withProtocol (protocol)
                                             .withDeviceInfo (deviceInfo)
                                             .withProductInstanceId (nsStringToJuce (endpoint.productInstanceID))
                                             .withReceiveJRSupport (endpoint.hasJRTSReceiveCapability)
                                             .withTransmitJRSupport (endpoint.hasJRTSTransmitCapability)
                                             .withStaticBlocks (endpoint.hasStaticFunctionBlocks)
                                             .withMidi1Support ((endpoint.supportedMIDIProtocols & kMIDIUMPProtocolOptionsMIDI1) != 0)
                                             .withMidi2Support ((endpoint.supportedMIDIProtocols & kMIDIUMPProtocolOptionsMIDI2) != 0)
                                             .withBlocks ({ blocks.data(), blockCount });

            return EndpointInfo::withSrcDst (item,
                                             endpoint.MIDISource,
                                             getGroupIdentifiers (findDevice (findEntity (endpoint.MIDISource)), ump::IOKind::src),
                                             endpoint.MIDIDestination,
                                             getGroupIdentifiers (findDevice (findEntity (endpoint.MIDIDestination)), ump::IOKind::dst));
        }
       #endif

        static std::map<ump::EndpointId, EndpointInfo> findNativeUMPEndpoints()
        {
           #if JUCE_COREMIDI_UMP_ENDPOINT_CAN_BE_BUILT
            if (@available (macOS 15, iOS 18, *))
            {
                std::map<ump::EndpointId, EndpointInfo> result;

                for (MIDIUMPEndpoint* endpoint in MIDIUMPEndpointManager.sharedInstance.UMPEndpoints)
                {
                    const auto id = getIdForNativeEndpoint (endpoint);
                    const auto info = getInfoForEndpoint (endpoint);
                    result.emplace (id, info);
                }

                return result;
            }
           #endif

            return {};
        }

        static std::optional<SInt32> getUMPActiveGroupBitmap ([[maybe_unused]] MIDIEndpointRef r)
        {
           #if JUCE_MAC_API_VERSION_CAN_BE_BUILT (14, 0) || JUCE_IOS_API_VERSION_CAN_BE_BUILT (17, 0)
            if (@available (macOS 14, iOS 17, *))
            {
                SInt32 bitmap{};

                // If there's no active group bitmap, then this endpoint might be a native UMP endpoint
                // that supports all groups, but we probably should have found that via the UMP endpoint manager.
                if (MIDIObjectGetIntegerProperty (r, kMIDIPropertyUMPActiveGroupBitmap, &bitmap) == noErr)
                    return bitmap;
            }
           #endif

            return {};
        }

        static bool canTransmitGroupless ([[maybe_unused]] MIDIEndpointRef endpoint)
        {
           #if JUCE_MAC_API_VERSION_CAN_BE_BUILT (14, 0) || JUCE_IOS_API_VERSION_CAN_BE_BUILT (17, 0)
            if (@available (macOS 14, iOS 17, *))
            {
                SInt32 result;
                if (MIDIObjectGetIntegerProperty (endpoint, kMIDIPropertyUMPCanTransmitGroupless, &result) == noErr)
                    return result;
            }
           #endif

            return false;
        }

        template <typename Callback>
        static void forEachEndpointInDevice (MIDIDeviceRef dev, ump::IOKind dir, Callback&& cb)
        {
            const auto entities = MIDIDeviceGetNumberOfEntities (dev);

            for (auto i = decltype (entities){}; i < entities; ++i)
            {
                const auto entity = MIDIDeviceGetEntity (dev, i);

                const auto endpoints = dir == ump::IOKind::src
                                     ? MIDIEntityGetNumberOfSources (entity)
                                     : MIDIEntityGetNumberOfDestinations (entity);

                for (auto j = decltype (endpoints){}; j < endpoints; ++j)
                {
                    const auto endpoint = dir == ump::IOKind::src
                                        ? MIDIEntityGetSource (entity, j)
                                        : MIDIEntityGetDestination (entity, j);

                    cb (endpoint);
                }
            }
        }

        static std::optional<MIDIEndpointRef> findGrouplessEndpoint (MIDIDeviceRef dev, ump::IOKind dir)
        {
            std::optional<MIDIEndpointRef> result;

            forEachEndpointInDevice (dev, dir, [&] (auto endpoint)
            {
                if (! canTransmitGroupless (endpoint))
                    return;

                // If this is hit, there's more than one groupless endpoint in this device!
                // Maybe the device has multiple UMP endpoints, which is a scenario I didn't
                // anticipate.
                jassert (! result.has_value());
                result = endpoint;
            });

            return result;
        }

        static std::map<ump::EndpointId, EndpointInfo> findAllUMPDevices()
        {
            std::set<MIDIDeviceRef> devices;

            for (const auto dir : ump::ioKinds)
            {
                forEachEndpointInDirection (dir, [&] (auto endpoint)
                {
                    const auto groupless = canTransmitGroupless (endpoint);
                    const auto bitmap = getUMPActiveGroupBitmap (endpoint);

                    if (! groupless && ! bitmap.has_value())
                        return;

                    // If any endpoint in the device has the active group bitmap set, or is able
                    // to transmit groupless messages, we assume the endpoint supports multiple
                    // groups, i.e. it is a UMP endpoint.
                    // This assumption does not hold if the device holds a UMP port and completely
                    // separate non-multiplexed MIDI 1.0 ports. It also does not hold if the
                    // endpoint has a single GTB spanning all 16 groups, although in that case
                    // we probably won't see individual ports referring to GTBs multiplexed on the
                    // endpoint, so I don't think we need to worry about that.
                    const auto device = findDevice (findEntity (endpoint));

                    if (device == MIDIDeviceRef{})
                        return;

                    devices.insert (device);
                });
            }

            std::array<ump::Block, 32> blocks;
            std::map<ump::EndpointId, EndpointInfo> result;

            for (const auto& dev : devices)
            {
                // If this is a UMP endpoint, then in each direction there should be one
                // MIDIEndpointRef that 'contains' the other MIDIEndpointRefs.
                // We assume this is the endpoint that can transmit groupless messages.
                const auto src = findGrouplessEndpoint (dev, ump::IOKind::src);
                const auto dst = findGrouplessEndpoint (dev, ump::IOKind::dst);
                const auto numBlocks = getSyntheticBlocks (dev, blocks);

                const auto item = ump::Endpoint{}.withName (findName (dev))
                                                 .withProtocol (ump::PacketProtocol::MIDI_1_0)
                                                 .withStaticBlocks (true)
                                                 .withBlocks (Span { blocks.data(), numBlocks });

                const auto srcIds = getGroupIdentifiers (dev, ump::IOKind::src);
                const auto dstIds = getGroupIdentifiers (dev, ump::IOKind::dst);

                const auto info = std::invoke ([&]() -> std::optional<std::pair<ump::EndpointId, EndpointInfo>>
                {
                    if (src.has_value() && dst.has_value())
                    {
                        const auto srcInfo = getConnectedEndpointInfo (*src);
                        const auto dstInfo = getConnectedEndpointInfo (*dst);
                        const auto id = ump::EndpointId::makeSrcDst (srcInfo.identifier, dstInfo.identifier);
                        return std::tuple (id, EndpointInfo::withSrcDst (item, *src, srcIds, *dst, dstIds));
                    }

                    if (src.has_value())
                    {
                        const auto srcInfo = getConnectedEndpointInfo (*src);
                        const auto id = ump::EndpointId::make (ump::IOKind::src, srcInfo.identifier);
                        return std::tuple (id, EndpointInfo::withSrc (item, *src, srcIds));
                    }

                    if (dst.has_value())
                    {
                        const auto dstInfo = getConnectedEndpointInfo (*dst);
                        const auto id = ump::EndpointId::make (ump::IOKind::dst, dstInfo.identifier);
                        return std::tuple (id, EndpointInfo::withDst (item, *dst, dstIds));
                    }

                    return {};
                });

                if (info.has_value())
                    result.insert (*info);
            }

            return result;
        }

        struct Endpoints
        {
        public:
            void getEndpoints (std::vector<ump::EndpointId>& x) const
            {
                x.insert (x.end(), orderedEndpoints.begin(), orderedEndpoints.end());
            }

            std::optional<EndpointInfo> getEndpointInfo (const ump::EndpointId& x) const
            {
                const auto iter = endpoints.find (x);

                if (iter == endpoints.end())
                    return {};

                return iter->second;
            }

            MIDIEndpointRef getNativeEndpointForId (ump::IOKind kind, const ump::EndpointId& x) const
            {
                const auto iter = endpoints.find (x);

                if (iter == endpoints.end())
                    return {};

                return iter->second.get (kind);
            }

        private:
            std::map<ump::EndpointId, EndpointInfo> endpoints = std::invoke ([&]
            {
                std::map<ump::EndpointId, EndpointInfo> result;

                // In CoreMIDI, each MIDI device may have any number of Entities, each of which may
                // hold any number of input and output ports. Ideally, we'd represent each MIDIDevice
                // as a separate UMP endpoint, but there are some difficulties with this approach:
                // - A UMP endpoint can only hold up to 16 input/output channels, so we'll run into
                //   problems with message addressing if the total number of endpoints on the device is
                //   greater than this.
                //   - e.g. the IAC client allows more than 16 buses, each of which is a single entity
                //     containing an input and and output endpoint.
                // - The mapping between entities and function blocks isn't clear. If an entity has
                //   only inputs or only outputs, this is fairly straightforward. But, if the entity
                //   has a mismatched non-zero number of inputs and outputs this can't be represented
                //   as a function block.
                // Instead of grouping ports in this way, we create a separate UMP endpoint for each
                // port. This means that, if the device holds more than 16 ins/outs, we can create
                // and endpoint for each, and we don't need to worry about addressing messages to
                // groups indices that can't be represented in UMP.

                // It seems that OSX can be a bit picky about the thread that's first used to
                // search for devices. It's safest to use the message thread for calling this.
                JUCE_ASSERT_MESSAGE_THREAD

                const auto bidiUmpEndpoints = findNativeUMPEndpoints();

                // Might include duplicates of the devices in the UMPEndpointManager
                const auto allUmpDevices = findAllUMPDevices();

                result.insert (bidiUmpEndpoints.begin(), bidiUmpEndpoints.end());
                // Resolve duplicates in favour of the devices that the system reported as UMP-native
                result.insert (allUmpDevices.begin(), allUmpDevices.end());

                // We've found all UMP endpoints; find the devices holding those endpoints
                const auto umpDevices = std::invoke ([&]
                {
                    std::set<MIDIDeviceRef> set;

                    for (const auto& item : result)
                    {
                        set.insert (findDevice (findEntity (item.second.src)));
                        set.insert (findDevice (findEntity (item.second.dst)));
                    }

                    return set;
                });

                // Now look for endpoints that aren't part of a UMP device
                for (const auto dir : ump::ioKinds)
                {
                    forEachEndpointInDirection (dir, [&] (MIDIEndpointRef e)
                    {
                        const auto endpointDevice = findDevice (findEntity (e));

                        if (umpDevices.count (endpointDevice) != 0)
                            return;

                        const auto port = getConnectedEndpointInfo (e);

                        if (port == MidiDeviceInfo{})
                            return;

                        const auto direction = dir == ump::IOKind::src ? ump::BlockDirection::sender : ump::BlockDirection::receiver;
                        const auto endpoint = ump::IOHelpers::makeProxyEndpoint (port, direction);
                        auto toEmplace = EndpointInfo::withEndpoint (endpoint.endpoint,
                                                                     dir,
                                                                     e,
                                                                     endpoint.info.getLegacyIdentifiers (dir));

                        const auto emplaceResult = result.emplace (endpoint.id, std::move (toEmplace));
                        jassertquiet (emplaceResult.second);
                    });
                }

                return result;
            });

            // Old JUCE versions return MIDI devices in the order they appear when iterating
            // endpoints with MIDIGetSource() and MIDIGetDestination(). The default device of each
            // type is the result of calling these functions with an argument of 0.
            // In order for the new MIDI implementation to return the same default devices and
            // a similar device ordering (for display in lists/dropdowns etc.) we attempt to
            // sort the devices so that they contine to match the order returned by MIDIGetSource()
            // and MIDIGetDestination().
            std::vector<ump::EndpointId> orderedEndpoints = std::invoke ([&]
            {
                const auto idForEndpoint = std::invoke ([&]
                {
                    std::map<MIDIEndpointRef, ump::EndpointId> map;

                    for (const auto& item : endpoints)
                    {
                        if (item.second.src != MIDIEndpointRef{})
                            map[item.second.src] = item.first;

                        if (item.second.dst != MIDIEndpointRef{})
                            map[item.second.dst] = item.first;
                    }

                    return map;
                });

                std::vector<ump::EndpointId> result;
                std::set<ump::EndpointId> encountered;

                for (const auto dir : ump::ioKinds)
                {
                    forEachEndpointInDirection (dir, [&] (MIDIEndpointRef e)
                    {
                        const auto iter = idForEndpoint.find (e);

                        if (iter == idForEndpoint.end())
                            return;

                        const auto id = iter->second;

                        if (encountered.count (id) != 0)
                            return;

                        encountered.insert (id);
                        result.push_back (id);
                    });
                }

                return result;
            });
        };

        void notify (std::optional<MIDIEndpointRef> removedEndpoint)
        {
            JUCE_ASSERT_MESSAGE_THREAD;

            endpoints = Endpoints{};

            if (removedEndpoint.has_value())
            {
                endpointRemovalListeners.call ([&] (auto& c)
                {
                    c.endpointRemoved (*removedEndpoint);
                });
            }

            listener.endpointsChanged();
        }

        struct Observers
        {
            API_AVAILABLE (macos (15.0), ios (18.0))
            explicit Observers ([[maybe_unused]] SharedEndpointsImplNative& owner)
               #if JUCE_COREMIDI_UMP_ENDPOINT_CAN_BE_BUILT
                : added     { MIDIUMPEndpointWasAddedNotification,        MIDIUMPEndpointManager.sharedInstance, [&owner] { owner.notify (std::nullopt); } },
                  // We should get the individual endpoint removal notifications through the global MIDI notification mechanism
                  removed   { MIDIUMPEndpointWasRemovedNotification,      MIDIUMPEndpointManager.sharedInstance, [&owner] { owner.notify (std::nullopt); } },
                  updated   { MIDIUMPEndpointWasUpdatedNotification,      MIDIUMPEndpointManager.sharedInstance, [&owner] { owner.notify (std::nullopt); } },
                  fbUpdated { MIDIUMPFunctionBlockWasUpdatedNotification, MIDIUMPEndpointManager.sharedInstance, [&owner] { owner.notify (std::nullopt); } }
                #endif
            {
            }

           #if JUCE_COREMIDI_UMP_ENDPOINT_CAN_BE_BUILT
            FunctionNotificationCenterObserver added, removed, updated, fbUpdated;
           #endif
        };

       #if JUCE_COREMIDI_UMP_ENDPOINT_CAN_BE_BUILT
        JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wunguarded-availability-new")
        std::map<ump::EndpointId, MIDIUMPEndpoint*> virtualEndpoints;
        JUCE_END_IGNORE_WARNINGS_GCC_LIKE
       #endif

        std::map<ump::EndpointId, EndpointInfo> legacyPorts;

        MIDIClientRef client;
        std::optional<Observers> observers;
        ump::EndpointsListener& listener;
        ListenerList<EndpointRemovalListener> endpointRemovalListeners;
        Endpoints endpoints;
    };

    struct TimeConverter
    {
        CoreAudioTimeConversions timeConversions;
       #if JUCE_IOS
        const MIDITimeStamp startTimeNative = mach_absolute_time();
       #else
        const MIDITimeStamp startTimeNative = AudioGetCurrentHostTime();
       #endif
        uint32_t startTimeMillis = Time::getMillisecondCounter();

        double convertToMillis (uint64_t packetTimeNative) const
        {
            const auto elapsedTime = packetTimeNative - startTimeNative;
            return startTimeMillis + (1e-6 * (double) timeConversions.hostTimeToNanos (elapsedTime));
        }
    };

    //==============================================================================
    template <ImplementationStrategy>
    struct Receiver;

    template <ImplementationStrategy>
    struct CreatorFunctions;

    using ReceiverToUse = Receiver<implementationStrategy>;
    using CreatorFunctionsToUse = CreatorFunctions<implementationStrategy>;

    template <>
    struct Receiver<ImplementationStrategy::onlyNew>
    {
        explicit Receiver (ump::Consumer& cb)
            : callback (cb) {}

        void pushUmp (const MIDIEventList* list)
        {
            auto* packet = list->packet;

            for (uint32_t i = 0; i < list->numPackets; ++i)
            {
                const auto juceTimeMillis = converter.convertToMillis (packet->timeStamp);

                static_assert (sizeof (uint32_t) == sizeof (UInt32)
                               && alignof (uint32_t) == alignof (UInt32),
                               "If this fails, the cast below will be broken too!");

                callback.consume (ump::Iterator { packet->words, packet->wordCount },
                                  ump::Iterator { packet->words + packet->wordCount, 0 },
                                  juceTimeMillis * 0.001);

                packet = MIDIEventPacketNext (packet);
            }
        }

    private:
        ump::Consumer& callback;
        TimeConverter converter;
    };

    template <>
    struct API_AVAILABLE (macos (11.0), ios (14.0)) CreatorFunctions<ImplementationStrategy::onlyNew>
    {
        static OSStatus createInputPort (ump::PacketProtocol protocol,
                                         MIDIClientRef client,
                                         CFStringRef portName,
                                         ReceiverToUse* refCon,
                                         MIDIPortRef* outPort)
        {
            return MIDIInputPortCreateWithProtocol (client,
                                                    portName,
                                                    convertToPacketProtocol (protocol),
                                                    outPort,
                                                    ^void (const MIDIEventList* l, void* src)
                                                    {
                                                        newMidiInputProc (l, refCon, src);
                                                    });
        }

        static OSStatus createDestination (ump::PacketProtocol protocol,
                                           MIDIClientRef client,
                                           CFStringRef name,
                                           ReceiverToUse* refCon,
                                           MIDIEndpointRef* outDest)
        {
            return MIDIDestinationCreateWithProtocol (client,
                                                      name,
                                                      convertToPacketProtocol (protocol),
                                                      outDest,
                                                      ^void (const MIDIEventList* l, void* src)
                                                      {
                                                          newMidiInputProc (l, refCon, src);
                                                      });
        }

        static OSStatus createSource (ump::PacketProtocol protocol,
                                      MIDIClientRef client,
                                      CFStringRef name,
                                      MIDIEndpointRef* outSrc)
        {
            return MIDISourceCreateWithProtocol (client,
                                                 name,
                                                 convertToPacketProtocol (protocol),
                                                 outSrc);
        }

    private:
        static void newMidiInputProc (const MIDIEventList* list, void* readProcRefCon, void*)
        {
            static_cast<ReceiverToUse*> (readProcRefCon)->pushUmp (list);
        }
    };

   #if JUCE_HAS_OLD_COREMIDI_API
    template <>
    struct Receiver<ImplementationStrategy::onlyOld>
    {
        explicit Receiver (ump::Consumer& cb)
            : dispatcher (0, ump::PacketProtocol::MIDI_1_0, 4096), callback (cb) {}

        void pushBytes (const MIDIPacketList* list)
        {
            auto* packet = list->packet;

            for (unsigned int i = 0; i < list->numPackets; ++i)
            {
                const auto juceTimeMillis = converter.convertToMillis (packet->timeStamp);
                const auto* bytes = unalignedPointerCast<const std::byte*> (packet->data);
                const auto len = readUnaligned<decltype (packet->length)> (&(packet->length));

                dispatcher.dispatch (Span (bytes, len), juceTimeMillis * 0.001, [this] (const ump::View& v, double time)
                {
                    ump::Iterator b { v.data(), v.size() };
                    auto e = std::next (b);
                    callback.consume (b, e, time);
                });

                packet = MIDIPacketNext (packet);
            }
        }

    private:
        ump::BytestreamToUMPDispatcher dispatcher;
        ump::Consumer& callback;
        TimeConverter converter;
    };

    template <>
    struct CreatorFunctions<ImplementationStrategy::onlyOld>
    {
        static OSStatus createInputPort (ump::PacketProtocol,
                                         MIDIClientRef client,
                                         CFStringRef portName,
                                         ReceiverToUse* refCon,
                                         MIDIPortRef* outPort)
        {
            return MIDIInputPortCreate (client, portName, oldMidiInputProc, refCon, outPort);
        }

        static OSStatus createDestination (ump::PacketProtocol,
                                           MIDIClientRef client,
                                           CFStringRef name,
                                           ReceiverToUse* refCon,
                                           MIDIEndpointRef* outDest)
        {
            return MIDIDestinationCreate (client, name, oldMidiInputProc, refCon, outDest);
        }

        static OSStatus createSource (ump::PacketProtocol,
                                      MIDIClientRef client,
                                      CFStringRef name,
                                      MIDIEndpointRef* outSrc)
        {
            return MIDISourceCreate (client, name, outSrc);
        }

    private:
        static void oldMidiInputProc (const MIDIPacketList* list, void* readProcRefCon, void*)
        {
            static_cast<ReceiverToUse*> (readProcRefCon)->pushBytes (list);
        }
    };

    template <>
    struct Receiver<ImplementationStrategy::both>
    {
        explicit Receiver (ump::Consumer& cb)
            : newReceiver (cb), oldReceiver (cb) {}

        void pushUmp (const MIDIEventList* list)
        {
            newReceiver.pushUmp (list);
        }

        void pushBytes (const MIDIPacketList* list)
        {
            oldReceiver.pushBytes (list);
        }

    private:
        Receiver<ImplementationStrategy::onlyNew> newReceiver;
        Receiver<ImplementationStrategy::onlyOld> oldReceiver;
    };

    template <>
    struct CreatorFunctions<ImplementationStrategy::both>
    {
    private:
        template <typename Fn>
        static auto call (Fn&& fn)
        {
            if (@available (macOS 11, iOS 14, *))
                return fn (CreatorFunctions<ImplementationStrategy::onlyNew>{});

            return fn (CreatorFunctions<ImplementationStrategy::onlyOld>{});
        }

    public:
        static OSStatus createInputPort (ump::PacketProtocol protocol,
                                         MIDIClientRef client,
                                         CFStringRef portName,
                                         ReceiverToUse* refCon,
                                         MIDIPortRef* outPort)
        {
            return call ([&] (const auto& fn) { return fn.createInputPort (protocol, client, portName, refCon, outPort); });
        }

        static OSStatus createDestination (ump::PacketProtocol protocol,
                                           MIDIClientRef client,
                                           CFStringRef name,
                                           ReceiverToUse* refCon,
                                           MIDIEndpointRef* outDest)
        {
            return call ([&] (const auto& fn) { return fn.createDestination (protocol, client, name, refCon, outDest); });
        }

        static OSStatus createSource (ump::PacketProtocol protocol,
                                      MIDIClientRef client,
                                      CFStringRef name,
                                      MIDIEndpointRef* outSrc)
        {
            return call ([&] (const auto& fn) { return fn.createSource (protocol, client, name, outSrc); });
        }
    };
   #endif

    //==============================================================================
    struct OutputInterface
    {
        virtual ~OutputInterface() = default;
        virtual bool send (const MidiPortAndEndpoint& portAndEndpoint, ump::Iterator b, ump::Iterator e) = 0;
    };

    template <ImplementationStrategy>
    class Output;

    using OutputToUse = Output<implementationStrategy>;

    template <>
    class API_AVAILABLE (macos (11.0), ios (14.0)) Output<ImplementationStrategy::onlyNew> final : public OutputInterface
    {
    public:
        bool send (const MidiPortAndEndpoint& portAndEndpoint, ump::Iterator b, ump::Iterator e) override
        {
           #if JUCE_IOS
            const MIDITimeStamp timeStamp = mach_absolute_time();
           #else
            const MIDITimeStamp timeStamp = AudioGetCurrentHostTime();
           #endif

            MIDIEventList stackList = {};
            MIDIEventPacket* end = nullptr;

            const auto isMessageKind = [] (const auto kind)
            {
                return [kind] (const ump::View& v) { return ump::Utils::getMessageType (v[0]) == kind; };
            };

            const auto containsChannelVoice2 = std::any_of (b, e, isMessageKind (ump::Utils::MessageKind::channelVoice2));

           #if JUCE_ASSERTIONS_ENABLED_OR_LOGGED
            const auto containsChannelVoice1 = std::any_of (b, e, isMessageKind (ump::Utils::MessageKind::channelVoice1));

            // If this is hit, you're trying to send a mixture of Channel Voice 1.0 and Channel Voice 2.0
            // messages. This is probably a mistake!
            jassert (! (containsChannelVoice1 && containsChannelVoice2));
           #endif

            const auto init = [&]
            {
                // We rely on the system to convert from the declared packet protocol if necessary.
                end = MIDIEventListInit (&stackList, containsChannelVoice2 ? kMIDIProtocol_2_0 : kMIDIProtocol_1_0);
            };

            const auto send = [&]
            {
                JUCE_CHECK_ERROR (portAndEndpoint.getPort() != 0 ? MIDISendEventList (portAndEndpoint.getPort(), portAndEndpoint.getEndpoint(), &stackList)
                                                                 : MIDIReceivedEventList (portAndEndpoint.getEndpoint(), &stackList));
            };

            const auto add = [&] (const ump::View& view)
            {
                static_assert (sizeof (uint32_t) == sizeof (UInt32)
                               && alignof (uint32_t) == alignof (UInt32),
                               "If this fails, the cast below will be broken too!");
                end = MIDIEventListAdd (&stackList,
                                        sizeof (MIDIEventList::packet),
                                        end,
                                        timeStamp,
                                        view.size(),
                                        reinterpret_cast<const UInt32*> (view.data()));
            };

            init();

            std::for_each (b, e, [&] (ump::View view)
            {
                add (view);

                if (end != nullptr)
                    return;

                send();
                init();
                add (view);
            });

            send();

            return true;
        }
    };

   #if JUCE_HAS_OLD_COREMIDI_API
    template <>
    class Output<ImplementationStrategy::onlyOld> final : public OutputInterface
    {
    public:
        bool send (const MidiPortAndEndpoint& portAndEndpoint, ump::Iterator b, ump::Iterator e) override
        {
            for (const auto& v : makeRange (b, e))
            {
                converter.convert (v, 0.0, [&] (ump::BytesOnGroup msg, double)
                {
                    sendBytes (portAndEndpoint, msg.bytes);
                });
            }

            return true;
        }

    private:
        void sendBytes (const MidiPortAndEndpoint& portAndEndpoint, Span<const std::byte> message)
        {
           #if JUCE_IOS
            const MIDITimeStamp timeStamp = mach_absolute_time();
           #else
            const MIDITimeStamp timeStamp = AudioGetCurrentHostTime();
           #endif

            HeapBlock<MIDIPacketList> allocatedPackets;
            MIDIPacketList stackPacket;
            auto* packetToSend = &stackPacket;
            auto dataSize = message.size();

            const auto isSysEx = ! message.empty() && message.front() == std::byte { 0xf0 };

            if (isSysEx)
            {
                const int maxPacketSize = 256;
                int pos = 0, bytesLeft = (int) dataSize;
                const int numPackets = (bytesLeft + maxPacketSize - 1) / maxPacketSize;
                allocatedPackets.malloc ((size_t) (32 * (size_t) numPackets + dataSize), 1);
                packetToSend = allocatedPackets;
                packetToSend->numPackets = (UInt32) numPackets;

                auto* p = packetToSend->packet;

                for (int i = 0; i < numPackets; ++i)
                {
                    p->timeStamp = timeStamp;
                    p->length = (UInt16) jmin (maxPacketSize, bytesLeft);
                    memcpy (p->data, message.data() + pos, p->length);
                    pos += p->length;
                    bytesLeft -= p->length;
                    p = MIDIPacketNext (p);
                }
            }
            else if (dataSize < 65536) // max packet size
            {
                auto stackCapacity = sizeof (stackPacket.packet->data);

                if (dataSize > stackCapacity)
                {
                    allocatedPackets.malloc ((sizeof (MIDIPacketList) - stackCapacity) + dataSize, 1);
                    packetToSend = allocatedPackets;
                }

                packetToSend->numPackets = 1;
                auto& p = *(packetToSend->packet);
                p.timeStamp = timeStamp;
                p.length = (UInt16) dataSize;
                memcpy (p.data, message.data(), dataSize);
            }
            else
            {
                jassertfalse; // packet too large to send!
                return;
            }

            if (portAndEndpoint.getPort() != 0)
                MIDISend (portAndEndpoint.getPort(), portAndEndpoint.getEndpoint(), packetToSend);
            else
                MIDIReceived (portAndEndpoint.getEndpoint(), packetToSend);
        }

        ump::ToBytestreamConverter converter { 4096 };
    };

    template <>
    class Output<ImplementationStrategy::both> final
    {
    public:
        bool send (const MidiPortAndEndpoint& portAndEndpoint, ump::Iterator b, ump::Iterator e)
        {
            return outputInterface->send (portAndEndpoint, b, e);
        }

    private:
        std::unique_ptr<OutputInterface> outputInterface = std::invoke ([&]() -> std::unique_ptr<OutputInterface>
        {
            if (@available (macOS 11, iOS 14, *))
                return std::make_unique<Output<ImplementationStrategy::onlyNew>>();

            return std::make_unique<Output<ImplementationStrategy::onlyOld>>();
        });
    };
   #endif

    class Connection : private EndpointRemovalListener
    {
    public:
        explicit Connection (std::shared_ptr<SharedEndpointsImplNative> k)
            : knownEndpoints (k)
        {
            jassert (knownEndpoints != nullptr);
            knownEndpoints->addEndpointRemovalListener (*this);
        }

        ~Connection() override
        {
            knownEndpoints->removeEndpointRemovalListener (*this);
        }

        void addDisconnectionListener (ump::DisconnectionListener& l)
        {
            disconnectListeners.add (&l);
        }

        void removeDisconnectionListener (ump::DisconnectionListener& l)
        {
            disconnectListeners.remove (&l);
        }

        std::shared_ptr<SharedEndpointsImplNative> getKnownEndpoints() const
        {
            return knownEndpoints;
        }

        MidiPortAndEndpoint portAndEndpoint;

    private:
        void endpointRemoved (MIDIEndpointRef x) override
        {
            if (x == portAndEndpoint.getEndpoint())
                disconnectListeners.call ([] (auto& c) { c.disconnected(); });
        }

        std::shared_ptr<SharedEndpointsImplNative> knownEndpoints;
        ListenerList<ump::DisconnectionListener> disconnectListeners;
    };

    static int createUniqueIDForMidiPort (String deviceName, ump::IOKind direction)
    {
        String uniqueID;

       #ifdef JucePlugin_CFBundleIdentifier
        uniqueID = JUCE_STRINGIFY (JucePlugin_CFBundleIdentifier);
       #else
        auto appBundle = File::getSpecialLocation (File::currentApplicationFile);
        CFUniquePtr<CFStringRef> appBundlePath (appBundle.getFullPathName().toCFString());

        if (auto bundleURL = CFUniquePtr<CFURLRef> (CFURLCreateWithFileSystemPath (kCFAllocatorDefault,
                                                                                   appBundlePath.get(),
                                                                                   kCFURLPOSIXPathStyle,
                                                                                   true)))
            if (auto bundleRef = CFUniquePtr<CFBundleRef> (CFBundleCreate (kCFAllocatorDefault, bundleURL.get())))
                if (auto bundleId = CFBundleGetIdentifier (bundleRef.get()))
                    uniqueID = String::fromCFString (bundleId);
       #endif

        if (uniqueID.isEmpty())
            uniqueID = String (Random::getSystemRandom().nextInt (1024));

        uniqueID += "." + deviceName + (direction == ump::IOKind::dst ? ".input" : ".output");
        return uniqueID.hashCode();
    }

    class ConnectionToSrc : private ump::Consumer
    {
    public:
        void addConsumer (ump::Consumer& l)
        {
            consumers.add (l);
        }

        void removeConsumer (ump::Consumer& l)
        {
            consumers.remove (l);
        }

        void addDisconnectionListener (ump::DisconnectionListener& l)
        {
            connection.addDisconnectionListener (l);
        }

        void removeDisconnectionListener (ump::DisconnectionListener& l)
        {
            connection.removeDisconnectionListener (l);
        }

        static std::shared_ptr<ConnectionToSrc> make (std::shared_ptr<SharedEndpointsImplNative> cachedEndpoints,
                                                      ump::EndpointId id,
                                                      ump::PacketProtocol protocol)
        {
            const auto endpoint = cachedEndpoints->getNativeEndpointForId (ump::IOKind::src, id);

            if (endpoint == MIDIEndpointRef{})
            {
                jassertfalse;
                return {};
            }

            CFObjectHolder<CFStringRef> cfName;

            if (! JUCE_CHECK_ERROR (MIDIObjectGetStringProperty (endpoint, kMIDIPropertyName, &cfName.object)))
                return {};

            auto receiver = rawToUniquePtr (new ConnectionToSrc (cachedEndpoints));

            MIDIPortRef port;
            const auto error = CreatorFunctionsToUse::createInputPort (protocol,
                                                                       cachedEndpoints->getClient(),
                                                                       cfName.object,
                                                                       &receiver->receiver,
                                                                       &port);

            if (! JUCE_CHECK_ERROR (error))
                return {};

            receiver->connection.portAndEndpoint = { ScopedPortRef { port }, ScopedEndpointRef { endpoint } };
            receiver->connection.portAndEndpoint.start();
            return receiver;
        }

       #if JUCE_COREMIDI_UMP_ENDPOINT_CAN_BE_BUILT
        API_AVAILABLE (macos (15), ios (18))
        static std::shared_ptr<ConnectionToSrc> makeVirtualEndpoint (std::shared_ptr<SharedEndpointsImplNative> cachedEndpoints,
                                                                     const String& deviceName,
                                                                     const ump::DeviceInfo& deviceInfo,
                                                                     const String& deviceIdentity,
                                                                     ump::PacketProtocol protocol)
        {
            std::shared_ptr receiver = rawToUniquePtr (new ConnectionToSrc (cachedEndpoints));
            std::weak_ptr weak = receiver;

            receiver->endpoint.reset ([[MIDIUMPMutableEndpoint alloc] initWithName: juceStringToNS (deviceName)
                                                                        deviceInfo: makeDeviceInfo (deviceInfo).get()
                                                                 productInstanceID: juceStringToNS (deviceIdentity)
                                                                      MIDIProtocol: convertToPacketProtocol (protocol)
                                                               destinationCallback: ^(const MIDIEventList* list, void*)
                                                               {
                                                                   if (auto strong = weak.lock())
                                                                       strong->receiver.pushUmp (list);
                                                               }]);

            if (receiver->endpoint == nullptr)
            {
                jassertfalse;
                return {};
            }

            receiver->connection.portAndEndpoint = { {}, ScopedEndpointRef { receiver->endpoint.get().MIDIDestination } };

            return receiver;
        }

        API_AVAILABLE (macos (15), ios (18))
        MIDIUMPMutableEndpoint* getEndpoint()
        {
            return endpoint.get();
        }
       #endif

       MIDIEndpointRef getMIDIEndpoint() const
       {
            return connection.portAndEndpoint.getEndpoint();
       }

        std::shared_ptr<SharedEndpointsImplNative> getKnownEndpoints() const
        {
            return connection.getKnownEndpoints();
        }

        static std::tuple<std::shared_ptr<ConnectionToSrc>, ump::EndpointId> makeVirtual (std::shared_ptr<SharedEndpointsImplNative> cachedEndpoints,
                                                                                          const String& deviceName)
        {
            const auto direction = ump::IOKind::dst;
            const auto deviceIdentifier = createUniqueIDForMidiPort (deviceName, direction);
            const auto endpointId = ump::EndpointId::make (direction, String (deviceIdentifier));

            const CFUniquePtr<CFStringRef> cfName (deviceName.toCFString());
            std::shared_ptr<ConnectionToSrc> receiver = rawToUniquePtr (new ConnectionToSrc (cachedEndpoints));

            MIDIEndpointRef endpoint{};
            const auto err = CreatorFunctionsToUse::createDestination (ump::PacketProtocol::MIDI_1_0,
                                                                       cachedEndpoints->getClient(),
                                                                       cfName.get(),
                                                                       &receiver->receiver,
                                                                       &endpoint);
            ScopedEndpointRef scopedEndpoint { endpoint };

           #if JUCE_IOS
            if (err == kMIDINotPermitted)
            {
                // If you've hit this assertion then you probably haven't enabled the "Audio Background Capability"
                // setting in the iOS exporter for your app - this is required if you want to create a MIDI device!
                jassertfalse;
                return {};
            }
           #endif

            if (err != noErr)
                return {};

            if (MIDIObjectSetIntegerProperty (*scopedEndpoint, kMIDIPropertyUniqueID, (SInt32) deviceIdentifier) != noErr)
                return {};

            receiver->connection.portAndEndpoint = { {}, std::move (scopedEndpoint) };
            return { receiver, endpointId };
        }

    private:
        explicit ConnectionToSrc (std::shared_ptr<SharedEndpointsImplNative> e)
            : connection (e)
        {
        }

       #if JUCE_COREMIDI_UMP_ENDPOINT_CAN_BE_BUILT
        API_AVAILABLE (macos (15), ios (18))
        static NSUniquePtr<MIDI2DeviceInfo> makeDeviceInfo (const ump::DeviceInfo& x)
        {
            const auto castToByte = [] (auto b) { return Byte (b); };

            MIDI2DeviceManufacturer manufacturer{};
            std::transform (x.manufacturer.begin(), x.manufacturer.end(), manufacturer.sysExIDByte, castToByte);

            MIDI2DeviceRevisionLevel revision{};
            std::transform (x.revision.begin(), x.revision.end(), revision.revisionLevel, castToByte);

            return NSUniquePtr<MIDI2DeviceInfo>
            {
                [[MIDI2DeviceInfo alloc] initWithManufacturerID: manufacturer
                                                         family: bytesToInt14 (x.family)
                                                    modelNumber: bytesToInt14 (x.modelNumber)
                                                  revisionLevel: revision]
            };
        }
       #endif

        void consume (ump::Iterator b, ump::Iterator e, double time) override
        {
            consumers.call ([&] (auto& c) { c.consume (b, e, time); });
        }

        Connection connection;
        WaitFreeListeners<ump::Consumer> consumers;
        ReceiverToUse receiver { *this };

       #if JUCE_COREMIDI_UMP_ENDPOINT_CAN_BE_BUILT
        JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wunguarded-availability-new")
        NSUniquePtr<MIDIUMPMutableEndpoint> endpoint;
        JUCE_END_IGNORE_WARNINGS_GCC_LIKE
       #endif
    };

    class ConnectionToDst
    {
    public:
        void send (ump::Iterator b, ump::Iterator e)
        {
            output.send (connection.portAndEndpoint, b, e);
        }

        void addDisconnectionListener (ump::DisconnectionListener& l)
        {
            connection.addDisconnectionListener (l);
        }

        void removeDisconnectionListener (ump::DisconnectionListener& l)
        {
            connection.removeDisconnectionListener (l);
        }

        MIDIEndpointRef getMIDIEndpoint() const
        {
            return connection.portAndEndpoint.getEndpoint();
        }

        static std::shared_ptr<ConnectionToDst> make (std::shared_ptr<SharedEndpointsImplNative> cachedEndpoints,
                                                      ump::EndpointId id)
        {
            const auto endpoint = cachedEndpoints->getNativeEndpointForId (ump::IOKind::dst, id);

            if (endpoint == MIDIEndpointRef{})
            {
                jassertfalse;
                return {};
            }

            CFObjectHolder<CFStringRef> cfName;

            if (! JUCE_CHECK_ERROR (MIDIObjectGetStringProperty (endpoint, kMIDIPropertyName, &cfName.object)))
                return {};

            MIDIPortRef port;

            if (! JUCE_CHECK_ERROR (MIDIOutputPortCreate (cachedEndpoints->getClient(), cfName.object, &port)))
                return {};

            auto result = rawToUniquePtr (new ConnectionToDst (cachedEndpoints));
            result->connection.portAndEndpoint = { ScopedPortRef { port }, ScopedEndpointRef { endpoint } };
            return result;
        }

        static std::shared_ptr<ConnectionToDst> fromVirtual (std::shared_ptr<SharedEndpointsImplNative> cachedEndpoints,
                                                             ScopedEndpointRef scopedEndpoint)
        {
            std::shared_ptr<ConnectionToDst> sender = rawToUniquePtr (new ConnectionToDst (cachedEndpoints));
            sender->connection.portAndEndpoint = { {}, std::move (scopedEndpoint) };
            return sender;
        }

        static std::tuple<std::shared_ptr<ConnectionToDst>, ump::EndpointId> makeVirtual (std::shared_ptr<SharedEndpointsImplNative> cachedEndpoints,
                                                                                          const String& deviceName)
        {
            const auto direction = ump::IOKind::src;
            const auto deviceIdentifier = createUniqueIDForMidiPort (deviceName, direction);
            const auto endpointId = ump::EndpointId::make (direction, String (deviceIdentifier));

            const CFUniquePtr<CFStringRef> cfName (deviceName.toCFString());

            MIDIEndpointRef endpoint{};
            const auto err = CreatorFunctionsToUse::createSource (ump::PacketProtocol::MIDI_1_0,
                                                                  cachedEndpoints->getClient(),
                                                                  cfName.get(),
                                                                  &endpoint);
            ScopedEndpointRef scopedEndpoint { endpoint };

           #if JUCE_IOS
            if (err == kMIDINotPermitted)
            {
                // If you've hit this assertion then you probably haven't enabled the "Audio Background Capability"
                // setting in the iOS exporter for your app - this is required if you want to create a MIDI device!
                jassertfalse;
                return {};
            }
           #endif

            if (err != noErr)
                return {};

            if (MIDIObjectSetIntegerProperty (*scopedEndpoint, kMIDIPropertyUniqueID, (SInt32) deviceIdentifier) != noErr)
                return {};

            return { fromVirtual (cachedEndpoints, std::move (scopedEndpoint)), endpointId };
        }

        std::shared_ptr<SharedEndpointsImplNative> getKnownEndpoints() const
        {
            return connection.getKnownEndpoints();
        }

    private:
        explicit ConnectionToDst (std::shared_ptr<SharedEndpointsImplNative> e)
            : connection (e)
        {
        }

        Connection connection;
        OutputToUse output;
    };

    class InputImplNative : public ump::Input::Impl::Native,
                            private ump::Consumer
    {
    public:
        InputImplNative (std::shared_ptr<ConnectionToSrc> k,
                         ump::EndpointId i,
                         ump::DisconnectionListener& l,
                         ump::PacketProtocol p,
                         ump::Consumer& cb)
            : converter (p),
              connection (k),
              id (std::move (i)),
              disconnectListener (l),
              consumer (cb)
        {
            connection->addDisconnectionListener (disconnectListener);
            connection->addConsumer (*this);
        }

        ~InputImplNative() override
        {
            connection->removeConsumer (*this);
            connection->removeDisconnectionListener (disconnectListener);
        }

        ump::EndpointId getEndpointId() const override
        {
            return id;
        }

        ump::PacketProtocol getProtocol() const override
        {
            return converter.getProtocol();
        }

    private:
        void consume (ump::Iterator b, ump::Iterator e, double time) override
        {
            for (const auto& view : makeRange (b, e))
            {
                converter.convert (view, [&] (const auto& v)
                {
                    ump::Iterator begin { v.data(), v.size() };
                    consumer.consume (begin, std::next (begin), time);
                });
            }
        }

        ump::GenericUMPConverter converter;
        std::shared_ptr<ConnectionToSrc> connection;
        ump::EndpointId id;
        ump::DisconnectionListener& disconnectListener;
        ump::Consumer& consumer;
    };

    class OutputImplNative final : public ump::Output::Impl::Native
    {
    public:
        OutputImplNative (std::shared_ptr<ConnectionToDst> k,
                          ump::EndpointId i,
                          ump::DisconnectionListener& l)
            : connection (k), id (std::move (i)), disconnectListener (l)
        {
            connection->addDisconnectionListener (disconnectListener);
        }

        ~OutputImplNative() override
        {
            connection->removeDisconnectionListener (disconnectListener);
        }

        bool send (ump::Iterator b, ump::Iterator e) override
        {
            connection->send (b, e);
            return true;
        }

        ump::EndpointId getEndpointId() const override
        {
            return id;
        }

    private:
        std::shared_ptr<ConnectionToDst> connection;
        ump::EndpointId id;
        ump::DisconnectionListener& disconnectListener;
    };

   #if JUCE_COREMIDI_UMP_ENDPOINT_CAN_BE_BUILT
    API_AVAILABLE (macos (15), ios (18))
    static NSUniquePtr<MIDIUMPMutableFunctionBlock> makeBlock (const ump::Block& block)
    {
        const auto direction = std::invoke ([&]
        {
            switch (block.getDirection())
            {
                case ump::BlockDirection::unknown:          return kMIDIUMPFunctionBlockDirectionUnknown;
                case ump::BlockDirection::receiver:         return kMIDIUMPFunctionBlockDirectionInput;
                case ump::BlockDirection::sender:           return kMIDIUMPFunctionBlockDirectionOutput;
                case ump::BlockDirection::bidirectional:    return kMIDIUMPFunctionBlockDirectionBidirectional;
            }

            return kMIDIUMPFunctionBlockDirectionUnknown;
        });

        const auto midi1Kind = std::invoke ([&]
        {
            switch (block.getMIDI1ProxyKind())
            {
                case ump::BlockMIDI1ProxyKind::inapplicable:            return kMIDIUMPFunctionBlockMIDI1InfoNotMIDI1;
                case ump::BlockMIDI1ProxyKind::unrestrictedBandwidth:   return kMIDIUMPFunctionBlockMIDI1InfoUnrestrictedBandwidth;
                case ump::BlockMIDI1ProxyKind::restrictedBandwidth:     return kMIDIUMPFunctionBlockMIDI1InfoRestrictedBandwidth;
            }

            return kMIDIUMPFunctionBlockMIDI1InfoNotMIDI1;
        });

        const auto uiHint = std::invoke ([&]
        {
            switch (block.getUiHint())
            {
                case ump::BlockUiHint::unknown:         return kMIDIUMPFunctionBlockUIHintUnknown;
                case ump::BlockUiHint::receiver:        return kMIDIUMPFunctionBlockUIHintReceiver;
                case ump::BlockUiHint::sender:          return kMIDIUMPFunctionBlockUIHintSender;
                case ump::BlockUiHint::bidirectional:   return kMIDIUMPFunctionBlockUIHintSenderReceiver;
            }

            return kMIDIUMPFunctionBlockUIHintUnknown;
        });

        return NSUniquePtr<MIDIUMPMutableFunctionBlock>
        {
            [[MIDIUMPMutableFunctionBlock alloc] initWithName: juceStringToNS (block.getName())
                                                    direction: direction
                                                   firstGroup: block.getFirstGroup()
                                           totalGroupsSpanned: block.getNumGroups()
                                             maxSysEx8Streams: block.getMaxSysex8Streams()
                                                    MIDI1Info: midi1Kind
                                                       UIHint: uiHint
                                                    isEnabled: block.isEnabled()]
        };
    }

    class API_AVAILABLE (macos (15), ios (18)) VirtualEndpointImplNative : public ump::VirtualEndpoint::Impl::Native
    {
    public:
        ~VirtualEndpointImplNative() override
        {
            auto* endpoint = src->getEndpoint();
            JUCE_CHECK_ERROR ([endpoint setEnabled: false error: nil]);
            src->getKnownEndpoints()->removeVirtualEndpoint (getId());
        }

        ump::EndpointId getId() const override
        {
            auto* endpoint = src->getEndpoint();
            return convertToVirtualId (getIdForNativeEndpoint (endpoint));
        }

        bool setBlock (uint8_t index, const ump::Block& block) override
        {
            auto* endpoint = src->getEndpoint();
            NSUniquePtr<NSMutableArray> blocks { endpoint.mutableFunctionBlocks.mutableCopy };
            [blocks.get() replaceObjectAtIndex: index withObject: makeBlock (block).get()];
            [endpoint setMutableFunctionBlocks: blocks.get()];
            return true;
        }

        bool setName (const String& name) override
        {
            auto* endpoint = src->getEndpoint();
            NSError* error{};
            return [endpoint setName: juceStringToNS (name) error: &error];
        }

        static std::unique_ptr<VirtualEndpointImplNative> make (std::shared_ptr<SharedEndpointsImplNative> cachedEndpoints,
                                                                const String& deviceName,
                                                                const ump::DeviceInfo& deviceInfo,
                                                                const String& deviceIdentity,
                                                                ump::PacketProtocol protocol,
                                                                Span<const ump::Block> blocks,
                                                                ump::BlocksAreStatic areStatic)
        {
            const auto src = ConnectionToSrc::makeVirtualEndpoint (cachedEndpoints,
                                                                   deviceName,
                                                                   deviceInfo,
                                                                   deviceIdentity,
                                                                   protocol);

            if (src == nullptr || src->getEndpoint() == nullptr)
                return {};

            auto* endpoint = src->getEndpoint();

            NSUniquePtr<NSMutableArray> mutableBlocks { [[NSMutableArray alloc] initWithCapacity: blocks.size()] };

            for (const auto& block : blocks)
                if (const auto mutableBlock = makeBlock (block))
                    [mutableBlocks.get() addObject: mutableBlock.get()];

            NSError* error = nullptr;
            auto** errorParam = std::invoke ([&error]() -> NSError**
            {
                // On macOS 15, passing a non-null NSError* will cause the function to fail.
                // This issue is fixed in macOS 26 beta 4.
                if (@available (macOS 26, iOS 26, *))
                    return &error;

                return nullptr;
            });

            const auto successAssigningBlocks = [endpoint registerFunctionBlocks: mutableBlocks.get()
                                                                    markAsStatic: (areStatic == ump::BlocksAreStatic::yes)
                                                                           error: errorParam];

            if (! successAssigningBlocks)
            {
                logNSError (error, __LINE__);

                // Something is wrong with the function block configuration.
                jassertfalse;
                return {};
            }

            const auto successSettingEnabled = [endpoint setEnabled: true error: &error];

            if (! successSettingEnabled)
            {
                logNSError (error, __LINE__);

                // Something went wrong when enabling the endpoint
                jassertfalse;
                return {};
            }

            const auto id = getIdForNativeEndpoint (endpoint);
            const auto dst = ConnectionToDst::fromVirtual (cachedEndpoints, ScopedEndpointRef { endpoint.MIDISource });
            return rawToUniquePtr (new VirtualEndpointImplNative (src, dst));
        }

        std::shared_ptr<ConnectionToSrc> getSrc() const { return src; }
        std::shared_ptr<ConnectionToDst> getDst() const { return dst; }

    private:
        VirtualEndpointImplNative (std::shared_ptr<ConnectionToSrc> s,
                                   std::shared_ptr<ConnectionToDst> d)
            : src (s),
              dst (d)
        {
            jassert (src != nullptr && src->getEndpoint() != nullptr);
            src->getKnownEndpoints()->addVirtualEndpoint (getId(), src->getEndpoint());
        }

        std::shared_ptr<ConnectionToSrc> src;
        std::shared_ptr<ConnectionToDst> dst;
    };
   #endif

    class LegacyVirtualEndpointImplNative : public ump::LegacyVirtualInput::Impl::Native,
                                            public ump::LegacyVirtualOutput::Impl::Native
    {
    public:
        ~LegacyVirtualEndpointImplNative() override
        {
            if (src != nullptr)
                src->getKnownEndpoints()->removeLegacyVirtualPort (getId());

            if (dst != nullptr)
                dst->getKnownEndpoints()->removeLegacyVirtualPort (getId());
        }

        ump::EndpointId getId() const override
        {
            return convertToVirtualId (id);
        }

        static std::unique_ptr<LegacyVirtualEndpointImplNative> make (std::shared_ptr<ConnectionToSrc> x,
                                                                      ump::EndpointId i)
        {
            return rawToUniquePtr (new LegacyVirtualEndpointImplNative (std::move (x), std::move (i)));
        }

        static std::unique_ptr<LegacyVirtualEndpointImplNative> make (std::shared_ptr<ConnectionToDst> x,
                                                                      ump::EndpointId i)
        {
            return rawToUniquePtr (new LegacyVirtualEndpointImplNative (std::move (x), std::move (i)));
        }

    private:
        LegacyVirtualEndpointImplNative (std::shared_ptr<ConnectionToSrc> x, ump::EndpointId i)
            : src (std::move (x)), id (i)
        {
            const auto port = getMidiObjectInfo (src->getMIDIEndpoint());
            auto endpoint = ump::IOHelpers::makeProxyEndpoint (port, ump::BlockDirection::receiver);
            src->getKnownEndpoints()->addLegacyVirtualPort (getId(),
                                                            EndpointInfo::withSrc (endpoint.endpoint,
                                                                                   src->getMIDIEndpoint(),
                                                                                   std::array<String, 16>{}));
        }

        LegacyVirtualEndpointImplNative (std::shared_ptr<ConnectionToDst> x, ump::EndpointId i)
            : dst (std::move (x)), id (i)
        {
            const auto port = getMidiObjectInfo (dst->getMIDIEndpoint());
            auto endpoint = ump::IOHelpers::makeProxyEndpoint (port, ump::BlockDirection::sender);
            dst->getKnownEndpoints()->addLegacyVirtualPort (getId(),
                                                            EndpointInfo::withDst (endpoint.endpoint,
                                                                                   dst->getMIDIEndpoint(),
                                                                                   std::array<String, 16>{}));
        }

        std::shared_ptr<ConnectionToSrc> src;
        std::shared_ptr<ConnectionToDst> dst;
        ump::EndpointId id;
    };

    class SessionImplNative : public ump::Session::Impl::Native
    {
    public:
        String getName() const override
        {
            return name;
        }

        std::unique_ptr<ump::Input::Impl::Native> connectInput (ump::DisconnectionListener& l,
                                                                const ump::EndpointId& id,
                                                                ump::PacketProtocol p,
                                                                ump::Consumer& c) override
        {
            if (auto src = findOrMakeSrc (id, p))
                return rawToUniquePtr (new InputImplNative { src, id, l, p, c });

            return {};
        }

        std::unique_ptr<ump::Output::Impl::Native> connectOutput (ump::DisconnectionListener& l,
                                                                  const ump::EndpointId& id) override
        {
            if (auto dst = findOrMakeDst (id))
                return rawToUniquePtr (new OutputImplNative { dst, id, l });

            return {};
        }

        std::unique_ptr<ump::VirtualEndpoint::Impl::Native> createNativeVirtualEndpoint ([[maybe_unused]] const String& deviceName,
                                                                                         [[maybe_unused]] const ump::DeviceInfo& deviceInfo,
                                                                                         [[maybe_unused]] const String& deviceIdentity,
                                                                                         [[maybe_unused]] ump::PacketProtocol protocol,
                                                                                         [[maybe_unused]] Span<const ump::Block> blocks,
                                                                                         [[maybe_unused]] ump::BlocksAreStatic areStatic) override
        {
           #if JUCE_COREMIDI_UMP_ENDPOINT_CAN_BE_BUILT
            if (@available (macOS 15, iOS 18, *))
            {
                auto connection = VirtualEndpointImplNative::make (cachedEndpoints,
                                                                   deviceName,
                                                                   deviceInfo,
                                                                   deviceIdentity,
                                                                   protocol,
                                                                   blocks,
                                                                   areStatic);

                if (connection != nullptr)
                {
                    connectionsSrc[{ connection->getId(), ump::PacketProtocol::MIDI_1_0 }] = connection->getSrc();
                    connectionsSrc[{ connection->getId(), ump::PacketProtocol::MIDI_2_0 }] = connection->getSrc();
                    connectionsDst[connection->getId()] = connection->getDst();
                }

                return connection;
            }
           #endif

            return {};
        }

        std::unique_ptr<ump::LegacyVirtualInput::Impl::Native> createLegacyVirtualInput (const String& deviceName) override
        {
            const auto [connection, id] = ConnectionToSrc::makeVirtual (cachedEndpoints, deviceName);

            if (connection == nullptr)
                return {};

            auto result = LegacyVirtualEndpointImplNative::make (connection, id);
            connectionsSrc[{ result->getId(), ump::PacketProtocol::MIDI_1_0 }] = connection;
            return result;
        }

        std::unique_ptr<ump::LegacyVirtualOutput::Impl::Native> createLegacyVirtualOutput (const String& deviceName) override
        {
            const auto [connection, id] = ConnectionToDst::makeVirtual (cachedEndpoints, deviceName);

            if (connection == nullptr)
                return {};

            auto result = LegacyVirtualEndpointImplNative::make (connection, id);
            connectionsDst[result->getId()] = connection;
            return result;
        }

        static std::unique_ptr<SessionImplNative> make (std::shared_ptr<SharedEndpointsImplNative> c, const String& n)
        {
            return rawToUniquePtr (new SessionImplNative (c, n));
        }

    private:
        SessionImplNative (std::shared_ptr<SharedEndpointsImplNative> c, const String& n)
            : cachedEndpoints (c), name (n) {}

        std::shared_ptr<ConnectionToSrc> findOrMakeSrc (const ump::EndpointId& id, ump::PacketProtocol protocol)
        {
            auto& weak = connectionsSrc[{ id, protocol }];

            if (auto strong = weak.lock())
                return strong;

            const auto strong = ConnectionToSrc::make (cachedEndpoints, id, protocol);
            weak = strong;
            return strong;
        }

        std::shared_ptr<ConnectionToDst> findOrMakeDst (const ump::EndpointId& id)
        {
            auto& weak = connectionsDst[id];

            if (auto strong = weak.lock())
                return strong;

            const auto strong = ConnectionToDst::make (cachedEndpoints, id);
            weak = strong;
            return strong;
        }

        struct SrcKey
        {
            ump::EndpointId id;
            ump::PacketProtocol protocol;

            auto tie() const { return std::tie (id, protocol); }
            auto operator< (const SrcKey& other) const { return tie() < other.tie(); }
        };

        std::map<SrcKey, std::weak_ptr<ConnectionToSrc>> connectionsSrc;
        std::map<ump::EndpointId, std::weak_ptr<ConnectionToDst>> connectionsDst;

        std::shared_ptr<SharedEndpointsImplNative> cachedEndpoints;
        String name;
    };

    class EndpointsImplNative : public ump::Endpoints::Impl::Native
    {
    public:
        ump::Backend getBackend() const override
        {
            return ump::Backend::coremidi;
        }

        bool isVirtualMidiBytestreamServiceActive() const override
        {
            return true;
        }

        bool isVirtualMidiUmpServiceActive() const override
        {
            if (@available (macOS 15, iOS 18, *))
                return true;

            return false;
        }

        void setVirtualMidiBytestreamServiceActive (bool) override {}
        void setVirtualMidiUmpServiceActive (bool) override {}

        void getEndpoints (std::vector<ump::EndpointId>& x) const override
        {
            strong->getEndpoints (x);
        }

        std::optional<ump::Endpoint> getEndpoint (const ump::EndpointId& x) const override
        {
            return strong->getEndpoint (x);
        }

        std::optional<ump::StaticDeviceInfo> getStaticDeviceInfo (const ump::EndpointId& x) const override
        {
            return strong->getStaticDeviceInfo (x);
        }

        std::unique_ptr<ump::Session::Impl::Native> makeSession (const String& x) override
        {
            return SessionImplNative::make (strong, x);
        }

        static std::unique_ptr<EndpointsImplNative> make (ump::EndpointsListener& listener)
        {
            if (auto inner = SharedEndpointsImplNative::make (listener))
                return rawToUniquePtr (new EndpointsImplNative (inner));

            return {};
        }

    private:
        explicit EndpointsImplNative (std::shared_ptr<SharedEndpointsImplNative> s)
            : strong (s) {}

        std::shared_ptr<SharedEndpointsImplNative> strong;
    };

    CoreMidiHelpers() = delete;

    #pragma pop_macro ("JUCE_CHECK_ERROR")
};

auto ump::Endpoints::Impl::Native::make (EndpointsListener& l) -> std::unique_ptr<Native>
{
    return CoreMidiHelpers::EndpointsImplNative::make (l);
}

} // namespace juce

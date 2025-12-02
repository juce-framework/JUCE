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

#ifndef DRV_QUERYDEVICEINTERFACE
 #define DRV_RESERVED                  0x0800
 #define DRV_QUERYDEVICEINTERFACE     (DRV_RESERVED + 12)
 #define DRV_QUERYDEVICEINTERFACESIZE (DRV_RESERVED + 13)
#endif

namespace juce
{

#if JUCE_USE_WINDOWS_MIDI_SERVICES

namespace wm2 = winrt::Microsoft::Windows::Devices::Midi2;
namespace wm2v = wm2::Endpoints::Virtual;
namespace mwdmi = Microsoft::Windows::Devices::Midi2::Initialization;

class MidiServices
{
public:
    [[nodiscard]] static std::unique_ptr<ump::Endpoints::Impl::Native> makeEndpoints (ump::EndpointsListener& l)
    {
        return EndpointsImplNative::make (l);
    }

private:
    /*  For both input and output.
        It's most resource-efficient to have only one connection to each endpoint.
        Therefore, we keep track of endpoints we've opened, and share the endpoints between
        Input and Output instances when possible.
    */
    class SharedConnection : private AsyncUpdater
    {
    public:
        template <typename... Args>
        static std::unique_ptr<SharedConnection> make (const wm2::MidiSession& session,
                                                       const winrt::param::hstring& id,
                                                       Args&&... args)
        {
            auto connection = session.CreateEndpointConnection (id);

            if (connection == nullptr)
                return {};

            setUpConnection (connection, args...);

            auto result = rawToUniquePtr (new SharedConnection (session, std::move (connection)));
            result->inputToken = result->connection.MessageReceived ([self = result.get()] (const auto&, const wm2::MidiMessageReceivedEventArgs& args)
            {
                std::array<uint32_t, 4> words{};
                args.FillWordArray (0, words);

                const ump::Iterator begin { words.data(), words.size() };
                const auto end = std::next (begin);

                const auto elapsedTime = args.Timestamp() - self->startTimeNative;
                const auto juceTimeMillis = self->startTimeMillis + wm2::MidiClock::ConvertTimestampTicksToMilliseconds (elapsedTime);

                self->consumers.call ([&] (auto& c) { c.consume (begin, end, juceTimeMillis * 0.001); });
            });

            result->disconnectToken = result->connection.EndpointDeviceDisconnected ([self = result.get()] (auto&&...)
            {
                self->triggerAsyncUpdate();
            });

            if (! result->connection.Open())
                return {};

            return result;
        }

        ~SharedConnection()
        {
            connection.MessageReceived (inputToken);
            connection.EndpointDeviceDisconnected (disconnectToken);

            cancelPendingUpdate();

            if (session != nullptr && connection != nullptr)
                session.DisconnectEndpointConnection (connection.ConnectionId());
        }

        ump::EndpointId getEndpointId() const
        {
            const auto id = toString (connection.ConnectedEndpointDeviceId());
            return ump::EndpointId::makeSrcDst (id, id);
        }

        void addConsumer (ump::Consumer& c)
        {
            consumers.add (c);
        }

        void removeConsumer (ump::Consumer& c)
        {
            consumers.remove (c);
        }

        void addDisconnectListener (ump::DisconnectionListener& l)
        {
            disconnectListeners.add (&l);
        }

        void removeDisconnectListener (ump::DisconnectionListener& l)
        {
            disconnectListeners.remove (&l);
        }

        bool send (ump::Iterator b, ump::Iterator e)
        {
            const auto result = connection.SendMultipleMessagesWordArray (0,
                                                                          0,
                                                                          (uint32_t) std::distance (b->data(), e->data()),
                                                                          { b->data(), e->data() });
            return wm2::MidiSendMessageResults::Succeeded == result;
        }

    private:
        SharedConnection (wm2::MidiSession s, wm2::MidiEndpointConnection c)
            : session (std::move (s)), connection (std::move (c)) {}

        void handleAsyncUpdate() override
        {
             disconnectListeners.call ([] (auto& c) { c.disconnected(); });
        }

        static void setUpConnection (const wm2::MidiEndpointConnection&)
        {
            // Do nothing
        }

        static void setUpConnection (const wm2::MidiEndpointConnection& connection,
                                     const wm2::IMidiEndpointMessageProcessingPlugin& plugin)
        {
            connection.AddMessageProcessingPlugin (plugin);
        }

        const uint64_t startTimeNative = wm2::MidiClock::Now();
        const uint32_t startTimeMillis = Time::getMillisecondCounter();

        wm2::MidiSession session;
        wm2::MidiEndpointConnection connection;
        WaitFreeListeners<ump::Consumer> consumers;
        ListenerList<ump::DisconnectionListener> disconnectListeners;
        winrt::event_token inputToken, disconnectToken;
    };

    class InputImplNative : public ump::Input::Impl::Native,
                            private ump::Consumer
    {
    public:
        ~InputImplNative() override
        {

            shared->removeDisconnectListener (listener);
            shared->removeConsumer (*this);
        }

        ump::EndpointId getEndpointId() const override
        {
            return shared->getEndpointId();
        }

        ump::PacketProtocol getProtocol() const override
        {
            return converter.getProtocol();
        }

        static std::unique_ptr<InputImplNative> make (std::shared_ptr<SharedConnection> x,
                                                      ump::DisconnectionListener& l,
                                                      ump::PacketProtocol protocol,
                                                      Consumer& consumer)
        {
            if (x == nullptr)
                return {};

            return rawToUniquePtr (new InputImplNative { x, l, protocol, consumer });
        }

    private:
        InputImplNative (std::shared_ptr<SharedConnection> s,
                         ump::DisconnectionListener& l,
                         ump::PacketProtocol p,
                         Consumer& c)
            : listener (l),
              consumer (c),
              shared (s),
              converter (p)
        {
            shared->addConsumer (*this);
            shared->addDisconnectListener (listener);
        }

        void consume (ump::Iterator b, ump::Iterator e, double time) override
        {
            converter.convert (b, e, [&] (ump::View v)
            {
                const ump::Iterator b { v.data(), v.size() };
                consumer.consume (b, std::next (b), time);
            });
        }

        ump::DisconnectionListener& listener;
        Consumer& consumer;
        std::shared_ptr<SharedConnection> shared;
        ump::GenericUMPConverter converter;
    };

    class OutputImplNative : public ump::Output::Impl::Native
    {
    public:
        ~OutputImplNative() override
        {
            shared->removeDisconnectListener (listener);
        }

        ump::EndpointId getEndpointId() const override
        {
            return shared->getEndpointId();
        }

        bool send (ump::Iterator b, ump::Iterator e) override
        {
            return shared->send (b, e);
        }

        static std::unique_ptr<OutputImplNative> make (std::shared_ptr<SharedConnection> c,
                                                       ump::DisconnectionListener& l)
        {
            if (c == nullptr)
                return {};

            return rawToUniquePtr (new OutputImplNative { std::move (c), l });
        }

    private:
        OutputImplNative (std::shared_ptr<SharedConnection> c, ump::DisconnectionListener& l)
            : listener (l), shared (std::move (c))
        {
            shared->addDisconnectListener (listener);
        }

        ump::DisconnectionListener& listener;
        std::shared_ptr<SharedConnection> shared;
    };

    class VirtualEndpoint
    {
    public:
        ump::EndpointId getId() const
        {
            const auto id = toString (device.DeviceEndpointDeviceId());
            return ump::EndpointId::makeSrcDst (id, id);
        }

        ump::Endpoint getEndpoint() const
        {
            return endpoint;
        }

        ump::StaticDeviceInfo getStaticDeviceInfo() const
        {
            return staticInfo;
        }

        bool setBlock (uint8_t i, const ump::Block& b)
        {
            if (! device.UpdateFunctionBlock (makeBlock (i, b)))
                return false;

            endpoint.getBlocks()[i] = b;
            return true;
        }

        bool setName (const String& x)
        {
            if (! device.UpdateEndpointName (x.toWideCharPointer()))
                return false;

            endpoint = endpoint.withName (x);
            return true;
        }

        static std::unique_ptr<VirtualEndpoint> make (std::shared_ptr<SharedConnection> c,
                                                      wm2v::MidiVirtualDevice d,
                                                      ump::Endpoint ep,
                                                      ump::StaticDeviceInfo si)
        {
            if (d == nullptr)
                return {};

            return rawToUniquePtr (new VirtualEndpoint { std::move (c),
                                                         std::move (d),
                                                         std::move (ep),
                                                         std::move (si) });
        }

    private:
        VirtualEndpoint (std::shared_ptr<SharedConnection> s,
                         wm2v::MidiVirtualDevice d,
                         ump::Endpoint ep,
                         ump::StaticDeviceInfo si)
            : shared (std::move (s)),
              device (std::move (d)),
              endpoint (ep),
              staticInfo (std::move (si))
        {
        }

        std::shared_ptr<SharedConnection> shared;
        wm2v::MidiVirtualDevice device;
        ump::Endpoint endpoint;
        ump::StaticDeviceInfo staticInfo;
    };

    struct VirtualEndpointRegistry
    {
        virtual ~VirtualEndpointRegistry() = default;
        virtual void virtualEndpointAdded (std::shared_ptr<VirtualEndpoint>) = 0;
    };

    class VirtualEndpointImplNative : public ump::VirtualEndpoint::Impl::Native,
                                      public ump::LegacyVirtualInput::Impl::Native,
                                      public ump::LegacyVirtualOutput::Impl::Native
    {
    public:
        ump::EndpointId getId() const override
        {
            return endpoint->getId();
        }

        bool setBlock (uint8_t i, const ump::Block& b) override
        {
            return endpoint->setBlock (i, b);
        }

        bool setName (const String& x) override
        {
            return endpoint->setName (x);
        }

        static std::unique_ptr<VirtualEndpointImplNative> make (std::shared_ptr<VirtualEndpoint> ep)
        {
            if (ep == nullptr)
                return {};

            return rawToUniquePtr (new VirtualEndpointImplNative { std::move (ep) });
        }

    private:
        explicit VirtualEndpointImplNative (std::shared_ptr<VirtualEndpoint> ep)
            : endpoint (std::move (ep)) {}

        std::shared_ptr<VirtualEndpoint> endpoint;
    };

    class SessionImplNative : public ump::Session::Impl::Native
    {
    public:
        ~SessionImplNative() override
        {
            session.Close();
        }

        String getName() const override
        {
            return toString (session.Name());
        }

        std::unique_ptr<ump::Input::Impl::Native> connectInput (ump::DisconnectionListener& listener,
                                                                const ump::EndpointId& id,
                                                                ump::PacketProtocol p,
                                                                ump::Consumer& consumer) override
        {
            const auto strong = findOrOpenConnection (id.src.toWideCharPointer());
            return InputImplNative::make (strong, listener, p, consumer);
        }

        std::unique_ptr<ump::Output::Impl::Native> connectOutput (ump::DisconnectionListener& listener,
                                                                  const ump::EndpointId& id) override
        {
            const auto strong = findOrOpenConnection (id.dst.toWideCharPointer());
            return OutputImplNative::make (strong, listener);
        }

        std::unique_ptr<ump::VirtualEndpoint::Impl::Native> createNativeVirtualEndpoint (const String& name,
                                                                                         const ump::DeviceInfo& info,
                                                                                         const String& productInstance,
                                                                                         ump::PacketProtocol protocol,
                                                                                         Span<const ump::Block> blocks,
                                                                                         ump::BlocksAreStatic areStatic) override
        {
            return createNativeVirtualEndpointImpl (name, info, productInstance, protocol, blocks, areStatic);
        }

        std::unique_ptr<ump::LegacyVirtualInput::Impl::Native> createLegacyVirtualInput (const String& name) override
        {
            const ump::Block blocks[] { ump::IOHelpers::makeLegacyBlock (true) };
            return createNativeVirtualEndpointImpl (name, {}, {}, ump::PacketProtocol::MIDI_1_0, blocks, ump::BlocksAreStatic::yes);
        }

        std::unique_ptr<ump::LegacyVirtualOutput::Impl::Native> createLegacyVirtualOutput (const String& name) override
        {
            const ump::Block blocks[] { ump::IOHelpers::makeLegacyBlock (false) };
            return createNativeVirtualEndpointImpl (name, {}, {}, ump::PacketProtocol::MIDI_1_0, blocks, ump::BlocksAreStatic::yes);
        }

        static std::unique_ptr<SessionImplNative> make (VirtualEndpointRegistry& r, const String& name)
        {
            if (auto s = wm2::MidiSession::Create (name.toWideCharPointer()))
                return rawToUniquePtr (new SessionImplNative (r, s));

            return {};
        }

    private:
        SessionImplNative (VirtualEndpointRegistry& r, const wm2::MidiSession& s)
            : registry (r), session (s) {}

        std::unique_ptr<VirtualEndpointImplNative> createNativeVirtualEndpointImpl (const String& name,
                                                                                    const ump::DeviceInfo& info,
                                                                                    const String& productInstance,
                                                                                    ump::PacketProtocol protocol,
                                                                                    Span<const ump::Block> blocks,
                                                                                    ump::BlocksAreStatic areStatic)
        {
            wm2::MidiDeclaredEndpointInfo e;
            e.Name = name.toWideCharPointer();
            e.HasStaticFunctionBlocks = areStatic == ump::BlocksAreStatic::yes;
            e.DeclaredFunctionBlockCount = (uint8_t) blocks.size();
            e.ProductInstanceId = productInstance.toWideCharPointer();
            e.SupportsMidi10Protocol = protocol == ump::PacketProtocol::MIDI_1_0;
            e.SupportsMidi20Protocol = protocol == ump::PacketProtocol::MIDI_2_0;
            e.SpecificationVersionMajor = 1;
            e.SpecificationVersionMinor = 1;
            e.SupportsReceivingJitterReductionTimestamps = false;
            e.SupportsSendingJitterReductionTimestamps = false;

            wm2v::MidiVirtualDeviceCreationConfig config { e.Name,
                                                           ump::Endpoints::Impl::getGlobalMidiClientName().toWideCharPointer(),
                                                           L"",
                                                           e,
                                                           makeDeviceInfo (info) };

            for (const auto [index, value] : enumerate (blocks, uint8_t{}))
                config.FunctionBlocks().Append (makeBlock (index, value));

            auto device = wm2v::MidiVirtualDeviceManager::CreateVirtualDevice (config);

            if (device == nullptr)
                return {};

            // In order to function, the device needs a client plugin installed, which in turn
            // requires opening a connection to the endpoint.
            auto connection = findOrOpenConnection (device.DeviceEndpointDeviceId(), device);

            if (connection == nullptr)
                return {};

            auto endpoint = ump::Endpoint{}.withName (name)
                                           .withDeviceInfo (info)
                                           .withProductInstanceId (productInstance)
                                           .withProtocol (protocol)
                                           .withMidi1Support (protocol == ump::PacketProtocol::MIDI_1_0)
                                           .withMidi2Support (protocol == ump::PacketProtocol::MIDI_2_0)
                                           .withStaticBlocks (areStatic == ump::BlocksAreStatic::yes)
                                           .withBlocks (blocks);

            auto staticInfo = ump::StaticDeviceInfo{}.withName (name)
                                                     .withManufacturer ("")
                                                     .withProduct ("")
                                                     .withTransport (ump::Transport::ump)
                                                     .withHasSource (true)
                                                     .withHasDestination (true);

            std::shared_ptr virtualEndpoint = VirtualEndpoint::make (std::move (connection),
                                                                     std::move (device),
                                                                     endpoint,
                                                                     staticInfo);

            if (virtualEndpoint == nullptr)
                return {};

            registry.virtualEndpointAdded (virtualEndpoint);

            return VirtualEndpointImplNative::make (std::move (virtualEndpoint));
        }

        template <typename... Args>
        std::shared_ptr<SharedConnection> findOrOpenConnection (const winrt::hstring& id,
                                                                Args&&... args)
        {
            auto& weak = weakConnections[toString (id)];

            if (const auto strong = weak.lock())
                return strong;

            const std::shared_ptr strong = SharedConnection::make (session, id, args...);
            weak = strong;
            return strong;
        }

        VirtualEndpointRegistry& registry;
        std::map<String, std::weak_ptr<SharedConnection>> weakConnections;
        wm2::MidiSession session;
    };

    static wm2::MidiFunctionBlock makeBlock (uint8_t index, const ump::Block& b)
    {
        const auto direction = std::invoke ([&]
        {
            switch (b.getDirection())
            {
                case ump::BlockDirection::bidirectional: return wm2::MidiFunctionBlockDirection::Bidirectional;
                case ump::BlockDirection::sender:        return wm2::MidiFunctionBlockDirection::BlockOutput;
                case ump::BlockDirection::receiver:      return wm2::MidiFunctionBlockDirection::BlockInput;
                case ump::BlockDirection::unknown:       return wm2::MidiFunctionBlockDirection::Undefined;
            }

            return wm2::MidiFunctionBlockDirection{};
        });

        const auto hint = std::invoke ([&]
        {
            switch (b.getUiHint())
            {
                case ump::BlockUiHint::bidirectional:   return wm2::MidiFunctionBlockUIHint::Bidirectional;
                case ump::BlockUiHint::sender:          return wm2::MidiFunctionBlockUIHint::Sender;
                case ump::BlockUiHint::receiver:        return wm2::MidiFunctionBlockUIHint::Receiver;
                case ump::BlockUiHint::unknown:         return wm2::MidiFunctionBlockUIHint::Unknown;
            }

            return wm2::MidiFunctionBlockUIHint{};
        });

        const auto proxy = std::invoke ([&]
        {
            switch (b.getMIDI1ProxyKind())
            {
                case ump::BlockMIDI1ProxyKind::inapplicable:                return wm2::MidiFunctionBlockRepresentsMidi10Connection::Not10;
                case ump::BlockMIDI1ProxyKind::restrictedBandwidth:         return wm2::MidiFunctionBlockRepresentsMidi10Connection::YesBandwidthRestricted;
                case ump::BlockMIDI1ProxyKind::unrestrictedBandwidth:       return wm2::MidiFunctionBlockRepresentsMidi10Connection::YesBandwidthUnrestricted;
            }

            return wm2::MidiFunctionBlockRepresentsMidi10Connection{};
        });

        wm2::MidiFunctionBlock result;
        result.Name (b.getName().toWideCharPointer());
        result.Number (index);
        result.IsActive (b.isEnabled());
        result.FirstGroup (wm2::MidiGroup { b.getFirstGroup() });
        result.GroupCount (b.getNumGroups());
        result.MaxSystemExclusive8Streams (b.getMaxSysex8Streams());
        result.Direction (direction);
        result.UIHint (hint);
        result.RepresentsMidi10Connection (proxy);

        return result;
    }

    struct IndexedBlock
    {
        uint8_t index;
        ump::Block block;
    };

    static IndexedBlock makeBlock (const wm2::MidiFunctionBlock& b)
    {
        const auto index = b.Number();

        const auto direction = std::invoke ([&]
        {
            switch (b.Direction())
            {
                case wm2::MidiFunctionBlockDirection::Bidirectional: return ump::BlockDirection::bidirectional;
                case wm2::MidiFunctionBlockDirection::BlockOutput:   return ump::BlockDirection::sender;
                case wm2::MidiFunctionBlockDirection::BlockInput:    return ump::BlockDirection::receiver;
                case wm2::MidiFunctionBlockDirection::Undefined:     return ump::BlockDirection::unknown;
            }

            return ump::BlockDirection{};
        });

        const auto hint = std::invoke ([&]
        {
            switch (b.UIHint())
            {
                case wm2::MidiFunctionBlockUIHint::Bidirectional:   return ump::BlockUiHint::bidirectional;
                case wm2::MidiFunctionBlockUIHint::Sender:          return ump::BlockUiHint::sender;
                case wm2::MidiFunctionBlockUIHint::Receiver:        return ump::BlockUiHint::receiver;
                case wm2::MidiFunctionBlockUIHint::Unknown:         return ump::BlockUiHint::unknown;
            }

            return ump::BlockUiHint{};
        });

        const auto proxy = std::invoke ([&]
        {
            switch (b.RepresentsMidi10Connection())
            {
                case wm2::MidiFunctionBlockRepresentsMidi10Connection::Not10:                    return ump::BlockMIDI1ProxyKind::inapplicable;
                case wm2::MidiFunctionBlockRepresentsMidi10Connection::YesBandwidthRestricted:   return ump::BlockMIDI1ProxyKind::restrictedBandwidth;
                case wm2::MidiFunctionBlockRepresentsMidi10Connection::YesBandwidthUnrestricted: return ump::BlockMIDI1ProxyKind::unrestrictedBandwidth;
                case wm2::MidiFunctionBlockRepresentsMidi10Connection::Reserved:                 break;
            }

            return ump::BlockMIDI1ProxyKind{};
        });

        const auto block = ump::Block{}.withDirection (direction)
                                       .withUiHint (hint)
                                       .withMIDI1ProxyKind (proxy)
                                       .withFirstGroup (b.FirstGroup().Index())
                                       .withNumGroups (b.GroupCount())
                                       .withEnabled (b.IsActive())
                                       .withName (toString (b.Name()))
                                       .withMaxSysex8Streams (b.MaxSystemExclusive8Streams());

        return { index, block };
    }

    static wm2::MidiDeclaredDeviceIdentity makeDeviceInfo (ump::DeviceInfo x)
    {
        wm2::MidiDeclaredDeviceIdentity result{};

        result.SystemExclusiveIdByte1 = (uint8_t) x.manufacturer[0];
        result.SystemExclusiveIdByte2 = (uint8_t) x.manufacturer[1];
        result.SystemExclusiveIdByte3 = (uint8_t) x.manufacturer[2];

        result.DeviceFamilyLsb = (uint8_t) x.family[0];
        result.DeviceFamilyMsb = (uint8_t) x.family[1];

        result.DeviceFamilyModelNumberLsb = (uint8_t) x.modelNumber[0];
        result.DeviceFamilyModelNumberMsb = (uint8_t) x.modelNumber[1];

        result.SoftwareRevisionLevelByte1 = (uint8_t) x.revision[0];
        result.SoftwareRevisionLevelByte2 = (uint8_t) x.revision[1];
        result.SoftwareRevisionLevelByte3 = (uint8_t) x.revision[2];
        result.SoftwareRevisionLevelByte4 = (uint8_t) x.revision[3];

        return result;
    }

    static ump::DeviceInfo makeDeviceInfo (const wm2::MidiDeclaredDeviceIdentity& x)
    {
        return ump::DeviceInfo
        {
            { std::byte (x.SystemExclusiveIdByte1),
              std::byte (x.SystemExclusiveIdByte2),
              std::byte (x.SystemExclusiveIdByte3) },

            { std::byte (x.DeviceFamilyLsb),
              std::byte (x.DeviceFamilyMsb) },

            { std::byte (x.DeviceFamilyModelNumberLsb),
              std::byte (x.DeviceFamilyModelNumberMsb) },

            { std::byte (x.SoftwareRevisionLevelByte1),
              std::byte (x.SoftwareRevisionLevelByte2),
              std::byte (x.SoftwareRevisionLevelByte3),
              std::byte (x.SoftwareRevisionLevelByte4) },
        };
    }

    class EndpointsImplNative : public ump::Endpoints::Impl::Native,
                                private VirtualEndpointRegistry,
                                private AsyncUpdater
    {
    public:
        ~EndpointsImplNative() override
        {
            watcher.Stop();
            cancelPendingUpdate();
        }

        ump::Backend getBackend() const
        {
            return ump::Backend::wms;
        }

        bool isVirtualMidiUmpServiceActive() const override
        {
            return wm2v::MidiVirtualDeviceManager::IsTransportAvailable();
        }

        bool isVirtualMidiBytestreamServiceActive() const override
        {
            return wm2v::MidiVirtualDeviceManager::IsTransportAvailable();
        }

        void getEndpoints (std::vector<ump::EndpointId>& buffer) const override
        {
            std::transform (cachedEndpoints.begin(),
                            cachedEndpoints.end(),
                            std::back_inserter (buffer),
                            [] (auto& pair) { return pair.first; });
        }

        std::optional<ump::Endpoint> getEndpoint (const ump::EndpointId& id) const override
        {
            if (const auto iter = virtualEndpoints.find (id); iter != virtualEndpoints.end())
                if (auto strong = iter->second.lock())
                    return strong->getEndpoint();

            if (const auto iter = cachedEndpoints.find (id); iter != cachedEndpoints.end())
                return iter->second.endpoint;

            return {};
        }

        std::optional<ump::StaticDeviceInfo> getStaticDeviceInfo (const ump::EndpointId& id) const override
        {
            if (const auto iter = virtualEndpoints.find (id); iter != virtualEndpoints.end())
                if (auto strong = iter->second.lock())
                    return strong->getStaticDeviceInfo();

            if (const auto iter = cachedEndpoints.find (id); iter != cachedEndpoints.end())
                return iter->second.info;

            return {};
        }

        std::unique_ptr<ump::Session::Impl::Native> makeSession (const String& name) override
        {
            return SessionImplNative::make (*this, name);
        }

        static std::unique_ptr<EndpointsImplNative> make (ump::EndpointsListener& l)
        {
            SharedResourcePointer<SdkInitialiser> initialiser;

            if (! initialiser->isValid())
            {
                // If you hit this, you've tried to initialise Windows MIDI Services but the
                // initialisation failed. Did you forget to install the Windows MIDI Services SDK?
                jassertfalse;
                return {};
            }

            auto watcher = wm2::MidiEndpointDeviceWatcher::Create();

            if (! watcher)
                return {};

            return rawToUniquePtr (new EndpointsImplNative { watcher, l });
        }

    private:
        EndpointsImplNative (wm2::MidiEndpointDeviceWatcher w, ump::EndpointsListener& l)
            : listener (l), watcher (w)
        {
            watcher.Added ([this] (auto&, const wm2::MidiEndpointDeviceInformationAddedEventArgs& args)
            {
                const auto device = args.AddedDevice();
                const auto id = toString (device.EndpointDeviceId());
                const auto endpoint = makeEndpoint (device);

                const std::scoped_lock lock { mutex };
                pendingWork.push_back ([this, id, endpoint]
                {
                    cachedEndpoints.insert_or_assign (ump::EndpointId::makeSrcDst (id, id), endpoint);
                });

                triggerAsyncUpdate();
            });

            watcher.Updated ([this] (auto&, const wm2::MidiEndpointDeviceInformationUpdatedEventArgs& args)
            {
                const auto id = toString (args.EndpointDeviceId());

                if (const auto info = wm2::MidiEndpointDeviceInformation::CreateFromEndpointDeviceId (args.EndpointDeviceId()))
                {
                    const auto endpoint = makeEndpoint (info);

                    const std::scoped_lock lock { mutex };
                    pendingWork.push_back ([this, id, endpoint]
                    {
                        cachedEndpoints.insert_or_assign (ump::EndpointId::makeSrcDst (id, id), endpoint);
                    });

                    triggerAsyncUpdate();
                }
            });

            watcher.Removed ([this] (auto&, const wm2::MidiEndpointDeviceInformationRemovedEventArgs& args)
            {
                const auto id = toString (args.EndpointDeviceId());

                const std::scoped_lock lock { mutex };
                pendingWork.push_back ([this, id]
                {
                    cachedEndpoints.erase (ump::EndpointId::makeSrcDst (id, id));
                });

                triggerAsyncUpdate();
            });

            watcher.EnumerationCompleted ([this] (auto&, auto&)
            {
                const std::scoped_lock lock { mutex };
                pendingWork.push_back ([this]
                {
                    const auto devices = watcher.EnumeratedEndpointDevices();

                    for (const auto& [id, device] : devices)
                    {
                        auto endpoint = makeEndpoint (device);
                        cachedEndpoints.insert_or_assign (endpoint.id, endpoint);
                    }
                });

                triggerAsyncUpdate();
            });

            watcher.Start();
        }

        void virtualEndpointAdded (std::shared_ptr<VirtualEndpoint> ep) override
        {
            if (ep != nullptr)
                virtualEndpoints[ep->getId()] = ep;
        }

        std::function<void()> popWork()
        {
            const std::scoped_lock lock { mutex };

            if (pendingWork.empty())
                return {};

            auto result = pendingWork.front();
            pendingWork.pop_front();
            return result;
        }

        void handleAsyncUpdate() override
        {
            while (auto fn = popWork())
                fn();

            listener.endpointsChanged();
        }

        static ump::EndpointAndStaticInfo makeEndpoint (const wm2::MidiEndpointDeviceInformation& info)
        {
            const auto transport = std::invoke ([&]
            {
                const auto t = info.GetTransportSuppliedInfo().NativeDataFormat;

                if (t == wm2::MidiEndpointNativeDataFormat::Midi1ByteFormat)
                    return ump::Transport::bytestream;

                return ump::Transport::ump;
            });

            const auto itemProtocol = std::invoke ([&]
            {
                const auto p = info.GetDeclaredStreamConfiguration().Protocol;

                if (p == wm2::MidiProtocol::Midi1 || transport == ump::Transport::bytestream)
                    return ump::PacketProtocol::MIDI_1_0;

                return ump::PacketProtocol::MIDI_2_0;
            });

            const auto deviceInfo = makeDeviceInfo (info.GetDeclaredDeviceIdentity());

            const auto id = toString (info.EndpointDeviceId());

            std::array<String, 16> legacyIds;

            for (auto [index, value] : enumerate (legacyIds))
            {
                auto obj = new DynamicObject;
                obj->setProperty ("endpoint", id);
                obj->setProperty ("group", index);
                value = JSON::toString (obj, true);
            }

            std::vector<ump::Block> blocks;

            for (const auto& fb : info.GetDeclaredFunctionBlocks())
                blocks.push_back (makeBlock (fb).block);

            if (blocks.empty())
            {
                for (const auto& gtb : info.GetGroupTerminalBlocks())
                    blocks.push_back (makeBlock (gtb.AsEquivalentFunctionBlock()).block);
            }

            const auto e = info.GetDeclaredEndpointInfo();
            const auto manufacturer = info.GetContainerDeviceInformation()
                                          .Properties()
                                          .TryLookup (L"System.Devices.Manufacturer");
            const auto product = info.GetContainerDeviceInformation()
                                     .Properties()
                                     .TryLookup (L"System.Devices.ModelName");

            const auto endpoint = ump::Endpoint{}.withName (toString (info.Name()))
                                                 .withProtocol (itemProtocol)
                                                 .withBlocks (blocks)
                                                 .withDeviceInfo (deviceInfo)
                                                 .withProductInstanceId (toString (info.GetDeclaredEndpointInfo().ProductInstanceId))
                                                 .withUMPVersion (e.SpecificationVersionMajor, e.SpecificationVersionMinor)
                                                 .withMidi1Support (e.SupportsMidi10Protocol)
                                                 .withMidi2Support (e.SupportsMidi20Protocol)
                                                 .withStaticBlocks (e.HasStaticFunctionBlocks)
                                                 .withReceiveJRSupport (e.SupportsReceivingJitterReductionTimestamps)
                                                 .withTransmitJRSupport (e.SupportsSendingJitterReductionTimestamps);

            const auto hasBlockDirection = [&] (auto direction)
            {
                const auto blockCanUseDirection = [&] (const wm2::MidiFunctionBlock& x)
                {
                    const auto d = x.Direction();
                    return d == wm2::MidiFunctionBlockDirection::Bidirectional || d == direction;
                };

                const auto fb = info.GetDeclaredFunctionBlocks();
                const auto gt = info.GetGroupTerminalBlocks();

                return std::any_of (fb.begin(), fb.end(), blockCanUseDirection)
                    || std::any_of (gt.begin(), gt.end(), [&] (const wm2::MidiGroupTerminalBlock& x)
                       {
                           return blockCanUseDirection (x.AsEquivalentFunctionBlock());
                       });
            };

            const auto staticInfo = ump::StaticDeviceInfo{}.withName (toString (info.Name()))
                                                           .withManufacturer (toString (winrt::unbox_value_or<winrt::hstring> (manufacturer, L"")))
                                                           .withProduct (toString (winrt::unbox_value_or<winrt::hstring> (product, L"")))
                                                           .withHasSource (hasBlockDirection (wm2::MidiFunctionBlockDirection::BlockOutput))
                                                           .withHasDestination (hasBlockDirection (wm2::MidiFunctionBlockDirection::BlockInput))
                                                           .withLegacyIdentifiersSrc (legacyIds)
                                                           .withLegacyIdentifiersDst (legacyIds)
                                                           .withTransport (transport);

            return { endpoint, staticInfo, ump::EndpointId::makeSrcDst (id, id) };
        }

        class SdkInitialiser
        {
        public:
            SdkInitialiser() = default;

            bool isValid() const { return ptr != nullptr; }

        private:
            ComSmartPtr<mwdmi::IMidiClientInitializer> ptr = std::invoke ([]() -> ComSmartPtr<mwdmi::IMidiClientInitializer>
            {
                try
                {
                    winrt::init_apartment (winrt::apartment_type::single_threaded);
                }
                catch (...)
                {
                    // We tried...
                }

                ComSmartPtr<mwdmi::IMidiClientInitializer> result;

                if (FAILED (CoCreateInstance (__uuidof (mwdmi::MidiClientInitializerUuid),
                                              nullptr,
                                              CLSCTX::CLSCTX_INPROC_SERVER | CLSCTX::CLSCTX_FROM_DEFAULT_CONTEXT,
                                              __uuidof (mwdmi::IMidiClientInitializer),
                                              (void**) result.resetAndGetPointerAddress())))
                    return {};

                if (result == nullptr)
                    return {};

                if (FAILED (result->EnsureServiceAvailable()))
                    return {};

                return result;
            });
        };

        SharedResourcePointer<SdkInitialiser> initialiser;

        std::mutex mutex;
        std::deque<std::function<void()>> pendingWork;

        ump::EndpointsListener& listener;
        std::map<ump::EndpointId, ump::EndpointAndStaticInfo> cachedEndpoints;
        std::map<ump::EndpointId, std::weak_ptr<VirtualEndpoint>> virtualEndpoints;
        wm2::MidiEndpointDeviceWatcher watcher;
    };

    static String toString (const winrt::hstring& str)
    {
        return CharPointer_UTF16 { str.data() };
    }

    MidiServices() = delete;
};

#endif

RTL_OSVERSIONINFOW getWindowsVersionInfo();

struct WindowsMidiHelpers
{
    #if JUCE_USE_WINRT_MIDI
    #ifndef JUCE_FORCE_WINRT_MIDI
    #define JUCE_FORCE_WINRT_MIDI 0
    #endif

    #ifndef JUCE_WINRT_MIDI_LOGGING
    #define JUCE_WINRT_MIDI_LOGGING 0
    #endif

    #if JUCE_WINRT_MIDI_LOGGING
    #define JUCE_WINRT_MIDI_LOG(x)  DBG(x)
    #else
    #define JUCE_WINRT_MIDI_LOG(x)
    #endif

    struct Winrt
    {
        using DeviceInformation             = ABI::Windows::Devices::Enumeration::DeviceInformation;
        using DeviceInformationKind         = ABI::Windows::Devices::Enumeration::DeviceInformationKind;
        using DeviceInformationUpdate       = ABI::Windows::Devices::Enumeration::DeviceInformationUpdate;
        using DeviceWatcher                 = ABI::Windows::Devices::Enumeration::DeviceWatcher;
        using IBuffer                       = ABI::Windows::Storage::Streams::IBuffer;
        using IBufferFactory                = ABI::Windows::Storage::Streams::IBufferFactory;
        using IBufferByteAccess             = Windows::Storage::Streams::IBufferByteAccess;
        using IDeviceInformation            = ABI::Windows::Devices::Enumeration::IDeviceInformation;
        using IDeviceInformationStatics2    = ABI::Windows::Devices::Enumeration::IDeviceInformationStatics2;
        using IDeviceInformationUpdate      = ABI::Windows::Devices::Enumeration::IDeviceInformationUpdate;
        using IDevicePicker                 = ABI::Windows::Devices::Enumeration::IDevicePicker;
        using IDeviceWatcher                = ABI::Windows::Devices::Enumeration::IDeviceWatcher;
        using IMidiInPort                   = ABI::Windows::Devices::Midi::IMidiInPort;
        using IMidiInPortStatics            = ABI::Windows::Devices::Midi::IMidiInPortStatics;
        using IMidiMessage                  = ABI::Windows::Devices::Midi::IMidiMessage;
        using IMidiMessageReceivedEventArgs = ABI::Windows::Devices::Midi::IMidiMessageReceivedEventArgs;
        using IMidiOutPort                  = ABI::Windows::Devices::Midi::IMidiOutPort;
        using IMidiOutPortStatics           = ABI::Windows::Devices::Midi::IMidiOutPortStatics;
        using MidiInPort                    = ABI::Windows::Devices::Midi::MidiInPort;
        using MidiMessageReceivedEventArgs  = ABI::Windows::Devices::Midi::MidiMessageReceivedEventArgs;

        template <typename T, typename U> using ITypedEventHandler  = ABI::Windows::Foundation::ITypedEventHandler<T, U>;
        template <typename T> using IAsyncOperation                 = ABI::Windows::Foundation::IAsyncOperation<T>;
        template <typename T> using IAsyncOperationCompletedHandler = ABI::Windows::Foundation::IAsyncOperationCompletedHandler<T>;
        template <typename T> using IIterable                       = ABI::Windows::Foundation::Collections::IIterable<T>;
        template <typename T> using IReference                      = ABI::Windows::Foundation::IReference<T>;
        template <typename T> using IVector                         = ABI::Windows::Foundation::Collections::IVector<T>;

        class DeviceCallbackHandler
        {
        public:
            struct Delegate
            {
                virtual ~Delegate() = default;
                virtual HRESULT addDevice (IDeviceInformation*) = 0;
                virtual HRESULT removeDevice (IDeviceInformationUpdate*) = 0;
                virtual HRESULT updateDevice (IDeviceInformationUpdate*) = 0;
            };

            static std::unique_ptr<DeviceCallbackHandler> make (Delegate* delegate,
                                                                HSTRING deviceSelector,
                                                                DeviceInformationKind infoKind)
            {
                auto* wrtWrapper = WinRTWrapper::getInstanceWithoutCreating();

                if (wrtWrapper == nullptr)
                {
                    JUCE_WINRT_MIDI_LOG ("Failed to get the WinRTWrapper singleton!");
                    return {};
                }

                auto deviceInfoFactory = wrtWrapper->getWRLFactory<IDeviceInformationStatics2> (&RuntimeClass_Windows_Devices_Enumeration_DeviceInformation[0]);

                if (deviceInfoFactory == nullptr)
                    return {};

                // A quick way of getting an IVector<HSTRING>...
                auto requestedProperties = [wrtWrapper]
                {
                    auto devicePicker = wrtWrapper->activateInstance<IDevicePicker> (&RuntimeClass_Windows_Devices_Enumeration_DevicePicker[0],
                                                                                     __uuidof (IDevicePicker));
                    jassert (devicePicker != nullptr);

                    IVector<HSTRING>* result;
                    auto hr = devicePicker->get_RequestedProperties (&result);
                    jassert (SUCCEEDED (hr));

                    hr = result->Clear();
                    jassert (SUCCEEDED (hr));

                    return result;
                }();

                StringArray propertyKeys ("System.Devices.ContainerId",
                                          "System.Devices.Aep.ContainerId",
                                          "System.Devices.Aep.IsConnected");

                for (auto& key : propertyKeys)
                {
                    WinRTWrapper::ScopedHString hstr (key);
                    auto hr = requestedProperties->Append (hstr.get());

                    if (FAILED (hr))
                    {
                        jassertfalse;
                        return {};
                    }
                }

                ComSmartPtr<IIterable<HSTRING>> iter;
                auto hr = requestedProperties->QueryInterface (__uuidof (IIterable<HSTRING>), (void**) iter.resetAndGetPointerAddress());

                if (FAILED (hr))
                {
                    jassertfalse;
                    return {};
                }

                auto result = rawToUniquePtr (new DeviceCallbackHandler);
                result->delegate = delegate;

                hr = deviceInfoFactory->CreateWatcherWithKindAqsFilterAndAdditionalProperties (deviceSelector, iter, infoKind,
                                                                                               result->watcher.resetAndGetPointerAddress());

                if (FAILED (hr))
                {
                    jassertfalse;
                    return {};
                }

                result->thread = std::thread { [r = result.get()]
                {
                    Thread::setCurrentThreadName (SystemStats::getJUCEVersion() + ": WinRT Device Enumeration Thread");

                    r->watcher->add_Added (
                            Microsoft::WRL::Callback<ITypedEventHandler<DeviceWatcher*, DeviceInformation*>> (
                                    [r] (IDeviceWatcher*, IDeviceInformation* info) { return r->delegate->addDevice (info); }
                            ).Get(),
                            &r->deviceAddedToken);

                    r->watcher->add_Removed (
                            Microsoft::WRL::Callback<ITypedEventHandler<DeviceWatcher*, DeviceInformationUpdate*>> (
                                    [r] (IDeviceWatcher*, IDeviceInformationUpdate* infoUpdate) { return r->delegate->removeDevice (infoUpdate); }
                            ).Get(),
                            &r->deviceRemovedToken);

                    r->watcher->add_Updated (
                            Microsoft::WRL::Callback<ITypedEventHandler<DeviceWatcher*, DeviceInformationUpdate*>> (
                                    [r] (IDeviceWatcher*, IDeviceInformationUpdate* infoUpdate) { return r->delegate->updateDevice (infoUpdate); }
                            ).Get(),
                            &r->deviceUpdatedToken);

                    r->watcher->Start();
                } };

                return result;
            }

            ~DeviceCallbackHandler()
            {
                if (thread.joinable())
                    thread.join();

                if (watcher == nullptr)
                    return;

                auto hr = watcher->Stop();
                jassert (SUCCEEDED (hr));

                if (deviceAddedToken.value != 0)
                {
                    hr = watcher->remove_Added (deviceAddedToken);
                    jassert (SUCCEEDED (hr));
                    deviceAddedToken.value = 0;
                }

                if (deviceUpdatedToken.value != 0)
                {
                    hr = watcher->remove_Updated (deviceUpdatedToken);
                    jassert (SUCCEEDED (hr));
                    deviceUpdatedToken.value = 0;
                }

                if (deviceRemovedToken.value != 0)
                {
                    hr = watcher->remove_Removed (deviceRemovedToken);
                    jassert (SUCCEEDED (hr));
                    deviceRemovedToken.value = 0;
                }

                watcher = nullptr;
            }

            template <typename InfoType>
            IInspectable* getValueFromDeviceInfo (String key, InfoType* info)
            {
                __FIMapView_2_HSTRING_IInspectable* properties;
                info->get_Properties (&properties);

                boolean found = false;
                WinRTWrapper::ScopedHString keyHstr (key);
                auto hr = properties->HasKey (keyHstr.get(), &found);

                if (FAILED (hr))
                {
                    jassertfalse;
                    return nullptr;
                }

                if (! found)
                    return nullptr;

                IInspectable* inspectable;
                hr = properties->Lookup (keyHstr.get(), &inspectable);

                if (FAILED (hr))
                {
                    jassertfalse;
                    return nullptr;
                }

                return inspectable;
            }

            String getGUIDFromInspectable (IInspectable& inspectable)
            {
                ComSmartPtr<IReference<GUID>> guidRef;
                auto hr = inspectable.QueryInterface (__uuidof (IReference<GUID>),
                                                      (void**) guidRef.resetAndGetPointerAddress());

                if (FAILED (hr))
                {
                    jassertfalse;
                    return {};
                }

                GUID result;
                hr = guidRef->get_Value (&result);

                if (FAILED (hr))
                {
                    jassertfalse;
                    return {};
                }

                OLECHAR* resultString;
                StringFromCLSID (result, &resultString);

                return resultString;
            }

            bool getBoolFromInspectable (IInspectable& inspectable)
            {
                ComSmartPtr<IReference<bool>> boolRef;
                auto hr = inspectable.QueryInterface (__uuidof (IReference<bool>),
                                                      (void**) boolRef.resetAndGetPointerAddress());

                if (FAILED (hr))
                {
                    jassertfalse;
                    return false;
                }

                boolean result;
                hr = boolRef->get_Value (&result);

                if (FAILED (hr))
                {
                    jassertfalse;
                    return false;
                }

                return result;
            }

        private:
            DeviceCallbackHandler() = default;

            Delegate* delegate = nullptr;
            ComSmartPtr<IDeviceWatcher> watcher;
            EventRegistrationToken deviceAddedToken { 0 }, deviceRemovedToken { 0 }, deviceUpdatedToken { 0 };
            std::thread thread;
        };

        struct BLEDeviceWatcherListener
        {
            virtual ~BLEDeviceWatcherListener() = default;
            virtual void bleDeviceAdded (const String& containerID) = 0;
            virtual void bleDeviceDisconnected (const String& containerID) = 0;
        };

        class BLEDeviceWatcher : private DeviceCallbackHandler::Delegate
        {
        public:
            static std::unique_ptr<BLEDeviceWatcher> make()
            {
                WinRTWrapper::ScopedHString deviceSelector ("System.Devices.Aep.ProtocolId:=\"{bb7bb05e-5972-42b5-94fc-76eaa7084d49}\""
                                                            " AND System.Devices.Aep.IsPaired:=System.StructuredQueryType.Boolean#True");

                auto result = rawToUniquePtr (new BLEDeviceWatcher);

                if (auto watcher = DeviceCallbackHandler::make (result.get(), deviceSelector.get(), DeviceInformationKind::DeviceInformationKind_AssociationEndpoint))
                {
                    result->watcher = std::move (watcher);
                    return result;
                }

                return {};
            }

            bool isBleDevice (const String& containerId) const
            {
                const ScopedLock lock (deviceChanges);
                return bleContainerIds.find (containerId) != bleContainerIds.end();
            }

            void addListener (BLEDeviceWatcherListener& l)
            {
                listeners.add (l);
            }

            void removeListener (BLEDeviceWatcherListener& l)
            {
                listeners.remove (l);
            }

        private:
            HRESULT addDevice (IDeviceInformation* addedDeviceInfo) override
            {
                HSTRING deviceIDHst;
                auto hr = addedDeviceInfo->get_Id (&deviceIDHst);

                if (FAILED (hr))
                {
                    JUCE_WINRT_MIDI_LOG ("Failed to query added BLE device ID!");
                    return S_OK;
                }

                auto* wrtWrapper = WinRTWrapper::getInstanceWithoutCreating();

                if (wrtWrapper == nullptr)
                {
                    JUCE_WINRT_MIDI_LOG ("Failed to get the WinRTWrapper singleton!");
                    return false;
                }

                auto deviceID = wrtWrapper->hStringToString (deviceIDHst);
                JUCE_WINRT_MIDI_LOG ("Detected paired BLE device: " << deviceID);

                if (auto* containerIDValue = watcher->getValueFromDeviceInfo ("System.Devices.Aep.ContainerId", addedDeviceInfo))
                {
                    auto containerID = watcher->getGUIDFromInspectable (*containerIDValue);

                    if (containerID.isNotEmpty())
                    {
                        listeners.call ([&containerID] (auto& l) { l.bleDeviceAdded (containerID); });

                        const ScopedLock lock (deviceChanges);
                        bleContainerIds.insert (containerID);
                        return S_OK;
                    }
                }

                JUCE_WINRT_MIDI_LOG ("Failed to get a container ID for BLE device: " << deviceID);
                return S_OK;
            }

            HRESULT removeDevice (IDeviceInformationUpdate* removedDeviceInfo) override
            {
                if (auto* containerIDValue = watcher->getValueFromDeviceInfo ("System.Devices.Aep.ContainerId", removedDeviceInfo))
                {
                    auto containerID = watcher->getGUIDFromInspectable (*containerIDValue);

                    if (containerID.isNotEmpty())
                    {
                        listeners.call ([&containerID] (auto& l) { l.bleDeviceDisconnected (containerID); });

                        const ScopedLock lock (deviceChanges);
                        bleContainerIds.erase (containerID);
                        return S_OK;
                    }
                }

                return E_FAIL;
            }

            HRESULT updateDevice (IDeviceInformationUpdate*) override
            {
                // This shouldn't change the device container
                return S_OK;
            }

            BLEDeviceWatcher() = default;

            WaitFreeListeners<BLEDeviceWatcherListener> listeners;
            std::set<String> bleContainerIds;
            CriticalSection deviceChanges;
            std::unique_ptr<DeviceCallbackHandler> watcher;

            JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BLEDeviceWatcher)
        };

        struct WinRTMIDIDeviceInfo
        {
            String deviceID, containerID, name;
            bool isDefault = false;
        };

        struct MidiIODeviceWatcherListener
        {
            virtual ~MidiIODeviceWatcherListener() = default;
            virtual void deviceAdded (const String&) = 0;
            virtual void deviceRemoved (const String&) = 0;
            virtual void deviceUpdated (const String&) = 0;
        };

        template <typename COMFactoryType>
        class MidiIODeviceWatcher : private DeviceCallbackHandler::Delegate
        {
        public:
            void getAvailableDevices (std::vector<ump::EndpointAndStaticInfo>& storage)
            {
                {
                    const ScopedLock lock (deviceChanges);
                    lastQueriedConnectedDevices = connectedDevices;
                }

                StringArray deviceNames, deviceIDs;

                for (auto info : lastQueriedConnectedDevices.get())
                {
                    deviceNames.add (info.name);
                    deviceIDs  .add (info.containerID);
                }

                deviceNames.appendNumbersToDuplicates (false, false, CharPointer_UTF8 ("-"), CharPointer_UTF8 (""));
                deviceIDs  .appendNumbersToDuplicates (false, false, CharPointer_UTF8 ("-"), CharPointer_UTF8 (""));

                constexpr auto direction = std::invoke ([]
                {
                    if constexpr (std::is_same_v<COMFactoryType, IMidiInPortStatics>)
                        return ump::BlockDirection::sender;
                    else if constexpr (std::is_same_v<COMFactoryType, IMidiOutPortStatics>)
                        return ump::BlockDirection::receiver;
                });

                for (int i = 0; i < deviceNames.size(); ++i)
                {
                    const auto fullInfo = ump::IOHelpers::makeProxyEndpoint (MidiDeviceInfo { deviceNames[i], deviceIDs[i] }, direction);
                    storage.push_back (fullInfo);
                }
            }

            WinRTMIDIDeviceInfo getWinRTDeviceInfoForDevice (const ump::EndpointId& deviceIdentifier)
            {
                std::vector<ump::EndpointAndStaticInfo> endpoints;
                getAvailableDevices (endpoints);

                for (const auto [index, value] : enumerate (endpoints))
                    if (value.id == deviceIdentifier)
                        return lastQueriedConnectedDevices.get()[(int) index];

                return {};
            }

            static std::unique_ptr<MidiIODeviceWatcher> make (ComSmartPtr<COMFactoryType> f)
            {
                HSTRING deviceSelector;
                auto hr = f->GetDeviceSelector (&deviceSelector);

                if (FAILED (hr))
                {
                    JUCE_WINRT_MIDI_LOG ("Failed to get MIDI device selector!");
                    return {};
                }

                auto result = rawToUniquePtr (new MidiIODeviceWatcher);
                result->factory = f;

                if (auto watcher = DeviceCallbackHandler::make (result.get(), deviceSelector, DeviceInformationKind::DeviceInformationKind_DeviceInterface))
                {
                    result->watcher = std::move (watcher);
                    return result;
                }

                return {};
            }

            void addListener (MidiIODeviceWatcherListener& l)
            {
                listeners.add (l);
            }

            void removeListener (MidiIODeviceWatcherListener& l)
            {
                listeners.remove (l);
            }

        private:
            MidiIODeviceWatcher() = default;

            HRESULT addDevice (IDeviceInformation* addedDeviceInfo) override
            {
                WinRTMIDIDeviceInfo info;

                HSTRING deviceID;
                auto hr = addedDeviceInfo->get_Id (&deviceID);

                if (FAILED (hr))
                {
                    JUCE_WINRT_MIDI_LOG ("Failed to query added MIDI device ID!");
                    return S_OK;
                }

                auto* wrtWrapper = WinRTWrapper::getInstanceWithoutCreating();

                if (wrtWrapper == nullptr)
                {
                    JUCE_WINRT_MIDI_LOG ("Failed to get the WinRTWrapper singleton!");
                    return false;
                }

                info.deviceID = wrtWrapper->hStringToString (deviceID);

                listeners.call ([&] (auto& l) { l.deviceAdded (info.deviceID); });

                JUCE_WINRT_MIDI_LOG ("Detected MIDI device: " << info.deviceID);

                boolean isEnabled = false;
                hr = addedDeviceInfo->get_IsEnabled (&isEnabled);

                if (FAILED (hr) || ! isEnabled)
                {
                    JUCE_WINRT_MIDI_LOG ("MIDI device not enabled: " << info.deviceID);
                    return S_OK;
                }

                // We use the container ID to match a MIDI device with a generic BLE device, if possible
                if (auto* containerIDValue = watcher->getValueFromDeviceInfo ("System.Devices.ContainerId", addedDeviceInfo))
                    info.containerID = watcher->getGUIDFromInspectable (*containerIDValue);

                HSTRING name;
                hr = addedDeviceInfo->get_Name (&name);

                if (FAILED (hr))
                {
                    JUCE_WINRT_MIDI_LOG ("Failed to query detected MIDI device name for " << info.deviceID);
                    return S_OK;
                }

                info.name = wrtWrapper->hStringToString (name);

                boolean isDefault = false;
                hr = addedDeviceInfo->get_IsDefault (&isDefault);

                if (FAILED (hr))
                {
                    JUCE_WINRT_MIDI_LOG ("Failed to query detected MIDI device defaultness for " << info.deviceID << " " << info.name);
                    return S_OK;
                }

                info.isDefault = isDefault;

                JUCE_WINRT_MIDI_LOG ("Adding MIDI device: " << info.deviceID << " " << info.containerID << " " << info.name);

                {
                    const ScopedLock lock (deviceChanges);
                    connectedDevices.add (info);
                }

                return S_OK;
            }

            HRESULT removeDevice (IDeviceInformationUpdate* removedDeviceInfo) override
            {
                HSTRING removedDeviceIdHstr;
                auto hr = removedDeviceInfo->get_Id (&removedDeviceIdHstr);

                if (FAILED (hr))
                {
                    JUCE_WINRT_MIDI_LOG ("Failed to query removed MIDI device ID!");
                    return S_OK;
                }

                auto* wrtWrapper = WinRTWrapper::getInstanceWithoutCreating();

                if (wrtWrapper == nullptr)
                {
                    JUCE_WINRT_MIDI_LOG ("Failed to get the WinRTWrapper singleton!");
                    return S_OK;
                }

                auto removedDeviceId = wrtWrapper->hStringToString (removedDeviceIdHstr);

                listeners.call ([&] (auto& l) { l.deviceRemoved (removedDeviceId); });

                JUCE_WINRT_MIDI_LOG ("Removing MIDI device: " << removedDeviceId);

                {
                    const ScopedLock lock (deviceChanges);

                    for (int i = 0; i < connectedDevices.size(); ++i)
                    {
                        if (connectedDevices[i].deviceID == removedDeviceId)
                        {
                            connectedDevices.remove (i);
                            JUCE_WINRT_MIDI_LOG ("Removed MIDI device: " << removedDeviceId);
                            break;
                        }
                    }
                }

                return S_OK;
            }

            // This is never called
            HRESULT updateDevice (IDeviceInformationUpdate* c) override
            {
                HSTRING removedDeviceIdHstr;
                auto hr = c->get_Id (&removedDeviceIdHstr);

                if (FAILED (hr))
                {
                    JUCE_WINRT_MIDI_LOG ("Failed to query removed MIDI device ID!");
                    return S_OK;
                }

                auto* wrtWrapper = WinRTWrapper::getInstanceWithoutCreating();

                if (wrtWrapper == nullptr)
                {
                    JUCE_WINRT_MIDI_LOG ("Failed to get the WinRTWrapper singleton!");
                    return S_OK;
                }

                auto id = wrtWrapper->hStringToString (removedDeviceIdHstr);

                listeners.call ([&] (auto& l) { l.deviceUpdated (id); });
                return S_OK;
            }

            ComSmartPtr<COMFactoryType> factory;

            Array<WinRTMIDIDeviceInfo> connectedDevices;
            CriticalSection deviceChanges;
            ThreadLocalValue<Array<WinRTMIDIDeviceInfo>> lastQueriedConnectedDevices;
            WaitFreeListeners<MidiIODeviceWatcherListener> listeners;

            std::unique_ptr<DeviceCallbackHandler> watcher;

            JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MidiIODeviceWatcher)
        };

        struct DeviceRemovalListener
        {
            virtual ~DeviceRemovalListener() = default;
            virtual void deviceRemoved (const String&) = 0;
        };

        class Client : private MidiIODeviceWatcherListener,
                       private BLEDeviceWatcherListener,
                       private AsyncUpdater
        {
        public:
            ~Client() override
            {
                inputDeviceWatcher->removeListener (*this);
                outputDeviceWatcher->removeListener (*this);
                bleDeviceWatcher->removeListener (*this);
            }

            WinRTMIDIDeviceInfo getWinRTDeviceInfoForInput (const ump::EndpointId& deviceIdentifier) const
            {
                return inputDeviceWatcher->getWinRTDeviceInfoForDevice (deviceIdentifier);
            }

            WinRTMIDIDeviceInfo getWinRTDeviceInfoForOutput (const ump::EndpointId& deviceIdentifier) const
            {
                return outputDeviceWatcher->getWinRTDeviceInfoForDevice (deviceIdentifier);
            }

            bool isBleDevice (const String& containerId) const
            {
                return bleDeviceWatcher->isBleDevice (containerId);
            }

            void addBleListener (BLEDeviceWatcherListener& l)
            {
                bleDeviceWatcher->addListener (l);
            }

            void removeBleListener (BLEDeviceWatcherListener& l)
            {
                bleDeviceWatcher->removeListener (l);
            }

            void getEndpoints (std::vector<ump::EndpointId>& x)
            {
                std::transform (cachedEndpoints.begin(),
                                cachedEndpoints.end(),
                                std::back_inserter (x),
                                [] (const auto& p) { return p.first; });
            }

            std::optional<ump::Endpoint> getEndpoint (const ump::EndpointId& x) const
            {
                const auto iter = cachedEndpoints.find (x);

                if (iter == cachedEndpoints.end())
                    return {};

                return iter->second.endpoint;
            }

            std::optional<ump::StaticDeviceInfo> getStaticDeviceInfo (const ump::EndpointId& x) const
            {
                const auto iter = cachedEndpoints.find (x);

                if (iter == cachedEndpoints.end())
                    return {};

                return iter->second.info;
            }

            static std::unique_ptr<Client> make (ump::EndpointsListener& l)
            {
                const auto windowsVersionInfo = getWindowsVersionInfo();

                if (windowsVersionInfo.dwMajorVersion < 10 || windowsVersionInfo.dwBuildNumber < 17763)
                    return {};

                auto* wrtWrapper = WinRTWrapper::getInstance();

                if (! wrtWrapper->isInitialised())
                    return {};

                auto midiInFactory = wrtWrapper->getWRLFactory<IMidiInPortStatics> (&RuntimeClass_Windows_Devices_Midi_MidiInPort[0]);

                if (midiInFactory == nullptr)
                    return {};

                auto midiOutFactory = wrtWrapper->getWRLFactory<IMidiOutPortStatics> (&RuntimeClass_Windows_Devices_Midi_MidiOutPort[0]);

                if (midiOutFactory == nullptr)
                    return {};

                // The WinRT BLE MIDI API doesn't provide callbacks when devices become disconnected,
                // but it does require a disconnection via the API before a device will reconnect again.
                // We can monitor the BLE connection state of paired devices to get callbacks when
                // connections are broken.
                auto bleDeviceWatcher = BLEDeviceWatcher::make();

                if (bleDeviceWatcher == nullptr)
                    return {};

                auto inputDeviceWatcher = MidiIODeviceWatcher<IMidiInPortStatics>::make (midiInFactory);

                if (inputDeviceWatcher == nullptr)
                    return {};

                auto outputDeviceWatcher = MidiIODeviceWatcher<IMidiOutPortStatics>::make (midiOutFactory);

                if (outputDeviceWatcher == nullptr)
                    return {};

                auto result = rawToUniquePtr (new Client { l });
                result->midiInFactory = std::move (midiInFactory);
                result->midiOutFactory = std::move (midiOutFactory);

                result->inputDeviceWatcher = std::move (inputDeviceWatcher);
                result->outputDeviceWatcher = std::move (outputDeviceWatcher);
                result->bleDeviceWatcher = std::move (bleDeviceWatcher);

                result->inputDeviceWatcher->addListener (*result);
                result->outputDeviceWatcher->addListener (*result);
                result->bleDeviceWatcher->addListener (*result);

                result->updateCachedEndpoints();

                return result;
            }

            ComSmartPtr<IMidiInPortStatics> getMidiInFactory() const { return midiInFactory; }
            ComSmartPtr<IMidiOutPortStatics> getMidiOutFactory() const { return midiOutFactory; }

            void addDeviceRemovalListener (DeviceRemovalListener& l)
            {
                listeners.add (l);
            }

            void removeDeviceRemovalListener (DeviceRemovalListener& l)
            {
                listeners.remove (l);
            }

        private:
            explicit Client (ump::EndpointsListener& l)
                : listener (l) {}

            void deviceAdded (const String&) override
            {
                triggerAsyncUpdate();
            }

            void deviceRemoved (const String& id) override
            {
                listeners.call ([&] (auto& c) { c.deviceRemoved (id); });
                triggerAsyncUpdate();
            }

            void deviceUpdated (const String&) override
            {
                triggerAsyncUpdate();
            }

            void bleDeviceAdded (const String&) override
            {
                triggerAsyncUpdate();
            }

            void bleDeviceDisconnected (const String&) override
            {
                triggerAsyncUpdate();
            }

            void handleAsyncUpdate() override
            {
                updateCachedEndpoints();
                listener.endpointsChanged();
            }

            void updateCachedEndpoints()
            {
                std::vector<ump::EndpointAndStaticInfo> buffer;
                inputDeviceWatcher->getAvailableDevices (buffer);
                outputDeviceWatcher->getAvailableDevices (buffer);

                cachedEndpoints.clear();

                for (auto& item : buffer)
                    cachedEndpoints.emplace (item.id, item);

                // If this is hit, we got an identical ID for an input and output device
                jassert (cachedEndpoints.size() == buffer.size());
            }

            ump::EndpointsListener& listener;
            std::map<ump::EndpointId, ump::EndpointAndStaticInfo> cachedEndpoints;

            ComSmartPtr<IMidiInPortStatics>  midiInFactory;
            ComSmartPtr<IMidiOutPortStatics> midiOutFactory;
            std::unique_ptr<MidiIODeviceWatcher<IMidiInPortStatics>>  inputDeviceWatcher;
            std::unique_ptr<MidiIODeviceWatcher<IMidiOutPortStatics>> outputDeviceWatcher;
            std::unique_ptr<BLEDeviceWatcher> bleDeviceWatcher;
            WaitFreeListeners<DeviceRemovalListener> listeners;

            JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Client)
        };

        template <bool isInput>
        class WinRTIOWrapper : private BLEDeviceWatcherListener,
                               private DeviceRemovalListener,
                               private AsyncUpdater
        {
        public:
            static std::unique_ptr<WinRTIOWrapper> make (std::shared_ptr<Client> c,
                                                         const ump::EndpointId& deviceIdentifier,
                                                         ump::DisconnectionListener& listener)
            {
                auto result = rawToUniquePtr (new WinRTIOWrapper);
                result->client = c;
                result->onDisconnect = &listener;
                result->deviceInfo = isInput ? c->getWinRTDeviceInfoForInput (deviceIdentifier)
                                             : c->getWinRTDeviceInfoForOutput (deviceIdentifier);

                if (result->deviceInfo.deviceID.isEmpty())
                    return {};

                JUCE_WINRT_MIDI_LOG ("Creating JUCE MIDI IO: " << deviceInfo.deviceID);

                result->client->addDeviceRemovalListener (*result);

                if (result->deviceInfo.containerID.isNotEmpty())
                {
                    result->isBLEDevice = result->client->isBleDevice (result->deviceInfo.containerID);
                    result->client->addBleListener (*result);
                }

                return result;
            }

            WinRTMIDIDeviceInfo getDeviceInfo() const { return deviceInfo; }
            bool isBLE() const { return isBLEDevice; }

            ump::EndpointId getEndpointId() const
            {
                return isInput ? ump::EndpointId::make (ump::IOKind::src, deviceInfo.containerID)
                               : ump::EndpointId::make (ump::IOKind::dst, deviceInfo.containerID);
            }

            ~WinRTIOWrapper() override
            {
                client->removeDeviceRemovalListener (*this);
                client->removeBleListener (*this);

                cancelPendingUpdate();
            }

        private:
            WinRTIOWrapper() = default;

            void bleDeviceAdded (const String& containerID) override
            {
                if (containerID == deviceInfo.containerID)
                    isBLEDevice = true;
            }

            void bleDeviceDisconnected (const String& containerID) override
            {
                if (containerID != deviceInfo.containerID)
                    triggerAsyncUpdate();

                JUCE_WINRT_MIDI_LOG ("Disconnecting MIDI port from BLE disconnection: " << deviceInfo.deviceID
                                     << " " << deviceInfo.containerID << " " << deviceInfo.name);
            }

            void deviceRemoved (const String& deviceId) override
            {
                if (deviceId == deviceInfo.deviceID)
                    triggerAsyncUpdate();
            }

            void handleAsyncUpdate() override
            {
                if (onDisconnect != nullptr)
                    onDisconnect->disconnected();
            }

            std::shared_ptr<Client> client;
            WinRTMIDIDeviceInfo deviceInfo;
            bool isBLEDevice = false;
            ump::DisconnectionListener* onDisconnect = nullptr;

            JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WinRTIOWrapper)
        };

        template <typename COMType, typename COMFactoryType, typename COMInterfaceType>
        static void openMidiPortThread (String threadName,
                                        String midiDeviceID,
                                        const ComSmartPtr<COMFactoryType>& comFactory,
                                        ComSmartPtr<COMInterfaceType>& comPort)
        {
            std::thread { [&]
            {
                Thread::setCurrentThreadName (threadName);

                const WinRTWrapper::ScopedHString hDeviceId { midiDeviceID };
                ComSmartPtr<IAsyncOperation<COMType*>> asyncOp;
                const auto hr = comFactory->FromIdAsync (hDeviceId.get(), asyncOp.resetAndGetPointerAddress());

                if (FAILED (hr))
                    return;

                std::promise<ComSmartPtr<COMInterfaceType>> promise;
                auto future = promise.get_future();

                auto callback = [p = std::move (promise)] (IAsyncOperation<COMType*>* asyncOpPtr, AsyncStatus) mutable
                {
                   if (asyncOpPtr == nullptr)
                   {
                       p.set_value (nullptr);
                       return E_ABORT;
                   }

                   ComSmartPtr<COMInterfaceType> result;
                   const auto hr = asyncOpPtr->GetResults (result.resetAndGetPointerAddress());

                   if (FAILED (hr))
                   {
                       p.set_value (nullptr);
                       return hr;
                   }

                   p.set_value (std::move (result));
                   return S_OK;
               };

               const auto ir = asyncOp->put_Completed (Microsoft::WRL::Callback<IAsyncOperationCompletedHandler<COMType*>> (std::move (callback)).Get());

               if (FAILED (ir))
                   return;

               if (future.wait_for (std::chrono::milliseconds (2000)) == std::future_status::ready)
                   comPort = future.get();
            } }.join();
        }

        class InputImplNative : public ump::Input::Impl::Native
        {
        public:
            ~InputImplNative()
            {
                disconnect();
            }

            ump::EndpointId getEndpointId() const override
            {
                return wrapper->getEndpointId();
            }

            ump::PacketProtocol getProtocol() const override
            {
                return dispatcher.getProtocol();
            }

            static std::unique_ptr<InputImplNative> open (std::shared_ptr<Client> c,
                                                          ump::DisconnectionListener& listener,
                                                          const ump::EndpointId& identifier,
                                                          ump::PacketProtocol protocol,
                                                          ump::Consumer& consumer)
            {
                auto result = rawToUniquePtr (new InputImplNative (c, protocol, consumer));

                auto w = WinRTIOWrapper<true>::make (c, identifier, listener);

                if (w == nullptr)
                    return {};

                result->wrapper = std::move (w);

                openMidiPortThread<MidiInPort> ("Open WinRT MIDI input port",
                                                result->wrapper->getDeviceInfo().deviceID,
                                                result->client->getMidiInFactory(),
                                                result->port);

                if (result->port == nullptr)
                {
                    JUCE_WINRT_MIDI_LOG ("Timed out waiting for midi input port creation");
                    return {};
                }

                result->startTime = Time::getMillisecondCounterHiRes();
                result->start();
                return result;
            }

        private:
            InputImplNative (std::shared_ptr<Client> c, ump::PacketProtocol p, ump::Consumer& cb)
                : client (c), consumer (cb), dispatcher { 0, p, 4096 }
            {
            }

            void start()
            {
                if (midiInMessageToken.value != 0)
                {
                    JUCE_WINRT_MIDI_LOG ("Input already started");
                    return;
                }

                const auto hr = port->add_MessageReceived (
                    Microsoft::WRL::Callback<ITypedEventHandler<MidiInPort*, MidiMessageReceivedEventArgs*>> (
                        [this] (IMidiInPort*, IMidiMessageReceivedEventArgs* args)
                        {
                            return midiInMessageReceived (args);
                        }
                    ).Get(),
                    &midiInMessageToken);

                if (FAILED (hr))
                {
                    JUCE_WINRT_MIDI_LOG ("Failed to set MIDI input callback");
                    jassertfalse;
                }
            }

            void stop()
            {
                if (port != nullptr && midiInMessageToken.value != 0)
                    port->remove_MessageReceived (midiInMessageToken);

                midiInMessageToken = {};
            }

            void disconnect()
            {
                stop();

                if (port != nullptr && wrapper->isBLE())
                    port->Release();

                port = nullptr;
            }

            HRESULT midiInMessageReceived (IMidiMessageReceivedEventArgs* args)
            {
                ComSmartPtr<IMidiMessage> message;
                auto hr = args->get_Message (message.resetAndGetPointerAddress());

                if (FAILED (hr))
                    return hr;

                ComSmartPtr<IBuffer> buffer;
                hr = message->get_RawData (buffer.resetAndGetPointerAddress());

                if (FAILED (hr))
                    return hr;

                ComSmartPtr<IBufferByteAccess> bufferByteAccess;
                hr = buffer->QueryInterface (bufferByteAccess.resetAndGetPointerAddress());

                if (FAILED (hr))
                    return hr;

                uint8_t* bufferData = nullptr;
                hr = bufferByteAccess->Buffer (&bufferData);

                if (FAILED (hr))
                    return hr;

                uint32_t numBytes = 0;
                hr = buffer->get_Length (&numBytes);

                if (FAILED (hr))
                    return hr;

                ABI::Windows::Foundation::TimeSpan timespan;
                hr = message->get_Timestamp (&timespan);

                if (FAILED (hr))
                    return hr;

                const Span bytes { unalignedPointerCast<const std::byte*> (bufferData), numBytes };
                const auto time = convertTimeStamp (timespan.Duration);
                dispatcher.dispatch (bytes, time, [this] (const ump::View& view, double timestamp)
                {
                    const ump::Iterator b { view.data(), view.size() };
                    const auto e = std::next (b);
                    consumer.consume (b, e, timestamp);
                });

                return S_OK;
            }

            double convertTimeStamp (int64 timestamp)
            {
                auto millisecondsSinceStart = static_cast<double> (timestamp) / 10000.0;
                auto t = startTime + millisecondsSinceStart;
                auto now = Time::getMillisecondCounterHiRes();

                if (t > now)
                {
                    if (t > now + 2.0)
                        startTime -= 1.0;

                    t = now;
                }

                return t * 0.001;
            }

            std::shared_ptr<Client> client;
            ComSmartPtr<IMidiInPort> port;
            ump::Consumer& consumer;
            EventRegistrationToken midiInMessageToken { 0 };
            double startTime = 0;
            ump::BytestreamToUMPDispatcher dispatcher;
            std::unique_ptr<WinRTIOWrapper<true>> wrapper;
        };

        class OutputImplNative : public ump::Output::Impl::Native
        {
        public:
            ~OutputImplNative()
            {
                disconnect();
            }

            ump::EndpointId getEndpointId() const override
            {
                return wrapper->getEndpointId();
            }

            bool send (ump::Iterator b, ump::Iterator e) override
            {
                for (const auto& view : makeRange (b, e))
                {
                    toBytestream.convert (view, 0, [&] (ump::BytesOnGroup bytesView, double)
                    {
                        sendBytestream (bytesView.bytes);
                    });
                }

                return true;
            }

            static std::unique_ptr<OutputImplNative> open (std::shared_ptr<Client> c,
                                                           ump::DisconnectionListener& listener,
                                                           const ump::EndpointId& identifier)
            {
                auto result = rawToUniquePtr (new OutputImplNative { c });

                auto w = WinRTIOWrapper<false>::make (c, identifier, listener);

                if (w == nullptr)
                    return {};

                result->wrapper = std::move (w);

                openMidiPortThread<IMidiOutPort> ("Open WinRT MIDI output port",
                                                  result->wrapper->getDeviceInfo().deviceID,
                                                  result->client->getMidiOutFactory(),
                                                  result->port);

                if (result->port == nullptr)
                    return {};

                auto* wrtWrapper = WinRTWrapper::getInstanceWithoutCreating();

                if (wrtWrapper == nullptr)
                    return {};

                auto bufferFactory = wrtWrapper->getWRLFactory<IBufferFactory> (&RuntimeClass_Windows_Storage_Streams_Buffer[0]);

                if (bufferFactory == nullptr)
                    return {};

                auto hr = bufferFactory->Create (static_cast<UINT32> (65536), result->buffer.resetAndGetPointerAddress());

                if (FAILED (hr))
                    return {};

                hr = result->buffer->QueryInterface (result->bufferByteAccess.resetAndGetPointerAddress());

                if (FAILED (hr))
                    return {};

                hr = result->bufferByteAccess->Buffer (&result->bufferData);

                if (FAILED (hr))
                    return {};

                return result;
            }

        private:
            explicit OutputImplNative (std::shared_ptr<Client> c)
                : client (std::move (c))
            {
            }

            void disconnect()
            {
                if (port != nullptr && wrapper->isBLE())
                    port->Release();

                port = nullptr;
            }

            void sendBytestream (Span<const std::byte> message)
            {
                if (port == nullptr)
                    return;

                auto numBytes = message.size();
                auto hr = buffer->put_Length ((UINT32) numBytes);

                if (FAILED (hr))
                {
                    jassertfalse;
                    return;
                }

                memcpy_s (bufferData, numBytes, message.data(), numBytes);
                port->SendBuffer (buffer);
            }

            std::shared_ptr<Client> client;
            ComSmartPtr<IMidiOutPort> port;
            ump::ToBytestreamConverter toBytestream { 4096 };
            ComSmartPtr<IBuffer> buffer;
            ComSmartPtr<IBufferByteAccess> bufferByteAccess;
            uint8_t* bufferData = nullptr;
            std::unique_ptr<WinRTIOWrapper<false>> wrapper;
        };

        class SessionImplNative : public ump::Session::Impl::Native
        {
        public:
            SessionImplNative (std::shared_ptr<Client> c, const String& x) : client (c),  name (x) {}

            String getName() const override
            {
                return name;
            }

            std::unique_ptr<ump::Input::Impl::Native> connectInput (ump::DisconnectionListener& listener,
                                                                    const ump::EndpointId& id,
                                                                    ump::PacketProtocol protocol,
                                                                    ump::Consumer& consumer) override
            {
                return InputImplNative::open (client, listener, id, protocol, consumer);
            }

            std::unique_ptr<ump::Output::Impl::Native> connectOutput (ump::DisconnectionListener& listener,
                                                                      const ump::EndpointId& id) override
            {
                return OutputImplNative::open (client, listener, id);
            }

        private:
            std::shared_ptr<Client> client;
            String name;
        };

        class EndpointsImplNative : public ump::Endpoints::Impl::Native
        {
        public:
            ump::Backend getBackend() const override
            {
                return ump::Backend::winrt;
            }

            bool isVirtualMidiUmpServiceActive() const override
            {
                return false;
            }

            bool isVirtualMidiBytestreamServiceActive() const override
            {
                return false;
            }

            void getEndpoints (std::vector<ump::EndpointId>& x) const override
            {
                client->getEndpoints (x);
            }

            std::optional<ump::Endpoint> getEndpoint (const ump::EndpointId& x) const override
            {
                return client->getEndpoint (x);
            }

            std::optional<ump::StaticDeviceInfo> getStaticDeviceInfo (const ump::EndpointId& x) const override
            {
                return client->getStaticDeviceInfo (x);
            }

            std::unique_ptr<ump::Session::Impl::Native> makeSession (const String& x) override
            {
                return std::make_unique<SessionImplNative> (client, x);
            }

            static std::unique_ptr<EndpointsImplNative> make (ump::EndpointsListener& l)
            {
                return rawToUniquePtr (new EndpointsImplNative { l });
            }

        private:
            explicit EndpointsImplNative (ump::EndpointsListener& l)
                : client (Client::make (l)) {}

            std::shared_ptr<Client> client;
        };
    };
   #endif

    struct Win32
    {
        struct ITraits
        {
            using Ptr = HMIDIIN;
            using Caps = MIDIINCAPS;
            static inline const auto getNum = midiInGetNumDevs;
            static inline const auto getCaps = midiInGetDevCaps;
            static inline const auto message = midiInMessage;
            static inline const auto isInput = true;
        };

        struct OTraits
        {
            using Ptr = HMIDIOUT;
            using Caps = MIDIOUTCAPS;
            static inline const auto getNum = midiOutGetNumDevs;
            static inline const auto getCaps = midiOutGetDevCaps;
            static inline const auto message = midiOutMessage;
            static inline const auto isInput = false;
        };

        template <typename Traits>
        static void getAvailableDevices (const Traits&, std::vector<ump::EndpointAndStaticInfo>& result)
        {
            const auto deviceCaps = [&]
            {
                std::vector<typename Traits::Caps> devices;

                const auto end = Traits::getNum();

                for (auto i = (decltype (end)) 0; i < end; ++i)
                    if (typename Traits::Caps mc{}; Traits::getCaps (i, &mc, sizeof (mc)) == MMSYSERR_NOERROR)
                        devices.push_back (mc);

                return devices;
            }();

            StringArray deviceIDs, deviceNames;

            for (const auto [i, device] : enumerate (deviceCaps))
            {
                const auto name = device.szPname;
                const auto identifier = std::invoke ([index = i]() -> String
                {
                    ULONG size = 0;

                    if (Traits::message ((typename Traits::Ptr) index, DRV_QUERYDEVICEINTERFACESIZE, (DWORD_PTR) &size, 0) != MMSYSERR_NOERROR)
                        return {};

                    WCHAR interfaceName[512]{};

                    if (! isPositiveAndBelow (size, std::size (interfaceName))
                        || Traits::message ((typename Traits::Ptr) index, DRV_QUERYDEVICEINTERFACE, (DWORD_PTR) interfaceName, sizeof (interfaceName)) != MMSYSERR_NOERROR)
                    {
                        return {};
                    }

                    return interfaceName;
                });

                deviceNames.add (name);
                deviceIDs.add (identifier.isNotEmpty() ? identifier : name);
            }

            for (auto* list : { &deviceIDs, &deviceNames })
                list->appendNumbersToDuplicates (false, false, CharPointer_UTF8 ("-"), CharPointer_UTF8 (""));

            constexpr auto direction = Traits::isInput ? ump::BlockDirection::sender : ump::BlockDirection::receiver;

            for (auto [index, id] : enumerate (deviceIDs, int{}))
            {
                const auto fullInfo = ump::IOHelpers::makeProxyEndpoint ({ deviceNames[index], id }, direction);
                result.push_back (fullInfo);
            }
        }

        class MidiHeader
        {
        public:
            void prepare (HMIDIIN device)
            {
                hdr = {};
                hdr.lpData = data.data();
                hdr.dwBufferLength = (DWORD) data.size();

                midiInPrepareHeader (device, &hdr, sizeof (hdr));
            }

            void unprepare (HMIDIIN device)
            {
                if ((hdr.dwFlags & WHDR_DONE) != 0)
                {
                    int c = 10;
                    while (--c >= 0 && midiInUnprepareHeader (device, &hdr, sizeof (hdr)) == MIDIERR_STILLPLAYING)
                        Thread::sleep (20);

                    jassert (c >= 0);
                }
            }

            void write (HMIDIIN device)
            {
                hdr.dwBytesRecorded = 0;
                midiInAddBuffer (device, &hdr, sizeof (hdr));
            }

            void writeIfFinished (HMIDIIN device)
            {
                if ((hdr.dwFlags & WHDR_DONE) != 0)
                    write (device);
            }

        private:
            MIDIHDR hdr{};
            std::array<char, 256> data{};
        };

        template <typename Device, auto openInternal>
        class DeviceCache
        {
        public:
            std::shared_ptr<Device> open (const ump::EndpointId& id)
            {
                const std::scoped_lock lock { mutex };

                if (const auto iter = devices.find (id); iter != devices.end())
                    if (auto strong = iter->second.lock())
                        return strong;

                std::unique_ptr uniqueDevice = openInternal (id);
                std::shared_ptr sharedDevice { std::move (uniqueDevice) };
                devices.emplace (id, sharedDevice);
                return sharedDevice;
            }

        private:
            std::mutex mutex;
            std::map<ump::EndpointId, std::weak_ptr<Device>> devices;
        };

        // A device can only be opened by one client at a time. In order to allow multiple inputs
        // to open the same device in JUCE, we share the device between all inputs that are
        // currently using it.
        class InputDevice : private AsyncUpdater
        {
        public:
            static std::shared_ptr<InputDevice> open (const ump::EndpointId& id)
            {
                static DeviceCache<InputDevice, &InputDevice::openInternal> devices;
                return devices.open (id);
            }

            ~InputDevice() override
            {
                allInputs().remove (*this);

                if (deviceHandle == nullptr)
                    return;

                unprepareAllHeaders();

                midiInReset (deviceHandle);
                midiInStop (deviceHandle);

                for (int count = 5; --count >= 0;)
                {
                    if (midiInClose (deviceHandle) == MMSYSERR_NOERROR)
                        break;

                    Sleep (20);
                }
            }

            ump::EndpointId getEndpointId() const
            {
                return endpointId;
            }

            void addConsumer (ump::Consumer& c)
            {
                consumers.add (c);
            }

            void removeConsumer (ump::Consumer& c)
            {
                consumers.add (c);
            }

            void addDisconnectListener (ump::DisconnectionListener& l)
            {
                disconnectListeners.add (&l);
            }

            void removeDisconnectListener (ump::DisconnectionListener& l)
            {
                disconnectListeners.remove (&l);
            }

        private:
            static std::unique_ptr<InputDevice> openInternal (const ump::EndpointId& id)
            {
                std::vector<ump::EndpointAndStaticInfo> endpoints;
                getAvailableDevices (ITraits{}, endpoints);
                const auto iter = std::find_if (endpoints.begin(),
                                                endpoints.end(),
                                                [&] (const auto& x) { return x.id == id; });

                if (iter == endpoints.end())
                    return {};

                const auto deviceID = std::distance (endpoints.begin(), iter);
                auto result = rawToUniquePtr (new InputDevice { id });

                HMIDIIN handle{};
                const auto err = midiInOpen (&handle,
                                             (UINT) deviceID,
                                             (DWORD_PTR) &midiInCallback,
                                             (DWORD_PTR) result.get(),
                                             CALLBACK_FUNCTION);

                if (err != MMSYSERR_NOERROR)
                    return {};

                result->deviceHandle = handle;

                for (auto& header : result->headers)
                {
                    header.prepare (handle);
                    header.write (handle);
                }

                if (midiInStart (handle) != MMSYSERR_NOERROR)
                    return {};

                return result;
            }

            explicit InputDevice (const ump::EndpointId& x)
                : endpointId (x)
            {
                allInputs().add (*this);
            }

            void handleMessage (const uint8* bytes, uint32 timeStamp)
            {
                if (bytes[0] >= 0x80)
                {
                    auto len = MidiMessage::getMessageLengthFromFirstByte (bytes[0]);
                    auto time = convertTimeStamp (timeStamp);
                    dispatcher.dispatch ({ unalignedPointerCast<const std::byte*> (bytes), (size_t) len }, time, [this] (const ump::View& view, double timestamp)
                    {
                        const ump::Iterator b { view.data(), view.size() };
                        const auto e = std::next (b);
                        consumers.call ([&] (ump::Consumer& c) { c.consume (b, e, timestamp); });
                    });

                    writeFinishedBlocks();
                }
            }

            void handleSysEx (MIDIHDR* hdr, uint32 timeStamp)
            {
                if (hdr->dwBytesRecorded > 0)
                {
                    auto time = convertTimeStamp (timeStamp);
                    dispatcher.dispatch ({ unalignedPointerCast<const std::byte*> (hdr->lpData), (size_t) hdr->dwBytesRecorded }, time, [this] (const ump::View& view, double timestamp)
                    {
                        const ump::Iterator b { view.data(), view.size() };
                        const auto e = std::next (b);
                        consumers.call ([&] (ump::Consumer& c) { c.consume (b, e, timestamp); });
                    });

                    writeFinishedBlocks();
                }
            }

            void disconnected()
            {
                triggerAsyncUpdate();
            }

            void handleAsyncUpdate() override
            {
                disconnectListeners.call ([] (auto& x) { x.disconnected(); });
            }

            void writeFinishedBlocks()
            {
                for (auto& header : headers)
                    header.writeIfFinished (deviceHandle);
            }

            void unprepareAllHeaders()
            {
                for (auto& header : headers)
                    header.unprepare (deviceHandle);
            }

            double convertTimeStamp (uint32 timeStamp)
            {
                auto t = startTime + timeStamp;
                auto now = Time::getMillisecondCounterHiRes();

                if (t > now)
                {
                    if (t > now + 2.0)
                        startTime -= 1.0;

                    t = now;
                }

                return t * 0.001;
            }

            static void CALLBACK midiInCallback (HMIDIIN,
                                                 UINT uMsg,
                                                 DWORD_PTR dwInstance,
                                                 DWORD_PTR midiMessage,
                                                 DWORD_PTR timeStamp)
            {
                auto* collector = reinterpret_cast<InputDevice*> (dwInstance);

                allInputs().call ([&] (auto& l)
                {
                    if (collector != &l)
                        return;

                    switch (uMsg)
                    {
                        case MIM_DATA:
                            l.handleMessage ((const uint8*) &midiMessage, (uint32) timeStamp);
                            break;

                        case MIM_LONGDATA:
                            l.handleSysEx ((MIDIHDR*) midiMessage, (uint32) timeStamp);
                            break;

                        case MIM_CLOSE:
                            l.disconnected();
                            break;
                    }
                });
            }

            static WaitFreeListeners<InputDevice>& allInputs()
            {
                static WaitFreeListeners<InputDevice> result;
                return result;
            }

            ump::EndpointId endpointId;
            HMIDIIN deviceHandle = nullptr;
            std::array<MidiHeader, 32> headers;
            double startTime = Time::getMillisecondCounterHiRes();
            WaitFreeListeners<ump::Consumer> consumers;
            ListenerList<ump::DisconnectionListener> disconnectListeners;
            // The shared input always converts to plain MIDI 1.0. Clients that want MIDI 2.0 have
            // their own converters.
            ump::BytestreamToUMPDispatcher dispatcher { 0, ump::PacketProtocol::MIDI_1_0, 4096 };

            JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (InputDevice)
        };

        class OutputDevice : private AsyncUpdater
        {
        public:
            ~OutputDevice() override
            {
                allOutputs().remove (*this);

                cancelPendingUpdate();

                if (handle != nullptr)
                    midiOutClose (handle);
            }

            static std::shared_ptr<OutputDevice> open (const ump::EndpointId& id)
            {
                static DeviceCache<OutputDevice, &OutputDevice::openInternal> devices;
                return devices.open (id);
            }

            ump::EndpointId getEndpointId() const
            {
                return endpointId;
            }

            bool send (ump::Iterator b, ump::Iterator e)
            {
                for (const auto& view : makeRange (b, e))
                {
                    toBytestream.convert (view, 0, [&] (ump::BytesOnGroup bytesView, double)
                    {
                        sendBytestream (bytesView.bytes);
                    });
                }

                return true;
            }

            void addDisconnectListener (ump::DisconnectionListener& l)
            {
                disconnectListeners.add (&l);
            }

            void removeDisconnectListener (ump::DisconnectionListener& l)
            {
                disconnectListeners.remove (&l);
            }

        private:
            static std::unique_ptr<OutputDevice> openInternal (const ump::EndpointId& id)
            {
                std::vector<ump::EndpointAndStaticInfo> endpoints;
                getAvailableDevices (OTraits{}, endpoints);
                const auto iter = std::find_if (endpoints.begin(),
                                                endpoints.end(),
                                                [&] (const auto& x) { return x.id == id; });

                if (iter == endpoints.end())
                    return {};

                const auto deviceID = std::distance (endpoints.begin(), iter);

                for (auto i = 0; i < 4; ++i)
                {
                    auto result = rawToUniquePtr (new OutputDevice (id));

                    HMIDIOUT h = nullptr;
                    auto res = midiOutOpen (&h,
                                            (UINT) deviceID,
                                            (DWORD_PTR) &midiOutCallback,
                                            (DWORD_PTR) result.get(),
                                            CALLBACK_FUNCTION);

                    switch (res)
                    {
                        case MMSYSERR_NOERROR:
                            result->handle = h;
                            return result;

                        case MMSYSERR_ALLOCATED:
                            Sleep (100);
                            break;

                        default:
                            return {};
                    }
                }

                return {};
            }

            explicit OutputDevice (const ump::EndpointId& x)
                : endpointId (x)
            {
                allOutputs().add (*this);
            }

            void sendBytestream (Span<const std::byte> message)
            {
                if (message.empty())
                    return;

                if (message.size() > 3 || message[0] == std::byte { 0xf0 })
                {
                    MIDIHDR h = {};

                    h.lpData = (char*) message.data();
                    h.dwBytesRecorded = h.dwBufferLength  = (DWORD) message.size();

                    if (midiOutPrepareHeader (handle, &h, sizeof (MIDIHDR)) == MMSYSERR_NOERROR)
                    {
                        auto res = midiOutLongMsg (handle, &h, sizeof (MIDIHDR));

                        if (res == MMSYSERR_NOERROR)
                        {
                            while ((h.dwFlags & MHDR_DONE) == 0)
                                Sleep (1);

                            int count = 500; // 1 sec timeout

                            while (--count >= 0)
                            {
                                res = midiOutUnprepareHeader (handle, &h, sizeof (MIDIHDR));

                                if (res == MIDIERR_STILLPLAYING)
                                    Sleep (2);
                                else
                                    break;
                            }
                        }
                    }
                }
                else
                {
                    const auto msg = ByteOrder::makeInt (0 < message.size() ? (uint8_t) message[0] : 0,
                                                         1 < message.size() ? (uint8_t) message[1] : 0,
                                                         2 < message.size() ? (uint8_t) message[2] : 0,
                                                         0);

                    for (int i = 0; i < 50; ++i)
                    {
                        if (midiOutShortMsg (handle, msg) != MIDIERR_NOTREADY)
                            break;

                        Sleep (1);
                    }
                }
            }

            void disconnected()
            {
                triggerAsyncUpdate();
            }

            void handleAsyncUpdate() override
            {
                disconnectListeners.call ([] (auto& x) { x.disconnected(); });
            }

            static void CALLBACK midiOutCallback (HMIDIOUT,
                                                  UINT wMsg,
                                                  DWORD_PTR dwInstance,
                                                  DWORD_PTR,
                                                  DWORD_PTR)
            {
                auto* collector = reinterpret_cast<OutputDevice*> (dwInstance);

                allOutputs().call ([&] (auto& l)
                {
                    if (collector != &l)
                        return;

                    switch (wMsg)
                    {
                        case MOM_CLOSE:
                            l.disconnected();
                            break;
                    }
                });
            }

            static WaitFreeListeners<OutputDevice>& allOutputs()
            {
                static WaitFreeListeners<OutputDevice> result;
                return result;
            }

            ump::EndpointId endpointId;
            HMIDIOUT handle = nullptr;
            ListenerList<ump::DisconnectionListener> disconnectListeners;
            ump::ToBytestreamConverter toBytestream { 4096 };

            JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OutputDevice)
        };

        class InputImplNative : public ump::Input::Impl::Native,
                                private ump::Consumer
        {
        public:
            InputImplNative (std::shared_ptr<InputDevice> d,
                             ump::DisconnectionListener& l,
                             ump::PacketProtocol p,
                             Consumer& c)
                : device (d), listener (l), consumer (c), converter (p)
            {
                device->addConsumer (*this);
                device->addDisconnectListener (listener);
            }

            ~InputImplNative() override
            {
                device->removeDisconnectListener (listener);
                device->removeConsumer (*this);
            }

            ump::EndpointId getEndpointId() const override
            {
                return device->getEndpointId();
            }

            ump::PacketProtocol getProtocol() const override
            {
                return converter.getProtocol();
            }

        private:
            void consume (ump::Iterator b, ump::Iterator e, double time) override
            {
                converter.convert (b, e, [&] (ump::View v)
                {
                    const ump::Iterator iter { v.data(), v.size() };
                    consumer.consume (iter, std::next (iter), time);
                });
            }

            std::shared_ptr<InputDevice> device;
            ump::DisconnectionListener& listener;
            Consumer& consumer;
            ump::GenericUMPConverter converter;
        };

        class SessionImplNative : public ump::Session::Impl::Native
        {
        public:
            explicit SessionImplNative (const String& x) : name (x) {}

            String getName() const override
            {
                return name;
            }

            std::unique_ptr<ump::Input::Impl::Native> connectInput (ump::DisconnectionListener& listener,
                                                                    const ump::EndpointId& endpoint,
                                                                    ump::PacketProtocol protocol,
                                                                    ump::Consumer& consumer) override
            {
                auto device = InputDevice::open (endpoint);

                if (device == nullptr)
                    return {};

                return std::make_unique<InputImplNative> (device, listener, protocol, consumer);
            }

            std::unique_ptr<ump::Output::Impl::Native> connectOutput (ump::DisconnectionListener& listener,
                                                                      const ump::EndpointId& endpoint) override
            {
                class Result : public ump::Output::Impl::Native
                {
                public:
                    Result (std::shared_ptr<OutputDevice> d, ump::DisconnectionListener& l)
                        : device (d), listener (l)
                    {
                        device->addDisconnectListener (listener);
                    }

                    ~Result() override
                    {
                        device->removeDisconnectListener (listener);
                    }

                    ump::EndpointId getEndpointId() const override { return device->getEndpointId(); }
                    bool send (ump::Iterator b, ump::Iterator e) override { return device->send (b, e); }

                private:
                    std::shared_ptr<OutputDevice> device;
                    ump::DisconnectionListener& listener;
                };

                auto device = OutputDevice::open (endpoint);

                if (device == nullptr)
                    return {};

                return std::make_unique<Result> (device, listener);
            }

        private:
            String name;
        };

        class EndpointsImplNative : public ump::Endpoints::Impl::Native
        {
        public:
            ump::Backend getBackend() const override
            {
                return ump::Backend::winmm;
            }

            bool isVirtualMidiUmpServiceActive() const override
            {
                return false;
            }

            bool isVirtualMidiBytestreamServiceActive() const override
            {
                return false;
            }

            void getEndpoints (std::vector<ump::EndpointId>& storage) const override
            {
                std::transform (cachedEndpoints.begin(),
                                cachedEndpoints.end(),
                                std::back_inserter (storage),
                                [] (auto& p) { return p.first; });
            }

            std::optional<ump::Endpoint> getEndpoint (const ump::EndpointId& x) const override
            {
                const auto iter = cachedEndpoints.find (x);

                if (iter != cachedEndpoints.end())
                    return iter->second.endpoint;

                return {};
            }

            std::optional<ump::StaticDeviceInfo> getStaticDeviceInfo (const ump::EndpointId& x) const override
            {
                const auto iter = cachedEndpoints.find (x);

                if (iter != cachedEndpoints.end())
                    return iter->second.info;

                return {};
            }

            std::unique_ptr<ump::Session::Impl::Native> makeSession (const String& x) override
            {
                return std::make_unique<SessionImplNative> (x);
            }

            static std::unique_ptr<EndpointsImplNative> make (ump::EndpointsListener& listener)
            {
                return rawToUniquePtr (new EndpointsImplNative { listener });
            }

        private:
            explicit EndpointsImplNative (ump::EndpointsListener& l)
                : listener (l)
            {
                updateCachedEndpoints();
            }

            void updateCachedEndpoints()
            {
                JUCE_ASSERT_MESSAGE_THREAD;

                std::vector<ump::EndpointAndStaticInfo> buffer;
                getAvailableDevices (ITraits{}, buffer);
                getAvailableDevices (OTraits{}, buffer);

                cachedEndpoints.clear();

                for (auto& item : buffer)
                    cachedEndpoints.emplace (item.id, item);

                // If this is hit, we got an identical ID for an input and output device
                jassert (cachedEndpoints.size() == buffer.size());
            }

            ump::EndpointsListener& listener;
            std::map<ump::EndpointId, ump::EndpointAndStaticInfo> cachedEndpoints;
            DeviceChangeDetector detector { L"JuceMidiDeviceDetector_", [&]
            {
                updateCachedEndpoints();
                listener.endpointsChanged();
            } };
        };
    };
};

auto ump::Endpoints::Impl::Native::make (EndpointsListener& l) -> std::unique_ptr<Native>
{
   #if JUCE_USE_WINDOWS_MIDI_SERVICES
    if (auto ptr = MidiServices::makeEndpoints (l))
        return ptr;
   #endif

   #if JUCE_USE_WINRT_MIDI
    if (auto winrtSession = WindowsMidiHelpers::Winrt::EndpointsImplNative::make (l))
        return winrtSession;
   #endif

   #if JUCE_FORCE_WINRT_MIDI
    return nullptr;
   #else
    return WindowsMidiHelpers::Win32::EndpointsImplNative::make (l);
   #endif
}

} // namespace juce

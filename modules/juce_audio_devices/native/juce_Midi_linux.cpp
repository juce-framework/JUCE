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

#if JUCE_ALSA

struct AlsaMidiHelpers
{
    struct InputCallback
    {
        virtual ~InputCallback() = default;
        virtual void pushUmp (int port, ump::View, double time) = 0;
        virtual void pushBytes (int port, ump::BytestreamMidiView) = 0;
    };

    struct PortExitCallback
    {
        virtual ~PortExitCallback() = default;
        virtual void portExit (snd_seq_addr_t port) = 0;
    };

    struct PortsChangedCallback
    {
        virtual ~PortsChangedCallback() = default;
        virtual void notifyPortsChanged() = 0;
    };

    class UpdateNotifier : private AsyncUpdater
    {
    public:
        explicit UpdateNotifier (PortsChangedCallback& c) : cb (c) {}
        ~UpdateNotifier() override { cancelPendingUpdate(); }
        using AsyncUpdater::triggerAsyncUpdate;

    private:
        void handleAsyncUpdate() override { cb.notifyPortsChanged(); }

        PortsChangedCallback& cb;
    };

    class SequencerThread
    {
    public:
        SequencerThread (snd_seq_t* h, InputCallback& cb, PortsChangedCallback& pc, PortExitCallback& pe)
            : seqHandle (h), inputCallback (cb), portExit (pe), notifier (pc) {}

        ~SequencerThread() noexcept
        {
            shouldStop = true;
            thread.join();

            if (0 <= queueId)
            {
                snd_seq_stop_queue (seqHandle, queueId, nullptr);
                snd_seq_free_queue (seqHandle, queueId);
            }
        }

        int getQueueId() const
        {
            return queueId;
        }

    private:
        static bool isUmp (const snd_seq_ump_event_t* ev) { return (ev->flags & (1 << 5)) != 0; }

        template <typename Event,
                  std::enable_if_t<   std::is_same_v<Event, snd_seq_event_t>
                                   || std::is_same_v<Event, snd_seq_ump_event_t>, int> = 0>
        double computeTimestampWithConvertedBase (const Event* ev) const
        {
            // We asked for wallclock timestamps - if the incoming event doesn't comply, then
            // we'll have to approximate a timestamp ourselves.
            if (snd_seq_ev_is_direct (ev) || ! snd_seq_ev_is_real (ev))
                return Time::getMillisecondCounter() * 0.001;

            const auto nativeTime = ev->time.time;
            const auto initialNanos = (double) startTimeNative.tv_sec * 1e9 + (double) startTimeNative.tv_nsec;
            const auto currentNanos = (double) nativeTime.tv_sec * 1e9 + (double) nativeTime.tv_nsec;
            const auto elapsedNanos = currentNanos - initialNanos;
            const auto elapsedMillis = elapsedNanos / 1e6;

            // Perhaps this could happen if creating the queue failed, or if the event timestamp isn't
            // populated for some other reason.
            if (elapsedMillis <= 0)
                return Time::getMillisecondCounter() * 0.001;

            return ((double) startTimeMillis + elapsedMillis) * 0.001;
        }

        void processEvent (Span<std::byte> buffer, snd_midi_event_t* midiParser)
        {
            constexpr int systemEvents[]
            {
                SND_SEQ_EVENT_CLIENT_CHANGE,
                SND_SEQ_EVENT_CLIENT_START,
                SND_SEQ_EVENT_CLIENT_EXIT,
                SND_SEQ_EVENT_PORT_CHANGE,
                SND_SEQ_EVENT_PORT_START,
                SND_SEQ_EVENT_PORT_EXIT,
                SND_SEQ_EVENT_PORT_SUBSCRIBED,
                SND_SEQ_EVENT_PORT_UNSUBSCRIBED,
            };

            const auto umpEvent = std::invoke ([&]
            {
                snd_seq_ump_event_t* ev = nullptr;
                return snd_seq_ump_event_input != nullptr && snd_seq_ump_event_input (seqHandle, &ev) >= 0 ? ev : nullptr;
            });

            if (umpEvent != nullptr && isUmp (umpEvent))
            {
                inputCallback.pushUmp (umpEvent->dest.port,
                                       ump::View { umpEvent->ump },
                                       computeTimestampWithConvertedBase (umpEvent));
                return;
            }

            const auto seqEvent = std::invoke ([&]
            {
                if (umpEvent != nullptr)
                    return reinterpret_cast<snd_seq_event_t*> (umpEvent);

                snd_seq_event_t* ev = nullptr;
                return snd_seq_event_input (seqHandle, &ev) >= 0 ? ev : nullptr;
            });

            if (seqEvent == nullptr)
                return;

            const ScopeGuard freeInputEvent { [&] { snd_seq_free_event (seqEvent); } };

            const auto foundEvent = std::find (std::begin (systemEvents),
                                               std::end   (systemEvents),
                                               seqEvent->type);

            if (foundEvent != std::end (systemEvents))
            {
                notifier.triggerAsyncUpdate();

                if (seqEvent->type == SND_SEQ_EVENT_PORT_EXIT)
                    portExit.portExit (seqEvent->data.addr);

                return;
            }

            // Disable running status for decoded MIDI messages
            snd_midi_event_no_status (midiParser, 1);

            // xxx what about SYSEXes that are too big for the buffer?
            const auto numBytes = snd_midi_event_decode (midiParser,
                                                         unalignedPointerCast<unsigned char*> (buffer.data()),
                                                         (long) buffer.size(),
                                                         seqEvent);

            snd_midi_event_reset_decode (midiParser);

            if (numBytes < 0)
            {
                // UMP messages may not convert to MIDI 1.0 events, in which case decoding will
                // return -ENOENT. This is permissible, but other failures probably indicate a real
                // problem.
                jassert (numBytes == -ENOENT);
                return;
            }

            const Span bytes (buffer.data(), (size_t) numBytes);
            inputCallback.pushBytes (seqEvent->dest.port,
                                     { bytes, computeTimestampWithConvertedBase (seqEvent) });
        }

        static constexpr auto maxEventSize = 16 * 1024;
        snd_seq_t* seqHandle = nullptr;
        int queueId = seqHandle != nullptr ? snd_seq_alloc_queue (seqHandle) : -1;
        InputCallback& inputCallback;
        PortExitCallback& portExit;
        std::atomic<bool> shouldStop { false };
        UpdateNotifier notifier;

        const snd_seq_real_time_t startTimeNative = std::invoke ([&]
        {
            if (queueId < 0)
                return snd_seq_real_time_t{};

            snd_seq_start_queue (seqHandle, queueId, nullptr);
            snd_seq_drain_output (seqHandle);

            snd_seq_queue_status_t* queueStatus{};
            snd_seq_queue_status_alloca (&queueStatus);

            if (snd_seq_get_queue_status (seqHandle, queueId, queueStatus) != 0)
                return snd_seq_real_time_t{};

            return *snd_seq_queue_status_get_real_time (queueStatus);
        });

        const uint32_t startTimeMillis = Time::getMillisecondCounter();

        std::thread thread { [this]
        {
            Thread::setCurrentThreadName (SystemStats::getJUCEVersion() + ": ALSA MIDI Input");

            snd_midi_event_t* midiParser;

            if (snd_midi_event_new (maxEventSize, &midiParser) < 0)
                return;

            const ScopeGuard freeMidiEvent { [&] { snd_midi_event_free (midiParser); } };

            const auto numPfds = snd_seq_poll_descriptors_count (seqHandle, POLLIN);
            std::vector<pollfd> pfd (static_cast<size_t> (numPfds));
            snd_seq_poll_descriptors (seqHandle, pfd.data(), (unsigned int) numPfds, POLLIN);

            std::vector<std::byte> buffer (maxEventSize);

            while (! shouldStop)
            {
                // This timeout shouldn't be too long, so that the program can exit in a timely manner
                if (poll (pfd.data(), (nfds_t) numPfds, 100) <= 0)
                    continue;

                if (shouldStop)
                    break;

                do
                {
                    processEvent (buffer, midiParser);
                }
                while (snd_seq_event_input_pending (seqHandle, 0) > 0);
            }
        } };
    };

    static String getFormattedPortIdentifier (int clientId, int portId)
    {
        return String (clientId) + "-" + String (portId);
    }

    template <size_t N>
    static constexpr std::array<std::byte, N> makeBytesLittleEndian (unsigned int b)
    {
        std::array<std::byte, N> result;

        for (size_t i = 0; i != N; ++i)
        {
            result[i] = std::byte { (uint8_t) b };
            b >>= 8;
        }

        return result;
    }

    static constexpr unsigned int fromBytesLittleEndian (Span<const std::byte> bytes)
    {
        unsigned int result{};

        for (auto& byte : bytes)
        {
            result <<= 8;
            result |= (uint8_t) byte;
        }

        return result;
    }

    struct AlsaClientInfo
    {
        static AlsaClientInfo makeUmpEndpoint (snd_seq_t* seq,
                                               snd_seq_client_info_t* client,
                                               const snd_ump_endpoint_info_t* endpoint)
        {
            const auto clientId = snd_seq_client_info_get_client (client);
            const auto portId = 0;

            const auto protocol = snd_ump_endpoint_info_get_protocol (endpoint) == SND_UMP_EP_INFO_PROTO_MIDI2
                                ? ump::PacketProtocol::MIDI_2_0
                                : ump::PacketProtocol::MIDI_1_0;

            const auto legacyId = getFormattedPortIdentifier (clientId, portId);
            const auto manufacturer = snd_ump_endpoint_info_get_manufacturer_id (endpoint);
            const auto family = snd_ump_endpoint_info_get_family_id (endpoint);
            const auto model = snd_ump_endpoint_info_get_model_id (endpoint);
            const auto revisionPtr = snd_ump_endpoint_info_get_sw_revision (endpoint);

            std::array<std::byte, 4> revision;
            std::transform (revisionPtr, revisionPtr + 4, revision.data(), [] (auto x) { return std::byte { x }; });

            const ump::DeviceInfo deviceInfo { makeBytesLittleEndian<3> (manufacturer),
                                               makeBytesLittleEndian<2> (family),
                                               makeBytesLittleEndian<2> (model),
                                               revision };

            const auto version = makeBytesLittleEndian<2> (snd_ump_endpoint_info_get_version (endpoint));
            const auto flags = snd_ump_endpoint_info_get_flags (endpoint);
            const auto caps = snd_ump_endpoint_info_get_protocol_caps (endpoint);

            std::array<ump::Block, 32> blocks;
            size_t numPushedBlocks = 0;
            const auto pushBlock = [&] (ump::Block x) { blocks[numPushedBlocks++] = x; };

            const auto numBlocks = snd_ump_endpoint_info_get_num_blocks (endpoint);
            snd_ump_block_info_t* block = nullptr;
            snd_ump_block_info_alloca (&block);

            for (auto i = decltype (numBlocks){}; i < numBlocks; ++i)
            {
                if (snd_seq_get_ump_block_info (seq, clientId, (int) i, block) != 0)
                    continue;

                const auto uiHint = std::invoke ([&]
                {
                    switch (snd_ump_block_info_get_ui_hint (block))
                    {
                        case SND_UMP_BLOCK_UI_HINT_BOTH: return ump::BlockUiHint::bidirectional;
                        case SND_UMP_BLOCK_UI_HINT_SENDER: return ump::BlockUiHint::sender;
                        case SND_UMP_BLOCK_UI_HINT_RECEIVER: return ump::BlockUiHint::receiver;
                        case SND_UMP_BLOCK_UI_HINT_UNKNOWN: break;
                    }

                    return ump::BlockUiHint::unknown;
                });

                const auto blockFlags = snd_ump_block_info_get_flags (block);

                const auto proxy = std::invoke ([&]
                {
                    if (blockFlags & SND_UMP_BLOCK_IS_MIDI1)
                    {
                        if (blockFlags & SND_UMP_BLOCK_IS_LOWSPEED)
                            return ump::BlockMIDI1ProxyKind::restrictedBandwidth;

                        return ump::BlockMIDI1ProxyKind::unrestrictedBandwidth;
                    }

                    return ump::BlockMIDI1ProxyKind::inapplicable;
                });

                const auto direction = std::invoke ([&]
                {
                    switch (snd_ump_block_info_get_direction (block))
                    {
                        case SND_UMP_DIR_INPUT: return ump::BlockDirection::receiver;
                        case SND_UMP_DIR_OUTPUT: return ump::BlockDirection::sender;
                        case SND_UMP_DIR_BIDIRECTION: return ump::BlockDirection::bidirectional;
                    }

                    return ump::BlockDirection::unknown;
                });

                const auto b = ump::Block{}.withName (snd_ump_block_info_get_name (block))
                                           .withFirstGroup ((uint8_t) snd_ump_block_info_get_first_group (block))
                                           .withNumGroups ((uint8_t) snd_ump_block_info_get_num_groups (block))
                                           .withMaxSysex8Streams ((uint8_t) snd_ump_block_info_get_sysex8_streams (block))
                                           .withEnabled (snd_ump_block_info_get_active (block))
                                           .withUiHint (uiHint)
                                           .withMIDI1ProxyKind (proxy)
                                           .withDirection (direction);
                pushBlock (b);
            }

            const auto ep = ump::Endpoint{}.withName (snd_seq_client_info_get_name (client))
                                           .withProtocol (protocol)
                                           .withDeviceInfo (deviceInfo)
                                           .withProductInstanceId (snd_ump_endpoint_info_get_product_id (endpoint))
                                           .withUMPVersion ((uint8_t) version[1], (uint8_t) version[0])
                                           .withStaticBlocks (flags & SND_UMP_EP_INFO_STATIC_BLOCKS)
                                           .withMidi1Support (caps & SND_UMP_EP_INFO_PROTO_MIDI1)
                                           .withMidi2Support (caps & SND_UMP_EP_INFO_PROTO_MIDI2)
                                           .withReceiveJRSupport (caps & SND_UMP_EP_INFO_PROTO_JRTS_RX)
                                           .withTransmitJRSupport (caps & SND_UMP_EP_INFO_PROTO_JRTS_TX)
                                           .withBlocks (Span { blocks.data(), numPushedBlocks });

            return { clientId,
                     portId,
                     { ep, getStaticDeviceInfo (seq, client), ump::EndpointId::makeSrcDst (legacyId, legacyId) } };
        }

        static ump::StaticDeviceInfo getStaticDeviceInfo (snd_seq_t* seq, snd_seq_client_info_t* client)
        {
            const auto clientId = snd_seq_client_info_get_client (client);

            const auto numPorts = snd_seq_client_info_get_num_ports (client);
            snd_seq_port_info_t* port = nullptr;
            snd_seq_port_info_alloca (&port);

            std::array<String, 16> identifiersSrc, identifiersDst;

            for (auto i = decltype (numPorts){}; i < numPorts; ++i)
            {
                if (snd_seq_get_any_port_info (seq, clientId, i, port) != 0)
                    continue;

                const auto groupIndexFrom1 = snd_seq_port_info_get_ump_group (port);

                if (groupIndexFrom1 == 0)
                    continue;

                const auto identifier = getFormattedPortIdentifier (clientId, i);
                const auto d = snd_seq_port_info_get_direction (port);

                const auto portCaps = snd_seq_port_info_get_capability (port);

                // Avoid giving identifiers to inactive ports, because we don't want disabled
                // ports to show up when listing MIDI 1.0 ports
                if ((portCaps & SND_SEQ_PORT_CAP_INACTIVE) != 0)
                    continue;

                if (d == SND_SEQ_PORT_DIR_BIDIRECTION || d == SND_SEQ_PORT_DIR_INPUT)
                    identifiersDst[(size_t) (groupIndexFrom1 - 1)] = identifier;

                if (d == SND_SEQ_PORT_DIR_BIDIRECTION || d == SND_SEQ_PORT_DIR_OUTPUT)
                    identifiersSrc[(size_t) (groupIndexFrom1 - 1)] = identifier;
            }

            const auto version = snd_seq_client_info_get_midi_version (client);

            const auto si = ump::StaticDeviceInfo{}.withName (snd_seq_client_info_get_name (client))
                                                   .withManufacturer ("")
                                                   .withProduct ("")
                                                   .withTransport (version == SND_SEQ_CLIENT_LEGACY_MIDI
                                                                   ? ump::Transport::bytestream
                                                                   : ump::Transport::ump)
                                                   .withLegacyIdentifiersSrc (identifiersSrc)
                                                   .withLegacyIdentifiersDst (identifiersDst);

            if (snd_seq_get_any_port_info (seq, clientId, 0, port) == 0)
            {
                return si.withHasSource (snd_seq_port_info_get_direction (port) != SND_SEQ_PORT_DIR_INPUT)
                         .withHasDestination (snd_seq_port_info_get_direction (port) != SND_SEQ_PORT_DIR_OUTPUT);
            }

            return si;
        }

        static AlsaClientInfo makeProxy (snd_seq_client_info_t* client,
                                         const snd_seq_port_info_t* port)
        {
            const auto clientId = snd_seq_client_info_get_client (client);
            const auto portId = snd_seq_port_info_get_port (port);
            const auto identifier = getFormattedPortIdentifier (clientId, portId);

            jassert (snd_seq_port_info_get_ump_group == nullptr || snd_seq_port_info_get_ump_group (port) == 0);

            const auto kind = std::invoke ([&]
            {
                if (snd_seq_port_info_get_direction == nullptr)
                {
                    const auto caps = snd_seq_port_info_get_capability (port);
                    constexpr auto mask = SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_WRITE;

                    if ((caps & mask) == mask)
                        return ump::BlockDirection::bidirectional;

                    if ((caps & mask) == SND_SEQ_PORT_CAP_READ)
                        return ump::BlockDirection::sender;

                    if ((caps & mask) == SND_SEQ_PORT_CAP_WRITE)
                        return ump::BlockDirection::receiver;

                    jassertfalse;
                    return ump::BlockDirection::unknown;
                }

                switch (snd_seq_port_info_get_direction (port))
                {
                    case SND_SEQ_PORT_DIR_INPUT:        return ump::BlockDirection::sender;
                    case SND_SEQ_PORT_DIR_OUTPUT:       return ump::BlockDirection::receiver;
                    case SND_SEQ_PORT_DIR_BIDIRECTION:  return ump::BlockDirection::bidirectional;
                }

                jassertfalse;
                return ump::BlockDirection::unknown;
            });

            const auto fullInfo = ump::IOHelpers::makeProxyEndpoint ({ snd_seq_port_info_get_name (port), identifier }, kind);

            return { clientId, portId, fullInfo };
        }

        int clientId;
        int portId;
        ump::EndpointAndStaticInfo fullInfo;
    };

    struct SeqDestructor
    {
        void operator() (snd_seq_t* ptr) const
        {
            if (ptr != nullptr)
                snd_seq_close (ptr);
        }
    };

    class Client : private InputCallback,
                   private PortExitCallback,
                   private PortsChangedCallback
    {
    public:
        static std::unique_ptr<Client> make (ump::EndpointsListener* l)
        {
            snd_seq_t* handle{};
            const auto error = snd_seq_open (&handle, "default", SND_SEQ_OPEN_DUPLEX, 0);

            if (error != 0 || handle == nullptr)
            {
                jassertfalse;
                return {};
            }

            snd_seq_nonblock (handle, SND_SEQ_NONBLOCK);
            snd_seq_set_client_name (handle, getAlsaMidiName().toRawUTF8());

            if (snd_seq_set_client_midi_version != nullptr)
                snd_seq_set_client_midi_version (handle, SND_SEQ_CLIENT_UMP_MIDI_2_0);

            return rawToUniquePtr (new Client (l, handle));
        }

        static String getAlsaMidiName()
        {
           #ifdef JUCE_ALSA_MIDI_NAME
            return JUCE_ALSA_MIDI_NAME
           #else
            return ump::Endpoints::Impl::getGlobalMidiClientName();
           #endif
        }

        snd_seq_t* getSequencer() const { return handle.get(); }

        void addInputCallback (InputCallback& c)
        {
            inputCallbacks.add (c);
        }

        void removeInputCallback (InputCallback& c)
        {
            inputCallbacks.remove (c);
        }

        void addPortExitCallback (PortExitCallback& c)
        {
            portExitCallbacks.add (c);
        }

        void removePortExitCallback (PortExitCallback& c)
        {
            portExitCallbacks.remove (c);
        }

        int getClientId() const { return clientId; }
        int getQueueId() const { return inputThread.getQueueId(); }

        String getName() const
        {
            snd_seq_client_info_t* info{};
            snd_seq_client_info_alloca (&info);
            snd_seq_get_client_info (handle.get(), info);
            return CharPointer_UTF8 { snd_seq_client_info_get_name (info) };
        }

        void getEndpoints (std::vector<ump::EndpointId>& result) const
        {
            std::transform (cachedEndpoints.begin(),
                            cachedEndpoints.end(),
                            std::back_inserter (result),
                            [] (auto& item) { return item.first; });
        }

        std::optional<AlsaClientInfo> getClientInfo (const ump::EndpointId& id) const
        {
            const auto iter = cachedEndpoints.find (id);

            if (iter == cachedEndpoints.end())
                return {};

            return iter->second;
        }

    private:
        Client (ump::EndpointsListener* l, snd_seq_t* h)
            : listener (l), handle (h)
        {
            jassert (handle != nullptr);
        }

        void pushUmp (int port, ump::View view, double time) override
        {
            inputCallbacks.call ([&] (auto& l) { l.pushUmp (port, view, time); });
        }

        void pushBytes (int port, ump::BytestreamMidiView view) override
        {
            inputCallbacks.call ([&] (auto& l) { l.pushBytes (port, view); });
        }

        void portExit (snd_seq_addr_t port) override
        {
            portExitCallbacks.call ([&] (auto& l) { l.portExit (port); });
        }

        void notifyPortsChanged() override
        {
            cachedEndpoints = findEndpoints (handle.get());

            if (listener != nullptr)
                listener->endpointsChanged();
        }

        static std::map<ump::EndpointId, AlsaClientInfo> findEndpoints (snd_seq_t* seq)
        {
            std::map<ump::EndpointId, AlsaClientInfo> result;

            snd_seq_client_info_t* clientInfo = nullptr;
            snd_seq_client_info_alloca (&clientInfo);

            snd_seq_port_info_t* portInfo = nullptr;
            snd_seq_port_info_alloca (&portInfo);

            while (snd_seq_query_next_client (seq, clientInfo) == 0)
            {
                const auto clientId = snd_seq_client_info_get_client (clientInfo);

                if (snd_seq_get_ump_endpoint_info != nullptr && snd_ump_endpoint_info_sizeof != nullptr)
                {
                    snd_ump_endpoint_info_t* endpointInfo = nullptr;
                    snd_ump_endpoint_info_alloca (&endpointInfo);

                    if (snd_seq_get_ump_endpoint_info (seq, clientId, endpointInfo) == 0)
                    {
                        // This is a UMP client, so treat this as a UMP endpoint
                        const auto info = AlsaClientInfo::makeUmpEndpoint (seq, clientInfo, endpointInfo);
                        result.emplace (info.fullInfo.id, info);
                        continue;
                    }
                }

                // This isn't a UMP endpoint, so iterate each port, creating a proxy endpoint for each
                snd_seq_port_info_set_client (portInfo, clientId);
                snd_seq_port_info_set_port (portInfo, -1);

                while (snd_seq_query_next_port (seq, portInfo) == 0)
                {
                    constexpr auto mask = SND_SEQ_PORT_CAP_SUBS_READ | SND_SEQ_PORT_CAP_SUBS_WRITE;

                    if ((snd_seq_port_info_get_capability (portInfo) & mask) == 0)
                        continue;

                    const auto info = AlsaClientInfo::makeProxy (clientInfo, portInfo);
                    result.emplace (info.fullInfo.id, info);
                }
            }

            return result;
        }

        ump::EndpointsListener* listener{};
        std::unique_ptr<snd_seq_t, SeqDestructor> handle;
        int clientId = snd_seq_client_id (handle.get());
        WaitFreeListeners<InputCallback> inputCallbacks;
        WaitFreeListeners<PortExitCallback> portExitCallbacks;
        std::map<ump::EndpointId, AlsaClientInfo> cachedEndpoints;
        SequencerThread inputThread { handle.get(), *this, *this, *this };
    };

    class Port : private InputCallback,
                 private PortExitCallback,
                 private AsyncUpdater
    {
    public:
        ~Port() override
        {
            client->removePortExitCallback (*this);
            client->removeInputCallback (*this);

            cancelPendingUpdate();

            if (portId >= 0)
            {
                if (connected.has_value())
                {
                    if (direction == ump::IOKind::src)
                        snd_seq_disconnect_from (client->getSequencer(), portId, connected->client, connected->port);
                    else
                        snd_seq_disconnect_to (client->getSequencer(), portId, connected->client, connected->port);
                }

                snd_seq_delete_simple_port (client->getSequencer(), portId);
            }
        }

        std::optional<snd_seq_addr_t> getConnected() const { return connected; }

        int getPortId() const { return jmax (0, portId); }

        ump::EndpointId getId() const
        {
            const auto id = connected.has_value()
                          ? getFormattedPortIdentifier (connected->client, connected->port)
                          : ("VIRTUAL" + getFormattedPortIdentifier (client->getClientId(), getPortId()));

            return ump::EndpointId::makeSrcDst (id, id);
        }

        ump::StaticDeviceInfo getStaticDeviceInfo() const
        {
            snd_seq_client_info_t* clientInfo{};
            snd_seq_client_info_alloca (&clientInfo);
            snd_seq_get_any_client_info (client->getSequencer(), client->getClientId(), clientInfo);
            return AlsaClientInfo::getStaticDeviceInfo (client->getSequencer(), clientInfo);
        }

        void addInputCallback (InputCallback& c)
        {
            inputCallbacks.add (c);
        }

        void removeInputCallback (InputCallback& c)
        {
            inputCallbacks.remove (c);
        }

        /*  Disconnection listener is called on the main thread. */
        void addDisconnectionListener (ump::DisconnectionListener& c)
        {
            disconnectCallbacks.add (&c);
        }

        void removeDisconnectionListener (ump::DisconnectionListener& c)
        {
            disconnectCallbacks.remove (&c);
        }

        std::shared_ptr<Client> getClient() const
        {
            return client;
        }

        bool isSrc() const { return ! direction.has_value() || *direction == ump::IOKind::src; }
        bool isDst() const { return ! direction.has_value() || *direction == ump::IOKind::dst; }

        static std::unique_ptr<Port> makeUmpEndpoint (const String& name,
                                                      const ump::DeviceInfo& info,
                                                      const String& productInstance,
                                                      ump::PacketProtocol protocol,
                                                      Span<const ump::Block> blocks,
                                                      ump::BlocksAreStatic areStatic)
        {
            if (snd_seq_create_ump_endpoint == nullptr)
                return {};

            auto virtualClient = Client::make (nullptr);

            if (virtualClient == nullptr)
                return {};

            const auto findMaxGroup = [] (auto acc, const auto& block)
            {
                return std::max (acc, block.getFirstGroup() + block.getNumGroups());
            };

            const auto numGroupsRequired = (unsigned int) std::accumulate (blocks.begin(),
                                                                           blocks.end(),
                                                                           0,
                                                                           findMaxGroup);

            const unsigned int caps = protocol == ump::PacketProtocol::MIDI_2_0
                                    ? SND_UMP_EP_INFO_PROTO_MIDI2
                                    : SND_UMP_EP_INFO_PROTO_MIDI1;

            snd_ump_endpoint_info_t* e{};
            snd_ump_endpoint_info_alloca (&e);

            snd_ump_endpoint_info_set_protocol (e, caps);
            snd_ump_endpoint_info_set_protocol_caps (e, caps);
            snd_ump_endpoint_info_set_name (e, name.toRawUTF8());
            snd_ump_endpoint_info_set_manufacturer_id (e, fromBytesLittleEndian (info.manufacturer));
            snd_ump_endpoint_info_set_family_id (e, fromBytesLittleEndian (info.family));
            snd_ump_endpoint_info_set_model_id (e, fromBytesLittleEndian (info.modelNumber));
            snd_ump_endpoint_info_set_sw_revision (e, reinterpret_cast<const unsigned char*> (info.revision.data()));
            snd_ump_endpoint_info_set_flags (e, areStatic == ump::BlocksAreStatic::yes ? SND_UMP_EP_INFO_STATIC_BLOCKS : 0);
            snd_ump_endpoint_info_set_num_blocks (e, (unsigned int) blocks.size());
            snd_ump_endpoint_info_set_product_id (e, productInstance.toRawUTF8());

            auto* sequencer = virtualClient->getSequencer();

            if (const auto code = snd_seq_create_ump_endpoint (sequencer, e, numGroupsRequired); code != 0)
                return {};

            for (const auto [index, block] : enumerate (blocks, uint8_t{}))
            {
                snd_ump_block_info_t* b{};
                snd_ump_block_info_alloca (&b);

                copyToBlock (*b, index, block);

                if (const auto code = snd_seq_create_ump_block (sequencer, index, b); code != 0)
                    return {};
            }

            return rawToUniquePtr (new Port { std::move (virtualClient), -1, std::nullopt, std::nullopt });
        }

        static std::unique_ptr<Port> make (std::shared_ptr<Client> c,
                                           ump::IOKind d,
                                           std::optional<snd_seq_addr_t> connected,
                                           String name)
        {
            if (c == nullptr || c->getSequencer() == nullptr)
                return {};

            const auto virtualFlags = ! connected.has_value()
                                    ? (d == ump::IOKind::src ? SND_SEQ_PORT_CAP_SUBS_WRITE : SND_SEQ_PORT_CAP_SUBS_READ)
                                    : SND_SEQ_PORT_CAP_NO_EXPORT;
            const auto readWriteFlags = d == ump::IOKind::src ? SND_SEQ_PORT_CAP_WRITE : SND_SEQ_PORT_CAP_READ;

            const auto caps = (unsigned int) (virtualFlags | readWriteFlags);

            const auto portId = snd_seq_create_simple_port (c->getSequencer(),
                                                            name.toRawUTF8(),
                                                            caps,
                                                            SND_SEQ_PORT_TYPE_MIDI_GENERIC | SND_SEQ_PORT_TYPE_APPLICATION);

            if (portId < 0)
            {
                jassertfalse;
                return {};
            }

            if (connected.has_value())
            {
                snd_seq_port_subscribe_t* subs{};
                snd_seq_port_subscribe_alloca (&subs);

                // Setting a queue for the subscription is necessary in order to receive timestamps
                snd_seq_port_subscribe_set_time_real (subs, 1);
                snd_seq_port_subscribe_set_time_update (subs, 1);
                snd_seq_port_subscribe_set_queue (subs, c->getQueueId());

                const snd_seq_addr_t selfPort { (unsigned char) c->getClientId(),
                                                (unsigned char) portId };

                snd_seq_port_subscribe_set_sender (subs, d == ump::IOKind::src ? &*connected : &selfPort);
                snd_seq_port_subscribe_set_dest   (subs, d == ump::IOKind::src ? &selfPort : &*connected);

                [[maybe_unused]] const auto code = snd_seq_subscribe_port (c->getSequencer(), subs);
                jassert (code == 0);
            }

            return rawToUniquePtr (new Port { c, portId, d, connected });
        }

        bool isUmpEndpoint() const
        {
            return portId == -1;
        }

    private:
        Port (std::shared_ptr<Client> c, int p, std::optional<ump::IOKind> dir, std::optional<snd_seq_addr_t> dst)
            : client (c),
              portId (p),
              direction (dir),
              connected (dst)
        {
            jassert (client != nullptr);

            client->addInputCallback (*this);
            client->addPortExitCallback (*this);
        }

        void pushUmp (int port, ump::View view, double time) override
        {
            if (portId == -1 || port == portId)
                inputCallbacks.call ([&] (auto& c) { c.pushUmp (port, view, time); });
        }

        void pushBytes (int port, ump::BytestreamMidiView view) override
        {
            // If this is hit, we've ended up pushing bytestream MIDI to a UMP endpoint, which
            // won't work. This is a JUCE bug, please let the JUCE maintainers know!
            jassert (portId != -1);

            if (port == portId)
                inputCallbacks.call ([&] (auto& c) { c.pushBytes (port, view); });
        }

        void portExit (snd_seq_addr_t port) override
        {
            const auto notify = connected.has_value()
                              ? (port.client == connected->client && port.port == connected->port)
                              : (port.client == client->getClientId() && port.port == portId);

            if (notify)
                triggerAsyncUpdate();
        }

        void handleAsyncUpdate() override
        {
            disconnectCallbacks.call ([&] (auto& c) { c.disconnected(); });
        }

        std::shared_ptr<Client> client;
        int portId = -1; // A negative portId indicates this is a special UMP virtual port
        std::optional<ump::IOKind> direction; // nullopt == bidirectional
        std::optional<snd_seq_addr_t> connected;
        WaitFreeListeners<InputCallback> inputCallbacks;
        // Disconnect listeners are called on the main thread
        ListenerList<ump::DisconnectionListener> disconnectCallbacks;

        JUCE_DECLARE_NON_COPYABLE (Port)
    };

    //==============================================================================
    class InputImplNative : public ump::Input::Impl::Native,
                            private InputCallback
    {
        auto getInputCallback (double time)
        {
            return [this, time] (ump::View v)
            {
                ump::Iterator b (v.data(), v.size());
                auto e = std::next (b);
                consumer.consume (b, e, time);
            };
        }

    public:
        InputImplNative (ump::DisconnectionListener& l,
                         std::shared_ptr<Port> p,
                         ump::PacketProtocol protocol,
                         ump::Consumer& c)
            : listener (l),
              port (std::move (p)),
              converter (protocol),
              consumer (c)
        {
            port->addInputCallback (*this);
            port->addDisconnectionListener (listener);
        }

        ~InputImplNative() override
        {
            port->removeDisconnectionListener (listener);
            port->removeInputCallback (*this);
        }

        ump::EndpointId getEndpointId() const override
        {
            return port->getId();
        }

        ump::PacketProtocol getProtocol() const override
        {
            return converter.getProtocol();
        }

    private:
        void pushUmp (int, ump::View view, double time) override
        {
            converter.convert (view, getInputCallback (time));
        }

        void pushBytes (int, ump::BytestreamMidiView view) override
        {
            // Bytestream messages that we're sent don't have a built-in group.
            // We currently make a separate endpoint for each bytestream port, rather than combining
            // ports into endpoints, so we can stick with group 0 here.
            converter.convert ({ 0, view.bytes }, getInputCallback (view.timestamp));
        }

        ump::DisconnectionListener& listener;
        std::shared_ptr<Port> port;
        ump::GenericUMPConverter converter;
        ump::Consumer& consumer;
    };

    class OutputImplNative : public ump::Output::Impl::Native
    {
    public:
        OutputImplNative (ump::DisconnectionListener& l, std::shared_ptr<Port> portIn)
            : listener (l), port (std::move (portIn))
        {
            snd_midi_event_t* ptr = nullptr;
            [[maybe_unused]] const auto code = snd_midi_event_new (maxEventSize, &ptr);
            jassert (code == 0);
            midiParser.reset (ptr);

            port->addDisconnectionListener (listener);
        }

        ~OutputImplNative() override
        {
            port->removeDisconnectionListener (listener);
        }

        bool send (ump::Iterator b, ump::Iterator e) override
        {
            if (snd_seq_ump_event_output_direct != nullptr)
            {
                for (const auto& v : makeRange (b, e))
                    sendUmp (v);
            }
            else
            {
                for (const auto& v : makeRange (b, e))
                    sendBytestream (v);
            }

            return true;
        }

        ump::EndpointId getEndpointId() const override
        {
            return port->getId();
        }

    private:
        struct SndMidiEventDeleter
        {
            void operator() (snd_midi_event_t* ptr) const
            {
                if (ptr != nullptr)
                    snd_midi_event_free (ptr);
            }
        };

        void sendUmp (ump::View v)
        {
            if (snd_seq_ump_event_output_direct == nullptr)
                return;

            snd_seq_ump_event_t event{};

            snd_seq_ev_set_source (&event, (unsigned char) port->getPortId());
            snd_seq_ev_set_subs (&event);
            snd_seq_ev_set_direct (&event);

            event.flags |= SND_SEQ_EVENT_UMP;
            event.type = 0;
            memcpy (event.ump, v.data(), sizeof (uint32_t) * v.size());

            [[maybe_unused]] const auto code = snd_seq_ump_event_output_direct (port->getClient()->getSequencer(), &event);
            jassert (code >= 0);
        }

        void sendBytestream (ump::View v)
        {
            toBytestream.convert (v, 0.0, [this] (ump::BytesOnGroup message, double)
            {
                if (message.bytes.size() > maxEventSize)
                {
                    maxEventSize = message.bytes.size();

                    snd_midi_event_t* ptr = nullptr;
                    const auto code = snd_midi_event_new (maxEventSize, &ptr);
                    jassertquiet (code == 0);
                    midiParser.reset (ptr);
                }

                snd_seq_event_t event;
                snd_seq_ev_clear (&event);

                auto numBytes = (long) message.bytes.size();
                auto* data = unalignedPointerCast<const unsigned char*> (message.bytes.data());

                const auto client = port->getClient();
                auto seqHandle = client->getSequencer();

                while (numBytes > 0)
                {
                    auto numSent = snd_midi_event_encode (midiParser.get(), data, numBytes, &event);

                    if (numSent <= 0)
                        break;

                    numBytes -= numSent;
                    data += numSent;

                    snd_seq_ev_set_source (&event, (unsigned char) port->getPortId());
                    snd_seq_ev_set_subs (&event);
                    snd_seq_ev_set_direct (&event);

                    if (snd_seq_event_output_direct (seqHandle, &event) < 0)
                        break;
                }

                snd_midi_event_reset_encode (midiParser.get());
            });
        }

        ump::DisconnectionListener& listener;
        std::shared_ptr<Port> port;
        ump::ToBytestreamConverter toBytestream { 4096 };

        std::unique_ptr<snd_midi_event_t, SndMidiEventDeleter> midiParser;
        size_t maxEventSize = 4096;
    };

    static void copyToBlock (snd_ump_block_info_t& dst, uint8_t index, const ump::Block& src)
    {
        const auto direction = std::invoke ([&]
        {
            switch (src.getDirection())
            {
                case ump::BlockDirection::bidirectional: return SND_UMP_DIR_BIDIRECTION;
                case ump::BlockDirection::sender: return SND_UMP_DIR_OUTPUT;
                case ump::BlockDirection::receiver: return SND_UMP_DIR_INPUT;
                case ump::BlockDirection::unknown: break;
            }

            return decltype (SND_UMP_DIR_BIDIRECTION){};
        });

        const auto hint = std::invoke ([&]
        {
            switch (src.getUiHint())
            {
                case ump::BlockUiHint::bidirectional: return SND_UMP_BLOCK_UI_HINT_BOTH;
                case ump::BlockUiHint::sender: return SND_UMP_BLOCK_UI_HINT_SENDER;
                case ump::BlockUiHint::receiver: return SND_UMP_BLOCK_UI_HINT_RECEIVER;
                case ump::BlockUiHint::unknown: break;
            }

            return SND_UMP_BLOCK_UI_HINT_UNKNOWN;
        });

        snd_ump_block_info_set_block_id (&dst, index);
        snd_ump_block_info_set_active (&dst, src.isEnabled());
        snd_ump_block_info_set_direction (&dst, (unsigned int) direction);
        snd_ump_block_info_set_ui_hint (&dst, (unsigned int) hint);
        snd_ump_block_info_set_first_group (&dst, src.getFirstGroup());
        snd_ump_block_info_set_num_groups (&dst, src.getNumGroups());
        snd_ump_block_info_set_name (&dst, src.getName().toRawUTF8());
        snd_ump_block_info_set_sysex8_streams (&dst, src.getMaxSysex8Streams());
    }

    class PortWithInfo
    {
    public:
        ump::EndpointId getId() const
        {
            return port->getId();
        }

        ump::Endpoint getEndpoint() const
        {
            return endpoint;
        }

        ump::StaticDeviceInfo getStaticDeviceInfo() const
        {
            return port->getStaticDeviceInfo();
        }

        bool setBlock (uint8_t index, const ump::Block& block)
        {
            if (snd_seq_set_ump_block_info == nullptr)
                return false;

            snd_ump_block_info_t* b{};
            snd_ump_block_info_alloca (&b);

            copyToBlock (*b, index, block);

            if (0 != snd_seq_set_ump_block_info (port->getClient()->getSequencer(), index, b))
                return false;

            endpoint.getBlocks()[index] = block;
            return true;
        }

        bool setName (const String& x)
        {
            if (0 != snd_seq_set_client_name (port->getClient()->getSequencer(), x.toRawUTF8()))
                return false;

            endpoint = endpoint.withName (x);
            return true;
        }

        static std::unique_ptr<PortWithInfo> make (std::shared_ptr<Port> p, ump::Endpoint e)
        {
            if (p == nullptr)
                return {};

            return rawToUniquePtr (new PortWithInfo { std::move (p), std::move (e) });
        }

        std::shared_ptr<Port> getPort() const { return port; }

    private:
        PortWithInfo (std::shared_ptr<Port> p, ump::Endpoint e)
            : port (std::move (p)), endpoint (e) {}

        std::shared_ptr<Port> port;
        ump::Endpoint endpoint;
    };

    class VirtualEndpointImplNative : public ump::VirtualEndpoint::Impl::Native,
                                      public ump::LegacyVirtualInput::Impl::Native,
                                      public ump::LegacyVirtualOutput::Impl::Native
    {
    public:
        ump::EndpointId getId() const override
        {
            return port->getId();
        }

        bool setBlock (uint8_t index, const ump::Block& block) override
        {
            return port->setBlock (index, block);
        }

        bool setName (const String& x) override
        {
            return port->setName (x);
        }

        static std::unique_ptr<VirtualEndpointImplNative> make (std::shared_ptr<PortWithInfo> p)
        {
            if (p == nullptr)
                return {};

            return rawToUniquePtr (new VirtualEndpointImplNative { std::move (p) });
        }

    private:
        explicit VirtualEndpointImplNative (std::shared_ptr<PortWithInfo> p)
            : port (std::move (p))
        {
        }

        std::shared_ptr<PortWithInfo> port;
    };

    struct VirtualEndpointRegistry
    {
        virtual ~VirtualEndpointRegistry() = default;
        virtual void virtualEndpointAdded (std::shared_ptr<PortWithInfo>) = 0;
    };

    class SessionImplNative : public ump::Session::Impl::Native
    {
    public:
        explicit SessionImplNative (VirtualEndpointRegistry& r, std::shared_ptr<Client> c, String n)
            : registry (r),
              client (c),
              name (n)
        {
        }

        String getName() const override
        {
            return name;
        }

        std::unique_ptr<ump::Input::Impl::Native> connectInput (ump::DisconnectionListener& listener,
                                                                const ump::EndpointId& identifier,
                                                                ump::PacketProtocol protocol,
                                                                ump::Consumer& callback) override
        {
            if (auto port = findOrCreatePort ({ identifier, ump::IOKind::src }))
                return rawToUniquePtr (new InputImplNative (listener, std::move (port), protocol, callback));

            return {};
        }

        std::unique_ptr<ump::Output::Impl::Native> connectOutput (ump::DisconnectionListener& listener,
                                                                  const ump::EndpointId& identifier) override
        {
            if (auto port = findOrCreatePort ({ identifier, ump::IOKind::dst }))
                return rawToUniquePtr (new OutputImplNative (listener, std::move (port)));

            return {};
        }

        std::unique_ptr<ump::VirtualEndpoint::Impl::Native> createNativeVirtualEndpoint (const String& n,
                                                                                         const ump::DeviceInfo& info,
                                                                                         const String& productInstance,
                                                                                         ump::PacketProtocol protocol,
                                                                                         Span<const ump::Block> blocks,
                                                                                         ump::BlocksAreStatic areStatic) override
        {
            return wrapPortAsEndpoint (createVirtualEndpointPort (n, info, productInstance, protocol, blocks, areStatic));
        }

        std::unique_ptr<ump::LegacyVirtualInput::Impl::Native> createLegacyVirtualInput (const String& deviceName) override
        {
            return wrapPortAsEndpoint (createLegacyEndpointPort (deviceName, ump::IOKind::dst));
        }

        std::unique_ptr<ump::LegacyVirtualOutput::Impl::Native> createLegacyVirtualOutput (const String& deviceName) override
        {
            return wrapPortAsEndpoint (createLegacyEndpointPort (deviceName, ump::IOKind::src));
        }

    private:
        struct Key
        {
            ump::EndpointId id;
            ump::IOKind direction;

            bool operator< (const Key& other) const
            {
                const auto tie = [] (auto& x) { return std::tie (x.id, x.direction); };
                return tie (*this) < tie (other);
            }
        };

        std::optional<AlsaClientInfo> getClientInfo (const ump::EndpointId& identifier) const
        {
            return client->getClientInfo (identifier);
        }

        std::shared_ptr<Port> findOrCreatePort (const Key& key)
        {
            auto& weak = weakPorts[key];

            if (auto strong = weak.lock())
                return strong;

            const auto info = getClientInfo (key.id);

            if (! info.has_value())
                return {};

            const std::shared_ptr strong = Port::make (client,
                                                       key.direction,
                                                       snd_seq_addr_t { (unsigned char) info->clientId,
                                                                        (unsigned char) info->portId },
                                                       client->getName());
            weak = strong;
            return strong;
        }

        static std::shared_ptr<PortWithInfo> createVirtualEndpointPort (const String& n,
                                                                        const ump::DeviceInfo& info,
                                                                        const String& productInstance,
                                                                        ump::PacketProtocol protocol,
                                                                        Span<const ump::Block> blocks,
                                                                        ump::BlocksAreStatic areStatic)
        {
            return PortWithInfo::make (Port::makeUmpEndpoint (n, info, productInstance, protocol, blocks, areStatic),
                                       ump::Endpoint{}.withName (n)
                                                      .withDeviceInfo (info)
                                                      .withProductInstanceId (productInstance)
                                                      .withProtocol (protocol)
                                                      .withMidi1Support (protocol == ump::PacketProtocol::MIDI_1_0)
                                                      .withMidi2Support (protocol == ump::PacketProtocol::MIDI_2_0)
                                                      .withStaticBlocks (areStatic == ump::BlocksAreStatic::yes)
                                                      .withBlocks (blocks));
        }

        std::shared_ptr<PortWithInfo> createLegacyEndpointPort (const String& portName, ump::IOKind direction)
        {
            const ump::Block blocks[] { ump::IOHelpers::makeLegacyBlock (direction == ump::IOKind::dst) };
            return PortWithInfo::make (Port::make (client,
                                                   direction == ump::IOKind::src ? ump::IOKind::dst : ump::IOKind::src,
                                                   std::nullopt,
                                                   portName),
                                       ump::Endpoint{}.withName (portName)
                                                      .withProtocol (ump::PacketProtocol::MIDI_1_0)
                                                      .withMidi1Support (true)
                                                      .withStaticBlocks (true)
                                                      .withBlocks (blocks));
        }

        std::unique_ptr<VirtualEndpointImplNative> wrapPortAsEndpoint (std::shared_ptr<PortWithInfo> port)
        {
            if (port == nullptr)
                return {};

            auto inner = port->getPort();

            if (inner == nullptr)
                return {};

            if (inner->isSrc())
                weakPorts[{ port->getId(), ump::IOKind::src }] = inner;

            if (inner->isDst())
                weakPorts[{ port->getId(), ump::IOKind::dst }] = inner;

            registry.virtualEndpointAdded (port);

            return VirtualEndpointImplNative::make (port);
        }

        VirtualEndpointRegistry& registry;
        std::shared_ptr<Client> client;
        std::map<Key, std::weak_ptr<Port>> weakPorts;
        String name;
    };

    class AnnouncementsPort
    {
    public:
        AnnouncementsPort() = default;

        explicit AnnouncementsPort (snd_seq_t* s)
            : seq (s)
        {
            if (seq != nullptr)
                snd_seq_connect_from (seq, portId, SND_SEQ_CLIENT_SYSTEM, SND_SEQ_PORT_SYSTEM_ANNOUNCE);
        }

        AnnouncementsPort (AnnouncementsPort&& other) noexcept
            : seq (std::exchange (other.seq, {})),
              portId (std::exchange (other.portId, {}))
        {
        }

        AnnouncementsPort& operator= (AnnouncementsPort&& other) noexcept
        {
            AnnouncementsPort tmp { std::move (other) };
            std::swap (tmp.seq, seq);
            std::swap (tmp.portId, portId);
            return *this;
        }

        AnnouncementsPort (const AnnouncementsPort&) = delete;
        AnnouncementsPort& operator= (const AnnouncementsPort&) = delete;

        ~AnnouncementsPort() noexcept
        {
            if (seq == nullptr)
                return;

            snd_seq_disconnect_from (seq, portId, SND_SEQ_CLIENT_SYSTEM, SND_SEQ_PORT_SYSTEM_ANNOUNCE);
            snd_seq_delete_simple_port (seq, portId);
        }

    private:
        snd_seq_t* seq = nullptr;
        int portId = seq != nullptr
                   ? snd_seq_create_simple_port (seq,
                                                 TRANS ("announcements").toRawUTF8(),
                                                 SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_NO_EXPORT,
                                                 SND_SEQ_PORT_TYPE_MIDI_GENERIC | SND_SEQ_PORT_TYPE_APPLICATION)
                   : 0;
    };

    class EndpointsImplNative : public ump::Endpoints::Impl::Native,
                                private VirtualEndpointRegistry
    {
    public:
        explicit EndpointsImplNative (std::shared_ptr<Client> c)
            : client (std::move (c))
        {
        }

        void getEndpoints (std::vector<ump::EndpointId>& x) const override
        {
            client->getEndpoints (x);
        }

        std::optional<ump::Endpoint> getEndpoint (const ump::EndpointId& x) const override
        {
            if (const auto iter = virtualPorts.find (x); iter != virtualPorts.end())
                if (auto strong = iter->second.lock())
                    return strong->getEndpoint();

            if (const auto& c = client->getClientInfo (x))
                return c->fullInfo.endpoint;

            return {};
        }

        std::optional<ump::StaticDeviceInfo> getStaticDeviceInfo (const ump::EndpointId& x) const override
        {
            if (const auto iter = virtualPorts.find (x); iter != virtualPorts.end())
                if (auto strong = iter->second.lock())
                    return strong->getStaticDeviceInfo();

            if (const auto& c = client->getClientInfo (x))
                return c->fullInfo.info;

            return {};
        }

        std::unique_ptr<ump::Session::Impl::Native> makeSession (const String& n) override
        {
            return rawToUniquePtr (new SessionImplNative { *this, client, n });
        }

        ump::Backend getBackend() const override
        {
            return ump::Backend::alsa;
        }

        bool isVirtualMidiBytestreamServiceActive() const override
        {
            return true;
        }

        bool isVirtualMidiUmpServiceActive() const override
        {
            return snd_seq_create_ump_endpoint != nullptr;
        }

        void setVirtualMidiBytestreamServiceActive (bool) override {}
        void setVirtualMidiUmpServiceActive (bool) override {}

    private:
        void virtualEndpointAdded (std::shared_ptr<PortWithInfo> p) override
        {
            if (p != nullptr)
                virtualPorts[p->getId()] = p;
        }

        std::map<ump::EndpointId, std::weak_ptr<PortWithInfo>> virtualPorts;
        std::shared_ptr<Client> client;
        AnnouncementsPort announcementsPort { client->getSequencer() };
    };

    //==============================================================================
    AlsaMidiHelpers() = delete;
};

auto ump::Endpoints::Impl::Native::make (EndpointsListener& l) -> std::unique_ptr<Native>
{
    if (auto client = AlsaMidiHelpers::Client::make (&l))
        return std::make_unique<AlsaMidiHelpers::EndpointsImplNative> (std::move (client));

    return {};
}

#else

auto ump::Endpoints::Impl::Native::make (EndpointsListener& l) -> std::unique_ptr<Native>
{
    return nullptr;
}

#endif

} // namespace juce

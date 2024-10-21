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

//==============================================================================
class AlsaClient
{
    auto lowerBound (int portId) const
    {
        const auto comparator = [] (const auto& port, const auto& id) { return port->getPortId() < id; };
        return std::lower_bound (ports.begin(), ports.end(), portId, comparator);
    }

    auto findPortIterator (int portId) const
    {
        const auto iter = lowerBound (portId);
        return (iter == ports.end() || (*iter)->getPortId() != portId) ? ports.end() : iter;
    }

public:
    ~AlsaClient()
    {
        inputThread.reset();

        jassert (activeCallbacks.get() == 0);

        if (handle != nullptr)
        {
            snd_seq_delete_simple_port (handle, announcementsIn);
            snd_seq_close (handle);
        }
    }

    static String getAlsaMidiName()
    {
        #ifdef JUCE_ALSA_MIDI_NAME
         return JUCE_ALSA_MIDI_NAME;
        #else
         if (auto* app = JUCEApplicationBase::getInstance())
             return app->getApplicationName();

         return "JUCE";
        #endif
    }

    //==============================================================================
    // represents an input or output port of the supplied AlsaClient
    struct Port
    {
        explicit Port (bool forInput) noexcept
            : isInput (forInput) {}

        ~Port()
        {
            if (isValid())
            {
                if (isInput)
                    enableCallback (false);
                else
                    snd_midi_event_free (midiParser);

                snd_seq_delete_simple_port (client->get(), portId);
            }
        }

        void connectWith (int sourceClient, int sourcePort) const noexcept
        {
            if (isInput)
                snd_seq_connect_from (client->get(), portId, sourceClient, sourcePort);
            else
                snd_seq_connect_to (client->get(), portId, sourceClient, sourcePort);
        }

        bool isValid() const noexcept
        {
            return client->get() != nullptr && portId >= 0;
        }

        void setupInput (MidiInput* input, MidiInputCallback* cb)
        {
            jassert (cb != nullptr && input != nullptr);
            callback = cb;
            midiInput = input;
        }

        void setupOutput()
        {
            jassert (! isInput);
            snd_midi_event_new ((size_t) maxEventSize, &midiParser);
        }

        void enableCallback (bool enable)
        {
            callbackEnabled = enable;
        }

        bool sendMessageNow (const MidiMessage& message)
        {
            if (message.getRawDataSize() > maxEventSize)
            {
                maxEventSize = message.getRawDataSize();
                snd_midi_event_free (midiParser);
                snd_midi_event_new ((size_t) maxEventSize, &midiParser);
            }

            snd_seq_event_t event;
            snd_seq_ev_clear (&event);

            auto numBytes = (long) message.getRawDataSize();
            auto* data = message.getRawData();

            auto seqHandle = client->get();
            bool success = true;

            while (numBytes > 0)
            {
                auto numSent = snd_midi_event_encode (midiParser, data, numBytes, &event);

                if (numSent <= 0)
                {
                    success = numSent == 0;
                    break;
                }

                numBytes -= numSent;
                data += numSent;

                snd_seq_ev_set_source (&event, (unsigned char) portId);
                snd_seq_ev_set_subs (&event);
                snd_seq_ev_set_direct (&event);

                if (snd_seq_event_output_direct (seqHandle, &event) < 0)
                {
                    success = false;
                    break;
                }
            }

            snd_midi_event_reset_encode (midiParser);
            return success;
        }


        bool operator== (const Port& lhs) const noexcept
        {
            return portId != -1 && portId == lhs.portId;
        }

        void createPort (const String& name, bool enableSubscription)
        {
            if (auto seqHandle = client->get())
            {
                const unsigned int caps =
                    isInput ? (SND_SEQ_PORT_CAP_WRITE | (enableSubscription ? SND_SEQ_PORT_CAP_SUBS_WRITE : 0))
                            : (SND_SEQ_PORT_CAP_READ  | (enableSubscription ? SND_SEQ_PORT_CAP_SUBS_READ : 0));

                portName = name;
                portId = snd_seq_create_simple_port (seqHandle, portName.toUTF8(), caps,
                                                     SND_SEQ_PORT_TYPE_MIDI_GENERIC |
                                                     SND_SEQ_PORT_TYPE_APPLICATION);
            }
        }

        void handleIncomingMidiMessage (const MidiMessage& message) const
        {
            if (callbackEnabled)
                callback->handleIncomingMidiMessage (midiInput, message);
        }

        void handlePartialSysexMessage (const uint8* messageData, int numBytesSoFar, double timeStamp)
        {
            if (callbackEnabled)
                callback->handlePartialSysexMessage (midiInput, messageData, numBytesSoFar, timeStamp);
        }

        int getPortId() const               { return portId; }
        const String& getPortName() const   { return portName; }

    private:
        const std::shared_ptr<AlsaClient> client = AlsaClient::getInstance();

        MidiInputCallback* callback = nullptr;
        snd_midi_event_t* midiParser = nullptr;
        MidiInput* midiInput = nullptr;

        String portName;

        int maxEventSize = 4096, portId = -1;
        std::atomic<bool> callbackEnabled { false };
        bool isInput = false;
    };

    static std::shared_ptr<AlsaClient> getInstance()
    {
        static std::weak_ptr<AlsaClient> ptr;

        if (auto locked = ptr.lock())
            return locked;

        std::shared_ptr<AlsaClient> result (new AlsaClient());
        ptr = result;
        return result;
    }

    void handleIncomingMidiMessage (snd_seq_event* event, const MidiMessage& message)
    {
        const ScopedLock sl (callbackLock);

        if (auto* port = findPort (event->dest.port))
            port->handleIncomingMidiMessage (message);
    }

    void handlePartialSysexMessage (snd_seq_event* event, const uint8* messageData, int numBytesSoFar, double timeStamp)
    {
        const ScopedLock sl (callbackLock);

        if (auto* port = findPort (event->dest.port))
            port->handlePartialSysexMessage (messageData, numBytesSoFar, timeStamp);
    }

    snd_seq_t* get() const noexcept     { return handle; }
    int getId() const noexcept          { return clientId; }

    Port* createPort (const String& name, bool forInput, bool enableSubscription)
    {
        const ScopedLock sl (callbackLock);

        auto port = new Port (forInput);
        port->createPort (name, enableSubscription);

        const auto iter = lowerBound (port->getPortId());
        jassert (iter == ports.end() || port->getPortId() < (*iter)->getPortId());
        ports.insert (iter, rawToUniquePtr (port));

        return port;
    }

    void deletePort (Port* port)
    {
        const ScopedLock sl (callbackLock);

        if (const auto iter = findPortIterator (port->getPortId()); iter != ports.end())
            ports.erase (iter);
    }

private:
    AlsaClient()
    {
        snd_seq_open (&handle, "default", SND_SEQ_OPEN_DUPLEX, 0);

        if (handle != nullptr)
        {
            snd_seq_nonblock (handle, SND_SEQ_NONBLOCK);
            snd_seq_set_client_name (handle, getAlsaMidiName().toRawUTF8());
            clientId = snd_seq_client_id (handle);

            // It's good idea to pre-allocate a good number of elements
            ports.reserve (32);

            announcementsIn = snd_seq_create_simple_port (handle,
                                                          TRANS ("announcements").toRawUTF8(),
                                                          SND_SEQ_PORT_CAP_WRITE,
                                                          SND_SEQ_PORT_TYPE_MIDI_GENERIC | SND_SEQ_PORT_TYPE_APPLICATION);
            snd_seq_connect_from (handle, announcementsIn, SND_SEQ_CLIENT_SYSTEM, SND_SEQ_PORT_SYSTEM_ANNOUNCE);

            inputThread.emplace (*this);
        }
    }

    Port* findPort (int portId)
    {
        if (const auto iter = findPortIterator (portId); iter != ports.end())
            return iter->get();

        return nullptr;
    }

    snd_seq_t* handle = nullptr;
    int clientId = 0;
    int announcementsIn = 0;
    std::vector<std::unique_ptr<Port>> ports;
    Atomic<int> activeCallbacks;
    CriticalSection callbackLock;

    //==============================================================================
    class SequencerThread
    {
    public:
        explicit SequencerThread (AlsaClient& c)
            : client (c)
        {
        }

        ~SequencerThread() noexcept
        {
            shouldStop = true;
            thread.join();
        }

    private:
        // If we directly call MidiDeviceListConnectionBroadcaster::get() from the background thread,
        // there's a possibility that we'll deadlock in the following scenario:
        // - The main thread calls MidiDeviceListConnectionBroadcaster::get() for the first time
        //   (e.g. to register a listener). The static MidiDeviceListConnectionBroadcaster singleton
        //   begins construction. During the constructor, an AlsaClient is created to iterate midi
        //   ins/outs.
        // - The AlsaClient starts a new SequencerThread. If connections are updated, the
        //   SequencerThread may call MidiDeviceListConnectionBroadcaster::get().notify()
        //   while the MidiDeviceListConnectionBroadcaster singleton is still being created.
        // - The SequencerThread blocks until the MidiDeviceListConnectionBroadcaster has been
        //   created on the main thread, but the MidiDeviceListConnectionBroadcaster's constructor
        //   can't complete until the AlsaClient's destructor has run, which in turn requires the
        //   SequencerThread to join.
        class UpdateNotifier final : private AsyncUpdater
        {
        public:
            ~UpdateNotifier() override { cancelPendingUpdate(); }
            using AsyncUpdater::triggerAsyncUpdate;

        private:
            void handleAsyncUpdate() override { MidiDeviceListConnectionBroadcaster::get().notify(); }
        };

        AlsaClient& client;
        MidiDataConcatenator concatenator { 2048 };
        std::atomic<bool> shouldStop { false };
        UpdateNotifier notifier;
        std::thread thread { [this]
        {
            Thread::setCurrentThreadName ("JUCE MIDI Input");

            auto seqHandle = client.get();

            const int maxEventSize = 16 * 1024;
            snd_midi_event_t* midiParser;

            if (snd_midi_event_new (maxEventSize, &midiParser) >= 0)
            {
                const ScopeGuard freeMidiEvent { [&] { snd_midi_event_free (midiParser); } };

                const auto numPfds = snd_seq_poll_descriptors_count (seqHandle, POLLIN);
                std::vector<pollfd> pfd (static_cast<size_t> (numPfds));
                snd_seq_poll_descriptors (seqHandle, pfd.data(), (unsigned int) numPfds, POLLIN);

                std::vector<uint8> buffer (maxEventSize);

                while (! shouldStop)
                {
                    // This timeout shouldn't be too long, so that the program can exit in a timely manner
                    if (poll (pfd.data(), (nfds_t) numPfds, 100) > 0)
                    {
                        if (shouldStop)
                            break;

                        do
                        {
                            snd_seq_event_t* inputEvent = nullptr;

                            if (snd_seq_event_input (seqHandle, &inputEvent) >= 0)
                            {
                                const ScopeGuard freeInputEvent { [&] { snd_seq_free_event (inputEvent); } };

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

                                const auto foundEvent = std::find (std::begin (systemEvents),
                                                                   std::end   (systemEvents),
                                                                   inputEvent->type);

                                if (foundEvent != std::end (systemEvents))
                                {
                                    notifier.triggerAsyncUpdate();
                                    continue;
                                }

                                // xxx what about SYSEXes that are too big for the buffer?
                                const auto numBytes = snd_midi_event_decode (midiParser,
                                                                             buffer.data(),
                                                                             maxEventSize,
                                                                             inputEvent);

                                snd_midi_event_reset_decode (midiParser);

                                concatenator.pushMidiData (buffer.data(), (int) numBytes,
                                                           Time::getMillisecondCounter() * 0.001,
                                                           inputEvent, client);
                            }
                        }
                        while (snd_seq_event_input_pending (seqHandle, 0) > 0);
                    }
                }
            }
        } };
    };

    std::optional<SequencerThread> inputThread;
};

//==============================================================================
static String getFormattedPortIdentifier (int clientId, int portId)
{
    return String (clientId) + "-" + String (portId);
}

static AlsaClient::Port* iterateMidiClient (AlsaClient& client,
                                            snd_seq_client_info_t* clientInfo,
                                            bool forInput,
                                            Array<MidiDeviceInfo>& devices,
                                            const String& deviceIdentifierToOpen)
{
    AlsaClient::Port* port = nullptr;

    auto seqHandle = client.get();
    snd_seq_port_info_t* portInfo = nullptr;

    snd_seq_port_info_alloca (&portInfo);
    jassert (portInfo != nullptr);
    auto numPorts = snd_seq_client_info_get_num_ports (clientInfo);
    auto sourceClient = snd_seq_client_info_get_client (clientInfo);

    snd_seq_port_info_set_client (portInfo, sourceClient);
    snd_seq_port_info_set_port (portInfo, -1);

    while (--numPorts >= 0)
    {
        if (snd_seq_query_next_port (seqHandle, portInfo) == 0
            && (snd_seq_port_info_get_capability (portInfo)
                & (forInput ? SND_SEQ_PORT_CAP_SUBS_READ : SND_SEQ_PORT_CAP_SUBS_WRITE)) != 0)
        {
            String portName (snd_seq_port_info_get_name (portInfo));
            auto portID = snd_seq_port_info_get_port (portInfo);

            MidiDeviceInfo device (portName, getFormattedPortIdentifier (sourceClient, portID));
            devices.add (device);

            if (deviceIdentifierToOpen.isNotEmpty() && deviceIdentifierToOpen == device.identifier)
            {
                if (portID != -1)
                {
                    port = client.createPort (portName, forInput, false);
                    jassert (port->isValid());
                    port->connectWith (sourceClient, portID);
                    break;
                }
            }
        }
    }

    return port;
}

static AlsaClient::Port* iterateMidiDevices (bool forInput,
                                             Array<MidiDeviceInfo>& devices,
                                             const String& deviceIdentifierToOpen)
{
    AlsaClient::Port* port = nullptr;
    auto client = AlsaClient::getInstance();

    if (auto seqHandle = client->get())
    {
        snd_seq_system_info_t* systemInfo = nullptr;
        snd_seq_client_info_t* clientInfo = nullptr;

        snd_seq_system_info_alloca (&systemInfo);
        jassert (systemInfo != nullptr);

        if (snd_seq_system_info (seqHandle, systemInfo) == 0)
        {
            snd_seq_client_info_alloca (&clientInfo);
            jassert (clientInfo != nullptr);

            auto numClients = snd_seq_system_info_get_cur_clients (systemInfo);

            while (--numClients >= 0)
            {
                if (snd_seq_query_next_client (seqHandle, clientInfo) == 0)
                {
                    port = iterateMidiClient (*client,
                                              clientInfo,
                                              forInput,
                                              devices,
                                              deviceIdentifierToOpen);

                    if (port != nullptr)
                        break;
                }
            }
        }
    }

    return port;
}

struct AlsaPortPtr
{
    explicit AlsaPortPtr (AlsaClient::Port* p)
        : ptr (p) {}

    virtual ~AlsaPortPtr() noexcept { AlsaClient::getInstance()->deletePort (ptr); }

    AlsaClient::Port* ptr = nullptr;
};

//==============================================================================
class MidiInput::Pimpl final : public AlsaPortPtr
{
public:
    using AlsaPortPtr::AlsaPortPtr;
};

Array<MidiDeviceInfo> MidiInput::getAvailableDevices()
{
    Array<MidiDeviceInfo> devices;
    iterateMidiDevices (true, devices, {});

    return devices;
}

MidiDeviceInfo MidiInput::getDefaultDevice()
{
    return getAvailableDevices().getFirst();
}

std::unique_ptr<MidiInput> MidiInput::openDevice (const String& deviceIdentifier, MidiInputCallback* callback)
{
    if (deviceIdentifier.isEmpty())
        return {};

    Array<MidiDeviceInfo> devices;
    auto* port = iterateMidiDevices (true, devices, deviceIdentifier);

    if (port == nullptr || ! port->isValid())
        return {};

    jassert (port->isValid());

    std::unique_ptr<MidiInput> midiInput (new MidiInput (port->getPortName(), deviceIdentifier));

    port->setupInput (midiInput.get(), callback);
    midiInput->internal = std::make_unique<Pimpl> (port);

    return midiInput;
}

std::unique_ptr<MidiInput> MidiInput::createNewDevice (const String& deviceName, MidiInputCallback* callback)
{
    auto client = AlsaClient::getInstance();
    auto* port = client->createPort (deviceName, true, true);

    if (port == nullptr || ! port->isValid())
        return {};

    std::unique_ptr<MidiInput> midiInput (new MidiInput (deviceName, getFormattedPortIdentifier (client->getId(), port->getPortId())));

    port->setupInput (midiInput.get(), callback);
    midiInput->internal = std::make_unique<Pimpl> (port);

    return midiInput;
}

StringArray MidiInput::getDevices()
{
    StringArray deviceNames;

    for (auto& d : getAvailableDevices())
        deviceNames.add (d.name);

    deviceNames.appendNumbersToDuplicates (true, true);

    return deviceNames;
}

int MidiInput::getDefaultDeviceIndex()
{
    return 0;
}

std::unique_ptr<MidiInput> MidiInput::openDevice (int index, MidiInputCallback* callback)
{
    return openDevice (getAvailableDevices()[index].identifier, callback);
}

MidiInput::MidiInput (const String& deviceName, const String& deviceIdentifier)
    : deviceInfo (deviceName, deviceIdentifier)
{
}

MidiInput::~MidiInput()
{
    stop();
}

void MidiInput::start()
{
    internal->ptr->enableCallback (true);
}

void MidiInput::stop()
{
    internal->ptr->enableCallback (false);
}

//==============================================================================
class MidiOutput::Pimpl final : public AlsaPortPtr
{
public:
    using AlsaPortPtr::AlsaPortPtr;
};

Array<MidiDeviceInfo> MidiOutput::getAvailableDevices()
{
    Array<MidiDeviceInfo> devices;
    iterateMidiDevices (false, devices, {});

    return devices;
}

MidiDeviceInfo MidiOutput::getDefaultDevice()
{
    return getAvailableDevices().getFirst();
}

std::unique_ptr<MidiOutput> MidiOutput::openDevice (const String& deviceIdentifier)
{
    if (deviceIdentifier.isEmpty())
        return {};

    Array<MidiDeviceInfo> devices;
    auto* port = iterateMidiDevices (false, devices, deviceIdentifier);

    if (port == nullptr || ! port->isValid())
        return {};

    std::unique_ptr<MidiOutput> midiOutput (new MidiOutput (port->getPortName(), deviceIdentifier));

    port->setupOutput();
    midiOutput->internal = std::make_unique<Pimpl> (port);

    return midiOutput;
}

std::unique_ptr<MidiOutput> MidiOutput::createNewDevice (const String& deviceName)
{
    auto client = AlsaClient::getInstance();
    auto* port = client->createPort (deviceName, false, true);

    if (port == nullptr || ! port->isValid())
        return {};

    std::unique_ptr<MidiOutput> midiOutput (new MidiOutput (deviceName, getFormattedPortIdentifier (client->getId(), port->getPortId())));

    port->setupOutput();
    midiOutput->internal = std::make_unique<Pimpl> (port);

    return midiOutput;
}

StringArray MidiOutput::getDevices()
{
    StringArray deviceNames;

    for (auto& d : getAvailableDevices())
        deviceNames.add (d.name);

    deviceNames.appendNumbersToDuplicates (true, true);

    return deviceNames;
}

int MidiOutput::getDefaultDeviceIndex()
{
    return 0;
}

std::unique_ptr<MidiOutput> MidiOutput::openDevice (int index)
{
    return openDevice (getAvailableDevices()[index].identifier);
}

MidiOutput::~MidiOutput()
{
    stopBackgroundThread();
}

void MidiOutput::sendMessageNow (const MidiMessage& message)
{
    internal->ptr->sendMessageNow (message);
}

MidiDeviceListConnection MidiDeviceListConnection::make (std::function<void()> cb)
{
    auto& broadcaster = MidiDeviceListConnectionBroadcaster::get();
    // We capture the AlsaClient instance here to ensure that it remains alive for at least as long
    // as the MidiDeviceListConnection. This is necessary because system change messages will only
    // be processed when the AlsaClient's SequencerThread is running.
    return { &broadcaster, broadcaster.add ([fn = std::move (cb), client = AlsaClient::getInstance()]
                                            {
                                                NullCheckedInvocation::invoke (fn);
                                            }) };
}

//==============================================================================
#else

class MidiInput::Pimpl {};

// (These are just stub functions if ALSA is unavailable...)
MidiInput::MidiInput (const String& deviceName, const String& deviceID)
    : deviceInfo (deviceName, deviceID)
{
}

MidiInput::~MidiInput()                                                                   {}
void MidiInput::start()                                                                   {}
void MidiInput::stop()                                                                    {}
Array<MidiDeviceInfo> MidiInput::getAvailableDevices()                                    { return {}; }
MidiDeviceInfo MidiInput::getDefaultDevice()                                              { return {}; }
std::unique_ptr<MidiInput> MidiInput::openDevice (const String&, MidiInputCallback*)      { return {}; }
std::unique_ptr<MidiInput> MidiInput::createNewDevice (const String&, MidiInputCallback*) { return {}; }
StringArray MidiInput::getDevices()                                                       { return {}; }
int MidiInput::getDefaultDeviceIndex()                                                    { return 0;}
std::unique_ptr<MidiInput> MidiInput::openDevice (int, MidiInputCallback*)                { return {}; }

class MidiOutput::Pimpl {};

MidiOutput::~MidiOutput()                                                                 {}
void MidiOutput::sendMessageNow (const MidiMessage&)                                      {}
Array<MidiDeviceInfo> MidiOutput::getAvailableDevices()                                   { return {}; }
MidiDeviceInfo MidiOutput::getDefaultDevice()                                             { return {}; }
std::unique_ptr<MidiOutput> MidiOutput::openDevice (const String&)                        { return {}; }
std::unique_ptr<MidiOutput> MidiOutput::createNewDevice (const String&)                   { return {}; }
StringArray MidiOutput::getDevices()                                                      { return {}; }
int MidiOutput::getDefaultDeviceIndex()                                                   { return 0;}
std::unique_ptr<MidiOutput> MidiOutput::openDevice (int)                                  { return {}; }

MidiDeviceListConnection MidiDeviceListConnection::make (std::function<void()> cb)
{
    auto& broadcaster = MidiDeviceListConnectionBroadcaster::get();
    return { &broadcaster, broadcaster.add (std::move (cb)) };
}

#endif

} // namespace juce

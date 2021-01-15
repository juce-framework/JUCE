/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

#ifndef JUCE_LOG_COREMIDI_ERRORS
 #define JUCE_LOG_COREMIDI_ERRORS 1
#endif

namespace CoreMidiHelpers
{
    static bool checkError (OSStatus err, int lineNum)
    {
        if (err == noErr)
            return true;

       #if JUCE_LOG_COREMIDI_ERRORS
        Logger::writeToLog ("CoreMIDI error: " + String (lineNum) + " - " + String::toHexString ((int) err));
       #endif

        ignoreUnused (lineNum);
        return false;
    }

    #undef CHECK_ERROR
    #define CHECK_ERROR(a) CoreMidiHelpers::checkError (a, __LINE__)

    enum class ImplementationStrategy
    {
        onlyNew,
        both,
        onlyOld
    };

    #if (defined (MAC_OS_VERSION_11_0) || defined (__IPHONE_14_0))
     #if (MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_VERSION_11_0 || __IPHONE_OS_VERSION_MIN_REQUIRED >= __IPHONE_14_0)
      #define JUCE_HAS_NEW_COREMIDI_API 1
      #define JUCE_HAS_OLD_COREMIDI_API 0
      constexpr auto implementationStrategy = ImplementationStrategy::onlyNew;
     #else
      #define JUCE_HAS_NEW_COREMIDI_API 1
      #define JUCE_HAS_OLD_COREMIDI_API 1
      constexpr auto implementationStrategy = ImplementationStrategy::both;
     #endif
    #else
     #define JUCE_HAS_NEW_COREMIDI_API 0
     #define JUCE_HAS_OLD_COREMIDI_API 1
     constexpr auto implementationStrategy = ImplementationStrategy::onlyOld;
    #endif

    struct SenderBase
    {
        virtual ~SenderBase() noexcept = default;

        virtual void send (MIDIPortRef port, MIDIEndpointRef endpoint, const MidiMessage& m) = 0;
        virtual void send (MIDIPortRef port, MIDIEndpointRef endpoint, ump::Iterator b, ump::Iterator e) = 0;

        virtual ump::MidiProtocol getProtocol() const noexcept = 0;
    };

    template <ImplementationStrategy>
    struct Sender;

   #if JUCE_HAS_NEW_COREMIDI_API
    JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wunguarded-availability-new")

    template <>
    struct Sender<ImplementationStrategy::onlyNew> : public SenderBase
    {
        explicit Sender (MIDIEndpointRef ep)
            : umpConverter (getProtocolForEndpoint (ep))
        {}

        void send (MIDIPortRef port, MIDIEndpointRef endpoint, const MidiMessage& m) override
        {
            newSendImpl (port, endpoint, m);
        }

        void send (MIDIPortRef port, MIDIEndpointRef endpoint, ump::Iterator b, ump::Iterator e) override
        {
            newSendImpl (port, endpoint, b, e);
        }

        ump::MidiProtocol getProtocol() const noexcept override
        {
            return umpConverter.getProtocol() == ump::PacketProtocol::MIDI_2_0 ? ump::MidiProtocol::UMP_MIDI_2_0
                                                                               : ump::MidiProtocol::UMP_MIDI_1_0;
        }

    private:
        ump::GenericUMPConverter umpConverter;

        static ump::PacketProtocol getProtocolForEndpoint (MIDIEndpointRef ep) noexcept
        {
            SInt32 protocol = 0;
            CHECK_ERROR (MIDIObjectGetIntegerProperty (ep, kMIDIPropertyProtocolID, &protocol));

            return protocol == kMIDIProtocol_2_0 ? ump::PacketProtocol::MIDI_2_0
                                                 : ump::PacketProtocol::MIDI_1_0;
        }

        template <typename... Params>
        void newSendImpl (MIDIPortRef port, MIDIEndpointRef endpoint, Params&&... params)
        {
            // The converter protocol got out-of-sync with the device protocol
            jassert (getProtocolForEndpoint (endpoint) == umpConverter.getProtocol());

           #if JUCE_IOS
            const MIDITimeStamp timeStamp = mach_absolute_time();
           #else
            const MIDITimeStamp timeStamp = AudioGetCurrentHostTime();
           #endif

            MIDIEventList stackList = {};
            MIDIEventPacket* end = nullptr;

            const auto init = [&]
            {
                end = MIDIEventListInit (&stackList,
                                         umpConverter.getProtocol() == ump::PacketProtocol::MIDI_2_0 ? kMIDIProtocol_2_0
                                                                                                     : kMIDIProtocol_1_0);
            };

            const auto send = [&]
            {
                CHECK_ERROR (port != 0 ? MIDISendEventList (port, endpoint, &stackList)
                                       : MIDIReceivedEventList (endpoint, &stackList));
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

            umpConverter.convert (params..., [&] (const ump::View& view)
            {
                add (view);

                if (end != nullptr)
                    return;

                send();
                init();
                add (view);
            });

            send();
        }
    };

    JUCE_END_IGNORE_WARNINGS_GCC_LIKE
   #endif

   #if JUCE_HAS_OLD_COREMIDI_API
    template <>
    struct Sender<ImplementationStrategy::onlyOld> : public SenderBase
    {
        explicit Sender (MIDIEndpointRef) {}

        void send (MIDIPortRef port, MIDIEndpointRef endpoint, const MidiMessage& m) override
        {
            oldSendImpl (port, endpoint, m);
        }

        void send (MIDIPortRef port, MIDIEndpointRef endpoint, ump::Iterator b, ump::Iterator e) override
        {
            std::for_each (b, e, [&] (const ump::View& v)
            {
                bytestreamConverter.convert (v, 0.0, [&] (const MidiMessage& m)
                {
                    send (port, endpoint, m);
                });
            });
        }

        ump::MidiProtocol getProtocol() const noexcept override
        {
            return ump::MidiProtocol::bytestream;
        }

    private:
        ump::ToBytestreamConverter bytestreamConverter { 2048 };

        void oldSendImpl (MIDIPortRef port, MIDIEndpointRef endpoint, const MidiMessage& message)
        {
           #if JUCE_IOS
            const MIDITimeStamp timeStamp = mach_absolute_time();
           #else
            const MIDITimeStamp timeStamp = AudioGetCurrentHostTime();
           #endif

            HeapBlock<MIDIPacketList> allocatedPackets;
            MIDIPacketList stackPacket;
            auto* packetToSend = &stackPacket;
            auto dataSize = (size_t) message.getRawDataSize();

            if (message.isSysEx())
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
                    memcpy (p->data, message.getRawData() + pos, p->length);
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
                memcpy (p.data, message.getRawData(), dataSize);
            }
            else
            {
                jassertfalse; // packet too large to send!
                return;
            }

            if (port != 0)
                MIDISend (port, endpoint, packetToSend);
            else
                MIDIReceived (endpoint, packetToSend);
        }
    };
   #endif

   #if JUCE_HAS_NEW_COREMIDI_API && JUCE_HAS_OLD_COREMIDI_API
    template <>
    struct Sender<ImplementationStrategy::both>
    {
        explicit Sender (MIDIEndpointRef ep)
            : sender (makeImpl (ep))
        {}

        void send (MIDIPortRef port, MIDIEndpointRef endpoint, const MidiMessage& m)
        {
            sender->send (port, endpoint, m);
        }

        void send (MIDIPortRef port, MIDIEndpointRef endpoint, ump::Iterator b, ump::Iterator e)
        {
            sender->send (port, endpoint, b, e);
        }

        ump::MidiProtocol getProtocol() const noexcept
        {
            return sender->getProtocol();
        }

    private:
        static std::unique_ptr<SenderBase> makeImpl (MIDIEndpointRef ep)
        {
            if (@available (macOS 11, iOS 14, *))
                return std::make_unique<Sender<ImplementationStrategy::onlyNew>> (ep);

            return std::make_unique<Sender<ImplementationStrategy::onlyOld>> (ep);
        }

        std::unique_ptr<SenderBase> sender;
    };
   #endif

    using SenderToUse = Sender<implementationStrategy>;

    //==============================================================================
    class MidiPortAndEndpoint
    {
    public:
        MidiPortAndEndpoint (MIDIPortRef p, MIDIEndpointRef ep) noexcept
            : port (p), endpoint (ep), sender (ep)
        {}

        ~MidiPortAndEndpoint() noexcept
        {
            if (port != 0)
                MIDIPortDispose (port);

            // if port == nullptr, it means we created the endpoint, so it's safe to delete it
            if (port == 0 && endpoint != 0)
                MIDIEndpointDispose (endpoint);
        }

        void send (const MidiMessage& m)
        {
            sender.send (port, endpoint, m);
        }

        void send (ump::Iterator b, ump::Iterator e)
        {
            sender.send (port, endpoint, b, e);
        }

        bool canStop() const noexcept  { return port != 0; }
        void stop() const              { CHECK_ERROR (MIDIPortDisconnectSource (port, endpoint)); }

        ump::MidiProtocol getProtocol() const noexcept
        {
            return sender.getProtocol();
        }

    private:
        MIDIPortRef port;
        MIDIEndpointRef endpoint;

        SenderToUse sender;
    };

    static MidiDeviceInfo getMidiObjectInfo (MIDIObjectRef entity)
    {
        MidiDeviceInfo info;

        {
            ScopedCFString str;

            if (CHECK_ERROR (MIDIObjectGetStringProperty (entity, kMIDIPropertyName, &str.cfString)))
                info.name = String::fromCFString (str.cfString);
        }

        SInt32 objectID = 0;

        if (CHECK_ERROR (MIDIObjectGetIntegerProperty (entity, kMIDIPropertyUniqueID, &objectID)))
        {
            info.identifier = String (objectID);
        }
        else
        {
            ScopedCFString str;

            if (CHECK_ERROR (MIDIObjectGetStringProperty (entity, kMIDIPropertyUniqueID, &str.cfString)))
                info.identifier = String::fromCFString (str.cfString);
        }

        return info;
    }

    static MidiDeviceInfo getEndpointInfo (MIDIEndpointRef endpoint, bool isExternal)
    {
        // NB: don't attempt to use nullptr for refs - it fails in some types of build.
        MIDIEntityRef entity = 0;
        MIDIEndpointGetEntity (endpoint, &entity);

        // probably virtual
        if (entity == 0)
            return getMidiObjectInfo (endpoint);

        auto result = getMidiObjectInfo (endpoint);

        // endpoint is empty - try the entity
        if (result == MidiDeviceInfo())
            result = getMidiObjectInfo (entity);

        // now consider the device
        MIDIDeviceRef device = 0;
        MIDIEntityGetDevice (entity, &device);

        if (device != 0)
        {
            auto info = getMidiObjectInfo (device);

            if (info != MidiDeviceInfo())
            {
                // if an external device has only one entity, throw away
                // the endpoint name and just use the device name
                if (isExternal && MIDIDeviceGetNumberOfEntities (device) < 2)
                {
                    result = info;
                }
                else if (! result.name.startsWithIgnoreCase (info.name))
                {
                    // prepend the device name and identifier to the entity's
                    result.name = (info.name + " " + result.name).trimEnd();
                    result.identifier = info.identifier + " " + result.identifier;
                }
            }
        }

        return result;
    }

    static MidiDeviceInfo getConnectedEndpointInfo (MIDIEndpointRef endpoint)
    {
        MidiDeviceInfo result;

        // Does the endpoint have connections?
        CFDataRef connections = nullptr;
        int numConnections = 0;

        MIDIObjectGetDataProperty (endpoint, kMIDIPropertyConnectionUniqueID, &connections);

        if (connections != nullptr)
        {
            numConnections = ((int) CFDataGetLength (connections)) / (int) sizeof (MIDIUniqueID);

            if (numConnections > 0)
            {
                auto* pid = reinterpret_cast<const SInt32*> (CFDataGetBytePtr (connections));

                for (int i = 0; i < numConnections; ++i, ++pid)
                {
                    auto id = (MIDIUniqueID) ByteOrder::swapIfLittleEndian ((uint32) *pid);
                    MIDIObjectRef connObject;
                    MIDIObjectType connObjectType;
                    auto err = MIDIObjectFindByUniqueID (id, &connObject, &connObjectType);

                    if (err == noErr)
                    {
                        MidiDeviceInfo deviceInfo;

                        if (connObjectType == kMIDIObjectType_ExternalSource
                             || connObjectType == kMIDIObjectType_ExternalDestination)
                        {
                            // Connected to an external device's endpoint (10.3 and later).
                            deviceInfo = getEndpointInfo (static_cast<MIDIEndpointRef> (connObject), true);
                        }
                        else
                        {
                            // Connected to an external device (10.2) (or something else, catch-all)
                            deviceInfo = getMidiObjectInfo (connObject);
                        }

                        if (deviceInfo != MidiDeviceInfo())
                        {
                            if (result.name.isNotEmpty())        result.name += ", ";
                            if (result.identifier.isNotEmpty())  result.identifier += ", ";

                            result.name       += deviceInfo.name;
                            result.identifier += deviceInfo.identifier;
                        }
                    }
                }
            }

            CFRelease (connections);
        }

        // Here, either the endpoint had no connections, or we failed to obtain names for them.
        if (result == MidiDeviceInfo())
            return getEndpointInfo (endpoint, false);

        return result;
    }

    static int createUniqueIDForMidiPort (String deviceName, bool isInput)
    {
        String uniqueID;

       #ifdef JucePlugin_CFBundleIdentifier
        uniqueID = JUCE_STRINGIFY (JucePlugin_CFBundleIdentifier);
       #else
        auto appBundle = File::getSpecialLocation (File::currentApplicationFile);
        ScopedCFString appBundlePath (appBundle.getFullPathName());

        if (auto bundleURL = CFURLCreateWithFileSystemPath (kCFAllocatorDefault,
                                                            appBundlePath.cfString,
                                                            kCFURLPOSIXPathStyle,
                                                            true))
        {
            auto bundleRef = CFBundleCreate (kCFAllocatorDefault, bundleURL);
            CFRelease (bundleURL);

            if (bundleRef != nullptr)
            {
                if (auto bundleId = CFBundleGetIdentifier (bundleRef))
                    uniqueID = String::fromCFString (bundleId);

                CFRelease (bundleRef);
            }
        }
       #endif

        if (uniqueID.isEmpty())
            uniqueID = String (Random::getSystemRandom().nextInt (1024));

        uniqueID += "." + deviceName + (isInput ? ".input" : ".output");
        return uniqueID.hashCode();
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

    static void globalSystemChangeCallback (const MIDINotification*, void*)
    {
        // TODO.. Should pass-on this notification..
    }

    static String getGlobalMidiClientName()
    {
        if (auto* app = JUCEApplicationBase::getInstance())
            return app->getApplicationName();

        return "JUCE";
    }

    static MIDIClientRef getGlobalMidiClient()
    {
        static MIDIClientRef globalMidiClient = 0;

        if (globalMidiClient == 0)
        {
            // Since OSX 10.6, the MIDIClientCreate function will only work
            // correctly when called from the message thread!
            JUCE_ASSERT_MESSAGE_THREAD

            enableSimulatorMidiSession();

            ScopedCFString name (getGlobalMidiClientName());
            CHECK_ERROR (MIDIClientCreate (name.cfString, &globalSystemChangeCallback, nullptr, &globalMidiClient));
        }

        return globalMidiClient;
    }

    static Array<MidiDeviceInfo> findDevices (bool forInput)
    {
        // It seems that OSX can be a bit picky about the thread that's first used to
        // search for devices. It's safest to use the message thread for calling this.
        JUCE_ASSERT_MESSAGE_THREAD

        if (getGlobalMidiClient() == 0)
        {
            jassertfalse;
            return {};
        }

        enableSimulatorMidiSession();

        Array<MidiDeviceInfo> devices;
        auto numDevices = (forInput ? MIDIGetNumberOfSources() : MIDIGetNumberOfDestinations());

        for (ItemCount i = 0; i < numDevices; ++i)
        {
            MidiDeviceInfo deviceInfo;

            if (auto dest = forInput ? MIDIGetSource (i) : MIDIGetDestination (i))
                deviceInfo = getConnectedEndpointInfo (dest);

            if (deviceInfo == MidiDeviceInfo())
                deviceInfo.name = deviceInfo.identifier = "<error>";

            devices.add (deviceInfo);
        }

        return devices;
    }

    //==============================================================================
    template <ImplementationStrategy>
    struct Receiver;

   #if JUCE_HAS_NEW_COREMIDI_API
    template <>
    struct Receiver<ImplementationStrategy::onlyNew>
    {
        Receiver (ump::PacketProtocol protocol, ump::Receiver& receiver)
            : u32InputHandler (std::make_unique<ump::U32ToUMPHandler> (protocol, receiver))
        {}

        Receiver (MidiInput& input, MidiInputCallback& callback)
            : u32InputHandler (std::make_unique<ump::U32ToBytestreamHandler> (input, callback))
        {}

        void dispatch (const MIDIEventList& list, double time) const
        {
            auto* packet = &list.packet[0];

            for (uint32_t i = 0; i < list.numPackets; ++i)
            {
                static_assert (sizeof (uint32_t) == sizeof (UInt32)
                               && alignof (uint32_t) == alignof (UInt32),
                               "If this fails, the cast below will be broken too!");
                u32InputHandler->pushMidiData (reinterpret_cast<const uint32_t*> (packet->words),
                                               reinterpret_cast<const uint32_t*> (packet->words + packet->wordCount),
                                               time);

                packet = MIDIEventPacketNext (packet);
            }
        }

    private:
        std::unique_ptr<ump::U32InputHandler> u32InputHandler;
    };
   #endif

   #if JUCE_HAS_OLD_COREMIDI_API
    template <>
    struct Receiver<ImplementationStrategy::onlyOld>
    {
        Receiver (ump::PacketProtocol protocol, ump::Receiver& receiver)
            : bytestreamInputHandler (std::make_unique<ump::BytestreamToUMPHandler> (protocol, receiver))
        {}

        Receiver (MidiInput& input, MidiInputCallback& callback)
            : bytestreamInputHandler (std::make_unique<ump::BytestreamToBytestreamHandler> (input, callback))
        {}

        void dispatch (const MIDIPacketList& list, double time) const
        {
            auto* packet = &list.packet[0];

            for (unsigned int i = 0; i < list.numPackets; ++i)
            {
                auto len = readUnaligned<decltype (packet->length)> (&(packet->length));
                bytestreamInputHandler->pushMidiData (packet->data, len, time);

                packet = MIDIPacketNext (packet);
            }
        }

    private:
        std::unique_ptr<ump::BytestreamInputHandler> bytestreamInputHandler;
    };
   #endif

   #if JUCE_HAS_NEW_COREMIDI_API && JUCE_HAS_OLD_COREMIDI_API
    template <>
    struct Receiver<ImplementationStrategy::both>
    {
        Receiver (ump::PacketProtocol protocol, ump::Receiver& receiver)
            : newReceiver (protocol, receiver), oldReceiver (protocol, receiver)
        {}

        Receiver (MidiInput& input, MidiInputCallback& callback)
            : newReceiver (input, callback), oldReceiver (input, callback)
        {}

        void dispatch (const MIDIEventList& list, double time) const
        {
            newReceiver.dispatch (list, time);
        }

        void dispatch (const MIDIPacketList& list, double time) const
        {
            oldReceiver.dispatch (list, time);
        }

    private:
        Receiver<ImplementationStrategy::onlyNew> newReceiver;
        Receiver<ImplementationStrategy::onlyOld> oldReceiver;
    };
   #endif

    using ReceiverToUse = Receiver<implementationStrategy>;

    class MidiPortAndCallback;
    CriticalSection callbackLock;
    Array<MidiPortAndCallback*> activeCallbacks;

    class MidiPortAndCallback
    {
    public:
        MidiPortAndCallback (MidiInput& inputIn, ReceiverToUse receiverIn)
            : input (&inputIn), receiver (std::move (receiverIn))
        {}

        ~MidiPortAndCallback()
        {
            active = false;

            {
                const ScopedLock sl (callbackLock);
                activeCallbacks.removeFirstMatchingValue (this);
            }

            if (portAndEndpoint != nullptr && portAndEndpoint->canStop())
                portAndEndpoint->stop();
        }

        template <typename EventList>
        void handlePackets (const EventList& list)
        {
            const auto time = Time::getMillisecondCounterHiRes() * 0.001;

            const ScopedLock sl (callbackLock);

            if (activeCallbacks.contains (this) && active)
                receiver.dispatch (list, time);
        }

        MidiInput* input = nullptr;
        std::atomic<bool> active { false };

        ReceiverToUse receiver;

        std::unique_ptr<MidiPortAndEndpoint> portAndEndpoint;

    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MidiPortAndCallback)
    };

    //==============================================================================
    static Array<MIDIEndpointRef> getEndpoints (bool isInput)
    {
        Array<MIDIEndpointRef> endpoints;
        auto numDevices = (isInput ? MIDIGetNumberOfSources() : MIDIGetNumberOfDestinations());

        for (ItemCount i = 0; i < numDevices; ++i)
            endpoints.add (isInput ? MIDIGetSource (i) : MIDIGetDestination (i));

        return endpoints;
    }

    struct CreatorFunctionPointers
    {
        OSStatus (*createInputPort) (ump::PacketProtocol protocol,
                                     MIDIClientRef client,
                                     CFStringRef portName,
                                     void* refCon,
                                     MIDIPortRef* outPort);

        OSStatus (*createDestination) (ump::PacketProtocol protocol,
                                       MIDIClientRef client,
                                       CFStringRef name,
                                       void* refCon,
                                       MIDIEndpointRef* outDest);

        OSStatus (*createSource) (ump::PacketProtocol protocol,
                                  MIDIClientRef client,
                                  CFStringRef name,
                                  MIDIEndpointRef* outSrc);
    };

    template <ImplementationStrategy>
    struct CreatorFunctions;

   #if JUCE_HAS_NEW_COREMIDI_API
    JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wunguarded-availability-new")

    template <>
    struct CreatorFunctions<ImplementationStrategy::onlyNew>
    {
        static OSStatus createInputPort (ump::PacketProtocol protocol,
                                         MIDIClientRef client,
                                         CFStringRef portName,
                                         void* refCon,
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
                                           void* refCon,
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

        static constexpr CreatorFunctionPointers getCreatorFunctionPointers()
        {
            return { createInputPort, createDestination, createSource };
        }

    private:
        static constexpr MIDIProtocolID convertToPacketProtocol (ump::PacketProtocol p)
        {
            return p == ump::PacketProtocol::MIDI_2_0 ? kMIDIProtocol_2_0
                                                      : kMIDIProtocol_1_0;
        }

        static void newMidiInputProc (const MIDIEventList* list, void* readProcRefCon, void*)
        {
            static_cast<MidiPortAndCallback*> (readProcRefCon)->handlePackets (*list);
        }
    };

    JUCE_END_IGNORE_WARNINGS_GCC_LIKE
   #endif

   #if JUCE_HAS_OLD_COREMIDI_API
    template <>
    struct CreatorFunctions<ImplementationStrategy::onlyOld>
    {
        static OSStatus createInputPort (ump::PacketProtocol,
                                         MIDIClientRef client,
                                         CFStringRef portName,
                                         void* refCon,
                                         MIDIPortRef* outPort)
        {
            return MIDIInputPortCreate (client, portName, oldMidiInputProc, refCon, outPort);
        }

        static OSStatus createDestination (ump::PacketProtocol,
                                           MIDIClientRef client,
                                           CFStringRef name,
                                           void* refCon,
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

        static constexpr CreatorFunctionPointers getCreatorFunctionPointers()
        {
            return { createInputPort, createDestination, createSource };
        }

    private:
        static void oldMidiInputProc (const MIDIPacketList* list, void* readProcRefCon, void*)
        {
            static_cast<MidiPortAndCallback*> (readProcRefCon)->handlePackets (*list);
        }
    };
   #endif

   #if JUCE_HAS_NEW_COREMIDI_API && JUCE_HAS_OLD_COREMIDI_API
    template <>
    struct CreatorFunctions<ImplementationStrategy::both>
    {
        static OSStatus createInputPort (ump::PacketProtocol protocol,
                                         MIDIClientRef client,
                                         CFStringRef portName,
                                         void* refCon,
                                         MIDIPortRef* outPort)
        {
            return getCreatorFunctionPointers().createInputPort (protocol, client, portName, refCon, outPort);
        }

        static OSStatus createDestination (ump::PacketProtocol protocol,
                                           MIDIClientRef client,
                                           CFStringRef name,
                                           void* refCon,
                                           MIDIEndpointRef* outDest)
        {
            return getCreatorFunctionPointers().createDestination (protocol, client, name, refCon, outDest);
        }

        static OSStatus createSource (ump::PacketProtocol protocol,
                                      MIDIClientRef client,
                                      CFStringRef name,
                                      MIDIEndpointRef* outSrc)
        {
            return getCreatorFunctionPointers().createSource (protocol, client, name, outSrc);
        }

    private:
        static CreatorFunctionPointers getCreatorFunctionPointers()
        {
            if (@available (macOS 11, iOS 14, *))
                return CreatorFunctions<ImplementationStrategy::onlyNew>::getCreatorFunctionPointers();

            return CreatorFunctions<ImplementationStrategy::onlyOld>::getCreatorFunctionPointers();
        }
    };
   #endif

    using CreatorFunctionsToUse = CreatorFunctions<implementationStrategy>;
}

//==============================================================================
class MidiInput::Pimpl : public CoreMidiHelpers::MidiPortAndCallback
{
public:
    using MidiPortAndCallback::MidiPortAndCallback;

    static std::unique_ptr<Pimpl> makePimpl (MidiInput& midiInput,
                                             ump::PacketProtocol packetProtocol,
                                             ump::Receiver& umpReceiver)
    {
        return std::make_unique<Pimpl> (midiInput, CoreMidiHelpers::ReceiverToUse (packetProtocol, umpReceiver));
    }

    static std::unique_ptr<Pimpl> makePimpl (MidiInput& midiInput,
                                             MidiInputCallback* midiInputCallback)
    {
        if (midiInputCallback == nullptr)
            return {};

        return std::make_unique<Pimpl> (midiInput, CoreMidiHelpers::ReceiverToUse (midiInput, *midiInputCallback));
    }

    template <typename... Args>
    static std::unique_ptr<MidiInput> makeInput (const String& name,
                                                 const String& identifier,
                                                 Args&&... args)
    {
        using namespace CoreMidiHelpers;

        if (auto midiInput = rawToUniquePtr (new MidiInput (name, identifier)))
        {
            if ((midiInput->internal = makePimpl (*midiInput, std::forward<Args> (args)...)))
            {
                const ScopedLock sl (callbackLock);
                activeCallbacks.add (midiInput->internal.get());

                return midiInput;
            }
        }

        return {};
    }

    template <typename... Args>
    static std::unique_ptr<MidiInput> openDevice (ump::PacketProtocol protocol,
                                                  const String& deviceIdentifier,
                                                  Args&&... args)
    {
        using namespace CoreMidiHelpers;

        if (deviceIdentifier.isEmpty())
            return {};

        if (auto client = getGlobalMidiClient())
        {
            for (auto& endpoint : getEndpoints (true))
            {
                auto endpointInfo = getConnectedEndpointInfo (endpoint);

                if (deviceIdentifier != endpointInfo.identifier)
                    continue;

                ScopedCFString cfName;

                if (! CHECK_ERROR (MIDIObjectGetStringProperty (endpoint, kMIDIPropertyName, &cfName.cfString)))
                    continue;

                if (auto input = makeInput (endpointInfo.name, endpointInfo.identifier, std::forward<Args> (args)...))
                {
                    MIDIPortRef port;

                    if (! CHECK_ERROR (CreatorFunctionsToUse::createInputPort (protocol, client, cfName.cfString, input->internal.get(), &port)))
                        continue;

                    if (! CHECK_ERROR (MIDIPortConnectSource (port, endpoint, nullptr)))
                    {
                        CHECK_ERROR (MIDIPortDispose (port));
                        continue;
                    }

                    input->internal->portAndEndpoint = std::make_unique<MidiPortAndEndpoint> (port, endpoint);
                    return input;
                }
            }
        }

        return {};
    }

    template <typename... Args>
    static std::unique_ptr<MidiInput> createDevice (ump::PacketProtocol protocol,
                                                    const String& deviceName,
                                                    Args&&... args)
    {
        using namespace CoreMidiHelpers;

        if (auto client = getGlobalMidiClient())
        {
            auto deviceIdentifier = createUniqueIDForMidiPort (deviceName, true);

            if (auto input = makeInput (deviceName, String (deviceIdentifier), std::forward<Args> (args)...))
            {
                MIDIEndpointRef endpoint;
                ScopedCFString name (deviceName);

                auto err = CreatorFunctionsToUse::createDestination (protocol, client, name.cfString, input->internal.get(), &endpoint);

               #if JUCE_IOS
                if (err == kMIDINotPermitted)
                {
                    // If you've hit this assertion then you probably haven't enabled the "Audio Background Capability"
                    // setting in the iOS exporter for your app - this is required if you want to create a MIDI device!
                    jassertfalse;
                    return {};
                }
               #endif

                if (! CHECK_ERROR (err))
                    return {};

                if (! CHECK_ERROR (MIDIObjectSetIntegerProperty (endpoint, kMIDIPropertyUniqueID, (SInt32) deviceIdentifier)))
                    return {};

                input->internal->portAndEndpoint = std::make_unique<MidiPortAndEndpoint> ((MIDIPortRef) 0, endpoint);
                return input;
            }
        }

        return {};
    }
};

//==============================================================================
Array<MidiDeviceInfo> MidiInput::getAvailableDevices()
{
    return CoreMidiHelpers::findDevices (true);
}

MidiDeviceInfo MidiInput::getDefaultDevice()
{
    return getAvailableDevices().getFirst();
}

std::unique_ptr<MidiInput> MidiInput::openDevice (const String& deviceIdentifier, MidiInputCallback* callback)
{
    if (callback == nullptr)
        return nullptr;

    return Pimpl::openDevice (ump::PacketProtocol::MIDI_1_0,
                              deviceIdentifier,
                              callback);
}

std::unique_ptr<MidiInput> MidiInput::createNewDevice (const String& deviceName, MidiInputCallback* callback)
{
    return Pimpl::createDevice (ump::PacketProtocol::MIDI_1_0,
                                deviceName,
                                callback);
}

StringArray MidiInput::getDevices()
{
    StringArray deviceNames;

    for (auto& d : getAvailableDevices())
        deviceNames.add (d.name);

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

MidiInput::~MidiInput() = default;

void MidiInput::start()
{
    const ScopedLock sl (CoreMidiHelpers::callbackLock);
    internal->active = true;
}

void MidiInput::stop()
{
    const ScopedLock sl (CoreMidiHelpers::callbackLock);
    internal->active = false;
}

//==============================================================================
class MidiOutput::Pimpl : public CoreMidiHelpers::MidiPortAndEndpoint
{
public:
    using MidiPortAndEndpoint::MidiPortAndEndpoint;
};

Array<MidiDeviceInfo> MidiOutput::getAvailableDevices()
{
    return CoreMidiHelpers::findDevices (false);
}

MidiDeviceInfo MidiOutput::getDefaultDevice()
{
    return getAvailableDevices().getFirst();
}

std::unique_ptr<MidiOutput> MidiOutput::openDevice (const String& deviceIdentifier)
{
    if (deviceIdentifier.isEmpty())
        return {};

    using namespace CoreMidiHelpers;

    if (auto client = getGlobalMidiClient())
    {
        for (auto& endpoint : getEndpoints (false))
        {
            auto endpointInfo = getConnectedEndpointInfo (endpoint);

            if (deviceIdentifier != endpointInfo.identifier)
                continue;

            ScopedCFString cfName;

            if (! CHECK_ERROR (MIDIObjectGetStringProperty (endpoint, kMIDIPropertyName, &cfName.cfString)))
                continue;

            MIDIPortRef port;

            if (! CHECK_ERROR (MIDIOutputPortCreate (client, cfName.cfString, &port)))
                continue;

            auto midiOutput = rawToUniquePtr (new MidiOutput (endpointInfo.name, endpointInfo.identifier));
            midiOutput->internal = std::make_unique<Pimpl> (port, endpoint);

            return midiOutput;
        }
    }

    return {};
}

std::unique_ptr<MidiOutput> MidiOutput::createNewDevice (const String& deviceName)
{
    using namespace CoreMidiHelpers;

    if (auto client = getGlobalMidiClient())
    {
        MIDIEndpointRef endpoint;

        ScopedCFString name (deviceName);

        auto err = CreatorFunctionsToUse::createSource (ump::PacketProtocol::MIDI_1_0, client, name.cfString, &endpoint);

       #if JUCE_IOS
        if (err == kMIDINotPermitted)
        {
            // If you've hit this assertion then you probably haven't enabled the "Audio Background Capability"
            // setting in the iOS exporter for your app - this is required if you want to create a MIDI device!
            jassertfalse;
            return {};
        }
       #endif

        if (! CHECK_ERROR (err))
            return {};

        auto deviceIdentifier = createUniqueIDForMidiPort (deviceName, false);

        if (! CHECK_ERROR (MIDIObjectSetIntegerProperty (endpoint, kMIDIPropertyUniqueID, (SInt32) deviceIdentifier)))
            return {};

        auto midiOutput = rawToUniquePtr (new MidiOutput (deviceName, String (deviceIdentifier)));
        midiOutput->internal = std::make_unique<Pimpl> ((UInt32) 0, endpoint);

        return midiOutput;
    }

    return {};
}

StringArray MidiOutput::getDevices()
{
    StringArray deviceNames;

    for (auto& d : getAvailableDevices())
        deviceNames.add (d.name);

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
    internal->send (message);
}

#undef CHECK_ERROR

} // namespace juce

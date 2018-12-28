/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

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

    //==============================================================================
    struct ScopedCFString
    {
        ScopedCFString() noexcept {}
        ~ScopedCFString() noexcept  { if (cfString != nullptr) CFRelease (cfString); }

        CFStringRef cfString = {};
    };

    static String getMidiObjectName (MIDIObjectRef entity)
    {
        String result;
        CFStringRef str = nullptr;
        MIDIObjectGetStringProperty (entity, kMIDIPropertyName, &str);

        if (str != nullptr)
        {
            result = String::fromCFString (str);
            CFRelease (str);
        }

        return result;
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

    static String getEndpointName (MIDIEndpointRef endpoint, bool isExternal)
    {
        auto result = getMidiObjectName (endpoint);

        MIDIEntityRef entity = 0;  // NB: don't attempt to use nullptr for refs - it fails in some types of build.
        MIDIEndpointGetEntity (endpoint, &entity);

        if (entity == 0)
            return result; // probably virtual

        if (result.isEmpty())
            result = getMidiObjectName (entity);  // endpoint name is empty - try the entity

        // now consider the device's name
        MIDIDeviceRef device = 0;
        MIDIEntityGetDevice (entity, &device);

        if (device != 0)
        {
            auto deviceName = getMidiObjectName (device);

            if (deviceName.isNotEmpty())
            {
                // if an external device has only one entity, throw away
                // the endpoint name and just use the device name
                if (isExternal && MIDIDeviceGetNumberOfEntities (device) < 2)
                {
                    result = deviceName;
                }
                else if (! result.startsWithIgnoreCase (deviceName))
                {
                    // prepend the device name to the entity name
                    result = (deviceName + " " + result).trimEnd();
                }
            }
        }

        return result;
    }

    static String getConnectedEndpointName (MIDIEndpointRef endpoint)
    {
        String result;

        // Does the endpoint have connections?
        CFDataRef connections = nullptr;
        int numConnections = 0;

        MIDIObjectGetDataProperty (endpoint, kMIDIPropertyConnectionUniqueID, &connections);

        if (connections != nullptr)
        {
            numConnections = ((int) CFDataGetLength (connections)) / (int) sizeof (MIDIUniqueID);

            if (numConnections > 0)
            {
                auto pid = reinterpret_cast<const SInt32*> (CFDataGetBytePtr (connections));

                for (int i = 0; i < numConnections; ++i, ++pid)
                {
                    auto uid = (MIDIUniqueID) ByteOrder::swapIfLittleEndian ((uint32) *pid);
                    MIDIObjectRef connObject;
                    MIDIObjectType connObjectType;
                    auto err = MIDIObjectFindByUniqueID (uid, &connObject, &connObjectType);

                    if (err == noErr)
                    {
                        String s;

                        if (connObjectType == kMIDIObjectType_ExternalSource
                             || connObjectType == kMIDIObjectType_ExternalDestination)
                        {
                            // Connected to an external device's endpoint (10.3 and later).
                            s = getEndpointName (static_cast<MIDIEndpointRef> (connObject), true);
                        }
                        else
                        {
                            // Connected to an external device (10.2) (or something else, catch-all)
                            s = getMidiObjectName (connObject);
                        }

                        if (s.isNotEmpty())
                        {
                            if (result.isNotEmpty())
                                result += ", ";

                            result += s;
                        }
                    }
                }
            }

            CFRelease (connections);
        }

        if (result.isEmpty())  // Here, either the endpoint had no connections, or we failed to obtain names for them.
            result = getEndpointName (endpoint, false);

        return result;
    }

    static void setUniqueIdForMidiPort (MIDIObjectRef device, const String& portName, bool isInput)
    {
        String portUniqueId;
       #if defined (JucePlugin_CFBundleIdentifier)
        portUniqueId = JUCE_STRINGIFY (JucePlugin_CFBundleIdentifier);
       #else
        auto appBundle = File::getSpecialLocation (File::currentApplicationFile);

        if (auto bundleURL = CFURLCreateWithFileSystemPath (kCFAllocatorDefault, appBundle.getFullPathName().toCFString(),
                                                            kCFURLPOSIXPathStyle, true))
        {
            auto bundleRef = CFBundleCreate (kCFAllocatorDefault, bundleURL);
            CFRelease (bundleURL);

            if (bundleRef != nullptr)
            {
                if (auto bundleId = CFBundleGetIdentifier (bundleRef))
                    portUniqueId = String::fromCFString (bundleId);

                CFRelease (bundleRef);
            }
        }
       #endif

        if (portUniqueId.isNotEmpty())
        {
            portUniqueId += "." + portName + (isInput ? ".input" : ".output");

            CHECK_ERROR (MIDIObjectSetStringProperty (device, kMIDIPropertyUniqueID, portUniqueId.toCFString()));
        }
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

            CoreMidiHelpers::ScopedCFString name;
            name.cfString = getGlobalMidiClientName().toCFString();
            CHECK_ERROR (MIDIClientCreate (name.cfString, &globalSystemChangeCallback, nullptr, &globalMidiClient));
        }

        return globalMidiClient;
    }

    static StringArray findDevices (bool forInput)
    {
        // It seems that OSX can be a bit picky about the thread that's first used to
        // search for devices. It's safest to use the message thread for calling this.
        JUCE_ASSERT_MESSAGE_THREAD

        if (getGlobalMidiClient() == 0)
        {
            jassertfalse;
            return {};
        }

        StringArray s;
        enableSimulatorMidiSession();

        auto num = forInput ? MIDIGetNumberOfSources()
                            : MIDIGetNumberOfDestinations();

        for (ItemCount i = 0; i < num; ++i)
        {
            String name;

            if (auto dest = forInput ? MIDIGetSource (i) : MIDIGetDestination (i))
                name = getConnectedEndpointName (dest);

            if (name.isEmpty())
                name = "<error>";

            s.add (name);
        }

        return s;
    }

    //==============================================================================
    class MidiPortAndEndpoint
    {
    public:
        MidiPortAndEndpoint (MIDIPortRef p, MIDIEndpointRef ep) noexcept
            : port (p), endPoint (ep)
        {
        }

        ~MidiPortAndEndpoint() noexcept
        {
            if (port != 0)
                MIDIPortDispose (port);

            if (port == 0 && endPoint != 0) // if port == nullptr, it means we created the endpoint, so it's safe to delete it
                MIDIEndpointDispose (endPoint);
        }

        void send (const MIDIPacketList* packets) noexcept
        {
            if (port != 0)
                MIDISend (port, endPoint, packets);
            else
                MIDIReceived (endPoint, packets);
        }

        MIDIPortRef port;
        MIDIEndpointRef endPoint;
    };

    //==============================================================================
    struct MidiPortAndCallback;
    CriticalSection callbackLock;
    Array<MidiPortAndCallback*> activeCallbacks;

    struct MidiPortAndCallback
    {
        MidiPortAndCallback (MidiInputCallback& cb)  : callback (cb) {}

        ~MidiPortAndCallback()
        {
            active = false;

            {
                const ScopedLock sl (callbackLock);
                activeCallbacks.removeFirstMatchingValue (this);
            }

            if (portAndEndpoint != nullptr && portAndEndpoint->port != 0)
                CHECK_ERROR (MIDIPortDisconnectSource (portAndEndpoint->port, portAndEndpoint->endPoint));
        }

        void handlePackets (const MIDIPacketList* pktlist)
        {
            auto time = Time::getMillisecondCounterHiRes() * 0.001;

            const ScopedLock sl (callbackLock);

            if (activeCallbacks.contains (this) && active)
            {
                auto* packet = &pktlist->packet[0];

                for (unsigned int i = 0; i < pktlist->numPackets; ++i)
                {
                    auto len = readUnaligned<decltype (packet->length)> (&(packet->length));
                    concatenator.pushMidiData (packet->data, (int) len, time, input, callback);

                    packet = MIDIPacketNext (packet);
                }
            }
        }

        MidiInput* input = nullptr;
        std::unique_ptr<MidiPortAndEndpoint> portAndEndpoint;
        std::atomic<bool> active { false };

    private:
        MidiInputCallback& callback;
        MidiDataConcatenator concatenator { 2048 };
    };

    static void midiInputProc (const MIDIPacketList* pktlist, void* readProcRefCon, void* /*srcConnRefCon*/)
    {
        static_cast<MidiPortAndCallback*> (readProcRefCon)->handlePackets (pktlist);
    }
}

//==============================================================================
StringArray MidiOutput::getDevices()        { return CoreMidiHelpers::findDevices (false); }
int MidiOutput::getDefaultDeviceIndex()     { return 0; }

MidiOutput* MidiOutput::openDevice (int index)
{
    MidiOutput* mo = nullptr;

    if (auto client = CoreMidiHelpers::getGlobalMidiClient())
    {
        if (isPositiveAndBelow (index, MIDIGetNumberOfDestinations()))
        {
            auto endPoint = MIDIGetDestination ((ItemCount) index);

            CoreMidiHelpers::ScopedCFString pname;

            if (CHECK_ERROR (MIDIObjectGetStringProperty (endPoint, kMIDIPropertyName, &pname.cfString)))
            {
                MIDIPortRef port;
                auto deviceName = CoreMidiHelpers::getConnectedEndpointName (endPoint);

                if (CHECK_ERROR (MIDIOutputPortCreate (client, pname.cfString, &port)))
                {
                    mo = new MidiOutput (deviceName);
                    mo->internal = new CoreMidiHelpers::MidiPortAndEndpoint (port, endPoint);
                }
            }
        }
    }

    return mo;
}

MidiOutput* MidiOutput::createNewDevice (const String& deviceName)
{
    if (auto client = CoreMidiHelpers::getGlobalMidiClient())
    {
        MIDIEndpointRef endPoint;

        CoreMidiHelpers::ScopedCFString name;
        name.cfString = deviceName.toCFString();

        if (CHECK_ERROR (MIDISourceCreate (client, name.cfString, &endPoint)))
        {
            CoreMidiHelpers::setUniqueIdForMidiPort (endPoint, deviceName, false);

            auto mo = new MidiOutput (deviceName);
            mo->internal = new CoreMidiHelpers::MidiPortAndEndpoint (0, endPoint);
            return mo;
        }
    }

    return nullptr;
}

MidiOutput::~MidiOutput()
{
    stopBackgroundThread();

    delete static_cast<CoreMidiHelpers::MidiPortAndEndpoint*> (internal);
}

void MidiOutput::sendMessageNow (const MidiMessage& message)
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

    static_cast<CoreMidiHelpers::MidiPortAndEndpoint*> (internal)->send (packetToSend);
}

//==============================================================================
StringArray MidiInput::getDevices()     { return CoreMidiHelpers::findDevices (true); }
int MidiInput::getDefaultDeviceIndex()  { return 0; }

MidiInput* MidiInput::openDevice (int index, MidiInputCallback* callback)
{
    jassert (callback != nullptr);

    using namespace CoreMidiHelpers;
    MidiInput* newInput = nullptr;

    if (auto client = getGlobalMidiClient())
    {
        if (isPositiveAndBelow (index, MIDIGetNumberOfSources()))
        {
            if (auto endPoint = MIDIGetSource ((ItemCount) index))
            {
                ScopedCFString name;

                if (CHECK_ERROR (MIDIObjectGetStringProperty (endPoint, kMIDIPropertyName, &name.cfString)))
                {
                    MIDIPortRef port;
                    std::unique_ptr<MidiPortAndCallback> mpc (new MidiPortAndCallback (*callback));

                    if (CHECK_ERROR (MIDIInputPortCreate (client, name.cfString, midiInputProc, mpc.get(), &port)))
                    {
                        if (CHECK_ERROR (MIDIPortConnectSource (port, endPoint, nullptr)))
                        {
                            mpc->portAndEndpoint.reset (new MidiPortAndEndpoint (port, endPoint));

                            newInput = new MidiInput (getDevices() [index]);
                            mpc->input = newInput;
                            newInput->internal = mpc.get();

                            const ScopedLock sl (callbackLock);
                            activeCallbacks.add (mpc.release());
                        }
                        else
                        {
                            CHECK_ERROR (MIDIPortDispose (port));
                        }
                    }
                }
            }
        }
    }

    return newInput;
}

MidiInput* MidiInput::createNewDevice (const String& deviceName, MidiInputCallback* callback)
{
    jassert (callback != nullptr);
    using namespace CoreMidiHelpers;

    if (auto client = getGlobalMidiClient())
    {
        std::unique_ptr<MidiPortAndCallback> mpc (new MidiPortAndCallback (*callback));
        mpc->active = false;

        MIDIEndpointRef endPoint;
        ScopedCFString name;
        name.cfString = deviceName.toCFString();

        if (CHECK_ERROR (MIDIDestinationCreate (client, name.cfString, midiInputProc, mpc.get(), &endPoint)))
        {
            setUniqueIdForMidiPort (endPoint, deviceName, true);

            mpc->portAndEndpoint.reset (new MidiPortAndEndpoint (0, endPoint));

            auto mi = new MidiInput (deviceName);
            mpc->input = mi;
            mi->internal = mpc.get();

            const ScopedLock sl (callbackLock);
            activeCallbacks.add (mpc.release());

            return mi;
        }
    }

    return nullptr;
}

MidiInput::MidiInput (const String& nm)  : name (nm)
{
}

MidiInput::~MidiInput()
{
    delete static_cast<CoreMidiHelpers::MidiPortAndCallback*> (internal);
}

void MidiInput::start()
{
    const ScopedLock sl (CoreMidiHelpers::callbackLock);
    static_cast<CoreMidiHelpers::MidiPortAndCallback*> (internal)->active = true;
}

void MidiInput::stop()
{
    const ScopedLock sl (CoreMidiHelpers::callbackLock);
    static_cast<CoreMidiHelpers::MidiPortAndCallback*> (internal)->active = false;
}

#undef CHECK_ERROR

} // namespace juce

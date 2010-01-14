/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

// (This file gets included by juce_mac_NativeCode.mm, rather than being
// compiled on its own).
#if JUCE_INCLUDED_FILE

#if JUCE_MAC

//==============================================================================
#undef log
#define log(a) Logger::writeToLog(a)

static bool logAnyErrorsMidi (const OSStatus err, const int lineNum)
{
    if (err == noErr)
        return true;

    log (T("CoreMidi error: ") + String (lineNum) + T(" - ") + String::toHexString ((int)err));
    jassertfalse
    return false;
}

#undef OK
#define OK(a) logAnyErrorsMidi(a, __LINE__)


//==============================================================================
static const String getEndpointName (MIDIEndpointRef endpoint, bool isExternal)
{
    String result;
    CFStringRef str = 0;

    MIDIObjectGetStringProperty (endpoint, kMIDIPropertyName, &str);

    if (str != 0)
    {
        result = PlatformUtilities::cfStringToJuceString (str);
        CFRelease (str);
        str = 0;
    }

    MIDIEntityRef entity = 0;
    MIDIEndpointGetEntity (endpoint, &entity);

    if (entity == 0)
        return result; // probably virtual

    if (result.isEmpty())
    {
        // endpoint name has zero length - try the entity
        MIDIObjectGetStringProperty (entity, kMIDIPropertyName, &str);

        if (str != 0)
        {
            result += PlatformUtilities::cfStringToJuceString (str);
            CFRelease (str);
            str = 0;
        }
    }

    // now consider the device's name
    MIDIDeviceRef device = 0;
    MIDIEntityGetDevice (entity, &device);
    if (device == 0)
        return result;

    MIDIObjectGetStringProperty (device, kMIDIPropertyName, &str);

    if (str != 0)
    {
        const String s (PlatformUtilities::cfStringToJuceString (str));
        CFRelease (str);

        // if an external device has only one entity, throw away
        // the endpoint name and just use the device name
        if (isExternal && MIDIDeviceGetNumberOfEntities (device) < 2)
        {
            result = s;
        }
        else if (! result.startsWithIgnoreCase (s))
        {
            // prepend the device name to the entity name
            result = (s + T(" ") + result).trimEnd();
        }
    }

    return result;
}

static const String getConnectedEndpointName (MIDIEndpointRef endpoint)
{
    String result;

    // Does the endpoint have connections?
    CFDataRef connections = 0;
    int numConnections = 0;

    MIDIObjectGetDataProperty (endpoint, kMIDIPropertyConnectionUniqueID, &connections);

    if (connections != 0)
    {
        numConnections = (int) (CFDataGetLength (connections) / sizeof (MIDIUniqueID));

        if (numConnections > 0)
        {
            const SInt32* pid = reinterpret_cast <const SInt32*> (CFDataGetBytePtr (connections));

            for (int i = 0; i < numConnections; ++i, ++pid)
            {
                MIDIUniqueID uid = EndianS32_BtoN (*pid);
                MIDIObjectRef connObject;
                MIDIObjectType connObjectType;
                OSStatus err = MIDIObjectFindByUniqueID (uid, &connObject, &connObjectType);

                if (err == noErr)
                {
                    String s;

                    if (connObjectType == kMIDIObjectType_ExternalSource
                         || connObjectType == kMIDIObjectType_ExternalDestination)
                    {
                        // Connected to an external device's endpoint (10.3 and later).
                        s = getEndpointName (static_cast <MIDIEndpointRef> (connObject), true);
                    }
                    else
                    {
                        // Connected to an external device (10.2) (or something else, catch-all)
                        CFStringRef str = 0;
                        MIDIObjectGetStringProperty (connObject, kMIDIPropertyName, &str);

                        if (str != 0)
                        {
                            s = PlatformUtilities::cfStringToJuceString (str);
                            CFRelease (str);
                        }
                    }

                    if (s.isNotEmpty())
                    {
                        if (result.isNotEmpty())
                            result += (", ");

                        result += s;
                    }
                }
            }
        }

        CFRelease (connections);
    }

    if (result.isNotEmpty())
        return result;

    // Here, either the endpoint had no connections, or we failed to obtain names for any of them.
    return getEndpointName (endpoint, false);
}

//==============================================================================
const StringArray MidiOutput::getDevices()
{
    StringArray s;

    const ItemCount num = MIDIGetNumberOfDestinations();
    for (ItemCount i = 0; i < num; ++i)
    {
        MIDIEndpointRef dest = MIDIGetDestination (i);

        if (dest != 0)
        {
            String name (getConnectedEndpointName (dest));

            if (name.isEmpty())
                name = "<error>";

            s.add (name);
        }
        else
        {
            s.add ("<error>");
        }
    }

    return s;
}

int MidiOutput::getDefaultDeviceIndex()
{
    return 0;
}

static MIDIClientRef globalMidiClient;
static bool hasGlobalClientBeenCreated = false;

static bool makeSureClientExists()
{
    if (! hasGlobalClientBeenCreated)
    {
        String name (T("JUCE"));

        if (JUCEApplication::getInstance() != 0)
            name = JUCEApplication::getInstance()->getApplicationName();

        CFStringRef appName = PlatformUtilities::juceStringToCFString (name);

        hasGlobalClientBeenCreated = OK (MIDIClientCreate (appName, 0, 0, &globalMidiClient));
        CFRelease (appName);
    }

    return hasGlobalClientBeenCreated;
}

class MidiPortAndEndpoint
{
public:
    MidiPortAndEndpoint (MIDIPortRef port_, MIDIEndpointRef endPoint_)
        : port (port_), endPoint (endPoint_)
    {
    }

    ~MidiPortAndEndpoint()
    {
        if (port != 0)
            MIDIPortDispose (port);

        if (port == 0 && endPoint != 0) // if port == 0, it means we created the endpoint, so it's safe to delete it
            MIDIEndpointDispose (endPoint);
    }

    MIDIPortRef port;
    MIDIEndpointRef endPoint;
};

MidiOutput* MidiOutput::openDevice (int index)
{
    MidiOutput* mo = 0;

    if (((unsigned int) index) < (unsigned int) MIDIGetNumberOfDestinations())
    {
        MIDIEndpointRef endPoint = MIDIGetDestination (index);

        CFStringRef pname;
        if (OK (MIDIObjectGetStringProperty (endPoint, kMIDIPropertyName, &pname)))
        {
            log (T("CoreMidi - opening out: ") + PlatformUtilities::cfStringToJuceString (pname));

            if (makeSureClientExists())
            {
                MIDIPortRef port;

                if (OK (MIDIOutputPortCreate (globalMidiClient, pname, &port)))
                {
                    mo = new MidiOutput();
                    mo->internal = (void*) new MidiPortAndEndpoint (port, endPoint);
                }
            }

            CFRelease (pname);
        }
    }

    return mo;
}

MidiOutput* MidiOutput::createNewDevice (const String& deviceName)
{
    MidiOutput* mo = 0;

    if (makeSureClientExists())
    {
        MIDIEndpointRef endPoint;
        CFStringRef name = PlatformUtilities::juceStringToCFString (deviceName);

        if (OK (MIDISourceCreate (globalMidiClient, name, &endPoint)))
        {
            mo = new MidiOutput();
            mo->internal = (void*) new MidiPortAndEndpoint (0, endPoint);
        }

        CFRelease (name);
    }

    return mo;
}

MidiOutput::~MidiOutput()
{
    delete (MidiPortAndEndpoint*) internal;
}

void MidiOutput::reset()
{
}

bool MidiOutput::getVolume (float& leftVol, float& rightVol)
{
    return false;
}

void MidiOutput::setVolume (float leftVol, float rightVol)
{
}

void MidiOutput::sendMessageNow (const MidiMessage& message)
{
    MidiPortAndEndpoint* const mpe = (MidiPortAndEndpoint*) internal;

    if (message.isSysEx())
    {
        const int maxPacketSize = 256;
        int pos = 0, bytesLeft = message.getRawDataSize();
        const int numPackets = (bytesLeft + maxPacketSize - 1) / maxPacketSize;
        HeapBlock <MIDIPacketList> packets;
        packets.malloc (32 * numPackets + message.getRawDataSize(), 1);
        packets->numPackets = numPackets;

        MIDIPacket* p = packets->packet;

        for (int i = 0; i < numPackets; ++i)
        {
            p->timeStamp = 0;
            p->length = jmin (maxPacketSize, bytesLeft);
            memcpy (p->data, message.getRawData() + pos, p->length);
            pos += p->length;
            bytesLeft -= p->length;
            p = MIDIPacketNext (p);
        }

        if (mpe->port != 0)
            MIDISend (mpe->port, mpe->endPoint, packets);
        else
            MIDIReceived (mpe->endPoint, packets);
    }
    else
    {
        MIDIPacketList packets;
        packets.numPackets = 1;
        packets.packet[0].timeStamp = 0;
        packets.packet[0].length = message.getRawDataSize();
        *(int*) (packets.packet[0].data) = *(const int*) message.getRawData();

        if (mpe->port != 0)
            MIDISend (mpe->port, mpe->endPoint, &packets);
        else
            MIDIReceived (mpe->endPoint, &packets);
    }
}

//==============================================================================
const StringArray MidiInput::getDevices()
{
    StringArray s;

    const ItemCount num = MIDIGetNumberOfSources();
    for (ItemCount i = 0; i < num; ++i)
    {
        MIDIEndpointRef source = MIDIGetSource (i);

        if (source != 0)
        {
            String name (getConnectedEndpointName (source));

            if (name.isEmpty())
                name = "<error>";

            s.add (name);
        }
        else
        {
            s.add ("<error>");
        }
    }

    return s;
}

int MidiInput::getDefaultDeviceIndex()
{
    return 0;
}

//==============================================================================
struct MidiPortAndCallback
{
    MidiInput* input;
    MidiPortAndEndpoint* portAndEndpoint;
    MidiInputCallback* callback;
    MemoryBlock pendingData;
    int pendingBytes;
    double pendingDataTime;
    bool active;
};

static CriticalSection callbackLock;
static VoidArray activeCallbacks;

static void processSysex (MidiPortAndCallback* const mpc, const uint8*& d, int& size, const double time)
{
    if (*d == 0xf0)
    {
        mpc->pendingBytes = 0;
        mpc->pendingDataTime = time;
    }

    mpc->pendingData.ensureSize (mpc->pendingBytes + size, false);
    uint8* totalMessage = (uint8*) mpc->pendingData.getData();

    uint8* dest = totalMessage + mpc->pendingBytes;

    while (size > 0)
    {
        if (mpc->pendingBytes > 0 && *d >= 0x80)
        {
            if (*d >= 0xfa || *d == 0xf8)
            {
                mpc->callback->handleIncomingMidiMessage (mpc->input, MidiMessage (*d, time));
                ++d;
                --size;
            }
            else
            {
                if (*d == 0xf7)
                {
                    *dest++ = *d++;
                    mpc->pendingBytes++;
                    --size;
                }

                break;
            }
        }
        else
        {
            *dest++ = *d++;
            mpc->pendingBytes++;
            --size;
        }
    }

    if (totalMessage [mpc->pendingBytes - 1] == 0xf7)
    {
        mpc->callback->handleIncomingMidiMessage (mpc->input, MidiMessage (totalMessage,
                                                                           mpc->pendingBytes,
                                                                           mpc->pendingDataTime));
        mpc->pendingBytes = 0;
    }
    else
    {
        mpc->callback->handlePartialSysexMessage (mpc->input,
                                                  totalMessage,
                                                  mpc->pendingBytes,
                                                  mpc->pendingDataTime);
    }
}

static void midiInputProc (const MIDIPacketList* pktlist,
                           void* readProcRefCon,
                           void* srcConnRefCon)
{
    double time = Time::getMillisecondCounterHiRes() * 0.001;
    const double originalTime = time;

    MidiPortAndCallback* const mpc = (MidiPortAndCallback*) readProcRefCon;
    const ScopedLock sl (callbackLock);

    if (activeCallbacks.contains (mpc) && mpc->active)
    {
        const MIDIPacket* packet = &pktlist->packet[0];

        for (unsigned int i = 0; i < pktlist->numPackets; ++i)
        {
            const uint8* d = (const uint8*) (packet->data);
            int size = packet->length;

            while (size > 0)
            {
                time = originalTime;

                if (mpc->pendingBytes > 0 || d[0] == 0xf0)
                {
                    processSysex (mpc, d, size, time);
                }
                else
                {
                    int used = 0;
                    const MidiMessage m (d, size, used, 0, time);

                    if (used <= 0)
                    {
                        jassertfalse // malformed midi message
                        break;
                    }
                    else
                    {
                        mpc->callback->handleIncomingMidiMessage (mpc->input, m);
                    }

                    size -= used;
                    d += used;
                }
            }

            packet = MIDIPacketNext (packet);
        }
    }
}

MidiInput* MidiInput::openDevice (int index, MidiInputCallback* callback)
{
    MidiInput* mi = 0;

    if (((unsigned int) index) < (unsigned int) MIDIGetNumberOfSources())
    {
        MIDIEndpointRef endPoint = MIDIGetSource (index);

        if (endPoint != 0)
        {
            CFStringRef pname;

            if (OK (MIDIObjectGetStringProperty (endPoint, kMIDIPropertyName, &pname)))
            {
                log (T("CoreMidi - opening inp: ") + PlatformUtilities::cfStringToJuceString (pname));

                if (makeSureClientExists())
                {
                    MIDIPortRef port;

                    ScopedPointer <MidiPortAndCallback> mpc (new MidiPortAndCallback());
                    mpc->active = false;

                    if (OK (MIDIInputPortCreate (globalMidiClient, pname, midiInputProc, mpc, &port)))
                    {
                        if (OK (MIDIPortConnectSource (port, endPoint, 0)))
                        {
                            mpc->portAndEndpoint = new MidiPortAndEndpoint (port, endPoint);
                            mpc->callback = callback;
                            mpc->pendingBytes = 0;
                            mpc->pendingData.ensureSize (128);

                            mi = new MidiInput (getDevices() [index]);
                            mpc->input = mi;
                            mi->internal = (void*) mpc;

                            const ScopedLock sl (callbackLock);
                            activeCallbacks.add (mpc.release());
                        }
                        else
                        {
                            OK (MIDIPortDispose (port));
                        }
                    }
                }
            }

            CFRelease (pname);
        }
    }

    return mi;
}

MidiInput* MidiInput::createNewDevice (const String& deviceName, MidiInputCallback* callback)
{
    MidiInput* mi = 0;

    if (makeSureClientExists())
    {
        ScopedPointer <MidiPortAndCallback> mpc (new MidiPortAndCallback());
        mpc->active = false;

        MIDIEndpointRef endPoint;
        CFStringRef name = PlatformUtilities::juceStringToCFString(deviceName);
        if (OK (MIDIDestinationCreate (globalMidiClient, name, midiInputProc, mpc, &endPoint)))
        {
            mpc->portAndEndpoint = new MidiPortAndEndpoint (0, endPoint);
            mpc->callback = callback;
            mpc->pendingBytes = 0;
            mpc->pendingData.ensureSize (128);

            mi = new MidiInput (deviceName);
            mpc->input = mi;
            mi->internal = (void*) mpc;

            const ScopedLock sl (callbackLock);
            activeCallbacks.add (mpc.release());
        }

        CFRelease (name);
    }

    return mi;
}

MidiInput::MidiInput (const String& name_)
    : name (name_)
{
}

MidiInput::~MidiInput()
{
    MidiPortAndCallback* const mpc = (MidiPortAndCallback*) internal;
    mpc->active = false;

    callbackLock.enter();
    activeCallbacks.removeValue (mpc);
    callbackLock.exit();

    if (mpc->portAndEndpoint->port != 0)
        OK (MIDIPortDisconnectSource (mpc->portAndEndpoint->port, mpc->portAndEndpoint->endPoint));

    delete mpc->portAndEndpoint;
    delete mpc;
}

void MidiInput::start()
{
    MidiPortAndCallback* const mpc = (MidiPortAndCallback*) internal;
    const ScopedLock sl (callbackLock);
    mpc->active = true;
}

void MidiInput::stop()
{
    MidiPortAndCallback* const mpc = (MidiPortAndCallback*) internal;
    const ScopedLock sl (callbackLock);
    mpc->active = false;
}

#undef log

#else

MidiOutput::~MidiOutput()
{
}

void MidiOutput::reset()
{
}

bool MidiOutput::getVolume (float& leftVol, float& rightVol)
{
    return false;
}

void MidiOutput::setVolume (float leftVol, float rightVol)
{
}

void MidiOutput::sendMessageNow (const MidiMessage& message)
{
}

const StringArray MidiOutput::getDevices()
{
    return StringArray();
}

MidiOutput* MidiOutput::openDevice (int index)
{
    return 0;
}

const StringArray MidiInput::getDevices()
{
    return StringArray();
}

MidiInput* MidiInput::openDevice (int index, MidiInputCallback* callback)
{
    return 0;
}


#endif

#endif

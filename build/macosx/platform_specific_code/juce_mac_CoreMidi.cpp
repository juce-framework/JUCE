/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

  ==============================================================================
*/

#include <CoreMIDI/MIDIServices.h>
#include "../../../src/juce_core/basics/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "../../../src/juce_appframework/audio/devices/juce_MidiOutput.h"
#include "../../../src/juce_appframework/audio/devices/juce_MidiInput.h"
#include "../../../src/juce_appframework/application/juce_Application.h"
#include "../../../src/juce_core/basics/juce_Time.h"
#include "../../../src/juce_core/containers/juce_MemoryBlock.h"
#include "../../../src/juce_core/misc/juce_PlatformUtilities.h"
#include "../../../src/juce_core/threads/juce_ScopedLock.h"

//==============================================================================
#define log(a) Logger::writeToLog(a)

static bool logAnyErrors (const OSStatus err, const int lineNum)
{
    if (err == noErr)
        return true;

    log (T("CoreMidi error: ") + String (lineNum) + T(" - ") + String::toHexString ((int)err));
    jassertfalse
    return false;
}

#define OK(a) logAnyErrors(a, __LINE__)


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
        numConnections = CFDataGetLength (connections) / sizeof (MIDIUniqueID);

        if (numConnections > 0)
        {
            const SInt32* pid = reinterpret_cast <const SInt32*> (CFDataGetBytePtr (connections));

            for (int i = 0; i < numConnections; ++i, ++pid)
            {
                MIDIUniqueID id = EndianS32_BtoN (*pid);
                MIDIObjectRef connObject;
                MIDIObjectType connObjectType;
                OSStatus err = MIDIObjectFindByUniqueID (id, &connObject, &connObjectType);

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

struct MidiPortAndEndpoint
{
    MIDIPortRef port;
    MIDIEndpointRef endPoint;
};

MidiOutput* MidiOutput::openDevice (int index)
{
    MidiOutput* mo = 0;

    if (index >= 0 && index < (int) MIDIGetNumberOfDestinations())
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
                    MidiPortAndEndpoint* mpe = new MidiPortAndEndpoint();
                    mpe->port = port;
                    mpe->endPoint = endPoint;

                    mo = new MidiOutput();
                    mo->internal = (void*)mpe;
                }
            }

            CFRelease (pname);
        }
    }

    return mo;
}

MidiOutput::~MidiOutput()
{
    MidiPortAndEndpoint* const mpe = (MidiPortAndEndpoint*)internal;
    MIDIPortDispose (mpe->port);
    delete mpe;
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
    MidiPortAndEndpoint* const mpe = (MidiPortAndEndpoint*)internal;

    if (message.isSysEx())
    {
        MIDIPacketList* const packets = (MIDIPacketList*) juce_malloc (32 + message.getRawDataSize());
        packets->numPackets = 1;
        packets->packet[0].timeStamp = 0;
        packets->packet[0].length = message.getRawDataSize();
        memcpy (packets->packet[0].data, message.getRawData(), message.getRawDataSize());

        MIDISend (mpe->port, mpe->endPoint, packets);
        juce_free (packets);
    }
    else
    {
        MIDIPacketList packets;
        packets.numPackets = 1;
        packets.packet[0].timeStamp = 0;
        packets.packet[0].length = message.getRawDataSize();
        *(int*) (packets.packet[0].data) = *(const int*) message.getRawData();

        MIDISend (mpe->port, mpe->endPoint, &packets);
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
    MIDIPortRef port;
    MIDIEndpointRef endPoint;
    MidiInputCallback* callback;
    MemoryBlock pendingData;
    int pendingBytes;
    double pendingDataTime;
    bool active;
};

static CriticalSection callbackLock;
static VoidArray activeCallbacks;

static void processSysex (MidiPortAndCallback* const mpe, const uint8*& d, int& size, const double time)
{
    if (*d == 0xf0)
    {
        mpe->pendingBytes = 0;
        mpe->pendingDataTime = time;
    }

    mpe->pendingData.ensureSize (mpe->pendingBytes + size, false);
    uint8* totalMessage = (uint8*) mpe->pendingData.getData();

    uint8* dest = totalMessage + mpe->pendingBytes;

    while (size > 0)
    {
        if (mpe->pendingBytes > 0 && *d >= 0x80)
        {
            if (*d >= 0xfa || *d == 0xf8)
            {
                mpe->callback->handleIncomingMidiMessage (mpe->input, MidiMessage (*d, time));
                ++d;
                --size;
            }
            else
            {
                if (*d == 0xf7)
                {
                    *dest++ = *d++;
                    mpe->pendingBytes++;
                    --size;
                }

                break;
            }
        }
        else
        {
            *dest++ = *d++;
            mpe->pendingBytes++;
            --size;
        }
    }

    if (totalMessage [mpe->pendingBytes - 1] == 0xf7)
    {
        mpe->callback->handleIncomingMidiMessage (mpe->input, MidiMessage (totalMessage,
                                                                           mpe->pendingBytes,
                                                                           mpe->pendingDataTime));
        mpe->pendingBytes = 0;
    }
    else
    {
        mpe->callback->handlePartialSysexMessage (mpe->input,
                                                  totalMessage,
                                                  mpe->pendingBytes,
                                                  mpe->pendingDataTime);
    }
}

static void midiInputProc (const MIDIPacketList* pktlist,
                           void* readProcRefCon,
                           void* srcConnRefCon)
{
    double time = Time::getMillisecondCounterHiRes() * 0.001;
    const double originalTime = time;

    MidiPortAndCallback* const mpe = (MidiPortAndCallback*) readProcRefCon;
    const ScopedLock sl (callbackLock);

    if (activeCallbacks.contains (mpe) && mpe->active)
    {
        const MIDIPacket* packet = &pktlist->packet[0];

        for (unsigned int i = 0; i < pktlist->numPackets; ++i)
        {
            const uint8* d = (const uint8*) (packet->data);
            int size = packet->length;

            while (size > 0)
            {
                time = originalTime;

                if (mpe->pendingBytes > 0 || d[0] == 0xf0)
                {
                    processSysex (mpe, d, size, time);
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
                        mpe->callback->handleIncomingMidiMessage (mpe->input, m);
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

    if (index >= 0 && index < (int) MIDIGetNumberOfSources())
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

                    MidiPortAndCallback* const mpe = new MidiPortAndCallback();
                    mpe->active = false;

                    if (OK (MIDIInputPortCreate (globalMidiClient, pname, midiInputProc, mpe, &port)))
                    {
                        if (OK (MIDIPortConnectSource (port, endPoint, 0)))
                        {
                            mpe->port = port;
                            mpe->endPoint = endPoint;
                            mpe->callback = callback;
                            mpe->pendingBytes = 0;
                            mpe->pendingData.ensureSize (128);

                            mi = new MidiInput (getDevices() [index]);
                            mpe->input = mi;
                            mi->internal = (void*) mpe;

                            const ScopedLock sl (callbackLock);
                            activeCallbacks.add (mpe);
                        }
                        else
                        {
                            OK (MIDIPortDispose (port));
                            delete mpe;
                        }
                    }
                    else
                    {
                        delete mpe;
                    }
                }
            }

            CFRelease (pname);
        }
    }

    return mi;
}

MidiInput::MidiInput (const String& name_)
    : name (name_)
{
}

MidiInput::~MidiInput()
{
    MidiPortAndCallback* const mpe = (MidiPortAndCallback*) internal;
    mpe->active = false;

    callbackLock.enter();
    activeCallbacks.removeValue (mpe);
    callbackLock.exit();

    OK (MIDIPortDisconnectSource (mpe->port, mpe->endPoint));
    OK (MIDIPortDispose (mpe->port));
    delete mpe;
}

void MidiInput::start()
{
    MidiPortAndCallback* const mpe = (MidiPortAndCallback*) internal;
    const ScopedLock sl (callbackLock);
    mpe->active = true;
}

void MidiInput::stop()
{
    MidiPortAndCallback* const mpe = (MidiPortAndCallback*) internal;
    const ScopedLock sl (callbackLock);
    mpe->active = false;
}

END_JUCE_NAMESPACE

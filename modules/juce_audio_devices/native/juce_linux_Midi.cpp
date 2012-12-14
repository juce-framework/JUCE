/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

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

#if JUCE_ALSA

// You can define these strings in your app if you want to override the default names:
#ifndef JUCE_ALSA_MIDI_INPUT_NAME
 #define JUCE_ALSA_MIDI_INPUT_NAME  "Juce Midi Input"
#endif

#ifndef JUCE_ALSA_MIDI_OUTPUT_NAME
 #define JUCE_ALSA_MIDI_OUTPUT_NAME "Juce Midi Output"
#endif

#ifndef JUCE_ALSA_MIDI_INPUT_PORT_NAME
 #define JUCE_ALSA_MIDI_INPUT_PORT_NAME  "Juce Midi In Port"
#endif

#ifndef JUCE_ALSA_MIDI_OUTPUT_PORT_NAME
 #define JUCE_ALSA_MIDI_OUTPUT_PORT_NAME "Juce Midi Out Port"
#endif

//==============================================================================
namespace
{
    snd_seq_t* iterateMidiClient (snd_seq_t* seqHandle,
                                  snd_seq_client_info_t* clientInfo,
                                  const bool forInput,
                                  StringArray& deviceNamesFound,
                                  const int deviceIndexToOpen)
    {
        snd_seq_t* returnedHandle = nullptr;

        snd_seq_port_info_t* portInfo;
        if (snd_seq_port_info_malloc (&portInfo) == 0)
        {
            int numPorts = snd_seq_client_info_get_num_ports (clientInfo);
            const int client = snd_seq_client_info_get_client (clientInfo);

            snd_seq_port_info_set_client (portInfo, client);
            snd_seq_port_info_set_port (portInfo, -1);

            while (--numPorts >= 0)
            {
                if (snd_seq_query_next_port (seqHandle, portInfo) == 0
                     && (snd_seq_port_info_get_capability (portInfo)
                           & (forInput ? SND_SEQ_PORT_CAP_READ
                                       : SND_SEQ_PORT_CAP_WRITE)) != 0)
                {
                    deviceNamesFound.add (snd_seq_client_info_get_name (clientInfo));

                    if (deviceNamesFound.size() == deviceIndexToOpen + 1)
                    {
                        const int sourcePort = snd_seq_port_info_get_port (portInfo);
                        const int sourceClient = snd_seq_client_info_get_client (clientInfo);

                        if (sourcePort != -1)
                        {
                            if (forInput)
                            {
                                snd_seq_set_client_name (seqHandle, JUCE_ALSA_MIDI_INPUT_NAME);

                                const int portId = snd_seq_create_simple_port (seqHandle, JUCE_ALSA_MIDI_INPUT_PORT_NAME,
                                                                               SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE,
                                                                               SND_SEQ_PORT_TYPE_MIDI_GENERIC);

                                snd_seq_connect_from (seqHandle, portId, sourceClient, sourcePort);
                            }
                            else
                            {
                                snd_seq_set_client_name (seqHandle, JUCE_ALSA_MIDI_OUTPUT_NAME);

                                const int portId = snd_seq_create_simple_port (seqHandle, JUCE_ALSA_MIDI_OUTPUT_PORT_NAME,
                                                                               SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_SUBS_READ,
                                                                               SND_SEQ_PORT_TYPE_MIDI_GENERIC);

                                snd_seq_connect_to (seqHandle, portId, sourceClient, sourcePort);
                            }

                            returnedHandle = seqHandle;
                        }
                    }
                }
            }

            snd_seq_port_info_free (portInfo);
        }

        return returnedHandle;
    }

    snd_seq_t* iterateMidiDevices (const bool forInput,
                                   StringArray& deviceNamesFound,
                                   const int deviceIndexToOpen)
    {
        snd_seq_t* returnedHandle = nullptr;
        snd_seq_t* seqHandle = nullptr;

        if (snd_seq_open (&seqHandle, "default", forInput ? SND_SEQ_OPEN_INPUT
                                                          : SND_SEQ_OPEN_OUTPUT, 0) == 0)
        {
            snd_seq_system_info_t* systemInfo = nullptr;
            snd_seq_client_info_t* clientInfo = nullptr;

            if (snd_seq_system_info_malloc (&systemInfo) == 0)
            {
                if (snd_seq_system_info (seqHandle, systemInfo) == 0
                     && snd_seq_client_info_malloc (&clientInfo) == 0)
                {
                    int numClients = snd_seq_system_info_get_cur_clients (systemInfo);

                    while (--numClients >= 0 && returnedHandle == 0)
                        if (snd_seq_query_next_client (seqHandle, clientInfo) == 0)
                            returnedHandle = iterateMidiClient (seqHandle, clientInfo,
                                                                forInput, deviceNamesFound,
                                                                deviceIndexToOpen);

                    snd_seq_client_info_free (clientInfo);
                }

                snd_seq_system_info_free (systemInfo);
            }

            if (returnedHandle == 0)
                snd_seq_close (seqHandle);
        }

        deviceNamesFound.appendNumbersToDuplicates (true, true);

        return returnedHandle;
    }

    snd_seq_t* createMidiDevice (const bool forInput, const String& deviceNameToOpen)
    {
        snd_seq_t* seqHandle = nullptr;

        if (snd_seq_open (&seqHandle, "default", forInput ? SND_SEQ_OPEN_INPUT
                                                          : SND_SEQ_OPEN_OUTPUT, 0) == 0)
        {
            snd_seq_set_client_name (seqHandle,
                                     (deviceNameToOpen + (forInput ? " Input" : " Output")).toUTF8());

            const int portId
                = snd_seq_create_simple_port (seqHandle,
                                              forInput ? "in"
                                                       : "out",
                                              forInput ? (SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE)
                                                       : (SND_SEQ_PORT_CAP_READ  | SND_SEQ_PORT_CAP_SUBS_READ),
                                              forInput ? SND_SEQ_PORT_TYPE_APPLICATION
                                                       : SND_SEQ_PORT_TYPE_MIDI_GENERIC);

            if (portId < 0)
            {
                snd_seq_close (seqHandle);
                seqHandle = nullptr;
            }
        }

        return seqHandle;
    }
}


//==============================================================================
class MidiOutputDevice
{
public:
    MidiOutputDevice (MidiOutput* const midiOutput_,
                      snd_seq_t* const seqHandle_)
        :
          midiOutput (midiOutput_),
          seqHandle (seqHandle_),
          maxEventSize (16 * 1024)
    {
        jassert (seqHandle != 0 && midiOutput != 0);
        snd_midi_event_new (maxEventSize, &midiParser);
    }

    ~MidiOutputDevice()
    {
        snd_midi_event_free (midiParser);
        snd_seq_close (seqHandle);
    }

    void sendMessageNow (const MidiMessage& message)
    {
        if (message.getRawDataSize() > maxEventSize)
        {
            maxEventSize = message.getRawDataSize();
            snd_midi_event_free (midiParser);
            snd_midi_event_new (maxEventSize, &midiParser);
        }

        snd_seq_event_t event;
        snd_seq_ev_clear (&event);

        long numBytes = (long) message.getRawDataSize();
        const uint8* data = message.getRawData();

        while (numBytes > 0)
        {
            const long numSent = snd_midi_event_encode (midiParser, data, numBytes, &event);
            if (numSent <= 0)
                break;

            numBytes -= numSent;
            data += numSent;

            snd_seq_ev_set_source (&event, 0);
            snd_seq_ev_set_subs (&event);
            snd_seq_ev_set_direct (&event);

            snd_seq_event_output (seqHandle, &event);
        }

        snd_seq_drain_output (seqHandle);
        snd_midi_event_reset_encode (midiParser);
    }

private:
    MidiOutput* const midiOutput;
    snd_seq_t* const seqHandle;
    snd_midi_event_t* midiParser;
    int maxEventSize;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MidiOutputDevice)
};

StringArray MidiOutput::getDevices()
{
    StringArray devices;
    iterateMidiDevices (false, devices, -1);
    return devices;
}

int MidiOutput::getDefaultDeviceIndex()
{
    return 0;
}

MidiOutput* MidiOutput::openDevice (int deviceIndex)
{
    MidiOutput* newDevice = nullptr;

    StringArray devices;
    snd_seq_t* const handle = iterateMidiDevices (false, devices, deviceIndex);

    if (handle != 0)
    {
        newDevice = new MidiOutput();
        newDevice->internal = new MidiOutputDevice (newDevice, handle);
    }

    return newDevice;
}

MidiOutput* MidiOutput::createNewDevice (const String& deviceName)
{
    MidiOutput* newDevice = nullptr;

    snd_seq_t* const handle = createMidiDevice (false, deviceName);

    if (handle != 0)
    {
        newDevice = new MidiOutput();
        newDevice->internal = new MidiOutputDevice (newDevice, handle);
    }

    return newDevice;
}

MidiOutput::~MidiOutput()
{
    delete static_cast <MidiOutputDevice*> (internal);
}

void MidiOutput::sendMessageNow (const MidiMessage& message)
{
    static_cast <MidiOutputDevice*> (internal)->sendMessageNow (message);
}


//==============================================================================
class MidiInputThread   : public Thread
{
public:
    MidiInputThread (MidiInput* const midiInput_,
                     snd_seq_t* const seqHandle_,
                     MidiInputCallback* const callback_)
        : Thread ("Juce MIDI Input"),
          midiInput (midiInput_),
          seqHandle (seqHandle_),
          callback (callback_)
    {
        jassert (seqHandle != 0 && callback != 0 && midiInput != 0);
    }

    ~MidiInputThread()
    {
        snd_seq_close (seqHandle);
    }

    void run()
    {
        const int maxEventSize = 16 * 1024;
        snd_midi_event_t* midiParser;

        if (snd_midi_event_new (maxEventSize, &midiParser) >= 0)
        {
            HeapBlock <uint8> buffer (maxEventSize);

            const int numPfds = snd_seq_poll_descriptors_count (seqHandle, POLLIN);
            struct pollfd* const pfd = (struct pollfd*) alloca (numPfds * sizeof (struct pollfd));

            snd_seq_poll_descriptors (seqHandle, pfd, numPfds, POLLIN);

            while (! threadShouldExit())
            {
                if (poll (pfd, numPfds, 500) > 0)
                {
                    snd_seq_event_t* inputEvent = nullptr;

                    snd_seq_nonblock (seqHandle, 1);

                    do
                    {
                        if (snd_seq_event_input (seqHandle, &inputEvent) >= 0)
                        {
                            // xxx what about SYSEXes that are too big for the buffer?
                            const int numBytes = snd_midi_event_decode (midiParser, buffer, maxEventSize, inputEvent);

                            snd_midi_event_reset_decode (midiParser);

                            if (numBytes > 0)
                            {
                                const MidiMessage message ((const uint8*) buffer,
                                                           numBytes,
                                                           Time::getMillisecondCounter() * 0.001);


                                callback->handleIncomingMidiMessage (midiInput, message);
                            }

                            snd_seq_free_event (inputEvent);
                        }
                    }
                    while (snd_seq_event_input_pending (seqHandle, 0) > 0);

                    snd_seq_free_event (inputEvent);
                }
            }

            snd_midi_event_free (midiParser);
        }
    };

private:
    MidiInput* const midiInput;
    snd_seq_t* const seqHandle;
    MidiInputCallback* const callback;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MidiInputThread)
};

//==============================================================================
MidiInput::MidiInput (const String& name_)
    : name (name_),
      internal (0)
{
}

MidiInput::~MidiInput()
{
    stop();
    delete static_cast <MidiInputThread*> (internal);
}

void MidiInput::start()
{
    static_cast <MidiInputThread*> (internal)->startThread();
}

void MidiInput::stop()
{
    static_cast <MidiInputThread*> (internal)->stopThread (3000);
}

int MidiInput::getDefaultDeviceIndex()
{
    return 0;
}

StringArray MidiInput::getDevices()
{
    StringArray devices;
    iterateMidiDevices (true, devices, -1);
    return devices;
}

MidiInput* MidiInput::openDevice (int deviceIndex, MidiInputCallback* callback)
{
    MidiInput* newDevice = nullptr;

    StringArray devices;
    snd_seq_t* const handle = iterateMidiDevices (true, devices, deviceIndex);

    if (handle != 0)
    {
        newDevice = new MidiInput (devices [deviceIndex]);
        newDevice->internal = new MidiInputThread (newDevice, handle, callback);
    }

    return newDevice;
}

MidiInput* MidiInput::createNewDevice (const String& deviceName, MidiInputCallback* callback)
{
    MidiInput* newDevice = nullptr;

    snd_seq_t* const handle = createMidiDevice (true, deviceName);

    if (handle != 0)
    {
        newDevice = new MidiInput (deviceName);
        newDevice->internal = new MidiInputThread (newDevice, handle, callback);
    }

    return newDevice;
}



//==============================================================================
#else

// (These are just stub functions if ALSA is unavailable...)

StringArray MidiOutput::getDevices()                                { return StringArray(); }
int MidiOutput::getDefaultDeviceIndex()                             { return 0; }
MidiOutput* MidiOutput::openDevice (int)                            { return nullptr; }
MidiOutput* MidiOutput::createNewDevice (const String&)             { return nullptr; }
MidiOutput::~MidiOutput()   {}
void MidiOutput::sendMessageNow (const MidiMessage&)    {}

MidiInput::MidiInput (const String& name_) : name (name_), internal (0)  {}
MidiInput::~MidiInput() {}
void MidiInput::start() {}
void MidiInput::stop()  {}
int MidiInput::getDefaultDeviceIndex()      { return 0; }
StringArray MidiInput::getDevices()         { return StringArray(); }
MidiInput* MidiInput::openDevice (int, MidiInputCallback*)                  { return nullptr; }
MidiInput* MidiInput::createNewDevice (const String&, MidiInputCallback*)   { return nullptr; }

#endif

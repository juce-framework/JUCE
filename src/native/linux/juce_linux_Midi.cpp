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

// (This file gets included by juce_linux_NativeCode.cpp, rather than being
// compiled on its own).
#if JUCE_INCLUDED_FILE
#if JUCE_ALSA

//==============================================================================
static snd_seq_t* iterateDevices (const bool forInput,
                                  StringArray& deviceNamesFound,
                                  const int deviceIndexToOpen)
{
    snd_seq_t* returnedHandle = 0;
    snd_seq_t* seqHandle;

    if (snd_seq_open (&seqHandle, "default", forInput ? SND_SEQ_OPEN_INPUT
                                                      : SND_SEQ_OPEN_OUTPUT, 0) == 0)
    {
        snd_seq_system_info_t* systemInfo;
        snd_seq_client_info_t* clientInfo;

        if (snd_seq_system_info_malloc (&systemInfo) == 0)
        {
            if (snd_seq_system_info (seqHandle, systemInfo) == 0
                 && snd_seq_client_info_malloc (&clientInfo) == 0)
            {
                int numClients = snd_seq_system_info_get_cur_clients (systemInfo);

                while (--numClients >= 0 && returnedHandle == 0)
                {
                    if (snd_seq_query_next_client (seqHandle, clientInfo) == 0)
                    {
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
                                            snd_seq_set_client_name (seqHandle,
                                                                     forInput ? "Juce Midi Input"
                                                                              : "Juce Midi Output");

                                            const int portId
                                                = snd_seq_create_simple_port (seqHandle,
                                                                              forInput ? "Juce Midi In Port"
                                                                                       : "Juce Midi Out Port",
                                                                              forInput ? (SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE)
                                                                                       : (SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_SUBS_READ),
                                                                              SND_SEQ_PORT_TYPE_MIDI_GENERIC);

                                            snd_seq_connect_from (seqHandle, portId, sourceClient, sourcePort);

                                            returnedHandle = seqHandle;
                                        }
                                    }
                                }
                            }

                            snd_seq_port_info_free (portInfo);
                        }
                    }
                }

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

static snd_seq_t* createDevice (const bool forInput,
                                const String& deviceNameToOpen)
{
    snd_seq_t* seqHandle = 0;

    if (snd_seq_open (&seqHandle, "default", forInput ? SND_SEQ_OPEN_INPUT
                                                      : SND_SEQ_OPEN_OUTPUT, 0) == 0)
    {
        snd_seq_set_client_name (seqHandle,
                                 (const char*) (forInput ? (deviceNameToOpen + T(" Input"))
                                                         : (deviceNameToOpen + T(" Output"))));

        const int portId
            = snd_seq_create_simple_port (seqHandle,
                                          forInput ? "in"
                                                   : "out",
                                          forInput ? (SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE)
                                                   : (SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_SUBS_READ),
                                          forInput ? SND_SEQ_PORT_TYPE_APPLICATION
                                                   : SND_SEQ_PORT_TYPE_MIDI_GENERIC);

        if (portId < 0)
        {
            snd_seq_close (seqHandle);
            seqHandle = 0;
        }
    }

    return seqHandle;
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

        snd_midi_event_encode (midiParser,
                               message.getRawData(),
                               message.getRawDataSize(),
                               &event);

        snd_midi_event_reset_encode (midiParser);

        snd_seq_ev_set_source (&event, 0);
        snd_seq_ev_set_subs (&event);
        snd_seq_ev_set_direct (&event);

        snd_seq_event_output (seqHandle, &event);
        snd_seq_drain_output (seqHandle);
    }

    juce_UseDebuggingNewOperator

private:
    MidiOutput* const midiOutput;
    snd_seq_t* const seqHandle;
    snd_midi_event_t* midiParser;
    int maxEventSize;
};

const StringArray MidiOutput::getDevices()
{
    StringArray devices;
    iterateDevices (false, devices, -1);
    return devices;
}

int MidiOutput::getDefaultDeviceIndex()
{
    return 0;
}

MidiOutput* MidiOutput::openDevice (int deviceIndex)
{
    MidiOutput* newDevice = 0;

    StringArray devices;
    snd_seq_t* const handle = iterateDevices (false, devices, deviceIndex);

    if (handle != 0)
    {
        newDevice = new MidiOutput();
        newDevice->internal = new MidiOutputDevice (newDevice, handle);
    }

    return newDevice;
}

MidiOutput* MidiOutput::createNewDevice (const String& deviceName)
{
    MidiOutput* newDevice = 0;

    snd_seq_t* const handle = createDevice (false, deviceName);

    if (handle != 0)
    {
        newDevice = new MidiOutput();
        newDevice->internal = new MidiOutputDevice (newDevice, handle);
    }

    return newDevice;
}

MidiOutput::~MidiOutput()
{
    MidiOutputDevice* const device = (MidiOutputDevice*) internal;
    delete device;
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
    ((MidiOutputDevice*) internal)->sendMessageNow (message);
}


//==============================================================================
class MidiInputThread   : public Thread
{
public:
    MidiInputThread (MidiInput* const midiInput_,
                     snd_seq_t* const seqHandle_,
                     MidiInputCallback* const callback_)
        : Thread (T("Juce MIDI Input")),
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
                    snd_seq_event_t* inputEvent = 0;

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

    juce_UseDebuggingNewOperator

private:
    MidiInput* const midiInput;
    snd_seq_t* const seqHandle;
    MidiInputCallback* const callback;
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
    MidiInputThread* const thread = (MidiInputThread*) internal;
    delete thread;
}

void MidiInput::start()
{
    ((MidiInputThread*) internal)->startThread();
}

void MidiInput::stop()
{
    ((MidiInputThread*) internal)->stopThread (3000);
}

int MidiInput::getDefaultDeviceIndex()
{
    return 0;
}

const StringArray MidiInput::getDevices()
{
    StringArray devices;
    iterateDevices (true, devices, -1);
    return devices;
}

MidiInput* MidiInput::openDevice (int deviceIndex, MidiInputCallback* callback)
{
    MidiInput* newDevice = 0;

    StringArray devices;
    snd_seq_t* const handle = iterateDevices (true, devices, deviceIndex);

    if (handle != 0)
    {
        newDevice = new MidiInput (devices [deviceIndex]);
        newDevice->internal = new MidiInputThread (newDevice, handle, callback);
    }

    return newDevice;
}

MidiInput* MidiInput::createNewDevice (const String& deviceName, MidiInputCallback* callback)
{
    MidiInput* newDevice = 0;

    snd_seq_t* const handle = createDevice (true, deviceName);

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

const StringArray MidiOutput::getDevices()                          { return StringArray(); }
int MidiOutput::getDefaultDeviceIndex()                             { return 0; }
MidiOutput* MidiOutput::openDevice (int)                            { return 0; }
MidiOutput* MidiOutput::createNewDevice (const String&)             { return 0; }
MidiOutput::~MidiOutput()   {}
void MidiOutput::reset()    {}
bool MidiOutput::getVolume (float&, float&)     { return false; }
void MidiOutput::setVolume (float, float)       {}
void MidiOutput::sendMessageNow (const MidiMessage&)    {}

MidiInput::MidiInput (const String& name_) : name (name_), internal (0)  {}
MidiInput::~MidiInput() {}
void MidiInput::start() {}
void MidiInput::stop()  {}
int MidiInput::getDefaultDeviceIndex()      { return 0; }
const StringArray MidiInput::getDevices()   { return StringArray(); }
MidiInput* MidiInput::openDevice (int, MidiInputCallback*)                  { return 0; }
MidiInput* MidiInput::createNewDevice (const String&, MidiInputCallback*)   { return 0; }

#endif
#endif

/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

class MidiInCollector
{
public:
    MidiInCollector (MidiInput* const input_,
                     MidiInputCallback& callback_)
        : deviceHandle (0),
          input (input_),
          callback (callback_),
          concatenator (4096),
          isStarted (false),
          startTime (0)
    {
    }

    ~MidiInCollector()
    {
        stop();

        if (deviceHandle != 0)
        {
            for (int count = 5; --count >= 0;)
            {
                if (midiInClose (deviceHandle) == MMSYSERR_NOERROR)
                    break;

                Sleep (20);
            }
        }
    }

    //==============================================================================
    void handleMessage (const uint8* bytes, const uint32 timeStamp)
    {
        if (bytes[0] >= 0x80 && isStarted)
        {
            concatenator.pushMidiData (bytes, MidiMessage::getMessageLengthFromFirstByte (bytes[0]),
                                       convertTimeStamp (timeStamp), input, callback);
            writeFinishedBlocks();
        }
    }

    void handleSysEx (MIDIHDR* const hdr, const uint32 timeStamp)
    {
        if (isStarted && hdr->dwBytesRecorded > 0)
        {
            concatenator.pushMidiData (hdr->lpData, (int) hdr->dwBytesRecorded,
                                       convertTimeStamp (timeStamp), input, callback);
            writeFinishedBlocks();
        }
    }

    void start()
    {
        if (deviceHandle != 0 && ! isStarted)
        {
            activeMidiCollectors.addIfNotAlreadyThere (this);

            for (int i = 0; i < (int) numHeaders; ++i)
            {
                headers[i].prepare (deviceHandle);
                headers[i].write (deviceHandle);
            }

            startTime = Time::getMillisecondCounterHiRes();
            MMRESULT res = midiInStart (deviceHandle);

            if (res == MMSYSERR_NOERROR)
            {
                concatenator.reset();
                isStarted = true;
            }
            else
            {
                unprepareAllHeaders();
            }
        }
    }

    void stop()
    {
        if (isStarted)
        {
            isStarted = false;
            midiInReset (deviceHandle);
            midiInStop (deviceHandle);
            activeMidiCollectors.removeFirstMatchingValue (this);
            unprepareAllHeaders();
            concatenator.reset();
        }
    }

    static void CALLBACK midiInCallback (HMIDIIN, UINT uMsg, DWORD_PTR dwInstance,
                                         DWORD_PTR midiMessage, DWORD_PTR timeStamp)
    {
        MidiInCollector* const collector = reinterpret_cast <MidiInCollector*> (dwInstance);

        if (activeMidiCollectors.contains (collector))
        {
            if (uMsg == MIM_DATA)
                collector->handleMessage ((const uint8*) &midiMessage, (uint32) timeStamp);
            else if (uMsg == MIM_LONGDATA)
                collector->handleSysEx ((MIDIHDR*) midiMessage, (uint32) timeStamp);
        }
    }

    HMIDIIN deviceHandle;

private:
    static Array <MidiInCollector*, CriticalSection> activeMidiCollectors;

    MidiInput* input;
    MidiInputCallback& callback;
    MidiDataConcatenator concatenator;
    bool volatile isStarted;
    double startTime;

    class MidiHeader
    {
    public:
        MidiHeader() {}

        void prepare (HMIDIIN deviceHandle)
        {
            zerostruct (hdr);
            hdr.lpData = data;
            hdr.dwBufferLength = (DWORD) numElementsInArray (data);

            midiInPrepareHeader (deviceHandle, &hdr, sizeof (hdr));
        }

        void unprepare (HMIDIIN deviceHandle)
        {
            if ((hdr.dwFlags & WHDR_DONE) != 0)
            {
                int c = 10;
                while (--c >= 0 && midiInUnprepareHeader (deviceHandle, &hdr, sizeof (hdr)) == MIDIERR_STILLPLAYING)
                    Thread::sleep (20);

                jassert (c >= 0);
            }
        }

        void write (HMIDIIN deviceHandle)
        {
            hdr.dwBytesRecorded = 0;
            midiInAddBuffer (deviceHandle, &hdr, sizeof (hdr));
        }

        void writeIfFinished (HMIDIIN deviceHandle)
        {
            if ((hdr.dwFlags & WHDR_DONE) != 0)
                write (deviceHandle);
        }

    private:
        MIDIHDR hdr;
        char data [256];

        JUCE_DECLARE_NON_COPYABLE (MidiHeader)
    };

    enum { numHeaders = 32 };
    MidiHeader headers [numHeaders];

    void writeFinishedBlocks()
    {
        for (int i = 0; i < (int) numHeaders; ++i)
            headers[i].writeIfFinished (deviceHandle);
    }

    void unprepareAllHeaders()
    {
        for (int i = 0; i < (int) numHeaders; ++i)
            headers[i].unprepare (deviceHandle);
    }

    double convertTimeStamp (uint32 timeStamp)
    {
        double t = startTime + timeStamp;

        const double now = Time::getMillisecondCounterHiRes();
        if (t > now)
        {
            if (t > now + 2.0)
                startTime -= 1.0;

            t = now;
        }

        return t * 0.001;
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MidiInCollector)
};

Array <MidiInCollector*, CriticalSection> MidiInCollector::activeMidiCollectors;


//==============================================================================
StringArray MidiInput::getDevices()
{
    StringArray s;
    const UINT num = midiInGetNumDevs();

    for (UINT i = 0; i < num; ++i)
    {
        MIDIINCAPS mc = { 0 };

        if (midiInGetDevCaps (i, &mc, sizeof (mc)) == MMSYSERR_NOERROR)
            s.add (String (mc.szPname, sizeof (mc.szPname)));
    }

    return s;
}

int MidiInput::getDefaultDeviceIndex()
{
    return 0;
}

MidiInput* MidiInput::openDevice (const int index, MidiInputCallback* const callback)
{
    if (callback == nullptr)
        return nullptr;

    UINT deviceId = MIDI_MAPPER;
    int n = 0;
    String name;

    const UINT num = midiInGetNumDevs();

    for (UINT i = 0; i < num; ++i)
    {
        MIDIINCAPS mc = { 0 };

        if (midiInGetDevCaps (i, &mc, sizeof (mc)) == MMSYSERR_NOERROR)
        {
            if (index == n)
            {
                deviceId = i;
                name = String (mc.szPname, (size_t) numElementsInArray (mc.szPname));
                break;
            }

            ++n;
        }
    }

    ScopedPointer <MidiInput> in (new MidiInput (name));
    ScopedPointer <MidiInCollector> collector (new MidiInCollector (in, *callback));

    HMIDIIN h;
    MMRESULT err = midiInOpen (&h, deviceId,
                               (DWORD_PTR) &MidiInCollector::midiInCallback,
                               (DWORD_PTR) (MidiInCollector*) collector,
                               CALLBACK_FUNCTION);

    if (err == MMSYSERR_NOERROR)
    {
        collector->deviceHandle = h;
        in->internal = collector.release();
        return in.release();
    }

    return nullptr;
}

MidiInput::MidiInput (const String& name_)
    : name (name_),
      internal (0)
{
}

MidiInput::~MidiInput()
{
    delete static_cast<MidiInCollector*> (internal);
}

void MidiInput::start()     { static_cast<MidiInCollector*> (internal)->start(); }
void MidiInput::stop()      { static_cast<MidiInCollector*> (internal)->stop(); }


//==============================================================================
struct MidiOutHandle
{
    int refCount;
    UINT deviceId;
    HMIDIOUT handle;

    static Array<MidiOutHandle*> activeHandles;

private:
    JUCE_LEAK_DETECTOR (MidiOutHandle)
};

Array<MidiOutHandle*> MidiOutHandle::activeHandles;

//==============================================================================
StringArray MidiOutput::getDevices()
{
    StringArray s;
    const UINT num = midiOutGetNumDevs();

    for (UINT i = 0; i < num; ++i)
    {
        MIDIOUTCAPS mc = { 0 };

        if (midiOutGetDevCaps (i, &mc, sizeof (mc)) == MMSYSERR_NOERROR)
            s.add (String (mc.szPname, sizeof (mc.szPname)));
    }

    return s;
}

int MidiOutput::getDefaultDeviceIndex()
{
    const UINT num = midiOutGetNumDevs();
    int n = 0;

    for (UINT i = 0; i < num; ++i)
    {
        MIDIOUTCAPS mc = { 0 };

        if (midiOutGetDevCaps (i, &mc, sizeof (mc)) == MMSYSERR_NOERROR)
        {
            if ((mc.wTechnology & MOD_MAPPER) != 0)
                return n;

             ++n;
        }
    }

    return 0;
}

MidiOutput* MidiOutput::openDevice (int index)
{
    UINT deviceId = MIDI_MAPPER;
    const UINT num = midiOutGetNumDevs();
    int n = 0;

    for (UINT i = 0; i < num; ++i)
    {
        MIDIOUTCAPS mc = { 0 };

        if (midiOutGetDevCaps (i, &mc, sizeof (mc)) == MMSYSERR_NOERROR)
        {
            // use the microsoft sw synth as a default - best not to allow deviceId
            // to be MIDI_MAPPER, or else device sharing breaks
            if (String (mc.szPname, sizeof (mc.szPname)).containsIgnoreCase ("microsoft"))
                deviceId = i;

            if (index == n)
            {
                deviceId = i;
                break;
            }

            ++n;
        }
    }

    for (int i = MidiOutHandle::activeHandles.size(); --i >= 0;)
    {
        MidiOutHandle* const han = MidiOutHandle::activeHandles.getUnchecked(i);

        if (han->deviceId == deviceId)
        {
            han->refCount++;

            MidiOutput* const out = new MidiOutput();
            out->internal = han;
            return out;
        }
    }

    for (int i = 4; --i >= 0;)
    {
        HMIDIOUT h = 0;
        MMRESULT res = midiOutOpen (&h, deviceId, 0, 0, CALLBACK_NULL);

        if (res == MMSYSERR_NOERROR)
        {
            MidiOutHandle* const han = new MidiOutHandle();
            han->deviceId = deviceId;
            han->refCount = 1;
            han->handle = h;
            MidiOutHandle::activeHandles.add (han);

            MidiOutput* const out = new MidiOutput();
            out->internal = han;
            return out;
        }
        else if (res == MMSYSERR_ALLOCATED)
        {
            Sleep (100);
        }
        else
        {
            break;
        }
    }

    return nullptr;
}

MidiOutput::~MidiOutput()
{
    stopBackgroundThread();

    MidiOutHandle* const h = static_cast<MidiOutHandle*> (internal);

    if (MidiOutHandle::activeHandles.contains (h) && --(h->refCount) == 0)
    {
        midiOutClose (h->handle);
        MidiOutHandle::activeHandles.removeFirstMatchingValue (h);
        delete h;
    }
}

void MidiOutput::sendMessageNow (const MidiMessage& message)
{
    const MidiOutHandle* const handle = static_cast<const MidiOutHandle*> (internal);

    if (message.getRawDataSize() > 3 || message.isSysEx())
    {
        MIDIHDR h = { 0 };

        h.lpData = (char*) message.getRawData();
        h.dwBytesRecorded = h.dwBufferLength  = (DWORD) message.getRawDataSize();

        if (midiOutPrepareHeader (handle->handle, &h, sizeof (MIDIHDR)) == MMSYSERR_NOERROR)
        {
            MMRESULT res = midiOutLongMsg (handle->handle, &h, sizeof (MIDIHDR));

            if (res == MMSYSERR_NOERROR)
            {
                while ((h.dwFlags & MHDR_DONE) == 0)
                    Sleep (1);

                int count = 500; // 1 sec timeout

                while (--count >= 0)
                {
                    res = midiOutUnprepareHeader (handle->handle, &h, sizeof (MIDIHDR));

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
        for (int i = 0; i < 50; ++i)
        {
            if (midiOutShortMsg (handle->handle, *(unsigned int*) message.getRawData()) != MIDIERR_NOTREADY)
                break;

            Sleep (1);
        }
    }
}

/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-10 by Raw Material Software Ltd.

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

// (This file gets included by juce_win32_NativeCode.cpp, rather than being
// compiled on its own).
#if JUCE_INCLUDED_FILE

//==============================================================================
class MidiInThread  : public Thread
{
public:
    //==============================================================================
    MidiInThread (MidiInput* const input_,
                  MidiInputCallback* const callback_)
        : Thread ("Juce Midi"),
          deviceHandle (0),
          input (input_),
          callback (callback_),
          isStarted (false),
          startTime (0)
    {
        pending.ensureSize ((int) defaultBufferSize);

        for (int i = (int) numInHeaders; --i >= 0;)
        {
            zeromem (&hdr[i], sizeof (MIDIHDR));
            hdr[i].lpData = inData[i];
            hdr[i].dwBufferLength = (int) inBufferSize;
        }
    };

    ~MidiInThread()
    {
        stop();

        if (deviceHandle != 0)
        {
            int count = 5;
            while (--count >= 0)
            {
                if (midiInClose (deviceHandle) == MMSYSERR_NOERROR)
                    break;

                Sleep (20);
            }
        }
    }

    //==============================================================================
    void handle (const uint32 message, const uint32 timeStamp)
    {
        const int byte = message & 0xff;
        if (byte < 0x80)
            return;

        const int time = timeStampToMs (timeStamp);

        {
            const ScopedLock sl (lock);
            pending.addEvent (&message, 3, time);
        }

        notify();
    }

    void handleSysEx (MIDIHDR* const hdr, const uint32 timeStamp)
    {
        const int time = timeStampToMs (timeStamp);
        const int num = hdr->dwBytesRecorded;

        if (num > 0)
        {
            {
                const ScopedLock sl (lock);
                pending.addEvent (hdr->lpData, num, time);
            }

            notify();
        }
    }

    void writeBlock (const int i)
    {
        hdr[i].dwBytesRecorded = 0;
        MMRESULT res = midiInPrepareHeader (deviceHandle, &hdr[i], sizeof (MIDIHDR));
        jassert (res == MMSYSERR_NOERROR);
        res = midiInAddBuffer (deviceHandle, &hdr[i], sizeof (MIDIHDR));
        jassert (res == MMSYSERR_NOERROR);
    }

    void run()
    {
        MidiBuffer newEvents;
        newEvents.ensureSize ((int) defaultBufferSize);

        while (! threadShouldExit())
        {
            for (int i = 0; i < (int) numInHeaders; ++i)
            {
                if ((hdr[i].dwFlags & WHDR_DONE) != 0)
                {
                    MMRESULT res = midiInUnprepareHeader (deviceHandle, &hdr[i], sizeof (MIDIHDR));
                    (void) res;
                    jassert (res == MMSYSERR_NOERROR);
                    writeBlock (i);
                }
            }

            newEvents.clear(); // (resets it without freeing allocated storage)

            {
                const ScopedLock sl (lock);
                newEvents.swapWith (pending);
            }

            //xxx needs to figure out if blocks are broken up or not

            if (newEvents.isEmpty())
            {
                wait (500);
            }
            else
            {
                MidiMessage message (0xf4, 0.0);
                int time;

                for (MidiBuffer::Iterator i (newEvents); i.getNextEvent (message, time);)
                {
                    message.setTimeStamp (time * 0.001);
                    callback->handleIncomingMidiMessage (input, message);
                }
            }
        }
    }

    void start()
    {
        jassert (deviceHandle != 0);
        if (deviceHandle != 0 && ! isStarted)
        {
            stop();

            activeMidiThreads.addIfNotAlreadyThere (this);

            int i;
            for (i = 0; i < (int) numInHeaders; ++i)
                writeBlock (i);

            startTime = Time::getMillisecondCounter();
            MMRESULT res = midiInStart (deviceHandle);
            jassert (res == MMSYSERR_NOERROR);

            if (res == MMSYSERR_NOERROR)
            {
                isStarted = true;
                pending.clear();
                startThread (6);
            }
        }
    }

    void stop()
    {
        if (isStarted)
        {
            stopThread (5000);

            midiInReset (deviceHandle);
            midiInStop (deviceHandle);

            activeMidiThreads.removeValue (this);

            { const ScopedLock sl (lock); }

            for (int i = (int) numInHeaders; --i >= 0;)
            {
                if ((hdr[i].dwFlags & WHDR_DONE) != 0)
                {
                    int c = 10;
                    while (--c >= 0 && midiInUnprepareHeader (deviceHandle, &hdr[i], sizeof (MIDIHDR)) == MIDIERR_STILLPLAYING)
                        Sleep (20);

                    jassert (c >= 0);
                }
            }

            isStarted = false;
            pending.clear();
        }
    }

    static void CALLBACK midiInCallback (HMIDIIN, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR midiMessage, DWORD_PTR timeStamp)
    {
        MidiInThread* const thread = reinterpret_cast <MidiInThread*> (dwInstance);

        if (thread != 0 && activeMidiThreads.contains (thread))
        {
            if (uMsg == MIM_DATA)
                thread->handle ((uint32) midiMessage, (uint32) timeStamp);
            else if (uMsg == MIM_LONGDATA)
                thread->handleSysEx ((MIDIHDR*) midiMessage, (uint32) timeStamp);
        }
    }

    juce_UseDebuggingNewOperator

    HMIDIIN deviceHandle;

private:
    static Array <void*, CriticalSection> activeMidiThreads;

    MidiInput* input;
    MidiInputCallback* callback;
    bool isStarted;
    uint32 startTime;
    CriticalSection lock;

    enum { defaultBufferSize = 8192,
           numInHeaders = 32,
           inBufferSize = 256 };

    MIDIHDR hdr [(int) numInHeaders];
    char inData [(int) numInHeaders] [(int) inBufferSize];

    MidiBuffer pending;

    int timeStampToMs (uint32 timeStamp)
    {
        timeStamp += startTime;

        const uint32 now = Time::getMillisecondCounter();
        if (timeStamp > now)
        {
            if (timeStamp > now + 2)
                --startTime;

            timeStamp = now;
        }

        return (int) timeStamp;
    }

    MidiInThread (const MidiInThread&);
    MidiInThread& operator= (const MidiInThread&);
};

Array <void*, CriticalSection> MidiInThread::activeMidiThreads;


//==============================================================================
const StringArray MidiInput::getDevices()
{
    StringArray s;
    const int num = midiInGetNumDevs();

    for (int i = 0; i < num; ++i)
    {
        MIDIINCAPS mc;
        zerostruct (mc);

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
    if (callback == 0)
        return 0;

    UINT deviceId = MIDI_MAPPER;
    int n = 0;
    String name;

    const int num = midiInGetNumDevs();

    for (int i = 0; i < num; ++i)
    {
        MIDIINCAPS mc;
        zerostruct (mc);

        if (midiInGetDevCaps (i, &mc, sizeof (mc)) == MMSYSERR_NOERROR)
        {
            if (index == n)
            {
                deviceId = i;
                name = String (mc.szPname, numElementsInArray (mc.szPname));
                break;
            }

            ++n;
        }
    }

    ScopedPointer <MidiInput> in (new MidiInput (name));
    ScopedPointer <MidiInThread> thread (new MidiInThread (in, callback));

    HMIDIIN h;
    HRESULT err = midiInOpen (&h, deviceId,
                              (DWORD_PTR) &MidiInThread::midiInCallback,
                              (DWORD_PTR) (MidiInThread*) thread,
                              CALLBACK_FUNCTION);

    if (err == MMSYSERR_NOERROR)
    {
        thread->deviceHandle = h;
        in->internal = thread.release();
        return in.release();
    }

    return 0;
}

MidiInput::MidiInput (const String& name_)
    : name (name_),
      internal (0)
{
}

MidiInput::~MidiInput()
{
    delete static_cast <MidiInThread*> (internal);
}

void MidiInput::start()
{
    static_cast <MidiInThread*> (internal)->start();
}

void MidiInput::stop()
{
    static_cast <MidiInThread*> (internal)->stop();
}


//==============================================================================
struct MidiOutHandle
{
    int refCount;
    UINT deviceId;
    HMIDIOUT handle;

    static Array<MidiOutHandle*> activeHandles;

    juce_UseDebuggingNewOperator
};

Array<MidiOutHandle*> MidiOutHandle::activeHandles;

//==============================================================================
const StringArray MidiOutput::getDevices()
{
    StringArray s;
    const int num = midiOutGetNumDevs();

    for (int i = 0; i < num; ++i)
    {
        MIDIOUTCAPS mc;
        zerostruct (mc);

        if (midiOutGetDevCaps (i, &mc, sizeof (mc)) == MMSYSERR_NOERROR)
            s.add (String (mc.szPname, sizeof (mc.szPname)));
    }

    return s;
}

int MidiOutput::getDefaultDeviceIndex()
{
    const int num = midiOutGetNumDevs();
    int n = 0;

    for (int i = 0; i < num; ++i)
    {
        MIDIOUTCAPS mc;
        zerostruct (mc);

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
    const int num = midiOutGetNumDevs();
    int i, n = 0;

    for (i = 0; i < num; ++i)
    {
        MIDIOUTCAPS mc;
        zerostruct (mc);

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

    for (i = MidiOutHandle::activeHandles.size(); --i >= 0;)
    {
        MidiOutHandle* const han = MidiOutHandle::activeHandles.getUnchecked(i);

        if (han != 0 && han->deviceId == deviceId)
        {
            han->refCount++;

            MidiOutput* const out = new MidiOutput();
            out->internal = han;
            return out;
        }
    }

    for (i = 4; --i >= 0;)
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

    return 0;
}

MidiOutput::~MidiOutput()
{
    MidiOutHandle* const h = static_cast <MidiOutHandle*> (internal);

    if (MidiOutHandle::activeHandles.contains (h) && --(h->refCount) == 0)
    {
        midiOutClose (h->handle);
        MidiOutHandle::activeHandles.removeValue (h);
        delete h;
    }
}

void MidiOutput::reset()
{
    const MidiOutHandle* const h = static_cast <const MidiOutHandle*> (internal);
    midiOutReset (h->handle);
}

bool MidiOutput::getVolume (float& leftVol, float& rightVol)
{
    const MidiOutHandle* const handle = static_cast <const MidiOutHandle*> (internal);

    DWORD n;
    if (midiOutGetVolume (handle->handle, &n) == MMSYSERR_NOERROR)
    {
        const unsigned short* const nn = reinterpret_cast<const unsigned short*> (&n);
        rightVol = nn[0] / (float) 0xffff;
        leftVol = nn[1] / (float) 0xffff;
        return true;
    }
    else
    {
        rightVol = leftVol = 1.0f;
        return false;
    }
}

void MidiOutput::setVolume (float leftVol, float rightVol)
{
    const MidiOutHandle* const handle = static_cast <MidiOutHandle*> (internal);

    DWORD n;
    unsigned short* const nn = reinterpret_cast<unsigned short*> (&n);
    nn[0] = (unsigned short) jlimit (0, 0xffff, (int) (rightVol * 0xffff));
    nn[1] = (unsigned short) jlimit (0, 0xffff, (int) (leftVol * 0xffff));
    midiOutSetVolume (handle->handle, n);
}

void MidiOutput::sendMessageNow (const MidiMessage& message)
{
    const MidiOutHandle* const handle = static_cast <const MidiOutHandle*> (internal);

    if (message.getRawDataSize() > 3
         || message.isSysEx())
    {
        MIDIHDR h;
        zerostruct (h);

        h.lpData = (char*) message.getRawData();
        h.dwBufferLength = message.getRawDataSize();
        h.dwBytesRecorded = message.getRawDataSize();

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
        midiOutShortMsg (handle->handle,
                         *(unsigned int*) message.getRawData());
    }
}

#endif

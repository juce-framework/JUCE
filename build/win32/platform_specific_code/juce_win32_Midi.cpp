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

#include "win32_headers.h"
#include "../../../src/juce_core/basics/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "../../../src/juce_appframework/audio/devices/juce_MidiOutput.h"
#include "../../../src/juce_appframework/audio/devices/juce_MidiInput.h"
#include "../../../src/juce_core/basics/juce_Time.h"
#include "../../../src/juce_core/threads/juce_Thread.h"
#include "../../../src/juce_core/containers/juce_MemoryBlock.h"

#if JUCE_MSVC
  #pragma warning (disable: 4312)
#endif

//==============================================================================
static const int midiBufferSize = 1024 * 10;
static const int numInHeaders = 32;
static const int inBufferSize = 256;

class MidiInThread  : public Thread
{
public:
    //==============================================================================
    MidiInThread (MidiInput* const input_,
                  MidiInputCallback* const callback_)
        : Thread ("Juce Midi"),
          hIn (0),
          input (input_),
          callback (callback_),
          isStarted (false),
          startTime (0),
          pendingLength(0)
    {
        for (int i = numInHeaders; --i >= 0;)
        {
            zeromem (&hdr[i], sizeof (MIDIHDR));
            hdr[i].lpData = inData[i];
            hdr[i].dwBufferLength = inBufferSize;
        }
    };

    ~MidiInThread()
    {
        stop();

        if (hIn != 0)
        {
            int count = 5;
            while (--count >= 0)
            {
                if (midiInClose (hIn) == MMSYSERR_NOERROR)
                    break;

                Sleep (20);
            }
        }
    }

    //==============================================================================
    void handle (const uint32 message, const uint32 timeStamp) throw()
    {
        jassert (validityInt == 0x12345678);
        if (validityInt != 0x12345678)
            return;

        const int byte = message & 0xff;
        if (byte < 0x80)
            return;

        const int numBytes = MidiMessage::getMessageLengthFromFirstByte ((uint8) byte);

        const double time = timeStampToTime (timeStamp);

        lock.enter();
        if (pendingLength < midiBufferSize - 12)
        {
            char* const p = pending + pendingLength;
            *(double*) p = time;
            *(uint32*) (p + 8) = numBytes;
            *(uint32*) (p + 12) = message;
            pendingLength += 12 + numBytes;
        }
        else
        {
            jassertfalse // midi buffer overflow! You might need to increase the size..
        }

        lock.exit();
        notify();
    }

    void handleSysEx (MIDIHDR* const hdr, const uint32 timeStamp) throw()
    {
        jassert (validityInt == 0x12345678);
        if (validityInt != 0x12345678)
            return;

        const int num = hdr->dwBytesRecorded;

        if (num > 0)
        {
            const double time = timeStampToTime (timeStamp);

            lock.enter();

            if (pendingLength < midiBufferSize - (8 + num))
            {
                char* const p = pending + pendingLength;
                *(double*) p = time;
                *(uint32*) (p + 8) = num;
                memcpy (p + 12, hdr->lpData, num);
                pendingLength += 12 + num;
            }
            else
            {
                jassertfalse // midi buffer overflow! You might need to increase the size..
            }

            lock.exit();
            notify();
        }
    }

    void writeBlock (const int i) throw()
    {
        hdr[i].dwBytesRecorded = 0;
        MMRESULT res = midiInPrepareHeader (hIn, &hdr[i], sizeof (MIDIHDR));
        jassert (res == MMSYSERR_NOERROR);
        res = midiInAddBuffer (hIn, &hdr[i], sizeof (MIDIHDR));
        jassert (res == MMSYSERR_NOERROR);
    }

    void run()
    {
        MemoryBlock pendingCopy (64);

        while (! threadShouldExit())
        {
            for (int i = 0; i < numInHeaders; ++i)
            {
                if ((hdr[i].dwFlags & WHDR_DONE) != 0)
                {
                    MMRESULT res = midiInUnprepareHeader (hIn, &hdr[i], sizeof (MIDIHDR));
                    (void) res;
                    jassert (res == MMSYSERR_NOERROR);
                    writeBlock (i);
                }
            }

            lock.enter();

            int len = pendingLength;

            if (len > 0)
            {
                pendingCopy.ensureSize (len);
                pendingCopy.copyFrom (pending, 0, len);
                pendingLength = 0;
            }

            lock.exit();

//xxx needs to figure out if blocks are broken up or not

            if (len == 0)
            {
                wait (500);
            }
            else
            {
                const char* p = (const char*) pendingCopy.getData();

                while (len > 0)
                {
                    const double time = *(const double*) p;
                    const int messageLen = *(const int*) (p + 8);

                    const MidiMessage message ((const uint8*) (p + 12), messageLen, time);

                    callback->handleIncomingMidiMessage (input, message);

                    p += 12 + messageLen;
                    len -= 12 + messageLen;
                }
            }
        }
    }

    void start() throw()
    {
        jassert (hIn != 0);
        if (hIn != 0 && ! isStarted)
        {
            stop();

            validityInt = 0x12345678;

            int i;
            for (i = 0; i < numInHeaders; ++i)
                writeBlock (i);

            startTime = Time::getMillisecondCounter();
            MMRESULT res = midiInStart (hIn);

            jassert (res == MMSYSERR_NOERROR);

            if (res == MMSYSERR_NOERROR)
            {
                isStarted = true;
                pendingLength = 0;
                startThread (6);
            }
        }
    }

    void stop() throw()
    {
        if (isStarted)
        {
            stopThread (5000);

            midiInReset (hIn);
            midiInStop (hIn);

            for (int i = numInHeaders; --i >= 0;)
            {
                if ((hdr[i].dwFlags & WHDR_DONE) != 0)
                {
                    int c = 10;
                    while (--c >= 0 && midiInUnprepareHeader (hIn, &hdr[i], sizeof (MIDIHDR)) == MIDIERR_STILLPLAYING)
                        Sleep (20);

                    jassert (c >= 0);
                }
            }

            isStarted = false;
            validityInt = 0;
        }
    }

    juce_UseDebuggingNewOperator

    HMIDIIN hIn;

private:
    int validityInt;
    MidiInput* input;
    MidiInputCallback* callback;
    bool isStarted;
    uint32 startTime;
    CriticalSection lock;

    MIDIHDR hdr [numInHeaders];
    char inData [numInHeaders] [inBufferSize];

    int pendingLength;
    char pending [midiBufferSize];

    double timeStampToTime (uint32 timeStamp) throw()
    {
        timeStamp += startTime;

        const uint32 now = Time::getMillisecondCounter();
        if (timeStamp > now)
        {
            if (timeStamp > now + 2)
                --startTime;

            timeStamp = now;
        }

        return 0.001 * timeStamp;
    }

    MidiInThread (const MidiInThread&);
    const MidiInThread& operator= (const MidiInThread&);
};

static void CALLBACK midiInCallback (HMIDIIN,
                                     UINT uMsg,
                                     DWORD dwInstance,
                                     DWORD midiMessage,
                                     DWORD timeStamp)
{
    MidiInThread* const thread = (MidiInThread*) dwInstance;

    if (thread != 0)
    {
        if (uMsg == MIM_DATA)
            thread->handle (midiMessage, timeStamp);
        else if (uMsg == MIM_LONGDATA)
            thread->handleSysEx ((MIDIHDR*) midiMessage, timeStamp);
    }
}

//==============================================================================
const StringArray MidiInput::getDevices()
{
    StringArray s;
    const int num = midiInGetNumDevs();

    for (int i = 0; i < num; ++i)
    {
        MIDIINCAPSW mc;
        zerostruct (mc);

        if (midiInGetDevCapsW (i, &mc, sizeof (mc)) == MMSYSERR_NOERROR)
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
        MIDIINCAPSW mc;
        zerostruct (mc);

        if (midiInGetDevCapsW (i, &mc, sizeof (mc)) == MMSYSERR_NOERROR)
        {
            if (index == n)
            {
                deviceId = i;
                name = String (mc.szPname, sizeof (mc.szPname));
                break;
            }

            ++n;
        }
    }

    MidiInput* const in = new MidiInput (name);
    MidiInThread* const thread = new MidiInThread (in, callback);

    HMIDIIN h;
    HRESULT err = midiInOpen (&h, deviceId,
                              (DWORD_PTR) &midiInCallback,
                              (DWORD_PTR) thread,
                              CALLBACK_FUNCTION);

    if (err == MMSYSERR_NOERROR)
    {
        thread->hIn = h;
        in->internal = (void*) thread;
        return in;
    }
    else
    {
        delete in;
        delete thread;
        return 0;
    }
}

MidiInput::MidiInput (const String& name_)
    : name (name_),
      internal (0)
{
}

MidiInput::~MidiInput()
{
    if (internal != 0)
    {
        MidiInThread* const thread = (MidiInThread*) internal;
        delete thread;
    }
}

void MidiInput::start()
{
    ((MidiInThread*) internal)->start();
}

void MidiInput::stop()
{
    ((MidiInThread*) internal)->stop();
}


//==============================================================================
struct MidiOutHandle
{
    int refCount;
    UINT deviceId;
    HMIDIOUT handle;

    juce_UseDebuggingNewOperator
};

static VoidArray handles (4);

//==============================================================================
const StringArray MidiOutput::getDevices()
{
    StringArray s;
    const int num = midiOutGetNumDevs();

    for (int i = 0; i < num; ++i)
    {
        MIDIOUTCAPSW mc;
        zerostruct (mc);

        if (midiOutGetDevCapsW (i, &mc, sizeof (mc)) == MMSYSERR_NOERROR)
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
        MIDIOUTCAPSW mc;
        zerostruct (mc);

        if (midiOutGetDevCapsW (i, &mc, sizeof (mc)) == MMSYSERR_NOERROR)
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
        MIDIOUTCAPSW mc;
        zerostruct (mc);

        if (midiOutGetDevCapsW (i, &mc, sizeof (mc)) == MMSYSERR_NOERROR)
        {
            // use the microsoft sw synth as a default - best not to allow deviceId
            // to be MIDI_MAPPER, or else device sharing breaks
            if (String (mc.szPname, sizeof (mc.szPname)).containsIgnoreCase (T("microsoft")))
                deviceId = i;

            if (index == n)
            {
                deviceId = i;
                break;
            }

            ++n;
        }
    }

    for (i = handles.size(); --i >= 0;)
    {
        MidiOutHandle* const han = (MidiOutHandle*) handles.getUnchecked(i);

        if (han != 0 && han->deviceId == deviceId)
        {
            han->refCount++;

            MidiOutput* const out = new MidiOutput();
            out->internal = (void*) han;
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
            handles.add (han);

            MidiOutput* const out = new MidiOutput();
            out->internal = (void*) han;
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
    MidiOutHandle* const h = (MidiOutHandle*) internal;

    if (handles.contains ((void*) h) && --(h->refCount) == 0)
    {
        midiOutClose (h->handle);
        handles.removeValue ((void*) h);
        delete h;
    }
}

void MidiOutput::reset()
{
    const MidiOutHandle* const h = (MidiOutHandle*) internal;
    midiOutReset (h->handle);
}

bool MidiOutput::getVolume (float& leftVol,
                            float& rightVol)
{
    const MidiOutHandle* const handle = (const MidiOutHandle*) internal;

    DWORD n;
    if (midiOutGetVolume (handle->handle, &n) == MMSYSERR_NOERROR)
    {
        const unsigned short* const nn = (const unsigned short*) &n;
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

void MidiOutput::setVolume (float leftVol,
                            float rightVol)
{
    const MidiOutHandle* const handle = (MidiOutHandle*) internal;

    DWORD n;
    unsigned short* const nn = (unsigned short*) &n;
    nn[0] = (unsigned short) jlimit (0, 0xffff, (int)(rightVol * 0xffff));
    nn[1] = (unsigned short) jlimit (0, 0xffff, (int)(leftVol * 0xffff));
    midiOutSetVolume (handle->handle, n);
}

void MidiOutput::sendMessageNow (const MidiMessage& message)
{
    const MidiOutHandle* const handle = (const MidiOutHandle*) internal;

    if (message.getRawDataSize() > 3)
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

END_JUCE_NAMESPACE

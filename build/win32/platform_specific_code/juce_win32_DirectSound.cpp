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

//==============================================================================
extern "C"
{

// Declare just the minimum number of interfaces for the DSound objects that we need..
typedef struct typeDSBUFFERDESC
{
    DWORD dwSize;
    DWORD dwFlags;
    DWORD dwBufferBytes;
    DWORD dwReserved;
    LPWAVEFORMATEX lpwfxFormat;
    GUID guid3DAlgorithm;
} DSBUFFERDESC;

struct IDirectSoundBuffer;

#undef INTERFACE
#define INTERFACE IDirectSound
DECLARE_INTERFACE_(IDirectSound, IUnknown)
{
    STDMETHOD(QueryInterface)       (THIS_ REFIID, LPVOID*) PURE;
    STDMETHOD_(ULONG,AddRef)        (THIS) PURE;
    STDMETHOD_(ULONG,Release)       (THIS) PURE;
    STDMETHOD(CreateSoundBuffer)    (THIS_ DSBUFFERDESC*, IDirectSoundBuffer**, LPUNKNOWN) PURE;
    STDMETHOD(GetCaps)              (THIS_ void*) PURE;
    STDMETHOD(DuplicateSoundBuffer) (THIS_ IDirectSoundBuffer*, IDirectSoundBuffer**) PURE;
    STDMETHOD(SetCooperativeLevel)  (THIS_ HWND, DWORD) PURE;
    STDMETHOD(Compact)              (THIS) PURE;
    STDMETHOD(GetSpeakerConfig)     (THIS_ LPDWORD) PURE;
    STDMETHOD(SetSpeakerConfig)     (THIS_ DWORD) PURE;
    STDMETHOD(Initialize)           (THIS_ const GUID*) PURE;
};

#undef INTERFACE
#define INTERFACE IDirectSoundBuffer
DECLARE_INTERFACE_(IDirectSoundBuffer, IUnknown)
{
    STDMETHOD(QueryInterface)       (THIS_ REFIID, LPVOID*) PURE;
    STDMETHOD_(ULONG,AddRef)        (THIS) PURE;
    STDMETHOD_(ULONG,Release)       (THIS) PURE;
    STDMETHOD(GetCaps)              (THIS_ void*) PURE;
    STDMETHOD(GetCurrentPosition)   (THIS_ LPDWORD, LPDWORD) PURE;
    STDMETHOD(GetFormat)            (THIS_ LPWAVEFORMATEX, DWORD, LPDWORD) PURE;
    STDMETHOD(GetVolume)            (THIS_ LPLONG) PURE;
    STDMETHOD(GetPan)               (THIS_ LPLONG) PURE;
    STDMETHOD(GetFrequency)         (THIS_ LPDWORD) PURE;
    STDMETHOD(GetStatus)            (THIS_ LPDWORD) PURE;
    STDMETHOD(Initialize)           (THIS_ IDirectSound*, DSBUFFERDESC*) PURE;
    STDMETHOD(Lock)                 (THIS_ DWORD, DWORD, LPVOID*, LPDWORD, LPVOID*, LPDWORD, DWORD) PURE;
    STDMETHOD(Play)                 (THIS_ DWORD, DWORD, DWORD) PURE;
    STDMETHOD(SetCurrentPosition)   (THIS_ DWORD) PURE;
    STDMETHOD(SetFormat)            (THIS_ const WAVEFORMATEX*) PURE;
    STDMETHOD(SetVolume)            (THIS_ LONG) PURE;
    STDMETHOD(SetPan)               (THIS_ LONG) PURE;
    STDMETHOD(SetFrequency)         (THIS_ DWORD) PURE;
    STDMETHOD(Stop)                 (THIS) PURE;
    STDMETHOD(Unlock)               (THIS_ LPVOID, DWORD, LPVOID, DWORD) PURE;
    STDMETHOD(Restore)              (THIS) PURE;
};

//==============================================================================
typedef struct typeDSCBUFFERDESC
{
    DWORD dwSize;
    DWORD dwFlags;
    DWORD dwBufferBytes;
    DWORD dwReserved;
    LPWAVEFORMATEX lpwfxFormat;
} DSCBUFFERDESC;

struct IDirectSoundCaptureBuffer;

#undef INTERFACE
#define INTERFACE IDirectSoundCapture
DECLARE_INTERFACE_(IDirectSoundCapture, IUnknown)
{
    STDMETHOD(QueryInterface)       (THIS_ REFIID, LPVOID*) PURE;
    STDMETHOD_(ULONG,AddRef)        (THIS) PURE;
    STDMETHOD_(ULONG,Release)       (THIS) PURE;
    STDMETHOD(CreateCaptureBuffer)  (THIS_ DSCBUFFERDESC*, IDirectSoundCaptureBuffer**, LPUNKNOWN) PURE;
    STDMETHOD(GetCaps)              (THIS_ void*) PURE;
    STDMETHOD(Initialize)           (THIS_ const GUID*) PURE;
};

#undef INTERFACE
#define INTERFACE IDirectSoundCaptureBuffer
DECLARE_INTERFACE_(IDirectSoundCaptureBuffer, IUnknown)
{
    STDMETHOD(QueryInterface)       (THIS_ REFIID, LPVOID*) PURE;
    STDMETHOD_(ULONG,AddRef)        (THIS) PURE;
    STDMETHOD_(ULONG,Release)       (THIS) PURE;
    STDMETHOD(GetCaps)              (THIS_ void*) PURE;
    STDMETHOD(GetCurrentPosition)   (THIS_ LPDWORD, LPDWORD) PURE;
    STDMETHOD(GetFormat)            (THIS_ LPWAVEFORMATEX, DWORD, LPDWORD) PURE;
    STDMETHOD(GetStatus)            (THIS_ LPDWORD) PURE;
    STDMETHOD(Initialize)           (THIS_ IDirectSoundCapture*, DSCBUFFERDESC*) PURE;
    STDMETHOD(Lock)                 (THIS_ DWORD, DWORD, LPVOID*, LPDWORD, LPVOID*, LPDWORD, DWORD) PURE;
    STDMETHOD(Start)                (THIS_ DWORD) PURE;
    STDMETHOD(Stop)                 (THIS) PURE;
    STDMETHOD(Unlock)               (THIS_ LPVOID, DWORD, LPVOID, DWORD) PURE;
};

};

//==============================================================================
BEGIN_JUCE_NAMESPACE

#include "../../../src/juce_appframework/audio/devices/juce_AudioIODeviceType.h"
#include "../../../src/juce_appframework/application/juce_Application.h"
#include "../../../src/juce_core/threads/juce_Thread.h"
#include "../../../src/juce_core/basics/juce_Singleton.h"
#include "../../../src/juce_core/basics/juce_Time.h"
#include "../../../src/juce_core/containers/juce_OwnedArray.h"
#include "../../../src/juce_core/text/juce_LocalisedStrings.h"
#include "../../../src/juce_appframework/application/juce_DeletedAtShutdown.h"

static const String getDSErrorMessage (HRESULT hr)
{
    const char* result = 0;

    switch (hr)
    {
    case MAKE_HRESULT(1, 0x878, 10):
        result = "Device already allocated";
        break;
    case MAKE_HRESULT(1, 0x878, 30):
        result = "Control unavailable";
        break;
    case E_INVALIDARG:
        result = "Invalid parameter";
        break;
    case MAKE_HRESULT(1, 0x878, 50):
        result = "Invalid call";
        break;
    case E_FAIL:
        result = "Generic error";
        break;
    case MAKE_HRESULT(1, 0x878, 70):
        result = "Priority level error";
        break;
    case E_OUTOFMEMORY:
        result = "Out of memory";
        break;
    case MAKE_HRESULT(1, 0x878, 100):
        result = "Bad format";
        break;
    case E_NOTIMPL:
        result = "Unsupported function";
        break;
    case MAKE_HRESULT(1, 0x878, 120):
        result = "No driver";
        break;
    case MAKE_HRESULT(1, 0x878, 130):
        result = "Already initialised";
        break;
    case CLASS_E_NOAGGREGATION:
        result = "No aggregation";
        break;
    case MAKE_HRESULT(1, 0x878, 150):
        result = "Buffer lost";
        break;
    case MAKE_HRESULT(1, 0x878, 160):
        result = "Another app has priority";
        break;
    case MAKE_HRESULT(1, 0x878, 170):
        result = "Uninitialised";
        break;
    case E_NOINTERFACE:
        result = "No interface";
        break;
    case S_OK:
        result = "No error";
        break;

    default:
        return "Unknown error: " + String ((int) hr);
    }

    return result;
}

//==============================================================================
#define DS_DEBUGGING 1

#ifdef DS_DEBUGGING
    #define CATCH JUCE_CATCH_EXCEPTION
    #define log(a) Logger::writeToLog(a);
    #define logError(a) logDSError(a, __LINE__);

    static void logDSError (HRESULT hr, int lineNum)
    {
        if (hr != S_OK)
        {
            String error ("DS error at line ");
            error << lineNum << T(" - ") << getDSErrorMessage (hr);
            log (error);
        }
    }
#else
    #define CATCH JUCE_CATCH_ALL
    #define log(a)
    #define logError(a)
#endif


//==============================================================================
#define DSOUND_FUNCTION(functionName, params) \
    typedef HRESULT (WINAPI *type##functionName) params; \
    static type##functionName ds##functionName = 0;

#define DSOUND_FUNCTION_LOAD(functionName) \
    ds##functionName = (type##functionName) GetProcAddress (h, #functionName);  \
    jassert (ds##functionName != 0);

typedef BOOL (CALLBACK *LPDSENUMCALLBACKW) (LPGUID, LPCWSTR, LPCWSTR, LPVOID);
typedef BOOL (CALLBACK *LPDSENUMCALLBACKA) (LPGUID, LPCSTR, LPCSTR, LPVOID);

DSOUND_FUNCTION (DirectSoundCreate, (const GUID*, IDirectSound**, LPUNKNOWN))
DSOUND_FUNCTION (DirectSoundCaptureCreate, (const GUID*, IDirectSoundCapture**, LPUNKNOWN))
DSOUND_FUNCTION (DirectSoundEnumerateW, (LPDSENUMCALLBACKW, LPVOID))
DSOUND_FUNCTION (DirectSoundCaptureEnumerateW, (LPDSENUMCALLBACKW, LPVOID))

#if JUCE_ENABLE_WIN98_COMPATIBILITY
 DSOUND_FUNCTION (DirectSoundEnumerateA, (LPDSENUMCALLBACKA, LPVOID))
 DSOUND_FUNCTION (DirectSoundCaptureEnumerateA, (LPDSENUMCALLBACKA, LPVOID))
#endif

static void initialiseDSoundFunctions()
{
    if (dsDirectSoundCreate == 0)
    {
        HMODULE h = LoadLibraryA ("dsound.dll");

        DSOUND_FUNCTION_LOAD (DirectSoundCreate)
        DSOUND_FUNCTION_LOAD (DirectSoundCaptureCreate)
        DSOUND_FUNCTION_LOAD (DirectSoundEnumerateW)
        DSOUND_FUNCTION_LOAD (DirectSoundCaptureEnumerateW)

#if JUCE_ENABLE_WIN98_COMPATIBILITY
        DSOUND_FUNCTION_LOAD (DirectSoundEnumerateA)
        DSOUND_FUNCTION_LOAD (DirectSoundCaptureEnumerateA)
#endif
    }
}

//==============================================================================
class DSoundInternalOutChannel
{
    String name;
    LPGUID guid;
    int sampleRate, bufferSizeSamples;
    float* leftBuffer;
    float* rightBuffer;

    IDirectSound* pDirectSound;
    IDirectSoundBuffer* pOutputBuffer;
    DWORD writeOffset;
    int totalBytesPerBuffer;
    int bytesPerBuffer;
    unsigned int lastPlayCursor;

public:
    int bitDepth;
    bool doneFlag;

    DSoundInternalOutChannel (const String& name_,
                              LPGUID guid_,
                              int rate,
                              int bufferSize,
                              float* left,
                              float* right)
        : name (name_),
          guid (guid_),
          sampleRate (rate),
          bufferSizeSamples (bufferSize),
          leftBuffer (left),
          rightBuffer (right),
          pDirectSound (0),
          pOutputBuffer (0),
          bitDepth (16)
    {
    }

    ~DSoundInternalOutChannel()
    {
        close();
    }

    void close()
    {
        HRESULT hr;

        if (pOutputBuffer != 0)
        {
            JUCE_TRY
            {
                log (T("closing dsound out: ") + name);
                hr = pOutputBuffer->Stop();
                logError (hr);
            }
            CATCH

            JUCE_TRY
            {
                hr = pOutputBuffer->Release();
                logError (hr);
            }
            CATCH

            pOutputBuffer = 0;
        }

        if (pDirectSound != 0)
        {
            JUCE_TRY
            {
                hr = pDirectSound->Release();
                logError (hr);
            }
            CATCH

            pDirectSound = 0;
        }
    }

    const String open()
    {
        log (T("opening dsound out device: ") + name
             + T("  rate=") + String (sampleRate)
             + T(" bits=") + String (bitDepth)
             + T(" buf=") + String (bufferSizeSamples));

        pDirectSound = 0;
        pOutputBuffer = 0;
        writeOffset = 0;

        String error;
        HRESULT hr = E_NOINTERFACE;

        if (dsDirectSoundCreate != 0)
            hr = dsDirectSoundCreate (guid, &pDirectSound, 0);

        if (hr == S_OK)
        {
            bytesPerBuffer = (bufferSizeSamples * (bitDepth >> 2)) & ~15;
            totalBytesPerBuffer = (3 * bytesPerBuffer) & ~15;
            const int numChannels = 2;

            hr = pDirectSound->SetCooperativeLevel (GetDesktopWindow(), 3 /* DSSCL_EXCLUSIVE */);
            logError (hr);

            if (hr == S_OK)
            {
                IDirectSoundBuffer* pPrimaryBuffer;

                DSBUFFERDESC primaryDesc;
                zerostruct (primaryDesc);

                primaryDesc.dwSize = sizeof (DSBUFFERDESC);
                primaryDesc.dwFlags = 1 /* DSBCAPS_PRIMARYBUFFER */;
                primaryDesc.dwBufferBytes = 0;
                primaryDesc.lpwfxFormat = 0;

                log ("opening dsound out step 2");
                hr = pDirectSound->CreateSoundBuffer (&primaryDesc, &pPrimaryBuffer, 0);
                logError (hr);

                if (hr == S_OK)
                {
                    WAVEFORMATEX wfFormat;
                    wfFormat.wFormatTag = WAVE_FORMAT_PCM;
                    wfFormat.nChannels = (unsigned short) numChannels;
                    wfFormat.nSamplesPerSec = sampleRate;
                    wfFormat.wBitsPerSample = (unsigned short) bitDepth;
                    wfFormat.nBlockAlign = (unsigned short) (wfFormat.nChannels * wfFormat.wBitsPerSample / 8);
                    wfFormat.nAvgBytesPerSec = wfFormat.nSamplesPerSec * wfFormat.nBlockAlign;
                    wfFormat.cbSize = 0;

                    hr = pPrimaryBuffer->SetFormat (&wfFormat);
                    logError (hr);

                    if (hr == S_OK)
                    {
                        DSBUFFERDESC secondaryDesc;
                        zerostruct (secondaryDesc);

                        secondaryDesc.dwSize = sizeof (DSBUFFERDESC);
                        secondaryDesc.dwFlags =  0x8000 /* DSBCAPS_GLOBALFOCUS */
                                                  | 0x10000 /* DSBCAPS_GETCURRENTPOSITION2 */;
                        secondaryDesc.dwBufferBytes = totalBytesPerBuffer;
                        secondaryDesc.lpwfxFormat = &wfFormat;

                        hr = pDirectSound->CreateSoundBuffer (&secondaryDesc, &pOutputBuffer, 0);
                        logError (hr);

                        if (hr == S_OK)
                        {
                            log ("opening dsound out step 3");

                            DWORD dwDataLen;
                            unsigned char* pDSBuffData;

                            hr = pOutputBuffer->Lock (0, totalBytesPerBuffer,
                                                      (LPVOID*) &pDSBuffData, &dwDataLen, 0, 0, 0);
                            logError (hr);

                            if (hr == S_OK)
                            {
                                zeromem (pDSBuffData, dwDataLen);

                                hr = pOutputBuffer->Unlock (pDSBuffData, dwDataLen, 0, 0);

                                if (hr == S_OK)
                                {
                                    hr = pOutputBuffer->SetCurrentPosition (0);

                                    if (hr == S_OK)
                                    {
                                        hr = pOutputBuffer->Play (0, 0, 1 /* DSBPLAY_LOOPING */);

                                        if (hr == S_OK)
                                            return String::empty;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        error = getDSErrorMessage (hr);
        close();
        return error;
    }

    void synchronisePosition()
    {
        if (pOutputBuffer != 0)
        {
            DWORD playCursor;
            pOutputBuffer->GetCurrentPosition (&playCursor, &writeOffset);
        }
    }

    bool service()
    {
        if (pOutputBuffer == 0)
            return true;

        DWORD playCursor, writeCursor;
        HRESULT hr = pOutputBuffer->GetCurrentPosition (&playCursor, &writeCursor);

        if (hr != S_OK)
        {
            logError (hr);
            jassertfalse
            return true;
        }

        int playWriteGap = writeCursor - playCursor;
        if (playWriteGap < 0)
            playWriteGap += totalBytesPerBuffer;

        int bytesEmpty = playCursor - writeOffset;

        if (bytesEmpty < 0)
            bytesEmpty += totalBytesPerBuffer;

        if (bytesEmpty > (totalBytesPerBuffer - playWriteGap))
        {
            writeOffset = writeCursor;
            bytesEmpty = totalBytesPerBuffer - playWriteGap;
        }

        if (bytesEmpty >= bytesPerBuffer)
        {
            LPBYTE lpbuf1 = 0;
            LPBYTE lpbuf2 = 0;
            DWORD dwSize1 = 0;
            DWORD dwSize2 = 0;

            HRESULT hr = pOutputBuffer->Lock (writeOffset,
                                              bytesPerBuffer,
                                              (void**) &lpbuf1, &dwSize1,
                                              (void**) &lpbuf2, &dwSize2, 0);

            if (hr == S_OK)
            {
                if (bitDepth == 16)
                {
                    const float gainL = 32767.0f;
                    const float gainR = 32767.0f;

                    int* dest = (int*)lpbuf1;
                    const float* left = leftBuffer;
                    const float* right = rightBuffer;
                    int samples1 = dwSize1 >> 2;
                    int samples2 = dwSize2 >> 2;

                    if (left == 0)
                    {
                        while (--samples1 >= 0)
                        {
                            int r = roundFloatToInt (gainR * *right++);

                            if (r < -32768)
                                r = -32768;
                            else if (r > 32767)
                                r = 32767;

                            *dest++ = (r << 16);
                        }

                        dest = (int*)lpbuf2;

                        while (--samples2 >= 0)
                        {
                            int r = roundFloatToInt (gainR * *right++);

                            if (r < -32768)
                                r = -32768;
                            else if (r > 32767)
                                r = 32767;

                            *dest++ = (r << 16);
                        }
                    }
                    else if (right == 0)
                    {
                        while (--samples1 >= 0)
                        {
                            int l = roundFloatToInt (gainL * *left++);

                            if (l < -32768)
                                l = -32768;
                            else if (l > 32767)
                                l = 32767;

                            l &= 0xffff;

                            *dest++ = l;
                        }

                        dest = (int*)lpbuf2;

                        while (--samples2 >= 0)
                        {
                            int l = roundFloatToInt (gainL * *left++);

                            if (l < -32768)
                                l = -32768;
                            else if (l > 32767)
                                l = 32767;

                            l &= 0xffff;

                            *dest++ = l;
                        }
                    }
                    else
                    {
                        while (--samples1 >= 0)
                        {
                            int l = roundFloatToInt (gainL * *left++);

                            if (l < -32768)
                                l = -32768;
                            else if (l > 32767)
                                l = 32767;

                            l &= 0xffff;

                            int r = roundFloatToInt (gainR * *right++);

                            if (r < -32768)
                                r = -32768;
                            else if (r > 32767)
                                r = 32767;

                            *dest++ = (r << 16) | l;
                        }

                        dest = (int*)lpbuf2;

                        while (--samples2 >= 0)
                        {
                            int l = roundFloatToInt (gainL * *left++);

                            if (l < -32768)
                                l = -32768;
                            else if (l > 32767)
                                l = 32767;

                            l &= 0xffff;

                            int r = roundFloatToInt (gainR * *right++);

                            if (r < -32768)
                                r = -32768;
                            else if (r > 32767)
                                r = 32767;

                            *dest++ = (r << 16) | l;
                        }
                    }
                }
                else
                {
                    jassertfalse
                }

                writeOffset = (writeOffset + dwSize1 + dwSize2) % totalBytesPerBuffer;

                pOutputBuffer->Unlock (lpbuf1, dwSize1, lpbuf2, dwSize2);
            }
            else
            {
                jassertfalse
                logError (hr);
            }

            bytesEmpty -= bytesPerBuffer;

            return true;
        }
        else
        {
            return false;
        }
    }
};

//==============================================================================
struct DSoundInternalInChannel
{
    String name;
    LPGUID guid;
    int sampleRate, bufferSizeSamples;
    float* leftBuffer;
    float* rightBuffer;

    IDirectSound* pDirectSound;
    IDirectSoundCapture* pDirectSoundCapture;
    IDirectSoundCaptureBuffer* pInputBuffer;

public:
    unsigned int readOffset;
    int bytesPerBuffer, totalBytesPerBuffer;
    int bitDepth;
    bool doneFlag;

    DSoundInternalInChannel (const String& name_,
                             LPGUID guid_,
                             int rate,
                             int bufferSize,
                             float* left,
                             float* right)
        : name (name_),
          guid (guid_),
          sampleRate (rate),
          bufferSizeSamples (bufferSize),
          leftBuffer (left),
          rightBuffer (right),
          pDirectSound (0),
          pDirectSoundCapture (0),
          pInputBuffer (0),
          bitDepth (16)
    {
    }

    ~DSoundInternalInChannel()
    {
        close();
    }

    void close()
    {
        HRESULT hr;

        if (pInputBuffer != 0)
        {
            JUCE_TRY
            {
                log (T("closing dsound in: ") + name);
                hr = pInputBuffer->Stop();
                logError (hr);
            }
            CATCH

            JUCE_TRY
            {
                hr = pInputBuffer->Release();
                logError (hr);
            }
            CATCH

            pInputBuffer = 0;
        }

        if (pDirectSoundCapture != 0)
        {
            JUCE_TRY
            {
                hr = pDirectSoundCapture->Release();
                logError (hr);
            }
            CATCH

            pDirectSoundCapture = 0;
        }

        if (pDirectSound != 0)
        {
            JUCE_TRY
            {
                hr = pDirectSound->Release();
                logError (hr);
            }
            CATCH

            pDirectSound = 0;
        }
    }

    const String open()
    {
        log (T("opening dsound in device: ") + name
             + T("  rate=") + String (sampleRate) + T(" bits=") + String (bitDepth) + T(" buf=") + String (bufferSizeSamples));

        pDirectSound = 0;
        pDirectSoundCapture = 0;
        pInputBuffer = 0;
        readOffset = 0;
        totalBytesPerBuffer = 0;

        String error;
        HRESULT hr = E_NOINTERFACE;

        if (dsDirectSoundCaptureCreate != 0)
            hr = dsDirectSoundCaptureCreate (guid, &pDirectSoundCapture, 0);

        logError (hr);

        if (hr == S_OK)
        {
            const int numChannels = 2;
            bytesPerBuffer = (bufferSizeSamples * (bitDepth >> 2)) & ~15;
            totalBytesPerBuffer = (3 * bytesPerBuffer) & ~15;

            WAVEFORMATEX wfFormat;
            wfFormat.wFormatTag = WAVE_FORMAT_PCM;
            wfFormat.nChannels = (unsigned short)numChannels;
            wfFormat.nSamplesPerSec = sampleRate;
            wfFormat.wBitsPerSample = (unsigned short)bitDepth;
            wfFormat.nBlockAlign = (unsigned short)(wfFormat.nChannels * (wfFormat.wBitsPerSample / 8));
            wfFormat.nAvgBytesPerSec = wfFormat.nSamplesPerSec * wfFormat.nBlockAlign;
            wfFormat.cbSize = 0;

            DSCBUFFERDESC captureDesc;
            zerostruct (captureDesc);

            captureDesc.dwSize = sizeof (DSCBUFFERDESC);
            captureDesc.dwFlags = 0;
            captureDesc.dwBufferBytes = totalBytesPerBuffer;
            captureDesc.lpwfxFormat = &wfFormat;

            log (T("opening dsound in step 2"));
            hr = pDirectSoundCapture->CreateCaptureBuffer (&captureDesc, &pInputBuffer, 0);

            logError (hr);

            if (hr == S_OK)
            {
                hr = pInputBuffer->Start (1 /* DSCBSTART_LOOPING */);
                logError (hr);

                if (hr == S_OK)
                    return String::empty;
            }
        }

        error = getDSErrorMessage (hr);
        close();

        return error;
    }

    void synchronisePosition()
    {
        if (pInputBuffer != 0)
        {
            DWORD capturePos;
            pInputBuffer->GetCurrentPosition (&capturePos, (DWORD*)&readOffset);
        }
    }

    bool service()
    {
        if (pInputBuffer == 0)
            return true;

        DWORD capturePos, readPos;
        HRESULT hr = pInputBuffer->GetCurrentPosition (&capturePos, &readPos);
        logError (hr);

        if (hr != S_OK)
            return true;

        int bytesFilled = readPos - readOffset;
        if (bytesFilled < 0)
            bytesFilled += totalBytesPerBuffer;

        if (bytesFilled >= bytesPerBuffer)
        {
            LPBYTE lpbuf1 = 0;
            LPBYTE lpbuf2 = 0;
            DWORD dwsize1 = 0;
            DWORD dwsize2 = 0;

            HRESULT hr = pInputBuffer->Lock (readOffset,
                                             bytesPerBuffer,
                                             (void**) &lpbuf1, &dwsize1,
                                             (void**) &lpbuf2, &dwsize2, 0);

            if (hr == S_OK)
            {
                if (bitDepth == 16)
                {
                    const float g = 1.0f / 32768.0f;

                    float* destL = leftBuffer;
                    float* destR = rightBuffer;
                    int samples1 = dwsize1 >> 2;
                    int samples2 = dwsize2 >> 2;

                    const short* src = (const short*)lpbuf1;

                    if (destL == 0)
                    {
                        while (--samples1 >= 0)
                        {
                            ++src;
                            *destR++ = *src++ * g;
                        }

                        src = (const short*)lpbuf2;

                        while (--samples2 >= 0)
                        {
                            ++src;
                            *destR++ = *src++ * g;
                        }
                    }
                    else if (destR == 0)
                    {
                        while (--samples1 >= 0)
                        {
                            *destL++ = *src++ * g;
                            ++src;
                        }

                        src = (const short*)lpbuf2;

                        while (--samples2 >= 0)
                        {
                            *destL++ = *src++ * g;
                            ++src;
                        }
                    }
                    else
                    {
                        while (--samples1 >= 0)
                        {
                            *destL++ = *src++ * g;
                            *destR++ = *src++ * g;
                        }

                        src = (const short*)lpbuf2;

                        while (--samples2 >= 0)
                        {
                            *destL++ = *src++ * g;
                            *destR++ = *src++ * g;
                        }
                    }
                }
                else
                {
                    jassertfalse
                }

                readOffset = (readOffset + dwsize1 + dwsize2) % totalBytesPerBuffer;

                pInputBuffer->Unlock (lpbuf1, dwsize1, lpbuf2, dwsize2);
            }
            else
            {
                logError (hr);
                jassertfalse
            }

            bytesFilled -= bytesPerBuffer;

            return true;
        }
        else
        {
            return false;
        }
    }
};

//==============================================================================
static int findBestMatchForName (const String& name, const StringArray& names)
{
    int i = names.indexOf (name);

    if (i >= 0)
        return i;

    StringArray tokens1;
    tokens1.addTokens (name, T(" :-"), 0);

    int bestResult = -1;
    int bestNumMatches = 1;

    for (i = 0; i < names.size(); ++i)
    {
        StringArray tokens2;
        tokens2.addTokens (names[i], T(" :-"), 0);

        int matches = 0;

        for (int j = tokens1.size(); --j >= 0;)
            if (tokens2.contains (tokens1 [j]))
                ++matches;

        if (matches > bestNumMatches)
            bestResult = i;
    }

    return bestResult;
}

class DSoundAudioIODevice  : public AudioIODevice,
                             public Thread
{
public:
    DSoundAudioIODevice (const String& deviceName,
                         const int index,
                         const int inputIndex_)
        : AudioIODevice (deviceName, "DirectSound"),
          Thread ("Juce DSound"),
          isOpen_ (false),
          isStarted (false),
          deviceIndex (index),
          inputIndex (inputIndex_),
          inChans (4),
          outChans (4),
          numInputBuffers (0),
          numOutputBuffers (0),
          totalSamplesOut (0),
          sampleRate (0.0),
          inputBuffers (0),
          outputBuffers (0),
          callback (0)
    {
    }

    ~DSoundAudioIODevice()
    {
        close();
    }

    const StringArray getOutputChannelNames()
    {
        return outChannels;
    }

    const StringArray getInputChannelNames()
    {
        return inChannels;
    }

    int getNumSampleRates()
    {
        return 4;
    }

    double getSampleRate (int index)
    {
        const double samps[] = { 44100.0, 48000.0, 88200.0, 96000.0 };

        return samps [jlimit (0, 3, index)];
    }

    int getNumBufferSizesAvailable()
    {
        return 50;
    }

    int getBufferSizeSamples (int index)
    {
        int n = 64;
        for (int i = 0; i < index; ++i)
            n += (n < 512) ? 32
                           : ((n < 1024) ? 64
                                         : ((n < 2048) ? 128 : 256));

        return n;
    }

    int getDefaultBufferSize()
    {
        return 2560;
    }

    const String open (const BitArray& inputChannels,
                       const BitArray& outputChannels,
                       double sampleRate,
                       int bufferSizeSamples)
    {
        BitArray ins, outs;

        if (deviceIndex >= 0)
        {
            if (outputChannels[0])
                outs.setBit (2 * deviceIndex);

            if (outputChannels[1])
                outs.setBit (2 * deviceIndex + 1);

            if (inputIndex >= 0)
            {
                if (inputChannels[0])
                    ins.setBit (2 * inputIndex);

                if (inputChannels[1])
                    ins.setBit (2 * inputIndex + 1);
            }
        }
        else
        {
            ins = inputChannels;
            outs = outputChannels;
        }

        lastError = openDevice (ins, outs, sampleRate, bufferSizeSamples);
        isOpen_ = lastError.isEmpty();

        return lastError;
    }

    void close()
    {
        stop();

        if (isOpen_)
        {
            closeDevice();
            isOpen_ = false;
        }
    }

    bool isOpen()
    {
        return isOpen_ && isThreadRunning();
    }

    int getCurrentBufferSizeSamples()
    {
        return bufferSizeSamples;
    }

    double getCurrentSampleRate()
    {
        return sampleRate;
    }

    int getCurrentBitDepth()
    {
        int i, bits = 256;

        for (i = inChans.size(); --i >= 0;)
            if (inChans[i] != 0)
                bits = jmin (bits, inChans[i]->bitDepth);

        for (i = outChans.size(); --i >= 0;)
            if (outChans[i] != 0)
                bits = jmin (bits, outChans[i]->bitDepth);

        if (bits > 32)
            bits = 16;

        return bits;
    }

    int getOutputLatencyInSamples()
    {
        return (int) (getCurrentBufferSizeSamples() * 1.5);
    }

    int getInputLatencyInSamples()
    {
        return getOutputLatencyInSamples();
    }

    void start (AudioIODeviceCallback* call)
    {
        if (isOpen_ && call != 0 && ! isStarted)
        {
            if (! isThreadRunning())
            {
                // something gone wrong and the thread's stopped..
                isOpen_ = false;
                return;
            }

            call->audioDeviceAboutToStart (sampleRate, bufferSizeSamples);

            const ScopedLock sl (startStopLock);
            callback = call;
            isStarted = true;
        }
    }

    void stop()
    {
        if (isStarted)
        {
            AudioIODeviceCallback* const callbackLocal = callback;

            {
                const ScopedLock sl (startStopLock);
                isStarted = false;
            }

            if (callbackLocal != 0)
                callbackLocal->audioDeviceStopped();
        }
    }

    bool isPlaying()
    {
        return isStarted && isOpen_ && isThreadRunning();
    }

    const String getLastError()
    {
        return lastError;
    }

    //==============================================================================
    juce_UseDebuggingNewOperator

    StringArray inChannels, outChannels;

private:
    bool isOpen_;
    bool isStarted;
    String lastError;

    int deviceIndex, inputIndex;
    OwnedArray <DSoundInternalInChannel> inChans;
    OwnedArray <DSoundInternalOutChannel> outChans;
    WaitableEvent startEvent;

    int numInputBuffers, numOutputBuffers, bufferSizeSamples;
    int volatile totalSamplesOut;
    int64 volatile lastBlockTime;
    double sampleRate;
    float** inputBuffers;
    float** outputBuffers;

    AudioIODeviceCallback* callback;
    CriticalSection startStopLock;

    DSoundAudioIODevice (const DSoundAudioIODevice&);
    const DSoundAudioIODevice& operator= (const DSoundAudioIODevice&);

    const String openDevice (const BitArray& inputChannels,
                             const BitArray& outputChannels,
                             double sampleRate_,
                             int bufferSizeSamples_);

    void closeDevice()
    {
        isStarted = false;
        stopThread (5000);

        inChans.clear();
        outChans.clear();

        int i;
        for (i = 0; i < numInputBuffers; ++i)
            juce_free (inputBuffers[i]);

        delete[] inputBuffers;
        inputBuffers = 0;
        numInputBuffers = 0;

        for (i = 0; i < numOutputBuffers; ++i)
            juce_free (outputBuffers[i]);

        delete[] outputBuffers;
        outputBuffers = 0;
        numOutputBuffers = 0;
    }

    void resync()
    {
        int i;
        for (i = outChans.size(); --i >= 0;)
        {
            DSoundInternalOutChannel* const out = outChans.getUnchecked(i);
            if (out != 0)
                out->close();
        }

        for (i = inChans.size(); --i >= 0;)
        {
            DSoundInternalInChannel* const in = inChans.getUnchecked(i);
            if (in != 0)
                in->close();
        }

        if (threadShouldExit())
            return;

        for (i = outChans.size(); --i >= 0;)
        {
            DSoundInternalOutChannel* const out = outChans.getUnchecked(i);
            if (out != 0)
                out->open();
        }

        for (i = inChans.size(); --i >= 0;)
        {
            DSoundInternalInChannel* const in = inChans.getUnchecked(i);
            if (in != 0)
                in->open();
        }

        if (threadShouldExit())
            return;

        sleep (5);

        for (i = 0; i < numOutputBuffers; ++i)
            if (outChans[i] != 0)
                outChans[i]->synchronisePosition();

        for (i = 0; i < numInputBuffers; ++i)
            if (inChans[i] != 0)
                inChans[i]->synchronisePosition();
    }

public:
    void run()
    {
        while (! threadShouldExit())
        {
            if (wait (100))
                break;
        }

        const int latencyMs = (int) (bufferSizeSamples * 1000.0 / sampleRate);
        const int maxTimeMS = jmax (5, 3 * latencyMs);

        while (! threadShouldExit())
        {
            int numToDo = 0;
            uint32 startTime = Time::getMillisecondCounter();

            int i;
            for (i = inChans.size(); --i >= 0;)
            {
                DSoundInternalInChannel* const in = inChans.getUnchecked(i);

                if (in != 0)
                {
                    in->doneFlag = false;
                    ++numToDo;
                }
            }

            for (i = outChans.size(); --i >= 0;)
            {
                DSoundInternalOutChannel* const out = outChans.getUnchecked(i);

                if (out != 0)
                {
                    out->doneFlag = false;
                    ++numToDo;
                }
            }

            if (numToDo > 0)
            {
                const int maxCount = 3;
                int count = maxCount;

                for (;;)
                {
                    for (i = inChans.size(); --i >= 0;)
                    {
                        DSoundInternalInChannel* const in = inChans.getUnchecked(i);

                        if (in != 0 && !in->doneFlag)
                        {
                            if (in->service())
                            {
                                in->doneFlag = true;
                                --numToDo;
                            }
                        }
                    }

                    for (i = outChans.size(); --i >= 0;)
                    {
                        DSoundInternalOutChannel* const out = outChans.getUnchecked(i);

                        if (out != 0 && !out->doneFlag)
                        {
                            if (out->service())
                            {
                                out->doneFlag = true;
                                --numToDo;
                            }
                        }
                    }

                    if (numToDo <= 0)
                        break;

                    if (Time::getMillisecondCounter() > startTime + maxTimeMS)
                    {
                        resync();
                        break;
                    }

                    if (--count <= 0)
                    {
                        Sleep (1);
                        count = maxCount;
                    }

                    if (threadShouldExit())
                        return;
                }
            }
            else
            {
                sleep (1);
            }

            const ScopedLock sl (startStopLock);

            if (isStarted)
            {
                JUCE_TRY
                {
                    callback->audioDeviceIOCallback ((const float**) inputBuffers,
                                                     numInputBuffers,
                                                     outputBuffers,
                                                     numOutputBuffers,
                                                     bufferSizeSamples);
                }
                JUCE_CATCH_EXCEPTION

                totalSamplesOut += bufferSizeSamples;
            }
            else
            {
                for (i = 0; i < numOutputBuffers; ++i)
                    if (outputBuffers[i] != 0)
                        zeromem (outputBuffers[i], bufferSizeSamples * sizeof (float));

                totalSamplesOut = 0;
                sleep (1);
            }
        }
    }
};

//==============================================================================
class DSoundAudioIODeviceType  : public AudioIODeviceType
{
public:
    DSoundAudioIODeviceType()
        : AudioIODeviceType (T("DirectSound")),
          hasScanned (false)
    {
        initialiseDSoundFunctions();
    }

    ~DSoundAudioIODeviceType()
    {
    }

    //==============================================================================
    void scanForDevices()
    {
        hasScanned = true;

        outputDeviceNames.clear();
        outputGuids.clear();
        inputDeviceNames.clear();
        inputGuids.clear();

        if (dsDirectSoundEnumerateW != 0)
        {
            dsDirectSoundEnumerateW (outputEnumProcW, this);
            dsDirectSoundCaptureEnumerateW (inputEnumProcW, this);
        }
#if JUCE_ENABLE_WIN98_COMPATIBILITY
        else if (dsDirectSoundEnumerateA != 0)
        {
            dsDirectSoundEnumerateA (outputEnumProcA, this);
            dsDirectSoundCaptureEnumerateA (inputEnumProcA, this);
        }
#endif
    }

    const StringArray getDeviceNames (const bool preferInputNames) const
    {
        jassert (hasScanned); // need to call scanForDevices() before doing this

        return preferInputNames ? inputDeviceNames
                                : outputDeviceNames;
    }

    const String getDefaultDeviceName (const bool preferInputNames) const
    {
        jassert (hasScanned); // need to call scanForDevices() before doing this

        return getDeviceNames (preferInputNames) [0];
    }

    AudioIODevice* createDevice (const String& deviceName)
    {
        jassert (hasScanned); // need to call scanForDevices() before doing this

        if (deviceName.isEmpty() || deviceName.equalsIgnoreCase (T("DirectSound")))
        {
            DSoundAudioIODevice* device = new DSoundAudioIODevice (deviceName, -1, -1);

            int i;
            for (i = 0; i < outputDeviceNames.size(); ++i)
            {
                device->outChannels.add (outputDeviceNames[i] + TRANS(" (left)"));
                device->outChannels.add (outputDeviceNames[i] + TRANS(" (right)"));
            }

            for (i = 0; i < inputDeviceNames.size(); ++i)
            {
                device->inChannels.add (inputDeviceNames[i] + TRANS(" (left)"));
                device->inChannels.add (inputDeviceNames[i] + TRANS(" (right)"));
            }

            return device;
        }
        else if (outputDeviceNames.contains (deviceName)
                  || inputDeviceNames.contains (deviceName))
        {
            int outputIndex = outputDeviceNames.indexOf (deviceName);
            int inputIndex = findBestMatchForName (deviceName, inputDeviceNames);

            if (outputIndex < 0)
            {
                // using an input device name instead..
                inputIndex = inputDeviceNames.indexOf (deviceName);
                outputIndex = jmax (0, findBestMatchForName (deviceName, outputDeviceNames));
            }

            DSoundAudioIODevice* device = new DSoundAudioIODevice (deviceName, outputIndex, inputIndex);

            device->outChannels.add (TRANS("Left"));
            device->outChannels.add (TRANS("Right"));

            if (inputIndex >= 0)
            {
                device->inChannels.add (TRANS("Left"));
                device->inChannels.add (TRANS("Right"));
            }

            return device;
        }

        return 0;
    }

    //==============================================================================
    juce_UseDebuggingNewOperator

    StringArray outputDeviceNames;
    OwnedArray <GUID> outputGuids;

    StringArray inputDeviceNames;
    OwnedArray <GUID> inputGuids;

private:
    bool hasScanned;

    //==============================================================================
    BOOL outputEnumProc (LPGUID lpGUID, String desc)
    {
        desc = desc.trim();

        if (desc.isNotEmpty())
        {
            const String origDesc (desc);

            int n = 2;
            while (outputDeviceNames.contains (desc))
                desc = origDesc + T(" (") + String (n++) + T(")");

            outputDeviceNames.add (desc);

            if (lpGUID != 0)
                outputGuids.add (new GUID (*lpGUID));
            else
                outputGuids.add (0);
        }

        return TRUE;
    }

    static BOOL CALLBACK outputEnumProcW (LPGUID lpGUID, LPCWSTR description, LPCWSTR, LPVOID object)
    {
        return ((DSoundAudioIODeviceType*) object)
                    ->outputEnumProc (lpGUID, String (description));
    }

    static BOOL CALLBACK outputEnumProcA (LPGUID lpGUID, LPCTSTR description, LPCTSTR, LPVOID object)
    {
        return ((DSoundAudioIODeviceType*) object)
                    ->outputEnumProc (lpGUID, String (description));
    }

    //==============================================================================
    BOOL CALLBACK inputEnumProc (LPGUID lpGUID, String desc)
    {
        desc = desc.trim();

        if (desc.isNotEmpty())
        {
            const String origDesc (desc);

            int n = 2;
            while (inputDeviceNames.contains (desc))
                desc = origDesc + T(" (") + String (n++) + T(")");

            inputDeviceNames.add (desc);

            if (lpGUID != 0)
                inputGuids.add (new GUID (*lpGUID));
            else
                inputGuids.add (0);
        }

        return TRUE;
    }

    static BOOL CALLBACK inputEnumProcW (LPGUID lpGUID, LPCWSTR description, LPCWSTR, LPVOID object)
    {
        return ((DSoundAudioIODeviceType*) object)
                    ->inputEnumProc (lpGUID, String (description));
    }

    static BOOL CALLBACK inputEnumProcA (LPGUID lpGUID, LPCTSTR description, LPCTSTR, LPVOID object)
    {
        return ((DSoundAudioIODeviceType*) object)
                    ->inputEnumProc (lpGUID, String (description));
    }

    //==============================================================================
    DSoundAudioIODeviceType (const DSoundAudioIODeviceType&);
    const DSoundAudioIODeviceType& operator= (const DSoundAudioIODeviceType&);
};

//==============================================================================
AudioIODeviceType* juce_createDefaultAudioIODeviceType()
{
    return new DSoundAudioIODeviceType();
}

//==============================================================================
const String DSoundAudioIODevice::openDevice (const BitArray& inputChannels,
                                              const BitArray& outputChannels,
                                              double sampleRate_,
                                              int bufferSizeSamples_)
{
    closeDevice();
    totalSamplesOut = 0;

    sampleRate = sampleRate_;

    if (bufferSizeSamples_ <= 0)
        bufferSizeSamples_ = 960; // use as a default size if none is set.

    bufferSizeSamples = bufferSizeSamples_ & ~7;

    DSoundAudioIODeviceType dlh;
    dlh.scanForDevices();

    numInputBuffers = 2 * dlh.inputDeviceNames.size();
    inputBuffers = new float* [numInputBuffers + 2];

    numOutputBuffers = 2 * dlh.outputDeviceNames.size();
    outputBuffers = new float* [numOutputBuffers + 2];

    int i;
    for (i = 0; i < numInputBuffers + 2; ++i)
    {
        inputBuffers[i] = (inputChannels[i] && i < numInputBuffers)
                            ? (float*) juce_calloc ((bufferSizeSamples + 16) * sizeof (float))
                            : 0;
    }

    for (i = 0; i < numOutputBuffers + 2; ++i)
    {
        outputBuffers[i] = (outputChannels[i] && i < numOutputBuffers)
                            ? (float*) juce_calloc ((bufferSizeSamples + 16) * sizeof (float))
                            : 0;
    }

    for (i = 0; i < numInputBuffers; ++i)
    {
        if (inputChannels[i] || inputChannels[i + 1])
        {
            inChans.add (new DSoundInternalInChannel (dlh.inputDeviceNames [i / 2],
                                                      dlh.inputGuids [i / 2],
                                                      (int) sampleRate, bufferSizeSamples,
                                                      inputBuffers[i], inputBuffers[i + 1]));
        }
        else
        {
            inChans.add (0);
        }

        ++i;
    }

    for (i = 0; i < numOutputBuffers; ++i)
    {
        if (outputChannels[i] || outputChannels[i + 1])
        {
            outChans.add (new DSoundInternalOutChannel (dlh.outputDeviceNames[i / 2],
                                                        dlh.outputGuids [i / 2],
                                                        (int) sampleRate, bufferSizeSamples,
                                                        outputBuffers[i], outputBuffers[i + 1]));
        }
        else
        {
            outChans.add (0);
        }

        ++i;
    }

    String error;

    for (i = 0; i < numOutputBuffers; ++i)
    {
        if (outChans[i] != 0)
        {
            error = outChans[i]->open();

            if (error.isNotEmpty())
            {
                error = T("Error opening ") + dlh.outputDeviceNames[i]
                         + T(": \"") + error + T("\"");
                break;
            }
        }
    }

    if (error.isEmpty())
    {
        for (i = 0; i < numInputBuffers; ++i)
        {
            if (inChans[i] != 0)
            {
                error = inChans[i]->open();

                if (error.isNotEmpty())
                {
                    error = T("Error opening ") + dlh.inputDeviceNames[i]
                                + T(": \"") + error + T("\"");
                    break;
                }
            }
        }
    }

    if (error.isNotEmpty())
    {
        log (error);
        return error;
    }

    totalSamplesOut = 0;

    startThread (9);
    sleep (10);

    for (i = 0; i < numOutputBuffers; ++i)
        if (outChans[i] != 0)
            outChans[i]->synchronisePosition();

    for (i = 0; i < numInputBuffers; ++i)
        if (inChans[i] != 0)
            inChans[i]->synchronisePosition();

    notify();

    return String::empty;
}


END_JUCE_NAMESPACE

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

#ifndef JUCE_WASAPI_LOGGING
 #define JUCE_WASAPI_LOGGING 0
#endif

//==============================================================================
namespace WasapiClasses
{

void logFailure (HRESULT hr)
{
    (void) hr;
    jassert (hr != 0x800401f0); // If you hit this, it means you're trying to call from
                                // a thread which hasn't been initialised with CoInitialize().

   #if JUCE_WASAPI_LOGGING
    if (FAILED (hr))
    {
        const char* m = nullptr;

        switch (hr)
        {
            case E_POINTER:                                 m = "E_POINTER"; break;
            case E_INVALIDARG:                              m = "E_INVALIDARG"; break;
            case AUDCLNT_E_NOT_INITIALIZED:                 m = "AUDCLNT_E_NOT_INITIALIZED"; break;
            case AUDCLNT_E_ALREADY_INITIALIZED:             m = "AUDCLNT_E_ALREADY_INITIALIZED"; break;
            case AUDCLNT_E_WRONG_ENDPOINT_TYPE:             m = "AUDCLNT_E_WRONG_ENDPOINT_TYPE"; break;
            case AUDCLNT_E_DEVICE_INVALIDATED:              m = "AUDCLNT_E_DEVICE_INVALIDATED"; break;
            case AUDCLNT_E_NOT_STOPPED:                     m = "AUDCLNT_E_NOT_STOPPED"; break;
            case AUDCLNT_E_BUFFER_TOO_LARGE:                m = "AUDCLNT_E_BUFFER_TOO_LARGE"; break;
            case AUDCLNT_E_OUT_OF_ORDER:                    m = "AUDCLNT_E_OUT_OF_ORDER"; break;
            case AUDCLNT_E_UNSUPPORTED_FORMAT:              m = "AUDCLNT_E_UNSUPPORTED_FORMAT"; break;
            case AUDCLNT_E_INVALID_SIZE:                    m = "AUDCLNT_E_INVALID_SIZE"; break;
            case AUDCLNT_E_DEVICE_IN_USE:                   m = "AUDCLNT_E_DEVICE_IN_USE"; break;
            case AUDCLNT_E_BUFFER_OPERATION_PENDING:        m = "AUDCLNT_E_BUFFER_OPERATION_PENDING"; break;
            case AUDCLNT_E_THREAD_NOT_REGISTERED:           m = "AUDCLNT_E_THREAD_NOT_REGISTERED"; break;
            case AUDCLNT_E_EXCLUSIVE_MODE_NOT_ALLOWED:      m = "AUDCLNT_E_EXCLUSIVE_MODE_NOT_ALLOWED"; break;
            case AUDCLNT_E_ENDPOINT_CREATE_FAILED:          m = "AUDCLNT_E_ENDPOINT_CREATE_FAILED"; break;
            case AUDCLNT_E_SERVICE_NOT_RUNNING:             m = "AUDCLNT_E_SERVICE_NOT_RUNNING"; break;
            case AUDCLNT_E_EVENTHANDLE_NOT_EXPECTED:        m = "AUDCLNT_E_EVENTHANDLE_NOT_EXPECTED"; break;
            case AUDCLNT_E_EXCLUSIVE_MODE_ONLY:             m = "AUDCLNT_E_EXCLUSIVE_MODE_ONLY"; break;
            case AUDCLNT_E_BUFDURATION_PERIOD_NOT_EQUAL:    m = "AUDCLNT_E_BUFDURATION_PERIOD_NOT_EQUAL"; break;
            case AUDCLNT_E_EVENTHANDLE_NOT_SET:             m = "AUDCLNT_E_EVENTHANDLE_NOT_SET"; break;
            case AUDCLNT_E_INCORRECT_BUFFER_SIZE:           m = "AUDCLNT_E_INCORRECT_BUFFER_SIZE"; break;
            case AUDCLNT_E_BUFFER_SIZE_ERROR:               m = "AUDCLNT_E_BUFFER_SIZE_ERROR"; break;
            case AUDCLNT_S_BUFFER_EMPTY:                    m = "AUDCLNT_S_BUFFER_EMPTY"; break;
            case AUDCLNT_S_THREAD_ALREADY_REGISTERED:       m = "AUDCLNT_S_THREAD_ALREADY_REGISTERED"; break;
            default:                                        break;
        }

        Logger::writeToLog ("WASAPI error: " + (m != nullptr ? String (m)
                                                             : String::toHexString ((int) hr)));
    }
   #endif
}

#undef check

bool check (HRESULT hr)
{
    logFailure (hr);
    return SUCCEEDED (hr);
}

//==============================================================================
namespace
{
    enum EDataFlow
    {
        eRender = 0,
        eCapture = (eRender + 1),
        eAll = (eCapture + 1)
    };

    enum { DEVICE_STATE_ACTIVE = 1 };

    struct __declspec (uuid ("D666063F-1587-4E43-81F1-B948E807363F")) IMMDevice : public IUnknown
    {
        virtual HRESULT STDMETHODCALLTYPE Activate(REFIID, DWORD, PROPVARIANT*, void**) = 0;
        virtual HRESULT STDMETHODCALLTYPE OpenPropertyStore(DWORD, IPropertyStore**) = 0;
        virtual HRESULT STDMETHODCALLTYPE GetId(LPWSTR*) = 0;
        virtual HRESULT STDMETHODCALLTYPE GetState(DWORD*) = 0;
    };

    struct __declspec (uuid ("1BE09788-6894-4089-8586-9A2A6C265AC5")) IMMEndpoint : public IUnknown
    {
        virtual HRESULT STDMETHODCALLTYPE GetDataFlow(EDataFlow*) = 0;
    };

    struct IMMDeviceCollection : public IUnknown
    {
        virtual HRESULT STDMETHODCALLTYPE GetCount(UINT*) = 0;
        virtual HRESULT STDMETHODCALLTYPE Item(UINT, IMMDevice**) = 0;
    };

    enum ERole
    {
        eConsole = 0,
        eMultimedia = (eConsole + 1),
        eCommunications = (eMultimedia + 1)
    };

    struct IMMNotificationClient : public IUnknown
    {
        virtual HRESULT STDMETHODCALLTYPE OnDeviceStateChanged(LPCWSTR, DWORD) = 0;
        virtual HRESULT STDMETHODCALLTYPE OnDeviceAdded(LPCWSTR) = 0;
        virtual HRESULT STDMETHODCALLTYPE OnDeviceRemoved(LPCWSTR) = 0;
        virtual HRESULT STDMETHODCALLTYPE OnDefaultDeviceChanged(EDataFlow, ERole, LPCWSTR) = 0;
        virtual HRESULT STDMETHODCALLTYPE OnPropertyValueChanged(LPCWSTR, const PROPERTYKEY) = 0;
    };

    struct __declspec (uuid ("A95664D2-9614-4F35-A746-DE8DB63617E6")) IMMDeviceEnumerator : public IUnknown
    {
        virtual HRESULT STDMETHODCALLTYPE EnumAudioEndpoints(EDataFlow, DWORD, IMMDeviceCollection**) = 0;
        virtual HRESULT STDMETHODCALLTYPE GetDefaultAudioEndpoint(EDataFlow, ERole, IMMDevice**) = 0;
        virtual HRESULT STDMETHODCALLTYPE GetDevice(LPCWSTR, IMMDevice**) = 0;
        virtual HRESULT STDMETHODCALLTYPE RegisterEndpointNotificationCallback(IMMNotificationClient*) = 0;
        virtual HRESULT STDMETHODCALLTYPE UnregisterEndpointNotificationCallback(IMMNotificationClient*) = 0;
    };

    struct __declspec (uuid ("BCDE0395-E52F-467C-8E3D-C4579291692E")) MMDeviceEnumerator;
}

String getDeviceID (IMMDevice* const device)
{
    String s;
    WCHAR* deviceId = nullptr;

    if (check (device->GetId (&deviceId)))
    {
        s = String (deviceId);
        CoTaskMemFree (deviceId);
    }

    return s;
}

EDataFlow getDataFlow (const ComSmartPtr<IMMDevice>& device)
{
    EDataFlow flow = eRender;
    ComSmartPtr <IMMEndpoint> endPoint;
    if (check (device.QueryInterface (endPoint)))
        (void) check (endPoint->GetDataFlow (&flow));

    return flow;
}

int refTimeToSamples (const REFERENCE_TIME& t, const double sampleRate) noexcept
{
    return roundDoubleToInt (sampleRate * ((double) t) * 0.0000001);
}

void copyWavFormat (WAVEFORMATEXTENSIBLE& dest, const WAVEFORMATEX* const src) noexcept
{
    memcpy (&dest, src, src->wFormatTag == WAVE_FORMAT_EXTENSIBLE ? sizeof (WAVEFORMATEXTENSIBLE)
                                                                  : sizeof (WAVEFORMATEX));
}

//==============================================================================
class WASAPIDeviceBase
{
public:
    WASAPIDeviceBase (const ComSmartPtr <IMMDevice>& device_, const bool useExclusiveMode_)
        : device (device_),
          sampleRate (0),
          defaultSampleRate (0),
          numChannels (0),
          actualNumChannels (0),
          minBufferSize (0),
          defaultBufferSize (0),
          latencySamples (0),
          useExclusiveMode (useExclusiveMode_),
          sampleRateHasChanged (false)
    {
        clientEvent = CreateEvent (0, false, false, _T("JuceWASAPI"));

        ComSmartPtr <IAudioClient> tempClient (createClient());
        if (tempClient == nullptr)
            return;

        REFERENCE_TIME defaultPeriod, minPeriod;
        if (! check (tempClient->GetDevicePeriod (&defaultPeriod, &minPeriod)))
            return;

        WAVEFORMATEX* mixFormat = nullptr;
        if (! check (tempClient->GetMixFormat (&mixFormat)))
            return;

        WAVEFORMATEXTENSIBLE format;
        copyWavFormat (format, mixFormat);
        CoTaskMemFree (mixFormat);

        actualNumChannels = numChannels = format.Format.nChannels;
        defaultSampleRate = format.Format.nSamplesPerSec;
        minBufferSize = refTimeToSamples (minPeriod, defaultSampleRate);
        defaultBufferSize = refTimeToSamples (defaultPeriod, defaultSampleRate);

        rates.addUsingDefaultSort (defaultSampleRate);

        static const double ratesToTest[] = { 44100.0, 48000.0, 88200.0, 96000.0 };

        for (int i = 0; i < numElementsInArray (ratesToTest); ++i)
        {
            if (ratesToTest[i] == defaultSampleRate)
                continue;

            format.Format.nSamplesPerSec = (DWORD) roundDoubleToInt (ratesToTest[i]);

            if (SUCCEEDED (tempClient->IsFormatSupported (useExclusiveMode ? AUDCLNT_SHAREMODE_EXCLUSIVE : AUDCLNT_SHAREMODE_SHARED,
                                                          (WAVEFORMATEX*) &format, 0)))
                if (! rates.contains (ratesToTest[i]))
                    rates.addUsingDefaultSort (ratesToTest[i]);
        }
    }

    ~WASAPIDeviceBase()
    {
        device = nullptr;
        CloseHandle (clientEvent);
    }

    bool isOk() const noexcept   { return defaultBufferSize > 0 && defaultSampleRate > 0; }

    bool openClient (const double newSampleRate, const BigInteger& newChannels)
    {
        sampleRate = newSampleRate;
        channels = newChannels;
        channels.setRange (actualNumChannels, channels.getHighestBit() + 1 - actualNumChannels, false);
        numChannels = channels.getHighestBit() + 1;

        if (numChannels == 0)
            return true;

        client = createClient();

        if (client != nullptr
             && (tryInitialisingWithFormat (true, 4) || tryInitialisingWithFormat (false, 4)
                  || tryInitialisingWithFormat (false, 3) || tryInitialisingWithFormat (false, 2)))
        {
            sampleRateHasChanged = false;

            channelMaps.clear();
            for (int i = 0; i <= channels.getHighestBit(); ++i)
                if (channels[i])
                    channelMaps.add (i);

            REFERENCE_TIME latency;
            if (check (client->GetStreamLatency (&latency)))
                latencySamples = refTimeToSamples (latency, sampleRate);

            (void) check (client->GetBufferSize (&actualBufferSize));

            createSessionEventCallback();

            return check (client->SetEventHandle (clientEvent));
        }

        return false;
    }

    void closeClient()
    {
        if (client != nullptr)
            client->Stop();

        deleteSessionEventCallback();
        client = nullptr;
        ResetEvent (clientEvent);
    }

    void deviceSampleRateChanged()
    {
        sampleRateHasChanged = true;
    }

    //==============================================================================
    ComSmartPtr <IMMDevice> device;
    ComSmartPtr <IAudioClient> client;
    double sampleRate, defaultSampleRate;
    int numChannels, actualNumChannels;
    int minBufferSize, defaultBufferSize, latencySamples;
    const bool useExclusiveMode;
    Array <double> rates;
    HANDLE clientEvent;
    BigInteger channels;
    Array <int> channelMaps;
    UINT32 actualBufferSize;
    int bytesPerSample;
    bool sampleRateHasChanged;

    virtual void updateFormat (bool isFloat) = 0;

private:
    //==============================================================================
    class SessionEventCallback  : public ComBaseClassHelper <IAudioSessionEvents>
    {
    public:
        SessionEventCallback (WASAPIDeviceBase& owner_) : owner (owner_) {}

        JUCE_COMRESULT OnDisplayNameChanged (LPCWSTR, LPCGUID)                 { return S_OK; }
        JUCE_COMRESULT OnIconPathChanged (LPCWSTR, LPCGUID)                    { return S_OK; }
        JUCE_COMRESULT OnSimpleVolumeChanged (float, BOOL, LPCGUID)            { return S_OK; }
        JUCE_COMRESULT OnChannelVolumeChanged (DWORD, float*, DWORD, LPCGUID)  { return S_OK; }
        JUCE_COMRESULT OnGroupingParamChanged (LPCGUID, LPCGUID)               { return S_OK; }
        JUCE_COMRESULT OnStateChanged (AudioSessionState)                      { return S_OK; }

        JUCE_COMRESULT OnSessionDisconnected (AudioSessionDisconnectReason reason)
        {
            if (reason == DisconnectReasonFormatChanged)
                owner.deviceSampleRateChanged();

            return S_OK;
        }

    private:
        WASAPIDeviceBase& owner;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SessionEventCallback)
    };

    ComSmartPtr <IAudioSessionControl> audioSessionControl;
    ComSmartPtr <SessionEventCallback> sessionEventCallback;

    void createSessionEventCallback()
    {
        deleteSessionEventCallback();
        client->GetService (__uuidof (IAudioSessionControl),
                            (void**) audioSessionControl.resetAndGetPointerAddress());

        if (audioSessionControl != nullptr)
        {
            sessionEventCallback = new SessionEventCallback (*this);
            audioSessionControl->RegisterAudioSessionNotification (sessionEventCallback);
            sessionEventCallback->Release(); // (required because ComBaseClassHelper objects are constructed with a ref count of 1)
        }
    }

    void deleteSessionEventCallback()
    {
        if (audioSessionControl != nullptr && sessionEventCallback != nullptr)
            audioSessionControl->UnregisterAudioSessionNotification (sessionEventCallback);

        audioSessionControl = nullptr;
        sessionEventCallback = nullptr;
    }

    //==============================================================================
    const ComSmartPtr <IAudioClient> createClient()
    {
        ComSmartPtr <IAudioClient> client;

        if (device != nullptr)
        {
            HRESULT hr = device->Activate (__uuidof (IAudioClient), CLSCTX_INPROC_SERVER, 0, (void**) client.resetAndGetPointerAddress());
            logFailure (hr);
        }

        return client;
    }

    bool tryInitialisingWithFormat (const bool useFloat, const int bytesPerSampleToTry)
    {
        WAVEFORMATEXTENSIBLE format = { 0 };

        if (numChannels <= 2 && bytesPerSampleToTry <= 2)
        {
            format.Format.wFormatTag = WAVE_FORMAT_PCM;
        }
        else
        {
            format.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
            format.Format.cbSize = sizeof (WAVEFORMATEXTENSIBLE) - sizeof (WAVEFORMATEX);
        }

        format.Format.nSamplesPerSec = (DWORD) roundDoubleToInt (sampleRate);
        format.Format.nChannels = (WORD) numChannels;
        format.Format.wBitsPerSample = (WORD) (8 * bytesPerSampleToTry);
        format.Format.nAvgBytesPerSec = (DWORD) (format.Format.nSamplesPerSec * numChannels * bytesPerSampleToTry);
        format.Format.nBlockAlign = (WORD) (numChannels * bytesPerSampleToTry);
        format.SubFormat = useFloat ? KSDATAFORMAT_SUBTYPE_IEEE_FLOAT : KSDATAFORMAT_SUBTYPE_PCM;
        format.Samples.wValidBitsPerSample = format.Format.wBitsPerSample;

        switch (numChannels)
        {
            case 1:     format.dwChannelMask = SPEAKER_FRONT_CENTER; break;
            case 2:     format.dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT; break;
            case 4:     format.dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_BACK_LEFT | SPEAKER_BACK_RIGHT; break;
            case 6:     format.dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_FRONT_CENTER | SPEAKER_LOW_FREQUENCY | SPEAKER_BACK_LEFT | SPEAKER_BACK_RIGHT; break;
            case 8:     format.dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_FRONT_CENTER | SPEAKER_LOW_FREQUENCY | SPEAKER_BACK_LEFT | SPEAKER_BACK_RIGHT | SPEAKER_FRONT_LEFT_OF_CENTER | SPEAKER_FRONT_RIGHT_OF_CENTER; break;
            default:    break;
        }

        WAVEFORMATEXTENSIBLE* nearestFormat = nullptr;

        HRESULT hr = client->IsFormatSupported (useExclusiveMode ? AUDCLNT_SHAREMODE_EXCLUSIVE : AUDCLNT_SHAREMODE_SHARED,
                                                (WAVEFORMATEX*) &format, useExclusiveMode ? nullptr : (WAVEFORMATEX**) &nearestFormat);
        logFailure (hr);

        if (hr == S_FALSE && format.Format.nSamplesPerSec == nearestFormat->Format.nSamplesPerSec)
        {
            copyWavFormat (format, (WAVEFORMATEX*) nearestFormat);
            hr = S_OK;
        }

        CoTaskMemFree (nearestFormat);

        REFERENCE_TIME defaultPeriod = 0, minPeriod = 0;
        if (useExclusiveMode)
            check (client->GetDevicePeriod (&defaultPeriod, &minPeriod));

        GUID session;
        if (hr == S_OK
             && check (client->Initialize (useExclusiveMode ? AUDCLNT_SHAREMODE_EXCLUSIVE : AUDCLNT_SHAREMODE_SHARED,
                                           AUDCLNT_STREAMFLAGS_EVENTCALLBACK,
                                           defaultPeriod, defaultPeriod, (WAVEFORMATEX*) &format, &session)))
        {
            actualNumChannels = format.Format.nChannels;
            const bool isFloat = format.Format.wFormatTag == WAVE_FORMAT_EXTENSIBLE && format.SubFormat == KSDATAFORMAT_SUBTYPE_IEEE_FLOAT;
            bytesPerSample = format.Format.wBitsPerSample / 8;

            updateFormat (isFloat);
            return true;
        }

        return false;
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WASAPIDeviceBase)
};

//==============================================================================
class WASAPIInputDevice  : public WASAPIDeviceBase
{
public:
    WASAPIInputDevice (const ComSmartPtr <IMMDevice>& device_, const bool useExclusiveMode_)
        : WASAPIDeviceBase (device_, useExclusiveMode_),
          reservoir (1, 1)
    {
    }

    ~WASAPIInputDevice()
    {
        close();
    }

    bool open (const double newSampleRate, const BigInteger& newChannels)
    {
        reservoirSize = 0;
        reservoirCapacity = 16384;
        reservoir.setSize (actualNumChannels * reservoirCapacity * sizeof (float));
        return openClient (newSampleRate, newChannels)
                && (numChannels == 0 || check (client->GetService (__uuidof (IAudioCaptureClient),
                                                                   (void**) captureClient.resetAndGetPointerAddress())));
    }

    void close()
    {
        closeClient();
        captureClient = nullptr;
        reservoir.setSize (0);
    }

    template <class SourceType>
    void updateFormatWithType (SourceType*)
    {
        typedef AudioData::Pointer <AudioData::Float32, AudioData::NativeEndian, AudioData::NonInterleaved, AudioData::NonConst> NativeType;
        converter = new AudioData::ConverterInstance <AudioData::Pointer <SourceType, AudioData::LittleEndian, AudioData::Interleaved, AudioData::Const>, NativeType> (actualNumChannels, 1);
    }

    void updateFormat (bool isFloat)
    {
        if (isFloat)                    updateFormatWithType ((AudioData::Float32*) 0);
        else if (bytesPerSample == 4)   updateFormatWithType ((AudioData::Int32*) 0);
        else if (bytesPerSample == 3)   updateFormatWithType ((AudioData::Int24*) 0);
        else                            updateFormatWithType ((AudioData::Int16*) 0);
    }

    void copyBuffers (float** destBuffers, int numDestBuffers, int bufferSize, Thread& thread)
    {
        if (numChannels <= 0)
            return;

        int offset = 0;

        while (bufferSize > 0)
        {
            if (reservoirSize > 0)  // There's stuff in the reservoir, so use that...
            {
                const int samplesToDo = jmin (bufferSize, (int) reservoirSize);

                for (int i = 0; i < numDestBuffers; ++i)
                    converter->convertSamples (destBuffers[i] + offset, 0, reservoir.getData(), channelMaps.getUnchecked(i), samplesToDo);

                bufferSize -= samplesToDo;
                offset += samplesToDo;
                reservoirSize = 0;
            }
            else
            {
                UINT32 packetLength = 0;
                if (! check (captureClient->GetNextPacketSize (&packetLength)))
                    break;

                if (packetLength == 0)
                {
                    if (thread.threadShouldExit()
                         || WaitForSingleObject (clientEvent, 1000) == WAIT_TIMEOUT)
                        break;

                    continue;
                }

                uint8* inputData;
                UINT32 numSamplesAvailable;
                DWORD flags;

                if (check (captureClient->GetBuffer (&inputData, &numSamplesAvailable, &flags, 0, 0)))
                {
                    const int samplesToDo = jmin (bufferSize, (int) numSamplesAvailable);

                    for (int i = 0; i < numDestBuffers; ++i)
                        converter->convertSamples (destBuffers[i] + offset, 0, inputData, channelMaps.getUnchecked(i), samplesToDo);

                    bufferSize -= samplesToDo;
                    offset += samplesToDo;

                    if (samplesToDo < (int) numSamplesAvailable)
                    {
                        reservoirSize = jmin ((int) (numSamplesAvailable - samplesToDo), reservoirCapacity);
                        memcpy ((uint8*) reservoir.getData(), inputData + bytesPerSample * actualNumChannels * samplesToDo,
                                (size_t) (bytesPerSample * actualNumChannels * reservoirSize));
                    }

                    captureClient->ReleaseBuffer (numSamplesAvailable);
                }
            }
        }
    }

    ComSmartPtr <IAudioCaptureClient> captureClient;
    MemoryBlock reservoir;
    int reservoirSize, reservoirCapacity;
    ScopedPointer <AudioData::Converter> converter;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WASAPIInputDevice)
};

//==============================================================================
class WASAPIOutputDevice  : public WASAPIDeviceBase
{
public:
    WASAPIOutputDevice (const ComSmartPtr <IMMDevice>& device_, const bool useExclusiveMode_)
        : WASAPIDeviceBase (device_, useExclusiveMode_)
    {
    }

    ~WASAPIOutputDevice()
    {
        close();
    }

    bool open (const double newSampleRate, const BigInteger& newChannels)
    {
        return openClient (newSampleRate, newChannels)
            && (numChannels == 0 || check (client->GetService (__uuidof (IAudioRenderClient), (void**) renderClient.resetAndGetPointerAddress())));
    }

    void close()
    {
        closeClient();
        renderClient = nullptr;
    }

    template <class DestType>
    void updateFormatWithType (DestType*)
    {
        typedef AudioData::Pointer <AudioData::Float32, AudioData::NativeEndian, AudioData::NonInterleaved, AudioData::Const> NativeType;
        converter = new AudioData::ConverterInstance <NativeType, AudioData::Pointer <DestType, AudioData::LittleEndian, AudioData::Interleaved, AudioData::NonConst> > (1, actualNumChannels);
    }

    void updateFormat (bool isFloat)
    {
        if (isFloat)                    updateFormatWithType ((AudioData::Float32*) 0);
        else if (bytesPerSample == 4)   updateFormatWithType ((AudioData::Int32*) 0);
        else if (bytesPerSample == 3)   updateFormatWithType ((AudioData::Int24*) 0);
        else                            updateFormatWithType ((AudioData::Int16*) 0);
    }

    void copyBuffers (const float** const srcBuffers, const int numSrcBuffers, int bufferSize, Thread& thread)
    {
        if (numChannels <= 0)
            return;

        int offset = 0;

        while (bufferSize > 0)
        {
            UINT32 padding = 0;
            if (! check (client->GetCurrentPadding (&padding)))
                return;

            int samplesToDo = useExclusiveMode ? bufferSize
                                               : jmin ((int) (actualBufferSize - padding), bufferSize);

            if (samplesToDo <= 0)
            {
                if (thread.threadShouldExit()
                     || WaitForSingleObject (clientEvent, 1000) == WAIT_TIMEOUT)
                    break;

                continue;
            }

            uint8* outputData = nullptr;
            if (check (renderClient->GetBuffer ((UINT32) samplesToDo, &outputData)))
            {
                for (int i = 0; i < numSrcBuffers; ++i)
                    converter->convertSamples (outputData, channelMaps.getUnchecked(i), srcBuffers[i] + offset, 0, samplesToDo);

                renderClient->ReleaseBuffer ((UINT32) samplesToDo, 0);

                offset += samplesToDo;
                bufferSize -= samplesToDo;
            }
        }
    }

    ComSmartPtr <IAudioRenderClient> renderClient;
    ScopedPointer <AudioData::Converter> converter;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WASAPIOutputDevice)
};

//==============================================================================
class WASAPIAudioIODevice  : public AudioIODevice,
                             public Thread
{
public:
    WASAPIAudioIODevice (const String& deviceName,
                         const String& outputDeviceId_,
                         const String& inputDeviceId_,
                         const bool useExclusiveMode_)
        : AudioIODevice (deviceName, "Windows Audio"),
          Thread ("Juce WASAPI"),
          outputDeviceId (outputDeviceId_),
          inputDeviceId (inputDeviceId_),
          useExclusiveMode (useExclusiveMode_),
          isOpen_ (false),
          isStarted (false),
          currentBufferSizeSamples (0),
          currentSampleRate (0),
          callback (nullptr)
    {
    }

    ~WASAPIAudioIODevice()
    {
        close();
    }

    bool initialise()
    {
        latencyIn = latencyOut = 0;
        Array <double> ratesIn, ratesOut;

        if (createDevices())
        {
            jassert (inputDevice != nullptr || outputDevice != nullptr);

            if (inputDevice != nullptr && outputDevice != nullptr)
            {
                defaultSampleRate = jmin (inputDevice->defaultSampleRate, outputDevice->defaultSampleRate);
                minBufferSize = jmin (inputDevice->minBufferSize, outputDevice->minBufferSize);
                defaultBufferSize = jmax (inputDevice->defaultBufferSize, outputDevice->defaultBufferSize);
                sampleRates = inputDevice->rates;
                sampleRates.removeValuesNotIn (outputDevice->rates);
            }
            else
            {
                WASAPIDeviceBase* d = inputDevice != nullptr ? static_cast<WASAPIDeviceBase*> (inputDevice)
                                                             : static_cast<WASAPIDeviceBase*> (outputDevice);
                defaultSampleRate = d->defaultSampleRate;
                minBufferSize = d->minBufferSize;
                defaultBufferSize = d->defaultBufferSize;
                sampleRates = d->rates;
            }

            bufferSizes.addUsingDefaultSort (defaultBufferSize);
            if (minBufferSize != defaultBufferSize)
                bufferSizes.addUsingDefaultSort (minBufferSize);

            int n = 64;
            for (int i = 0; i < 40; ++i)
            {
                if (n >= minBufferSize && n <= 2048 && ! bufferSizes.contains (n))
                    bufferSizes.addUsingDefaultSort (n);

                n += (n < 512) ? 32 : (n < 1024 ? 64 : 128);
            }

            return true;
        }

        return false;
    }

    StringArray getOutputChannelNames()
    {
        StringArray outChannels;

        if (outputDevice != nullptr)
            for (int i = 1; i <= outputDevice->actualNumChannels; ++i)
                outChannels.add ("Output channel " + String (i));

        return outChannels;
    }

    StringArray getInputChannelNames()
    {
        StringArray inChannels;

        if (inputDevice != nullptr)
            for (int i = 1; i <= inputDevice->actualNumChannels; ++i)
                inChannels.add ("Input channel " + String (i));

        return inChannels;
    }

    int getNumSampleRates()                             { return sampleRates.size(); }
    double getSampleRate (int index)                    { return sampleRates [index]; }
    int getNumBufferSizesAvailable()                    { return bufferSizes.size(); }
    int getBufferSizeSamples (int index)                { return bufferSizes [index]; }
    int getDefaultBufferSize()                          { return defaultBufferSize; }

    int getCurrentBufferSizeSamples()                   { return currentBufferSizeSamples; }
    double getCurrentSampleRate()                       { return currentSampleRate; }
    int getCurrentBitDepth()                            { return 32; }
    int getOutputLatencyInSamples()                     { return latencyOut; }
    int getInputLatencyInSamples()                      { return latencyIn; }
    BigInteger getActiveOutputChannels() const          { return outputDevice != nullptr ? outputDevice->channels : BigInteger(); }
    BigInteger getActiveInputChannels() const           { return inputDevice != nullptr ? inputDevice->channels : BigInteger(); }
    String getLastError()                               { return lastError; }


    String open (const BigInteger& inputChannels, const BigInteger& outputChannels,
                 double sampleRate, int bufferSizeSamples)
    {
        close();
        lastError = String::empty;

        if (sampleRates.size() == 0 && inputDevice != nullptr && outputDevice != nullptr)
        {
            lastError = "The input and output devices don't share a common sample rate!";
            return lastError;
        }

        currentBufferSizeSamples = bufferSizeSamples <= 0 ? defaultBufferSize : jmax (bufferSizeSamples, minBufferSize);
        currentSampleRate = sampleRate > 0 ? sampleRate : defaultSampleRate;

        if (inputDevice != nullptr && ! inputDevice->open (currentSampleRate, inputChannels))
        {
            lastError = "Couldn't open the input device!";
            return lastError;
        }

        if (outputDevice != nullptr && ! outputDevice->open (currentSampleRate, outputChannels))
        {
            close();
            lastError = "Couldn't open the output device!";
            return lastError;
        }

        if (inputDevice != nullptr)   ResetEvent (inputDevice->clientEvent);
        if (outputDevice != nullptr)  ResetEvent (outputDevice->clientEvent);

        startThread (8);
        Thread::sleep (5);

        if (inputDevice != nullptr && inputDevice->client != nullptr)
        {
            latencyIn = (int) (inputDevice->latencySamples + currentBufferSizeSamples);

            if (! check (inputDevice->client->Start()))
            {
                close();
                lastError = "Couldn't start the input device!";
                return lastError;
            }
        }

        if (outputDevice != nullptr && outputDevice->client != nullptr)
        {
            latencyOut = (int) (outputDevice->latencySamples + currentBufferSizeSamples);

            if (! check (outputDevice->client->Start()))
            {
                close();
                lastError = "Couldn't start the output device!";
                return lastError;
            }
        }

        isOpen_ = true;
        return lastError;
    }

    void close()
    {
        stop();
        signalThreadShouldExit();

        if (inputDevice != nullptr)   SetEvent (inputDevice->clientEvent);
        if (outputDevice != nullptr)  SetEvent (outputDevice->clientEvent);

        stopThread (5000);

        if (inputDevice != nullptr)   inputDevice->close();
        if (outputDevice != nullptr)  outputDevice->close();

        isOpen_ = false;
    }

    bool isOpen()       { return isOpen_ && isThreadRunning(); }
    bool isPlaying()    { return isStarted && isOpen_ && isThreadRunning(); }

    void start (AudioIODeviceCallback* call)
    {
        if (isOpen_ && call != nullptr && ! isStarted)
        {
            if (! isThreadRunning())
            {
                // something's gone wrong and the thread's stopped..
                isOpen_ = false;
                return;
            }

            call->audioDeviceAboutToStart (this);

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

            if (callbackLocal != nullptr)
                callbackLocal->audioDeviceStopped();
        }
    }

    void setMMThreadPriority()
    {
        DynamicLibrary dll ("avrt.dll");
        JUCE_LOAD_WINAPI_FUNCTION (dll, AvSetMmThreadCharacteristicsW, avSetMmThreadCharacteristics, HANDLE, (LPCWSTR, LPDWORD))
        JUCE_LOAD_WINAPI_FUNCTION (dll, AvSetMmThreadPriority, avSetMmThreadPriority, HANDLE, (HANDLE, AVRT_PRIORITY))

        if (avSetMmThreadCharacteristics != 0 && avSetMmThreadPriority != 0)
        {
            DWORD dummy = 0;
            HANDLE h = avSetMmThreadCharacteristics (L"Pro Audio", &dummy);

            if (h != 0)
                avSetMmThreadPriority (h, AVRT_PRIORITY_NORMAL);
        }
    }

    void run()
    {
        setMMThreadPriority();

        const int bufferSize = currentBufferSizeSamples;
        const int numInputBuffers = getActiveInputChannels().countNumberOfSetBits();
        const int numOutputBuffers = getActiveOutputChannels().countNumberOfSetBits();
        bool sampleRateChanged = false;

        AudioSampleBuffer ins (jmax (1, numInputBuffers), bufferSize + 32);
        AudioSampleBuffer outs (jmax (1, numOutputBuffers), bufferSize + 32);
        float** const inputBuffers = ins.getArrayOfChannels();
        float** const outputBuffers = outs.getArrayOfChannels();
        ins.clear();

        while (! threadShouldExit())
        {
            if (inputDevice != nullptr)
            {
                inputDevice->copyBuffers (inputBuffers, numInputBuffers, bufferSize, *this);

                if (threadShouldExit())
                    break;

                if (inputDevice->sampleRateHasChanged)
                    sampleRateChanged = true;
            }

            {
                const ScopedLock sl (startStopLock);

                if (isStarted)
                    callback->audioDeviceIOCallback (const_cast <const float**> (inputBuffers), numInputBuffers,
                                                     outputBuffers, numOutputBuffers, bufferSize);
                else
                    outs.clear();
            }

            if (outputDevice != nullptr)
            {
                outputDevice->copyBuffers (const_cast <const float**> (outputBuffers), numOutputBuffers, bufferSize, *this);

                if (outputDevice->sampleRateHasChanged)
                    sampleRateChanged = true;
            }

            if (sampleRateChanged)
            {
                // xxx one of the devices has had its sample rate changed externally.. not 100% sure how
                // to handle this..
            }
        }
    }

    //==============================================================================
    String outputDeviceId, inputDeviceId;
    String lastError;

private:
    // Device stats...
    ScopedPointer<WASAPIInputDevice> inputDevice;
    ScopedPointer<WASAPIOutputDevice> outputDevice;
    const bool useExclusiveMode;
    double defaultSampleRate;
    int minBufferSize, defaultBufferSize;
    int latencyIn, latencyOut;
    Array <double> sampleRates;
    Array <int> bufferSizes;

    // Active state...
    bool isOpen_, isStarted;
    int currentBufferSizeSamples;
    double currentSampleRate;

    AudioIODeviceCallback* callback;
    CriticalSection startStopLock;

    //==============================================================================
    bool createDevices()
    {
        ComSmartPtr <IMMDeviceEnumerator> enumerator;
        if (! check (enumerator.CoCreateInstance (__uuidof (MMDeviceEnumerator))))
            return false;

        ComSmartPtr <IMMDeviceCollection> deviceCollection;
        if (! check (enumerator->EnumAudioEndpoints (eAll, DEVICE_STATE_ACTIVE, deviceCollection.resetAndGetPointerAddress())))
            return false;

        UINT32 numDevices = 0;
        if (! check (deviceCollection->GetCount (&numDevices)))
            return false;

        for (UINT32 i = 0; i < numDevices; ++i)
        {
            ComSmartPtr <IMMDevice> device;
            if (! check (deviceCollection->Item (i, device.resetAndGetPointerAddress())))
                continue;

            const String deviceId (getDeviceID (device));
            if (deviceId.isEmpty())
                continue;

            const EDataFlow flow = getDataFlow (device);

            if (deviceId == inputDeviceId && flow == eCapture)
                inputDevice = new WASAPIInputDevice (device, useExclusiveMode);
            else if (deviceId == outputDeviceId && flow == eRender)
                outputDevice = new WASAPIOutputDevice (device, useExclusiveMode);
        }

        return (outputDeviceId.isEmpty() || (outputDevice != nullptr && outputDevice->isOk()))
            && (inputDeviceId.isEmpty() || (inputDevice != nullptr && inputDevice->isOk()));
    }

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WASAPIAudioIODevice)
};


//==============================================================================
class WASAPIAudioIODeviceType  : public AudioIODeviceType,
                                 private DeviceChangeDetector
{
public:
    WASAPIAudioIODeviceType()
        : AudioIODeviceType ("Windows Audio"),
          DeviceChangeDetector (L"Windows Audio"),
          hasScanned (false)
    {
    }

    //==============================================================================
    void scanForDevices()
    {
        hasScanned = true;

        outputDeviceNames.clear();
        inputDeviceNames.clear();
        outputDeviceIds.clear();
        inputDeviceIds.clear();

        scan (outputDeviceNames, inputDeviceNames,
              outputDeviceIds, inputDeviceIds);
    }

    StringArray getDeviceNames (bool wantInputNames) const
    {
        jassert (hasScanned); // need to call scanForDevices() before doing this

        return wantInputNames ? inputDeviceNames
                              : outputDeviceNames;
    }

    int getDefaultDeviceIndex (bool /*forInput*/) const
    {
        jassert (hasScanned); // need to call scanForDevices() before doing this
        return 0;
    }

    int getIndexOfDevice (AudioIODevice* device, bool asInput) const
    {
        jassert (hasScanned); // need to call scanForDevices() before doing this
        WASAPIAudioIODevice* const d = dynamic_cast <WASAPIAudioIODevice*> (device);
        return d == nullptr ? -1 : (asInput ? inputDeviceIds.indexOf (d->inputDeviceId)
                                            : outputDeviceIds.indexOf (d->outputDeviceId));
    }

    bool hasSeparateInputsAndOutputs() const    { return true; }

    AudioIODevice* createDevice (const String& outputDeviceName,
                                 const String& inputDeviceName)
    {
        jassert (hasScanned); // need to call scanForDevices() before doing this

        const bool useExclusiveMode = false;
        ScopedPointer<WASAPIAudioIODevice> device;

        const int outputIndex = outputDeviceNames.indexOf (outputDeviceName);
        const int inputIndex = inputDeviceNames.indexOf (inputDeviceName);

        if (outputIndex >= 0 || inputIndex >= 0)
        {
            device = new WASAPIAudioIODevice (outputDeviceName.isNotEmpty() ? outputDeviceName
                                                                            : inputDeviceName,
                                              outputDeviceIds [outputIndex],
                                              inputDeviceIds [inputIndex],
                                              useExclusiveMode);

            if (! device->initialise())
                device = nullptr;
        }

        return device.release();
    }

    //==============================================================================
    StringArray outputDeviceNames, outputDeviceIds;
    StringArray inputDeviceNames, inputDeviceIds;

private:
    bool hasScanned;

    //==============================================================================
    static String getDefaultEndpoint (IMMDeviceEnumerator* const enumerator, const bool forCapture)
    {
        String s;
        IMMDevice* dev = nullptr;
        if (check (enumerator->GetDefaultAudioEndpoint (forCapture ? eCapture : eRender,
                                                        eMultimedia, &dev)))
        {
            WCHAR* deviceId = nullptr;
            if (check (dev->GetId (&deviceId)))
            {
                s = deviceId;
                CoTaskMemFree (deviceId);
            }

            dev->Release();
        }

        return s;
    }

    //==============================================================================
    void scan (StringArray& outputDeviceNames,
               StringArray& inputDeviceNames,
               StringArray& outputDeviceIds,
               StringArray& inputDeviceIds)
    {
        ComSmartPtr <IMMDeviceEnumerator> enumerator;
        if (! check (enumerator.CoCreateInstance (__uuidof (MMDeviceEnumerator))))
            return;

        const String defaultRenderer (getDefaultEndpoint (enumerator, false));
        const String defaultCapture (getDefaultEndpoint (enumerator, true));

        ComSmartPtr <IMMDeviceCollection> deviceCollection;
        UINT32 numDevices = 0;

        if (! (check (enumerator->EnumAudioEndpoints (eAll, DEVICE_STATE_ACTIVE, deviceCollection.resetAndGetPointerAddress()))
                && check (deviceCollection->GetCount (&numDevices))))
            return;

        for (UINT32 i = 0; i < numDevices; ++i)
        {
            ComSmartPtr <IMMDevice> device;
            if (! check (deviceCollection->Item (i, device.resetAndGetPointerAddress())))
                continue;

            DWORD state = 0;
            if (! (check (device->GetState (&state)) && state == DEVICE_STATE_ACTIVE))
                continue;

            const String deviceId (getDeviceID (device));
            String name;

            {
                ComSmartPtr <IPropertyStore> properties;
                if (! check (device->OpenPropertyStore (STGM_READ, properties.resetAndGetPointerAddress())))
                    continue;

                PROPVARIANT value;
                PropVariantInit (&value);
                if (check (properties->GetValue (PKEY_Device_FriendlyName, &value)))
                    name = value.pwszVal;

                PropVariantClear (&value);
            }

            const EDataFlow flow = getDataFlow (device);

            if (flow == eRender)
            {
                const int index = (deviceId == defaultRenderer) ? 0 : -1;
                outputDeviceIds.insert (index, deviceId);
                outputDeviceNames.insert (index, name);
            }
            else if (flow == eCapture)
            {
                const int index = (deviceId == defaultCapture) ? 0 : -1;
                inputDeviceIds.insert (index, deviceId);
                inputDeviceNames.insert (index, name);
            }
        }

        inputDeviceNames.appendNumbersToDuplicates (false, false);
        outputDeviceNames.appendNumbersToDuplicates (false, false);
    }

    //==============================================================================
    void systemDeviceChanged()
    {
        StringArray newOutNames, newInNames, newOutIds, newInIds;
        scan (newOutNames, newInNames, newOutIds, newInIds);

        if (newOutNames != outputDeviceNames
             || newInNames != inputDeviceNames
             || newOutIds != outputDeviceIds
             || newInIds != inputDeviceIds)
        {
            hasScanned = true;
            outputDeviceNames = newOutNames;
            inputDeviceNames = newInNames;
            outputDeviceIds = newOutIds;
            inputDeviceIds = newInIds;

            callDeviceChangeListeners();
        }
    }

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WASAPIAudioIODeviceType)
};

}

//==============================================================================
AudioIODeviceType* AudioIODeviceType::createAudioIODeviceType_WASAPI()
{
    if (SystemStats::getOperatingSystemType() >= SystemStats::WinVista)
        return new WasapiClasses::WASAPIAudioIODeviceType();

    return nullptr;
}

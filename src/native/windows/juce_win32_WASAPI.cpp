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

// (This file gets included by juce_win32_NativeCode.cpp, rather than being
// compiled on its own).
#if JUCE_INCLUDED_FILE && JUCE_WASAPI

//==============================================================================
#if 1

const String getAudioErrorDesc (HRESULT hr)
{
    const char* e = 0;

    switch (hr)
    {
        case E_POINTER:                                 e = "E_POINTER"; break;
        case E_INVALIDARG:                              e = "E_INVALIDARG"; break;
        case AUDCLNT_E_NOT_INITIALIZED:                 e = "AUDCLNT_E_NOT_INITIALIZED"; break;
        case AUDCLNT_E_ALREADY_INITIALIZED:             e = "AUDCLNT_E_ALREADY_INITIALIZED"; break;
        case AUDCLNT_E_WRONG_ENDPOINT_TYPE:             e = "AUDCLNT_E_WRONG_ENDPOINT_TYPE"; break;
        case AUDCLNT_E_DEVICE_INVALIDATED:              e = "AUDCLNT_E_DEVICE_INVALIDATED"; break;
        case AUDCLNT_E_NOT_STOPPED:                     e = "AUDCLNT_E_NOT_STOPPED"; break;
        case AUDCLNT_E_BUFFER_TOO_LARGE:                e = "AUDCLNT_E_BUFFER_TOO_LARGE"; break;
        case AUDCLNT_E_OUT_OF_ORDER:                    e = "AUDCLNT_E_OUT_OF_ORDER"; break;
        case AUDCLNT_E_UNSUPPORTED_FORMAT:              e = "AUDCLNT_E_UNSUPPORTED_FORMAT"; break;
        case AUDCLNT_E_INVALID_SIZE:                    e = "AUDCLNT_E_INVALID_SIZE"; break;
        case AUDCLNT_E_DEVICE_IN_USE:                   e = "AUDCLNT_E_DEVICE_IN_USE"; break;
        case AUDCLNT_E_BUFFER_OPERATION_PENDING:        e = "AUDCLNT_E_BUFFER_OPERATION_PENDING"; break;
        case AUDCLNT_E_THREAD_NOT_REGISTERED:           e = "AUDCLNT_E_THREAD_NOT_REGISTERED"; break;
        case AUDCLNT_E_EXCLUSIVE_MODE_NOT_ALLOWED:      e = "AUDCLNT_E_EXCLUSIVE_MODE_NOT_ALLOWED"; break;
        case AUDCLNT_E_ENDPOINT_CREATE_FAILED:          e = "AUDCLNT_E_ENDPOINT_CREATE_FAILED"; break;
        case AUDCLNT_E_SERVICE_NOT_RUNNING:             e = "AUDCLNT_E_SERVICE_NOT_RUNNING"; break;
        case AUDCLNT_E_EVENTHANDLE_NOT_EXPECTED:        e = "AUDCLNT_E_EVENTHANDLE_NOT_EXPECTED"; break;
        case AUDCLNT_E_EXCLUSIVE_MODE_ONLY:             e = "AUDCLNT_E_EXCLUSIVE_MODE_ONLY"; break;
        case AUDCLNT_E_BUFDURATION_PERIOD_NOT_EQUAL:    e = "AUDCLNT_E_BUFDURATION_PERIOD_NOT_EQUAL"; break;
        case AUDCLNT_E_EVENTHANDLE_NOT_SET:             e = "AUDCLNT_E_EVENTHANDLE_NOT_SET"; break;
        case AUDCLNT_E_INCORRECT_BUFFER_SIZE:           e = "AUDCLNT_E_INCORRECT_BUFFER_SIZE"; break;
        case AUDCLNT_E_BUFFER_SIZE_ERROR:               e = "AUDCLNT_E_BUFFER_SIZE_ERROR"; break;
        case AUDCLNT_S_BUFFER_EMPTY:                    e = "AUDCLNT_S_BUFFER_EMPTY"; break;
        case AUDCLNT_S_THREAD_ALREADY_REGISTERED:       e = "AUDCLNT_S_THREAD_ALREADY_REGISTERED"; break;
        default:                                        return String::toHexString ((int) hr);
    }

    return e;
}

#define logFailure(hr) { if (FAILED (hr)) { DBG ("WASAPI FAIL! " + getAudioErrorDesc (hr)); jassertfalse } }
#define OK(a) wasapi_checkResult(a)

static bool wasapi_checkResult (HRESULT hr)
{
    logFailure (hr);
    return SUCCEEDED (hr);
}

#else
 #define logFailure(hr) {}
 #define OK(a) SUCCEEDED(a)
#endif

//==============================================================================
static const String wasapi_getDeviceID (IMMDevice* const device)
{
    String s;
    WCHAR* deviceId = 0;

    if (OK (device->GetId (&deviceId)))
    {
        s = String (deviceId);
        CoTaskMemFree (deviceId);
    }

    return s;
}

static EDataFlow wasapi_getDataFlow (IMMDevice* const device)
{
    EDataFlow flow = eRender;
    ComSmartPtr <IMMEndpoint> endPoint;
    if (OK (device->QueryInterface (__uuidof (IMMEndpoint), (void**) &endPoint)))
        (void) OK (endPoint->GetDataFlow (&flow));

    return flow;
}

static int wasapi_refTimeToSamples (const REFERENCE_TIME& t, const double sampleRate) throw()
{
    return roundDoubleToInt (sampleRate * ((double) t) * 0.0000001);
}

static void wasapi_copyWavFormat (WAVEFORMATEXTENSIBLE& dest, const WAVEFORMATEX* const src) throw()
{
    memcpy (&dest, src, src->wFormatTag == WAVE_FORMAT_EXTENSIBLE ? sizeof (WAVEFORMATEXTENSIBLE)
                                                                  : sizeof (WAVEFORMATEX));
}

//==============================================================================
class WASAPIDeviceBase
{
public:
    WASAPIDeviceBase (const ComSmartPtr <IMMDevice>& device_)
        : device (device_),
          sampleRate (0),
          numChannels (0),
          actualNumChannels (0),
          defaultSampleRate (0),
          minBufferSize (0),
          defaultBufferSize (0),
          latencySamples (0)
    {
        clientEvent = CreateEvent (0, false, false, _T("JuceWASAPI"));

        ComSmartPtr <IAudioClient> tempClient (createClient());
        if (tempClient == 0)
            return;

        REFERENCE_TIME defaultPeriod, minPeriod;
        if (! OK (tempClient->GetDevicePeriod (&defaultPeriod, &minPeriod)))
            return;

        WAVEFORMATEX* mixFormat = 0;
        if (! OK (tempClient->GetMixFormat (&mixFormat)))
            return;

        WAVEFORMATEXTENSIBLE format;
        wasapi_copyWavFormat (format, mixFormat);
        CoTaskMemFree (mixFormat);

        actualNumChannels = numChannels = format.Format.nChannels;
        defaultSampleRate = format.Format.nSamplesPerSec;
        minBufferSize = wasapi_refTimeToSamples (minPeriod, defaultSampleRate);
        defaultBufferSize = wasapi_refTimeToSamples (defaultPeriod, defaultSampleRate);

        FloatElementComparator<double> comparator;
        rates.addSorted (comparator, defaultSampleRate);

        static const double ratesToTest[] = { 44100.0, 48000.0, 88200.0, 96000.0 };

        for (int i = 0; i < numElementsInArray (ratesToTest); ++i)
        {
            if (ratesToTest[i] == defaultSampleRate)
                continue;

            format.Format.nSamplesPerSec = roundDoubleToInt (ratesToTest[i]);

            if (SUCCEEDED (tempClient->IsFormatSupported (AUDCLNT_SHAREMODE_SHARED, (WAVEFORMATEX*) &format, 0)))
                if (! rates.contains (ratesToTest[i]))
                    rates.addSorted (comparator, ratesToTest[i]);
        }
    }

    ~WASAPIDeviceBase()
    {
        device = 0;
        CloseHandle (clientEvent);
    }

    bool isOk() const throw()     { return defaultBufferSize > 0 && defaultSampleRate > 0; }

    bool openClient (const double newSampleRate, const BitArray& newChannels)
    {
        sampleRate = newSampleRate;
        channels = newChannels;
        channels.setRange (actualNumChannels, channels.getHighestBit() + 1 - actualNumChannels, false);
        numChannels = channels.getHighestBit() + 1;

        if (numChannels == 0)
            return true;

        client = createClient();

        if (client != 0
             && (tryInitialisingWithFormat (true, 4) || tryInitialisingWithFormat (false, 4)
                  || tryInitialisingWithFormat (false, 3) || tryInitialisingWithFormat (false, 2)))
        {
            channelMaps.clear();
            for (int i = 0; i <= channels.getHighestBit(); ++i)
                if (channels[i])
                    channelMaps.add (i);

            REFERENCE_TIME latency;
            if (OK (client->GetStreamLatency (&latency)))
                latencySamples = wasapi_refTimeToSamples (latency, sampleRate);

            (void) OK (client->GetBufferSize (&actualBufferSize));

            return OK (client->SetEventHandle (clientEvent));
        }

        return false;
    }

    void closeClient()
    {
        if (client != 0)
            client->Stop();

        client = 0;
        ResetEvent (clientEvent);
    }

    ComSmartPtr <IMMDevice> device;
    ComSmartPtr <IAudioClient> client;
    double sampleRate, defaultSampleRate;
    int numChannels, actualNumChannels;
    int minBufferSize, defaultBufferSize, latencySamples;
    Array <double> rates;
    HANDLE clientEvent;
    BitArray channels;
    AudioDataConverters::DataFormat dataFormat;
    Array <int> channelMaps;
    UINT32 actualBufferSize;
    int bytesPerSample;

private:
    const ComSmartPtr <IAudioClient> createClient()
    {
        ComSmartPtr <IAudioClient> client;

        if (device != 0)
        {
            HRESULT hr = device->Activate (__uuidof (IAudioClient), CLSCTX_INPROC_SERVER, 0, (void**) &client);
            logFailure (hr);
        }

        return client;
    }

    bool tryInitialisingWithFormat (const bool useFloat, const int bytesPerSampleToTry)
    {
        WAVEFORMATEXTENSIBLE format;
        zerostruct (format);

        if (numChannels <= 2 && bytesPerSampleToTry <= 2)
        {
            format.Format.wFormatTag = WAVE_FORMAT_PCM;
        }
        else
        {
            format.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
            format.Format.cbSize = sizeof (WAVEFORMATEXTENSIBLE) - sizeof (WAVEFORMATEX);
        }

        format.Format.nSamplesPerSec = roundDoubleToInt (sampleRate);
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

        WAVEFORMATEXTENSIBLE* nearestFormat = 0;

        HRESULT hr = client->IsFormatSupported (AUDCLNT_SHAREMODE_SHARED, (WAVEFORMATEX*) &format, (WAVEFORMATEX**) &nearestFormat);
        logFailure (hr);

        if (hr == S_FALSE && format.Format.nSamplesPerSec == nearestFormat->Format.nSamplesPerSec)
        {
            wasapi_copyWavFormat (format, (WAVEFORMATEX*) nearestFormat);
            hr = S_OK;
        }

        CoTaskMemFree (nearestFormat);

        GUID session;
        if (hr == S_OK
             && OK (client->Initialize (AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_EVENTCALLBACK,
                                        0, 0, (WAVEFORMATEX*) &format, &session)))
        {
            actualNumChannels = format.Format.nChannels;
            const bool isFloat = format.Format.wFormatTag == WAVE_FORMAT_EXTENSIBLE && format.SubFormat == KSDATAFORMAT_SUBTYPE_IEEE_FLOAT;
            bytesPerSample = format.Format.wBitsPerSample / 8;
            dataFormat = isFloat ? AudioDataConverters::float32LE
                                 : (bytesPerSample == 4 ? AudioDataConverters::int32LE
                                                        : ((bytesPerSample == 3 ? AudioDataConverters::int24LE
                                                                                : AudioDataConverters::int16LE)));
            return true;
        }

        return false;
    }
};

//==============================================================================
class WASAPIInputDevice  : public WASAPIDeviceBase
{
public:
    WASAPIInputDevice (const ComSmartPtr <IMMDevice>& device_)
        : WASAPIDeviceBase (device_),
          reservoir (1, 1)
    {
    }

    ~WASAPIInputDevice()
    {
        close();
    }

    bool open (const double newSampleRate, const BitArray& newChannels)
    {
        reservoirSize = 0;
        reservoirCapacity = 16384;
        reservoir.setSize (actualNumChannels * reservoirCapacity * sizeof (float));
        return openClient (newSampleRate, newChannels)
                && (numChannels == 0 || OK (client->GetService (__uuidof (IAudioCaptureClient), (void**) &captureClient)));
    }

    void close()
    {
        closeClient();
        captureClient = 0;
        reservoir.setSize (0);
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
                {
                    float* const dest = destBuffers[i] + offset;
                    const int srcChan = channelMaps.getUnchecked(i);

                    switch (dataFormat)
                    {
                    case AudioDataConverters::float32LE:
                        AudioDataConverters::convertFloat32LEToFloat (((uint8*) reservoir.getData()) + 4 * srcChan, dest, samplesToDo, 4 * actualNumChannels);
                        break;

                    case AudioDataConverters::int32LE:
                        AudioDataConverters::convertInt32LEToFloat (((uint8*) reservoir.getData()) + 4 * srcChan, dest, samplesToDo, 4 * actualNumChannels);
                        break;

                    case AudioDataConverters::int24LE:
                        AudioDataConverters::convertInt24LEToFloat (((uint8*) reservoir.getData()) + 3 * srcChan, dest, samplesToDo, 3 * actualNumChannels);
                        break;

                    case AudioDataConverters::int16LE:
                        AudioDataConverters::convertInt16LEToFloat (((uint8*) reservoir.getData()) + 2 * srcChan, dest, samplesToDo, 2 * actualNumChannels);
                        break;

                    default: jassertfalse; break;
                    }
                }

                bufferSize -= samplesToDo;
                offset += samplesToDo;
                reservoirSize -= samplesToDo;
            }
            else
            {
                UINT32 packetLength = 0;
                if (! OK (captureClient->GetNextPacketSize (&packetLength)))
                    break;

                if (packetLength == 0)
                {
                    if (thread.threadShouldExit())
                        break;

                    Thread::sleep (1);
                    continue;
                }

                uint8* inputData = 0;
                UINT32 numSamplesAvailable;
                DWORD flags;

                if (OK (captureClient->GetBuffer (&inputData, &numSamplesAvailable, &flags, 0, 0)))
                {
                    const int samplesToDo = jmin (bufferSize, (int) numSamplesAvailable);

                    for (int i = 0; i < numDestBuffers; ++i)
                    {
                        float* const dest = destBuffers[i] + offset;
                        const int srcChan = channelMaps.getUnchecked(i);

                        switch (dataFormat)
                        {
                        case AudioDataConverters::float32LE:
                            AudioDataConverters::convertFloat32LEToFloat (inputData + 4 * srcChan, dest, samplesToDo, 4 * actualNumChannels);
                            break;

                        case AudioDataConverters::int32LE:
                            AudioDataConverters::convertInt32LEToFloat (inputData + 4 * srcChan, dest, samplesToDo, 4 * actualNumChannels);
                            break;

                        case AudioDataConverters::int24LE:
                            AudioDataConverters::convertInt24LEToFloat (inputData + 3 * srcChan, dest, samplesToDo, 3 * actualNumChannels);
                            break;

                        case AudioDataConverters::int16LE:
                            AudioDataConverters::convertInt16LEToFloat (inputData + 2 * srcChan, dest, samplesToDo, 2 * actualNumChannels);
                            break;

                        default: jassertfalse; break;
                        }
                    }

                    bufferSize -= samplesToDo;
                    offset += samplesToDo;

                    if (samplesToDo < numSamplesAvailable)
                    {
                        reservoirSize = jmin ((int) (numSamplesAvailable - samplesToDo), reservoirCapacity);
                        memcpy ((uint8*) reservoir.getData(), inputData + bytesPerSample * actualNumChannels * samplesToDo,
                                bytesPerSample * actualNumChannels * reservoirSize);
                    }

                    captureClient->ReleaseBuffer (numSamplesAvailable);
                }
            }
        }
    }

    ComSmartPtr <IAudioCaptureClient> captureClient;
    MemoryBlock reservoir;
    int reservoirSize, reservoirCapacity;
};

//==============================================================================
class WASAPIOutputDevice  : public WASAPIDeviceBase
{
public:
    WASAPIOutputDevice (const ComSmartPtr <IMMDevice>& device_)
        : WASAPIDeviceBase (device_)
    {
    }

    ~WASAPIOutputDevice()
    {
        close();
    }

    bool open (const double newSampleRate, const BitArray& newChannels)
    {
        return openClient (newSampleRate, newChannels)
                && (numChannels == 0 || OK (client->GetService (__uuidof (IAudioRenderClient), (void**) &renderClient)));
    }

    void close()
    {
        closeClient();
        renderClient = 0;
    }

    void copyBuffers (const float** const srcBuffers, const int numSrcBuffers, int bufferSize, Thread& thread)
    {
        if (numChannels <= 0)
            return;

        int offset = 0;

        while (bufferSize > 0)
        {
            UINT32 padding = 0;
            if (! OK (client->GetCurrentPadding (&padding)))
                return;

            const int samplesToDo = jmin ((int) (actualBufferSize - padding), bufferSize);

            if (samplesToDo <= 0)
            {
                if (thread.threadShouldExit())
                    break;

                Thread::sleep (0);
                continue;
            }

            uint8* outputData = 0;
            if (OK (renderClient->GetBuffer (samplesToDo, &outputData)))
            {
                for (int i = 0; i < numSrcBuffers; ++i)
                {
                    const float* const source = srcBuffers[i] + offset;
                    const int destChan = channelMaps.getUnchecked(i);

                    switch (dataFormat)
                    {
                    case AudioDataConverters::float32LE:
                        AudioDataConverters::convertFloatToFloat32LE (source, outputData + 4 * destChan, samplesToDo, 4 * actualNumChannels);
                        break;

                    case AudioDataConverters::int32LE:
                        AudioDataConverters::convertFloatToInt32LE (source, outputData + 4 * destChan, samplesToDo, 4 * actualNumChannels);
                        break;

                    case AudioDataConverters::int24LE:
                        AudioDataConverters::convertFloatToInt24LE (source, outputData + 3 * destChan, samplesToDo, 3 * actualNumChannels);
                        break;

                    case AudioDataConverters::int16LE:
                        AudioDataConverters::convertFloatToInt16LE (source, outputData + 2 * destChan, samplesToDo, 2 * actualNumChannels);
                        break;

                    default: jassertfalse; break;
                    }
                }

                renderClient->ReleaseBuffer (samplesToDo, 0);

                offset += samplesToDo;
                bufferSize -= samplesToDo;
            }
        }
    }

    ComSmartPtr <IAudioRenderClient> renderClient;
};

//==============================================================================
class WASAPIAudioIODevice  : public AudioIODevice,
                             public Thread
{
public:
    WASAPIAudioIODevice (const String& deviceName,
                         const String& outputDeviceId_,
                         const String& inputDeviceId_)
        : AudioIODevice (deviceName, "Windows Audio"),
          Thread ("Juce WASAPI"),
          isOpen_ (false),
          isStarted (false),
          outputDevice (0),
          outputDeviceId (outputDeviceId_),
          inputDevice (0),
          inputDeviceId (inputDeviceId_),
          currentBufferSizeSamples (0),
          currentSampleRate (0),
          callback (0)
    {
    }

    ~WASAPIAudioIODevice()
    {
        close();

        deleteAndZero (inputDevice);
        deleteAndZero (outputDevice);
    }

    bool initialise()
    {
        double defaultSampleRateIn = 0, defaultSampleRateOut = 0;
        int minBufferSizeIn = 0, defaultBufferSizeIn = 0, minBufferSizeOut = 0, defaultBufferSizeOut = 0;
        latencyIn = latencyOut = 0;
        Array <double> ratesIn, ratesOut;

        if (createDevices())
        {
            jassert (inputDevice != 0 || outputDevice != 0);

            if (inputDevice != 0 && outputDevice != 0)
            {
                defaultSampleRate = jmin (inputDevice->defaultSampleRate, outputDevice->defaultSampleRate);
                minBufferSize = jmin (inputDevice->minBufferSize, outputDevice->minBufferSize);
                defaultBufferSize = jmax (inputDevice->defaultBufferSize, outputDevice->defaultBufferSize);
                sampleRates = inputDevice->rates;
                sampleRates.removeValuesNotIn (outputDevice->rates);
            }
            else
            {
                WASAPIDeviceBase* const d = inputDevice != 0 ? (WASAPIDeviceBase*) inputDevice : (WASAPIDeviceBase*) outputDevice;
                defaultSampleRate = d->defaultSampleRate;
                minBufferSize = d->minBufferSize;
                defaultBufferSize = d->defaultBufferSize;
                sampleRates = d->rates;
            }

            IntegerElementComparator<int> comparator;
            bufferSizes.addSorted (comparator, defaultBufferSize);
            if (minBufferSize != defaultBufferSize)
                bufferSizes.addSorted (comparator, minBufferSize);

            int n = 64;
            for (int i = 0; i < 40; ++i)
            {
                if (n >= minBufferSize && n <= 2048 && ! bufferSizes.contains (n))
                    bufferSizes.addSorted (comparator, n);

                n += (n < 512) ? 32 : (n < 1024 ? 64 : 128);
            }

            return true;
        }

        return false;
    }

    const StringArray getOutputChannelNames()
    {
        StringArray outChannels;

        if (outputDevice != 0)
            for (int i = 1; i <= outputDevice->actualNumChannels; ++i)
                outChannels.add ("Output channel " + String (i));

        return outChannels;
    }

    const StringArray getInputChannelNames()
    {
        StringArray inChannels;

        if (inputDevice != 0)
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
    const BitArray getActiveOutputChannels() const      { return outputDevice != 0 ? outputDevice->channels : BitArray(); }
    const BitArray getActiveInputChannels() const       { return inputDevice != 0 ? inputDevice->channels : BitArray(); }
    const String getLastError() { return lastError; }


    const String open (const BitArray& inputChannels, const BitArray& outputChannels,
                       double sampleRate, int bufferSizeSamples)
    {
        close();
        lastError = String::empty;

        if (sampleRates.size() == 0 && inputDevice != 0 && outputDevice != 0)
        {
            lastError = "The input and output devices don't share a common sample rate!";
            return lastError;
        }

        currentBufferSizeSamples = bufferSizeSamples <= 0 ? defaultBufferSize : jmax (bufferSizeSamples, minBufferSize);
        currentSampleRate = sampleRate > 0 ? sampleRate : defaultSampleRate;

        if (inputDevice != 0 && ! inputDevice->open (currentSampleRate, inputChannels))
        {
            lastError = "Couldn't open the input device!";
            return lastError;
        }

        if (outputDevice != 0 && ! outputDevice->open (currentSampleRate, outputChannels))
        {
            close();
            lastError = "Couldn't open the output device!";
            return lastError;
        }

        if (inputDevice != 0)
            ResetEvent (inputDevice->clientEvent);
        if (outputDevice != 0)
            ResetEvent (outputDevice->clientEvent);

        startThread (8);
        Thread::sleep (5);

        if (inputDevice != 0 && inputDevice->client != 0)
        {
            latencyIn = inputDevice->latencySamples + inputDevice->actualBufferSize + inputDevice->minBufferSize;
            HRESULT hr = inputDevice->client->Start();
            logFailure (hr); //xxx handle this
        }

        if (outputDevice != 0 && outputDevice->client != 0)
        {
            latencyOut = outputDevice->latencySamples + outputDevice->actualBufferSize + outputDevice->minBufferSize;
            HRESULT hr = outputDevice->client->Start();
            logFailure (hr); //xxx handle this
        }

        isOpen_ = true;
        return lastError;
    }

    void close()
    {
        stop();

        if (inputDevice != 0)
            SetEvent (inputDevice->clientEvent);

        if (outputDevice != 0)
            SetEvent (outputDevice->clientEvent);

        stopThread (5000);

        if (inputDevice != 0)
            inputDevice->close();

        if (outputDevice != 0)
            outputDevice->close();

        isOpen_ = false;
    }

    bool isOpen()       { return isOpen_ && isThreadRunning(); }
    bool isPlaying()    { return isStarted && isOpen_ && isThreadRunning(); }

    void start (AudioIODeviceCallback* call)
    {
        if (isOpen_ && call != 0 && ! isStarted)
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

            if (callbackLocal != 0)
                callbackLocal->audioDeviceStopped();
        }
    }

    void run()
    {
        const int bufferSize = currentBufferSizeSamples;

        HANDLE events[2];
        int numEvents = 0;
        if (inputDevice != 0)
            events [numEvents++] = inputDevice->clientEvent;
        if (outputDevice != 0)
            events [numEvents++] = outputDevice->clientEvent;

        const int numInputBuffers = getActiveInputChannels().countNumberOfSetBits();
        const int numOutputBuffers = getActiveOutputChannels().countNumberOfSetBits();

        AudioSampleBuffer ins (jmax (1, numInputBuffers), bufferSize + 32);
        AudioSampleBuffer outs (jmax (1, numOutputBuffers), bufferSize + 32);
        float** const inputBuffers = ins.getArrayOfChannels();
        float** const outputBuffers = outs.getArrayOfChannels();
        ins.clear();

        while (! threadShouldExit())
        {
            const DWORD result = WaitForMultipleObjects (numEvents, events, true, 1000);

            if (result == WAIT_TIMEOUT)
                continue;

            if (threadShouldExit())
                break;

            if (inputDevice != 0)
                inputDevice->copyBuffers (inputBuffers, numInputBuffers, bufferSize, *this);

            // Make the callback..
            {
                const ScopedLock sl (startStopLock);

                if (isStarted)
                {
                    JUCE_TRY
                    {
                        callback->audioDeviceIOCallback ((const float**) inputBuffers,
                                                         numInputBuffers,
                                                         outputBuffers,
                                                         numOutputBuffers,
                                                         bufferSize);
                    }
                    JUCE_CATCH_EXCEPTION
                }
                else
                {
                    outs.clear();
                }
            }

            if (outputDevice != 0)
                outputDevice->copyBuffers ((const float**) outputBuffers, numOutputBuffers, bufferSize, *this);
        }
    }

    //==============================================================================
    juce_UseDebuggingNewOperator

    //==============================================================================
    String outputDeviceId, inputDeviceId;
    String lastError;

private:
    // Device stats...
    WASAPIInputDevice* inputDevice;
    WASAPIOutputDevice* outputDevice;
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
        if (! OK (enumerator.CoCreateInstance (__uuidof (MMDeviceEnumerator), CLSCTX_INPROC_SERVER)))
            return false;

        ComSmartPtr <IMMDeviceCollection> deviceCollection;
        if (! OK (enumerator->EnumAudioEndpoints (eAll, DEVICE_STATE_ACTIVE, &deviceCollection)))
            return false;

        UINT32 numDevices = 0;
        if (! OK (deviceCollection->GetCount (&numDevices)))
            return false;

        for (UINT32 i = 0; i < numDevices; ++i)
        {
            ComSmartPtr <IMMDevice> device;
            if (! OK (deviceCollection->Item (i, &device)))
                continue;

            const String deviceId (wasapi_getDeviceID (device));
            if (deviceId.isEmpty())
                continue;

            const EDataFlow flow = wasapi_getDataFlow (device);

            if (deviceId == inputDeviceId && flow == eCapture)
                inputDevice = new WASAPIInputDevice (device);
            else if (deviceId == outputDeviceId && flow == eRender)
                outputDevice = new WASAPIOutputDevice (device);
        }

        return (outputDeviceId.isEmpty() || (outputDevice != 0 && outputDevice->isOk()))
            && (inputDeviceId.isEmpty() || (inputDevice != 0 && inputDevice->isOk()));
    }

    //==============================================================================
    WASAPIAudioIODevice (const WASAPIAudioIODevice&);
    const WASAPIAudioIODevice& operator= (const WASAPIAudioIODevice&);
};


//==============================================================================
class WASAPIAudioIODeviceType  : public AudioIODeviceType
{
public:
    WASAPIAudioIODeviceType()
        : AudioIODeviceType (T("Windows Audio")),
          hasScanned (false)
    {
    }

    ~WASAPIAudioIODeviceType()
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

        ComSmartPtr <IMMDeviceEnumerator> enumerator;
        if (! OK (enumerator.CoCreateInstance (__uuidof (MMDeviceEnumerator), CLSCTX_INPROC_SERVER)))
            return;

        const String defaultRenderer = getDefaultEndpoint (enumerator, false);
        const String defaultCapture = getDefaultEndpoint (enumerator, true);

        ComSmartPtr <IMMDeviceCollection> deviceCollection;
        UINT32 numDevices = 0;

        if (! (OK (enumerator->EnumAudioEndpoints (eAll, DEVICE_STATE_ACTIVE, &deviceCollection))
                && OK (deviceCollection->GetCount (&numDevices))))
            return;

        for (UINT32 i = 0; i < numDevices; ++i)
        {
            ComSmartPtr <IMMDevice> device;
            if (! OK (deviceCollection->Item (i, &device)))
                continue;

            const String deviceId (wasapi_getDeviceID (device));

            DWORD state = 0;
            if (! OK (device->GetState (&state)))
                continue;

            if (state != DEVICE_STATE_ACTIVE)
                continue;

            String name;

            {
                ComSmartPtr <IPropertyStore> properties;
                if (! OK (device->OpenPropertyStore (STGM_READ, &properties)))
                    continue;

                PROPVARIANT value;
                PropVariantInit (&value);
                if (OK (properties->GetValue (PKEY_Device_FriendlyName, &value)))
                    name = value.pwszVal;

                PropVariantClear (&value);
            }

            const EDataFlow flow = wasapi_getDataFlow (device);

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

    const StringArray getDeviceNames (const bool wantInputNames) const
    {
        jassert (hasScanned); // need to call scanForDevices() before doing this

        return wantInputNames ? inputDeviceNames
                              : outputDeviceNames;
    }

    int getDefaultDeviceIndex (const bool /*forInput*/) const
    {
        jassert (hasScanned); // need to call scanForDevices() before doing this
        return 0;
    }

    int getIndexOfDevice (AudioIODevice* device, const bool asInput) const
    {
        jassert (hasScanned); // need to call scanForDevices() before doing this
        WASAPIAudioIODevice* const d = dynamic_cast <WASAPIAudioIODevice*> (device);
        return d == 0 ? -1 : (asInput ? inputDeviceIds.indexOf (d->inputDeviceId)
                                      : outputDeviceIds.indexOf (d->outputDeviceId));
    }

    bool hasSeparateInputsAndOutputs() const    { return true; }

    AudioIODevice* createDevice (const String& outputDeviceName,
                                 const String& inputDeviceName)
    {
        jassert (hasScanned); // need to call scanForDevices() before doing this

        WASAPIAudioIODevice* d = 0;

        const int outputIndex = outputDeviceNames.indexOf (outputDeviceName);
        const int inputIndex = inputDeviceNames.indexOf (inputDeviceName);

        if (outputIndex >= 0 || inputIndex >= 0)
        {
            d = new WASAPIAudioIODevice (outputDeviceName.isNotEmpty() ? outputDeviceName
                                                                       : inputDeviceName,
                                         outputDeviceIds [outputIndex],
                                         inputDeviceIds [inputIndex]);

            if (! d->initialise())
                deleteAndZero (d);
        }

        return d;
    }

    //==============================================================================
    juce_UseDebuggingNewOperator

    StringArray outputDeviceNames, outputDeviceIds;
    StringArray inputDeviceNames, inputDeviceIds;

private:
    bool hasScanned;

    //==============================================================================
    static const String getDefaultEndpoint (IMMDeviceEnumerator* const enumerator, const bool forCapture)
    {
        String s;
        IMMDevice* dev = 0;
        if (OK (enumerator->GetDefaultAudioEndpoint (forCapture ? eCapture : eRender,
                                                     eMultimedia, &dev)))
        {
            WCHAR* deviceId = 0;
            if (OK (dev->GetId (&deviceId)))
            {
                s = String (deviceId);
                CoTaskMemFree (deviceId);
            }

            dev->Release();
        }

        return s;
    }

    //==============================================================================
    WASAPIAudioIODeviceType (const WASAPIAudioIODeviceType&);
    const WASAPIAudioIODeviceType& operator= (const WASAPIAudioIODeviceType&);
};

//==============================================================================
AudioIODeviceType* juce_createAudioIODeviceType_WASAPI()
{
    return new WASAPIAudioIODeviceType();
}

#undef logFailure
#undef OK

#endif

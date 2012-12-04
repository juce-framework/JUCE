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

namespace
{
    void getDeviceSampleRates (snd_pcm_t* handle, Array <int>& rates)
    {
        const int ratesToTry[] = { 22050, 32000, 44100, 48000, 88200, 96000, 176400, 192000, 0 };

        snd_pcm_hw_params_t* hwParams;
        snd_pcm_hw_params_alloca (&hwParams);

        for (int i = 0; ratesToTry[i] != 0; ++i)
        {
            if (snd_pcm_hw_params_any (handle, hwParams) >= 0
                 && snd_pcm_hw_params_test_rate (handle, hwParams, ratesToTry[i], 0) == 0)
            {
                rates.addIfNotAlreadyThere (ratesToTry[i]);
            }
        }
    }

    void getDeviceNumChannels (snd_pcm_t* handle, unsigned int* minChans, unsigned int* maxChans)
    {
        snd_pcm_hw_params_t *params;
        snd_pcm_hw_params_alloca (&params);

        if (snd_pcm_hw_params_any (handle, params) >= 0)
        {
            snd_pcm_hw_params_get_channels_min (params, minChans);
            snd_pcm_hw_params_get_channels_max (params, maxChans);
        }
    }

    void getDeviceProperties (const String& deviceID,
                              unsigned int& minChansOut,
                              unsigned int& maxChansOut,
                              unsigned int& minChansIn,
                              unsigned int& maxChansIn,
                              Array <int>& rates)
    {
        if (deviceID.isEmpty())
            return;

        snd_ctl_t* handle;

        if (snd_ctl_open (&handle, deviceID.upToLastOccurrenceOf (",", false, false).toUTF8(), SND_CTL_NONBLOCK) >= 0)
        {
            snd_pcm_info_t* info;
            snd_pcm_info_alloca (&info);

            snd_pcm_info_set_stream (info, SND_PCM_STREAM_PLAYBACK);
            snd_pcm_info_set_device (info, deviceID.fromLastOccurrenceOf (",", false, false).getIntValue());
            snd_pcm_info_set_subdevice (info, 0);

            if (snd_ctl_pcm_info (handle, info) >= 0)
            {
                snd_pcm_t* pcmHandle;
                if (snd_pcm_open (&pcmHandle, deviceID.toUTF8(), SND_PCM_STREAM_PLAYBACK, SND_PCM_ASYNC | SND_PCM_NONBLOCK) >= 0)
                {
                    getDeviceNumChannels (pcmHandle, &minChansOut, &maxChansOut);
                    getDeviceSampleRates (pcmHandle, rates);

                    snd_pcm_close (pcmHandle);
                }
            }

            snd_pcm_info_set_stream (info, SND_PCM_STREAM_CAPTURE);

            if (snd_ctl_pcm_info (handle, info) >= 0)
            {
                snd_pcm_t* pcmHandle;
                if (snd_pcm_open (&pcmHandle, deviceID.toUTF8(), SND_PCM_STREAM_CAPTURE, SND_PCM_ASYNC | SND_PCM_NONBLOCK) >= 0)
                {
                    getDeviceNumChannels (pcmHandle, &minChansIn, &maxChansIn);

                    if (rates.size() == 0)
                        getDeviceSampleRates (pcmHandle, rates);

                    snd_pcm_close (pcmHandle);
                }
            }

            snd_ctl_close (handle);
        }
    }
}

//==============================================================================
class ALSADevice
{
public:
    ALSADevice (const String& deviceID, bool forInput)
        : handle (0),
          bitDepth (16),
          numChannelsRunning (0),
          latency (0),
          isInput (forInput),
          isInterleaved (true)
    {
        failed (snd_pcm_open (&handle, deviceID.toUTF8(),
                              forInput ? SND_PCM_STREAM_CAPTURE : SND_PCM_STREAM_PLAYBACK,
                              SND_PCM_ASYNC));
    }

    ~ALSADevice()
    {
        if (handle != 0)
            snd_pcm_close (handle);
    }

    bool setParameters (unsigned int sampleRate, int numChannels, int bufferSize)
    {
        if (handle == 0)
            return false;

        snd_pcm_hw_params_t* hwParams;
        snd_pcm_hw_params_alloca (&hwParams);

        if (failed (snd_pcm_hw_params_any (handle, hwParams)))
            return false;

        if (snd_pcm_hw_params_set_access (handle, hwParams, SND_PCM_ACCESS_RW_NONINTERLEAVED) >= 0)
            isInterleaved = false;
        else if (snd_pcm_hw_params_set_access (handle, hwParams, SND_PCM_ACCESS_RW_INTERLEAVED) >= 0)
            isInterleaved = true;
        else
        {
            jassertfalse;
            return false;
        }

        enum { isFloatBit = 1 << 16, isLittleEndianBit = 1 << 17 };

        const int formatsToTry[] = { SND_PCM_FORMAT_FLOAT_LE,   32 | isFloatBit | isLittleEndianBit,
                                     SND_PCM_FORMAT_FLOAT_BE,   32 | isFloatBit,
                                     SND_PCM_FORMAT_S32_LE,     32 | isLittleEndianBit,
                                     SND_PCM_FORMAT_S32_BE,     32,
                                     SND_PCM_FORMAT_S24_3LE,    24 | isLittleEndianBit,
                                     SND_PCM_FORMAT_S24_3BE,    24,
                                     SND_PCM_FORMAT_S16_LE,     16 | isLittleEndianBit,
                                     SND_PCM_FORMAT_S16_BE,     16 };
        bitDepth = 0;

        for (int i = 0; i < numElementsInArray (formatsToTry); i += 2)
        {
            if (snd_pcm_hw_params_set_format (handle, hwParams, (_snd_pcm_format) formatsToTry [i]) >= 0)
            {
                bitDepth = formatsToTry [i + 1] & 255;
                const bool isFloat = (formatsToTry [i + 1] & isFloatBit) != 0;
                const bool isLittleEndian = (formatsToTry [i + 1] & isLittleEndianBit) != 0;
                converter = createConverter (isInput, bitDepth, isFloat, isLittleEndian, numChannels);
                break;
            }
        }

        if (bitDepth == 0)
        {
            error = "device doesn't support a compatible PCM format";
            DBG ("ALSA error: " + error + "\n");
            return false;
        }

        int dir = 0;
        unsigned int periods = 4;
        snd_pcm_uframes_t samplesPerPeriod = bufferSize;

        if (failed (snd_pcm_hw_params_set_rate_near (handle, hwParams, &sampleRate, 0))
            || failed (snd_pcm_hw_params_set_channels (handle, hwParams, numChannels))
            || failed (snd_pcm_hw_params_set_periods_near (handle, hwParams, &periods, &dir))
            || failed (snd_pcm_hw_params_set_period_size_near (handle, hwParams, &samplesPerPeriod, &dir))
            || failed (snd_pcm_hw_params (handle, hwParams)))
        {
            return false;
        }

        snd_pcm_uframes_t frames = 0;

        if (failed (snd_pcm_hw_params_get_period_size (hwParams, &frames, &dir))
             || failed (snd_pcm_hw_params_get_periods (hwParams, &periods, &dir)))
            latency = 0;
        else
            latency = frames * (periods - 1); // (this is the method JACK uses to guess the latency..)

        snd_pcm_sw_params_t* swParams;
        snd_pcm_sw_params_alloca (&swParams);
        snd_pcm_uframes_t boundary;

        if (failed (snd_pcm_sw_params_current (handle, swParams))
            || failed (snd_pcm_sw_params_get_boundary (swParams, &boundary))
            || failed (snd_pcm_sw_params_set_silence_threshold (handle, swParams, 0))
            || failed (snd_pcm_sw_params_set_silence_size (handle, swParams, boundary))
            || failed (snd_pcm_sw_params_set_start_threshold (handle, swParams, samplesPerPeriod))
            || failed (snd_pcm_sw_params_set_stop_threshold (handle, swParams, boundary))
            || failed (snd_pcm_sw_params (handle, swParams)))
        {
            return false;
        }

      #if 0
        // enable this to dump the config of the devices that get opened
        snd_output_t* out;
        snd_output_stdio_attach (&out, stderr, 0);
        snd_pcm_hw_params_dump (hwParams, out);
        snd_pcm_sw_params_dump (swParams, out);
      #endif

        numChannelsRunning = numChannels;

        return true;
    }

    //==============================================================================
    bool writeToOutputDevice (AudioSampleBuffer& outputChannelBuffer, const int numSamples)
    {
        jassert (numChannelsRunning <= outputChannelBuffer.getNumChannels());
        float** const data = outputChannelBuffer.getArrayOfChannels();
        snd_pcm_sframes_t numDone = 0;

        if (isInterleaved)
        {
            scratch.ensureSize (sizeof (float) * numSamples * numChannelsRunning, false);

            for (int i = 0; i < numChannelsRunning; ++i)
                converter->convertSamples (scratch.getData(), i, data[i], 0, numSamples);

            numDone = snd_pcm_writei (handle, scratch.getData(), numSamples);
        }
        else
        {
            for (int i = 0; i < numChannelsRunning; ++i)
                converter->convertSamples (data[i], data[i], numSamples);

            numDone = snd_pcm_writen (handle, (void**) data, numSamples);
        }

        if (failed (numDone))
        {
            if (numDone == -EPIPE)
            {
                if (failed (snd_pcm_prepare (handle)))
                    return false;
            }
            else if (numDone != -ESTRPIPE)
                return false;
        }

        return true;
    }

    bool readFromInputDevice (AudioSampleBuffer& inputChannelBuffer, const int numSamples)
    {
        jassert (numChannelsRunning <= inputChannelBuffer.getNumChannels());
        float** const data = inputChannelBuffer.getArrayOfChannels();

        if (isInterleaved)
        {
            scratch.ensureSize (sizeof (float) * numSamples * numChannelsRunning, false);
            scratch.fillWith (0); // (not clearing this data causes warnings in valgrind)

            snd_pcm_sframes_t num = snd_pcm_readi (handle, scratch.getData(), numSamples);

            if (failed (num))
            {
                if (num == -EPIPE)
                {
                    if (failed (snd_pcm_prepare (handle)))
                        return false;
                }
                else if (num != -ESTRPIPE)
                    return false;
            }

            for (int i = 0; i < numChannelsRunning; ++i)
                converter->convertSamples (data[i], 0, scratch.getData(), i, numSamples);
        }
        else
        {
            snd_pcm_sframes_t num = snd_pcm_readn (handle, (void**) data, numSamples);

            if (failed (num) && num != -EPIPE && num != -ESTRPIPE)
                return false;

            for (int i = 0; i < numChannelsRunning; ++i)
                converter->convertSamples (data[i], data[i], numSamples);
        }

        return true;
    }

    //==============================================================================
    snd_pcm_t* handle;
    String error;
    int bitDepth, numChannelsRunning, latency;

    //==============================================================================
private:
    const bool isInput;
    bool isInterleaved;
    MemoryBlock scratch;
    ScopedPointer<AudioData::Converter> converter;

    //==============================================================================
    template <class SampleType>
    struct ConverterHelper
    {
        static AudioData::Converter* createConverter (const bool forInput, const bool isLittleEndian, const int numInterleavedChannels)
        {
            if (forInput)
            {
                typedef AudioData::Pointer <AudioData::Float32, AudioData::NativeEndian, AudioData::NonInterleaved, AudioData::NonConst> DestType;

                if (isLittleEndian)
                    return new AudioData::ConverterInstance <AudioData::Pointer <SampleType, AudioData::LittleEndian, AudioData::Interleaved, AudioData::Const>, DestType> (numInterleavedChannels, 1);
                else
                    return new AudioData::ConverterInstance <AudioData::Pointer <SampleType, AudioData::BigEndian, AudioData::Interleaved, AudioData::Const>, DestType> (numInterleavedChannels, 1);
            }
            else
            {
                typedef AudioData::Pointer <AudioData::Float32, AudioData::NativeEndian, AudioData::NonInterleaved, AudioData::Const> SourceType;

                if (isLittleEndian)
                    return new AudioData::ConverterInstance <SourceType, AudioData::Pointer <SampleType, AudioData::LittleEndian, AudioData::Interleaved, AudioData::NonConst> > (1, numInterleavedChannels);
                else
                    return new AudioData::ConverterInstance <SourceType, AudioData::Pointer <SampleType, AudioData::BigEndian, AudioData::Interleaved, AudioData::NonConst> > (1, numInterleavedChannels);
            }
        }
    };

    static AudioData::Converter* createConverter (const bool forInput, const int bitDepth, const bool isFloat, const bool isLittleEndian, const int numInterleavedChannels)
    {
        switch (bitDepth)
        {
            case 16:    return ConverterHelper <AudioData::Int16>::createConverter (forInput, isLittleEndian,  numInterleavedChannels);
            case 24:    return ConverterHelper <AudioData::Int24>::createConverter (forInput, isLittleEndian,  numInterleavedChannels);
            case 32:    return isFloat ? ConverterHelper <AudioData::Float32>::createConverter (forInput, isLittleEndian,  numInterleavedChannels)
                                       : ConverterHelper <AudioData::Int32>::createConverter (forInput, isLittleEndian,  numInterleavedChannels);
            default:    jassertfalse; break; // unsupported format!
        }

        return nullptr;
    }

    //==============================================================================
    bool failed (const int errorNum)
    {
        if (errorNum >= 0)
            return false;

        error = snd_strerror (errorNum);
        DBG ("ALSA error: " + error + "\n");
        return true;
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ALSADevice)
};

//==============================================================================
class ALSAThread  : public Thread
{
public:
    ALSAThread (const String& inputId_,
                const String& outputId_)
        : Thread ("Juce ALSA"),
          sampleRate (0),
          bufferSize (0),
          outputLatency (0),
          inputLatency (0),
          callback (0),
          inputId (inputId_),
          outputId (outputId_),
          numCallbacks (0),
          inputChannelBuffer (1, 1),
          outputChannelBuffer (1, 1)
    {
        initialiseRatesAndChannels();
    }

    ~ALSAThread()
    {
        close();
    }

    void open (BigInteger inputChannels,
               BigInteger outputChannels,
               const double sampleRate_,
               const int bufferSize_)
    {
        close();

        error = String::empty;
        sampleRate = sampleRate_;
        bufferSize = bufferSize_;

        inputChannelBuffer.setSize (jmax ((int) minChansIn, inputChannels.getHighestBit()) + 1, bufferSize);
        inputChannelBuffer.clear();
        inputChannelDataForCallback.clear();
        currentInputChans.clear();

        if (inputChannels.getHighestBit() >= 0)
        {
            for (int i = 0; i <= jmax (inputChannels.getHighestBit(), (int) minChansIn); ++i)
            {
                if (inputChannels[i])
                {
                    inputChannelDataForCallback.add (inputChannelBuffer.getSampleData (i));
                    currentInputChans.setBit (i);
                }
            }
        }

        outputChannelBuffer.setSize (jmax ((int) minChansOut, outputChannels.getHighestBit()) + 1, bufferSize);
        outputChannelBuffer.clear();
        outputChannelDataForCallback.clear();
        currentOutputChans.clear();

        if (outputChannels.getHighestBit() >= 0)
        {
            for (int i = 0; i <= jmax (outputChannels.getHighestBit(), (int) minChansOut); ++i)
            {
                if (outputChannels[i])
                {
                    outputChannelDataForCallback.add (outputChannelBuffer.getSampleData (i));
                    currentOutputChans.setBit (i);
                }
            }
        }

        if (outputChannelDataForCallback.size() > 0 && outputId.isNotEmpty())
        {
            outputDevice = new ALSADevice (outputId, false);

            if (outputDevice->error.isNotEmpty())
            {
                error = outputDevice->error;
                outputDevice = nullptr;
                return;
            }

            currentOutputChans.setRange (0, minChansOut, true);

            if (! outputDevice->setParameters ((unsigned int) sampleRate,
                                               jlimit ((int) minChansOut, (int) maxChansOut, currentOutputChans.getHighestBit() + 1),
                                               bufferSize))
            {
                error = outputDevice->error;
                outputDevice = nullptr;
                return;
            }

            outputLatency = outputDevice->latency;
        }

        if (inputChannelDataForCallback.size() > 0 && inputId.isNotEmpty())
        {
            inputDevice = new ALSADevice (inputId, true);

            if (inputDevice->error.isNotEmpty())
            {
                error = inputDevice->error;
                inputDevice = nullptr;
                return;
            }

            currentInputChans.setRange (0, minChansIn, true);

            if (! inputDevice->setParameters ((unsigned int) sampleRate,
                                              jlimit ((int) minChansIn, (int) maxChansIn, currentInputChans.getHighestBit() + 1),
                                              bufferSize))
            {
                error = inputDevice->error;
                inputDevice = nullptr;
                return;
            }

            inputLatency = inputDevice->latency;
        }

        if (outputDevice == nullptr && inputDevice == nullptr)
        {
            error = "no channels";
            return;
        }

        if (outputDevice != nullptr && inputDevice != nullptr)
        {
            snd_pcm_link (outputDevice->handle, inputDevice->handle);
        }

        if (inputDevice != nullptr && failed (snd_pcm_prepare (inputDevice->handle)))
            return;

        if (outputDevice != nullptr && failed (snd_pcm_prepare (outputDevice->handle)))
            return;

        startThread (9);

        int count = 1000;

        while (numCallbacks == 0)
        {
            sleep (5);

            if (--count < 0 || ! isThreadRunning())
            {
                error = "device didn't start";
                break;
            }
        }
    }

    void close()
    {
        stopThread (6000);

        inputDevice = nullptr;
        outputDevice = nullptr;

        inputChannelBuffer.setSize (1, 1);
        outputChannelBuffer.setSize (1, 1);

        numCallbacks = 0;
    }

    void setCallback (AudioIODeviceCallback* const newCallback) noexcept
    {
        const ScopedLock sl (callbackLock);
        callback = newCallback;
    }

    void run()
    {
        while (! threadShouldExit())
        {
            if (inputDevice != nullptr)
            {
                if (! inputDevice->readFromInputDevice (inputChannelBuffer, bufferSize))
                {
                    DBG ("ALSA: read failure");
                    break;
                }
            }

            if (threadShouldExit())
                break;

            {
                const ScopedLock sl (callbackLock);
                ++numCallbacks;

                if (callback != nullptr)
                {
                    callback->audioDeviceIOCallback ((const float**) inputChannelDataForCallback.getRawDataPointer(),
                                                     inputChannelDataForCallback.size(),
                                                     outputChannelDataForCallback.getRawDataPointer(),
                                                     outputChannelDataForCallback.size(),
                                                     bufferSize);
                }
                else
                {
                    for (int i = 0; i < outputChannelDataForCallback.size(); ++i)
                        zeromem (outputChannelDataForCallback[i], sizeof (float) * bufferSize);
                }
            }

            if (outputDevice != nullptr)
            {
                failed (snd_pcm_wait (outputDevice->handle, 2000));

                if (threadShouldExit())
                    break;

                failed (snd_pcm_avail_update (outputDevice->handle));

                if (! outputDevice->writeToOutputDevice (outputChannelBuffer, bufferSize))
                {
                    DBG ("ALSA: write failure");
                    break;
                }
            }
        }
    }

    int getBitDepth() const noexcept
    {
        if (outputDevice != nullptr)
            return outputDevice->bitDepth;

        if (inputDevice != nullptr)
            return inputDevice->bitDepth;

        return 16;
    }

    //==============================================================================
    String error;
    double sampleRate;
    int bufferSize, outputLatency, inputLatency;
    BigInteger currentInputChans, currentOutputChans;

    Array <int> sampleRates;
    StringArray channelNamesOut, channelNamesIn;
    AudioIODeviceCallback* callback;

private:
    //==============================================================================
    const String inputId, outputId;
    ScopedPointer<ALSADevice> outputDevice, inputDevice;
    int numCallbacks;

    CriticalSection callbackLock;

    AudioSampleBuffer inputChannelBuffer, outputChannelBuffer;
    Array<float*> inputChannelDataForCallback, outputChannelDataForCallback;

    unsigned int minChansOut, maxChansOut;
    unsigned int minChansIn, maxChansIn;

    bool failed (const int errorNum)
    {
        if (errorNum >= 0)
            return false;

        error = snd_strerror (errorNum);
        DBG ("ALSA error: " + error + "\n");
        return true;
    }

    void initialiseRatesAndChannels()
    {
        sampleRates.clear();
        channelNamesOut.clear();
        channelNamesIn.clear();
        minChansOut = 0;
        maxChansOut = 0;
        minChansIn = 0;
        maxChansIn = 0;
        unsigned int dummy = 0;

        getDeviceProperties (inputId, dummy, dummy, minChansIn, maxChansIn, sampleRates);
        getDeviceProperties (outputId, minChansOut, maxChansOut, dummy, dummy, sampleRates);

        for (unsigned int i = 0; i < maxChansOut; ++i)
            channelNamesOut.add ("channel " + String ((int) i + 1));

        for (unsigned int i = 0; i < maxChansIn; ++i)
            channelNamesIn.add ("channel " + String ((int) i + 1));
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ALSAThread)
};


//==============================================================================
class ALSAAudioIODevice   : public AudioIODevice
{
public:
    ALSAAudioIODevice (const String& deviceName,
                       const String& inputId_,
                       const String& outputId_)
        : AudioIODevice (deviceName, "ALSA"),
          inputId (inputId_),
          outputId (outputId_),
          isOpen_ (false),
          isStarted (false),
          internal (inputId_, outputId_)
    {
    }

    ~ALSAAudioIODevice()
    {
        close();
    }

    StringArray getOutputChannelNames()             { return internal.channelNamesOut; }
    StringArray getInputChannelNames()              { return internal.channelNamesIn; }

    int getNumSampleRates()                         { return internal.sampleRates.size(); }
    double getSampleRate (int index)                { return internal.sampleRates [index]; }

    int getDefaultBufferSize()                      { return 512; }
    int getNumBufferSizesAvailable()                { return 50; }

    int getBufferSizeSamples (int index)
    {
        int n = 16;
        for (int i = 0; i < index; ++i)
            n += n < 64 ? 16
                        : (n < 512 ? 32
                                   : (n < 1024 ? 64
                                               : (n < 2048 ? 128 : 256)));

        return n;
    }

    String open (const BigInteger& inputChannels,
                 const BigInteger& outputChannels,
                 double sampleRate,
                 int bufferSizeSamples)
    {
        close();

        if (bufferSizeSamples <= 0)
            bufferSizeSamples = getDefaultBufferSize();

        if (sampleRate <= 0)
        {
            for (int i = 0; i < getNumSampleRates(); ++i)
            {
                if (getSampleRate (i) >= 44100)
                {
                    sampleRate = getSampleRate (i);
                    break;
                }
            }
        }

        internal.open (inputChannels, outputChannels,
                       sampleRate, bufferSizeSamples);

        isOpen_ = internal.error.isEmpty();
        return internal.error;
    }

    void close()
    {
        stop();
        internal.close();
        isOpen_ = false;
    }

    bool isOpen()                           { return isOpen_; }
    bool isPlaying()                        { return isStarted && internal.error.isEmpty(); }
    String getLastError()                   { return internal.error; }

    int getCurrentBufferSizeSamples()       { return internal.bufferSize; }
    double getCurrentSampleRate()           { return internal.sampleRate; }
    int getCurrentBitDepth()                { return internal.getBitDepth(); }

    BigInteger getActiveOutputChannels() const    { return internal.currentOutputChans; }
    BigInteger getActiveInputChannels() const     { return internal.currentInputChans; }

    int getOutputLatencyInSamples()         { return internal.outputLatency; }
    int getInputLatencyInSamples()          { return internal.inputLatency; }

    void start (AudioIODeviceCallback* callback)
    {
        if (! isOpen_)
            callback = nullptr;

        if (callback != nullptr)
            callback->audioDeviceAboutToStart (this);

        internal.setCallback (callback);

        isStarted = (callback != nullptr);
    }

    void stop()
    {
        AudioIODeviceCallback* const oldCallback = internal.callback;

        start (0);

        if (oldCallback != nullptr)
            oldCallback->audioDeviceStopped();
    }

    String inputId, outputId;

private:
    bool isOpen_, isStarted;
    ALSAThread internal;
};


//==============================================================================
class ALSAAudioIODeviceType  : public AudioIODeviceType
{
public:
    //==============================================================================
    ALSAAudioIODeviceType()
        : AudioIODeviceType ("ALSA"),
          hasScanned (false)
    {
    }

    ~ALSAAudioIODeviceType()
    {
    }

    //==============================================================================
    void scanForDevices()
    {
        if (hasScanned)
            return;

        hasScanned = true;
        inputNames.clear();
        inputIds.clear();
        outputNames.clear();
        outputIds.clear();

/*        void** hints = 0;
        if (snd_device_name_hint (-1, "pcm", &hints) >= 0)
        {
            for (void** hint = hints; *hint != 0; ++hint)
            {
                const String name (getHint (*hint, "NAME"));

                if (name.isNotEmpty())
                {
                    const String ioid (getHint (*hint, "IOID"));

                    String desc (getHint (*hint, "DESC"));
                    if (desc.isEmpty())
                        desc = name;

                    desc = desc.replaceCharacters ("\n\r", "  ");

                    DBG ("name: " << name << "\ndesc: " << desc << "\nIO: " << ioid);

                    if (ioid.isEmpty() || ioid == "Input")
                    {
                        inputNames.add (desc);
                        inputIds.add (name);
                    }

                    if (ioid.isEmpty() || ioid == "Output")
                    {
                        outputNames.add (desc);
                        outputIds.add (name);
                    }
                }
            }

            snd_device_name_free_hint (hints);
        }
*/
        snd_ctl_t* handle = nullptr;
        snd_ctl_card_info_t* info = nullptr;
        snd_ctl_card_info_alloca (&info);

        int cardNum = -1;

        while (outputIds.size() + inputIds.size() <= 32)
        {
            snd_card_next (&cardNum);

            if (cardNum < 0)
                break;

            if (snd_ctl_open (&handle, ("hw:" + String (cardNum)).toUTF8(), SND_CTL_NONBLOCK) >= 0)
            {
                if (snd_ctl_card_info (handle, info) >= 0)
                {
                    String cardId (snd_ctl_card_info_get_id (info));

                    if (cardId.removeCharacters ("0123456789").isEmpty())
                        cardId = String (cardNum);

                    int device = -1;

                    for (;;)
                    {
                        if (snd_ctl_pcm_next_device (handle, &device) < 0 || device < 0)
                            break;

                        String id, name;
                        id << "hw:" << cardId << ',' << device;

                        bool isInput, isOutput;

                        if (testDevice (id, isInput, isOutput))
                        {
                            name << snd_ctl_card_info_get_name (info);

                            if (name.isEmpty())
                                name = id;

                            if (isInput)
                            {
                                inputNames.add (name);
                                inputIds.add (id);
                            }

                            if (isOutput)
                            {
                                outputNames.add (name);
                                outputIds.add (id);
                            }
                        }
                    }
                }

                snd_ctl_close (handle);
            }
        }

        inputNames.appendNumbersToDuplicates (false, true);
        outputNames.appendNumbersToDuplicates (false, true);
    }

    StringArray getDeviceNames (bool wantInputNames) const
    {
        jassert (hasScanned); // need to call scanForDevices() before doing this

        return wantInputNames ? inputNames : outputNames;
    }

    int getDefaultDeviceIndex (bool /* forInput */) const
    {
        jassert (hasScanned); // need to call scanForDevices() before doing this
        return 0;
    }

    bool hasSeparateInputsAndOutputs() const    { return true; }

    int getIndexOfDevice (AudioIODevice* device, bool asInput) const
    {
        jassert (hasScanned); // need to call scanForDevices() before doing this

        ALSAAudioIODevice* d = dynamic_cast <ALSAAudioIODevice*> (device);
        if (d == nullptr)
            return -1;

        return asInput ? inputIds.indexOf (d->inputId)
                       : outputIds.indexOf (d->outputId);
    }

    AudioIODevice* createDevice (const String& outputDeviceName,
                                 const String& inputDeviceName)
    {
        jassert (hasScanned); // need to call scanForDevices() before doing this

        const int inputIndex = inputNames.indexOf (inputDeviceName);
        const int outputIndex = outputNames.indexOf (outputDeviceName);

        String deviceName (outputIndex >= 0 ? outputDeviceName
                                            : inputDeviceName);

        if (inputIndex >= 0 || outputIndex >= 0)
            return new ALSAAudioIODevice (deviceName,
                                          inputIds [inputIndex],
                                          outputIds [outputIndex]);

        return nullptr;
    }

    //==============================================================================
private:
    StringArray inputNames, outputNames, inputIds, outputIds;
    bool hasScanned;

    static bool testDevice (const String& id, bool& isInput, bool& isOutput)
    {
        unsigned int minChansOut = 0, maxChansOut = 0;
        unsigned int minChansIn = 0, maxChansIn = 0;
        Array <int> rates;

        getDeviceProperties (id, minChansOut, maxChansOut, minChansIn, maxChansIn, rates);

        DBG ("ALSA device: " + id
              + " outs=" + String ((int) minChansOut) + "-" + String ((int) maxChansOut)
              + " ins=" + String ((int) minChansIn) + "-" + String ((int) maxChansIn)
              + " rates=" + String (rates.size()));

        isInput = maxChansIn > 0;
        isOutput = maxChansOut > 0;

        return (isInput || isOutput) && rates.size() > 0;
    }

    /*static String getHint (void* hint, const char* type)
    {
        char* const n = snd_device_name_get_hint (hint, type);
        const String s ((const char*) n);
        free (n);
        return s;
    }*/

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ALSAAudioIODeviceType)
};

//==============================================================================
AudioIODeviceType* AudioIODeviceType::createAudioIODeviceType_ALSA()
{
    return new ALSAAudioIODeviceType();
}

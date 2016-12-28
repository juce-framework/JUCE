/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2016 - ROLI Ltd.

   Permission is granted to use this software under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license/

   Permission to use, copy, modify, and/or distribute this software for any
   purpose with or without fee is hereby granted, provided that the above
   copyright notice and this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND ISC DISCLAIMS ALL WARRANTIES WITH REGARD
   TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
   FITNESS. IN NO EVENT SHALL ISC BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT,
   OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
   USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
   TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
   OF THIS SOFTWARE.

   -----------------------------------------------------------------------------

   To release a closed-source product which uses other parts of JUCE not
   licensed under the ISC terms, commercial licenses are available: visit
   www.juce.com for more information.

  ==============================================================================
*/

#undef check

const char* const openSLTypeName = "Android OpenSL";

bool isOpenSLAvailable()
{
    DynamicLibrary library;
    return library.open ("libOpenSLES.so");
}

//==============================================================================
class OpenSLAudioIODevice  : public AudioIODevice,
                             private Thread
{
public:
    OpenSLAudioIODevice (const String& deviceName)
        : AudioIODevice (deviceName, openSLTypeName),
          Thread ("OpenSL"),
          callback (nullptr), sampleRate (0), deviceOpen (false),
          inputBuffer (2, 2), outputBuffer (2, 2)
    {
        // OpenSL has piss-poor support for determining latency, so the only way I can find to
        // get a number for this is by asking the AudioTrack/AudioRecord classes..
        AndroidAudioIODevice javaDevice (deviceName);

        // this is a total guess about how to calculate the latency, but seems to vaguely agree
        // with the devices I've tested.. YMMV
        inputLatency  = (javaDevice.minBufferSizeIn  * 2) / 3;
        outputLatency = (javaDevice.minBufferSizeOut * 2) / 3;

        const int64 longestLatency = jmax (inputLatency, outputLatency);
        const int64 totalLatency = inputLatency + outputLatency;
        inputLatency  = (int) ((longestLatency * inputLatency)  / totalLatency) & ~15;
        outputLatency = (int) ((longestLatency * outputLatency) / totalLatency) & ~15;
    }

    ~OpenSLAudioIODevice()
    {
        close();
    }

    bool openedOk() const       { return engine.outputMixObject != nullptr; }

    StringArray getOutputChannelNames() override
    {
        StringArray s;
        s.add ("Left");
        s.add ("Right");
        return s;
    }

    StringArray getInputChannelNames() override
    {
        StringArray s;
        s.add ("Audio Input");
        return s;
    }

    Array<double> getAvailableSampleRates() override
    {
        static const double rates[] = { 8000.0, 16000.0, 32000.0, 44100.0, 48000.0 };
        Array<double> retval (rates, numElementsInArray (rates));

        // make sure the native sample rate is pafrt of the list
        double native = getNativeSampleRate();
        if (native != 0.0 && ! retval.contains (native))
            retval.add (native);

        return retval;
    }

    Array<int> getAvailableBufferSizes() override
    {
        // we need to offer the lowest possible buffer size which
        // is the native buffer size
        const int defaultNumMultiples = 8;
        const int nativeBufferSize = getNativeBufferSize();

        Array<int> retval;
        for (int i = 1; i < defaultNumMultiples; ++i)
            retval.add (i * nativeBufferSize);

        return retval;
    }

    String open (const BigInteger& inputChannels,
                 const BigInteger& outputChannels,
                 double requestedSampleRate,
                 int bufferSize) override
    {
        close();

        lastError.clear();
        sampleRate = (int) requestedSampleRate;

        int preferredBufferSize = (bufferSize <= 0) ? getDefaultBufferSize() : bufferSize;

        activeOutputChans = outputChannels;
        activeOutputChans.setRange (2, activeOutputChans.getHighestBit(), false);
        numOutputChannels = activeOutputChans.countNumberOfSetBits();

        activeInputChans = inputChannels;
        activeInputChans.setRange (1, activeInputChans.getHighestBit(), false);
        numInputChannels = activeInputChans.countNumberOfSetBits();

        actualBufferSize = preferredBufferSize;

        inputBuffer.setSize  (jmax (1, numInputChannels),  actualBufferSize);
        outputBuffer.setSize (jmax (1, numOutputChannels), actualBufferSize);
        outputBuffer.clear();

        const int audioBuffersToEnqueue = hasLowLatencyAudioPath() ? buffersToEnqueueForLowLatency
                                                                   : buffersToEnqueueSlowAudio;

        DBG ("OpenSL: numInputChannels = " << numInputChannels
              << ", numOutputChannels = " << numOutputChannels
              << ", nativeBufferSize = " << getNativeBufferSize()
              << ", nativeSampleRate = " << getNativeSampleRate()
              << ", actualBufferSize = " << actualBufferSize
              << ", audioBuffersToEnqueue = " << audioBuffersToEnqueue
              << ", sampleRate = " << sampleRate);

        if (numInputChannels > 0)
        {
            if (! RuntimePermissions::isGranted (RuntimePermissions::recordAudio))
            {
                // If you hit this assert, you probably forgot to get RuntimePermissions::recordAudio
                // before trying to open an audio input device. This is not going to work!
                jassertfalse;
                lastError = "Error opening OpenSL input device: the app was not granted android.permission.RECORD_AUDIO";
            }
            else
            {
                recorder = engine.createRecorder (numInputChannels,  sampleRate,
                                                  audioBuffersToEnqueue, actualBufferSize);

                if (recorder == nullptr)
                    lastError = "Error opening OpenSL input device: creating Recorder failed.";
            }
        }

        if (numOutputChannels > 0)
        {
            player = engine.createPlayer   (numOutputChannels, sampleRate,
                                            audioBuffersToEnqueue, actualBufferSize);

            if (player == nullptr)
                lastError = "Error opening OpenSL input device: creating Player failed.";
        }

        // pre-fill buffers
        for (int i = 0; i < audioBuffersToEnqueue; ++i)
            processBuffers();

        startThread (8);

        deviceOpen = true;
        return lastError;
    }

    void close() override
    {
        stop();
        stopThread (6000);
        deviceOpen = false;
        recorder = nullptr;
        player = nullptr;
    }

    int getOutputLatencyInSamples() override            { return outputLatency; }
    int getInputLatencyInSamples() override             { return inputLatency; }
    bool isOpen() override                              { return deviceOpen; }
    int getCurrentBufferSizeSamples() override          { return actualBufferSize; }
    int getCurrentBitDepth() override                   { return 16; }
    BigInteger getActiveOutputChannels() const override { return activeOutputChans; }
    BigInteger getActiveInputChannels() const override  { return activeInputChans; }
    String getLastError() override                      { return lastError; }
    bool isPlaying() override                           { return callback != nullptr; }

    int getDefaultBufferSize() override
    {
        // Only on a Pro-Audio device will we set the lowest possible buffer size
        // by default. We need to be more conservative on other devices
        // as they may be low-latency, but still have a crappy CPU.
        return (isProAudioDevice() ? 1 : 6)
                 * defaultBufferSizeIsMultipleOfNative * getNativeBufferSize();
    }

    double getCurrentSampleRate() override
    {
        return (sampleRate == 0.0 ? getNativeSampleRate() : sampleRate);
    }

    void start (AudioIODeviceCallback* newCallback) override
    {
        stop();

        if (deviceOpen && callback != newCallback)
        {
            if (newCallback != nullptr)
                newCallback->audioDeviceAboutToStart (this);

            setCallback (newCallback);
        }
    }

    void stop() override
    {
        if (AudioIODeviceCallback* const oldCallback = setCallback (nullptr))
            oldCallback->audioDeviceStopped();
    }

    bool setAudioPreprocessingEnabled (bool enable) override
    {
        return recorder != nullptr && recorder->setAudioPreprocessingEnabled (enable);
    }

private:
    //==============================================================================
    CriticalSection callbackLock;
    AudioIODeviceCallback* callback;
    int actualBufferSize, sampleRate;
    int inputLatency, outputLatency;
    bool deviceOpen;
    String lastError;
    BigInteger activeOutputChans, activeInputChans;
    int numInputChannels, numOutputChannels;
    AudioSampleBuffer inputBuffer, outputBuffer;
    struct Player;
    struct Recorder;

    enum
    {
        // The number of buffers to enqueue needs to be at least two for the audio to use the low-latency
        // audio path (see "Performance" section in ndk/docs/Additional_library_docs/opensles/index.html)
        buffersToEnqueueForLowLatency = 2,
        buffersToEnqueueSlowAudio = 4,
        defaultBufferSizeIsMultipleOfNative = 1
    };

    //==============================================================================
    static String audioManagerGetProperty (const String& property)
    {
        const LocalRef<jstring> jProperty (javaString (property));
        const LocalRef<jstring> text ((jstring) android.activity.callObjectMethod (JuceAppActivity.audioManagerGetProperty,
                                                                                   jProperty.get()));
        if (text.get() != 0)
            return juceString (text);

        return String();
    }

    static bool androidHasSystemFeature (const String& property)
    {
        const LocalRef<jstring> jProperty (javaString (property));
        return android.activity.callBooleanMethod (JuceAppActivity.hasSystemFeature, jProperty.get());
    }

    static double getNativeSampleRate()
    {
        return audioManagerGetProperty ("android.media.property.OUTPUT_SAMPLE_RATE").getDoubleValue();
    }

    static int getNativeBufferSize()
    {
        const int val = audioManagerGetProperty ("android.media.property.OUTPUT_FRAMES_PER_BUFFER").getIntValue();
        return val > 0 ? val : 512;
    }

    static bool isProAudioDevice()
    {
        return androidHasSystemFeature ("android.hardware.audio.pro");
    }

    static bool hasLowLatencyAudioPath()
    {
        return androidHasSystemFeature ("android.hardware.audio.low_latency");
    }

    //==============================================================================
    AudioIODeviceCallback* setCallback (AudioIODeviceCallback* const newCallback)
    {
        const ScopedLock sl (callbackLock);
        AudioIODeviceCallback* const oldCallback = callback;
        callback = newCallback;
        return oldCallback;
    }

    void processBuffers()
    {
        if (recorder != nullptr)
            recorder->readNextBlock (inputBuffer, *this);

        {
            const ScopedLock sl (callbackLock);

            if (callback != nullptr)
                callback->audioDeviceIOCallback (numInputChannels  > 0 ? inputBuffer.getArrayOfReadPointers()   : nullptr, numInputChannels,
                                                 numOutputChannels > 0 ? outputBuffer.getArrayOfWritePointers() : nullptr, numOutputChannels,
                                                 actualBufferSize);
            else
                outputBuffer.clear();
        }

        if (player != nullptr)
            player->writeBuffer (outputBuffer, *this);
    }

    void run() override
    {
        setThreadToAudioPriority();

        if (recorder != nullptr)    recorder->start();
        if (player != nullptr)      player->start();

        while (! threadShouldExit())
            processBuffers();
    }

    void setThreadToAudioPriority()
    {
        // see android.os.Process.THREAD_PRIORITY_AUDIO
        const int THREAD_PRIORITY_AUDIO = -16;
        jint priority = THREAD_PRIORITY_AUDIO;

        if (priority != android.activity.callIntMethod (JuceAppActivity.setCurrentThreadPriority, (jint) priority))
            DBG ("Unable to set audio thread priority: priority is still " << priority);
    }

    //==============================================================================
    struct Engine
    {
        Engine()
            : engineObject (nullptr), engineInterface (nullptr), outputMixObject (nullptr)
        {
            if (library.open ("libOpenSLES.so"))
            {
                typedef SLresult (*CreateEngineFunc) (SLObjectItf*, SLuint32, const SLEngineOption*,
                                                      SLuint32, const SLInterfaceID*, const SLboolean*);

                if (CreateEngineFunc createEngine = (CreateEngineFunc) library.getFunction ("slCreateEngine"))
                {
                    check (createEngine (&engineObject, 0, nullptr, 0, nullptr, nullptr));

                    SLInterfaceID* SL_IID_ENGINE    = (SLInterfaceID*) library.getFunction ("SL_IID_ENGINE");
                    SL_IID_ANDROIDSIMPLEBUFFERQUEUE = (SLInterfaceID*) library.getFunction ("SL_IID_ANDROIDSIMPLEBUFFERQUEUE");
                    SL_IID_PLAY                     = (SLInterfaceID*) library.getFunction ("SL_IID_PLAY");
                    SL_IID_RECORD                   = (SLInterfaceID*) library.getFunction ("SL_IID_RECORD");
                    SL_IID_ANDROIDCONFIGURATION     = (SLInterfaceID*) library.getFunction ("SL_IID_ANDROIDCONFIGURATION");

                    check ((*engineObject)->Realize (engineObject, SL_BOOLEAN_FALSE));
                    check ((*engineObject)->GetInterface (engineObject, *SL_IID_ENGINE, &engineInterface));

                    check ((*engineInterface)->CreateOutputMix (engineInterface, &outputMixObject, 0, nullptr, nullptr));
                    check ((*outputMixObject)->Realize (outputMixObject, SL_BOOLEAN_FALSE));
                }
            }
        }

        ~Engine()
        {
            if (outputMixObject != nullptr) (*outputMixObject)->Destroy (outputMixObject);
            if (engineObject != nullptr)    (*engineObject)->Destroy (engineObject);
        }

        Player* createPlayer (const int numChannels, const int sampleRate, const int numBuffers, const int bufferSize)
        {
            if (numChannels <= 0)
                return nullptr;

            ScopedPointer<Player> player (new Player (numChannels, sampleRate, *this, numBuffers, bufferSize));
            return player->openedOk() ? player.release() : nullptr;
        }

        Recorder* createRecorder (const int numChannels, const int sampleRate, const int numBuffers, const int bufferSize)
        {
            if (numChannels <= 0)
                return nullptr;

            ScopedPointer<Recorder> recorder (new Recorder (numChannels, sampleRate, *this, numBuffers, bufferSize));
            return recorder->openedOk() ? recorder.release() : nullptr;
        }

        SLObjectItf engineObject;
        SLEngineItf engineInterface;
        SLObjectItf outputMixObject;

        SLInterfaceID* SL_IID_ANDROIDSIMPLEBUFFERQUEUE;
        SLInterfaceID* SL_IID_PLAY;
        SLInterfaceID* SL_IID_RECORD;
        SLInterfaceID* SL_IID_ANDROIDCONFIGURATION;

    private:
        DynamicLibrary library;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Engine)
    };

    //==============================================================================
    struct BufferList
    {
        BufferList (const int numChannels_, const int numBuffers_, const int numSamples_)
            : numChannels (numChannels_), numBuffers (numBuffers_), numSamples (numSamples_),
              bufferSpace (numChannels_ * numSamples * numBuffers), nextBlock (0)
        {
        }

        int16* waitForFreeBuffer (Thread& threadToCheck) noexcept
        {
            while (numBlocksOut.get() == numBuffers)
            {
                dataArrived.wait (1);

                if (threadToCheck.threadShouldExit())
                    return nullptr;
            }

            return getNextBuffer();
        }

        int16* getNextBuffer() noexcept
        {
            if (++nextBlock == numBuffers)
                nextBlock = 0;

            return bufferSpace + nextBlock * numChannels * numSamples;
        }

        void bufferReturned() noexcept      { --numBlocksOut; dataArrived.signal(); }
        void bufferSent() noexcept          { ++numBlocksOut; dataArrived.signal(); }

        int getBufferSizeBytes() const noexcept     { return numChannels * numSamples * sizeof (int16); }

        const int numChannels, numBuffers, numSamples;

    private:
        HeapBlock<int16> bufferSpace;
        int nextBlock;
        Atomic<int> numBlocksOut;
        WaitableEvent dataArrived;
    };

    //==============================================================================
    struct Player
    {
        Player (int numChannels, int sampleRate, Engine& engine, int playerNumBuffers, int playerBufferSize)
            : playerObject (nullptr), playerPlay (nullptr), playerBufferQueue (nullptr),
              bufferList (numChannels, playerNumBuffers, playerBufferSize)
        {
            SLDataFormat_PCM pcmFormat =
            {
                SL_DATAFORMAT_PCM,
                (SLuint32) numChannels,
                (SLuint32) (sampleRate * 1000),
                SL_PCMSAMPLEFORMAT_FIXED_16,
                SL_PCMSAMPLEFORMAT_FIXED_16,
                (numChannels == 1) ? SL_SPEAKER_FRONT_CENTER : (SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT),
                SL_BYTEORDER_LITTLEENDIAN
            };

            SLDataLocator_AndroidSimpleBufferQueue bufferQueue = { SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE,
                                                                   static_cast<SLuint32> (bufferList.numBuffers) };
            SLDataSource audioSrc = { &bufferQueue, &pcmFormat };

            SLDataLocator_OutputMix outputMix = { SL_DATALOCATOR_OUTPUTMIX, engine.outputMixObject };
            SLDataSink audioSink = { &outputMix, nullptr };

            // (SL_IID_BUFFERQUEUE is not guaranteed to remain future-proof, so use SL_IID_ANDROIDSIMPLEBUFFERQUEUE)
            const SLInterfaceID interfaceIDs[] = { *engine.SL_IID_ANDROIDSIMPLEBUFFERQUEUE };
            const SLboolean flags[] = { SL_BOOLEAN_TRUE };

            check ((*engine.engineInterface)->CreateAudioPlayer (engine.engineInterface, &playerObject, &audioSrc, &audioSink,
                                                                 1, interfaceIDs, flags));

            check ((*playerObject)->Realize (playerObject, SL_BOOLEAN_FALSE));
            check ((*playerObject)->GetInterface (playerObject, *engine.SL_IID_PLAY, &playerPlay));
            check ((*playerObject)->GetInterface (playerObject, *engine.SL_IID_ANDROIDSIMPLEBUFFERQUEUE, &playerBufferQueue));
            check ((*playerBufferQueue)->RegisterCallback (playerBufferQueue, staticCallback, this));
        }

        ~Player()
        {
            if (playerPlay != nullptr)
                check ((*playerPlay)->SetPlayState (playerPlay, SL_PLAYSTATE_STOPPED));

            if (playerBufferQueue != nullptr)
                check ((*playerBufferQueue)->Clear (playerBufferQueue));

            if (playerObject != nullptr)
                (*playerObject)->Destroy (playerObject);
        }

        bool openedOk() const noexcept      { return playerBufferQueue != nullptr; }

        void start()
        {
            jassert (openedOk());

            check ((*playerPlay)->SetPlayState (playerPlay, SL_PLAYSTATE_PLAYING));
        }

        void writeBuffer (const AudioSampleBuffer& buffer, Thread& thread) noexcept
        {
            jassert (buffer.getNumChannels() == bufferList.numChannels);
            jassert (buffer.getNumSamples() < bufferList.numSamples * bufferList.numBuffers);

            int offset = 0;
            int numSamples = buffer.getNumSamples();

            while (numSamples > 0)
            {
                if (int16* const destBuffer = bufferList.waitForFreeBuffer (thread))
                {
                    for (int i = 0; i < bufferList.numChannels; ++i)
                    {
                        typedef AudioData::Pointer<AudioData::Int16,   AudioData::LittleEndian, AudioData::Interleaved, AudioData::NonConst> DstSampleType;
                        typedef AudioData::Pointer<AudioData::Float32, AudioData::NativeEndian, AudioData::NonInterleaved, AudioData::Const> SrcSampleType;

                        DstSampleType dstData (destBuffer + i, bufferList.numChannels);
                        SrcSampleType srcData (buffer.getReadPointer (i, offset));
                        dstData.convertSamples (srcData, bufferList.numSamples);
                    }

                    enqueueBuffer (destBuffer);

                    numSamples -= bufferList.numSamples;
                    offset += bufferList.numSamples;
                }
                else
                {
                    break;
                }
            }
        }

    private:
        SLObjectItf playerObject;
        SLPlayItf playerPlay;
        SLAndroidSimpleBufferQueueItf playerBufferQueue;

        BufferList bufferList;

        void enqueueBuffer (int16* buffer) noexcept
        {
            check ((*playerBufferQueue)->Enqueue (playerBufferQueue, buffer, bufferList.getBufferSizeBytes()));
            bufferList.bufferSent();
        }

        static void staticCallback (SLAndroidSimpleBufferQueueItf queue, void* context) noexcept
        {
            jassert (queue == static_cast<Player*> (context)->playerBufferQueue); ignoreUnused (queue);
            static_cast<Player*> (context)->bufferList.bufferReturned();
        }

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Player)
    };

    //==============================================================================
    struct Recorder
    {
        Recorder (int numChannels, int sampleRate, Engine& engine, const int numBuffers, const int numSamples)
            : recorderObject (nullptr), recorderRecord (nullptr),
              recorderBufferQueue (nullptr), configObject (nullptr),
              bufferList (numChannels, numBuffers, numSamples)
        {
            SLDataFormat_PCM pcmFormat =
            {
                SL_DATAFORMAT_PCM,
                (SLuint32) numChannels,
                (SLuint32) (sampleRate * 1000), // (sample rate units are millihertz)
                SL_PCMSAMPLEFORMAT_FIXED_16,
                SL_PCMSAMPLEFORMAT_FIXED_16,
                (numChannels == 1) ? SL_SPEAKER_FRONT_CENTER : (SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT),
                SL_BYTEORDER_LITTLEENDIAN
            };

            SLDataLocator_IODevice ioDevice = { SL_DATALOCATOR_IODEVICE, SL_IODEVICE_AUDIOINPUT, SL_DEFAULTDEVICEID_AUDIOINPUT, nullptr };
            SLDataSource audioSrc = { &ioDevice, nullptr };

            SLDataLocator_AndroidSimpleBufferQueue bufferQueue = { SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE,
                                                                   static_cast<SLuint32> (bufferList.numBuffers) };
            SLDataSink audioSink = { &bufferQueue, &pcmFormat };

            const SLInterfaceID interfaceIDs[] = { *engine.SL_IID_ANDROIDSIMPLEBUFFERQUEUE };
            const SLboolean flags[] = { SL_BOOLEAN_TRUE };

            if (check ((*engine.engineInterface)->CreateAudioRecorder (engine.engineInterface, &recorderObject, &audioSrc,
                                                                       &audioSink, 1, interfaceIDs, flags)))
            {
                if (check ((*recorderObject)->Realize (recorderObject, SL_BOOLEAN_FALSE)))
                {
                    check ((*recorderObject)->GetInterface (recorderObject, *engine.SL_IID_RECORD, &recorderRecord));
                    check ((*recorderObject)->GetInterface (recorderObject, *engine.SL_IID_ANDROIDSIMPLEBUFFERQUEUE, &recorderBufferQueue));
                    // not all android versions seem to have a config object
                    SLresult result = (*recorderObject)->GetInterface (recorderObject,
                                                                       *engine.SL_IID_ANDROIDCONFIGURATION, &configObject);
                    if (result != SL_RESULT_SUCCESS)
                        configObject = nullptr;

                    check ((*recorderBufferQueue)->RegisterCallback (recorderBufferQueue, staticCallback, this));
                    check ((*recorderRecord)->SetRecordState (recorderRecord, SL_RECORDSTATE_STOPPED));
                }
            }
        }

        ~Recorder()
        {
            if (recorderRecord != nullptr)
                check ((*recorderRecord)->SetRecordState (recorderRecord, SL_RECORDSTATE_STOPPED));

            if (recorderBufferQueue != nullptr)
                check ((*recorderBufferQueue)->Clear (recorderBufferQueue));

            if (recorderObject != nullptr)
                (*recorderObject)->Destroy (recorderObject);
        }

        bool openedOk() const noexcept      { return recorderBufferQueue != nullptr; }

        void start()
        {
            jassert (openedOk());
            check ((*recorderRecord)->SetRecordState (recorderRecord, SL_RECORDSTATE_RECORDING));
        }

        void readNextBlock (AudioSampleBuffer& buffer, Thread& thread)
        {
            jassert (buffer.getNumChannels() == bufferList.numChannels);
            jassert (buffer.getNumSamples() < bufferList.numSamples * bufferList.numBuffers);
            jassert ((buffer.getNumSamples() % bufferList.numSamples) == 0);

            int offset = 0;
            int numSamples = buffer.getNumSamples();

            while (numSamples > 0)
            {
                if (int16* const srcBuffer = bufferList.waitForFreeBuffer (thread))
                {
                    for (int i = 0; i < bufferList.numChannels; ++i)
                    {
                        typedef AudioData::Pointer<AudioData::Float32, AudioData::NativeEndian, AudioData::NonInterleaved, AudioData::NonConst> DstSampleType;
                        typedef AudioData::Pointer<AudioData::Int16,   AudioData::LittleEndian, AudioData::Interleaved, AudioData::Const> SrcSampleType;

                        DstSampleType dstData (buffer.getWritePointer (i, offset));
                        SrcSampleType srcData (srcBuffer + i, bufferList.numChannels);
                        dstData.convertSamples (srcData, bufferList.numSamples);
                    }

                    enqueueBuffer (srcBuffer);

                    numSamples -= bufferList.numSamples;
                    offset += bufferList.numSamples;
                }
                else
                {
                    break;
                }
            }
        }

        bool setAudioPreprocessingEnabled (bool enable)
        {
            SLuint32 mode = enable ? SL_ANDROID_RECORDING_PRESET_GENERIC
                                   : SL_ANDROID_RECORDING_PRESET_VOICE_RECOGNITION;

            return configObject != nullptr
                     && check ((*configObject)->SetConfiguration (configObject, SL_ANDROID_KEY_RECORDING_PRESET, &mode, sizeof (mode)));
        }

    private:
        SLObjectItf recorderObject;
        SLRecordItf recorderRecord;
        SLAndroidSimpleBufferQueueItf recorderBufferQueue;
        SLAndroidConfigurationItf configObject;

        BufferList bufferList;

        void enqueueBuffer (int16* buffer) noexcept
        {
            check ((*recorderBufferQueue)->Enqueue (recorderBufferQueue, buffer, bufferList.getBufferSizeBytes()));
            bufferList.bufferSent();
        }

        static void staticCallback (SLAndroidSimpleBufferQueueItf queue, void* context) noexcept
        {
            jassert (queue == static_cast<Recorder*> (context)->recorderBufferQueue); ignoreUnused (queue);
            static_cast<Recorder*> (context)->bufferList.bufferReturned();
        }

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Recorder)
    };


    //==============================================================================
    Engine engine;

    ScopedPointer<Player> player;
    ScopedPointer<Recorder> recorder;

    //==============================================================================
    static bool check (const SLresult result) noexcept
    {
        jassert (result == SL_RESULT_SUCCESS);
        return result == SL_RESULT_SUCCESS;
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OpenSLAudioIODevice)
};


//==============================================================================
class OpenSLAudioDeviceType  : public AudioIODeviceType
{
public:
    OpenSLAudioDeviceType()  : AudioIODeviceType (openSLTypeName) {}

    //==============================================================================
    void scanForDevices() override {}
    StringArray getDeviceNames (bool wantInputNames) const override              { return StringArray (openSLTypeName); }
    int getDefaultDeviceIndex (bool forInput) const override                     { return 0; }
    int getIndexOfDevice (AudioIODevice* device, bool asInput) const override    { return device != nullptr ? 0 : -1; }
    bool hasSeparateInputsAndOutputs() const override                            { return false; }

    AudioIODevice* createDevice (const String& outputDeviceName,
                                 const String& inputDeviceName) override
    {
        ScopedPointer<OpenSLAudioIODevice> dev;

        if (outputDeviceName.isNotEmpty() || inputDeviceName.isNotEmpty())
        {
            dev = new OpenSLAudioIODevice (outputDeviceName.isNotEmpty() ? outputDeviceName
                                                                         : inputDeviceName);
            if (! dev->openedOk())
                dev = nullptr;
        }

        return dev.release();
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OpenSLAudioDeviceType)
};


//==============================================================================
AudioIODeviceType* AudioIODeviceType::createAudioIODeviceType_OpenSLES()
{
    return isOpenSLAvailable() ? new OpenSLAudioDeviceType() : nullptr;
}

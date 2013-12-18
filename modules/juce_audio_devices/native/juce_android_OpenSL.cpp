/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

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

const char* const openSLTypeName = "Android OpenSL";

bool isOpenSLAvailable()
{
    DynamicLibrary library;
    return library.open ("libOpenSLES.so");
}

//==============================================================================
class OpenSLAudioIODevice  : public AudioIODevice,
                             public Thread
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
        AndroidAudioIODevice javaDevice (String::empty);

        // this is a total guess about how to calculate the latency, but seems to vaguely agree
        // with the devices I've tested.. YMMV
        inputLatency  = ((javaDevice.minBufferSizeIn  * 2) / 3);
        outputLatency = ((javaDevice.minBufferSizeOut * 2) / 3);

        const int longestLatency = jmax (inputLatency, outputLatency);
        const int totalLatency = inputLatency + outputLatency;
        inputLatency  = ((longestLatency * inputLatency)  / totalLatency) & ~15;
        outputLatency = ((longestLatency * outputLatency) / totalLatency) & ~15;
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
        static const double rates[]       = { 8000.0, 16000.0, 32000.0, 44100.0, 48000.0 };
        return Array<double> (rates, numElementsInArray (rates));
    }

    Array<int> getAvailableBufferSizes() override
    {
        static const int sizes[] = { 256, 512, 768, 1024, 1280, 1600 }; // must all be multiples of the block size
        return Array<int> (sizes, numElementsInArray (sizes));
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

        recorder = engine.createRecorder (numInputChannels,  sampleRate);
        player   = engine.createPlayer   (numOutputChannels, sampleRate);

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

    int getDefaultBufferSize() override                 { return 1024; }
    int getOutputLatencyInSamples() override            { return outputLatency; }
    int getInputLatencyInSamples() override             { return inputLatency; }
    bool isOpen() override                              { return deviceOpen; }
    int getCurrentBufferSizeSamples() override          { return actualBufferSize; }
    int getCurrentBitDepth() override                   { return 16; }
    double getCurrentSampleRate() override              { return sampleRate; }
    BigInteger getActiveOutputChannels() const override { return activeOutputChans; }
    BigInteger getActiveInputChannels() const override  { return activeInputChans; }
    String getLastError() override                      { return lastError; }
    bool isPlaying() override                           { return callback != nullptr; }

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

    void run() override
    {
        if (recorder != nullptr)    recorder->start();
        if (player != nullptr)      player->start();

        while (! threadShouldExit())
        {
            if (player != nullptr)      player->writeBuffer (outputBuffer, *this);
            if (recorder != nullptr)    recorder->readNextBlock (inputBuffer, *this);

            const ScopedLock sl (callbackLock);

            if (callback != nullptr)
            {
                callback->audioDeviceIOCallback (numInputChannels > 0 ? (const float**) inputBuffer.getArrayOfChannels() : nullptr,
                                                 numInputChannels,
                                                 numOutputChannels > 0 ? outputBuffer.getArrayOfChannels() : nullptr,
                                                 numOutputChannels,
                                                 actualBufferSize);
            }
            else
            {
                outputBuffer.clear();
            }
        }
    }

private:
    //==================================================================================================
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

    AudioIODeviceCallback* setCallback (AudioIODeviceCallback* const newCallback)
    {
        const ScopedLock sl (callbackLock);
        AudioIODeviceCallback* const oldCallback = callback;
        callback = newCallback;
        return oldCallback;
    }

    //==================================================================================================
    struct Engine
    {
        Engine()
            : engineObject (nullptr), engineInterface (nullptr), outputMixObject (nullptr)
        {
            if (library.open ("libOpenSLES.so"))
            {
                typedef SLresult (*CreateEngineFunc) (SLObjectItf*, SLuint32, const SLEngineOption*, SLuint32, const SLInterfaceID*, const SLboolean*);

                if (CreateEngineFunc createEngine = (CreateEngineFunc) library.getFunction ("slCreateEngine"))
                {
                    check (createEngine (&engineObject, 0, nullptr, 0, nullptr, nullptr));

                    SLInterfaceID* SL_IID_ENGINE    = (SLInterfaceID*) library.getFunction ("SL_IID_ENGINE");
                    SL_IID_ANDROIDSIMPLEBUFFERQUEUE = (SLInterfaceID*) library.getFunction ("SL_IID_ANDROIDSIMPLEBUFFERQUEUE");
                    SL_IID_PLAY                     = (SLInterfaceID*) library.getFunction ("SL_IID_PLAY");
                    SL_IID_RECORD                   = (SLInterfaceID*) library.getFunction ("SL_IID_RECORD");

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

        Player* createPlayer (const int numChannels, const int sampleRate)
        {
            if (numChannels <= 0)
                return nullptr;

            ScopedPointer<Player> player (new Player (numChannels, sampleRate, *this));
            return player->openedOk() ? player.release() : nullptr;
        }

        Recorder* createRecorder (const int numChannels, const int sampleRate)
        {
            if (numChannels <= 0)
                return nullptr;

            ScopedPointer<Recorder> recorder (new Recorder (numChannels, sampleRate, *this));
            return recorder->openedOk() ? recorder.release() : nullptr;
        }

        SLObjectItf engineObject;
        SLEngineItf engineInterface;
        SLObjectItf outputMixObject;

        SLInterfaceID* SL_IID_ANDROIDSIMPLEBUFFERQUEUE;
        SLInterfaceID* SL_IID_PLAY;
        SLInterfaceID* SL_IID_RECORD;

    private:
        DynamicLibrary library;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Engine)
    };

    //==================================================================================================
    struct BufferList
    {
        BufferList (const int numChannels_)
            : numChannels (numChannels_), bufferSpace (numChannels_ * numSamples * numBuffers), nextBlock (0)
        {
        }

        int16* waitForFreeBuffer (Thread& threadToCheck)
        {
            while (numBlocksOut.get() == numBuffers)
            {
                dataArrived.wait (1);

                if (threadToCheck.threadShouldExit())
                    return nullptr;
            }

            return getNextBuffer();
        }

        int16* getNextBuffer()
        {
            if (++nextBlock == numBuffers)
                nextBlock = 0;

            return bufferSpace + nextBlock * numChannels * numSamples;
        }

        void bufferReturned()           { --numBlocksOut; dataArrived.signal(); }
        void bufferSent()               { ++numBlocksOut; dataArrived.signal(); }

        int getBufferSizeBytes() const  { return numChannels * numSamples * sizeof (int16); }

        const int numChannels;
        enum { numSamples = 256, numBuffers = 16 };

    private:
        HeapBlock<int16> bufferSpace;
        int nextBlock;
        Atomic<int> numBlocksOut;
        WaitableEvent dataArrived;
    };

    //==================================================================================================
    struct Player
    {
        Player (int numChannels, int sampleRate, Engine& engine)
            : playerObject (nullptr), playerPlay (nullptr), playerBufferQueue (nullptr),
              bufferList (numChannels)
        {
            jassert (numChannels == 2);

            SLDataFormat_PCM pcmFormat =
            {
                SL_DATAFORMAT_PCM,
                numChannels,
                sampleRate * 1000, // (sample rate units are millihertz)
                SL_PCMSAMPLEFORMAT_FIXED_16,
                SL_PCMSAMPLEFORMAT_FIXED_16,
                SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT,
                SL_BYTEORDER_LITTLEENDIAN
            };

            SLDataLocator_AndroidSimpleBufferQueue bufferQueue = { SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, bufferList.numBuffers };
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

        void writeBuffer (const AudioSampleBuffer& buffer, Thread& thread)
        {
            jassert (buffer.getNumChannels() == bufferList.numChannels);
            jassert (buffer.getNumSamples() < bufferList.numSamples * bufferList.numBuffers);

            int offset = 0;
            int numSamples = buffer.getNumSamples();

            while (numSamples > 0)
            {
                int16* const destBuffer = bufferList.waitForFreeBuffer (thread);

                if (destBuffer == nullptr)
                    break;

                for (int i = 0; i < bufferList.numChannels; ++i)
                {
                    typedef AudioData::Pointer <AudioData::Int16,   AudioData::LittleEndian, AudioData::Interleaved, AudioData::NonConst> DstSampleType;
                    typedef AudioData::Pointer <AudioData::Float32, AudioData::NativeEndian, AudioData::NonInterleaved, AudioData::Const> SrcSampleType;

                    DstSampleType dstData (destBuffer + i, bufferList.numChannels);
                    SrcSampleType srcData (buffer.getSampleData (i, offset));
                    dstData.convertSamples (srcData, bufferList.numSamples);
                }

                check ((*playerBufferQueue)->Enqueue (playerBufferQueue, destBuffer, bufferList.getBufferSizeBytes()));
                bufferList.bufferSent();

                numSamples -= bufferList.numSamples;
                offset += bufferList.numSamples;
            }
        }

    private:
        SLObjectItf playerObject;
        SLPlayItf playerPlay;
        SLAndroidSimpleBufferQueueItf playerBufferQueue;

        BufferList bufferList;

        static void staticCallback (SLAndroidSimpleBufferQueueItf queue, void* context)
        {
            jassert (queue == static_cast <Player*> (context)->playerBufferQueue); (void) queue;
            static_cast <Player*> (context)->bufferList.bufferReturned();
        }

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Player)
    };

    //==================================================================================================
    struct Recorder
    {
        Recorder (int numChannels, int sampleRate, Engine& engine)
            : recorderObject (nullptr), recorderRecord (nullptr), recorderBufferQueue (nullptr),
              bufferList (numChannels)
        {
            jassert (numChannels == 1); // STEREO doesn't always work!!

            SLDataFormat_PCM pcmFormat =
            {
                SL_DATAFORMAT_PCM,
                numChannels,
                sampleRate * 1000, // (sample rate units are millihertz)
                SL_PCMSAMPLEFORMAT_FIXED_16,
                SL_PCMSAMPLEFORMAT_FIXED_16,
                (numChannels == 1) ? SL_SPEAKER_FRONT_CENTER : (SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT),
                SL_BYTEORDER_LITTLEENDIAN
            };

            SLDataLocator_IODevice ioDevice = { SL_DATALOCATOR_IODEVICE, SL_IODEVICE_AUDIOINPUT, SL_DEFAULTDEVICEID_AUDIOINPUT, nullptr };
            SLDataSource audioSrc = { &ioDevice, nullptr };

            SLDataLocator_AndroidSimpleBufferQueue bufferQueue = { SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, bufferList.numBuffers };
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
                    check ((*recorderBufferQueue)->RegisterCallback (recorderBufferQueue, staticCallback, this));
                    check ((*recorderRecord)->SetRecordState (recorderRecord, SL_RECORDSTATE_STOPPED));

                    for (int i = bufferList.numBuffers; --i >= 0;)
                    {
                        int16* const buffer = bufferList.getNextBuffer();
                        jassert (buffer != nullptr);
                        enqueueBuffer (buffer);
                    }
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
                int16* const srcBuffer = bufferList.waitForFreeBuffer (thread);

                if (srcBuffer == nullptr)
                    break;

                for (int i = 0; i < bufferList.numChannels; ++i)
                {
                    typedef AudioData::Pointer <AudioData::Float32, AudioData::NativeEndian, AudioData::NonInterleaved, AudioData::NonConst> DstSampleType;
                    typedef AudioData::Pointer <AudioData::Int16,   AudioData::LittleEndian, AudioData::Interleaved, AudioData::Const> SrcSampleType;

                    DstSampleType dstData (buffer.getSampleData (i, offset));
                    SrcSampleType srcData (srcBuffer + i, bufferList.numChannels);
                    dstData.convertSamples (srcData, bufferList.numSamples);
                }

                enqueueBuffer (srcBuffer);

                numSamples -= bufferList.numSamples;
                offset += bufferList.numSamples;
            }
        }

    private:
        SLObjectItf recorderObject;
        SLRecordItf recorderRecord;
        SLAndroidSimpleBufferQueueItf recorderBufferQueue;

        BufferList bufferList;

        void enqueueBuffer (int16* buffer)
        {
            check ((*recorderBufferQueue)->Enqueue (recorderBufferQueue, buffer, bufferList.getBufferSizeBytes()));
            bufferList.bufferSent();
        }

        static void staticCallback (SLAndroidSimpleBufferQueueItf queue, void* context)
        {
            jassert (queue == static_cast <Recorder*> (context)->recorderBufferQueue); (void) queue;
            static_cast <Recorder*> (context)->bufferList.bufferReturned();
        }

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Recorder)
    };


    //==============================================================================
    Engine engine;

    ScopedPointer<Player> player;
    ScopedPointer<Recorder> recorder;

    //==============================================================================
    static bool check (const SLresult result)
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
    void scanForDevices() {}
    StringArray getDeviceNames (bool wantInputNames) const              { return StringArray (openSLTypeName); }
    int getDefaultDeviceIndex (bool forInput) const                     { return 0; }
    int getIndexOfDevice (AudioIODevice* device, bool asInput) const    { return device != nullptr ? 0 : -1; }
    bool hasSeparateInputsAndOutputs() const                            { return false; }

    AudioIODevice* createDevice (const String& outputDeviceName,
                                 const String& inputDeviceName)
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

/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD, CALLBACK) \
 STATICMETHOD (getMinBufferSize,            "getMinBufferSize",             "(III)I") \
 STATICMETHOD (getNativeOutputSampleRate,   "getNativeOutputSampleRate",    "(I)I") \
 METHOD (constructor,   "<init>",   "(IIIIII)V") \
 METHOD (getState,      "getState", "()I") \
 METHOD (play,          "play",     "()V") \
 METHOD (stop,          "stop",     "()V") \
 METHOD (release,       "release",  "()V") \
 METHOD (flush,         "flush",    "()V") \
 METHOD (write,         "write",    "([SII)I") \

DECLARE_JNI_CLASS (AudioTrack, "android/media/AudioTrack")
#undef JNI_CLASS_MEMBERS

//==============================================================================
#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD, CALLBACK) \
 STATICMETHOD (getMinBufferSize, "getMinBufferSize", "(III)I") \
 METHOD (constructor,       "<init>",           "(IIIII)V") \
 METHOD (getState,          "getState",         "()I") \
 METHOD (startRecording,    "startRecording",   "()V") \
 METHOD (stop,              "stop",             "()V") \
 METHOD (read,              "read",             "([SII)I") \
 METHOD (release,           "release",          "()V") \

DECLARE_JNI_CLASS (AudioRecord, "android/media/AudioRecord")
#undef JNI_CLASS_MEMBERS

//==============================================================================
enum
{
    CHANNEL_OUT_STEREO  = 12,
    CHANNEL_IN_STEREO   = 12,
    CHANNEL_IN_MONO     = 16,
    ENCODING_PCM_16BIT  = 2,
    STREAM_MUSIC        = 3,
    MODE_STREAM         = 1,
    STATE_UNINITIALIZED = 0
};

const char* const javaAudioTypeName = "Android Audio";

//==============================================================================
class AndroidAudioIODevice final : public AudioIODevice,
                                   public Thread
{
public:
    //==============================================================================
    AndroidAudioIODevice (const String& deviceName)
        : AudioIODevice (deviceName, javaAudioTypeName),
          Thread (SystemStats::getJUCEVersion() + ": audio"),
          minBufferSizeOut (0), minBufferSizeIn (0), callback (nullptr), sampleRate (0),
          numClientInputChannels (0), numDeviceInputChannels (0), numDeviceInputChannelsAvailable (2),
          numClientOutputChannels (0), numDeviceOutputChannels (0),
          actualBufferSize (0), isRunning (false),
          inputChannelBuffer (1, 1),
          outputChannelBuffer (1, 1)
    {
        JNIEnv* env = getEnv();
        sampleRate = env->CallStaticIntMethod (AudioTrack, AudioTrack.getNativeOutputSampleRate, MODE_STREAM);

        minBufferSizeOut = (int) env->CallStaticIntMethod (AudioTrack,  AudioTrack.getMinBufferSize,  sampleRate, CHANNEL_OUT_STEREO, ENCODING_PCM_16BIT);
        minBufferSizeIn  = (int) env->CallStaticIntMethod (AudioRecord, AudioRecord.getMinBufferSize, sampleRate, CHANNEL_IN_STEREO,  ENCODING_PCM_16BIT);

        if (minBufferSizeIn <= 0)
        {
            minBufferSizeIn = env->CallStaticIntMethod (AudioRecord, AudioRecord.getMinBufferSize, sampleRate, CHANNEL_IN_MONO, ENCODING_PCM_16BIT);

            if (minBufferSizeIn > 0)
                numDeviceInputChannelsAvailable = 1;
            else
                numDeviceInputChannelsAvailable = 0;
        }

        DBG ("Audio device - min buffers: " << minBufferSizeOut << ", " << minBufferSizeIn << "; "
              << sampleRate << " Hz; input chans: " << numDeviceInputChannelsAvailable);
    }

    ~AndroidAudioIODevice() override
    {
        close();
    }

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

        if (numDeviceInputChannelsAvailable == 2)
        {
            s.add ("Left");
            s.add ("Right");
        }
        else if (numDeviceInputChannelsAvailable == 1)
        {
            s.add ("Audio Input");
        }

        return s;
    }

    Array<double> getAvailableSampleRates() override
    {
        Array<double> r;
        r.add ((double) sampleRate);
        return r;
    }

    Array<int> getAvailableBufferSizes() override
    {
        Array<int> b;
        int n = 16;

        for (int i = 0; i < 50; ++i)
        {
            b.add (n);
            n += n < 64 ? 16
                        : (n < 512 ? 32
                                   : (n < 1024 ? 64
                                               : (n < 2048 ? 128 : 256)));
        }

        return b;
    }

    int getDefaultBufferSize() override                 { return 2048; }

    String open (const BigInteger& inputChannels,
                 const BigInteger& outputChannels,
                 double requestedSampleRate,
                 int bufferSize) override
    {
        close();

        if (sampleRate != (int) requestedSampleRate)
            return "Sample rate not allowed";

        lastError.clear();
        int preferredBufferSize = (bufferSize <= 0) ? getDefaultBufferSize() : bufferSize;

        numDeviceInputChannels = 0;
        numDeviceOutputChannels = 0;

        activeOutputChans = outputChannels;
        activeOutputChans.setRange (2, activeOutputChans.getHighestBit(), false);
        numClientOutputChannels = activeOutputChans.countNumberOfSetBits();

        activeInputChans = inputChannels;
        activeInputChans.setRange (2, activeInputChans.getHighestBit(), false);
        numClientInputChannels = activeInputChans.countNumberOfSetBits();

        actualBufferSize = preferredBufferSize;
        inputChannelBuffer.setSize (2, actualBufferSize);
        inputChannelBuffer.clear();
        outputChannelBuffer.setSize (2, actualBufferSize);
        outputChannelBuffer.clear();

        JNIEnv* env = getEnv();

        if (numClientOutputChannels > 0)
        {
            numDeviceOutputChannels = 2;
            outputDevice = GlobalRef (LocalRef<jobject> (env->NewObject (AudioTrack, AudioTrack.constructor,
                                                                         STREAM_MUSIC, sampleRate, CHANNEL_OUT_STEREO, ENCODING_PCM_16BIT,
                                                                         (jint) (minBufferSizeOut * numDeviceOutputChannels * static_cast<int> (sizeof (int16))), MODE_STREAM)));

            getUnderrunCount = env->GetMethodID (AudioTrack, "getUnderrunCount", "()I");

            int outputDeviceState = env->CallIntMethod (outputDevice, AudioTrack.getState);
            if (outputDeviceState > 0)
            {
                isRunning = true;
            }
            else
            {
                 // failed to open the device
                outputDevice.clear();
                lastError = "Error opening audio output device: android.media.AudioTrack failed with state = " + String (outputDeviceState);
            }
        }

        if (numClientInputChannels > 0 && numDeviceInputChannelsAvailable > 0)
        {
            if (! RuntimePermissions::isGranted (RuntimePermissions::recordAudio))
            {
                // If you hit this assert, you probably forgot to get RuntimePermissions::recordAudio
                // before trying to open an audio input device. This is not going to work!
                jassertfalse;

                inputDevice.clear();
                lastError = "Error opening audio input device: the app was not granted android.permission.RECORD_AUDIO";
            }
            else
            {
                numDeviceInputChannels = jmin (numClientInputChannels, numDeviceInputChannelsAvailable);
                inputDevice = GlobalRef (LocalRef<jobject> (env->NewObject (AudioRecord, AudioRecord.constructor,
                                                                            0 /* (default audio source) */, sampleRate,
                                                                            numDeviceInputChannelsAvailable > 1 ? CHANNEL_IN_STEREO : CHANNEL_IN_MONO,
                                                                            ENCODING_PCM_16BIT,
                                                                            (jint) (minBufferSizeIn * numDeviceInputChannels * static_cast<int> (sizeof (int16))))));

                int inputDeviceState = env->CallIntMethod (inputDevice, AudioRecord.getState);
                if (inputDeviceState > 0)
                {
                    isRunning = true;
                }
                else
                {
                     // failed to open the device
                    inputDevice.clear();
                    lastError = "Error opening audio input device: android.media.AudioRecord failed with state = " + String (inputDeviceState);
                }
            }
        }

        if (isRunning)
        {
            if (outputDevice != nullptr)
                env->CallVoidMethod (outputDevice, AudioTrack.play);

            if (inputDevice != nullptr)
                env->CallVoidMethod (inputDevice, AudioRecord.startRecording);

            startThread (Priority::high);
        }
        else
        {
            closeDevices();
        }

        return lastError;
    }

    void close() override
    {
        if (isRunning)
        {
            stopThread (2000);
            isRunning = false;
            closeDevices();
        }
    }

    int getOutputLatencyInSamples() override             { return (minBufferSizeOut * 3) / 4; }
    int getInputLatencyInSamples() override              { return (minBufferSizeIn * 3) / 4; }
    bool isOpen() override                               { return isRunning; }
    int getCurrentBufferSizeSamples() override           { return actualBufferSize; }
    int getCurrentBitDepth() override                    { return 16; }
    double getCurrentSampleRate() override               { return sampleRate; }
    BigInteger getActiveOutputChannels() const override  { return activeOutputChans; }
    BigInteger getActiveInputChannels() const override   { return activeInputChans; }
    String getLastError() override                       { return lastError; }
    bool isPlaying() override                            { return isRunning && callback != nullptr; }

    int getXRunCount() const noexcept override
    {
        if (outputDevice != nullptr && getUnderrunCount != nullptr)
            return getEnv()->CallIntMethod (outputDevice, getUnderrunCount);

        return -1;
    }

    void start (AudioIODeviceCallback* newCallback) override
    {
        if (isRunning && callback != newCallback)
        {
            if (newCallback != nullptr)
                newCallback->audioDeviceAboutToStart (this);

            const ScopedLock sl (callbackLock);
            callback = newCallback;
        }
    }

    void stop() override
    {
        if (isRunning)
        {
            AudioIODeviceCallback* lastCallback;

            {
                const ScopedLock sl (callbackLock);
                lastCallback = callback;
                callback = nullptr;
            }

            if (lastCallback != nullptr)
                lastCallback->audioDeviceStopped();
        }
    }

    void run() override
    {
        JNIEnv* env = getEnv();
        jshortArray audioBuffer = env->NewShortArray (actualBufferSize * jmax (numDeviceOutputChannels, numDeviceInputChannels));

        using NativeInt16   = AudioData::Format<AudioData::Int16,   AudioData::NativeEndian>;
        using NativeFloat32 = AudioData::Format<AudioData::Float32, AudioData::NativeEndian>;

        while (! threadShouldExit())
        {
            if (inputDevice != nullptr)
            {
                jint numRead = env->CallIntMethod (inputDevice, AudioRecord.read, audioBuffer, 0, actualBufferSize * numDeviceInputChannels);

                if (numRead < actualBufferSize * numDeviceInputChannels)
                {
                    DBG ("Audio read under-run! " << numRead);
                }

                jshort* const src = env->GetShortArrayElements (audioBuffer, nullptr);

                AudioData::deinterleaveSamples (AudioData::InterleavedSource<NativeInt16>    { reinterpret_cast<const uint16*> (src),        numDeviceInputChannels },
                                                AudioData::NonInterleavedDest<NativeFloat32> { inputChannelBuffer.getArrayOfWritePointers(), inputChannelBuffer.getNumChannels() },
                                                actualBufferSize);

                env->ReleaseShortArrayElements (audioBuffer, src, 0);
            }

            if (threadShouldExit())
                break;

            {
                const ScopedLock sl (callbackLock);

                if (callback != nullptr)
                {
                    callback->audioDeviceIOCallbackWithContext (inputChannelBuffer.getArrayOfReadPointers(),
                                                                numClientInputChannels,
                                                                outputChannelBuffer.getArrayOfWritePointers(),
                                                                numClientOutputChannels,
                                                                actualBufferSize, {});
                }
                else
                {
                    outputChannelBuffer.clear();
                }
            }

            if (outputDevice != nullptr)
            {
                if (threadShouldExit())
                    break;

                jshort* const dest = env->GetShortArrayElements (audioBuffer, nullptr);

                AudioData::interleaveSamples (AudioData::NonInterleavedSource<NativeFloat32> { outputChannelBuffer.getArrayOfReadPointers(), outputChannelBuffer.getNumChannels() },
                                              AudioData::InterleavedDest<NativeInt16>        { reinterpret_cast<uint16*> (dest),             numDeviceOutputChannels },
                                              actualBufferSize);

                env->ReleaseShortArrayElements (audioBuffer, dest, 0);
                jint numWritten = env->CallIntMethod (outputDevice, AudioTrack.write, audioBuffer, 0, actualBufferSize * numDeviceOutputChannels);

                if (numWritten < actualBufferSize * numDeviceOutputChannels)
                {
                    DBG ("Audio write underrun! " << numWritten);
                }
            }
        }
    }

    int minBufferSizeOut, minBufferSizeIn;

private:
    //==============================================================================
    CriticalSection callbackLock;
    AudioIODeviceCallback* callback;
    jint sampleRate;
    int numClientInputChannels, numDeviceInputChannels, numDeviceInputChannelsAvailable;
    int numClientOutputChannels, numDeviceOutputChannels;
    int actualBufferSize;
    bool isRunning;
    String lastError;
    BigInteger activeOutputChans, activeInputChans;
    GlobalRef outputDevice, inputDevice;
    AudioBuffer<float> inputChannelBuffer, outputChannelBuffer;
    jmethodID getUnderrunCount = nullptr;

    void closeDevices()
    {
        if (outputDevice != nullptr)
        {
            outputDevice.callVoidMethod (AudioTrack.stop);
            outputDevice.callVoidMethod (AudioTrack.release);
            outputDevice.clear();
        }

        if (inputDevice != nullptr)
        {
            inputDevice.callVoidMethod (AudioRecord.stop);
            inputDevice.callVoidMethod (AudioRecord.release);
            inputDevice.clear();
        }
    }

    JUCE_DECLARE_NON_COPYABLE (AndroidAudioIODevice)
};

//==============================================================================
class AndroidAudioIODeviceType final : public AudioIODeviceType
{
public:
    AndroidAudioIODeviceType() : AudioIODeviceType (javaAudioTypeName) {}

    //==============================================================================
    void scanForDevices() {}
    StringArray getDeviceNames (bool) const                             { return StringArray (javaAudioTypeName); }
    int getDefaultDeviceIndex (bool) const                              { return 0; }
    int getIndexOfDevice (AudioIODevice* device, bool) const            { return device != nullptr ? 0 : -1; }
    bool hasSeparateInputsAndOutputs() const                            { return false; }

    AudioIODevice* createDevice (const String& outputDeviceName,
                                 const String& inputDeviceName)
    {
        std::unique_ptr<AndroidAudioIODevice> dev;

        if (outputDeviceName.isNotEmpty() || inputDeviceName.isNotEmpty())
        {
            dev.reset (new AndroidAudioIODevice (outputDeviceName.isNotEmpty() ? outputDeviceName
                                                                               : inputDeviceName));

            if (dev->getCurrentSampleRate() <= 0 || dev->getDefaultBufferSize() <= 0)
                dev = nullptr;
        }

        return dev.release();
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AndroidAudioIODeviceType)
};


//==============================================================================
extern bool isOboeAvailable();
extern bool isOpenSLAvailable();

} // namespace juce

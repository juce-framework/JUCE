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

// (This file gets included by juce_android_NativeCode.cpp, rather than being
// compiled on its own).
#if JUCE_INCLUDED_FILE

//==============================================================================
#define CHANNEL_OUT_STEREO  ((jint) 12)
#define CHANNEL_IN_STEREO   ((jint) 12)
#define CHANNEL_IN_MONO     ((jint) 16)
#define ENCODING_PCM_16BIT  ((jint) 2)
#define STREAM_MUSIC        ((jint) 3)
#define MODE_STREAM         ((jint) 1)

//==============================================================================
class AndroidAudioIODevice  : public AudioIODevice,
                              public Thread
{
public:
    //==============================================================================
    AndroidAudioIODevice (const String& deviceName)
        : AudioIODevice (deviceName, "Audio"),
          Thread ("audio"),
          callback (0), sampleRate (0),
          numClientInputChannels (0), numDeviceInputChannels (0), numDeviceInputChannelsAvailable (2),
          numClientOutputChannels (0), numDeviceOutputChannels (0),
          minbufferSize (0), actualBufferSize (0),
          isRunning (false),
          outputChannelBuffer (1, 1),
          inputChannelBuffer (1, 1)
    {
        JNIEnv* env = getEnv();
        sampleRate = env->CallStaticIntMethod (android.audioTrackClass, android.getNativeOutputSampleRate, MODE_STREAM);

        const jint outMinBuffer = env->CallStaticIntMethod (android.audioTrackClass, android.getMinBufferSize, sampleRate, CHANNEL_OUT_STEREO, ENCODING_PCM_16BIT);

        jint inMinBuffer = env->CallStaticIntMethod (android.audioRecordClass, android.getMinRecordBufferSize, sampleRate, CHANNEL_IN_STEREO, ENCODING_PCM_16BIT);
        if (inMinBuffer <= 0)
        {
            inMinBuffer = env->CallStaticIntMethod (android.audioRecordClass, android.getMinRecordBufferSize, sampleRate, CHANNEL_IN_MONO, ENCODING_PCM_16BIT);

            if (inMinBuffer > 0)
                numDeviceInputChannelsAvailable = 1;
            else
                numDeviceInputChannelsAvailable = 0;
        }

        minbufferSize = jmax (outMinBuffer, inMinBuffer) / 4;

        DBG ("Audio device - min buffers: " << outMinBuffer << ", " << inMinBuffer << "; "
              << sampleRate << " Hz; input chans: " << numDeviceInputChannelsAvailable);
    }

    ~AndroidAudioIODevice()
    {
        close();
    }

    StringArray getOutputChannelNames()
    {
        StringArray s;
        s.add ("Left");
        s.add ("Right");
        return s;
    }

    StringArray getInputChannelNames()
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

    int getNumSampleRates()             { return 1;}
    double getSampleRate (int index)    { return sampleRate; }

    int getDefaultBufferSize()              { return minbufferSize; }
    int getNumBufferSizesAvailable()        { return 10; }
    int getBufferSizeSamples (int index)    { return getDefaultBufferSize() + index * 128; }

    const String open (const BigInteger& inputChannels,
                       const BigInteger& outputChannels,
                       double requestedSampleRate,
                       int bufferSize)
    {
        close();

        if (sampleRate != (int) requestedSampleRate)
            return "Sample rate not allowed";

        lastError = String::empty;
        int preferredBufferSize = (bufferSize <= 0) ? getDefaultBufferSize() : jmax (minbufferSize, bufferSize);

        numDeviceInputChannels = 0;
        numDeviceOutputChannels = 0;

        activeOutputChans = outputChannels;
        activeOutputChans.setRange (2, activeOutputChans.getHighestBit(), false);
        numClientOutputChannels = activeOutputChans.countNumberOfSetBits();

        activeInputChans = inputChannels;
        activeInputChans.setRange (2, activeInputChans.getHighestBit(), false);
        numClientInputChannels = activeInputChans.countNumberOfSetBits();

        actualBufferSize = preferredBufferSize;
        inputChannelBuffer.setSize (actualBufferSize, 2);
        outputChannelBuffer.setSize (actualBufferSize, 2);
        inputChannelBuffer.clear();
        outputChannelBuffer.clear();

        JNIEnv* env = getEnv();

        if (numClientOutputChannels > 0)
        {
            numDeviceOutputChannels = 2;
            outputDevice = GlobalRef (env->NewObject (android.audioTrackClass, android.audioTrackConstructor,
                                                      STREAM_MUSIC, sampleRate, CHANNEL_OUT_STEREO, ENCODING_PCM_16BIT,
                                                      (jint) (actualBufferSize * numDeviceOutputChannels * sizeof (float)), MODE_STREAM));
            isRunning = true;
        }

        if (numClientInputChannels > 0 && numDeviceInputChannelsAvailable > 0)
        {
            numDeviceInputChannels = jmin (numClientInputChannels, numDeviceInputChannelsAvailable);
            inputDevice = GlobalRef (env->NewObject (android.audioRecordClass, android.audioRecordConstructor,
                                                     0 /* (default audio source) */, sampleRate,
                                                     numDeviceInputChannelsAvailable > 1 ? CHANNEL_IN_STEREO : CHANNEL_IN_MONO,
                                                     ENCODING_PCM_16BIT,
                                                     (jint) (actualBufferSize * numDeviceInputChannels * sizeof (float))));
            isRunning = true;
        }

        if (isRunning)
        {
            if (outputDevice != nullptr)
                env->CallVoidMethod (outputDevice, android.audioTrackPlay);

            if (inputDevice != nullptr)
                env->CallVoidMethod (inputDevice, android.startRecording);

            startThread (8);
        }
        else
        {
            closeDevices();
        }

        return lastError;
    }

    void close()
    {
        if (isRunning)
        {
            stopThread (2000);
            isRunning = false;
            closeDevices();
        }
    }

    int getOutputLatencyInSamples()                     { return 0; } // TODO
    int getInputLatencyInSamples()                      { return 0; } // TODO
    bool isOpen()                                       { return isRunning; }
    int getCurrentBufferSizeSamples()                   { return actualBufferSize; }
    int getCurrentBitDepth()                            { return 16; }
    double getCurrentSampleRate()                       { return sampleRate; }
    const BigInteger getActiveOutputChannels() const    { return activeOutputChans; }
    const BigInteger getActiveInputChannels() const     { return activeInputChans; }
    const String getLastError()                         { return lastError; }
    bool isPlaying()                                    { return isRunning && callback != 0; }

    void start (AudioIODeviceCallback* newCallback)
    {
        if (isRunning && callback != newCallback)
        {
            if (newCallback != nullptr)
                newCallback->audioDeviceAboutToStart (this);

            const ScopedLock sl (callbackLock);
            callback = newCallback;
        }
    }

    void stop()
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

    void run()
    {
        JNIEnv* env = getEnv();
        jshortArray audioBuffer = env->NewShortArray (actualBufferSize * jmax (numDeviceOutputChannels, numDeviceInputChannels));

        while (! threadShouldExit())
        {
            if (inputDevice != nullptr)
            {
                jint numRead = env->CallIntMethod (inputDevice, android.audioRecordRead, audioBuffer, 0, actualBufferSize * numDeviceInputChannels);

                if (numRead < actualBufferSize * numDeviceInputChannels)
                {
                    DBG ("Audio read under-run! " << numRead);
                }

                jshort* const src = env->GetShortArrayElements (audioBuffer, 0);

                for (int chan = 0; chan < numDeviceInputChannels; ++chan)
                {
                    AudioData::Pointer <AudioData::Int16, AudioData::NativeEndian, AudioData::Interleaved, AudioData::Const> s (src + chan, numDeviceInputChannels);
                    AudioData::Pointer <AudioData::Float32, AudioData::NativeEndian, AudioData::NonInterleaved, AudioData::NonConst> d (inputChannelBuffer.getSampleData (chan));
                    d.convertSamples (s, actualBufferSize);
                }

                env->ReleaseShortArrayElements (audioBuffer, src, 0);
            }

            if (threadShouldExit())
                break;

            {
                const ScopedLock sl (callbackLock);

                if (callback != nullptr)
                {
                    callback->audioDeviceIOCallback ((const float**) inputChannelBuffer.getArrayOfChannels(), numClientInputChannels,
                                                     outputChannelBuffer.getArrayOfChannels(), numClientOutputChannels,
                                                     actualBufferSize);
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

                jshort* const dest = env->GetShortArrayElements (audioBuffer, 0);

                for (int chan = 0; chan < numDeviceOutputChannels; ++chan)
                {
                    AudioData::Pointer <AudioData::Int16, AudioData::NativeEndian, AudioData::Interleaved, AudioData::NonConst> d (dest + chan, numDeviceOutputChannels);
                    AudioData::Pointer <AudioData::Float32, AudioData::NativeEndian, AudioData::NonInterleaved, AudioData::Const> s (outputChannelBuffer.getSampleData (chan));
                    d.convertSamples (s, actualBufferSize);
                }

                env->ReleaseShortArrayElements (audioBuffer, dest, 0);
                jint numWritten = env->CallIntMethod (outputDevice, android.audioTrackWrite, audioBuffer, 0, actualBufferSize * numDeviceOutputChannels);

                if (numWritten < actualBufferSize * numDeviceOutputChannels)
                {
                    DBG ("Audio write underrun! " << numWritten);
                }
            }
        }
    }

private:
    //==================================================================================================
    CriticalSection callbackLock;
    AudioIODeviceCallback* callback;
    jint sampleRate;
    int numClientInputChannels, numDeviceInputChannels, numDeviceInputChannelsAvailable;
    int numClientOutputChannels, numDeviceOutputChannels;
    int minbufferSize, actualBufferSize;
    bool isRunning;
    String lastError;
    BigInteger activeOutputChans, activeInputChans;
    GlobalRef outputDevice, inputDevice;
    AudioSampleBuffer inputChannelBuffer, outputChannelBuffer;

    void closeDevices()
    {
        if (outputDevice != nullptr)
        {
            outputDevice.callVoidMethod (android.audioTrackStop);
            outputDevice.callVoidMethod (android.audioTrackRelease);
            outputDevice.clear();
        }

        if (inputDevice != nullptr)
        {
            inputDevice.callVoidMethod (android.stopRecording);
            inputDevice.callVoidMethod (android.audioRecordRelease);
            inputDevice.clear();
        }
    }

    JUCE_DECLARE_NON_COPYABLE (AndroidAudioIODevice);
};

//==============================================================================
class AndroidAudioIODeviceType  : public AudioIODeviceType
{
public:
    AndroidAudioIODeviceType()
        : AudioIODeviceType ("Android Audio")
    {
    }

    //==============================================================================
    void scanForDevices() {}
    int getDefaultDeviceIndex (bool forInput) const                     { return 0; }
    int getIndexOfDevice (AudioIODevice* device, bool asInput) const    { return device != nullptr ? 0 : -1; }
    bool hasSeparateInputsAndOutputs() const                            { return false; }

    StringArray getDeviceNames (bool wantInputNames) const
    {
        StringArray s;
        s.add ("Android Audio");
        return s;
    }

    AudioIODevice* createDevice (const String& outputDeviceName,
                                 const String& inputDeviceName)
    {
        ScopedPointer<AndroidAudioIODevice> dev;

        if (outputDeviceName.isNotEmpty() || inputDeviceName.isNotEmpty())
        {
            dev = new AndroidAudioIODevice (outputDeviceName.isNotEmpty() ? outputDeviceName
                                                                          : inputDeviceName);

            if (dev->getCurrentSampleRate() <= 0 || dev->getDefaultBufferSize() <= 0)
                dev = nullptr;
        }

        return dev.release();
    }

private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AndroidAudioIODeviceType);
};


//==============================================================================
AudioIODeviceType* AudioIODeviceType::createAudioIODeviceType_Android()
{
    return new AndroidAudioIODeviceType();
}


#endif

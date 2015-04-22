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

class iOSAudioIODevice  : public AudioIODevice
{
public:
    iOSAudioIODevice (const String& deviceName)
        : AudioIODevice (deviceName, "Audio"),
          actualBufferSize (0),
          isRunning (false),
          audioUnit (0),
          callback (nullptr),
          floatData (1, 2)
    {
        getSessionHolder().activeDevices.add (this);

        numInputChannels = 2;
        numOutputChannels = 2;
        preferredBufferSize = 0;

        updateDeviceInfo();
    }

    ~iOSAudioIODevice()
    {
        getSessionHolder().activeDevices.removeFirstMatchingValue (this);
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
        if (audioInputIsAvailable)
        {
            s.add ("Left");
            s.add ("Right");
        }
        return s;
    }

    Array<double> getAvailableSampleRates() override
    {
        // can't find a good way to actually ask the device for which of these it supports..
        static const double rates[] = { 8000.0, 16000.0, 22050.0, 32000.0, 44100.0, 48000.0 };
        return Array<double> (rates, numElementsInArray (rates));
    }

    Array<int> getAvailableBufferSizes() override
    {
        Array<int> r;

        for (int i = 6; i < 12; ++i)
            r.add (1 << i);

        return r;
    }

    int getDefaultBufferSize() override         { return 1024; }

    String open (const BigInteger& inputChannelsWanted,
                 const BigInteger& outputChannelsWanted,
                 double targetSampleRate, int bufferSize) override
    {
        close();

        lastError.clear();
        preferredBufferSize = (bufferSize <= 0) ? getDefaultBufferSize() : bufferSize;

        //  xxx set up channel mapping

        activeOutputChans = outputChannelsWanted;
        activeOutputChans.setRange (2, activeOutputChans.getHighestBit(), false);
        numOutputChannels = activeOutputChans.countNumberOfSetBits();
        monoOutputChannelNumber = activeOutputChans.findNextSetBit (0);

        activeInputChans = inputChannelsWanted;
        activeInputChans.setRange (2, activeInputChans.getHighestBit(), false);
        numInputChannels = activeInputChans.countNumberOfSetBits();
        monoInputChannelNumber = activeInputChans.findNextSetBit (0);

        AudioSessionSetActive (true);

        if (numInputChannels > 0 && audioInputIsAvailable)
        {
            setSessionUInt32Property (kAudioSessionProperty_AudioCategory, kAudioSessionCategory_PlayAndRecord);
            setSessionUInt32Property (kAudioSessionProperty_OverrideCategoryEnableBluetoothInput, 1);
        }
        else
        {
            setSessionUInt32Property (kAudioSessionProperty_AudioCategory, kAudioSessionCategory_MediaPlayback);
        }

        AudioSessionAddPropertyListener (kAudioSessionProperty_AudioRouteChange, routingChangedStatic, this);

        fixAudioRouteIfSetToReceiver();

        setSessionFloat64Property (kAudioSessionProperty_PreferredHardwareSampleRate, targetSampleRate);
        updateDeviceInfo();

        setSessionFloat32Property (kAudioSessionProperty_PreferredHardwareIOBufferDuration, preferredBufferSize / sampleRate);
        updateCurrentBufferSize();

        prepareFloatBuffers (actualBufferSize);

        isRunning = true;
        routingChanged (nullptr);  // creates and starts the AU

        lastError = audioUnit != 0 ? "" : "Couldn't open the device";
        return lastError;
    }

    void close() override
    {
        if (isRunning)
        {
            isRunning = false;

            setSessionUInt32Property (kAudioSessionProperty_AudioCategory, kAudioSessionCategory_MediaPlayback);

            AudioSessionRemovePropertyListenerWithUserData (kAudioSessionProperty_AudioRouteChange, routingChangedStatic, this);
            AudioSessionSetActive (false);

            if (audioUnit != 0)
            {
                AudioComponentInstanceDispose (audioUnit);
                audioUnit = 0;
            }
        }
    }

    bool isOpen() override                       { return isRunning; }

    int getCurrentBufferSizeSamples() override   { return actualBufferSize; }
    double getCurrentSampleRate() override       { return sampleRate; }
    int getCurrentBitDepth() override            { return 16; }

    BigInteger getActiveOutputChannels() const override    { return activeOutputChans; }
    BigInteger getActiveInputChannels() const override     { return activeInputChans; }

    int getOutputLatencyInSamples() override    { return getLatency (kAudioSessionProperty_CurrentHardwareOutputLatency); }
    int getInputLatencyInSamples() override     { return getLatency (kAudioSessionProperty_CurrentHardwareInputLatency); }

    int getLatency (AudioSessionPropertyID propID)
    {
        Float32 latency = 0;
        getSessionProperty (propID, latency);
        return roundToInt (latency * getCurrentSampleRate());
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

    bool isPlaying() override            { return isRunning && callback != nullptr; }
    String getLastError() override       { return lastError; }

    bool setAudioPreprocessingEnabled (bool enable) override
    {
        return setSessionUInt32Property (kAudioSessionProperty_Mode, enable ? kAudioSessionMode_Default
                                                                            : kAudioSessionMode_Measurement);
    }

private:
    //==================================================================================================
    CriticalSection callbackLock;
    Float64 sampleRate;
    int numInputChannels, numOutputChannels;
    int preferredBufferSize, actualBufferSize;
    bool isRunning;
    String lastError;

    AudioStreamBasicDescription format;
    AudioUnit audioUnit;
    UInt32 audioInputIsAvailable;
    AudioIODeviceCallback* callback;
    BigInteger activeOutputChans, activeInputChans;

    AudioSampleBuffer floatData;
    float* inputChannels[3];
    float* outputChannels[3];
    bool monoInputChannelNumber, monoOutputChannelNumber;

    void prepareFloatBuffers (int bufferSize)
    {
        if (numInputChannels + numOutputChannels > 0)
        {
            floatData.setSize (numInputChannels + numOutputChannels, bufferSize);
            zeromem (inputChannels, sizeof (inputChannels));
            zeromem (outputChannels, sizeof (outputChannels));

            for (int i = 0; i < numInputChannels; ++i)
                inputChannels[i] = floatData.getWritePointer (i);

            for (int i = 0; i < numOutputChannels; ++i)
                outputChannels[i] = floatData.getWritePointer (i + numInputChannels);
        }
    }

    //==================================================================================================
    OSStatus process (AudioUnitRenderActionFlags* flags, const AudioTimeStamp* time,
                      const UInt32 numFrames, AudioBufferList* data)
    {
        OSStatus err = noErr;

        if (audioInputIsAvailable && numInputChannels > 0)
            err = AudioUnitRender (audioUnit, flags, time, 1, numFrames, data);

        const ScopedLock sl (callbackLock);

        if (callback != nullptr)
        {
            if ((int) numFrames > floatData.getNumSamples())
                prepareFloatBuffers ((int) numFrames);

            if (audioInputIsAvailable && numInputChannels > 0)
            {
                short* shortData = (short*) data->mBuffers[0].mData;

                if (numInputChannels >= 2)
                {
                    for (UInt32 i = 0; i < numFrames; ++i)
                    {
                        inputChannels[0][i] = *shortData++ * (1.0f / 32768.0f);
                        inputChannels[1][i] = *shortData++ * (1.0f / 32768.0f);
                    }
                }
                else
                {
                    if (monoInputChannelNumber > 0)
                        ++shortData;

                    for (UInt32 i = 0; i < numFrames; ++i)
                    {
                        inputChannels[0][i] = *shortData++ * (1.0f / 32768.0f);
                        ++shortData;
                    }
                }
            }
            else
            {
                for (int i = numInputChannels; --i >= 0;)
                    zeromem (inputChannels[i], sizeof (float) * numFrames);
            }

            callback->audioDeviceIOCallback ((const float**) inputChannels, numInputChannels,
                                             outputChannels, numOutputChannels, (int) numFrames);

            short* shortData = (short*) data->mBuffers[0].mData;
            int n = 0;

            if (numOutputChannels >= 2)
            {
                for (UInt32 i = 0; i < numFrames; ++i)
                {
                    shortData [n++] = (short) (outputChannels[0][i] * 32767.0f);
                    shortData [n++] = (short) (outputChannels[1][i] * 32767.0f);
                }
            }
            else if (numOutputChannels == 1)
            {
                for (UInt32 i = 0; i < numFrames; ++i)
                {
                    const short s = (short) (outputChannels[monoOutputChannelNumber][i] * 32767.0f);
                    shortData [n++] = s;
                    shortData [n++] = s;
                }
            }
            else
            {
                zeromem (data->mBuffers[0].mData, 2 * sizeof (short) * numFrames);
            }
        }
        else
        {
            zeromem (data->mBuffers[0].mData, 2 * sizeof (short) * numFrames);
        }

        return err;
    }

    void updateDeviceInfo()
    {
        getSessionProperty (kAudioSessionProperty_CurrentHardwareSampleRate, sampleRate);
        getSessionProperty (kAudioSessionProperty_AudioInputAvailable, audioInputIsAvailable);
    }

    void updateCurrentBufferSize()
    {
        Float32 bufferDuration = sampleRate > 0 ? (Float32) (preferredBufferSize / sampleRate) : 0.0f;
        getSessionProperty (kAudioSessionProperty_CurrentHardwareIOBufferDuration, bufferDuration);
        actualBufferSize = (int) (sampleRate * bufferDuration + 0.5);
    }

    void routingChanged (const void* propertyValue)
    {
        if (! isRunning)
            return;

        if (propertyValue != nullptr)
        {
            CFDictionaryRef routeChangeDictionary = (CFDictionaryRef) propertyValue;
            CFNumberRef routeChangeReasonRef = (CFNumberRef) CFDictionaryGetValue (routeChangeDictionary,
                                                                                   CFSTR (kAudioSession_AudioRouteChangeKey_Reason));

            SInt32 routeChangeReason;
            CFNumberGetValue (routeChangeReasonRef, kCFNumberSInt32Type, &routeChangeReason);

            if (routeChangeReason == kAudioSessionRouteChangeReason_OldDeviceUnavailable)
            {
                const ScopedLock sl (callbackLock);

                if (callback != nullptr)
                    callback->audioDeviceError ("Old device unavailable");
            }
        }

        updateDeviceInfo();
        createAudioUnit();

        AudioSessionSetActive (true);

        if (audioUnit != 0)
        {
            UInt32 formatSize = sizeof (format);
            AudioUnitGetProperty (audioUnit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Output, 1, &format, &formatSize);

            updateCurrentBufferSize();
            AudioOutputUnitStart (audioUnit);
        }
    }

    //==================================================================================================
    struct AudioSessionHolder
    {
        AudioSessionHolder()
        {
            AudioSessionInitialize (0, 0, interruptionListenerCallback, this);
        }

        static void interruptionListenerCallback (void* client, UInt32 interruptionType)
        {
            const Array <iOSAudioIODevice*>& activeDevices = static_cast <AudioSessionHolder*> (client)->activeDevices;

            for (int i = activeDevices.size(); --i >= 0;)
                activeDevices.getUnchecked(i)->interruptionListener (interruptionType);
        }

        Array <iOSAudioIODevice*> activeDevices;
    };

    static AudioSessionHolder& getSessionHolder()
    {
        static AudioSessionHolder audioSessionHolder;
        return audioSessionHolder;
    }

    void interruptionListener (const UInt32 interruptionType)
    {
        if (interruptionType == kAudioSessionBeginInterruption)
        {
            isRunning = false;
            AudioOutputUnitStop (audioUnit);
            AudioSessionSetActive (false);

            const ScopedLock sl (callbackLock);

            if (callback != nullptr)
                callback->audioDeviceError ("iOS audio session interruption");
        }

        if (interruptionType == kAudioSessionEndInterruption)
        {
            isRunning = true;
            AudioSessionSetActive (true);
            AudioOutputUnitStart (audioUnit);

            const ScopedLock sl (callbackLock);

            if (callback != nullptr)
                callback->audioDeviceError ("iOS audio session resumed");
        }
    }

    //==================================================================================================
    static OSStatus processStatic (void* client, AudioUnitRenderActionFlags* flags, const AudioTimeStamp* time,
                                   UInt32 /*busNumber*/, UInt32 numFrames, AudioBufferList* data)
    {
        return static_cast<iOSAudioIODevice*> (client)->process (flags, time, numFrames, data);
    }

    static void routingChangedStatic (void* client, AudioSessionPropertyID, UInt32 /*inDataSize*/, const void* propertyValue)
    {
        static_cast<iOSAudioIODevice*> (client)->routingChanged (propertyValue);
    }

    //==================================================================================================
    void resetFormat (const int numChannels) noexcept
    {
        zerostruct (format);
        format.mFormatID = kAudioFormatLinearPCM;
        format.mFormatFlags = kLinearPCMFormatFlagIsSignedInteger | kLinearPCMFormatFlagIsPacked | kAudioFormatFlagsNativeEndian;
        format.mBitsPerChannel = 8 * sizeof (short);
        format.mChannelsPerFrame = (UInt32) numChannels;
        format.mFramesPerPacket = 1;
        format.mBytesPerFrame = format.mBytesPerPacket = (UInt32) numChannels * sizeof (short);
    }

    bool createAudioUnit()
    {
        if (audioUnit != 0)
        {
            AudioComponentInstanceDispose (audioUnit);
            audioUnit = 0;
        }

        resetFormat (2);

        AudioComponentDescription desc;
        desc.componentType = kAudioUnitType_Output;
        desc.componentSubType = kAudioUnitSubType_RemoteIO;
        desc.componentManufacturer = kAudioUnitManufacturer_Apple;
        desc.componentFlags = 0;
        desc.componentFlagsMask = 0;

        AudioComponent comp = AudioComponentFindNext (0, &desc);
        AudioComponentInstanceNew (comp, &audioUnit);

        if (audioUnit == 0)
            return false;

        if (numInputChannels > 0)
        {
            const UInt32 one = 1;
            AudioUnitSetProperty (audioUnit, kAudioOutputUnitProperty_EnableIO, kAudioUnitScope_Input, 1, &one, sizeof (one));
        }

        {
            AudioChannelLayout layout;
            layout.mChannelBitmap = 0;
            layout.mNumberChannelDescriptions = 0;
            layout.mChannelLayoutTag = kAudioChannelLayoutTag_Stereo;
            AudioUnitSetProperty (audioUnit, kAudioUnitProperty_AudioChannelLayout, kAudioUnitScope_Input,  0, &layout, sizeof (layout));
            AudioUnitSetProperty (audioUnit, kAudioUnitProperty_AudioChannelLayout, kAudioUnitScope_Output, 0, &layout, sizeof (layout));
        }

        {
            AURenderCallbackStruct inputProc;
            inputProc.inputProc = processStatic;
            inputProc.inputProcRefCon = this;
            AudioUnitSetProperty (audioUnit, kAudioUnitProperty_SetRenderCallback, kAudioUnitScope_Input, 0, &inputProc, sizeof (inputProc));
        }

        AudioUnitSetProperty (audioUnit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Input,  0, &format, sizeof (format));
        AudioUnitSetProperty (audioUnit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Output, 1, &format, sizeof (format));

        AudioUnitInitialize (audioUnit);
        return true;
    }

    // If the routing is set to go through the receiver (i.e. the speaker, but quiet), this re-routes it
    // to make it loud. Needed because by default when using an input + output, the output is kept quiet.
    static void fixAudioRouteIfSetToReceiver()
    {
        CFStringRef audioRoute = 0;
        if (getSessionProperty (kAudioSessionProperty_AudioRoute, audioRoute) == noErr)
        {
            NSString* route = (NSString*) audioRoute;

            //DBG ("audio route: " + nsStringToJuce (route));

            if ([route hasPrefix: @"Receiver"])
                setSessionUInt32Property (kAudioSessionProperty_OverrideAudioRoute, kAudioSessionOverrideAudioRoute_Speaker);

            CFRelease (audioRoute);
        }
    }

    template <typename Type>
    static OSStatus getSessionProperty (AudioSessionPropertyID propID, Type& result) noexcept
    {
        UInt32 valueSize = sizeof (result);
        return AudioSessionGetProperty (propID, &valueSize, &result);
    }

    static bool setSessionUInt32Property  (AudioSessionPropertyID propID, UInt32  v) noexcept  { return AudioSessionSetProperty (propID, sizeof (v), &v) == kAudioSessionNoError; }
    static bool setSessionFloat32Property (AudioSessionPropertyID propID, Float32 v) noexcept  { return AudioSessionSetProperty (propID, sizeof (v), &v) == kAudioSessionNoError; }
    static bool setSessionFloat64Property (AudioSessionPropertyID propID, Float64 v) noexcept  { return AudioSessionSetProperty (propID, sizeof (v), &v) == kAudioSessionNoError; }

    JUCE_DECLARE_NON_COPYABLE (iOSAudioIODevice)
};


//==============================================================================
class iOSAudioIODeviceType  : public AudioIODeviceType
{
public:
    iOSAudioIODeviceType()  : AudioIODeviceType ("iOS Audio") {}

    void scanForDevices() {}
    StringArray getDeviceNames (bool /*wantInputNames*/) const       { return StringArray ("iOS Audio"); }
    int getDefaultDeviceIndex (bool /*forInput*/) const              { return 0; }
    int getIndexOfDevice (AudioIODevice* d, bool /*asInput*/) const  { return d != nullptr ? 0 : -1; }
    bool hasSeparateInputsAndOutputs() const                         { return false; }

    AudioIODevice* createDevice (const String& outputDeviceName, const String& inputDeviceName)
    {
        if (outputDeviceName.isNotEmpty() || inputDeviceName.isNotEmpty())
            return new iOSAudioIODevice (outputDeviceName.isNotEmpty() ? outputDeviceName
                                                                       : inputDeviceName);

        return nullptr;
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (iOSAudioIODeviceType)
};

//==============================================================================
AudioIODeviceType* AudioIODeviceType::createAudioIODeviceType_iOSAudio()
{
    return new iOSAudioIODeviceType();
}

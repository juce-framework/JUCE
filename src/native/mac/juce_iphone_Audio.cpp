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

// (This file gets included by juce_mac_NativeCode.mm, rather than being
// compiled on its own).
#if JUCE_INCLUDED_FILE


//==================================================================================================
class IPhoneAudioIODevice  : public AudioIODevice
{
public:
    //==============================================================================
    IPhoneAudioIODevice (const String& deviceName)
        : AudioIODevice (deviceName, T("Audio")),
          audioUnit (0),
          isRunning (false),
          callback (0),
          actualBufferSize (0),
          floatData (1, 2)
    {
        numInputChannels = 2;
        numOutputChannels = 2;
        preferredBufferSize = 0;

        AudioSessionInitialize (0, 0, interruptionListenerStatic, this);
        updateDeviceInfo();
    }

    ~IPhoneAudioIODevice()
    {
        close();
    }

    const StringArray getOutputChannelNames()
    {
        StringArray s;
        s.add ("Left");
        s.add ("Right");
        return s;
    }

    const StringArray getInputChannelNames()
    {
        StringArray s;
        if (audioInputIsAvailable)
        {
            s.add ("Left");
            s.add ("Right");
        }
        return s;
    }

    int getNumSampleRates()
    {
        return 1;
    }

    double getSampleRate (int index)
    {
        return sampleRate;
    }

    int getNumBufferSizesAvailable()
    {
        return 1;
    }

    int getBufferSizeSamples (int index)
    {
        return getDefaultBufferSize();
    }

    int getDefaultBufferSize()
    {
        return 1024;
    }

    const String open (const BitArray& inputChannels,
                       const BitArray& outputChannels,
                       double sampleRate,
                       int bufferSize)
    {
        close();

        lastError = String::empty;
        preferredBufferSize = (bufferSize <= 0) ? getDefaultBufferSize() : bufferSize;

        //  xxx set up channel mapping

        activeOutputChans = outputChannels;
        activeOutputChans.setRange (2, activeOutputChans.getHighestBit(), false);
        numOutputChannels = activeOutputChans.countNumberOfSetBits();
        monoOutputChannelNumber = activeOutputChans.findNextSetBit (0);

        activeInputChans = inputChannels;
        activeInputChans.setRange (2, activeInputChans.getHighestBit(), false);
        numInputChannels = activeInputChans.countNumberOfSetBits();
        monoInputChannelNumber = activeInputChans.findNextSetBit (0);

        AudioSessionSetActive (true);

        UInt32 audioCategory = kAudioSessionCategory_PlayAndRecord;
        AudioSessionSetProperty (kAudioSessionProperty_AudioCategory, sizeof (audioCategory), &audioCategory);
        AudioSessionAddPropertyListener (kAudioSessionProperty_AudioRouteChange, propertyChangedStatic, this);

        fixAudioRouteIfSetToReceiver();
        updateDeviceInfo();

        Float32 bufferDuration = preferredBufferSize / sampleRate;
        AudioSessionSetProperty (kAudioSessionProperty_PreferredHardwareIOBufferDuration, sizeof (bufferDuration), &bufferDuration);
        actualBufferSize = preferredBufferSize;

        prepareFloatBuffers();

        isRunning = true;
        propertyChanged (0, 0, 0);  // creates and starts the AU

        lastError = audioUnit != 0 ? String::empty
                                   : T("Couldn't open the device");
        return lastError;
    }

    void close()
    {
        if (isRunning)
        {
            isRunning = false;
            AudioSessionSetActive (false);

            if (audioUnit != 0)
            {
                AudioComponentInstanceDispose (audioUnit);
                audioUnit = 0;
            }
        }
    }

    bool isOpen()
    {
        return isRunning;
    }

    int getCurrentBufferSizeSamples()
    {
        return actualBufferSize;
    }

    double getCurrentSampleRate()
    {
        return sampleRate;
    }

    int getCurrentBitDepth()
    {
        return 16;
    }

    const BitArray getActiveOutputChannels() const
    {
        return activeOutputChans;
    }

    const BitArray getActiveInputChannels() const
    {
        return activeInputChans;
    }

    int getOutputLatencyInSamples()
    {
        return 0; //xxx
    }

    int getInputLatencyInSamples()
    {
        return 0; //xxx
    }

    void start (AudioIODeviceCallback* callback_)
    {
        if (isRunning && callback != callback_)
        {
            if (callback_ != 0)
                callback_->audioDeviceAboutToStart (this);

            callbackLock.enter();
            callback = callback_;
            callbackLock.exit();
        }
    }

    void stop()
    {
        if (isRunning)
        {
            callbackLock.enter();
            AudioIODeviceCallback* const lastCallback = callback;
            callback = 0;
            callbackLock.exit();

            if (lastCallback != 0)
                lastCallback->audioDeviceStopped();
        }
    }

    bool isPlaying()
    {
        return isRunning && callback != 0;
    }

    const String getLastError()
    {
        return lastError;
    }

private:
    //==================================================================================================
    CriticalSection callbackLock;
    Float64 sampleRate;
    int numInputChannels, numOutputChannels;
    int preferredBufferSize;
    int actualBufferSize;
    bool isRunning;
    String lastError;

    AudioStreamBasicDescription format;
    AudioUnit audioUnit;
    UInt32 audioInputIsAvailable;
    AudioIODeviceCallback* callback;
    BitArray activeOutputChans, activeInputChans;

    AudioSampleBuffer floatData;
    float* inputChannels[3];
    float* outputChannels[3];
    bool monoInputChannelNumber, monoOutputChannelNumber;

    void prepareFloatBuffers()
    {
        floatData.setSize (numInputChannels + numOutputChannels, actualBufferSize);
        zerostruct (inputChannels);
        zerostruct (outputChannels);

        for (int i = 0; i < numInputChannels; ++i)
            inputChannels[i] = floatData.getSampleData (i);

        for (int i = 0; i < numOutputChannels; ++i)
            outputChannels[i] = floatData.getSampleData (i + numInputChannels);
    }

    //==================================================================================================
    OSStatus process (AudioUnitRenderActionFlags* ioActionFlags, const AudioTimeStamp* inTimeStamp,
                      UInt32 inBusNumber, UInt32 inNumberFrames, AudioBufferList* ioData)
    {
        OSStatus err = noErr;

        if (audioInputIsAvailable)
            err = AudioUnitRender (audioUnit, ioActionFlags, inTimeStamp, 1, inNumberFrames, ioData);

        const ScopedLock sl (callbackLock);

        if (callback != 0)
        {
            if (audioInputIsAvailable && numInputChannels > 0)
            {
                short* shortData = (short*) ioData->mBuffers[0].mData;

                if (numInputChannels >= 2)
                {
                    for (UInt32 i = 0; i < inNumberFrames; ++i)
                    {
                        inputChannels[0][i] = *shortData++ * (1.0f / 32768.0f);
                        inputChannels[1][i] = *shortData++ * (1.0f / 32768.0f);
                    }
                }
                else
                {
                    if (monoInputChannelNumber > 0)
                        ++shortData;

                    for (UInt32 i = 0; i < inNumberFrames; ++i)
                    {
                        inputChannels[0][i] = *shortData++ * (1.0f / 32768.0f);
                        ++shortData;
                    }
                }
            }
            else
            {
                for (int i = numInputChannels; --i >= 0;)
                    zeromem (inputChannels[i], sizeof (float) * inNumberFrames);
            }

            callback->audioDeviceIOCallback ((const float**) inputChannels, numInputChannels,
                                             outputChannels, numOutputChannels,
                                             (int) inNumberFrames);

            short* shortData = (short*) ioData->mBuffers[0].mData;
            int n = 0;

            if (numOutputChannels >= 2)
            {
                for (UInt32 i = 0; i < inNumberFrames; ++i)
                {
                    shortData [n++] = (short) (outputChannels[0][i] * 32767.0f);
                    shortData [n++] = (short) (outputChannels[1][i] * 32767.0f);
                }
            }
            else if (numOutputChannels == 1)
            {
                for (UInt32 i = 0; i < inNumberFrames; ++i)
                {
                    const short s = (short) (outputChannels[monoOutputChannelNumber][i] * 32767.0f);
                    shortData [n++] = s;
                    shortData [n++] = s;
                }
            }
            else
            {
                zeromem (ioData->mBuffers[0].mData, 2 * sizeof (short) * inNumberFrames);
            }
        }
        else
        {
            zeromem (ioData->mBuffers[0].mData, 2 * sizeof (short) * inNumberFrames);
        }

        return err;
    }

    void updateDeviceInfo()
    {
        UInt32 size = sizeof (sampleRate);
        AudioSessionGetProperty (kAudioSessionProperty_CurrentHardwareSampleRate, &size, &sampleRate);

        size = sizeof (audioInputIsAvailable);
        AudioSessionGetProperty (kAudioSessionProperty_AudioInputAvailable, &size, &audioInputIsAvailable);
    }

    void propertyChanged (AudioSessionPropertyID inID, UInt32 inDataSize, const void* inPropertyValue)
    {
        if (! isRunning)
            return;

        if (inPropertyValue != 0)
        {
            CFDictionaryRef routeChangeDictionary = (CFDictionaryRef) inPropertyValue;
            CFNumberRef routeChangeReasonRef = (CFNumberRef) CFDictionaryGetValue (routeChangeDictionary,
                                                                                   CFSTR (kAudioSession_AudioRouteChangeKey_Reason));

            SInt32 routeChangeReason;
            CFNumberGetValue (routeChangeReasonRef, kCFNumberSInt32Type, &routeChangeReason);

            if (routeChangeReason == kAudioSessionRouteChangeReason_OldDeviceUnavailable)
                fixAudioRouteIfSetToReceiver();
        }

        updateDeviceInfo();
        createAudioUnit();

        AudioSessionSetActive (true);

        if (audioUnit != 0)
        {
            UInt32 formatSize = sizeof (format);
            AudioUnitGetProperty (audioUnit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Output, 1, &format, &formatSize);

            Float32 bufferDuration = preferredBufferSize / sampleRate;
            UInt32 bufferDurationSize = sizeof (bufferDuration);
            AudioSessionGetProperty (kAudioSessionProperty_CurrentHardwareIOBufferDuration, &bufferDurationSize, &bufferDurationSize);
            actualBufferSize = (int) (sampleRate * bufferDuration + 0.5);

            AudioOutputUnitStart (audioUnit);
        }
    }

    void interruptionListener (UInt32 inInterruption)
    {
        /*if (inInterruption == kAudioSessionBeginInterruption)
        {
            isRunning = false;
            AudioOutputUnitStop (audioUnit);

            if (juce_iPhoneShowModalAlert ("Audio Interrupted",
                                           "This could have been interrupted by another application or by unplugging a headset",
                                           @"Resume",
                                           @"Cancel"))
            {
                isRunning = true;
                propertyChanged (0, 0, 0);
            }
        }*/

        if (inInterruption == kAudioSessionEndInterruption)
        {
            isRunning = true;
            AudioSessionSetActive (true);
            AudioOutputUnitStart (audioUnit);
        }
    }

    //==================================================================================================
    static OSStatus processStatic (void* inRefCon, AudioUnitRenderActionFlags* ioActionFlags, const AudioTimeStamp* inTimeStamp,
                                   UInt32 inBusNumber, UInt32 inNumberFrames, AudioBufferList* ioData)
    {
        return ((IPhoneAudioIODevice*) inRefCon)->process (ioActionFlags, inTimeStamp, inBusNumber, inNumberFrames, ioData);
    }

    static void propertyChangedStatic (void* inClientData, AudioSessionPropertyID inID, UInt32 inDataSize, const void* inPropertyValue)
    {
        ((IPhoneAudioIODevice*) inClientData)->propertyChanged (inID, inDataSize, inPropertyValue);
    }

    static void interruptionListenerStatic (void* inClientData, UInt32 inInterruption)
    {
        ((IPhoneAudioIODevice*) inClientData)->interruptionListener (inInterruption);
    }

    //==================================================================================================
    void resetFormat (const int numChannels)
    {
        memset (&format, 0, sizeof (format));
        format.mFormatID = kAudioFormatLinearPCM;
        format.mFormatFlags = kLinearPCMFormatFlagIsSignedInteger | kLinearPCMFormatFlagIsPacked;
        format.mBitsPerChannel = 8 * sizeof (short);
        format.mChannelsPerFrame = 2;
        format.mFramesPerPacket = 1;
        format.mBytesPerFrame = format.mBytesPerPacket = 2 * sizeof (short);
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

        const UInt32 one = 1;
        AudioUnitSetProperty (audioUnit, kAudioOutputUnitProperty_EnableIO, kAudioUnitScope_Input, 1, &one, sizeof (one));

        AudioChannelLayout layout;
        layout.mChannelBitmap = 0;
        layout.mNumberChannelDescriptions = 0;
        layout.mChannelLayoutTag = kAudioChannelLayoutTag_Stereo;
        AudioUnitSetProperty (audioUnit, kAudioUnitProperty_AudioChannelLayout, kAudioUnitScope_Input, 0, &layout, sizeof (layout));
        AudioUnitSetProperty (audioUnit, kAudioUnitProperty_AudioChannelLayout, kAudioUnitScope_Output, 0, &layout, sizeof (layout));

        AURenderCallbackStruct inputProc;
        inputProc.inputProc = processStatic;
        inputProc.inputProcRefCon = this;
        AudioUnitSetProperty (audioUnit, kAudioUnitProperty_SetRenderCallback, kAudioUnitScope_Input, 0, &inputProc, sizeof (inputProc));

        AudioUnitSetProperty (audioUnit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Input, 0, &format, sizeof (format));
        AudioUnitSetProperty (audioUnit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Output, 1, &format, sizeof (format));

        AudioUnitInitialize (audioUnit);
        return true;
    }

    // If the routing is set to go through the receiver (i.e. the speaker, but quiet), this re-routes it
    // to make it loud. Needed because by default when using an input + output, the output is kept quiet.
    static void fixAudioRouteIfSetToReceiver()
    {
        CFStringRef audioRoute = 0;
        UInt32 propertySize = sizeof (audioRoute);
        if (AudioSessionGetProperty (kAudioSessionProperty_AudioRoute, &propertySize, &audioRoute) == noErr)
        {
            NSString* route = (NSString*) audioRoute;

            //DBG ("audio route: " + nsStringToJuce (route));

            if ([route hasPrefix: @"Receiver"])
            {
                UInt32 audioRouteOverride = kAudioSessionOverrideAudioRoute_Speaker;
                AudioSessionSetProperty (kAudioSessionProperty_OverrideAudioRoute, sizeof (audioRouteOverride), &audioRouteOverride);
            }

            CFRelease (audioRoute);
        }
    }

    IPhoneAudioIODevice (const IPhoneAudioIODevice&);
    const IPhoneAudioIODevice& operator= (const IPhoneAudioIODevice&);
};


//==============================================================================
class IPhoneAudioIODeviceType  : public AudioIODeviceType
{
public:
    //==============================================================================
    IPhoneAudioIODeviceType()
        : AudioIODeviceType (T("iPhone Audio"))
    {
    }

    ~IPhoneAudioIODeviceType()
    {
    }

    //==============================================================================
    void scanForDevices()
    {
    }

    const StringArray getDeviceNames (const bool wantInputNames) const
    {
        StringArray s;
        s.add ("iPhone Audio");
        return s;
    }

    int getDefaultDeviceIndex (const bool forInput) const
    {
        return 0;
    }

    int getIndexOfDevice (AudioIODevice* device, const bool asInput) const
    {
        return device != 0 ? 0 : -1;
    }

    bool hasSeparateInputsAndOutputs() const    { return false; }

    AudioIODevice* createDevice (const String& outputDeviceName,
                                 const String& inputDeviceName)
    {
        if (outputDeviceName.isNotEmpty() || inputDeviceName.isNotEmpty())
        {
            return new IPhoneAudioIODevice (outputDeviceName.isNotEmpty() ? outputDeviceName
                                                                          : inputDeviceName);
        }

        return 0;
    }

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    IPhoneAudioIODeviceType (const IPhoneAudioIODeviceType&);
    const IPhoneAudioIODeviceType& operator= (const IPhoneAudioIODeviceType&);
};

//==============================================================================
AudioIODeviceType* juce_createAudioIODeviceType_iPhoneAudio()
{
    return new IPhoneAudioIODeviceType();
}

#endif

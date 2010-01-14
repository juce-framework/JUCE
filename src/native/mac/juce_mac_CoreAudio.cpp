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


//==============================================================================
#ifndef JUCE_COREAUDIO_ERROR_LOGGING_ENABLED
  #define JUCE_COREAUDIO_ERROR_LOGGING_ENABLED 1
#endif

//==============================================================================
#undef log
#if JUCE_COREAUDIO_LOGGING_ENABLED
  #define log(a) Logger::writeToLog (a)
#else
  #define log(a)
#endif

#undef OK
#if JUCE_COREAUDIO_ERROR_LOGGING_ENABLED
  static bool logAnyErrors_CoreAudio (const OSStatus err, const int lineNum)
  {
      if (err == noErr)
          return true;

      Logger::writeToLog (T("CoreAudio error: ") + String (lineNum) + T(" - ") + String::toHexString ((int)err));
      jassertfalse
      return false;
  }

  #define OK(a) logAnyErrors_CoreAudio (a, __LINE__)
#else
  #define OK(a) (a == noErr)
#endif


//==============================================================================
class CoreAudioInternal  : public Timer
{
public:
    //==============================================================================
    CoreAudioInternal (AudioDeviceID id)
       : inputLatency (0),
         outputLatency (0),
         callback (0),
#if MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_5
         audioProcID (0),
#endif
         inputDevice (0),
         isSlaveDevice (false),
         deviceID (id),
         started (false),
         sampleRate (0),
         bufferSize (512),
         numInputChans (0),
         numOutputChans (0),
         callbacksAllowed (true),
         numInputChannelInfos (0),
         numOutputChannelInfos (0)
    {
        jassert (deviceID != 0);

        updateDetailsFromDevice();

        AudioObjectPropertyAddress pa;
        pa.mSelector = kAudioObjectPropertySelectorWildcard;
        pa.mScope = kAudioObjectPropertyScopeWildcard;
        pa.mElement = kAudioObjectPropertyElementWildcard;

        AudioObjectAddPropertyListener (deviceID, &pa, deviceListenerProc, this);
    }

    ~CoreAudioInternal()
    {
        AudioObjectPropertyAddress pa;
        pa.mSelector = kAudioObjectPropertySelectorWildcard;
        pa.mScope = kAudioObjectPropertyScopeWildcard;
        pa.mElement = kAudioObjectPropertyElementWildcard;

        AudioObjectRemovePropertyListener (deviceID, &pa, deviceListenerProc, this);

        stop (false);
        delete inputDevice;
    }

    void allocateTempBuffers()
    {
        const int tempBufSize = bufferSize + 4;
        audioBuffer.calloc ((numInputChans + numOutputChans) * tempBufSize);

        tempInputBuffers.calloc (numInputChans + 2);
        tempOutputBuffers.calloc (numOutputChans + 2);

        int i, count = 0;
        for (i = 0; i < numInputChans; ++i)
            tempInputBuffers[i] = audioBuffer + count++ * tempBufSize;

        for (i = 0; i < numOutputChans; ++i)
            tempOutputBuffers[i] = audioBuffer + count++ * tempBufSize;
    }

    // returns the number of actual available channels
    void fillInChannelInfo (const bool input)
    {
        int chanNum = 0;
        UInt32 size;

        AudioObjectPropertyAddress pa;
        pa.mSelector = kAudioDevicePropertyStreamConfiguration;
        pa.mScope = input ? kAudioDevicePropertyScopeInput : kAudioDevicePropertyScopeOutput;
        pa.mElement = kAudioObjectPropertyElementMaster;

        if (OK (AudioObjectGetPropertyDataSize (deviceID, &pa, 0, 0, &size)))
        {
            HeapBlock <AudioBufferList> bufList;
            bufList.calloc (size, 1);

            if (OK (AudioObjectGetPropertyData (deviceID, &pa, 0, 0, &size, bufList)))
            {
                const int numStreams = bufList->mNumberBuffers;

                for (int i = 0; i < numStreams; ++i)
                {
                    const AudioBuffer& b = bufList->mBuffers[i];

                    for (unsigned int j = 0; j < b.mNumberChannels; ++j)
                    {
                        String name;

                        {
                            uint8 channelName [256];
                            zerostruct (channelName);
                            UInt32 nameSize = sizeof (channelName);
                            UInt32 channelNum = chanNum + 1;
                            pa.mSelector = kAudioDevicePropertyChannelName;

                            if (AudioObjectGetPropertyData (deviceID, &pa, sizeof (channelNum), &channelNum, &nameSize, channelName) == noErr)
                                name = String::fromUTF8 (channelName, nameSize);
                        }

                        if (input)
                        {
                            if (activeInputChans[chanNum])
                            {
                                inputChannelInfo [numInputChannelInfos].streamNum = i;
                                inputChannelInfo [numInputChannelInfos].dataOffsetSamples = j;
                                inputChannelInfo [numInputChannelInfos].dataStrideSamples = b.mNumberChannels;
                                ++numInputChannelInfos;
                            }

                            if (name.isEmpty())
                                name << "Input " << (chanNum + 1);

                            inChanNames.add (name);
                        }
                        else
                        {
                            if (activeOutputChans[chanNum])
                            {
                                outputChannelInfo [numOutputChannelInfos].streamNum = i;
                                outputChannelInfo [numOutputChannelInfos].dataOffsetSamples = j;
                                outputChannelInfo [numOutputChannelInfos].dataStrideSamples = b.mNumberChannels;
                                ++numOutputChannelInfos;
                            }

                            if (name.isEmpty())
                                name << "Output " << (chanNum + 1);

                            outChanNames.add (name);
                        }

                        ++chanNum;
                    }
                }
            }
        }
    }

    void updateDetailsFromDevice()
    {
        stopTimer();

        if (deviceID == 0)
            return;

        const ScopedLock sl (callbackLock);

        Float64 sr;
        UInt32 size = sizeof (Float64);

        AudioObjectPropertyAddress pa;
        pa.mSelector = kAudioDevicePropertyNominalSampleRate;
        pa.mScope = kAudioObjectPropertyScopeWildcard;
        pa.mElement = kAudioObjectPropertyElementMaster;

        if (OK (AudioObjectGetPropertyData (deviceID, &pa, 0, 0, &size, &sr)))
            sampleRate = sr;

        UInt32 framesPerBuf;
        size = sizeof (framesPerBuf);

        pa.mSelector = kAudioDevicePropertyBufferFrameSize;
        if (OK (AudioObjectGetPropertyData (deviceID, &pa, 0, 0, &size, &framesPerBuf)))
        {
            bufferSize = framesPerBuf;
            allocateTempBuffers();
        }

        bufferSizes.clear();

        pa.mSelector = kAudioDevicePropertyBufferFrameSizeRange;

        if (OK (AudioObjectGetPropertyDataSize (deviceID, &pa, 0, 0, &size)))
        {
            HeapBlock <AudioValueRange> ranges;
            ranges.calloc (size, 1);

            if (OK (AudioObjectGetPropertyData (deviceID, &pa, 0, 0, &size, ranges)))
            {
                bufferSizes.add ((int) ranges[0].mMinimum);

                for (int i = 32; i < 8192; i += 32)
                {
                    for (int j = size / (int) sizeof (AudioValueRange); --j >= 0;)
                    {
                        if (i >= ranges[j].mMinimum && i <= ranges[j].mMaximum)
                        {
                            bufferSizes.addIfNotAlreadyThere (i);
                            break;
                        }
                    }
                }

                if (bufferSize > 0)
                    bufferSizes.addIfNotAlreadyThere (bufferSize);
            }
        }

        if (bufferSizes.size() == 0 && bufferSize > 0)
            bufferSizes.add (bufferSize);

        sampleRates.clear();
        const double possibleRates[] = { 44100.0, 48000.0, 88200.0, 96000.0, 176400.0, 192000.0 };
        String rates;

        pa.mSelector = kAudioDevicePropertyAvailableNominalSampleRates;

        if (OK (AudioObjectGetPropertyDataSize (deviceID, &pa, 0, 0, &size)))
        {
            HeapBlock <AudioValueRange> ranges;
            ranges.calloc (size, 1);

            if (OK (AudioObjectGetPropertyData (deviceID, &pa, 0, 0, &size, ranges)))
            {
                for (int i = 0; i < numElementsInArray (possibleRates); ++i)
                {
                    bool ok = false;

                    for (int j = size / (int) sizeof (AudioValueRange); --j >= 0;)
                        if (possibleRates[i] >= ranges[j].mMinimum - 2 && possibleRates[i] <= ranges[j].mMaximum + 2)
                            ok = true;

                    if (ok)
                    {
                        sampleRates.add (possibleRates[i]);
                        rates << possibleRates[i] << T(" ");
                    }
                }
            }
        }

        if (sampleRates.size() == 0 && sampleRate > 0)
        {
            sampleRates.add (sampleRate);
            rates << sampleRate;
        }

        log (T("sr: ") + rates);

        inputLatency = 0;
        outputLatency = 0;
        UInt32 lat;
        size = sizeof (lat);
        pa.mSelector = kAudioDevicePropertyLatency;
        pa.mScope = kAudioDevicePropertyScopeInput;
        //if (AudioDeviceGetProperty (deviceID, 0, true, kAudioDevicePropertyLatency, &size, &lat) == noErr)
        if (AudioObjectGetPropertyData (deviceID, &pa, 0, 0, &size, &lat) == noErr)
            inputLatency = (int) lat;

        pa.mScope = kAudioDevicePropertyScopeOutput;
        size = sizeof (lat);

        if (AudioObjectGetPropertyData (deviceID, &pa, 0, 0, &size, &lat) == noErr)
            outputLatency = (int) lat;

        log (T("lat: ") + String (inputLatency) + T(" ") + String (outputLatency));

        inChanNames.clear();
        outChanNames.clear();

        inputChannelInfo.calloc (numInputChans + 2);
        numInputChannelInfos = 0;

        outputChannelInfo.calloc (numOutputChans + 2);
        numOutputChannelInfos = 0;

        fillInChannelInfo (true);
        fillInChannelInfo (false);
    }

    //==============================================================================
    const StringArray getSources (bool input)
    {
        StringArray s;
        HeapBlock <OSType> types;
        const int num = getAllDataSourcesForDevice (deviceID, input, types);

        for (int i = 0; i < num; ++i)
        {
            AudioValueTranslation avt;
            char buffer[256];

            avt.mInputData = (void*) &(types[i]);
            avt.mInputDataSize = sizeof (UInt32);
            avt.mOutputData = buffer;
            avt.mOutputDataSize = 256;

            UInt32 transSize = sizeof (avt);

            AudioObjectPropertyAddress pa;
            pa.mSelector = kAudioDevicePropertyDataSourceNameForID;
            pa.mScope = input ? kAudioDevicePropertyScopeInput : kAudioDevicePropertyScopeOutput;
            pa.mElement = kAudioObjectPropertyElementMaster;

            if (OK (AudioObjectGetPropertyData (deviceID, &pa, 0, 0, &transSize, &avt)))
            {
                DBG (buffer);
                s.add (buffer);
            }
        }

        return s;
    }

    int getCurrentSourceIndex (bool input) const
    {
        OSType currentSourceID = 0;
        UInt32 size = sizeof (currentSourceID);
        int result = -1;

        AudioObjectPropertyAddress pa;
        pa.mSelector = kAudioDevicePropertyDataSource;
        pa.mScope = input ? kAudioDevicePropertyScopeInput : kAudioDevicePropertyScopeOutput;
        pa.mElement = kAudioObjectPropertyElementMaster;

        if (deviceID != 0)
        {
            if (OK (AudioObjectGetPropertyData (deviceID, &pa, 0, 0, &size, &currentSourceID)))
            {
                HeapBlock <OSType> types;
                const int num = getAllDataSourcesForDevice (deviceID, input, types);

                for (int i = 0; i < num; ++i)
                {
                    if (types[num] == currentSourceID)
                    {
                        result = i;
                        break;
                    }
                }
            }
        }

        return result;
    }

    void setCurrentSourceIndex (int index, bool input)
    {
        if (deviceID != 0)
        {
            HeapBlock <OSType> types;
            const int num = getAllDataSourcesForDevice (deviceID, input, types);

            if (((unsigned int) index) < (unsigned int) num)
            {
                AudioObjectPropertyAddress pa;
                pa.mSelector = kAudioDevicePropertyDataSource;
                pa.mScope = input ? kAudioDevicePropertyScopeInput : kAudioDevicePropertyScopeOutput;
                pa.mElement = kAudioObjectPropertyElementMaster;

                OSType typeId = types[index];

                OK (AudioObjectSetPropertyData (deviceID, &pa, 0, 0, sizeof (typeId), &typeId));
            }
        }
    }

    //==============================================================================
    const String reopen (const BitArray& inputChannels,
                         const BitArray& outputChannels,
                         double newSampleRate,
                         int bufferSizeSamples)
    {
        String error;
        log ("CoreAudio reopen");
        callbacksAllowed = false;
        stopTimer();

        stop (false);

        activeInputChans = inputChannels;
        activeInputChans.setRange (inChanNames.size(),
                                   activeInputChans.getHighestBit() + 1 - inChanNames.size(),
                                   false);

        activeOutputChans = outputChannels;
        activeOutputChans.setRange (outChanNames.size(),
                                    activeOutputChans.getHighestBit() + 1 - outChanNames.size(),
                                    false);

        numInputChans = activeInputChans.countNumberOfSetBits();
        numOutputChans = activeOutputChans.countNumberOfSetBits();

        // set sample rate
        AudioObjectPropertyAddress pa;
        pa.mSelector = kAudioDevicePropertyNominalSampleRate;
        pa.mScope = kAudioObjectPropertyScopeWildcard;
        pa.mElement = kAudioObjectPropertyElementMaster;
        Float64 sr = newSampleRate;

        if (! OK (AudioObjectSetPropertyData (deviceID, &pa, 0, 0, sizeof (sr), &sr)))
        {
            error = "Couldn't change sample rate";
        }
        else
        {
            // change buffer size
            UInt32 framesPerBuf = bufferSizeSamples;
            pa.mSelector = kAudioDevicePropertyBufferFrameSize;

            if (! OK (AudioObjectSetPropertyData (deviceID, &pa, 0, 0, sizeof (framesPerBuf), &framesPerBuf)))
            {
                error = "Couldn't change buffer size";
            }
            else
            {
                // Annoyingly, after changing the rate and buffer size, some devices fail to
                // correctly report their new settings until some random time in the future, so
                // after calling updateDetailsFromDevice, we need to manually bodge these values
                // to make sure we're using the correct numbers..
                updateDetailsFromDevice();
                sampleRate = newSampleRate;
                bufferSize = bufferSizeSamples;

                if (sampleRates.size() == 0)
                    error = "Device has no available sample-rates";
                else if (bufferSizes.size() == 0)
                    error = "Device has no available buffer-sizes";
                else if (inputDevice != 0)
                    error = inputDevice->reopen (inputChannels,
                                                 outputChannels,
                                                 newSampleRate,
                                                 bufferSizeSamples);
            }
        }

        callbacksAllowed = true;
        return error;
    }

    bool start (AudioIODeviceCallback* cb)
    {
        if (! started)
        {
            callback = 0;

            if (deviceID != 0)
            {
#if MAC_OS_X_VERSION_MIN_REQUIRED < MAC_OS_X_VERSION_10_5
                if (OK (AudioDeviceAddIOProc (deviceID, audioIOProc, (void*) this)))
#else
                if (OK (AudioDeviceCreateIOProcID (deviceID, audioIOProc, (void*) this, &audioProcID)))
#endif
                {
                    if (OK (AudioDeviceStart (deviceID, audioIOProc)))
                    {
                        started = true;
                    }
                    else
                    {
#if MAC_OS_X_VERSION_MIN_REQUIRED < MAC_OS_X_VERSION_10_5
                        OK (AudioDeviceRemoveIOProc (deviceID, audioIOProc));
#else
                        OK (AudioDeviceDestroyIOProcID (deviceID, audioProcID));
                        audioProcID = 0;
#endif
                    }
                }
            }
        }

        if (started)
        {
            const ScopedLock sl (callbackLock);
            callback = cb;
        }

        if (inputDevice != 0)
            return started && inputDevice->start (cb);
        else
            return started;
    }

    void stop (bool leaveInterruptRunning)
    {
        callbackLock.enter();
        callback = 0;
        callbackLock.exit();

        if (started
             && (deviceID != 0)
             && ! leaveInterruptRunning)
        {
            OK (AudioDeviceStop (deviceID, audioIOProc));

#if MAC_OS_X_VERSION_MIN_REQUIRED < MAC_OS_X_VERSION_10_5
            OK (AudioDeviceRemoveIOProc (deviceID, audioIOProc));
#else
            OK (AudioDeviceDestroyIOProcID (deviceID, audioProcID));
            audioProcID = 0;
#endif
            started = false;

            callbackLock.enter();
            callbackLock.exit();

            // wait until it's definately stopped calling back..
            for (int i = 40; --i >= 0;)
            {
                Thread::sleep (50);

                UInt32 running = 0;
                UInt32 size = sizeof (running);

                AudioObjectPropertyAddress pa;
                pa.mSelector = kAudioDevicePropertyDeviceIsRunning;
                pa.mScope = kAudioObjectPropertyScopeWildcard;
                pa.mElement = kAudioObjectPropertyElementMaster;

                OK (AudioObjectGetPropertyData (deviceID, &pa, 0, 0, &size, &running));

                if (running == 0)
                    break;
            }

            callbackLock.enter();
            callbackLock.exit();
        }

        if (inputDevice != 0)
            inputDevice->stop (leaveInterruptRunning);
    }

    double getSampleRate() const
    {
        return sampleRate;
    }

    int getBufferSize() const
    {
        return bufferSize;
    }

    void audioCallback (const AudioBufferList* inInputData,
                        AudioBufferList* outOutputData)
    {
        int i;
        const ScopedLock sl (callbackLock);

        if (callback != 0)
        {
            if (inputDevice == 0)
            {
                for (i = numInputChans; --i >= 0;)
                {
                    const CallbackDetailsForChannel& info = inputChannelInfo[i];
                    float* dest = tempInputBuffers [i];
                    const float* src = ((const float*) inInputData->mBuffers[info.streamNum].mData)
                                        + info.dataOffsetSamples;
                    const int stride = info.dataStrideSamples;

                    if (stride != 0) // if this is zero, info is invalid
                    {
                        for (int j = bufferSize; --j >= 0;)
                        {
                            *dest++ = *src;
                            src += stride;
                        }
                    }
                }
            }

            if (! isSlaveDevice)
            {
                if (inputDevice == 0)
                {
                    callback->audioDeviceIOCallback ((const float**) tempInputBuffers,
                                                     numInputChans,
                                                     tempOutputBuffers,
                                                     numOutputChans,
                                                     bufferSize);
                }
                else
                {
                    jassert (inputDevice->bufferSize == bufferSize);

                    // Sometimes the two linked devices seem to get their callbacks in
                    // parallel, so we need to lock both devices to stop the input data being
                    // changed while inside our callback..
                    const ScopedLock sl2 (inputDevice->callbackLock);

                    callback->audioDeviceIOCallback ((const float**) inputDevice->tempInputBuffers,
                                                     inputDevice->numInputChans,
                                                     tempOutputBuffers,
                                                     numOutputChans,
                                                     bufferSize);
                }

                for (i = numOutputChans; --i >= 0;)
                {
                    const CallbackDetailsForChannel& info = outputChannelInfo[i];
                    const float* src = tempOutputBuffers [i];
                    float* dest = ((float*) outOutputData->mBuffers[info.streamNum].mData)
                                    + info.dataOffsetSamples;
                    const int stride = info.dataStrideSamples;

                    if (stride != 0) // if this is zero, info is invalid
                    {
                        for (int j = bufferSize; --j >= 0;)
                        {
                            *dest = *src++;
                            dest += stride;
                        }
                    }
                }
            }
        }
        else
        {
            for (i = jmin (numOutputChans, numOutputChannelInfos); --i >= 0;)
            {
                const CallbackDetailsForChannel& info = outputChannelInfo[i];
                float* dest = ((float*) outOutputData->mBuffers[info.streamNum].mData)
                                + info.dataOffsetSamples;
                const int stride = info.dataStrideSamples;

                if (stride != 0) // if this is zero, info is invalid
                {
                    for (int j = bufferSize; --j >= 0;)
                    {
                        *dest = 0.0f;
                        dest += stride;
                    }
                }
            }
        }
    }

    // called by callbacks
    void deviceDetailsChanged()
    {
        if (callbacksAllowed)
            startTimer (100);
    }

    void timerCallback()
    {
        stopTimer();
        log ("CoreAudio device changed callback");

        const double oldSampleRate = sampleRate;
        const int oldBufferSize = bufferSize;
        updateDetailsFromDevice();

        if (oldBufferSize != bufferSize || oldSampleRate != sampleRate)
        {
            callbacksAllowed = false;
            stop (false);
            updateDetailsFromDevice();
            callbacksAllowed = true;
        }
    }

    CoreAudioInternal* getRelatedDevice() const
    {
        UInt32 size = 0;
        ScopedPointer <CoreAudioInternal> result;

        AudioObjectPropertyAddress pa;
        pa.mSelector = kAudioDevicePropertyRelatedDevices;
        pa.mScope = kAudioObjectPropertyScopeWildcard;
        pa.mElement = kAudioObjectPropertyElementMaster;

        if (deviceID != 0
             && AudioObjectGetPropertyDataSize (deviceID, &pa, 0, 0, &size) == noErr
             && size > 0)
        {
            HeapBlock <AudioDeviceID> devs;
            devs.calloc (size, 1);

            if (OK (AudioObjectGetPropertyData (deviceID, &pa, 0, 0, &size, devs)))
            {
                for (unsigned int i = 0; i < size / sizeof (AudioDeviceID); ++i)
                {
                    if (devs[i] != deviceID && devs[i] != 0)
                    {
                        result = new CoreAudioInternal (devs[i]);

                        const bool thisIsInput = inChanNames.size() > 0 && outChanNames.size() == 0;
                        const bool otherIsInput = result->inChanNames.size() > 0 && result->outChanNames.size() == 0;

                        if (thisIsInput != otherIsInput
                             || (inChanNames.size() + outChanNames.size() == 0)
                             || (result->inChanNames.size() + result->outChanNames.size()) == 0)
                            break;

                        result = 0;
                    }
                }
            }
        }

        return result.release();
    }

    //==============================================================================
    juce_UseDebuggingNewOperator

    int inputLatency, outputLatency;
    BitArray activeInputChans, activeOutputChans;
    StringArray inChanNames, outChanNames;
    Array <double> sampleRates;
    Array <int> bufferSizes;
    AudioIODeviceCallback* callback;
#if MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_5
    AudioDeviceIOProcID audioProcID;
#endif

    CoreAudioInternal* inputDevice;
    bool isSlaveDevice;

private:
    CriticalSection callbackLock;
    AudioDeviceID deviceID;
    bool started;
    double sampleRate;
    int bufferSize;
    HeapBlock <float> audioBuffer;
    int numInputChans, numOutputChans;
    bool callbacksAllowed;

    struct CallbackDetailsForChannel
    {
        int streamNum;
        int dataOffsetSamples;
        int dataStrideSamples;
    };

    int numInputChannelInfos, numOutputChannelInfos;
    HeapBlock <CallbackDetailsForChannel> inputChannelInfo, outputChannelInfo;
    HeapBlock <float*> tempInputBuffers, tempOutputBuffers;

    CoreAudioInternal (const CoreAudioInternal&);
    const CoreAudioInternal& operator= (const CoreAudioInternal&);

    //==============================================================================
    static OSStatus audioIOProc (AudioDeviceID inDevice,
                                 const AudioTimeStamp* inNow,
                                 const AudioBufferList* inInputData,
                                 const AudioTimeStamp* inInputTime,
                                 AudioBufferList* outOutputData,
                                 const AudioTimeStamp* inOutputTime,
                                 void* device)
    {
        ((CoreAudioInternal*) device)->audioCallback (inInputData, outOutputData);
        return noErr;
    }

    static OSStatus deviceListenerProc (AudioDeviceID /*inDevice*/, UInt32 /*inLine*/, const AudioObjectPropertyAddress* pa, void* inClientData)
    {
        CoreAudioInternal* const intern = (CoreAudioInternal*) inClientData;

        switch (pa->mSelector)
        {
            case kAudioDevicePropertyBufferSize:
            case kAudioDevicePropertyBufferFrameSize:
            case kAudioDevicePropertyNominalSampleRate:
            case kAudioDevicePropertyStreamFormat:
            case kAudioDevicePropertyDeviceIsAlive:
                intern->deviceDetailsChanged();
                break;

            case kAudioDevicePropertyBufferSizeRange:
            case kAudioDevicePropertyVolumeScalar:
            case kAudioDevicePropertyMute:
            case kAudioDevicePropertyPlayThru:
            case kAudioDevicePropertyDataSource:
            case kAudioDevicePropertyDeviceIsRunning:
                break;
        }

        return noErr;
    }

    //==============================================================================
    static int getAllDataSourcesForDevice (AudioDeviceID deviceID, const bool input, HeapBlock <OSType>& types)
    {
        AudioObjectPropertyAddress pa;
        pa.mSelector = kAudioDevicePropertyDataSources;
        pa.mScope = kAudioObjectPropertyScopeWildcard;
        pa.mElement = kAudioObjectPropertyElementMaster;
        UInt32 size = 0;

        if (deviceID != 0
             && OK (AudioObjectGetPropertyDataSize (deviceID, &pa, 0, 0, &size)))
        {
            types.calloc (size, 1);

            if (OK (AudioObjectGetPropertyData (deviceID, &pa, 0, 0, &size, types)))
                return size / (int) sizeof (OSType);
        }

        return 0;
    }
};


//==============================================================================
class CoreAudioIODevice   : public AudioIODevice
{
public:
    CoreAudioIODevice (const String& deviceName,
                       AudioDeviceID inputDeviceId,
                       const int inputIndex_,
                       AudioDeviceID outputDeviceId,
                       const int outputIndex_)
        : AudioIODevice (deviceName, "CoreAudio"),
          inputIndex (inputIndex_),
          outputIndex (outputIndex_),
          isOpen_ (false),
          isStarted (false)
    {
        internal = 0;
        CoreAudioInternal* device = 0;

        if (outputDeviceId == 0 || outputDeviceId == inputDeviceId)
        {
            jassert (inputDeviceId != 0);

            device = new CoreAudioInternal (inputDeviceId);
        }
        else
        {
            device = new CoreAudioInternal (outputDeviceId);

            if (inputDeviceId != 0)
            {
                CoreAudioInternal* secondDevice = new CoreAudioInternal (inputDeviceId);

                device->inputDevice = secondDevice;
                secondDevice->isSlaveDevice = true;
            }
        }

        internal = device;

        AudioObjectPropertyAddress pa;
        pa.mSelector = kAudioObjectPropertySelectorWildcard;
        pa.mScope = kAudioObjectPropertyScopeWildcard;
        pa.mElement = kAudioObjectPropertyElementWildcard;

        AudioObjectAddPropertyListener (kAudioObjectSystemObject, &pa, hardwareListenerProc, internal);
    }

    ~CoreAudioIODevice()
    {
        AudioObjectPropertyAddress pa;
        pa.mSelector = kAudioObjectPropertySelectorWildcard;
        pa.mScope = kAudioObjectPropertyScopeWildcard;
        pa.mElement = kAudioObjectPropertyElementWildcard;

        AudioObjectRemovePropertyListener (kAudioObjectSystemObject, &pa, hardwareListenerProc, internal);

        delete internal;
    }

    const StringArray getOutputChannelNames()
    {
        return internal->outChanNames;
    }

    const StringArray getInputChannelNames()
    {
        if (internal->inputDevice != 0)
            return internal->inputDevice->inChanNames;
        else
            return internal->inChanNames;
    }

    int getNumSampleRates()
    {
        return internal->sampleRates.size();
    }

    double getSampleRate (int index)
    {
        return internal->sampleRates [index];
    }

    int getNumBufferSizesAvailable()
    {
        return internal->bufferSizes.size();
    }

    int getBufferSizeSamples (int index)
    {
        return internal->bufferSizes [index];
    }

    int getDefaultBufferSize()
    {
        for (int i = 0; i < getNumBufferSizesAvailable(); ++i)
            if (getBufferSizeSamples(i) >= 512)
                return getBufferSizeSamples(i);

        return 512;
    }

    const String open (const BitArray& inputChannels,
                       const BitArray& outputChannels,
                       double sampleRate,
                       int bufferSizeSamples)
    {
        isOpen_ = true;

        if (bufferSizeSamples <= 0)
            bufferSizeSamples = getDefaultBufferSize();

        lastError = internal->reopen (inputChannels, outputChannels, sampleRate, bufferSizeSamples);
        isOpen_ = lastError.isEmpty();
        return lastError;
    }

    void close()
    {
        isOpen_ = false;
        internal->stop (false);
    }

    bool isOpen()
    {
        return isOpen_;
    }

    int getCurrentBufferSizeSamples()
    {
        return internal != 0 ? internal->getBufferSize() : 512;
    }

    double getCurrentSampleRate()
    {
        return internal != 0 ? internal->getSampleRate() : 0;
    }

    int getCurrentBitDepth()
    {
        return 32;  // no way to find out, so just assume it's high..
    }

    const BitArray getActiveOutputChannels() const
    {
        return internal != 0 ? internal->activeOutputChans : BitArray();
    }

    const BitArray getActiveInputChannels() const
    {
        BitArray chans;

        if (internal != 0)
        {
            chans = internal->activeInputChans;

            if (internal->inputDevice != 0)
                chans.orWith (internal->inputDevice->activeInputChans);
        }

        return chans;
    }

    int getOutputLatencyInSamples()
    {
        if (internal == 0)
            return 0;

        // this seems like a good guess at getting the latency right - comparing
        // this with a round-trip measurement, it gets it to within a few millisecs
        // for the built-in mac soundcard
        return internal->outputLatency + internal->getBufferSize() * 2;
    }

    int getInputLatencyInSamples()
    {
        if (internal == 0)
            return 0;

        return internal->inputLatency + internal->getBufferSize() * 2;
    }

    void start (AudioIODeviceCallback* callback)
    {
        if (internal != 0 && ! isStarted)
        {
            if (callback != 0)
                callback->audioDeviceAboutToStart (this);

            isStarted = true;
            internal->start (callback);
        }
    }

    void stop()
    {
        if (isStarted && internal != 0)
        {
            AudioIODeviceCallback* const lastCallback = internal->callback;

            isStarted = false;
            internal->stop (true);

            if (lastCallback != 0)
                lastCallback->audioDeviceStopped();
        }
    }

    bool isPlaying()
    {
        if (internal->callback == 0)
            isStarted = false;

        return isStarted;
    }

    const String getLastError()
    {
        return lastError;
    }

    int inputIndex, outputIndex;

    juce_UseDebuggingNewOperator

private:
    CoreAudioInternal* internal;
    bool isOpen_, isStarted;
    String lastError;

    static OSStatus hardwareListenerProc (AudioDeviceID /*inDevice*/, UInt32 /*inLine*/, const AudioObjectPropertyAddress* pa, void* inClientData)
    {
        CoreAudioInternal* const intern = (CoreAudioInternal*) inClientData;

        switch (pa->mSelector)
        {
            case kAudioHardwarePropertyDevices:
                intern->deviceDetailsChanged();
                break;

            case kAudioHardwarePropertyDefaultOutputDevice:
            case kAudioHardwarePropertyDefaultInputDevice:
            case kAudioHardwarePropertyDefaultSystemOutputDevice:
                break;
        }

        return noErr;
    }

    CoreAudioIODevice (const CoreAudioIODevice&);
    const CoreAudioIODevice& operator= (const CoreAudioIODevice&);
};

//==============================================================================
class CoreAudioIODeviceType  : public AudioIODeviceType
{
public:
    //==============================================================================
    CoreAudioIODeviceType()
        : AudioIODeviceType (T("CoreAudio")),
          hasScanned (false)
    {
    }

    ~CoreAudioIODeviceType()
    {
    }

    //==============================================================================
    void scanForDevices()
    {
        hasScanned = true;

        inputDeviceNames.clear();
        outputDeviceNames.clear();
        inputIds.clear();
        outputIds.clear();

        UInt32 size;

        AudioObjectPropertyAddress pa;
        pa.mSelector = kAudioHardwarePropertyDevices;
        pa.mScope = kAudioObjectPropertyScopeWildcard;
        pa.mElement = kAudioObjectPropertyElementMaster;

        if (OK (AudioObjectGetPropertyDataSize (kAudioObjectSystemObject, &pa, 0, 0, &size)))
        {
            HeapBlock <AudioDeviceID> devs;
            devs.calloc (size, 1);

            if (OK (AudioObjectGetPropertyData (kAudioObjectSystemObject, &pa, 0, 0, &size, devs)))
            {
                static bool alreadyLogged = false;
                const int num = size / (int) sizeof (AudioDeviceID);
                for (int i = 0; i < num; ++i)
                {
                    char name [1024];
                    size = sizeof (name);
                    pa.mSelector = kAudioDevicePropertyDeviceName;

                    if (OK (AudioObjectGetPropertyData (devs[i], &pa, 0, 0, &size, name)))
                    {
                        const String nameString (String::fromUTF8 ((const uint8*) name, (int) strlen (name)));

                        if (! alreadyLogged)
                            log (T("CoreAudio device: ") + nameString);

                        const int numIns = getNumChannels (devs[i], true);
                        const int numOuts = getNumChannels (devs[i], false);

                        if (numIns > 0)
                        {
                            inputDeviceNames.add (nameString);
                            inputIds.add (devs[i]);
                        }

                        if (numOuts > 0)
                        {
                            outputDeviceNames.add (nameString);
                            outputIds.add (devs[i]);
                        }
                    }
                }

                alreadyLogged = true;
            }
        }

        inputDeviceNames.appendNumbersToDuplicates (false, true);
        outputDeviceNames.appendNumbersToDuplicates (false, true);
    }

    const StringArray getDeviceNames (const bool wantInputNames) const
    {
        jassert (hasScanned); // need to call scanForDevices() before doing this

        if (wantInputNames)
            return inputDeviceNames;
        else
            return outputDeviceNames;
    }

    int getDefaultDeviceIndex (const bool forInput) const
    {
        jassert (hasScanned); // need to call scanForDevices() before doing this

        AudioDeviceID deviceID;
        UInt32 size = sizeof (deviceID);

        // if they're asking for any input channels at all, use the default input, so we
        // get the built-in mic rather than the built-in output with no inputs..

        AudioObjectPropertyAddress pa;
        pa.mSelector = forInput ? kAudioHardwarePropertyDefaultInputDevice : kAudioHardwarePropertyDefaultOutputDevice;
        pa.mScope = kAudioObjectPropertyScopeWildcard;
        pa.mElement = kAudioObjectPropertyElementMaster;

        if (AudioObjectGetPropertyData (kAudioObjectSystemObject, &pa, 0, 0, &size, &deviceID) == noErr)
        {
            if (forInput)
            {
                for (int i = inputIds.size(); --i >= 0;)
                    if (inputIds[i] == deviceID)
                        return i;
            }
            else
            {
                for (int i = outputIds.size(); --i >= 0;)
                    if (outputIds[i] == deviceID)
                        return i;
            }
        }

        return 0;
    }

    int getIndexOfDevice (AudioIODevice* device, const bool asInput) const
    {
        jassert (hasScanned); // need to call scanForDevices() before doing this

        CoreAudioIODevice* const d = dynamic_cast <CoreAudioIODevice*> (device);
        if (d == 0)
            return -1;

        return asInput ? d->inputIndex
                       : d->outputIndex;
    }

    bool hasSeparateInputsAndOutputs() const    { return true; }

    AudioIODevice* createDevice (const String& outputDeviceName,
                                 const String& inputDeviceName)
    {
        jassert (hasScanned); // need to call scanForDevices() before doing this

        const int inputIndex = inputDeviceNames.indexOf (inputDeviceName);
        const int outputIndex = outputDeviceNames.indexOf (outputDeviceName);

        String deviceName (outputDeviceName);
        if (deviceName.isEmpty())
            deviceName = inputDeviceName;

        if (index >= 0)
            return new CoreAudioIODevice (deviceName,
                                          inputIds [inputIndex],
                                          inputIndex,
                                          outputIds [outputIndex],
                                          outputIndex);

        return 0;
    }

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    StringArray inputDeviceNames, outputDeviceNames;
    Array <AudioDeviceID> inputIds, outputIds;

    bool hasScanned;

    static int getNumChannels (AudioDeviceID deviceID, bool input)
    {
        int total = 0;
        UInt32 size;

        AudioObjectPropertyAddress pa;
        pa.mSelector = kAudioDevicePropertyStreamConfiguration;
        pa.mScope = input ? kAudioDevicePropertyScopeInput : kAudioDevicePropertyScopeOutput;
        pa.mElement = kAudioObjectPropertyElementMaster;

        if (OK (AudioObjectGetPropertyDataSize (deviceID, &pa, 0, 0, &size)))
        {
            HeapBlock <AudioBufferList> bufList;
            bufList.calloc (size, 1);

            if (OK (AudioObjectGetPropertyData (deviceID, &pa, 0, 0, &size, bufList)))
            {
                const int numStreams = bufList->mNumberBuffers;

                for (int i = 0; i < numStreams; ++i)
                {
                    const AudioBuffer& b = bufList->mBuffers[i];
                    total += b.mNumberChannels;
                }
            }
        }

        return total;
    }

    CoreAudioIODeviceType (const CoreAudioIODeviceType&);
    const CoreAudioIODeviceType& operator= (const CoreAudioIODeviceType&);
};

//==============================================================================
AudioIODeviceType* juce_createAudioIODeviceType_CoreAudio()
{
    return new CoreAudioIODeviceType();
}

#undef log

#endif

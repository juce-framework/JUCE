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

#if JUCE_COREAUDIO_LOGGING_ENABLED
 #define JUCE_COREAUDIOLOG(a) Logger::writeToLog (a)
#else
 #define JUCE_COREAUDIOLOG(a)
#endif

//==============================================================================
struct SystemVol
{
    SystemVol (AudioObjectPropertySelector selector)
        : outputDeviceID (kAudioObjectUnknown)
    {
        addr.mScope    = kAudioObjectPropertyScopeGlobal;
        addr.mElement  = kAudioObjectPropertyElementMaster;
        addr.mSelector = kAudioHardwarePropertyDefaultOutputDevice;

        if (AudioHardwareServiceHasProperty (kAudioObjectSystemObject, &addr))
        {
            UInt32 deviceIDSize = sizeof (outputDeviceID);
            OSStatus status = AudioHardwareServiceGetPropertyData (kAudioObjectSystemObject, &addr, 0,
                                                                   nullptr, &deviceIDSize, &outputDeviceID);

            if (status == noErr)
            {
                addr.mElement  = kAudioObjectPropertyElementMaster;
                addr.mSelector = selector;
                addr.mScope    = kAudioDevicePropertyScopeOutput;

                if (! AudioHardwareServiceHasProperty (outputDeviceID, &addr))
                    outputDeviceID = kAudioObjectUnknown;
            }
        }
    }

    float getGain()
    {
        Float32 gain = 0;

        if (outputDeviceID != kAudioObjectUnknown)
        {
            UInt32 size = sizeof (gain);
            AudioHardwareServiceGetPropertyData (outputDeviceID, &addr,
                                                 0, nullptr, &size, &gain);
        }

        return (float) gain;
    }

    bool setGain (float gain)
    {
        if (outputDeviceID != kAudioObjectUnknown && canSetVolume())
        {
            Float32 newVolume = gain;
            UInt32 size = sizeof (newVolume);

            return AudioHardwareServiceSetPropertyData (outputDeviceID, &addr, 0, nullptr,
                                                        size, &newVolume) == noErr;
        }

        return false;
    }

    bool isMuted()
    {
        UInt32 muted = 0;

        if (outputDeviceID != kAudioObjectUnknown)
        {
            UInt32 size = sizeof (muted);
            AudioHardwareServiceGetPropertyData (outputDeviceID, &addr,
                                                 0, nullptr, &size, &muted);
        }

        return muted != 0;
    }

    bool setMuted (bool mute)
    {
        if (outputDeviceID != kAudioObjectUnknown && canSetVolume())
        {
            UInt32 newMute = mute ? 1 : 0;
            UInt32 size = sizeof (newMute);

            return AudioHardwareServiceSetPropertyData (outputDeviceID, &addr, 0, nullptr,
                                                        size, &newMute) == noErr;
        }

        return false;
    }

private:
    AudioDeviceID outputDeviceID;
    AudioObjectPropertyAddress addr;

    bool canSetVolume()
    {
        Boolean isSettable = NO;
        return AudioHardwareServiceIsPropertySettable (outputDeviceID, &addr, &isSettable) == noErr
                 && isSettable;
    }
};

#define JUCE_SYSTEMAUDIOVOL_IMPLEMENTED 1
float JUCE_CALLTYPE SystemAudioVolume::getGain()              { return SystemVol (kAudioHardwareServiceDeviceProperty_VirtualMasterVolume).getGain(); }
bool  JUCE_CALLTYPE SystemAudioVolume::setGain (float gain)   { return SystemVol (kAudioHardwareServiceDeviceProperty_VirtualMasterVolume).setGain (gain); }
bool  JUCE_CALLTYPE SystemAudioVolume::isMuted()              { return SystemVol (kAudioDevicePropertyMute).isMuted(); }
bool  JUCE_CALLTYPE SystemAudioVolume::setMuted (bool mute)   { return SystemVol (kAudioDevicePropertyMute).setMuted (mute); }

//==============================================================================
struct CoreAudioClasses
{

class CoreAudioIODevice;

//==============================================================================
class CoreAudioInternal  : private Timer
{
public:
    CoreAudioInternal (CoreAudioIODevice& d, AudioDeviceID id, bool isSlave)
       : owner (d),
         inputLatency (0),
         outputLatency (0),
         callback (nullptr),
        #if MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_5
         audioProcID (0),
        #endif
         isSlaveDevice (isSlave),
         deviceID (id),
         started (false),
         sampleRate (0),
         bufferSize (512),
         numInputChans (0),
         numOutputChans (0),
         callbacksAllowed (true)
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
    }

    void allocateTempBuffers()
    {
        const int tempBufSize = bufferSize + 4;
        audioBuffer.calloc ((size_t) ((numInputChans + numOutputChans) * tempBufSize));

        tempInputBuffers.calloc ((size_t) numInputChans + 2);
        tempOutputBuffers.calloc ((size_t) numOutputChans + 2);

        int count = 0;
        for (int i = 0; i < numInputChans; ++i)
            tempInputBuffers[i] = audioBuffer + count++ * tempBufSize;

        for (int i = 0; i < numOutputChans; ++i)
            tempOutputBuffers[i] = audioBuffer + count++ * tempBufSize;
    }

    struct CallbackDetailsForChannel
    {
        int streamNum;
        int dataOffsetSamples;
        int dataStrideSamples;
    };

    // returns the number of actual available channels
    StringArray getChannelInfo (const bool input, Array<CallbackDetailsForChannel>& newChannelInfo) const
    {
        StringArray newNames;
        int chanNum = 0;
        UInt32 size;

        AudioObjectPropertyAddress pa;
        pa.mSelector = kAudioDevicePropertyStreamConfiguration;
        pa.mScope = input ? kAudioDevicePropertyScopeInput : kAudioDevicePropertyScopeOutput;
        pa.mElement = kAudioObjectPropertyElementMaster;

        if (OK (AudioObjectGetPropertyDataSize (deviceID, &pa, 0, 0, &size)))
        {
            HeapBlock<AudioBufferList> bufList;
            bufList.calloc (size, 1);

            if (OK (AudioObjectGetPropertyData (deviceID, &pa, 0, 0, &size, bufList)))
            {
                const int numStreams = (int) bufList->mNumberBuffers;

                for (int i = 0; i < numStreams; ++i)
                {
                    const AudioBuffer& b = bufList->mBuffers[i];

                    for (unsigned int j = 0; j < b.mNumberChannels; ++j)
                    {
                        String name;

                        {
                            char channelName [256] = { 0 };
                            UInt32 nameSize = sizeof (channelName);
                            UInt32 channelNum = (UInt32) chanNum + 1;
                            pa.mSelector = kAudioDevicePropertyChannelName;

                            if (AudioObjectGetPropertyData (deviceID, &pa, sizeof (channelNum), &channelNum, &nameSize, channelName) == noErr)
                                name = String::fromUTF8 (channelName, (int) nameSize);
                        }

                        if ((input ? activeInputChans : activeOutputChans) [chanNum])
                        {
                            CallbackDetailsForChannel info = { i, (int) j, (int) b.mNumberChannels };
                            newChannelInfo.add (info);
                        }

                        if (name.isEmpty())
                            name << (input ? "Input " : "Output ") << (chanNum + 1);

                        newNames.add (name);
                        ++chanNum;
                    }
                }
            }
        }

        return newNames;
    }

    Array<double> getSampleRatesFromDevice() const
    {
        Array<double> newSampleRates;
        String rates;

        AudioObjectPropertyAddress pa;
        pa.mScope = kAudioObjectPropertyScopeWildcard;
        pa.mElement = kAudioObjectPropertyElementMaster;
        pa.mSelector = kAudioDevicePropertyAvailableNominalSampleRates;
        UInt32 size = 0;

        if (OK (AudioObjectGetPropertyDataSize (deviceID, &pa, 0, 0, &size)))
        {
            HeapBlock <AudioValueRange> ranges;
            ranges.calloc (size, 1);

            if (OK (AudioObjectGetPropertyData (deviceID, &pa, 0, 0, &size, ranges)))
            {
                static const double possibleRates[] = { 44100.0, 48000.0, 88200.0, 96000.0, 176400.0, 192000.0 };

                for (int i = 0; i < numElementsInArray (possibleRates); ++i)
                {
                    for (int j = size / (int) sizeof (AudioValueRange); --j >= 0;)
                    {
                        if (possibleRates[i] >= ranges[j].mMinimum - 2 && possibleRates[i] <= ranges[j].mMaximum + 2)
                        {
                            newSampleRates.add (possibleRates[i]);
                            rates << possibleRates[i] << ' ';
                            break;
                        }
                    }
                }
            }
        }

        if (newSampleRates.size() == 0 && sampleRate > 0)
        {
            newSampleRates.add (sampleRate);
            rates << sampleRate;
        }

        JUCE_COREAUDIOLOG ("rates: " + rates);
        return newSampleRates;
    }

    Array<int> getBufferSizesFromDevice() const
    {
        Array<int> newBufferSizes;

        AudioObjectPropertyAddress pa;
        pa.mScope = kAudioObjectPropertyScopeWildcard;
        pa.mElement = kAudioObjectPropertyElementMaster;
        pa.mSelector = kAudioDevicePropertyBufferFrameSizeRange;
        UInt32 size = 0;

        if (OK (AudioObjectGetPropertyDataSize (deviceID, &pa, 0, 0, &size)))
        {
            HeapBlock <AudioValueRange> ranges;
            ranges.calloc (size, 1);

            if (OK (AudioObjectGetPropertyData (deviceID, &pa, 0, 0, &size, ranges)))
            {
                newBufferSizes.add ((int) (ranges[0].mMinimum + 15) & ~15);

                for (int i = 32; i < 2048; i += 32)
                {
                    for (int j = size / (int) sizeof (AudioValueRange); --j >= 0;)
                    {
                        if (i >= ranges[j].mMinimum && i <= ranges[j].mMaximum)
                        {
                            newBufferSizes.addIfNotAlreadyThere (i);
                            break;
                        }
                    }
                }

                if (bufferSize > 0)
                    newBufferSizes.addIfNotAlreadyThere (bufferSize);
            }
        }

        if (newBufferSizes.size() == 0 && bufferSize > 0)
            newBufferSizes.add (bufferSize);

        return newBufferSizes;
    }

    int getLatencyFromDevice (AudioObjectPropertyScope scope) const
    {
        UInt32 lat = 0;
        UInt32 size = sizeof (lat);
        AudioObjectPropertyAddress pa;
        pa.mElement = kAudioObjectPropertyElementMaster;
        pa.mSelector = kAudioDevicePropertyLatency;
        pa.mScope = scope;
        AudioObjectGetPropertyData (deviceID, &pa, 0, 0, &size, &lat);
        return (int) lat;
    }

    void updateDetailsFromDevice()
    {
        stopTimer();

        if (deviceID == 0)
            return;

        // this collects all the new details from the device without any locking, then
        // locks + swaps them afterwards.
        AudioObjectPropertyAddress pa;
        pa.mScope = kAudioObjectPropertyScopeWildcard;
        pa.mElement = kAudioObjectPropertyElementMaster;

        UInt32 isAlive;
        UInt32 size = sizeof (isAlive);
        pa.mSelector = kAudioDevicePropertyDeviceIsAlive;
        if (OK (AudioObjectGetPropertyData (deviceID, &pa, 0, 0, &size, &isAlive)) && isAlive == 0)
            return;

        Float64 sr;
        size = sizeof (sr);
        pa.mSelector = kAudioDevicePropertyNominalSampleRate;
        if (OK (AudioObjectGetPropertyData (deviceID, &pa, 0, 0, &size, &sr)))
            sampleRate = sr;

        UInt32 framesPerBuf = bufferSize;
        size = sizeof (framesPerBuf);
        pa.mSelector = kAudioDevicePropertyBufferFrameSize;
        AudioObjectGetPropertyData (deviceID, &pa, 0, 0, &size, &framesPerBuf);

        Array<int> newBufferSizes (getBufferSizesFromDevice());
        Array<double> newSampleRates (getSampleRatesFromDevice());

        inputLatency  = getLatencyFromDevice (kAudioDevicePropertyScopeInput);
        outputLatency = getLatencyFromDevice (kAudioDevicePropertyScopeOutput);
        JUCE_COREAUDIOLOG ("lat: " + String (inputLatency) + " " + String (outputLatency));

        Array<CallbackDetailsForChannel> newInChans, newOutChans;
        StringArray newInNames  (getChannelInfo (true,  newInChans));
        StringArray newOutNames (getChannelInfo (false, newOutChans));

        // after getting the new values, lock + apply them
        const ScopedLock sl (callbackLock);

        bufferSize = (int) framesPerBuf;
        allocateTempBuffers();

        sampleRates.swapWith (newSampleRates);
        bufferSizes.swapWith (newBufferSizes);

        inChanNames.swapWith (newInNames);
        outChanNames.swapWith (newOutNames);

        inputChannelInfo.swapWith (newInChans);
        outputChannelInfo.swapWith (newOutChans);
    }

    //==============================================================================
    StringArray getSources (bool input)
    {
        StringArray s;
        HeapBlock <OSType> types;
        const int num = getAllDataSourcesForDevice (deviceID, types);

        for (int i = 0; i < num; ++i)
        {
            AudioValueTranslation avt;
            char buffer[256];

            avt.mInputData = &(types[i]);
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
                const int num = getAllDataSourcesForDevice (deviceID, types);

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
            const int num = getAllDataSourcesForDevice (deviceID, types);

            if (isPositiveAndBelow (index, num))
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
    String reopen (const BigInteger& inputChannels,
                   const BigInteger& outputChannels,
                   double newSampleRate,
                   int bufferSizeSamples)
    {
        String error;
        JUCE_COREAUDIOLOG ("CoreAudio reopen");
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
            UInt32 framesPerBuf = (UInt32) bufferSizeSamples;
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
            callback = nullptr;

            if (deviceID != 0)
            {
               #if MAC_OS_X_VERSION_MIN_REQUIRED < MAC_OS_X_VERSION_10_5
                if (OK (AudioDeviceAddIOProc (deviceID, audioIOProc, this)))
               #else
                if (OK (AudioDeviceCreateIOProcID (deviceID, audioIOProc, this, &audioProcID)))
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

        return started && (inputDevice == nullptr || inputDevice->start (cb));
    }

    void stop (bool leaveInterruptRunning)
    {
        {
            const ScopedLock sl (callbackLock);
            callback = nullptr;
        }

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

            { const ScopedLock sl (callbackLock); }

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

            const ScopedLock sl (callbackLock);
        }

        if (inputDevice != nullptr)
            inputDevice->stop (leaveInterruptRunning);
    }

    double getSampleRate() const  { return sampleRate; }
    int getBufferSize() const     { return bufferSize; }

    void audioCallback (const AudioBufferList* inInputData,
                        AudioBufferList* outOutputData)
    {
        const ScopedLock sl (callbackLock);

        if (callback != nullptr)
        {
            if (inputDevice == 0)
            {
                for (int i = numInputChans; --i >= 0;)
                {
                    const CallbackDetailsForChannel& info = inputChannelInfo.getReference(i);
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
                    callback->audioDeviceIOCallback (const_cast<const float**> (tempInputBuffers.getData()),
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

                    callback->audioDeviceIOCallback (const_cast<const float**> (inputDevice->tempInputBuffers.getData()),
                                                     inputDevice->numInputChans,
                                                     tempOutputBuffers,
                                                     numOutputChans,
                                                     bufferSize);
                }

                for (int i = numOutputChans; --i >= 0;)
                {
                    const CallbackDetailsForChannel& info = outputChannelInfo.getReference(i);
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
            for (int i = jmin (numOutputChans, outputChannelInfo.size()); --i >= 0;)
            {
                const CallbackDetailsForChannel& info = outputChannelInfo.getReference(i);
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

    void timerCallback() override
    {
        stopTimer();
        JUCE_COREAUDIOLOG ("CoreAudio device changed callback");

        const double oldSampleRate = sampleRate;
        const int oldBufferSize = bufferSize;
        updateDetailsFromDevice();

        if (oldBufferSize != bufferSize || oldSampleRate != sampleRate)
            owner.restart();
    }

    //==============================================================================
    CoreAudioIODevice& owner;
    int inputLatency, outputLatency;
    BigInteger activeInputChans, activeOutputChans;
    StringArray inChanNames, outChanNames;
    Array <double> sampleRates;
    Array <int> bufferSizes;
    AudioIODeviceCallback* callback;
   #if MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_5
    AudioDeviceIOProcID audioProcID;
   #endif

    ScopedPointer<CoreAudioInternal> inputDevice;
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

    Array<CallbackDetailsForChannel> inputChannelInfo, outputChannelInfo;
    HeapBlock <float*> tempInputBuffers, tempOutputBuffers;

    //==============================================================================
    static OSStatus audioIOProc (AudioDeviceID /*inDevice*/,
                                 const AudioTimeStamp* /*inNow*/,
                                 const AudioBufferList* inInputData,
                                 const AudioTimeStamp* /*inInputTime*/,
                                 AudioBufferList* outOutputData,
                                 const AudioTimeStamp* /*inOutputTime*/,
                                 void* device)
    {
        static_cast <CoreAudioInternal*> (device)->audioCallback (inInputData, outOutputData);
        return noErr;
    }

    static OSStatus deviceListenerProc (AudioDeviceID /*inDevice*/, UInt32 /*inLine*/, const AudioObjectPropertyAddress* pa, void* inClientData)
    {
        CoreAudioInternal* const intern = static_cast <CoreAudioInternal*> (inClientData);

        switch (pa->mSelector)
        {
            case kAudioDevicePropertyBufferSize:
            case kAudioDevicePropertyBufferFrameSize:
            case kAudioDevicePropertyNominalSampleRate:
            case kAudioDevicePropertyStreamFormat:
            case kAudioDevicePropertyDeviceIsAlive:
            case kAudioStreamPropertyPhysicalFormat:
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
    static int getAllDataSourcesForDevice (AudioDeviceID deviceID, HeapBlock <OSType>& types)
    {
        AudioObjectPropertyAddress pa;
        pa.mSelector = kAudioDevicePropertyDataSources;
        pa.mScope = kAudioObjectPropertyScopeWildcard;
        pa.mElement = kAudioObjectPropertyElementMaster;
        UInt32 size = 0;

        if (deviceID != 0
             && AudioObjectGetPropertyDataSize (deviceID, &pa, 0, 0, &size) == noErr)
        {
            types.calloc (size, 1);

            if (AudioObjectGetPropertyData (deviceID, &pa, 0, 0, &size, types) == noErr)
                return size / (int) sizeof (OSType);
        }

        return 0;
    }

    bool OK (const OSStatus errorCode) const
    {
        if (errorCode == noErr)
            return true;

        const String errorMessage ("CoreAudio error: " + String::toHexString ((int) errorCode));
        JUCE_COREAUDIOLOG (errorMessage);

        if (callback != nullptr)
            callback->audioDeviceError (errorMessage);

        return false;
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CoreAudioInternal)
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
        CoreAudioInternal* device = nullptr;

        if (outputDeviceId == 0 || outputDeviceId == inputDeviceId)
        {
            jassert (inputDeviceId != 0);

            device = new CoreAudioInternal (*this, inputDeviceId, false);
        }
        else
        {
            device = new CoreAudioInternal (*this, outputDeviceId, false);

            if (inputDeviceId != 0)
                device->inputDevice = new CoreAudioInternal (*this, inputDeviceId, true);
        }

        internal = device;
        jassert (device != nullptr);

        AudioObjectPropertyAddress pa;
        pa.mSelector = kAudioObjectPropertySelectorWildcard;
        pa.mScope = kAudioObjectPropertyScopeWildcard;
        pa.mElement = kAudioObjectPropertyElementWildcard;

        AudioObjectAddPropertyListener (kAudioObjectSystemObject, &pa, hardwareListenerProc, internal);
    }

    ~CoreAudioIODevice()
    {
        close();

        AudioObjectPropertyAddress pa;
        pa.mSelector = kAudioObjectPropertySelectorWildcard;
        pa.mScope = kAudioObjectPropertyScopeWildcard;
        pa.mElement = kAudioObjectPropertyElementWildcard;

        AudioObjectRemovePropertyListener (kAudioObjectSystemObject, &pa, hardwareListenerProc, internal);
    }

    StringArray getOutputChannelNames()
    {
        return internal->outChanNames;
    }

    StringArray getInputChannelNames()
    {
        if (internal->inputDevice != 0)
            return internal->inputDevice->inChanNames;

        return internal->inChanNames;
    }

    bool isOpen()                        { return isOpen_; }

    int getNumSampleRates()              { return internal->sampleRates.size(); }
    double getSampleRate (int index)     { return internal->sampleRates [index]; }
    double getCurrentSampleRate()        { return internal->getSampleRate(); }

    int getCurrentBitDepth()             { return 32; }  // no way to find out, so just assume it's high..

    int getNumBufferSizesAvailable()     { return internal->bufferSizes.size(); }
    int getBufferSizeSamples (int index) { return internal->bufferSizes [index]; }
    int getCurrentBufferSizeSamples()    { return internal->getBufferSize(); }

    int getDefaultBufferSize()
    {
        int best = 0;

        for (int i = 0; best < 512 && i < getNumBufferSizesAvailable(); ++i)
            best = getBufferSizeSamples(i);

        if (best == 0)
            best = 512;

        return best;
    }

    String open (const BigInteger& inputChannels,
                 const BigInteger& outputChannels,
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

    void restart()
    {
        AudioIODeviceCallback* oldCallback = internal->callback;
        stop();

        if (oldCallback != nullptr)
            start (oldCallback);
    }

    BigInteger getActiveOutputChannels() const
    {
        return internal->activeOutputChans;
    }

    BigInteger getActiveInputChannels() const
    {
        BigInteger chans (internal->activeInputChans);

        if (internal->inputDevice != nullptr)
            chans |= internal->inputDevice->activeInputChans;

        return chans;
    }

    int getOutputLatencyInSamples()
    {
        // this seems like a good guess at getting the latency right - comparing
        // this with a round-trip measurement, it gets it to within a few millisecs
        // for the built-in mac soundcard
        return internal->outputLatency + internal->getBufferSize() * 2;
    }

    int getInputLatencyInSamples()
    {
        return internal->inputLatency + internal->getBufferSize() * 2;
    }

    void start (AudioIODeviceCallback* callback)
    {
        if (! isStarted)
        {
            if (callback != nullptr)
                callback->audioDeviceAboutToStart (this);

            isStarted = true;
            internal->start (callback);
        }
    }

    void stop()
    {
        if (isStarted)
        {
            AudioIODeviceCallback* const lastCallback = internal->callback;

            isStarted = false;
            internal->stop (true);

            if (lastCallback != nullptr)
                lastCallback->audioDeviceStopped();
        }
    }

    bool isPlaying()
    {
        if (internal->callback == nullptr)
            isStarted = false;

        return isStarted;
    }

    String getLastError()
    {
        return lastError;
    }

    int inputIndex, outputIndex;

private:
    ScopedPointer<CoreAudioInternal> internal;
    bool isOpen_, isStarted;
    String lastError;

    static OSStatus hardwareListenerProc (AudioDeviceID /*inDevice*/, UInt32 /*inLine*/, const AudioObjectPropertyAddress* pa, void* inClientData)
    {
        CoreAudioInternal* const intern = static_cast <CoreAudioInternal*> (inClientData);

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

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CoreAudioIODevice)
};

//==============================================================================
class CoreAudioIODeviceType  : public AudioIODeviceType
{
public:
    //==============================================================================
    CoreAudioIODeviceType()
        : AudioIODeviceType ("CoreAudio"),
          hasScanned (false)
    {
        AudioObjectPropertyAddress pa;
        pa.mSelector = kAudioHardwarePropertyDevices;
        pa.mScope = kAudioObjectPropertyScopeWildcard;
        pa.mElement = kAudioObjectPropertyElementWildcard;

        AudioObjectAddPropertyListener (kAudioObjectSystemObject, &pa, hardwareListenerProc, this);
    }

    ~CoreAudioIODeviceType()
    {
        AudioObjectPropertyAddress pa;
        pa.mSelector = kAudioHardwarePropertyDevices;
        pa.mScope = kAudioObjectPropertyScopeWildcard;
        pa.mElement = kAudioObjectPropertyElementWildcard;

        AudioObjectRemovePropertyListener (kAudioObjectSystemObject, &pa, hardwareListenerProc, this);
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

        if (AudioObjectGetPropertyDataSize (kAudioObjectSystemObject, &pa, 0, 0, &size) == noErr)
        {
            HeapBlock <AudioDeviceID> devs;
            devs.calloc (size, 1);

            if (AudioObjectGetPropertyData (kAudioObjectSystemObject, &pa, 0, 0, &size, devs) == noErr)
            {
                const int num = size / (int) sizeof (AudioDeviceID);
                for (int i = 0; i < num; ++i)
                {
                    char name [1024];
                    size = sizeof (name);
                    pa.mSelector = kAudioDevicePropertyDeviceName;

                    if (AudioObjectGetPropertyData (devs[i], &pa, 0, 0, &size, name) == noErr)
                    {
                        const String nameString (String::fromUTF8 (name, (int) strlen (name)));
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
            }
        }

        inputDeviceNames.appendNumbersToDuplicates (false, true);
        outputDeviceNames.appendNumbersToDuplicates (false, true);
    }

    StringArray getDeviceNames (bool wantInputNames) const
    {
        jassert (hasScanned); // need to call scanForDevices() before doing this

        return wantInputNames ? inputDeviceNames
                              : outputDeviceNames;
    }

    int getDefaultDeviceIndex (bool forInput) const
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

    int getIndexOfDevice (AudioIODevice* device, bool asInput) const
    {
        jassert (hasScanned); // need to call scanForDevices() before doing this

        CoreAudioIODevice* const d = dynamic_cast <CoreAudioIODevice*> (device);
        if (d == nullptr)
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

        return nullptr;
    }

    //==============================================================================
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

        if (AudioObjectGetPropertyDataSize (deviceID, &pa, 0, 0, &size) == noErr)
        {
            HeapBlock <AudioBufferList> bufList;
            bufList.calloc (size, 1);

            if (AudioObjectGetPropertyData (deviceID, &pa, 0, 0, &size, bufList) == noErr)
            {
                const int numStreams = (int) bufList->mNumberBuffers;

                for (int i = 0; i < numStreams; ++i)
                {
                    const AudioBuffer& b = bufList->mBuffers[i];
                    total += b.mNumberChannels;
                }
            }
        }

        return total;
    }

    void audioDeviceListChanged()
    {
        scanForDevices();
        callDeviceChangeListeners();
    }

    static OSStatus hardwareListenerProc (AudioDeviceID, UInt32, const AudioObjectPropertyAddress*, void* clientData)
    {
        static_cast <CoreAudioIODeviceType*> (clientData)->audioDeviceListChanged();
        return noErr;
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CoreAudioIODeviceType)
};

};

//==============================================================================
AudioIODeviceType* AudioIODeviceType::createAudioIODeviceType_CoreAudio()
{
    return new CoreAudioClasses::CoreAudioIODeviceType();
}

#undef JUCE_COREAUDIOLOG

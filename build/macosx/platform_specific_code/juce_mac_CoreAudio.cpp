/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

  ==============================================================================
*/

#include "../../../src/juce_core/basics/juce_StandardHeader.h"
#include <CoreAudio/AudioHardware.h>

BEGIN_JUCE_NAMESPACE


#include "../../../src/juce_appframework/audio/devices/juce_AudioIODeviceType.h"
#include "../../../src/juce_appframework/events/juce_Timer.h"
#include "../../../src/juce_core/threads/juce_ScopedLock.h"
#include "../../../src/juce_core/threads/juce_Thread.h"
#include "../../../src/juce_core/text/juce_LocalisedStrings.h"


//==============================================================================
#ifndef JUCE_COREAUDIO_ERROR_LOGGING_ENABLED
  #define JUCE_COREAUDIO_ERROR_LOGGING_ENABLED 1
#endif

//==============================================================================
#if JUCE_COREAUDIO_LOGGING_ENABLED
  #define log(a) Logger::writeToLog (a)
#else
  #define log(a)
#endif

#if JUCE_COREAUDIO_ERROR_LOGGING_ENABLED
  static bool logAnyErrors (const OSStatus err, const int lineNum)
  {
      if (err == noErr)
          return true;

      Logger::writeToLog (T("CoreAudio error: ") + String (lineNum) + T(" - ") + String::toHexString ((int)err));
      jassertfalse
      return false;
  }

  #define OK(a) logAnyErrors (a, __LINE__)
#else
  #define OK(a) (a == noErr)
#endif

//==============================================================================
static const int maxNumChans = 96;


//==============================================================================
class CoreAudioInternal  : public Timer
{
public:
    //==============================================================================
    CoreAudioInternal (AudioDeviceID id)
       : deviceID (id),
         started (false),
         audioBuffer (0),
         numInputChans (0),
         numOutputChans (0),
         callbacksAllowed (true),
         inputLatency (0),
         outputLatency (0),
         callback (0),
         inputDevice (0),
         isSlaveDevice (false)
    {
        sampleRate = 0;
        bufferSize = 512;

        if (deviceID == 0)
        {
            error = TRANS("can't open device");
        }
        else
        {
            updateDetailsFromDevice();

            AudioDeviceAddPropertyListener (deviceID,
                                            kAudioPropertyWildcardChannel,
                                            kAudioPropertyWildcardSection,
                                            kAudioPropertyWildcardPropertyID,
                                            deviceListenerProc, this);
        }
    }

    ~CoreAudioInternal()
    {
        AudioDeviceRemovePropertyListener (deviceID,
                                           kAudioPropertyWildcardChannel,
                                           kAudioPropertyWildcardSection,
                                           kAudioPropertyWildcardPropertyID,
                                           deviceListenerProc);

        stop (false);

        juce_free (audioBuffer);
        delete inputDevice;
    }

    void setTempBufferSize (const int numChannels, const int numSamples)
    {
        juce_free (audioBuffer);

        audioBuffer = (float*) juce_calloc (32 + numChannels * numSamples * sizeof (float));

        zeromem (tempInputBuffers, sizeof (tempInputBuffers));
        zeromem (tempOutputBuffers, sizeof (tempOutputBuffers));

        int count = 0;
        int i;
        for (i = maxNumChans; --i >= 0;)
            if (activeInputChans[i])
                tempInputBuffers[i] = audioBuffer + count++ * numSamples;

        for (i = maxNumChans; --i >= 0;)
            if (activeOutputChans[i])
                tempOutputBuffers[i] = audioBuffer + count++ * numSamples;
    }

    // returns the number of actual available channels
    void fillInChannelInfo (bool input)
    {
        int chanNum = 0, activeChans = 0;
        UInt32 size;

        if (OK (AudioDeviceGetPropertyInfo (deviceID, 0, input, kAudioDevicePropertyStreamConfiguration, &size, 0)))
        {
            AudioBufferList* const bufList = (AudioBufferList*) juce_calloc (size);

            if (OK (AudioDeviceGetProperty (deviceID, 0, input, kAudioDevicePropertyStreamConfiguration, &size, bufList)))
            {
                const int numStreams = bufList->mNumberBuffers;

                for (int i = 0; i < numStreams; ++i)
                {
                    const AudioBuffer& b = bufList->mBuffers[i];

                    for (unsigned int j = 0; j < b.mNumberChannels; ++j)
                    {
                        if (input)
                        {
                            if (activeInputChans[chanNum])
                            {
                                inputChannelInfo [activeChans].sourceChannelNum = chanNum;
                                inputChannelInfo [activeChans].streamNum = i;
                                inputChannelInfo [activeChans].dataOffsetSamples = j;
                                inputChannelInfo [activeChans].dataStrideSamples = b.mNumberChannels;
                                ++activeChans;
                                numInputChannelInfos = activeChans;
                            }

                            inChanNames.add (T("input ") + String (chanNum + 1));
                        }
                        else
                        {
                            if (activeOutputChans[chanNum])
                            {
                                outputChannelInfo [activeChans].sourceChannelNum = chanNum;
                                outputChannelInfo [activeChans].streamNum = i;
                                outputChannelInfo [activeChans].dataOffsetSamples = j;
                                outputChannelInfo [activeChans].dataStrideSamples = b.mNumberChannels;
                                ++activeChans;
                                numOutputChannelInfos = activeChans;
                            }

                            outChanNames.add (T("output ") + String (chanNum + 1));
                        }

                        ++chanNum;
                    }
                }
            }

            juce_free (bufList);
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
        if (OK (AudioDeviceGetProperty (deviceID, 0, false, kAudioDevicePropertyNominalSampleRate, &size, &sr)))
            sampleRate = sr;

        UInt32 framesPerBuf;
        size = sizeof (framesPerBuf);

        if (OK (AudioDeviceGetProperty (deviceID, 0, false, kAudioDevicePropertyBufferFrameSize, &size, &framesPerBuf)))
        {
            bufferSize = framesPerBuf;

            if (bufferSize > 0)
                setTempBufferSize (numInputChans + numOutputChans, bufferSize);
        }

        bufferSizes.clear();

        if (OK (AudioDeviceGetPropertyInfo (deviceID, 0, false, kAudioDevicePropertyBufferFrameSizeRange, &size, 0)))
        {
            AudioValueRange* ranges = (AudioValueRange*) juce_calloc (size);

            if (OK (AudioDeviceGetProperty (deviceID, 0, false, kAudioDevicePropertyBufferFrameSizeRange, &size, ranges)))
            {
                bufferSizes.add ((int) ranges[0].mMinimum);

                for (int i = 32; i < 8192; i += 32)
                {
                    for (int j = size / sizeof (AudioValueRange); --j >= 0;)
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

            juce_free (ranges);
        }

        if (bufferSizes.size() == 0 && bufferSize > 0)
            bufferSizes.add (bufferSize);

        sampleRates.clear();
        const double possibleRates[] = { 44100.0, 48000.0, 88200.0, 96000.0, 176400.0, 192000.0 };
        String rates;

        if (OK (AudioDeviceGetPropertyInfo (deviceID, 0, false, kAudioDevicePropertyAvailableNominalSampleRates, &size, 0)))
        {
            AudioValueRange* ranges = (AudioValueRange*) juce_calloc (size);

            if (OK (AudioDeviceGetProperty (deviceID, 0, false, kAudioDevicePropertyAvailableNominalSampleRates, &size, ranges)))
            {
                for (int i = 0; i < numElementsInArray (possibleRates); ++i)
                {
                    bool ok = false;

                    for (int j = size / sizeof (AudioValueRange); --j >= 0;)
                        if (possibleRates[i] >= ranges[j].mMinimum - 2 && possibleRates[i] <= ranges[j].mMaximum + 2)
                            ok = true;

                    if (ok)
                    {
                        sampleRates.add (possibleRates[i]);
                        rates << possibleRates[i] << T(" ");
                    }
                }
            }

            juce_free (ranges);
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
        size = sizeof (UInt32);
        if (AudioDeviceGetProperty (deviceID, 0, true, kAudioDevicePropertyLatency, &size, &lat) == noErr)
            inputLatency = (int) lat;

        if (AudioDeviceGetProperty (deviceID, 0, false, kAudioDevicePropertyLatency, &size, &lat) == noErr)
            outputLatency = (int) lat;

        log (T("lat: ") + String (inputLatency) + T(" ") + String (outputLatency));

        inChanNames.clear();
        outChanNames.clear();

        zeromem (inputChannelInfo, sizeof (inputChannelInfo));
        zeromem (outputChannelInfo, sizeof (outputChannelInfo));

        fillInChannelInfo (true);
        fillInChannelInfo (false);
    }

    //==============================================================================
    const StringArray getSources (bool input)
    {
        StringArray s;
        int num = 0;
        OSType* types = getAllDataSourcesForDevice (deviceID, input, num);

        if (types != 0)
        {
            for (int i = 0; i < num; ++i)
            {
                AudioValueTranslation avt;
                char buffer[256];

                avt.mInputData = (void*) &(types[i]);
                avt.mInputDataSize = sizeof (UInt32);
                avt.mOutputData = buffer;
                avt.mOutputDataSize = 256;

                UInt32 transSize = sizeof (avt);
                if (OK (AudioDeviceGetProperty (deviceID, 0, input, kAudioDevicePropertyDataSourceNameForID, &transSize, &avt)))
                {
                    DBG (buffer);
                    s.add (buffer);
                }
            }

            juce_free (types);
        }

        return s;
    }

    int getCurrentSourceIndex (bool input) const
    {
        OSType currentSourceID = 0;
        UInt32 size = 0;
        int result = -1;

        if (deviceID != 0
             && OK (AudioDeviceGetPropertyInfo (deviceID, 0, input, kAudioDevicePropertyDataSource, &size, 0)))
        {
            if (OK (AudioDeviceGetProperty (deviceID, 0, input, kAudioDevicePropertyDataSource, &size, &currentSourceID)))
            {
                int num = 0;
                OSType* const types = getAllDataSourcesForDevice (deviceID, input, num);

                if (types != 0)
                {
                    for (int i = 0; i < num; ++i)
                    {
                        if (types[num] == currentSourceID)
                        {
                            result = i;
                            break;
                        }
                    }

                    juce_free (types);
                }
            }
        }

        return result;
    }

    void setCurrentSourceIndex (int index, bool input)
    {
        if (deviceID != 0)
        {
            int num = 0;
            OSType* types = getAllDataSourcesForDevice (deviceID, input, num);

            if (types != 0)
            {
                if (index >= 0 && index < num)
                {
                    OSType id = types[index];
                    AudioDeviceSetProperty (deviceID, 0, 0, input, kAudioDevicePropertyDataSource, sizeof (id), &id);
                }

                juce_free (types);
            }
        }
    }

    //==============================================================================
    const String reopen (const BitArray& inputChannels,
                         const BitArray& outputChannels,
                         double newSampleRate,
                         int bufferSizeSamples)
    {
        error = String::empty;
        log ("CoreAudio reopen");
        callbacksAllowed = false;
        stopTimer();

        stop (false);

        activeInputChans = inputChannels;
        activeOutputChans = outputChannels;
        numInputChans = inputChannels.countNumberOfSetBits();
        numOutputChans = outputChannels.countNumberOfSetBits();

        // set sample rate
        Float64 sr = newSampleRate;
        UInt32 size = sizeof (sr);
        OK (AudioDeviceSetProperty (deviceID, 0, 0, false, kAudioDevicePropertyNominalSampleRate, size, &sr));
        OK (AudioDeviceSetProperty (deviceID, 0, 0, true, kAudioDevicePropertyNominalSampleRate, size, &sr));

        // change buffer size
        UInt32 framesPerBuf = bufferSizeSamples;
        size = sizeof (framesPerBuf);

        OK (AudioDeviceSetProperty (deviceID, 0, 0, false, kAudioDevicePropertyBufferFrameSize, size, &framesPerBuf));
        OK (AudioDeviceSetProperty (deviceID, 0, 0, true, kAudioDevicePropertyBufferFrameSize, size, &framesPerBuf));

        // wait for the changes to happen (on some devices)
        int i = 30;
        while (--i >= 0)
        {
            updateDetailsFromDevice();

            if (sampleRate == newSampleRate && bufferSizeSamples == bufferSize)
                break;

            Thread::sleep (100);
        }

        if (i < 0)
            error = "Couldn't change sample rate/buffer size";

        if (sampleRates.size() == 0)
            error = "Device has no available sample-rates";

        if (bufferSizes.size() == 0)
            error = "Device has no available buffer-sizes";

        numInputChans = jmin (numInputChans, numInputChannelInfos);
        numOutputChans = jmin (numOutputChans, numOutputChannelInfos);

        if (inputDevice != 0 && error.isEmpty())
            error = inputDevice->reopen (inputChannels,
                                         outputChannels,
                                         newSampleRate,
                                         bufferSizeSamples);

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
                if (OK (AudioDeviceAddIOProc (deviceID, audioIOProc, (void*) this)))
                {
                    if (OK (AudioDeviceStart (deviceID, audioIOProc)))
                    {
                        started = true;
                    }
                    else
                    {
                        OK (AudioDeviceRemoveIOProc (deviceID, audioIOProc));
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
            OK (AudioDeviceRemoveIOProc (deviceID, audioIOProc));
            started = false;

            callbackLock.enter();
            callbackLock.exit();

            // wait until it's definately stopped calling back..
            for (int i = 40; --i >= 0;)
            {
                Thread::sleep (50);

                UInt32 running = 0;
                UInt32 size = sizeof (running);
                OK (AudioDeviceGetProperty (deviceID, 0, false, kAudioDevicePropertyDeviceIsRunning, &size, &running));
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
                    float* dest = tempInputBuffers [info.sourceChannelNum];
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

                    callback->audioDeviceIOCallback ((const float**) inputDevice->tempInputBuffers,
                                                     inputDevice->numInputChans,
                                                     tempOutputBuffers,
                                                     numOutputChans,
                                                     bufferSize);
                }

                for (i = numOutputChans; --i >= 0;)
                {
                    const CallbackDetailsForChannel& info = outputChannelInfo[i];
                    const float* src = tempOutputBuffers [info.sourceChannelNum];
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
        CoreAudioInternal* result = 0;

        if (deviceID != 0
             && AudioDeviceGetPropertyInfo (deviceID, 0, false, kAudioDevicePropertyRelatedDevices, &size, 0) == noErr
             && size > 0)
        {
            AudioDeviceID* devs = (AudioDeviceID*) juce_calloc (size);

            if (OK (AudioDeviceGetProperty (deviceID, 0, false, kAudioDevicePropertyRelatedDevices, &size, devs)))
            {
                for (unsigned int i = 0; i < size / sizeof (AudioDeviceID); ++i)
                {
                    if (devs[i] != deviceID && devs[i] != 0)
                    {
                        result = new CoreAudioInternal (devs[i]);

                        if (result->error.isEmpty())
                        {
                            const bool thisIsInput = inChanNames.size() > 0 && outChanNames.size() == 0;
                            const bool otherIsInput = result->inChanNames.size() > 0 && result->outChanNames.size() == 0;

                            if (thisIsInput != otherIsInput)
                                break;
                        }

                        deleteAndZero (result);
                    }
                }
            }

            juce_free (devs);
        }

        return result;
    }

    //==============================================================================
    juce_UseDebuggingNewOperator

    String error;
    int inputLatency, outputLatency;
    StringArray inChanNames, outChanNames;
    Array <double> sampleRates;
    Array <int> bufferSizes;
    AudioIODeviceCallback* callback;

    CoreAudioInternal* inputDevice;
    bool isSlaveDevice;

private:
    CriticalSection callbackLock;
    AudioDeviceID deviceID;
    bool started;
    double sampleRate;
    int bufferSize;
    float* audioBuffer;
    BitArray activeInputChans, activeOutputChans;
    int numInputChans, numOutputChans;
    bool callbacksAllowed;

    struct CallbackDetailsForChannel
    {
        int sourceChannelNum;
        int streamNum;
        int dataOffsetSamples;
        int dataStrideSamples;
    };

    int numInputChannelInfos, numOutputChannelInfos;
    CallbackDetailsForChannel inputChannelInfo [maxNumChans];
    CallbackDetailsForChannel outputChannelInfo [maxNumChans];
    float* tempInputBuffers [maxNumChans];
    float* tempOutputBuffers [maxNumChans];

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

    static OSStatus deviceListenerProc (AudioDeviceID inDevice,
                                        UInt32 inLine,
                                        Boolean isInput,
                                        AudioDevicePropertyID inPropertyID,
                                        void* inClientData)
    {
        CoreAudioInternal* const intern = (CoreAudioInternal*) inClientData;

        switch (inPropertyID)
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
    static OSType* getAllDataSourcesForDevice (AudioDeviceID deviceID, const bool input, int& num)
    {
        OSType* types = 0;
        UInt32 size = 0;
        num = 0;

        if (deviceID != 0
             && OK (AudioDeviceGetPropertyInfo (deviceID, 0, input, kAudioDevicePropertyDataSources, &size, 0)))
        {
            types = (OSType*) juce_calloc (size);

            if (OK (AudioDeviceGetProperty (deviceID, 0, input, kAudioDevicePropertyDataSources, &size, types)))
            {
                num = size / sizeof (OSType);
            }
            else
            {
                juce_free (types);
                types = 0;
            }
        }

        return types;
    }
};


//==============================================================================
class CoreAudioIODevice   : public AudioIODevice
{
public:
    CoreAudioIODevice (const String& deviceName,
                       AudioDeviceID deviceId1)
        : AudioIODevice (deviceName, "CoreAudio"),
          isOpen_ (false),
          isStarted (false)
    {
        internal = 0;

        CoreAudioInternal* device = new CoreAudioInternal (deviceId1);
        lastError = device->error;

        if (lastError.isNotEmpty())
        {
            deleteAndZero (device);
        }
        else
        {
            CoreAudioInternal* secondDevice = device->getRelatedDevice();

            if (secondDevice != 0)
            {
                if (device->inChanNames.size() > secondDevice->inChanNames.size())
                    swapVariables (device, secondDevice);

                device->inputDevice = secondDevice;
                secondDevice->isSlaveDevice = true;
            }
        }

        internal = device;

        AudioHardwareAddPropertyListener (kAudioPropertyWildcardPropertyID,
                                          hardwareListenerProc, internal);
    }

    ~CoreAudioIODevice()
    {
        AudioHardwareRemovePropertyListener (kAudioPropertyWildcardPropertyID,
                                             hardwareListenerProc);

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

        internal->reopen (inputChannels, outputChannels, sampleRate, bufferSizeSamples);
        lastError = internal->error;
        return lastError;
    }

    void close()
    {
        isOpen_ = false;
    }

    bool isOpen()
    {
        return isOpen_;
    }

    int getCurrentBufferSizeSamples()
    {
        if (internal == 0)
            return 512;

        return internal->getBufferSize();
    }

    double getCurrentSampleRate()
    {
        if (internal == 0)
            return 0;

        return internal->getSampleRate();
    }

    int getCurrentBitDepth()
    {
        return 32;  // no way to find out, so just assume it's high..
    }

    int getOutputLatencyInSamples()
    {
        if (internal == 0)
            return 0;

        return internal->outputLatency;
    }

    int getInputLatencyInSamples()
    {
        if (internal == 0)
            return 0;

        return internal->inputLatency;
    }

    void start (AudioIODeviceCallback* callback)
    {
        if (internal != 0 && ! isStarted)
        {
            if (callback != 0)
                callback->audioDeviceAboutToStart (getCurrentSampleRate(),
                                                   getCurrentBufferSizeSamples());

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

    juce_UseDebuggingNewOperator

private:
    CoreAudioInternal* internal;
    bool isOpen_, isStarted;
    String lastError;

    static OSStatus hardwareListenerProc (AudioHardwarePropertyID inPropertyID, void* inClientData)
    {
        CoreAudioInternal* const intern = (CoreAudioInternal*) inClientData;

        switch (inPropertyID)
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

        names.clear();
        ids.clear();

        UInt32 size;
        if (OK (AudioHardwareGetPropertyInfo (kAudioHardwarePropertyDevices, &size, 0)))
        {
            AudioDeviceID* const devs = (AudioDeviceID*) juce_calloc (size);

            if (OK (AudioHardwareGetProperty (kAudioHardwarePropertyDevices, &size, devs)))
            {
                static bool alreadyLogged = false;
                const int num = size / sizeof (AudioDeviceID);
                for (int i = 0; i < num; ++i)
                {
                    char name[1024];
                    size = sizeof (name);
                    if (OK (AudioDeviceGetProperty (devs[i], 0, false, kAudioDevicePropertyDeviceName, &size, name)))
                    {
                        const String nameString (String::fromUTF8 ((const uint8*) name, strlen (name)));

                        if (! alreadyLogged)
                            log (T("CoreAudio device: ") + nameString);

                        names.add (nameString);
                        ids.add (devs[i]);
                    }
                }

                alreadyLogged = true;
            }

            juce_free (devs);
        }
    }

    const StringArray getDeviceNames (const bool /*preferInputNames*/) const
    {
        jassert (hasScanned); // need to call scanForDevices() before doing this

        StringArray namesCopy (names);
        namesCopy.removeDuplicates (true);

        return namesCopy;
    }

    const String getDefaultDeviceName (const bool preferInputNames) const
    {
        jassert (hasScanned); // need to call scanForDevices() before doing this

        String result (names[0]);

        AudioDeviceID deviceID;
        UInt32 size = sizeof (deviceID);

        if (AudioHardwareGetProperty (preferInputNames ? kAudioHardwarePropertyDefaultInputDevice
                                                       : kAudioHardwarePropertyDefaultOutputDevice,
                                      &size, &deviceID) == noErr)
        {
            for (int i = ids.size(); --i >= 0;)
                if (ids[i] == deviceID)
                    result = names[i];
        }

        return result;
    }

    AudioIODevice* createDevice (const String& deviceName)
    {
        jassert (hasScanned); // need to call scanForDevices() before doing this

        const int index = names.indexOf (deviceName);

        if (index >= 0)
            return new CoreAudioIODevice (deviceName, ids [index]);

        return 0;
    }

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    StringArray names;
    Array <AudioDeviceID> ids;

    bool hasScanned;

    CoreAudioIODeviceType (const CoreAudioIODeviceType&);
    const CoreAudioIODeviceType& operator= (const CoreAudioIODeviceType&);
};

//==============================================================================
AudioIODeviceType* juce_createDefaultAudioIODeviceType()
{
    return new CoreAudioIODeviceType();
}


END_JUCE_NAMESPACE

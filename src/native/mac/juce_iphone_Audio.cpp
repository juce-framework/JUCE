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
#ifdef JUCE_INCLUDED_FILE




class IPhoneAudioIODevice  : public AudioIODeviceType
{
public:
    //==============================================================================
    IPhoneAudioIODevice (const String& deviceName, const bool isInput_)
        : AudioIODevice (deviceName, T("Audio")),
          isInput (isInput_),
          isOpen_ (false),
          isStarted (false)
    {
    }

    ~IPhoneAudioIODeviceType()
    {
    }

    const StringArray getOutputChannelNames()
    {
        StringArray s;
        if (! isInput)
        {
            s.add ("Left");
            s.add ("Right");
        }
        return s;
    }

    const StringArray getInputChannelNames()
    {
        StringArray s;
        if (isInput)
        {
            s.add ("Left");
            s.add ("Right");
        }
        return s;
    }

    int getNumSampleRates()
    {
        return sampleRates.size();
    }

    double getSampleRate (int index)
    {
        return sampleRates [index];
    }

    int getNumBufferSizesAvailable()
    {
        return bufferSizes.size();
    }

    int getBufferSizeSamples (int index)
    {
        return bufferSizes [index];
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

        lastError = String::empty;



        isOpen_ = lastError.isEmpty();
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
class IPhoneAudioIODeviceType  : public AudioIODeviceType
{
public:
    //==============================================================================
    IPhoneAudioIODeviceType()
        : AudioIODeviceType (T("iPhone Audio")),
          hasScanned (false)
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
        return s;
    }

    int getDefaultDeviceIndex (const bool forInput) const
    {
        return 0;
    }

    int getIndexOfDevice (AudioIODevice* device, const bool asInput) const
    {
        return 0;
    }

    bool hasSeparateInputsAndOutputs() const    { return true; }

    AudioIODevice* createDevice (const String& outputDeviceName,
                                 const String& inputDeviceName)
    {
        if (outputDeviceName.isNotEmpty() && inputDeviceName.isNotEmpty())
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
    IPhoneAudioIODeviceType (const IPhoneAudioIODeviceType&);
    const IPhoneAudioIODeviceType& operator= (const IPhoneAudioIODeviceType&);
};

//==============================================================================
AudioIODeviceType* juce_createAudioIODeviceType_iPhoneAudio()
{
    return new IPhoneAudioIODeviceType();
}

#endif

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
class AndroidAudioIODevice  : public AudioIODevice
{
public:
    //==============================================================================
    AndroidAudioIODevice (const String& deviceName)
        : AudioIODevice (deviceName, "Audio"),
          callback (0),
          sampleRate (0),
          numInputChannels (0),
          numOutputChannels (0),
          actualBufferSize (0),
          isRunning (false)
    {
        numInputChannels = 2;
        numOutputChannels = 2;

        // TODO
    }

    ~AndroidAudioIODevice()
    {
        close();
    }

    const StringArray getOutputChannelNames()
    {
        StringArray s;
        s.add ("Left");  // TODO
        s.add ("Right");
        return s;
    }

    const StringArray getInputChannelNames()
    {
        StringArray s;
        s.add ("Left");
        s.add ("Right");
        return s;
    }

    int getNumSampleRates()             { return 1;}
    double getSampleRate (int index)    { return sampleRate; }

    int getNumBufferSizesAvailable()        { return 1; }
    int getBufferSizeSamples (int index)    { return getDefaultBufferSize(); }
    int getDefaultBufferSize()              { return 1024; }

    const String open (const BigInteger& inputChannels,
                       const BigInteger& outputChannels,
                       double sampleRate,
                       int bufferSize)
    {
        close();

        lastError = String::empty;
        int preferredBufferSize = (bufferSize <= 0) ? getDefaultBufferSize() : bufferSize;

        activeOutputChans = outputChannels;
        activeOutputChans.setRange (2, activeOutputChans.getHighestBit(), false);
        numOutputChannels = activeOutputChans.countNumberOfSetBits();

        activeInputChans = inputChannels;
        activeInputChans.setRange (2, activeInputChans.getHighestBit(), false);
        numInputChannels = activeInputChans.countNumberOfSetBits();

        // TODO



        actualBufferSize = 0; // whatever is possible based on preferredBufferSize

        isRunning = true;

        return lastError;
    }

    void close()
    {
        if (isRunning)
        {
            isRunning = false;

            // TODO
        }
    }

    int getOutputLatencyInSamples()
    {
        return 0; // TODO
    }

    int getInputLatencyInSamples()
    {
        return 0; // TODO
    }

    bool isOpen()                                       { return isRunning; }
    int getCurrentBufferSizeSamples()                   { return actualBufferSize; }
    int getCurrentBitDepth()                            { return 16; }
    double getCurrentSampleRate()                       { return sampleRate; }
    const BigInteger getActiveOutputChannels() const    { return activeOutputChans; }
    const BigInteger getActiveInputChannels() const     { return activeInputChans; }
    const String getLastError()                         { return lastError; }
    bool isPlaying()                                    { return isRunning && callback != 0; }

    // TODO

    void start (AudioIODeviceCallback* newCallback)
    {
        if (isRunning && callback != newCallback)
        {
            if (newCallback != 0)
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
                callback = 0;
            }

            if (lastCallback != 0)
                lastCallback->audioDeviceStopped();
        }
    }

private:
    //==================================================================================================
    CriticalSection callbackLock;
    AudioIODeviceCallback* callback;
    double sampleRate;
    int numInputChannels, numOutputChannels;
    int actualBufferSize;
    bool isRunning;
    String lastError;
    BigInteger activeOutputChans, activeInputChans;

    JUCE_DECLARE_NON_COPYABLE (AndroidAudioIODevice);
};

//==============================================================================
// TODO
class AndroidAudioIODeviceType  : public AudioIODeviceType
{
public:
    AndroidAudioIODeviceType()
        : AudioIODeviceType ("Android Audio")
    {
    }

    //==============================================================================
    void scanForDevices()
    {
    }

    const StringArray getDeviceNames (bool wantInputNames) const
    {
        StringArray s;
        s.add ("Android Audio");
        return s;
    }

    int getDefaultDeviceIndex (bool forInput) const
    {
        return 0;
    }

    int getIndexOfDevice (AudioIODevice* device, bool asInput) const
    {
        return device != 0 ? 0 : -1;
    }

    bool hasSeparateInputsAndOutputs() const    { return false; }

    AudioIODevice* createDevice (const String& outputDeviceName,
                                 const String& inputDeviceName)
    {
        if (outputDeviceName.isNotEmpty() || inputDeviceName.isNotEmpty())
            return new AndroidAudioIODevice (outputDeviceName.isNotEmpty() ? outputDeviceName
                                                                           : inputDeviceName);

        return 0;
    }

private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AndroidAudioIODeviceType);
};


//==============================================================================
AudioIODeviceType* juce_createAudioIODeviceType_Android()
{
    return new AndroidAudioIODeviceType();
}


#endif

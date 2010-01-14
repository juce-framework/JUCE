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

// (This file gets included by juce_win32_NativeCode.cpp, rather than being
// compiled on its own).
#if JUCE_INCLUDED_FILE && JUCE_ASIO

#undef WINDOWS

//==============================================================================
// #define ASIO_DEBUGGING

#ifdef ASIO_DEBUGGING
  #define log(a) { Logger::writeToLog (a); DBG (a) }
#else
  #define log(a) {}
#endif


//==============================================================================
#ifdef ASIO_DEBUGGING
static void logError (const String& context, long error)
{
    String err ("unknown error");

    if (error == ASE_NotPresent)
        err = "Not Present";
    else if (error == ASE_HWMalfunction)
        err = "Hardware Malfunction";
    else if (error == ASE_InvalidParameter)
        err = "Invalid Parameter";
    else if (error == ASE_InvalidMode)
        err = "Invalid Mode";
    else if (error == ASE_SPNotAdvancing)
        err = "Sample position not advancing";
    else if (error == ASE_NoClock)
        err = "No Clock";
    else if (error == ASE_NoMemory)
        err = "Out of memory";

    log (T("!!error: ") + context + T(" - ") + err);
}
#else
  #define logError(a, b) {}
#endif

//==============================================================================
class ASIOAudioIODevice;
static ASIOAudioIODevice* volatile currentASIODev[3] = { 0, 0, 0 };

static const int maxASIOChannels = 160;


//==============================================================================
class JUCE_API  ASIOAudioIODevice  : public AudioIODevice,
                                     private Timer
{
public:
    Component ourWindow;

    ASIOAudioIODevice (const String& name_, const CLSID classId_, const int slotNumber,
                       const String& optionalDllForDirectLoading_)
       : AudioIODevice (name_, T("ASIO")),
         asioObject (0),
         classId (classId_),
         optionalDllForDirectLoading (optionalDllForDirectLoading_),
         currentBitDepth (16),
         currentSampleRate (0),
         isOpen_ (false),
         isStarted (false),
         postOutput (true),
         insideControlPanelModalLoop (false),
         shouldUsePreferredSize (false)
    {
        name = name_;

        ourWindow.addToDesktop (0);
        windowHandle = ourWindow.getWindowHandle();

        jassert (currentASIODev [slotNumber] == 0);
        currentASIODev [slotNumber] = this;

        openDevice();
    }

    ~ASIOAudioIODevice()
    {
        for (int i = 0; i < numElementsInArray (currentASIODev); ++i)
            if (currentASIODev[i] == this)
                currentASIODev[i] = 0;

        close();
        log ("ASIO - exiting");
        removeCurrentDriver();
    }

    void updateSampleRates()
    {
        // find a list of sample rates..
        const double possibleSampleRates[] = { 44100.0, 48000.0, 88200.0, 96000.0, 176400.0, 192000.0 };
        sampleRates.clear();

        if (asioObject != 0)
        {
            for (int index = 0; index < numElementsInArray (possibleSampleRates); ++index)
            {
                const long err = asioObject->canSampleRate (possibleSampleRates[index]);

                if (err == 0)
                {
                    sampleRates.add ((int) possibleSampleRates[index]);
                    log (T("rate: ") + String ((int) possibleSampleRates[index]));
                }
                else if (err != ASE_NoClock)
                {
                    logError (T("CanSampleRate"), err);
                }
            }

            if (sampleRates.size() == 0)
            {
                double cr = 0;
                const long err = asioObject->getSampleRate (&cr);
                log (T("No sample rates supported - current rate: ") + String ((int) cr));

                if (err == 0)
                    sampleRates.add ((int) cr);
            }
        }
    }

    const StringArray getOutputChannelNames()
    {
        return outputChannelNames;
    }

    const StringArray getInputChannelNames()
    {
        return inputChannelNames;
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
        return preferredSize;
    }

    const String open (const BitArray& inputChannels,
                       const BitArray& outputChannels,
                       double sr,
                       int bufferSizeSamples)
    {
        close();
        currentCallback = 0;

        if (bufferSizeSamples <= 0)
            shouldUsePreferredSize = true;

        if (asioObject == 0 || ! isASIOOpen)
        {
            log ("Warning: device not open");
            const String err (openDevice());

            if (asioObject == 0 || ! isASIOOpen)
                return err;
        }

        isStarted = false;
        bufferIndex = -1;
        long err = 0;

        long newPreferredSize = 0;

        // if the preferred size has just changed, assume it's a control panel thing and use it as the new value.
        minSize = 0;
        maxSize = 0;
        newPreferredSize = 0;
        granularity = 0;

        if (asioObject->getBufferSize (&minSize, &maxSize, &newPreferredSize, &granularity) == 0)
        {
            if (preferredSize != 0 && newPreferredSize != 0 && newPreferredSize != preferredSize)
                shouldUsePreferredSize = true;

            preferredSize = newPreferredSize;
        }

        // unfortunate workaround for certain manufacturers whose drivers crash horribly if you make
        // dynamic changes to the buffer size...
        shouldUsePreferredSize = shouldUsePreferredSize
                                   || getName().containsIgnoreCase (T("Digidesign"));

        if (shouldUsePreferredSize)
        {
            log ("Using preferred size for buffer..");

            if ((err = asioObject->getBufferSize (&minSize, &maxSize, &preferredSize, &granularity)) == 0)
            {
                bufferSizeSamples = preferredSize;
            }
            else
            {
                bufferSizeSamples = 1024;
                logError ("GetBufferSize1", err);
            }

            shouldUsePreferredSize = false;
        }

        int sampleRate = roundDoubleToInt (sr);
        currentSampleRate = sampleRate;
        currentBlockSizeSamples = bufferSizeSamples;
        currentChansOut.clear();
        currentChansIn.clear();
        zeromem (inBuffers, sizeof (inBuffers));
        zeromem (outBuffers, sizeof (outBuffers));

        updateSampleRates();

        if (sampleRate == 0 || (sampleRates.size() > 0 && ! sampleRates.contains (sampleRate)))
            sampleRate = sampleRates[0];

        jassert (sampleRate != 0);
        if (sampleRate == 0)
            sampleRate = 44100;

        long numSources = 32;
        ASIOClockSource clocks[32];
        zeromem (clocks, sizeof (clocks));
        asioObject->getClockSources (clocks, &numSources);
        bool isSourceSet = false;

        // careful not to remove this loop because it does more than just logging!
        int i;
        for (i = 0; i < numSources; ++i)
        {
            String s ("clock: ");
            s += clocks[i].name;

            if (clocks[i].isCurrentSource)
            {
                isSourceSet = true;
                s << " (cur)";
            }

            log (s);
        }

        if (numSources > 1 && ! isSourceSet)
        {
            log ("setting clock source");
            asioObject->setClockSource (clocks[0].index);
            Thread::sleep (20);
        }
        else
        {
            if (numSources == 0)
            {
                log ("ASIO - no clock sources!");
            }
        }

        double cr = 0;
        err = asioObject->getSampleRate (&cr);
        if (err == 0)
        {
            currentSampleRate = cr;
        }
        else
        {
            logError ("GetSampleRate", err);
            currentSampleRate = 0;
        }

        error = String::empty;
        needToReset = false;
        isReSync = false;
        err = 0;
        bool buffersCreated = false;

        if (currentSampleRate != sampleRate)
        {
            log (T("ASIO samplerate: ") + String (currentSampleRate) + T(" to ") + String (sampleRate));
            err = asioObject->setSampleRate (sampleRate);

            if (err == ASE_NoClock && numSources > 0)
            {
                log ("trying to set a clock source..");
                Thread::sleep (10);
                err = asioObject->setClockSource (clocks[0].index);
                if (err != 0)
                {
                    logError ("SetClock", err);
                }

                Thread::sleep (10);
                err = asioObject->setSampleRate (sampleRate);
            }
        }

        if (err == 0)
        {
            currentSampleRate = sampleRate;

            if (needToReset)
            {
                if (isReSync)
                {
                    log ("Resync request");
                }

                log ("! Resetting ASIO after sample rate change");
                removeCurrentDriver();

                loadDriver();
                const String error (initDriver());

                if (error.isNotEmpty())
                {
                    log (T("ASIOInit: ") + error);
                }

                needToReset = false;
                isReSync = false;
            }

            numActiveInputChans = 0;
            numActiveOutputChans = 0;

            ASIOBufferInfo* info = bufferInfos;
            int i;
            for (i = 0; i < totalNumInputChans; ++i)
            {
                if (inputChannels[i])
                {
                    currentChansIn.setBit (i);
                    info->isInput = 1;
                    info->channelNum = i;
                    info->buffers[0] = info->buffers[1] = 0;
                    ++info;
                    ++numActiveInputChans;
                }
            }

            for (i = 0; i < totalNumOutputChans; ++i)
            {
                if (outputChannels[i])
                {
                    currentChansOut.setBit (i);
                    info->isInput = 0;
                    info->channelNum = i;
                    info->buffers[0] = info->buffers[1] = 0;
                    ++info;
                    ++numActiveOutputChans;
                }
            }

            const int totalBuffers = numActiveInputChans + numActiveOutputChans;

            callbacks.sampleRateDidChange = &sampleRateChangedCallback;

            if (currentASIODev[0] == this)
            {
                callbacks.bufferSwitch = &bufferSwitchCallback0;
                callbacks.asioMessage = &asioMessagesCallback0;
                callbacks.bufferSwitchTimeInfo = &bufferSwitchTimeInfoCallback0;
            }
            else if (currentASIODev[1] == this)
            {
                callbacks.bufferSwitch = &bufferSwitchCallback1;
                callbacks.asioMessage = &asioMessagesCallback1;
                callbacks.bufferSwitchTimeInfo = &bufferSwitchTimeInfoCallback1;
            }
            else if (currentASIODev[2] == this)
            {
                callbacks.bufferSwitch = &bufferSwitchCallback2;
                callbacks.asioMessage = &asioMessagesCallback2;
                callbacks.bufferSwitchTimeInfo = &bufferSwitchTimeInfoCallback2;
            }
            else
            {
                jassertfalse
            }

            log ("disposing buffers");
            err = asioObject->disposeBuffers();

            log (T("creating buffers: ") + String (totalBuffers) + T(", ") + String (currentBlockSizeSamples));
            err = asioObject->createBuffers (bufferInfos,
                                             totalBuffers,
                                             currentBlockSizeSamples,
                                             &callbacks);

            if (err != 0)
            {
                currentBlockSizeSamples = preferredSize;
                logError ("create buffers 2", err);

                asioObject->disposeBuffers();
                err = asioObject->createBuffers (bufferInfos,
                                                 totalBuffers,
                                                 currentBlockSizeSamples,
                                                 &callbacks);
            }

            if (err == 0)
            {
                buffersCreated = true;

                tempBuffer.calloc (totalBuffers * currentBlockSizeSamples + 32);

                int n = 0;
                Array <int> types;
                currentBitDepth = 16;

                for (i = 0; i < jmin ((int) totalNumInputChans, maxASIOChannels); ++i)
                {
                    if (inputChannels[i])
                    {
                        inBuffers[n] = tempBuffer + (currentBlockSizeSamples * n);

                        ASIOChannelInfo channelInfo;
                        zerostruct (channelInfo);

                        channelInfo.channel = i;
                        channelInfo.isInput = 1;
                        asioObject->getChannelInfo (&channelInfo);

                        types.addIfNotAlreadyThere (channelInfo.type);
                        typeToFormatParameters (channelInfo.type,
                                                inputChannelBitDepths[n],
                                                inputChannelBytesPerSample[n],
                                                inputChannelIsFloat[n],
                                                inputChannelLittleEndian[n]);

                        currentBitDepth = jmax (currentBitDepth, inputChannelBitDepths[n]);

                        ++n;
                    }
                }

                jassert (numActiveInputChans == n);
                n = 0;

                for (i = 0; i < jmin ((int) totalNumOutputChans, maxASIOChannels); ++i)
                {
                    if (outputChannels[i])
                    {
                        outBuffers[n] = tempBuffer + (currentBlockSizeSamples * (numActiveInputChans + n));

                        ASIOChannelInfo channelInfo;
                        zerostruct (channelInfo);

                        channelInfo.channel = i;
                        channelInfo.isInput = 0;
                        asioObject->getChannelInfo (&channelInfo);

                        types.addIfNotAlreadyThere (channelInfo.type);
                        typeToFormatParameters (channelInfo.type,
                                                outputChannelBitDepths[n],
                                                outputChannelBytesPerSample[n],
                                                outputChannelIsFloat[n],
                                                outputChannelLittleEndian[n]);

                        currentBitDepth = jmax (currentBitDepth, outputChannelBitDepths[n]);

                        ++n;
                    }
                }

                jassert (numActiveOutputChans == n);

                for (i = types.size(); --i >= 0;)
                {
                    log (T("channel format: ") + String (types[i]));
                }

                jassert (n <= totalBuffers);

                for (i = 0; i < numActiveOutputChans; ++i)
                {
                    const int size = currentBlockSizeSamples * (outputChannelBitDepths[i] >> 3);

                    if (bufferInfos [numActiveInputChans + i].buffers[0] == 0
                        || bufferInfos [numActiveInputChans + i].buffers[1] == 0)
                    {
                        log ("!! Null buffers");
                    }
                    else
                    {
                        zeromem (bufferInfos[numActiveInputChans + i].buffers[0], size);
                        zeromem (bufferInfos[numActiveInputChans + i].buffers[1], size);
                    }
                }

                inputLatency = outputLatency = 0;

                if (asioObject->getLatencies (&inputLatency, &outputLatency) != 0)
                {
                    log ("ASIO - no latencies");
                }
                else
                {
                    log (T("ASIO latencies: ")
                           + String ((int) outputLatency)
                           + T(", ")
                           + String ((int) inputLatency));
                }

                isOpen_ = true;

                log ("starting ASIO");
                calledback = false;
                err = asioObject->start();

                if (err != 0)
                {
                    isOpen_ = false;
                    log ("ASIO - stop on failure");
                    Thread::sleep (10);
                    asioObject->stop();
                    error = "Can't start device";
                    Thread::sleep (10);
                }
                else
                {
                    int count = 300;
                    while (--count > 0 && ! calledback)
                        Thread::sleep (10);

                    isStarted = true;

                    if (! calledback)
                    {
                        error = "Device didn't start correctly";
                        log ("ASIO didn't callback - stopping..");
                        asioObject->stop();
                    }
                }
            }
            else
            {
                error = "Can't create i/o buffers";
            }
        }
        else
        {
            error = "Can't set sample rate: ";
            error << sampleRate;
        }

        if (error.isNotEmpty())
        {
            logError (error, err);

            if (asioObject != 0 && buffersCreated)
                asioObject->disposeBuffers();

            Thread::sleep (20);
            isStarted = false;
            isOpen_ = false;
            close();
        }

        needToReset = false;
        isReSync = false;

        return error;
    }

    void close()
    {
        error = String::empty;
        stopTimer();
        stop();

        if (isASIOOpen && isOpen_)
        {
            const ScopedLock sl (callbackLock);

            isOpen_ = false;
            isStarted = false;
            needToReset = false;
            isReSync = false;

            log ("ASIO - stopping");

            if (asioObject != 0)
            {
                Thread::sleep (20);
                asioObject->stop();
                Thread::sleep (10);
                asioObject->disposeBuffers();
            }

            Thread::sleep (10);
        }
    }

    bool isOpen()
    {
        return isOpen_ || insideControlPanelModalLoop;
    }

    int getCurrentBufferSizeSamples()
    {
        return currentBlockSizeSamples;
    }

    double getCurrentSampleRate()
    {
        return currentSampleRate;
    }

    const BitArray getActiveOutputChannels() const
    {
        return currentChansOut;
    }

    const BitArray getActiveInputChannels() const
    {
        return currentChansIn;
    }

    int getCurrentBitDepth()
    {
        return currentBitDepth;
    }

    int getOutputLatencyInSamples()
    {
        return outputLatency + currentBlockSizeSamples / 4;
    }

    int getInputLatencyInSamples()
    {
        return inputLatency + currentBlockSizeSamples / 4;
    }

    void start (AudioIODeviceCallback* callback)
    {
        if (callback != 0)
        {
            callback->audioDeviceAboutToStart (this);

            const ScopedLock sl (callbackLock);
            currentCallback = callback;
        }
    }

    void stop()
    {
        AudioIODeviceCallback* const lastCallback = currentCallback;

        {
            const ScopedLock sl (callbackLock);
            currentCallback = 0;
        }

        if (lastCallback != 0)
            lastCallback->audioDeviceStopped();
    }

    bool isPlaying()
    {
        return isASIOOpen && (currentCallback != 0);
    }

    const String getLastError()
    {
        return error;
    }

    bool hasControlPanel() const
    {
        return true;
    }

    bool showControlPanel()
    {
        log ("ASIO - showing control panel");

        Component modalWindow (String::empty);
        modalWindow.setOpaque (true);
        modalWindow.addToDesktop (0);
        modalWindow.enterModalState();
        bool done = false;

        JUCE_TRY
        {
            // are there are devices that need to be closed before showing their control panel?
            // close();
            insideControlPanelModalLoop = true;

            const uint32 started = Time::getMillisecondCounter();

            if (asioObject != 0)
            {
                asioObject->controlPanel();

                const int spent = (int) Time::getMillisecondCounter() - (int) started;

                log (T("spent: ") + String (spent));

                if (spent > 300)
                {
                    shouldUsePreferredSize = true;
                    done = true;
                }
            }
        }
        JUCE_CATCH_ALL

        insideControlPanelModalLoop = false;
        return done;
    }

    void resetRequest() throw()
    {
        needToReset = true;
    }

    void resyncRequest() throw()
    {
        needToReset = true;
        isReSync = true;
    }

    void timerCallback()
    {
        if (! insideControlPanelModalLoop)
        {
            stopTimer();

            // used to cause a reset
            log ("! ASIO restart request!");

            if (isOpen_)
            {
                AudioIODeviceCallback* const oldCallback = currentCallback;

                close();
                open (BitArray (currentChansIn), BitArray (currentChansOut),
                      currentSampleRate, currentBlockSizeSamples);

                if (oldCallback != 0)
                    start (oldCallback);
            }
        }
        else
        {
            startTimer (100);
        }
    }

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    //==============================================================================
    IASIO* volatile asioObject;
    ASIOCallbacks callbacks;

    void* windowHandle;
    CLSID classId;
    const String optionalDllForDirectLoading;
    String error;

    long totalNumInputChans, totalNumOutputChans;
    StringArray inputChannelNames, outputChannelNames;

    Array<int> sampleRates, bufferSizes;
    long inputLatency, outputLatency;
    long minSize, maxSize, preferredSize, granularity;

    int volatile currentBlockSizeSamples;
    int volatile currentBitDepth;
    double volatile currentSampleRate;
    BitArray currentChansOut, currentChansIn;
    AudioIODeviceCallback* volatile currentCallback;
    CriticalSection callbackLock;

    ASIOBufferInfo bufferInfos [maxASIOChannels];
    float* inBuffers [maxASIOChannels];
    float* outBuffers [maxASIOChannels];

    int inputChannelBitDepths [maxASIOChannels];
    int outputChannelBitDepths [maxASIOChannels];
    int inputChannelBytesPerSample [maxASIOChannels];
    int outputChannelBytesPerSample [maxASIOChannels];
    bool inputChannelIsFloat [maxASIOChannels];
    bool outputChannelIsFloat [maxASIOChannels];
    bool inputChannelLittleEndian [maxASIOChannels];
    bool outputChannelLittleEndian [maxASIOChannels];

    WaitableEvent event1;
    HeapBlock <float> tempBuffer;
    int volatile bufferIndex, numActiveInputChans, numActiveOutputChans;

    bool isOpen_, isStarted;
    bool volatile isASIOOpen;
    bool volatile calledback;
    bool volatile littleEndian, postOutput, needToReset, isReSync;
    bool volatile insideControlPanelModalLoop;
    bool volatile shouldUsePreferredSize;


    //==============================================================================
    void removeCurrentDriver()
    {
        if (asioObject != 0)
        {
            asioObject->Release();
            asioObject = 0;
        }
    }

    bool loadDriver()
    {
        removeCurrentDriver();

        JUCE_TRY
        {
            if (CoCreateInstance (classId, 0, CLSCTX_INPROC_SERVER,
                                  classId, (void**) &asioObject) == S_OK)
            {
                return true;
            }

            // If a class isn't registered but we have a path for it, we can fallback to
            // doing a direct load of the COM object (only available via the juce_createASIOAudioIODeviceForGUID function).
            if (optionalDllForDirectLoading.isNotEmpty())
            {
                HMODULE h = LoadLibrary (optionalDllForDirectLoading);

                if (h != 0)
                {
                    typedef HRESULT (CALLBACK* DllGetClassObjectFunc) (REFCLSID clsid, REFIID iid, LPVOID* ppv);
                    DllGetClassObjectFunc dllGetClassObject = (DllGetClassObjectFunc) GetProcAddress (h, "DllGetClassObject");

                    if (dllGetClassObject != 0)
                    {
                        IClassFactory* classFactory = 0;
                        HRESULT hr = dllGetClassObject (classId, IID_IClassFactory, (void**) &classFactory);

                        if (classFactory != 0)
                        {
                            hr = classFactory->CreateInstance (0, classId, (void**) &asioObject);
                            classFactory->Release();
                        }

                        return asioObject != 0;
                    }
                }
            }
        }
        JUCE_CATCH_ALL

        asioObject = 0;
        return false;
    }

    const String initDriver()
    {
        if (asioObject != 0)
        {
            char buffer [256];
            zeromem (buffer, sizeof (buffer));

            if (! asioObject->init (windowHandle))
            {
                asioObject->getErrorMessage (buffer);
                return String (buffer, sizeof (buffer) - 1);
            }

            // just in case any daft drivers expect this to be called..
            asioObject->getDriverName (buffer);

            return String::empty;
        }

        return "No Driver";
    }

    const String openDevice()
    {
        // use this in case the driver starts opening dialog boxes..
        Component modalWindow (String::empty);
        modalWindow.setOpaque (true);
        modalWindow.addToDesktop (0);
        modalWindow.enterModalState();

        // open the device and get its info..
        log (T("opening ASIO device: ") + getName());

        needToReset = false;
        isReSync = false;
        outputChannelNames.clear();
        inputChannelNames.clear();
        bufferSizes.clear();
        sampleRates.clear();
        isASIOOpen = false;
        isOpen_ = false;
        totalNumInputChans = 0;
        totalNumOutputChans = 0;
        numActiveInputChans = 0;
        numActiveOutputChans = 0;
        currentCallback = 0;

        error = String::empty;

        if (getName().isEmpty())
            return error;

        long err = 0;

        if (loadDriver())
        {
            if ((error = initDriver()).isEmpty())
            {
                numActiveInputChans = 0;
                numActiveOutputChans = 0;
                totalNumInputChans = 0;
                totalNumOutputChans = 0;

                if (asioObject != 0
                     && (err = asioObject->getChannels (&totalNumInputChans, &totalNumOutputChans)) == 0)
                {
                    log (String ((int) totalNumInputChans) + T(" in, ") + String ((int) totalNumOutputChans) + T(" out"));

                    if ((err = asioObject->getBufferSize (&minSize, &maxSize, &preferredSize, &granularity)) == 0)
                    {
                        // find a list of buffer sizes..
                        log (String ((int) minSize) + T(" ") + String ((int) maxSize) + T(" ") + String ((int)preferredSize) + T(" ") + String ((int)granularity));

                        if (granularity >= 0)
                        {
                            granularity = jmax (1, (int) granularity);

                            for (int i = jmax ((int) minSize, (int) granularity); i < jmin (6400, (int) maxSize); i += granularity)
                                bufferSizes.addIfNotAlreadyThere (granularity * (i / granularity));
                        }
                        else if (granularity < 0)
                        {
                            for (int i = 0; i < 18; ++i)
                            {
                                const int s = (1 << i);

                                if (s >= minSize && s <= maxSize)
                                    bufferSizes.add (s);
                            }
                        }

                        if (! bufferSizes.contains (preferredSize))
                            bufferSizes.insert (0, preferredSize);

                        double currentRate = 0;
                        asioObject->getSampleRate (&currentRate);

                        if (currentRate <= 0.0 || currentRate > 192001.0)
                        {
                            log ("setting sample rate");
                            err = asioObject->setSampleRate (44100.0);
                            if (err != 0)
                            {
                                logError ("setting sample rate", err);
                            }

                            asioObject->getSampleRate (&currentRate);
                        }

                        currentSampleRate = currentRate;

                        postOutput = (asioObject->outputReady() == 0);
                        if (postOutput)
                        {
                            log ("ASIO outputReady = ok");
                        }

                        updateSampleRates();

                        // ..because cubase does it at this point
                        inputLatency = outputLatency = 0;
                        if (asioObject->getLatencies (&inputLatency, &outputLatency) != 0)
                        {
                            log ("ASIO - no latencies");
                        }

                        log (String ("latencies: ")
                                + String ((int) inputLatency)
                                + T(", ") + String ((int) outputLatency));

                        // create some dummy buffers now.. because cubase does..
                        numActiveInputChans = 0;
                        numActiveOutputChans = 0;

                        ASIOBufferInfo* info = bufferInfos;
                        int i, numChans = 0;
                        for (i = 0; i < jmin (2, (int) totalNumInputChans); ++i)
                        {
                            info->isInput = 1;
                            info->channelNum = i;
                            info->buffers[0] = info->buffers[1] = 0;
                            ++info;
                            ++numChans;
                        }

                        const int outputBufferIndex = numChans;

                        for (i = 0; i < jmin (2, (int) totalNumOutputChans); ++i)
                        {
                            info->isInput = 0;
                            info->channelNum = i;
                            info->buffers[0] = info->buffers[1] = 0;
                            ++info;
                            ++numChans;
                        }


                        callbacks.sampleRateDidChange = &sampleRateChangedCallback;

                        if (currentASIODev[0] == this)
                        {
                            callbacks.bufferSwitch = &bufferSwitchCallback0;
                            callbacks.asioMessage = &asioMessagesCallback0;
                            callbacks.bufferSwitchTimeInfo = &bufferSwitchTimeInfoCallback0;
                        }
                        else if (currentASIODev[1] == this)
                        {
                            callbacks.bufferSwitch = &bufferSwitchCallback1;
                            callbacks.asioMessage = &asioMessagesCallback1;
                            callbacks.bufferSwitchTimeInfo = &bufferSwitchTimeInfoCallback1;
                        }
                        else if (currentASIODev[2] == this)
                        {
                            callbacks.bufferSwitch = &bufferSwitchCallback2;
                            callbacks.asioMessage = &asioMessagesCallback2;
                            callbacks.bufferSwitchTimeInfo = &bufferSwitchTimeInfoCallback2;
                        }
                        else
                        {
                            jassertfalse
                        }

                        log (T("creating buffers (dummy): ") + String (numChans)
                               + T(", ") + String ((int) preferredSize));

                        if (preferredSize > 0)
                        {
                            err = asioObject->createBuffers (bufferInfos, numChans, preferredSize, &callbacks);
                            if (err != 0)
                            {
                                logError ("dummy buffers", err);
                            }
                        }

                        long newInps = 0, newOuts = 0;
                        asioObject->getChannels (&newInps, &newOuts);

                        if (totalNumInputChans != newInps || totalNumOutputChans != newOuts)
                        {
                            totalNumInputChans = newInps;
                            totalNumOutputChans = newOuts;

                            log (String ((int) totalNumInputChans) + T(" in; ")
                                  + String ((int) totalNumOutputChans) + T(" out"));
                        }

                        updateSampleRates();

                        ASIOChannelInfo channelInfo;
                        channelInfo.type = 0;

                        for (i = 0; i < totalNumInputChans; ++i)
                        {
                            zerostruct (channelInfo);
                            channelInfo.channel = i;
                            channelInfo.isInput = 1;
                            asioObject->getChannelInfo (&channelInfo);

                            inputChannelNames.add (String (channelInfo.name));
                        }

                        for (i = 0; i < totalNumOutputChans; ++i)
                        {
                            zerostruct (channelInfo);
                            channelInfo.channel = i;
                            channelInfo.isInput = 0;
                            asioObject->getChannelInfo (&channelInfo);

                            outputChannelNames.add (String (channelInfo.name));

                            typeToFormatParameters (channelInfo.type,
                                                    outputChannelBitDepths[i],
                                                    outputChannelBytesPerSample[i],
                                                    outputChannelIsFloat[i],
                                                    outputChannelLittleEndian[i]);

                            if (i < 2)
                            {
                                // clear the channels that are used with the dummy stuff
                                const int bytesPerBuffer = preferredSize * (outputChannelBitDepths[i] >> 3);
                                zeromem (bufferInfos [outputBufferIndex + i].buffers[0], bytesPerBuffer);
                                zeromem (bufferInfos [outputBufferIndex + i].buffers[1], bytesPerBuffer);
                            }
                        }

                        outputChannelNames.trim();
                        inputChannelNames.trim();
                        outputChannelNames.appendNumbersToDuplicates (false, true);
                        inputChannelNames.appendNumbersToDuplicates (false, true);

                        // start and stop because cubase does it..
                        asioObject->getLatencies (&inputLatency, &outputLatency);

                        if ((err = asioObject->start()) != 0)
                        {
                            // ignore an error here, as it might start later after setting other stuff up
                            logError ("ASIO start", err);
                        }

                        Thread::sleep (100);
                        asioObject->stop();
                    }
                    else
                    {
                        error = "Can't detect buffer sizes";
                    }
                }
                else
                {
                    error = "Can't detect asio channels";
                }
            }
        }
        else
        {
            error = "No such device";
        }

        if (error.isNotEmpty())
        {
            logError (error, err);

            if (asioObject != 0)
                asioObject->disposeBuffers();

            removeCurrentDriver();
            isASIOOpen = false;
        }
        else
        {
            isASIOOpen = true;
            log ("ASIO device open");
        }

        isOpen_ = false;
        needToReset = false;
        isReSync = false;

        return error;
    }

    //==============================================================================
    void callback (const long index) throw()
    {
        if (isStarted)
        {
            bufferIndex = index;
            processBuffer();
        }
        else
        {
            if (postOutput && (asioObject != 0))
                asioObject->outputReady();
        }

        calledback = true;
    }

    void processBuffer() throw()
    {
        const ASIOBufferInfo* const infos = bufferInfos;
        const int bi = bufferIndex;

        const ScopedLock sl (callbackLock);

        if (needToReset)
        {
            needToReset = false;

            if (isReSync)
            {
                log ("! ASIO resync");
                isReSync = false;
            }
            else
            {
                startTimer (20);
            }
        }

        if (bi >= 0)
        {
            const int samps = currentBlockSizeSamples;

            if (currentCallback != 0)
            {
                int i;
                for (i = 0; i < numActiveInputChans; ++i)
                {
                    float* const dst = inBuffers[i];

                    jassert (dst != 0);

                    const char* const src = (const char*) (infos[i].buffers[bi]);

                    if (inputChannelIsFloat[i])
                    {
                        memcpy (dst, src, samps * sizeof (float));
                    }
                    else
                    {
                        jassert (dst == tempBuffer + (samps * i));

                        switch (inputChannelBitDepths[i])
                        {
                        case 16:
                            convertInt16ToFloat (src, dst, inputChannelBytesPerSample[i],
                                                 samps, inputChannelLittleEndian[i]);
                            break;

                        case 24:
                            convertInt24ToFloat (src, dst, inputChannelBytesPerSample[i],
                                                 samps, inputChannelLittleEndian[i]);
                            break;

                        case 32:
                            convertInt32ToFloat (src, dst, inputChannelBytesPerSample[i],
                                                 samps, inputChannelLittleEndian[i]);
                            break;

                        case 64:
                            jassertfalse
                            break;
                        }
                    }
                }

                currentCallback->audioDeviceIOCallback ((const float**) inBuffers,
                                                        numActiveInputChans,
                                                        outBuffers,
                                                        numActiveOutputChans,
                                                        samps);

                for (i = 0; i < numActiveOutputChans; ++i)
                {
                    float* const src = outBuffers[i];

                    jassert (src != 0);

                    char* const dst = (char*) (infos [numActiveInputChans + i].buffers[bi]);

                    if (outputChannelIsFloat[i])
                    {
                        memcpy (dst, src, samps * sizeof (float));
                    }
                    else
                    {
                        jassert (src == tempBuffer + (samps * (numActiveInputChans + i)));

                        switch (outputChannelBitDepths[i])
                        {
                        case 16:
                            convertFloatToInt16 (src, dst, outputChannelBytesPerSample[i],
                                                 samps, outputChannelLittleEndian[i]);
                            break;

                        case 24:
                            convertFloatToInt24 (src, dst, outputChannelBytesPerSample[i],
                                                 samps, outputChannelLittleEndian[i]);
                            break;

                        case 32:
                            convertFloatToInt32 (src, dst, outputChannelBytesPerSample[i],
                                                 samps, outputChannelLittleEndian[i]);
                            break;

                        case 64:
                            jassertfalse
                            break;
                        }
                    }
                }
            }
            else
            {
                for (int i = 0; i < numActiveOutputChans; ++i)
                {
                    const int bytesPerBuffer = samps * (outputChannelBitDepths[i] >> 3);
                    zeromem (infos[numActiveInputChans + i].buffers[bi], bytesPerBuffer);
                }
            }
        }

        if (postOutput)
            asioObject->outputReady();
    }

    //==============================================================================
    static ASIOTime* bufferSwitchTimeInfoCallback0 (ASIOTime*, long index, long) throw()
    {
        if (currentASIODev[0] != 0)
            currentASIODev[0]->callback (index);

        return 0;
    }

    static ASIOTime* bufferSwitchTimeInfoCallback1 (ASIOTime*, long index, long) throw()
    {
        if (currentASIODev[1] != 0)
            currentASIODev[1]->callback (index);

        return 0;
    }

    static ASIOTime* bufferSwitchTimeInfoCallback2 (ASIOTime*, long index, long) throw()
    {
        if (currentASIODev[2] != 0)
            currentASIODev[2]->callback (index);

        return 0;
    }

    static void bufferSwitchCallback0 (long index, long) throw()
    {
        if (currentASIODev[0] != 0)
            currentASIODev[0]->callback (index);
    }

    static void bufferSwitchCallback1 (long index, long) throw()
    {
        if (currentASIODev[1] != 0)
            currentASIODev[1]->callback (index);
    }

    static void bufferSwitchCallback2 (long index, long) throw()
    {
        if (currentASIODev[2] != 0)
            currentASIODev[2]->callback (index);
    }

    static long asioMessagesCallback0 (long selector, long value, void*, double*) throw()
    {
        return asioMessagesCallback (selector, value, 0);
    }

    static long asioMessagesCallback1 (long selector, long value, void*, double*) throw()
    {
        return asioMessagesCallback (selector, value, 1);
    }

    static long asioMessagesCallback2 (long selector, long value, void*, double*) throw()
    {
        return asioMessagesCallback (selector, value, 2);
    }

    //==============================================================================
    static long asioMessagesCallback (long selector, long value, const int deviceIndex) throw()
    {
        switch (selector)
        {
        case kAsioSelectorSupported:
            if (value == kAsioResetRequest
                || value == kAsioEngineVersion
                || value == kAsioResyncRequest
                || value == kAsioLatenciesChanged
                || value == kAsioSupportsInputMonitor)
                return 1;
            break;

        case kAsioBufferSizeChange:
            break;

        case kAsioResetRequest:
            if (currentASIODev[deviceIndex] != 0)
                currentASIODev[deviceIndex]->resetRequest();

            return 1;

        case kAsioResyncRequest:
            if (currentASIODev[deviceIndex] != 0)
                currentASIODev[deviceIndex]->resyncRequest();

            return 1;

        case kAsioLatenciesChanged:
            return 1;

        case kAsioEngineVersion:
            return 2;

        case kAsioSupportsTimeInfo:
        case kAsioSupportsTimeCode:
            return 0;
        }

        return 0;
    }

    static void sampleRateChangedCallback (ASIOSampleRate) throw()
    {
    }

    //==============================================================================
    static void convertInt16ToFloat (const char* src,
                                     float* dest,
                                     const int srcStrideBytes,
                                     int numSamples,
                                     const bool littleEndian) throw()
    {
        const double g = 1.0 / 32768.0;

        if (littleEndian)
        {
            while (--numSamples >= 0)
            {
                *dest++ = (float) (g * (short) ByteOrder::littleEndianShort (src));
                src += srcStrideBytes;
            }
        }
        else
        {
            while (--numSamples >= 0)
            {
                *dest++ = (float) (g * (short) ByteOrder::bigEndianShort (src));
                src += srcStrideBytes;
            }
        }
    }

    static void convertFloatToInt16 (const float* src,
                                     char* dest,
                                     const int dstStrideBytes,
                                     int numSamples,
                                     const bool littleEndian) throw()
    {
        const double maxVal = (double) 0x7fff;

        if (littleEndian)
        {
            while (--numSamples >= 0)
            {
                *(uint16*) dest = ByteOrder::swapIfBigEndian ((uint16) (short) roundDoubleToInt (jlimit (-maxVal, maxVal, maxVal * *src++)));
                dest += dstStrideBytes;
            }
        }
        else
        {
            while (--numSamples >= 0)
            {
                *(uint16*) dest = ByteOrder::swapIfLittleEndian ((uint16) (short) roundDoubleToInt (jlimit (-maxVal, maxVal, maxVal * *src++)));
                dest += dstStrideBytes;
            }
        }
    }

    static void convertInt24ToFloat (const char* src,
                                     float* dest,
                                     const int srcStrideBytes,
                                     int numSamples,
                                     const bool littleEndian) throw()
    {
        const double g = 1.0 / 0x7fffff;

        if (littleEndian)
        {
            while (--numSamples >= 0)
            {
                *dest++ = (float) (g * ByteOrder::littleEndian24Bit (src));
                src += srcStrideBytes;
            }
        }
        else
        {
            while (--numSamples >= 0)
            {
                *dest++ = (float) (g * ByteOrder::bigEndian24Bit (src));
                src += srcStrideBytes;
            }
        }
    }

    static void convertFloatToInt24 (const float* src,
                                     char* dest,
                                     const int dstStrideBytes,
                                     int numSamples,
                                     const bool littleEndian) throw()
    {
        const double maxVal = (double) 0x7fffff;

        if (littleEndian)
        {
            while (--numSamples >= 0)
            {
                ByteOrder::littleEndian24BitToChars ((uint32) roundDoubleToInt (jlimit (-maxVal, maxVal, maxVal * *src++)), dest);
                dest += dstStrideBytes;
            }
        }
        else
        {
            while (--numSamples >= 0)
            {
                ByteOrder::bigEndian24BitToChars ((uint32) roundDoubleToInt (jlimit (-maxVal, maxVal, maxVal * *src++)), dest);
                dest += dstStrideBytes;
            }
        }
    }

    static void convertInt32ToFloat (const char* src,
                                     float* dest,
                                     const int srcStrideBytes,
                                     int numSamples,
                                     const bool littleEndian) throw()
    {
        const double g = 1.0 / 0x7fffffff;

        if (littleEndian)
        {
            while (--numSamples >= 0)
            {
                *dest++ = (float) (g * (int) ByteOrder::littleEndianInt (src));
                src += srcStrideBytes;
            }
        }
        else
        {
            while (--numSamples >= 0)
            {
                *dest++ = (float) (g * (int) ByteOrder::bigEndianInt (src));
                src += srcStrideBytes;
            }
        }
    }

    static void convertFloatToInt32 (const float* src,
                                     char* dest,
                                     const int dstStrideBytes,
                                     int numSamples,
                                     const bool littleEndian) throw()
    {
        const double maxVal = (double) 0x7fffffff;

        if (littleEndian)
        {
            while (--numSamples >= 0)
            {
                *(uint32*) dest = ByteOrder::swapIfBigEndian ((uint32) roundDoubleToInt (jlimit (-maxVal, maxVal, maxVal * *src++)));
                dest += dstStrideBytes;
            }
        }
        else
        {
            while (--numSamples >= 0)
            {
                *(uint32*) dest = ByteOrder::swapIfLittleEndian ((uint32) roundDoubleToInt (jlimit (-maxVal, maxVal, maxVal * *src++)));
                dest += dstStrideBytes;
            }
        }
    }

    //==============================================================================
    static void typeToFormatParameters (const long type,
                                        int& bitDepth,
                                        int& byteStride,
                                        bool& formatIsFloat,
                                        bool& littleEndian) throw()
    {
        bitDepth = 0;
        littleEndian = false;
        formatIsFloat = false;

        switch (type)
        {
            case ASIOSTInt16MSB:
            case ASIOSTInt16LSB:
            case ASIOSTInt32MSB16:
            case ASIOSTInt32LSB16:
                bitDepth = 16; break;

            case ASIOSTFloat32MSB:
            case ASIOSTFloat32LSB:
                formatIsFloat = true;
                bitDepth = 32; break;

            case ASIOSTInt32MSB:
            case ASIOSTInt32LSB:
                bitDepth = 32; break;

            case ASIOSTInt24MSB:
            case ASIOSTInt24LSB:
            case ASIOSTInt32MSB24:
            case ASIOSTInt32LSB24:
            case ASIOSTInt32MSB18:
            case ASIOSTInt32MSB20:
            case ASIOSTInt32LSB18:
            case ASIOSTInt32LSB20:
                bitDepth = 24; break;

            case ASIOSTFloat64MSB:
            case ASIOSTFloat64LSB:
            default:
                bitDepth = 64;
                break;
        }

        switch (type)
        {
            case ASIOSTInt16MSB:
            case ASIOSTInt32MSB16:
            case ASIOSTFloat32MSB:
            case ASIOSTFloat64MSB:
            case ASIOSTInt32MSB:
            case ASIOSTInt32MSB18:
            case ASIOSTInt32MSB20:
            case ASIOSTInt32MSB24:
            case ASIOSTInt24MSB:
                littleEndian = false; break;

            case ASIOSTInt16LSB:
            case ASIOSTInt32LSB16:
            case ASIOSTFloat32LSB:
            case ASIOSTFloat64LSB:
            case ASIOSTInt32LSB:
            case ASIOSTInt32LSB18:
            case ASIOSTInt32LSB20:
            case ASIOSTInt32LSB24:
            case ASIOSTInt24LSB:
                littleEndian = true; break;

            default:
                break;
        }

        switch (type)
        {
            case ASIOSTInt16LSB:
            case ASIOSTInt16MSB:
                byteStride = 2; break;

            case ASIOSTInt24LSB:
            case ASIOSTInt24MSB:
                byteStride = 3; break;

            case ASIOSTInt32MSB16:
            case ASIOSTInt32LSB16:
            case ASIOSTInt32MSB:
            case ASIOSTInt32MSB18:
            case ASIOSTInt32MSB20:
            case ASIOSTInt32MSB24:
            case ASIOSTInt32LSB:
            case ASIOSTInt32LSB18:
            case ASIOSTInt32LSB20:
            case ASIOSTInt32LSB24:
            case ASIOSTFloat32LSB:
            case ASIOSTFloat32MSB:
                byteStride = 4; break;

            case ASIOSTFloat64MSB:
            case ASIOSTFloat64LSB:
                byteStride = 8; break;

            default:
                break;
        }
    }
};

//==============================================================================
class ASIOAudioIODeviceType  : public AudioIODeviceType
{
public:
    ASIOAudioIODeviceType()
        : AudioIODeviceType (T("ASIO")),
          hasScanned (false)
    {
        CoInitialize (0);
    }

    ~ASIOAudioIODeviceType()
    {
    }

    //==============================================================================
    void scanForDevices()
    {
        hasScanned = true;

        deviceNames.clear();
        classIds.clear();

        HKEY hk = 0;
        int index = 0;

        if (RegOpenKeyA (HKEY_LOCAL_MACHINE, "software\\asio", &hk) == ERROR_SUCCESS)
        {
            for (;;)
            {
                char name [256];

                if (RegEnumKeyA (hk, index++, name, 256) == ERROR_SUCCESS)
                {
                    addDriverInfo (name, hk);
                }
                else
                {
                    break;
                }
            }

            RegCloseKey (hk);
        }
    }

    const StringArray getDeviceNames (const bool /*wantInputNames*/) const
    {
        jassert (hasScanned); // need to call scanForDevices() before doing this

        return deviceNames;
    }

    int getDefaultDeviceIndex (const bool) const
    {
        jassert (hasScanned); // need to call scanForDevices() before doing this

        for (int i = deviceNames.size(); --i >= 0;)
            if (deviceNames[i].containsIgnoreCase (T("asio4all")))
                return i; // asio4all is a safe choice for a default..

#if JUCE_DEBUG
        if (deviceNames.size() > 1 && deviceNames[0].containsIgnoreCase (T("digidesign")))
            return 1; // (the digi m-box driver crashes the app when you run
                      // it in the debugger, which can be a bit annoying)
#endif

        return 0;
    }

    static int findFreeSlot()
    {
        for (int i = 0; i < numElementsInArray (currentASIODev); ++i)
            if (currentASIODev[i] == 0)
                return i;

        jassertfalse;  // unfortunately you can only have a finite number
                       // of ASIO devices open at the same time..
        return -1;
    }

    int getIndexOfDevice (AudioIODevice* d, const bool /*asInput*/) const
    {
        jassert (hasScanned); // need to call scanForDevices() before doing this

        return d == 0 ? -1 : deviceNames.indexOf (d->getName());
    }

    bool hasSeparateInputsAndOutputs() const    { return false; }

    AudioIODevice* createDevice (const String& outputDeviceName,
                                 const String& inputDeviceName)
    {
        // ASIO can't open two different devices for input and output - they must be the same one.
        jassert (inputDeviceName == outputDeviceName || outputDeviceName.isEmpty() || inputDeviceName.isEmpty());
        jassert (hasScanned); // need to call scanForDevices() before doing this

        const int index = deviceNames.indexOf (outputDeviceName.isNotEmpty() ? outputDeviceName
                                                                             : inputDeviceName);

        if (index >= 0)
        {
            const int freeSlot = findFreeSlot();

            if (freeSlot >= 0)
                return new ASIOAudioIODevice (outputDeviceName, *(classIds [index]), freeSlot, String::empty);
        }

        return 0;
    }

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    StringArray deviceNames;
    OwnedArray <CLSID> classIds;

    bool hasScanned;

    //==============================================================================
    static bool checkClassIsOk (const String& classId)
    {
        HKEY hk = 0;
        bool ok = false;

        if (RegOpenKeyA (HKEY_CLASSES_ROOT, "clsid", &hk) == ERROR_SUCCESS)
        {
            int index = 0;

            for (;;)
            {
                char buf [512];

                if (RegEnumKeyA (hk, index++, buf, 512) == ERROR_SUCCESS)
                {
                    if (classId.equalsIgnoreCase (buf))
                    {
                        HKEY subKey, pathKey;

                        if (RegOpenKeyExA (hk, buf, 0, KEY_READ, &subKey) == ERROR_SUCCESS)
                        {
                            if (RegOpenKeyExA (subKey, "InprocServer32", 0, KEY_READ, &pathKey) == ERROR_SUCCESS)
                            {
                                char pathName [600];
                                DWORD dtype = REG_SZ;
                                DWORD dsize = sizeof (pathName);

                                if (RegQueryValueExA (pathKey, 0, 0, &dtype,
                                                      (LPBYTE) pathName, &dsize) == ERROR_SUCCESS)
                                {
                                    OFSTRUCT of;
                                    zerostruct (of);

                                    of.cBytes = sizeof (of);

                                    ok = (OpenFile (String (pathName), &of, OF_EXIST) != 0);
                                }

                                RegCloseKey (pathKey);
                            }

                            RegCloseKey (subKey);
                        }

                        break;
                    }
                }
                else
                {
                    break;
                }
            }

            RegCloseKey (hk);
        }

        return ok;
    }

    //==============================================================================
    void addDriverInfo (const String& keyName, HKEY hk)
    {
        HKEY subKey;

        if (RegOpenKeyExA (hk, keyName, 0, KEY_READ, &subKey) == ERROR_SUCCESS)
        {
            char buf [256];
            DWORD dtype = REG_SZ;
            DWORD dsize = sizeof (buf);
            zeromem (buf, dsize);

            if (RegQueryValueExA (subKey, "clsid", 0, &dtype, (LPBYTE) buf, &dsize) == ERROR_SUCCESS)
            {
                if (dsize > 0 && checkClassIsOk (buf))
                {
                    wchar_t classIdStr [130];
                    MultiByteToWideChar (CP_ACP, 0, buf, -1, classIdStr, 128);

                    String deviceName;
                    CLSID classId;

                    if (CLSIDFromString ((LPOLESTR) classIdStr, &classId) == S_OK)
                    {
                        dtype = REG_SZ;
                        dsize = sizeof (buf);

                        if (RegQueryValueExA (subKey, "description", 0, &dtype, (LPBYTE) buf, &dsize) == ERROR_SUCCESS)
                            deviceName = buf;
                        else
                            deviceName = keyName;

                        log (T("found ") + deviceName);
                        deviceNames.add (deviceName);
                        classIds.add (new CLSID (classId));
                    }
                }

                RegCloseKey (subKey);
            }
        }
    }

    ASIOAudioIODeviceType (const ASIOAudioIODeviceType&);
    const ASIOAudioIODeviceType& operator= (const ASIOAudioIODeviceType&);
};

AudioIODeviceType* juce_createAudioIODeviceType_ASIO()
{
    return new ASIOAudioIODeviceType();
}

AudioIODevice* juce_createASIOAudioIODeviceForGUID (const String& name,
                                                    void* guid,
                                                    const String& optionalDllForDirectLoading)
{
    const int freeSlot = ASIOAudioIODeviceType::findFreeSlot();

    if (freeSlot < 0)
        return 0;

    return new ASIOAudioIODevice (name, *(CLSID*) guid, freeSlot, optionalDllForDirectLoading);
}

#undef log

#endif

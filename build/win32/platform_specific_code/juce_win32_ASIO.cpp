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

#include "win32_headers.h"

#if JUCE_ASIO

//==============================================================================
/*
    This is very frustrating - we only need to use a handful of definitions from
    a couple of the header files in Steinberg's ASIO SDK, and it'd be easy to copy
    about 30 lines of code into this cpp file to create a fully stand-alone ASIO
    implementation...

    ..unfortunately that would break Steinberg's license agreement for use of
    their SDK, so I'm not allowed to do this.

    This means that anyone who wants to use JUCE's ASIO abilities will have to:

    1) Agree to Steinberg's licensing terms and download the ASIO SDK
        (see www.steinberg.net/Steinberg/Developers.asp).

    2) Rebuild the whole of JUCE, setting the global definition JUCE_ASIO (you
       can un-comment the "#define JUCE_ASIO" line in juce_Config.h
       if you prefer). Make sure that your header search path will find the
       iasiodrv.h file that comes with the SDK. (Only about 2-3 of the SDK header
       files are actually needed - so to simplify things, you could just copy
       these into your JUCE directory).
*/
#include "iasiodrv.h"   // if you're compiling and this line causes an error because
                        // you don't have the ASIO SDK installed, you can disable ASIO
                        // support by commenting-out the "#define JUCE_ASIO" line in
                        // juce_Config.h

#include "../../../src/juce_core/basics/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "../../../src/juce_appframework/audio/devices/juce_AudioIODeviceType.h"
#include "../../../src/juce_core/threads/juce_ScopedLock.h"
#include "../../../src/juce_appframework/gui/components/juce_Component.h"
#include "../../../src/juce_core/basics/juce_Time.h"
#include "../../../src/juce_core/threads/juce_Thread.h"
#include "../../../src/juce_appframework/events/juce_Timer.h"
#include "../../../src/juce_appframework/events/juce_MessageManager.h"


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
static ASIOAudioIODevice* volatile currentASIODev = 0;

static IASIO* volatile asioObject = 0;

static const int maxASIOChannels = 160;

static ASIOCallbacks callbacks;
static ASIOBufferInfo bufferInfos[64];

static bool volatile insideControlPanelModalLoop = false;
static bool volatile shouldUsePreferredSize = false;


//==============================================================================
class JUCE_API  ASIOAudioIODevice  : public AudioIODevice,
                                     private Thread,
                                     private Timer
{
public:
    Component ourWindow;

    ASIOAudioIODevice (const String& name_, CLSID classId_)
       : AudioIODevice (name_, T("ASIO")),
         Thread ("Juce ASIO"),
         classId (classId_),
         currentBitDepth (16),
         currentSampleRate (0),
         tempBuffer (0),
         isOpen_ (false),
         isStarted (false),
         postOutput (true)
    {
        name = name_;

        ourWindow.addToDesktop (0);
        windowHandle = ourWindow.getWindowHandle();

        jassert (currentASIODev == 0);
        currentASIODev = this;
        shouldUseThread = false;

        openDevice();
    }

    ~ASIOAudioIODevice()
    {
        jassert (currentASIODev == this);
        if (currentASIODev == this)
            currentASIODev = 0;

        close();
        log ("ASIO - exiting");
        removeCurrentDriver();

        juce_free (tempBuffer);

        if (isUsingThread)
        {
            signalThreadShouldExit();
            event1.signal();
            stopThread (3000);
        }
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
        currentChansOut = outputChannels;
        currentChansIn = inputChannels;

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
            for (i = 0; i < numInputs; ++i)
            {
                if (inputChannels[i])
                {
                    info->isInput = 1;
                    info->channelNum = i;
                    info->buffers[0] = info->buffers[1] = 0;
                    ++info;
                    ++numActiveInputChans;
                }
            }

            for (i = 0; i < numOutputs; ++i)
            {
                if (outputChannels[i])
                {
                    info->isInput = 0;
                    info->channelNum = i;
                    info->buffers[0] = info->buffers[1] = 0;
                    ++info;
                    ++numActiveOutputChans;
                }
            }

            const int totalBuffers = numActiveInputChans + numActiveOutputChans;

            callbacks.bufferSwitch = &bufferSwitchCallback;
            callbacks.sampleRateDidChange = &sampleRateChangedCallback;
            callbacks.asioMessage = &asioMessagesCallback;
            callbacks.bufferSwitchTimeInfo = &bufferSwitchTimeInfoCallback;

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

                jassert (! isThreadRunning());

                juce_free (tempBuffer);

                tempBuffer = (float*) juce_calloc (totalBuffers * currentBlockSizeSamples * sizeof (float) + 128);

                int n = 0;
                Array <int> types;
                currentBitDepth = 16;

                for (i = 0; i < jmin (numInputs, maxASIOChannels); ++i)
                {
                    if (inputChannels[i])
                    {
                        inBuffers[i] = tempBuffer + (currentBlockSizeSamples * n++);

                        ASIOChannelInfo channelInfo;
                        zerostruct (channelInfo);

                        channelInfo.channel = i;
                        channelInfo.isInput = 1;
                        asioObject->getChannelInfo (&channelInfo);

                        types.addIfNotAlreadyThere (channelInfo.type);
                        typeToFormatParameters (channelInfo.type,
                                                inputChannelBitDepths[i],
                                                inputChannelBytesPerSample[i],
                                                inputChannelIsFloat[i],
                                                inputChannelLittleEndian[i]);

                        currentBitDepth = jmax (currentBitDepth, inputChannelBitDepths[i]);
                    }
                    else
                    {
                        inBuffers[i] = 0;
                    }
                }

                for (i = 0; i < jmin (numOutputs, maxASIOChannels); ++i)
                {
                    if (outputChannels[i])
                    {
                        outBuffers[i] = tempBuffer + (currentBlockSizeSamples * n++);

                        ASIOChannelInfo channelInfo;
                        zerostruct (channelInfo);

                        channelInfo.channel = i;
                        channelInfo.isInput = 0;
                        asioObject->getChannelInfo (&channelInfo);

                        types.addIfNotAlreadyThere (channelInfo.type);
                        typeToFormatParameters (channelInfo.type,
                                                outputChannelBitDepths[i],
                                                outputChannelBytesPerSample[i],
                                                outputChannelIsFloat[i],
                                                outputChannelLittleEndian[i]);

                        currentBitDepth = jmax (currentBitDepth, outputChannelBitDepths[i]);
                    }
                    else
                    {
                        outBuffers[i] = 0;
                    }
                }

                for (i = types.size(); --i >= 0;)
                {
                    log (T("channel format: ") + String (types[i]));
                }

                jassert (n <= totalBuffers);

                n = numActiveInputChans;
                for (i = 0; i < numOutputs; ++i)
                {
                    if (outputChannels[i])
                    {
                        const int size = currentBlockSizeSamples * (outputChannelBitDepths[i] >> 3);

                        if (bufferInfos[n].buffers[0] == 0
                            || bufferInfos[n].buffers[1] == 0)
                        {
                            log ("!! Null buffers");
                        }
                        else
                        {
                            zeromem (bufferInfos[n].buffers[0], size);
                            zeromem (bufferInfos[n].buffers[1], size);
                        }

                        ++n;
                    }
                }

                jassert (n <= totalBuffers);

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
                isThreadReady = false;

                if (isUsingThread)
                {
                    event1.wait (1); // reset the event in case it was flipped by a callback from the ASIO->start call in openDevice()
                    startThread (8);

                    int count = 5000;
                    while (--count > 0 && ! isThreadReady)
                        sleep (1);
                }

                if (isUsingThread && ! isThreadRunning())
                {
                    error = "Can't start thread!";
                }
                else
                {
                    log ("starting ASIO");
                    calledback = false;
                    err = asioObject->start();

                    if (err != 0)
                    {
                        if (isUsingThread)
                        {
                            signalThreadShouldExit();
                            event1.signal();
                            stopThread (3000);
                        }

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

            if (isUsingThread)
            {
                signalThreadShouldExit();
                event1.signal();
                stopThread (3000);
            }

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

    int getCurrentBitDepth()
    {
        return currentBitDepth;
    }

    int getOutputLatencyInSamples()
    {
        return outputLatency;
    }

    int getInputLatencyInSamples()
    {
        return inputLatency;
    }

    void start (AudioIODeviceCallback* callback)
    {
        if (callback != 0)
        {
            callback->audioDeviceAboutToStart (currentSampleRate, currentBlockSizeSamples);

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
        return isASIOOpen
                && (isThreadRunning() || ! isUsingThread)
                && (currentCallback != 0);
    }

    const String getLastError()
    {
        return error;
    }

    void setUsingThread (bool b)
    {
        shouldUseThread = b;
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
            close();
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

    void run()
    {
        isThreadReady = true;

        for (;;)
        {
            event1.wait();

            if (threadShouldExit())
                break;

            processBuffer();
        }

        if (bufferIndex < 0)
        {
            log ("! ASIO callback never called");
        }
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
                open (currentChansIn, currentChansOut,
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
    void* windowHandle;
    CLSID classId;
    String error;

    long numInputs, numOutputs;
    StringArray outputChannelNames, inputChannelNames;

    Array<int> sampleRates, bufferSizes;
    long inputLatency, outputLatency;
    long minSize, maxSize, preferredSize, granularity;

    int volatile currentBlockSizeSamples;
    int volatile currentBitDepth;
    double volatile currentSampleRate;
    BitArray currentChansOut, currentChansIn;
    AudioIODeviceCallback* volatile currentCallback;
    CriticalSection callbackLock;

    float* inBuffers[maxASIOChannels];
    float* outBuffers[maxASIOChannels];
    int inputChannelBitDepths[maxASIOChannels];
    int outputChannelBitDepths[maxASIOChannels];
    int inputChannelBytesPerSample[maxASIOChannels];
    int outputChannelBytesPerSample[maxASIOChannels];
    bool inputChannelIsFloat[maxASIOChannels];
    bool outputChannelIsFloat[maxASIOChannels];
    bool inputChannelLittleEndian[maxASIOChannels];
    bool outputChannelLittleEndian[maxASIOChannels];

    WaitableEvent event1;
    float* tempBuffer;
    int volatile bufferIndex, numActiveInputChans, numActiveOutputChans;

    bool isOpen_, isStarted;
    bool isUsingThread, shouldUseThread;
    bool volatile isASIOOpen;
    bool volatile calledback;
    bool volatile littleEndian, postOutput, needToReset, isReSync, isThreadReady;


    //==============================================================================
    static void removeCurrentDriver()
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

        isUsingThread = shouldUseThread;

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
        numInputs = 0;
        numOutputs = 0;
        currentCallback = 0;

        error = String::empty;

        if (getName().isEmpty())
            return error;

        long err = 0;

        if (loadDriver())
        {
            String driverName;

            if ((error = initDriver()).isEmpty())
            {
                numInputs = 0;
                numOutputs = 0;

                if (asioObject != 0
                    && (err = asioObject->getChannels (&numInputs, &numOutputs)) == 0)
                {
                    log (String ((int) numInputs) + T(" in, ") + String ((int) numOutputs) + T(" out"));

                    if ((err = asioObject->getBufferSize (&minSize, &maxSize, &preferredSize, &granularity)) == 0)
                    {
                        // find a list of buffer sizes..
                        log (String ((int) minSize) + T(" ") + String ((int) maxSize) + T(" ") + String ((int)preferredSize) + T(" ") + String ((int)granularity));

                        if (granularity >= 0)
                        {
                            granularity = jmax (1, (int) granularity);

                            for (int i = jmax (minSize, (int) granularity); i < jmin (6400, maxSize); i += granularity)
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
                        for (i = 0; i < jmin (2, numInputs); ++i)
                        {
                            info->isInput = 1;
                            info->channelNum = i;
                            info->buffers[0] = info->buffers[1] = 0;
                            ++info;
                            ++numChans;
                        }

                        const int outputBufferIndex = numChans;

                        for (i = 0; i < jmin (2, numOutputs); ++i)
                        {
                            info->isInput = 0;
                            info->channelNum = i;
                            info->buffers[0] = info->buffers[1] = 0;
                            ++info;
                            ++numChans;
                        }

                        callbacks.bufferSwitch = &bufferSwitchCallback;
                        callbacks.sampleRateDidChange = &sampleRateChangedCallback;
                        callbacks.asioMessage = &asioMessagesCallback;
                        callbacks.bufferSwitchTimeInfo = &bufferSwitchTimeInfoCallback;

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

                        if (numInputs != newInps || numOutputs != newOuts)
                        {
                            numInputs = newInps;
                            numOutputs = newOuts;

                            log (String ((int) numInputs) + T(" in; ")
                                  + String ((int) numOutputs) + T(" out"));
                        }

                        updateSampleRates();

                        ASIOChannelInfo channelInfo;
                        channelInfo.type = 0;

                        for (i = 0; i < numInputs; ++i)
                        {
                            zerostruct (channelInfo);
                            channelInfo.channel = i;
                            channelInfo.isInput = 1;
                            asioObject->getChannelInfo (&channelInfo);

                            inputChannelNames.add (String (channelInfo.name));
                        }

                        for (i = 0; i < numOutputs; ++i)
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

            if (isUsingThread) // if not started, just use processBuffer() to clear the buffers directly
            {
                event1.signal();

                if (postOutput && (! isThreadRunning()) && asioObject != 0)
                    asioObject->outputReady();
            }
            else
            {
                processBuffer();
            }
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
                int n = 0;
                int i;
                for (i = 0; i < numInputs; ++i)
                {
                    float* const dst = inBuffers[i];

                    if (dst != 0)
                    {
                        const char* const src = (const char*) (infos[n].buffers[bi]);

                        if (inputChannelIsFloat[i])
                        {
                            memcpy (dst, src, samps * sizeof (float));
                        }
                        else
                        {
                            jassert (dst == tempBuffer + (samps * n));

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

                        ++n;
                    }
                }

                currentCallback->audioDeviceIOCallback ((const float**) inBuffers,
                                                        numInputs,
                                                        outBuffers,
                                                        numOutputs,
                                                        samps);

                for (i = 0; i < numOutputs; ++i)
                {
                    float* const src = outBuffers[i];

                    if (src != 0)
                    {
                        char* const dst = (char*) (infos[n].buffers[bi]);

                        if (outputChannelIsFloat[i])
                        {
                            memcpy (dst, src, samps * sizeof (float));
                        }
                        else
                        {
                            jassert (src == tempBuffer + (samps * n));

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

                        ++n;
                    }
                }
            }
            else
            {
                int n = 0;
                int i;

                for (i = 0; i < numInputs; ++i)
                    if (inBuffers[i] != 0)
                        ++n;

                for (i = 0; i < numOutputs; ++i)
                {
                    if (outBuffers[i] != 0)
                    {
                        const int bytesPerBuffer = samps * (outputChannelBitDepths[i] >> 3);
                        zeromem (infos[n].buffers[bi], bytesPerBuffer);
                        ++n;
                    }
                }
            }
        }

        if (postOutput)
            asioObject->outputReady();
    }

    //==============================================================================
    static ASIOTime* bufferSwitchTimeInfoCallback (ASIOTime*, long index, long) throw()
    {
        if (currentASIODev != 0)
            currentASIODev->callback (index);

        return 0;
    }

    static void bufferSwitchCallback (long index, long) throw()
    {
        if (currentASIODev != 0)
            currentASIODev->callback (index);
    }

    static long asioMessagesCallback (long selector, long value, void*, double*) throw()
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
            if (currentASIODev != 0)
                currentASIODev->resetRequest();

            return 1;

        case kAsioResyncRequest:
            if (currentASIODev != 0)
                currentASIODev->resyncRequest();

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
                *dest++ = (float) (g * (short) littleEndianShort (src));
                src += srcStrideBytes;
            }
        }
        else
        {
            while (--numSamples >= 0)
            {
                *dest++ = (float) (g * (short) bigEndianShort (src));
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
                *(uint16*) dest = swapIfBigEndian ((uint16) (short) roundDoubleToInt (jlimit (-maxVal, maxVal, maxVal * *src++)));
                dest += dstStrideBytes;
            }
        }
        else
        {
            while (--numSamples >= 0)
            {
                *(uint16*) dest = swapIfLittleEndian ((uint16) (short) roundDoubleToInt (jlimit (-maxVal, maxVal, maxVal * *src++)));
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
                *dest++ = (float) (g * littleEndian24Bit (src));
                src += srcStrideBytes;
            }
        }
        else
        {
            while (--numSamples >= 0)
            {
                *dest++ = (float) (g * bigEndian24Bit (src));
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
                littleEndian24BitToChars ((uint32) roundDoubleToInt (jlimit (-maxVal, maxVal, maxVal * *src++)), dest);
                dest += dstStrideBytes;
            }
        }
        else
        {
            while (--numSamples >= 0)
            {
                bigEndian24BitToChars ((uint32) roundDoubleToInt (jlimit (-maxVal, maxVal, maxVal * *src++)), dest);
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
                *dest++ = (float) (g * (int) littleEndianInt (src));
                src += srcStrideBytes;
            }
        }
        else
        {
            while (--numSamples >= 0)
            {
                *dest++ = (float) (g * (int) bigEndianInt (src));
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
                *(uint32*) dest = swapIfBigEndian ((uint32) roundDoubleToInt (jlimit (-maxVal, maxVal, maxVal * *src++)));
                dest += dstStrideBytes;
            }
        }
        else
        {
            while (--numSamples >= 0)
            {
                *(uint32*) dest = swapIfLittleEndian ((uint32) roundDoubleToInt (jlimit (-maxVal, maxVal, maxVal * *src++)));
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
          classIds (2),
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

    const StringArray getDeviceNames (const bool /*preferInputNames*/) const
    {
        jassert (hasScanned); // need to call scanForDevices() before doing this

        return deviceNames;
    }

    const String getDefaultDeviceName (const bool /*preferInputNames*/) const
    {
        jassert (hasScanned); // need to call scanForDevices() before doing this

        return deviceNames [0];
    }

    AudioIODevice* createDevice (const String& deviceName)
    {
        jassert (hasScanned); // need to call scanForDevices() before doing this

        const int index = deviceNames.indexOf (deviceName);

        if (index >= 0)
        {
            jassert (currentASIODev == 0);  // unfortunately you can't have more than one ASIO device
                                            // open at the same time..

            if (currentASIODev == 0)
                return new ASIOAudioIODevice (deviceName, *(classIds [index]));
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

AudioIODeviceType* juce_createASIOAudioIODeviceType()
{
    return new ASIOAudioIODeviceType();
}


END_JUCE_NAMESPACE

#endif

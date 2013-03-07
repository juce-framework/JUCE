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

#undef WINDOWS

/* The ASIO SDK *should* declare its callback functions as being __cdecl, but different versions seem
   to be pretty random about whether or not they do this. If you hit an error using these functions
   it'll be because you're trying to build using __stdcall, in which case you'd need to either get hold of
   an ASIO SDK which correctly specifies __cdecl, or add the __cdecl keyword to its functions yourself.
*/
#define JUCE_ASIOCALLBACK __cdecl

//==============================================================================
namespace ASIODebugging
{
   #if JUCE_ASIO_DEBUGGING
    #define JUCE_ASIO_LOG(a)            ASIODebugging::logMessage (a)
    #define JUCE_ASIO_LOG_ERROR(a, b)   ASIODebugging::logError ((a), (b))

    static void logMessage (String message)
    {
        message = "ASIO: " + message;
        DBG (message);
        Logger::writeToLog (message);
    }

    static void logError (const String& context, long error)
    {
        const char* err = "Unknown error";

        switch (error)
        {
            case 0:                    return;
            case ASE_NotPresent:       err = "Not Present"; break;
            case ASE_HWMalfunction:    err = "Hardware Malfunction"; break;
            case ASE_InvalidParameter: err = "Invalid Parameter"; break;
            case ASE_InvalidMode:      err = "Invalid Mode"; break;
            case ASE_SPNotAdvancing:   err = "Sample position not advancing"; break;
            case ASE_NoClock:          err = "No Clock"; break;
            case ASE_NoMemory:         err = "Out of memory"; break;
            default:                   break;
        }

        logMessage ("error: " + context + " - " + err);
    }
   #else
    static void dummyLog() {}
    #define JUCE_ASIO_LOG(a)            ASIODebugging::dummyLog()
    #define JUCE_ASIO_LOG_ERROR(a, b)   ASIODebugging::dummyLog()
   #endif
}

//==============================================================================
struct ASIOSampleFormat
{
    ASIOSampleFormat() noexcept {}

    ASIOSampleFormat (const long type) noexcept
        : bitDepth (24),
          littleEndian (true),
          formatIsFloat (false),
          byteStride (4)
    {
        switch (type)
        {
            case ASIOSTInt16MSB:    byteStride = 2; littleEndian = false; bitDepth = 16; break;
            case ASIOSTInt24MSB:    byteStride = 3; littleEndian = false; break;
            case ASIOSTInt32MSB:    bitDepth = 32; littleEndian = false; break;
            case ASIOSTFloat32MSB:  bitDepth = 32; littleEndian = false; formatIsFloat = true; break;
            case ASIOSTFloat64MSB:  bitDepth = 64; byteStride = 8; littleEndian = false; break;
            case ASIOSTInt32MSB16:  bitDepth = 16; littleEndian = false; break;
            case ASIOSTInt32MSB18:  littleEndian = false; break;
            case ASIOSTInt32MSB20:  littleEndian = false; break;
            case ASIOSTInt32MSB24:  littleEndian = false; break;
            case ASIOSTInt16LSB:    byteStride = 2; bitDepth = 16; break;
            case ASIOSTInt24LSB:    byteStride = 3; break;
            case ASIOSTInt32LSB:    bitDepth = 32; break;
            case ASIOSTFloat32LSB:  bitDepth = 32; formatIsFloat = true; break;
            case ASIOSTFloat64LSB:  bitDepth = 64; byteStride = 8; break;
            case ASIOSTInt32LSB16:  bitDepth = 16; break;
            case ASIOSTInt32LSB18:  break; // (unhandled)
            case ASIOSTInt32LSB20:  break; // (unhandled)
            case ASIOSTInt32LSB24:  break;

            case ASIOSTDSDInt8LSB1: break; // (unhandled)
            case ASIOSTDSDInt8MSB1: break; // (unhandled)
            case ASIOSTDSDInt8NER8: break; // (unhandled)

            default:
                jassertfalse;  // (not a valid format code..)
                break;
        }
    }

    void convertToFloat (const void* const src, float* const dst, const int samps) const noexcept
    {
        if (formatIsFloat)
        {
            memcpy (dst, src, samps * sizeof (float));
        }
        else
        {
            switch (bitDepth)
            {
                case 16: convertInt16ToFloat (static_cast <const char*> (src), dst, byteStride, samps, littleEndian); break;
                case 24: convertInt24ToFloat (static_cast <const char*> (src), dst, byteStride, samps, littleEndian); break;
                case 32: convertInt32ToFloat (static_cast <const char*> (src), dst, byteStride, samps, littleEndian); break;
                default: jassertfalse; break;
            }
        }
    }

    void convertFromFloat (const float* const src, void* const dst, const int samps) const noexcept
    {
        if (formatIsFloat)
        {
            memcpy (dst, src, samps * sizeof (float));
        }
        else
        {
            switch (bitDepth)
            {
                case 16: convertFloatToInt16 (src, static_cast <char*> (dst), byteStride, samps, littleEndian); break;
                case 24: convertFloatToInt24 (src, static_cast <char*> (dst), byteStride, samps, littleEndian); break;
                case 32: convertFloatToInt32 (src, static_cast <char*> (dst), byteStride, samps, littleEndian); break;
                default: jassertfalse; break;
            }
        }
    }

    void clear (void* dst, const int numSamps) noexcept
    {
        if (dst != nullptr)
            zeromem (dst, numSamps * byteStride);
    }

    int bitDepth, byteStride;
    bool formatIsFloat, littleEndian;

private:
    static void convertInt16ToFloat (const char* src, float* dest, const int srcStrideBytes,
                                     int numSamples, const bool littleEndian) noexcept
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

    static void convertFloatToInt16 (const float* src, char* dest, const int dstStrideBytes,
                                     int numSamples, const bool littleEndian) noexcept
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

    static void convertInt24ToFloat (const char* src, float* dest, const int srcStrideBytes,
                                     int numSamples, const bool littleEndian) noexcept
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

    static void convertFloatToInt24 (const float* src, char* dest, const int dstStrideBytes,
                                     int numSamples, const bool littleEndian) noexcept
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

    static void convertInt32ToFloat (const char* src, float* dest, const int srcStrideBytes,
                                     int numSamples, const bool littleEndian) noexcept
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

    static void convertFloatToInt32 (const float* src, char* dest, const int dstStrideBytes,
                                     int numSamples, const bool littleEndian) noexcept
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
};

//==============================================================================
class ASIOAudioIODevice;
static ASIOAudioIODevice* volatile currentASIODev[3] = { 0 };

extern HWND juce_messageWindowHandle;

//==============================================================================
class ASIOAudioIODevice  : public AudioIODevice,
                           private Timer
{
public:
    ASIOAudioIODevice (const String& devName, const CLSID clsID, const int slotNumber,
                       const String& dllForDirectLoading)
       : AudioIODevice (devName, "ASIO"),
         asioObject (nullptr),
         classId (clsID),
         optionalDllForDirectLoading (dllForDirectLoading),
         currentBitDepth (16),
         currentSampleRate (0),
         deviceIsOpen (false),
         isStarted (false),
         buffersCreated (false),
         postOutput (true),
         insideControlPanelModalLoop (false),
         shouldUsePreferredSize (false)
    {
        name = devName;
        inBuffers.calloc (4);
        outBuffers.calloc (4);

        jassert (currentASIODev [slotNumber] == nullptr);
        currentASIODev [slotNumber] = this;

        openDevice();
    }

    ~ASIOAudioIODevice()
    {
        for (int i = 0; i < numElementsInArray (currentASIODev); ++i)
            if (currentASIODev[i] == this)
                currentASIODev[i] = nullptr;

        close();
        JUCE_ASIO_LOG ("closed");
        removeCurrentDriver();
    }

    void updateSampleRates()
    {
        // find a list of sample rates..
        const int possibleSampleRates[] = { 44100, 48000, 88200, 96000, 176400, 192000 };
        sampleRates.clear();

        if (asioObject != nullptr)
        {
            for (int index = 0; index < numElementsInArray (possibleSampleRates); ++index)
            {
                const long err = asioObject->canSampleRate ((double) possibleSampleRates[index]);
                JUCE_ASIO_LOG_ERROR ("canSampleRate " + String (possibleSampleRates[index]), err);

                if (err == 0)
                {
                    sampleRates.add (possibleSampleRates[index]);
                    JUCE_ASIO_LOG ("rate: " + String (possibleSampleRates[index]));
                }
            }

            if (sampleRates.size() == 0)
            {
                double cr = 0;
                const long err = asioObject->getSampleRate (&cr);
                JUCE_ASIO_LOG ("No sample rates supported - current rate: " + String ((int) cr));
                JUCE_ASIO_LOG_ERROR ("getSampleRate", err);

                if (err == 0)
                    sampleRates.add ((int) cr);
            }
        }
    }

    StringArray getOutputChannelNames()         { return outputChannelNames; }
    StringArray getInputChannelNames()          { return inputChannelNames; }

    int getNumSampleRates()                     { return sampleRates.size(); }
    double getSampleRate (int index)            { return sampleRates [index]; }

    int getNumBufferSizesAvailable()            { return bufferSizes.size(); }
    int getBufferSizeSamples (int index)        { return bufferSizes [index]; }
    int getDefaultBufferSize()                  { return preferredSize; }

    String open (const BigInteger& inputChannels,
                 const BigInteger& outputChannels,
                 double sr,
                 int bufferSizeSamples)
    {
        close();
        currentCallback = nullptr;

        if (bufferSizeSamples <= 0)
            shouldUsePreferredSize = true;

        if (asioObject == nullptr || ! isASIOOpen)
        {
            JUCE_ASIO_LOG ("Warning: device not open");
            const String err (openDevice());

            if (asioObject == nullptr || ! isASIOOpen)
                return err;
        }

        isStarted = false;
        bufferIndex = -1;
        long err = 0;
        long newPreferredSize = 0;
        minSize = 0;
        maxSize = 0;
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
                                   || getName().containsIgnoreCase ("Digidesign");

        if (shouldUsePreferredSize)
        {
            JUCE_ASIO_LOG ("Using preferred size for buffer..");

            if ((err = asioObject->getBufferSize (&minSize, &maxSize, &preferredSize, &granularity)) == 0)
            {
                bufferSizeSamples = preferredSize;
            }
            else
            {
                bufferSizeSamples = 1024;
                JUCE_ASIO_LOG_ERROR ("getBufferSize1", err);
            }

            shouldUsePreferredSize = false;
        }

        int sampleRate = roundDoubleToInt (sr);
        currentSampleRate = sampleRate;
        currentBlockSizeSamples = bufferSizeSamples;
        currentChansOut.clear();
        currentChansIn.clear();
        inBuffers.clear (totalNumInputChans + 1);
        outBuffers.clear (totalNumOutputChans + 1);

        updateSampleRates();

        if (sampleRate == 0 || (sampleRates.size() > 0 && ! sampleRates.contains (sampleRate)))
            sampleRate = sampleRates[0];

        jassert (sampleRate != 0);
        if (sampleRate == 0)
            sampleRate = 44100;

        ASIOClockSource clocks[32] = { 0 };
        long numSources = numElementsInArray (clocks);
        asioObject->getClockSources (clocks, &numSources);
        bool isSourceSet = false;

        // careful not to remove this loop because it does more than just logging!
        for (int i = 0; i < numSources; ++i)
        {
            String s ("clock: ");
            s += clocks[i].name;

            if (clocks[i].isCurrentSource)
            {
                isSourceSet = true;
                s << " (cur)";
            }

            JUCE_ASIO_LOG (s);
        }

        if (numSources > 1 && ! isSourceSet)
        {
            JUCE_ASIO_LOG ("setting clock source");
            err = asioObject->setClockSource (clocks[0].index);
            JUCE_ASIO_LOG_ERROR ("setClockSource1", err);
            Thread::sleep (20);
        }
        else
        {
            if (numSources == 0)
                JUCE_ASIO_LOG ("no clock sources!");
        }

        {
            double cr = 0;
            err = asioObject->getSampleRate (&cr);
            JUCE_ASIO_LOG_ERROR ("getSampleRate", err);
            currentSampleRate = cr;
        }

        error = String::empty;
        err = 0;
        buffersCreated = false;

        if (currentSampleRate != sampleRate)
        {
            JUCE_ASIO_LOG ("rate change: " + String (currentSampleRate) + " to " + String (sampleRate));
            err = asioObject->setSampleRate (sampleRate);

            if (err == ASE_NoClock && numSources > 0)
            {
                JUCE_ASIO_LOG ("trying to set a clock source..");
                Thread::sleep (10);
                err = asioObject->setClockSource (clocks[0].index);
                JUCE_ASIO_LOG_ERROR ("setClockSource2", err);

                Thread::sleep (10);
                err = asioObject->setSampleRate (sampleRate);
            }

            if (err == 0)
                currentSampleRate = sampleRate;

            // on fail, ignore the attempt to change rate, and run with the current one..
        }

        if (needToReset)
        {
            JUCE_ASIO_LOG (" Resetting");
            removeCurrentDriver();

            loadDriver();
            const String error (initDriver());

            if (error.isNotEmpty())
                JUCE_ASIO_LOG ("ASIOInit: " + error);

            needToReset = false;
        }

        const int totalBuffers = resetBuffers (inputChannels, outputChannels);

        setCallbackFunctions();

        JUCE_ASIO_LOG ("disposing buffers");
        err = asioObject->disposeBuffers();

        JUCE_ASIO_LOG ("creating buffers: " + String (totalBuffers) + ", " + String (currentBlockSizeSamples));
        err = asioObject->createBuffers (bufferInfos,
                                         totalBuffers,
                                         currentBlockSizeSamples,
                                         &callbacks);

        if (err != 0)
        {
            currentBlockSizeSamples = preferredSize;
            JUCE_ASIO_LOG_ERROR ("create buffers 2", err);

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

            for (int i = 0; i < (int) totalNumInputChans; ++i)
            {
                if (inputChannels[i])
                {
                    inBuffers[n] = tempBuffer + (currentBlockSizeSamples * n);

                    ASIOChannelInfo channelInfo = { 0 };
                    channelInfo.channel = i;
                    channelInfo.isInput = 1;
                    asioObject->getChannelInfo (&channelInfo);

                    types.addIfNotAlreadyThere (channelInfo.type);
                    inputFormat[n] = ASIOSampleFormat (channelInfo.type);

                    currentBitDepth = jmax (currentBitDepth, inputFormat[n].bitDepth);
                    ++n;
                }
            }

            jassert (numActiveInputChans == n);
            n = 0;

            for (int i = 0; i < (int) totalNumOutputChans; ++i)
            {
                if (outputChannels[i])
                {
                    outBuffers[n] = tempBuffer + (currentBlockSizeSamples * (numActiveInputChans + n));

                    ASIOChannelInfo channelInfo = { 0 };
                    channelInfo.channel = i;
                    channelInfo.isInput = 0;
                    asioObject->getChannelInfo (&channelInfo);

                    types.addIfNotAlreadyThere (channelInfo.type);
                    outputFormat[n] = ASIOSampleFormat (channelInfo.type);

                    currentBitDepth = jmax (currentBitDepth, outputFormat[n].bitDepth);
                    ++n;
                }
            }

            jassert (numActiveOutputChans == n);

            for (int i = types.size(); --i >= 0;)
                JUCE_ASIO_LOG ("channel format: " + String (types[i]));

            jassert (n <= totalBuffers);

            for (int i = 0; i < numActiveOutputChans; ++i)
            {
                outputFormat[i].clear (bufferInfos [numActiveInputChans + i].buffers[0], currentBlockSizeSamples);
                outputFormat[i].clear (bufferInfos [numActiveInputChans + i].buffers[1], currentBlockSizeSamples);
            }

            inputLatency = outputLatency = 0;

            if (asioObject->getLatencies (&inputLatency, &outputLatency) != 0)
                JUCE_ASIO_LOG ("no latencies");
            else
                JUCE_ASIO_LOG ("latencies: " + String ((int) outputLatency) + ", " + String ((int) inputLatency));

            deviceIsOpen = true;

            JUCE_ASIO_LOG ("starting");
            calledback = false;
            err = asioObject->start();

            if (err != 0)
            {
                deviceIsOpen = false;
                JUCE_ASIO_LOG ("stop on failure");
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
                    JUCE_ASIO_LOG ("no callbacks - stopping..");
                    asioObject->stop();
                }
            }
        }
        else
        {
            error = "Can't create i/o buffers";
        }

        if (error.isNotEmpty())
        {
            JUCE_ASIO_LOG_ERROR (error, err);
            disposeBuffers();

            Thread::sleep (20);
            isStarted = false;
            deviceIsOpen = false;

            const String errorCopy (error);
            close(); // (this resets the error string)
            error = errorCopy;
        }

        needToReset = false;
        return error;
    }

    void close()
    {
        error = String::empty;
        stopTimer();
        stop();

        if (isASIOOpen && deviceIsOpen)
        {
            const ScopedLock sl (callbackLock);

            deviceIsOpen = false;
            isStarted = false;
            needToReset = false;

            JUCE_ASIO_LOG ("stopping");

            if (asioObject != nullptr)
            {
                Thread::sleep (20);
                asioObject->stop();
                Thread::sleep (10);
                disposeBuffers();
            }

            Thread::sleep (10);
        }
    }

    bool isOpen()                       { return deviceIsOpen || insideControlPanelModalLoop; }
    bool isPlaying()                    { return isASIOOpen && (currentCallback != nullptr); }

    int getCurrentBufferSizeSamples()   { return currentBlockSizeSamples; }
    double getCurrentSampleRate()       { return currentSampleRate; }
    int getCurrentBitDepth()            { return currentBitDepth; }

    BigInteger getActiveOutputChannels() const    { return currentChansOut; }
    BigInteger getActiveInputChannels() const     { return currentChansIn; }

    int getOutputLatencyInSamples()     { return outputLatency + currentBlockSizeSamples / 4; }
    int getInputLatencyInSamples()      { return inputLatency + currentBlockSizeSamples / 4; }

    void start (AudioIODeviceCallback* callback)
    {
        if (callback != nullptr)
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
            currentCallback = nullptr;
        }

        if (lastCallback != nullptr)
            lastCallback->audioDeviceStopped();
    }

    String getLastError()           { return error; }
    bool hasControlPanel() const    { return true; }

    bool showControlPanel()
    {
        JUCE_ASIO_LOG ("showing control panel");

        bool done = false;

        JUCE_TRY
        {
            // are there are devices that need to be closed before showing their control panel?
            // close();
            insideControlPanelModalLoop = true;

            const uint32 started = Time::getMillisecondCounter();

            if (asioObject != nullptr)
            {
                asioObject->controlPanel();

                const int spent = (int) Time::getMillisecondCounter() - (int) started;

                JUCE_ASIO_LOG ("spent: " + String (spent));

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

    void resetRequest() noexcept
    {
        startTimer (500);
    }

    void timerCallback()
    {
        if (! insideControlPanelModalLoop)
        {
            stopTimer();

            // used to cause a reset
            JUCE_ASIO_LOG ("restart request!");

            if (deviceIsOpen)
            {
                AudioIODeviceCallback* const oldCallback = currentCallback;

                close();

                needToReset = true;
                open (BigInteger (currentChansIn), BigInteger (currentChansOut),
                      currentSampleRate, currentBlockSizeSamples);

                reloadChannelNames();

                if (oldCallback != nullptr)
                    start (oldCallback);
            }
        }
        else
        {
            startTimer (100);
        }
    }

private:
    //==============================================================================
    IASIO* volatile asioObject;
    ASIOCallbacks callbacks;

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
    BigInteger currentChansOut, currentChansIn;
    AudioIODeviceCallback* volatile currentCallback;
    CriticalSection callbackLock;

    HeapBlock<ASIOBufferInfo> bufferInfos;
    HeapBlock<float*> inBuffers, outBuffers;
    HeapBlock<ASIOSampleFormat> inputFormat, outputFormat;

    WaitableEvent event1;
    HeapBlock <float> tempBuffer;
    int volatile bufferIndex, numActiveInputChans, numActiveOutputChans;

    bool deviceIsOpen, isStarted, buffersCreated;
    bool volatile isASIOOpen;
    bool volatile calledback;
    bool volatile littleEndian, postOutput, needToReset;
    bool volatile insideControlPanelModalLoop;
    bool volatile shouldUsePreferredSize;

    //==============================================================================
    static String convertASIOString (char* const text, int length)
    {
        if (CharPointer_UTF8::isValidString (text, length))
            return String::fromUTF8 (text, length);

        WCHAR wideVersion [64] = { 0 };
        MultiByteToWideChar (CP_ACP, 0, text, length, wideVersion, numElementsInArray (wideVersion));
        return wideVersion;
    }

    String getChannelName (int index, bool isInput) const
    {
        ASIOChannelInfo channelInfo = { 0 };
        channelInfo.channel = index;
        channelInfo.isInput = isInput ? 1 : 0;
        asioObject->getChannelInfo (&channelInfo);

        return convertASIOString (channelInfo.name, sizeof (channelInfo.name));
    }

    void reloadChannelNames()
    {
        long totalNumInputChans = 0;
        long totalNumOutputChans = 0;

        if (asioObject != nullptr
             && asioObject->getChannels (&totalNumInputChans, &totalNumOutputChans) == ASE_OK)
        {
            inputChannelNames.clear();
            outputChannelNames.clear();

            for (int i = 0; i < totalNumInputChans; ++i)
                inputChannelNames.add (getChannelName (i, true));

            for (int i = 0; i < totalNumOutputChans; ++i)
                outputChannelNames.add (getChannelName (i, false));

            outputChannelNames.trim();
            inputChannelNames.trim();
            outputChannelNames.appendNumbersToDuplicates (false, true);
            inputChannelNames.appendNumbersToDuplicates (false, true);
        }
    }

    int resetBuffers (const BigInteger& inputChannels,
                      const BigInteger& outputChannels)
    {
        numActiveInputChans = 0;
        numActiveOutputChans = 0;

        ASIOBufferInfo* info = bufferInfos;
        for (int i = 0; i < totalNumInputChans; ++i)
        {
            if (inputChannels[i])
            {
                currentChansIn.setBit (i);
                info->isInput = 1;
                info->channelNum = i;
                info->buffers[0] = info->buffers[1] = nullptr;
                ++info;
                ++numActiveInputChans;
            }
        }

        for (int i = 0; i < totalNumOutputChans; ++i)
        {
            if (outputChannels[i])
            {
                currentChansOut.setBit (i);
                info->isInput = 0;
                info->channelNum = i;
                info->buffers[0] = info->buffers[1] = nullptr;
                ++info;
                ++numActiveOutputChans;
            }
        }

        return numActiveInputChans + numActiveOutputChans;
    }

    void removeCurrentDriver()
    {
        if (asioObject != nullptr)
        {
            asioObject->Release();
            asioObject = nullptr;
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
                HMODULE h = LoadLibrary (optionalDllForDirectLoading.toWideCharPointer());

                if (h != 0)
                {
                    typedef HRESULT (CALLBACK* DllGetClassObjectFunc) (REFCLSID clsid, REFIID iid, LPVOID* ppv);
                    DllGetClassObjectFunc dllGetClassObject = (DllGetClassObjectFunc) GetProcAddress (h, "DllGetClassObject");

                    if (dllGetClassObject != 0)
                    {
                        IClassFactory* classFactory = nullptr;
                        HRESULT hr = dllGetClassObject (classId, IID_IClassFactory, (void**) &classFactory);

                        if (classFactory != nullptr)
                        {
                            hr = classFactory->CreateInstance (0, classId, (void**) &asioObject);
                            classFactory->Release();
                        }

                        return asioObject != nullptr;
                    }
                }
            }
        }
        JUCE_CATCH_ALL

        asioObject = nullptr;
        return false;
    }

    String getLastDriverError() const
    {
        jassert (asioObject != nullptr);
        char buffer [512] = { 0 };
        asioObject->getErrorMessage (buffer);
        return String (buffer, sizeof (buffer) - 1);
    }

    String initDriver()
    {
        if (asioObject == nullptr)
            return "No Driver";

        const bool initOk = !! asioObject->init (juce_messageWindowHandle);
        String driverError;

        // Get error message if init() failed, or if it's a buggy Denon driver,
        // which returns true from init() even when it fails.
        if ((! initOk) || getName().containsIgnoreCase ("denon dj"))
            driverError = getLastDriverError();

        if ((! initOk) && driverError.isEmpty())
            driverError = "Driver failed to initialise";

        if (driverError.isEmpty())
        {
            char buffer [512];
            asioObject->getDriverName (buffer); // just in case any flimsy drivers expect this to be called..
        }

        return driverError;
    }

    String openDevice()
    {
        // open the device and get its info..
        JUCE_ASIO_LOG ("opening device: " + getName());

        needToReset = false;
        outputChannelNames.clear();
        inputChannelNames.clear();
        bufferSizes.clear();
        sampleRates.clear();
        isASIOOpen = false;
        deviceIsOpen = false;
        totalNumInputChans = 0;
        totalNumOutputChans = 0;
        numActiveInputChans = 0;
        numActiveOutputChans = 0;
        currentCallback = nullptr;

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

                if (asioObject != nullptr
                     && (err = asioObject->getChannels (&totalNumInputChans, &totalNumOutputChans)) == 0)
                {
                    JUCE_ASIO_LOG (String ((int) totalNumInputChans) + " in, " + String ((int) totalNumOutputChans) + " out");

                    const int chansToAllocate = totalNumInputChans + totalNumOutputChans + 4;
                    bufferInfos.calloc (chansToAllocate);
                    inBuffers.calloc (chansToAllocate);
                    outBuffers.calloc (chansToAllocate);
                    inputFormat.calloc (chansToAllocate);
                    outputFormat.calloc (chansToAllocate);

                    if ((err = asioObject->getBufferSize (&minSize, &maxSize, &preferredSize, &granularity)) == 0)
                    {
                        // find a list of buffer sizes..
                        JUCE_ASIO_LOG (String ((int) minSize) + " " + String ((int) maxSize) + " " + String ((int) preferredSize) + " " + String ((int) granularity));

                        if (granularity >= 0)
                        {
                            granularity = jmax (16, (int) granularity);

                            for (int i = jmax ((int) (minSize + 15) & ~15, (int) granularity); i < jmin (6400, (int) maxSize); i += granularity)
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
                            JUCE_ASIO_LOG ("setting sample rate");
                            err = asioObject->setSampleRate (44100.0);
                            JUCE_ASIO_LOG_ERROR ("setting sample rate", err);

                            asioObject->getSampleRate (&currentRate);
                        }

                        currentSampleRate = currentRate;

                        postOutput = (asioObject->outputReady() == 0);
                        if (postOutput)
                            JUCE_ASIO_LOG ("outputReady true");

                        updateSampleRates();

                        // ..because cubase does it at this point
                        inputLatency = outputLatency = 0;
                        if (asioObject->getLatencies (&inputLatency, &outputLatency) != 0)
                            JUCE_ASIO_LOG ("no latencies");

                        JUCE_ASIO_LOG ("latencies: " + String ((int) inputLatency) + ", " + String ((int) outputLatency));

                        // create some dummy buffers now.. because cubase does..
                        numActiveInputChans = 0;
                        numActiveOutputChans = 0;

                        ASIOBufferInfo* info = bufferInfos;
                        int numChans = 0;

                        for (int i = 0; i < jmin (2, (int) totalNumInputChans); ++i)
                        {
                            info->isInput = 1;
                            info->channelNum = i;
                            info->buffers[0] = info->buffers[1] = nullptr;
                            ++info;
                            ++numChans;
                        }

                        const int outputBufferIndex = numChans;

                        for (int i = 0; i < jmin (2, (int) totalNumOutputChans); ++i)
                        {
                            info->isInput = 0;
                            info->channelNum = i;
                            info->buffers[0] = info->buffers[1] = nullptr;
                            ++info;
                            ++numChans;
                        }

                        setCallbackFunctions();

                        JUCE_ASIO_LOG ("creating buffers (dummy): " + String (numChans) + ", " + String ((int) preferredSize));

                        if (preferredSize > 0)
                        {
                            err = asioObject->createBuffers (bufferInfos, numChans, preferredSize, &callbacks);
                            JUCE_ASIO_LOG_ERROR ("dummy buffers", err);
                        }

                        long newInps = 0, newOuts = 0;
                        asioObject->getChannels (&newInps, &newOuts);

                        if (totalNumInputChans != newInps || totalNumOutputChans != newOuts)
                        {
                            totalNumInputChans = newInps;
                            totalNumOutputChans = newOuts;

                            JUCE_ASIO_LOG (String ((int) totalNumInputChans) + " in; " + String ((int) totalNumOutputChans) + " out");
                        }

                        updateSampleRates();

                        reloadChannelNames();

                        for (int i = 0; i < totalNumOutputChans; ++i)
                        {
                            ASIOChannelInfo channelInfo = { 0 };
                            channelInfo.channel = i;
                            channelInfo.isInput = 0;
                            asioObject->getChannelInfo (&channelInfo);

                            outputFormat[i] = ASIOSampleFormat (channelInfo.type);

                            if (i < 2)
                            {
                                // clear the channels that are used with the dummy stuff
                                outputFormat[i].clear (bufferInfos [outputBufferIndex + i].buffers[0], preferredSize);
                                outputFormat[i].clear (bufferInfos [outputBufferIndex + i].buffers[1], preferredSize);
                            }
                        }

                        // start and stop because cubase does it..
                        asioObject->getLatencies (&inputLatency, &outputLatency);

                        err = asioObject->start();
                        // ignore an error here, as it might start later after setting other stuff up
                        JUCE_ASIO_LOG_ERROR ("start", err);

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
            JUCE_ASIO_LOG_ERROR (error, err);
            disposeBuffers();
            removeCurrentDriver();
            isASIOOpen = false;
        }
        else
        {
            isASIOOpen = true;
            JUCE_ASIO_LOG ("device open");
        }

        deviceIsOpen = false;
        needToReset = false;
        return error;
    }

    void disposeBuffers()
    {
        if (asioObject != nullptr && buffersCreated)
        {
            buffersCreated = false;
            asioObject->disposeBuffers();
        }
    }

    //==============================================================================
    void JUCE_ASIOCALLBACK callback (const long index)
    {
        if (isStarted)
        {
            bufferIndex = index;
            processBuffer();
        }
        else
        {
            if (postOutput && (asioObject != nullptr))
                asioObject->outputReady();
        }

        calledback = true;
    }

    void processBuffer()
    {
        const ASIOBufferInfo* const infos = bufferInfos;
        const int bi = bufferIndex;

        const ScopedLock sl (callbackLock);

        if (bi >= 0)
        {
            const int samps = currentBlockSizeSamples;

            if (currentCallback != nullptr)
            {
                for (int i = 0; i < numActiveInputChans; ++i)
                {
                    jassert (inBuffers[i] != nullptr);
                    inputFormat[i].convertToFloat (infos[i].buffers[bi], inBuffers[i], samps);
                }

                currentCallback->audioDeviceIOCallback (const_cast <const float**> (inBuffers.getData()), numActiveInputChans,
                                                        outBuffers, numActiveOutputChans, samps);

                for (int i = 0; i < numActiveOutputChans; ++i)
                {
                    jassert (outBuffers[i] != nullptr);
                    outputFormat[i].convertFromFloat (outBuffers[i], infos [numActiveInputChans + i].buffers[bi], samps);
                }
            }
            else
            {
                for (int i = 0; i < numActiveOutputChans; ++i)
                     outputFormat[i].clear (infos[numActiveInputChans + i].buffers[bi], samps);
            }
        }

        if (postOutput)
            asioObject->outputReady();
    }

    //==============================================================================
    template <int deviceIndex>
    struct ASIOCallbackFunctions
    {
        static ASIOTime* JUCE_ASIOCALLBACK bufferSwitchTimeInfoCallback (ASIOTime*, long index, long)
        {
            if (currentASIODev[deviceIndex] != nullptr)
                currentASIODev[deviceIndex]->callback (index);

            return nullptr;
        }

        static void JUCE_ASIOCALLBACK bufferSwitchCallback (long index, long)
        {
            if (currentASIODev[deviceIndex] != nullptr)
                currentASIODev[deviceIndex]->callback (index);
        }

        static long JUCE_ASIOCALLBACK asioMessagesCallback (long selector, long value, void*, double*)
        {
            return ASIOAudioIODevice::asioMessagesCallback (selector, value, deviceIndex);
        }

        static void setCallbacks (ASIOCallbacks& callbacks)
        {
            callbacks.bufferSwitch = &bufferSwitchCallback;
            callbacks.asioMessage = &asioMessagesCallback;
            callbacks.bufferSwitchTimeInfo = &bufferSwitchTimeInfoCallback;
        }
    };

    void setCallbackFunctions()
    {
        callbacks.sampleRateDidChange = &sampleRateChangedCallback;

        if      (currentASIODev[0] == this)  ASIOCallbackFunctions<0>::setCallbacks (callbacks);
        else if (currentASIODev[1] == this)  ASIOCallbackFunctions<1>::setCallbacks (callbacks);
        else if (currentASIODev[2] == this)  ASIOCallbackFunctions<2>::setCallbacks (callbacks);
        else                                 jassertfalse;
    }

    //==============================================================================
    static long asioMessagesCallback (long selector, long value, const int deviceIndex)
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
        case kAsioResetRequest:
        case kAsioResyncRequest:
            if (currentASIODev[deviceIndex] != nullptr)
                currentASIODev[deviceIndex]->resetRequest();

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

    static void JUCE_ASIOCALLBACK sampleRateChangedCallback (ASIOSampleRate)
    {
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ASIOAudioIODevice)
};

//==============================================================================
class ASIOAudioIODeviceType  : public AudioIODeviceType
{
public:
    ASIOAudioIODeviceType()
        : AudioIODeviceType ("ASIO"),
          hasScanned (false)
    {
        CoInitialize (0);
    }

    //==============================================================================
    void scanForDevices()
    {
        hasScanned = true;

        deviceNames.clear();
        classIds.clear();

        HKEY hk = 0;
        int index = 0;

        if (RegOpenKey (HKEY_LOCAL_MACHINE, _T("software\\asio"), &hk) == ERROR_SUCCESS)
        {
            TCHAR name [256];

            while (RegEnumKey (hk, index++, name, numElementsInArray (name)) == ERROR_SUCCESS)
                addDriverInfo (name, hk);

            RegCloseKey (hk);
        }
    }

    StringArray getDeviceNames (bool /*wantInputNames*/) const
    {
        jassert (hasScanned); // need to call scanForDevices() before doing this

        return deviceNames;
    }

    int getDefaultDeviceIndex (bool) const
    {
        jassert (hasScanned); // need to call scanForDevices() before doing this

        for (int i = deviceNames.size(); --i >= 0;)
            if (deviceNames[i].containsIgnoreCase ("asio4all"))
                return i; // asio4all is a safe choice for a default..

       #if JUCE_DEBUG
        if (deviceNames.size() > 1 && deviceNames[0].containsIgnoreCase ("digidesign"))
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

    int getIndexOfDevice (AudioIODevice* d, bool /*asInput*/) const
    {
        jassert (hasScanned); // need to call scanForDevices() before doing this

        return d == nullptr ? -1 : deviceNames.indexOf (d->getName());
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

        return nullptr;
    }

    //==============================================================================
private:
    StringArray deviceNames;
    OwnedArray <CLSID> classIds;

    bool hasScanned;

    //==============================================================================
    static bool checkClassIsOk (const String& classId)
    {
        HKEY hk = 0;
        bool ok = false;

        if (RegOpenKey (HKEY_CLASSES_ROOT, _T("clsid"), &hk) == ERROR_SUCCESS)
        {
            int index = 0;
            TCHAR name [512];

            while (RegEnumKey (hk, index++, name, numElementsInArray (name)) == ERROR_SUCCESS)
            {
                if (classId.equalsIgnoreCase (name))
                {
                    HKEY subKey, pathKey;

                    if (RegOpenKeyEx (hk, name, 0, KEY_READ, &subKey) == ERROR_SUCCESS)
                    {
                        if (RegOpenKeyEx (subKey, _T("InprocServer32"), 0, KEY_READ, &pathKey) == ERROR_SUCCESS)
                        {
                            TCHAR pathName [1024] = { 0 };
                            DWORD dtype = REG_SZ;
                            DWORD dsize = sizeof (pathName);

                            if (RegQueryValueEx (pathKey, 0, 0, &dtype, (LPBYTE) pathName, &dsize) == ERROR_SUCCESS)
                                // In older code, this used to check for the existance of the file, but there are situations
                                // where our process doesn't have access to it, but where the driver still loads ok..
                                ok = (pathName[0] != 0);

                            RegCloseKey (pathKey);
                        }

                        RegCloseKey (subKey);
                    }

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

        if (RegOpenKeyEx (hk, keyName.toWideCharPointer(), 0, KEY_READ, &subKey) == ERROR_SUCCESS)
        {
            TCHAR buf [256] = { 0 };
            DWORD dtype = REG_SZ;
            DWORD dsize = sizeof (buf);

            if (RegQueryValueEx (subKey, _T("clsid"), 0, &dtype, (LPBYTE) buf, &dsize) == ERROR_SUCCESS)
            {
                if (dsize > 0 && checkClassIsOk (buf))
                {
                    CLSID classId;
                    if (CLSIDFromString ((LPOLESTR) buf, &classId) == S_OK)
                    {
                        dtype = REG_SZ;
                        dsize = sizeof (buf);
                        String deviceName;

                        if (RegQueryValueEx (subKey, _T("description"), 0, &dtype, (LPBYTE) buf, &dsize) == ERROR_SUCCESS)
                            deviceName = buf;
                        else
                            deviceName = keyName;

                        JUCE_ASIO_LOG ("found " + deviceName);
                        deviceNames.add (deviceName);
                        classIds.add (new CLSID (classId));
                    }
                }

                RegCloseKey (subKey);
            }
        }
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ASIOAudioIODeviceType)
};

AudioIODeviceType* AudioIODeviceType::createAudioIODeviceType_ASIO()
{
    return new ASIOAudioIODeviceType();
}

AudioIODevice* juce_createASIOAudioIODeviceForGUID (const String& name, void* guid,
                                                    const String& optionalDllForDirectLoading)
{
    const int freeSlot = ASIOAudioIODeviceType::findFreeSlot();

    if (freeSlot < 0)
        return nullptr;

    return new ASIOAudioIODevice (name, *(CLSID*) guid, freeSlot, optionalDllForDirectLoading);
}

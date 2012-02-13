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
#undef log

// #define ASIO_DEBUGGING 1

#if ASIO_DEBUGGING
 #define log(a) { Logger::writeToLog (a); DBG (a) }
#else
 #define log(a) {}
#endif

/* The ASIO SDK *should* declare its callback functions as being __cdecl, but different versions seem
   to be pretty random about whether or not they do this. If you hit an error using these functions
   it'll be because you're trying to build using __stdcall, in which case you'd need to either get hold of
   an ASIO SDK which correctly specifies __cdecl, or add the __cdecl keyword to its functions yourself.
*/
#define JUCE_ASIOCALLBACK __cdecl

//==============================================================================
namespace ASIODebugging
{
  #if ASIO_DEBUGGING
    static void log (const String& context, long error)
    {
        const char* err = "unknown error";

        if (error == ASE_NotPresent)            err = "Not Present";
        else if (error == ASE_HWMalfunction)    err = "Hardware Malfunction";
        else if (error == ASE_InvalidParameter) err = "Invalid Parameter";
        else if (error == ASE_InvalidMode)      err = "Invalid Mode";
        else if (error == ASE_SPNotAdvancing)   err = "Sample position not advancing";
        else if (error == ASE_NoClock)          err = "No Clock";
        else if (error == ASE_NoMemory)         err = "Out of memory";

        log ("!!error: " + context + " - " + err);
    }

    #define logError(a, b) ASIODebugging::log ((a), (b))
  #else
    #define logError(a, b) {}
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
    ASIOAudioIODevice (const String& name_, const CLSID classId_, const int slotNumber,
                       const String& optionalDllForDirectLoading_)
       : AudioIODevice (name_, "ASIO"),
         asioObject (nullptr),
         classId (classId_),
         optionalDllForDirectLoading (optionalDllForDirectLoading_),
         currentBitDepth (16),
         currentSampleRate (0),
         deviceIsOpen (false),
         isStarted (false),
         buffersCreated (false),
         postOutput (true),
         insideControlPanelModalLoop (false),
         shouldUsePreferredSize (false)
    {
        name = name_;

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
        log ("ASIO - exiting");
        removeCurrentDriver();
    }

    void updateSampleRates()
    {
        // find a list of sample rates..
        const double possibleSampleRates[] = { 44100.0, 48000.0, 88200.0, 96000.0, 176400.0, 192000.0 };
        sampleRates.clear();

        if (asioObject != nullptr)
        {
            for (int index = 0; index < numElementsInArray (possibleSampleRates); ++index)
            {
                const long err = asioObject->canSampleRate (possibleSampleRates[index]);

                if (err == 0)
                {
                    sampleRates.add ((int) possibleSampleRates[index]);
                    log ("rate: " + String ((int) possibleSampleRates[index]));
                }
                else if (err != ASE_NoClock)
                {
                    logError ("CanSampleRate", err);
                }
            }

            if (sampleRates.size() == 0)
            {
                double cr = 0;
                const long err = asioObject->getSampleRate (&cr);
                log ("No sample rates supported - current rate: " + String ((int) cr));

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
            log ("Warning: device not open");
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

        ASIOClockSource clocks[32] = { 0 };
        long numSources = numElementsInArray (clocks);
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
        buffersCreated = false;

        if (currentSampleRate != sampleRate)
        {
            log ("ASIO samplerate: " + String (currentSampleRate) + " to " + String (sampleRate));
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
                    log ("ASIOInit: " + error);
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
                    info->buffers[0] = info->buffers[1] = nullptr;
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
                    info->buffers[0] = info->buffers[1] = nullptr;
                    ++info;
                    ++numActiveOutputChans;
                }
            }

            const int totalBuffers = numActiveInputChans + numActiveOutputChans;

            setCallbackFunctions();

            log ("disposing buffers");
            err = asioObject->disposeBuffers();

            log ("creating buffers: " + String (totalBuffers) + ", " + String (currentBlockSizeSamples));
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

                for (i = 0; i < jmin ((int) totalNumInputChans, (int) maxASIOChannels); ++i)
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

                for (i = 0; i < jmin ((int) totalNumOutputChans, (int) maxASIOChannels); ++i)
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

                for (i = types.size(); --i >= 0;)
                {
                    log ("channel format: " + String (types[i]));
                }

                jassert (n <= totalBuffers);

                for (i = 0; i < numActiveOutputChans; ++i)
                {
                    outputFormat[i].clear (bufferInfos [numActiveInputChans + i].buffers[0], currentBlockSizeSamples);
                    outputFormat[i].clear (bufferInfos [numActiveInputChans + i].buffers[1], currentBlockSizeSamples);
                }

                inputLatency = outputLatency = 0;

                if (asioObject->getLatencies (&inputLatency, &outputLatency) != 0)
                {
                    log ("ASIO - no latencies");
                }
                else
                {
                    log ("ASIO latencies: " + String ((int) outputLatency) + ", " + String ((int) inputLatency));
                }

                deviceIsOpen = true;

                log ("starting ASIO");
                calledback = false;
                err = asioObject->start();

                if (err != 0)
                {
                    deviceIsOpen = false;
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
            disposeBuffers();

            Thread::sleep (20);
            isStarted = false;
            deviceIsOpen = false;

            const String errorCopy (error);
            close(); // (this resets the error string)
            error = errorCopy;
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

        if (isASIOOpen && deviceIsOpen)
        {
            const ScopedLock sl (callbackLock);

            deviceIsOpen = false;
            isStarted = false;
            needToReset = false;
            isReSync = false;

            log ("ASIO - stopping");

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
        log ("ASIO - showing control panel");

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

                log ("spent: " + String (spent));

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
        needToReset = true;
    }

    void resyncRequest() noexcept
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

            if (deviceIsOpen)
            {
                AudioIODeviceCallback* const oldCallback = currentCallback;

                close();
                open (BigInteger (currentChansIn), BigInteger (currentChansOut),
                      currentSampleRate, currentBlockSizeSamples);

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

    enum { maxASIOChannels = 160 };

    ASIOBufferInfo bufferInfos [maxASIOChannels];
    float* inBuffers [maxASIOChannels];
    float* outBuffers [maxASIOChannels];

    ASIOSampleFormat inputFormat [maxASIOChannels];
    ASIOSampleFormat outputFormat [maxASIOChannels];

    WaitableEvent event1;
    HeapBlock <float> tempBuffer;
    int volatile bufferIndex, numActiveInputChans, numActiveOutputChans;

    bool deviceIsOpen, isStarted, buffersCreated;
    bool volatile isASIOOpen;
    bool volatile calledback;
    bool volatile littleEndian, postOutput, needToReset, isReSync;
    bool volatile insideControlPanelModalLoop;
    bool volatile shouldUsePreferredSize;

    //==============================================================================
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

    String initDriver()
    {
        if (asioObject != nullptr)
        {
            char buffer [256] = { 0 };

            if (! asioObject->init (juce_messageWindowHandle))
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

    String openDevice()
    {
        // open the device and get its info..
        log ("opening ASIO device: " + getName());

        needToReset = false;
        isReSync = false;
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
                    log (String ((int) totalNumInputChans) + " in, " + String ((int) totalNumOutputChans) + " out");

                    if ((err = asioObject->getBufferSize (&minSize, &maxSize, &preferredSize, &granularity)) == 0)
                    {
                        // find a list of buffer sizes..
                        log (String ((int) minSize) + " " + String ((int) maxSize) + " " + String ((int) preferredSize) + " " + String ((int) granularity));

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

                        log ("latencies: " + String ((int) inputLatency) + ", " + String ((int) outputLatency));

                        // create some dummy buffers now.. because cubase does..
                        numActiveInputChans = 0;
                        numActiveOutputChans = 0;

                        ASIOBufferInfo* info = bufferInfos;
                        int i, numChans = 0;
                        for (i = 0; i < jmin (2, (int) totalNumInputChans); ++i)
                        {
                            info->isInput = 1;
                            info->channelNum = i;
                            info->buffers[0] = info->buffers[1] = nullptr;
                            ++info;
                            ++numChans;
                        }

                        const int outputBufferIndex = numChans;

                        for (i = 0; i < jmin (2, (int) totalNumOutputChans); ++i)
                        {
                            info->isInput = 0;
                            info->channelNum = i;
                            info->buffers[0] = info->buffers[1] = nullptr;
                            ++info;
                            ++numChans;
                        }

                        setCallbackFunctions();

                        log ("creating buffers (dummy): " + String (numChans) + ", " + String ((int) preferredSize));

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

                            log (String ((int) totalNumInputChans) + " in; " + String ((int) totalNumOutputChans) + " out");
                        }

                        updateSampleRates();

                        for (i = 0; i < totalNumInputChans; ++i)
                        {
                            ASIOChannelInfo channelInfo = { 0 };
                            channelInfo.channel = i;
                            channelInfo.isInput = 1;
                            asioObject->getChannelInfo (&channelInfo);

                            inputChannelNames.add (String (CharPointer_UTF8 (channelInfo.name)));
                        }

                        for (i = 0; i < totalNumOutputChans; ++i)
                        {
                            ASIOChannelInfo channelInfo = { 0 };
                            channelInfo.channel = i;
                            channelInfo.isInput = 0;
                            asioObject->getChannelInfo (&channelInfo);

                            outputChannelNames.add (String (CharPointer_UTF8 (channelInfo.name)));
                            outputFormat[i] = ASIOSampleFormat (channelInfo.type);

                            if (i < 2)
                            {
                                // clear the channels that are used with the dummy stuff
                                outputFormat[i].clear (bufferInfos [outputBufferIndex + i].buffers[0], preferredSize);
                                outputFormat[i].clear (bufferInfos [outputBufferIndex + i].buffers[1], preferredSize);
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
            disposeBuffers();
            removeCurrentDriver();
            isASIOOpen = false;
        }
        else
        {
            isASIOOpen = true;
            log ("ASIO device open");
        }

        deviceIsOpen = false;
        needToReset = false;
        isReSync = false;

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

            if (currentCallback != nullptr)
            {
                int i;
                for (i = 0; i < numActiveInputChans; ++i)
                {
                    jassert (inBuffers[i]!= nullptr);
                    inputFormat[i].convertToFloat (infos[i].buffers[bi], inBuffers[i], samps);
                }

                currentCallback->audioDeviceIOCallback ((const float**) inBuffers, numActiveInputChans,
                                                        outBuffers, numActiveOutputChans, samps);

                for (i = 0; i < numActiveOutputChans; ++i)
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
            break;

        case kAsioResetRequest:
            if (currentASIODev[deviceIndex] != nullptr)
                currentASIODev[deviceIndex]->resetRequest();

            return 1;

        case kAsioResyncRequest:
            if (currentASIODev[deviceIndex] != nullptr)
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

    static void JUCE_ASIOCALLBACK sampleRateChangedCallback (ASIOSampleRate)
    {
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ASIOAudioIODevice);
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

                        log ("found " + deviceName);
                        deviceNames.add (deviceName);
                        classIds.add (new CLSID (classId));
                    }
                }

                RegCloseKey (subKey);
            }
        }
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ASIOAudioIODeviceType);
};

AudioIODeviceType* AudioIODeviceType::createAudioIODeviceType_ASIO()
{
    return new ASIOAudioIODeviceType();
}

AudioIODevice* juce_createASIOAudioIODeviceForGUID (const String& name,
                                                    void* guid,
                                                    const String& optionalDllForDirectLoading)
{
    const int freeSlot = ASIOAudioIODeviceType::findFreeSlot();

    if (freeSlot < 0)
        return nullptr;

    return new ASIOAudioIODevice (name, *(CLSID*) guid, freeSlot, optionalDllForDirectLoading);
}

#undef logError
#undef log

/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#include "../../juce_core/system/juce_TargetPlatform.h"
#include "../utility/juce_CheckSettingMacros.h"

#if JucePlugin_Build_VST

#ifdef _MSC_VER
 #pragma warning (disable : 4996 4100)
#endif

#include "../utility/juce_IncludeSystemHeaders.h"

#ifdef PRAGMA_ALIGN_SUPPORTED
 #undef PRAGMA_ALIGN_SUPPORTED
 #define PRAGMA_ALIGN_SUPPORTED 1
#endif

#ifndef _MSC_VER
 #define __cdecl
#endif

#if JUCE_CLANG
 #pragma clang diagnostic push
 #pragma clang diagnostic ignored "-Wconversion"
 #pragma clang diagnostic ignored "-Wshadow"
 #pragma clang diagnostic ignored "-Wdeprecated-register"
 #pragma clang diagnostic ignored "-Wunused-parameter"
 #pragma clang diagnostic ignored "-Wdeprecated-writable-strings"
 #pragma clang diagnostic ignored "-Wnon-virtual-dtor"
#endif

#ifdef _MSC_VER
 #pragma warning (push)
 #pragma warning (disable : 4458)
#endif

#include <juce_core/juce_core.h>
#include "../../juce_audio_processors/format_types/juce_VSTInterface.h"

#ifdef _MSC_VER
 #pragma warning (pop)
#endif

#if JUCE_CLANG
 #pragma clang diagnostic pop
#endif

//==============================================================================
#ifdef _MSC_VER
 #pragma pack (push, 8)
#endif

#include "../utility/juce_IncludeModuleHeaders.h"
#include "../utility/juce_FakeMouseMoveGenerator.h"
#include "../utility/juce_WindowsHooks.h"

#include "../../juce_audio_processors/format_types/juce_VSTCommon.h"

#ifdef _MSC_VER
 #pragma pack (pop)
#endif

#undef MemoryBlock

class JuceVSTWrapper;
static bool recursionCheck = false;

namespace juce
{
 #if JUCE_MAC
  extern JUCE_API void initialiseMacVST();
  extern JUCE_API void* attachComponentToWindowRefVST (Component*, void* parent, bool isNSView);
  extern JUCE_API void detachComponentFromWindowRefVST (Component*, void* window, bool isNSView);
  extern JUCE_API void setNativeHostWindowSizeVST (void* window, Component*, int newWidth, int newHeight, bool isNSView);
  extern JUCE_API void checkWindowVisibilityVST (void* window, Component*, bool isNSView);
  extern JUCE_API bool forwardCurrentKeyEventToHostVST (Component*, bool isNSView);
 #if ! JUCE_64BIT
  extern JUCE_API void updateEditorCompBoundsVST (Component*);
 #endif
 #endif

  extern JUCE_API bool handleManufacturerSpecificVST2Opcode (int32, pointer_sized_int, void*, float);
}


//==============================================================================
#if JUCE_WINDOWS

namespace
{
    // Returns the actual container window, unlike GetParent, which can also return a separate owner window.
    static HWND getWindowParent (HWND w) noexcept    { return GetAncestor (w, GA_PARENT); }

    static HWND findMDIParentOf (HWND w)
    {
        const int frameThickness = GetSystemMetrics (SM_CYFIXEDFRAME);

        while (w != 0)
        {
            auto parent = getWindowParent (w);

            if (parent == 0)
                break;

            TCHAR windowType[32] = { 0 };
            GetClassName (parent, windowType, 31);

            if (String (windowType).equalsIgnoreCase ("MDIClient"))
                return parent;

            RECT windowPos, parentPos;
            GetWindowRect (w, &windowPos);
            GetWindowRect (parent, &parentPos);

            auto dw = (parentPos.right - parentPos.left) - (windowPos.right - windowPos.left);
            auto dh = (parentPos.bottom - parentPos.top) - (windowPos.bottom - windowPos.top);

            if (dw > 100 || dh > 100)
                break;

            w = parent;

            if (dw == 2 * frameThickness)
                break;
        }

        return w;
    }

    static bool messageThreadIsDefinitelyCorrect = false;
}

//==============================================================================
#elif JUCE_LINUX

struct SharedMessageThread  : public Thread
{
    SharedMessageThread ()
        : Thread ("VstMessageThread")
    {
        startThread (7);

        while (! initialised)
            sleep (1);
    }

    ~SharedMessageThread()
    {
        signalThreadShouldExit();
        JUCEApplicationBase::quit();
        waitForThreadToExit (5000);
        clearSingletonInstance();
    }

    void run() override
    {
        initialiseJuce_GUI();
        initialised = true;

        MessageManager::getInstance()->setCurrentThreadAsMessageThread();

        ScopedXDisplay xDisplay;

        while ((! threadShouldExit()) && MessageManager::getInstance()->runDispatchLoopUntil (250))
        {}
    }

    juce_DeclareSingleton (SharedMessageThread, false)

    bool initialised = false;
};

juce_ImplementSingleton (SharedMessageThread)

#endif

static Array<void*> activePlugins;

//==============================================================================
// Ableton Live host specific commands
struct AbletonLiveHostSpecific
{
    enum
    {
        KCantBeSuspended = (1 << 2)
    };

    uint32 magic;        // 'AbLi'
    int cmd;             // 5 = realtime properties
    size_t commandSize;  // sizeof (int)
    int flags;           // KCantBeSuspended = (1 << 2)
};

//==============================================================================
/**
    This is an AudioEffectX object that holds and wraps our AudioProcessor...
*/
class JuceVSTWrapper  : public AudioProcessorListener,
                        public AudioPlayHead,
                        private Timer,
                        private AsyncUpdater
{
private:
    //==============================================================================
    template <typename FloatType>
    struct VstTempBuffers
    {
        VstTempBuffers() {}
        ~VstTempBuffers() { release(); }

        void release() noexcept
        {
            for (auto* c : tempChannels)
                delete[] c;

            tempChannels.clear();
        }

        HeapBlock<FloatType*> channels;
        Array<FloatType*> tempChannels;  // see note in processReplacing()
        juce::AudioBuffer<FloatType> processTempBuffer;
    };

    /** Use the same names as the VST SDK. */
    struct VstOpCodeArguments
    {
        int32 index;
        pointer_sized_int value;
        void* ptr;
        float opt;
    };

public:
    //==============================================================================
    JuceVSTWrapper (VstHostCallback cb, AudioProcessor* af)
       : hostCallback (cb),
         processor (af)
    {
        // VST-2 does not support disabling buses: so always enable all of them
        processor->enableAllBuses();

        findMaxTotalChannels (maxNumInChannels, maxNumOutChannels);

        // You must at least have some channels
        jassert (processor->isMidiEffect() || (maxNumInChannels > 0 || maxNumOutChannels > 0));

        if (processor->isMidiEffect())
            maxNumInChannels = maxNumOutChannels = 2;

       #ifdef JucePlugin_PreferredChannelConfigurations
        processor->setPlayConfigDetails (maxNumInChannels, maxNumOutChannels, 44100.0, 1024);
       #endif

        processor->setRateAndBufferSizeDetails (0, 0);
        processor->setPlayHead (this);
        processor->addListener (this);

        memset (&vstEffect, 0, sizeof (vstEffect));
        vstEffect.interfaceIdentifier = juceVstInterfaceIdentifier;
        vstEffect.dispatchFunction = dispatcherCB;
        vstEffect.processAudioFunction = nullptr;
        vstEffect.setParameterValueFunction = setParameterCB;
        vstEffect.getParameterValueFunction = getParameterCB;
        vstEffect.numPrograms = jmax (1, af->getNumPrograms());
        vstEffect.numParameters = af->getNumParameters();
        vstEffect.numInputChannels = maxNumInChannels;
        vstEffect.numOutputChannels = maxNumOutChannels;
        vstEffect.latency = processor->getLatencySamples();
        vstEffect.effectPointer = this;
        vstEffect.plugInIdentifier = JucePlugin_VSTUniqueID;

       #ifdef JucePlugin_VSTChunkStructureVersion
        vstEffect.plugInVersion = convertHexVersionToDecimal (JucePlugin_VSTChunkStructureVersion);
       #else
        vstEffect.plugInVersion = convertHexVersionToDecimal (JucePlugin_VersionCode);
       #endif

        vstEffect.processAudioInplaceFunction = processReplacingCB;
        vstEffect.processDoubleAudioInplaceFunction = processDoubleReplacingCB;

        vstEffect.flags |= vstEffectFlagHasEditor;

        vstEffect.flags |= vstEffectFlagInplaceAudio;
        if (processor->supportsDoublePrecisionProcessing())
            vstEffect.flags |= vstEffectFlagInplaceDoubleAudio;

       #if JucePlugin_IsSynth
        vstEffect.flags |= vstEffectFlagIsSynth;
       #endif

        vstEffect.flags |= vstEffectFlagDataInChunks;

        activePlugins.add (this);
    }

    ~JuceVSTWrapper()
    {
        JUCE_AUTORELEASEPOOL
        {
            {
               #if JUCE_LINUX
                MessageManagerLock mmLock;
               #endif
                stopTimer();
                deleteEditor (false);

                hasShutdown = true;

                delete processor;
                processor = nullptr;

                jassert (editorComp == nullptr);

                deleteTempChannels();

                jassert (activePlugins.contains (this));
                activePlugins.removeFirstMatchingValue (this);
            }

            if (activePlugins.size() == 0)
            {
               #if JUCE_LINUX
                SharedMessageThread::deleteInstance();
               #endif
                shutdownJuce_GUI();

               #if JUCE_WINDOWS
                messageThreadIsDefinitelyCorrect = false;
               #endif
            }
        }
    }

    VstEffectInterface* getVstEffectInterface() noexcept    { return &vstEffect; }

    template <typename FloatType>
    void internalProcessReplacing (FloatType** inputs, FloatType** outputs,
                                   int32 numSamples, VstTempBuffers<FloatType>& tmpBuffers)
    {
        const bool isMidiEffect = processor->isMidiEffect();

        if (firstProcessCallback)
        {
            firstProcessCallback = false;

            // if this fails, the host hasn't called resume() before processing
            jassert (isProcessing);

            // (tragically, some hosts actually need this, although it's stupid to have
            //  to do it here..)
            if (! isProcessing)
                resume();

            processor->setNonRealtime (isProcessLevelOffline());

           #if JUCE_WINDOWS
            if (getHostType().isWavelab())
            {
                int priority = GetThreadPriority (GetCurrentThread());

                if (priority <= THREAD_PRIORITY_NORMAL && priority >= THREAD_PRIORITY_LOWEST)
                    processor->setNonRealtime (true);
            }
           #endif
        }

       #if JUCE_DEBUG && ! (JucePlugin_ProducesMidiOutput || JucePlugin_IsMidiEffect)
        const int numMidiEventsComingIn = midiEvents.getNumEvents();
       #endif

        jassert (activePlugins.contains (this));

        {
            const int numIn  = processor->getTotalNumInputChannels();
            const int numOut = processor->getTotalNumOutputChannels();

            const ScopedLock sl (processor->getCallbackLock());

            if (processor->isSuspended())
            {
                for (int i = 0; i < numOut; ++i)
                    if (outputs[i] != nullptr)
                        FloatVectorOperations::clear (outputs[i], numSamples);
            }
            else
            {
                int i;
                for (i = 0; i < numOut; ++i)
                {
                    auto* chan = tmpBuffers.tempChannels.getUnchecked(i);

                    if (chan == nullptr)
                    {
                        chan = outputs[i];

                        bool bufferPointerReusedForOtherChannels = false;

                        for (int j = i; --j >= 0;)
                        {
                            if (outputs[j] == chan)
                            {
                                bufferPointerReusedForOtherChannels = true;
                                break;
                            }
                        }

                        // if some output channels are disabled, some hosts supply the same buffer
                        // for multiple channels or supply a nullptr - this buggers up our method
                        // of copying the inputs over the outputs, so we need to create unique temp
                        // buffers in this case..
                        if (bufferPointerReusedForOtherChannels || chan == nullptr)
                        {
                            chan = new FloatType [(size_t) blockSize * 2];
                            tmpBuffers.tempChannels.set (i, chan);
                        }
                    }

                    if (i < numIn)
                    {
                        if (chan != inputs[i])
                            memcpy (chan, inputs[i], sizeof (FloatType) * (size_t) numSamples);
                    }
                    else
                    {
                        FloatVectorOperations::clear (chan, numSamples);
                    }

                    tmpBuffers.channels[i] = chan;
                }

                for (; i < numIn; ++i)
                    tmpBuffers.channels[i] = inputs[i];

                {
                    const int numChannels = jmax (numIn, numOut);
                    AudioBuffer<FloatType> chans (tmpBuffers.channels, isMidiEffect ? 0 : numChannels, numSamples);

                    if (isBypassed)
                        processor->processBlockBypassed (chans, midiEvents);
                    else
                        processor->processBlock (chans, midiEvents);
                }

                // copy back any temp channels that may have been used..
                for (i = 0; i < numOut; ++i)
                    if (auto* chan = tmpBuffers.tempChannels.getUnchecked(i))
                        if (auto* dest = outputs[i])
                            memcpy (dest, chan, sizeof (FloatType) * (size_t) numSamples);
            }
        }

        if (! midiEvents.isEmpty())
        {
           #if JucePlugin_ProducesMidiOutput || JucePlugin_IsMidiEffect
            auto numEvents = midiEvents.getNumEvents();

            outgoingEvents.ensureSize (numEvents);
            outgoingEvents.clear();

            const uint8* midiEventData;
            int midiEventSize, midiEventPosition;
            MidiBuffer::Iterator i (midiEvents);

            while (i.getNextEvent (midiEventData, midiEventSize, midiEventPosition))
            {
                jassert (midiEventPosition >= 0 && midiEventPosition < numSamples);

                outgoingEvents.addEvent (midiEventData, midiEventSize, midiEventPosition);
            }

            // Send VST events to the host.
            if (hostCallback != nullptr)
                hostCallback (&vstEffect, hostOpcodePreAudioProcessingEvents, 0, 0, outgoingEvents.events, 0);
           #elif JUCE_DEBUG
            /*  This assertion is caused when you've added some events to the
                midiMessages array in your processBlock() method, which usually means
                that you're trying to send them somewhere. But in this case they're
                getting thrown away.

                If your plugin does want to send midi messages, you'll need to set
                the JucePlugin_ProducesMidiOutput macro to 1 in your
                JucePluginCharacteristics.h file.

                If you don't want to produce any midi output, then you should clear the
                midiMessages array at the end of your processBlock() method, to
                indicate that you don't want any of the events to be passed through
                to the output.
            */
            jassert (midiEvents.getNumEvents() <= numMidiEventsComingIn);
           #endif

            midiEvents.clear();
        }
    }

    void processReplacing (float** inputs, float** outputs, int32 sampleFrames)
    {
        jassert (! processor->isUsingDoublePrecision());
        internalProcessReplacing (inputs, outputs, sampleFrames, floatTempBuffers);
    }

    static void processReplacingCB (VstEffectInterface* vstInterface, float** inputs, float** outputs, int32 sampleFrames)
    {
        getWrapper (vstInterface)->processReplacing (inputs, outputs, sampleFrames);
    }

    void processDoubleReplacing (double** inputs, double** outputs, int32 sampleFrames)
    {
        jassert (processor->isUsingDoublePrecision());
        internalProcessReplacing (inputs, outputs, sampleFrames, doubleTempBuffers);
    }

    static void processDoubleReplacingCB (VstEffectInterface* vstInterface, double** inputs, double** outputs, int32 sampleFrames)
    {
        getWrapper (vstInterface)->processDoubleReplacing (inputs, outputs, sampleFrames);
    }

    //==============================================================================
    void resume()
    {
        if (processor != nullptr)
        {
            isProcessing = true;

            auto numInAndOutChannels = static_cast<size_t> (vstEffect.numInputChannels + vstEffect.numOutputChannels);
            floatTempBuffers .channels.calloc (numInAndOutChannels);
            doubleTempBuffers.channels.calloc (numInAndOutChannels);

            auto currentRate = sampleRate;
            auto currentBlockSize = blockSize;

            firstProcessCallback = true;

            processor->setNonRealtime (isProcessLevelOffline());
            processor->setRateAndBufferSizeDetails (currentRate, currentBlockSize);

            deleteTempChannels();

            processor->prepareToPlay (currentRate, currentBlockSize);

            midiEvents.ensureSize (2048);
            midiEvents.clear();

            vstEffect.latency = processor->getLatencySamples();

            /** If this plug-in is a synth or it can receive midi events we need to tell the
                host that we want midi. In the SDK this method is marked as deprecated, but
                some hosts rely on this behaviour.
            */
            if (vstEffect.flags & vstEffectFlagIsSynth || JucePlugin_WantsMidiInput || JucePlugin_IsMidiEffect)
            {
                if (hostCallback != nullptr)
                    hostCallback (&vstEffect, hostOpcodePlugInWantsMidi, 0, 1, 0, 0);
            }

            if (getHostType().isAbletonLive()
                 && hostCallback != nullptr
                 && processor->getTailLengthSeconds() == std::numeric_limits<double>::max())
            {
                AbletonLiveHostSpecific hostCmd;

                hostCmd.magic = 0x41624c69; // 'AbLi'
                hostCmd.cmd = 5;
                hostCmd.commandSize = sizeof (int);
                hostCmd.flags = AbletonLiveHostSpecific::KCantBeSuspended;

                hostCallback (&vstEffect, hostOpcodeManufacturerSpecific, 0, 0, &hostCmd, 0.0f);
            }

           #if JucePlugin_ProducesMidiOutput || JucePlugin_IsMidiEffect
            outgoingEvents.ensureSize (512);
           #endif
        }
    }

    void suspend()
    {
        if (processor != nullptr)
        {
            processor->releaseResources();
            outgoingEvents.freeEvents();

            isProcessing = false;
            floatTempBuffers.channels.free();
            doubleTempBuffers.channels.free();

            deleteTempChannels();
        }
    }

    //==============================================================================
    bool getCurrentPosition (AudioPlayHead::CurrentPositionInfo& info) override
    {
        const VstTimingInformation* ti = nullptr;

        if (hostCallback != nullptr)
        {
            int32 flags = vstTimingInfoFlagMusicalPositionValid | vstTimingInfoFlagTempoValid
                              | vstTimingInfoFlagLastBarPositionValid | vstTimingInfoFlagLoopPositionValid
                              | vstTimingInfoFlagTimeSignatureValid | vstTimingInfoFlagSmpteValid
                              | vstTimingInfoFlagNearestClockValid;

            auto result = hostCallback (&vstEffect, hostOpcodeGetTimingInfo, 0, flags, 0, 0);
            ti = reinterpret_cast<VstTimingInformation*> (result);
        }

        if (ti == nullptr || ti->sampleRate <= 0)
            return false;

        info.bpm = (ti->flags & vstTimingInfoFlagTempoValid) != 0 ? ti->tempoBPM : 0.0;

        if ((ti->flags & vstTimingInfoFlagTimeSignatureValid) != 0)
        {
            info.timeSigNumerator   = ti->timeSignatureNumerator;
            info.timeSigDenominator = ti->timeSignatureDenominator;
        }
        else
        {
            info.timeSigNumerator   = 4;
            info.timeSigDenominator = 4;
        }

        info.timeInSamples = (int64) (ti->samplePosition + 0.5);
        info.timeInSeconds = ti->samplePosition / ti->sampleRate;
        info.ppqPosition = (ti->flags & vstTimingInfoFlagMusicalPositionValid) != 0 ? ti->musicalPosition : 0.0;
        info.ppqPositionOfLastBarStart = (ti->flags & vstTimingInfoFlagLastBarPositionValid) != 0 ? ti->lastBarPosition : 0.0;

        if ((ti->flags & vstTimingInfoFlagSmpteValid) != 0)
        {
            AudioPlayHead::FrameRateType rate = AudioPlayHead::fpsUnknown;
            double fps = 1.0;

            switch (ti->smpteRate)
            {
                case vstSmpteRateFps239:       rate = AudioPlayHead::fps23976;    fps = 24.0 * 1000.0 / 1001.0; break;
                case vstSmpteRateFps24:        rate = AudioPlayHead::fps24;       fps = 24.0;  break;
                case vstSmpteRateFps25:        rate = AudioPlayHead::fps25;       fps = 25.0;  break;
                case vstSmpteRateFps2997:      rate = AudioPlayHead::fps2997;     fps = 30.0 * 1000.0 / 1001.0; break;
                case vstSmpteRateFps30:        rate = AudioPlayHead::fps30;       fps = 30.0;  break;
                case vstSmpteRateFps2997drop:  rate = AudioPlayHead::fps2997drop; fps = 30.0 * 1000.0 / 1001.0; break;
                case vstSmpteRateFps30drop:    rate = AudioPlayHead::fps30drop;   fps = 30.0;  break;

                case vstSmpteRate16mmFilm:
                case vstSmpteRate35mmFilm:     fps = 24.0; break;

                case vstSmpteRateFps249:       fps = 25.0 * 1000.0 / 1001.0; break;
                case vstSmpteRateFps599:       fps = 60.0 * 1000.0 / 1001.0; break;
                case vstSmpteRateFps60:        fps = 60; break;

                default:                       jassertfalse; // unknown frame-rate..
            }

            info.frameRate = rate;
            info.editOriginTime = ti->smpteOffset / (80.0 * fps);
        }
        else
        {
            info.frameRate = AudioPlayHead::fpsUnknown;
            info.editOriginTime = 0;
        }

        info.isRecording = (ti->flags & vstTimingInfoFlagCurrentlyRecording) != 0;
        info.isPlaying   = (ti->flags & (vstTimingInfoFlagCurrentlyRecording | vstTimingInfoFlagCurrentlyPlaying)) != 0;
        info.isLooping   = (ti->flags & vstTimingInfoFlagLoopActive) != 0;

        if ((ti->flags & vstTimingInfoFlagLoopPositionValid) != 0)
        {
            info.ppqLoopStart = ti->loopStartPosition;
            info.ppqLoopEnd   = ti->loopEndPosition;
        }
        else
        {
            info.ppqLoopStart = 0;
            info.ppqLoopEnd = 0;
        }

        return true;
    }

    //==============================================================================
    float getParameter (int32 index) const
    {
        if (processor == nullptr)
            return 0.0f;

        jassert (isPositiveAndBelow (index, processor->getNumParameters()));
        return processor->getParameter (index);
    }

    static float getParameterCB (VstEffectInterface* vstInterface, int32 index)
    {
        return getWrapper (vstInterface)->getParameter (index);
    }

    void setParameter (int32 index, float value)
    {
        if (processor != nullptr)
        {
            jassert (isPositiveAndBelow (index, processor->getNumParameters()));
            processor->setParameter (index, value);
        }
    }

    static void setParameterCB (VstEffectInterface* vstInterface, int32 index, float value)
    {
        getWrapper (vstInterface)->setParameter (index, value);
    }

    void audioProcessorParameterChanged (AudioProcessor*, int index, float newValue) override
    {
        if (hostCallback != nullptr)
            hostCallback (&vstEffect, hostOpcodeParameterChanged, index, 0, 0, newValue);
    }

    void audioProcessorParameterChangeGestureBegin (AudioProcessor*, int index) override
    {
        if (hostCallback != nullptr)
            hostCallback (&vstEffect, hostOpcodeParameterChangeGestureBegin, index, 0, 0, 0);
    }

    void audioProcessorParameterChangeGestureEnd (AudioProcessor*, int index) override
    {
        if (hostCallback != nullptr)
            hostCallback (&vstEffect, hostOpcodeParameterChangeGestureEnd, index, 0, 0, 0);
    }

    void audioProcessorChanged (AudioProcessor*) override
    {
        vstEffect.latency = processor->getLatencySamples();

        if (hostCallback != nullptr)
            hostCallback (&vstEffect, hostOpcodeUpdateView, 0, 0, 0, 0);

        triggerAsyncUpdate();
    }

    void handleAsyncUpdate() override
    {
        if (hostCallback != nullptr)
            hostCallback (&vstEffect, hostOpcodeIOModified, 0, 0, 0, 0);
    }

    bool getPinProperties (VstPinInfo& properties, bool direction, int index) const
    {
        if (processor->isMidiEffect())
            return false;

        int channelIdx, busIdx;

        // fill with default
        properties.flags = 0;
        properties.text[0] = 0;
        properties.shortText[0] = 0;
        properties.configurationType = vstSpeakerConfigTypeEmpty;

        if ((channelIdx = processor->getOffsetInBusBufferForAbsoluteChannelIndex (direction, index, busIdx)) >= 0)
        {
            auto& bus = *processor->getBus (direction, busIdx);
            auto& channelSet = bus.getCurrentLayout();
            auto channelType = channelSet.getTypeOfChannel (channelIdx);

            properties.flags = vstPinInfoFlagIsActive | vstPinInfoFlagValid;
            properties.configurationType = SpeakerMappings::channelSetToVstArrangementType (channelSet);
            String label = bus.getName();

           #ifdef JucePlugin_PreferredChannelConfigurations
            label += " " + String (channelIdx);
           #else
            if (channelSet.size() > 1)
                label += " " + AudioChannelSet::getAbbreviatedChannelTypeName (channelType);
           #endif

            label.copyToUTF8 (properties.text, (size_t) (vstMaxParameterOrPinLabelLength + 1));
            label.copyToUTF8 (properties.shortText, (size_t) (vstMaxParameterOrPinShortLabelLength + 1));

            if (channelType == AudioChannelSet::left
                || channelType == AudioChannelSet::leftSurround
                || channelType == AudioChannelSet::leftCentre
                || channelType == AudioChannelSet::leftSurroundSide
                || channelType == AudioChannelSet::topFrontLeft
                || channelType == AudioChannelSet::topRearLeft
                || channelType == AudioChannelSet::leftSurroundRear
                || channelType == AudioChannelSet::wideLeft)
                properties.flags |= vstPinInfoFlagIsStereo;

            return true;
        }

        return false;
    }

    //==============================================================================
    struct SpeakerMappings  : private AudioChannelSet // (inheritance only to give easier access to items in the namespace)
    {
        struct Mapping
        {
            int32 vst2;
            ChannelType channels[13];

            bool matches (const Array<ChannelType>& chans) const noexcept
            {
                const int n = sizeof (channels) / sizeof (ChannelType);

                for (int i = 0; i < n; ++i)
                {
                    if (channels[i] == unknown)  return (i == chans.size());
                    if (i == chans.size())       return (channels[i] == unknown);

                    if (channels[i] != chans.getUnchecked(i))
                        return false;
                }

                return true;
            }
        };

        static AudioChannelSet vstArrangementTypeToChannelSet (const VstSpeakerConfiguration& arr)
        {
            if (arr.type == vstSpeakerConfigTypeEmpty)          return AudioChannelSet::disabled();
            if (arr.type == vstSpeakerConfigTypeMono)           return AudioChannelSet::mono();
            if (arr.type == vstSpeakerConfigTypeLR)             return AudioChannelSet::stereo();
            if (arr.type == vstSpeakerConfigTypeLRC)            return AudioChannelSet::createLCR();
            if (arr.type == vstSpeakerConfigTypeLRS)            return AudioChannelSet::createLRS();
            if (arr.type == vstSpeakerConfigTypeLRCS)           return AudioChannelSet::createLCRS();
            if (arr.type == vstSpeakerConfigTypeLRCLsRs)        return AudioChannelSet::create5point0();
            if (arr.type == vstSpeakerConfigTypeLRCLfeLsRs)     return AudioChannelSet::create5point1();
            if (arr.type == vstSpeakerConfigTypeLRCLsRsCs)      return AudioChannelSet::create6point0();
            if (arr.type == vstSpeakerConfigTypeLRCLfeLsRsCs)   return AudioChannelSet::create6point1();
            if (arr.type == vstSpeakerConfigTypeLRLsRsSlSr)     return AudioChannelSet::create6point0Music();
            if (arr.type == vstSpeakerConfigTypeLRLfeLsRsSlSr)  return AudioChannelSet::create6point1Music();
            if (arr.type == vstSpeakerConfigTypeLRCLsRsSlSr)    return AudioChannelSet::create7point0();
            if (arr.type == vstSpeakerConfigTypeLRCLsRsLcRc)    return AudioChannelSet::create7point0SDDS();
            if (arr.type == vstSpeakerConfigTypeLRCLfeLsRsSlSr) return AudioChannelSet::create7point1();
            if (arr.type == vstSpeakerConfigTypeLRCLfeLsRsLcRc) return AudioChannelSet::create7point1SDDS();
            if (arr.type == vstSpeakerConfigTypeLRLsRs)         return AudioChannelSet::quadraphonic();

            for (auto* m = getMappings(); m->vst2 != vstSpeakerConfigTypeEmpty; ++m)
            {
                if (m->vst2 == arr.type)
                {
                    AudioChannelSet s;

                    for (int i = 0; m->channels[i] != 0; ++i)
                        s.addChannel (m->channels[i]);

                    return s;
                }
            }

            return AudioChannelSet::discreteChannels (arr.numberOfChannels);
        }

        static int32 channelSetToVstArrangementType (AudioChannelSet channels)
        {
            if (channels == AudioChannelSet::disabled())           return vstSpeakerConfigTypeEmpty;
            if (channels == AudioChannelSet::mono())               return vstSpeakerConfigTypeMono;
            if (channels == AudioChannelSet::stereo())             return vstSpeakerConfigTypeLR;
            if (channels == AudioChannelSet::createLCR())          return vstSpeakerConfigTypeLRC;
            if (channels == AudioChannelSet::createLRS())          return vstSpeakerConfigTypeLRS;
            if (channels == AudioChannelSet::createLCRS())         return vstSpeakerConfigTypeLRCS;
            if (channels == AudioChannelSet::create5point0())      return vstSpeakerConfigTypeLRCLsRs;
            if (channels == AudioChannelSet::create5point1())      return vstSpeakerConfigTypeLRCLfeLsRs;
            if (channels == AudioChannelSet::create6point0())      return vstSpeakerConfigTypeLRCLsRsCs;
            if (channels == AudioChannelSet::create6point1())      return vstSpeakerConfigTypeLRCLfeLsRsCs;
            if (channels == AudioChannelSet::create6point0Music()) return vstSpeakerConfigTypeLRLsRsSlSr;
            if (channels == AudioChannelSet::create6point1Music()) return vstSpeakerConfigTypeLRLfeLsRsSlSr;
            if (channels == AudioChannelSet::create7point0())      return vstSpeakerConfigTypeLRCLsRsSlSr;
            if (channels == AudioChannelSet::create7point0SDDS())  return vstSpeakerConfigTypeLRCLsRsLcRc;
            if (channels == AudioChannelSet::create7point1())      return vstSpeakerConfigTypeLRCLfeLsRsSlSr;
            if (channels == AudioChannelSet::create7point1SDDS())  return vstSpeakerConfigTypeLRCLfeLsRsLcRc;
            if (channels == AudioChannelSet::quadraphonic())       return vstSpeakerConfigTypeLRLsRs;

            if (channels == AudioChannelSet::disabled())
                return vstSpeakerConfigTypeEmpty;

            auto chans = channels.getChannelTypes();

            for (auto* m = getMappings(); m->vst2 != vstSpeakerConfigTypeEmpty; ++m)
                if (m->matches (chans))
                    return m->vst2;

            return vstSpeakerConfigTypeUser;
        }

        static void channelSetToVstArrangement (const AudioChannelSet& channels, VstSpeakerConfiguration& result)
        {
            result.type = channelSetToVstArrangementType (channels);
            result.numberOfChannels = channels.size();

            for (int i = 0; i < result.numberOfChannels; ++i)
            {
                auto& speaker = result.speakers[i];

                zeromem (&speaker, sizeof (VstIndividualSpeakerInfo));
                speaker.type = getSpeakerType (channels.getTypeOfChannel (i));
            }
        }

        static const Mapping* getMappings() noexcept
        {
            static const Mapping mappings[] =
            {
                { vstSpeakerConfigTypeMono,                          { centre, unknown } },
                { vstSpeakerConfigTypeLR,                            { left, right, unknown } },
                { vstSpeakerConfigTypeLsRs,                          { leftSurround, rightSurround, unknown } },
                { vstSpeakerConfigTypeLcRc,                          { leftCentre, rightCentre, unknown } },
                { vstSpeakerConfigTypeSlSr,                          { leftSurroundRear, rightSurroundRear, unknown } },
                { vstSpeakerConfigTypeCLfe,                          { centre, LFE, unknown } },
                { vstSpeakerConfigTypeLRC,                           { left, right, centre, unknown } },
                { vstSpeakerConfigTypeLRS,                           { left, right, surround, unknown } },
                { vstSpeakerConfigTypeLRCLfe,                        { left, right, centre, LFE, unknown } },
                { vstSpeakerConfigTypeLRLfeS,                        { left, right, LFE, surround, unknown } },
                { vstSpeakerConfigTypeLRCS,                          { left, right, centre, surround, unknown } },
                { vstSpeakerConfigTypeLRLsRs,                        { left, right, leftSurround, rightSurround, unknown } },
                { vstSpeakerConfigTypeLRCLfeS,                       { left, right, centre, LFE, surround, unknown } },
                { vstSpeakerConfigTypeLRLfeLsRs,                     { left, right, LFE, leftSurround, rightSurround, unknown } },
                { vstSpeakerConfigTypeLRCLsRs,                       { left, right, centre, leftSurround, rightSurround, unknown } },
                { vstSpeakerConfigTypeLRCLfeLsRs,                    { left, right, centre, LFE, leftSurround, rightSurround, unknown } },
                { vstSpeakerConfigTypeLRCLsRsCs,                     { left, right, centre, leftSurround, rightSurround, surround, unknown } },
                { vstSpeakerConfigTypeLRLsRsSlSr,                    { left, right, leftSurround, rightSurround, leftSurroundRear, rightSurroundRear, unknown } },
                { vstSpeakerConfigTypeLRCLfeLsRsCs,                  { left, right, centre, LFE, leftSurround, rightSurround, surround, unknown } },
                { vstSpeakerConfigTypeLRLfeLsRsSlSr,                 { left, right, LFE, leftSurround, rightSurround, leftSurroundRear, rightSurroundRear, unknown } },
                { vstSpeakerConfigTypeLRCLsRsLcRc,                   { left, right, centre, leftSurround, rightSurround, topFrontLeft, topFrontRight, unknown } },
                { vstSpeakerConfigTypeLRCLsRsSlSr,                   { left, right, centre, leftSurround, rightSurround, leftSurroundRear, rightSurroundRear, unknown } },
                { vstSpeakerConfigTypeLRCLfeLsRsLcRc,                { left, right, centre, LFE, leftSurround, rightSurround, topFrontLeft, topFrontRight, unknown } },
                { vstSpeakerConfigTypeLRCLfeLsRsSlSr,                { left, right, centre, LFE, leftSurround, rightSurround, leftSurroundRear, rightSurroundRear, unknown } },
                { vstSpeakerConfigTypeLRCLsRsLcRcCs,                 { left, right, centre, leftSurround, rightSurround, topFrontLeft, topFrontRight, surround, unknown } },
                { vstSpeakerConfigTypeLRCLsRsCsSlSr,                 { left, right, centre, leftSurround, rightSurround, surround, leftSurroundRear, rightSurroundRear, unknown } },
                { vstSpeakerConfigTypeLRCLfeLsRsLcRcCs,              { left, right, centre, LFE, leftSurround, rightSurround, topFrontLeft, topFrontRight, surround, unknown } },
                { vstSpeakerConfigTypeLRCLfeLsRsCsSlSr,              { left, right, centre, LFE, leftSurround, rightSurround, surround, leftSurroundRear, rightSurroundRear, unknown } },
                { vstSpeakerConfigTypeLRCLfeLsRsTflTfcTfrTrlTrrLfe2, { left, right, centre, LFE, leftSurround, rightSurround, topFrontLeft, topFrontCentre, topFrontRight, topRearLeft, topRearRight, LFE2, unknown } },
                { vstSpeakerConfigTypeEmpty,                         { unknown } }
            };

            return mappings;
        }

        static inline int32 getSpeakerType (AudioChannelSet::ChannelType type) noexcept
        {
            switch (type)
            {
                case AudioChannelSet::left:              return vstIndividualSpeakerTypeLeft;
                case AudioChannelSet::right:             return vstIndividualSpeakerTypeRight;
                case AudioChannelSet::centre:            return vstIndividualSpeakerTypeCentre;
                case AudioChannelSet::LFE:               return vstIndividualSpeakerTypeLFE;
                case AudioChannelSet::leftSurround:      return vstIndividualSpeakerTypeLeftSurround;
                case AudioChannelSet::rightSurround:     return vstIndividualSpeakerTypeRightSurround;
                case AudioChannelSet::leftCentre:        return vstIndividualSpeakerTypeLeftCentre;
                case AudioChannelSet::rightCentre:       return vstIndividualSpeakerTypeRightCentre;
                case AudioChannelSet::surround:          return vstIndividualSpeakerTypeSurround;
                case AudioChannelSet::leftSurroundRear:  return vstIndividualSpeakerTypeLeftRearSurround;
                case AudioChannelSet::rightSurroundRear: return vstIndividualSpeakerTypeRightRearSurround;
                case AudioChannelSet::topMiddle:         return vstIndividualSpeakerTypeTopMiddle;
                case AudioChannelSet::topFrontLeft:      return vstIndividualSpeakerTypeTopFrontLeft;
                case AudioChannelSet::topFrontCentre:    return vstIndividualSpeakerTypeTopFrontCentre;
                case AudioChannelSet::topFrontRight:     return vstIndividualSpeakerTypeTopFrontRight;
                case AudioChannelSet::topRearLeft:       return vstIndividualSpeakerTypeTopRearLeft;
                case AudioChannelSet::topRearCentre:     return vstIndividualSpeakerTypeTopRearCentre;
                case AudioChannelSet::topRearRight:      return vstIndividualSpeakerTypeTopRearRight;
                case AudioChannelSet::LFE2:              return vstIndividualSpeakerTypeLFE2;
                default: break;
            }

            return 0;
        }

        static inline AudioChannelSet::ChannelType getChannelType (int32 type) noexcept
        {
            switch (type)
            {
                case vstIndividualSpeakerTypeLeft:              return AudioChannelSet::left;
                case vstIndividualSpeakerTypeRight:             return AudioChannelSet::right;
                case vstIndividualSpeakerTypeCentre:            return AudioChannelSet::centre;
                case vstIndividualSpeakerTypeLFE:               return AudioChannelSet::LFE;
                case vstIndividualSpeakerTypeLeftSurround:      return AudioChannelSet::leftSurround;
                case vstIndividualSpeakerTypeRightSurround:     return AudioChannelSet::rightSurround;
                case vstIndividualSpeakerTypeLeftCentre:        return AudioChannelSet::leftCentre;
                case vstIndividualSpeakerTypeRightCentre:       return AudioChannelSet::rightCentre;
                case vstIndividualSpeakerTypeSurround:          return AudioChannelSet::surround;
                case vstIndividualSpeakerTypeLeftRearSurround:  return AudioChannelSet::leftSurroundRear;
                case vstIndividualSpeakerTypeRightRearSurround: return AudioChannelSet::rightSurroundRear;
                case vstIndividualSpeakerTypeTopMiddle:         return AudioChannelSet::topMiddle;
                case vstIndividualSpeakerTypeTopFrontLeft:      return AudioChannelSet::topFrontLeft;
                case vstIndividualSpeakerTypeTopFrontCentre:    return AudioChannelSet::topFrontCentre;
                case vstIndividualSpeakerTypeTopFrontRight:     return AudioChannelSet::topFrontRight;
                case vstIndividualSpeakerTypeTopRearLeft:       return AudioChannelSet::topRearLeft;
                case vstIndividualSpeakerTypeTopRearCentre:     return AudioChannelSet::topRearCentre;
                case vstIndividualSpeakerTypeTopRearRight:      return AudioChannelSet::topRearRight;
                case vstIndividualSpeakerTypeLFE2:              return AudioChannelSet::LFE2;
                default: break;
            }

            return AudioChannelSet::unknown;
        }
    };

    void timerCallback() override
    {
        if (shouldDeleteEditor)
        {
            shouldDeleteEditor = false;
            deleteEditor (true);
        }

        if (chunkMemoryTime > 0
             && chunkMemoryTime < juce::Time::getApproximateMillisecondCounter() - 2000
             && ! recursionCheck)
        {
            chunkMemory.reset();
            chunkMemoryTime = 0;
        }

        if (editorComp != nullptr)
            editorComp->checkVisibility();
    }

    void createEditorComp()
    {
        if (hasShutdown || processor == nullptr)
            return;

        if (editorComp == nullptr)
        {
            if (auto* ed = processor->createEditorIfNeeded())
            {
                vstEffect.flags |= vstEffectFlagHasEditor;
                editorComp = new EditorCompWrapper (*this, *ed);

               #if ! (JUCE_MAC || JUCE_IOS)
                ed->setScaleFactor (editorScaleFactor);
               #endif
            }
            else
            {
                vstEffect.flags &= ~vstEffectFlagHasEditor;
            }
        }

        shouldDeleteEditor = false;
    }

    void deleteEditor (bool canDeleteLaterIfModal)
    {
        JUCE_AUTORELEASEPOOL
        {
            PopupMenu::dismissAllActiveMenus();

            jassert (! recursionCheck);
            ScopedValueSetter<bool> svs (recursionCheck, true, false);

            if (editorComp != nullptr)
            {
                if (auto* modalComponent = Component::getCurrentlyModalComponent())
                {
                    modalComponent->exitModalState (0);

                    if (canDeleteLaterIfModal)
                    {
                        shouldDeleteEditor = true;
                        return;
                    }
                }

                editorComp->detachHostWindow();

                if (auto* ed = editorComp->getEditorComp())
                    processor->editorBeingDeleted (ed);

                editorComp = nullptr;

                // there's some kind of component currently modal, but the host
                // is trying to delete our plugin. You should try to avoid this happening..
                jassert (Component::getCurrentlyModalComponent() == nullptr);
            }
        }
    }

    pointer_sized_int dispatcher (int32 opCode, VstOpCodeArguments args)
    {
        if (hasShutdown)
            return 0;

        switch (opCode)
        {
            case plugInOpcodeOpen:                        return handleOpen (args);
            case plugInOpcodeClose:                       return handleClose (args);
            case plugInOpcodeSetCurrentProgram:           return handleSetCurrentProgram (args);
            case plugInOpcodeGetCurrentProgram:           return handleGetCurrentProgram (args);
            case plugInOpcodeSetCurrentProgramName:       return handleSetCurrentProgramName (args);
            case plugInOpcodeGetCurrentProgramName:       return handleGetCurrentProgramName (args);
            case plugInOpcodeGetParameterLabel:           return handleGetParameterLabel (args);
            case plugInOpcodeGetParameterText:            return handleGetParameterText (args);
            case plugInOpcodeGetParameterName:            return handleGetParameterName (args);
            case plugInOpcodeSetSampleRate:               return handleSetSampleRate (args);
            case plugInOpcodeSetBlockSize:                return handleSetBlockSize (args);
            case plugInOpcodeResumeSuspend:               return handleResumeSuspend (args);
            case plugInOpcodeGetEditorBounds:             return handleGetEditorBounds (args);
            case plugInOpcodeOpenEditor:                  return handleOpenEditor (args);
            case plugInOpcodeCloseEditor:                 return handleCloseEditor (args);
            case plugInOpcodeIdentify:                    return (pointer_sized_int) ByteOrder::bigEndianInt ("NvEf");
            case plugInOpcodeGetData:                     return handleGetData (args);
            case plugInOpcodeSetData:                     return handleSetData (args);
            case plugInOpcodePreAudioProcessingEvents:    return handlePreAudioProcessingEvents (args);
            case plugInOpcodeIsParameterAutomatable:      return handleIsParameterAutomatable (args);
            case plugInOpcodeParameterValueForText:       return handleParameterValueForText (args);
            case plugInOpcodeGetProgramName:              return handleGetProgramName (args);
            case plugInOpcodeGetInputPinProperties:       return handleGetInputPinProperties (args);
            case plugInOpcodeGetOutputPinProperties:      return handleGetOutputPinProperties (args);
            case plugInOpcodeGetPlugInCategory:           return handleGetPlugInCategory (args);
            case plugInOpcodeSetSpeakerConfiguration:     return handleSetSpeakerConfiguration (args);
            case plugInOpcodeSetBypass:                   return handleSetBypass (args);
            case plugInOpcodeGetPlugInName:               return handleGetPlugInName (args);
            case plugInOpcodeGetManufacturerProductName:  return handleGetPlugInName (args);
            case plugInOpcodeGetManufacturerName:         return handleGetManufacturerName (args);
            case plugInOpcodeGetManufacturerVersion:      return handleGetManufacturerVersion (args);
            case plugInOpcodeManufacturerSpecific:        return handleManufacturerSpecific (args);
            case plugInOpcodeCanPlugInDo:                 return handleCanPlugInDo (args);
            case plugInOpcodeGetTailSize:                 return handleGetTailSize (args);
            case plugInOpcodeKeyboardFocusRequired:       return handleKeyboardFocusRequired (args);
            case plugInOpcodeGetVstInterfaceVersion:      return handleGetVstInterfaceVersion (args);
            case plugInOpcodeGetCurrentMidiProgram:       return handleGetCurrentMidiProgram (args);
            case plugInOpcodeGetSpeakerArrangement:       return handleGetSpeakerConfiguration (args);
            case plugInOpcodeSetNumberOfSamplesToProcess: return handleSetNumberOfSamplesToProcess (args);
            case plugInOpcodeSetSampleFloatType:          return handleSetSampleFloatType (args);
            case pluginOpcodeGetNumMidiInputChannels:     return handleGetNumMidiInputChannels();
            case pluginOpcodeGetNumMidiOutputChannels:    return handleGetNumMidiOutputChannels();
            default:                                      return 0;
        }
    }

    static pointer_sized_int dispatcherCB (VstEffectInterface* vstInterface, int32 opCode, int32 index,
                                           pointer_sized_int value, void* ptr, float opt)
    {
        auto* wrapper = getWrapper (vstInterface);
        VstOpCodeArguments args = { index, value, ptr, opt };

        if (opCode == plugInOpcodeClose)
        {
            wrapper->dispatcher (opCode, args);
            delete wrapper;
            return 1;
        }

        return wrapper->dispatcher (opCode, args);
    }

    //==============================================================================
    // A component to hold the AudioProcessorEditor, and cope with some housekeeping
    // chores when it changes or repaints.
    struct EditorCompWrapper  : public Component
    {
        EditorCompWrapper (JuceVSTWrapper& w, AudioProcessorEditor& editor)  : wrapper (w)
        {
            editor.setOpaque (true);
            editor.setVisible (true);
            setOpaque (true);

            setTopLeftPosition (editor.getPosition());
            editor.setTopLeftPosition (0, 0);
            auto b = getLocalArea (&editor, editor.getLocalBounds());
            setSize (b.getWidth(), b.getHeight());

            addAndMakeVisible (editor);

           #if JUCE_WINDOWS
            if (! getHostType().isReceptor())
                addMouseListener (this, true);
           #endif

            ignoreUnused (fakeMouseGenerator);
        }

        ~EditorCompWrapper()
        {
            deleteAllChildren(); // note that we can't use a ScopedPointer because the editor may
                                 // have been transferred to another parent which takes over ownership.
        }

        void paint (Graphics&) override {}

        void getEditorBounds (VstEditorBounds& bounds)
        {
            auto b = getSizeToContainChild();

            bounds.upper     = 0;
            bounds.leftmost  = 0;
            bounds.lower     = (int16) b.getHeight();
            bounds.rightmost = (int16) b.getWidth();
        }

        void attachToHost (VstOpCodeArguments args)
        {
            setOpaque (true);
            setVisible (false);

           #if JUCE_WINDOWS
            addToDesktop (0, args.ptr);
            hostWindow = (HWND) args.ptr;
           #elif JUCE_LINUX
            addToDesktop (0, args.ptr);
            hostWindow = (Window) args.ptr;
            XReparentWindow (display.display, (Window) getWindowHandle(), hostWindow, 0, 0);
           #else
            hostWindow = attachComponentToWindowRefVST (this, args.ptr, wrapper.useNSView);
           #endif

            setVisible (true);
        }

        void detachHostWindow()
        {
           #if JUCE_MAC
            if (hostWindow != 0)
            {
                detachComponentFromWindowRefVST (this, hostWindow, wrapper.useNSView);
                hostWindow = 0;
            }
           #endif

           #if JUCE_LINUX
            hostWindow = 0;
           #endif
        }

        void checkVisibility()
        {
           #if JUCE_MAC
            if (hostWindow != 0)
                checkWindowVisibilityVST (hostWindow, this, wrapper.useNSView);
           #endif
        }

        AudioProcessorEditor* getEditorComp() const noexcept
        {
            return dynamic_cast<AudioProcessorEditor*> (getChildComponent(0));
        }

        void resized() override
        {
            if (auto* ed = getEditorComp())
            {
                ed->setTopLeftPosition (0, 0);
                ed->setBounds (ed->getLocalArea (this, getLocalBounds()));

                if (! getHostType().isBitwigStudio())
                    updateWindowSize();
            }

           #if JUCE_MAC && ! JUCE_64BIT
            if (! wrapper.useNSView)
                updateEditorCompBoundsVST (this);
           #endif
        }

        void childBoundsChanged (Component*) override
        {
            updateWindowSize();
        }

        juce::Rectangle<int> getSizeToContainChild()
        {
            if (auto* ed = getEditorComp())
                return getLocalArea (ed, ed->getLocalBounds());

            return {};
        }

        void updateWindowSize()
        {
            if (! isInSizeWindow)
            {
                if (auto* ed = getEditorComp())
                {
                    ed->setTopLeftPosition (0, 0);
                    auto pos = getSizeToContainChild();

                   #if JUCE_MAC
                    if (wrapper.useNSView)
                        setTopLeftPosition (0, getHeight() - pos.getHeight());
                   #endif

                    resizeHostWindow (pos.getWidth(), pos.getHeight());

                   #if ! JUCE_LINUX // setSize() on linux causes renoise and energyxt to fail.
                    setSize (pos.getWidth(), pos.getHeight());
                   #else
                    XResizeWindow (display.display, (Window) getWindowHandle(), pos.getWidth(), pos.getHeight());
                   #endif

                   #if JUCE_MAC
                    resizeHostWindow (pos.getWidth(), pos.getHeight()); // (doing this a second time seems to be necessary in tracktion)
                   #endif
                }
            }
        }

        void resizeHostWindow (int newWidth, int newHeight)
        {
            bool sizeWasSuccessful = false;

            if (auto host = wrapper.hostCallback)
            {
                auto status = host (wrapper.getVstEffectInterface(), hostOpcodeCanHostDo, 0, 0, const_cast<char*> ("sizeWindow"), 0);

                if (status == (pointer_sized_int) 1 || getHostType().isAbletonLive())
                {
                    isInSizeWindow = true;
                    sizeWasSuccessful = (host (wrapper.getVstEffectInterface(), hostOpcodeWindowSize, newWidth, newHeight, 0, 0) != 0);
                    isInSizeWindow = false;
                }
            }

            if (! sizeWasSuccessful)
            {
                // some hosts don't support the sizeWindow call, so do it manually..
               #if JUCE_MAC
                setNativeHostWindowSizeVST (hostWindow, this, newWidth, newHeight, wrapper.useNSView);

               #elif JUCE_LINUX
                // (Currently, all linux hosts support sizeWindow, so this should never need to happen)
                setSize (newWidth, newHeight);

               #else
                int dw = 0;
                int dh = 0;
                const int frameThickness = GetSystemMetrics (SM_CYFIXEDFRAME);

                HWND w = (HWND) getWindowHandle();

                while (w != 0)
                {
                    HWND parent = getWindowParent (w);

                    if (parent == 0)
                        break;

                    TCHAR windowType [32] = { 0 };
                    GetClassName (parent, windowType, 31);

                    if (String (windowType).equalsIgnoreCase ("MDIClient"))
                        break;

                    RECT windowPos, parentPos;
                    GetWindowRect (w, &windowPos);
                    GetWindowRect (parent, &parentPos);

                    SetWindowPos (w, 0, 0, 0, newWidth + dw, newHeight + dh,
                                  SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOOWNERZORDER);

                    dw = (parentPos.right - parentPos.left) - (windowPos.right - windowPos.left);
                    dh = (parentPos.bottom - parentPos.top) - (windowPos.bottom - windowPos.top);

                    w = parent;

                    if (dw == 2 * frameThickness)
                        break;

                    if (dw > 100 || dh > 100)
                        w = 0;
                }

                if (w != 0)
                    SetWindowPos (w, 0, 0, 0, newWidth + dw, newHeight + dh,
                                  SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOOWNERZORDER);
               #endif
            }

            if (auto* peer = getPeer())
            {
                peer->handleMovedOrResized();
                repaint();
            }
        }

       #if JUCE_WINDOWS
        void mouseDown (const MouseEvent&) override
        {
            broughtToFront();
        }

        void broughtToFront() override
        {
            // for hosts like nuendo, need to also pop the MDI container to the
            // front when our comp is clicked on.
            if (! isCurrentlyBlockedByAnotherModalComponent())
                if (HWND parent = findMDIParentOf ((HWND) getWindowHandle()))
                    SetWindowPos (parent, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
        }
       #endif

       #if JUCE_MAC
        bool keyPressed (const KeyPress&) override
        {
            // If we have an unused keypress, move the key-focus to a host window
            // and re-inject the event..
            return forwardCurrentKeyEventToHostVST (this, wrapper.useNSView);
        }
       #endif

        //==============================================================================
        JuceVSTWrapper& wrapper;
        FakeMouseMoveGenerator fakeMouseGenerator;
        bool isInSizeWindow = false;

       #if JUCE_MAC
        void* hostWindow = {};
       #elif JUCE_LINUX
        ScopedXDisplay display;
        Window hostWindow = {};
       #else
        HWND hostWindow = {};
        WindowsHooks hooks;
       #endif

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EditorCompWrapper)
    };

    //==============================================================================
private:
    VstHostCallback hostCallback;
    AudioProcessor* processor = {};
    double sampleRate = 44100.0;
    int32 blockSize = 1024;
    VstEffectInterface vstEffect;
    juce::MemoryBlock chunkMemory;
    juce::uint32 chunkMemoryTime = 0;
    ScopedPointer<EditorCompWrapper> editorComp;
    VstEditorBounds editorBounds;
    MidiBuffer midiEvents;
    VSTMidiEventList outgoingEvents;
    float editorScaleFactor = 1.0f;

    bool isProcessing = false, isBypassed = false, hasShutdown = false;
    bool firstProcessCallback = true, shouldDeleteEditor = false;

   #if JUCE_64BIT
    bool useNSView = true;
   #else
    bool useNSView = false;
   #endif

    VstTempBuffers<float> floatTempBuffers;
    VstTempBuffers<double> doubleTempBuffers;
    int maxNumInChannels = 0, maxNumOutChannels = 0;

    HeapBlock<VstSpeakerConfiguration> cachedInArrangement, cachedOutArrangement;

    static JuceVSTWrapper* getWrapper (VstEffectInterface* v) noexcept  { return static_cast<JuceVSTWrapper*> (v->effectPointer); }

    bool isProcessLevelOffline()
    {
        return hostCallback != nullptr
                && (int32) hostCallback (&vstEffect, hostOpcodeGetCurrentAudioProcessingLevel, 0, 0, 0, 0) == 4;
    }

    static inline int32 convertHexVersionToDecimal (const unsigned int hexVersion)
    {
       #if JUCE_VST_RETURN_HEX_VERSION_NUMBER_DIRECTLY
        return (int32) hexVersion;
       #else
        return (int32) (((hexVersion >> 24) & 0xff) * 1000
                         + ((hexVersion >> 16) & 0xff) * 100
                         + ((hexVersion >> 8)  & 0xff) * 10
                         + (hexVersion & 0xff));
       #endif
    }

    //==============================================================================
   #if JUCE_WINDOWS
    // Workarounds for hosts which attempt to open editor windows on a non-GUI thread.. (Grrrr...)
    static void checkWhetherMessageThreadIsCorrect()
    {
        auto host = getHostType();

        if (host.isWavelab() || host.isCubaseBridged() || host.isPremiere())
        {
            if (! messageThreadIsDefinitelyCorrect)
            {
                MessageManager::getInstance()->setCurrentThreadAsMessageThread();

                struct MessageThreadCallback  : public CallbackMessage
                {
                    MessageThreadCallback (bool& tr) : triggered (tr) {}
                    void messageCallback() override     { triggered = true; }

                    bool& triggered;
                };

                (new MessageThreadCallback (messageThreadIsDefinitelyCorrect))->post();
            }
        }
    }
   #else
    static void checkWhetherMessageThreadIsCorrect() {}
   #endif

    //==============================================================================
    template <typename FloatType>
    void deleteTempChannels (VstTempBuffers<FloatType>& tmpBuffers)
    {
        tmpBuffers.release();

        if (processor != nullptr)
            tmpBuffers.tempChannels.insertMultiple (0, nullptr, vstEffect.numInputChannels
                                                                 + vstEffect.numOutputChannels);
    }

    void deleteTempChannels()
    {
        deleteTempChannels (floatTempBuffers);
        deleteTempChannels (doubleTempBuffers);
    }

    //==============================================================================
    void findMaxTotalChannels (int& maxTotalIns, int& maxTotalOuts)
    {
       #ifdef JucePlugin_PreferredChannelConfigurations
        int configs[][2] = { JucePlugin_PreferredChannelConfigurations };
        maxTotalIns = maxTotalOuts = 0;

        for (auto& config : configs)
        {
            maxTotalIns =  jmax (maxTotalIns,  config[0]);
            maxTotalOuts = jmax (maxTotalOuts, config[1]);
        }
       #else
        auto numInputBuses  = processor->getBusCount (true);
        auto numOutputBuses = processor->getBusCount (false);

        if (numInputBuses > 1 || numOutputBuses > 1)
        {
            maxTotalIns = maxTotalOuts = 0;

            for (int i = 0; i < numInputBuses; ++i)
                maxTotalIns  += processor->getChannelCountOfBus (true, i);

            for (int i = 0; i < numOutputBuses; ++i)
                maxTotalOuts += processor->getChannelCountOfBus (false, i);
        }
        else
        {
            maxTotalIns  = numInputBuses  > 0 ? processor->getBus (true,  0)->getMaxSupportedChannels (64) : 0;
            maxTotalOuts = numOutputBuses > 0 ? processor->getBus (false, 0)->getMaxSupportedChannels (64) : 0;
        }
       #endif
    }

    bool pluginHasSidechainsOrAuxs() const  { return (processor->getBusCount (true) > 1 || processor->getBusCount (false) > 1); }

    //==============================================================================
    /** Host to plug-in calls. */

    pointer_sized_int handleOpen (VstOpCodeArguments)
    {
        // Note: most hosts call this on the UI thread, but wavelab doesn't, so be careful in here.
        if (processor->hasEditor())
            vstEffect.flags |= vstEffectFlagHasEditor;
        else
            vstEffect.flags &= ~vstEffectFlagHasEditor;

        return 0;
    }

    pointer_sized_int handleClose (VstOpCodeArguments)
    {
        // Note: most hosts call this on the UI thread, but wavelab doesn't, so be careful in here.
        stopTimer();

        if (MessageManager::getInstance()->isThisTheMessageThread())
            deleteEditor (false);

        return 0;
    }

    pointer_sized_int handleSetCurrentProgram (VstOpCodeArguments args)
    {
        if (processor != nullptr && isPositiveAndBelow ((int) args.value, processor->getNumPrograms()))
            processor->setCurrentProgram ((int) args.value);

        return 0;
    }

    pointer_sized_int handleGetCurrentProgram (VstOpCodeArguments)
    {
        return (processor != nullptr && processor->getNumPrograms() > 0 ? processor->getCurrentProgram() : 0);
    }

    pointer_sized_int handleSetCurrentProgramName (VstOpCodeArguments args)
    {
        if (processor != nullptr && processor->getNumPrograms() > 0)
            processor->changeProgramName (processor->getCurrentProgram(), (char*) args.ptr);

        return 0;
    }

    pointer_sized_int handleGetCurrentProgramName (VstOpCodeArguments args)
    {
        if (processor != nullptr && processor->getNumPrograms() > 0)
            processor->getProgramName (processor->getCurrentProgram()).copyToUTF8 ((char*) args.ptr, 24 + 1);

        return 0;
    }

    pointer_sized_int handleGetParameterLabel (VstOpCodeArguments args)
    {
        if (processor != nullptr)
        {
            jassert (isPositiveAndBelow (args.index, processor->getNumParameters()));
            // length should technically be kVstMaxParamStrLen, which is 8, but hosts will normally allow a bit more.
            processor->getParameterLabel (args.index).copyToUTF8 ((char*) args.ptr, 24 + 1);
        }

        return 0;
    }

    pointer_sized_int handleGetParameterText (VstOpCodeArguments args)
    {
        if (processor != nullptr)
        {
            jassert (isPositiveAndBelow (args.index, processor->getNumParameters()));
            // length should technically be kVstMaxParamStrLen, which is 8, but hosts will normally allow a bit more.
            processor->getParameterText (args.index, 24).copyToUTF8 ((char*) args.ptr, 24 + 1);
        }

        return 0;
    }

    pointer_sized_int handleGetParameterName (VstOpCodeArguments args)
    {
        if (processor != nullptr)
        {
            jassert (isPositiveAndBelow (args.index, processor->getNumParameters()));
            // length should technically be kVstMaxParamStrLen, which is 8, but hosts will normally allow a bit more.
            processor->getParameterName (args.index, 32).copyToUTF8 ((char*) args.ptr, 32 + 1);
        }

        return 0;
    }

    pointer_sized_int handleSetSampleRate (VstOpCodeArguments args)
    {
        sampleRate = args.opt;
        return 0;
    }

    pointer_sized_int handleSetBlockSize (VstOpCodeArguments args)
    {
        blockSize = (int32) args.value;
        return 0;
    }

    pointer_sized_int handleResumeSuspend (VstOpCodeArguments args)
    {
        if (args.value)
            resume();
        else
            suspend();

        return 0;
    }

    pointer_sized_int handleGetEditorBounds (VstOpCodeArguments args)
    {
        checkWhetherMessageThreadIsCorrect();
        const MessageManagerLock mmLock;
        createEditorComp();

        if (editorComp != nullptr)
        {
            editorComp->getEditorBounds (editorBounds);
            *((VstEditorBounds**) args.ptr) = &editorBounds;
            return (pointer_sized_int) &editorBounds;
        }

        return 0;
    }

    pointer_sized_int handleOpenEditor (VstOpCodeArguments args)
    {
        checkWhetherMessageThreadIsCorrect();
        const MessageManagerLock mmLock;
        jassert (! recursionCheck);

        startTimerHz (4); // performs misc housekeeping chores

        deleteEditor (true);
        createEditorComp();

        if (editorComp != nullptr)
        {
            editorComp->attachToHost (args);
            return 1;
        }

        return 0;
    }

    pointer_sized_int handleCloseEditor (VstOpCodeArguments)
    {
        checkWhetherMessageThreadIsCorrect();
        const MessageManagerLock mmLock;
        deleteEditor (true);
        return 0;
    }

    pointer_sized_int handleGetData (VstOpCodeArguments args)
    {
        if (processor == nullptr)
            return 0;

        auto data = (void**) args.ptr;
        bool onlyStoreCurrentProgramData = (args.index != 0);

        chunkMemory.reset();
        if (onlyStoreCurrentProgramData)
            processor->getCurrentProgramStateInformation (chunkMemory);
        else
            processor->getStateInformation (chunkMemory);

        *data = (void*) chunkMemory.getData();

        // because the chunk is only needed temporarily by the host (or at least you'd
        // hope so) we'll give it a while and then free it in the timer callback.
        chunkMemoryTime = juce::Time::getApproximateMillisecondCounter();

        return (int32) chunkMemory.getSize();
    }

    pointer_sized_int handleSetData (VstOpCodeArguments args)
    {
        if (processor != nullptr)
        {
            void* data = args.ptr;
            int32 byteSize = (int32) args.value;
            bool onlyRestoreCurrentProgramData = (args.index != 0);

            chunkMemory.reset();
            chunkMemoryTime = 0;

            if (byteSize > 0 && data != nullptr)
            {
                if (onlyRestoreCurrentProgramData)
                    processor->setCurrentProgramStateInformation (data, byteSize);
                else
                    processor->setStateInformation (data, byteSize);
            }
        }

        return 0;
    }

    pointer_sized_int handlePreAudioProcessingEvents (VstOpCodeArguments args)
    {
       #if JucePlugin_WantsMidiInput || JucePlugin_IsMidiEffect
        VSTMidiEventList::addEventsToMidiBuffer ((VstEventBlock*) args.ptr, midiEvents);
        return 1;
       #else
        ignoreUnused (args);
        return 0;
       #endif
    }

    pointer_sized_int handleIsParameterAutomatable (VstOpCodeArguments args)
    {
        if (processor == nullptr)
            return 0;

        const bool isMeter = (((processor->getParameterCategory (args.index) & 0xffff0000) >> 16) == 2);
        return (processor->isParameterAutomatable (args.index) && (! isMeter) ? 1 : 0);
    }

    pointer_sized_int handleParameterValueForText (VstOpCodeArguments args)
    {
        if (processor != nullptr)
        {
            jassert (isPositiveAndBelow (args.index, processor->getNumParameters()));

            if (auto* p = processor->getParameters()[args.index])
            {
                processor->setParameter (args.index, p->getValueForText (String::fromUTF8 ((char*) args.ptr)));
                return 1;
            }
        }

        return 0;
    }

    pointer_sized_int handleGetProgramName (VstOpCodeArguments args)
    {
        if (processor != nullptr && isPositiveAndBelow (args.index, processor->getNumPrograms()))
        {
            processor->getProgramName (args.index).copyToUTF8 ((char*) args.ptr, 24 + 1);
            return 1;
        }

        return 0;
    }

    pointer_sized_int handleGetInputPinProperties (VstOpCodeArguments args)
    {
        return (processor != nullptr && getPinProperties (*(VstPinInfo*) args.ptr, true, args.index)) ? 1 : 0;
    }

    pointer_sized_int handleGetOutputPinProperties (VstOpCodeArguments args)
    {
        return (processor != nullptr && getPinProperties (*(VstPinInfo*) args.ptr, false, args.index)) ? 1 : 0;
    }

    pointer_sized_int handleGetPlugInCategory (VstOpCodeArguments)
    {
        return JucePlugin_VSTCategory;
    }

    pointer_sized_int handleSetSpeakerConfiguration (VstOpCodeArguments args)
    {
        auto* pluginInput  = reinterpret_cast<VstSpeakerConfiguration*> (args.value);
        auto* pluginOutput = reinterpret_cast<VstSpeakerConfiguration*> (args.ptr);

        if (processor->isMidiEffect())
            return 0;

        auto numIns  = processor->getBusCount (true);
        auto numOuts = processor->getBusCount (false);

        if (pluginInput != nullptr && pluginInput->type >= 0)
        {
            // inconsistent request?
            if (SpeakerMappings::vstArrangementTypeToChannelSet (*pluginInput).size() != pluginInput->numberOfChannels)
                return 0;
        }

        if (pluginOutput != nullptr && pluginOutput->type >= 0)
        {
            // inconsistent request?
            if (SpeakerMappings::vstArrangementTypeToChannelSet (*pluginOutput).size() != pluginOutput->numberOfChannels)
                return 0;
        }

        if (pluginInput != nullptr  && pluginInput->numberOfChannels  > 0 && numIns  == 0)
            return 0;

        if (pluginOutput != nullptr && pluginOutput->numberOfChannels > 0 && numOuts == 0)
            return 0;

        auto layouts = processor->getBusesLayout();

        if (pluginInput != nullptr && pluginInput-> numberOfChannels >= 0 && numIns  > 0)
            layouts.getChannelSet (true,  0) = SpeakerMappings::vstArrangementTypeToChannelSet (*pluginInput);

        if (pluginOutput != nullptr && pluginOutput->numberOfChannels >= 0 && numOuts > 0)
            layouts.getChannelSet (false, 0) = SpeakerMappings::vstArrangementTypeToChannelSet (*pluginOutput);

       #ifdef JucePlugin_PreferredChannelConfigurations
        short configs[][2] = { JucePlugin_PreferredChannelConfigurations };
        if (! AudioProcessor::containsLayout (layouts, configs))
            return 0;
       #endif

        return processor->setBusesLayout (layouts) ? 1 : 0;
    }

    pointer_sized_int handleSetBypass (VstOpCodeArguments args)
    {
        isBypassed = (args.value != 0);
        return 1;
    }

    pointer_sized_int handleGetPlugInName (VstOpCodeArguments args)
    {
        String (JucePlugin_Name).copyToUTF8 ((char*) args.ptr, 64 + 1);
        return 1;
    }

    pointer_sized_int handleGetManufacturerName (VstOpCodeArguments args)
    {
        String (JucePlugin_Manufacturer).copyToUTF8 ((char*) args.ptr, 64 + 1);
        return 1;
    }

    pointer_sized_int handleGetManufacturerVersion (VstOpCodeArguments)
    {
        return convertHexVersionToDecimal (JucePlugin_VersionCode);
    }

    pointer_sized_int handleManufacturerSpecific (VstOpCodeArguments args)
    {
        if (handleManufacturerSpecificVST2Opcode (args.index, args.value, args.ptr, args.opt))
            return 1;

        if (args.index == presonusVendorID && args.value == presonusSetContentScaleFactor)
            return handleSetContentScaleFactor (args.opt);

        if (auto callbackHandler = dynamic_cast<VSTCallbackHandler*> (processor))
            return callbackHandler->handleVstManufacturerSpecific (args.index, args.value, args.ptr, args.opt);

        return 0;
    }

    pointer_sized_int handleCanPlugInDo (VstOpCodeArguments args)
    {
        auto text = (const char*) args.ptr;
        auto matches = [=](const char* s) { return strcmp (text, s) == 0; };

        if (matches ("receiveVstEvents")
             || matches ("receiveVstMidiEvent")
             || matches ("receiveVstMidiEvents"))
        {
           #if JucePlugin_WantsMidiInput || JucePlugin_IsMidiEffect
            return 1;
           #else
            return -1;
           #endif
        }

        if (matches ("sendVstEvents")
             || matches ("sendVstMidiEvent")
             || matches ("sendVstMidiEvents"))
        {
           #if JucePlugin_ProducesMidiOutput || JucePlugin_IsMidiEffect
            return 1;
           #else
            return -1;
           #endif
        }

        if (matches ("receiveVstTimeInfo")
             || matches ("conformsToWindowRules")
             || matches ("supportsViewDpiScaling")
             || matches ("bypass"))
        {
            return 1;
        }

        // This tells Wavelab to use the UI thread to invoke open/close,
        // like all other hosts do.
        if (matches ("openCloseAnyThread"))
            return -1;

        if (matches ("MPE"))
            return processor->supportsMPE() ? 1 : 0;

       #if JUCE_MAC
        if (matches ("hasCockosViewAsConfig"))
        {
            useNSView = true;
            return (int32) 0xbeef0000;
        }
       #endif

        return 0;
    }

    pointer_sized_int handleGetTailSize (VstOpCodeArguments)
    {
        if (processor != nullptr)
            return (pointer_sized_int) (processor->getTailLengthSeconds() * sampleRate);

        return 0;
    }

    pointer_sized_int handleKeyboardFocusRequired (VstOpCodeArguments)
    {
        return (JucePlugin_EditorRequiresKeyboardFocus != 0) ? 1 : 0;
    }

    pointer_sized_int handleGetVstInterfaceVersion (VstOpCodeArguments)
    {
        return juceVstInterfaceVersion;
    }

    pointer_sized_int handleGetCurrentMidiProgram (VstOpCodeArguments)
    {
        return -1;
    }

    pointer_sized_int handleGetSpeakerConfiguration (VstOpCodeArguments args)
    {
        auto** pluginInput  = reinterpret_cast<VstSpeakerConfiguration**> (args.value);
        auto** pluginOutput = reinterpret_cast<VstSpeakerConfiguration**> (args.ptr);

        if (pluginHasSidechainsOrAuxs() || processor->isMidiEffect())
            return false;

        auto inputLayout  = processor->getChannelLayoutOfBus (true,  0);
        auto outputLayout = processor->getChannelLayoutOfBus (false,  0);

        auto speakerBaseSize = sizeof (VstSpeakerConfiguration) - (sizeof (VstIndividualSpeakerInfo) * 8);

        cachedInArrangement .malloc (speakerBaseSize + (static_cast<std::size_t> (inputLayout. size()) * sizeof (VstSpeakerConfiguration)), 1);
        cachedOutArrangement.malloc (speakerBaseSize + (static_cast<std::size_t> (outputLayout.size()) * sizeof (VstSpeakerConfiguration)), 1);

        *pluginInput  = cachedInArrangement. getData();
        *pluginOutput = cachedOutArrangement.getData();

        SpeakerMappings::channelSetToVstArrangement (processor->getChannelLayoutOfBus (true,  0), **pluginInput);
        SpeakerMappings::channelSetToVstArrangement (processor->getChannelLayoutOfBus (false, 0), **pluginOutput);

        return 1;
    }

    pointer_sized_int handleSetNumberOfSamplesToProcess (VstOpCodeArguments args)
    {
        return args.value;
    }

    pointer_sized_int handleSetSampleFloatType (VstOpCodeArguments args)
    {
        if (! isProcessing)
        {
            if (processor != nullptr)
            {
                processor->setProcessingPrecision ((args.value == vstProcessingSampleTypeDouble
                                                     && processor->supportsDoublePrecisionProcessing())
                                                         ? AudioProcessor::doublePrecision
                                                         : AudioProcessor::singlePrecision);

                return 1;
            }
        }

        return 0;
    }

    pointer_sized_int handleSetContentScaleFactor (float scale)
    {
        if (editorScaleFactor != scale)
        {
            editorScaleFactor = scale;

           #if ! (JUCE_MAC || JUCE_IOS)
            if (editorComp != nullptr)
            {
                if (auto* ed = editorComp->getEditorComp())
                    ed->setScaleFactor (editorScaleFactor);

                if (editorComp != nullptr)
                    editorComp->updateWindowSize();
            }
           #endif
        }

        return 1;
    }

    //==============================================================================
    pointer_sized_int handleGetNumMidiInputChannels()
    {
       #if JucePlugin_WantsMidiInput || JucePlugin_IsMidiEffect
        return 16;
       #else
        return 0;
       #endif
    }

    pointer_sized_int handleGetNumMidiOutputChannels()
    {
       #if JucePlugin_ProducesMidiOutput || JucePlugin_IsMidiEffect
        return 16;
       #else
        return 0;
       #endif
    }

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (JuceVSTWrapper)
};


//==============================================================================
namespace
{
    VstEffectInterface* pluginEntryPoint (VstHostCallback audioMaster)
    {
        JUCE_AUTORELEASEPOOL
        {
            initialiseJuce_GUI();

            try
            {
                if (audioMaster (0, hostOpcodeVstVersion, 0, 0, 0, 0) != 0)
                {
                   #if JUCE_LINUX
                    MessageManagerLock mmLock;
                   #endif

                    auto* processor = createPluginFilterOfType (AudioProcessor::wrapperType_VST);
                    auto* wrapper = new JuceVSTWrapper (audioMaster, processor);
                    return wrapper->getVstEffectInterface();
                }
            }
            catch (...)
            {}
        }

        return nullptr;
    }
}

#if ! JUCE_WINDOWS
 #define JUCE_EXPORTED_FUNCTION extern "C" __attribute__ ((visibility("default")))
#endif

//==============================================================================
// Mac startup code..
#if JUCE_MAC

    JUCE_EXPORTED_FUNCTION VstEffectInterface* VSTPluginMain (VstHostCallback audioMaster);
    JUCE_EXPORTED_FUNCTION VstEffectInterface* VSTPluginMain (VstHostCallback audioMaster)
    {
        PluginHostType::jucePlugInClientCurrentWrapperType = AudioProcessor::wrapperType_VST;

        initialiseMacVST();
        return pluginEntryPoint (audioMaster);
    }

    JUCE_EXPORTED_FUNCTION VstEffectInterface* main_macho (VstHostCallback audioMaster);
    JUCE_EXPORTED_FUNCTION VstEffectInterface* main_macho (VstHostCallback audioMaster)
    {
        PluginHostType::jucePlugInClientCurrentWrapperType = AudioProcessor::wrapperType_VST;

        initialiseMacVST();
        return pluginEntryPoint (audioMaster);
    }

//==============================================================================
// Linux startup code..
#elif JUCE_LINUX

    JUCE_EXPORTED_FUNCTION VstEffectInterface* VSTPluginMain (VstHostCallback audioMaster);
    JUCE_EXPORTED_FUNCTION VstEffectInterface* VSTPluginMain (VstHostCallback audioMaster)
    {
        PluginHostType::jucePlugInClientCurrentWrapperType = AudioProcessor::wrapperType_VST;

        SharedMessageThread::getInstance();
        return pluginEntryPoint (audioMaster);
    }

    JUCE_EXPORTED_FUNCTION VstEffectInterface* main_plugin (VstHostCallback audioMaster) asm ("main");
    JUCE_EXPORTED_FUNCTION VstEffectInterface* main_plugin (VstHostCallback audioMaster)
    {
        PluginHostType::jucePlugInClientCurrentWrapperType = AudioProcessor::wrapperType_VST;

        return VSTPluginMain (audioMaster);
    }

    // don't put initialiseJuce_GUI or shutdownJuce_GUI in these... it will crash!
    __attribute__((constructor)) void myPluginInit() {}
    __attribute__((destructor))  void myPluginFini() {}

//==============================================================================
// Win32 startup code..
#else

    extern "C" __declspec (dllexport) VstEffectInterface* VSTPluginMain (VstHostCallback audioMaster)
    {
        PluginHostType::jucePlugInClientCurrentWrapperType = AudioProcessor::wrapperType_VST;

        return pluginEntryPoint (audioMaster);
    }

   #ifndef JUCE_64BIT // (can't compile this on win64, but it's not needed anyway with VST2.4)
    extern "C" __declspec (dllexport) int main (VstHostCallback audioMaster)
    {
        PluginHostType::jucePlugInClientCurrentWrapperType = AudioProcessor::wrapperType_VST;

        return (int) pluginEntryPoint (audioMaster);
    }
   #endif

    extern "C" BOOL WINAPI DllMain (HINSTANCE instance, DWORD reason, LPVOID)
    {
        if (reason == DLL_PROCESS_ATTACH)
            Process::setCurrentModuleInstanceHandle (instance);

        return true;
    }
#endif

#endif

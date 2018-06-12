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
#include <juce_core/juce_core.h>

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

#define VST_FORCE_DEPRECATED 0

namespace Vst2
{
#include "../../juce_audio_processors/format_types/VST3_SDK/pluginterfaces/vst2.x/aeffect.h"
#include "../../juce_audio_processors/format_types/VST3_SDK/pluginterfaces/vst2.x/aeffectx.h"
}

using namespace juce;

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

#define JUCE_VSTINTERFACE_H_INCLUDED 1

#include "../utility/juce_IncludeModuleHeaders.h"
#include "../utility/juce_FakeMouseMoveGenerator.h"
#include "../utility/juce_WindowsHooks.h"

#include "../../juce_audio_processors/format_types/juce_LegacyAudioParameter.cpp"
#include "../../juce_audio_processors/format_types/juce_VSTCommon.h"

#if JUCE_BIG_ENDIAN
 #define JUCE_MULTICHAR_CONSTANT(a, b, c, d) (a | (((uint32) b) << 8) | (((uint32) c) << 16) | (((uint32) d) << 24))
#else
 #define JUCE_MULTICHAR_CONSTANT(a, b, c, d) (d | (((uint32) c) << 8) | (((uint32) b) << 16) | (((uint32) a) << 24))
#endif

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
    SharedMessageThread()  : Thread ("VstMessageThread")
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

    JUCE_DECLARE_SINGLETON (SharedMessageThread, false)

    bool initialised = false;
};

JUCE_IMPLEMENT_SINGLETON (SharedMessageThread)

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
                        private AsyncUpdater,
                        private AudioProcessorParameter::Listener
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
    JuceVSTWrapper (Vst2::audioMasterCallback cb, AudioProcessor* af)
       : hostCallback (cb),
         processor (af)
    {
        inParameterChangedCallback = false;

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

        if (auto* juceParam = processor->getBypassParameter())
            juceParam->addListener (this);

        juceParameters.update (*processor, false);

        memset (&vstEffect, 0, sizeof (vstEffect));
        vstEffect.magic = 0x56737450 /* 'VstP' */;
        vstEffect.dispatcher = (Vst2::AEffectDispatcherProc) dispatcherCB;
        vstEffect.process = nullptr;
        vstEffect.setParameter = (Vst2::AEffectSetParameterProc) setParameterCB;
        vstEffect.getParameter = (Vst2::AEffectGetParameterProc) getParameterCB;
        vstEffect.numPrograms = jmax (1, af->getNumPrograms());
        vstEffect.numParams = juceParameters.getNumParameters();
        vstEffect.numInputs = maxNumInChannels;
        vstEffect.numOutputs = maxNumOutChannels;
        vstEffect.initialDelay = processor->getLatencySamples();
        vstEffect.object = this;
        vstEffect.uniqueID = JucePlugin_VSTUniqueID;

       #ifdef JucePlugin_VSTChunkStructureVersion
        vstEffect.version = JucePlugin_VSTChunkStructureVersion;
       #else
        vstEffect.version = JucePlugin_VersionCode;
       #endif

        vstEffect.processReplacing = (Vst2::AEffectProcessProc) processReplacingCB;
        vstEffect.processDoubleReplacing = (Vst2::AEffectProcessDoubleProc) processDoubleReplacingCB;

        vstEffect.flags |= Vst2::effFlagsHasEditor;

        vstEffect.flags |= Vst2::effFlagsCanReplacing;
        if (processor->supportsDoublePrecisionProcessing())
            vstEffect.flags |= Vst2::effFlagsCanDoubleReplacing;

        vstEffect.flags |= Vst2::effFlagsProgramChunks;

       #if JucePlugin_IsSynth
        vstEffect.flags |= Vst2::effFlagsIsSynth;
       #else
        if (processor->getTailLengthSeconds() == 0.0)
            vstEffect.flags |= Vst2::effFlagsNoSoundInStop;
       #endif

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

    Vst2::AEffect* getAEffect() noexcept    { return &vstEffect; }

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
                hostCallback (&vstEffect, Vst2::audioMasterProcessEvents, 0, 0, outgoingEvents.events, 0);
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

    static void processReplacingCB (Vst2::AEffect* vstInterface, float** inputs, float** outputs, int32 sampleFrames)
    {
        getWrapper (vstInterface)->processReplacing (inputs, outputs, sampleFrames);
    }

    void processDoubleReplacing (double** inputs, double** outputs, int32 sampleFrames)
    {
        jassert (processor->isUsingDoublePrecision());
        internalProcessReplacing (inputs, outputs, sampleFrames, doubleTempBuffers);
    }

    static void processDoubleReplacingCB (Vst2::AEffect* vstInterface, double** inputs, double** outputs, int32 sampleFrames)
    {
        getWrapper (vstInterface)->processDoubleReplacing (inputs, outputs, sampleFrames);
    }

    //==============================================================================
    void resume()
    {
        if (processor != nullptr)
        {
            isProcessing = true;

            auto numInAndOutChannels = static_cast<size_t> (vstEffect.numInputs + vstEffect.numOutputs);
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

            vstEffect.initialDelay = processor->getLatencySamples();

            /** If this plug-in is a synth or it can receive midi events we need to tell the
                host that we want midi. In the SDK this method is marked as deprecated, but
                some hosts rely on this behaviour.
            */
            if (vstEffect.flags & Vst2::effFlagsIsSynth || JucePlugin_WantsMidiInput || JucePlugin_IsMidiEffect)
            {
                if (hostCallback != nullptr)
                    hostCallback (&vstEffect, Vst2::audioMasterWantMidi, 0, 1, 0, 0);
            }

            if (getHostType().isAbletonLive()
                 && hostCallback != nullptr
                 && processor->getTailLengthSeconds() == std::numeric_limits<double>::infinity())
            {
                AbletonLiveHostSpecific hostCmd;

                hostCmd.magic = 0x41624c69; // 'AbLi'
                hostCmd.cmd = 5;
                hostCmd.commandSize = sizeof (int);
                hostCmd.flags = AbletonLiveHostSpecific::KCantBeSuspended;

                hostCallback (&vstEffect, Vst2::audioMasterVendorSpecific, 0, 0, &hostCmd, 0.0f);
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
        const Vst2::VstTimeInfo* ti = nullptr;

        if (hostCallback != nullptr)
        {
            int32 flags = Vst2::kVstPpqPosValid | Vst2::kVstTempoValid
                              | Vst2::kVstBarsValid | Vst2::kVstCyclePosValid
                              | Vst2::kVstTimeSigValid | Vst2::kVstSmpteValid
                              | Vst2::kVstClockValid;

            auto result = hostCallback (&vstEffect, Vst2::audioMasterGetTime, 0, flags, 0, 0);
            ti = reinterpret_cast<Vst2::VstTimeInfo*> (result);
        }

        if (ti == nullptr || ti->sampleRate <= 0)
            return false;

        info.bpm = (ti->flags & Vst2::kVstTempoValid) != 0 ? ti->tempo : 0.0;

        if ((ti->flags & Vst2::kVstTimeSigValid) != 0)
        {
            info.timeSigNumerator   = ti->timeSigNumerator;
            info.timeSigDenominator = ti->timeSigDenominator;
        }
        else
        {
            info.timeSigNumerator   = 4;
            info.timeSigDenominator = 4;
        }

        info.timeInSamples = (int64) (ti->samplePos + 0.5);
        info.timeInSeconds = ti->samplePos / ti->sampleRate;
        info.ppqPosition = (ti->flags & Vst2::kVstPpqPosValid) != 0 ? ti->ppqPos : 0.0;
        info.ppqPositionOfLastBarStart = (ti->flags & Vst2::kVstBarsValid) != 0 ? ti->barStartPos : 0.0;

        if ((ti->flags & Vst2::kVstSmpteValid) != 0)
        {
            AudioPlayHead::FrameRateType rate = AudioPlayHead::fpsUnknown;
            double fps = 1.0;

            switch (ti->smpteFrameRate)
            {
                case Vst2::kVstSmpte239fps:       rate = AudioPlayHead::fps23976;    fps = 24.0 * 1000.0 / 1001.0; break;
                case Vst2::kVstSmpte24fps:        rate = AudioPlayHead::fps24;       fps = 24.0;  break;
                case Vst2::kVstSmpte25fps:        rate = AudioPlayHead::fps25;       fps = 25.0;  break;
                case Vst2::kVstSmpte2997fps:      rate = AudioPlayHead::fps2997;     fps = 30.0 * 1000.0 / 1001.0; break;
                case Vst2::kVstSmpte30fps:        rate = AudioPlayHead::fps30;       fps = 30.0;  break;
                case Vst2::kVstSmpte2997dfps:     rate = AudioPlayHead::fps2997drop; fps = 30.0 * 1000.0 / 1001.0; break;
                case Vst2::kVstSmpte30dfps:       rate = AudioPlayHead::fps30drop;   fps = 30.0;  break;

                case Vst2::kVstSmpteFilm16mm:
                case Vst2::kVstSmpteFilm35mm:     fps = 24.0; break;

                case Vst2::kVstSmpte249fps:       fps = 25.0 * 1000.0 / 1001.0; break;
                case Vst2::kVstSmpte599fps:       fps = 60.0 * 1000.0 / 1001.0; break;
                case Vst2::kVstSmpte60fps:        fps = 60; break;

                default:                          jassertfalse; // unknown frame-rate..
            }

            info.frameRate = rate;
            info.editOriginTime = ti->smpteOffset / (80.0 * fps);
        }
        else
        {
            info.frameRate = AudioPlayHead::fpsUnknown;
            info.editOriginTime = 0;
        }

        info.isRecording = (ti->flags & Vst2::kVstTransportRecording) != 0;
        info.isPlaying   = (ti->flags & (Vst2::kVstTransportRecording | Vst2::kVstTransportPlaying)) != 0;
        info.isLooping   = (ti->flags & Vst2::kVstTransportCycleActive) != 0;

        if ((ti->flags & Vst2::kVstCyclePosValid) != 0)
        {
            info.ppqLoopStart = ti->cycleStartPos;
            info.ppqLoopEnd   = ti->cycleEndPos;
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
        if (auto* param = juceParameters.getParamForIndex (index))
            return param->getValue();

        return 0.0f;
    }

    static float getParameterCB (Vst2::AEffect* vstInterface, int32 index)
    {
        return getWrapper (vstInterface)->getParameter (index);
    }

    void setParameter (int32 index, float value)
    {
        if (auto* param = juceParameters.getParamForIndex (index))
        {
            param->setValue (value);

            inParameterChangedCallback = true;
            param->sendValueChangedMessageToListeners (value);
        }
    }

    static void setParameterCB (Vst2::AEffect* vstInterface, int32 index, float value)
    {
        getWrapper (vstInterface)->setParameter (index, value);
    }

    void audioProcessorParameterChanged (AudioProcessor*, int index, float newValue) override
    {
        if (inParameterChangedCallback.get())
        {
            inParameterChangedCallback = false;
            return;
        }

        if (hostCallback != nullptr)
            hostCallback (&vstEffect, Vst2::audioMasterAutomate, index, 0, 0, newValue);
    }

    void audioProcessorParameterChangeGestureBegin (AudioProcessor*, int index) override
    {
        if (hostCallback != nullptr)
            hostCallback (&vstEffect, Vst2::audioMasterBeginEdit, index, 0, 0, 0);
    }

    void audioProcessorParameterChangeGestureEnd (AudioProcessor*, int index) override
    {
        if (hostCallback != nullptr)
            hostCallback (&vstEffect, Vst2::audioMasterEndEdit, index, 0, 0, 0);
    }

    void parameterValueChanged (int, float newValue) override
    {
        // this can only come from the bypass parameter
        isBypassed = (newValue != 0.0f);
    }

    void parameterGestureChanged (int, bool) override {}

    void audioProcessorChanged (AudioProcessor*) override
    {
        vstEffect.initialDelay = processor->getLatencySamples();

        if (hostCallback != nullptr)
            hostCallback (&vstEffect, Vst2::audioMasterUpdateDisplay, 0, 0, 0, 0);

        triggerAsyncUpdate();
    }

    void handleAsyncUpdate() override
    {
        if (hostCallback != nullptr)
            hostCallback (&vstEffect, Vst2::audioMasterIOChanged, 0, 0, 0, 0);
    }

    bool getPinProperties (Vst2::VstPinProperties& properties, bool direction, int index) const
    {
        if (processor->isMidiEffect())
            return false;

        int channelIdx, busIdx;

        // fill with default
        properties.flags = 0;
        properties.label[0] = 0;
        properties.shortLabel[0] = 0;
        properties.arrangementType = Vst2::kSpeakerArrEmpty;

        if ((channelIdx = processor->getOffsetInBusBufferForAbsoluteChannelIndex (direction, index, busIdx)) >= 0)
        {
            auto& bus = *processor->getBus (direction, busIdx);
            auto& channelSet = bus.getCurrentLayout();
            auto channelType = channelSet.getTypeOfChannel (channelIdx);

            properties.flags = Vst2::kVstPinIsActive | Vst2::kVstPinUseSpeaker;
            properties.arrangementType = SpeakerMappings::channelSetToVstArrangementType (channelSet);
            String label = bus.getName();

           #ifdef JucePlugin_PreferredChannelConfigurations
            label += " " + String (channelIdx);
           #else
            if (channelSet.size() > 1)
                label += " " + AudioChannelSet::getAbbreviatedChannelTypeName (channelType);
           #endif

            label.copyToUTF8 (properties.label, (size_t) (Vst2::kVstMaxLabelLen + 1));
            label.copyToUTF8 (properties.shortLabel, (size_t) (Vst2::kVstMaxShortLabelLen + 1));

            if (channelType == AudioChannelSet::left
                || channelType == AudioChannelSet::leftSurround
                || channelType == AudioChannelSet::leftCentre
                || channelType == AudioChannelSet::leftSurroundSide
                || channelType == AudioChannelSet::topFrontLeft
                || channelType == AudioChannelSet::topRearLeft
                || channelType == AudioChannelSet::leftSurroundRear
                || channelType == AudioChannelSet::wideLeft)
                properties.flags |= Vst2::kVstPinIsStereo;

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

        static AudioChannelSet vstArrangementTypeToChannelSet (const Vst2::VstSpeakerArrangement& arr)
        {
            if (arr.type == Vst2::kSpeakerArrEmpty)      return AudioChannelSet::disabled();
            if (arr.type == Vst2::kSpeakerArrMono)       return AudioChannelSet::mono();
            if (arr.type == Vst2::kSpeakerArrStereo)     return AudioChannelSet::stereo();
            if (arr.type == Vst2::kSpeakerArr30Cine)     return AudioChannelSet::createLCR();
            if (arr.type == Vst2::kSpeakerArr30Music)    return AudioChannelSet::createLRS();
            if (arr.type == Vst2::kSpeakerArr40Cine)     return AudioChannelSet::createLCRS();
            if (arr.type == Vst2::kSpeakerArr50)         return AudioChannelSet::create5point0();
            if (arr.type == Vst2::kSpeakerArr51)         return AudioChannelSet::create5point1();
            if (arr.type == Vst2::kSpeakerArr60Cine)     return AudioChannelSet::create6point0();
            if (arr.type == Vst2::kSpeakerArr61Cine)     return AudioChannelSet::create6point1();
            if (arr.type == Vst2::kSpeakerArr60Music)    return AudioChannelSet::create6point0Music();
            if (arr.type == Vst2::kSpeakerArr61Music)    return AudioChannelSet::create6point1Music();
            if (arr.type == Vst2::kSpeakerArr70Music)    return AudioChannelSet::create7point0();
            if (arr.type == Vst2::kSpeakerArr70Cine)     return AudioChannelSet::create7point0SDDS();
            if (arr.type == Vst2::kSpeakerArr71Music)    return AudioChannelSet::create7point1();
            if (arr.type == Vst2::kSpeakerArr71Cine)     return AudioChannelSet::create7point1SDDS();
            if (arr.type == Vst2::kSpeakerArr40Music)    return AudioChannelSet::quadraphonic();

            for (auto* m = getMappings(); m->vst2 != Vst2::kSpeakerArrEmpty; ++m)
            {
                if (m->vst2 == arr.type)
                {
                    AudioChannelSet s;

                    for (int i = 0; m->channels[i] != 0; ++i)
                        s.addChannel (m->channels[i]);

                    return s;
                }
            }

            return AudioChannelSet::discreteChannels (arr.numChannels);
        }

        static int32 channelSetToVstArrangementType (AudioChannelSet channels)
        {
            if (channels == AudioChannelSet::disabled())           return Vst2::kSpeakerArrEmpty;
            if (channels == AudioChannelSet::mono())               return Vst2::kSpeakerArrMono;
            if (channels == AudioChannelSet::stereo())             return Vst2::kSpeakerArrStereo;
            if (channels == AudioChannelSet::createLCR())          return Vst2::kSpeakerArr30Cine;
            if (channels == AudioChannelSet::createLRS())          return Vst2::kSpeakerArr30Music;
            if (channels == AudioChannelSet::createLCRS())         return Vst2::kSpeakerArr40Cine;
            if (channels == AudioChannelSet::create5point0())      return Vst2::kSpeakerArr50;
            if (channels == AudioChannelSet::create5point1())      return Vst2::kSpeakerArr51;
            if (channels == AudioChannelSet::create6point0())      return Vst2::kSpeakerArr60Cine;
            if (channels == AudioChannelSet::create6point1())      return Vst2::kSpeakerArr61Cine;
            if (channels == AudioChannelSet::create6point0Music()) return Vst2::kSpeakerArr60Music;
            if (channels == AudioChannelSet::create6point1Music()) return Vst2::kSpeakerArr61Music;
            if (channels == AudioChannelSet::create7point0())      return Vst2::kSpeakerArr70Music;
            if (channels == AudioChannelSet::create7point0SDDS())  return Vst2::kSpeakerArr70Cine;
            if (channels == AudioChannelSet::create7point1())      return Vst2::kSpeakerArr71Music;
            if (channels == AudioChannelSet::create7point1SDDS())  return Vst2::kSpeakerArr71Cine;
            if (channels == AudioChannelSet::quadraphonic())       return Vst2::kSpeakerArr40Music;

            if (channels == AudioChannelSet::disabled())
                return Vst2::kSpeakerArrEmpty;

            auto chans = channels.getChannelTypes();

            for (auto* m = getMappings(); m->vst2 != Vst2::kSpeakerArrEmpty; ++m)
                if (m->matches (chans))
                    return m->vst2;

            return Vst2::kSpeakerArrUserDefined;
        }

        static void channelSetToVstArrangement (const AudioChannelSet& channels, Vst2::VstSpeakerArrangement& result)
        {
            result.type = channelSetToVstArrangementType (channels);
            result.numChannels = channels.size();

            for (int i = 0; i < result.numChannels; ++i)
            {
                auto& speaker = result.speakers[i];

                zeromem (&speaker, sizeof (Vst2::VstSpeakerProperties));
                speaker.type = getSpeakerType (channels.getTypeOfChannel (i));
            }
        }

        static const Mapping* getMappings() noexcept
        {
            static const Mapping mappings[] =
            {
                { Vst2::kSpeakerArrMono,           { centre, unknown } },
                { Vst2::kSpeakerArrStereo,         { left, right, unknown } },
                { Vst2::kSpeakerArrStereoSurround, { leftSurround, rightSurround, unknown } },
                { Vst2::kSpeakerArrStereoCenter,   { leftCentre, rightCentre, unknown } },
                { Vst2::kSpeakerArrStereoSide,     { leftSurroundRear, rightSurroundRear, unknown } },
                { Vst2::kSpeakerArrStereoCLfe,     { centre, LFE, unknown } },
                { Vst2::kSpeakerArr30Cine,         { left, right, centre, unknown } },
                { Vst2::kSpeakerArr30Music,        { left, right, surround, unknown } },
                { Vst2::kSpeakerArr31Cine,         { left, right, centre, LFE, unknown } },
                { Vst2::kSpeakerArr31Music,        { left, right, LFE, surround, unknown } },
                { Vst2::kSpeakerArr40Cine,         { left, right, centre, surround, unknown } },
                { Vst2::kSpeakerArr40Music,        { left, right, leftSurround, rightSurround, unknown } },
                { Vst2::kSpeakerArr41Cine,         { left, right, centre, LFE, surround, unknown } },
                { Vst2::kSpeakerArr41Music,        { left, right, LFE, leftSurround, rightSurround, unknown } },
                { Vst2::kSpeakerArr50,             { left, right, centre, leftSurround, rightSurround, unknown } },
                { Vst2::kSpeakerArr51,             { left, right, centre, LFE, leftSurround, rightSurround, unknown } },
                { Vst2::kSpeakerArr60Cine,         { left, right, centre, leftSurround, rightSurround, surround, unknown } },
                { Vst2::kSpeakerArr60Music,        { left, right, leftSurround, rightSurround, leftSurroundRear, rightSurroundRear, unknown } },
                { Vst2::kSpeakerArr61Cine,         { left, right, centre, LFE, leftSurround, rightSurround, surround, unknown } },
                { Vst2::kSpeakerArr61Music,        { left, right, LFE, leftSurround, rightSurround, leftSurroundRear, rightSurroundRear, unknown } },
                { Vst2::kSpeakerArr70Cine,         { left, right, centre, leftSurround, rightSurround, topFrontLeft, topFrontRight, unknown } },
                { Vst2::kSpeakerArr70Music,        { left, right, centre, leftSurround, rightSurround, leftSurroundRear, rightSurroundRear, unknown } },
                { Vst2::kSpeakerArr71Cine,         { left, right, centre, LFE, leftSurround, rightSurround, topFrontLeft, topFrontRight, unknown } },
                { Vst2::kSpeakerArr71Music,        { left, right, centre, LFE, leftSurround, rightSurround, leftSurroundRear, rightSurroundRear, unknown } },
                { Vst2::kSpeakerArr80Cine,         { left, right, centre, leftSurround, rightSurround, topFrontLeft, topFrontRight, surround, unknown } },
                { Vst2::kSpeakerArr80Music,        { left, right, centre, leftSurround, rightSurround, surround, leftSurroundRear, rightSurroundRear, unknown } },
                { Vst2::kSpeakerArr81Cine,         { left, right, centre, LFE, leftSurround, rightSurround, topFrontLeft, topFrontRight, surround, unknown } },
                { Vst2::kSpeakerArr81Music,        { left, right, centre, LFE, leftSurround, rightSurround, surround, leftSurroundRear, rightSurroundRear, unknown } },
                { Vst2::kSpeakerArr102, { left, right, centre, LFE, leftSurround, rightSurround, topFrontLeft, topFrontCentre, topFrontRight, topRearLeft, topRearRight, LFE2, unknown } },
                { Vst2::kSpeakerArrEmpty,          { unknown } }
            };

            return mappings;
        }

        static inline int32 getSpeakerType (AudioChannelSet::ChannelType type) noexcept
        {
            switch (type)
            {
                case AudioChannelSet::left:              return Vst2::kSpeakerL;
                case AudioChannelSet::right:             return Vst2::kSpeakerR;
                case AudioChannelSet::centre:            return Vst2::kSpeakerC;
                case AudioChannelSet::LFE:               return Vst2::kSpeakerLfe;
                case AudioChannelSet::leftSurround:      return Vst2::kSpeakerLs;
                case AudioChannelSet::rightSurround:     return Vst2::kSpeakerRs;
                case AudioChannelSet::leftCentre:        return Vst2::kSpeakerLc;
                case AudioChannelSet::rightCentre:       return Vst2::kSpeakerRc;
                case AudioChannelSet::surround:          return Vst2::kSpeakerS;
                case AudioChannelSet::leftSurroundRear:  return Vst2::kSpeakerSl;
                case AudioChannelSet::rightSurroundRear: return Vst2::kSpeakerSr;
                case AudioChannelSet::topMiddle:         return Vst2::kSpeakerTm;
                case AudioChannelSet::topFrontLeft:      return Vst2::kSpeakerTfl;
                case AudioChannelSet::topFrontCentre:    return Vst2::kSpeakerTfc;
                case AudioChannelSet::topFrontRight:     return Vst2::kSpeakerTfr;
                case AudioChannelSet::topRearLeft:       return Vst2::kSpeakerTrl;
                case AudioChannelSet::topRearCentre:     return Vst2::kSpeakerTrc;
                case AudioChannelSet::topRearRight:      return Vst2::kSpeakerTrr;
                case AudioChannelSet::LFE2:              return Vst2::kSpeakerLfe2;
                default: break;
            }

            return 0;
        }

        static inline AudioChannelSet::ChannelType getChannelType (int32 type) noexcept
        {
            switch (type)
            {
                case Vst2::kSpeakerL:       return AudioChannelSet::left;
                case Vst2::kSpeakerR:       return AudioChannelSet::right;
                case Vst2::kSpeakerC:       return AudioChannelSet::centre;
                case Vst2::kSpeakerLfe:     return AudioChannelSet::LFE;
                case Vst2::kSpeakerLs:      return AudioChannelSet::leftSurround;
                case Vst2::kSpeakerRs:      return AudioChannelSet::rightSurround;
                case Vst2::kSpeakerLc:      return AudioChannelSet::leftCentre;
                case Vst2::kSpeakerRc:      return AudioChannelSet::rightCentre;
                case Vst2::kSpeakerS:       return AudioChannelSet::surround;
                case Vst2::kSpeakerSl:      return AudioChannelSet::leftSurroundRear;
                case Vst2::kSpeakerSr:      return AudioChannelSet::rightSurroundRear;
                case Vst2::kSpeakerTm:      return AudioChannelSet::topMiddle;
                case Vst2::kSpeakerTfl:     return AudioChannelSet::topFrontLeft;
                case Vst2::kSpeakerTfc:     return AudioChannelSet::topFrontCentre;
                case Vst2::kSpeakerTfr:     return AudioChannelSet::topFrontRight;
                case Vst2::kSpeakerTrl:     return AudioChannelSet::topRearLeft;
                case Vst2::kSpeakerTrc:     return AudioChannelSet::topRearCentre;
                case Vst2::kSpeakerTrr:     return AudioChannelSet::topRearRight;
                case Vst2::kSpeakerLfe2:    return AudioChannelSet::LFE2;
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
                vstEffect.flags |= Vst2::effFlagsHasEditor;
                editorComp.reset (new EditorCompWrapper (*this, *ed));

               #if ! (JUCE_MAC || JUCE_IOS)
                ed->setScaleFactor (editorScaleFactor);
               #endif
            }
            else
            {
                vstEffect.flags &= ~Vst2::effFlagsHasEditor;
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
            case Vst2::effOpen:                     return handleOpen (args);
            case Vst2::effClose:                    return handleClose (args);
            case Vst2::effSetProgram:               return handleSetCurrentProgram (args);
            case Vst2::effGetProgram:               return handleGetCurrentProgram (args);
            case Vst2::effSetProgramName:           return handleSetCurrentProgramName (args);
            case Vst2::effGetProgramName:           return handleGetCurrentProgramName (args);
            case Vst2::effGetParamLabel:            return handleGetParameterLabel (args);
            case Vst2::effGetParamDisplay:          return handleGetParameterText (args);
            case Vst2::effGetParamName:             return handleGetParameterName (args);
            case Vst2::effSetSampleRate:            return handleSetSampleRate (args);
            case Vst2::effSetBlockSize:             return handleSetBlockSize (args);
            case Vst2::effMainsChanged:             return handleResumeSuspend (args);
            case Vst2::effEditGetRect:              return handleGetEditorBounds (args);
            case Vst2::effEditOpen:                 return handleOpenEditor (args);
            case Vst2::effEditClose:                return handleCloseEditor (args);
            case Vst2::effIdentify:                 return (pointer_sized_int) ByteOrder::bigEndianInt ("NvEf");
            case Vst2::effGetChunk:                 return handleGetData (args);
            case Vst2::effSetChunk:                 return handleSetData (args);
            case Vst2::effProcessEvents:            return handlePreAudioProcessingEvents (args);
            case Vst2::effCanBeAutomated:           return handleIsParameterAutomatable (args);
            case Vst2::effString2Parameter:         return handleParameterValueForText (args);
            case Vst2::effGetProgramNameIndexed:    return handleGetProgramName (args);
            case Vst2::effGetInputProperties:       return handleGetInputPinProperties (args);
            case Vst2::effGetOutputProperties:      return handleGetOutputPinProperties (args);
            case Vst2::effGetPlugCategory:          return handleGetPlugInCategory (args);
            case Vst2::effSetSpeakerArrangement:    return handleSetSpeakerConfiguration (args);
            case Vst2::effSetBypass:                return handleSetBypass (args);
            case Vst2::effGetEffectName:            return handleGetPlugInName (args);
            case Vst2::effGetProductString:         return handleGetPlugInName (args);
            case Vst2::effGetVendorString:          return handleGetManufacturerName (args);
            case Vst2::effGetVendorVersion:         return handleGetManufacturerVersion (args);
            case Vst2::effVendorSpecific:           return handleManufacturerSpecific (args);
            case Vst2::effCanDo:                    return handleCanPlugInDo (args);
            case Vst2::effGetTailSize:              return handleGetTailSize (args);
            case Vst2::effKeysRequired:             return handleKeyboardFocusRequired (args);
            case Vst2::effGetVstVersion:            return handleGetVstInterfaceVersion (args);
            case Vst2::effGetCurrentMidiProgram:    return handleGetCurrentMidiProgram (args);
            case Vst2::effGetSpeakerArrangement:    return handleGetSpeakerConfiguration (args);
            case Vst2::effSetTotalSampleToProcess:  return handleSetNumberOfSamplesToProcess (args);
            case Vst2::effSetProcessPrecision:      return handleSetSampleFloatType (args);
            case Vst2::effGetNumMidiInputChannels:  return handleGetNumMidiInputChannels();
            case Vst2::effGetNumMidiOutputChannels: return handleGetNumMidiOutputChannels();
            default:                                return 0;
        }
    }

    static pointer_sized_int dispatcherCB (Vst2::AEffect* vstInterface, int32 opCode, int32 index,
                                           pointer_sized_int value, void* ptr, float opt)
    {
        auto* wrapper = getWrapper (vstInterface);
        VstOpCodeArguments args = { index, value, ptr, opt };

        if (opCode == Vst2::effClose)
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
            deleteAllChildren(); // note that we can't use a std::unique_ptr because the editor may
                                 // have been transferred to another parent which takes over ownership.
        }

        void paint (Graphics&) override {}

        void getEditorBounds (Vst2::ERect& bounds)
        {
            auto b = getSizeToContainChild();

            bounds.top     = 0;
            bounds.left  = 0;
            bounds.bottom     = (int16) b.getHeight();
            bounds.right = (int16) b.getWidth();
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

                if (shouldResizeEditor)
                    ed->setBounds (ed->getLocalArea (this, getLocalBounds()));

                if (! getHostType().isBitwigStudio())
                    updateWindowSize (false);
            }

           #if JUCE_MAC && ! JUCE_64BIT
            if (! wrapper.useNSView)
                updateEditorCompBoundsVST (this);
           #endif
        }

        void childBoundsChanged (Component*) override
        {
            updateWindowSize (false);
        }

        juce::Rectangle<int> getSizeToContainChild()
        {
            if (auto* ed = getEditorComp())
                return getLocalArea (ed, ed->getLocalBounds());

            return {};
        }

        void updateWindowSize (bool resizeEditor)
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
                    if (! resizeEditor) // this is needed to prevent an infinite resizing loop due to coordinate rounding
                        shouldResizeEditor = false;

                    setSize (pos.getWidth(), pos.getHeight());

                    shouldResizeEditor = true;
                   #else
                    ignoreUnused (resizeEditor);
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
                auto status = host (wrapper.getAEffect(), Vst2::audioMasterCanDo, 0, 0, const_cast<char*> ("sizeWindow"), 0);

                if (status == (pointer_sized_int) 1 || getHostType().isAbletonLive())
                {
                    isInSizeWindow = true;
                    sizeWasSuccessful = (host (wrapper.getAEffect(), Vst2::audioMasterSizeWindow, newWidth, newHeight, 0, 0) != 0);
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
        bool shouldResizeEditor = true;

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
    Vst2::audioMasterCallback hostCallback;
    AudioProcessor* processor = {};
    double sampleRate = 44100.0;
    int32 blockSize = 1024;
    Vst2::AEffect vstEffect;
    juce::MemoryBlock chunkMemory;
    juce::uint32 chunkMemoryTime = 0;
    std::unique_ptr<EditorCompWrapper> editorComp;
    Vst2::ERect editorBounds;
    MidiBuffer midiEvents;
    VSTMidiEventList outgoingEvents;
    float editorScaleFactor = 1.0f;

    LegacyAudioParametersWrapper juceParameters;

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

    HeapBlock<Vst2::VstSpeakerArrangement> cachedInArrangement, cachedOutArrangement;

    ThreadLocalValue<bool> inParameterChangedCallback;

    static JuceVSTWrapper* getWrapper (Vst2::AEffect* v) noexcept  { return static_cast<JuceVSTWrapper*> (v->object); }

    bool isProcessLevelOffline()
    {
        return hostCallback != nullptr
                && (int32) hostCallback (&vstEffect, Vst2::audioMasterGetCurrentProcessLevel, 0, 0, 0, 0) == 4;
    }

    static inline int32 convertHexVersionToDecimal (const unsigned int hexVersion)
    {
       #if JUCE_VST_RETURN_HEX_VERSION_NUMBER_DIRECTLY
        return (int32) hexVersion;
       #else
        // Currently, only Cubase displays the version number to the user
        // We are hoping here that when other DAWs start to display the version
        // number, that they do so according to yfede's encoding table in the link
        // below. If not, then this code will need an if (isSteinberg()) in the
        // future.
        int major = (hexVersion >> 16) & 0xff;
        int minor = (hexVersion >> 8) & 0xff;
        int bugfix = hexVersion & 0xff;

        // for details, see: https://forum.juce.com/t/issues-with-version-integer-reported-by-vst2/23867

        // Encoding B
        if (major < 1)
            return major * 1000 + minor * 100 + bugfix * 10;

        // Encoding E
        if (major > 100)
            return major * 10000000 + minor * 100000 + bugfix * 1000;

        // Encoding D
        return static_cast<int32> (hexVersion);
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
            tmpBuffers.tempChannels.insertMultiple (0, nullptr, vstEffect.numInputs
                                                                 + vstEffect.numOutputs);
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
            vstEffect.flags |= Vst2::effFlagsHasEditor;
        else
            vstEffect.flags &= ~Vst2::effFlagsHasEditor;

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
        if (auto* param = juceParameters.getParamForIndex (args.index))
        {
            // length should technically be kVstMaxParamStrLen, which is 8, but hosts will normally allow a bit more.
            param->getLabel().copyToUTF8 ((char*) args.ptr, 24 + 1);
        }

        return 0;
    }

    pointer_sized_int handleGetParameterText (VstOpCodeArguments args)
    {
        if (auto* param = juceParameters.getParamForIndex (args.index))
        {
            // length should technically be kVstMaxParamStrLen, which is 8, but hosts will normally allow a bit more.
            param->getCurrentValueAsText().copyToUTF8 ((char*) args.ptr, 24 + 1);
        }

        return 0;
    }

    pointer_sized_int handleGetParameterName (VstOpCodeArguments args)
    {
        if (auto* param = juceParameters.getParamForIndex (args.index))
        {
            // length should technically be kVstMaxParamStrLen, which is 8, but hosts will normally allow a bit more.
            param->getName (32).copyToUTF8 ((char*) args.ptr, 32 + 1);
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
            *((Vst2::ERect**) args.ptr) = &editorBounds;
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
        VSTMidiEventList::addEventsToMidiBuffer ((Vst2::VstEvents*) args.ptr, midiEvents);
        return 1;
       #else
        ignoreUnused (args);
        return 0;
       #endif
    }

    pointer_sized_int handleIsParameterAutomatable (VstOpCodeArguments args)
    {
        if (auto* param = juceParameters.getParamForIndex (args.index))
        {
            const bool isMeter = (((param->getCategory() & 0xffff0000) >> 16) == 2);
            return (param->isAutomatable() && (! isMeter) ? 1 : 0);
        }

        return 0;
    }

    pointer_sized_int handleParameterValueForText (VstOpCodeArguments args)
    {
        if (auto* param = juceParameters.getParamForIndex (args.index))
        {
            if (! LegacyAudioParameter::isLegacy (param))
            {
                auto value = param->getValueForText (String::fromUTF8 ((char*) args.ptr));
                param->setValue (value);

                inParameterChangedCallback = true;
                param->sendValueChangedMessageToListeners (value);

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
        return (processor != nullptr && getPinProperties (*(Vst2::VstPinProperties*) args.ptr, true, args.index)) ? 1 : 0;
    }

    pointer_sized_int handleGetOutputPinProperties (VstOpCodeArguments args)
    {
        return (processor != nullptr && getPinProperties (*(Vst2::VstPinProperties*) args.ptr, false, args.index)) ? 1 : 0;
    }

    pointer_sized_int handleGetPlugInCategory (VstOpCodeArguments)
    {
        return Vst2::JucePlugin_VSTCategory;
    }

    pointer_sized_int handleSetSpeakerConfiguration (VstOpCodeArguments args)
    {
        auto* pluginInput  = reinterpret_cast<Vst2::VstSpeakerArrangement*> (args.value);
        auto* pluginOutput = reinterpret_cast<Vst2::VstSpeakerArrangement*> (args.ptr);

        if (processor->isMidiEffect())
            return 0;

        auto numIns  = processor->getBusCount (true);
        auto numOuts = processor->getBusCount (false);

        if (pluginInput != nullptr && pluginInput->type >= 0)
        {
            // inconsistent request?
            if (SpeakerMappings::vstArrangementTypeToChannelSet (*pluginInput).size() != pluginInput->numChannels)
                return 0;
        }

        if (pluginOutput != nullptr && pluginOutput->type >= 0)
        {
            // inconsistent request?
            if (SpeakerMappings::vstArrangementTypeToChannelSet (*pluginOutput).size() != pluginOutput->numChannels)
                return 0;
        }

        if (pluginInput != nullptr  && pluginInput->numChannels  > 0 && numIns  == 0)
            return 0;

        if (pluginOutput != nullptr && pluginOutput->numChannels > 0 && numOuts == 0)
            return 0;

        auto layouts = processor->getBusesLayout();

        if (pluginInput != nullptr && pluginInput-> numChannels >= 0 && numIns  > 0)
            layouts.getChannelSet (true,  0) = SpeakerMappings::vstArrangementTypeToChannelSet (*pluginInput);

        if (pluginOutput != nullptr && pluginOutput->numChannels >= 0 && numOuts > 0)
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

        if (auto* bypass = processor->getBypassParameter())
            bypass->setValueNotifyingHost (isBypassed ? 1.0f : 0.0f);

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

        if (args.index == JUCE_MULTICHAR_CONSTANT ('P', 'r', 'e', 'S')
             && args.value == JUCE_MULTICHAR_CONSTANT ('A', 'e', 'C', 's'))
            return handleSetContentScaleFactor (args.opt);

        if (args.index == Vst2::effGetParamDisplay)
            return handleCockosGetParameterText (args.value, args.ptr, args.opt);

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

        if (matches ("hasCockosExtensions"))
            return (int32) 0xbeef0000;

        if (auto callbackHandler = dynamic_cast<VSTCallbackHandler*> (processor))
            return callbackHandler->handleVstPluginCanDo (args.index, args.value, args.ptr, args.opt);

        return 0;
    }

    pointer_sized_int handleGetTailSize (VstOpCodeArguments)
    {
        if (processor != nullptr)
        {
            int32 result;

            auto tailSeconds = processor->getTailLengthSeconds();

            if (tailSeconds == std::numeric_limits<double>::infinity())
                result = std::numeric_limits<int32>::max();
            else
                result = static_cast<int32> (tailSeconds * sampleRate);

            return result; // Vst2 expects an int32 upcasted to a intptr_t here
        }

        return 0;
    }

    pointer_sized_int handleKeyboardFocusRequired (VstOpCodeArguments)
    {
        return (JucePlugin_EditorRequiresKeyboardFocus != 0) ? 1 : 0;
    }

    pointer_sized_int handleGetVstInterfaceVersion (VstOpCodeArguments)
    {
        return kVstVersion;
    }

    pointer_sized_int handleGetCurrentMidiProgram (VstOpCodeArguments)
    {
        return -1;
    }

    pointer_sized_int handleGetSpeakerConfiguration (VstOpCodeArguments args)
    {
        auto** pluginInput  = reinterpret_cast<Vst2::VstSpeakerArrangement**> (args.value);
        auto** pluginOutput = reinterpret_cast<Vst2::VstSpeakerArrangement**> (args.ptr);

        if (pluginHasSidechainsOrAuxs() || processor->isMidiEffect())
            return false;

        auto inputLayout  = processor->getChannelLayoutOfBus (true,  0);
        auto outputLayout = processor->getChannelLayoutOfBus (false,  0);

        auto speakerBaseSize = sizeof (Vst2::VstSpeakerArrangement) - (sizeof (Vst2::VstSpeakerProperties) * 8);

        cachedInArrangement .malloc (speakerBaseSize + (static_cast<std::size_t> (inputLayout. size()) * sizeof (Vst2::VstSpeakerArrangement)), 1);
        cachedOutArrangement.malloc (speakerBaseSize + (static_cast<std::size_t> (outputLayout.size()) * sizeof (Vst2::VstSpeakerArrangement)), 1);

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
                processor->setProcessingPrecision ((args.value == Vst2::kVstProcessPrecision64
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
                    editorComp->updateWindowSize (true);
            }
           #endif
        }

        return 1;
    }

    pointer_sized_int handleCockosGetParameterText (pointer_sized_int paramIndex,
                                                    void* dest,
                                                    float value)
    {
        if (processor != nullptr && dest != nullptr)
        {
            if (auto* param = juceParameters.getParamForIndex ((int) paramIndex))
            {
                if (! LegacyAudioParameter::isLegacy (param))
                {
                    String text (param->getText (value, 1024));
                    memcpy (dest, text.toRawUTF8(), ((size_t) text.length()) + 1);
                    return 0xbeef;
                }
            }
        }

        return 0;
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
    Vst2::AEffect* pluginEntryPoint (Vst2::audioMasterCallback audioMaster)
    {
        JUCE_AUTORELEASEPOOL
        {
            initialiseJuce_GUI();

            try
            {
                if (audioMaster (0, Vst2::audioMasterVersion, 0, 0, 0, 0) != 0)
                {
                   #if JUCE_LINUX
                    MessageManagerLock mmLock;
                   #endif

                    auto* processor = createPluginFilterOfType (AudioProcessor::wrapperType_VST);
                    auto* wrapper = new JuceVSTWrapper (audioMaster, processor);
                    return wrapper->getAEffect();
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

    JUCE_EXPORTED_FUNCTION Vst2::AEffect* VSTPluginMain (Vst2::audioMasterCallback audioMaster);
    JUCE_EXPORTED_FUNCTION Vst2::AEffect* VSTPluginMain (Vst2::audioMasterCallback audioMaster)
    {
        PluginHostType::jucePlugInClientCurrentWrapperType = AudioProcessor::wrapperType_VST;

        initialiseMacVST();
        return pluginEntryPoint (audioMaster);
    }

    JUCE_EXPORTED_FUNCTION Vst2::AEffect* main_macho (Vst2::audioMasterCallback audioMaster);
    JUCE_EXPORTED_FUNCTION Vst2::AEffect* main_macho (Vst2::audioMasterCallback audioMaster)
    {
        PluginHostType::jucePlugInClientCurrentWrapperType = AudioProcessor::wrapperType_VST;

        initialiseMacVST();
        return pluginEntryPoint (audioMaster);
    }

//==============================================================================
// Linux startup code..
#elif JUCE_LINUX

    JUCE_EXPORTED_FUNCTION Vst2::AEffect* VSTPluginMain (Vst2::audioMasterCallback audioMaster);
    JUCE_EXPORTED_FUNCTION Vst2::AEffect* VSTPluginMain (Vst2::audioMasterCallback audioMaster)
    {
        PluginHostType::jucePlugInClientCurrentWrapperType = AudioProcessor::wrapperType_VST;

        SharedMessageThread::getInstance();
        return pluginEntryPoint (audioMaster);
    }

    JUCE_EXPORTED_FUNCTION Vst2::AEffect* main_plugin (Vst2::audioMasterCallback audioMaster) asm ("main");
    JUCE_EXPORTED_FUNCTION Vst2::AEffect* main_plugin (Vst2::audioMasterCallback audioMaster)
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

    extern "C" __declspec (dllexport) Vst2::AEffect* VSTPluginMain (Vst2::audioMasterCallback audioMaster)
    {
        PluginHostType::jucePlugInClientCurrentWrapperType = AudioProcessor::wrapperType_VST;

        return pluginEntryPoint (audioMaster);
    }

   #ifndef JUCE_64BIT // (can't compile this on win64, but it's not needed anyway with VST2.4)
    extern "C" __declspec (dllexport) int main (Vst2::audioMasterCallback audioMaster)
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

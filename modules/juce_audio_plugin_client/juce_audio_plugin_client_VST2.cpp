/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

#include <juce_core/system/juce_CompilerWarnings.h>
#include <juce_core/system/juce_TargetPlatform.h>
#include <juce_audio_plugin_client/detail/juce_CheckSettingMacros.h>

#if JucePlugin_Build_VST

JUCE_BEGIN_IGNORE_WARNINGS_MSVC (4996 4100)

#include <juce_audio_plugin_client/detail/juce_IncludeSystemHeaders.h>
#include <juce_core/juce_core.h>

#if JucePlugin_VersionCode < 0x010000   // Major < 0

 #if (JucePlugin_VersionCode & 0x00FF00) > (9 * 0x100) // check if Minor number exceeds 9
  JUCE_COMPILER_WARNING ("When version has 'major' = 0, VST2 has trouble displaying 'minor' exceeding 9")
 #endif

 #if (JucePlugin_VersionCode & 0xFF) > 9   // check if Bugfix number exceeds 9
  JUCE_COMPILER_WARNING ("When version has 'major' = 0, VST2 has trouble displaying 'bugfix' exceeding 9")
 #endif

#elif JucePlugin_VersionCode >= 0x650000   // Major >= 101

 #if (JucePlugin_VersionCode & 0x00FF00) > (99 * 0x100) // check if Minor number exceeds 99
  JUCE_COMPILER_WARNING ("When version has 'major' > 100, VST2 has trouble displaying 'minor' exceeding 99")
 #endif

 #if (JucePlugin_VersionCode & 0xFF) > 99  // check if Bugfix number exceeds 99
  JUCE_COMPILER_WARNING ("When version has 'major' > 100, VST2 has trouble displaying 'bugfix' exceeding 99")
 #endif

#endif

#ifdef PRAGMA_ALIGN_SUPPORTED
 #undef PRAGMA_ALIGN_SUPPORTED
 #define PRAGMA_ALIGN_SUPPORTED 1
#endif

#if ! JUCE_MSVC && ! defined (__cdecl)
 #define __cdecl
#endif

JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wconversion",
                                     "-Wshadow",
                                     "-Wdeprecated-register",
                                     "-Wdeprecated-declarations",
                                     "-Wunused-parameter",
                                     "-Wdeprecated-writable-strings",
                                     "-Wnon-virtual-dtor",
                                     "-Wzero-as-null-pointer-constant",
                                     "-Wlanguage-extension-token")
JUCE_BEGIN_IGNORE_WARNINGS_MSVC (4458)

#define VST_FORCE_DEPRECATED 0

namespace Vst2
{
// If the following files cannot be found then you are probably trying to build
// a VST2 plug-in or a VST2-compatible VST3 plug-in. To do this you must have a
// VST2 SDK in your header search paths or use the "VST (Legacy) SDK Folder"
// field in the Projucer. The VST2 SDK can be obtained from the
// vstsdk3610_11_06_2018_build_37 (or older) VST3 SDK or JUCE version 5.3.2. You
// also need a VST2 license from Steinberg to distribute VST2 plug-ins.
#include "pluginterfaces/vst2.x/aeffect.h"
#include "pluginterfaces/vst2.x/aeffectx.h"
}

JUCE_END_IGNORE_WARNINGS_MSVC
JUCE_END_IGNORE_WARNINGS_GCC_LIKE

//==============================================================================
#if JUCE_MSVC
 #pragma pack (push, 8)
#endif

#define JUCE_VSTINTERFACE_H_INCLUDED 1
#define JUCE_GUI_BASICS_INCLUDE_XHEADERS 1

#include <juce_audio_plugin_client/detail/juce_PluginUtilities.h>

using namespace juce;

#include <juce_gui_basics/native/juce_WindowsHooks_windows.h>
#include <juce_audio_plugin_client/detail/juce_LinuxMessageThread.h>
#include <juce_audio_plugin_client/detail/juce_VSTWindowUtilities.h>

#include <juce_audio_processors/format_types/juce_LegacyAudioParameter.cpp>
#include <juce_audio_processors/format_types/juce_VSTCommon.h>

#ifdef JUCE_MSVC
 #pragma pack (pop)
#endif

#undef MemoryBlock

class JuceVSTWrapper;
static bool recursionCheck = false;

namespace juce
{
 #if JUCE_WINDOWS && JUCE_WIN_PER_MONITOR_DPI_AWARE
  JUCE_API double getScaleFactorForWindow (HWND);
 #endif
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

        while (w != nullptr)
        {
            auto parent = getWindowParent (w);

            if (parent == nullptr)
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

    static int numActivePlugins = 0;
    static bool messageThreadIsDefinitelyCorrect = false;
}

#endif

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
class JuceVSTWrapper final : public AudioProcessorListener,
                             public AudioPlayHead,
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
    JuceVSTWrapper (Vst2::audioMasterCallback cb, std::unique_ptr<AudioProcessor> af)
       : hostCallback (cb),
         processor (std::move (af))
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
        vstEffect.dispatcher = [] (Vst2::AEffect* vstInterface,
                                   Vst2::VstInt32 opCode,
                                   Vst2::VstInt32 index,
                                   Vst2::VstIntPtr value,
                                   void* ptr,
                                   float opt) -> Vst2::VstIntPtr
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
        };

        vstEffect.process = nullptr;

        vstEffect.setParameter = [] (Vst2::AEffect* vstInterface, Vst2::VstInt32 index, float value)
        {
            getWrapper (vstInterface)->setParameter (index, value);
        };

        vstEffect.getParameter = [] (Vst2::AEffect* vstInterface, Vst2::VstInt32 index) -> float
        {
            return getWrapper (vstInterface)->getParameter (index);
        };

        vstEffect.numPrograms = jmax (1, processor->getNumPrograms());
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

        vstEffect.processReplacing = [] (Vst2::AEffect* vstInterface,
                                         float** inputs,
                                         float** outputs,
                                         Vst2::VstInt32 sampleFrames)
        {
            getWrapper (vstInterface)->processReplacing (inputs, outputs, sampleFrames);
        };

        vstEffect.processDoubleReplacing = [] (Vst2::AEffect* vstInterface,
                                               double** inputs,
                                               double** outputs,
                                               Vst2::VstInt32 sampleFrames)
        {
            getWrapper (vstInterface)->processDoubleReplacing (inputs, outputs, sampleFrames);
        };

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

       #if JUCE_WINDOWS
        ++numActivePlugins;
       #endif
    }

    ~JuceVSTWrapper() override
    {
        JUCE_AUTORELEASEPOOL
        {
           #if JUCE_LINUX || JUCE_BSD
            MessageManagerLock mmLock;
           #endif

            timedCallback.stopTimer();
            deleteEditor (false);

            hasShutdown = true;

            processor = nullptr;

            jassert (editorComp == nullptr);

            deleteTempChannels();

           #if JUCE_WINDOWS
            if (--numActivePlugins == 0)
                messageThreadIsDefinitelyCorrect = false;
           #endif
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
            //  to do it here.)
            if (! isProcessing)
                resume();

            processor->setNonRealtime (isProcessLevelOffline());

           #if JUCE_WINDOWS
            if (detail::PluginUtilities::getHostType().isWavelab())
            {
                int priority = GetThreadPriority (GetCurrentThread());

                if (priority <= THREAD_PRIORITY_NORMAL && priority >= THREAD_PRIORITY_LOWEST)
                    processor->setNonRealtime (true);
            }
           #endif
        }

        const auto numMidiEventsComingIn = midiEvents.getNumEvents();

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
                updateCallbackContextInfo();

                int i;
                for (i = 0; i < numOut; ++i)
                {
                    auto* chan = tmpBuffers.tempChannels.getUnchecked (i);

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
                            memcpy (chan, inputs[i], (size_t) numSamples * sizeof (FloatType));
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

                    if (isBypassed && processor->getBypassParameter() == nullptr)
                        processor->processBlockBypassed (chans, midiEvents);
                    else
                        processor->processBlock (chans, midiEvents);
                }

                // copy back any temp channels that may have been used..
                for (i = 0; i < numOut; ++i)
                    if (auto* chan = tmpBuffers.tempChannels.getUnchecked (i))
                        if (auto* dest = outputs[i])
                            memcpy (dest, chan, (size_t) numSamples * sizeof (FloatType));
            }
        }

        if (! midiEvents.isEmpty())
        {
            if (supportsMidiOut)
            {
                auto numEvents = midiEvents.getNumEvents();

                outgoingEvents.ensureSize (numEvents);
                outgoingEvents.clear();

                for (const auto metadata : midiEvents)
                {
                    jassert (metadata.samplePosition >= 0 && metadata.samplePosition < numSamples);

                    outgoingEvents.addEvent (metadata.data, metadata.numBytes, metadata.samplePosition);
                }

                // Send VST events to the host.
                NullCheckedInvocation::invoke (hostCallback, &vstEffect, Vst2::audioMasterProcessEvents, 0, 0, outgoingEvents.events, 0.0f);
            }
            else
            {
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
                jassertquiet (midiEvents.getNumEvents() <= numMidiEventsComingIn);
            }

            midiEvents.clear();
        }
    }

    void processReplacing (float** inputs, float** outputs, int32 sampleFrames)
    {
        jassert (! processor->isUsingDoublePrecision());
        internalProcessReplacing (inputs, outputs, sampleFrames, floatTempBuffers);
    }

    void processDoubleReplacing (double** inputs, double** outputs, int32 sampleFrames)
    {
        jassert (processor->isUsingDoublePrecision());
        internalProcessReplacing (inputs, outputs, sampleFrames, doubleTempBuffers);
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
            if (vstEffect.flags & Vst2::effFlagsIsSynth || supportsMidiIn)
                NullCheckedInvocation::invoke (hostCallback, &vstEffect, Vst2::audioMasterWantMidi, 0, 1, nullptr, 0.0f);

            if (detail::PluginUtilities::getHostType().isAbletonLive()
                 && hostCallback != nullptr
                 && std::isinf (processor->getTailLengthSeconds()))
            {
                AbletonLiveHostSpecific hostCmd;

                hostCmd.magic = 0x41624c69; // 'AbLi'
                hostCmd.cmd = 5;
                hostCmd.commandSize = sizeof (int);
                hostCmd.flags = AbletonLiveHostSpecific::KCantBeSuspended;

                hostCallback (&vstEffect, Vst2::audioMasterVendorSpecific, 0, 0, &hostCmd, 0.0f);
            }

            if (supportsMidiOut)
                outgoingEvents.ensureSize (512);
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

    void updateCallbackContextInfo()
    {
        const Vst2::VstTimeInfo* ti = nullptr;

        if (hostCallback != nullptr)
        {
            int32 flags = Vst2::kVstPpqPosValid  | Vst2::kVstTempoValid
                          | Vst2::kVstBarsValid    | Vst2::kVstCyclePosValid
                          | Vst2::kVstTimeSigValid | Vst2::kVstSmpteValid
                          | Vst2::kVstClockValid   | Vst2::kVstNanosValid;

            auto result = hostCallback (&vstEffect, Vst2::audioMasterGetTime, 0, flags, nullptr, 0);
            ti = reinterpret_cast<Vst2::VstTimeInfo*> (result);
        }

        if (ti == nullptr || ti->sampleRate <= 0)
        {
            currentPosition.reset();
            return;
        }

        auto& info = currentPosition.emplace();
        info.setBpm ((ti->flags & Vst2::kVstTempoValid) != 0 ? makeOptional (ti->tempo) : nullopt);

        info.setTimeSignature ((ti->flags & Vst2::kVstTimeSigValid) != 0 ? makeOptional (TimeSignature { ti->timeSigNumerator, ti->timeSigDenominator })
                                                                         : nullopt);

        info.setTimeInSamples ((int64) (ti->samplePos + 0.5));
        info.setTimeInSeconds (ti->samplePos / ti->sampleRate);
        info.setPpqPosition ((ti->flags & Vst2::kVstPpqPosValid) != 0 ? makeOptional (ti->ppqPos) : nullopt);
        info.setPpqPositionOfLastBarStart ((ti->flags & Vst2::kVstBarsValid) != 0 ? makeOptional (ti->barStartPos) : nullopt);

        if ((ti->flags & Vst2::kVstSmpteValid) != 0)
        {
            info.setFrameRate ([&]() -> Optional<FrameRate>
            {
                switch (ti->smpteFrameRate)
                {
                    case Vst2::kVstSmpte24fps:          return FrameRate().withBaseRate (24);
                    case Vst2::kVstSmpte239fps:         return FrameRate().withBaseRate (24).withPullDown();

                    case Vst2::kVstSmpte25fps:          return FrameRate().withBaseRate (25);
                    case Vst2::kVstSmpte249fps:         return FrameRate().withBaseRate (25).withPullDown();

                    case Vst2::kVstSmpte30fps:          return FrameRate().withBaseRate (30);
                    case Vst2::kVstSmpte30dfps:         return FrameRate().withBaseRate (30).withDrop();
                    case Vst2::kVstSmpte2997fps:        return FrameRate().withBaseRate (30).withPullDown();
                    case Vst2::kVstSmpte2997dfps:       return FrameRate().withBaseRate (30).withPullDown().withDrop();

                    case Vst2::kVstSmpte60fps:          return FrameRate().withBaseRate (60);
                    case Vst2::kVstSmpte599fps:         return FrameRate().withBaseRate (60).withPullDown();

                    case Vst2::kVstSmpteFilm16mm:
                    case Vst2::kVstSmpteFilm35mm:       return FrameRate().withBaseRate (24);
                }

                return nullopt;
            }());

            const auto effectiveRate = info.getFrameRate().hasValue() ? info.getFrameRate()->getEffectiveRate() : 0.0;
            info.setEditOriginTime (! approximatelyEqual (effectiveRate, 0.0) ? makeOptional (ti->smpteOffset / (80.0 * effectiveRate)) : nullopt);
        }

        info.setIsRecording ((ti->flags & Vst2::kVstTransportRecording) != 0);
        info.setIsPlaying   ((ti->flags & (Vst2::kVstTransportRecording | Vst2::kVstTransportPlaying)) != 0);
        info.setIsLooping   ((ti->flags & Vst2::kVstTransportCycleActive) != 0);

        info.setLoopPoints ((ti->flags & Vst2::kVstCyclePosValid) != 0 ? makeOptional (LoopPoints { ti->cycleStartPos, ti->cycleEndPos })
                                                                       : nullopt);

        info.setHostTimeNs ((ti->flags & Vst2::kVstNanosValid) != 0 ? makeOptional ((uint64_t) ti->nanoSeconds) : nullopt);
    }

    //==============================================================================
    Optional<PositionInfo> getPosition() const override
    {
        return currentPosition;
    }

    //==============================================================================
    float getParameter (int32 index) const
    {
        if (auto* param = juceParameters.getParamForIndex (index))
            return param->getValue();

        return 0.0f;
    }

    void setParameter (int32 index, float value)
    {
        if (auto* param = juceParameters.getParamForIndex (index))
            setValueAndNotifyIfChanged (*param, value);
    }

    void audioProcessorParameterChanged (AudioProcessor*, int index, float newValue) override
    {
        if (inParameterChangedCallback.get())
        {
            inParameterChangedCallback = false;
            return;
        }

        NullCheckedInvocation::invoke (hostCallback, &vstEffect, Vst2::audioMasterAutomate, index, 0, nullptr, newValue);
    }

    void audioProcessorParameterChangeGestureBegin (AudioProcessor*, int index) override
    {
        NullCheckedInvocation::invoke (hostCallback, &vstEffect, Vst2::audioMasterBeginEdit, index, 0, nullptr, 0.0f);
    }

    void audioProcessorParameterChangeGestureEnd (AudioProcessor*, int index) override
    {
        NullCheckedInvocation::invoke (hostCallback, &vstEffect, Vst2::audioMasterEndEdit, index, 0, nullptr, 0.0f);
    }

    void parameterValueChanged (int, float newValue) override
    {
        // this can only come from the bypass parameter
        isBypassed = (newValue >= 0.5f);
    }

    void parameterGestureChanged (int, bool) override {}

    void audioProcessorChanged (AudioProcessor*, const ChangeDetails& details) override
    {
        hostChangeUpdater.update (details);
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
    void setHasEditorFlag (bool shouldSetHasEditor)
    {
        auto hasEditor = (vstEffect.flags & Vst2::effFlagsHasEditor) != 0;

        if (shouldSetHasEditor == hasEditor)
            return;

        if (shouldSetHasEditor)
            vstEffect.flags |= Vst2::effFlagsHasEditor;
        else
            vstEffect.flags &= ~Vst2::effFlagsHasEditor;
    }

    void createEditorComp()
    {
        if (hasShutdown || processor == nullptr)
            return;

        if (editorComp == nullptr)
        {
            if (auto* ed = processor->createEditorIfNeeded())
            {
                setHasEditorFlag (true);
                editorComp.reset (new EditorCompWrapper (*this, *ed, editorScaleFactor));
            }
            else
            {
                setHasEditorFlag (false);
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
            case Vst2::effGetMidiKeyName:           return handleGetMidiKeyName (args);
            case Vst2::effEditIdle:                 return handleEditIdle();
            default:                                return 0;
        }
    }

    //==============================================================================
    // A component to hold the AudioProcessorEditor, and cope with some housekeeping
    // chores when it changes or repaints.
    struct EditorCompWrapper final : public Component
                             #if JUCE_WINDOWS && JUCE_WIN_PER_MONITOR_DPI_AWARE
                              , public Timer
                             #endif
    {
        EditorCompWrapper (JuceVSTWrapper& w, AudioProcessorEditor& editor, [[maybe_unused]] float initialScale)
            : wrapper (w)
        {
            editor.setOpaque (true);
           #if ! JUCE_MAC
            editor.setScaleFactor (initialScale);
           #endif
            addAndMakeVisible (editor);

            auto editorBounds = getSizeToContainChild();
            setSize (editorBounds.getWidth(), editorBounds.getHeight());

           #if JUCE_WINDOWS
            if (! detail::PluginUtilities::getHostType().isReceptor())
                addMouseListener (this, true);
           #endif

            setOpaque (true);
        }

        ~EditorCompWrapper() override
        {
            deleteAllChildren(); // note that we can't use a std::unique_ptr because the editor may
                                 // have been transferred to another parent which takes over ownership.
        }

        void paint (Graphics& g) override
        {
            g.fillAll (Colours::black);
        }

        void getEditorBounds (Vst2::ERect& bounds)
        {
            auto editorBounds = getSizeToContainChild();
            bounds = convertToHostBounds ({ 0, 0, (int16) editorBounds.getHeight(), (int16) editorBounds.getWidth() });
        }

        void attachToHost (VstOpCodeArguments args)
        {
            setVisible (false);

            const auto desktopFlags = detail::PluginUtilities::getDesktopFlags (getEditorComp());

           #if JUCE_WINDOWS || JUCE_LINUX || JUCE_BSD
            addToDesktop (desktopFlags, args.ptr);
            hostWindow = (HostWindowType) args.ptr;

            #if JUCE_LINUX || JUCE_BSD
             X11Symbols::getInstance()->xReparentWindow (display,
                                                         (Window) getWindowHandle(),
                                                         (HostWindowType) hostWindow,
                                                         0, 0);
             // The host is likely to attempt to move/resize the window directly after this call,
             // and we need to ensure that the X server knows that our window has been attached
             // before that happens.
             X11Symbols::getInstance()->xFlush (display);
            #elif JUCE_WINDOWS && JUCE_WIN_PER_MONITOR_DPI_AWARE
             checkHostWindowScaleFactor (true);
             startTimer (500);
            #endif
           #elif JUCE_MAC
            hostWindow = detail::VSTWindowUtilities::attachComponentToWindowRefVST (this, desktopFlags, args.ptr);
           #endif

            setVisible (true);
        }

        void detachHostWindow()
        {
           #if JUCE_MAC
            if (hostWindow != nullptr)
                detail::VSTWindowUtilities::detachComponentFromWindowRefVST (this, hostWindow);
           #endif

            hostWindow = {};
        }

        AudioProcessorEditor* getEditorComp() const noexcept
        {
            return dynamic_cast<AudioProcessorEditor*> (getChildComponent (0));
        }

        void resized() override
        {
            if (auto* pluginEditor = getEditorComp())
            {
                if (! resizingParent)
                {
                    auto newBounds = getLocalBounds();

                    {
                        const ScopedValueSetter<bool> resizingChildSetter (resizingChild, true);
                        pluginEditor->setBounds (pluginEditor->getLocalArea (this, newBounds).withPosition (0, 0));
                    }

                    lastBounds = newBounds;
                }

                updateWindowSize();
            }
        }

        void parentSizeChanged() override
        {
            updateWindowSize();
            repaint();
        }

        void childBoundsChanged (Component*) override
        {
            if (resizingChild)
                return;

            auto newBounds = getSizeToContainChild();

            if (newBounds != lastBounds)
            {
                updateWindowSize();
                lastBounds = newBounds;
            }
        }

        juce::Rectangle<int> getSizeToContainChild()
        {
            if (auto* pluginEditor = getEditorComp())
                return getLocalArea (pluginEditor, pluginEditor->getLocalBounds());

            return {};
        }

        void resizeHostWindow (juce::Rectangle<int> bounds)
        {
            auto rect = convertToHostBounds ({ 0, 0, (int16) bounds.getHeight(), (int16) bounds.getWidth() });
            const auto newWidth = rect.right - rect.left;
            const auto newHeight = rect.bottom - rect.top;

            bool sizeWasSuccessful = false;

            if (auto host = wrapper.hostCallback)
            {
                auto status = host (wrapper.getAEffect(), Vst2::audioMasterCanDo, 0, 0, const_cast<char*> ("sizeWindow"), 0);

                if (status == (pointer_sized_int) 1 || detail::PluginUtilities::getHostType().isAbletonLive())
                {
                    const ScopedValueSetter<bool> resizingParentSetter (resizingParent, true);

                    sizeWasSuccessful = (host (wrapper.getAEffect(), Vst2::audioMasterSizeWindow,
                                               newWidth, newHeight, nullptr, 0) != 0);
                }
            }

            // some hosts don't support the sizeWindow call, so do it manually..
            if (! sizeWasSuccessful)
            {
                const ScopedValueSetter<bool> resizingParentSetter (resizingParent, true);

               #if JUCE_MAC
                detail::VSTWindowUtilities::setNativeHostWindowSizeVST (hostWindow, this, newWidth, newHeight);
               #elif JUCE_LINUX || JUCE_BSD
                // (Currently, all linux hosts support sizeWindow, so this should never need to happen)
                setSize (newWidth, newHeight);
               #else
                int dw = 0;
                int dh = 0;
                const int frameThickness = GetSystemMetrics (SM_CYFIXEDFRAME);

                HWND w = (HWND) getWindowHandle();

                while (w != nullptr)
                {
                    HWND parent = getWindowParent (w);

                    if (parent == nullptr)
                        break;

                    TCHAR windowType [32] = { 0 };
                    GetClassName (parent, windowType, 31);

                    if (String (windowType).equalsIgnoreCase ("MDIClient"))
                        break;

                    RECT windowPos, parentPos;
                    GetWindowRect (w, &windowPos);
                    GetWindowRect (parent, &parentPos);

                    if (w != (HWND) getWindowHandle())
                        SetWindowPos (w, nullptr, 0, 0, newWidth + dw, newHeight + dh,
                                      SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOOWNERZORDER);

                    dw = (parentPos.right - parentPos.left) - (windowPos.right - windowPos.left);
                    dh = (parentPos.bottom - parentPos.top) - (windowPos.bottom - windowPos.top);

                    w = parent;

                    if (dw == 2 * frameThickness)
                        break;

                    if (dw > 100 || dh > 100)
                        w = nullptr;
                }

                if (w != nullptr)
                    SetWindowPos (w, nullptr, 0, 0, newWidth + dw, newHeight + dh,
                                  SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOOWNERZORDER);
               #endif
            }

           #if JUCE_LINUX || JUCE_BSD
            X11Symbols::getInstance()->xResizeWindow (display, (Window) getWindowHandle(),
                                                      static_cast<unsigned int> (rect.right - rect.left),
                                                      static_cast<unsigned int> (rect.bottom - rect.top));
           #endif
        }

        void setContentScaleFactor (float scale)
        {
            if (auto* pluginEditor = getEditorComp())
            {
                auto prevEditorBounds = pluginEditor->getLocalArea (this, lastBounds);

                {
                    const ScopedValueSetter<bool> resizingChildSetter (resizingChild, true);

                    pluginEditor->setScaleFactor (scale);
                    pluginEditor->setBounds (prevEditorBounds.withPosition (0, 0));
                }

                lastBounds = getSizeToContainChild();
                updateWindowSize();
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

        #if JUCE_WIN_PER_MONITOR_DPI_AWARE
         void checkHostWindowScaleFactor (bool force = false)
         {
             auto hostWindowScale = (float) getScaleFactorForWindow ((HostWindowType) hostWindow);

             if (force || (hostWindowScale > 0.0f && ! approximatelyEqual (hostWindowScale, wrapper.editorScaleFactor)))
                 wrapper.handleSetContentScaleFactor (hostWindowScale, force);
         }

         void timerCallback() override
         {
             checkHostWindowScaleFactor();
         }
        #endif
       #endif

    private:
        void updateWindowSize()
        {
            if (! resizingParent
                && getEditorComp() != nullptr
                && hostWindow != HostWindowType{})
            {
                const auto editorBounds = getSizeToContainChild();
                resizeHostWindow (editorBounds);

                {
                    const ScopedValueSetter<bool> resizingParentSetter (resizingParent, true);

                    // setSize() on linux causes renoise and energyxt to fail.
                    // We'll resize our peer during resizeHostWindow() instead.
                   #if ! (JUCE_LINUX || JUCE_BSD)
                    setSize (editorBounds.getWidth(), editorBounds.getHeight());
                   #endif

                    if (auto* p = getPeer())
                        p->updateBounds();
                }

               #if JUCE_MAC
                resizeHostWindow (editorBounds); // (doing this a second time seems to be necessary in tracktion)
               #endif
            }
        }

        //==============================================================================
        static Vst2::ERect convertToHostBounds (const Vst2::ERect& rect)
        {
            auto desktopScale = Desktop::getInstance().getGlobalScaleFactor();

            if (approximatelyEqual (desktopScale, 1.0f))
                return rect;

            return { (int16) roundToInt (rect.top    * desktopScale),
                     (int16) roundToInt (rect.left   * desktopScale),
                     (int16) roundToInt (rect.bottom * desktopScale),
                     (int16) roundToInt (rect.right  * desktopScale) };
        }

        //==============================================================================
       #if JUCE_LINUX || JUCE_BSD
        SharedResourcePointer<detail::HostDrivenEventLoop> hostEventLoop;
       #endif

        //==============================================================================
        JuceVSTWrapper& wrapper;
        bool resizingChild = false, resizingParent = false;

        juce::Rectangle<int> lastBounds;

       #if JUCE_LINUX || JUCE_BSD
        using HostWindowType = ::Window;
        ::Display* display = XWindowSystem::getInstance()->getDisplay();
       #elif JUCE_WINDOWS
        using HostWindowType = HWND;
        detail::WindowsHooks hooks;
       #else
        using HostWindowType = void*;
       #endif

        HostWindowType hostWindow = {};

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EditorCompWrapper)
    };

    //==============================================================================
private:
    struct HostChangeUpdater final : private AsyncUpdater
    {
        explicit HostChangeUpdater (JuceVSTWrapper& o)  : owner (o) {}
        ~HostChangeUpdater() override  { cancelPendingUpdate(); }

        void update (const ChangeDetails& details)
        {
            if (details.latencyChanged)
            {
                owner.vstEffect.initialDelay = owner.processor->getLatencySamples();
                callbackBits |= audioMasterIOChangedBit;
            }

            if (details.parameterInfoChanged || details.programChanged)
                callbackBits |= audioMasterUpdateDisplayBit;

            triggerAsyncUpdate();
        }

    private:
        void handleAsyncUpdate() override
        {
            const auto callbacksToFire = callbackBits.exchange (0);

            if (auto* callback = owner.hostCallback)
            {
                struct FlagPair
                {
                    Vst2::AudioMasterOpcodesX opcode;
                    int bit;
                };

                constexpr FlagPair pairs[] { { Vst2::audioMasterUpdateDisplay, audioMasterUpdateDisplayBit },
                                             { Vst2::audioMasterIOChanged,     audioMasterIOChangedBit } };

                for (const auto& pair : pairs)
                    if ((callbacksToFire & pair.bit) != 0)
                        callback (&owner.vstEffect, pair.opcode, 0, 0, nullptr, 0);
            }
        }

        static constexpr auto audioMasterUpdateDisplayBit = 1 << 0;
        static constexpr auto audioMasterIOChangedBit     = 1 << 1;

        JuceVSTWrapper& owner;
        std::atomic<int> callbackBits { 0 };
    };

    static JuceVSTWrapper* getWrapper (Vst2::AEffect* v) noexcept  { return static_cast<JuceVSTWrapper*> (v->object); }

    bool isProcessLevelOffline()
    {
        return hostCallback != nullptr
                && (int32) hostCallback (&vstEffect, Vst2::audioMasterGetCurrentProcessLevel, 0, 0, nullptr, 0) == 4;
    }

    static int32 convertHexVersionToDecimal (const unsigned int hexVersion)
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
        auto host = detail::PluginUtilities::getHostType();

        if (host.isWavelab() || host.isCubaseBridged() || host.isPremiere())
        {
            if (! messageThreadIsDefinitelyCorrect)
            {
                MessageManager::getInstance()->setCurrentThreadAsMessageThread();

                struct MessageThreadCallback final : public CallbackMessage
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

    void setValueAndNotifyIfChanged (AudioProcessorParameter& param, float newValue)
    {
        if (approximatelyEqual (param.getValue(), newValue))
            return;

        inParameterChangedCallback = true;
        param.setValueNotifyingHost (newValue);
    }

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
        setHasEditorFlag (processor->hasEditor());

        return 0;
    }

    pointer_sized_int handleClose (VstOpCodeArguments)
    {
        // Note: most hosts call this on the UI thread, but wavelab doesn't, so be careful in here.
        timedCallback.stopTimer();

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
       #if JUCE_LINUX || JUCE_BSD
        SharedResourcePointer<detail::HostDrivenEventLoop> hostDrivenEventLoop;
       #else
        const MessageManagerLock mmLock;
       #endif
        createEditorComp();

        if (editorComp != nullptr)
        {
            editorComp->getEditorBounds (editorRect);
            *((Vst2::ERect**) args.ptr) = &editorRect;
            return (pointer_sized_int) &editorRect;
        }

        return 0;
    }

    pointer_sized_int handleOpenEditor (VstOpCodeArguments args)
    {
        checkWhetherMessageThreadIsCorrect();
       #if JUCE_LINUX || JUCE_BSD
        SharedResourcePointer<detail::HostDrivenEventLoop> hostDrivenEventLoop;
       #else
        const MessageManagerLock mmLock;
       #endif
        jassert (! recursionCheck);

        timedCallback.startTimerHz (4); // performs misc housekeeping chores

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

       #if JUCE_LINUX || JUCE_BSD
        SharedResourcePointer<detail::HostDrivenEventLoop> hostDrivenEventLoop;
       #else
        const MessageManagerLock mmLock;
       #endif

        deleteEditor (true);

        return 0;
    }

    pointer_sized_int handleGetData (VstOpCodeArguments args)
    {
        if (processor == nullptr)
            return 0;

        auto data = (void**) args.ptr;
        bool onlyStoreCurrentProgramData = (args.index != 0);

        MemoryBlock block;

        if (onlyStoreCurrentProgramData)
            processor->getCurrentProgramStateInformation (block);
        else
            processor->getStateInformation (block);

        // IMPORTANT! Don't call getStateInfo while holding this lock!
        const ScopedLock lock (stateInformationLock);

        chunkMemory = std::move (block);
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

            {
                const ScopedLock lock (stateInformationLock);

                chunkMemory.reset();
                chunkMemoryTime = 0;
            }

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

    pointer_sized_int handlePreAudioProcessingEvents ([[maybe_unused]] VstOpCodeArguments args)
    {
        if (supportsMidiIn)
        {
            VSTMidiEventList::addEventsToMidiBuffer ((Vst2::VstEvents*) args.ptr, midiEvents);
            return 1;
        }

        return 0;
    }

    pointer_sized_int handleIsParameterAutomatable (VstOpCodeArguments args)
    {
        if (auto* param = juceParameters.getParamForIndex (args.index))
        {
            const bool isMeter = ((((unsigned int) param->getCategory() & 0xffff0000) >> 16) == 2);
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
                setValueAndNotifyIfChanged (*param, param->getValueForText (String::fromUTF8 ((char*) args.ptr)));
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
        isBypassed = args.value != 0;

        if (auto* param = processor->getBypassParameter())
            param->setValueNotifyingHost (isBypassed ? 1.0f : 0.0f);

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

    static std::optional<pointer_sized_int> handleVST3Compatibility ([[maybe_unused]] VstOpCodeArguments args)
    {
       #if ! JUCE_VST3_CAN_REPLACE_VST2
        return {};
       #else
        if (args.index != (int32) ByteOrder::bigEndianInt ("stCA")
            && args.index != (int32) ByteOrder::bigEndianInt ("stCa"))
            return {};

        if (args.value != (int32) ByteOrder::bigEndianInt ("FUID"))
            return {};

        if (args.ptr == nullptr)
            return 0;

        const auto uid = VST3Interface::vst2PluginId (JucePlugin_VSTUniqueID,
                                                      JucePlugin_Name,
                                                      VST3Interface::Type::component);
        std::copy (uid.begin(), uid.end(), reinterpret_cast<std::byte*> (args.ptr));
        return 1;
       #endif
    }

    pointer_sized_int handleManufacturerSpecific (VstOpCodeArguments args)
    {
        if (const auto result = handleVST3Compatibility (args))
            return *result;

        if (args.index == (int32) ByteOrder::bigEndianInt ("PreS")
             && args.value == (int32) ByteOrder::bigEndianInt ("AeCs"))
            return handleSetContentScaleFactor (args.opt);

        if (args.index == Vst2::effGetParamDisplay)
            return handleCockosGetParameterText (args.value, args.ptr, args.opt);

        if (auto callbackHandler = processor->getVST2ClientExtensions())
            return callbackHandler->handleVstManufacturerSpecific (args.index, args.value, args.ptr, args.opt);

        return 0;
    }

    pointer_sized_int handleCanPlugInDo (VstOpCodeArguments args)
    {
        auto text = (const char*) args.ptr;
        auto matches = [=] (const char* s) { return strcmp (text, s) == 0; };

        if (matches ("receiveVstEvents")
         || matches ("receiveVstMidiEvent")
         || matches ("receiveVstMidiEvents"))
        {
            return supportsMidiIn ? 1 : -1;
        }

        if (matches ("sendVstEvents")
         || matches ("sendVstMidiEvent")
         || matches ("sendVstMidiEvents"))
        {
            return supportsMidiOut ? 1 : -1;
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
            return (int32) 0xbeef0000;
        }
       #endif

        if (matches ("hasCockosExtensions"))
            return (int32) 0xbeef0000;

        if (auto callbackHandler = processor->getVST2ClientExtensions())
            return callbackHandler->handleVstPluginCanDo (args.index, args.value, args.ptr, args.opt);

        return 0;
    }

    pointer_sized_int handleGetTailSize (VstOpCodeArguments)
    {
        if (processor != nullptr)
        {
            int32 result;

            auto tailSeconds = processor->getTailLengthSeconds();

            if (std::isinf (tailSeconds))
                result = std::numeric_limits<int32>::max();
            else
                result = static_cast<int32> (tailSeconds * sampleRate);

            return result; // Vst2 expects an int32 upcasted to a intptr_t here
        }

        return 0;
    }

    pointer_sized_int handleKeyboardFocusRequired (VstOpCodeArguments)
    {
        JUCE_BEGIN_IGNORE_WARNINGS_MSVC (6326)
        return (JucePlugin_EditorRequiresKeyboardFocus != 0) ? 1 : 0;
        JUCE_END_IGNORE_WARNINGS_MSVC
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

        auto inputLayout  = processor->getChannelLayoutOfBus (true, 0);
        auto outputLayout = processor->getChannelLayoutOfBus (false, 0);

        const auto speakerBaseSize = offsetof (Vst2::VstSpeakerArrangement, speakers);

        cachedInArrangement .malloc (speakerBaseSize + (static_cast<std::size_t> (inputLayout. size()) * sizeof (Vst2::VstSpeakerProperties)), 1);
        cachedOutArrangement.malloc (speakerBaseSize + (static_cast<std::size_t> (outputLayout.size()) * sizeof (Vst2::VstSpeakerProperties)), 1);

        *pluginInput  = cachedInArrangement. getData();
        *pluginOutput = cachedOutArrangement.getData();

        if (*pluginInput != nullptr)
            SpeakerMappings::channelSetToVstArrangement (processor->getChannelLayoutOfBus (true,  0), **pluginInput);

        if (*pluginOutput != nullptr)
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

    pointer_sized_int handleSetContentScaleFactor ([[maybe_unused]] float scale, [[maybe_unused]] bool force = false)
    {
        checkWhetherMessageThreadIsCorrect();
       #if JUCE_LINUX || JUCE_BSD
        SharedResourcePointer<detail::HostDrivenEventLoop> hostDrivenEventLoop;
       #else
        const MessageManagerLock mmLock;
       #endif

       #if ! JUCE_MAC
        if (force || ! approximatelyEqual (scale, editorScaleFactor))
        {
            editorScaleFactor = scale;

            if (editorComp != nullptr)
                editorComp->setContentScaleFactor (editorScaleFactor);
        }
       #endif

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
        if (supportsMidiIn)
        {
           #ifdef JucePlugin_VSTNumMidiInputs
            return JucePlugin_VSTNumMidiInputs;
           #else
            return 16;
           #endif
        }

        return 0;
    }

    pointer_sized_int handleGetNumMidiOutputChannels()
    {
        if (supportsMidiOut)
        {
           #ifdef JucePlugin_VSTNumMidiOutputs
            return JucePlugin_VSTNumMidiOutputs;
           #else
            return 16;
           #endif
        }

        return 0;
    }

    pointer_sized_int handleGetMidiKeyName (VstOpCodeArguments args)
    {
        if (processor != nullptr)
        {
            auto keyName = (Vst2::MidiKeyName*) args.ptr;

            if (auto name = processor->getNameForMidiNoteNumber (keyName->thisKeyNumber, args.index))
            {
                name->copyToUTF8 (keyName->keyName, Vst2::kVstMaxNameLen);
                return 1;
            }
        }

        return 0;
    }

    pointer_sized_int handleEditIdle()
    {
       #if JUCE_LINUX || JUCE_BSD
        SharedResourcePointer<detail::HostDrivenEventLoop> hostDrivenEventLoop;
        hostDrivenEventLoop->processPendingEvents();
       #endif

        return 0;
    }

    //==============================================================================
    ScopedJuceInitialiser_GUI libraryInitialiser;

   #if JUCE_LINUX || JUCE_BSD
    SharedResourcePointer<detail::MessageThread> messageThread;
   #endif

    TimedCallback timedCallback { [this]
    {
        if (shouldDeleteEditor)
        {
            shouldDeleteEditor = false;
            deleteEditor (true);
        }

        {
            ScopedLock lock (stateInformationLock);

            if (chunkMemoryTime > 0
                && chunkMemoryTime < juce::Time::getApproximateMillisecondCounter() - 2000
                && ! recursionCheck)
            {
                chunkMemory.reset();
                chunkMemoryTime = 0;
            }
        }
    } };

    Vst2::audioMasterCallback hostCallback;
    std::unique_ptr<AudioProcessor> processor;
    double sampleRate = 44100.0;
    int32 blockSize = 1024;
    Vst2::AEffect vstEffect;
    CriticalSection stateInformationLock;
    juce::MemoryBlock chunkMemory;
    uint32 chunkMemoryTime = 0;
    float editorScaleFactor = 1.0f;
    std::unique_ptr<EditorCompWrapper> editorComp;
    Vst2::ERect editorRect;
    MidiBuffer midiEvents;
    VSTMidiEventList outgoingEvents;
    Optional<PositionInfo> currentPosition;

    LegacyAudioParametersWrapper juceParameters;

    bool isProcessing = false, isBypassed = false, hasShutdown = false;
    bool firstProcessCallback = true, shouldDeleteEditor = false;
    const bool supportsMidiIn  = processor->isMidiEffect() || processor->acceptsMidi();
    const bool supportsMidiOut = processor->isMidiEffect() || processor->producesMidi();

    VstTempBuffers<float> floatTempBuffers;
    VstTempBuffers<double> doubleTempBuffers;
    int maxNumInChannels = 0, maxNumOutChannels = 0;

    HeapBlock<Vst2::VstSpeakerArrangement> cachedInArrangement, cachedOutArrangement;

    ThreadLocalValue<bool> inParameterChangedCallback;

    HostChangeUpdater hostChangeUpdater { *this };

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
            ScopedJuceInitialiser_GUI libraryInitialiser;

           #if JUCE_LINUX || JUCE_BSD
            SharedResourcePointer<detail::HostDrivenEventLoop> hostDrivenEventLoop;
           #endif

            try
            {
                if (audioMaster (nullptr, Vst2::audioMasterVersion, 0, 0, nullptr, 0) != 0)
                {
                    std::unique_ptr<AudioProcessor> processor { createPluginFilterOfType (AudioProcessor::wrapperType_VST) };
                    auto* processorPtr = processor.get();
                    auto* wrapper = new JuceVSTWrapper (audioMaster, std::move (processor));
                    auto* aEffect = wrapper->getAEffect();

                    if (auto* callbackHandler = processorPtr->getVST2ClientExtensions())
                    {
                        callbackHandler->handleVstHostCallbackAvailable ([audioMaster, aEffect] (int32 opcode, int32 index, pointer_sized_int value, void* ptr, float opt)
                        {
                            return audioMaster (aEffect, opcode, index, value, ptr, opt);
                        });
                    }

                    return aEffect;
                }
            }
            catch (...)
            {}
        }

        return nullptr;
    }
}

#if ! JUCE_WINDOWS
 #define JUCE_EXPORTED_FUNCTION extern "C" __attribute__ ((visibility ("default")))
#endif

JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wmissing-prototypes")

//==============================================================================
// Mac startup code..
#if JUCE_MAC

    JUCE_EXPORTED_FUNCTION Vst2::AEffect* VSTPluginMain (Vst2::audioMasterCallback audioMaster);
    JUCE_EXPORTED_FUNCTION Vst2::AEffect* VSTPluginMain (Vst2::audioMasterCallback audioMaster)
    {
        return pluginEntryPoint (audioMaster);
    }

    JUCE_EXPORTED_FUNCTION Vst2::AEffect* main_macho (Vst2::audioMasterCallback audioMaster);
    JUCE_EXPORTED_FUNCTION Vst2::AEffect* main_macho (Vst2::audioMasterCallback audioMaster)
    {
        return pluginEntryPoint (audioMaster);
    }

//==============================================================================
// Linux startup code..
#elif JUCE_LINUX || JUCE_BSD

    JUCE_EXPORTED_FUNCTION Vst2::AEffect* VSTPluginMain (Vst2::audioMasterCallback audioMaster);
    JUCE_EXPORTED_FUNCTION Vst2::AEffect* VSTPluginMain (Vst2::audioMasterCallback audioMaster)
    {
        return pluginEntryPoint (audioMaster);
    }

    JUCE_EXPORTED_FUNCTION Vst2::AEffect* main_plugin (Vst2::audioMasterCallback audioMaster) asm ("main");
    JUCE_EXPORTED_FUNCTION Vst2::AEffect* main_plugin (Vst2::audioMasterCallback audioMaster)
    {
        return VSTPluginMain (audioMaster);
    }

    // don't put initialiseJuce_GUI or shutdownJuce_GUI in these... it will crash!
    __attribute__ ((constructor)) void myPluginInit() {}
    __attribute__ ((destructor))  void myPluginFini() {}

//==============================================================================
// Win32 startup code..
#else

    extern "C" __declspec (dllexport) Vst2::AEffect* VSTPluginMain (Vst2::audioMasterCallback audioMaster)
    {
        return pluginEntryPoint (audioMaster);
    }

   #if ! defined (JUCE_64BIT) && JUCE_MSVC // (can't compile this on win64, but it's not needed anyway with VST2.4)
    extern "C" __declspec (dllexport) int main (Vst2::audioMasterCallback audioMaster)
    {
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

JUCE_END_IGNORE_WARNINGS_GCC_LIKE

JUCE_END_IGNORE_WARNINGS_MSVC

#endif

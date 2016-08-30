/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

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

#ifdef __clang__
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

#include "../../juce_audio_processors/format_types/juce_VSTInterface.h"

#ifndef JUCE_VST3_CAN_REPLACE_VST2
 #define JUCE_VST3_CAN_REPLACE_VST2 1
#endif

#if JucePlugin_Build_VST3 && JUCE_VST3_CAN_REPLACE_VST2
 #include <pluginterfaces/base/funknown.h>
 namespace juce { extern Steinberg::FUID getJuceVST3ComponentIID(); }
#endif

#ifdef _MSC_VER
 #pragma warning (pop)
#endif

#ifdef __clang__
 #pragma clang diagnostic pop
#endif

//==============================================================================
#ifdef _MSC_VER
 #pragma pack (push, 8)
#endif

#include "../utility/juce_IncludeModuleHeaders.h"
#include "../utility/juce_FakeMouseMoveGenerator.h"
#include "../utility/juce_WindowsHooks.h"
#include "../utility/juce_PluginBusUtilities.h"

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

 #if JUCE_LINUX
  extern Display* display;
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

        while (w != 0)
        {
            HWND parent = getWindowParent (w);

            if (parent == 0)
                break;

            TCHAR windowType[32] = { 0 };
            GetClassName (parent, windowType, 31);

            if (String (windowType).equalsIgnoreCase ("MDIClient"))
                return parent;

            RECT windowPos, parentPos;
            GetWindowRect (w, &windowPos);
            GetWindowRect (parent, &parentPos);

            const int dw = (parentPos.right - parentPos.left) - (windowPos.right - windowPos.left);
            const int dh = (parentPos.bottom - parentPos.top) - (windowPos.bottom - windowPos.top);

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

class SharedMessageThread : public Thread
{
public:
    SharedMessageThread()
      : Thread ("VstMessageThread"),
        initialised (false)
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

        while ((! threadShouldExit()) && MessageManager::getInstance()->runDispatchLoopUntil (250))
        {}
    }

    juce_DeclareSingleton (SharedMessageThread, false)

private:
    bool initialised;
};

juce_ImplementSingleton (SharedMessageThread)

#endif

static Array<void*> activePlugins;

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
            for (int i = tempChannels.size(); --i >= 0;)
                delete[] (tempChannels.getUnchecked(i));

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
    JuceVSTWrapper (VstHostCallback cb, AudioProcessor* const af)
       : hostCallback (cb),
         sampleRate (44100.f),
         blockSize (1024),
         filter (af),
         busUtils (*filter, true, 64),
         chunkMemoryTime (0),
         isProcessing (false),
         isBypassed (false),
         hasShutdown (false),
         isInSizeWindow (false),
         firstProcessCallback (true),
         shouldDeleteEditor (false),
        #if JUCE_64BIT
         useNSView (true),
        #else
         useNSView (false),
        #endif
         hostWindow (0)
    {
        busUtils.init();

        // VST-2 does not support disabling buses: so always enable all of them
        if (busUtils.hasDynamicInBuses() || busUtils.hasDynamicOutBuses())
            busUtils.enableAllBuses();

        {
            // Using the legacy Projucer field? Then keep the maximum number of channels
            // as the default plug-in layout. Otherwise, leave it up to the user
            // which default layout the prefer.
          #ifndef JucePlugin_PreferredChannelConfigurations
            PluginBusUtilities::ScopedBusRestorer busRestorer (busUtils);
          #endif

            findMaxTotalChannels (maxNumInChannels, maxNumOutChannels);
            bool success = setBusArrangementFromTotalChannelNum (maxNumInChannels, maxNumOutChannels);
            ignoreUnused (success);

            // please file a bug if you hit this assertion!
            jassert (maxNumInChannels  == busUtils.findTotalNumChannels (true) && success
                  && maxNumOutChannels == busUtils.findTotalNumChannels (false));
        }

        filter->setRateAndBufferSizeDetails (0, 0);
        filter->setPlayHead (this);
        filter->addListener (this);

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
        vstEffect.latency = filter->getLatencySamples();
        vstEffect.effectPointer = this;
        vstEffect.plugInIdentifier = JucePlugin_VSTUniqueID;
        vstEffect.plugInVersion = convertHexVersionToDecimal (JucePlugin_VersionCode);
        vstEffect.processAudioInplaceFunction = processReplacingCB;
        vstEffect.processDoubleAudioInplaceFunction = processDoubleReplacingCB;

        vstEffect.flags |= vstEffectFlagHasEditor;

        vstEffect.flags |= vstEffectFlagInplaceAudio;
        if (filter->supportsDoublePrecisionProcessing())
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

                delete filter;
                filter = nullptr;

                jassert (editorComp == 0);

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
        if (firstProcessCallback)
        {
            firstProcessCallback = false;

            // if this fails, the host hasn't called resume() before processing
            jassert (isProcessing);

            // (tragically, some hosts actually need this, although it's stupid to have
            //  to do it here..)
            if (! isProcessing)
                resume();

            filter->setNonRealtime (isProcessLevelOffline());

           #if JUCE_WINDOWS
            if (getHostType().isWavelab())
            {
                int priority = GetThreadPriority (GetCurrentThread());

                if (priority <= THREAD_PRIORITY_NORMAL && priority >= THREAD_PRIORITY_LOWEST)
                    filter->setNonRealtime (true);
            }
           #endif
        }

       #if JUCE_DEBUG && ! JucePlugin_ProducesMidiOutput
        const int numMidiEventsComingIn = midiEvents.getNumEvents();
       #endif

        jassert (activePlugins.contains (this));

        {
            const int numIn  = filter->getTotalNumInputChannels();
            const int numOut = filter->getTotalNumOutputChannels();

            const ScopedLock sl (filter->getCallbackLock());

            if (filter->isSuspended())
            {
                for (int i = 0; i < numOut; ++i)
                    FloatVectorOperations::clear (outputs[i], numSamples);
            }
            else
            {
                int i;
                for (i = 0; i < numOut; ++i)
                {
                    FloatType* chan = tmpBuffers.tempChannels.getUnchecked(i);

                    if (chan == nullptr)
                    {
                        chan = outputs[i];

                        // if some output channels are disabled, some hosts supply the same buffer
                        // for multiple channels - this buggers up our method of copying the
                        // inputs over the outputs, so we need to create unique temp buffers in this case..
                        for (int j = i; --j >= 0;)
                        {
                            if (outputs[j] == chan)
                            {
                                chan = new FloatType [blockSize * 2];
                                tmpBuffers.tempChannels.set (i, chan);
                                break;
                            }
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
                    AudioBuffer<FloatType> chans (tmpBuffers.channels, numChannels, numSamples);

                    if (isBypassed)
                        filter->processBlockBypassed (chans, midiEvents);
                    else
                        filter->processBlock (chans, midiEvents);
                }

                // copy back any temp channels that may have been used..
                for (i = 0; i < numOut; ++i)
                    if (const FloatType* const chan = tmpBuffers.tempChannels.getUnchecked(i))
                        memcpy (outputs[i], chan, sizeof (FloatType) * (size_t) numSamples);
            }
        }

        if (! midiEvents.isEmpty())
        {
           #if JucePlugin_ProducesMidiOutput
            const int numEvents = midiEvents.getNumEvents();

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
        jassert (! filter->isUsingDoublePrecision());
        internalProcessReplacing (inputs, outputs, sampleFrames, floatTempBuffers);
    }

    static void processReplacingCB (VstEffectInterface* vstInterface, float** inputs, float** outputs, int32 sampleFrames)
    {
        getWrapper (vstInterface)->processReplacing (inputs, outputs, sampleFrames);
    }

    void processDoubleReplacing (double** inputs, double** outputs, int32 sampleFrames)
    {
        jassert (filter->isUsingDoublePrecision());
        internalProcessReplacing (inputs, outputs, sampleFrames, doubleTempBuffers);
    }

    static void processDoubleReplacingCB (VstEffectInterface* vstInterface, double** inputs, double** outputs, int32 sampleFrames)
    {
        getWrapper (vstInterface)->processDoubleReplacing (inputs, outputs, sampleFrames);
    }

    //==============================================================================
    void resume()
    {
        if (filter != nullptr)
        {
            isProcessing = true;

            const size_t nInAndOutChannels = static_cast<size_t> (vstEffect.numInputChannels + vstEffect.numOutputChannels);
            floatTempBuffers .channels.calloc (nInAndOutChannels);
            doubleTempBuffers.channels.calloc (nInAndOutChannels);

            const double currentRate = sampleRate;
            const int currentBlockSize = blockSize;

            firstProcessCallback = true;

            filter->setNonRealtime (isProcessLevelOffline());
            filter->setRateAndBufferSizeDetails (currentRate, currentBlockSize);

            deleteTempChannels();

            filter->prepareToPlay (currentRate, currentBlockSize);

            midiEvents.ensureSize (2048);
            midiEvents.clear();

            vstEffect.latency = filter->getLatencySamples();

            /** If this plug-in is a synth or it can receive midi events we need to tell the
                host that we want midi. In the SDK this method is marked as deprecated, but
                some hosts rely on this behaviour.
            */
            if (vstEffect.flags & vstEffectFlagIsSynth || JucePlugin_WantsMidiInput)
            {
                if (hostCallback != nullptr)
                    hostCallback (&vstEffect, hostOpcodePlugInWantsMidi, 0, 1, 0, 0);
            }

           #if JucePlugin_ProducesMidiOutput
            outgoingEvents.ensureSize (512);
           #endif
        }
    }

    void suspend()
    {
        if (filter != nullptr)
        {
            filter->releaseResources();
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

            pointer_sized_int result = hostCallback (&vstEffect, hostOpcodeGetTimingInfo, 0, flags, 0, 0);
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
                case vstSmpteRateFps24:        rate = AudioPlayHead::fps24;       fps = 24.0;  break;
                case vstSmpteRateFps25:        rate = AudioPlayHead::fps25;       fps = 25.0;  break;
                case vstSmpteRateFps2997:      rate = AudioPlayHead::fps2997;     fps = 29.97; break;
                case vstSmpteRateFps30:        rate = AudioPlayHead::fps30;       fps = 30.0;  break;
                case vstSmpteRateFps2997drop:  rate = AudioPlayHead::fps2997drop; fps = 29.97; break;
                case vstSmpteRateFps30drop:    rate = AudioPlayHead::fps30drop;   fps = 30.0;  break;

                case vstSmpteRate16mmFilm:
                case vstSmpteRate35mmFilm:     fps = 24.0; break;

                case vstSmpteRateFps239:       fps = 23.976; break;
                case vstSmpteRateFps249:       fps = 24.976; break;
                case vstSmpteRateFps599:       fps = 59.94; break;
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
        if (filter == nullptr)
            return 0.0f;

        jassert (isPositiveAndBelow (index, filter->getNumParameters()));
        return filter->getParameter (index);
    }

    static float getParameterCB (VstEffectInterface* vstInterface, int32 index)
    {
        return getWrapper (vstInterface)->getParameter (index);
    }

    void setParameter (int32 index, float value)
    {
        if (filter != nullptr)
        {
            jassert (isPositiveAndBelow (index, filter->getNumParameters()));
            filter->setParameter (index, value);
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
        vstEffect.latency = filter->getLatencySamples();

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
        // fill with default
        properties.flags = vstPinInfoFlagValid;
        properties.text[0] = 0;
        properties.shortText[0] = 0;
        properties.configurationType = vstSpeakerConfigTypeEmpty;

        // index refers to the absolute index when combining all channels of every bus
        if (index >= (direction ? vstEffect.numInputChannels : vstEffect.numOutputChannels))
            return false;

        const int n = busUtils.getBusCount(direction);
        int busIdx;
        for (busIdx = 0; busIdx < n; ++busIdx)
        {
            const int numChans = busUtils.getNumChannels (direction, busIdx);
            if (index < numChans)
                break;

            index -= numChans;
        }

        if (busIdx >= n)
            return true;

        const AudioProcessor::AudioProcessorBus& busInfo = busUtils.getFilterBus (direction).getReference (busIdx);

       #ifdef JucePlugin_PreferredChannelConfigurations
        String abbvChannelName = String (index);
       #else
        String abbvChannelName = AudioChannelSet::getAbbreviatedChannelTypeName (busInfo.channels.getTypeOfChannel(index));
       #endif

        String channelName = busInfo.name + String (" ") + abbvChannelName;

        channelName.copyToUTF8 (properties.text, (size_t) (vstMaxParameterOrPinLabelLength + 1));
        channelName.copyToUTF8 (properties.shortText, (size_t) (vstMaxParameterOrPinShortLabelLength + 1));

        properties.flags = vstPinInfoFlagValid | vstPinInfoFlagIsActive;
        properties.configurationType = SpeakerMappings::channelSetToVstArrangementType (busInfo.channels);

        if (properties.configurationType == vstSpeakerConfigTypeEmpty)
            properties.flags &= vstPinInfoFlagIsActive;

        if (busInfo.channels.size() == 2)
            properties.flags |= vstPinInfoFlagIsStereo;

        return true;
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
            for (const Mapping* m = getMappings(); m->vst2 != vstSpeakerConfigTypeEmpty; ++m)
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
            Array<AudioChannelSet::ChannelType> chans (channels.getChannelTypes());

            if (channels == AudioChannelSet::disabled())
                return vstSpeakerConfigTypeEmpty;

            for (const Mapping* m = getMappings(); m->vst2 != vstSpeakerConfigTypeEmpty; ++m)
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
                VstIndividualSpeakerInfo& speaker = result.speakers[i];

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
                { vstSpeakerConfigTypeSlSr,                          { leftRearSurround, rightRearSurround, unknown } },
                { vstSpeakerConfigTypeCLfe,                          { centre, subbass, unknown } },
                { vstSpeakerConfigTypeLRC,                           { left, right, centre, unknown } },
                { vstSpeakerConfigTypeLRS,                           { left, right, surround, unknown } },
                { vstSpeakerConfigTypeLRCLfe,                        { left, right, centre, subbass, unknown } },
                { vstSpeakerConfigTypeLRLfeS,                        { left, right, subbass, surround, unknown } },
                { vstSpeakerConfigTypeLRCS,                          { left, right, centre, surround, unknown } },
                { vstSpeakerConfigTypeLRLsRs,                        { left, right, leftSurround, rightSurround, unknown } },
                { vstSpeakerConfigTypeLRCLfeS,                       { left, right, centre, subbass, surround, unknown } },
                { vstSpeakerConfigTypeLRLfeLsRs,                     { left, right, subbass, leftSurround, rightSurround, unknown } },
                { vstSpeakerConfigTypeLRCLsRs,                       { left, right, centre, leftSurround, rightSurround, unknown } },
                { vstSpeakerConfigTypeLRCLfeLsRs,                    { left, right, centre, subbass, leftSurround, rightSurround, unknown } },
                { vstSpeakerConfigTypeLRCLsRsCs,                     { left, right, centre, leftSurround, rightSurround, surround, unknown } },
                { vstSpeakerConfigTypeLRLsRsSlSr,                    { left, right, leftSurround, rightSurround, leftRearSurround, rightRearSurround, unknown } },
                { vstSpeakerConfigTypeLRCLfeLsRsCs,                  { left, right, centre, subbass, leftSurround, rightSurround, surround, unknown } },
                { vstSpeakerConfigTypeLRLfeLsRsSlSr,                 { left, right, subbass, leftSurround, rightSurround, leftRearSurround, rightRearSurround, unknown } },
                { vstSpeakerConfigTypeLRCLsRsLcRc,                   { left, right, centre, leftSurround, rightSurround, topFrontLeft, topFrontRight, unknown } },
                { vstSpeakerConfigTypeLRCLsRsSlSr,                   { left, right, centre, leftSurround, rightSurround, leftRearSurround, rightRearSurround, unknown } },
                { vstSpeakerConfigTypeLRCLfeLsRsLcRc,                { left, right, centre, subbass, leftSurround, rightSurround, topFrontLeft, topFrontRight, unknown } },
                { vstSpeakerConfigTypeLRCLfeLsRsSlSr,                { left, right, centre, subbass, leftSurround, rightSurround, leftRearSurround, rightRearSurround, unknown } },
                { vstSpeakerConfigTypeLRCLsRsLcRcCs,                 { left, right, centre, leftSurround, rightSurround, topFrontLeft, topFrontRight, surround, unknown } },
                { vstSpeakerConfigTypeLRCLsRsCsSlSr,                 { left, right, centre, leftSurround, rightSurround, surround, leftRearSurround, rightRearSurround, unknown } },
                { vstSpeakerConfigTypeLRCLfeLsRsLcRcCs,              { left, right, centre, subbass, leftSurround, rightSurround, topFrontLeft, topFrontRight, surround, unknown } },
                { vstSpeakerConfigTypeLRCLfeLsRsCsSlSr,              { left, right, centre, subbass, leftSurround, rightSurround, surround, leftRearSurround, rightRearSurround, unknown } },
                { vstSpeakerConfigTypeLRCLfeLsRsTflTfcTfrTrlTrrLfe2, { left, right, centre, subbass, leftSurround, rightSurround, topFrontLeft, topFrontCentre, topFrontRight, topRearLeft, topRearRight, subbass2, unknown } },
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
                case AudioChannelSet::subbass:           return vstIndividualSpeakerTypeSubbass;
                case AudioChannelSet::leftSurround:      return vstIndividualSpeakerTypeLeftSurround;
                case AudioChannelSet::rightSurround:     return vstIndividualSpeakerTypeRightSurround;
                case AudioChannelSet::leftCentre:        return vstIndividualSpeakerTypeLeftCentre;
                case AudioChannelSet::rightCentre:       return vstIndividualSpeakerTypeRightCentre;
                case AudioChannelSet::surround:          return vstIndividualSpeakerTypeSurround;
                case AudioChannelSet::leftRearSurround:  return vstIndividualSpeakerTypeLeftRearSurround;
                case AudioChannelSet::rightRearSurround: return vstIndividualSpeakerTypeRightRearSurround;
                case AudioChannelSet::topMiddle:         return vstIndividualSpeakerTypeTopMiddle;
                case AudioChannelSet::topFrontLeft:      return vstIndividualSpeakerTypeTopFrontLeft;
                case AudioChannelSet::topFrontCentre:    return vstIndividualSpeakerTypeTopFrontCentre;
                case AudioChannelSet::topFrontRight:     return vstIndividualSpeakerTypeTopFrontRight;
                case AudioChannelSet::topRearLeft:       return vstIndividualSpeakerTypeTopRearLeft;
                case AudioChannelSet::topRearCentre:     return vstIndividualSpeakerTypeTopRearCentre;
                case AudioChannelSet::topRearRight:      return vstIndividualSpeakerTypeTopRearRight;
                case AudioChannelSet::subbass2:          return vstIndividualSpeakerTypeSubbass2;
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
                case vstIndividualSpeakerTypeSubbass:           return AudioChannelSet::subbass;
                case vstIndividualSpeakerTypeLeftSurround:      return AudioChannelSet::leftSurround;
                case vstIndividualSpeakerTypeRightSurround:     return AudioChannelSet::rightSurround;
                case vstIndividualSpeakerTypeLeftCentre:        return AudioChannelSet::leftCentre;
                case vstIndividualSpeakerTypeRightCentre:       return AudioChannelSet::rightCentre;
                case vstIndividualSpeakerTypeSurround:          return AudioChannelSet::surround;
                case vstIndividualSpeakerTypeLeftRearSurround:  return AudioChannelSet::leftRearSurround;
                case vstIndividualSpeakerTypeRightRearSurround: return AudioChannelSet::rightRearSurround;
                case vstIndividualSpeakerTypeTopMiddle:         return AudioChannelSet::topMiddle;
                case vstIndividualSpeakerTypeTopFrontLeft:      return AudioChannelSet::topFrontLeft;
                case vstIndividualSpeakerTypeTopFrontCentre:    return AudioChannelSet::topFrontCentre;
                case vstIndividualSpeakerTypeTopFrontRight:     return AudioChannelSet::topFrontRight;
                case vstIndividualSpeakerTypeTopRearLeft:       return AudioChannelSet::topRearLeft;
                case vstIndividualSpeakerTypeTopRearCentre:     return AudioChannelSet::topRearCentre;
                case vstIndividualSpeakerTypeTopRearRight:      return AudioChannelSet::topRearRight;
                case vstIndividualSpeakerTypeSubbass2:          return AudioChannelSet::subbass2;
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

       #if JUCE_MAC
        if (hostWindow != 0)
            checkWindowVisibilityVST (hostWindow, editorComp, useNSView);
       #endif
    }

    void createEditorComp()
    {
        if (hasShutdown || filter == nullptr)
            return;

        if (editorComp == nullptr)
        {
            if (AudioProcessorEditor* const ed = filter->createEditorIfNeeded())
            {
                vstEffect.flags |= vstEffectFlagHasEditor;
                ed->setOpaque (true);
                ed->setVisible (true);

                editorComp = new EditorCompWrapper (*this, ed);
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
                if (Component* const modalComponent = Component::getCurrentlyModalComponent())
                {
                    modalComponent->exitModalState (0);

                    if (canDeleteLaterIfModal)
                    {
                        shouldDeleteEditor = true;
                        return;
                    }
                }

               #if JUCE_MAC
                if (hostWindow != 0)
                {
                    detachComponentFromWindowRefVST (editorComp, hostWindow, useNSView);
                    hostWindow = 0;
                }
               #endif

                filter->editorBeingDeleted (editorComp->getEditorComp());

                editorComp = nullptr;

                // there's some kind of component currently modal, but the host
                // is trying to delete our plugin. You should try to avoid this happening..
                jassert (Component::getCurrentlyModalComponent() == nullptr);
            }

           #if JUCE_LINUX
            hostWindow = 0;
           #endif
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
            default:                                      return 0;
        }
    }

    static pointer_sized_int dispatcherCB (VstEffectInterface* vstInterface, int32 opCode, int32 index,
                                           pointer_sized_int value, void* ptr, float opt)
    {
        JuceVSTWrapper* wrapper = getWrapper (vstInterface);
        VstOpCodeArguments args = { index, value, ptr, opt };

        if (opCode == plugInOpcodeClose)
        {
            wrapper->dispatcher (opCode, args);
            delete wrapper;
            return 1;
        }

        return wrapper->dispatcher (opCode, args);
    }

    void resizeHostWindow (int newWidth, int newHeight)
    {
        if (editorComp != nullptr)
        {
            bool sizeWasSuccessful = false;

            if (hostCallback != nullptr)
            {
                if (hostCallback (&vstEffect, hostOpcodeCanHostDo, 0, 0, const_cast<char*> ("sizeWindow"), 0))
                {
                    isInSizeWindow = true;
                    sizeWasSuccessful = (hostCallback (&vstEffect, hostOpcodeWindowSize, newWidth, newHeight, 0, 0) != 0);
                    isInSizeWindow = false;
                }
            }

            if (! sizeWasSuccessful)
            {
                // some hosts don't support the sizeWindow call, so do it manually..
               #if JUCE_MAC
                setNativeHostWindowSizeVST (hostWindow, editorComp, newWidth, newHeight, useNSView);

               #elif JUCE_LINUX
                // (Currently, all linux hosts support sizeWindow, so this should never need to happen)
                editorComp->setSize (newWidth, newHeight);

               #else
                int dw = 0;
                int dh = 0;
                const int frameThickness = GetSystemMetrics (SM_CYFIXEDFRAME);

                HWND w = (HWND) editorComp->getWindowHandle();

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

            if (ComponentPeer* peer = editorComp->getPeer())
            {
                peer->handleMovedOrResized();
                peer->getComponent().repaint();
            }
        }
    }

    //==============================================================================
    // A component to hold the AudioProcessorEditor, and cope with some housekeeping
    // chores when it changes or repaints.
    class EditorCompWrapper  : public Component
    {
    public:
        EditorCompWrapper (JuceVSTWrapper& w, AudioProcessorEditor* editor)
            : wrapper (w)
        {
            setOpaque (true);
            editor->setOpaque (true);

            setBounds (editor->getBounds());
            editor->setTopLeftPosition (0, 0);
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

       #if JUCE_MAC
        bool keyPressed (const KeyPress&) override
        {
            // If we have an unused keypress, move the key-focus to a host window
            // and re-inject the event..
            return forwardCurrentKeyEventToHostVST (this, wrapper.useNSView);
        }
       #endif

        AudioProcessorEditor* getEditorComp() const
        {
            return dynamic_cast<AudioProcessorEditor*> (getChildComponent(0));
        }

        void resized() override
        {
            if (Component* const editorChildComp = getChildComponent(0))
                editorChildComp->setBounds (getLocalBounds());

           #if JUCE_MAC && ! JUCE_64BIT
            if (! wrapper.useNSView)
                updateEditorCompBoundsVST (this);
           #endif
        }

        void childBoundsChanged (Component* child) override
        {
            if (! wrapper.isInSizeWindow)
            {
                child->setTopLeftPosition (0, 0);

                const int cw = child->getWidth();
                const int ch = child->getHeight();

               #if JUCE_MAC
                if (wrapper.useNSView)
                    setTopLeftPosition (0, getHeight() - ch);
               #endif

                wrapper.resizeHostWindow (cw, ch);

               #if ! JUCE_LINUX // setSize() on linux causes renoise and energyxt to fail.
                setSize (cw, ch);
               #else
                XResizeWindow (display, (Window) getWindowHandle(), (unsigned int) cw, (unsigned int) ch);
               #endif

               #if JUCE_MAC
                wrapper.resizeHostWindow (cw, ch);  // (doing this a second time seems to be necessary in tracktion)
               #endif
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

    private:
        //==============================================================================
        JuceVSTWrapper& wrapper;
        FakeMouseMoveGenerator fakeMouseGenerator;

       #if JUCE_WINDOWS
        WindowsHooks hooks;
       #endif

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EditorCompWrapper)
    };

    //==============================================================================
private:
    VstHostCallback hostCallback;
    float sampleRate;
    int32 blockSize;
    VstEffectInterface vstEffect;
    AudioProcessor* filter;
    PluginBusUtilities busUtils;
    juce::MemoryBlock chunkMemory;
    juce::uint32 chunkMemoryTime;
    ScopedPointer<EditorCompWrapper> editorComp;
    VstEditorBounds editorBounds;
    MidiBuffer midiEvents;
    VSTMidiEventList outgoingEvents;
    bool isProcessing, isBypassed, hasShutdown, isInSizeWindow, firstProcessCallback;
    bool shouldDeleteEditor, useNSView;
    VstTempBuffers<float> floatTempBuffers;
    VstTempBuffers<double> doubleTempBuffers;
    int maxNumInChannels, maxNumOutChannels;

   #if JUCE_MAC
    void* hostWindow;
   #elif JUCE_LINUX
    Window hostWindow;
   #else
    HWND hostWindow;
   #endif

    static JuceVSTWrapper* getWrapper (VstEffectInterface* vstInterface) noexcept { return static_cast<JuceVSTWrapper*> (vstInterface->effectPointer); }

    bool isProcessLevelOffline()
    {
        return hostCallback != nullptr && (int32) hostCallback (&vstEffect, hostOpcodeGetCurrentAudioProcessingLevel, 0, 0, 0, 0) == 4;
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
        const PluginHostType host (getHostType());

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

        if (filter != nullptr)
        {
            const int nInputAndOutputChannels = vstEffect.numInputChannels + vstEffect.numOutputChannels;
            tmpBuffers.tempChannels.insertMultiple (0, nullptr, nInputAndOutputChannels);
        }
    }

    void deleteTempChannels()
    {
        deleteTempChannels (floatTempBuffers);
        deleteTempChannels (doubleTempBuffers);
    }

    //==============================================================================
    void findMaxTotalChannels (int& maxTotalIns, int& maxTotalOuts)
    {
        setMaxChannelsOnAllBuses (true);
        setMaxChannelsOnAllBuses (false);

        maxTotalIns = busUtils.findTotalNumChannels (true);
        maxTotalOuts = busUtils.findTotalNumChannels (false);
    }

    void setMaxChannelsOnAllBuses (bool isInput)
    {
        const int n = busUtils.getBusCount (isInput);

        for (int i = 0; i < n; ++i)
        {
            int ch = busUtils.findMaxNumberOfChannelsForBus (isInput, i);

            if (ch == -1)
            {
                // VST-2 requires a maximum number of channels. If you are hitting this assertion
                // then make sure that your setPreferredBusArrangement only accepts layouts
                // up to a maximum number of channels
                jassertfalse;

                // do something sensible if the above assertions was hit
                ch = busUtils.getDefaultLayoutForBus (isInput, i).size();
            }

            AudioChannelSet set =
                busUtils.getDefaultLayoutForChannelNumAndBus (isInput, i, ch);

            bool success = filter->setPreferredBusArrangement (isInput, i, set);
            ignoreUnused (success);

            // If you are hitting this assertsion then please file bug!
            jassert ((! set.isDisabled()) && success);
        }
    }


    //==============================================================================
    void resetAuxChannelsToDefaultLayout (bool isInput) const
    {
        // set side-chain and aux channels to their default layout
        for (int busIdx = 1; busIdx < busUtils.getBusCount (isInput); ++busIdx)
        {
            bool success = filter->setPreferredBusArrangement (isInput, busIdx, busUtils.getDefaultLayoutForBus (isInput, busIdx));

            // VST 2 only supports a static channel layout on aux/sidechain channels
            // You must at least support the default layout regardless of the layout of the main bus.
            // If this is a problem for your plug-in, then consider using VST-3.
            jassert (success);
            ignoreUnused (success);
        }
    }

    bool setBusArrangementFromTotalChannelNum (const int numInChannels, const int numOutChannels)
    {
        PluginBusUtilities::ScopedBusRestorer busRestorer (busUtils);
        const int numIns  = busUtils.getBusCount (true);
        const int numOuts = busUtils.getBusCount (false);

        const int n = numIns + numOuts;
        HeapBlock<int> config (static_cast<size_t> (n), true);
        HeapBlock<int> maxChans (static_cast<size_t> (n), true);

        for (int i = 0; i < numIns; ++i)
            maxChans[i] = busUtils.findMaxNumberOfChannelsForBus (true, i, numInChannels);

        for (int i = 0; i < numOuts; ++i)
            maxChans[i + numIns] = busUtils.findMaxNumberOfChannelsForBus (false, i, numOutChannels);

        const int* inConfig  = config.getData();
        const int* outConfig = config.getData() + numIns;

       #if JUCE_DEBUG
        bool firstMatch = true;
        AudioProcessor::AudioBusArrangement saveFirstMatch;
       #endif

        for (int i = 0; i < n;)
        {
            if  (sumOfConfig (inConfig,  numIns)  == numInChannels
              && sumOfConfig (outConfig, numOuts) == numOutChannels)
            {
                if (applyConfiguration (inConfig, outConfig))
                {
                   #if JUCE_DEBUG
                    if (! firstMatch)
                    {
                        // Unfortunately, VST-2 requires that there is a unique plug-in bus
                        // arrangement for every x,y pair where x,y is the total number of
                        // input, output channels respectively.
                        jassertfalse;

                        busUtils.restoreBusArrangement (saveFirstMatch);
                        return true;
                    }

                    saveFirstMatch = filter->busArrangement;
                    firstMatch = false;
                   #else
                    busRestorer.release();
                    return true;
                   #endif
                }
            }

            for (i = 0; i < n; ++i)
                if ((config[i] = (config[i] + 1) % maxChans[i]) > 0)
                    break;
        }

       #if JUCE_DEBUG
        if (! firstMatch)
        {
            busRestorer.release();
            busUtils.restoreBusArrangement (saveFirstMatch);
            return true;
        }
       #endif

        return false;
    }

    bool setBusConfiguration (bool isInput, const int config[], const int n)
    {
        Array<AudioProcessor::AudioProcessorBus>& busArray = isInput ? filter->busArrangement.inputBuses
                                                                     : filter->busArrangement.outputBuses;

        int idx;
        for (idx = 0; idx < n; ++idx)
        {
            if (busArray.getReference (idx).channels.size() == (config [idx] + 1))
                continue;

            AudioChannelSet set;
            if ((set = busUtils.getDefaultLayoutForChannelNumAndBus (isInput, idx, config [idx] + 1)).isDisabled())
                break;

            if (! filter->setPreferredBusArrangement (isInput, idx, set))
                break;
        }

        return (idx >= n);
    }

    bool configurationMatches (bool isInput, const int config[], const int n)
    {
        Array<AudioProcessor::AudioProcessorBus>& busArray = isInput ? filter->busArrangement.inputBuses
                                                                     : filter->busArrangement.outputBuses;

        int idx;
        for (idx = 0; idx < n; ++idx)
            if (busArray.getReference (idx).channels.size() != (config [idx] + 1))
                break;

        return (idx >= n);
    }

    bool applyConfiguration (const int inConfig[], const int outConfig[])
    {
        const int numIns  = busUtils.getBusCount (true);
        const int numOuts = busUtils.getBusCount (false);

        if (! setBusConfiguration (true,  inConfig,  numIns))
            return false;

        if (! setBusConfiguration (false, outConfig, numOuts))
            return false;

        // re-check configuration
        if (configurationMatches (true, inConfig, numIns) && configurationMatches (false, outConfig, numOuts))
            return true;

        return false;
    }

    void allocateSpeakerArrangement (VstSpeakerConfiguration** arrangement, int32 nChannels)
    {
        if (*arrangement)
            delete [] (char*) *arrangement;

        // The last member of a full VstSpeakerConfiguration struct is an array of 8
        // VstIndividualSpeakerInfo. Here we only allocate space for channels we will
        // actually use.
        const size_t allocationSizeToSubtract = static_cast<size_t> (8 - nChannels) * sizeof (VstIndividualSpeakerInfo);
        const size_t allocationSize = sizeof (VstSpeakerConfiguration) - allocationSizeToSubtract;
        char* newAllocation = new char[allocationSize];
        memset (newAllocation, 0, allocationSize);

        *arrangement = (VstSpeakerConfiguration*) newAllocation;
        (*arrangement)->numberOfChannels = nChannels;
    }

    //==============================================================================
    bool pluginHasSidechainsOrAuxs() const
    {
        return (busUtils.getBusCount (true) > 1 || busUtils.getBusCount (false) > 1);
    }

    static int sumOfConfig (const int config[], int num) noexcept
    {
        int retval = 0;
        for (int i = 0; i < num; ++i)
            retval += (config [i] + 1);

        return retval;
    }

    //==============================================================================
    /** Host to plug-in calls. */

    pointer_sized_int handleOpen (VstOpCodeArguments)
    {
        // Note: most hosts call this on the UI thread, but wavelab doesn't, so be careful in here.
        if (filter->hasEditor())
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
        if (filter != nullptr && isPositiveAndBelow((int) args.value, filter->getNumPrograms()))
            filter->setCurrentProgram ((int) args.value);

        return 0;
    }

    pointer_sized_int handleGetCurrentProgram (VstOpCodeArguments)
    {
        return (filter != nullptr && filter->getNumPrograms() > 0 ? filter->getCurrentProgram() : 0);
    }

    pointer_sized_int handleSetCurrentProgramName (VstOpCodeArguments args)
    {
        if (filter != nullptr && filter->getNumPrograms() > 0)
            filter->changeProgramName (filter->getCurrentProgram(), (char*) args.ptr);

        return 0;
    }

    pointer_sized_int handleGetCurrentProgramName (VstOpCodeArguments args)
    {
        if (filter != nullptr && filter->getNumPrograms() > 0)
            filter->getProgramName (filter->getCurrentProgram()).copyToUTF8 ((char*) args.ptr, 24 + 1);

        return 0;
    }

    pointer_sized_int handleGetParameterLabel (VstOpCodeArguments args)
    {
        if (filter != nullptr)
        {
            jassert (isPositiveAndBelow (args.index, filter->getNumParameters()));
            // length should technically be kVstMaxParamStrLen, which is 8, but hosts will normally allow a bit more.
            filter->getParameterLabel (args.index).copyToUTF8 ((char*) args.ptr, 24 + 1);
        }

        return 0;
    }

    pointer_sized_int handleGetParameterText (VstOpCodeArguments args)
    {
        if (filter != nullptr)
        {
            jassert (isPositiveAndBelow (args.index, filter->getNumParameters()));
            // length should technically be kVstMaxParamStrLen, which is 8, but hosts will normally allow a bit more.
            filter->getParameterText (args.index, 24).copyToUTF8 ((char*) args.ptr, 24 + 1);
        }

        return 0;
    }

    pointer_sized_int handleGetParameterName (VstOpCodeArguments args)
    {
        if (filter != nullptr)
        {
            jassert (isPositiveAndBelow (args.index, filter->getNumParameters()));
            // length should technically be kVstMaxParamStrLen, which is 8, but hosts will normally allow a bit more.
            filter->getParameterName (args.index, 16).copyToUTF8 ((char*) args.ptr, 16 + 1);
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
            editorBounds.upper     = 0;
            editorBounds.leftmost  = 0;
            editorBounds.lower     = (int16) editorComp->getHeight();
            editorBounds.rightmost = (int16) editorComp->getWidth();

            *((VstEditorBounds**) args.ptr) = &editorBounds;

            return (pointer_sized_int) (pointer_sized_int) &editorBounds;
        }

        return 0;
    }

    pointer_sized_int handleOpenEditor (VstOpCodeArguments args)
    {
        checkWhetherMessageThreadIsCorrect();
        const MessageManagerLock mmLock;
        jassert (! recursionCheck);

        startTimer (1000 / 4); // performs misc housekeeping chores

        deleteEditor (true);
        createEditorComp();

        if (editorComp != nullptr)
        {
            editorComp->setOpaque (true);
            editorComp->setVisible (false);

           #if JUCE_WINDOWS
            editorComp->addToDesktop (0, args.ptr);
            hostWindow = (HWND) args.ptr;
           #elif JUCE_LINUX
            editorComp->addToDesktop (0, args.ptr);
            hostWindow = (Window) args.ptr;
            Window editorWnd = (Window) editorComp->getWindowHandle();
            XReparentWindow (display, editorWnd, hostWindow, 0, 0);
           #else
            hostWindow = attachComponentToWindowRefVST (editorComp, args.ptr, useNSView);
           #endif
            editorComp->setVisible (true);

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
        void** data = (void**) args.ptr;
        bool onlyStoreCurrentProgramData = (args.index != 0);

        if (filter == nullptr)
            return 0;

        chunkMemory.reset();
        if (onlyStoreCurrentProgramData)
            filter->getCurrentProgramStateInformation (chunkMemory);
        else
            filter->getStateInformation (chunkMemory);

        *data = (void*) chunkMemory.getData();

        // because the chunk is only needed temporarily by the host (or at least you'd
        // hope so) we'll give it a while and then free it in the timer callback.
        chunkMemoryTime = juce::Time::getApproximateMillisecondCounter();

        return (int32) chunkMemory.getSize();
    }

    pointer_sized_int handleSetData (VstOpCodeArguments args)
    {
        void* data = args.ptr;
        int32 byteSize = (int32) args.value;
        bool onlyRestoreCurrentProgramData = (args.index != 0);

        if (filter != nullptr)
        {
            chunkMemory.reset();
            chunkMemoryTime = 0;

            if (byteSize > 0 && data != nullptr)
            {
                if (onlyRestoreCurrentProgramData)
                    filter->setCurrentProgramStateInformation (data, byteSize);
                else
                    filter->setStateInformation (data, byteSize);
            }
        }

        return 0;
    }

    pointer_sized_int handlePreAudioProcessingEvents (VstOpCodeArguments args)
    {
       #if JucePlugin_WantsMidiInput
        VSTMidiEventList::addEventsToMidiBuffer ((VstEventBlock*) args.ptr, midiEvents);
        return 1;
       #else
        ignoreUnused (args);
        return 0;
       #endif
    }

    pointer_sized_int handleIsParameterAutomatable (VstOpCodeArguments args)
    {
        return (filter != nullptr && filter->isParameterAutomatable (args.index)) ? 1 : 0;
    }

    pointer_sized_int handleParameterValueForText (VstOpCodeArguments args)
    {
        if (filter != nullptr)
        {
            jassert (isPositiveAndBelow (args.index, filter->getNumParameters()));

            if (AudioProcessorParameter* p = filter->getParameters()[args.index])
            {
                filter->setParameter (args.index, p->getValueForText (String::fromUTF8 ((char*) args.ptr)));
                return 1;
            }
        }

        return 0;
    }

    pointer_sized_int handleGetProgramName (VstOpCodeArguments args)
    {
        if (filter != nullptr && isPositiveAndBelow (args.index, filter->getNumPrograms()))
        {
            filter->getProgramName (args.index).copyToUTF8 ((char*) args.ptr, 24 + 1);
            return 1;
        }

        return 0;
    }

    pointer_sized_int handleGetInputPinProperties (VstOpCodeArguments args)
    {
        return (filter != nullptr && getPinProperties (*(VstPinInfo*) args.ptr, true, args.index)) ? 1 : 0;
    }

    pointer_sized_int handleGetOutputPinProperties (VstOpCodeArguments args)
    {
        return (filter != nullptr && getPinProperties (*(VstPinInfo*) args.ptr, false, args.index)) ? 1 : 0;
    }

    pointer_sized_int handleGetPlugInCategory (VstOpCodeArguments)
    {
        return JucePlugin_VSTCategory;
    }

    pointer_sized_int handleSetSpeakerConfiguration (VstOpCodeArguments args)
    {
        VstSpeakerConfiguration* pluginInput = reinterpret_cast<VstSpeakerConfiguration*> (args.value);
        VstSpeakerConfiguration* pluginOutput = (VstSpeakerConfiguration*) args.ptr;

        const int numIns  = busUtils.getBusCount (true);
        const int numOuts = busUtils.getBusCount (false);;

        if (pluginInput != nullptr && numIns == 0)
            return 0;

        if (pluginOutput != nullptr && numOuts == 0)
            return 0;

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

        if (numIns > 1 || numOuts > 1)
        {
            int newNumInChannels  = (pluginInput  != nullptr && pluginInput-> numberOfChannels >= 0)
                ? pluginInput-> numberOfChannels
                : busUtils.findTotalNumChannels (true);
            int newNumOutChannels = (pluginOutput != nullptr && pluginOutput->numberOfChannels >= 0)
                ? pluginOutput->numberOfChannels
                : busUtils.findTotalNumChannels (false);

            newNumInChannels  = jmin (newNumInChannels,  maxNumInChannels);
            newNumOutChannels = jmin (newNumOutChannels, maxNumOutChannels);

            if (! setBusArrangementFromTotalChannelNum (newNumInChannels, newNumOutChannels))
                return 0;
        }
        else
        {
            PluginBusUtilities::ScopedBusRestorer busRestorer (busUtils);
            AudioChannelSet inLayoutType;

            if (pluginInput  != nullptr && pluginInput-> numberOfChannels >= 0)
            {
                inLayoutType = SpeakerMappings::vstArrangementTypeToChannelSet (*pluginInput);
                if (busUtils.getChannelSet (true, 0) != inLayoutType)
                    if (! filter->setPreferredBusArrangement (true, 0, inLayoutType))
                        return 0;
            }

            if (pluginOutput != nullptr && pluginOutput->numberOfChannels >= 0)
            {
                AudioChannelSet newType = SpeakerMappings::vstArrangementTypeToChannelSet (*pluginOutput);

                if (busUtils.getChannelSet (false, 0) != newType)
                    if (! filter->setPreferredBusArrangement (false, 0, newType))
                        return 0;

                // re-check the input
                if ((! inLayoutType.isDisabled()) && busUtils.getChannelSet (true, 0) != inLayoutType)
                    return 0;

                busRestorer.release();
            }
        }

        filter->setRateAndBufferSizeDetails(0, 0);
        return 1;
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
       #if JucePlugin_Build_VST3 && JUCE_VST3_CAN_REPLACE_VST2
        if ((args.index == 'stCA' || args.index == 'stCa') && args.value == 'FUID' && args.ptr != nullptr)
        {
            memcpy (args.ptr, getJuceVST3ComponentIID(), 16);
            return 1;
        }
       #else
        ignoreUnused (args);
       #endif
        return 0;
    }

    pointer_sized_int handleCanPlugInDo (VstOpCodeArguments args)
    {
        char* text = (char*) args.ptr;
        if (strcmp (text, "receiveVstEvents") == 0
             || strcmp (text, "receiveVstMidiEvent") == 0
             || strcmp (text, "receiveVstMidiEvents") == 0)
        {
           #if JucePlugin_WantsMidiInput
            return 1;
           #else
            return -1;
           #endif
        }

        if (strcmp (text, "sendVstEvents") == 0
             || strcmp (text, "sendVstMidiEvent") == 0
             || strcmp (text, "sendVstMidiEvents") == 0)
        {
           #if JucePlugin_ProducesMidiOutput
            return 1;
           #else
            return -1;
           #endif
        }

        if (strcmp (text, "receiveVstTimeInfo") == 0
             || strcmp (text, "conformsToWindowRules") == 0
             || strcmp (text, "bypass") == 0)
        {
            return 1;
        }

        // This tells Wavelab to use the UI thread to invoke open/close,
        // like all other hosts do.
        if (strcmp (text, "openCloseAnyThread") == 0)
            return -1;

        if (strcmp (text, "MPE") == 0)
            return filter->supportsMPE() ? 1 : 0;

       #if JUCE_MAC
        if (strcmp (text, "hasCockosViewAsConfig") == 0)
        {
            useNSView = true;
            return (int32) 0xbeef0000;
        }
       #endif

        return 0;
    }

    pointer_sized_int handleGetTailSize (VstOpCodeArguments)
    {
        if (filter != nullptr)
            return (pointer_sized_int) (filter->getTailLengthSeconds() * sampleRate);

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
        VstSpeakerConfiguration** pluginInput = reinterpret_cast<VstSpeakerConfiguration**> (args.value);
        VstSpeakerConfiguration** pluginOutput = (VstSpeakerConfiguration**) args.ptr;
        *pluginInput  = nullptr;
        *pluginOutput = nullptr;

        allocateSpeakerArrangement (pluginInput, busUtils.findTotalNumChannels (true));
        allocateSpeakerArrangement (pluginOutput, busUtils.findTotalNumChannels (false));

        if (pluginHasSidechainsOrAuxs())
        {
            int numIns  = busUtils.findTotalNumChannels (true);
            int numOuts = busUtils.findTotalNumChannels (false);

            AudioChannelSet layout = AudioChannelSet::canonicalChannelSet (numIns);
            SpeakerMappings::channelSetToVstArrangement (layout,  **pluginInput);

            layout = AudioChannelSet::canonicalChannelSet (numOuts);
            SpeakerMappings::channelSetToVstArrangement (layout,  **pluginOutput);
        }
        else
        {
            SpeakerMappings::channelSetToVstArrangement (busUtils.getChannelSet (true,  0), **pluginInput);
            SpeakerMappings::channelSetToVstArrangement (busUtils.getChannelSet (false, 0), **pluginOutput);
        }

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
            if (filter != nullptr)
            {
                filter->setProcessingPrecision (args.value == vstProcessingSampleTypeDouble
                                                && filter->supportsDoublePrecisionProcessing()
                                                    ? AudioProcessor::doublePrecision
                                                    : AudioProcessor::singlePrecision);

                return 1;
            }
        }

        return 0;
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

                    AudioProcessor* const filter = createPluginFilterOfType (AudioProcessor::wrapperType_VST);
                    JuceVSTWrapper* const wrapper = new JuceVSTWrapper (audioMaster, filter);
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
#endif

#endif

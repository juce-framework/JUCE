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

/*  These files come with the Steinberg VST SDK - to get them, you'll need to
    visit the Steinberg website and agree to whatever is currently required to
    get them. The best version to get is the VST3 SDK, which also contains
    the older VST2.4 files.

    Then, you'll need to make sure your include path contains your "VST SDK3"
    directory (or whatever you've named it on your machine). The introjucer has
    a special box for setting this path.
*/
#include <public.sdk/source/vst2.x/audioeffectx.h>
#include <public.sdk/source/vst2.x/aeffeditor.h>
#include <public.sdk/source/vst2.x/audioeffectx.cpp>
#include <public.sdk/source/vst2.x/audioeffect.cpp>

#if ! VST_2_4_EXTENSIONS
 #error "It looks like you're trying to include an out-of-date VSTSDK version - make sure you have at least version 2.4"
#endif

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
static juce::uint32 lastMasterIdleCall = 0;

namespace juce
{
 #if JUCE_MAC
  extern void initialiseMacVST();
  extern void* attachComponentToWindowRefVST (Component*, void* parent, bool isNSView);
  extern void detachComponentFromWindowRefVST (Component*, void* window, bool isNSView);
  extern void setNativeHostWindowSizeVST (void* window, Component*, int newWidth, int newHeight, bool isNSView);
  extern void checkWindowVisibilityVST (void* window, Component*, bool isNSView);
  extern bool forwardCurrentKeyEventToHostVST (Component*, bool isNSView);
 #if ! JUCE_64BIT
  extern void updateEditorCompBoundsVST (Component*);
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
class JuceVSTWrapper  : public AudioEffectX,
                        public AudioProcessorListener,
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

public:
    //==============================================================================
    JuceVSTWrapper (audioMasterCallback audioMasterCB, AudioProcessor* const af)
       : AudioEffectX (audioMasterCB, af->getNumPrograms(), af->getNumParameters()),
         filter (af),
         busUtils (*filter, false),
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
        busUtils.findAllCompatibleLayouts();

        // VST-2 does not support disabling buses: so always enable all of them
        if (busUtils.hasDynamicInBuses() || busUtils.hasDynamicOutBuses())
            busUtils.enableAllBuses();

        int maxNumInChannels  = busUtils.getBusCount (true)  > 0 ? busUtils.getSupportedBusLayouts (true,  0).maxNumberOfChannels() : 0;
        int maxNumOutChannels = busUtils.getBusCount (false) > 0 ? busUtils.getSupportedBusLayouts (false, 0).maxNumberOfChannels() : 0;

        // try setting the number of channels
        if (maxNumInChannels > 0)
            filter->setPreferredBusArrangement (true,  0, busUtils.getDefaultLayoutForChannelNumAndBus (true,  0, maxNumInChannels));

        if (maxNumOutChannels > 0)
            filter->setPreferredBusArrangement (false, 0, busUtils.getDefaultLayoutForChannelNumAndBus (false, 0, maxNumOutChannels));

        resetAuxChannelsToDefaultLayout (true);
        resetAuxChannelsToDefaultLayout (false);

        const int totalNumInChannels  = busUtils.findTotalNumChannels (true);
        const int totalNumOutChannels = busUtils.findTotalNumChannels (false);

        filter->setRateAndBufferSizeDetails (0, 0);
        filter->setPlayHead (this);
        filter->addListener (this);

        cEffect.flags |= effFlagsHasEditor;
        cEffect.version = convertHexVersionToDecimal (JucePlugin_VersionCode);

        setUniqueID ((int) (JucePlugin_VSTUniqueID));

        setNumInputs  (totalNumInChannels);
        setNumOutputs (totalNumOutChannels);

        canProcessReplacing (true);
        canDoubleReplacing (filter->supportsDoublePrecisionProcessing());

        isSynth ((JucePlugin_IsSynth) != 0);
        setInitialDelay (filter->getLatencySamples());
        programsAreChunks (true);

        // NB: For reasons best known to themselves, some hosts fail to load/save plugin
        // state correctly if the plugin doesn't report that it has at least 1 program.
        jassert (af->getNumPrograms() > 0);

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

    void open() override
    {
        // Note: most hosts call this on the UI thread, but wavelab doesn't, so be careful in here.
        if (filter->hasEditor())
            cEffect.flags |= effFlagsHasEditor;
        else
            cEffect.flags &= ~effFlagsHasEditor;
    }

    void close() override
    {
        // Note: most hosts call this on the UI thread, but wavelab doesn't, so be careful in here.
        stopTimer();

        if (MessageManager::getInstance()->isThisTheMessageThread())
            deleteEditor (false);
    }

    //==============================================================================
    bool getEffectName (char* name) override
    {
        String (JucePlugin_Name).copyToUTF8 (name, 64);
        return true;
    }

    bool getVendorString (char* text) override
    {
        String (JucePlugin_Manufacturer).copyToUTF8 (text, 64);
        return true;
    }

    bool getProductString (char* text) override  { return getEffectName (text); }
    VstInt32 getVendorVersion() override         { return convertHexVersionToDecimal (JucePlugin_VersionCode); }
    VstPlugCategory getPlugCategory() override   { return JucePlugin_VSTCategory; }
    bool keysRequired()                          { return (JucePlugin_EditorRequiresKeyboardFocus) != 0; }

    VstInt32 canDo (char* text) override
    {
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
            return (VstInt32) 0xbeef0000;
        }
       #endif

        return 0;
    }

    VstIntPtr vendorSpecific (VstInt32 lArg, VstIntPtr lArg2, void* ptrArg, float floatArg) override
    {
        ignoreUnused (lArg, lArg2, ptrArg, floatArg);

       #if JucePlugin_Build_VST3 && JUCE_VST3_CAN_REPLACE_VST2
        if ((lArg == 'stCA' || lArg == 'stCa') && lArg2 == 'FUID' && ptrArg != nullptr)
        {
            memcpy (ptrArg, getJuceVST3ComponentIID(), 16);
            return 1;
        }
       #endif

        return 0;
    }

    bool setBypass (bool b) override
    {
        isBypassed = b;
        return true;
    }

    VstInt32 getGetTailSize() override
    {
        if (filter != nullptr)
            return (VstInt32) (filter->getTailLengthSeconds() * getSampleRate());

        return 0;
    }

    //==============================================================================
    VstInt32 processEvents (VstEvents* events) override
    {
       #if JucePlugin_WantsMidiInput
        VSTMidiEventList::addEventsToMidiBuffer (events, midiEvents);
        return 1;
       #else
        ignoreUnused (events);
        return 0;
       #endif
    }

    template <typename FloatType>
    void internalProcessReplacing (FloatType** inputs, FloatType** outputs,
                                   VstInt32 numSamples, VstTempBuffers<FloatType>& tmpBuffers)
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

            filter->setNonRealtime (getCurrentProcessLevel() == 4 /* kVstProcessLevelOffline */);
        }

       #if JUCE_DEBUG && ! JucePlugin_ProducesMidiOutput
        const int numMidiEventsComingIn = midiEvents.getNumEvents();
       #endif

        jassert (activePlugins.contains (this));

        {
            const int numIn  = cEffect.numInputs;
            const int numOut = cEffect.numOutputs;

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

                    if (i < numIn && chan != inputs[i])
                        memcpy (chan, inputs[i], sizeof (FloatType) * (size_t) numSamples);

                    tmpBuffers.channels[i] = chan;
                }

                for (; i < numIn; ++i)
                    tmpBuffers.channels[i] = inputs[i];

                {
                    AudioBuffer<FloatType> chans (tmpBuffers.channels, jmax (numIn, numOut), numSamples);

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

            const juce::uint8* midiEventData;
            int midiEventSize, midiEventPosition;
            MidiBuffer::Iterator i (midiEvents);

            while (i.getNextEvent (midiEventData, midiEventSize, midiEventPosition))
            {
                jassert (midiEventPosition >= 0 && midiEventPosition < numSamples);

                outgoingEvents.addEvent (midiEventData, midiEventSize, midiEventPosition);
            }

            sendVstEventsToHost (outgoingEvents.events);
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

    void processReplacing (float** inputs, float** outputs, VstInt32 sampleFrames) override
    {
        jassert (! filter->isUsingDoublePrecision());
        internalProcessReplacing (inputs, outputs, sampleFrames, floatTempBuffers);
    }

    void processDoubleReplacing (double** inputs, double** outputs, VstInt32 sampleFrames) override
    {
        jassert (filter->isUsingDoublePrecision());
        internalProcessReplacing (inputs, outputs, sampleFrames, doubleTempBuffers);
    }

    //==============================================================================
    VstInt32 startProcess() override  { return 0; }
    VstInt32 stopProcess() override   { return 0; }

    //==============================================================================
    bool setProcessPrecision (VstInt32 vstPrecision) override
    {
        if (! isProcessing)
        {
            if (filter != nullptr)
            {
                filter->setProcessingPrecision (vstPrecision == kVstProcessPrecision64 && filter->supportsDoublePrecisionProcessing()
                                                    ? AudioProcessor::doublePrecision
                                                    : AudioProcessor::singlePrecision);

                return true;
            }
        }

        return false;
    }

    void resume() override
    {
        if (filter != nullptr)
        {
            isProcessing = true;

            const int numInChans  = filter->busArrangement.getTotalNumInputChannels();
            const int numOutChans = filter->busArrangement.getTotalNumOutputChannels();

            setNumInputs (numInChans);
            setNumOutputs (numOutChans);

            floatTempBuffers.channels.calloc ((size_t) (numInChans + numOutChans));
            doubleTempBuffers.channels.calloc ((size_t) (numInChans + numOutChans));

            double rate = getSampleRate();
            jassert (rate > 0);
            if (rate <= 0.0)
                rate = 44100.0;

            const int currentBlockSize = getBlockSize();
            jassert (currentBlockSize > 0);

            firstProcessCallback = true;

            filter->setNonRealtime (getCurrentProcessLevel() == 4 /* kVstProcessLevelOffline */);
            filter->setRateAndBufferSizeDetails (rate, currentBlockSize);

            deleteTempChannels();

            filter->prepareToPlay (rate, currentBlockSize);

            midiEvents.ensureSize (2048);
            midiEvents.clear();

            setInitialDelay (filter->getLatencySamples());

            AudioEffectX::resume();

           #if JucePlugin_ProducesMidiOutput
            outgoingEvents.ensureSize (512);
           #endif
        }
    }

    void suspend() override
    {
        if (filter != nullptr)
        {
            AudioEffectX::suspend();

            filter->releaseResources();
            outgoingEvents.freeEvents();

            isProcessing = false;
            floatTempBuffers.channels.free();
            doubleTempBuffers.channels.free();

            deleteTempChannels();
        }
    }

    bool getCurrentPosition (AudioPlayHead::CurrentPositionInfo& info) override
    {
        const VstTimeInfo* const ti = getTimeInfo (kVstPpqPosValid | kVstTempoValid | kVstBarsValid | kVstCyclePosValid
                                                    | kVstTimeSigValid | kVstSmpteValid | kVstClockValid);

        if (ti == nullptr || ti->sampleRate <= 0)
            return false;

        info.bpm = (ti->flags & kVstTempoValid) != 0 ? ti->tempo : 0.0;

        if ((ti->flags & kVstTimeSigValid) != 0)
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
        info.ppqPosition = (ti->flags & kVstPpqPosValid) != 0 ? ti->ppqPos : 0.0;
        info.ppqPositionOfLastBarStart = (ti->flags & kVstBarsValid) != 0 ? ti->barStartPos : 0.0;

        if ((ti->flags & kVstSmpteValid) != 0)
        {
            AudioPlayHead::FrameRateType rate = AudioPlayHead::fpsUnknown;
            double fps = 1.0;

            switch (ti->smpteFrameRate)
            {
                case kVstSmpte24fps:        rate = AudioPlayHead::fps24;       fps = 24.0;  break;
                case kVstSmpte25fps:        rate = AudioPlayHead::fps25;       fps = 25.0;  break;
                case kVstSmpte2997fps:      rate = AudioPlayHead::fps2997;     fps = 29.97; break;
                case kVstSmpte30fps:        rate = AudioPlayHead::fps30;       fps = 30.0;  break;
                case kVstSmpte2997dfps:     rate = AudioPlayHead::fps2997drop; fps = 29.97; break;
                case kVstSmpte30dfps:       rate = AudioPlayHead::fps30drop;   fps = 30.0;  break;

                case kVstSmpteFilm16mm:
                case kVstSmpteFilm35mm:     fps = 24.0; break;

                case kVstSmpte239fps:       fps = 23.976; break;
                case kVstSmpte249fps:       fps = 24.976; break;
                case kVstSmpte599fps:       fps = 59.94; break;
                case kVstSmpte60fps:        fps = 60; break;

                default:                    jassertfalse; // unknown frame-rate..
            }

            info.frameRate = rate;
            info.editOriginTime = ti->smpteOffset / (80.0 * fps);
        }
        else
        {
            info.frameRate = AudioPlayHead::fpsUnknown;
            info.editOriginTime = 0;
        }

        info.isRecording = (ti->flags & kVstTransportRecording) != 0;
        info.isPlaying   = (ti->flags & (kVstTransportRecording | kVstTransportPlaying)) != 0;
        info.isLooping   = (ti->flags & kVstTransportCycleActive) != 0;

        if ((ti->flags & kVstCyclePosValid) != 0)
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
    VstInt32 getProgram() override
    {
        return filter != nullptr ? filter->getCurrentProgram() : 0;
    }

    void setProgram (VstInt32 program) override
    {
        if (filter != nullptr)
            filter->setCurrentProgram (program);
    }

    void setProgramName (char* name) override
    {
        if (filter != nullptr)
            filter->changeProgramName (filter->getCurrentProgram(), name);
    }

    void getProgramName (char* name) override
    {
        if (filter != nullptr)
            filter->getProgramName (filter->getCurrentProgram()).copyToUTF8 (name, 24);
    }

    bool getProgramNameIndexed (VstInt32 /*category*/, VstInt32 index, char* text) override
    {
        if (filter != nullptr && isPositiveAndBelow (index, filter->getNumPrograms()))
        {
            filter->getProgramName (index).copyToUTF8 (text, 24);
            return true;
        }

        return false;
    }

    //==============================================================================
    float getParameter (VstInt32 index) override
    {
        if (filter == nullptr)
            return 0.0f;

        jassert (isPositiveAndBelow (index, filter->getNumParameters()));
        return filter->getParameter (index);
    }

    void setParameter (VstInt32 index, float value) override
    {
        if (filter != nullptr)
        {
            jassert (isPositiveAndBelow (index, filter->getNumParameters()));
            filter->setParameter (index, value);
        }
    }

    void getParameterDisplay (VstInt32 index, char* text) override
    {
        if (filter != nullptr)
        {
            jassert (isPositiveAndBelow (index, filter->getNumParameters()));
            filter->getParameterText (index, 24).copyToUTF8 (text, 24); // length should technically be kVstMaxParamStrLen, which is 8, but hosts will normally allow a bit more.
        }
    }

    bool string2parameter (VstInt32 index, char* text) override
    {
        if (filter != nullptr)
        {
            jassert (isPositiveAndBelow (index, filter->getNumParameters()));

            if (AudioProcessorParameter* p = filter->getParameters()[index])
            {
                filter->setParameter (index, p->getValueForText (String::fromUTF8 (text)));
                return true;
            }
        }

        return false;
    }

    void getParameterName (VstInt32 index, char* text) override
    {
        if (filter != nullptr)
        {
            jassert (isPositiveAndBelow (index, filter->getNumParameters()));
            filter->getParameterName (index, 16).copyToUTF8 (text, 16); // length should technically be kVstMaxParamStrLen, which is 8, but hosts will normally allow a bit more.
        }
    }

    void getParameterLabel (VstInt32 index, char* text) override
    {
        if (filter != nullptr)
        {
            jassert (isPositiveAndBelow (index, filter->getNumParameters()));
            filter->getParameterLabel (index).copyToUTF8 (text, 24); // length should technically be kVstMaxParamStrLen, which is 8, but hosts will normally allow a bit more.
        }
    }

    void audioProcessorParameterChanged (AudioProcessor*, int index, float newValue) override
    {
        if (audioMaster != nullptr)
            audioMaster (&cEffect, audioMasterAutomate, index, 0, 0, newValue);
    }

    void audioProcessorParameterChangeGestureBegin (AudioProcessor*, int index) override   { beginEdit (index); }
    void audioProcessorParameterChangeGestureEnd   (AudioProcessor*, int index) override   { endEdit   (index); }

    void audioProcessorChanged (AudioProcessor*) override
    {
        setInitialDelay (filter->getLatencySamples());
        updateDisplay();
        triggerAsyncUpdate();
    }

    void handleAsyncUpdate() override
    {
        ioChanged();
    }

    bool canParameterBeAutomated (VstInt32 index) override
    {
        return filter != nullptr && filter->isParameterAutomatable ((int) index);
    }

    bool setSpeakerArrangement (VstSpeakerArrangement* pluginInput,
                                VstSpeakerArrangement* pluginOutput) override
    {
        if (pluginInput != nullptr && filter->busArrangement.inputBuses.size() == 0)
            return false;

        if (pluginOutput != nullptr && filter->busArrangement.outputBuses.size() == 0)
            return false;

        PluginBusUtilities::ScopedBusRestorer busRestorer (busUtils);

        resetAuxChannelsToDefaultLayout (true);
        resetAuxChannelsToDefaultLayout (false);

        if (pluginInput != nullptr && pluginInput->numChannels >= 0)
        {
            AudioChannelSet newType;

           // subtract the number of channels which are used by the aux channels
            int mainNumChannels = pluginInput->numChannels - busUtils.findTotalNumChannels (true, 1);

            if (mainNumChannels < 0)
                return false;

            if (mainNumChannels == pluginInput->numChannels)
                newType = SpeakerMappings::vstArrangementTypeToChannelSet (*pluginInput);
            else
                newType = AudioChannelSet::canonicalChannelSet(mainNumChannels);

            if (busUtils.getChannelSet (true, 0) != newType)
                if (! filter->setPreferredBusArrangement (true, 0, newType))
                    return false;
        }

        if (pluginOutput != nullptr && pluginOutput->numChannels >= 0)
        {
            AudioChannelSet newType;

            // subtract the number of channels which are used by the aux channels
            int mainNumChannels = pluginOutput->numChannels - busUtils.findTotalNumChannels (false, 1);

            if (mainNumChannels < 0)
                return false;

            if (mainNumChannels == pluginOutput->numChannels)
                newType = SpeakerMappings::vstArrangementTypeToChannelSet (*pluginOutput);
            else
                newType = AudioChannelSet::canonicalChannelSet(mainNumChannels);

            AudioChannelSet oldOutputLayout = busUtils.getChannelSet (false, 0);
            AudioChannelSet oldInputLayout  = busUtils.getChannelSet (true, 0);

            if (busUtils.getChannelSet (false, 0) != newType)
                if (! filter->setPreferredBusArrangement (false, 0, newType))
                    return false;

            // did this change the input layout? If yes, change it back
            if (oldInputLayout != busUtils.getChannelSet (true, 0)
             && (busUtils.getBusCount (true) > 1 || busUtils.getBusCount (false) > 1))
            {
                bool success = filter->setPreferredBusArrangement (false, 0, oldOutputLayout);

                jassert (success);
                ignoreUnused (success);
            }
        }

        busRestorer.release();

        const int totalNumInChannels = busUtils.findTotalNumChannels(true);
        const int totalNumOutChannels = busUtils.findTotalNumChannels(false);

        filter->setRateAndBufferSizeDetails(0, 0);
        setNumInputs (totalNumInChannels);
        setNumOutputs(totalNumOutChannels);

        ioChanged();

        return true;
    }

    bool getSpeakerArrangement (VstSpeakerArrangement** pluginInput, VstSpeakerArrangement** pluginOutput) override
    {
        *pluginInput = 0;
        *pluginOutput = 0;

        if (! AudioEffectX::allocateArrangement (pluginInput, busUtils.findTotalNumChannels (true)))
            return false;

        if (! AudioEffectX::allocateArrangement (pluginOutput, busUtils.findTotalNumChannels (false)))
        {
            AudioEffectX::deallocateArrangement (pluginInput);
            *pluginInput = 0;
            return false;
        }

        if (busUtils.getBusCount (true) > 1)
        {
            AudioChannelSet layout = AudioChannelSet::canonicalChannelSet (busUtils.findTotalNumChannels(true));
            SpeakerMappings::channelSetToVstArrangement (layout,  **pluginInput);
        }
        else
        {
            SpeakerMappings::channelSetToVstArrangement (busUtils.getChannelSet (true, 0),  **pluginInput);
        }

        if (busUtils.getBusCount (false) > 1)
        {
            AudioChannelSet layout = AudioChannelSet::canonicalChannelSet (busUtils.findTotalNumChannels(false));
            SpeakerMappings::channelSetToVstArrangement (layout,  **pluginOutput);
        }
        else
        {
            SpeakerMappings::channelSetToVstArrangement (busUtils.getChannelSet (false, 0), **pluginOutput);
        }

        return true;
    }

    bool getInputProperties (VstInt32 index, VstPinProperties* properties) override
    {
        return filter != nullptr
                && getPinProperties (*properties, true, (int) index);
    }

    bool getOutputProperties (VstInt32 index, VstPinProperties* properties) override
    {
        return filter != nullptr
                && getPinProperties (*properties, false, (int) index);
    }

    bool getPinProperties (VstPinProperties& properties, bool direction, int index) const
    {
        // index refers to the absolute index when combining all channels of every bus
        const int n = busUtils.getBusCount(direction);
        int busIdx;
        for (busIdx = 0; busIdx < n; ++busIdx)
        {
            const int numChans = busUtils.getNumChannels(direction, busIdx);
            if (index < numChans)
                break;

            index -= numChans;
        }

        if (busIdx >= n)
            return false;

        const AudioProcessor::AudioProcessorBus& busInfo = busUtils.getFilterBus (direction).getReference (busIdx);

        busInfo.name.copyToUTF8 (properties.label, (size_t) (kVstMaxLabelLen - 1));
        busInfo.name.copyToUTF8 (properties.shortLabel, (size_t) (kVstMaxShortLabelLen - 1));

        VstInt32 type = SpeakerMappings::channelSetToVstArrangementType (busInfo.channels);

        if (type != kSpeakerArrEmpty)
        {
            properties.flags = kVstPinUseSpeaker | kVstPinIsActive;
            properties.arrangementType = type;
        }
        else
        {
            properties.flags = 0;
            properties.arrangementType = 0;
        }

        if (busInfo.channels.size() == 2)
            properties.flags |= kVstPinIsStereo;

        return true;
    }

    //==============================================================================
    struct SpeakerMappings  : private AudioChannelSet // (inheritance only to give easier access to items in the namespace)
    {
        struct Mapping
        {
            VstInt32 vst2;
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

        static AudioChannelSet vstArrangementTypeToChannelSet (const VstSpeakerArrangement& arr)
        {
            for (const Mapping* m = getMappings(); m->vst2 != kSpeakerArrEmpty; ++m)
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

        static VstInt32 channelSetToVstArrangementType (AudioChannelSet channels)
        {
            Array<AudioChannelSet::ChannelType> chans (channels.getChannelTypes());

            for (const Mapping* m = getMappings(); m->vst2 != kSpeakerArrEmpty; ++m)
                if (m->matches (chans))
                    return m->vst2;

            return kSpeakerArrEmpty;
        }

        static void channelSetToVstArrangement (const AudioChannelSet& channels, VstSpeakerArrangement& result)
        {
            result.type = channelSetToVstArrangementType (channels);
            result.numChannels = channels.size();

            for (int i = 0; i < result.numChannels; ++i)
            {
                VstSpeakerProperties& speaker = result.speakers[i];

                zeromem (&speaker, sizeof (VstSpeakerProperties));
                speaker.type = getSpeakerType (channels.getTypeOfChannel (i));
            }
        }

        static const Mapping* getMappings() noexcept
        {
            static const Mapping mappings[] =
            {
                { kSpeakerArrMono,           { centre, unknown } },
                { kSpeakerArrStereo,         { left, right, unknown } },
                { kSpeakerArrStereoSurround, { surroundLeft, surroundRight, unknown } },
                { kSpeakerArrStereoCenter,   { centreLeft, centreRight, unknown } },
                { kSpeakerArrStereoSide,     { sideLeft, sideRight, unknown } },
                { kSpeakerArrStereoCLfe,     { centre, subbass, unknown } },
                { kSpeakerArr30Cine,         { left, right, centre, unknown } },
                { kSpeakerArr30Music,        { left, right, surround, unknown } },
                { kSpeakerArr31Cine,         { left, right, centre, subbass, unknown } },
                { kSpeakerArr31Music,        { left, right, subbass, surround, unknown } },
                { kSpeakerArr40Cine,         { left, right, centre, surround, unknown } },
                { kSpeakerArr40Music,        { left, right, surroundLeft, surroundRight, unknown } },
                { kSpeakerArr41Cine,         { left, right, centre, subbass, surround, unknown } },
                { kSpeakerArr41Music,        { left, right, subbass, surroundLeft, surroundRight, unknown } },
                { kSpeakerArr50,             { left, right, centre, surroundLeft, surroundRight, unknown } },
                { kSpeakerArr51,             { left, right, centre, subbass, surroundLeft, surroundRight, unknown } },
                { kSpeakerArr60Cine,         { left, right, centre, surroundLeft, surroundRight, surround, unknown } },
                { kSpeakerArr60Music,        { left, right, surroundLeft, surroundRight, sideLeft, sideRight, unknown } },
                { kSpeakerArr61Cine,         { left, right, centre, subbass, surroundLeft, surroundRight, surround, unknown } },
                { kSpeakerArr61Music,        { left, right, subbass, surroundLeft, surroundRight, sideLeft, sideRight, unknown } },
                { kSpeakerArr70Cine,         { left, right, centre, surroundLeft, surroundRight, topFrontLeft, topFrontRight, unknown } },
                { kSpeakerArr70Music,        { left, right, centre, surroundLeft, surroundRight, sideLeft, sideRight, unknown } },
                { kSpeakerArr71Cine,         { left, right, centre, subbass, surroundLeft, surroundRight, topFrontLeft, topFrontRight, unknown } },
                { kSpeakerArr71Music,        { left, right, centre, subbass, surroundLeft, surroundRight, sideLeft, sideRight, unknown } },
                { kSpeakerArr80Cine,         { left, right, centre, surroundLeft, surroundRight, topFrontLeft, topFrontRight, surround, unknown } },
                { kSpeakerArr80Music,        { left, right, centre, surroundLeft, surroundRight, surround, sideLeft, sideRight, unknown } },
                { kSpeakerArr81Cine,         { left, right, centre, subbass, surroundLeft, surroundRight, topFrontLeft, topFrontRight, surround, unknown } },
                { kSpeakerArr81Music,        { left, right, centre, subbass, surroundLeft, surroundRight, surround, sideLeft, sideRight, unknown } },
                { kSpeakerArr102,            { left, right, centre, subbass, surroundLeft, surroundRight, topFrontLeft, topFrontCentre, topFrontRight, topRearLeft, topRearRight, subbass2, unknown } },
                { kSpeakerArrEmpty,          { unknown } }
            };

            return mappings;
        }

        static inline VstInt32 getSpeakerType (AudioChannelSet::ChannelType type) noexcept
        {
            switch (type)
            {
                case AudioChannelSet::left:              return kSpeakerL;
                case AudioChannelSet::right:             return kSpeakerR;
                case AudioChannelSet::centre:            return kSpeakerC;
                case AudioChannelSet::subbass:           return kSpeakerLfe;
                case AudioChannelSet::surroundLeft:      return kSpeakerLs;
                case AudioChannelSet::surroundRight:     return kSpeakerRs;
                case AudioChannelSet::centreLeft:        return kSpeakerLc;
                case AudioChannelSet::centreRight:       return kSpeakerRc;
                case AudioChannelSet::surround:          return kSpeakerS;
                case AudioChannelSet::sideLeft:          return kSpeakerSl;
                case AudioChannelSet::sideRight:         return kSpeakerSr;
                case AudioChannelSet::topMiddle:         return kSpeakerTm;
                case AudioChannelSet::topFrontLeft:      return kSpeakerTfl;
                case AudioChannelSet::topFrontCentre:    return kSpeakerTfc;
                case AudioChannelSet::topFrontRight:     return kSpeakerTfr;
                case AudioChannelSet::topRearLeft:       return kSpeakerTrl;
                case AudioChannelSet::topRearCentre:     return kSpeakerTrc;
                case AudioChannelSet::topRearRight:      return kSpeakerTrr;
                case AudioChannelSet::subbass2:          return kSpeakerLfe2;
                default: break;
            }

            return 0;
        }

        static inline AudioChannelSet::ChannelType getChannelType (VstInt32 type) noexcept
        {
            switch (type)
            {
                case kSpeakerL:     return AudioChannelSet::left;
                case kSpeakerR:     return AudioChannelSet::right;
                case kSpeakerC:     return AudioChannelSet::centre;
                case kSpeakerLfe:   return AudioChannelSet::subbass;
                case kSpeakerLs:    return AudioChannelSet::surroundLeft;
                case kSpeakerRs:    return AudioChannelSet::surroundRight;
                case kSpeakerLc:    return AudioChannelSet::centreLeft;
                case kSpeakerRc:    return AudioChannelSet::centreRight;
                case kSpeakerS:     return AudioChannelSet::surround;
                case kSpeakerSl:    return AudioChannelSet::sideLeft;
                case kSpeakerSr:    return AudioChannelSet::sideRight;
                case kSpeakerTm:    return AudioChannelSet::topMiddle;
                case kSpeakerTfl:   return AudioChannelSet::topFrontLeft;
                case kSpeakerTfc:   return AudioChannelSet::topFrontCentre;
                case kSpeakerTfr:   return AudioChannelSet::topFrontRight;
                case kSpeakerTrl:   return AudioChannelSet::topRearLeft;
                case kSpeakerTrc:   return AudioChannelSet::topRearCentre;
                case kSpeakerTrr:   return AudioChannelSet::topRearRight;
                case kSpeakerLfe2:  return AudioChannelSet::subbass2;
                default: break;
            }

            return AudioChannelSet::unknown;
        }
    };

    //==============================================================================
    VstInt32 getChunk (void** data, bool onlyStoreCurrentProgramData) override
    {
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

        return (VstInt32) chunkMemory.getSize();
    }

    VstInt32 setChunk (void* data, VstInt32 byteSize, bool onlyRestoreCurrentProgramData) override
    {
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

        tryMasterIdle();
    }

    void tryMasterIdle()
    {
        if (Component::isMouseButtonDownAnywhere() && ! recursionCheck)
        {
            const juce::uint32 now = juce::Time::getMillisecondCounter();

            if (now > lastMasterIdleCall + 20 && editorComp != nullptr)
            {
                lastMasterIdleCall = now;

                recursionCheck = true;
                masterIdle();
                recursionCheck = false;
            }
        }
    }

    void doIdleCallback()
    {
        // (wavelab calls this on a separate thread and causes a deadlock)..
        if (MessageManager::getInstance()->isThisTheMessageThread()
             && ! recursionCheck)
        {
            recursionCheck = true;

            JUCE_AUTORELEASEPOOL
            {
                Timer::callPendingTimersSynchronously();

                for (int i = ComponentPeer::getNumPeers(); --i >= 0;)
                    if (ComponentPeer* p = ComponentPeer::getPeer(i))
                        p->performAnyPendingRepaintsNow();

                recursionCheck = false;
            }
        }
    }

    void createEditorComp()
    {
        if (hasShutdown || filter == nullptr)
            return;

        if (editorComp == nullptr)
        {
            if (AudioProcessorEditor* const ed = filter->createEditorIfNeeded())
            {
                cEffect.flags |= effFlagsHasEditor;
                ed->setOpaque (true);
                ed->setVisible (true);

                editorComp = new EditorCompWrapper (*this, ed);
            }
            else
            {
                cEffect.flags &= ~effFlagsHasEditor;
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
            recursionCheck = true;

            if (editorComp != nullptr)
            {
                if (Component* const modalComponent = Component::getCurrentlyModalComponent())
                {
                    modalComponent->exitModalState (0);

                    if (canDeleteLaterIfModal)
                    {
                        shouldDeleteEditor = true;
                        recursionCheck = false;
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

            recursionCheck = false;
        }
    }

    VstIntPtr dispatcher (VstInt32 opCode, VstInt32 index, VstIntPtr value, void* ptr, float opt) override
    {
        if (hasShutdown)
            return 0;

        if (opCode == effEditIdle)
        {
            doIdleCallback();
            return 0;
        }
        else if (opCode == effEditOpen)
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
                editorComp->addToDesktop (0, ptr);
                hostWindow = (HWND) ptr;
              #elif JUCE_LINUX
                editorComp->addToDesktop (0, ptr);
                hostWindow = (Window) ptr;
                Window editorWnd = (Window) editorComp->getWindowHandle();
                XReparentWindow (display, editorWnd, hostWindow, 0, 0);
              #else
                hostWindow = attachComponentToWindowRefVST (editorComp, ptr, useNSView);
              #endif
                editorComp->setVisible (true);

                return 1;
            }
        }
        else if (opCode == effEditClose)
        {
            checkWhetherMessageThreadIsCorrect();
            const MessageManagerLock mmLock;
            deleteEditor (true);
            return 0;
        }
        else if (opCode == effEditGetRect)
        {
            checkWhetherMessageThreadIsCorrect();
            const MessageManagerLock mmLock;
            createEditorComp();

            if (editorComp != nullptr)
            {
                editorSize.left = 0;
                editorSize.top = 0;
                editorSize.right = (VstInt16) editorComp->getWidth();
                editorSize.bottom = (VstInt16) editorComp->getHeight();

                *((ERect**) ptr) = &editorSize;

                return (VstIntPtr) (pointer_sized_int) &editorSize;
            }

            return 0;
        }

        return AudioEffectX::dispatcher (opCode, index, value, ptr, opt);
    }

    void resizeHostWindow (int newWidth, int newHeight)
    {
        if (editorComp != nullptr)
        {
            bool sizeWasSuccessful = false;

            if (canHostDo (const_cast<char*> ("sizeWindow")))
            {
                isInSizeWindow = true;
                sizeWasSuccessful = sizeWindow (newWidth, newHeight);
                isInSizeWindow = false;
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
    class EditorCompWrapper  : public Component,
                               public AsyncUpdater
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

        void paintOverChildren (Graphics&) override
        {
            // this causes an async call to masterIdle() to help
            // creaky old DAWs like Nuendo repaint themselves while we're
            // repainting. Otherwise they just seem to give up and sit there
            // waiting.
            triggerAsyncUpdate();
        }

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

        void handleAsyncUpdate() override
        {
            wrapper.tryMasterIdle();
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
    AudioProcessor* filter;
    PluginBusUtilities busUtils;
    juce::MemoryBlock chunkMemory;
    juce::uint32 chunkMemoryTime;
    ScopedPointer<EditorCompWrapper> editorComp;
    ERect editorSize;
    MidiBuffer midiEvents;
    VSTMidiEventList outgoingEvents;
    bool isProcessing, isBypassed, hasShutdown, isInSizeWindow, firstProcessCallback;
    bool shouldDeleteEditor, useNSView;
    VstTempBuffers<float> floatTempBuffers;
    VstTempBuffers<double> doubleTempBuffers;

   #if JUCE_MAC
    void* hostWindow;
   #elif JUCE_LINUX
    Window hostWindow;
   #else
    HWND hostWindow;
   #endif

    static inline VstInt32 convertHexVersionToDecimal (const unsigned int hexVersion)
    {
       #if JUCE_VST_RETURN_HEX_VERSION_NUMBER_DIRECTLY
        return (VstInt32) hexVersion;
       #else
        return (VstInt32) (((hexVersion >> 24) & 0xff) * 1000
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
            int numChannels = filter->getTotalNumInputChannels() + filter->getTotalNumOutputChannels();
            tmpBuffers.tempChannels.insertMultiple (0, nullptr, numChannels);
        }
    }

    void deleteTempChannels()
    {
        deleteTempChannels (floatTempBuffers);
        deleteTempChannels (doubleTempBuffers);
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

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (JuceVSTWrapper)
};

//==============================================================================
namespace
{
    AEffect* pluginEntryPoint (audioMasterCallback audioMaster)
    {
        JUCE_AUTORELEASEPOOL
        {
            initialiseJuce_GUI();

            try
            {
                if (audioMaster (0, audioMasterVersion, 0, 0, 0, 0) != 0)
                {
                   #if JUCE_LINUX
                    MessageManagerLock mmLock;
                   #endif

                    AudioProcessor* const filter = createPluginFilterOfType (AudioProcessor::wrapperType_VST);
                    JuceVSTWrapper* const wrapper = new JuceVSTWrapper (audioMaster, filter);
                    return wrapper->getAeffect();
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

    JUCE_EXPORTED_FUNCTION AEffect* VSTPluginMain (audioMasterCallback audioMaster);
    JUCE_EXPORTED_FUNCTION AEffect* VSTPluginMain (audioMasterCallback audioMaster)
    {
        initialiseMacVST();
        return pluginEntryPoint (audioMaster);
    }

    JUCE_EXPORTED_FUNCTION AEffect* main_macho (audioMasterCallback audioMaster);
    JUCE_EXPORTED_FUNCTION AEffect* main_macho (audioMasterCallback audioMaster)
    {
        initialiseMacVST();
        return pluginEntryPoint (audioMaster);
    }

//==============================================================================
// Linux startup code..
#elif JUCE_LINUX

    JUCE_EXPORTED_FUNCTION AEffect* VSTPluginMain (audioMasterCallback audioMaster);
    JUCE_EXPORTED_FUNCTION AEffect* VSTPluginMain (audioMasterCallback audioMaster)
    {
        SharedMessageThread::getInstance();
        return pluginEntryPoint (audioMaster);
    }

    JUCE_EXPORTED_FUNCTION AEffect* main_plugin (audioMasterCallback audioMaster) asm ("main");
    JUCE_EXPORTED_FUNCTION AEffect* main_plugin (audioMasterCallback audioMaster)
    {
        return VSTPluginMain (audioMaster);
    }

    // don't put initialiseJuce_GUI or shutdownJuce_GUI in these... it will crash!
    __attribute__((constructor)) void myPluginInit() {}
    __attribute__((destructor))  void myPluginFini() {}

//==============================================================================
// Win32 startup code..
#else

    extern "C" __declspec (dllexport) AEffect* VSTPluginMain (audioMasterCallback audioMaster)
    {
        return pluginEntryPoint (audioMaster);
    }

   #ifndef JUCE_64BIT // (can't compile this on win64, but it's not needed anyway with VST2.4)
    extern "C" __declspec (dllexport) int main (audioMasterCallback audioMaster)
    {
        return (int) pluginEntryPoint (audioMaster);
    }
   #endif
#endif

#endif

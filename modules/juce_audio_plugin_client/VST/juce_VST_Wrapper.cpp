/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

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

// Your project must contain an AppConfig.h file with your project-specific settings in it,
// and your header search path must make it accessible to the module's files.
#include "AppConfig.h"

#include "../utility/juce_CheckSettingMacros.h"

#if JucePlugin_Build_VST

#ifdef _MSC_VER
 #pragma warning (disable : 4996 4100)
#endif

#ifdef _WIN32
 #undef _WIN32_WINNT
 #define _WIN32_WINNT 0x500
 #undef STRICT
 #define STRICT 1
 #include <windows.h>

 #ifdef __MINGW32__
  struct MOUSEHOOKSTRUCTEX  : public MOUSEHOOKSTRUCT
  {
     DWORD mouseData;
  };
 #endif
#elif defined (LINUX)
 #include <X11/Xlib.h>
 #include <X11/Xutil.h>
 #include <X11/Xatom.h>
 #undef KeyPress
#else
 #include <Carbon/Carbon.h>
#endif

#ifdef PRAGMA_ALIGN_SUPPORTED
 #undef PRAGMA_ALIGN_SUPPORTED
 #define PRAGMA_ALIGN_SUPPORTED 1
#endif

//==============================================================================
/*  These files come with the Steinberg VST SDK - to get them, you'll need to
    visit the Steinberg website and jump through some hoops to sign up as a
    VST developer.

    Then, you'll need to make sure your include path contains your "vstsdk2.4" directory.
*/
#ifndef _MSC_VER
 #define __cdecl
#endif

#ifdef __clang__
 #pragma clang diagnostic push
 #pragma clang diagnostic ignored "-Wconversion"
 #pragma clang diagnostic ignored "-Wshadow"
 #pragma clang diagnostic ignored "-Wunused-parameter"
#endif

// VSTSDK V2.4 includes..
#include <public.sdk/source/vst2.x/audioeffectx.h>
#include <public.sdk/source/vst2.x/aeffeditor.h>
#include <public.sdk/source/vst2.x/audioeffectx.cpp>
#include <public.sdk/source/vst2.x/audioeffect.cpp>

#if ! VST_2_4_EXTENSIONS
 #error "It looks like you're trying to include an out-of-date VSTSDK version - make sure you have at least version 2.4"
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
#include "../utility/juce_PluginHostType.h"

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
  extern void initialiseMac();
  extern void* attachComponentToWindowRef (Component*, void* windowRef);
  extern void detachComponentFromWindowRef (Component*, void* nsWindow);
  extern void setNativeHostWindowSize (void* nsWindow, Component*, int newWidth, int newHeight);
  extern void checkWindowVisibility (void* nsWindow, Component*);
  extern bool forwardCurrentKeyEventToHost (Component*);
 #if ! JUCE_64BIT
  extern void updateEditorCompBounds (Component*);
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
    HWND findMDIParentOf (HWND w)
    {
        const int frameThickness = GetSystemMetrics (SM_CYFIXEDFRAME);

        while (w != 0)
        {
            HWND parent = GetParent (w);

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

    //==============================================================================
    static HHOOK mouseWheelHook = 0;
    static int mouseHookUsers = 0;

    LRESULT CALLBACK mouseWheelHookCallback (int nCode, WPARAM wParam, LPARAM lParam)
    {
        if (nCode >= 0 && wParam == WM_MOUSEWHEEL)
        {
            const MOUSEHOOKSTRUCTEX& hs = *(MOUSEHOOKSTRUCTEX*) lParam;

            if (Component* const comp = Desktop::getInstance().findComponentAt (Point<int> (hs.pt.x, hs.pt.y)))
                if (comp->getWindowHandle() != 0)
                    return PostMessage ((HWND) comp->getWindowHandle(), WM_MOUSEWHEEL,
                                        hs.mouseData & 0xffff0000, (hs.pt.x & 0xffff) | (hs.pt.y << 16));
        }

        return CallNextHookEx (mouseWheelHook, nCode, wParam, lParam);
    }

    void registerMouseWheelHook()
    {
        if (mouseHookUsers++ == 0)
            mouseWheelHook = SetWindowsHookEx (WH_MOUSE, mouseWheelHookCallback,
                                               (HINSTANCE) Process::getCurrentModuleInstanceHandle(),
                                               GetCurrentThreadId());
    }

    void unregisterMouseWheelHook()
    {
        if (--mouseHookUsers == 0 && mouseWheelHook != 0)
        {
            UnhookWindowsHookEx (mouseWheelHook);
            mouseWheelHook = 0;
        }
    }

   #if JUCE_WINDOWS
    static bool messageThreadIsDefinitelyCorrect = false;
   #endif
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

    juce_DeclareSingleton (SharedMessageThread, false);

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
                        private Timer,
                        public AudioProcessorListener,
                        public AudioPlayHead
{
public:
    //==============================================================================
    JuceVSTWrapper (audioMasterCallback audioMaster, AudioProcessor* const af)
       : AudioEffectX (audioMaster, af->getNumPrograms(), af->getNumParameters()),
         filter (af),
         chunkMemoryTime (0),
         speakerIn (kSpeakerArrEmpty),
         speakerOut (kSpeakerArrEmpty),
         numInChans (JucePlugin_MaxNumInputChannels),
         numOutChans (JucePlugin_MaxNumOutputChannels),
         isProcessing (false),
         isBypassed (false),
         hasShutdown (false),
         firstProcessCallback (true),
         shouldDeleteEditor (false),
         hostWindow (0)
    {
        filter->setPlayConfigDetails (numInChans, numOutChans, 0, 0);
        filter->setPlayHead (this);
        filter->addListener (this);

        cEffect.flags |= effFlagsHasEditor;
        cEffect.version = convertHexVersionToDecimal (JucePlugin_VersionCode);

        setUniqueID ((int) (JucePlugin_VSTUniqueID));

        setNumInputs (numInChans);
        setNumOutputs (numOutChans);

        canProcessReplacing (true);

        isSynth ((JucePlugin_IsSynth) != 0);
        noTail (filter->getTailLengthSeconds() <= 0);
        setInitialDelay (filter->getLatencySamples());
        programsAreChunks (true);

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

                channels.free();
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

    void open()
    {
        // Note: most hosts call this on the UI thread, but wavelab doesn't, so be careful in here.
        if (filter->hasEditor())
            cEffect.flags |= effFlagsHasEditor;
        else
            cEffect.flags &= ~effFlagsHasEditor;
    }

    void close()
    {
        // Note: most hosts call this on the UI thread, but wavelab doesn't, so be careful in here.
        stopTimer();

        if (MessageManager::getInstance()->isThisTheMessageThread())
            deleteEditor (false);
    }

    //==============================================================================
    bool getEffectName (char* name)
    {
        String (JucePlugin_Name).copyToUTF8 (name, 64);
        return true;
    }

    bool getVendorString (char* text)
    {
        String (JucePlugin_Manufacturer).copyToUTF8 (text, 64);
        return true;
    }

    bool getProductString (char* text)  { return getEffectName (text); }
    VstInt32 getVendorVersion()         { return convertHexVersionToDecimal (JucePlugin_VersionCode); }
    VstPlugCategory getPlugCategory()   { return JucePlugin_VSTCategory; }
    bool keysRequired()                 { return (JucePlugin_EditorRequiresKeyboardFocus) != 0; }

    VstInt32 canDo (char* text)
    {
        VstInt32 result = 0;

        if (strcmp (text, "receiveVstEvents") == 0
            || strcmp (text, "receiveVstMidiEvent") == 0
            || strcmp (text, "receiveVstMidiEvents") == 0)
        {
           #if JucePlugin_WantsMidiInput
            result = 1;
           #else
            result = -1;
           #endif
        }
        else if (strcmp (text, "sendVstEvents") == 0
                 || strcmp (text, "sendVstMidiEvent") == 0
                 || strcmp (text, "sendVstMidiEvents") == 0)
        {
           #if JucePlugin_ProducesMidiOutput
            result = 1;
           #else
            result = -1;
           #endif
        }
        else if (strcmp (text, "receiveVstTimeInfo") == 0
                 || strcmp (text, "conformsToWindowRules") == 0
                 || strcmp (text, "bypass") == 0)
        {
            result = 1;
        }
        else if (strcmp (text, "openCloseAnyThread") == 0)
        {
            // This tells Wavelab to use the UI thread to invoke open/close,
            // like all other hosts do.
            result = -1;
        }

        return result;
    }

    bool getInputProperties (VstInt32 index, VstPinProperties* properties)
    {
        if (filter == nullptr || index >= JucePlugin_MaxNumInputChannels)
            return false;

        setPinProperties (*properties, filter->getInputChannelName ((int) index),
                          speakerIn, filter->isInputChannelStereoPair ((int) index));
        return true;
    }

    bool getOutputProperties (VstInt32 index, VstPinProperties* properties)
    {
        if (filter == nullptr || index >= JucePlugin_MaxNumOutputChannels)
            return false;

        setPinProperties (*properties, filter->getOutputChannelName ((int) index),
                          speakerOut, filter->isOutputChannelStereoPair ((int) index));
        return true;
    }

    static void setPinProperties (VstPinProperties& properties, const String& name,
                                  VstSpeakerArrangementType type, const bool isPair)
    {
        name.copyToUTF8 (properties.label, (size_t) (kVstMaxLabelLen - 1));
        name.copyToUTF8 (properties.shortLabel, (size_t) (kVstMaxShortLabelLen - 1));

        if (type != kSpeakerArrEmpty)
        {
            properties.flags = kVstPinUseSpeaker;
            properties.arrangementType = type;
        }
        else
        {
            properties.flags = kVstPinIsActive;
            properties.arrangementType = 0;

            if (isPair)
                properties.flags |= kVstPinIsStereo;
        }
    }

    bool setBypass (bool b)
    {
        isBypassed = b;
        return true;
    }

    VstInt32 getGetTailSize()
    {
        if (filter != nullptr)
            return (VstInt32) (filter->getTailLengthSeconds() * getSampleRate());

        return 0;
    }

    //==============================================================================
    VstInt32 processEvents (VstEvents* events)
    {
       #if JucePlugin_WantsMidiInput
        VSTMidiEventList::addEventsToMidiBuffer (events, midiEvents);
        return 1;
       #else
        return 0;
       #endif
    }

    void process (float** inputs, float** outputs, VstInt32 numSamples)
    {
        const int numIn = numInChans;
        const int numOut = numOutChans;

        AudioSampleBuffer temp (numIn, numSamples);
        for (int i = numIn; --i >= 0;)
            memcpy (temp.getSampleData (i), outputs[i], sizeof (float) * (size_t) numSamples);

        processReplacing (inputs, outputs, numSamples);

        AudioSampleBuffer dest (outputs, numOut, numSamples);

        for (int i = jmin (numIn, numOut); --i >= 0;)
            dest.addFrom (i, 0, temp, i, 0, numSamples);
    }

    void processReplacing (float** inputs, float** outputs, VstInt32 numSamples)
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

           #if JUCE_WINDOWS
            if (GetThreadPriority (GetCurrentThread()) <= THREAD_PRIORITY_NORMAL
                  && GetThreadPriority (GetCurrentThread()) >= THREAD_PRIORITY_LOWEST)
                filter->setNonRealtime (true);
           #endif
        }

       #if JUCE_DEBUG && ! JucePlugin_ProducesMidiOutput
        const int numMidiEventsComingIn = midiEvents.getNumEvents();
       #endif

        jassert (activePlugins.contains (this));

        {
            const ScopedLock sl (filter->getCallbackLock());

            const int numIn = numInChans;
            const int numOut = numOutChans;

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
                    float* chan = tempChannels.getUnchecked(i);

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
                                chan = new float [blockSize * 2];
                                tempChannels.set (i, chan);
                                break;
                            }
                        }
                    }

                    if (i < numIn && chan != inputs[i])
                        memcpy (chan, inputs[i], sizeof (float) * (size_t) numSamples);

                    channels[i] = chan;
                }

                for (; i < numIn; ++i)
                    channels[i] = inputs[i];

                {
                    AudioSampleBuffer chans (channels, jmax (numIn, numOut), numSamples);

                    if (isBypassed)
                        filter->processBlockBypassed (chans, midiEvents);
                    else
                        filter->processBlock (chans, midiEvents);
                }

                // copy back any temp channels that may have been used..
                for (i = 0; i < numOut; ++i)
                    if (const float* const chan = tempChannels.getUnchecked(i))
                        memcpy (outputs[i], chan, sizeof (float) * (size_t) numSamples);
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

    //==============================================================================
    VstInt32 startProcess()  { return 0; }
    VstInt32 stopProcess()   { return 0; }

    void resume()
    {
        if (filter != nullptr)
        {
            isProcessing = true;
            channels.calloc ((size_t) (numInChans + numOutChans));

            double rate = getSampleRate();
            jassert (rate > 0);
            if (rate <= 0.0)
                rate = 44100.0;

            const int blockSize = getBlockSize();
            jassert (blockSize > 0);

            firstProcessCallback = true;

            filter->setNonRealtime (getCurrentProcessLevel() == 4 /* kVstProcessLevelOffline */);
            filter->setPlayConfigDetails (numInChans, numOutChans, rate, blockSize);

            deleteTempChannels();

            filter->prepareToPlay (rate, blockSize);

            midiEvents.ensureSize (2048);
            midiEvents.clear();

            setInitialDelay (filter->getLatencySamples());

            AudioEffectX::resume();

           #if JucePlugin_ProducesMidiOutput
            outgoingEvents.ensureSize (512);
           #endif
        }
    }

    void suspend()
    {
        if (filter != nullptr)
        {
            AudioEffectX::suspend();

            filter->releaseResources();
            outgoingEvents.freeEvents();

            isProcessing = false;
            channels.free();

            deleteTempChannels();
        }
    }

    bool getCurrentPosition (AudioPlayHead::CurrentPositionInfo& info)
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
    VstInt32 getProgram()
    {
        return filter != nullptr ? filter->getCurrentProgram() : 0;
    }

    void setProgram (VstInt32 program)
    {
        if (filter != nullptr)
            filter->setCurrentProgram (program);
    }

    void setProgramName (char* name)
    {
        if (filter != nullptr)
            filter->changeProgramName (filter->getCurrentProgram(), name);
    }

    void getProgramName (char* name)
    {
        if (filter != nullptr)
            filter->getProgramName (filter->getCurrentProgram()).copyToUTF8 (name, 24);
    }

    bool getProgramNameIndexed (VstInt32 /*category*/, VstInt32 index, char* text)
    {
        if (filter != nullptr && isPositiveAndBelow (index, filter->getNumPrograms()))
        {
            filter->getProgramName (index).copyToUTF8 (text, 24);
            return true;
        }

        return false;
    }

    //==============================================================================
    float getParameter (VstInt32 index)
    {
        if (filter == nullptr)
            return 0.0f;

        jassert (isPositiveAndBelow (index, filter->getNumParameters()));
        return filter->getParameter (index);
    }

    void setParameter (VstInt32 index, float value)
    {
        if (filter != nullptr)
        {
            jassert (isPositiveAndBelow (index, filter->getNumParameters()));
            filter->setParameter (index, value);
        }
    }

    void getParameterDisplay (VstInt32 index, char* text)
    {
        if (filter != nullptr)
        {
            jassert (isPositiveAndBelow (index, filter->getNumParameters()));
            filter->getParameterText (index).copyToUTF8 (text, 24); // length should technically be kVstMaxParamStrLen, which is 8, but hosts will normally allow a bit more.
        }
    }

    void getParameterName (VstInt32 index, char* text)
    {
        if (filter != nullptr)
        {
            jassert (isPositiveAndBelow (index, filter->getNumParameters()));
            filter->getParameterName (index).copyToUTF8 (text, 16); // length should technically be kVstMaxParamStrLen, which is 8, but hosts will normally allow a bit more.
        }
    }

    void audioProcessorParameterChanged (AudioProcessor*, int index, float newValue)
    {
        if (audioMaster != nullptr)
            audioMaster (&cEffect, audioMasterAutomate, index, 0, 0, newValue);
    }

    void audioProcessorParameterChangeGestureBegin (AudioProcessor*, int index)   { beginEdit (index); }
    void audioProcessorParameterChangeGestureEnd   (AudioProcessor*, int index)   { endEdit   (index); }

    void audioProcessorChanged (AudioProcessor*)
    {
        setInitialDelay (filter->getLatencySamples());
        ioChanged();
        updateDisplay();
    }

    bool canParameterBeAutomated (VstInt32 index)
    {
        return filter != nullptr && filter->isParameterAutomatable ((int) index);
    }

    struct ChannelConfigComparator
    {
        static int compareElements (const short* const first, const short* const second) noexcept
        {
            if (first[0] < second[0])  return -1;
            if (first[0] > second[0])  return 1;
            if (first[1] < second[1])  return -1;
            if (first[1] > second[1])  return 1;
            return 0;
        }
    };

    bool setSpeakerArrangement (VstSpeakerArrangement* pluginInput,
                                VstSpeakerArrangement* pluginOutput)
    {
        short channelConfigs[][2] = { JucePlugin_PreferredChannelConfigurations };

        Array <short*> channelConfigsSorted;
        ChannelConfigComparator comp;

        for (int i = 0; i < numElementsInArray (channelConfigs); ++i)
            channelConfigsSorted.addSorted (comp, channelConfigs[i]);

        for (int i = channelConfigsSorted.size(); --i >= 0;)
        {
            const short* const config = channelConfigsSorted.getUnchecked(i);
            bool inCountMatches  = (config[0] == pluginInput->numChannels);
            bool outCountMatches = (config[1] == pluginOutput->numChannels);

            if (inCountMatches && outCountMatches)
            {
                speakerIn  = (VstSpeakerArrangementType) pluginInput->type;
                speakerOut = (VstSpeakerArrangementType) pluginOutput->type;
                numInChans  = pluginInput->numChannels;
                numOutChans = pluginOutput->numChannels;

                filter->setPlayConfigDetails (numInChans, numOutChans,
                                              filter->getSampleRate(),
                                              filter->getBlockSize());

                filter->setSpeakerArrangement (getSpeakerArrangementString (speakerIn),
                                               getSpeakerArrangementString (speakerOut));
                return true;
            }
        }

        filter->setSpeakerArrangement (String::empty, String::empty);
        return false;
    }

    static const char* getSpeakerArrangementString (VstSpeakerArrangementType type) noexcept
    {
        switch (type)
        {
            case kSpeakerArrMono:           return "M";
            case kSpeakerArrStereo:         return "L R";
            case kSpeakerArrStereoSurround: return "Ls Rs";
            case kSpeakerArrStereoCenter:   return "Lc Rc";
            case kSpeakerArrStereoSide:     return "Sl Sr";
            case kSpeakerArrStereoCLfe:     return "C Lfe";
            case kSpeakerArr30Cine:         return "L R C";
            case kSpeakerArr30Music:        return "L R S";
            case kSpeakerArr31Cine:         return "L R C Lfe";
            case kSpeakerArr31Music:        return "L R Lfe S";
            case kSpeakerArr40Cine:         return "L R C S";
            case kSpeakerArr40Music:        return "L R Ls Rs";
            case kSpeakerArr41Cine:         return "L R C Lfe S";
            case kSpeakerArr41Music:        return "L R Lfe Ls Rs";
            case kSpeakerArr50:             return "L R C Ls Rs" ;
            case kSpeakerArr51:             return "L R C Lfe Ls Rs";
            case kSpeakerArr60Cine:         return "L R C Ls Rs Cs";
            case kSpeakerArr60Music:        return "L R Ls Rs Sl Sr ";
            case kSpeakerArr61Cine:         return "L R C Lfe Ls Rs Cs";
            case kSpeakerArr61Music:        return "L R Lfe Ls Rs Sl Sr";
            case kSpeakerArr70Cine:         return "L R C Ls Rs Lc Rc ";
            case kSpeakerArr70Music:        return "L R C Ls Rs Sl Sr";
            case kSpeakerArr71Cine:         return "L R C Lfe Ls Rs Lc Rc";
            case kSpeakerArr71Music:        return "L R C Lfe Ls Rs Sl Sr";
            case kSpeakerArr80Cine:         return "L R C Ls Rs Lc Rc Cs";
            case kSpeakerArr80Music:        return "L R C Ls Rs Cs Sl Sr";
            case kSpeakerArr81Cine:         return "L R C Lfe Ls Rs Lc Rc Cs";
            case kSpeakerArr81Music:        return "L R C Lfe Ls Rs Cs Sl Sr" ;
            case kSpeakerArr102:            return "L R C Lfe Ls Rs Tfl Tfc Tfr Trl Trr Lfe2";
            default:                        break;
        }

        return nullptr;
    }

    //==============================================================================
    VstInt32 getChunk (void** data, bool onlyStoreCurrentProgramData)
    {
        if (filter == nullptr)
            return 0;

        chunkMemory.setSize (0);
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

    VstInt32 setChunk (void* data, VstInt32 byteSize, bool onlyRestoreCurrentProgramData)
    {
        if (filter != nullptr)
        {
            chunkMemory.setSize (0);
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
            chunkMemoryTime = 0;
            chunkMemory.setSize (0);
        }

       #if JUCE_MAC
        if (hostWindow != 0)
            checkWindowVisibility (hostWindow, editorComp);
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
                    ComponentPeer::getPeer (i)->performAnyPendingRepaintsNow();

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
                    detachComponentFromWindowRef (editorComp, hostWindow);
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

    VstIntPtr dispatcher (VstInt32 opCode, VstInt32 index, VstIntPtr value, void* ptr, float opt)
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
                editorComp->addToDesktop (0);
                hostWindow = (Window) ptr;
                Window editorWnd = (Window) editorComp->getWindowHandle();
                XReparentWindow (display, editorWnd, hostWindow, 0, 0);
              #else
                hostWindow = attachComponentToWindowRef (editorComp, ptr);
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
            if (! (canHostDo (const_cast <char*> ("sizeWindow")) && sizeWindow (newWidth, newHeight)))
            {
                // some hosts don't support the sizeWindow call, so do it manually..
               #if JUCE_MAC
                setNativeHostWindowSize (hostWindow, editorComp, newWidth, newHeight);

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
                    HWND parent = GetParent (w);

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
                peer->handleMovedOrResized();
        }
    }

    static PluginHostType& getHostType()
    {
        static PluginHostType hostType;
        return hostType;
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

            registerMouseWheelHook();
           #endif
        }

        ~EditorCompWrapper()
        {
           #if JUCE_WINDOWS
            unregisterMouseWheelHook();
           #endif

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
            return forwardCurrentKeyEventToHost (this);
        }
       #endif

        AudioProcessorEditor* getEditorComp() const
        {
            return dynamic_cast<AudioProcessorEditor*> (getChildComponent(0));
        }

        void resized() override
        {
            if (Component* const editor = getChildComponent(0))
                editor->setBounds (getLocalBounds());

           #if JUCE_MAC && ! JUCE_64BIT
            updateEditorCompBounds (this);
           #endif
        }

        void childBoundsChanged (Component* child) override
        {
            child->setTopLeftPosition (0, 0);

            const int cw = child->getWidth();
            const int ch = child->getHeight();

           #if JUCE_MAC && JUCE_64BIT
            setTopLeftPosition (0, getHeight() - ch);
           #endif

            wrapper.resizeHostWindow (cw, ch);

           #if ! JUCE_LINUX // setSize() on linux causes renoise and energyxt to fail.
            setSize (cw, ch);
           #else
            XResizeWindow (display, (Window) getWindowHandle(), cw, ch);
           #endif

           #if JUCE_MAC
            wrapper.resizeHostWindow (cw, ch);  // (doing this a second time seems to be necessary in tracktion)
           #endif
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
            if (HWND parent = findMDIParentOf ((HWND) getWindowHandle()))
                SetWindowPos (parent, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
        }
       #endif

    private:
        //==============================================================================
        JuceVSTWrapper& wrapper;
        FakeMouseMoveGenerator fakeMouseGenerator;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EditorCompWrapper)
    };

    //==============================================================================
private:
    AudioProcessor* filter;
    juce::MemoryBlock chunkMemory;
    juce::uint32 chunkMemoryTime;
    ScopedPointer<EditorCompWrapper> editorComp;
    ERect editorSize;
    MidiBuffer midiEvents;
    VSTMidiEventList outgoingEvents;
    VstSpeakerArrangementType speakerIn, speakerOut;
    int numInChans, numOutChans;
    bool isProcessing, isBypassed, hasShutdown, firstProcessCallback, shouldDeleteEditor;
    HeapBlock<float*> channels;
    Array<float*> tempChannels;  // see note in processReplacing()

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

                class MessageThreadCallback  : public CallbackMessage
                {
                public:
                    MessageThreadCallback (bool& tr) : triggered (tr) {}

                    void messageCallback() override
                    {
                        triggered = true;
                    }

                private:
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
    void deleteTempChannels()
    {
        for (int i = tempChannels.size(); --i >= 0;)
            delete[] (tempChannels.getUnchecked(i));

        tempChannels.clear();

        if (filter != nullptr)
            tempChannels.insertMultiple (0, 0, filter->getNumInputChannels() + filter->getNumOutputChannels());
    }

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
        initialiseMac();
        return pluginEntryPoint (audioMaster);
    }

    JUCE_EXPORTED_FUNCTION AEffect* main_macho (audioMasterCallback audioMaster);
    JUCE_EXPORTED_FUNCTION AEffect* main_macho (audioMasterCallback audioMaster)
    {
        initialiseMac();
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

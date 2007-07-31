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

/*
                        ***  DON't EDIT THIS FILE!!  ***

    The idea is that everyone's plugins should share this same wrapper
    code, so if you start hacking around in here you're missing the point!

    If there's a bug or a function you need that can't be done without changing
    some of the code in here, give me a shout so we can add it to the library,
    rather than branching off and going it alone!
*/


//==============================================================================
#ifdef _MSC_VER
  #pragma warning (disable : 4996)
#endif

#ifdef _WIN32
  #include <windows.h>
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

#include "../../juce_IncludeCharacteristics.h"

//==============================================================================
/*  These files come with the Steinberg VST SDK - to get them, you'll need to
    visit the Steinberg website and jump through some hoops to sign up as a
    VST developer.

    Then, you'll need to make sure your include path contains your "vstsdk2.3" or
    "vstsdk2.4" directory.

    Note that the JUCE_USE_VSTSDK_2_4 macro should be defined in JucePluginCharacteristics.h
*/
#if JUCE_USE_VSTSDK_2_4
 // VSTSDK V2.4 includes..
 #include "public.sdk/source/vst2.x/audioeffectx.h"
 #include "public.sdk/source/vst2.x/aeffeditor.h"
 #include "public.sdk/source/vst2.x/audioeffectx.cpp"
 #include "public.sdk/source/vst2.x/audioeffect.cpp"
#else
 // VSTSDK V2.3 includes..
 #include "source/common/audioeffectx.h"
 #include "source/common/AEffEditor.hpp"
 #include "source/common/audioeffectx.cpp"
 #include "source/common/AudioEffect.cpp"
 typedef long VstInt32;
 typedef long VstIntPtr;
#endif

//==============================================================================
#include "../../juce_AudioFilterBase.h"
#undef MemoryBlock

class JuceVSTWrapper;
static bool recursionCheck = false;
static uint32 lastMasterIdleCall = 0;

BEGIN_JUCE_NAMESPACE
 extern void juce_callAnyTimersSynchronously();

 #if JUCE_MAC
  extern void juce_macDoPendingRepaintsNow();
 #elif JUCE_LINUX
  extern Display* display;
  extern bool juce_postMessageToSystemQueue (void* message);
 #endif
END_JUCE_NAMESPACE


//==============================================================================
#if JUCE_WIN32

static HWND findMDIParentOf (HWND w)
{
    const int frameThickness = GetSystemMetrics (SM_CYFIXEDFRAME);

    while (w != 0)
    {
        HWND parent = GetParent (w);

        if (parent == 0)
            break;

        TCHAR windowType [32];
        zeromem (windowType, sizeof (windowType));
        GetClassName (parent, windowType, 31);

        if (String (windowType).equalsIgnoreCase (T("MDIClient")))
        {
            w = parent;
            break;
        }

        RECT windowPos;
        GetWindowRect (w, &windowPos);

        RECT parentPos;
        GetWindowRect (parent, &parentPos);

        int dw = (parentPos.right - parentPos.left) - (windowPos.right - windowPos.left);
        int dh = (parentPos.bottom - parentPos.top) - (windowPos.bottom - windowPos.top);

        if (dw > 100 || dh > 100)
            break;

        w = parent;

        if (dw == 2 * frameThickness)
            break;
    }

    return w;
}

//==============================================================================
#elif JUCE_LINUX

class SharedMessageThread : public Thread
{
public:
    SharedMessageThread()
      : Thread (T("VstMessageThread"))
    {
        startThread (7);
    }

    ~SharedMessageThread()
    {
        signalThreadShouldExit();

        const int quitMessageId = 0xfffff321;
        Message* const m = new Message (quitMessageId, 1, 0, 0);

        if (! juce_postMessageToSystemQueue (m))
            delete m;

        clearSingletonInstance();
    }

    void run()
    {
        MessageManager* const messageManager = MessageManager::getInstance();

        const int originalThreadId = messageManager->getCurrentMessageThread();
        messageManager->setCurrentMessageThread (getThreadId());

        while (! threadShouldExit()
                && messageManager->dispatchNextMessage())
        {
        }

        messageManager->setCurrentMessageThread (originalThreadId);
    }

    juce_DeclareSingleton (SharedMessageThread, false)
};

juce_ImplementSingleton (SharedMessageThread);

#endif

//==============================================================================
// A component to hold the AudioFilterEditor, and cope with some housekeeping
// chores when it changes or repaints.
class EditorCompWrapper  : public Component,
                           public AsyncUpdater
{
    JuceVSTWrapper* wrapper;

public:
    EditorCompWrapper (JuceVSTWrapper* const wrapper_,
                       AudioFilterEditor* const editor)
        : wrapper (wrapper_)
    {
        setOpaque (true);
        editor->setOpaque (true);

        setBounds (editor->getBounds());
        editor->setTopLeftPosition (0, 0);
        addAndMakeVisible (editor);

#if JUCE_WIN32
        addMouseListener (this, true);
#endif
    }

    ~EditorCompWrapper()
    {
        deleteAllChildren();
    }

    void paint (Graphics& g)
    {
    }

    void paintOverChildren (Graphics& g)
    {
        // this causes an async call to masterIdle() to help
        // creaky old DAWs like Nuendo repaint themselves while we're
        // repainting. Otherwise they just seem to give up and sit there
        // waiting.
        triggerAsyncUpdate();
    }

    AudioFilterEditor* getEditorComp() const
    {
        return dynamic_cast <AudioFilterEditor*> (getChildComponent (0));
    }

    void resized()
    {
        Component* const c = getChildComponent (0);

        if (c != 0)
            c->setBounds (0, 0, getWidth(), getHeight());
    }

    void childBoundsChanged (Component* child);
    void handleAsyncUpdate();

#if JUCE_WIN32
    void mouseDown (const MouseEvent&)
    {
        broughtToFront();
    }

    void broughtToFront()
    {
        // for hosts like nuendo, need to also pop the MDI container to the
        // front when our comp is clicked on.
        HWND parent = findMDIParentOf ((HWND) getWindowHandle());

        if (parent != 0)
        {
            SetWindowPos (parent,
                          HWND_TOP,
                          0, 0, 0, 0,
                          SWP_NOMOVE | SWP_NOSIZE);
        }
    }
#endif

    //==============================================================================
    juce_UseDebuggingNewOperator
};

static VoidArray activePlugins;


//==============================================================================
/**
    This wraps an AudioFilterBase as an AudioEffectX...
*/
class JuceVSTWrapper  : public AudioEffectX,
                        private Timer,
                        public AudioFilterBase::FilterNativeCallbacks
{
public:
    //==============================================================================
    JuceVSTWrapper (audioMasterCallback audioMaster,
                    AudioFilterBase* const filter_)
       : AudioEffectX (audioMaster,
                       filter_->getNumPrograms(),
                       filter_->getNumParameters()),
         filter (filter_)
    {
        filter->numInputChannels = JucePlugin_MaxNumInputChannels;
        filter->numOutputChannels = JucePlugin_MaxNumOutputChannels;

        filter_->initialiseInternal (this);

        editorComp = 0;
        outgoingEvents = 0;
        outgoingEventSize = 0;
        chunkMemoryTime = 0;
        isProcessing = false;
        firstResize = true;

#if JUCE_MAC || JUCE_LINUX
        hostWindow = 0;
#endif

        cEffect.flags |= effFlagsHasEditor;

        setUniqueID ((int) (JucePlugin_VSTUniqueID));
        getAeffect()->version = (long) (JucePlugin_VersionCode);

#if JucePlugin_WantsMidiInput && ! JUCE_USE_VSTSDK_2_4
        wantEvents();
#endif

        setNumInputs (filter->numInputChannels);
        setNumOutputs (filter->numOutputChannels);

        canProcessReplacing (true);

#if ! JUCE_USE_VSTSDK_2_4
        hasVu (false);
        hasClip (false);
#endif

        isSynth ((JucePlugin_IsSynth) != 0);
        noTail ((JucePlugin_SilenceInProducesSilenceOut) != 0);
        setInitialDelay (JucePlugin_Latency);
        programsAreChunks (true);

        VstPinProperties props;

        int i;
        for (i = 0; i < filter->numInputChannels; ++i)
        {
            String s (i + 1);

            if (getInputProperties (i, &props))
            {
                s = props.label;

                if (s.isEmpty())
                {
                    if ((props.flags & kVstPinIsStereo) != 0)
                        s = ((i & 1) == 0) ? T("left") : T("right");
                    else
                        s = String (i + 1);
                }
            }

            filter->inputNames.add (s);
        }

        for (i = 0; i < filter->numOutputChannels; ++i)
        {
            String s (i + 1);

            if (getOutputProperties (i, &props))
            {
                s = props.label;

                if (s.isEmpty())
                {
                    if ((props.flags & kVstPinIsStereo) != 0)
                        s = ((i & 1) == 0) ? T("left") : T("right");
                    else
                        s = String (i + 1);
                }
            }

            filter->outputNames.add (s);
        }

        activePlugins.add (this);
    }

    ~JuceVSTWrapper()
    {
        stopTimer();
        deleteEditor();

        delete filter;
        filter = 0;

        if (outgoingEvents != 0)
        {
            for (int i = outgoingEventSize; --i >= 0;)
                juce_free (outgoingEvents->events[i]);

            juce_free (outgoingEvents);
            outgoingEvents = 0;
        }

        jassert (editorComp == 0);

        jassert (activePlugins.contains (this));
        activePlugins.removeValue (this);

#if JUCE_MAC || JUCE_LINUX
        if (activePlugins.size() == 0)
        {
#if JUCE_LINUX
            SharedMessageThread::deleteInstance();
#endif
            shutdownJuce_GUI();
        }
#endif
    }

    void open()
    {
        startTimer (1000 / 4);
    }

    void close()
    {
        jassert (! recursionCheck);

        stopTimer();
        deleteEditor();
    }

    //==============================================================================
    bool getEffectName (char* name)
    {
        String (JucePlugin_Name).copyToBuffer (name, 64);
        return true;
    }

    bool getVendorString (char* text)
    {
        String (JucePlugin_Manufacturer).copyToBuffer (text, 64);
        return true;
    }

    bool getProductString (char* text)
    {
        return getEffectName (text);
    }

    VstInt32 getVendorVersion()
    {
        return 1000;
    }

    VstPlugCategory getPlugCategory()
    {
        return JucePlugin_VSTCategory;
    }

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
        else if (strcmp (text, "receiveVstTimeInfo") == 0)
        {
            result = 1;
        }
        else if (strcmp (text, "conformsToWindowRules") == 0)
        {
            result = 1;
        }

        return result;
    }

    bool keysRequired()
    {
        return (JucePlugin_EditorRequiresKeyboardFocus) != 0;
    }

    //==============================================================================
    VstInt32 processEvents (VstEvents* events)
    {
#if JucePlugin_WantsMidiInput
        for (int i = 0; i < events->numEvents; ++i)
        {
            const VstEvent* const e = events->events[i];

            if (e != 0 && e->type == kVstMidiType)
            {
                const VstMidiEvent* const vme = (const VstMidiEvent*) e;

                midiEvents.addEvent ((const uint8*) vme->midiData,
                                     4,
                                     vme->deltaFrames);
            }
        }

        return 1;
#else
        return 0;
#endif
    }

    void process (float** inputs, float** outputs,
                  const VstInt32 numSamples, const bool accumulate)
    {
        // if this fails, the host hasn't called resume() before processing
        jassert (isProcessing);

        // (tragically, some hosts actually need this, although it's stupid to have
        //  to do it here..)
        if (! isProcessing)
            resume();

#ifdef JUCE_DEBUG
        const int numMidiEventsComingIn = midiEvents.getNumEvents();
#endif

        jassert (activePlugins.contains (this));

        {
            const AudioSampleBuffer input (inputs, filter->numInputChannels, numSamples);
            AudioSampleBuffer output (outputs, filter->numOutputChannels, numSamples);

            const ScopedLock sl (filter->getCallbackLock());

            if (filter->suspended)
            {
                if (! accumulate)
                    output.clear();
            }
            else
            {
                filter->processBlock (input, output, accumulate, midiEvents);
            }
        }

        if (! midiEvents.isEmpty())
        {
#if JucePlugin_ProducesMidiOutput
            const int numEvents = midiEvents.getNumEvents();

            ensureOutgoingEventSize (numEvents);
            outgoingEvents->numEvents = 0;

            const uint8* midiEventData;
            int midiEventSize, midiEventPosition;
            MidiBuffer::Iterator i (midiEvents);

            while (i.getNextEvent (midiEventData, midiEventSize, midiEventPosition))
            {
                if (midiEventSize <= 4)
                {
                    VstMidiEvent* const vme = (VstMidiEvent*) outgoingEvents->events [outgoingEvents->numEvents++];

                    memcpy (vme->midiData, midiEventData, midiEventSize);
                    vme->deltaFrames = midiEventPosition;

                    jassert (vme->deltaFrames >= 0 && vme->deltaFrames < numSamples);
                }
            }

            sendVstEventsToHost (outgoingEvents);
#else
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

    void process (float** inputs, float** outputs, VstInt32 numSamples)
    {
        process (inputs, outputs, numSamples, true);
    }

    void processReplacing (float** inputs, float** outputs, VstInt32 numSamples)
    {
        process (inputs, outputs, numSamples, false);
    }

    //==============================================================================
    void resume()
    {
        isProcessing = true;

        filter->sampleRate = getSampleRate();

        jassert (filter->sampleRate > 0);
        if (filter->sampleRate <= 0)
            filter->sampleRate = 44100.0;

        filter->blockSize = getBlockSize();
        jassert (filter->blockSize > 0);

        filter->prepareToPlay (filter->sampleRate, filter->blockSize);
        midiEvents.clear();

        AudioEffectX::resume();

#if JucePlugin_ProducesMidiOutput
        ensureOutgoingEventSize (64);
#endif

#if JucePlugin_WantsMidiInput && ! JUCE_USE_VSTSDK_2_4
        wantEvents();
#endif
    }

    void suspend()
    {
        AudioEffectX::suspend();

        filter->releaseResources();
        midiEvents.clear();

        isProcessing = false;
    }

    bool JUCE_CALLTYPE getCurrentPositionInfo (AudioFilterBase::CurrentPositionInfo& info)
    {
        const VstTimeInfo* const ti = getTimeInfo (kVstPpqPosValid
                                                   | kVstTempoValid
                                                   | kVstBarsValid
                                                   //| kVstCyclePosValid
                                                   | kVstTimeSigValid
                                                   | kVstSmpteValid
                                                   | kVstClockValid);

        if (ti == 0 || ti->sampleRate <= 0)
            return false;

        if ((ti->flags & kVstTempoValid) != 0)
            info.bpm = ti->tempo;
        else
            info.bpm = 0.0;

        if ((ti->flags & kVstTimeSigValid) != 0)
        {
            info.timeSigNumerator = ti->timeSigNumerator;
            info.timeSigDenominator = ti->timeSigDenominator;
        }
        else
        {
            info.timeSigNumerator = 4;
            info.timeSigDenominator = 4;
        }

        info.timeInSeconds = ti->samplePos / ti->sampleRate;

        if ((ti->flags & kVstPpqPosValid) != 0)
            info.ppqPosition = ti->ppqPos;
        else
            info.ppqPosition = 0.0;

        if ((ti->flags & kVstBarsValid) != 0)
            info.ppqPositionOfLastBarStart = ti->barStartPos;
        else
            info.ppqPositionOfLastBarStart = 0.0;

        if ((ti->flags & kVstSmpteValid) != 0)
        {
            info.frameRate = (AudioFilterBase::CurrentPositionInfo::FrameRateType) (int) ti->smpteFrameRate;

            const double fpsDivisors[] = { 24.0, 25.0, 30.0, 30.0, 30.0, 30.0, 1.0 };
            info.editOriginTime = (ti->smpteOffset / (80.0 * fpsDivisors [(int) info.frameRate]));
        }
        else
        {
            info.frameRate = AudioFilterBase::CurrentPositionInfo::fpsUnknown;
            info.editOriginTime = 0;
        }

        info.isRecording = (ti->flags & kVstTransportRecording) != 0;
        info.isPlaying   = (ti->flags & kVstTransportPlaying) != 0 || info.isRecording;

        return true;
    }

    //==============================================================================
    VstInt32 getProgram()
    {
        return filter->getCurrentProgram();
    }

    void setProgram (VstInt32 program)
    {
        filter->setCurrentProgram (program);
    }

    void setProgramName (char* name)
    {
        filter->changeProgramName (filter->getCurrentProgram(), name);
    }

    void getProgramName (char* name)
    {
        filter->getProgramName (filter->getCurrentProgram()).copyToBuffer (name, 24);
    }

    bool getProgramNameIndexed (VstInt32 category, VstInt32 index, char* text)
    {
        if (index >= 0 && index < filter->getNumPrograms())
        {
            filter->getProgramName (index).copyToBuffer (text, 24);
            return true;
        }

        return false;
    }

    //==============================================================================
    float getParameter (VstInt32 index)
    {
        jassert (index >= 0 && index < filter->getNumParameters());
        return filter->getParameter (index);
    }

    void setParameter (VstInt32 index, float value)
    {
        jassert (index >= 0 && index < filter->getNumParameters());
        filter->setParameter (index, value);
    }

    void getParameterDisplay (VstInt32 index, char* text)
    {
        jassert (index >= 0 && index < filter->getNumParameters());
        filter->getParameterText (index).copyToBuffer (text, 64);
    }

    void getParameterName (VstInt32 index, char* text)
    {
        jassert (index >= 0 && index < filter->getNumParameters());
        filter->getParameterName (index).copyToBuffer (text, 8);
    }

    void JUCE_CALLTYPE informHostOfParameterChange (int index, float newValue)
    {
        setParameterAutomated (index, newValue);
    }

    //==============================================================================
    VstInt32 getChunk (void** data, bool onlyStoreCurrentProgramData)
    {
        chunkMemory.setSize (0);
        if (onlyStoreCurrentProgramData)
            filter->getCurrentProgramStateInformation (chunkMemory);
        else
            filter->getStateInformation (chunkMemory);

        *data = (void*) chunkMemory;

        // because the chunk is only needed temporarily by the host (or at least you'd
        // hope so) we'll give it a while and then free it in the timer callback.
        chunkMemoryTime = JUCE_NAMESPACE::Time::getApproximateMillisecondCounter();

        return chunkMemory.getSize();
    }

    VstInt32 setChunk (void* data, VstInt32 byteSize, bool onlyRestoreCurrentProgramData)
    {
        chunkMemory.setSize (0);
        chunkMemoryTime = 0;

        if (byteSize > 0 && data != 0)
        {
            if (onlyRestoreCurrentProgramData)
                filter->setCurrentProgramStateInformation (data, byteSize);
            else
                filter->setStateInformation (data, byteSize);
        }

        return 0;
    }

    void timerCallback()
    {
        if (chunkMemoryTime > 0
             && chunkMemoryTime < JUCE_NAMESPACE::Time::getApproximateMillisecondCounter() - 2000
             && ! recursionCheck)
        {
            chunkMemoryTime = 0;
            chunkMemory.setSize (0);
        }

        tryMasterIdle();
    }

    void tryMasterIdle()
    {
        if (Component::isMouseButtonDownAnywhere()
             && ! recursionCheck)
        {
            const uint32 now = JUCE_NAMESPACE::Time::getMillisecondCounter();

            if (now > lastMasterIdleCall + 20 && editorComp != 0)
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
        if (MessageManager::getInstance()->isThisTheMessageThread())
        {
            if (! recursionCheck)
            {
                const MessageManagerLock mml;

                recursionCheck = true;

                juce_callAnyTimersSynchronously();

                for (int i = ComponentPeer::getNumPeers(); --i >= 0;)
                    ComponentPeer::getPeer (i)->performAnyPendingRepaintsNow();

                recursionCheck = false;
            }
        }
    }

    void createEditorComp()
    {
        if (editorComp == 0)
        {
#if JUCE_LINUX
            const MessageManagerLock mml;
#endif

            AudioFilterEditor* const ed = filter->createEditorIfNeeded();

            if (ed != 0)
            {
                ed->setOpaque (true);
                ed->setVisible (true);

                editorComp = new EditorCompWrapper (this, ed);
            }
        }
    }

    void deleteEditor()
    {
        PopupMenu::dismissAllActiveMenus();

        jassert (! recursionCheck);
        recursionCheck = true;

#if JUCE_LINUX
        const MessageManagerLock mml;
#endif

        if (editorComp != 0)
        {
            Component* const modalComponent = Component::getCurrentlyModalComponent();
            if (modalComponent != 0)
                modalComponent->exitModalState (0);

            filter->editorBeingDeleted (editorComp->getEditorComp());

            deleteAndZero (editorComp);

            // there's some kind of component currently modal, but the host
            // is trying to delete our plugin. You should try to avoid this happening..
            jassert (Component::getCurrentlyModalComponent() == 0);
        }

#if JUCE_MAC || JUCE_LINUX
        hostWindow = 0;
#endif

        recursionCheck = false;
    }

    VstIntPtr dispatcher (VstInt32 opCode, VstInt32 index, VstIntPtr value, void* ptr, float opt)
    {
        if (opCode == effEditIdle)
        {
            doIdleCallback();
            return 0;
        }
        else if (opCode == effEditOpen)
        {
            jassert (! recursionCheck);

            deleteEditor();
            createEditorComp();

            if (editorComp != 0)
            {
#if JUCE_LINUX
                const MessageManagerLock mml;
#endif

                editorComp->setOpaque (true);
                editorComp->setVisible (false);

#if JUCE_WIN32
                editorComp->addToDesktop (0);

                hostWindow = (HWND) ptr;
                HWND editorWnd = (HWND) editorComp->getWindowHandle();

                SetParent (editorWnd, hostWindow);

                DWORD val = GetWindowLong (editorWnd, GWL_STYLE);
                val = (val & ~WS_POPUP) | WS_CHILD;
                SetWindowLong (editorWnd, GWL_STYLE, val);

                editorComp->setVisible (true);
#elif JUCE_LINUX
                editorComp->addToDesktop (0);

                hostWindow = (Window) ptr;

                Window editorWnd = (Window) editorComp->getWindowHandle();

                XReparentWindow (display, editorWnd, hostWindow, 0, 0);

                editorComp->setVisible (true);
#else
                hostWindow = (WindowRef) ptr;
                firstResize = true;

                SetAutomaticControlDragTrackingEnabledForWindow (hostWindow, true);

                WindowAttributes attributes;
                GetWindowAttributes (hostWindow, &attributes);

                HIViewRef parentView = 0;

                if ((attributes & kWindowCompositingAttribute) != 0)
                {
                    HIViewRef root = HIViewGetRoot (hostWindow);
                    HIViewFindByID (root, kHIViewWindowContentID, &parentView);

                    if (parentView == 0)
                        parentView = root;
                }
                else
                {
                    GetRootControl (hostWindow, (ControlRef*) &parentView);

                    if (parentView == 0)
                        CreateRootControl (hostWindow, (ControlRef*) &parentView);
                }

                jassert (parentView != 0); // agh - the host has to provide a compositing window..

                editorComp->setVisible (true);
                editorComp->addToDesktop (0, (void*) parentView);
#endif

                return 1;
            }
        }
        else if (opCode == effEditClose)
        {
            deleteEditor();
            return 0;
        }
        else if (opCode == effEditGetRect)
        {
            createEditorComp();

            if (editorComp != 0)
            {
                editorSize.left = 0;
                editorSize.top = 0;
                editorSize.right = editorComp->getWidth();
                editorSize.bottom = editorComp->getHeight();

                *((ERect**) ptr) = &editorSize;

                return (VstIntPtr) &editorSize;
            }
            else
            {
                return 0;
            }
        }

        return AudioEffectX::dispatcher (opCode, index, value, ptr, opt);
    }

    void resizeHostWindow (int newWidth, int newHeight)
    {
        if (editorComp != 0)
        {
#if ! JUCE_LINUX // linux hosts shouldn't be trusted!
            if (! (canHostDo ("sizeWindow") && sizeWindow (newWidth, newHeight)))
#endif
            {
                // some hosts don't support the sizeWindow call, so do it manually..
#if JUCE_MAC
                Rect r;
                GetWindowBounds (hostWindow, kWindowContentRgn, &r);

                if (firstResize)
                {
                    diffW = (r.right - r.left) - editorComp->getWidth();
                    diffH = (r.bottom - r.top) - editorComp->getHeight();
                    firstResize = false;
                }

                r.right = r.left + newWidth + diffW;
                r.bottom = r.top + newHeight + diffH;

                SetWindowBounds (hostWindow, kWindowContentRgn, &r);

                r.bottom -= r.top;
                r.right -= r.left;
                r.left = r.top = 0;
                InvalWindowRect (hostWindow, &r);
#elif JUCE_LINUX
                Window root;
                int x, y;
                unsigned int width, height, border, depth;

                XGetGeometry (display, hostWindow, &root,
                              &x, &y, &width, &height, &border, &depth);

                newWidth += (width + border) - editorComp->getWidth();
                newHeight += (height + border) - editorComp->getHeight();

                XResizeWindow (display, hostWindow, newWidth, newHeight);
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

                    TCHAR windowType [32];
                    zeromem (windowType, sizeof (windowType));
                    GetClassName (parent, windowType, 31);

                    if (String (windowType).equalsIgnoreCase (T("MDIClient")))
                        break;

                    RECT windowPos;
                    GetWindowRect (w, &windowPos);

                    RECT parentPos;
                    GetWindowRect (parent, &parentPos);

                    SetWindowPos (w, 0, 0, 0,
                                  newWidth + dw,
                                  newHeight + dh,
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
                    SetWindowPos (w, 0, 0, 0,
                                  newWidth + dw,
                                  newHeight + dh,
                                  SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOOWNERZORDER);
#endif
            }

            if (editorComp->getPeer() != 0)
                editorComp->getPeer()->handleMovedOrResized();
        }
    }


    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    AudioFilterBase* filter;
    juce::MemoryBlock chunkMemory;
    uint32 chunkMemoryTime;
    EditorCompWrapper* editorComp;
    ERect editorSize;
    MidiBuffer midiEvents;
    VstEvents* outgoingEvents;
    int outgoingEventSize;
    bool isProcessing;
    bool firstResize;
    int diffW, diffH;

    void ensureOutgoingEventSize (int numEvents)
    {
        if (outgoingEventSize < numEvents)
        {
            numEvents += 32;
            const int size = 16 + sizeof (VstEvent*) * numEvents;

            if (outgoingEvents == 0)
                outgoingEvents = (VstEvents*) juce_calloc (size);
            else
                outgoingEvents = (VstEvents*) juce_realloc (outgoingEvents, size);

            for (int i = outgoingEventSize; i < numEvents; ++i)
            {
                VstMidiEvent* const e = (VstMidiEvent*) juce_calloc (sizeof (VstMidiEvent));
                e->type = kVstMidiType;
                e->byteSize = 24;

                outgoingEvents->events[i] = (VstEvent*) e;
            }

            outgoingEventSize = numEvents;
        }
    }

    const String getHostName()
    {
        char host[256];
        zeromem (host, sizeof (host));
        getHostProductString (host);
        return host;
    }

#if JUCE_MAC
    WindowRef hostWindow;
#elif JUCE_LINUX
    Window hostWindow;
#else
    HWND hostWindow;
#endif
};

//==============================================================================
void EditorCompWrapper::childBoundsChanged (Component* child)
{
    child->setTopLeftPosition (0, 0);

    const int cw = child->getWidth();
    const int ch = child->getHeight();

    wrapper->resizeHostWindow (cw, ch);
    setSize (cw, ch);

#if JUCE_MAC
    wrapper->resizeHostWindow (cw, ch);  // (doing this a second time seems to be necessary in tracktion)
#endif
}

void EditorCompWrapper::handleAsyncUpdate()
{
    wrapper->tryMasterIdle();
}

//==============================================================================
static AEffect* pluginEntryPoint (audioMasterCallback audioMaster)
{
#if JUCE_MAC || JUCE_LINUX
    initialiseJuce_GUI();
#endif

    MessageManager::getInstance()->setTimeBeforeShowingWaitCursor (0);

    try
    {
        if (audioMaster (0, audioMasterVersion, 0, 0, 0, 0) != 0)
        {
            AudioFilterBase* const filter = createPluginFilter();

            if (filter != 0)
            {
                JuceVSTWrapper* const wrapper = new JuceVSTWrapper (audioMaster, filter);
                return wrapper->getAeffect();
            }
        }
    }
    catch (...)
    {}

    return 0;
}


//==============================================================================
// Mac startup code..
#if JUCE_MAC

extern "C" __attribute__ ((visibility("default"))) AEffect* VSTPluginMain (audioMasterCallback audioMaster)
{
    return pluginEntryPoint (audioMaster);
}

extern "C" __attribute__ ((visibility("default"))) AEffect* main_macho (audioMasterCallback audioMaster)
{
    return pluginEntryPoint (audioMaster);
}

//==============================================================================
// Linux startup code..
#elif JUCE_LINUX

extern "C" AEffect* main_plugin (audioMasterCallback audioMaster) asm ("main");

extern "C" AEffect* main_plugin (audioMasterCallback audioMaster)
{
    initialiseJuce_GUI();
    SharedMessageThread::getInstance();

    return pluginEntryPoint (audioMaster);
}

__attribute__((constructor)) void myPluginInit()
{
    // don't put initialiseJuce_GUI here... it will crash !
}

__attribute__((destructor)) void myPluginFini()
{
    // don't put shutdownJuce_GUI here... it will crash !
}

//==============================================================================
// Win32 startup code..
#else

extern "C" __declspec (dllexport) AEffect* VSTPluginMain (audioMasterCallback audioMaster)
{
    return pluginEntryPoint (audioMaster);
}

extern "C" __declspec (dllexport) void* main (audioMasterCallback audioMaster)
{
    return (void*) pluginEntryPoint (audioMaster);
}

BOOL WINAPI DllMain (HINSTANCE instance, DWORD dwReason, LPVOID)
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        PlatformUtilities::setCurrentModuleInstanceHandle (instance);
        initialiseJuce_GUI();
    }
    else if (dwReason == DLL_PROCESS_DETACH)
    {
        shutdownJuce_GUI();
    }

    return TRUE;
}

#endif

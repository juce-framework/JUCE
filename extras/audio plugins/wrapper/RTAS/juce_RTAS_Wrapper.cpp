/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

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

#ifdef _MSC_VER
 // (this is a workaround for a build problem in VC9)
 #define _DO_NOT_DECLARE_INTERLOCKED_INTRINSICS_IN_MEMORY
 #include <intrin.h>
#endif

#include "juce_RTAS_DigiCode_Header.h"

#if JucePlugin_Build_RTAS

#ifdef _MSC_VER
  #include "Mac2Win.H"
#endif

/* Note about include paths
   ------------------------

   To be able to include all the Digidesign headers correctly, you'll need to add this
   lot to your include path:

    c:\yourdirectory\PT_80_SDK\AlturaPorts\TDMPlugins\PluginLibrary\EffectClasses
    c:\yourdirectory\PT_80_SDK\AlturaPorts\TDMPlugins\PluginLibrary\ProcessClasses
    c:\yourdirectory\PT_80_SDK\AlturaPorts\TDMPlugins\PluginLibrary\ProcessClasses\Interfaces
    c:\yourdirectory\PT_80_SDK\AlturaPorts\TDMPlugins\PluginLibrary\Utilities
    c:\yourdirectory\PT_80_SDK\AlturaPorts\TDMPlugins\PluginLibrary\RTASP_Adapt
    c:\yourdirectory\PT_80_SDK\AlturaPorts\TDMPlugins\PluginLibrary\CoreClasses
    c:\yourdirectory\PT_80_SDK\AlturaPorts\TDMPlugins\PluginLibrary\Controls
    c:\yourdirectory\PT_80_SDK\AlturaPorts\TDMPlugins\PluginLibrary\Meters
    c:\yourdirectory\PT_80_SDK\AlturaPorts\TDMPlugins\PluginLibrary\ViewClasses
    c:\yourdirectory\PT_80_SDK\AlturaPorts\TDMPlugins\PluginLibrary\DSPClasses
    c:\yourdirectory\PT_80_SDK\AlturaPorts\TDMPlugins\PluginLibrary\Interfaces
    c:\yourdirectory\PT_80_SDK\AlturaPorts\TDMPlugins\common
    c:\yourdirectory\PT_80_SDK\AlturaPorts\TDMPlugins\common\Platform
    c:\yourdirectory\PT_80_SDK\AlturaPorts\TDMPlugins\SignalProcessing\Public
    C:\yourdirectory\PT_80_SDK\AlturaPorts\TDMPlugIns\DSPManager\Interfaces
    c:\yourdirectory\PT_80_SDK\AlturaPorts\SADriver\Interfaces
    c:\yourdirectory\PT_80_SDK\AlturaPorts\DigiPublic\Interfaces
    c:\yourdirectory\PT_80_SDK\AlturaPorts\Fic\Interfaces\DAEClient
    c:\yourdirectory\PT_80_SDK\AlturaPorts\NewFileLibs\Cmn
    c:\yourdirectory\PT_80_SDK\AlturaPorts\NewFileLibs\DOA
    c:\yourdirectory\PT_80_SDK\AlturaPorts\AlturaSource\PPC_H
    c:\yourdirectory\PT_80_SDK\AlturaPorts\AlturaSource\AppSupport
    c:\yourdirectory\PT_80_SDK\AvidCode\AVX2sdk\AVX\avx2\avx2sdk\inc
    C:\yourdirectory\PT_80_SDK\xplat\AVX\avx2\avx2sdk\inc

   NB. If you hit a huge pile of bugs around here, make sure that you've not got the
   Apple QuickTime headers before the PT headers in your path, because there are
   some filename clashes between them.

*/
#include "CEffectGroupMIDI.h"
#include "CEffectProcessMIDI.h"
#include "CEffectProcessRTAS.h"
#include "CCustomView.h"
#include "CEffectTypeRTAS.h"
#include "CPluginControl.h"
#include "CPluginControl_OnOff.h"
#include "FicProcessTokens.h"

//==============================================================================
#ifdef _MSC_VER
  #pragma pack (push, 8)
#endif

#include "../juce_PluginHeaders.h"

#ifdef _MSC_VER
  #pragma pack (pop)

  #if JUCE_DEBUG
   #define PT_LIB_PATH  JucePlugin_WinBag_path "\\Debug\\lib\\"
  #else
   #define PT_LIB_PATH  JucePlugin_WinBag_path "\\Release\\lib\\"
  #endif

  #pragma comment(lib, PT_LIB_PATH "DAE.lib")
  #pragma comment(lib, PT_LIB_PATH "DigiExt.lib")
  #pragma comment(lib, PT_LIB_PATH "DSI.lib")
  #pragma comment(lib, PT_LIB_PATH "PluginLib.lib")

#endif

#undef Component
#undef MemoryBlock

//==============================================================================
#if JUCE_WIN32
  extern void JUCE_CALLTYPE attachSubWindow (void* hostWindow, int& titleW, int& titleH, JUCE_NAMESPACE::Component* comp);
  extern void JUCE_CALLTYPE resizeHostWindow (void* hostWindow, int& titleW, int& titleH, JUCE_NAMESPACE::Component* comp);
 #if ! JucePlugin_EditorRequiresKeyboardFocus
  extern void JUCE_CALLTYPE passFocusToHostWindow (void* hostWindow);
 #endif
#else
  extern void* attachSubWindow (void* hostWindowRef, JUCE_NAMESPACE::Component* comp);
  extern void removeSubWindow (void* nsWindow, JUCE_NAMESPACE::Component* comp);
  extern void forwardCurrentKeyEventToHostWindow();
#endif

const int midiBufferSize = 1024;
const OSType juceChunkType = 'juce';
static const int bypassControlIndex = 1;

//==============================================================================
/** Somewhere in the codebase of your plugin, you need to implement this function
    and make it return a new instance of the filter subclass that you're building.
*/
extern AudioProcessor* JUCE_CALLTYPE createPluginFilter();


//==============================================================================
static float longToFloat (const long n) throw()
{
    return (float) ((((double) n) + (double) 0x80000000) / (double) 0xffffffff);
}

static long floatToLong (const float n) throw()
{
    return roundDoubleToInt (jlimit (-(double) 0x80000000,
                                     (double) 0x7fffffff,
                                     n * (double) 0xffffffff - (double) 0x80000000));
}

static int numInstances = 0;

//==============================================================================
class JucePlugInProcess  : public CEffectProcessMIDI,
                           public CEffectProcessRTAS,
                           public AudioProcessorListener,
                           public AudioPlayHead,
                           public AsyncUpdater
{
public:
    //==============================================================================
    JucePlugInProcess()
        : midiBufferNode (0),
          midiTransport (0),
          channels (0),
          prepared (false),
          sampleRate (44100.0)
    {
        juceFilter = createPluginFilter();
        jassert (juceFilter != 0);

        AddChunk (juceChunkType, "Juce Audio Plugin Data");

        ++numInstances;
    }

    ~JucePlugInProcess()
    {
        if (mLoggedIn)
            MIDILogOut();

        deleteAndZero (midiBufferNode);
        deleteAndZero (midiTransport);

        if (prepared)
            juceFilter->releaseResources();

        delete juceFilter;
        juce_free (channels);

        if (--numInstances == 0)
            shutdownJuce_GUI();
    }

    //==============================================================================
    class JuceCustomUIView  : public CCustomView,
                              public Timer
    {
    public:
        //==============================================================================
        JuceCustomUIView (AudioProcessor* const filter_,
                          JucePlugInProcess* const process_)
            : filter (filter_),
              process (process_),
              wrapper (0),
              editorComp (0)
        {
            // setting the size in here crashes PT for some reason, so keep it simple..
        }

        ~JuceCustomUIView()
        {
            deleteEditorComp();
        }

        //==============================================================================
        void updateSize()
        {
            if (editorComp == 0)
            {
                editorComp = filter->createEditorIfNeeded();
                jassert (editorComp != 0);
            }

            Rect oldRect;
            GetRect (&oldRect);

            Rect r;
            r.left = 0;
            r.top = 0;
            r.right = editorComp->getWidth();
            r.bottom = editorComp->getHeight();
            SetRect (&r);

            if ((oldRect.right != r.right) || (oldRect.bottom != r.bottom))
                startTimer (50);
        }

        void timerCallback()
        {
            if (! JUCE_NAMESPACE::Component::isMouseButtonDownAnywhere())
            {
                stopTimer();

                // Send a token to the host to tell it about the resize
                SSetProcessWindowResizeToken token (process->fRootNameId, process->fRootNameId);
                FicSDSDispatchToken (&token);
            }
        }

        void attachToWindow (GrafPtr port)
        {
            if (port != 0)
            {
                updateSize();

#if JUCE_WIN32
                void* const hostWindow = (void*) ASI_GethWnd ((WindowPtr) port);
#else
                void* const hostWindow = (void*) GetWindowFromPort (port);
#endif
                deleteAndZero (wrapper);

                wrapper = new EditorCompWrapper (hostWindow, editorComp, this);

                process->touchAllParameters();
            }
            else
            {
                deleteEditorComp();
            }
        }

        void DrawContents (Rect*)
        {
#if JUCE_WIN32
            if (wrapper != 0)
            {
                ComponentPeer* const peer = wrapper->getPeer();

                if (peer != 0)
                {
                    // (seems to be required in PT6.4, but not in 7.x)
                    peer->repaint (0, 0, wrapper->getWidth(), wrapper->getHeight());
                }
            }
#endif
        }

        void DrawBackground (Rect*)  {}

        //==============================================================================
    private:
        AudioProcessor* const filter;
        JucePlugInProcess* const process;
        JUCE_NAMESPACE::Component* wrapper;
        AudioProcessorEditor* editorComp;

        void deleteEditorComp()
        {
            if (editorComp != 0 || wrapper != 0)
            {
#if JUCE_MAC
                const ScopedAutoReleasePool pool;
#endif
                PopupMenu::dismissAllActiveMenus();

                JUCE_NAMESPACE::Component* const modalComponent = JUCE_NAMESPACE::Component::getCurrentlyModalComponent();
                if (modalComponent != 0)
                    modalComponent->exitModalState (0);

                filter->editorBeingDeleted (editorComp);

                deleteAndZero (editorComp);
                deleteAndZero (wrapper);
            }
        }

        //==============================================================================
        // A component to hold the AudioProcessorEditor, and cope with some housekeeping
        // chores when it changes or repaints.
        class EditorCompWrapper  : public JUCE_NAMESPACE::Component
#if ! JUCE_MAC
                                   , public FocusChangeListener
#endif
        {
        public:
            EditorCompWrapper (void* const hostWindow_,
                               Component* const editorComp,
                               JuceCustomUIView* const owner_)
                : hostWindow (hostWindow_),
                  owner (owner_),
                  titleW (0),
                  titleH (0)
            {
#if ! JucePlugin_EditorRequiresKeyboardFocus
                setMouseClickGrabsKeyboardFocus (false);
                setWantsKeyboardFocus (false);
#endif
                setOpaque (true);
                setBroughtToFrontOnMouseClick (true);
                setBounds (editorComp->getBounds());
                editorComp->setTopLeftPosition (0, 0);
                addAndMakeVisible (editorComp);

#if JUCE_WIN32
                attachSubWindow (hostWindow, titleW, titleH, this);
#else
                nsWindow = attachSubWindow (hostWindow, this);
#endif
                setVisible (true);

#if JUCE_WIN32 && ! JucePlugin_EditorRequiresKeyboardFocus
                Desktop::getInstance().addFocusChangeListener (this);
#endif
            }

            ~EditorCompWrapper()
            {
#if JUCE_WIN32 && ! JucePlugin_EditorRequiresKeyboardFocus
                Desktop::getInstance().removeFocusChangeListener (this);
#endif

#if JUCE_MAC
                removeSubWindow (nsWindow, this);
#endif
            }

            void paint (Graphics&)
            {
            }

            void resized()
            {
                JUCE_NAMESPACE::Component* const c = getChildComponent (0);

                if (c != 0)
                    c->setBounds (0, 0, getWidth(), getHeight());

                repaint();
            }

#if JUCE_WIN32
            void globalFocusChanged (JUCE_NAMESPACE::Component*)
            {
  #if ! JucePlugin_EditorRequiresKeyboardFocus
                if (hasKeyboardFocus (true))
                    passFocusToHostWindow (hostWindow);
  #endif
            }
#endif

            void childBoundsChanged (JUCE_NAMESPACE::Component* child)
            {
                setSize (child->getWidth(), child->getHeight());
                child->setTopLeftPosition (0, 0);

#if JUCE_WIN32
                resizeHostWindow (hostWindow, titleW, titleH, this);
#endif
                owner->updateSize();
            }

            void userTriedToCloseWindow()
            {
            }

#if JUCE_MAC && JucePlugin_EditorRequiresKeyboardFocus
            bool keyPressed (const KeyPress& kp)
            {
                owner->updateSize();
                forwardCurrentKeyEventToHostWindow();
                return true;
            }
#endif
            //==============================================================================
            juce_UseDebuggingNewOperator

        private:
            void* const hostWindow;
            void* nsWindow;
            JuceCustomUIView* const owner;
            int titleW, titleH;
        };
    };

    JuceCustomUIView* getView() const
    {
        return dynamic_cast <JuceCustomUIView*> (fOurPlugInView);
    }

    void GetViewRect (Rect* size)
    {
        if (getView() != 0)
            getView()->updateSize();

        CEffectProcessRTAS::GetViewRect (size);
    }

    CPlugInView* CreateCPlugInView()
    {
        return new JuceCustomUIView (juceFilter, this);
    }

    void SetViewPort (GrafPtr port)
    {
        CEffectProcessRTAS::SetViewPort (port);

        if (getView() != 0)
            getView()->attachToWindow (port);
    }

    //==============================================================================
protected:
    ComponentResult GetDelaySamplesLong (long* aNumSamples)
    {
        if (aNumSamples != 0)
            *aNumSamples = juceFilter != 0 ? juceFilter->getLatencySamples() : 0;

        return noErr;
    }

    //==============================================================================
    void EffectInit()
    {
        SFicPlugInStemFormats stems;
        GetProcessType()->GetStemFormats (&stems);

        juceFilter->setPlayConfigDetails (fNumInputs, fNumOutputs,
                                          juceFilter->getSampleRate(), juceFilter->getBlockSize());

        AddControl (new CPluginControl_OnOff ('bypa', "Master Bypass\nMastrByp\nMByp\nByp", false, true));
        DefineMasterBypassControlIndex (bypassControlIndex);

        for (int i = 0; i < juceFilter->getNumParameters(); ++i)
            AddControl (new JucePluginControl (juceFilter, i));

        // we need to do this midi log-in to get timecode, regardless of whether
        // the plugin actually uses midi...
        if (MIDILogIn() == noErr)
        {
#if JucePlugin_WantsMidiInput
            CEffectType* const type = dynamic_cast <CEffectType*> (this->GetProcessType());

            if (type != 0)
            {
                char nodeName [64];
                type->GetProcessTypeName (63, nodeName);
                p2cstrcpy (nodeName, reinterpret_cast <unsigned char*> (nodeName));

                midiBufferNode = new CEffectMIDIOtherBufferedNode (&mMIDIWorld,
                                                                   8192,
                                                                   eLocalNode,
                                                                   nodeName,
                                                                   midiBuffer);

                midiBufferNode->Initialize (1, true);
            }
#endif
        }

        midiTransport = new CEffectMIDITransport (&mMIDIWorld);

        juceFilter->setPlayHead (this);
        juceFilter->addListener (this);
    }

    void handleAsyncUpdate()
    {
        if (! prepared)
        {
            sampleRate = gProcessGroup->GetSampleRate();
            jassert (sampleRate > 0);

            juce_free (channels);
            channels = (float**) juce_calloc (sizeof (float*) * jmax (juceFilter->getNumInputChannels(),
                                                                      juceFilter->getNumOutputChannels()));

            juceFilter->setPlayConfigDetails (fNumInputs, fNumOutputs,
                                              sampleRate, mRTGlobals->mHWBufferSizeInSamples);

            juceFilter->prepareToPlay (sampleRate,
                                       mRTGlobals->mHWBufferSizeInSamples);

            prepared = true;
        }
    }

    void RenderAudio (float** inputs, float** outputs, long numSamples)
    {
        if (! prepared)
        {
            triggerAsyncUpdate();
            bypassBuffers (inputs, outputs, numSamples);
            return;
        }

        if (mBypassed)
        {
            bypassBuffers (inputs, outputs, numSamples);
            return;
        }

#if JucePlugin_WantsMidiInput
        midiEvents.clear();

        const Cmn_UInt32 bufferSize = mRTGlobals->mHWBufferSizeInSamples;

        if (midiBufferNode->GetAdvanceScheduleTime() != bufferSize)
            midiBufferNode->SetAdvanceScheduleTime (bufferSize);

        if (midiBufferNode->FillMIDIBuffer (mRTGlobals->mRunningTime, numSamples) == noErr)
        {
            jassert (midiBufferNode->GetBufferPtr() != 0);
            const int numMidiEvents = midiBufferNode->GetBufferSize();

            for (int i = 0; i < numMidiEvents; ++i)
            {
                const DirectMidiPacket& m = midiBuffer[i];

                jassert ((int) m.mTimestamp < numSamples);

                midiEvents.addEvent (m.mData, m.mLength,
                                     jlimit (0, (int) numSamples - 1, (int) m.mTimestamp));
            }
        }
#endif

#if defined (JUCE_DEBUG) || JUCE_LOG_ASSERTIONS
        const int numMidiEventsComingIn = midiEvents.getNumEvents();
        (void) numMidiEventsComingIn;
#endif

        {
            const ScopedLock sl (juceFilter->getCallbackLock());

            const int numIn = juceFilter->getNumInputChannels();
            const int numOut = juceFilter->getNumOutputChannels();
            const int totalChans = jmax (numIn, numOut);

            if (juceFilter->isSuspended())
            {
                for (int i = 0; i < numOut; ++i)
                    zeromem (outputs [i], sizeof (float) * numSamples);
            }
            else
            {
                {
                    int i;
                    for (i = 0; i < numOut; ++i)
                    {
                        channels[i] = outputs [i];

                        if (i < numIn && inputs != outputs)
                            memcpy (outputs [i], inputs[i], sizeof (float) * numSamples);
                    }

                    for (; i < numIn; ++i)
                        channels [i] = inputs [i];
                }

                AudioSampleBuffer chans (channels, totalChans, numSamples);

                juceFilter->processBlock (chans, midiEvents);
            }
        }

        if (! midiEvents.isEmpty())
        {
#if JucePlugin_ProducesMidiOutput
            const juce::uint8* midiEventData;
            int midiEventSize, midiEventPosition;
            MidiBuffer::Iterator i (midiEvents);

            while (i.getNextEvent (midiEventData, midiEventSize, midiEventPosition))
            {
//                        jassert (midiEventPosition >= 0 && midiEventPosition < (int) numSamples);

                //xxx
            }
#else
            // if your plugin creates midi messages, you'll need to set
            // the JucePlugin_ProducesMidiOutput macro to 1 in your
            // JucePluginCharacteristics.h file
            jassert (midiEvents.getNumEvents() <= numMidiEventsComingIn);
#endif

            midiEvents.clear();
        }
    }

    //==============================================================================
    ComponentResult GetChunkSize (OSType chunkID, long* size)
    {
        if (chunkID == juceChunkType)
        {
            tempFilterData.setSize (0);
            juceFilter->getStateInformation (tempFilterData);

            *size = sizeof (SFicPlugInChunkHeader) + tempFilterData.getSize();
            return noErr;
        }

        return CEffectProcessMIDI::GetChunkSize (chunkID, size);
    }

    ComponentResult GetChunk (OSType chunkID, SFicPlugInChunk* chunk)
    {
        if (chunkID == juceChunkType)
        {
            if (tempFilterData.getSize() == 0)
                juceFilter->getStateInformation (tempFilterData);

            chunk->fSize = sizeof (SFicPlugInChunkHeader) + tempFilterData.getSize();
            tempFilterData.copyTo ((void*) chunk->fData, 0, tempFilterData.getSize());

            tempFilterData.setSize (0);

            return noErr;
        }

        return CEffectProcessMIDI::GetChunk (chunkID, chunk);
    }

    ComponentResult SetChunk (OSType chunkID, SFicPlugInChunk* chunk)
    {
        if (chunkID == juceChunkType)
        {
            tempFilterData.setSize (0);

            if (chunk->fSize - sizeof (SFicPlugInChunkHeader) > 0)
            {
                juceFilter->setStateInformation ((void*) chunk->fData,
                                                 chunk->fSize - sizeof (SFicPlugInChunkHeader));
            }

            return noErr;
        }

        return CEffectProcessMIDI::SetChunk (chunkID, chunk);
    }

    //==============================================================================
    ComponentResult UpdateControlValue (long controlIndex, long value)
    {
        if (controlIndex != bypassControlIndex)
            juceFilter->setParameter (controlIndex - 2, longToFloat (value));
        else
            mBypassed = (value > 0);

        return CProcess::UpdateControlValue (controlIndex, value);
    }

    //==============================================================================
    bool getCurrentPosition (AudioPlayHead::CurrentPositionInfo& info)
    {
        // this method can only be called while the plugin is running
        jassert (prepared);

        Cmn_Float64 bpm = 120.0;
        Cmn_Int32 num = 4, denom = 4;
        Cmn_Int64 ticks = 0;
        Cmn_Bool isPlaying = false;

        if (midiTransport != 0)
        {
            midiTransport->GetCurrentTempo (&bpm);
            midiTransport->IsTransportPlaying (&isPlaying);
            midiTransport->GetCurrentMeter (&num, &denom);
            midiTransport->GetCurrentTickPosition (&ticks);
        }

        info.bpm = bpm;
        info.timeSigNumerator = num;
        info.timeSigDenominator = denom;
        info.isPlaying = isPlaying;
        info.isRecording = false;
        info.ppqPosition = ticks / 960000.0;
        info.ppqPositionOfLastBarStart = 0; //xxx no idea how to get this correctly..

        // xxx incorrect if there are tempo changes, but there's no
        // other way of getting this info..
        info.timeInSeconds = ticks * (60.0 / 960000.0) / bpm;

        double framesPerSec = 24.0;

        switch (fTimeCodeInfo.mFrameRate)
        {
        case ficFrameRate_24Frame:
            info.frameRate = AudioPlayHead::fps24;
            break;

        case ficFrameRate_25Frame:
            info.frameRate = AudioPlayHead::fps25;
            framesPerSec = 25.0;
            break;

        case ficFrameRate_2997NonDrop:
            info.frameRate = AudioPlayHead::fps2997;
            framesPerSec = 29.97002997;
            break;

        case ficFrameRate_2997DropFrame:
            info.frameRate = AudioPlayHead::fps2997drop;
            framesPerSec = 29.97002997;
            break;

        case ficFrameRate_30NonDrop:
            info.frameRate = AudioPlayHead::fps30;
            framesPerSec = 30.0;
            break;

        case ficFrameRate_30DropFrame:
            info.frameRate = AudioPlayHead::fps30drop;
            framesPerSec = 30.0;
            break;

        case ficFrameRate_23976:
            // xxx not strictly true..
            info.frameRate = AudioPlayHead::fps24;
            framesPerSec = 23.976;
            break;

        default:
            info.frameRate = AudioPlayHead::fpsUnknown;
            break;
        }

        info.editOriginTime = fTimeCodeInfo.mFrameOffset / framesPerSec;

        return true;
    }

    void audioProcessorParameterChanged (AudioProcessor*, int index, float newValue)
    {
        SetControlValue (index + 2, floatToLong (newValue));
    }

    void audioProcessorParameterChangeGestureBegin (AudioProcessor*, int index)
    {
        TouchControl (index + 2);
    }

    void audioProcessorParameterChangeGestureEnd (AudioProcessor*, int index)
    {
        ReleaseControl (index + 2);
    }

    void audioProcessorChanged (AudioProcessor*)
    {
        // xxx is there an RTAS equivalent?
    }

    void touchAllParameters()
    {
        for (int i = 0; i < juceFilter->getNumParameters(); ++i)
        {
            audioProcessorParameterChangeGestureBegin (0, i);
            audioProcessorParameterChanged (0, i, juceFilter->getParameter (i));
            audioProcessorParameterChangeGestureEnd (0, i);
        }
    }

    //==============================================================================
private:
    AudioProcessor* juceFilter;
    MidiBuffer midiEvents;
    CEffectMIDIOtherBufferedNode* midiBufferNode;
    CEffectMIDITransport* midiTransport;
    DirectMidiPacket midiBuffer [midiBufferSize];

    JUCE_NAMESPACE::MemoryBlock tempFilterData;
    float** channels;
    bool prepared;
    double sampleRate;

    void bypassBuffers (float** const inputs, float** const outputs, const long numSamples) const
    {
        for (int i = fNumOutputs; --i >= 0;)
        {
            if (i < fNumInputs)
                memcpy (outputs[i], inputs[i], numSamples * sizeof (float));
            else
                zeromem (outputs[i], numSamples * sizeof (float));
        }
    }

    //==============================================================================
    class JucePluginControl   : public CPluginControl
    {
    public:
        //==============================================================================
        JucePluginControl (AudioProcessor* const juceFilter_, const int index_)
            : juceFilter (juceFilter_),
              index (index_)
        {
        }

        ~JucePluginControl()
        {
        }

        //==============================================================================
        OSType GetID() const            { return index + 1; }
        long GetDefaultValue() const    { return floatToLong (0); }
        void SetDefaultValue (long)     {}
        long GetNumSteps() const        { return 0xffffffff; }

        long ConvertStringToValue (const char* valueString) const
        {
            return floatToLong (String (valueString).getFloatValue());
        }

        Cmn_Bool IsKeyValid (long key) const    { return true; }

        void GetNameOfLength (char* name, int maxLength, OSType inControllerType) const
        {
            juceFilter->getParameterName (index).copyToBuffer (name, maxLength);
        }

        long GetPriority() const        { return kFicCooperativeTaskPriority; }

        long GetOrientation() const
        {
            return kDAE_LeftMinRightMax | kDAE_BottomMinTopMax
                | kDAE_RotarySingleDotMode | kDAE_RotaryLeftMinRightMax;
        }

        long GetControlType() const     { return kDAE_ContinuousValues; }

        void GetValueString (char* valueString, int maxLength, long value) const
        {
            juceFilter->getParameterText (index).copyToBuffer (valueString, maxLength);
        }

        Cmn_Bool IsAutomatable() const
        {
            return juceFilter->isParameterAutomatable (index);
        }

    private:
        //==============================================================================
        AudioProcessor* const juceFilter;
        const int index;

        JucePluginControl (const JucePluginControl&);
        const JucePluginControl& operator= (const JucePluginControl&);
    };
};

//==============================================================================
class JucePlugInGroup   : public CEffectGroupMIDI
{
public:
    //==============================================================================
    JucePlugInGroup()
    {
        DefineManufacturerNamesAndID (JucePlugin_Manufacturer, JucePlugin_RTASManufacturerCode);
        DefinePlugInNamesAndVersion (createRTASName(), JucePlugin_VersionCode);

#ifndef JUCE_DEBUG
        AddGestalt (pluginGestalt_IsCacheable);
#endif
    }

    ~JucePlugInGroup()
    {
        shutdownJuce_GUI();
        shutdownJuce_NonGUI();
    }

    //==============================================================================
    void CreateEffectTypes()
    {
        const short channelConfigs[][2] = { JucePlugin_PreferredChannelConfigurations };
        const int numConfigs = numElementsInArray (channelConfigs);

        // You need to actually add some configurations to the JucePlugin_PreferredChannelConfigurations
        // value in your JucePluginCharacteristics.h file..
        jassert (numConfigs > 0);

        for (int i = 0; i < numConfigs; ++i)
        {
            CEffectType* const type
                = new CEffectTypeRTAS ('jcaa' + i,
                                       JucePlugin_RTASProductId,
                                       JucePlugin_RTASCategory);

            type->DefineTypeNames (createRTASName());
            type->DefineSampleRateSupport (eSupports48kAnd96kAnd192k);

            type->DefineStemFormats (getFormatForChans (channelConfigs [i][0] != 0 ? channelConfigs [i][0] : channelConfigs [i][1]),
                                     getFormatForChans (channelConfigs [i][1] != 0 ? channelConfigs [i][1] : channelConfigs [i][0]));

            type->AddGestalt (pluginGestalt_CanBypass);
            type->AddGestalt (pluginGestalt_SupportsVariableQuanta);
            type->AttachEffectProcessCreator (createNewProcess);

            AddEffectType (type);
        }
    }

    void Initialize()
    {
        CEffectGroupMIDI::Initialize();
    }

    //==============================================================================
private:
    static CEffectProcess* createNewProcess()
    {
        // Juce setup
#if JUCE_WIN32
        PlatformUtilities::setCurrentModuleInstanceHandle (gThisModule);
#endif
        initialiseJuce_GUI();

        return new JucePlugInProcess();
    }

    static const String createRTASName()
    {
        return String (JucePlugin_Name) + T("\n")
                    + String (JucePlugin_Name).substring (0, 4);
    }

    static EPlugIn_StemFormat getFormatForChans (const int numChans) throw()
    {
        switch (numChans)
        {
        case 0:
            return ePlugIn_StemFormat_Generic;

        case 1:
            return ePlugIn_StemFormat_Mono;

        case 2:
            return ePlugIn_StemFormat_Stereo;

        case 3:
            return ePlugIn_StemFormat_LCR;

        case 4:
            return ePlugIn_StemFormat_Quad;

        case 5:
            return ePlugIn_StemFormat_5dot0;

        case 6:
            return ePlugIn_StemFormat_5dot1;

        case 7:
            return ePlugIn_StemFormat_6dot1;

        case 8:
            return ePlugIn_StemFormat_7dot1;

        default:
            jassertfalse // hmm - not a valid number of chans for RTAS..
            break;
        }

        return ePlugIn_StemFormat_Generic;
    }
};

void initialiseMacRTAS();

CProcessGroupInterface* CProcessGroup::CreateProcessGroup()
{
#if JUCE_MAC
    initialiseMacRTAS();
#endif
    initialiseJuce_NonGUI();
    return new JucePlugInGroup();
}

#endif

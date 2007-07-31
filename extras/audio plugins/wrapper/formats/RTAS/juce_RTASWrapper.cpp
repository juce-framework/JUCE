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

#include "juce_RTASCompileFlags.h"

#ifdef _MSC_VER
  // the Digidesign projects all use a struct alignment of 2..
  #pragma pack (2)
  #pragma warning (disable: 4267)

  #include "ForcedInclude.h"
  #include "Mac2Win.H"
#endif

/* Note about include paths
   ------------------------

   To be able to include all the Digidesign headers correctly, you'll need to add this
   lot to your include path:

    c:\yourdirectory\PT_711_SDK\AlturaPorts\TDMPlugins\PluginLibrary\EffectClasses
    c:\yourdirectory\PT_711_SDK\AlturaPorts\TDMPlugins\PluginLibrary\ProcessClasses
    c:\yourdirectory\PT_711_SDK\AlturaPorts\TDMPlugins\PluginLibrary\ProcessClasses\Interfaces
    c:\yourdirectory\PT_711_SDK\AlturaPorts\TDMPlugins\PluginLibrary\Utilities
    c:\yourdirectory\PT_711_SDK\AlturaPorts\TDMPlugins\PluginLibrary\RTASP_Adapt
    c:\yourdirectory\PT_711_SDK\AlturaPorts\TDMPlugins\PluginLibrary\CoreClasses
    c:\yourdirectory\PT_711_SDK\AlturaPorts\TDMPlugins\PluginLibrary\Controls
    c:\yourdirectory\PT_711_SDK\AlturaPorts\TDMPlugins\PluginLibrary\Meters
    c:\yourdirectory\PT_711_SDK\AlturaPorts\TDMPlugins\PluginLibrary\ViewClasses
    c:\yourdirectory\PT_711_SDK\AlturaPorts\TDMPlugins\PluginLibrary\DSPClasses
    c:\yourdirectory\PT_711_SDK\AlturaPorts\TDMPlugins\PluginLibrary\Interfaces
    c:\yourdirectory\PT_711_SDK\AlturaPorts\TDMPlugins\common
    c:\yourdirectory\PT_711_SDK\AlturaPorts\TDMPlugins\common\Platform
    c:\yourdirectory\PT_711_SDK\AlturaPorts\TDMPlugins\SignalProcessing\Public
    c:\yourdirectory\PT_711_SDK\AlturaPorts\TDMPlugIns\DSPManager\Interfaces
    c:\yourdirectory\PT_711_SDK\AlturaPorts\SADriver\Interfaces
    c:\yourdirectory\PT_711_SDK\AlturaPorts\DigiPublic\Interfaces
    c:\yourdirectory\PT_711_SDK\AlturaPorts\Fic\Interfaces\DAEClient
    c:\yourdirectory\PT_711_SDK\AlturaPorts\NewFileLibs\Cmn
    c:\yourdirectory\PT_711_SDK\AlturaPorts\NewFileLibs\DOA
    c:\yourdirectory\PT_711_SDK\AlturaPorts\AlturaSource\PPC_H
    c:\yourdirectory\PT_711_SDK\AlturaPorts\AlturaSource\AppSupport
*/

#include "CEffectGroupMIDI.h"
#include "CEffectProcessMIDI.h"
#include "CEffectProcessRTAS.h"
#include "CCustomView.h"
#include "CEffectTypeRTAS.h"
#include "CPluginControl.h"
#include "CPluginControl_OnOff.h"

//==============================================================================
#include "../../juce_AudioFilterBase.h"
#include "../../juce_AudioFilterEditor.h"
#include "../../juce_IncludeCharacteristics.h"
#undef Component

//==============================================================================
#if JUCE_WIN32
  extern void JUCE_CALLTYPE attachSubWindow (void* hostWindow, int& titleW, int& titleH, juce::Component* comp);
  extern void JUCE_CALLTYPE resizeHostWindow (void* hostWindow, int& titleW, int& titleH, juce::Component* comp);
 #if ! JucePlugin_EditorRequiresKeyboardFocus
  extern void JUCE_CALLTYPE passFocusToHostWindow (void* hostWindow);
 #endif
#endif

const int midiBufferSize = 1024;
const OSType juceChunkType = 'juce';
static const int bypassControlIndex = 1;


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

//==============================================================================
class JucePlugInProcess  : public CEffectProcessMIDI,
                           public CEffectProcessRTAS,
                           public AudioFilterBase::FilterNativeCallbacks,
                           public AsyncUpdater
{
public:
    //==============================================================================
    JucePlugInProcess()
        : prepared (false),
          midiBufferNode (0),
          midiTransport (0),
          sampleRate (44100.0)
    {
        juceFilter = createPluginFilter();
        jassert (juceFilter != 0);

        AddChunk (juceChunkType, "Juce Audio Plugin Data");
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
    }

    //==============================================================================
    class JuceCustomUIView  : public CCustomView
    {
    public:
        //==============================================================================
        JuceCustomUIView (AudioFilterBase* const filter_)
            : filter (filter_),
              editorComp (0),
              wrapper (0)
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

            Rect r;
            r.left = 0;
            r.top = 0;
            r.right = editorComp->getWidth();
            r.bottom = editorComp->getHeight();

            SetRect (&r);
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
            }
            else
            {
                deleteEditorComp();
            }
        }

        void DrawContents (Rect* r)
        {
            if (wrapper != 0)
            {
                ComponentPeer* const peer = wrapper->getPeer();

                if (peer != 0)
                {
#if JUCE_WIN32
                    // (seems to be required in PT6.4, but not in 7.x)
                    peer->repaint (0, 0, wrapper->getWidth(), wrapper->getHeight());

#elif JUCE_PPC
                    // This crap is needed because if you resize a window, PT doesn't
                    // update its clip region, so only part of your new window gets drawn.
                    // This overrides the clipping region that's being passed into the Draw
                    // method.
                    Rect visible;
                    GetVisibleRect (&visible);

                    RestoreFocus();
                    Focus (&visible);
#endif
                    peer->performAnyPendingRepaintsNow();
                }
            }
        }

        void DrawBackground (Rect* r)
        {
        }

        //==============================================================================
    private:
        AudioFilterBase* const filter;
        juce::Component* wrapper;
        AudioFilterEditor* editorComp;

        void deleteEditorComp()
        {
            if (editorComp != 0)
            {
                PopupMenu::dismissAllActiveMenus();

                juce::Component* const modalComponent = juce::Component::getCurrentlyModalComponent();
                if (modalComponent != 0)
                    modalComponent->exitModalState (0);

                filter->editorBeingDeleted (editorComp);
                deleteAndZero (editorComp);
                deleteAndZero (wrapper);
            }
        }

        //==============================================================================
        // A component to hold the AudioFilterEditor, and cope with some housekeeping
        // chores when it changes or repaints.
        class EditorCompWrapper  : public juce::Component,
#if JUCE_MAC
                                   public Timer
#else
                                   public FocusChangeListener
#endif
        {
        public:
            EditorCompWrapper (void* const hostWindow_,
                               AudioFilterEditor* const editorComp,
                               JuceCustomUIView* const owner_)
                : hostWindow (hostWindow_),
                  owner (owner_),
                  titleW (0),
                  titleH (0)
#if JUCE_MAC
                  , forcedRepaintTimer (0)
#endif
            {
#if ! JucePlugin_EditorRequiresKeyboardFocus
                setWantsKeyboardFocus (false);
#endif
                setOpaque (true);
                setBroughtToFrontOnMouseClick (true);
                setBounds (editorComp->getBounds());
                editorComp->setTopLeftPosition (0, 0);
                addAndMakeVisible (editorComp);

#if JUCE_WIN32
                attachSubWindow (hostWindow, titleW, titleH, this);
                setVisible (true);
#else
                SetAutomaticControlDragTrackingEnabledForWindow ((WindowRef) hostWindow_, true);

                WindowAttributes attributes;
                GetWindowAttributes ((WindowRef) hostWindow_, &attributes);

                parentView = 0;

                if ((attributes & kWindowCompositingAttribute) != 0)
                {
                    HIViewRef root = HIViewGetRoot ((WindowRef) hostWindow_);
                    HIViewFindByID (root, kHIViewWindowContentID, &parentView);

                    if (parentView == 0)
                        parentView = root;
                }
                else
                {
                    GetRootControl ((WindowRef) hostWindow_, (ControlRef*) &parentView);

                    if (parentView == 0)
                        CreateRootControl ((WindowRef) hostWindow_, (ControlRef*) &parentView);
                }

                jassert (parentView != 0);
                Rect clientRect;
                GetWindowBounds ((WindowRef) hostWindow, kWindowContentRgn, &clientRect);

                titleW = clientRect.right - clientRect.left;
                titleH = jmax (0, (clientRect.bottom - clientRect.top) - getHeight());
                setTopLeftPosition (0, 0);

                HIViewSetNeedsDisplay (parentView, true);

                setVisible (true);
                addToDesktop (ComponentPeer::windowRepaintedExplictly, (void*) parentView);

                HIViewRef pluginView = HIViewGetFirstSubview (parentView);

  #if ! JucePlugin_EditorRequiresKeyboardFocus
                HIViewSetActivated (pluginView, false);
  #endif
                /* This is a convoluted and desperate workaround for a Digi (or maybe Apple)
                   layout bug. Until the parent control gets some kind of mouse-move
                   event, then our plugin's HIView remains stuck at (0, 0) in the
                   window (despite drawing correctly), which blocks mouse events from
                   getting to the widgets above it.

                   After days of frustration the only hack I can find that works
                   is to use this arcane function to redirect mouse events to
                   the parent, while running a timer to spot the moment when our
                   view mysteriously snaps back to its correct location.

                   If anyone at Digi or Apple is reading this and they realise that it's
                   their fault, could they please give me back the week of my life that
                   they made me waste on this rubbish. Thankyou.
                */
                SetControlSupervisor (pluginView, parentView);
                startTimer (150);
#endif

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
                delete forcedRepaintTimer;
#endif
            }

            void paint (Graphics& g)
            {
#if JUCE_MAC
                if (forcedRepaintTimer != 0)
                    forcedRepaintTimer->stopTimer();
#endif
            }

            void resized()
            {
                juce::Component* const c = getChildComponent (0);

                if (c != 0)
                    c->setBounds (0, 0, getWidth(), getHeight());

                repaint();
            }

#if JUCE_MAC
            void timerCallback()
            {
                // Wait for the moment when PT deigns to allow our view to
                // take up its actual location (see rant above)
                HIViewRef content = 0;
                HIViewFindByID (HIViewGetRoot ((WindowRef) hostWindow), kHIViewWindowContentID, &content);
                HIPoint p = { 0.0f, 0.0f };

                HIViewRef v = HIViewGetFirstSubview (parentView);
                HIViewConvertPoint (&p, v, content);

                if (p.y > 12)
                {
                    HIViewRef v = HIViewGetFirstSubview (parentView);
                    SetControlSupervisor (v, 0);
                    stopTimer();

                    forcedRepaintTimer = new RepaintCheckTimer (*this);
                }
            }
#endif

#if JUCE_WIN32
            void globalFocusChanged (juce::Component*)
            {
  #if ! JucePlugin_EditorRequiresKeyboardFocus
                if (hasKeyboardFocus (true))
                    passFocusToHostWindow (hostWindow);
  #endif
            }
#endif

            void childBoundsChanged (juce::Component* child)
            {
                setSize (child->getWidth(), child->getHeight());
                child->setTopLeftPosition (0, 0);

#if JUCE_WIN32
                resizeHostWindow (hostWindow, titleW, titleH, this);
#else
                Rect r;
                GetWindowBounds ((WindowRef) hostWindow, kWindowContentRgn, &r);

                HIRect p;
                zerostruct (p);
                HIViewConvertRect (&p, parentView, 0); // find the X position of our view in case there's space to the left of it

                r.right = r.left + jmax (titleW, ((int) p.origin.x) + getWidth());
                r.bottom = r.top + getHeight() + titleH;

                SetWindowBounds ((WindowRef) hostWindow, kWindowContentRgn, &r);

                owner->updateSize();
                owner->Invalidate();
#endif
            }

            //==============================================================================
            juce_UseDebuggingNewOperator

#if JUCE_MAC
        protected:
            void internalRepaint (int x, int y, int w, int h)
            {
                Component::internalRepaint (x, y, w, h);
                owner->Invalidate();

                if (forcedRepaintTimer != 0 && ! forcedRepaintTimer->isTimerRunning())
                    forcedRepaintTimer->startTimer (1000 / 25);
            }

            HIViewRef parentView;
#endif

        private:
            void* const hostWindow;
            JuceCustomUIView* const owner;
            int titleW, titleH;

#if JUCE_MAC
            // if PT makes us wait too long for a redraw after we've
            // asked for one, this should kick in and force one to happen
            class RepaintCheckTimer  : public Timer
            {
            public:
                RepaintCheckTimer (EditorCompWrapper& owner_)
                    : owner (owner_)
                {
                }

                void timerCallback()
                {
                    stopTimer();
                    ComponentPeer* const peer = owner.getPeer();

                    if (peer != 0)
                        peer->performAnyPendingRepaintsNow();
                }

                EditorCompWrapper& owner;
            };

            RepaintCheckTimer* forcedRepaintTimer;
#endif
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
        return new JuceCustomUIView (juceFilter);
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
            *aNumSamples = JucePlugin_Latency;

        return noErr;
    }

    //==============================================================================
    void EffectInit()
    {
        SFicPlugInStemFormats stems;
        GetProcessType()->GetStemFormats (&stems);

        juceFilter->numInputChannels = fNumInputs;
        juceFilter->numOutputChannels = fNumOutputs;

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

        juceFilter->initialiseInternal (this);
    }

    void handleAsyncUpdate()
    {
        if (! prepared)
        {
            sampleRate = gProcessGroup->GetSampleRate();
            jassert (sampleRate > 0);

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
#endif

        {
            const AudioSampleBuffer input (inputs, juceFilter->numInputChannels, numSamples);
            AudioSampleBuffer output (outputs, juceFilter->numOutputChannels, numSamples);

            const ScopedLock sl (juceFilter->getCallbackLock());

            if (juceFilter->suspended)
                bypassBuffers (inputs, outputs, numSamples);
            else
                juceFilter->processBlock (input, output, false, midiEvents);
        }

        if (! midiEvents.isEmpty())
        {
#if JucePlugin_ProducesMidiOutput
            const uint8* midiEventData;
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
    bool JUCE_CALLTYPE getCurrentPositionInfo (AudioFilterBase::CurrentPositionInfo& info)
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
            info.frameRate = AudioFilterBase::CurrentPositionInfo::fps24;
            break;

        case ficFrameRate_25Frame:
            info.frameRate = AudioFilterBase::CurrentPositionInfo::fps25;
            framesPerSec = 25.0;
            break;

        case ficFrameRate_2997NonDrop:
            info.frameRate = AudioFilterBase::CurrentPositionInfo::fps2997;
            framesPerSec = 29.97002997;
            break;

        case ficFrameRate_2997DropFrame:
            info.frameRate = AudioFilterBase::CurrentPositionInfo::fps2997drop;
            framesPerSec = 29.97002997;
            break;

        case ficFrameRate_30NonDrop:
            info.frameRate = AudioFilterBase::CurrentPositionInfo::fps30;
            framesPerSec = 30.0;
            break;

        case ficFrameRate_30DropFrame:
            info.frameRate = AudioFilterBase::CurrentPositionInfo::fps30drop;
            framesPerSec = 30.0;
            break;

        case ficFrameRate_23976:
            // xxx not strictly true..
            info.frameRate = AudioFilterBase::CurrentPositionInfo::fps24;
            framesPerSec = 23.976;
            break;

        default:
            info.frameRate = AudioFilterBase::CurrentPositionInfo::fpsUnknown;
            break;
        }

        info.editOriginTime = fTimeCodeInfo.mFrameOffset / framesPerSec;

        return true;
    }

    void JUCE_CALLTYPE informHostOfParameterChange (int index, float newValue)
    {
        SetControlValue (index + 2, floatToLong (newValue));
    }

    //==============================================================================
private:
    AudioFilterBase* juceFilter;
    MidiBuffer midiEvents;
    CEffectMIDIOtherBufferedNode* midiBufferNode;
    CEffectMIDITransport* midiTransport;
    DirectMidiPacket midiBuffer [midiBufferSize];

    juce::MemoryBlock tempFilterData;
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
        JucePluginControl (AudioFilterBase* const juceFilter_, const int index_)
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

        Cmn_Bool IsKeyValid(long key) const     { return true; }

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

        Cmn_Bool IsAutomatable() const  { return true; }

    private:
        //==============================================================================
        AudioFilterBase* const juceFilter;
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

            type->DefineStemFormats (getFormatForChans (channelConfigs [i][0]),
                                     getFormatForChans (channelConfigs [i][1]));

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

CProcessGroupInterface* CProcessGroup::CreateProcessGroup()
{
    initialiseJuce_NonGUI();
    return new JucePlugInGroup();
}

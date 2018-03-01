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

#if JucePlugin_Build_RTAS

#include "../../juce_core/system/juce_TargetPlatform.h"
#include "../utility/juce_CheckSettingMacros.h"

#ifdef _MSC_VER
 // (this is a workaround for a build problem in VC9)
 #define _DO_NOT_DECLARE_INTERLOCKED_INTRINSICS_IN_MEMORY
 #include <intrin.h>

 #ifndef JucePlugin_WinBag_path
  #error "You need to define the JucePlugin_WinBag_path value!"
 #endif
#endif

#include "juce_RTAS_DigiCode_Header.h"

#ifdef _MSC_VER
 #include <Mac2Win.H>
#endif

#ifdef __clang__
 #pragma clang diagnostic push
 #pragma clang diagnostic ignored "-Widiomatic-parentheses"
 #pragma clang diagnostic ignored "-Wnon-virtual-dtor"
 #pragma clang diagnostic ignored "-Wcomment"
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
    c:\yourdirectory\PT_80_SDK\AlturaPorts\DigiPublic
    c:\yourdirectory\PT_80_SDK\AvidCode\AVX2sdk\AVX\avx2\avx2sdk\inc
    c:\yourdirectory\PT_80_SDK\xplat\AVX\avx2\avx2sdk\inc

   NB. If you hit a huge pile of bugs around here, make sure that you've not got the
   Apple QuickTime headers before the PT headers in your path, because there are
   some filename clashes between them.

*/
#include <CEffectGroupMIDI.h>
#include <CEffectProcessMIDI.h>
#include <CEffectProcessRTAS.h>
#include <CCustomView.h>
#include <CEffectTypeRTAS.h>
#include <CPluginControl.h>
#include <CPluginControl_OnOff.h>
#include <FicProcessTokens.h>
#include <ExternalVersionDefines.h>

#ifdef __clang__
 #pragma clang diagnostic pop
#endif

//==============================================================================
#ifdef _MSC_VER
 #pragma pack (push, 8)
 #pragma warning (disable: 4263 4264 4250)
#endif

#include "../utility/juce_IncludeModuleHeaders.h"

using namespace juce;

namespace juce
{

#ifdef _MSC_VER
 #pragma pack (pop)

 // This JUCE_RTAS_LINK_TO_DEBUG_LIB setting can be used to force linkage
 // against only the release build of the RTAS lib, since in older SDKs there
 // can be problems with the debug build.
 #if JUCE_DEBUG && ! defined (JUCE_RTAS_LINK_TO_DEBUG_LIB)
  #define JUCE_RTAS_LINK_TO_DEBUG_LIB 1
 #endif

 #if JUCE_RTAS_LINK_TO_DEBUG_LIB
  #define PT_LIB_PATH  JucePlugin_WinBag_path "\\Debug\\lib\\"
 #else
  #define PT_LIB_PATH  JucePlugin_WinBag_path "\\Release\\lib\\"
 #endif

 #pragma comment(lib, PT_LIB_PATH "DAE.lib")
 #pragma comment(lib, PT_LIB_PATH "DigiExt.lib")
 #pragma comment(lib, PT_LIB_PATH "DSI.lib")
 #pragma comment(lib, PT_LIB_PATH "PluginLib.lib")
 #pragma comment(lib, PT_LIB_PATH "DSPManager.lib")
 #pragma comment(lib, PT_LIB_PATH "DSPManagerClientLib.lib")
 #pragma comment(lib, PT_LIB_PATH "RTASClientLib.lib")
#endif

#undef MemoryBlock

//==============================================================================
#if JUCE_WINDOWS
  extern void JUCE_CALLTYPE attachSubWindow (void* hostWindow, int& titleW, int& titleH, Component* comp);
  extern void JUCE_CALLTYPE resizeHostWindow (void* hostWindow, int& titleW, int& titleH, Component* comp);
 #if ! JucePlugin_EditorRequiresKeyboardFocus
  extern void JUCE_CALLTYPE passFocusToHostWindow (void* hostWindow);
 #endif
#else
  extern void* attachSubWindow (void* hostWindowRef, Component* comp);
  extern void removeSubWindow (void* nsWindow, Component* comp);
  extern void forwardCurrentKeyEventToHostWindow();
#endif

#if ! (JUCE_DEBUG || defined (JUCE_RTAS_PLUGINGESTALT_IS_CACHEABLE))
 #define JUCE_RTAS_PLUGINGESTALT_IS_CACHEABLE 1
#endif

const int midiBufferSize = 1024;
const OSType juceChunkType = 'juce';
static const int bypassControlIndex = 1;

static int numInstances = 0;

//==============================================================================
class JucePlugInProcess  : public CEffectProcessMIDI,
                           public CEffectProcessRTAS,
                           public AudioProcessorListener,
                           public AudioPlayHead
{
public:
    //==============================================================================
    JucePlugInProcess()
        : sampleRate (44100.0)
    {
        juceFilter = createPluginFilterOfType (AudioProcessor::wrapperType_RTAS);

        AddChunk (juceChunkType, "Juce Audio Plugin Data");

        ++numInstances;
    }

    ~JucePlugInProcess()
    {
        JUCE_AUTORELEASEPOOL
        {
            if (mLoggedIn)
                MIDILogOut();

            midiBufferNode = nullptr;
            midiTransport = nullptr;

            if (juceFilter != nullptr)
            {
                juceFilter->releaseResources();
                juceFilter = nullptr;
            }

            if (--numInstances == 0)
            {
               #if JUCE_MAC
                // Hack to allow any NSWindows to clear themselves up before returning to PT..
                for (int i = 20; --i >= 0;)
                    MessageManager::getInstance()->runDispatchLoopUntil (1);
               #endif

                shutdownJuce_GUI();
            }
        }
    }

    //==============================================================================
    class JuceCustomUIView  : public CCustomView,
                              public Timer
    {
    public:
        //==============================================================================
        JuceCustomUIView (AudioProcessor* ap, JucePlugInProcess* p)
            : filter (ap), process (p)
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
            if (editorComp == nullptr)
            {
                editorComp = filter->createEditorIfNeeded();
                jassert (editorComp != nullptr);
            }

            if (editorComp->getWidth() != 0 && editorComp->getHeight() != 0)
            {
                Rect oldRect;
                GetRect (&oldRect);

                Rect r;
                r.left = 0;
                r.top = 0;
                r.right = editorComp->getWidth();
                r.bottom = editorComp->getHeight();
                SetRect (&r);

                if (oldRect.right != r.right || oldRect.bottom != r.bottom)
                    startTimer (50);
            }
        }

        void timerCallback() override
        {
            if (! Component::isMouseButtonDownAnywhere())
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
                JUCE_AUTORELEASEPOOL
                {
                    updateSize();

                   #if JUCE_WINDOWS
                    void* const hostWindow = (void*) ASI_GethWnd ((WindowPtr) port);
                   #else
                    void* const hostWindow = (void*) GetWindowFromPort (port);
                   #endif
                    wrapper = nullptr;
                    wrapper = new EditorCompWrapper (hostWindow, editorComp, this);
                }
            }
            else
            {
                deleteEditorComp();
            }
        }

        void DrawContents (Rect*) override
        {
           #if JUCE_WINDOWS
            if (wrapper != nullptr)
            {
                if (ComponentPeer* const peer = wrapper->getPeer())
                    peer->repaint (wrapper->getLocalBounds());  // (seems to be required in PT6.4, but not in 7.x)
            }
           #endif
        }

        void DrawBackground (Rect*) override  {}

        //==============================================================================
    private:
        AudioProcessor* const filter;
        JucePlugInProcess* const process;
        ScopedPointer<Component> wrapper;
        ScopedPointer<AudioProcessorEditor> editorComp;

        void deleteEditorComp()
        {
            if (editorComp != nullptr || wrapper != nullptr)
            {
                JUCE_AUTORELEASEPOOL
                {
                    PopupMenu::dismissAllActiveMenus();

                    if (Component* const modalComponent = Component::getCurrentlyModalComponent())
                        modalComponent->exitModalState (0);

                    filter->editorBeingDeleted (editorComp);

                    editorComp = nullptr;
                    wrapper = nullptr;
                }
            }
        }

        //==============================================================================
        // A component to hold the AudioProcessorEditor, and cope with some housekeeping
        // chores when it changes or repaints.
        class EditorCompWrapper  : public Component
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

               #if JUCE_WINDOWS
                attachSubWindow (hostWindow, titleW, titleH, this);
               #else
                nsWindow = attachSubWindow (hostWindow, this);
               #endif
                setVisible (true);

               #if JUCE_WINDOWS && ! JucePlugin_EditorRequiresKeyboardFocus
                Desktop::getInstance().addFocusChangeListener (this);
               #endif
            }

            ~EditorCompWrapper()
            {
                removeChildComponent (getEditor());

               #if JUCE_WINDOWS && ! JucePlugin_EditorRequiresKeyboardFocus
                Desktop::getInstance().removeFocusChangeListener (this);
               #endif

               #if JUCE_MAC
                removeSubWindow (nsWindow, this);
               #endif
            }

            void paint (Graphics&) override {}

            void resized() override
            {
                if (Component* const ed = getEditor())
                    ed->setBounds (getLocalBounds());

                repaint();
            }

           #if JUCE_WINDOWS
            void globalFocusChanged (Component*) override
            {
               #if ! JucePlugin_EditorRequiresKeyboardFocus
                if (hasKeyboardFocus (true))
                    passFocusToHostWindow (hostWindow);
               #endif
            }
           #endif

            void childBoundsChanged (Component* child) override
            {
                setSize (child->getWidth(), child->getHeight());
                child->setTopLeftPosition (0, 0);

               #if JUCE_WINDOWS
                resizeHostWindow (hostWindow, titleW, titleH, this);
               #endif
                owner->updateSize();
            }

            void userTriedToCloseWindow() override {}

           #if JUCE_MAC && JucePlugin_EditorRequiresKeyboardFocus
            bool keyPressed (const KeyPress& kp) override
            {
                owner->updateSize();
                forwardCurrentKeyEventToHostWindow();
                return true;
            }
           #endif

        private:
            //==============================================================================
            void* const hostWindow;
            void* nsWindow;
            JuceCustomUIView* const owner;
            int titleW, titleH;

            Component* getEditor() const        { return getChildComponent (0); }

            JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EditorCompWrapper)
        };
    };

    JuceCustomUIView* getView() const
    {
        return dynamic_cast<JuceCustomUIView*> (fOurPlugInView);
    }

    void GetViewRect (Rect* size) override
    {
        if (JuceCustomUIView* const v = getView())
            v->updateSize();

        CEffectProcessRTAS::GetViewRect (size);
    }

    CPlugInView* CreateCPlugInView() override
    {
        return new JuceCustomUIView (juceFilter, this);
    }

    void SetViewPort (GrafPtr port) override
    {
        CEffectProcessRTAS::SetViewPort (port);

        if (JuceCustomUIView* const v = getView())
            v->attachToWindow (port);
    }

    //==============================================================================
    ComponentResult GetDelaySamplesLong (long* aNumSamples) override
    {
        if (aNumSamples != nullptr)
            *aNumSamples = juceFilter != nullptr ? juceFilter->getLatencySamples() : 0;

        return noErr;
    }

    //==============================================================================
    void EffectInit() override
    {
        sampleRate = (double) GetSampleRate();
        jassert (sampleRate > 0);
        const int maxBlockSize = (int) CEffectProcessRTAS::GetMaximumRTASQuantum();
        jassert (maxBlockSize > 0);

        SFicPlugInStemFormats stems;
        GetProcessType()->GetStemFormats (&stems);

        juceFilter->setPlayConfigDetails (fNumInputs, fNumOutputs, sampleRate, maxBlockSize);

        AddControl (new CPluginControl_OnOff ('bypa', "Master Bypass\nMastrByp\nMByp\nByp", false, true));
        DefineMasterBypassControlIndex (bypassControlIndex);

        const int numParameters = juceFilter->getNumParameters();

       #if JUCE_FORCE_USE_LEGACY_PARAM_IDS
        const bool usingManagedParameters = false;
       #else
        const bool usingManagedParameters = (juceFilter->getParameters().size() == numParameters);
       #endif

        for (int i = 0; i < numParameters; ++i)
        {
            OSType rtasParamID = static_cast<OSType> (usingManagedParameters ? juceFilter->getParameterID (i).hashCode() : i);
            AddControl (new JucePluginControl (*juceFilter, i, rtasParamID));
        }

        // we need to do this midi log-in to get timecode, regardless of whether
        // the plugin actually uses midi...
        if (MIDILogIn() == noErr)
        {
           #if JucePlugin_WantsMidiInput
            if (CEffectType* const type = dynamic_cast<CEffectType*> (this->GetProcessType()))
            {
                char nodeName[80] = { 0 };
                type->GetProcessTypeName (63, nodeName);
                nodeName[nodeName[0] + 1] = 0;

                midiBufferNode = new CEffectMIDIOtherBufferedNode (&mMIDIWorld,
                                                                   8192,
                                                                   eLocalNode,
                                                                   nodeName + 1,
                                                                   midiBuffer);

                midiBufferNode->Initialize (0xffff, true);
            }
           #endif
        }

        midiTransport = new CEffectMIDITransport (&mMIDIWorld);
        midiEvents.ensureSize (2048);

        channels.calloc (jmax (juceFilter->getTotalNumInputChannels(),
                               juceFilter->getTotalNumOutputChannels()));

        juceFilter->setPlayHead (this);
        juceFilter->addListener (this);

        juceFilter->prepareToPlay (sampleRate, maxBlockSize);
    }

    void RenderAudio (float** inputs, float** outputs, long numSamples) override
    {
       #if JucePlugin_WantsMidiInput
        midiEvents.clear();

        const Cmn_UInt32 bufferSize = mRTGlobals->mHWBufferSizeInSamples;

        if (midiBufferNode != nullptr)
        {
            if (midiBufferNode->GetAdvanceScheduleTime() != bufferSize)
                midiBufferNode->SetAdvanceScheduleTime (bufferSize);

            if (midiBufferNode->FillMIDIBuffer (mRTGlobals->mRunningTime, numSamples) == noErr)
            {
                jassert (midiBufferNode->GetBufferPtr() != nullptr);
                const int numMidiEvents = midiBufferNode->GetBufferSize();

                for (int i = 0; i < numMidiEvents; ++i)
                {
                    const DirectMidiPacket& m = midiBuffer[i];

                    jassert ((int) m.mTimestamp < numSamples);

                    midiEvents.addEvent (m.mData, m.mLength,
                                         jlimit (0, (int) numSamples - 1, (int) m.mTimestamp));
                }
            }
        }
       #endif

       #if JUCE_DEBUG || JUCE_LOG_ASSERTIONS
        const int numMidiEventsComingIn = midiEvents.getNumEvents();
        ignoreUnused (numMidiEventsComingIn);
       #endif

        {
            const ScopedLock sl (juceFilter->getCallbackLock());

            const int numIn  = juceFilter->getTotalNumInputChannels();
            const int numOut = juceFilter->getTotalNumOutputChannels();
            const int totalChans = jmax (numIn, numOut);

            if (juceFilter->isSuspended())
            {
                for (int i = 0; i < numOut; ++i)
                    FloatVectorOperations::clear (outputs [i], numSamples);
            }
            else
            {
                {
                    int i;
                    for (i = 0; i < numOut; ++i)
                    {
                        channels[i] = outputs [i];

                        if (i < numIn && inputs != outputs)
                            FloatVectorOperations::copy (outputs [i], inputs[i], numSamples);
                    }

                    for (; i < numIn; ++i)
                        channels [i] = inputs [i];
                }

                AudioBuffer<float> chans (channels, totalChans, numSamples);

                if (mBypassed)
                    juceFilter->processBlockBypassed (chans, midiEvents);
                else
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
                //jassert (midiEventPosition >= 0 && midiEventPosition < (int) numSamples);
            }
           #elif JUCE_DEBUG || JUCE_LOG_ASSERTIONS
            // if your plugin creates midi messages, you'll need to set
            // the JucePlugin_ProducesMidiOutput macro to 1 in your
            // JucePluginCharacteristics.h file
            jassert (midiEvents.getNumEvents() <= numMidiEventsComingIn);
           #endif

            midiEvents.clear();
        }
    }

    //==============================================================================
    ComponentResult GetChunkSize (OSType chunkID, long* size) override
    {
        if (chunkID == juceChunkType)
        {
            tempFilterData.reset();
            juceFilter->getStateInformation (tempFilterData);

            *size = sizeof (SFicPlugInChunkHeader) + tempFilterData.getSize();
            return noErr;
        }

        return CEffectProcessMIDI::GetChunkSize (chunkID, size);
    }

    ComponentResult GetChunk (OSType chunkID, SFicPlugInChunk* chunk) override
    {
        if (chunkID == juceChunkType)
        {
            if (tempFilterData.getSize() == 0)
                juceFilter->getStateInformation (tempFilterData);

            chunk->fSize = sizeof (SFicPlugInChunkHeader) + tempFilterData.getSize();
            tempFilterData.copyTo ((void*) chunk->fData, 0, tempFilterData.getSize());

            tempFilterData.reset();

            return noErr;
        }

        return CEffectProcessMIDI::GetChunk (chunkID, chunk);
    }

    ComponentResult SetChunk (OSType chunkID, SFicPlugInChunk* chunk) override
    {
        if (chunkID == juceChunkType)
        {
            tempFilterData.reset();

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
    ComponentResult UpdateControlValue (long controlIndex, long value) override
    {
        if (controlIndex != bypassControlIndex)
        {
            auto paramIndex = controlIndex - 2;
            auto floatValue = longToFloat (value);

            if (auto* param = owner.getParameters()[paramIndex])
            {
                param->setValue (floatValue);
                param->sendValueChangedMessageToListeners (floatValue);
            }
            else
            {
                juceFilter->setParameter (paramIndex, floatValue);
            }
        }
        else
        {
            mBypassed = (value > 0);
        }

        return CProcess::UpdateControlValue (controlIndex, value);
    }

   #if JUCE_WINDOWS
    Boolean HandleKeystroke (EventRecord* e) override
    {
        if (Component* modalComp = Component::getCurrentlyModalComponent())
        {
            if (Component* focused = modalComp->getCurrentlyFocusedComponent())
            {
                switch (e->message & charCodeMask)
                {
                    case kReturnCharCode:
                    case kEnterCharCode:    focused->keyPressed (KeyPress (KeyPress::returnKey)); break;
                    case kEscapeCharCode:   focused->keyPressed (KeyPress (KeyPress::escapeKey)); break;
                    default: break;
                }

                return true;
            }
        }

        return false;
    }
   #endif

    //==============================================================================
    bool getCurrentPosition (AudioPlayHead::CurrentPositionInfo& info) override
    {
        Cmn_Float64 bpm = 120.0;
        Cmn_Int32 num = 4, denom = 4;
        Cmn_Int64 ticks = 0;
        Cmn_Bool isPlaying = false;

        if (midiTransport != nullptr)
        {
            midiTransport->GetCurrentTempo (&bpm);
            midiTransport->IsTransportPlaying (&isPlaying);
            midiTransport->GetCurrentMeter (&num, &denom);

            // (The following is a work-around because GetCurrentTickPosition() doesn't work correctly).
            Cmn_Int64 sampleLocation;

            if (isPlaying)
                midiTransport->GetCurrentRTASSampleLocation (&sampleLocation);
            else
                midiTransport->GetCurrentTDMSampleLocation (&sampleLocation);

            midiTransport->GetCustomTickPosition (&ticks, sampleLocation);

            info.timeInSamples = (int64) sampleLocation;
            info.timeInSeconds = sampleLocation / sampleRate;
        }
        else
        {
            info.timeInSamples = 0;
            info.timeInSeconds = 0;
        }

        info.bpm = bpm;
        info.timeSigNumerator = num;
        info.timeSigDenominator = denom;
        info.isPlaying = isPlaying;
        info.isRecording = false;
        info.ppqPosition = ticks / 960000.0;
        info.ppqPositionOfLastBarStart = 0; //xxx no idea how to get this correctly..
        info.isLooping = false;
        info.ppqLoopStart = 0;
        info.ppqLoopEnd = 0;

        double framesPerSec = 24.0;

        switch (fTimeCodeInfo.mFrameRate)
        {
            case ficFrameRate_24Frame:       info.frameRate = AudioPlayHead::fps24;       break;
            case ficFrameRate_25Frame:       info.frameRate = AudioPlayHead::fps25;       framesPerSec = 25.0; break;
            case ficFrameRate_2997NonDrop:   info.frameRate = AudioPlayHead::fps2997;     framesPerSec = 30.0 * 1000.0 / 1001.0; break;
            case ficFrameRate_2997DropFrame: info.frameRate = AudioPlayHead::fps2997drop; framesPerSec = 30.0 * 1000.0 / 1001.0; break;
            case ficFrameRate_30NonDrop:     info.frameRate = AudioPlayHead::fps30;       framesPerSec = 30.0; break;
            case ficFrameRate_30DropFrame:   info.frameRate = AudioPlayHead::fps30drop;   framesPerSec = 30.0; break;
            case ficFrameRate_23976:         info.frameRate = AudioPlayHead::fps23976;    framesPerSec = 24.0 * 1000.0 / 1001.0; break;
            default:                         info.frameRate = AudioPlayHead::fpsUnknown;  break;
        }

        info.editOriginTime = fTimeCodeInfo.mFrameOffset / framesPerSec;

        return true;
    }

    void audioProcessorParameterChanged (AudioProcessor*, int index, float newValue) override
    {
        SetControlValue (index + 2, floatToLong (newValue));
    }

    void audioProcessorParameterChangeGestureBegin (AudioProcessor*, int index) override
    {
        TouchControl (index + 2);
    }

    void audioProcessorParameterChangeGestureEnd (AudioProcessor*, int index) override
    {
        ReleaseControl (index + 2);
    }

    void audioProcessorChanged (AudioProcessor*) override
    {
        // xxx is there an RTAS equivalent?
    }

private:
    ScopedPointer<AudioProcessor> juceFilter;
    MidiBuffer midiEvents;
    ScopedPointer<CEffectMIDIOtherBufferedNode> midiBufferNode;
    ScopedPointer<CEffectMIDITransport> midiTransport;
    DirectMidiPacket midiBuffer [midiBufferSize];

    juce::MemoryBlock tempFilterData;
    HeapBlock<float*> channels;
    double sampleRate;

    static float longToFloat (const long n) noexcept
    {
        return (float) ((((double) n) + (double) 0x80000000) / (double) 0xffffffff);
    }

    static long floatToLong (const float n) noexcept
    {
        return roundToInt (jlimit (-(double) 0x80000000, (double) 0x7fffffff,
                                   n * (double) 0xffffffff - (double) 0x80000000));
    }

    void bypassBuffers (float** const inputs, float** const outputs, const long numSamples) const
    {
        for (int i = fNumOutputs; --i >= 0;)
        {
            if (i < fNumInputs)
                FloatVectorOperations::copy (outputs[i], inputs[i], numSamples);
            else
                FloatVectorOperations::clear (outputs[i], numSamples);
        }
    }

    //==============================================================================
    class JucePluginControl   : public CPluginControl
    {
    public:
        //==============================================================================
        JucePluginControl (AudioProcessor& p, const int i, OSType rtasParamID)
            : processor (p), index (i), paramID (rtasParamID)
        {
            CPluginControl::SetValue (GetDefaultValue());
        }

        //==============================================================================
        OSType GetID() const            { return paramID; }
        long GetDefaultValue() const    { return floatToLong (processor.getParameterDefaultValue (index)); }
        void SetDefaultValue (long)     {}
        long GetNumSteps() const        { return processor.getParameterNumSteps (index); }

        long ConvertStringToValue (const char* valueString) const
        {
            return floatToLong (String (valueString).getFloatValue());
        }

        Cmn_Bool IsKeyValid (long key) const    { return true; }

        void GetNameOfLength (char* name, int maxLength, OSType inControllerType) const
        {
            // Pro-tools expects all your parameters to have valid names!
            jassert (processor.getParameterName (index, maxLength).isNotEmpty());

            processor.getParameterName (index, maxLength).copyToUTF8 (name, (size_t) maxLength + 1);
        }

        long GetPriority() const        { return kFicCooperativeTaskPriority; }

        long GetOrientation() const
        {
            return processor.isParameterOrientationInverted (index)
                     ? kDAE_RightMinLeftMax | kDAE_TopMinBottomMax | kDAE_RotarySingleDotMode | kDAE_RotaryRightMinLeftMax
                     : kDAE_LeftMinRightMax | kDAE_BottomMinTopMax | kDAE_RotarySingleDotMode | kDAE_RotaryLeftMinRightMax;
        }

        long GetControlType() const     { return kDAE_ContinuousValues; }

        void GetValueString (char* valueString, int maxLength, long value) const
        {
            processor.getParameterText (index, maxLength).copyToUTF8 (valueString, (size_t) maxLength + 1);
        }

        Cmn_Bool IsAutomatable() const
        {
            return processor.isParameterAutomatable (index);
        }

    private:
        //==============================================================================
        AudioProcessor& processor;
        const int index;
        const OSType paramID;

        JUCE_DECLARE_NON_COPYABLE (JucePluginControl)
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
        DefinePlugInNamesAndVersion (createRTASName().toUTF8(), JucePlugin_VersionCode);

       #if JUCE_RTAS_PLUGINGESTALT_IS_CACHEABLE
        AddGestalt (pluginGestalt_IsCacheable);
       #endif
    }

    ~JucePlugInGroup()
    {
        shutdownJuce_GUI();
    }

    static AudioChannelSet rtasChannelSet (int numChannels)
    {
        if (numChannels == 0) return AudioChannelSet::disabled();
        if (numChannels == 1) return AudioChannelSet::mono();
        if (numChannels == 2) return AudioChannelSet::stereo();
        if (numChannels == 3) return AudioChannelSet::createLCR();
        if (numChannels == 4) return AudioChannelSet::quadraphonic();
        if (numChannels == 5) return AudioChannelSet::create5point0();
        if (numChannels == 6) return AudioChannelSet::create5point1();

        #if PT_VERS_MAJOR >= 9
        if (numChannels == 7) return AudioChannelSet::create7point0();
        if (numChannels == 8) return AudioChannelSet::create7point1();
        #else
        if (numChannels == 7) return AudioChannelSet::create7point0SDDS();
        if (numChannels == 8) return AudioChannelSet::create7point1SDDS();
        #endif

        jassertfalse;

        return AudioChannelSet::discreteChannels (numChannels);
    }

    //==============================================================================
    void CreateEffectTypes()
    {
        ScopedPointer<AudioProcessor> plugin = createPluginFilterOfType (AudioProcessor::wrapperType_RTAS);

        const short channelConfigs[][2] = { JucePlugin_PreferredChannelConfigurations };
        const int numConfigs = numElementsInArray (channelConfigs);

        // You need to actually add some configurations to the JucePlugin_PreferredChannelConfigurations
        // value in your JucePluginCharacteristics.h file..
        jassert (numConfigs > 0);

        for (int i = 0; i < numConfigs; ++i)
        {
            if (channelConfigs[i][0] <= 8 && channelConfigs[i][1] <= 8)
            {
                const AudioChannelSet inputLayout  (rtasChannelSet (channelConfigs[i][0]));
                const AudioChannelSet outputLayout (rtasChannelSet (channelConfigs[i][1]));

                const int32 pluginId = plugin->getAAXPluginIDForMainBusConfig (inputLayout, outputLayout, false);

                CEffectType* const type
                    = new CEffectTypeRTAS (pluginId,
                                           JucePlugin_RTASProductId,
                                           JucePlugin_RTASCategory);

                type->DefineTypeNames (createRTASName().toRawUTF8());
                type->DefineSampleRateSupport (eSupports48kAnd96kAnd192k);

                type->DefineStemFormats (getFormatForChans (channelConfigs [i][0] != 0 ? channelConfigs [i][0] : channelConfigs [i][1]),
                                         getFormatForChans (channelConfigs [i][1] != 0 ? channelConfigs [i][1] : channelConfigs [i][0]));

               #if ! JucePlugin_RTASDisableBypass
                type->AddGestalt (pluginGestalt_CanBypass);
               #endif

               #if JucePlugin_RTASDisableMultiMono
                type->AddGestalt (pluginGestalt_DoesntSupportMultiMono);
               #endif

                type->AddGestalt (pluginGestalt_SupportsVariableQuanta);
                type->AttachEffectProcessCreator (createNewProcess);

                AddEffectType (type);
            }
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
       #if JUCE_WINDOWS
        Process::setCurrentModuleInstanceHandle (gThisModule);
       #endif
        PluginHostType::jucePlugInClientCurrentWrapperType = AudioProcessor::wrapperType_RTAS;
        initialiseJuce_GUI();

        return new JucePlugInProcess();
    }

    static String createRTASName()
    {
        return String (JucePlugin_Name) + "\n"
                 + String (JucePlugin_Desc);
    }

    static EPlugIn_StemFormat getFormatForChans (const int numChans) noexcept
    {
        switch (numChans)
        {
            case 0:   return ePlugIn_StemFormat_Generic;
            case 1:   return ePlugIn_StemFormat_Mono;
            case 2:   return ePlugIn_StemFormat_Stereo;
            case 3:   return ePlugIn_StemFormat_LCR;
            case 4:   return ePlugIn_StemFormat_Quad;
            case 5:   return ePlugIn_StemFormat_5dot0;
            case 6:   return ePlugIn_StemFormat_5dot1;

           #if PT_VERS_MAJOR >= 9
            case 7:   return ePlugIn_StemFormat_7dot0DTS;
            case 8:   return ePlugIn_StemFormat_7dot1DTS;
           #else
            case 7:   return ePlugIn_StemFormat_7dot0;
            case 8:   return ePlugIn_StemFormat_7dot1;
           #endif

            default:  jassertfalse; break; // hmm - not a valid number of chans for RTAS..
        }

        return ePlugIn_StemFormat_Generic;
    }
};

void initialiseMacRTAS();

} // namespace juce

CProcessGroupInterface* CProcessGroup::CreateProcessGroup()
{
   #if JUCE_MAC
    initialiseMacRTAS();
   #endif

    return new JucePlugInGroup();
}

#endif

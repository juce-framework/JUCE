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

#include <AudioUnit/AudioUnit.h>
#include "AUMIDIEffectBase.h"
#include "MusicDeviceBase.h"
#include "AUCarbonViewBase.h"
#include "../../juce_IncludeCharacteristics.h"
#include "../../../../../juce.h"

//==============================================================================
#define juceFilterObjectPropertyID 0x1a45ffe9
static VoidArray activePlugins;

static const short channelConfigs[][2] = { JucePlugin_PreferredChannelConfigurations };
static const int numChannelConfigs = numElementsInArray (channelConfigs);

BEGIN_JUCE_NAMESPACE
extern void juce_setCurrentExecutableFileNameFromBundleId (const String& bundleId) throw();
END_JUCE_NAMESPACE

#if JucePlugin_IsSynth
 #define JuceAUBaseClass MusicDeviceBase
#else
 #define JuceAUBaseClass AUMIDIEffectBase
#endif

//==============================================================================
/** Somewhere in the codebase of your plugin, you need to implement this function
    and make it create an instance of the filter subclass that you're building.
*/
extern AudioProcessor* JUCE_CALLTYPE createPluginFilter();


//==============================================================================
class JuceAU   : public JuceAUBaseClass,
                 public AudioProcessorListener,
                 public AudioPlayHead
{
public:
    //==============================================================================
    JuceAU (AudioUnit component)
#if JucePlugin_IsSynth
        : MusicDeviceBase (component, 0, 1),
#else
        : AUMIDIEffectBase (component),
#endif
          juceFilter (0),
          bufferSpace (2, 16),
          channels (0),
          prepared (false)
    {
        CreateElements();

        if (activePlugins.size() == 0)
        {
            initialiseJuce_GUI();

#ifdef JucePlugin_CFBundleIdentifier
            juce_setCurrentExecutableFileNameFromBundleId (JucePlugin_CFBundleIdentifier);
#endif

            MessageManager::getInstance()->setTimeBeforeShowingWaitCursor (0);
        }

        juceFilter = createPluginFilter();
        juceFilter->setPlayHead (this);
        juceFilter->addListener (this);

        jassert (juceFilter != 0);
        Globals()->UseIndexedParameters (juceFilter->getNumParameters());

        activePlugins.add (this);

        zerostruct (auEvent);
        auEvent.mArgument.mParameter.mAudioUnit = GetComponentInstance();
        auEvent.mArgument.mParameter.mScope = kAudioUnitScope_Global;
        auEvent.mArgument.mParameter.mElement = 0;
    }

    ~JuceAU()
    {
        delete juceFilter;
        juceFilter = 0;

        juce_free (channels);
        channels = 0;

        jassert (activePlugins.contains (this));
        activePlugins.removeValue (this);

        if (activePlugins.size() == 0)
            shutdownJuce_GUI();
    }

    //==============================================================================
    ComponentResult GetPropertyInfo (AudioUnitPropertyID inID,
                                     AudioUnitScope inScope,
                                     AudioUnitElement inElement,
                                     UInt32& outDataSize,
                                     Boolean& outWritable)
    {
        if (inScope == kAudioUnitScope_Global)
        {
            if (inID == juceFilterObjectPropertyID)
            {
                outWritable = false;
                outDataSize = sizeof (void*);
                return noErr;
            }
        }

        return JuceAUBaseClass::GetPropertyInfo (inID, inScope, inElement, outDataSize, outWritable);
    }

    ComponentResult GetProperty (AudioUnitPropertyID inID,
                                 AudioUnitScope inScope,
                                 AudioUnitElement inElement,
                                 void* outData)
    {
        if (inScope == kAudioUnitScope_Global)
        {
            if (inID == juceFilterObjectPropertyID)
            {
                *((void**) outData) = (void*) juceFilter;
                return noErr;
            }
        }

        return JuceAUBaseClass::GetProperty (inID, inScope, inElement, outData);
    }

    ComponentResult SaveState (CFPropertyListRef* outData)
    {
        ComponentResult err = JuceAUBaseClass::SaveState (outData);

        if (err != noErr)
            return err;

        jassert (CFGetTypeID (*outData) == CFDictionaryGetTypeID());

        CFMutableDictionaryRef dict = (CFMutableDictionaryRef) *outData;

        if (juceFilter != 0)
        {
            MemoryBlock state;
            juceFilter->getCurrentProgramStateInformation (state);

            if (state.getSize() > 0)
            {
                CFDataRef ourState = CFDataCreate (kCFAllocatorDefault, (const uint8*) state, state.getSize());
                CFDictionarySetValue (dict, CFSTR("jucePluginState"), ourState);
                CFRelease (ourState);
            }
        }

        return noErr;
    }

    ComponentResult RestoreState (CFPropertyListRef inData)
    {
        ComponentResult err = JuceAUBaseClass::RestoreState (inData);

        if (err != noErr)
            return err;

        if (juceFilter != 0)
        {
            CFDictionaryRef dict = (CFDictionaryRef) inData;
            CFDataRef data = 0;

            if (CFDictionaryGetValueIfPresent (dict, CFSTR("jucePluginState"),
                                               (const void**) &data))
            {
                if (data != 0)
                {
                    const int numBytes = (int) CFDataGetLength (data);
                    const uint8* const rawBytes = CFDataGetBytePtr (data);

                    if (numBytes > 0)
                        juceFilter->setCurrentProgramStateInformation (rawBytes, numBytes);
                }
            }
        }

        return noErr;
    }

    UInt32 SupportedNumChannels (const AUChannelInfo** outInfo)
    {
        // You need to actually add some configurations to the JucePlugin_PreferredChannelConfigurations
        // value in your JucePluginCharacteristics.h file..
        jassert (numChannelConfigs > 0);

        if (outInfo != 0)
        {
            *outInfo = channelInfo;

            for (int i = 0; i < numChannelConfigs; ++i)
            {
#if JucePlugin_IsSynth
                channelInfo[i].inChannels = 0;
#else
                channelInfo[i].inChannels = channelConfigs[i][0];
#endif
                channelInfo[i].outChannels = channelConfigs[i][1];
            }
        }

        return numChannelConfigs;
    }

    //==============================================================================
    ComponentResult GetParameterInfo (AudioUnitScope inScope,
                                      AudioUnitParameterID inParameterID,
                                      AudioUnitParameterInfo& outParameterInfo)
    {
        const int index = (int) inParameterID;

        if (inScope == kAudioUnitScope_Global
             && juceFilter != 0
             && index < juceFilter->getNumParameters())
        {
            outParameterInfo.flags = kAudioUnitParameterFlag_IsWritable
                                      | kAudioUnitParameterFlag_IsReadable
                                      | kAudioUnitParameterFlag_HasCFNameString;

            const String name (juceFilter->getParameterName (index));

            // set whether the param is automatable (unnamed parameters aren't allowed to be automated)
            if (name.isEmpty() || ! juceFilter->isParameterAutomatable (index))
                outParameterInfo.flags |= kAudioUnitParameterFlag_NonRealTime;

            AUBase::FillInParameterName (outParameterInfo,
                                         PlatformUtilities::juceStringToCFString (name),
                                         false);

            outParameterInfo.minValue = 0.0f;
            outParameterInfo.maxValue = 1.0f;
            outParameterInfo.defaultValue = 0.0f;
            outParameterInfo.unit = kAudioUnitParameterUnit_Generic;

            return noErr;
        }
        else
        {
            return kAudioUnitErr_InvalidParameter;
        }
    }

    ComponentResult GetParameter (AudioUnitParameterID inID,
                                  AudioUnitScope inScope,
                                  AudioUnitElement inElement,
                                  Float32& outValue)
    {
        if (inScope == kAudioUnitScope_Global && juceFilter != 0)
        {
            outValue = juceFilter->getParameter ((int) inID);
            return noErr;
        }

        return AUBase::GetParameter (inID, inScope, inElement, outValue);
    }

    ComponentResult SetParameter (AudioUnitParameterID inID,
                                  AudioUnitScope inScope,
                                  AudioUnitElement inElement,
                                  Float32 inValue,
                                  UInt32 inBufferOffsetInFrames)
    {
        if (inScope == kAudioUnitScope_Global && juceFilter != 0)
        {
            juceFilter->setParameter ((int) inID, inValue);
            return noErr;
        }

        return AUBase::SetParameter (inID, inScope, inElement, inValue, inBufferOffsetInFrames);
    }

    //==============================================================================
    ComponentResult Version()                   { return JucePlugin_VersionCode; }

    bool SupportsTail()                         { return true; }
    Float64 GetTailTime()                       { return 0; }

    Float64 GetSampleRate()
    {
        return GetOutput(0)->GetStreamFormat().mSampleRate;
    }

    Float64 GetLatency()
    {
        jassert (GetSampleRate() > 0);

        if (GetSampleRate() <= 0)
            return 0.0;

        return juceFilter->getLatencySamples() / GetSampleRate();
    }

    //==============================================================================
    int GetNumCustomUIComponents()              { return 1; }

    void GetUIComponentDescs (ComponentDescription* inDescArray)
    {
        inDescArray[0].componentType = kAudioUnitCarbonViewComponentType;
        inDescArray[0].componentSubType = JucePlugin_AUSubType;
        inDescArray[0].componentManufacturer = JucePlugin_AUManufacturerCode;
        inDescArray[0].componentFlags = 0;
        inDescArray[0].componentFlagsMask = 0;
    }

    //==============================================================================
    bool getCurrentPosition (AudioPlayHead::CurrentPositionInfo& info)
    {
        info.timeSigNumerator = 0;
        info.timeSigDenominator = 0;
        info.timeInSeconds = 0;
        info.editOriginTime = 0;
        info.ppqPositionOfLastBarStart = 0;
        info.isPlaying = false;
        info.isRecording = false;

        switch (lastSMPTETime.mType)
        {
            case kSMPTETimeType24:
                info.frameRate = AudioPlayHead::fps24;
                break;

            case kSMPTETimeType25:
                info.frameRate = AudioPlayHead::fps25;
                break;

            case kSMPTETimeType30Drop:
                info.frameRate = AudioPlayHead::fps30drop;
                break;

            case kSMPTETimeType30:
                info.frameRate = AudioPlayHead::fps30;
                break;

            case kSMPTETimeType2997:
                info.frameRate = AudioPlayHead::fps2997;
                break;

            case kSMPTETimeType2997Drop:
                info.frameRate = AudioPlayHead::fps2997drop;
                break;

            //case kSMPTETimeType60:
            //case kSMPTETimeType5994:
            default:
                info.frameRate = AudioPlayHead::fpsUnknown;
                break;
        }

        if (CallHostBeatAndTempo (&info.ppqPosition, &info.bpm) != noErr)
        {
            info.ppqPosition = 0;
            info.bpm = 0;
        }

        UInt32 outDeltaSampleOffsetToNextBeat;
        double outCurrentMeasureDownBeat;
        float num;
        UInt32 den;

        if (CallHostMusicalTimeLocation (&outDeltaSampleOffsetToNextBeat, &num, &den,
                                         &outCurrentMeasureDownBeat) == noErr)
        {
            info.timeSigNumerator = (int) num;
            info.timeSigDenominator = den;
            info.ppqPositionOfLastBarStart = outCurrentMeasureDownBeat;
        }

        double outCurrentSampleInTimeLine, outCycleStartBeat, outCycleEndBeat;
        Boolean playing, playchanged, looping;

        if (CallHostTransportState (&playing,
                                    &playchanged,
                                    &outCurrentSampleInTimeLine,
                                    &looping,
                                    &outCycleStartBeat,
                                    &outCycleEndBeat) == noErr)
        {
            info.isPlaying = playing;
            info.timeInSeconds = outCurrentSampleInTimeLine / GetSampleRate();
        }

        return true;
    }

    void sendAUEvent (const AudioUnitEventType type, const int index) throw()
    {
        if (AUEventListenerNotify != 0)
        {
            auEvent.mEventType = type;
            auEvent.mArgument.mParameter.mParameterID = (AudioUnitParameterID) index;
            AUEventListenerNotify (0, 0, &auEvent);
        }
    }

    void audioProcessorParameterChanged (AudioProcessor*, int index, float newValue)
    {
        sendAUEvent (kAudioUnitEvent_ParameterValueChange, index);
    }

    void audioProcessorParameterChangeGestureBegin (AudioProcessor*, int index)
    {
        sendAUEvent (kAudioUnitEvent_BeginParameterChangeGesture, index);
    }

    void audioProcessorParameterChangeGestureEnd (AudioProcessor*, int index)
    {
        sendAUEvent (kAudioUnitEvent_EndParameterChangeGesture, index);
    }

    void audioProcessorChanged (AudioProcessor*)
    {
        // xxx is there an AU equivalent?
    }

    bool StreamFormatWritable (AudioUnitScope inScope, AudioUnitElement element)
    {
        return ! IsInitialized();
    }

    // (these two slightly different versions are because the definition changed between 10.4 and 10.5)
    ComponentResult StartNote (MusicDeviceInstrumentID, MusicDeviceGroupID, NoteInstanceID&, UInt32, const MusicDeviceNoteParams&) { return noErr; }
    ComponentResult StartNote (MusicDeviceInstrumentID, MusicDeviceGroupID, NoteInstanceID*, UInt32, const MusicDeviceNoteParams&) { return noErr; }
    ComponentResult StopNote (MusicDeviceGroupID, NoteInstanceID, UInt32)   { return noErr; }

    //==============================================================================
    ComponentResult Initialize()
    {
        SetMaxFramesPerSlice (16384);

#if ! JucePlugin_IsSynth
        const int numIns = GetInput(0) != 0 ? GetInput(0)->GetStreamFormat().mChannelsPerFrame : 0;
#endif
        const int numOuts = GetOutput(0) != 0 ? GetOutput(0)->GetStreamFormat().mChannelsPerFrame : 0;

        bool isValidChannelConfig = false;

        for (int i = 0; i < numChannelConfigs; ++i)
#if JucePlugin_IsSynth
            if (numOuts == channelConfigs[i][1])
#else
            if (numIns == channelConfigs[i][0] && numOuts == channelConfigs[i][1])
#endif
                isValidChannelConfig = true;

        if (! isValidChannelConfig)
            return kAudioUnitErr_FormatNotSupported;

        JuceAUBaseClass::Initialize();
        prepareToPlay();
        return noErr;
    }

    void Cleanup()
    {
        JuceAUBaseClass::Cleanup();

        if (juceFilter != 0)
            juceFilter->releaseResources();

        bufferSpace.setSize (2, 16);
        midiEvents.clear();
        prepared = false;
    }

    ComponentResult Reset (AudioUnitScope inScope, AudioUnitElement inElement)
    {
        if (! prepared)
            prepareToPlay();

        return JuceAUBaseClass::Reset (inScope, inElement);
    }

    void prepareToPlay()
    {
        if (juceFilter != 0)
        {
#if ! JucePlugin_IsSynth
            juceFilter->setPlayConfigDetails (GetInput(0)->GetStreamFormat().mChannelsPerFrame,
#else
            juceFilter->setPlayConfigDetails (0,
#endif
                                              GetOutput(0)->GetStreamFormat().mChannelsPerFrame,
                                              GetSampleRate(),
                                              GetMaxFramesPerSlice());

            bufferSpace.setSize (juceFilter->getNumInputChannels() + juceFilter->getNumOutputChannels(),
                                 GetMaxFramesPerSlice() + 32);

            juceFilter->prepareToPlay (GetSampleRate(),
                                       GetMaxFramesPerSlice());

            midiEvents.clear();

            juce_free (channels);
            channels = (float**) juce_calloc (sizeof (float*) * jmax (juceFilter->getNumInputChannels(),
                                                                      juceFilter->getNumOutputChannels()) + 4);

            prepared = true;
        }
    }

    ComponentResult Render (AudioUnitRenderActionFlags &ioActionFlags,
                            const AudioTimeStamp& inTimeStamp,
                            UInt32 nFrames)
    {
        lastSMPTETime = inTimeStamp.mSMPTETime;

#if ! JucePlugin_IsSynth
        return JuceAUBaseClass::Render (ioActionFlags, inTimeStamp, nFrames);
#else
        // synths can't have any inputs..
        AudioBufferList inBuffer;
        inBuffer.mNumberBuffers = 0;

        return ProcessBufferLists (ioActionFlags, inBuffer, GetOutput(0)->GetBufferList(), nFrames);
#endif
    }


    OSStatus ProcessBufferLists (AudioUnitRenderActionFlags& ioActionFlags,
                                 const AudioBufferList& inBuffer,
                                 AudioBufferList& outBuffer,
                                 UInt32 numSamples)
    {
        if (juceFilter != 0)
        {
            jassert (prepared);

            int numOutChans = 0;
            int nextSpareBufferChan = 0;
            bool needToReinterleave = false;
            const int numIn = juceFilter->getNumInputChannels();
            const int numOut = juceFilter->getNumOutputChannels();

            unsigned int i;
            for (i = 0; i < outBuffer.mNumberBuffers; ++i)
            {
                AudioBuffer& buf = outBuffer.mBuffers[i];

                if (buf.mNumberChannels == 1)
                {
                    channels [numOutChans++] = (float*) buf.mData;
                }
                else
                {
                    needToReinterleave = true;

                    for (unsigned int subChan = 0; subChan < buf.mNumberChannels && numOutChans < numOut; ++subChan)
                        channels [numOutChans++] = bufferSpace.getSampleData (nextSpareBufferChan++);
                }

                if (numOutChans >= numOut)
                    break;
            }

            int numInChans = 0;

            for (i = 0; i < inBuffer.mNumberBuffers; ++i)
            {
                const AudioBuffer& buf = inBuffer.mBuffers[i];

                if (buf.mNumberChannels == 1)
                {
                    if (numInChans < numOutChans)
                        memcpy (channels [numInChans], (const float*) buf.mData, sizeof (float) * numSamples);
                    else
                        channels [numInChans] = (float*) buf.mData;

                    ++numInChans;
                }
                else
                {
                    // need to de-interleave..
                    for (unsigned int subChan = 0; subChan < buf.mNumberChannels && numInChans < numIn; ++subChan)
                    {
                        float* dest;

                        if (numInChans < numOutChans)
                        {
                            dest = channels [numInChans++];
                        }
                        else
                        {
                            dest = bufferSpace.getSampleData (nextSpareBufferChan++);
                            channels [numInChans++] = dest;
                        }

                        const float* src = ((const float*) buf.mData) + subChan;

                        for (int j = numSamples; --j >= 0;)
                        {
                            *dest++ = *src;
                            src += buf.mNumberChannels;
                        }
                    }
                }

                if (numInChans >= numIn)
                    break;
            }

            {
                AudioSampleBuffer buffer (channels, jmax (numIn, numOut), numSamples);

                const ScopedLock sl (juceFilter->getCallbackLock());

                if (juceFilter->isSuspended())
                {
                    for (int i = 0; i < numOut; ++i)
                        zeromem (channels [i], sizeof (float) * numSamples);
                }
                else
                {
                    juceFilter->processBlock (buffer, midiEvents);
                }
            }

            if (! midiEvents.isEmpty())
            {
#if JucePlugin_ProducesMidiOutput
                const uint8* midiEventData;
                int midiEventSize, midiEventPosition;
                MidiBuffer::Iterator i (midiEvents);

                while (i.getNextEvent (midiEventData, midiEventSize, midiEventPosition))
                {
                    jassert (((unsigned int) midiEventPosition) < (unsigned int) numSamples);



                    //xxx
                }
#else
                // if your plugin creates midi messages, you'll need to set
                // the JucePlugin_ProducesMidiOutput macro to 1 in your
                // JucePluginCharacteristics.h file
                //jassert (midiEvents.getNumEvents() <= numMidiEventsComingIn);
#endif
                midiEvents.clear();
            }

            if (needToReinterleave)
            {
                nextSpareBufferChan = 0;

                for (i = 0; i < outBuffer.mNumberBuffers; ++i)
                {
                    AudioBuffer& buf = outBuffer.mBuffers[i];

                    if (buf.mNumberChannels > 1)
                    {
                        for (unsigned int subChan = 0; subChan < buf.mNumberChannels; ++subChan)
                        {
                            const float* src = bufferSpace.getSampleData (nextSpareBufferChan++);
                            float* dest = ((float*) buf.mData) + subChan;

                            for (int j = numSamples; --j >= 0;)
                            {
                                *dest = *src++;
                                dest += buf.mNumberChannels;
                            }
                        }
                    }
                }
            }

#if ! JucePlugin_SilenceInProducesSilenceOut
            ioActionFlags &= ~kAudioUnitRenderAction_OutputIsSilence;
#endif
        }

        return noErr;
    }

protected:
    OSStatus HandleMidiEvent (UInt8 nStatus,
                              UInt8 inChannel,
                              UInt8 inData1,
                              UInt8 inData2,
                              long inStartFrame)
    {
#if JucePlugin_WantsMidiInput
        uint8 data [4];
        data[0] = nStatus | inChannel;
        data[1] = inData1;
        data[2] = inData2;

        midiEvents.addEvent (data, 3, inStartFrame);
#endif

        return noErr;
    }

    //==============================================================================
private:
    AudioProcessor* juceFilter;
    AudioSampleBuffer bufferSpace;
    float** channels;
    MidiBuffer midiEvents;
    bool prepared;
    SMPTETime lastSMPTETime;
    AUChannelInfo channelInfo [numChannelConfigs];
    AudioUnitEvent auEvent;
};


//==============================================================================
class JuceAUComponentHolder  : public Component
{
public:
    JuceAUComponentHolder (Component* const editorComp)
    {
        addAndMakeVisible (editorComp);
        setOpaque (true);
        setVisible (true);
        setBroughtToFrontOnMouseClick (true);

#if ! JucePlugin_EditorRequiresKeyboardFocus
        setWantsKeyboardFocus (false);
#endif
    }

    ~JuceAUComponentHolder()
    {
    }

    void resized()
    {
        if (getNumChildComponents() > 0)
            getChildComponent (0)->setBounds (0, 0, getWidth(), getHeight());
    }

    void paint (Graphics& g)
    {
    }
};

//==============================================================================
class JuceAUView  : public AUCarbonViewBase,
                    public ComponentListener,
                    public MouseListener,
                    public Timer
{
    AudioProcessor* juceFilter;
    AudioProcessorEditor* editorComp;
    Component* windowComp;
    bool recursive;
    int mx, my;

public:
    JuceAUView (AudioUnitCarbonView auview)
      : AUCarbonViewBase (auview),
        juceFilter (0),
        editorComp (0),
        windowComp (0),
        recursive (false),
        mx (0),
        my (0)
    {
    }

    ~JuceAUView()
    {
        deleteUI();
    }

    ComponentResult CreateUI (Float32 inXOffset, Float32 inYOffset)
    {
        if (juceFilter == 0)
        {
            UInt32 propertySize = sizeof (&juceFilter);

            AudioUnitGetProperty (GetEditAudioUnit(),
                                  juceFilterObjectPropertyID,
                                  kAudioUnitScope_Global,
                                  0,
                                  &juceFilter,
                                  &propertySize);
        }

        if (juceFilter != 0)
        {
            deleteUI();

            editorComp = juceFilter->createEditorIfNeeded();

            const int w = editorComp->getWidth();
            const int h = editorComp->getHeight();

            editorComp->setOpaque (true);
            editorComp->setVisible (true);

            windowComp = new JuceAUComponentHolder (editorComp);
            windowComp->setBounds ((int) inXOffset, (int) inYOffset, w, h);

            windowComp->addToDesktop (0, (void*) mCarbonPane);
            SizeControl (mCarbonPane, w, h);

            editorComp->addComponentListener (this);
            windowComp->addMouseListener (this, true);

            startTimer (20);
        }
        else
        {
            jassertfalse // can't get a pointer to our effect
        }

        return noErr;
    }

    void componentMovedOrResized (Component& component,
                                  bool wasMoved,
                                  bool wasResized)
    {
        if (! recursive)
        {
            recursive = true;

            if (editorComp != 0 && wasResized)
            {
                const int w = jmax (32, editorComp->getWidth());
                const int h = jmax (32, editorComp->getHeight());

                SizeControl (mCarbonPane, w, h);

                if (windowComp->getWidth() != w
                     || windowComp->getHeight() != h)
                {
                    windowComp->setSize (w, h);
                }

                editorComp->repaint();
            }

            recursive = false;
        }
    }

    void timerCallback()
    {
        // for some stupid Apple-related reason, mouse move events just don't seem to get sent
        // to the windows in an AU, so we have to bodge it here and simulate them with a
        // timer..
        if (editorComp != 0)
        {
            int x, y;
            Desktop::getInstance().getMousePosition (x, y);

            if (x != mx || y != my)
            {
                mx = x;
                my = y;

                if (! ModifierKeys::getCurrentModifiers().isAnyMouseButtonDown())
                {
                    for (int i = ComponentPeer::getNumPeers(); --i >= 0;)
                    {
                        ComponentPeer* const peer = ComponentPeer::getPeer (i);

                        int rx = x, ry = y;
                        peer->getComponent()->globalPositionToRelative (rx, ry);

                        if (peer->contains (rx, ry, false) && peer->getComponent()->isShowing())
                        {
                            peer->handleMouseMove (rx, ry, Time::currentTimeMillis());
                            break;
                        }
                    }
                }
            }
        }
    }

    void mouseMove (const MouseEvent& e)
    {
        Desktop::getInstance().getMousePosition (mx, my);
        startTimer (20);
    }

private:
    void deleteUI()
    {
        PopupMenu::dismissAllActiveMenus();

        // there's some kind of component currently modal, but the host
        // is trying to delete our plugin..
        jassert (Component::getCurrentlyModalComponent() == 0);

        if (editorComp != 0)
            juceFilter->editorBeingDeleted (editorComp);

        deleteAndZero (editorComp);
        deleteAndZero (windowComp);
    }
};

//==============================================================================
#define JUCE_COMPONENT_ENTRYX(Class, Name, Suffix) \
extern "C" __attribute__((visibility("default"))) ComponentResult Name ## Suffix (ComponentParameters* params, Class* obj); \
extern "C" __attribute__((visibility("default"))) ComponentResult Name ## Suffix (ComponentParameters* params, Class* obj) \
{ \
    return ComponentEntryPoint<Class>::Dispatch(params, obj); \
}

#define JUCE_COMPONENT_ENTRY(Class, Name, Suffix) JUCE_COMPONENT_ENTRYX(Class, Name, Suffix)

JUCE_COMPONENT_ENTRY (JuceAU, JucePlugin_AUExportPrefix, Entry)
JUCE_COMPONENT_ENTRY (JuceAUView, JucePlugin_AUExportPrefix, ViewEntry)

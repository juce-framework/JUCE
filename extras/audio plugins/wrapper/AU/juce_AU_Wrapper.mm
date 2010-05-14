/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-10 by Raw Material Software Ltd.

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

#include <Cocoa/Cocoa.h>
#include <AudioUnit/AUCocoaUIView.h>
#include <AudioUnit/AudioUnit.h>
#include <AudioToolbox/AudioUnitUtilities.h>
#include "AUMIDIEffectBase.h"
#include "MusicDeviceBase.h"
#include "../juce_IncludeCharacteristics.h"

#if JucePlugin_Build_AU

/** The BUILD_AU_CARBON_UI flag lets you specify whether old-school carbon hosts are supported as
    well as ones that can open a cocoa view. If this is enabled, you'll need to also add the AUCarbonBase
    files to your project.
*/
#ifndef BUILD_AU_CARBON_UI
 #define BUILD_AU_CARBON_UI 1
#endif

#if BUILD_AU_CARBON_UI
 #undef Button
 #include "AUCarbonViewBase.h"
 class JuceAUView;
#endif

#include "../juce_PluginHeaders.h"
#include "../juce_PluginHostType.h"

//==============================================================================
#define juceFilterObjectPropertyID 0x1a45ffe9
static Array<void*> activePlugins, activeUIs;

static const short channelConfigs[][2] = { JucePlugin_PreferredChannelConfigurations };
static const int numChannelConfigs = sizeof (channelConfigs) / sizeof (*channelConfigs);

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
#define appendMacro1(a, b, c, d) a ## _ ## b ## _ ## c ## _ ## d
#define appendMacro2(a, b, c, d) appendMacro1(a, b, c, d)
#define MakeObjCClassName(rootName)  appendMacro2 (rootName, JUCE_MAJOR_VERSION, JUCE_MINOR_VERSION, JucePlugin_AUExportPrefix)

#define JuceUICreationClass         JucePlugin_AUCocoaViewClassName
#define JuceUIViewClass             MakeObjCClassName(JuceUIViewClass)

class JuceAU;
class EditorCompHolder;

//==============================================================================
@interface JuceUICreationClass   : NSObject <AUCocoaUIBase>
{
}

- (JuceUICreationClass*) init;
- (void) dealloc;
- (unsigned) interfaceVersion;
- (NSString *) description;
- (NSView*) uiViewForAudioUnit: (AudioUnit) inAudioUnit
                      withSize: (NSSize) inPreferredSize;
@end

//==============================================================================
@interface JuceUIViewClass : NSView
{
    AudioProcessor* filter;
    JuceAU* au;
    EditorCompHolder* editorComp;
}

- (JuceUIViewClass*) initWithFilter: (AudioProcessor*) filter
                             withAU: (JuceAU*) au
                      withComponent: (AudioProcessorEditor*) editorComp;
- (void) dealloc;
- (void) viewDidMoveToWindow;
- (BOOL) mouseDownCanMoveWindow;
- (void) filterBeingDeleted: (JuceAU*) au_;
- (void) deleteEditor;

@end


//==============================================================================
class JuceAU   : public JuceAUBaseClass,
                 public AudioProcessorListener,
                 public AudioPlayHead,
                 public ComponentListener
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
          prepared (false)
    {
        if (activePlugins.size() + activeUIs.size() == 0)
        {
#if BUILD_AU_CARBON_UI
            NSApplicationLoad();
#endif

            initialiseJuce_GUI();
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
        for (int i = activeUIs.size(); --i >= 0;)
            [((JuceUIViewClass*) activeUIs.getUnchecked(i)) filterBeingDeleted: this];

        delete juceFilter;
        juceFilter = 0;

        jassert (activePlugins.contains (this));
        activePlugins.removeValue (this);

        if (activePlugins.size() + activeUIs.size() == 0)
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
                outDataSize = sizeof (void*) * 2;
                return noErr;
            }
            else if (inID == kAudioUnitProperty_OfflineRender)
            {
                outWritable = true;
                outDataSize = sizeof (UInt32);
                return noErr;
            }
            else if (inID == kMusicDeviceProperty_InstrumentCount)
            {
                outDataSize = sizeof (UInt32);
                outWritable = false;
                return noErr;
            }
            else if (inID == kAudioUnitProperty_CocoaUI)
            {
#if MAC_OS_X_VERSION_MIN_REQUIRED < MAC_OS_X_VERSION_10_5
                // (On 10.4, there's a random obj-c dispatching crash when trying to load a cocoa UI)
                if (PlatformUtilities::getOSXMinorVersionNumber() > 4)
#endif
                {
                    outDataSize = sizeof (AudioUnitCocoaViewInfo);
                    outWritable = true;
                    return noErr;
                }
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
                ((void**) outData)[0] = (void*) juceFilter;
                ((void**) outData)[1] = (void*) this;
                return noErr;
            }
            else if (inID == kAudioUnitProperty_OfflineRender)
            {
                *(UInt32*) outData = (juceFilter != 0 && juceFilter->isNonRealtime()) ? 1 : 0;
                return noErr;
            }
            else if (inID == kMusicDeviceProperty_InstrumentCount)
            {
                *(UInt32*) outData = 1;
                return noErr;
            }
            else if (inID == kAudioUnitProperty_CocoaUI)
            {
#if MAC_OS_X_VERSION_MIN_REQUIRED < MAC_OS_X_VERSION_10_5
                // (On 10.4, there's a random obj-c dispatching crash when trying to load a cocoa UI)
                if (PlatformUtilities::getOSXMinorVersionNumber() > 4)
#endif
                {
                    const ScopedAutoReleasePool pool;

                    AudioUnitCocoaViewInfo* info = (AudioUnitCocoaViewInfo*) outData;

                    const File bundleFile (File::getSpecialLocation (File::currentApplicationFile));
                    NSString* bundlePath = [NSString stringWithUTF8String: (const char*) bundleFile.getFullPathName().toUTF8()];
                    NSBundle* b = [NSBundle bundleWithPath: bundlePath];

                    info->mCocoaAUViewClass[0] = (CFStringRef) [[[JuceUICreationClass class] className] retain];
                    info->mCocoaAUViewBundleLocation = (CFURLRef) [[NSURL fileURLWithPath: [b bundlePath]] retain];

                    return noErr;
                }
            }
        }

        return JuceAUBaseClass::GetProperty (inID, inScope, inElement, outData);
    }

    ComponentResult SetProperty (AudioUnitPropertyID inID,
                                 AudioUnitScope inScope,
                                 AudioUnitElement inElement,
                                 const void* inData,
                                 UInt32 inDataSize)
    {
        if (inScope == kAudioUnitScope_Global && inID == kAudioUnitProperty_OfflineRender)
        {
            if (juceFilter != 0)
                juceFilter->setNonRealtime ((*(UInt32*) inData) != 0);

            return noErr;
        }

        return JuceAUBaseClass::SetProperty (inID, inScope, inElement, inData, inDataSize);
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
                CFDataRef ourState = CFDataCreate (kCFAllocatorDefault, (const UInt8*) state.getData(), state.getSize());
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
                    const JUCE_NAMESPACE::uint8* const rawBytes = CFDataGetBytePtr (data);

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

            if (juceFilter->isMetaParameter (index))
                outParameterInfo.flags |= kAudioUnitParameterFlag_IsGlobalMeta;

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
    Float64 GetTailTime()                       { return (JucePlugin_TailLengthSeconds); }

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
#if BUILD_AU_CARBON_UI
    int GetNumCustomUIComponents()              { return 1; }

    void GetUIComponentDescs (ComponentDescription* inDescArray)
    {
        inDescArray[0].componentType = kAudioUnitCarbonViewComponentType;
        inDescArray[0].componentSubType = JucePlugin_AUSubType;
        inDescArray[0].componentManufacturer = JucePlugin_AUManufacturerCode;
        inDescArray[0].componentFlags = 0;
        inDescArray[0].componentFlagsMask = 0;
    }
#endif

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

    void sendAUEvent (const AudioUnitEventType type, const int index)
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
        incomingEvents.clear();
        prepared = false;
    }

    ComponentResult Reset (AudioUnitScope inScope, AudioUnitElement inElement)
    {
        if (! prepared)
            prepareToPlay();

        if (juceFilter != 0)
            juceFilter->reset();

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

            midiEvents.ensureSize (2048);
            midiEvents.clear();
            incomingEvents.ensureSize (2048);
            incomingEvents.clear();

            channels.calloc (jmax (juceFilter->getNumInputChannels(),
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
                const ScopedLock sl (incomingMidiLock);
                midiEvents.clear();
                incomingEvents.swapWith (midiEvents);
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
                const JUCE_NAMESPACE::uint8* midiEventData;
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
#if defined(MAC_OS_X_VERSION_10_5)
                              UInt32 inStartFrame)
#else
                              long inStartFrame)
#endif
    {
#if JucePlugin_WantsMidiInput
        const ScopedLock sl (incomingMidiLock);
        JUCE_NAMESPACE::uint8 data [4];
        data[0] = nStatus | inChannel;
        data[1] = inData1;
        data[2] = inData2;

        incomingEvents.addEvent (data, 3, inStartFrame);
#endif

        return noErr;
    }

    OSStatus HandleSysEx (const UInt8* inData, UInt32 inLength)
    {
#if JucePlugin_WantsMidiInput
        const ScopedLock sl (incomingMidiLock);
        incomingEvents.addEvent (inData, inLength, 0);
#endif
        return noErr;
    }

    //==============================================================================
    ComponentResult GetPresets (CFArrayRef* outData) const
    {
        if (outData != 0)
        {
            const int numPrograms = juceFilter->getNumPrograms();
            presetsArray.ensureSize (sizeof (AUPreset) * numPrograms, true);
            AUPreset* const presets = (AUPreset*) presetsArray.getData();

            CFMutableArrayRef presetsArray = CFArrayCreateMutable (0, numPrograms, 0);

            for (int i = 0; i < numPrograms; ++i)
            {
                presets[i].presetNumber = i;
                presets[i].presetName = PlatformUtilities::juceStringToCFString (juceFilter->getProgramName (i));

                CFArrayAppendValue (presetsArray, presets + i);
            }

            *outData = (CFArrayRef) presetsArray;
        }

        return noErr;
    }

    OSStatus NewFactoryPresetSet (const AUPreset& inNewFactoryPreset)
    {
        const int numPrograms = juceFilter->getNumPrograms();
        const SInt32 chosenPresetNumber = (int) inNewFactoryPreset.presetNumber;

        if (chosenPresetNumber >= numPrograms)
            return kAudioUnitErr_InvalidProperty;

        AUPreset chosenPreset;
        chosenPreset.presetNumber = chosenPresetNumber;
        chosenPreset.presetName = PlatformUtilities::juceStringToCFString (juceFilter->getProgramName (chosenPresetNumber));

        juceFilter->setCurrentProgram (chosenPresetNumber);
        SetAFactoryPresetAsCurrent (chosenPreset);

        return noErr;
    }

    void componentMovedOrResized (Component& component, bool wasMoved, bool wasResized)
    {
        //if (wasResized)
        {
            NSView* view = (NSView*) component.getWindowHandle();
            NSRect r = [[view superview] frame];
            r.origin.y = r.origin.y + r.size.height - component.getHeight();
            r.size.width = component.getWidth();
            r.size.height = component.getHeight();
            [[view superview] setFrame: r];
            [view setFrame: NSMakeRect (0, 0, component.getWidth(), component.getHeight())];
            [view setNeedsDisplay: YES];
        }
    }

    //==============================================================================
private:
    AudioProcessor* juceFilter;
    AudioSampleBuffer bufferSpace;
    HeapBlock <float*> channels;
    MidiBuffer midiEvents, incomingEvents;
    bool prepared;
    SMPTETime lastSMPTETime;
    AUChannelInfo channelInfo [numChannelConfigs];
    AudioUnitEvent auEvent;
    mutable MemoryBlock presetsArray;
    CriticalSection incomingMidiLock;
};

//==============================================================================
class EditorCompHolder : public Component,
                         public ComponentListener
{
public:
    EditorCompHolder (AudioProcessorEditor* const editorComp)
    {
        setSize (editorComp->getWidth(), editorComp->getHeight());
        addAndMakeVisible (editorComp);
        editorComp->addComponentListener (this);

#if ! JucePlugin_EditorRequiresKeyboardFocus
        setWantsKeyboardFocus (false);
#else
        setWantsKeyboardFocus (true);
#endif
    }

    ~EditorCompHolder()
    {
        deleteAllChildren();
    }

    void componentParentHierarchyChanged (Component& component)
    {
        if (component.getParentComponent() != this)
            component.removeComponentListener (this);
    }

    void componentMovedOrResized (Component& component, bool wasMoved, bool wasResized)
    {
        Component* comp = getChildComponent(0);

        if (comp != 0 && wasResized)
        {
            const int w = jmax (32, comp->getWidth());
            const int h = jmax (32, comp->getHeight());

            if (getWidth() != w || getHeight() != h)
                setSize (w, h);

            NSView* view = (NSView*) getWindowHandle();
            NSRect r = [[view superview] frame];
            r.size.width = component.getWidth();
            r.size.height = component.getHeight();
            [[view superview] setFrame: r];
            [view setFrame: NSMakeRect (0, 0, component.getWidth(), component.getHeight())];
            [view setNeedsDisplay: YES];
        }
    }
};

//==============================================================================
@implementation JuceUIViewClass

- (JuceUIViewClass*) initWithFilter: (AudioProcessor*) filter_
                             withAU: (JuceAU*) au_
                      withComponent: (AudioProcessorEditor*) editorComp_
{
    filter = filter_;
    au = au_;
    editorComp = new EditorCompHolder (editorComp_);

    [super initWithFrame: NSMakeRect (0, 0, editorComp_->getWidth(), editorComp_->getHeight())];
    [self setHidden: NO];
    [self setPostsFrameChangedNotifications: YES];

    activeUIs.add (self);

    editorComp->addToDesktop (0, (void*) self);
    editorComp->setVisible (true);

    return self;
}

- (void) dealloc
{
    // there's some kind of component currently modal, but the host
    // is trying to delete our plugin..
    jassert (Component::getCurrentlyModalComponent() == 0);

    [self deleteEditor];

    jassert (activeUIs.contains (self));
    activeUIs.removeValue (self);
    if (activePlugins.size() + activeUIs.size() == 0)
        shutdownJuce_GUI();

    [super dealloc];
}

- (void) viewDidMoveToWindow
{
    if ([self window] != 0)
    {
        [[self window] setAcceptsMouseMovedEvents: YES];

        if (editorComp != 0)
            [[self window] makeFirstResponder: (NSView*) editorComp->getWindowHandle()];
    }
}

- (BOOL) mouseDownCanMoveWindow
{
    return NO;
}

- (void) deleteEditor
{
    if (editorComp != 0 && editorComp->isValidComponent())
    {
        if (editorComp->getChildComponent(0) != 0)
            if (activePlugins.contains ((void*) au)) // plugin may have been deleted before the UI
                filter->editorBeingDeleted ((AudioProcessorEditor*) editorComp->getChildComponent(0));

        deleteAndZero (editorComp);
    }

    editorComp = 0;
}

- (void) filterBeingDeleted: (JuceAU*) au_
{
    if (au_ == au)
        [self deleteEditor];
}

@end

//==============================================================================
@implementation JuceUICreationClass

- (JuceUICreationClass*) init
{
    return [super init];
}

- (void) dealloc
{
    [super dealloc];
}

- (unsigned) interfaceVersion
{
    return 0;
}

- (NSString*) description
{
    return [NSString stringWithString: @JucePlugin_Name];
}

- (NSView*) uiViewForAudioUnit: (AudioUnit) inAudioUnit
                      withSize: (NSSize) inPreferredSize
{
    void* pointers[2];
    UInt32 propertySize = sizeof (pointers);

    if (AudioUnitGetProperty (inAudioUnit,
                              juceFilterObjectPropertyID,
                              kAudioUnitScope_Global,
                              0,
                              pointers,
                              &propertySize) != noErr)
        return 0;

    AudioProcessor* filter = (AudioProcessor*) pointers[0];
    JuceAU* au = (JuceAU*) pointers[1];

    if (filter == 0)
        return 0;

    AudioProcessorEditor* editorComp = filter->createEditorIfNeeded();

    if (editorComp == 0)
        return 0;

    return [[[JuceUIViewClass alloc] initWithFilter: filter
                                             withAU: au
                                      withComponent: editorComp] autorelease];
}
@end


#if BUILD_AU_CARBON_UI

//==============================================================================
class JuceAUView  : public AUCarbonViewBase
{
    AudioProcessor* juceFilter;
    Component* windowComp;

public:
    JuceAUView (AudioUnitCarbonView auview)
      : AUCarbonViewBase (auview),
        juceFilter (0),
        windowComp (0)
    {
    }

    ~JuceAUView()
    {
        deleteUI();
    }

    ComponentResult CreateUI (Float32 /*inXOffset*/, Float32 /*inYOffset*/)
    {
        const ScopedAutoReleasePool pool;

        if (juceFilter == 0)
        {
            void* pointers[2];
            UInt32 propertySize = sizeof (pointers);

            AudioUnitGetProperty (GetEditAudioUnit(),
                                  juceFilterObjectPropertyID,
                                  kAudioUnitScope_Global,
                                  0,
                                  pointers,
                                  &propertySize);

            juceFilter = (AudioProcessor*) pointers[0];
        }

        if (juceFilter != 0)
        {
            deleteUI();

            AudioProcessorEditor* editorComp = juceFilter->createEditorIfNeeded();
            editorComp->setOpaque (true);
            windowComp = new ComponentInHIView (editorComp, mCarbonPane);
        }
        else
        {
            jassertfalse // can't get a pointer to our effect
        }

        return noErr;
    }

    AudioUnitCarbonViewEventListener getEventListener() const   { return mEventListener; }
    void* getEventListenerUserData() const                      { return mEventListenerUserData; }

private:
    void deleteUI()
    {
        if (windowComp != 0)
        {
            PopupMenu::dismissAllActiveMenus();

            // there's some kind of component currently modal, but the host
            // is trying to delete our plugin..
            jassert (Component::getCurrentlyModalComponent() == 0);

            if (windowComp != 0 && windowComp->getChildComponent(0) != 0)
                juceFilter->editorBeingDeleted ((AudioProcessorEditor*) windowComp->getChildComponent(0));

            deleteAndZero (windowComp);
        }
    }

    //==============================================================================
    // Uses a child NSWindow to sit in front of a HIView and display our component
    class ComponentInHIView  : public Component,
                               public ComponentListener
    {
    public:
        //==============================================================================
        ComponentInHIView (Component* const contentComp, HIViewRef parentHIView)
            : parentView (parentHIView),
              recursive (false)
        {
            const ScopedAutoReleasePool pool;

            jassert (contentComp != 0);
            addAndMakeVisible (contentComp);
            setOpaque (true);
            setVisible (true);
            setBroughtToFrontOnMouseClick (true);

            setSize (contentComp->getWidth(), contentComp->getHeight());
            SizeControl (parentHIView, contentComp->getWidth(), contentComp->getHeight());

            WindowRef windowRef = HIViewGetWindow (parentHIView);
            hostWindow = [[NSWindow alloc] initWithWindowRef: windowRef];

            [hostWindow retain];
            [hostWindow setCanHide: YES];
            [hostWindow setReleasedWhenClosed: YES];

            updateWindowPos();

#if ! JucePlugin_EditorRequiresKeyboardFocus
            addToDesktop (ComponentPeer::windowIsTemporary | ComponentPeer::windowIgnoresKeyPresses);
            setWantsKeyboardFocus (false);
#else
            addToDesktop (ComponentPeer::windowIsTemporary);
            setWantsKeyboardFocus (true);
#endif

            setVisible (true);
            toFront (false);

            addSubWindow();

            NSWindow* pluginWindow = [((NSView*) getWindowHandle()) window];
            [pluginWindow setNextResponder: hostWindow];

            // Adds a callback bodge to work around some problems with wrapped
            // carbon windows..
            const EventTypeSpec eventsToCatch[] = {
                { kEventClassWindow, kEventWindowShown },
                { kEventClassWindow, kEventWindowHidden }
            };

            InstallWindowEventHandler ((WindowRef) windowRef,
                                       NewEventHandlerUPP (windowVisibilityBodge),
                                       GetEventTypeCount (eventsToCatch), eventsToCatch,
                                       (void*) hostWindow, 0);

            contentComp->addComponentListener (this);
        }

        ~ComponentInHIView()
        {
            Component* const comp = getChildComponent(0);

            if (comp != 0)
                comp->removeComponentListener (this);

            const ScopedAutoReleasePool pool;

            NSWindow* pluginWindow = [((NSView*) getWindowHandle()) window];
            [hostWindow removeChildWindow: pluginWindow];
            removeFromDesktop();

            [hostWindow release];
            hostWindow = 0;

            deleteAllChildren();
        }

        void updateWindowPos()
        {
            HIPoint f;
            f.x = f.y = 0;
            HIPointConvert (&f, kHICoordSpaceView, parentView, kHICoordSpaceScreenPixel, 0);
            setTopLeftPosition ((int) f.x, (int) f.y);
        }

        void addSubWindow()
        {
            NSWindow* pluginWindow = [((NSView*) getWindowHandle()) window];
            [pluginWindow setExcludedFromWindowsMenu: YES];
            [pluginWindow setCanHide: YES];

            [hostWindow addChildWindow: pluginWindow
                               ordered: NSWindowAbove];
            [hostWindow orderFront: nil];
            [pluginWindow orderFront: nil];
        }

        void resized()
        {
            if (getChildComponent(0) != 0)
                getChildComponent(0)->setBounds (0, 0, getWidth(), getHeight());
        }

        void paint (Graphics& g) {}

        void componentParentHierarchyChanged (Component& component)
        {
            if (component.getParentComponent() != this)
                component.removeComponentListener (this);
        }

        void componentMovedOrResized (Component& component, bool wasMoved, bool wasResized)
        {
            if (! recursive)
            {
                recursive = true;

                Component* comp = getChildComponent(0);

                if (comp != 0 && wasResized)
                {
                    const int w = jmax (32, comp->getWidth());
                    const int h = jmax (32, comp->getHeight());

                    SizeControl (parentView, w, h);

                    if (getWidth() != w || getHeight() != h)
                        setSize (w, h);

                    comp->repaint();

                    updateWindowPos();
                    addSubWindow(); // (need this for AULab)
                }

                recursive = false;
            }
        }

        bool keyPressed (const KeyPress& kp)
        {
            if (! kp.getModifiers().isCommandDown())
            {
                // If we have an unused keypress, move the key-focus to a host window
                // and re-inject the event..
                static NSTimeInterval lastEventTime = 0; // check we're not recursively sending the same event

                if (lastEventTime != [[NSApp currentEvent] timestamp])
                {
                    lastEventTime = [[NSApp currentEvent] timestamp];
                    [[hostWindow parentWindow] makeKeyWindow];
                    [NSApp postEvent: [NSApp currentEvent] atStart: YES];
                }
            }

            return false;
        }

    private:
        HIViewRef parentView;
        NSWindow* hostWindow;
        bool recursive;

        /* When you wrap a WindowRef as an NSWindow, it seems to bugger up the HideWindow
           function, so when the host tries (and fails) to hide the window, this catches
           the event and does the job properly.
        */
        static pascal OSStatus windowVisibilityBodge (EventHandlerCallRef, EventRef e, void* user)
        {
            NSWindow* hostWindow = (NSWindow*) user;

            switch (GetEventKind (e))
            {
            case kEventWindowShown:
                [hostWindow orderFront: nil];
                break;
            case kEventWindowHidden:
                [hostWindow orderOut: nil];
                break;
            }

            return eventNotHandledErr;
        }
    };
};

#endif

//==============================================================================
#define JUCE_COMPONENT_ENTRYX(Class, Name, Suffix) \
extern "C" __attribute__((visibility("default"))) ComponentResult Name ## Suffix (ComponentParameters* params, Class* obj); \
extern "C" __attribute__((visibility("default"))) ComponentResult Name ## Suffix (ComponentParameters* params, Class* obj) \
{ \
    return ComponentEntryPoint<Class>::Dispatch(params, obj); \
}

#define JUCE_COMPONENT_ENTRY(Class, Name, Suffix) JUCE_COMPONENT_ENTRYX(Class, Name, Suffix)

JUCE_COMPONENT_ENTRY (JuceAU, JucePlugin_AUExportPrefix, Entry)

#if BUILD_AU_CARBON_UI
  JUCE_COMPONENT_ENTRY (JuceAUView, JucePlugin_AUExportPrefix, ViewEntry)
#endif

#endif

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

#if JucePlugin_Build_AU

#if __LP64__
 #undef JUCE_SUPPORT_CARBON
 #define JUCE_SUPPORT_CARBON 0
#endif

#ifdef __clang__
 #pragma clang diagnostic push
 #pragma clang diagnostic ignored "-Wshorten-64-to-32"
 #pragma clang diagnostic ignored "-Wunused-parameter"
 #pragma clang diagnostic ignored "-Wdeprecated-declarations"
 #pragma clang diagnostic ignored "-Wsign-conversion"
 #pragma clang diagnostic ignored "-Wconversion"
 #pragma clang diagnostic ignored "-Woverloaded-virtual"
#endif

#include "../utility/juce_IncludeSystemHeaders.h"

#include <AudioUnit/AUCocoaUIView.h>
#include <AudioUnit/AudioUnit.h>
#include <AudioToolbox/AudioUnitUtilities.h>
#include <CoreMIDI/MIDIServices.h>

#if JUCE_SUPPORT_CARBON
 #define Point CarbonDummyPointName
 #define Component CarbonDummyCompName
#endif

/*
    Got an include error here?

    You probably need to install Apple's AU classes - see the
    juce website for more info on how to get them:
    http://www.juce.com/forum/topic/aus-xcode
*/
#include "AUMIDIEffectBase.h"
#include "MusicDeviceBase.h"
#undef Point
#undef Component

/** The BUILD_AU_CARBON_UI flag lets you specify whether old-school carbon hosts are supported as
    well as ones that can open a cocoa view. If this is enabled, you'll need to also add the AUCarbonBase
    files to your project.
*/
#if ! (defined (BUILD_AU_CARBON_UI) || JUCE_64BIT)
 #define BUILD_AU_CARBON_UI 1
#endif

#ifdef __LP64__
 #undef BUILD_AU_CARBON_UI  // (not possible in a 64-bit build)
#endif

#if BUILD_AU_CARBON_UI
 #undef Button
 #define Point CarbonDummyPointName
 #include "AUCarbonViewBase.h"
 #undef Point
#endif

#ifdef __clang__
 #pragma clang diagnostic pop
#endif

#define JUCE_MAC_WINDOW_VISIBITY_BODGE 1

#include "../utility/juce_IncludeModuleHeaders.h"
#include "../utility/juce_FakeMouseMoveGenerator.h"
#include "../utility/juce_CarbonVisibility.h"
#include "../../juce_core/native/juce_osx_ObjCHelpers.h"

//==============================================================================
static Array<void*> activePlugins, activeUIs;

static const AudioUnitPropertyID juceFilterObjectPropertyID = 0x1a45ffe9;

static const short channelConfigs[][2] = { JucePlugin_PreferredChannelConfigurations };
static const int numChannelConfigs = sizeof (channelConfigs) / sizeof (*channelConfigs);

#if JucePlugin_IsSynth
class JuceAUBaseClass   : public MusicDeviceBase
{
public:
    JuceAUBaseClass (AudioComponentInstance comp)  : MusicDeviceBase (comp, 0, 1) {}
};
#else
class JuceAUBaseClass   : public AUMIDIEffectBase
{
public:
    JuceAUBaseClass (AudioComponentInstance comp)  : AUMIDIEffectBase (comp, false) {}

    OSStatus MIDIEvent (UInt32 inStatus, UInt32 inData1, UInt32 inData2, UInt32 inOffsetSampleFrame)
    {
        return AUMIDIBase::MIDIEvent (inStatus, inData1, inData2, inOffsetSampleFrame);
    }

    OSStatus SysEx (const UInt8* inData, UInt32 inLength)
    {
        return AUMIDIBase::SysEx (inData, inLength);
    }
};
#endif


// This macro can be set if you need to override this internal name for some reason..
#ifndef JUCE_STATE_DICTIONARY_KEY
 #define JUCE_STATE_DICTIONARY_KEY   CFSTR("jucePluginState")
#endif

//==============================================================================
class JuceAU   : public JuceAUBaseClass,
                 public AudioProcessorListener,
                 public AudioPlayHead,
                 public ComponentListener
{
public:
    //==============================================================================
    JuceAU (AudioUnit component)
        : JuceAUBaseClass (component),
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

        juceFilter = createPluginFilterOfType (AudioProcessor::wrapperType_AudioUnit);

        juceFilter->setPlayHead (this);
        juceFilter->addListener (this);

        Globals()->UseIndexedParameters (juceFilter->getNumParameters());

        activePlugins.add (this);

        zerostruct (auEvent);
        auEvent.mArgument.mParameter.mAudioUnit = GetComponentInstance();
        auEvent.mArgument.mParameter.mScope = kAudioUnitScope_Global;
        auEvent.mArgument.mParameter.mElement = 0;

        zerostruct (midiCallback);

        CreateElements();

        CAStreamBasicDescription streamDescription;
        streamDescription.mSampleRate = getSampleRate();
        streamDescription.SetCanonical ((UInt32) channelConfigs[0][1], false);
        Outputs().GetIOElement(0)->SetStreamFormat (streamDescription);

       #if ! JucePlugin_IsSynth
        streamDescription.SetCanonical ((UInt32) channelConfigs[0][0], false);
        Inputs().GetIOElement(0)->SetStreamFormat (streamDescription);
       #endif
    }

    ~JuceAU()
    {
        deleteActiveEditors();
        juceFilter = nullptr;
        clearPresetsArray();

        jassert (activePlugins.contains (this));
        activePlugins.removeFirstMatchingValue (this);

        if (activePlugins.size() + activeUIs.size() == 0)
            shutdownJuce_GUI();
    }

    void deleteActiveEditors()
    {
        for (int i = activeUIs.size(); --i >= 0;)
        {
            id ui = (id) activeUIs.getUnchecked(i);

            if (JuceUIViewClass::getAU (ui) == this)
                JuceUIViewClass::deleteEditor (ui);
        }
    }

    //==============================================================================
    ComponentResult GetPropertyInfo (AudioUnitPropertyID inID,
                                     AudioUnitScope inScope,
                                     AudioUnitElement inElement,
                                     UInt32& outDataSize,
                                     Boolean& outWritable) override
    {
        if (inScope == kAudioUnitScope_Global)
        {
            switch (inID)
            {
                case juceFilterObjectPropertyID:
                    outWritable = false;
                    outDataSize = sizeof (void*) * 2;
                    return noErr;

                case kAudioUnitProperty_OfflineRender:
                    outWritable = true;
                    outDataSize = sizeof (UInt32);
                    return noErr;

                case kMusicDeviceProperty_InstrumentCount:
                    outDataSize = sizeof (UInt32);
                    outWritable = false;
                    return noErr;

                case kAudioUnitProperty_CocoaUI:
                   #if MAC_OS_X_VERSION_MIN_REQUIRED < MAC_OS_X_VERSION_10_5
                    // (On 10.4, there's a random obj-c dispatching crash when trying to load a cocoa UI)
                    if (SystemStats::getOperatingSystemType() >= SystemStats::MacOSX_10_5)
                   #endif
                    {
                        outDataSize = sizeof (AudioUnitCocoaViewInfo);
                        outWritable = true;
                        return noErr;
                    }

                    break;

               #if JucePlugin_ProducesMidiOutput
                case kAudioUnitProperty_MIDIOutputCallbackInfo:
                    outDataSize = sizeof (CFArrayRef);
                    outWritable = false;
                    return noErr;

                case kAudioUnitProperty_MIDIOutputCallback:
                    outDataSize = sizeof (AUMIDIOutputCallbackStruct);
                    outWritable = true;
                    return noErr;
               #endif

                case kAudioUnitProperty_ParameterStringFromValue:
                     outDataSize = sizeof (AudioUnitParameterStringFromValue);
                     outWritable = false;
                     return noErr;

                case kAudioUnitProperty_ParameterValueFromString:
                     outDataSize = sizeof (AudioUnitParameterValueFromString);
                     outWritable = false;
                     return noErr;

                default: break;
            }
        }

        return JuceAUBaseClass::GetPropertyInfo (inID, inScope, inElement, outDataSize, outWritable);
    }

    ComponentResult GetProperty (AudioUnitPropertyID inID,
                                 AudioUnitScope inScope,
                                 AudioUnitElement inElement,
                                 void* outData) override
    {
        if (inScope == kAudioUnitScope_Global)
        {
            switch (inID)
            {
                case juceFilterObjectPropertyID:
                    ((void**) outData)[0] = (void*) static_cast<AudioProcessor*> (juceFilter);
                    ((void**) outData)[1] = (void*) this;
                    return noErr;

                case kAudioUnitProperty_OfflineRender:
                    *(UInt32*) outData = (juceFilter != nullptr && juceFilter->isNonRealtime()) ? 1 : 0;
                    return noErr;

                case kMusicDeviceProperty_InstrumentCount:
                    *(UInt32*) outData = 1;
                    return noErr;

                case kAudioUnitProperty_CocoaUI:
                   #if MAC_OS_X_VERSION_MIN_REQUIRED < MAC_OS_X_VERSION_10_5
                    // (On 10.4, there's a random obj-c dispatching crash when trying to load a cocoa UI)
                    if (SystemStats::getOperatingSystemType() >= SystemStats::MacOSX_10_5)
                   #endif
                    {
                        JUCE_AUTORELEASEPOOL
                        {
                            static JuceUICreationClass cls;

                            // (NB: this may be the host's bundle, not necessarily the component's)
                            NSBundle* bundle = [NSBundle bundleForClass: cls.cls];

                            AudioUnitCocoaViewInfo* info = static_cast<AudioUnitCocoaViewInfo*> (outData);
                            info->mCocoaAUViewClass[0] = (CFStringRef) [juceStringToNS (class_getName (cls.cls)) retain];
                            info->mCocoaAUViewBundleLocation = (CFURLRef) [[NSURL fileURLWithPath: [bundle bundlePath]] retain];
                        }

                        return noErr;
                    }

                    break;

               #if JucePlugin_ProducesMidiOutput
                case kAudioUnitProperty_MIDIOutputCallbackInfo:
                {
                    CFStringRef strs[1];
                    strs[0] = CFSTR ("MIDI Callback");

                    CFArrayRef callbackArray = CFArrayCreate (nullptr, (const void**) strs, 1, &kCFTypeArrayCallBacks);
                    *(CFArrayRef*) outData = callbackArray;
                    return noErr;
                }
               #endif

                case kAudioUnitProperty_ParameterValueFromString:
                {
                    if (AudioUnitParameterValueFromString* pv = (AudioUnitParameterValueFromString*) outData)
                    {
                        if (juceFilter != nullptr)
                        {
                            const String text (String::fromCFString (pv->inString));

                            if (AudioProcessorParameter* param = juceFilter->getParameters() [(int) pv->inParamID])
                                pv->outValue = param->getValueForText (text);
                            else
                                pv->outValue = text.getFloatValue();

                            return noErr;
                        }
                    }
                }
                break;

                case kAudioUnitProperty_ParameterStringFromValue:
                {
                    if (AudioUnitParameterStringFromValue* pv = (AudioUnitParameterStringFromValue*) outData)
                    {
                        if (juceFilter != nullptr)
                        {
                            const float value = (float) *(pv->inValue);
                            String text;

                            if (AudioProcessorParameter* param = juceFilter->getParameters() [(int) pv->inParamID])
                                text = param->getText ((float) *(pv->inValue), 0);
                            else
                                text = String (value);

                            pv->outString = text.toCFString();
                            return noErr;
                        }
                    }
                }
                break;

                default:
                    break;
            }
        }

        return JuceAUBaseClass::GetProperty (inID, inScope, inElement, outData);
    }

    ComponentResult SetProperty (AudioUnitPropertyID inID,
                                 AudioUnitScope inScope,
                                 AudioUnitElement inElement,
                                 const void* inData,
                                 UInt32 inDataSize) override
    {
        if (inScope == kAudioUnitScope_Global)
        {
            switch (inID)
            {
               #if JucePlugin_ProducesMidiOutput
                case kAudioUnitProperty_MIDIOutputCallback:
                    if (inDataSize < sizeof (AUMIDIOutputCallbackStruct))
                        return kAudioUnitErr_InvalidPropertyValue;

                    if (AUMIDIOutputCallbackStruct* callbackStruct = (AUMIDIOutputCallbackStruct*) inData)
                        midiCallback = *callbackStruct;

                    return noErr;
               #endif

                case kAudioUnitProperty_OfflineRender:
                    if (juceFilter != nullptr)
                        juceFilter->setNonRealtime ((*(UInt32*) inData) != 0);

                    return noErr;

                default: break;
            }
        }

        return JuceAUBaseClass::SetProperty (inID, inScope, inElement, inData, inDataSize);
    }

    ComponentResult SaveState (CFPropertyListRef* outData) override
    {
        ComponentResult err = JuceAUBaseClass::SaveState (outData);

        if (err != noErr)
            return err;

        jassert (CFGetTypeID (*outData) == CFDictionaryGetTypeID());

        CFMutableDictionaryRef dict = (CFMutableDictionaryRef) *outData;

        if (juceFilter != nullptr)
        {
            juce::MemoryBlock state;
            juceFilter->getCurrentProgramStateInformation (state);

            if (state.getSize() > 0)
            {
                CFDataRef ourState = CFDataCreate (kCFAllocatorDefault, (const UInt8*) state.getData(), (CFIndex) state.getSize());
                CFDictionarySetValue (dict, JUCE_STATE_DICTIONARY_KEY, ourState);
                CFRelease (ourState);
            }
        }

        return noErr;
    }

    ComponentResult RestoreState (CFPropertyListRef inData) override
    {
        {
            // Remove the data entry from the state to prevent the superclass loading the parameters
            CFMutableDictionaryRef copyWithoutData = CFDictionaryCreateMutableCopy (nullptr, 0, (CFDictionaryRef) inData);
            CFDictionaryRemoveValue (copyWithoutData, CFSTR (kAUPresetDataKey));
            ComponentResult err = JuceAUBaseClass::RestoreState (copyWithoutData);
            CFRelease (copyWithoutData);

            if (err != noErr)
                return err;
        }

        if (juceFilter != nullptr)
        {
            CFDictionaryRef dict = (CFDictionaryRef) inData;
            CFDataRef data = 0;

            if (CFDictionaryGetValueIfPresent (dict, JUCE_STATE_DICTIONARY_KEY, (const void**) &data))
            {
                if (data != 0)
                {
                    const int numBytes = (int) CFDataGetLength (data);
                    const juce::uint8* const rawBytes = CFDataGetBytePtr (data);

                    if (numBytes > 0)
                        juceFilter->setCurrentProgramStateInformation (rawBytes, numBytes);
                }
            }
        }

        return noErr;
    }

    UInt32 SupportedNumChannels (const AUChannelInfo** outInfo) override
    {
        // If you hit this, then you need to add some configurations to your
        // JucePlugin_PreferredChannelConfigurations setting..
        jassert (numChannelConfigs > 0);

        if (outInfo != nullptr)
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
                                      AudioUnitParameterInfo& outParameterInfo) override
    {
        const int index = (int) inParameterID;

        if (inScope == kAudioUnitScope_Global
             && juceFilter != nullptr
             && index < juceFilter->getNumParameters())
        {
            outParameterInfo.flags = (UInt32) (kAudioUnitParameterFlag_IsWritable
                                                | kAudioUnitParameterFlag_IsReadable
                                                | kAudioUnitParameterFlag_HasCFNameString
                                                | kAudioUnitParameterFlag_ValuesHaveStrings);

            const String name (juceFilter->getParameterName (index));

            // set whether the param is automatable (unnamed parameters aren't allowed to be automated)
            if (name.isEmpty() || ! juceFilter->isParameterAutomatable (index))
                outParameterInfo.flags |= kAudioUnitParameterFlag_NonRealTime;

            if (juceFilter->isMetaParameter (index))
                outParameterInfo.flags |= kAudioUnitParameterFlag_IsGlobalMeta;

            AUBase::FillInParameterName (outParameterInfo, name.toCFString(), true);

            outParameterInfo.minValue = 0.0f;
            outParameterInfo.maxValue = 1.0f;
            outParameterInfo.defaultValue = juceFilter->getParameterDefaultValue (index);
            jassert (outParameterInfo.defaultValue >= outParameterInfo.minValue
                      && outParameterInfo.defaultValue <= outParameterInfo.maxValue);
            outParameterInfo.unit = kAudioUnitParameterUnit_Generic;

            return noErr;
        }

        return kAudioUnitErr_InvalidParameter;
    }

    ComponentResult GetParameter (AudioUnitParameterID inID,
                                  AudioUnitScope inScope,
                                  AudioUnitElement inElement,
                                  Float32& outValue) override
    {
        if (inScope == kAudioUnitScope_Global && juceFilter != nullptr)
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
                                  UInt32 inBufferOffsetInFrames) override
    {
        if (inScope == kAudioUnitScope_Global && juceFilter != nullptr)
        {
            juceFilter->setParameter ((int) inID, inValue);
            return noErr;
        }

        return AUBase::SetParameter (inID, inScope, inElement, inValue, inBufferOffsetInFrames);
    }

    // No idea what this method actually does or what it should return. Current Apple docs say nothing about it.
    // (Note that this isn't marked 'override' in case older versions of the SDK don't include it)
    bool CanScheduleParameters() const                   { return false; }

    //==============================================================================
    ComponentResult Version() override                   { return JucePlugin_VersionCode; }
    bool SupportsTail() override                         { return true; }
    Float64 GetTailTime() override                       { return juceFilter->getTailLengthSeconds(); }
    double getSampleRate()                               { return GetOutput(0)->GetStreamFormat().mSampleRate; }

    Float64 GetLatency() override
    {
        const double rate = getSampleRate();
        jassert (rate > 0);
        return rate > 0 ? juceFilter->getLatencySamples() / rate : 0;
    }

    //==============================================================================
   #if BUILD_AU_CARBON_UI
    int GetNumCustomUIComponents() override
    {
        return getHostType().isDigitalPerformer() ? 0 : 1;
    }

    void GetUIComponentDescs (ComponentDescription* inDescArray) override
    {
        inDescArray[0].componentType = kAudioUnitCarbonViewComponentType;
        inDescArray[0].componentSubType = JucePlugin_AUSubType;
        inDescArray[0].componentManufacturer = JucePlugin_AUManufacturerCode;
        inDescArray[0].componentFlags = 0;
        inDescArray[0].componentFlagsMask = 0;
    }
   #endif

    //==============================================================================
    bool getCurrentPosition (AudioPlayHead::CurrentPositionInfo& info) override
    {
        info.timeSigNumerator = 0;
        info.timeSigDenominator = 0;
        info.editOriginTime = 0;
        info.ppqPositionOfLastBarStart = 0;
        info.isRecording = false;
        info.ppqLoopStart = 0;
        info.ppqLoopEnd = 0;

        switch (lastTimeStamp.mSMPTETime.mType)
        {
            case kSMPTETimeType24:          info.frameRate = AudioPlayHead::fps24; break;
            case kSMPTETimeType25:          info.frameRate = AudioPlayHead::fps25; break;
            case kSMPTETimeType30Drop:      info.frameRate = AudioPlayHead::fps30drop; break;
            case kSMPTETimeType30:          info.frameRate = AudioPlayHead::fps30; break;
            case kSMPTETimeType2997:        info.frameRate = AudioPlayHead::fps2997; break;
            case kSMPTETimeType2997Drop:    info.frameRate = AudioPlayHead::fps2997drop; break;
            //case kSMPTETimeType60:
            //case kSMPTETimeType5994:
            default:                        info.frameRate = AudioPlayHead::fpsUnknown; break;
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
            info.timeSigNumerator   = (int) num;
            info.timeSigDenominator = (int) den;
            info.ppqPositionOfLastBarStart = outCurrentMeasureDownBeat;
        }

        double outCurrentSampleInTimeLine, outCycleStartBeat, outCycleEndBeat;
        Boolean playing = false, looping = false, playchanged;

        if (CallHostTransportState (&playing,
                                    &playchanged,
                                    &outCurrentSampleInTimeLine,
                                    &looping,
                                    &outCycleStartBeat,
                                    &outCycleEndBeat) != noErr)
        {
            // If the host doesn't support this callback, use the sample time from lastTimeStamp:
            outCurrentSampleInTimeLine = lastTimeStamp.mSampleTime;
        }

        info.isPlaying = playing;
        info.timeInSamples = (int64) (outCurrentSampleInTimeLine + 0.5);
        info.timeInSeconds = info.timeInSamples / getSampleRate();
        info.isLooping = looping;

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

    void audioProcessorParameterChanged (AudioProcessor*, int index, float /*newValue*/)
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
        PropertyChanged (kAudioUnitProperty_Latency,       kAudioUnitScope_Global, 0);
        PropertyChanged (kAudioUnitProperty_ParameterList, kAudioUnitScope_Global, 0);
        PropertyChanged (kAudioUnitProperty_ParameterInfo, kAudioUnitScope_Global, 0);
    }

    bool StreamFormatWritable (AudioUnitScope, AudioUnitElement) override
    {
        return ! IsInitialized();
    }

    // (these two slightly different versions are because the definition changed between 10.4 and 10.5)
    ComponentResult StartNote (MusicDeviceInstrumentID, MusicDeviceGroupID, NoteInstanceID&, UInt32, const MusicDeviceNoteParams&) { return noErr; }
    ComponentResult StartNote (MusicDeviceInstrumentID, MusicDeviceGroupID, NoteInstanceID*, UInt32, const MusicDeviceNoteParams&) { return noErr; }
    ComponentResult StopNote (MusicDeviceGroupID, NoteInstanceID, UInt32)   { return noErr; }

    //==============================================================================
    ComponentResult Initialize() override
    {
       #if ! JucePlugin_IsSynth
        const int numIns  = findNumInputChannels();
       #endif
        const int numOuts = findNumOutputChannels();

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

    void Cleanup() override
    {
        JuceAUBaseClass::Cleanup();

        if (juceFilter != nullptr)
            juceFilter->releaseResources();

        bufferSpace.setSize (2, 16);
        midiEvents.clear();
        incomingEvents.clear();
        prepared = false;
    }

    ComponentResult Reset (AudioUnitScope inScope, AudioUnitElement inElement) override
    {
        if (! prepared)
            prepareToPlay();

        if (juceFilter != nullptr)
            juceFilter->reset();

        return JuceAUBaseClass::Reset (inScope, inElement);
    }

    int findNumInputChannels()
    {
       #if ! JucePlugin_IsSynth
        if (AUInputElement* e = GetInput(0))
            return (int) e->GetStreamFormat().mChannelsPerFrame;
       #endif

        return 0;
    }

    int findNumOutputChannels()
    {
        if (AUOutputElement* e = GetOutput(0))
            return (int) e->GetStreamFormat().mChannelsPerFrame;

        return 0;
    }

    void prepareToPlay()
    {
        if (juceFilter != nullptr)
        {
            juceFilter->setPlayConfigDetails (findNumInputChannels(),
                                              findNumOutputChannels(),
                                              getSampleRate(),
                                              (int) GetMaxFramesPerSlice());

            bufferSpace.setSize (juceFilter->getNumInputChannels() + juceFilter->getNumOutputChannels(),
                                 (int) GetMaxFramesPerSlice() + 32);

            juceFilter->prepareToPlay (getSampleRate(), (int) GetMaxFramesPerSlice());

            midiEvents.ensureSize (2048);
            midiEvents.clear();
            incomingEvents.ensureSize (2048);
            incomingEvents.clear();

            channels.calloc ((size_t) jmax (juceFilter->getNumInputChannels(),
                                            juceFilter->getNumOutputChannels()) + 4);

            prepared = true;
        }
    }

    ComponentResult Render (AudioUnitRenderActionFlags &ioActionFlags,
                            const AudioTimeStamp& inTimeStamp,
                            UInt32 nFrames) override
    {
        lastTimeStamp = inTimeStamp;

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
                                 UInt32 numSamples) override
    {
        if (juceFilter != nullptr)
        {
            jassert (prepared);

            int numOutChans = 0;
            int nextSpareBufferChan = 0;
            bool needToReinterleave = false;
            const int numIn = juceFilter->getNumInputChannels();
            const int numOut = juceFilter->getNumOutputChannels();

            for (unsigned int i = 0; i < outBuffer.mNumberBuffers; ++i)
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
                        channels [numOutChans++] = bufferSpace.getWritePointer (nextSpareBufferChan++);
                }

                if (numOutChans >= numOut)
                    break;
            }

            int numInChans = 0;

            for (unsigned int i = 0; i < inBuffer.mNumberBuffers; ++i)
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
                            dest = bufferSpace.getWritePointer (nextSpareBufferChan++);
                            channels [numInChans++] = dest;
                        }

                        const float* src = ((const float*) buf.mData) + subChan;

                        for (int j = (int) numSamples; --j >= 0;)
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
                AudioSampleBuffer buffer (channels, jmax (numIn, numOut), (int) numSamples);

                const ScopedLock sl (juceFilter->getCallbackLock());

                if (juceFilter->isSuspended())
                {
                    for (int j = 0; j < numOut; ++j)
                        zeromem (channels [j], sizeof (float) * numSamples);
                }
               #if ! JucePlugin_IsSynth
                else if (ShouldBypassEffect())
                {
                    juceFilter->processBlockBypassed (buffer, midiEvents);
                }
               #endif
                else
                {
                    juceFilter->processBlock (buffer, midiEvents);
                }
            }

            if (! midiEvents.isEmpty())
            {
               #if JucePlugin_ProducesMidiOutput
                if (midiCallback.midiOutputCallback != nullptr)
                {
                    UInt32 numPackets = 0;
                    size_t dataSize = 0;

                    const juce::uint8* midiEventData;
                    int midiEventSize, midiEventPosition;

                    for (MidiBuffer::Iterator i (midiEvents); i.getNextEvent (midiEventData, midiEventSize, midiEventPosition);)
                    {
                        jassert (isPositiveAndBelow (midiEventPosition, (int) numSamples));
                        dataSize += (size_t) midiEventSize;
                        ++numPackets;
                    }

                    MIDIPacket* p;
                    const size_t packetMembersSize     = sizeof (MIDIPacket)     - sizeof (p->data); // NB: GCC chokes on "sizeof (MidiMessage::data)"
                    const size_t packetListMembersSize = sizeof (MIDIPacketList) - sizeof (p->data);

                    HeapBlock<MIDIPacketList> packetList;
                    packetList.malloc (packetListMembersSize + packetMembersSize * numPackets + dataSize, 1);
                    packetList->numPackets = numPackets;

                    p = packetList->packet;

                    for (MidiBuffer::Iterator i (midiEvents); i.getNextEvent (midiEventData, midiEventSize, midiEventPosition);)
                    {
                        p->timeStamp = (MIDITimeStamp) midiEventPosition;
                        p->length = (UInt16) midiEventSize;
                        memcpy (p->data, midiEventData, (size_t) midiEventSize);
                        p = MIDIPacketNext (p);
                    }

                    midiCallback.midiOutputCallback (midiCallback.userData, &lastTimeStamp, 0, packetList);
                }
               #endif

                midiEvents.clear();
            }

            if (needToReinterleave)
            {
                nextSpareBufferChan = 0;

                for (unsigned int i = 0; i < outBuffer.mNumberBuffers; ++i)
                {
                    AudioBuffer& buf = outBuffer.mBuffers[i];

                    if (buf.mNumberChannels > 1)
                    {
                        for (unsigned int subChan = 0; subChan < buf.mNumberChannels; ++subChan)
                        {
                            const float* src = bufferSpace.getReadPointer (nextSpareBufferChan++);
                            float* dest = ((float*) buf.mData) + subChan;

                            for (int j = (int) numSamples; --j >= 0;)
                            {
                                *dest = *src++;
                                dest += buf.mNumberChannels;
                            }
                        }
                    }
                }
            }

           #if ! JucePlugin_SilenceInProducesSilenceOut
            ioActionFlags &= (AudioUnitRenderActionFlags) ~kAudioUnitRenderAction_OutputIsSilence;
           #endif
        }

        return noErr;
    }

    OSStatus HandleMidiEvent (UInt8 nStatus, UInt8 inChannel, UInt8 inData1, UInt8 inData2, UInt32 inStartFrame) override
    {
       #if JucePlugin_WantsMidiInput
        const ScopedLock sl (incomingMidiLock);
        const juce::uint8 data[] = { (juce::uint8) (nStatus | inChannel),
                                     (juce::uint8) inData1,
                                     (juce::uint8) inData2 };

        incomingEvents.addEvent (data, 3, (int) inStartFrame);
        return noErr;
       #else
        (void) nStatus; (void) inChannel; (void) inData1; (void) inData2; (void) inStartFrame;
        return kAudioUnitErr_PropertyNotInUse;
       #endif
    }

    OSStatus HandleSysEx (const UInt8* inData, UInt32 inLength) override
    {
       #if JucePlugin_WantsMidiInput
        const ScopedLock sl (incomingMidiLock);
        incomingEvents.addEvent (inData, (int) inLength, 0);
        return noErr;
       #else
        (void) inData; (void) inLength;
        return kAudioUnitErr_PropertyNotInUse;
       #endif
    }

    //==============================================================================
    ComponentResult GetPresets (CFArrayRef* outData) const override
    {
        if (outData != nullptr)
        {
            const int numPrograms = juceFilter->getNumPrograms();

            clearPresetsArray();
            presetsArray.insertMultiple (0, AUPreset(), numPrograms);

            CFMutableArrayRef presetsArrayRef = CFArrayCreateMutable (0, numPrograms, 0);

            for (int i = 0; i < numPrograms; ++i)
            {
                String name (juceFilter->getProgramName(i));
                if (name.isEmpty())
                    name = "Untitled";

                AUPreset& p = presetsArray.getReference(i);
                p.presetNumber = i;
                p.presetName = name.toCFString();

                CFArrayAppendValue (presetsArrayRef, &p);
            }

            *outData = (CFArrayRef) presetsArrayRef;
        }

        return noErr;
    }

    OSStatus NewFactoryPresetSet (const AUPreset& inNewFactoryPreset) override
    {
        const int numPrograms = juceFilter->getNumPrograms();
        const SInt32 chosenPresetNumber = (int) inNewFactoryPreset.presetNumber;

        if (chosenPresetNumber >= numPrograms)
            return kAudioUnitErr_InvalidProperty;

        AUPreset chosenPreset;
        chosenPreset.presetNumber = chosenPresetNumber;
        chosenPreset.presetName = juceFilter->getProgramName (chosenPresetNumber).toCFString();

        juceFilter->setCurrentProgram (chosenPresetNumber);
        SetAFactoryPresetAsCurrent (chosenPreset);

        return noErr;
    }

    void componentMovedOrResized (Component& component, bool /*wasMoved*/, bool /*wasResized*/) override
    {
        NSView* view = (NSView*) component.getWindowHandle();
        NSRect r = [[view superview] frame];
        r.origin.y = r.origin.y + r.size.height - component.getHeight();
        r.size.width = component.getWidth();
        r.size.height = component.getHeight();
        [[view superview] setFrame: r];
        [view setFrame: makeNSRect (component.getLocalBounds())];
        [view setNeedsDisplay: YES];
    }

    //==============================================================================
    class EditorCompHolder  : public Component
    {
    public:
        EditorCompHolder (AudioProcessorEditor* const editor)
        {
            setSize (editor->getWidth(), editor->getHeight());
            addAndMakeVisible (editor);

           #if ! JucePlugin_EditorRequiresKeyboardFocus
            setWantsKeyboardFocus (false);
           #else
            setWantsKeyboardFocus (true);
           #endif
        }

        ~EditorCompHolder()
        {
            deleteAllChildren(); // note that we can't use a ScopedPointer because the editor may
                                 // have been transferred to another parent which takes over ownership.
        }

        static NSView* createViewFor (AudioProcessor* filter, JuceAU* au, AudioProcessorEditor* const editor)
        {
            EditorCompHolder* editorCompHolder = new EditorCompHolder (editor);
            NSRect r = makeNSRect (editorCompHolder->getLocalBounds());

            static JuceUIViewClass cls;
            NSView* view = [[cls.createInstance() initWithFrame: r] autorelease];

            JuceUIViewClass::setFilter (view, filter);
            JuceUIViewClass::setAU (view, au);
            JuceUIViewClass::setEditor (view, editorCompHolder);

            [view setHidden: NO];
            [view setPostsFrameChangedNotifications: YES];

            [[NSNotificationCenter defaultCenter] addObserver: view
                                                     selector: @selector (applicationWillTerminate:)
                                                         name: NSApplicationWillTerminateNotification
                                                       object: nil];
            activeUIs.add (view);

            editorCompHolder->addToDesktop (0, (void*) view);
            editorCompHolder->setVisible (view);
            return view;
        }

        void childBoundsChanged (Component*) override
        {
            if (Component* editor = getChildComponent(0))
            {
                const int w = jmax (32, editor->getWidth());
                const int h = jmax (32, editor->getHeight());

                if (getWidth() != w || getHeight() != h)
                    setSize (w, h);

                NSView* view = (NSView*) getWindowHandle();
                NSRect r = [[view superview] frame];
                r.size.width = editor->getWidth();
                r.size.height = editor->getHeight();
                [[view superview] setFrame: r];
                [view setFrame: makeNSRect (editor->getLocalBounds())];
                [view setNeedsDisplay: YES];
            }
        }

        bool keyPressed (const KeyPress&) override
        {
            if (getHostType().isAbletonLive())
            {
                static NSTimeInterval lastEventTime = 0; // check we're not recursively sending the same event
                NSTimeInterval eventTime = [[NSApp currentEvent] timestamp];

                if (lastEventTime != eventTime)
                {
                    lastEventTime = eventTime;

                    NSView* view = (NSView*) getWindowHandle();
                    NSView* hostView = [view superview];
                    NSWindow* hostWindow = [hostView window];

                    [hostWindow makeFirstResponder: hostView];
                    [hostView keyDown: [NSApp currentEvent]];
                    [hostWindow makeFirstResponder: view];
                }
            }

            return false;
        }

    private:
        JUCE_DECLARE_NON_COPYABLE (EditorCompHolder)
    };

    //==============================================================================
    struct JuceUIViewClass  : public ObjCClass<NSView>
    {
        JuceUIViewClass()  : ObjCClass<NSView> ("JUCEAUView_")
        {
            addIvar<AudioProcessor*> ("filter");
            addIvar<JuceAU*> ("au");
            addIvar<EditorCompHolder*> ("editor");

            addMethod (@selector (dealloc),                     dealloc,                    "v@:");
            addMethod (@selector (applicationWillTerminate:),   applicationWillTerminate,   "v@:@");
            addMethod (@selector (viewDidMoveToWindow),         viewDidMoveToWindow,        "v@:");
            addMethod (@selector (mouseDownCanMoveWindow),      mouseDownCanMoveWindow,     "c@:");

            registerClass();
        }

        static void deleteEditor (id self)
        {
            ScopedPointer<EditorCompHolder> editorComp (getEditor (self));

            if (editorComp != nullptr)
            {
                if (editorComp->getChildComponent(0) != nullptr
                     && activePlugins.contains (getAU (self))) // plugin may have been deleted before the UI
                {
                    AudioProcessor* const filter = getIvar<AudioProcessor*> (self, "filter");
                    filter->editorBeingDeleted ((AudioProcessorEditor*) editorComp->getChildComponent(0));
                }

                editorComp = nullptr;
                setEditor (self, nullptr);
            }
        }

        static JuceAU* getAU (id self)                          { return getIvar<JuceAU*> (self, "au"); }
        static EditorCompHolder* getEditor (id self)            { return getIvar<EditorCompHolder*> (self, "editor"); }

        static void setFilter (id self, AudioProcessor* filter) { object_setInstanceVariable (self, "filter", filter); }
        static void setAU (id self, JuceAU* au)                 { object_setInstanceVariable (self, "au", au); }
        static void setEditor (id self, EditorCompHolder* e)    { object_setInstanceVariable (self, "editor", e); }

    private:
        static void dealloc (id self, SEL)
        {
            if (activeUIs.contains (self))
                shutdown (self);

            sendSuperclassMessage (self, @selector (dealloc));
        }

        static void applicationWillTerminate (id self, SEL, NSNotification*)
        {
            shutdown (self);
        }

        static void shutdown (id self)
        {
            [[NSNotificationCenter defaultCenter] removeObserver: self];
            deleteEditor (self);

            jassert (activeUIs.contains (self));
            activeUIs.removeFirstMatchingValue (self);

            if (activePlugins.size() + activeUIs.size() == 0)
            {
                // there's some kind of component currently modal, but the host
                // is trying to delete our plugin..
                jassert (Component::getCurrentlyModalComponent() == nullptr);

                shutdownJuce_GUI();
            }
        }

        static void viewDidMoveToWindow (id self, SEL)
        {
            if (NSWindow* w = [(NSView*) self window])
            {
                [w setAcceptsMouseMovedEvents: YES];

                if (EditorCompHolder* const editorComp = getEditor (self))
                    [w makeFirstResponder: (NSView*) editorComp->getWindowHandle()];
            }
        }

        static BOOL mouseDownCanMoveWindow (id, SEL)
        {
            return NO;
        }
    };

    //==============================================================================
    struct JuceUICreationClass  : public ObjCClass<NSObject>
    {
        JuceUICreationClass()  : ObjCClass<NSObject> ("JUCE_AUCocoaViewClass_")
        {
            addMethod (@selector (interfaceVersion),             interfaceVersion,    @encode (unsigned int), "@:");
            addMethod (@selector (description),                  description,         @encode (NSString*),    "@:");
            addMethod (@selector (uiViewForAudioUnit:withSize:), uiViewForAudioUnit,  @encode (NSView*),      "@:", @encode (AudioUnit), @encode (NSSize));

            addProtocol (@protocol (AUCocoaUIBase));

            registerClass();
        }

    private:
        static unsigned int interfaceVersion (id, SEL)   { return 0; }

        static NSString* description (id, SEL)
        {
            return [NSString stringWithString: nsStringLiteral (JucePlugin_Name)];
        }

        static NSView* uiViewForAudioUnit (id, SEL, AudioUnit inAudioUnit, NSSize)
        {
            void* pointers[2];
            UInt32 propertySize = sizeof (pointers);

            if (AudioUnitGetProperty (inAudioUnit, juceFilterObjectPropertyID,
                                      kAudioUnitScope_Global, 0, pointers, &propertySize) == noErr)
            {
                if (AudioProcessor* filter = static_cast<AudioProcessor*> (pointers[0]))
                    if (AudioProcessorEditor* editorComp = filter->createEditorIfNeeded())
                        return EditorCompHolder::createViewFor (filter, static_cast<JuceAU*> (pointers[1]), editorComp);
            }

            return nil;
        }
    };

private:
    //==============================================================================
    ScopedPointer<AudioProcessor> juceFilter;
    AudioSampleBuffer bufferSpace;
    HeapBlock<float*> channels;
    MidiBuffer midiEvents, incomingEvents;
    bool prepared;
    AUChannelInfo channelInfo [numChannelConfigs];
    AudioUnitEvent auEvent;
    mutable Array<AUPreset> presetsArray;
    CriticalSection incomingMidiLock;
    AUMIDIOutputCallbackStruct midiCallback;
    AudioTimeStamp lastTimeStamp;

    void clearPresetsArray() const
    {
        for (int i = presetsArray.size(); --i >= 0;)
            CFRelease (presetsArray.getReference(i).presetName);

        presetsArray.clear();
    }

    JUCE_DECLARE_NON_COPYABLE (JuceAU)
};


//==============================================================================
#if BUILD_AU_CARBON_UI

class JuceAUView  : public AUCarbonViewBase
{
public:
    JuceAUView (AudioUnitCarbonView auview)
      : AUCarbonViewBase (auview),
        juceFilter (nullptr)
    {
    }

    ~JuceAUView()
    {
        deleteUI();
    }

    ComponentResult CreateUI (Float32 /*inXOffset*/, Float32 /*inYOffset*/) override
    {
        JUCE_AUTORELEASEPOOL
        {
            if (juceFilter == nullptr)
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

            if (juceFilter != nullptr)
            {
                deleteUI();

                if (AudioProcessorEditor* editorComp = juceFilter->createEditorIfNeeded())
                {
                    editorComp->setOpaque (true);
                    windowComp = new ComponentInHIView (editorComp, mCarbonPane);
                }
            }
            else
            {
                jassertfalse; // can't get a pointer to our effect
            }
        }

        return noErr;
    }

    AudioUnitCarbonViewEventListener getEventListener() const   { return mEventListener; }
    void* getEventListenerUserData() const                      { return mEventListenerUserData; }

private:
    //==============================================================================
    AudioProcessor* juceFilter;
    ScopedPointer<Component> windowComp;
    FakeMouseMoveGenerator fakeMouseGenerator;

    void deleteUI()
    {
        if (windowComp != nullptr)
        {
            PopupMenu::dismissAllActiveMenus();

            /* This assertion is triggered when there's some kind of modal component active, and the
               host is trying to delete our plugin.
               If you must use modal components, always use them in a non-blocking way, by never
               calling runModalLoop(), but instead using enterModalState() with a callback that
               will be performed on completion. (Note that this assertion could actually trigger
               a false alarm even if you're doing it correctly, but is here to catch people who
               aren't so careful) */
            jassert (Component::getCurrentlyModalComponent() == nullptr);

            if (JuceAU::EditorCompHolder* editorCompHolder = dynamic_cast<JuceAU::EditorCompHolder*> (windowComp->getChildComponent(0)))
                if (AudioProcessorEditor* audioProcessEditor = dynamic_cast<AudioProcessorEditor*> (editorCompHolder->getChildComponent(0)))
                    juceFilter->editorBeingDeleted (audioProcessEditor);

            windowComp = nullptr;
        }
    }

    //==============================================================================
    // Uses a child NSWindow to sit in front of a HIView and display our component
    class ComponentInHIView  : public Component
    {
    public:
        ComponentInHIView (AudioProcessorEditor* ed, HIViewRef parentHIView)
            : parentView (parentHIView),
              editor (ed),
              recursive (false)
        {
            JUCE_AUTORELEASEPOOL
            {
                jassert (ed != nullptr);
                addAndMakeVisible (editor);
                setOpaque (true);
                setVisible (true);
                setBroughtToFrontOnMouseClick (true);

                setSize (editor.getWidth(), editor.getHeight());
                SizeControl (parentHIView, (SInt16) editor.getWidth(), (SInt16) editor.getHeight());

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

                attachWindowHidingHooks (this, (WindowRef) windowRef, hostWindow);
            }
        }

        ~ComponentInHIView()
        {
            JUCE_AUTORELEASEPOOL
            {
                removeWindowHidingHooks (this);

                NSWindow* pluginWindow = [((NSView*) getWindowHandle()) window];
                [hostWindow removeChildWindow: pluginWindow];
                removeFromDesktop();

                [hostWindow release];
                hostWindow = nil;
            }
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

        void resized() override
        {
            if (Component* const child = getChildComponent (0))
                child->setBounds (getLocalBounds());
        }

        void paint (Graphics&) override {}

        void childBoundsChanged (Component*) override
        {
            if (! recursive)
            {
                recursive = true;

                const int w = jmax (32, editor.getWidth());
                const int h = jmax (32, editor.getHeight());

                SizeControl (parentView, (SInt16) w, (SInt16) h);

                if (getWidth() != w || getHeight() != h)
                    setSize (w, h);

                editor.repaint();

                updateWindowPos();
                addSubWindow(); // (need this for AULab)

                recursive = false;
            }
        }

        bool keyPressed (const KeyPress& kp) override
        {
            if (! kp.getModifiers().isCommandDown())
            {
                // If we have an unused keypress, move the key-focus to a host window
                // and re-inject the event..
                static NSTimeInterval lastEventTime = 0; // check we're not recursively sending the same event
                NSTimeInterval eventTime = [[NSApp currentEvent] timestamp];

                if (lastEventTime != eventTime)
                {
                    lastEventTime = eventTime;

                    [[hostWindow parentWindow] makeKeyWindow];
                    repostCurrentNSEvent();
                }
            }

            return false;
        }

    private:
        HIViewRef parentView;
        NSWindow* hostWindow;
        JuceAU::EditorCompHolder editor;
        bool recursive;
    };
};

#endif

//==============================================================================
#define JUCE_COMPONENT_ENTRYX(Class, Name, Suffix) \
    extern "C" __attribute__((visibility("default"))) ComponentResult Name ## Suffix (ComponentParameters* params, Class* obj); \
    extern "C" __attribute__((visibility("default"))) ComponentResult Name ## Suffix (ComponentParameters* params, Class* obj) \
    { \
        return ComponentEntryPoint<Class>::Dispatch (params, obj); \
    }

#if JucePlugin_ProducesMidiOutput || JucePlugin_WantsMidiInput
 #define FACTORY_BASE_CLASS AUMIDIEffectFactory
#else
 #define FACTORY_BASE_CLASS AUBaseFactory
#endif

#define JUCE_FACTORY_ENTRYX(Class, Name) \
    extern "C" __attribute__((visibility("default"))) void* Name ## Factory (const AudioComponentDescription* desc); \
    extern "C" __attribute__((visibility("default"))) void* Name ## Factory (const AudioComponentDescription* desc) \
    { \
        return FACTORY_BASE_CLASS<Class>::Factory (desc); \
    }

#define JUCE_COMPONENT_ENTRY(Class, Name, Suffix)   JUCE_COMPONENT_ENTRYX(Class, Name, Suffix)
#define JUCE_FACTORY_ENTRY(Class, Name)             JUCE_FACTORY_ENTRYX(Class, Name)

//==============================================================================
JUCE_COMPONENT_ENTRY (JuceAU, JucePlugin_AUExportPrefix, Entry)

#ifndef AUDIOCOMPONENT_ENTRY
 #define JUCE_DISABLE_AU_FACTORY_ENTRY 1
#endif

#if ! JUCE_DISABLE_AU_FACTORY_ENTRY  // (You might need to disable this for old Xcode 3 builds)
JUCE_FACTORY_ENTRY   (JuceAU, JucePlugin_AUExportPrefix)
#endif

#if BUILD_AU_CARBON_UI
 JUCE_COMPONENT_ENTRY (JuceAUView, JucePlugin_AUExportPrefix, ViewEntry)
#endif

#if ! JUCE_DISABLE_AU_FACTORY_ENTRY
 #include "AUPlugInDispatch.cpp"
#endif

#endif

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
#include "CoreAudioUtilityClasses/MusicDeviceBase.h"
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
 #include "CoreAudioUtilityClasses/AUCarbonViewBase.h"
 #undef Point
#endif

#ifdef __clang__
 #pragma clang diagnostic pop
#endif

#define JUCE_MAC_WINDOW_VISIBITY_BODGE 1

#include "../utility/juce_IncludeModuleHeaders.h"
#include "../utility/juce_FakeMouseMoveGenerator.h"
#include "../utility/juce_CarbonVisibility.h"
#include "../utility/juce_PluginBusUtilities.h"
#include "../../juce_core/native/juce_osx_ObjCHelpers.h"

//==============================================================================
static Array<void*> activePlugins, activeUIs;

static const AudioUnitPropertyID juceFilterObjectPropertyID = 0x1a45ffe9;

// This macro can be set if you need to override this internal name for some reason..
#ifndef JUCE_STATE_DICTIONARY_KEY
 #define JUCE_STATE_DICTIONARY_KEY   CFSTR("jucePluginState")
#endif

// make sure the audio processor is initialized before the AUBase class
struct AudioProcessorHolder
{
    AudioProcessorHolder(bool initialiseGUI)
    {
        if (initialiseGUI)
        {
           #if BUILD_AU_CARBON_UI
            NSApplicationLoad();
           #endif

            initialiseJuce_GUI();
        }

        juceFilter = createPluginFilterOfType (AudioProcessor::wrapperType_AudioUnit);
    }

    ScopedPointer<AudioProcessor> juceFilter;
};

//==============================================================================
class JuceAU   : public AudioProcessorHolder,
                 public MusicDeviceBase,
                 public AudioProcessorListener,
                 public AudioPlayHead,
                 public ComponentListener
{
public:
    JuceAU (AudioUnit component)
        : AudioProcessorHolder(activePlugins.size() + activeUIs.size() == 0),
          MusicDeviceBase (component, (UInt32) PluginBusUtilities (*juceFilter, false).getNumEnabledBuses (true),
                                      (UInt32) PluginBusUtilities (*juceFilter, false).getNumEnabledBuses (false)),
          isBypassed (false),
          busUtils (*juceFilter, true)
    {
        busUtils.findAllCompatibleLayouts();
        populateAUChannelInfo();

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

        if (syncAudioUnitWithProcessor () != noErr)
            jassertfalse;
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

    //==============================================================================
    ComponentResult Initialize() override
    {
        ComponentResult err;
        PluginBusUtilities::ScopedBusRestorer restorer (busUtils);

        if ((err = syncProcessorWithAudioUnit()) != noErr)
            return err;

        if ((err = MusicDeviceBase::Initialize()) != noErr)
            return err;

        prepareToPlay();
        restorer.release();

        return noErr;
    }

    void Cleanup() override
    {
        MusicDeviceBase::Cleanup();

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

        return MusicDeviceBase::Reset (inScope, inElement);
    }

    //==============================================================================
    void prepareToPlay()
    {
        if (juceFilter != nullptr)
        {
            juceFilter->setRateAndBufferSizeDetails (getSampleRate(), (int) GetMaxFramesPerSlice());

            bufferSpace.setSize (jmax (busUtils.findTotalNumChannels (true), busUtils.findTotalNumChannels (false)),
                                 (int) GetMaxFramesPerSlice() + 32);

            juceFilter->prepareToPlay (getSampleRate(), (int) GetMaxFramesPerSlice());

            midiEvents.ensureSize (2048);
            midiEvents.clear();
            incomingEvents.ensureSize (2048);
            incomingEvents.clear();

            channels.calloc ((size_t) jmax (juceFilter->getTotalNumInputChannels(),
                                            juceFilter->getTotalNumOutputChannels()) + 4);

            prepared = true;
        }
    }

    //==============================================================================
    static OSStatus ComponentEntryDispatch (ComponentParameters* params, JuceAU* effect)
    {
        if (effect == nullptr)
            return paramErr;

        switch (params->what)
        {
            case kMusicDeviceMIDIEventSelect:
            case kMusicDeviceSysExSelect:
                return AUMIDIBase::ComponentEntryDispatch (params, effect);
            default:
                break;
        }

        return MusicDeviceBase::ComponentEntryDispatch (params, effect);
    }

    //==============================================================================
    bool BusCountWritable (AudioUnitScope scope) override
    {
        bool isInput;

        if (scopeToDirection (scope, isInput) != noErr)
            return false;

       #if JucePlugin_IsMidiEffect
        return false;
       #elif JucePlugin_IsSynth
        if (isInput) return busUtils.hasDynamicInBuses();
       #endif

        return isInput ? (busUtils.getBusCount (true)  > 1 && busUtils.hasDynamicInBuses())
                       : (busUtils.getBusCount (false) > 1 && busUtils.hasDynamicOutBuses());
    }

    OSStatus SetBusCount (AudioUnitScope scope, UInt32 count) override
    {
        OSStatus err = noErr;
        bool isInput;

        if ((err = scopeToDirection (scope, isInput)) != noErr)
            return err;

        if (count != GetScope (scope).GetNumberOfElements())
        {
            if ((isInput && (! busUtils.hasDynamicInBuses())) || ((! isInput) && (! busUtils.hasDynamicOutBuses())))
                return kAudioUnitErr_PropertyNotWritable;

            // Similar as with the stream format, we don't really tell the AudioProcessor about
            // the bus count change until Initialize is called. We only generally test if
            // this bus count can work.
            if (static_cast<int> (count) > busUtils.getBusCount (isInput))
                return kAudioUnitErr_FormatNotSupported;

            // we need to already create the underlying elements so that we can change their formats
            if ((err = MusicDeviceBase::SetBusCount (scope, count)) != noErr)
                return err;

            // however we do need to update the format tag: we need to do the same thing in SetFormat, for example
            const int currentNumBus = busUtils.getNumEnabledBuses (isInput);
            const int requestedNumBus = static_cast<int> (count);

            if (currentNumBus < requestedNumBus)
            {
                for (int busNr = currentNumBus; busNr < requestedNumBus; ++busNr)
                    if ((err = syncAudioUnitWithChannelSet (isInput, busNr, busUtils.getDefaultLayoutForBus (isInput, busNr))) != noErr)
                        return err;
            }
            else
            {
                AudioChannelLayoutTag nulltag = ChannelSetToCALayoutTag (AudioChannelSet());

                for (int busNr = requestedNumBus; busNr < currentNumBus; ++busNr)
                    getCurrentLayout (isInput, busNr) = nulltag;
            }
        }

        return MusicDeviceBase::SetBusCount (scope, count);
    }

    UInt32 SupportedNumChannels (const AUChannelInfo** outInfo) override
    {
        if (outInfo != nullptr)
            *outInfo = channelInfo.getRawDataPointer();

        return (UInt32) channelInfo.size();
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
                    outDataSize = sizeof (AudioUnitCocoaViewInfo);
                    outWritable = true;
                    return noErr;

               #if JucePlugin_ProducesMidiOutput || JucePlugin_IsMidiEffect
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

                case kAudioUnitProperty_BypassEffect:
                    outDataSize = sizeof (UInt32);
                    outWritable = true;
                    return noErr;

                default: break;
            }
        }

        return MusicDeviceBase::GetPropertyInfo (inID, inScope, inElement, outDataSize, outWritable);
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

                case kAudioUnitProperty_BypassEffect:
                    *(UInt32*) outData = isBypassed ? 1 : 0;
                    return noErr;

                case kAudioUnitProperty_CocoaUI:
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

               #if JucePlugin_ProducesMidiOutput || JucePlugin_IsMidiEffect
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

        return MusicDeviceBase::GetProperty (inID, inScope, inElement, outData);
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
               #if JucePlugin_ProducesMidiOutput || JucePlugin_IsMidiEffect
                case kAudioUnitProperty_MIDIOutputCallback:
                    if (inDataSize < sizeof (AUMIDIOutputCallbackStruct))
                        return kAudioUnitErr_InvalidPropertyValue;

                    if (AUMIDIOutputCallbackStruct* callbackStruct = (AUMIDIOutputCallbackStruct*) inData)
                        midiCallback = *callbackStruct;

                    return noErr;
               #endif

                case kAudioUnitProperty_BypassEffect:
                {
                    if (inDataSize < sizeof (UInt32))
                        return kAudioUnitErr_InvalidPropertyValue;

                    const bool newBypass = *((UInt32*) inData) != 0;

                    if (newBypass != isBypassed)
                    {
                        isBypassed = newBypass;

                        if (! isBypassed && IsInitialized()) // turning bypass off and we're initialized
                            Reset (0, 0);
                    }

                    return noErr;
                }

                case kAudioUnitProperty_OfflineRender:
                    if (juceFilter != nullptr)
                        juceFilter->setNonRealtime ((*(UInt32*) inData) != 0);

                    return noErr;

                default: break;
            }
        }

        return MusicDeviceBase::SetProperty (inID, inScope, inElement, inData, inDataSize);
    }

    //==============================================================================
    ComponentResult SaveState (CFPropertyListRef* outData) override
    {
        ComponentResult err = MusicDeviceBase::SaveState (outData);

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
            ComponentResult err = MusicDeviceBase::RestoreState (copyWithoutData);
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

    //==============================================================================
    UInt32 GetAudioChannelLayout (AudioUnitScope scope, AudioUnitElement element,
                                  AudioChannelLayout* outLayoutPtr, Boolean& outWritable) override
    {
        bool isInput;
        int busNr;

        outWritable = false;

        if (elementToBusIdx (scope, element, isInput, busNr) != noErr)
            return 0;

        if (busUtils.getSupportedBusLayouts (isInput, busNr).busIgnoresLayout)
            return 0;

        outWritable = true;

        const size_t sizeInBytes = sizeof (AudioChannelLayout) - sizeof (AudioChannelDescription);

        if (outLayoutPtr != nullptr)
        {
            zeromem (outLayoutPtr, sizeInBytes);
            outLayoutPtr->mChannelLayoutTag = getCurrentLayout (isInput, busNr);
        }

        return sizeInBytes;
    }

    UInt32 GetChannelLayoutTags (AudioUnitScope scope, AudioUnitElement element, AudioChannelLayoutTag* outLayoutTags) override
    {
        bool isInput;
        int busNr;

        if (elementToBusIdx (scope, element, isInput, busNr) != noErr)
            return 0;

        if (busUtils.getSupportedBusLayouts (isInput, busNr).busIgnoresLayout)
            return 0;

        const Array<AudioChannelLayoutTag>& layouts = getSupportedBusLayouts (isInput, busNr);

        if (outLayoutTags != nullptr)
            std::copy (layouts.begin(), layouts.end(), outLayoutTags);

        return (UInt32) layouts.size();
    }

    OSStatus SetAudioChannelLayout(AudioUnitScope scope, AudioUnitElement element, const AudioChannelLayout* inLayout) override
    {
        bool isInput;
        int busNr;
        OSStatus err;

        if ((err = elementToBusIdx (scope, element, isInput, busNr)) != noErr)
            return err;

        if (busUtils.getSupportedBusLayouts (isInput, busNr).busIgnoresLayout)
            return kAudioUnitErr_PropertyNotWritable;

        if (inLayout == nullptr)
            return kAudioUnitErr_InvalidPropertyValue;

        if (const AUIOElement* ioElement = GetIOElement (isInput ? kAudioUnitScope_Input :  kAudioUnitScope_Output, element))
        {
            const AudioChannelSet newChannelSet = CoreAudioChannelLayoutToJuceType (*inLayout);
            const int currentNumChannels = static_cast<int> (ioElement->GetStreamFormat().NumberChannels());

            if (currentNumChannels != newChannelSet.size())
                return kAudioUnitErr_InvalidPropertyValue;

            // check if the new layout could be potentially set
            PluginBusUtilities::ScopedBusRestorer restorer (busUtils);

            bool success = juceFilter->setPreferredBusArrangement (isInput, busNr, newChannelSet);

            if (!success)
                return kAudioUnitErr_FormatNotSupported;

            getCurrentLayout (isInput, busNr) = ChannelSetToCALayoutTag (newChannelSet);

            return noErr;
        }
        else
            jassertfalse;

        return kAudioUnitErr_InvalidElement;
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

           #if JucePlugin_AUHighResolutionParameters
            outParameterInfo.flags |= (UInt32) kAudioUnitParameterFlag_IsHighResolution;
           #endif

            const String name (juceFilter->getParameterName (index));

            // set whether the param is automatable (unnamed parameters aren't allowed to be automated)
            if (name.isEmpty() || ! juceFilter->isParameterAutomatable (index))
                outParameterInfo.flags |= kAudioUnitParameterFlag_NonRealTime;

            if (juceFilter->isMetaParameter (index))
                outParameterInfo.flags |= kAudioUnitParameterFlag_IsGlobalMeta;

            MusicDeviceBase::FillInParameterName (outParameterInfo, name.toCFString(), true);

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

        return MusicDeviceBase::GetParameter (inID, inScope, inElement, outValue);
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

        return MusicDeviceBase::SetParameter (inID, inScope, inElement, inValue, inBufferOffsetInFrames);
    }

    // No idea what this method actually does or what it should return. Current Apple docs say nothing about it.
    // (Note that this isn't marked 'override' in case older versions of the SDK don't include it)
    bool CanScheduleParameters() const override          { return false; }

    //==============================================================================
    ComponentResult Version() override                   { return JucePlugin_VersionCode; }
    bool SupportsTail() override                         { return true; }
    Float64 GetTailTime() override                       { return juceFilter->getTailLengthSeconds(); }
    double getSampleRate()                               { return busUtils.getNumEnabledBuses (false) > 0 ? GetOutput(0)->GetStreamFormat().mSampleRate : 44100.0; }

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

        switch (lastTimeStamp.mSMPTETime.mType)
        {
            case kSMPTETimeType24:          info.frameRate = AudioPlayHead::fps24; break;
            case kSMPTETimeType25:          info.frameRate = AudioPlayHead::fps25; break;
            case kSMPTETimeType30Drop:      info.frameRate = AudioPlayHead::fps30drop; break;
            case kSMPTETimeType30:          info.frameRate = AudioPlayHead::fps30; break;
            case kSMPTETimeType2997:        info.frameRate = AudioPlayHead::fps2997; break;
            case kSMPTETimeType2997Drop:    info.frameRate = AudioPlayHead::fps2997drop; break;
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

        double outCurrentSampleInTimeLine, outCycleStartBeat = 0, outCycleEndBeat = 0;
        Boolean playing = false, looping = false, playchanged;

        if (CallHostTransportState (&playing,
                                    &playchanged,
                                    &outCurrentSampleInTimeLine,
                                    &looping,
                                    &outCycleStartBeat,
                                    &outCycleEndBeat) != noErr)
        {
            // If the host doesn't support this callback, then use the sample time from lastTimeStamp:
            outCurrentSampleInTimeLine = lastTimeStamp.mSampleTime;
        }

        info.isPlaying = playing;
        info.timeInSamples = (int64) (outCurrentSampleInTimeLine + 0.5);
        info.timeInSeconds = info.timeInSamples / getSampleRate();
        info.isLooping = looping;
        info.ppqLoopStart = outCycleStartBeat;
        info.ppqLoopEnd = outCycleEndBeat;

        return true;
    }

    void sendAUEvent (const AudioUnitEventType type, const int index)
    {
        auEvent.mEventType = type;
        auEvent.mArgument.mParameter.mParameterID = (AudioUnitParameterID) index;
        AUEventListenerNotify (0, 0, &auEvent);
    }

    void audioProcessorParameterChanged (AudioProcessor*, int index, float /*newValue*/) override
    {
        sendAUEvent (kAudioUnitEvent_ParameterValueChange, index);
    }

    void audioProcessorParameterChangeGestureBegin (AudioProcessor*, int index) override
    {
        sendAUEvent (kAudioUnitEvent_BeginParameterChangeGesture, index);
    }

    void audioProcessorParameterChangeGestureEnd (AudioProcessor*, int index) override
    {
        sendAUEvent (kAudioUnitEvent_EndParameterChangeGesture, index);
    }

    void audioProcessorChanged (AudioProcessor*) override
    {
        PropertyChanged (kAudioUnitProperty_Latency,       kAudioUnitScope_Global, 0);
        PropertyChanged (kAudioUnitProperty_ParameterList, kAudioUnitScope_Global, 0);
        PropertyChanged (kAudioUnitProperty_ParameterInfo, kAudioUnitScope_Global, 0);

        refreshCurrentPreset();

        PropertyChanged (kAudioUnitProperty_PresentPreset, kAudioUnitScope_Global, 0);
    }

    //==============================================================================
    bool StreamFormatWritable (AudioUnitScope scope, AudioUnitElement element) override
    {
        bool ignore;
        int busIdx;

        return ((! IsInitialized()) && (elementToBusIdx (scope, element, ignore, busIdx) == noErr));
    }

    bool ValidFormat (AudioUnitScope scope, AudioUnitElement element, const CAStreamBasicDescription& format) override
    {
        bool isInput;
        int busNr;

        if (elementToBusIdx (scope, element, isInput, busNr) != noErr)
            return false;

        const int newNumChannels = static_cast<int> (format.NumberChannels());
        const int oldNumChannels = busUtils.getNumChannels (isInput, busNr);

        if (newNumChannels == oldNumChannels)
            return true;

        PluginBusUtilities::ScopedBusRestorer restorer (busUtils);

        return juceFilter->setPreferredBusArrangement (isInput, busNr, busUtils.getDefaultLayoutForChannelNumAndBus (isInput, busNr, newNumChannels))
                && MusicDeviceBase::ValidFormat (scope, element, format);
    }

    // AU requires us to override this for the sole reason that we need to find a default layout tag if the number of channels have changed
    OSStatus ChangeStreamFormat (AudioUnitScope scope, AudioUnitElement element, const CAStreamBasicDescription& old, const CAStreamBasicDescription& format) override
    {
        bool isInput;
        int busNr;
        OSStatus err = elementToBusIdx (scope, element, isInput, busNr);

        if (err != noErr)
            return err;

        AudioChannelLayoutTag& currentTag = getCurrentLayout (isInput, busNr);

        const int newNumChannels = static_cast<int> (format.NumberChannels());
        const int oldNumChannels = busUtils.getNumChannels (isInput, busNr);

        // predict channel layout
        AudioChannelSet set = (newNumChannels != oldNumChannels) ? busUtils.getDefaultLayoutForChannelNumAndBus (isInput, busNr, newNumChannels)
                                                                 : busUtils.getChannelSet (isInput, busNr);

        if (set == AudioChannelSet())
            return kAudioUnitErr_FormatNotSupported;

        err = MusicDeviceBase::ChangeStreamFormat (scope, element, old, format);

        if (err == noErr)
            currentTag = ChannelSetToCALayoutTag (set);

        return err;
    }

    //==============================================================================
    ComponentResult Render (AudioUnitRenderActionFlags &ioActionFlags,
                            const AudioTimeStamp& inTimeStamp,
                            const UInt32 nFrames) override
    {
        lastTimeStamp = inTimeStamp;

        const unsigned int numInputBuses  = GetScope (kAudioUnitScope_Input) .GetNumberOfElements();
        const unsigned int numOutputBuses = GetScope (kAudioUnitScope_Output).GetNumberOfElements();

        for (unsigned int i = 0; i < numInputBuses; ++i)
        {
            AudioUnitRenderActionFlags flags = ioActionFlags;
            AUInputElement* input  = GetInput (i);

            OSStatus result = input->PullInput (flags, inTimeStamp, i, nFrames);

            if (result != noErr)
                return noErr; // logic sometimes doesn't connect all the inputs immedietely

            if ((flags & kAudioUnitRenderAction_OutputIsSilence) != 0)
            {
                AudioBufferList& inBuffer = input->GetBufferList();

                for (unsigned int ch = 0; ch < inBuffer.mNumberBuffers; ++ch)
                    zeromem (inBuffer.mBuffers[ch].mData, inBuffer.mBuffers[ch].mDataByteSize);
            }
        }

        {
            int idx = 0, scratchIdx = 0;
            float** scratchBuffers = bufferSpace.getArrayOfWritePointers();

            for (unsigned int busIdx = 0; busIdx < jmax (numInputBuses, numOutputBuses); ++busIdx)
            {
                AUInputElement*  input  = (busIdx < numInputBuses)  ? GetInput (busIdx)  : nullptr;
                AUOutputElement* output = (busIdx < numOutputBuses) ? GetOutput (busIdx) : nullptr;

                const unsigned int numInChannels  = (input != nullptr  ? input ->GetStreamFormat().mChannelsPerFrame : 0);
                const unsigned int numOutChannels = (output != nullptr ? output->GetStreamFormat().mChannelsPerFrame : 0);

                if (numOutChannels > numInChannels)
                {
                    if (output->WillAllocateBuffer())
                        output->PrepareBuffer (nFrames);

                    const AudioBufferList& outBuffer = output->GetBufferList();

                    for (unsigned int chIdx = 0; chIdx < numOutChannels; ++chIdx)
                    {
                        const bool isOutputInterleaved = (numOutChannels > 1) && (outBuffer.mNumberBuffers == 1);
                        float* outData = isOutputInterleaved ? scratchBuffers[scratchIdx++] : static_cast<float*> (outBuffer.mBuffers[chIdx].mData);

                        if (chIdx < numInChannels)
                        {
                            const AudioBufferList& inBuffer = input->GetBufferList();
                            const bool isInputInterleaved = (numInChannels > 1) && (inBuffer.mNumberBuffers == 1);
                            const float* inData = static_cast<float*> (inBuffer.mBuffers[isInputInterleaved ? 0 : chIdx].mData);

                            if (isInputInterleaved)
                            {
                                for (unsigned int i = 0; i < nFrames; ++i)
                                {
                                    outData [i] = inData[chIdx];
                                    inData += numInChannels;
                                }
                            }
                            else
                                std::copy (inData, inData + nFrames, outData);
                        }

                        channels[idx++] = outData;
                    }
                }
                else
                {
                    const AudioBufferList& inBuffer = input->GetBufferList();
                    const bool isInputInterleaved = (numInChannels > 1) && (inBuffer.mNumberBuffers == 1);

                    for (unsigned int chIdx = 0; chIdx < numInChannels; ++chIdx)
                    {
                        float* buffer = isInputInterleaved ? scratchBuffers[scratchIdx++]
                                                           : static_cast<float*> (inBuffer.mBuffers[chIdx].mData);

                        if (isInputInterleaved)
                        {
                            const float* inData = static_cast<float*> (inBuffer.mBuffers[0].mData);
                            for (unsigned int i = 0; i < nFrames; ++i)
                            {
                                buffer [i] = inData [chIdx];
                                inData += numInChannels;
                            }
                        }

                        channels[idx++] = buffer;
                    }
                }
            }

            jassert (idx == bufferSpace.getNumChannels());
        }

        {
            const ScopedLock sl (incomingMidiLock);
            midiEvents.clear();
            incomingEvents.swapWith (midiEvents);
        }

        {
            const ScopedLock sl (juceFilter->getCallbackLock());
            AudioSampleBuffer buffer (channels, bufferSpace.getNumChannels(), (int) nFrames);

            if (juceFilter->isSuspended())
            {
                for (int j = 0; j < buffer.getNumChannels(); ++j)
                    zeromem (channels [j], sizeof (float) * nFrames);
            }
            else if (isBypassed)
            {
                juceFilter->processBlockBypassed (buffer, midiEvents);
            }
            else
            {
                juceFilter->processBlock (buffer, midiEvents);
            }
        }

        // copy output back
        {
            int idx = 0;

            for (unsigned int busIdx = 0; busIdx < jmax (numInputBuses, numOutputBuses); ++busIdx)
            {
                AUInputElement*   input  = (busIdx < numInputBuses)  ? GetInput (busIdx)  : nullptr;
                AUOutputElement* output  = (busIdx < numOutputBuses) ? GetOutput (busIdx) : nullptr;

                const unsigned int numInChannels  = (input != nullptr  ? input ->GetStreamFormat().mChannelsPerFrame : 0);
                const unsigned int numOutChannels = (output != nullptr ? output->GetStreamFormat().mChannelsPerFrame : 0);

                if (numOutChannels > 0 && numInChannels >= numOutChannels)
                {
                    // the input buffers were used. We must copy the output
                    if (output->WillAllocateBuffer())
                        output->PrepareBuffer (nFrames);

                    const AudioBufferList& outBuffer = output->GetBufferList();
                    const bool isOutputInterleaved = (numOutChannels > 1) && (outBuffer.mNumberBuffers == 1);

                    for (unsigned int chIdx = 0; chIdx < numOutChannels; ++chIdx)
                    {
                        float* outData = static_cast<float*> (outBuffer.mBuffers[isOutputInterleaved ? 0 : chIdx].mData);
                        const float* buffer = static_cast<float*> (channels [idx++]);

                        if (isOutputInterleaved)
                        {
                            for (unsigned int i = 0; i < nFrames; ++i)
                            {
                                outData [chIdx] = buffer[i];
                                outData += numOutChannels;
                            }
                        }
                        else
                            std::copy (buffer, buffer + nFrames, outData);
                    }
                    idx += numInChannels - numOutChannels;
                }
            }
        }

        if (! midiEvents.isEmpty())
        {
           #if JucePlugin_ProducesMidiOutput || JucePlugin_IsMidiEffect
            if (midiCallback.midiOutputCallback != nullptr)
            {
                UInt32 numPackets = 0;
                size_t dataSize = 0;

                const juce::uint8* midiEventData;
                int midiEventSize, midiEventPosition;

                for (MidiBuffer::Iterator i (midiEvents); i.getNextEvent (midiEventData, midiEventSize, midiEventPosition);)
                {
                    jassert (isPositiveAndBelow (midiEventPosition, (int) nFrames));
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

       #if ! JucePlugin_SilenceInProducesSilenceOut
        ioActionFlags &= (AudioUnitRenderActionFlags) ~kAudioUnitRenderAction_OutputIsSilence;
       #else
        ignoreUnused (ioActionFlags);
       #endif

        return noErr;
    }

    //==============================================================================
    ComponentResult StartNote (MusicDeviceInstrumentID, MusicDeviceGroupID, NoteInstanceID*, UInt32, const MusicDeviceNoteParams&) override { return noErr; }
    ComponentResult StopNote (MusicDeviceGroupID, NoteInstanceID, UInt32) override   { return noErr; }

    //==============================================================================
    OSStatus HandleMidiEvent (UInt8 nStatus, UInt8 inChannel, UInt8 inData1, UInt8 inData2, UInt32 inStartFrame) override
    {
       #if JucePlugin_WantsMidiInput || JucePlugin_IsMidiEffect
        const juce::uint8 data[] = { (juce::uint8) (nStatus | inChannel),
                                     (juce::uint8) inData1,
                                     (juce::uint8) inData2 };

        const ScopedLock sl (incomingMidiLock);
        incomingEvents.addEvent (data, 3, (int) inStartFrame);
        return noErr;
       #else
        ignoreUnused (nStatus, inChannel, inData1);
        ignoreUnused (inData2, inStartFrame);
        return kAudioUnitErr_PropertyNotInUse;
       #endif
    }

    OSStatus HandleSysEx (const UInt8* inData, UInt32 inLength) override
    {
       #if JucePlugin_WantsMidiInput || JucePlugin_IsMidiEffect
        const ScopedLock sl (incomingMidiLock);
        incomingEvents.addEvent (inData, (int) inLength, 0);
        return noErr;
       #else
        ignoreUnused (inData, inLength);
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
    AudioSampleBuffer bufferSpace;
    HeapBlock<float*> channels;
    MidiBuffer midiEvents, incomingEvents;
    bool prepared, isBypassed;
    AudioUnitEvent auEvent;
    mutable Array<AUPreset> presetsArray;
    CriticalSection incomingMidiLock;
    AUMIDIOutputCallbackStruct midiCallback;
    AudioTimeStamp lastTimeStamp;
    PluginBusUtilities busUtils;

    //==============================================================================
    Array<AUChannelInfo> channelInfo;
    Array<Array<AudioChannelLayoutTag> > supportedInputLayouts, supportedOutputLayouts;
    Array<AudioChannelLayoutTag> currentInputLayout, currentOutputLayout;

    //==============================================================================
    static OSStatus scopeToDirection (AudioUnitScope scope, bool& isInput) noexcept
    {
        isInput = (scope == kAudioUnitScope_Input);

        return (scope != kAudioUnitScope_Input
             && scope != kAudioUnitScope_Output)
              ? kAudioUnitErr_InvalidScope : noErr;
    }

    OSStatus elementToBusIdx (AudioUnitScope scope, AudioUnitElement element, bool& isInput, int& busIdx) noexcept
    {
        OSStatus err;

        busIdx = static_cast<int> (element);

        if ((err = scopeToDirection (scope, isInput)) != noErr) return err;
        if (isPositiveAndBelow (busIdx, busUtils.getBusCount (isInput))) return noErr;

        return kAudioUnitErr_InvalidElement;
    }

    //==============================================================================
    OSStatus syncAudioUnitWithProcessor()
    {
        OSStatus err = noErr;
        const int enabledInputs  = busUtils.getNumEnabledBuses (true);
        const int enabledOutputs = busUtils.getNumEnabledBuses (false);

        if ((err =  MusicDeviceBase::SetBusCount (kAudioUnitScope_Input,  static_cast<UInt32> (enabledInputs))) != noErr)
            return err;

        if ((err =  MusicDeviceBase::SetBusCount (kAudioUnitScope_Output, static_cast<UInt32> (enabledOutputs))) != noErr)
            return err;

        addSupportedLayoutTags();

        for (int i = 0; i < juceFilter->busArrangement.inputBuses.size(); ++i)
            if ((err = syncAudioUnitWithChannelSet (true, i,  busUtils.getChannelSet (true,  i))) != noErr) return err;

        for (int i = 0; i < juceFilter->busArrangement.outputBuses.size(); ++i)
            if ((err = syncAudioUnitWithChannelSet (false, i, busUtils.getChannelSet (false, i))) != noErr) return err;

        // if you are hitting this assertion then your plug-in allows disabling/enabling buses (i.e. you
        // do not return false in setPreferredBusArrangement when the number of channels is zero), however,
        // AudioUnits require at least the main bus to be enabled by default in this case. Please assign
        // a non-zero number of channels to your main input or output bus in the constructor of your AudioProcessor
        jassert ((! busUtils.hasDynamicInBuses() && ! busUtils.hasDynamicOutBuses()) || (enabledInputs > 0) || (enabledOutputs > 0));

        return noErr;
    }

    OSStatus syncProcessorWithAudioUnit()
    {
        OSStatus err;
        const int numInputElements  = static_cast<int> (GetScope(kAudioUnitScope_Input). GetNumberOfElements());
        const int numOutputElements = static_cast<int> (GetScope(kAudioUnitScope_Output).GetNumberOfElements());

        for (int i = 0; i < numInputElements; ++i)
            if ((err = syncProcessorWithAudioUnitForBus (true, i)) != noErr) return err;

        for (int i = 0; i < numOutputElements; ++i)
            if ((err = syncProcessorWithAudioUnitForBus (false, i)) != noErr) return err;

        if (numInputElements != busUtils.getNumEnabledBuses (true) || numOutputElements != busUtils.getNumEnabledBuses (false))
            return kAudioUnitErr_FormatNotSupported;

        // re-check the format of all buses to see if it matches what CoreAudio actually requested
        for (int i = 0; i < busUtils.getNumEnabledBuses (true); ++i)
            if (! audioUnitAndProcessorIsFormatMatching (true, i)) return kAudioUnitErr_FormatNotSupported;

        for (int i = 0; i < busUtils.getNumEnabledBuses (false); ++i)
            if (! audioUnitAndProcessorIsFormatMatching (false, i)) return kAudioUnitErr_FormatNotSupported;

        return noErr;
    }

    //==============================================================================
    OSStatus syncProcessorWithAudioUnitForBus (bool isInput, int busNr)
    {
        if (const AUIOElement* element = GetIOElement (isInput ? kAudioUnitScope_Input :  kAudioUnitScope_Output, (UInt32) busNr))
        {
            const int numChannels = static_cast<int> (element->GetStreamFormat().NumberChannels());

            AudioChannelLayoutTag currentLayoutTag = isInput ? currentInputLayout[busNr] : currentOutputLayout[busNr];
            const int tagNumChannels = currentLayoutTag & 0xffff;

            if (numChannels != tagNumChannels)
                return kAudioUnitErr_FormatNotSupported;

            if (juceFilter->setPreferredBusArrangement (isInput, busNr, CALayoutTagToChannelSet(currentLayoutTag)))
                return noErr;
        }
        else
            jassertfalse;

        return kAudioUnitErr_FormatNotSupported;
    }

    OSStatus syncAudioUnitWithChannelSet (bool isInput, int busNr, const AudioChannelSet& channelSet)
    {
        const int numChannels = channelSet.size();

        // is this bus activated?
        if (numChannels == 0)
            return noErr;

        if (AUIOElement* element = GetIOElement (isInput ? kAudioUnitScope_Input :  kAudioUnitScope_Output, (UInt32) busNr))
        {
            getCurrentLayout (isInput, busNr) = ChannelSetToCALayoutTag (channelSet);

            element->SetName ((CFStringRef) juceStringToNS (busUtils.getFilterBus (isInput).getReference (busNr).name));

            CAStreamBasicDescription streamDescription;
            streamDescription.mSampleRate = getSampleRate();

            streamDescription.SetCanonical ((UInt32) numChannels, false);
            return element->SetStreamFormat (streamDescription);
        }
        else
            jassertfalse;

        return kAudioUnitErr_InvalidElement;
    }

    //==============================================================================
    bool audioUnitAndProcessorIsFormatMatching (bool isInput, int busNr)
    {
        const AudioProcessor::AudioProcessorBus& bus = isInput ? juceFilter->busArrangement.inputBuses. getReference (busNr)
        : juceFilter->busArrangement.outputBuses.getReference (busNr);

        if (const AUIOElement* element = GetIOElement (isInput ? kAudioUnitScope_Input :  kAudioUnitScope_Output, (UInt32) busNr))
        {
            const int numChannels = static_cast<int> (element->GetStreamFormat().NumberChannels());

            return (numChannels == bus.channels.size());
        }
        else
            jassertfalse;

        return false;
    }

    //==============================================================================
    void populateAUChannelInfo()
    {
        channelInfo.clear();

        const AudioProcessor::AudioBusArrangement& arr = juceFilter->busArrangement;
        PluginBusUtilities::ScopedBusRestorer restorer (busUtils);

        const bool hasMainInputBus  = (busUtils.getNumEnabledBuses (true)  > 0);
        const bool hasMainOutputBus = (busUtils.getNumEnabledBuses (false) > 0);

        if ((! hasMainInputBus)  && (! hasMainOutputBus))
        {
            // midi effect plug-in: no audio
            AUChannelInfo info;
            info.inChannels = 0;
            info.outChannels = 0;

            channelInfo.add (info);
            return;
        }
        else
        {
            const uint32_t maxNumChanToCheckFor = 9;

            uint32_t defaultInputs  = static_cast<uint32_t> (busUtils.getNumChannels (true,  0));
            uint32_t defaultOutputs = static_cast<uint32_t> (busUtils.getNumChannels (false, 0));

            uint32_t lastInputs  = defaultInputs;
            uint32_t lastOutputs = defaultOutputs;

            SortedSet<uint32_t> supportedChannels;

            // add the current configuration
            if (lastInputs != 0 || lastOutputs != 0)
                supportedChannels.add ((lastInputs << 16) | lastOutputs);

            for (uint32_t inChanNum = hasMainInputBus ? 1 : 0; inChanNum <= (hasMainInputBus ? maxNumChanToCheckFor : 0); ++inChanNum)
            {
                const AudioChannelSet* dfltInLayout = nullptr;

                if (inChanNum != 0 && (dfltInLayout = busUtils.getSupportedBusLayouts (true, 0).getDefaultLayoutForChannelNum (static_cast<int> (inChanNum))) == nullptr)
                    continue;

                for (uint32_t outChanNum = hasMainOutputBus ? 1 : 0; outChanNum <= (hasMainOutputBus ? maxNumChanToCheckFor : 0); ++outChanNum)
                {
                    const AudioChannelSet* dfltOutLayout = nullptr;

                    if (outChanNum != 0 && (dfltOutLayout = busUtils.getSupportedBusLayouts (false, 0).getDefaultLayoutForChannelNum (static_cast<int> (outChanNum))) == nullptr)
                        continue;

                    // get the number of channels again. This is only needed for some processors that change their configuration
                    // even when they indicate that setPreferredBusArrangement failed.
                    lastInputs  = hasMainInputBus  ? static_cast<uint32_t> (arr.inputBuses. getReference (0). channels.size()) : 0;
                    lastOutputs = hasMainOutputBus ? static_cast<uint32_t> (arr.outputBuses.getReference (0). channels.size()) : 0;

                    uint32_t channelConfiguration = (inChanNum << 16) | outChanNum;

                    // did we already try this configuration?
                    if (supportedChannels.contains (channelConfiguration)) continue;

                    if (lastInputs != inChanNum && dfltInLayout != nullptr)
                    {
                        if (! juceFilter->setPreferredBusArrangement (true, 0, *dfltInLayout)) continue;

                        lastInputs = inChanNum;
                        lastOutputs = hasMainOutputBus ? static_cast<uint32_t> (arr.outputBuses.getReference (0). channels.size()) : 0;

                        supportedChannels.add ((lastInputs << 16) | lastOutputs);
                    }

                    if (lastOutputs != outChanNum && dfltOutLayout != nullptr)
                    {
                        if (! juceFilter->setPreferredBusArrangement (false, 0, *dfltOutLayout)) continue;

                        lastInputs = hasMainInputBus ? static_cast<uint32_t> (arr.inputBuses.getReference (0).channels.size()) : 0;
                        lastOutputs = outChanNum;

                        supportedChannels.add ((lastInputs << 16) | lastOutputs);
                    }
                }
            }

            bool hasInOutMismatch = false;
            for (int i = 0; i < supportedChannels.size(); ++i)
            {
                const uint32_t numInputs  = (supportedChannels[i] >> 16) & 0xffff;
                const uint32_t numOutputs = (supportedChannels[i] >> 0)  & 0xffff;

                if (numInputs != numOutputs)
                {
                    hasInOutMismatch = true;
                    break;
                }
            }

            bool hasUnsupportedInput = ! hasMainOutputBus, hasUnsupportedOutput = ! hasMainInputBus;
            for (uint32_t inChanNum = hasMainInputBus ? 1 : 0; inChanNum <= (hasMainInputBus ? maxNumChanToCheckFor : 0); ++inChanNum)
            {
                uint32_t channelConfiguration = (inChanNum << 16) | (hasInOutMismatch ? defaultOutputs : inChanNum);
                if (! supportedChannels.contains (channelConfiguration))
                {
                    hasUnsupportedInput = true;
                    break;
                }
            }

            for (uint32_t outChanNum = hasMainOutputBus ? 1 : 0; outChanNum <= (hasMainOutputBus ? maxNumChanToCheckFor : 0); ++outChanNum)
            {
                uint32_t channelConfiguration = ((hasInOutMismatch ? defaultInputs : outChanNum) << 16) | outChanNum;
                if (! supportedChannels.contains (channelConfiguration))
                {
                    hasUnsupportedOutput = true;
                    break;
                }
            }

            for (int i = 0; i < supportedChannels.size(); ++i)
            {
                const int numInputs  = (supportedChannels[i] >> 16) & 0xffff;
                const int numOutputs = (supportedChannels[i] >> 0)  & 0xffff;

                AUChannelInfo info;

                // see here: https://developer.apple.com/library/mac/documentation/MusicAudio/Conceptual/AudioUnitProgrammingGuide/TheAudioUnit/TheAudioUnit.html
                info.inChannels  = static_cast<SInt16> (hasMainInputBus  ? (hasUnsupportedInput  ? numInputs :  (hasInOutMismatch && (! hasUnsupportedOutput) ? -2 : -1)) : 0);
                info.outChannels = static_cast<SInt16> (hasMainOutputBus ? (hasUnsupportedOutput ? numOutputs : (hasInOutMismatch && (! hasUnsupportedInput)  ? -2 : -1)) : 0);

                if (info.inChannels == -2 && info.outChannels == -2)
                    info.inChannels = -1;

                int j;
                for (j = 0; j < channelInfo.size(); ++j)
                    if (channelInfo[j].inChannels == info.inChannels && channelInfo[j].outChannels == info.outChannels)
                        break;

                if (j >= channelInfo.size())
                    channelInfo.add (info);
            }
        }
    }

    //==============================================================================
    void clearPresetsArray() const
    {
        for (int i = presetsArray.size(); --i >= 0;)
            CFRelease (presetsArray.getReference(i).presetName);

        presetsArray.clear();
    }

    void refreshCurrentPreset()
    {
        // this will make the AU host re-read and update the current preset name
        // in case it was changed here in the plug-in:

        const int currentProgramNumber = juceFilter->getCurrentProgram();
        const String currentProgramName = juceFilter->getProgramName (currentProgramNumber);

        AUPreset currentPreset;
        currentPreset.presetNumber = currentProgramNumber;
        currentPreset.presetName = currentProgramName.toCFString();

        SetAFactoryPresetAsCurrent (currentPreset);
    }

    //==============================================================================
    Array<AudioChannelLayoutTag>&       getSupportedBusLayouts (bool isInput, int bus) noexcept       { return (isInput ? supportedInputLayouts : supportedOutputLayouts).getReference (bus); }
    const Array<AudioChannelLayoutTag>& getSupportedBusLayouts (bool isInput, int bus) const noexcept { return (isInput ? supportedInputLayouts : supportedOutputLayouts).getReference (bus); }
    AudioChannelLayoutTag& getCurrentLayout (bool isInput, int bus) noexcept               { return (isInput ? currentInputLayout : currentOutputLayout).getReference (bus); }
    AudioChannelLayoutTag  getCurrentLayout (bool isInput, int bus) const noexcept         { return (isInput ? currentInputLayout : currentOutputLayout)[bus]; }

    bool toggleBus (bool isInput, int busIdx)
    {
        const PluginBusUtilities::SupportedBusLayouts& layouts = busUtils.getSupportedBusLayouts (isInput, busIdx);

        if (! layouts.canBeDisabled)
            return false;

        AudioChannelSet newSet;

        if (! busUtils.isBusEnabled (isInput, busIdx))
            newSet = layouts.getDefault();

        return juceFilter->setPreferredBusArrangement (isInput, busIdx, newSet);
    }

    //==============================================================================
    static AudioChannelSet::ChannelType CoreAudioChannelLabelToJuceType (AudioChannelLabel label) noexcept
    {
        if (label >= kAudioChannelLabel_Discrete_0 && label <= kAudioChannelLabel_Discrete_65535)
        {
            const unsigned int discreteChannelNum = label - kAudioChannelLabel_Discrete_0;
            return static_cast<AudioChannelSet::ChannelType> (AudioChannelSet::discreteChannel0 + discreteChannelNum);
        }

        switch (label)
        {
            case kAudioChannelLabel_Center:
            case kAudioChannelLabel_Mono:                   return AudioChannelSet::centre;
            case kAudioChannelLabel_Left:
            case kAudioChannelLabel_HeadphonesLeft:         return AudioChannelSet::left;
            case kAudioChannelLabel_Right:
            case kAudioChannelLabel_HeadphonesRight:        return AudioChannelSet::right;
            case kAudioChannelLabel_LFEScreen:              return AudioChannelSet::subbass;
            case kAudioChannelLabel_LeftSurround:           return AudioChannelSet::surroundLeft;
            case kAudioChannelLabel_RightSurround:          return AudioChannelSet::surroundRight;
            case kAudioChannelLabel_LeftCenter:             return AudioChannelSet::centreLeft;
            case kAudioChannelLabel_RightCenter:            return AudioChannelSet::centreRight;
            case kAudioChannelLabel_CenterSurround:         return AudioChannelSet::surround;
            case kAudioChannelLabel_LeftSurroundDirect:     return AudioChannelSet::sideLeft;
            case kAudioChannelLabel_RightSurroundDirect:    return AudioChannelSet::sideRight;
            case kAudioChannelLabel_TopCenterSurround:      return AudioChannelSet::topMiddle;
            case kAudioChannelLabel_VerticalHeightLeft:     return AudioChannelSet::topFrontLeft;
            case kAudioChannelLabel_VerticalHeightRight:    return AudioChannelSet::topFrontRight;
            case kAudioChannelLabel_VerticalHeightCenter:   return AudioChannelSet::topFrontCentre;
            case kAudioChannelLabel_TopBackLeft:
            case kAudioChannelLabel_RearSurroundLeft:       return AudioChannelSet::topRearLeft;
            case kAudioChannelLabel_TopBackRight:
            case kAudioChannelLabel_RearSurroundRight:      return AudioChannelSet::topRearRight;
            case kAudioChannelLabel_TopBackCenter:          return AudioChannelSet::topRearCentre;
            case kAudioChannelLabel_LFE2:                   return AudioChannelSet::subbass2;
            default:                                        return AudioChannelSet::unknown;
        }
    }

    static AudioChannelSet CoreAudioChannelBitmapToJuceType (UInt32 bitmap) noexcept
    {
        AudioChannelSet set;

        if ((bitmap & kAudioChannelBit_Left)                 != 0) set.addChannel (AudioChannelSet::left);
        if ((bitmap & kAudioChannelBit_Right)                != 0) set.addChannel (AudioChannelSet::right);
        if ((bitmap & kAudioChannelBit_Center)               != 0) set.addChannel (AudioChannelSet::centre);
        if ((bitmap & kAudioChannelBit_LFEScreen)            != 0) set.addChannel (AudioChannelSet::subbass);
        if ((bitmap & kAudioChannelBit_LeftSurround)         != 0) set.addChannel (AudioChannelSet::surroundLeft);
        if ((bitmap & kAudioChannelBit_RightSurround)        != 0) set.addChannel (AudioChannelSet::surroundRight);
        if ((bitmap & kAudioChannelBit_LeftCenter)           != 0) set.addChannel (AudioChannelSet::centreLeft);
        if ((bitmap & kAudioChannelBit_RightCenter)          != 0) set.addChannel (AudioChannelSet::centreRight);
        if ((bitmap & kAudioChannelBit_CenterSurround)       != 0) set.addChannel (AudioChannelSet::surround);
        if ((bitmap & kAudioChannelBit_LeftSurroundDirect)   != 0) set.addChannel (AudioChannelSet::sideLeft);
        if ((bitmap & kAudioChannelBit_RightSurroundDirect)  != 0) set.addChannel (AudioChannelSet::sideRight);
        if ((bitmap & kAudioChannelBit_TopCenterSurround)    != 0) set.addChannel (AudioChannelSet::topMiddle);
        if ((bitmap & kAudioChannelBit_VerticalHeightLeft)   != 0) set.addChannel (AudioChannelSet::topFrontLeft);
        if ((bitmap & kAudioChannelBit_VerticalHeightCenter) != 0) set.addChannel (AudioChannelSet::topFrontCentre);
        if ((bitmap & kAudioChannelBit_VerticalHeightRight)  != 0) set.addChannel (AudioChannelSet::topFrontRight);
        if ((bitmap & kAudioChannelBit_TopBackLeft)          != 0) set.addChannel (AudioChannelSet::topRearLeft);
        if ((bitmap & kAudioChannelBit_TopBackCenter)        != 0) set.addChannel (AudioChannelSet::topRearCentre);
        if ((bitmap & kAudioChannelBit_TopBackRight)         != 0) set.addChannel (AudioChannelSet::topRearRight);

        return set;
    }

    static AudioChannelSet CoreAudioChannelLayoutToJuceType (const AudioChannelLayout& layout) noexcept
    {
        const AudioChannelLayoutTag tag = layout.mChannelLayoutTag;

        if (tag == kAudioChannelLayoutTag_UseChannelBitmap)         return CoreAudioChannelBitmapToJuceType (layout.mChannelBitmap);
        if (tag == kAudioChannelLayoutTag_UseChannelDescriptions)
        {
            AudioChannelSet set;
            for (unsigned int i = 0; i < layout.mNumberChannelDescriptions; ++i)
                set.addChannel (CoreAudioChannelLabelToJuceType (layout.mChannelDescriptions[i].mChannelLabel));

            return set;
        }

        return CALayoutTagToChannelSet (tag);
    }

    static AudioChannelSet CALayoutTagToChannelSet (AudioChannelLayoutTag tag) noexcept
    {
        switch (tag)
        {
            case kAudioChannelLayoutTag_Mono:                   return AudioChannelSet::mono();
            case kAudioChannelLayoutTag_Stereo:
            case kAudioChannelLayoutTag_StereoHeadphones:
            case kAudioChannelLayoutTag_Binaural:               return AudioChannelSet::stereo();
            case kAudioChannelLayoutTag_Quadraphonic:           return AudioChannelSet::quadraphonic();
            case kAudioChannelLayoutTag_Pentagonal:             return AudioChannelSet::pentagonal();
            case kAudioChannelLayoutTag_Hexagonal:              return AudioChannelSet::hexagonal();
            case kAudioChannelLayoutTag_Octagonal:              return AudioChannelSet::octagonal();
            case kAudioChannelLayoutTag_Ambisonic_B_Format:     return AudioChannelSet::ambisonic();
            case kAudioChannelLayoutTag_AudioUnit_6_0:          return AudioChannelSet::create6point0();
            case kAudioChannelLayoutTag_MPEG_6_1_A:             return AudioChannelSet::create6point1();
            case kAudioChannelLayoutTag_MPEG_5_0_B:             return AudioChannelSet::create5point0();
            case kAudioChannelLayoutTag_MPEG_5_1_A:             return AudioChannelSet::create5point1();
            case kAudioChannelLayoutTag_DTS_7_1:
            case kAudioChannelLayoutTag_MPEG_7_1_C:             return AudioChannelSet::create7point1();
            case kAudioChannelLayoutTag_AudioUnit_7_0_Front:    return AudioChannelSet::createFront7point0();
            case kAudioChannelLayoutTag_AudioUnit_7_1_Front:    return AudioChannelSet::createFront7point1();
        }

        if (int numChannels = static_cast<int> (tag) & 0xffff)
            return AudioChannelSet::discreteChannels (numChannels);

        // Bitmap and channel description array layout tags are currently unsupported :-(
        jassertfalse;
        return AudioChannelSet();
    }

    static AudioChannelLayoutTag ChannelSetToCALayoutTag (const AudioChannelSet& set) noexcept
    {
        if (set == AudioChannelSet::mono())               return kAudioChannelLayoutTag_Mono;
        if (set == AudioChannelSet::stereo())             return kAudioChannelLayoutTag_Stereo;
        if (set == AudioChannelSet::quadraphonic())       return kAudioChannelLayoutTag_Quadraphonic;
        if (set == AudioChannelSet::pentagonal())         return kAudioChannelLayoutTag_Pentagonal;
        if (set == AudioChannelSet::hexagonal())          return kAudioChannelLayoutTag_Hexagonal;
        if (set == AudioChannelSet::octagonal())          return kAudioChannelLayoutTag_Octagonal;
        if (set == AudioChannelSet::ambisonic())          return kAudioChannelLayoutTag_Ambisonic_B_Format;
        if (set == AudioChannelSet::create5point0())      return kAudioChannelLayoutTag_MPEG_5_0_B;
        if (set == AudioChannelSet::create5point1())      return kAudioChannelLayoutTag_MPEG_5_1_A;
        if (set == AudioChannelSet::create6point0())      return kAudioChannelLayoutTag_AudioUnit_6_0;
        if (set == AudioChannelSet::create6point1())      return kAudioChannelLayoutTag_MPEG_6_1_A;
        if (set == AudioChannelSet::create7point0())      return kAudioChannelLayoutTag_AudioUnit_7_0;
        if (set == AudioChannelSet::create7point1())      return kAudioChannelLayoutTag_MPEG_7_1_C;
        if (set == AudioChannelSet::createFront7point0()) return kAudioChannelLayoutTag_AudioUnit_7_0_Front;
        if (set == AudioChannelSet::createFront7point1()) return kAudioChannelLayoutTag_AudioUnit_7_1_Front;

        return static_cast<AudioChannelLayoutTag> ((int) kAudioChannelLayoutTag_DiscreteInOrder | set.size());
    }

    //==============================================================================
    void addSupportedLayoutTagsForBus (bool isInput, int busNum, Array<AudioChannelLayoutTag>& tags)
    {
        const PluginBusUtilities::SupportedBusLayouts& layouts = busUtils.getSupportedBusLayouts (isInput, busNum);

        if (! layouts.busIgnoresLayout)
            for (int i = 0; i < layouts.supportedLayouts.size(); ++i)
                tags.add (ChannelSetToCALayoutTag (layouts.supportedLayouts.getReference (i)));
    }

    void addSupportedLayoutTagsForDirection (bool isInput)
    {
        Array<Array<AudioChannelLayoutTag> >& layouts = isInput ? supportedInputLayouts : supportedOutputLayouts;
        layouts.clear();

        for (int busNr = 0; busNr < busUtils.getBusCount (isInput); ++busNr)
        {
            Array<AudioChannelLayoutTag> busLayouts;
            addSupportedLayoutTagsForBus (isInput, busNr, busLayouts);

            layouts.add (busLayouts);
        }
    }

    void addSupportedLayoutTags()
    {
        currentInputLayout.clear(); currentOutputLayout.clear();

        currentInputLayout. resize (juceFilter->busArrangement.inputBuses. size());
        currentOutputLayout.resize (juceFilter->busArrangement.outputBuses.size());

        addSupportedLayoutTagsForDirection (true);
        addSupportedLayoutTagsForDirection (false);
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

#if JucePlugin_ProducesMidiOutput || JucePlugin_WantsMidiInput || JucePlugin_IsMidiEffect
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
 #include "CoreAudioUtilityClasses/AUPlugInDispatch.cpp"
#endif

#endif

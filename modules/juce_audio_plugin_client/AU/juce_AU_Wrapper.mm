/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#include <juce_core/system/juce_TargetPlatform.h>
#include <juce_core/system/juce_CompilerWarnings.h>
#include "../utility/juce_CheckSettingMacros.h"

#if JucePlugin_Build_AU

#if __LP64__
 #undef JUCE_SUPPORT_CARBON
 #define JUCE_SUPPORT_CARBON 0
#endif

JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wshorten-64-to-32",
                                     "-Wunused-parameter",
                                     "-Wdeprecated-declarations",
                                     "-Wsign-conversion",
                                     "-Wconversion",
                                     "-Woverloaded-virtual",
                                     "-Wextra-semi",
                                     "-Wcast-align",
                                     "-Wshadow",
                                     "-Wswitch-enum",
                                     "-Wzero-as-null-pointer-constant",
                                     "-Wnullable-to-nonnull-conversion",
                                     "-Wgnu-zero-variadic-macro-arguments",
                                     "-Wformat-pedantic",
                                     "-Wdeprecated-anon-enum-enum-conversion")

#include "../utility/juce_IncludeSystemHeaders.h"

#include <AudioUnit/AUCocoaUIView.h>
#include <AudioUnit/AudioUnit.h>
#include <AudioToolbox/AudioUnitUtilities.h>
#include <CoreMIDI/MIDIServices.h>
#include <QuartzCore/QuartzCore.h>
#include "CoreAudioUtilityClasses/MusicDeviceBase.h"

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
 #include "CoreAudioUtilityClasses/AUCarbonViewBase.h"
#endif

JUCE_END_IGNORE_WARNINGS_GCC_LIKE

#define JUCE_MAC_WINDOW_VISIBITY_BODGE 1
#define JUCE_CORE_INCLUDE_OBJC_HELPERS 1

#include "../utility/juce_IncludeModuleHeaders.h"
#include "../utility/juce_FakeMouseMoveGenerator.h"
#include "../utility/juce_CarbonVisibility.h"

#include <juce_audio_basics/native/juce_mac_CoreAudioLayouts.h>
#include <juce_audio_processors/format_types/juce_LegacyAudioParameter.cpp>
#include <juce_audio_processors/format_types/juce_AU_Shared.h>

//==============================================================================
using namespace juce;

static Array<void*> activePlugins, activeUIs;

static const AudioUnitPropertyID juceFilterObjectPropertyID = 0x1a45ffe9;

template <> struct ContainerDeletePolicy<const __CFString>   { static void destroy (const __CFString* o) { if (o != nullptr) CFRelease (o); } };

// make sure the audio processor is initialized before the AUBase class
struct AudioProcessorHolder
{
    AudioProcessorHolder (bool initialiseGUI)
    {
        if (initialiseGUI)
        {
           #if BUILD_AU_CARBON_UI
            NSApplicationLoad();
           #endif

            initialiseJuce_GUI();
        }

        juceFilter.reset (createPluginFilterOfType (AudioProcessor::wrapperType_AudioUnit));

        // audio units do not have a notion of enabled or un-enabled buses
        juceFilter->enableAllBuses();
    }

    std::unique_ptr<AudioProcessor> juceFilter;
};

//==============================================================================
class JuceAU   : public AudioProcessorHolder,
                 public MusicDeviceBase,
                 public AudioProcessorListener,
                 public AudioPlayHead,
                 public AudioProcessorParameter::Listener
{
public:
    JuceAU (AudioUnit component)
        : AudioProcessorHolder (activePlugins.size() + activeUIs.size() == 0),
          MusicDeviceBase (component,
                           (UInt32) AudioUnitHelpers::getBusCount (juceFilter.get(), true),
                           (UInt32) AudioUnitHelpers::getBusCount (juceFilter.get(), false)),
          mapper (*juceFilter)
    {
        inParameterChangedCallback = false;

       #ifdef JucePlugin_PreferredChannelConfigurations
        short configs[][2] = {JucePlugin_PreferredChannelConfigurations};
        const int numConfigs = sizeof (configs) / sizeof (short[2]);

        jassert (numConfigs > 0 && (configs[0][0] > 0 || configs[0][1] > 0));
        juceFilter->setPlayConfigDetails (configs[0][0], configs[0][1], 44100.0, 1024);

        for (int i = 0; i < numConfigs; ++i)
        {
            AUChannelInfo info;

            info.inChannels  = configs[i][0];
            info.outChannels = configs[i][1];

            channelInfo.add (info);
        }
       #else
        channelInfo = AudioUnitHelpers::getAUChannelInfo (*juceFilter);
       #endif

        AddPropertyListener (kAudioUnitProperty_ContextName, auPropertyListenerDispatcher, this);

        totalInChannels  = juceFilter->getTotalNumInputChannels();
        totalOutChannels = juceFilter->getTotalNumOutputChannels();

        juceFilter->setPlayHead (this);
        juceFilter->addListener (this);

        addParameters();

        activePlugins.add (this);

        zerostruct (auEvent);
        auEvent.mArgument.mParameter.mAudioUnit = GetComponentInstance();
        auEvent.mArgument.mParameter.mScope = kAudioUnitScope_Global;
        auEvent.mArgument.mParameter.mElement = 0;

        zerostruct (midiCallback);

        CreateElements();

        if (syncAudioUnitWithProcessor() != noErr)
            jassertfalse;
    }

    ~JuceAU() override
    {
        if (bypassParam != nullptr)
            bypassParam->removeListener (this);

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

        if ((err = syncProcessorWithAudioUnit()) != noErr)
            return err;

        if ((err = MusicDeviceBase::Initialize()) != noErr)
            return err;

        mapper.alloc();
        pulledSucceeded.calloc (static_cast<size_t> (AudioUnitHelpers::getBusCount (juceFilter.get(), true)));

        prepareToPlay();

        return noErr;
    }

    void Cleanup() override
    {
        MusicDeviceBase::Cleanup();

        pulledSucceeded.free();
        mapper.release();

        if (juceFilter != nullptr)
            juceFilter->releaseResources();

        audioBuffer.release();
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

            audioBuffer.prepare (totalInChannels, totalOutChannels, (int) GetMaxFramesPerSlice() + 32);
            juceFilter->prepareToPlay (getSampleRate(), (int) GetMaxFramesPerSlice());

            midiEvents.ensureSize (2048);
            midiEvents.clear();
            incomingEvents.ensureSize (2048);
            incomingEvents.clear();

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
       #ifdef JucePlugin_PreferredChannelConfigurations
        ignoreUnused (scope);

        return false;
       #else
        bool isInput;

        if (scopeToDirection (scope, isInput) != noErr)
            return false;

       #if JucePlugin_IsMidiEffect
        return false;
       #elif JucePlugin_IsSynth
        if (isInput) return false;
       #endif

        const int busCount = AudioUnitHelpers::getBusCount (juceFilter.get(), isInput);
        return (juceFilter->canAddBus (isInput) || (busCount > 0 && juceFilter->canRemoveBus (isInput)));
       #endif
    }

    OSStatus SetBusCount (AudioUnitScope scope, UInt32 count) override
    {
        OSStatus err = noErr;
        bool isInput;

        if ((err = scopeToDirection (scope, isInput)) != noErr)
            return err;

        if (count != (UInt32) AudioUnitHelpers::getBusCount (juceFilter.get(), isInput))
        {
           #ifdef JucePlugin_PreferredChannelConfigurations
            return kAudioUnitErr_PropertyNotWritable;
           #else
            const int busCount = AudioUnitHelpers::getBusCount (juceFilter.get(), isInput);

            if  ((! juceFilter->canAddBus (isInput)) && ((busCount == 0) || (! juceFilter->canRemoveBus (isInput))))
                return kAudioUnitErr_PropertyNotWritable;

            // we need to already create the underlying elements so that we can change their formats
            err = MusicDeviceBase::SetBusCount (scope, count);

            if (err != noErr)
                return err;

            // however we do need to update the format tag: we need to do the same thing in SetFormat, for example
            const int requestedNumBus = static_cast<int> (count);
            {
                (isInput ? currentInputLayout : currentOutputLayout).resize (requestedNumBus);

                int busNr;

                for (busNr = (busCount - 1); busNr != (requestedNumBus - 1); busNr += (requestedNumBus > busCount ? 1 : -1))
                {
                    if (requestedNumBus > busCount)
                    {
                        if (! juceFilter->addBus (isInput))
                            break;

                        err = syncAudioUnitWithChannelSet (isInput, busNr,
                                                           juceFilter->getBus (isInput, busNr + 1)->getDefaultLayout());
                        if (err != noErr)
                            break;
                    }
                    else
                    {
                        if (! juceFilter->removeBus (isInput))
                            break;
                    }
                }

                err = (busNr == (requestedNumBus - 1) ? (OSStatus) noErr : (OSStatus) kAudioUnitErr_FormatNotSupported);
            }

            // was there an error?
            if (err != noErr)
            {
                // restore bus state
                const int newBusCount = AudioUnitHelpers::getBusCount (juceFilter.get(), isInput);
                for (int i = newBusCount; i != busCount; i += (busCount > newBusCount ? 1 : -1))
                {
                    if (busCount > newBusCount)
                        juceFilter->addBus (isInput);
                    else
                        juceFilter->removeBus (isInput);
                }

                (isInput ? currentInputLayout : currentOutputLayout).resize (busCount);
                MusicDeviceBase::SetBusCount (scope, static_cast<UInt32> (busCount));

                return kAudioUnitErr_FormatNotSupported;
            }

            // update total channel count
            totalInChannels  = juceFilter->getTotalNumInputChannels();
            totalOutChannels = juceFilter->getTotalNumOutputChannels();

            if (err != noErr)
                return err;
           #endif
        }

        return noErr;
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

                case kAudioUnitProperty_SupportsMPE:
                    outDataSize = sizeof (UInt32);
                    outWritable = false;
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
                case kAudioUnitProperty_ParameterClumpName:

                    if (auto* clumpNameInfo = (AudioUnitParameterNameInfo*) outData)
                    {
                        if (juceFilter != nullptr)
                        {
                            auto clumpIndex = clumpNameInfo->inID - 1;
                            const auto* group = parameterGroups[(int) clumpIndex];
                            auto name = group->getName();

                            while (group->getParent() != &juceFilter->getParameterTree())
                            {
                                group = group->getParent();
                                name = group->getName() + group->getSeparator() + name;
                            }

                            clumpNameInfo->outName = name.toCFString();
                            return noErr;
                        }
                    }

                    // Failed to find a group corresponding to the clump ID.
                    jassertfalse;
                    break;

                case juceFilterObjectPropertyID:
                    ((void**) outData)[0] = (void*) static_cast<AudioProcessor*> (juceFilter.get());
                    ((void**) outData)[1] = (void*) this;
                    return noErr;

                case kAudioUnitProperty_OfflineRender:
                    *(UInt32*) outData = (juceFilter != nullptr && juceFilter->isNonRealtime()) ? 1 : 0;
                    return noErr;

                case kMusicDeviceProperty_InstrumentCount:
                    *(UInt32*) outData = 1;
                    return noErr;

                case kAudioUnitProperty_BypassEffect:
                    if (bypassParam != nullptr)
                        *(UInt32*) outData = (bypassParam->getValue() != 0.0f ? 1 : 0);
                    else
                        *(UInt32*) outData = isBypassed ? 1 : 0;
                    return noErr;

                case kAudioUnitProperty_SupportsMPE:
                    *(UInt32*) outData = (juceFilter != nullptr && juceFilter->supportsMPE()) ? 1 : 0;
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
                            if (auto* param = getParameterForAUParameterID (pv->inParamID))
                            {
                                const String text (String::fromCFString (pv->inString));

                                if (LegacyAudioParameter::isLegacy (param))
                                    pv->outValue = text.getFloatValue();
                                else
                                    pv->outValue = param->getValueForText (text) * getMaximumParameterValue (param);


                                return noErr;
                            }
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
                            if (auto* param = getParameterForAUParameterID (pv->inParamID))
                            {
                                const float value = (float) *(pv->inValue);
                                String text;

                                if (LegacyAudioParameter::isLegacy (param))
                                    text = String (value);
                                else
                                    text = param->getText (value / getMaximumParameterValue (param), 0);

                                pv->outString = text.toCFString();

                                return noErr;
                            }
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
                    const bool currentlyBypassed = (bypassParam != nullptr ? (bypassParam->getValue() != 0.0f) : isBypassed);

                    if (newBypass != currentlyBypassed)
                    {
                        if (bypassParam != nullptr)
                            bypassParam->setValueNotifyingHost (newBypass ? 1.0f : 0.0f);
                        else
                            isBypassed = newBypass;

                        if (! currentlyBypassed && IsInitialized()) // turning bypass off and we're initialized
                            Reset (0, 0);
                    }

                    return noErr;
                }

                case kAudioUnitProperty_OfflineRender:
                {
                    auto shouldBeRealtime = (*reinterpret_cast<const UInt32*> (inData) != 0);

                    if (juceFilter != nullptr)
                    {
                        auto isCurrentlyRealtime = juceFilter->isNonRealtime();

                        if (isCurrentlyRealtime != shouldBeRealtime)
                        {
                            const ScopedLock sl (juceFilter->getCallbackLock());

                            juceFilter->setNonRealtime (shouldBeRealtime);
                            juceFilter->prepareToPlay (getSampleRate(), (int) GetMaxFramesPerSlice());
                        }
                    }

                    return noErr;
                }

               #if defined (MAC_OS_X_VERSION_10_12)
                case kAudioUnitProperty_AUHostIdentifier:
                {
                    if (inDataSize < sizeof (AUHostVersionIdentifier))
                        return kAudioUnitErr_InvalidPropertyValue;

                    const auto* identifier = static_cast<const AUHostVersionIdentifier*> (inData);
                    PluginHostType::hostIdReportedByWrapper = String::fromCFString (identifier->hostName);

                    return noErr;
                }
               #endif

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

           #if JUCE_AU_WRAPPERS_SAVE_PROGRAM_STATES
            juceFilter->getCurrentProgramStateInformation (state);
           #else
            juceFilter->getStateInformation (state);
           #endif

            if (state.getSize() > 0)
            {
                CFDataRef ourState = CFDataCreate (kCFAllocatorDefault, (const UInt8*) state.getData(), (CFIndex) state.getSize());

                CFStringRef key = CFStringCreateWithCString (kCFAllocatorDefault, JUCE_STATE_DICTIONARY_KEY, kCFStringEncodingUTF8);
                CFDictionarySetValue (dict, key, ourState);
                CFRelease (key);
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
            CFDataRef data = nullptr;

            CFStringRef key = CFStringCreateWithCString (kCFAllocatorDefault, JUCE_STATE_DICTIONARY_KEY, kCFStringEncodingUTF8);

            bool valuePresent = CFDictionaryGetValueIfPresent (dict, key, (const void**) &data);
            CFRelease (key);

            if (valuePresent)
            {
                if (data != nullptr)
                {
                    const int numBytes = (int) CFDataGetLength (data);
                    const juce::uint8* const rawBytes = CFDataGetBytePtr (data);

                    if (numBytes > 0)
                    {
                       #if JUCE_AU_WRAPPERS_SAVE_PROGRAM_STATES
                        juceFilter->setCurrentProgramStateInformation (rawBytes, numBytes);
                       #else
                        juceFilter->setStateInformation (rawBytes, numBytes);
                       #endif
                    }
                }
            }
        }

        return noErr;
    }

    //==============================================================================
    bool busIgnoresLayout (bool isInput, int busNr) const
    {
       #ifdef JucePlugin_PreferredChannelConfigurations
        ignoreUnused (isInput, busNr);

        return true;
       #else
        if (const AudioProcessor::Bus* bus = juceFilter->getBus (isInput, busNr))
        {
            AudioChannelSet discreteRangeSet;
            const int n = bus->getDefaultLayout().size();
            for (int i = 0; i < n; ++i)
                discreteRangeSet.addChannel ((AudioChannelSet::ChannelType) (256 + i));

            // if the audioprocessor supports this it cannot
            // really be interested in the bus layouts
            return bus->isLayoutSupported (discreteRangeSet);
        }

        return true;
       #endif
    }

    UInt32 GetAudioChannelLayout (AudioUnitScope scope, AudioUnitElement element,
                                  AudioChannelLayout* outLayoutPtr, Boolean& outWritable) override
    {
        bool isInput;
        int busNr;

        outWritable = false;

        if (elementToBusIdx (scope, element, isInput, busNr) != noErr)
            return 0;

        if (busIgnoresLayout (isInput, busNr))
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

        if (busIgnoresLayout (isInput, busNr))
            return 0;

        const Array<AudioChannelLayoutTag>& layouts = getSupportedBusLayouts (isInput, busNr);

        if (outLayoutTags != nullptr)
            std::copy (layouts.begin(), layouts.end(), outLayoutTags);

        return (UInt32) layouts.size();
    }

    OSStatus SetAudioChannelLayout (AudioUnitScope scope, AudioUnitElement element, const AudioChannelLayout* inLayout) override
    {
        bool isInput;
        int busNr;
        OSStatus err;

        if ((err = elementToBusIdx (scope, element, isInput, busNr)) != noErr)
            return err;

        if (busIgnoresLayout (isInput, busNr))
            return kAudioUnitErr_PropertyNotWritable;

        if (inLayout == nullptr)
            return kAudioUnitErr_InvalidPropertyValue;

        if (const AUIOElement* ioElement = GetIOElement (isInput ? kAudioUnitScope_Input : kAudioUnitScope_Output, element))
        {
            const AudioChannelSet newChannelSet = CoreAudioLayouts::fromCoreAudio (*inLayout);
            const int currentNumChannels = static_cast<int> (ioElement->GetStreamFormat().NumberChannels());
            const int newChannelNum = newChannelSet.size();

            if (currentNumChannels != newChannelNum)
                return kAudioUnitErr_InvalidPropertyValue;

            // check if the new layout could be potentially set
           #ifdef JucePlugin_PreferredChannelConfigurations
            short configs[][2] = {JucePlugin_PreferredChannelConfigurations};

            if (! AudioUnitHelpers::isLayoutSupported (*juceFilter, isInput, busNr, newChannelNum, configs))
                return kAudioUnitErr_FormatNotSupported;
           #else
            if (! juceFilter->getBus (isInput, busNr)->isLayoutSupported (newChannelSet))
                return kAudioUnitErr_FormatNotSupported;
           #endif

            getCurrentLayout (isInput, busNr) = CoreAudioLayouts::toCoreAudio (newChannelSet);

            return noErr;
        }
        else
            jassertfalse;

        return kAudioUnitErr_InvalidElement;
    }

    //==============================================================================
    // When parameters are discrete we need to use integer values.
    float getMaximumParameterValue (AudioProcessorParameter* juceParam)
    {
       #if JUCE_FORCE_LEGACY_PARAMETER_AUTOMATION_TYPE
        ignoreUnused (juceParam);
        return 1.0f;
       #else
        return juceParam->isDiscrete() ? (float) (juceParam->getNumSteps() - 1) : 1.0f;
       #endif
    }

    ComponentResult GetParameterInfo (AudioUnitScope inScope,
                                      AudioUnitParameterID inParameterID,
                                      AudioUnitParameterInfo& outParameterInfo) override
    {
        if (inScope == kAudioUnitScope_Global && juceFilter != nullptr)
        {
            if (auto* param = getParameterForAUParameterID (inParameterID))
            {
                outParameterInfo.unit = kAudioUnitParameterUnit_Generic;
                outParameterInfo.flags = (UInt32) (kAudioUnitParameterFlag_IsWritable
                                                    | kAudioUnitParameterFlag_IsReadable
                                                    | kAudioUnitParameterFlag_HasCFNameString
                                                    | kAudioUnitParameterFlag_ValuesHaveStrings);

               #if ! JUCE_FORCE_LEGACY_PARAMETER_AUTOMATION_TYPE
                outParameterInfo.flags |= (UInt32) kAudioUnitParameterFlag_IsHighResolution;
               #endif

                const String name = param->getName (1024);

                // Set whether the param is automatable (unnamed parameters aren't allowed to be automated)
                if (name.isEmpty() || ! param->isAutomatable())
                    outParameterInfo.flags |= kAudioUnitParameterFlag_NonRealTime;

                const bool isParameterDiscrete = param->isDiscrete();

                if (! isParameterDiscrete)
                    outParameterInfo.flags |= kAudioUnitParameterFlag_CanRamp;

                if (param->isMetaParameter())
                    outParameterInfo.flags |= kAudioUnitParameterFlag_IsGlobalMeta;

                auto parameterGroupHierarchy = juceFilter->getParameterTree().getGroupsForParameter (param);

                if (! parameterGroupHierarchy.isEmpty())
                {
                    outParameterInfo.flags |= kAudioUnitParameterFlag_HasClump;
                    outParameterInfo.clumpID = (UInt32) parameterGroups.indexOf (parameterGroupHierarchy.getLast()) + 1;
                }

                // Is this a meter?
                if ((((unsigned int) param->getCategory() & 0xffff0000) >> 16) == 2)
                {
                    outParameterInfo.flags &= ~kAudioUnitParameterFlag_IsWritable;
                    outParameterInfo.flags |= kAudioUnitParameterFlag_MeterReadOnly | kAudioUnitParameterFlag_DisplayLogarithmic;
                    outParameterInfo.unit = kAudioUnitParameterUnit_LinearGain;
                }
                else
                {
                   #if ! JUCE_FORCE_LEGACY_PARAMETER_AUTOMATION_TYPE
                    if (isParameterDiscrete)
                        outParameterInfo.unit = param->isBoolean() ? kAudioUnitParameterUnit_Boolean
                                                                   : kAudioUnitParameterUnit_Indexed;
                   #endif
                }

                MusicDeviceBase::FillInParameterName (outParameterInfo, name.toCFString(), true);

                outParameterInfo.minValue = 0.0f;
                outParameterInfo.maxValue = getMaximumParameterValue (param);
                outParameterInfo.defaultValue = param->getDefaultValue() * getMaximumParameterValue (param);
                jassert (outParameterInfo.defaultValue >= outParameterInfo.minValue
                      && outParameterInfo.defaultValue <= outParameterInfo.maxValue);

                return noErr;
            }
        }

        return kAudioUnitErr_InvalidParameter;
    }

    ComponentResult GetParameterValueStrings (AudioUnitScope inScope,
                                              AudioUnitParameterID inParameterID,
                                              CFArrayRef *outStrings) override
    {
        if (outStrings == nullptr)
            return noErr;

        if (inScope == kAudioUnitScope_Global && juceFilter != nullptr)
        {
            if (auto* param = getParameterForAUParameterID (inParameterID))
            {
                if (param->isDiscrete())
                {
                    auto index = LegacyAudioParameter::getParamIndex (*juceFilter, param);

                    if (auto* valueStrings = parameterValueStringArrays[index])
                    {
                        *outStrings = CFArrayCreate (nullptr,
                                                     (const void **) valueStrings->getRawDataPointer(),
                                                     valueStrings->size(),
                                                     nullptr);

                        return noErr;
                    }
                }
            }
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
            if (auto* param = getParameterForAUParameterID (inID))
            {
                const auto normValue = param->getValue();

                outValue = normValue * getMaximumParameterValue (param);
                return noErr;
            }
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
            if (auto* param = getParameterForAUParameterID (inID))
            {
                auto value = inValue / getMaximumParameterValue (param);

                param->setValue (value);

                inParameterChangedCallback = true;
                param->sendValueChangedMessageToListeners (value);

                return noErr;
            }
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
    double getSampleRate()                               { return AudioUnitHelpers::getBusCount (juceFilter.get(), false) > 0 ? GetOutput (0)->GetStreamFormat().mSampleRate : 44100.0; }

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
            case kSMPTETimeType2398:        info.frameRate = AudioPlayHead::fps23976;    break;
            case kSMPTETimeType24:          info.frameRate = AudioPlayHead::fps24;       break;
            case kSMPTETimeType25:          info.frameRate = AudioPlayHead::fps25;       break;
            case kSMPTETimeType30Drop:      info.frameRate = AudioPlayHead::fps30drop;   break;
            case kSMPTETimeType30:          info.frameRate = AudioPlayHead::fps30;       break;
            case kSMPTETimeType2997:        info.frameRate = AudioPlayHead::fps2997;     break;
            case kSMPTETimeType2997Drop:    info.frameRate = AudioPlayHead::fps2997drop; break;
            case kSMPTETimeType60:          info.frameRate = AudioPlayHead::fps60;       break;
            case kSMPTETimeType60Drop:      info.frameRate = AudioPlayHead::fps60drop;   break;
            case kSMPTETimeType5994:
            case kSMPTETimeType5994Drop:
            case kSMPTETimeType50:
            default:                        info.frameRate = AudioPlayHead::fpsUnknown;  break;
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

    void sendAUEvent (const AudioUnitEventType type, const int juceParamIndex)
    {
        auEvent.mEventType = type;
        auEvent.mArgument.mParameter.mParameterID = getAUParameterIDForIndex (juceParamIndex);
        AUEventListenerNotify (nullptr, nullptr, &auEvent);
    }

    void audioProcessorParameterChanged (AudioProcessor*, int index, float /*newValue*/) override
    {
        if (inParameterChangedCallback.get())
        {
            inParameterChangedCallback = false;
            return;
        }

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

    void audioProcessorChanged (AudioProcessor*, const ChangeDetails& details) override
    {
        if (details.latencyChanged)
            PropertyChanged (kAudioUnitProperty_Latency, kAudioUnitScope_Global, 0);

        if (details.parameterInfoChanged)
        {
            PropertyChanged (kAudioUnitProperty_ParameterList, kAudioUnitScope_Global, 0);
            PropertyChanged (kAudioUnitProperty_ParameterInfo, kAudioUnitScope_Global, 0);
        }

        PropertyChanged (kAudioUnitProperty_ClassInfo, kAudioUnitScope_Global, 0);

        if (details.programChanged)
        {
            refreshCurrentPreset();
            PropertyChanged (kAudioUnitProperty_PresentPreset, kAudioUnitScope_Global, 0);
        }
    }

    //==============================================================================
    // this will only ever be called by the bypass parameter
    void parameterValueChanged (int, float) override
    {
        PropertyChanged (kAudioUnitProperty_BypassEffect, kAudioUnitScope_Global, 0);
    }

    void parameterGestureChanged (int, bool) override {}

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

        // DSP Quattro incorrectly uses global scope for the ValidFormat call
        if (scope == kAudioUnitScope_Global)
            return ValidFormat (kAudioUnitScope_Input,  element, format)
                || ValidFormat (kAudioUnitScope_Output, element, format);

        if (elementToBusIdx (scope, element, isInput, busNr) != noErr)
            return false;

        const int newNumChannels = static_cast<int> (format.NumberChannels());
        const int oldNumChannels = juceFilter->getChannelCountOfBus (isInput, busNr);

        if (newNumChannels == oldNumChannels)
            return true;

        if (AudioProcessor::Bus* bus = juceFilter->getBus (isInput, busNr))
        {
            if (! MusicDeviceBase::ValidFormat (scope, element, format))
                return false;

           #ifdef JucePlugin_PreferredChannelConfigurations
            short configs[][2] = {JucePlugin_PreferredChannelConfigurations};

            ignoreUnused (bus);
            return AudioUnitHelpers::isLayoutSupported (*juceFilter, isInput, busNr, newNumChannels, configs);
           #else
            return bus->isNumberOfChannelsSupported (newNumChannels);
           #endif
        }

        return false;
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
        const int oldNumChannels = juceFilter->getChannelCountOfBus (isInput, busNr);

       #ifdef JucePlugin_PreferredChannelConfigurations
        short configs[][2] = {JucePlugin_PreferredChannelConfigurations};

        if (! AudioUnitHelpers::isLayoutSupported (*juceFilter, isInput, busNr, newNumChannels, configs))
            return kAudioUnitErr_FormatNotSupported;
       #endif

        // predict channel layout
        AudioChannelSet set = (newNumChannels != oldNumChannels) ? juceFilter->getBus (isInput, busNr)->supportedLayoutWithChannels (newNumChannels)
                                                                 : juceFilter->getChannelLayoutOfBus (isInput, busNr);

        if (set == AudioChannelSet())
            return kAudioUnitErr_FormatNotSupported;

        err = MusicDeviceBase::ChangeStreamFormat (scope, element, old, format);

        if (err == noErr)
            currentTag = CoreAudioLayouts::toCoreAudio (set);

        return err;
    }

    //==============================================================================
    ComponentResult Render (AudioUnitRenderActionFlags& ioActionFlags,
                            const AudioTimeStamp& inTimeStamp,
                            const UInt32 nFrames) override
    {
        lastTimeStamp = inTimeStamp;

        // prepare buffers
        {
            pullInputAudio (ioActionFlags, inTimeStamp, nFrames);
            prepareOutputBuffers (nFrames);
            audioBuffer.reset();
        }

        ioActionFlags &= ~kAudioUnitRenderAction_OutputIsSilence;

        const int numInputBuses  = static_cast<int> (GetScope (kAudioUnitScope_Input) .GetNumberOfElements());
        const int numOutputBuses = static_cast<int> (GetScope (kAudioUnitScope_Output).GetNumberOfElements());

        // set buffer pointers to minimize copying
        {
            int chIdx = 0, numChannels = 0;
            bool interleaved = false;
            AudioBufferList* buffer = nullptr;

            // use output pointers
            for (int busIdx = 0; busIdx < numOutputBuses; ++busIdx)
            {
                GetAudioBufferList (false, busIdx, buffer, interleaved, numChannels);
                const int* outLayoutMap = mapper.get (false, busIdx);

                for (int ch = 0; ch < numChannels; ++ch)
                    audioBuffer.setBuffer (chIdx++, interleaved ? nullptr : static_cast<float*> (buffer->mBuffers[outLayoutMap[ch]].mData));
            }

            // use input pointers on remaining channels
            for (int busIdx = 0; chIdx < totalInChannels;)
            {
                int channelIndexInBus = juceFilter->getOffsetInBusBufferForAbsoluteChannelIndex (true, chIdx, busIdx);
                const bool badData = ! pulledSucceeded[busIdx];

                if (! badData)
                    GetAudioBufferList (true, busIdx, buffer, interleaved, numChannels);

                const int* inLayoutMap = mapper.get (true, busIdx);

                const int n = juceFilter->getChannelCountOfBus (true, busIdx);
                for (int ch = channelIndexInBus; ch < n; ++ch)
                    audioBuffer.setBuffer (chIdx++, interleaved || badData ? nullptr : static_cast<float*> (buffer->mBuffers[inLayoutMap[ch]].mData));
            }
        }

        // copy input
        {
            for (int busIdx = 0; busIdx < numInputBuses; ++busIdx)
            {
                if (pulledSucceeded[busIdx])
                {
                    audioBuffer.push (GetInput ((UInt32) busIdx)->GetBufferList(), mapper.get (true, busIdx));
                }
                else
                {
                    const int n = juceFilter->getChannelCountOfBus (true, busIdx);

                    for (int ch = 0; ch < n; ++ch)
                        zeromem (audioBuffer.push(), sizeof (float) * nFrames);
                }
            }

            // clear remaining channels
            for (int i = totalInChannels; i < totalOutChannels; ++i)
                zeromem (audioBuffer.push(), sizeof (float) * nFrames);
        }

        // swap midi buffers
        {
            const ScopedLock sl (incomingMidiLock);
            midiEvents.clear();
            incomingEvents.swapWith (midiEvents);
        }

        // process audio
        processBlock (audioBuffer.getBuffer (nFrames), midiEvents);

        // copy back
        {
            for (int busIdx = 0; busIdx < numOutputBuses; ++busIdx)
                audioBuffer.pop (GetOutput ((UInt32) busIdx)->GetBufferList(), mapper.get (false, busIdx));
        }

        // process midi output
      #if JucePlugin_ProducesMidiOutput || JucePlugin_IsMidiEffect
        if (! midiEvents.isEmpty() && midiCallback.midiOutputCallback != nullptr)
            pushMidiOutput (nFrames);
      #endif

        midiEvents.clear();

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

            CFMutableArrayRef presetsArrayRef = CFArrayCreateMutable (nullptr, numPrograms, nullptr);

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

    //==============================================================================
    class EditorCompHolder  : public Component
    {
    public:
        EditorCompHolder (AudioProcessorEditor* const editor)
        {
            addAndMakeVisible (editor);

           #if ! JucePlugin_EditorRequiresKeyboardFocus
            setWantsKeyboardFocus (false);
           #else
            setWantsKeyboardFocus (true);
           #endif

            ignoreUnused (fakeMouseGenerator);
            setBounds (getSizeToContainChild());

            lastBounds = getBounds();
        }

        ~EditorCompHolder() override
        {
            deleteAllChildren(); // note that we can't use a std::unique_ptr because the editor may
                                 // have been transferred to another parent which takes over ownership.
        }

        Rectangle<int> getSizeToContainChild()
        {
            if (auto* editor = getChildComponent (0))
                return getLocalArea (editor, editor->getLocalBounds());

            return {};
        }

        static NSView* createViewFor (AudioProcessor* filter, JuceAU* au, AudioProcessorEditor* const editor)
        {
            auto* editorCompHolder = new EditorCompHolder (editor);
            auto r = convertToHostBounds (makeNSRect (editorCompHolder->getSizeToContainChild()));

            static JuceUIViewClass cls;
            auto* view = [[cls.createInstance() initWithFrame: r] autorelease];

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

        void parentSizeChanged() override
        {
            resizeHostWindow();

            if (auto* editor = getChildComponent (0))
                editor->repaint();
        }

        void childBoundsChanged (Component*) override
        {
            auto b = getSizeToContainChild();

            if (lastBounds != b)
            {
                lastBounds = b;
                setSize (jmax (32, b.getWidth()), jmax (32, b.getHeight()));

                resizeHostWindow();
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
                    [hostView keyDown: (NSEvent*) [NSApp currentEvent]];
                    [hostWindow makeFirstResponder: view];
                }
            }

            return false;
        }

        void resizeHostWindow()
        {
            [CATransaction begin];
            [CATransaction setValue:(id) kCFBooleanTrue forKey:kCATransactionDisableActions];

            auto rect = convertToHostBounds (makeNSRect (lastBounds));
            auto* view = (NSView*) getWindowHandle();

            auto superRect = [[view superview] frame];
            superRect.size.width  = rect.size.width;
            superRect.size.height = rect.size.height;

            [[view superview] setFrame: superRect];
            [view setFrame: rect];
            [CATransaction commit];

            [view setNeedsDisplay: YES];
        }

    private:
        FakeMouseMoveGenerator fakeMouseGenerator;
        Rectangle<int> lastBounds;

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
            std::unique_ptr<EditorCompHolder> editorComp (getEditor (self));

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

            sendSuperclassMessage<void> (self, @selector (dealloc));
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
    AudioUnitHelpers::CoreAudioBufferList audioBuffer;
    MidiBuffer midiEvents, incomingEvents;
    bool prepared = false, isBypassed = false;

    //==============================================================================
   #if JUCE_FORCE_USE_LEGACY_PARAM_IDS
    static constexpr bool forceUseLegacyParamIDs = true;
   #else
    static constexpr bool forceUseLegacyParamIDs = false;
   #endif

    //==============================================================================
    LegacyAudioParametersWrapper juceParameters;
    HashMap<int32, AudioProcessorParameter*> paramMap;
    Array<AudioUnitParameterID> auParamIDs;
    Array<const AudioProcessorParameterGroup*> parameterGroups;

    //==============================================================================
    // According to the docs, this is the maximum size of a MIDIPacketList.
    static constexpr UInt32 packetListBytes = 65536;

    AudioUnitEvent auEvent;
    mutable Array<AUPreset> presetsArray;
    CriticalSection incomingMidiLock;
    AUMIDIOutputCallbackStruct midiCallback;
    AudioTimeStamp lastTimeStamp;
    int totalInChannels, totalOutChannels;
    HeapBlock<bool> pulledSucceeded;
    HeapBlock<MIDIPacketList> packetList { packetListBytes, 1 };

    ThreadLocalValue<bool> inParameterChangedCallback;

    //==============================================================================
    Array<AUChannelInfo> channelInfo;
    Array<Array<AudioChannelLayoutTag>> supportedInputLayouts, supportedOutputLayouts;
    Array<AudioChannelLayoutTag> currentInputLayout, currentOutputLayout;

    //==============================================================================
    AudioUnitHelpers::ChannelRemapper mapper;

    //==============================================================================
    OwnedArray<OwnedArray<const __CFString>> parameterValueStringArrays;

    //==============================================================================
    AudioProcessorParameter* bypassParam = nullptr;

    //==============================================================================
    static NSRect convertToHostBounds (NSRect pluginRect)
    {
        auto desktopScale = Desktop::getInstance().getGlobalScaleFactor();

        if (approximatelyEqual (desktopScale, 1.0f))
            return pluginRect;

        return NSMakeRect (static_cast<CGFloat> (pluginRect.origin.x    * desktopScale),
                           static_cast<CGFloat> (pluginRect.origin.y    * desktopScale),
                           static_cast<CGFloat> (pluginRect.size.width  * desktopScale),
                           static_cast<CGFloat> (pluginRect.size.height * desktopScale));
    }

    static NSRect convertFromHostBounds (NSRect hostRect)
    {
        auto desktopScale = Desktop::getInstance().getGlobalScaleFactor();

        if (approximatelyEqual (desktopScale, 1.0f))
            return hostRect;

        return NSMakeRect (static_cast<CGFloat> (hostRect.origin.x    / desktopScale),
                           static_cast<CGFloat> (hostRect.origin.y    / desktopScale),
                           static_cast<CGFloat> (hostRect.size.width  / desktopScale),
                           static_cast<CGFloat> (hostRect.size.height / desktopScale));
    }

    //==============================================================================
    void pullInputAudio (AudioUnitRenderActionFlags& flags, const AudioTimeStamp& timestamp, const UInt32 nFrames) noexcept
    {
        const unsigned int numInputBuses = GetScope (kAudioUnitScope_Input).GetNumberOfElements();

        for (unsigned int i = 0; i < numInputBuses; ++i)
        {
            if (AUInputElement* input = GetInput (i))
            {
                const bool succeeded = (input->PullInput (flags, timestamp, i, nFrames) == noErr);

                if ((flags & kAudioUnitRenderAction_OutputIsSilence) != 0 && succeeded)
                    AudioUnitHelpers::clearAudioBuffer (input->GetBufferList());

                pulledSucceeded[i] = succeeded;
            }
        }
    }

    void prepareOutputBuffers (const UInt32 nFrames) noexcept
    {
        const unsigned int numOutputBuses = GetScope (kAudioUnitScope_Output).GetNumberOfElements();

        for (unsigned int busIdx = 0; busIdx < numOutputBuses; ++busIdx)
        {
            AUOutputElement* output = GetOutput (busIdx);

            if (output->WillAllocateBuffer())
                output->PrepareBuffer (nFrames);
        }
    }

    void processBlock (juce::AudioBuffer<float>& buffer, MidiBuffer& midiBuffer) noexcept
    {
        const ScopedLock sl (juceFilter->getCallbackLock());

        if (juceFilter->isSuspended())
        {
            buffer.clear();
        }
        else if (bypassParam == nullptr && isBypassed)
        {
            juceFilter->processBlockBypassed (buffer, midiBuffer);
        }
        else
        {
            juceFilter->processBlock (buffer, midiBuffer);
        }
    }

    void pushMidiOutput (UInt32 nFrames) noexcept
    {
        MIDIPacket* end = nullptr;

        const auto init = [&]
        {
            end = MIDIPacketListInit (packetList);
        };

        const auto send = [&]
        {
            midiCallback.midiOutputCallback (midiCallback.userData, &lastTimeStamp, 0, packetList);
        };

        const auto add = [&] (const MidiMessageMetadata& metadata)
        {
            end = MIDIPacketListAdd (packetList,
                                     packetListBytes,
                                     end,
                                     static_cast<MIDITimeStamp> (metadata.samplePosition),
                                     static_cast<ByteCount> (metadata.numBytes),
                                     metadata.data);
        };

        init();

        for (const auto metadata : midiEvents)
        {
            jassert (isPositiveAndBelow (metadata.samplePosition, nFrames));
            ignoreUnused (nFrames);

            add (metadata);

            if (end == nullptr)
            {
                send();
                init();
                add (metadata);

                if (end == nullptr)
                {
                    // If this is hit, the size of this midi packet exceeds the maximum size of
                    // a MIDIPacketList. Large SysEx messages should be broken up into smaller
                    // chunks.
                    jassertfalse;
                    init();
                }
            }
        }

        send();
    }

    void GetAudioBufferList (bool isInput, int busIdx, AudioBufferList*& bufferList, bool& interleaved, int& numChannels)
    {
        AUIOElement* element = GetElement (isInput ? kAudioUnitScope_Input : kAudioUnitScope_Output, static_cast<UInt32> (busIdx))->AsIOElement();
        jassert (element != nullptr);

        bufferList = &element->GetBufferList();

        jassert (bufferList->mNumberBuffers > 0);

        interleaved = AudioUnitHelpers::isAudioBufferInterleaved (*bufferList);
        numChannels = static_cast<int> (interleaved ? bufferList->mBuffers[0].mNumberChannels : bufferList->mNumberBuffers);
    }

    //==============================================================================
    static OSStatus scopeToDirection (AudioUnitScope scope, bool& isInput) noexcept
    {
        isInput = (scope == kAudioUnitScope_Input);

        return (scope != kAudioUnitScope_Input
             && scope != kAudioUnitScope_Output)
              ? (OSStatus) kAudioUnitErr_InvalidScope : (OSStatus) noErr;
    }

    OSStatus elementToBusIdx (AudioUnitScope scope, AudioUnitElement element, bool& isInput, int& busIdx) noexcept
    {
        OSStatus err;

        busIdx = static_cast<int> (element);

        if ((err = scopeToDirection (scope, isInput)) != noErr) return err;
        if (isPositiveAndBelow (busIdx, AudioUnitHelpers::getBusCount (juceFilter.get(), isInput))) return noErr;

        return kAudioUnitErr_InvalidElement;
    }

    //==============================================================================
    void addParameters()
    {
        parameterGroups = juceFilter->getParameterTree().getSubgroups (true);

        juceParameters.update (*juceFilter, forceUseLegacyParamIDs);
        const int numParams = juceParameters.getNumParameters();

        if (forceUseLegacyParamIDs)
        {
            Globals()->UseIndexedParameters (numParams);
        }
        else
        {
            for (auto* param : juceParameters.params)
            {
                const AudioUnitParameterID auParamID = generateAUParameterID (param);

                // Consider yourself very unlucky if you hit this assertion. The hash codes of your
                // parameter ids are not unique.
                jassert (! paramMap.contains (static_cast<int32> (auParamID)));

                auParamIDs.add (auParamID);
                paramMap.set (static_cast<int32> (auParamID), param);
                Globals()->SetParameter (auParamID, param->getValue());
            }
        }

       #if JUCE_DEBUG
        // Some hosts can't handle the huge numbers of discrete parameter values created when
        // using the default number of steps.
        for (auto* param : juceParameters.params)
            if (param->isDiscrete())
                jassert (param->getNumSteps() != AudioProcessor::getDefaultNumParameterSteps());
       #endif

        parameterValueStringArrays.ensureStorageAllocated (numParams);

        for (auto* param : juceParameters.params)
        {
            OwnedArray<const __CFString>* stringValues = nullptr;

            auto initialValue = param->getValue();
            bool paramIsLegacy = dynamic_cast<LegacyAudioParameter*> (param) != nullptr;

            if (param->isDiscrete() && (! forceUseLegacyParamIDs))
            {
                const auto numSteps = param->getNumSteps();
                stringValues = new OwnedArray<const __CFString>();
                stringValues->ensureStorageAllocated (numSteps);

                const auto maxValue = getMaximumParameterValue (param);

                auto getTextValue = [param, paramIsLegacy] (float value)
                {
                    if (paramIsLegacy)
                    {
                        param->setValue (value);
                        return param->getCurrentValueAsText();
                    }

                    return param->getText (value, 256);
                };

                for (int i = 0; i < numSteps; ++i)
                {
                    auto value = (float) i / maxValue;
                    stringValues->add (CFStringCreateCopy (nullptr, (getTextValue (value).toCFString())));
                }
            }

            if (paramIsLegacy)
                param->setValue (initialValue);

            parameterValueStringArrays.add (stringValues);
        }

        if ((bypassParam = juceFilter->getBypassParameter()) != nullptr)
            bypassParam->addListener (this);
    }

    //==============================================================================
    AudioUnitParameterID generateAUParameterID (AudioProcessorParameter* param) const
    {
        const String& juceParamID = LegacyAudioParameter::getParamID (param, forceUseLegacyParamIDs);
        AudioUnitParameterID paramHash = static_cast<AudioUnitParameterID> (juceParamID.hashCode());

       #if JUCE_USE_STUDIO_ONE_COMPATIBLE_PARAMETERS
        // studio one doesn't like negative parameters
        paramHash &= ~(((AudioUnitParameterID) 1) << (sizeof (AudioUnitParameterID) * 8 - 1));
       #endif

        return forceUseLegacyParamIDs ? static_cast<AudioUnitParameterID> (juceParamID.getIntValue())
                                      : paramHash;
    }

    inline AudioUnitParameterID getAUParameterIDForIndex (int paramIndex) const noexcept
    {
        return forceUseLegacyParamIDs ? static_cast<AudioUnitParameterID> (paramIndex)
                                      : auParamIDs.getReference (paramIndex);
    }

    AudioProcessorParameter* getParameterForAUParameterID (AudioUnitParameterID address) const noexcept
    {
        auto index = static_cast<int32> (address);
        return forceUseLegacyParamIDs ? juceParameters.getParamForIndex (index)
                                      : paramMap[index];
    }

    //==============================================================================
    OSStatus syncAudioUnitWithProcessor()
    {
        OSStatus err = noErr;
        const int enabledInputs  = AudioUnitHelpers::getBusCount (juceFilter.get(), true);
        const int enabledOutputs = AudioUnitHelpers::getBusCount (juceFilter.get(), false);

        if ((err =  MusicDeviceBase::SetBusCount (kAudioUnitScope_Input,  static_cast<UInt32> (enabledInputs))) != noErr)
            return err;

        if ((err =  MusicDeviceBase::SetBusCount (kAudioUnitScope_Output, static_cast<UInt32> (enabledOutputs))) != noErr)
            return err;

        addSupportedLayoutTags();

        for (int i = 0; i < enabledInputs; ++i)
            if ((err = syncAudioUnitWithChannelSet (true,  i, juceFilter->getChannelLayoutOfBus (true,  i))) != noErr)
                return err;

        for (int i = 0; i < enabledOutputs; ++i)
            if ((err = syncAudioUnitWithChannelSet (false, i, juceFilter->getChannelLayoutOfBus (false, i))) != noErr)
                return err;

        return noErr;
    }

    OSStatus syncProcessorWithAudioUnit()
    {
        const int numInputBuses  = AudioUnitHelpers::getBusCount (juceFilter.get(), true);
        const int numOutputBuses = AudioUnitHelpers::getBusCount (juceFilter.get(), false);

        const int numInputElements  = static_cast<int> (GetScope(kAudioUnitScope_Input). GetNumberOfElements());
        const int numOutputElements = static_cast<int> (GetScope(kAudioUnitScope_Output).GetNumberOfElements());

        AudioProcessor::BusesLayout requestedLayouts;
        for (int dir = 0; dir < 2; ++dir)
        {
            const bool isInput = (dir == 0);
            const int n = (isInput ? numInputBuses : numOutputBuses);
            const int numAUElements  = (isInput ? numInputElements : numOutputElements);
            Array<AudioChannelSet>& requestedBuses = (isInput ? requestedLayouts.inputBuses : requestedLayouts.outputBuses);

            for (int busIdx = 0; busIdx < n; ++busIdx)
            {
                const AUIOElement* element = (busIdx < numAUElements ? GetIOElement (isInput ? kAudioUnitScope_Input :  kAudioUnitScope_Output, (UInt32) busIdx) : nullptr);
                const int numChannels = (element != nullptr ? static_cast<int> (element->GetStreamFormat().NumberChannels()) : 0);

                AudioChannelLayoutTag currentLayoutTag = isInput ? currentInputLayout[busIdx] : currentOutputLayout[busIdx];
                const int tagNumChannels = currentLayoutTag & 0xffff;

                if (numChannels != tagNumChannels)
                    return kAudioUnitErr_FormatNotSupported;

                requestedBuses.add (CoreAudioLayouts::fromCoreAudio (currentLayoutTag));
            }
        }

       #ifdef JucePlugin_PreferredChannelConfigurations
        short configs[][2] = {JucePlugin_PreferredChannelConfigurations};

        if (! AudioProcessor::containsLayout (requestedLayouts, configs))
            return kAudioUnitErr_FormatNotSupported;
       #endif

        if (! AudioUnitHelpers::setBusesLayout (juceFilter.get(), requestedLayouts))
            return kAudioUnitErr_FormatNotSupported;

        // update total channel count
        totalInChannels  = juceFilter->getTotalNumInputChannels();
        totalOutChannels = juceFilter->getTotalNumOutputChannels();

        return noErr;
    }

    OSStatus syncAudioUnitWithChannelSet (bool isInput, int busNr, const AudioChannelSet& channelSet)
    {
        const int numChannels = channelSet.size();

        getCurrentLayout (isInput, busNr) = CoreAudioLayouts::toCoreAudio (channelSet);

        // is this bus activated?
        if (numChannels == 0)
            return noErr;

        if (AUIOElement* element = GetIOElement (isInput ? kAudioUnitScope_Input :  kAudioUnitScope_Output, (UInt32) busNr))
        {
            element->SetName ((CFStringRef) juceStringToNS (juceFilter->getBus (isInput, busNr)->getName()));

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

    //==============================================================================
    void addSupportedLayoutTagsForBus (bool isInput, int busNum, Array<AudioChannelLayoutTag>& tags)
    {
        if (AudioProcessor::Bus* bus = juceFilter->getBus (isInput, busNum))
        {
           #ifndef JucePlugin_PreferredChannelConfigurations
            auto& knownTags = CoreAudioLayouts::getKnownCoreAudioTags();

            for (auto tag : knownTags)
                if (bus->isLayoutSupported (CoreAudioLayouts::fromCoreAudio (tag)))
                    tags.addIfNotAlreadyThere (tag);
           #endif

            // add discrete layout tags
            int n = bus->getMaxSupportedChannels (maxChannelsToProbeFor());

            for (int ch = 0; ch < n; ++ch)
            {
               #ifdef JucePlugin_PreferredChannelConfigurations
                const short configs[][2] = { JucePlugin_PreferredChannelConfigurations };
                if (AudioUnitHelpers::isLayoutSupported (*juceFilter, isInput, busNum, ch, configs))
                    tags.addIfNotAlreadyThere (static_cast<AudioChannelLayoutTag> ((int) kAudioChannelLayoutTag_DiscreteInOrder | ch));
               #else
                if (bus->isLayoutSupported (AudioChannelSet::discreteChannels (ch)))
                    tags.addIfNotAlreadyThere (static_cast<AudioChannelLayoutTag> ((int) kAudioChannelLayoutTag_DiscreteInOrder | ch));
               #endif
            }
        }
    }

    void addSupportedLayoutTagsForDirection (bool isInput)
    {
        auto& layouts = isInput ? supportedInputLayouts : supportedOutputLayouts;
        layouts.clear();
        auto numBuses = AudioUnitHelpers::getBusCount (juceFilter.get(), isInput);

        for (int busNr = 0; busNr < numBuses; ++busNr)
        {
            Array<AudioChannelLayoutTag> busLayouts;
            addSupportedLayoutTagsForBus (isInput, busNr, busLayouts);

            layouts.add (busLayouts);
        }
    }

    void addSupportedLayoutTags()
    {
        currentInputLayout.clear(); currentOutputLayout.clear();

        currentInputLayout. resize (AudioUnitHelpers::getBusCount (juceFilter.get(), true));
        currentOutputLayout.resize (AudioUnitHelpers::getBusCount (juceFilter.get(), false));

        addSupportedLayoutTagsForDirection (true);
        addSupportedLayoutTagsForDirection (false);
    }

    static int maxChannelsToProbeFor()
    {
        return (getHostType().isLogic() ? 8 : 64);
    }

    //==============================================================================
    void auPropertyListener (AudioUnitPropertyID propId, AudioUnitScope scope, AudioUnitElement)
    {
        if (scope == kAudioUnitScope_Global && propId == kAudioUnitProperty_ContextName
             && juceFilter != nullptr && mContextName != nullptr)
        {
            AudioProcessor::TrackProperties props;
            props.name = String::fromCFString (mContextName);

            juceFilter->updateTrackProperties (props);
        }
    }

    static void auPropertyListenerDispatcher (void* inRefCon, AudioUnit, AudioUnitPropertyID propId,
                                              AudioUnitScope scope, AudioUnitElement element)
    {
        static_cast<JuceAU*> (inRefCon)->auPropertyListener (propId, scope, element);
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
                    windowComp.reset (new ComponentInHIView (editorComp, mCarbonPane));
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
    std::unique_ptr<Component> windowComp;
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

                // not really sure why this is needed in older OS X versions
                // but JUCE plug-ins crash without it
                if ((SystemStats::getOperatingSystemType() & 0xff) < 12)
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
        PluginHostType::jucePlugInClientCurrentWrapperType = AudioProcessor::wrapperType_AudioUnit; \
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
        PluginHostType::jucePlugInClientCurrentWrapperType = AudioProcessor::wrapperType_AudioUnit; \
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
 JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wcast-align", "-Wzero-as-null-pointer-constant")

 #include "CoreAudioUtilityClasses/AUPlugInDispatch.cpp"

 JUCE_END_IGNORE_WARNINGS_GCC_LIKE
#endif

#endif

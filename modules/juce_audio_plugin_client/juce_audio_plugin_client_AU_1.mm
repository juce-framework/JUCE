/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

#include <juce_core/system/juce_TargetPlatform.h>
#include <juce_core/system/juce_CompilerWarnings.h>
#include <juce_audio_plugin_client/detail/juce_CheckSettingMacros.h>

#if JucePlugin_Build_AU

JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wcast-align",
                                     "-Wconversion",
                                     "-Wdeprecated-anon-enum-enum-conversion",
                                     "-Wdeprecated-declarations",
                                     "-Wextra-semi",
                                     "-Wfloat-equal",
                                     "-Wformat-pedantic",
                                     "-Wgnu-zero-variadic-macro-arguments",
                                     "-Wnullable-to-nonnull-conversion",
                                     "-Woverloaded-virtual",
                                     "-Wshadow",
                                     "-Wshorten-64-to-32",
                                     "-Wsign-conversion",
                                     "-Wswitch-enum",
                                     "-Wunused-parameter",
                                     "-Wzero-as-null-pointer-constant")

#include <juce_audio_plugin_client/detail/juce_IncludeSystemHeaders.h>

#include <AudioUnit/AUCocoaUIView.h>
#include <AudioUnit/AudioUnit.h>
#include <AudioToolbox/AudioUnitUtilities.h>
#include <CoreMIDI/MIDIServices.h>
#include <QuartzCore/QuartzCore.h>
#include "AudioUnitSDK/MusicDeviceBase.h"

JUCE_END_IGNORE_WARNINGS_GCC_LIKE

#define JUCE_CORE_INCLUDE_OBJC_HELPERS 1

#include <juce_audio_plugin_client/detail/juce_PluginUtilities.h>

#include <juce_audio_basics/native/juce_CoreAudioLayouts_mac.h>
#include <juce_audio_basics/native/juce_CoreAudioTimeConversions_mac.h>
#include <juce_audio_basics/native/juce_AudioWorkgroup_mac.h>
#include <juce_audio_processors/format_types/juce_LegacyAudioParameter.cpp>
#include <juce_audio_processors/format_types/juce_AU_Shared.h>
#include <juce_gui_basics/detail/juce_ComponentPeerHelpers.h>

#if JucePlugin_Enable_ARA
 #include <juce_audio_processors/utilities/ARA/juce_AudioProcessor_ARAExtensions.h>
 #include <ARA_API/ARAAudioUnit.h>
 #if ARA_SUPPORT_VERSION_1
  #error "Unsupported ARA version - only ARA version 2 and onward are supported by the current JUCE ARA implementation"
 #endif
#endif

#include <set>

JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wfour-char-constants")
constexpr auto pluginIsMidiEffect = JucePlugin_AUMainType == kAudioUnitType_MIDIProcessor;
JUCE_END_IGNORE_WARNINGS_GCC_LIKE

constexpr auto pluginProducesMidiOutput =
#if JucePlugin_ProducesMidiOutput
        true;
#else
        pluginIsMidiEffect;
#endif

constexpr auto pluginWantsMidiInput =
#if JucePlugin_WantsMidiInput
        true;
#else
        pluginIsMidiEffect;
#endif

//==============================================================================
using namespace juce;

static Array<void*> activePlugins, activeUIs;

static const AudioUnitPropertyID juceFilterObjectPropertyID = 0x1a45ffe9;

template <> struct ContainerDeletePolicy<const __CFString>   { static void destroy (const __CFString* o) { if (o != nullptr) CFRelease (o); } };

// make sure the audio processor is initialized before the AUBase class
struct AudioProcessorHolder
{
    AudioProcessorHolder()
    {
        // audio units do not have a notion of enabled or un-enabled buses
        juceFilter->enableAllBuses();
    }

    ScopedJuceInitialiser_GUI scopedInitialiser;
    std::unique_ptr<AudioProcessor> juceFilter { createPluginFilterOfType (AudioProcessor::wrapperType_AudioUnit) };
};

//==============================================================================
class JuceAU final : public AudioProcessorHolder,
                     public ausdk::MusicDeviceBase,
                     public AudioProcessorListener,
                     public AudioProcessorParameter::Listener
{
    auto& getSupportedBusLayouts (bool isInput, int bus) noexcept
    {
        auto& layout = isInput ? supportedInputLayouts : supportedOutputLayouts;
        jassert (isPositiveAndBelow (bus, layout.size()));
        return layout.getReference (bus);
    }

    auto& getCurrentLayout (bool isInput, int bus) noexcept
    {
        auto& layout = isInput ? currentInputLayout : currentOutputLayout;
        jassert (isPositiveAndBelow (bus, layout.size()));
        return layout.getReference (bus);
    }

public:
    explicit JuceAU (AudioUnit component)
        : MusicDeviceBase (component,
                           (UInt32) AudioUnitHelpers::getBusCountForWrapper (*juceFilter, true),
                           (UInt32) AudioUnitHelpers::getBusCountForWrapper (*juceFilter, false))
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
        auto channelInfoSet = AudioUnitHelpers::getAUChannelInfo (*juceFilter);
        channelInfo.resize ((int) channelInfoSet.size());
        std::transform (channelInfoSet.begin(),
                        channelInfoSet.end(),
                        channelInfo.begin(),
                        [] (auto x) { return x.makeChannelInfo(); });
       #endif

        AddPropertyListener (kAudioUnitProperty_ContextName, auPropertyListenerDispatcher, this);

        totalInChannels  = juceFilter->getTotalNumInputChannels();
        totalOutChannels = juceFilter->getTotalNumOutputChannels();

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
    }

    //==============================================================================
    ComponentResult Initialize() override
    {
        ComponentResult err;

        if ((err = syncProcessorWithAudioUnit()) != noErr)
            return err;

        if ((err = MusicDeviceBase::Initialize()) != noErr)
            return err;

        mapper.alloc (*juceFilter);
        pulledSucceeded.calloc (static_cast<size_t> (AudioUnitHelpers::getBusCountForWrapper (*juceFilter, true)));

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

            audioBuffer.prepare (AudioUnitHelpers::getBusesLayout (juceFilter.get()), (int) GetMaxFramesPerSlice() + 32);
            juceFilter->prepareToPlay (getSampleRate(), (int) GetMaxFramesPerSlice());

            midiEvents.ensureSize (2048);
            midiEvents.clear();
            incomingEvents.ensureSize (2048);
            incomingEvents.clear();

            prepared = true;
        }
    }

    //==============================================================================
    bool BusCountWritable ([[maybe_unused]] AudioUnitScope scope) override
    {
     #ifdef JucePlugin_PreferredChannelConfigurations
        return false;
     #else
        if constexpr (pluginIsMidiEffect)
            return false;

        const auto optIsInput = scopeToDirection (scope);

        if (! optIsInput.has_value())
            return false;

        const auto isInput = *optIsInput;

       #if JucePlugin_IsSynth
        if (isInput)
            return false;
       #endif

        const auto busCount = AudioUnitHelpers::getBusCount (*juceFilter, isInput);
        return (juceFilter->canAddBus (isInput) || (busCount > 0 && juceFilter->canRemoveBus (isInput)));
     #endif
    }

    OSStatus SetBusCount (AudioUnitScope scope, UInt32 count) override
    {
        const auto optIsInput = scopeToDirection (scope);

        if (! optIsInput.has_value())
            return kAudioUnitErr_InvalidScope;

        const auto isInput = *optIsInput;

        if (count != (UInt32) AudioUnitHelpers::getBusCount (*juceFilter, isInput))
        {
           #ifdef JucePlugin_PreferredChannelConfigurations
            return kAudioUnitErr_PropertyNotWritable;
           #else
            const int busCount = AudioUnitHelpers::getBusCount (*juceFilter, isInput);

            if  ((! juceFilter->canAddBus (isInput)) && ((busCount == 0) || (! juceFilter->canRemoveBus (isInput))))
                return kAudioUnitErr_PropertyNotWritable;

            // we need to already create the underlying elements so that we can change their formats
            auto err = MusicDeviceBase::SetBusCount (scope, count);

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
                const int newBusCount = AudioUnitHelpers::getBusCount (*juceFilter, isInput);
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

            addSupportedLayoutTagsForDirection (isInput);

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
                                     bool& outWritable) override
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

               #if JUCE_APPLE_MIDI_EVENT_LIST_SUPPORTED
                case kAudioUnitProperty_AudioUnitMIDIProtocol:
                    outDataSize = sizeof (SInt32);
                    outWritable = false;
                    return noErr;

                case kAudioUnitProperty_HostMIDIProtocol:
                    outDataSize = sizeof (SInt32);
                    outWritable = true;
                    return noErr;
               #endif

               #if JUCE_AUDIOWORKGROUP_TYPES_AVAILABLE
                case kAudioUnitProperty_RenderContextObserver:
                    outDataSize = sizeof (AURenderContextObserver);
                    outWritable = false;
                    return noErr;
               #endif

                case kAudioUnitProperty_MIDIOutputCallbackInfo:
                    if constexpr (pluginProducesMidiOutput)
                    {
                        outDataSize = sizeof (CFArrayRef);
                        outWritable = false;
                        return noErr;
                    }
                    break;

                case kAudioUnitProperty_MIDIOutputCallback:
                    if constexpr (pluginProducesMidiOutput)
                    {
                        outDataSize = sizeof (AUMIDIOutputCallbackStruct);
                        outWritable = true;
                        return noErr;
                    }
                    break;

               #if JUCE_APPLE_MIDI_EVENT_LIST_SUPPORTED
                case kAudioUnitProperty_MIDIOutputEventListCallback:
                    if constexpr (pluginProducesMidiOutput)
                    {
                        outDataSize = sizeof (AUMIDIEventListBlock);
                        outWritable = true;
                        return noErr;
                    }
                    break;
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

               #if JucePlugin_Enable_ARA
                case ARA::kAudioUnitProperty_ARAFactory:
                    outWritable = false;
                    outDataSize = sizeof (ARA::ARAAudioUnitFactory);
                    return noErr;
                case ARA::kAudioUnitProperty_ARAPlugInExtensionBindingWithRoles:
                    outWritable = false;
                    outDataSize = sizeof (ARA::ARAAudioUnitPlugInExtensionBinding);
                    return noErr;
               #endif

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

                    //==============================================================================
               #if JucePlugin_Enable_ARA
                case ARA::kAudioUnitProperty_ARAFactory:
                {
                    auto auFactory = static_cast<ARA::ARAAudioUnitFactory*> (outData);
                    if (auFactory->inOutMagicNumber != ARA::kARAAudioUnitMagic)
                        return kAudioUnitErr_InvalidProperty;   // if the magic value isn't found, the property ID is re-used outside the ARA context with different, unsupported sematics

                    auFactory->outFactory = createARAFactory();
                    return noErr;
                }

                case ARA::kAudioUnitProperty_ARAPlugInExtensionBindingWithRoles:
                {
                    auto binding = static_cast<ARA::ARAAudioUnitPlugInExtensionBinding*> (outData);
                    if (binding->inOutMagicNumber != ARA::kARAAudioUnitMagic)
                        return kAudioUnitErr_InvalidProperty;   // if the magic value isn't found, the property ID is re-used outside the ARA context with different, unsupported sematics

                    AudioProcessorARAExtension* araAudioProcessorExtension = dynamic_cast<AudioProcessorARAExtension*> (juceFilter.get());
                    binding->outPlugInExtension = araAudioProcessorExtension->bindToARA (binding->inDocumentControllerRef, binding->knownRoles, binding->assignedRoles);
                    if (binding->outPlugInExtension == nullptr)
                        return kAudioUnitErr_CannotDoInCurrentContext;  // bindToARA() returns null if binding is already established

                    return noErr;
                }
               #endif

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

               #if JUCE_APPLE_MIDI_EVENT_LIST_SUPPORTED
                case kAudioUnitProperty_AudioUnitMIDIProtocol:
                {
                    // This will become configurable in the future
                    *static_cast<SInt32*> (outData) = kMIDIProtocol_1_0;
                    return noErr;
                }
               #endif

               #if JUCE_AUDIOWORKGROUP_TYPES_AVAILABLE
                case kAudioUnitProperty_RenderContextObserver:
                {
                    AURenderContextObserver callback = ^(const AudioUnitRenderContext* context)
                    {
                        jassert (juceFilter != nullptr);
                        const auto workgroup = makeRealAudioWorkgroup (context != nullptr ? context->workgroup : nullptr);
                        juceFilter->audioWorkgroupContextChanged (workgroup);
                    };

                    *(AURenderContextObserver*) outData = [callback copy];
                    return noErr;
                }
               #endif

                case kAudioUnitProperty_MIDIOutputCallbackInfo:
                    if constexpr (pluginProducesMidiOutput)
                    {
                        CFStringRef strs[1];
                        strs[0] = CFSTR ("MIDI Callback");

                        CFArrayRef callbackArray = CFArrayCreate (nullptr, (const void**) strs, 1, &kCFTypeArrayCallBacks);
                        *(CFArrayRef*) outData = callbackArray;
                        return noErr;
                    }
                    break;

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
                case kAudioUnitProperty_MIDIOutputCallback:
                    if constexpr (pluginProducesMidiOutput)
                    {
                        if (inDataSize < sizeof (AUMIDIOutputCallbackStruct))
                            return kAudioUnitErr_InvalidPropertyValue;

                        if (AUMIDIOutputCallbackStruct* callbackStruct = (AUMIDIOutputCallbackStruct*) inData)
                            midiCallback = *callbackStruct;

                        return noErr;
                    }
                    break;

               #if JUCE_APPLE_MIDI_EVENT_LIST_SUPPORTED
                case kAudioUnitProperty_MIDIOutputEventListCallback:
                    if constexpr (pluginProducesMidiOutput)
                    {
                        if (inDataSize != sizeof (AUMIDIEventListBlock))
                            return kAudioUnitErr_InvalidPropertyValue;

                        if (@available (macos 12, *))
                            eventListOutput.setBlock (*static_cast<const AUMIDIEventListBlock*> (inData));

                        return noErr;
                    }
                    break;
               #endif

               #if JUCE_APPLE_MIDI_EVENT_LIST_SUPPORTED
                case kAudioUnitProperty_HostMIDIProtocol:
                {
                    if (inDataSize != sizeof (SInt32))
                        return kAudioUnitErr_InvalidPropertyValue;

                    hostProtocol = *static_cast<const SInt32*> (inData);
                    return noErr;
                }
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
                    const auto shouldBeOffline = (*reinterpret_cast<const UInt32*> (inData) != 0);

                    if (juceFilter != nullptr)
                    {
                        const auto isOffline = juceFilter->isNonRealtime();

                        if (isOffline != shouldBeOffline)
                        {
                            const ScopedLock sl (juceFilter->getCallbackLock());

                            juceFilter->setNonRealtime (shouldBeOffline);

                            if (prepared)
                                juceFilter->prepareToPlay (getSampleRate(), (int) GetMaxFramesPerSlice());
                        }
                    }

                    return noErr;
                }

                case kAudioUnitProperty_AUHostIdentifier:
                {
                    if (inDataSize < sizeof (AUHostVersionIdentifier))
                        return kAudioUnitErr_InvalidPropertyValue;

                    const auto* identifier = static_cast<const AUHostVersionIdentifier*> (inData);
                    PluginHostType::hostIdReportedByWrapper = String::fromCFString (identifier->hostName);

                    return noErr;
                }

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
                CFUniquePtr<CFDataRef> ourState (CFDataCreate (kCFAllocatorDefault, (const UInt8*) state.getData(), (CFIndex) state.getSize()));
                CFUniquePtr<CFStringRef> key (CFStringCreateWithCString (kCFAllocatorDefault, JUCE_STATE_DICTIONARY_KEY, kCFStringEncodingUTF8));
                CFDictionarySetValue (dict, key.get(), ourState.get());
            }
        }

        return noErr;
    }

    ComponentResult RestoreState (CFPropertyListRef inData) override
    {
        const ScopedValueSetter<bool> scope { restoringState, true };

        {
            // Remove the data entry from the state to prevent the superclass loading the parameters
            CFUniquePtr<CFMutableDictionaryRef> copyWithoutData (CFDictionaryCreateMutableCopy (nullptr, 0, (CFDictionaryRef) inData));
            CFDictionaryRemoveValue (copyWithoutData.get(), CFSTR (kAUPresetDataKey));

            auto* originalVersion = static_cast<CFNumberRef> (CFDictionaryGetValue (copyWithoutData.get(), CFSTR (kAUPresetVersionKey)));
            if (originalVersion != nullptr && CFGetTypeID (originalVersion) == CFNumberGetTypeID())
            {
                SInt32 value = 0;
                CFNumberGetValue (originalVersion, kCFNumberSInt32Type, &value);

                // Data with a version of "1" is generated by AUv3 plug-ins.
                // This data appears to be compatible with RestoreState below, but RestoreState
                // fails when "version" is not 0.
                // We only overwrite the version if it is 1 so that if future preset versions are
                // completely incompatible, RestoreState will be bypassed rather than passed data
                // which could put the plugin into a broken state.
                if (value == 1)
                {
                    const SInt32 zero = 0;
                    CFUniquePtr<CFNumberRef> newVersion (CFNumberCreate (nullptr, kCFNumberSInt32Type, &zero));
                    CFDictionarySetValue (copyWithoutData.get(), CFSTR (kAUPresetVersionKey), newVersion.get());
                }
            }

            ComponentResult err = MusicDeviceBase::RestoreState (copyWithoutData.get());

            if (err != noErr)
                return err;
        }

        if (juceFilter != nullptr)
        {
            CFDictionaryRef dict = (CFDictionaryRef) inData;
            CFDataRef data = nullptr;

            CFUniquePtr<CFStringRef> key (CFStringCreateWithCString (kCFAllocatorDefault, JUCE_STATE_DICTIONARY_KEY, kCFStringEncodingUTF8));

            bool valuePresent = CFDictionaryGetValueIfPresent (dict, key.get(), (const void**) &data);

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
    bool busIgnoresLayout ([[maybe_unused]] bool isInput, [[maybe_unused]] int busNr) const
    {
       #ifdef JucePlugin_PreferredChannelConfigurations
        return true;
       #else
        if (const AudioProcessor::Bus* bus = juceFilter->getBus (isInput, busNr))
        {
            AudioChannelSet discreteRangeSet;

           // Suppressing clang-analyzer-optin.core.EnumCastOutOfRange
           #ifndef __clang_analyzer__
            const int n = bus->getDefaultLayout().size();

            for (int i = 0; i < n; ++i)
                discreteRangeSet.addChannel ((AudioChannelSet::ChannelType) (256 + i));
           #endif

            // if the audioprocessor supports this it cannot
            // really be interested in the bus layouts
            return bus->isLayoutSupported (discreteRangeSet);
        }

        return true;
       #endif
    }

    UInt32 GetAudioChannelLayout (AudioUnitScope scope,
                                  AudioUnitElement element,
                                  AudioChannelLayout* outLayoutPtr,
                                  bool& outWritable) override
    {
        outWritable = false;

        const auto info = getElementInfo (scope, element);

        if (info.error != noErr)
            return 0;

        if (busIgnoresLayout (info.isInput, info.busNr))
            return 0;

        outWritable = true;

        const size_t sizeInBytes = sizeof (AudioChannelLayout) - sizeof (AudioChannelDescription);

        if (outLayoutPtr != nullptr)
        {
            zeromem (outLayoutPtr, sizeInBytes);
            outLayoutPtr->mChannelLayoutTag = getCurrentLayout (info.isInput, info.busNr);
        }

        return sizeInBytes;
    }

    std::vector<AudioChannelLayoutTag> GetChannelLayoutTags (AudioUnitScope inScope, AudioUnitElement inElement) override
    {
        const auto info = getElementInfo (inScope, inElement);

        if (info.error != noErr)
            return {};

        if (busIgnoresLayout (info.isInput, info.busNr))
            return {};

        return getSupportedBusLayouts (info.isInput, info.busNr);
    }

    OSStatus SetAudioChannelLayout (AudioUnitScope scope, AudioUnitElement element, const AudioChannelLayout* inLayout) override
    {
        const auto info = getElementInfo (scope, element);

        if (info.error != noErr)
            return info.error;

        if (busIgnoresLayout (info.isInput, info.busNr))
            return kAudioUnitErr_PropertyNotWritable;

        if (inLayout == nullptr)
            return kAudioUnitErr_InvalidPropertyValue;

        auto& ioElement = IOElement (info.isInput ? kAudioUnitScope_Input : kAudioUnitScope_Output, element);

        const AudioChannelSet newChannelSet = CoreAudioLayouts::fromCoreAudio (*inLayout);
        const int currentNumChannels = static_cast<int> (ioElement.NumberChannels());
        const int newChannelNum = newChannelSet.size();

        if (currentNumChannels != newChannelNum)
            return kAudioUnitErr_InvalidPropertyValue;

        // check if the new layout could be potentially set
       #ifdef JucePlugin_PreferredChannelConfigurations
        short configs[][2] = {JucePlugin_PreferredChannelConfigurations};

        if (! AudioUnitHelpers::isLayoutSupported (*juceFilter, info.isInput, info.busNr, newChannelNum, configs))
            return kAudioUnitErr_FormatNotSupported;
       #else
        if (! juceFilter->getBus (info.isInput, info.busNr)->isLayoutSupported (newChannelSet))
            return kAudioUnitErr_FormatNotSupported;
       #endif

        getCurrentLayout (info.isInput, info.busNr) = CoreAudioLayouts::toCoreAudio (newChannelSet);

        return noErr;
    }

    //==============================================================================
    // When parameters are discrete we need to use integer values.
    float getMaximumParameterValue ([[maybe_unused]] AudioProcessorParameter* juceParam)
    {
       #if JUCE_FORCE_LEGACY_PARAMETER_AUTOMATION_TYPE
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

                if (! approximatelyEqual (value, param->getValue()))
                {
                    inParameterChangedCallback = true;
                    param->setValueNotifyingHost (value);
                }

                return noErr;
            }
        }

        return MusicDeviceBase::SetParameter (inID, inScope, inElement, inValue, inBufferOffsetInFrames);
    }

    // No idea what this method actually does or what it should return. Current Apple docs say nothing about it.
    // (Note that this isn't marked 'override' in case older versions of the SDK don't include it)
    bool CanScheduleParameters() const override          { return false; }

    //==============================================================================
    bool SupportsTail() override                         { return true; }
    Float64 GetTailTime() override                       { return juceFilter->getTailLengthSeconds(); }

    double getSampleRate()
    {
        if (AudioUnitHelpers::getBusCountForWrapper (*juceFilter, false) > 0)
            return Output (0).GetStreamFormat().mSampleRate;

        return 44100.0;
    }

    Float64 GetLatency() override
    {
        const double rate = getSampleRate();
        jassert (rate > 0);
       #if JucePlugin_Enable_ARA
        jassert (juceFilter->getLatencySamples() == 0 || ! dynamic_cast<AudioProcessorARAExtension*> (juceFilter.get())->isBoundToARA());
       #endif
        return rate > 0 ? juceFilter->getLatencySamples() / rate : 0;
    }

    class ScopedPlayHead final : private AudioPlayHead
    {
    public:
        explicit ScopedPlayHead (JuceAU& juceAudioUnit)
            : audioUnit (juceAudioUnit)
        {
            audioUnit.juceFilter->setPlayHead (this);
        }

        ~ScopedPlayHead() override
        {
            audioUnit.juceFilter->setPlayHead (nullptr);
        }

    private:
        Optional<PositionInfo> getPosition() const override
        {
            PositionInfo info;

            info.setFrameRate ([this]() -> Optional<FrameRate>
            {
                switch (audioUnit.lastTimeStamp.mSMPTETime.mType)
                {
                    case kSMPTETimeType2398:        return FrameRate().withBaseRate (24).withPullDown();
                    case kSMPTETimeType24:          return FrameRate().withBaseRate (24);
                    case kSMPTETimeType25:          return FrameRate().withBaseRate (25);
                    case kSMPTETimeType30Drop:      return FrameRate().withBaseRate (30).withDrop();
                    case kSMPTETimeType30:          return FrameRate().withBaseRate (30);
                    case kSMPTETimeType2997:        return FrameRate().withBaseRate (30).withPullDown();
                    case kSMPTETimeType2997Drop:    return FrameRate().withBaseRate (30).withPullDown().withDrop();
                    case kSMPTETimeType60:          return FrameRate().withBaseRate (60);
                    case kSMPTETimeType60Drop:      return FrameRate().withBaseRate (60).withDrop();
                    case kSMPTETimeType5994:        return FrameRate().withBaseRate (60).withPullDown();
                    case kSMPTETimeType5994Drop:    return FrameRate().withBaseRate (60).withPullDown().withDrop();
                    case kSMPTETimeType50:          return FrameRate().withBaseRate (50);
                    default:                        break;
                }

                return {};
            }());

            double ppqPosition = 0.0;
            double bpm = 0.0;

            if (audioUnit.CallHostBeatAndTempo (&ppqPosition, &bpm) == noErr)
            {
                info.setPpqPosition (ppqPosition);
                info.setBpm (bpm);
            }

            UInt32 outDeltaSampleOffsetToNextBeat;
            double outCurrentMeasureDownBeat;
            float num;
            UInt32 den;

            if (audioUnit.CallHostMusicalTimeLocation (&outDeltaSampleOffsetToNextBeat,
                                                       &num,
                                                       &den,
                                                       &outCurrentMeasureDownBeat) == noErr)
            {
                info.setTimeSignature (TimeSignature { (int) num, (int) den });
                info.setPpqPositionOfLastBarStart (outCurrentMeasureDownBeat);
            }

            double outCurrentSampleInTimeLine = 0, outCycleStartBeat = 0, outCycleEndBeat = 0;
            Boolean playing = false, looping = false, playchanged;

            const auto setTimeInSamples = [&] (auto timeInSamples)
            {
                info.setTimeInSamples ((int64) (timeInSamples + 0.5));
                info.setTimeInSeconds ((double) (*info.getTimeInSamples()) / audioUnit.getSampleRate());
            };

            if (audioUnit.CallHostTransportState (&playing,
                                                  &playchanged,
                                                  &outCurrentSampleInTimeLine,
                                                  &looping,
                                                  &outCycleStartBeat,
                                                  &outCycleEndBeat) == noErr)
            {
                info.setIsPlaying (playing);
                info.setIsLooping (looping);
                info.setLoopPoints (LoopPoints { outCycleStartBeat, outCycleEndBeat });
                setTimeInSamples (outCurrentSampleInTimeLine);
            }
            else
            {
                // If the host doesn't support this callback, then use the sample time from lastTimeStamp
                setTimeInSamples (audioUnit.lastTimeStamp.mSampleTime);
            }

            info.setHostTimeNs ((audioUnit.lastTimeStamp.mFlags & kAudioTimeStampHostTimeValid) != 0
                                ? makeOptional (audioUnit.timeConversions.hostTimeToNanos (audioUnit.lastTimeStamp.mHostTime))
                                : nullopt);

            return info;
        }

        JuceAU& audioUnit;
    };

    //==============================================================================
    void sendAUEvent (const AudioUnitEventType type, const int juceParamIndex)
    {
        if (restoringState)
            return;

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
        audioProcessorChangedUpdater.update (details);
    }

    //==============================================================================
    // this will only ever be called by the bypass parameter
    void parameterValueChanged (int, float) override
    {
        if (! restoringState)
            PropertyChanged (kAudioUnitProperty_BypassEffect, kAudioUnitScope_Global, 0);
    }

    void parameterGestureChanged (int, bool) override {}

    //==============================================================================
    bool StreamFormatWritable (AudioUnitScope scope, AudioUnitElement element) override
    {
        const auto info = getElementInfo (scope, element);

        return ((! IsInitialized()) && (info.error == noErr));
    }

    bool ValidFormat (AudioUnitScope inScope,
                      AudioUnitElement inElement,
                      const AudioStreamBasicDescription& inNewFormat) override
    {
        // DSP Quattro incorrectly uses global scope for the ValidFormat call
        if (inScope == kAudioUnitScope_Global)
            return ValidFormat (kAudioUnitScope_Input,  inElement, inNewFormat)
                || ValidFormat (kAudioUnitScope_Output, inElement, inNewFormat);

        const auto info = getElementInfo (inScope, inElement);

        if (info.error != noErr)
            return false;

        if (info.kind == BusKind::wrapperOnly)
            return true;

        const auto newNumChannels = static_cast<int> (inNewFormat.mChannelsPerFrame);
        const auto oldNumChannels = juceFilter->getChannelCountOfBus (info.isInput, info.busNr);

        if (newNumChannels == oldNumChannels)
            return true;

        if ([[maybe_unused]] AudioProcessor::Bus* bus = juceFilter->getBus (info.isInput, info.busNr))
        {
            if (! MusicDeviceBase::ValidFormat (inScope, inElement, inNewFormat))
                return false;

           #ifdef JucePlugin_PreferredChannelConfigurations
            short configs[][2] = {JucePlugin_PreferredChannelConfigurations};

            return AudioUnitHelpers::isLayoutSupported (*juceFilter, info.isInput, info.busNr, newNumChannels, configs);
           #else
            return bus->isNumberOfChannelsSupported (newNumChannels);
           #endif
        }

        return false;
    }

    // AU requires us to override this for the sole reason that we need to find a default layout tag if the number of channels have changed
    OSStatus ChangeStreamFormat (AudioUnitScope inScope,
                                 AudioUnitElement inElement,
                                 const AudioStreamBasicDescription& inPrevFormat,
                                 const AudioStreamBasicDescription& inNewFormat) override
    {
        const auto info = getElementInfo (inScope, inElement);

        if (info.error != noErr)
            return info.error;

        AudioChannelLayoutTag& currentTag = getCurrentLayout (info.isInput, info.busNr);

        const auto newNumChannels = static_cast<int> (inNewFormat.mChannelsPerFrame);
        const auto oldNumChannels = juceFilter->getChannelCountOfBus (info.isInput, info.busNr);

       #ifdef JucePlugin_PreferredChannelConfigurations
        short configs[][2] = {JucePlugin_PreferredChannelConfigurations};

        if (! AudioUnitHelpers::isLayoutSupported (*juceFilter, info.isInput, info.busNr, newNumChannels, configs))
            return kAudioUnitErr_FormatNotSupported;
       #endif

        // predict channel layout
        const auto set = [&]
        {
            if (info.kind == BusKind::wrapperOnly)
                return AudioChannelSet::discreteChannels (newNumChannels);

            if (newNumChannels != oldNumChannels)
                return juceFilter->getBus (info.isInput, info.busNr)->supportedLayoutWithChannels (newNumChannels);

            return juceFilter->getChannelLayoutOfBus (info.isInput, info.busNr);
        }();

        if (set == AudioChannelSet())
            return kAudioUnitErr_FormatNotSupported;

        const auto err = MusicDeviceBase::ChangeStreamFormat (inScope, inElement, inPrevFormat, inNewFormat);

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

        const int numInputBuses  = AudioUnitHelpers::getBusCount (*juceFilter, true);
        const int numOutputBuses = AudioUnitHelpers::getBusCount (*juceFilter, false);

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
                    audioBuffer.set (busIdx, Input ((UInt32) busIdx).GetBufferList(), mapper.get (true, busIdx));
                else
                    audioBuffer.clearInputBus (busIdx, (int) nFrames);
            }

            audioBuffer.clearUnusedChannels ((int) nFrames);
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
                audioBuffer.get (busIdx, Output ((UInt32) busIdx).GetBufferList(), mapper.get (false, busIdx));
        }

        // process midi output
        if constexpr (pluginProducesMidiOutput)
            pushMidiOutput (nFrames);

        midiEvents.clear();

        return noErr;
    }

    //==============================================================================
    ComponentResult StartNote (MusicDeviceInstrumentID, MusicDeviceGroupID, NoteInstanceID*, UInt32, const MusicDeviceNoteParams&) override { return noErr; }
    ComponentResult StopNote (MusicDeviceGroupID, NoteInstanceID, UInt32) override   { return noErr; }

    //==============================================================================
    OSStatus HandleMIDIEvent ([[maybe_unused]] UInt8 inStatus,
                              [[maybe_unused]] UInt8 inChannel,
                              [[maybe_unused]] UInt8 inData1,
                              [[maybe_unused]] UInt8 inData2,
                              [[maybe_unused]] UInt32 inStartFrame) override
    {
        if constexpr (pluginWantsMidiInput)
        {
            const juce::uint8 data[] = { (juce::uint8) (inStatus | inChannel),
                                         (juce::uint8) inData1,
                                         (juce::uint8) inData2 };

            const ScopedLock sl (incomingMidiLock);
            incomingEvents.addEvent (data, 3, (int) inStartFrame);
            return noErr;
        }

        return kAudioUnitErr_PropertyNotInUse;
    }

    OSStatus HandleSysEx ([[maybe_unused]] const UInt8* inData, [[maybe_unused]] UInt32 inLength) override
    {
        if constexpr (pluginWantsMidiInput)
        {
            const ScopedLock sl (incomingMidiLock);
            incomingEvents.addEvent (inData, (int) inLength, 0);
            return noErr;
        }

        return kAudioUnitErr_PropertyNotInUse;
    }

   #if JUCE_APPLE_MIDI_EVENT_LIST_SUPPORTED
    OSStatus MIDIEventList (UInt32 inOffsetSampleFrame, const struct MIDIEventList* list) override
    {
        const ScopedLock sl (incomingMidiLock);

        auto* packet = &list->packet[0];

        for (uint32_t i = 0; i < list->numPackets; ++i)
        {
            toBytestreamDispatcher.dispatch (reinterpret_cast<const uint32_t*> (packet->words),
                                             reinterpret_cast<const uint32_t*> (packet->words + packet->wordCount),
                                             static_cast<double> (packet->timeStamp + inOffsetSampleFrame),
                                             [this] (const ump::BytestreamMidiView& message)
                                             {
                                                 incomingEvents.addEvent (message.getMessage(), (int) message.timestamp);
                                             });

            packet = MIDIEventPacketNext (packet);
        }

        return noErr;
    }
   #endif

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
                String name (juceFilter->getProgramName (i));
                if (name.isEmpty())
                    name = "Untitled";

                AUPreset& p = presetsArray.getReference (i);
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
    class EditorCompHolder final : public Component
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
            auto r = convertToHostBounds (makeCGRect (editorCompHolder->getSizeToContainChild()));

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

            editorCompHolder->addToDesktop (detail::PluginUtilities::getDesktopFlags (editor), view);
            editorCompHolder->setVisible (true);

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
            if (detail::PluginUtilities::getHostType().isAbletonLive())
            {
                NSEvent* currentEvent = [NSApp currentEvent];

                if (currentEvent != nil)
                {
                    static NSTimeInterval lastEventTime = 0; // check we're not recursively sending the same event
                    NSTimeInterval eventTime = [currentEvent timestamp];

                    if (! approximatelyEqual (lastEventTime, eventTime))
                    {
                        lastEventTime = eventTime;

                        if (auto* peer = getPeer())
                            if (detail::ComponentPeerHelpers::isInPerformKeyEquivalent (*peer))
                                return false;

                        auto* view = (NSView*) getWindowHandle();
                        auto* hostView = [view superview];

                        [[hostView window] makeFirstResponder: hostView];
                        [hostView keyDown: currentEvent];

                        if ((hostView = [view superview]))
                            if (auto* hostWindow = [hostView window])
                                [hostWindow makeFirstResponder: view];
                    }
                }
            }

            return false;
        }

        void resizeHostWindow()
        {
            [CATransaction begin];
            [CATransaction setValue: (id) kCFBooleanTrue forKey:kCATransactionDisableActions];

            auto rect = convertToHostBounds (makeCGRect (lastBounds));
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
        ScopedJuceInitialiser_GUI scopedInitialiser;
        Rectangle<int> lastBounds;

        JUCE_DECLARE_NON_COPYABLE (EditorCompHolder)
    };

    void deleteActiveEditors()
    {
        for (int i = activeUIs.size(); --i >= 0;)
        {
            id ui = (id) activeUIs.getUnchecked (i);

            if (JuceUIViewClass::getAU (ui) == this)
                JuceUIViewClass::deleteEditor (ui);
        }
    }

    //==============================================================================
    struct JuceUIViewClass final : public ObjCClass<NSView>
    {
        JuceUIViewClass()  : ObjCClass<NSView> ("JUCEAUView_")
        {
            addIvar<AudioProcessor*> ("filter");
            addIvar<JuceAU*> ("au");
            addIvar<EditorCompHolder*> ("editor");

            addMethod (@selector (dealloc), [] (id self, SEL)
            {
                if (activeUIs.contains (self))
                    shutdown (self);

                sendSuperclassMessage<void> (self, @selector (dealloc));
            });

            addMethod (@selector (applicationWillTerminate:), [] (id self, SEL, NSNotification*)
            {
                shutdown (self);
            });

            addMethod (@selector (viewDidMoveToWindow), [] (id self, SEL)
            {
                if (NSWindow* w = [(NSView*) self window])
                {
                    [w setAcceptsMouseMovedEvents: YES];

                    if (EditorCompHolder* const editorComp = getEditor (self))
                        [w makeFirstResponder: (NSView*) editorComp->getWindowHandle()];
                }
            });

            addMethod (@selector (mouseDownCanMoveWindow), [] (id, SEL)
            {
                return NO;
            });

            JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wundeclared-selector")
            addMethod (@selector (clipsToBounds), [] (id, SEL) { return YES; });
            JUCE_END_IGNORE_WARNINGS_GCC_LIKE

            registerClass();
        }

        static void deleteEditor (id self)
        {
            std::unique_ptr<EditorCompHolder> editorComp (getEditor (self));

            if (editorComp != nullptr)
            {
                if (editorComp->getChildComponent (0) != nullptr
                     && activePlugins.contains (getAU (self))) // plugin may have been deleted before the UI
                {
                    AudioProcessor* const filter = getIvar<AudioProcessor*> (self, "filter");
                    filter->editorBeingDeleted ((AudioProcessorEditor*) editorComp->getChildComponent (0));
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
                jassert (ModalComponentManager::getInstanceWithoutCreating() == nullptr
                         || Component::getCurrentlyModalComponent() == nullptr);
            }
        }
    };

    //==============================================================================
    struct JuceUICreationClass final : public ObjCClass<NSObject>
    {
        JuceUICreationClass()  : ObjCClass<NSObject> ("JUCE_AUCocoaViewClass_")
        {
            addMethod (@selector (interfaceVersion), [] (id, SEL) { return 0; });
            addMethod (@selector (description), [] (id, SEL)
            {
                return [NSString stringWithString: nsStringLiteral (JucePlugin_Name)];
            });

            addMethod (@selector (uiViewForAudioUnit:withSize:), [] (id, SEL, AudioUnit inAudioUnit, NSSize) -> NSView*
            {
                void* pointers[2];
                UInt32 propertySize = sizeof (pointers);

                if (AudioUnitGetProperty (inAudioUnit, juceFilterObjectPropertyID,
                                          kAudioUnitScope_Global, 0, pointers, &propertySize) == noErr)
                {
                    if (AudioProcessor* filter = static_cast<AudioProcessor*> (pointers[0]))
                    {
                        if (AudioProcessorEditor* editorComp = filter->createEditorIfNeeded())
                        {
                           #if JucePlugin_Enable_ARA
                            jassert (dynamic_cast<AudioProcessorEditorARAExtension*> (editorComp) != nullptr);
                            // for proper view embedding, ARA plug-ins must be resizable
                            jassert (editorComp->isResizable());
                           #endif
                            return EditorCompHolder::createViewFor (filter, static_cast<JuceAU*> (pointers[1]), editorComp);
                        }
                    }
                }

                return nil;
            });

            addProtocol (@protocol (AUCocoaUIBase));

            registerClass();
        }
    };

private:
    //==============================================================================
    /*  The call to AUBase::PropertyChanged may allocate hence the need for this class */
    class AudioProcessorChangedUpdater final  : private AsyncUpdater
    {
    public:
        explicit AudioProcessorChangedUpdater (JuceAU& o) : owner (o) {}
        ~AudioProcessorChangedUpdater() override { cancelPendingUpdate(); }

        void update (const ChangeDetails& details)
        {
            int flags = 0;

            if (details.latencyChanged)
                flags |= latencyChangedFlag;

            if (details.parameterInfoChanged)
                flags |= parameterInfoChangedFlag;

            if (details.programChanged)
                flags |= programChangedFlag;

            if (flags != 0)
            {
                callbackFlags.fetch_or (flags);

                if (MessageManager::getInstance()->isThisTheMessageThread())
                    handleAsyncUpdate();
                else
                    triggerAsyncUpdate();
            }
        }

    private:
        void handleAsyncUpdate() override
        {
            const auto flags = callbackFlags.exchange (0);

            if ((flags & latencyChangedFlag) != 0)
                owner.PropertyChanged (kAudioUnitProperty_Latency, kAudioUnitScope_Global, 0);

            if ((flags & parameterInfoChangedFlag) != 0)
            {
                owner.PropertyChanged (kAudioUnitProperty_ParameterList, kAudioUnitScope_Global, 0);
                owner.PropertyChanged (kAudioUnitProperty_ParameterInfo, kAudioUnitScope_Global, 0);
            }

            owner.PropertyChanged (kAudioUnitProperty_ClassInfo, kAudioUnitScope_Global, 0);

            if ((flags & programChangedFlag) != 0)
            {
                owner.refreshCurrentPreset();
                owner.PropertyChanged (kAudioUnitProperty_PresentPreset, kAudioUnitScope_Global, 0);
            }
        }

        JuceAU& owner;

        static constexpr int latencyChangedFlag       = 1 << 0,
                             parameterInfoChangedFlag = 1 << 1,
                             programChangedFlag       = 1 << 2;

        std::atomic<int> callbackFlags { 0 };
    };

    //==============================================================================
    AudioUnitHelpers::CoreAudioBufferList audioBuffer;
    MidiBuffer midiEvents, incomingEvents;
    bool prepared = false, isBypassed = false, restoringState = false;

    //==============================================================================
   #if JUCE_FORCE_USE_LEGACY_PARAM_IDS
    static constexpr bool forceUseLegacyParamIDs = true;
   #else
    static constexpr bool forceUseLegacyParamIDs = false;
   #endif

    //==============================================================================
    LegacyAudioParametersWrapper juceParameters;
    std::unordered_map<int32, AudioProcessorParameter*> paramMap;
    Array<AudioUnitParameterID> auParamIDs;
    Array<const AudioProcessorParameterGroup*> parameterGroups;

    // Stores the parameter IDs in the order that they will be reported to the host.
    std::vector<AudioUnitParameterID> cachedParameterList;

    //==============================================================================
    // According to the docs, this is the maximum size of a MIDIPacketList.
    static constexpr UInt32 packetListBytes = 65536;

    CoreAudioTimeConversions timeConversions;
    AudioUnitEvent auEvent;
    mutable Array<AUPreset> presetsArray;
    CriticalSection incomingMidiLock;
    AUMIDIOutputCallbackStruct midiCallback;

   #if JUCE_APPLE_MIDI_EVENT_LIST_SUPPORTED
    AudioUnitHelpers::EventListOutput eventListOutput;
    std::optional<SInt32> hostProtocol;
    ump::ToBytestreamDispatcher toBytestreamDispatcher { 2048 };
   #endif

    AudioTimeStamp lastTimeStamp;
    int totalInChannels, totalOutChannels;
    HeapBlock<bool> pulledSucceeded;
    HeapBlock<MIDIPacketList> packetList { packetListBytes, 1 };

    ThreadLocalValue<bool> inParameterChangedCallback;

    AudioProcessorChangedUpdater audioProcessorChangedUpdater { *this };

    //==============================================================================
    Array<AUChannelInfo> channelInfo;
    Array<std::vector<AudioChannelLayoutTag>> supportedInputLayouts, supportedOutputLayouts;
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
            auto& input = Input (i);

            const bool succeeded = (input.PullInput (flags, timestamp, i, nFrames) == noErr);

            if ((flags & kAudioUnitRenderAction_OutputIsSilence) != 0 && succeeded)
                AudioUnitHelpers::clearAudioBuffer (input.GetBufferList());

            pulledSucceeded[i] = succeeded;
        }
    }

    void prepareOutputBuffers (const UInt32 nFrames) noexcept
    {
        const auto numProcessorBuses = AudioUnitHelpers::getBusCount (*juceFilter, false);
        const auto numWrapperBuses = GetScope (kAudioUnitScope_Output).GetNumberOfElements();

        for (UInt32 busIdx = 0; busIdx < numWrapperBuses; ++busIdx)
        {
            auto& output = Output (busIdx);

            if (output.WillAllocateBuffer())
                output.PrepareBuffer (nFrames);

            if (busIdx >= (UInt32) numProcessorBuses)
                AudioUnitHelpers::clearAudioBuffer (output.GetBufferList());
        }
    }

    void processBlock (juce::AudioBuffer<float>& buffer, MidiBuffer& midiBuffer) noexcept
    {
        const ScopedLock sl (juceFilter->getCallbackLock());
        const ScopedPlayHead playhead { *this };

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

    void pushMidiOutput ([[maybe_unused]] UInt32 nFrames) noexcept
    {
        if (midiEvents.isEmpty())
            return;

       #if JUCE_APPLE_MIDI_EVENT_LIST_SUPPORTED
        if (@available (macOS 12.0, iOS 15.0, *))
        {
            if (eventListOutput.trySend (midiEvents, (int64_t) lastTimeStamp.mSampleTime))
                return;
        }
       #endif

        if (midiCallback.midiOutputCallback)
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
    }

    void GetAudioBufferList (bool isInput, int busIdx, AudioBufferList*& bufferList, bool& interleaved, int& numChannels)
    {
        auto* element = Element (isInput ? kAudioUnitScope_Input : kAudioUnitScope_Output, static_cast<UInt32> (busIdx)).AsIOElement();
        jassert (element != nullptr);

        bufferList = &element->GetBufferList();

        jassert (bufferList->mNumberBuffers > 0);

        interleaved = AudioUnitHelpers::isAudioBufferInterleaved (*bufferList);
        numChannels = static_cast<int> (interleaved ? bufferList->mBuffers[0].mNumberChannels : bufferList->mNumberBuffers);
    }

    //==============================================================================
    static std::optional<bool> scopeToDirection (AudioUnitScope scope) noexcept
    {
        if (scope != kAudioUnitScope_Input && scope != kAudioUnitScope_Output)
            return {};

        return scope == kAudioUnitScope_Input;
    }

    enum class BusKind
    {
        processor,
        wrapperOnly,
    };

    struct ElementInfo
    {
        int busNr;
        BusKind kind;
        bool isInput;
        OSStatus error;
    };

    ElementInfo getElementInfo (AudioUnitScope scope, AudioUnitElement element) noexcept
    {
        const auto optIsInput = scopeToDirection (scope);

        if (! optIsInput.has_value())
            return { {}, {}, {}, kAudioUnitErr_InvalidScope };

        const auto isInput = *optIsInput;
        const auto busIdx = static_cast<int> (element);

        if (isPositiveAndBelow (busIdx, AudioUnitHelpers::getBusCount (*juceFilter, isInput)))
            return { busIdx, BusKind::processor, isInput, noErr };

        if (isPositiveAndBelow (busIdx, AudioUnitHelpers::getBusCountForWrapper (*juceFilter, isInput)))
            return { busIdx, BusKind::wrapperOnly, isInput, noErr };

        return { {}, {}, {}, kAudioUnitErr_InvalidElement };
    }

    OSStatus GetParameterList (AudioUnitScope inScope, AudioUnitParameterID* outParameterList, UInt32& outNumParameters) override
    {
        if (forceUseLegacyParamIDs || inScope != kAudioUnitScope_Global)
            return MusicDeviceBase::GetParameterList (inScope, outParameterList, outNumParameters);

        outNumParameters = (UInt32) juceParameters.size();

        if (outParameterList == nullptr)
            return noErr;

        if (cachedParameterList.empty())
        {
            struct ParamInfo
            {
                AudioUnitParameterID identifier;
                int versionHint;
            };

            std::vector<ParamInfo> vec;
            vec.reserve (juceParameters.size());

            for (const auto* param : juceParameters)
                vec.push_back ({ generateAUParameterID (*param), param->getVersionHint() });

            std::sort        (vec.begin(), vec.end(), [] (auto a, auto b) { return a.identifier  < b.identifier; });
            std::stable_sort (vec.begin(), vec.end(), [] (auto a, auto b) { return a.versionHint < b.versionHint; });
            std::transform   (vec.begin(), vec.end(), std::back_inserter (cachedParameterList), [] (auto x) { return x.identifier; });
        }

        std::copy (cachedParameterList.begin(), cachedParameterList.end(), outParameterList);

        return noErr;
    }

    //==============================================================================
    void addParameters()
    {
        parameterGroups = juceFilter->getParameterTree().getSubgroups (true);

        juceParameters.update (*juceFilter, forceUseLegacyParamIDs);
        const int numParams = juceParameters.getNumParameters();

        if (forceUseLegacyParamIDs)
        {
            Globals()->UseIndexedParameters (static_cast<UInt32> (numParams));
        }
        else
        {
            for (auto* param : juceParameters)
            {
                const AudioUnitParameterID auParamID = generateAUParameterID (*param);

                // Consider yourself very unlucky if you hit this assertion. The hash codes of your
                // parameter ids are not unique.
                jassert (paramMap.find (static_cast<int32> (auParamID)) == paramMap.end());

                auParamIDs.add (auParamID);
                paramMap.emplace (static_cast<int32> (auParamID), param);
                Globals()->SetParameter (auParamID, param->getValue());
            }
        }

       #if JUCE_DEBUG
        // Some hosts can't handle the huge numbers of discrete parameter values created when
        // using the default number of steps.
        for (auto* param : juceParameters)
            if (param->isDiscrete())
                jassert (param->getNumSteps() != AudioProcessor::getDefaultNumParameterSteps());
       #endif

        parameterValueStringArrays.ensureStorageAllocated (numParams);

        for (auto* param : juceParameters)
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
    static AudioUnitParameterID generateAUParameterID (const AudioProcessorParameter& param)
    {
        const String& juceParamID = LegacyAudioParameter::getParamID (&param, forceUseLegacyParamIDs);
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
        const auto index = static_cast<int32> (address);

        if (forceUseLegacyParamIDs)
            return juceParameters.getParamForIndex (index);

        const auto iter = paramMap.find (index);
        return iter != paramMap.end() ? iter->second : nullptr;
    }

    //==============================================================================
    OSStatus syncAudioUnitWithProcessor()
    {
        OSStatus err = noErr;
        const auto numWrapperInputs  = AudioUnitHelpers::getBusCountForWrapper (*juceFilter, true);
        const auto numWrapperOutputs = AudioUnitHelpers::getBusCountForWrapper (*juceFilter, false);

        if ((err =  MusicDeviceBase::SetBusCount (kAudioUnitScope_Input,  static_cast<UInt32> (numWrapperInputs))) != noErr)
            return err;

        if ((err =  MusicDeviceBase::SetBusCount (kAudioUnitScope_Output, static_cast<UInt32> (numWrapperOutputs))) != noErr)
            return err;

        addSupportedLayoutTags();

        const auto numProcessorInputs  = AudioUnitHelpers::getBusCount (*juceFilter, true);
        const auto numProcessorOutputs = AudioUnitHelpers::getBusCount (*juceFilter, false);

        for (int i = 0; i < numProcessorInputs; ++i)
            if ((err = syncAudioUnitWithChannelSet (true,  i, juceFilter->getChannelLayoutOfBus (true,  i))) != noErr)
                return err;

        for (int i = 0; i < numProcessorOutputs; ++i)
            if ((err = syncAudioUnitWithChannelSet (false, i, juceFilter->getChannelLayoutOfBus (false, i))) != noErr)
                return err;

        return noErr;
    }

    OSStatus syncProcessorWithAudioUnit()
    {
        const int numInputBuses  = AudioUnitHelpers::getBusCount (*juceFilter, true);
        const int numOutputBuses = AudioUnitHelpers::getBusCount (*juceFilter, false);

        const int numInputElements  = static_cast<int> (GetScope (kAudioUnitScope_Input). GetNumberOfElements());
        const int numOutputElements = static_cast<int> (GetScope (kAudioUnitScope_Output).GetNumberOfElements());

        AudioProcessor::BusesLayout requestedLayouts;
        for (int dir = 0; dir < 2; ++dir)
        {
            const bool isInput = (dir == 0);
            const int n = (isInput ? numInputBuses : numOutputBuses);
            const int numAUElements  = (isInput ? numInputElements : numOutputElements);
            Array<AudioChannelSet>& requestedBuses = (isInput ? requestedLayouts.inputBuses : requestedLayouts.outputBuses);

            for (int busIdx = 0; busIdx < n; ++busIdx)
            {
                const auto* element = (busIdx < numAUElements ? &IOElement (isInput ? kAudioUnitScope_Input : kAudioUnitScope_Output, (UInt32) busIdx) : nullptr);
                const int numChannels = (element != nullptr ? static_cast<int> (element->NumberChannels()) : 0);

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

        auto& element = IOElement (isInput ? kAudioUnitScope_Input :  kAudioUnitScope_Output, (UInt32) busNr);

        element.SetName ((CFStringRef) juceStringToNS (juceFilter->getBus (isInput, busNr)->getName()));

        const auto streamDescription = ausdk::ASBD::CreateCommonFloat32 (getSampleRate(), (UInt32) numChannels);
        return element.SetStreamFormat (streamDescription);
    }

    //==============================================================================
    void clearPresetsArray() const
    {
        for (int i = presetsArray.size(); --i >= 0;)
            CFRelease (presetsArray.getReference (i).presetName);

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
    std::vector<AudioChannelLayoutTag> getSupportedLayoutTagsForBus (bool isInput, int busNum) const
    {
        std::set<AudioChannelLayoutTag> tags;

        if (AudioProcessor::Bus* bus = juceFilter->getBus (isInput, busNum))
        {
           #ifndef JucePlugin_PreferredChannelConfigurations
            auto& knownTags = CoreAudioLayouts::getKnownCoreAudioTags();

            for (auto tag : knownTags)
                if (bus->isLayoutSupported (CoreAudioLayouts::fromCoreAudio (tag)))
                    tags.insert (tag);
           #endif

            // add discrete layout tags
            int n = bus->getMaxSupportedChannels (maxChannelsToProbeFor());

            for (int ch = 0; ch < n; ++ch)
            {
               #ifdef JucePlugin_PreferredChannelConfigurations
                const short configs[][2] = { JucePlugin_PreferredChannelConfigurations };
                if (AudioUnitHelpers::isLayoutSupported (*juceFilter, isInput, busNum, ch, configs))
                    tags.insert (static_cast<AudioChannelLayoutTag> ((int) kAudioChannelLayoutTag_DiscreteInOrder | ch));
               #else
                if (bus->isLayoutSupported (AudioChannelSet::discreteChannels (ch)))
                    tags.insert (static_cast<AudioChannelLayoutTag> ((int) kAudioChannelLayoutTag_DiscreteInOrder | ch));
               #endif
            }
        }

        return std::vector<AudioChannelLayoutTag> (tags.begin(), tags.end());
    }

    void addSupportedLayoutTagsForDirection (bool isInput)
    {
        auto& layouts = isInput ? supportedInputLayouts : supportedOutputLayouts;
        layouts.clearQuick();
        auto numBuses = AudioUnitHelpers::getBusCount (*juceFilter, isInput);

        for (int busNr = 0; busNr < numBuses; ++busNr)
            layouts.add (getSupportedLayoutTagsForBus (isInput, busNr));
    }

    void addSupportedLayoutTags()
    {
        for (auto& [layout, isInput] : { std::tuple (&currentInputLayout,  true),
                                         std::tuple (&currentOutputLayout, false) })
        {
            layout->clear();
            layout->resize (AudioUnitHelpers::getBusCountForWrapper (*juceFilter, isInput));
            addSupportedLayoutTagsForDirection (isInput);
        }
    }

    static int maxChannelsToProbeFor()
    {
        return (detail::PluginUtilities::getHostType().isLogic() ? 8 : 64);
    }

    //==============================================================================
    void auPropertyListener (AudioUnitPropertyID propId, AudioUnitScope scope, AudioUnitElement)
    {
        if (scope == kAudioUnitScope_Global && propId == kAudioUnitProperty_ContextName
             && juceFilter != nullptr && GetContextName() != nullptr)
        {
            AudioProcessor::TrackProperties props;
            props.name = std::make_optional (String::fromCFString (GetContextName()));

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
             extern "C" void* JuceAUFactory (const AudioComponentDescription* inDesc);
AUSDK_EXPORT extern "C" void* JuceAUFactory (const AudioComponentDescription* inDesc)
{
    if constexpr (pluginWantsMidiInput || pluginProducesMidiOutput)
        return ausdk::AUMusicDeviceFactory<JuceAU>::Factory (inDesc);
    else
        return ausdk::AUBaseFactory<JuceAU>::Factory (inDesc);
}

#define JUCE_AU_ENTRY_POINT_NAME JUCE_CONCAT (JucePlugin_AUExportPrefix, Factory)

             extern "C" void* JUCE_AU_ENTRY_POINT_NAME (const AudioComponentDescription* inDesc);
AUSDK_EXPORT extern "C" void* JUCE_AU_ENTRY_POINT_NAME (const AudioComponentDescription* inDesc)
{
    return JuceAUFactory (inDesc);
}

#endif

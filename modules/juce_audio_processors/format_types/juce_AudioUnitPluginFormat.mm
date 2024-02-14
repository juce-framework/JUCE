/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#if JUCE_PLUGINHOST_AU && (JUCE_MAC || JUCE_IOS)

JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wdeprecated-declarations")

#if JUCE_MAC
#include <AudioUnit/AUCocoaUIView.h>
#include <CoreAudioKit/AUGenericView.h>
#include <AudioToolbox/AudioUnitUtilities.h>

#if JUCE_PLUGINHOST_ARA
 #include <ARA_API/ARAAudioUnit.h>
#endif

#endif

#include <CoreMIDI/MIDIServices.h>

#include <CoreAudioKit/AUViewController.h>

#include <juce_audio_basics/native/juce_CoreAudioTimeConversions_mac.h>
#include <juce_audio_basics/native/juce_CoreAudioLayouts_mac.h>
#include "juce_AU_Shared.h"

namespace juce
{

// Change this to disable logging of various activities
#ifndef AU_LOGGING
 #define AU_LOGGING 1
#endif

#if AU_LOGGING
 #define JUCE_AU_LOG(a) Logger::writeToLog(a);
#else
 #define JUCE_AU_LOG(a)
#endif

namespace AudioUnitFormatHelpers
{
   #if JUCE_DEBUG
    static ThreadLocalValue<int> insideCallback;
   #endif

    static String osTypeToString (OSType type) noexcept
    {
        const juce_wchar s[4] = { (juce_wchar) ((type >> 24) & 0xff),
                                  (juce_wchar) ((type >> 16) & 0xff),
                                  (juce_wchar) ((type >> 8) & 0xff),
                                  (juce_wchar) (type & 0xff) };
        return String (s, 4);
    }

    static OSType stringToOSType (String s)
    {
        if (s.trim().length() >= 4) // (to avoid trimming leading spaces)
            s = s.trim();

        s += "    ";

        return (((OSType) (unsigned char) s[0]) << 24)
             | (((OSType) (unsigned char) s[1]) << 16)
             | (((OSType) (unsigned char) s[2]) << 8)
             |  ((OSType) (unsigned char) s[3]);
    }

    static const char* auIdentifierPrefix = "AudioUnit:";

    static String createPluginIdentifier (const AudioComponentDescription& desc)
    {
        String s (auIdentifierPrefix);

        if (desc.componentType == kAudioUnitType_MusicDevice)
            s << "Synths/";
        else if (desc.componentType == kAudioUnitType_MusicEffect
                  || desc.componentType == kAudioUnitType_Effect)
            s << "Effects/";
        else if (desc.componentType == kAudioUnitType_Generator)
            s << "Generators/";
        else if (desc.componentType == kAudioUnitType_Panner)
            s << "Panners/";
        else if (desc.componentType == kAudioUnitType_Mixer)
            s << "Mixers/";
        else if (desc.componentType == kAudioUnitType_MIDIProcessor)
            s << "MidiEffects/";

        s << osTypeToString (desc.componentType) << ","
          << osTypeToString (desc.componentSubType) << ","
          << osTypeToString (desc.componentManufacturer);

        return s;
    }

    static void getNameAndManufacturer (AudioComponent comp, String& name, String& manufacturer)
    {
        CFObjectHolder<CFStringRef> cfName;

        if (AudioComponentCopyName (comp, &cfName.object) == noErr)
            name = String::fromCFString (cfName.object);

        if (name.containsChar (':'))
        {
            manufacturer = name.upToFirstOccurrenceOf (":", false, false).trim();
            name         = name.fromFirstOccurrenceOf (":", false, false).trim();
        }

        if (name.isEmpty())
            name = "<Unknown>";
    }

    static bool getComponentDescFromIdentifier (const String& fileOrIdentifier, AudioComponentDescription& desc,
                                                String& name, String& version, String& manufacturer)
    {
        if (fileOrIdentifier.startsWithIgnoreCase (auIdentifierPrefix))
        {
            String s (fileOrIdentifier.substring (jmax (fileOrIdentifier.lastIndexOfChar (':'),
                                                        fileOrIdentifier.lastIndexOfChar ('/')) + 1));

            StringArray tokens;
            tokens.addTokens (s, ",", StringRef());
            tokens.removeEmptyStrings();

            if (tokens.size() == 3)
            {
                zerostruct (desc);
                desc.componentType         = stringToOSType (tokens[0]);
                desc.componentSubType      = stringToOSType (tokens[1]);
                desc.componentManufacturer = stringToOSType (tokens[2]);

                if (AudioComponent comp = AudioComponentFindNext (nullptr, &desc))
                {
                    getNameAndManufacturer (comp, name, manufacturer);

                    if (manufacturer.isEmpty())
                        manufacturer = tokens[2];

                    if (version.isEmpty())
                    {
                        UInt32 versionNum;

                        if (AudioComponentGetVersion (comp, &versionNum) == noErr)
                        {
                            version << (int) (versionNum >> 16) << "."
                                    << (int) ((versionNum >> 8) & 0xff) << "."
                                    << (int) (versionNum & 0xff);
                        }
                    }

                    return true;
                }
            }
        }

        return false;
    }

    static bool getComponentDescFromFile ([[maybe_unused]] const String& fileOrIdentifier,
                                          [[maybe_unused]] AudioComponentDescription& desc,
                                          [[maybe_unused]] String& name,
                                          [[maybe_unused]] String& version,
                                          [[maybe_unused]] String& manufacturer)
    {
        zerostruct (desc);

       #if JUCE_IOS
        return false;
       #else
        const File file (fileOrIdentifier);
        if (! file.hasFileExtension (".component") && ! file.hasFileExtension (".appex"))
            return false;

        const char* const utf8 = fileOrIdentifier.toUTF8();

        if (auto url = CFUniquePtr<CFURLRef> (CFURLCreateFromFileSystemRepresentation (nullptr, (const UInt8*) utf8,
                                                                                       (CFIndex) strlen (utf8), file.isDirectory())))
        {
            if (auto bundleRef = CFUniquePtr<CFBundleRef> (CFBundleCreate (kCFAllocatorDefault, url.get())))
            {
                CFTypeRef bundleName = CFBundleGetValueForInfoDictionaryKey (bundleRef.get(), CFSTR ("CFBundleName"));

                if (bundleName != nullptr && CFGetTypeID (bundleName) == CFStringGetTypeID())
                    name = String::fromCFString ((CFStringRef) bundleName);

                if (name.isEmpty())
                    name = file.getFileNameWithoutExtension();

                CFTypeRef versionString = CFBundleGetValueForInfoDictionaryKey (bundleRef.get(), CFSTR ("CFBundleVersion"));

                if (versionString != nullptr && CFGetTypeID (versionString) == CFStringGetTypeID())
                    version = String::fromCFString ((CFStringRef) versionString);

                CFTypeRef manuString = CFBundleGetValueForInfoDictionaryKey (bundleRef.get(), CFSTR ("CFBundleGetInfoString"));

                if (manuString != nullptr && CFGetTypeID (manuString) == CFStringGetTypeID())
                    manufacturer = String::fromCFString ((CFStringRef) manuString);

                class ScopedBundleResourceMap final
                {
                public:
                    explicit ScopedBundleResourceMap (CFBundleRef refIn) : ref (refIn),
                                                                           resFileId (CFBundleOpenBundleResourceMap (ref)),
                                                                           valid (resFileId != -1)
                    {
                        if (valid)
                            UseResFile (resFileId);
                    }

                    ~ScopedBundleResourceMap()
                    {
                        if (valid)
                            CFBundleCloseBundleResourceMap (ref, resFileId);
                    }

                    bool isValid() const noexcept
                    {
                        return valid;
                    }

                private:
                    const CFBundleRef ref;
                    const ResFileRefNum resFileId;
                    const bool valid;
                };

                const ScopedBundleResourceMap resourceMap { bundleRef.get() };

                const OSType thngType = stringToOSType ("thng");
                auto numResources = Count1Resources (thngType);

                if (resourceMap.isValid() && numResources > 0)
                {
                    for (ResourceIndex i = 1; i <= numResources; ++i)
                    {
                        if (Handle h = Get1IndResource (thngType, i))
                        {
                            HLock (h);
                            uint32 types[3];
                            std::memcpy (types, *h, sizeof (types));

                            if (types[0] == kAudioUnitType_MusicDevice
                                 || types[0] == kAudioUnitType_MusicEffect
                                 || types[0] == kAudioUnitType_Effect
                                 || types[0] == kAudioUnitType_Generator
                                 || types[0] == kAudioUnitType_Panner
                                 || types[0] == kAudioUnitType_Mixer
                                 || types[0] == kAudioUnitType_MIDIProcessor)
                            {
                                desc.componentType = types[0];
                                desc.componentSubType = types[1];
                                desc.componentManufacturer = types[2];

                                if (AudioComponent comp = AudioComponentFindNext (nullptr, &desc))
                                    getNameAndManufacturer (comp, name, manufacturer);

                                break;
                            }

                            HUnlock (h);
                            ReleaseResource (h);
                        }
                    }
                }
                else
                {
                    NSBundle* bundle = [[NSBundle alloc] initWithPath: (NSString*) fileOrIdentifier.toCFString()];

                    NSArray* audioComponents = [bundle objectForInfoDictionaryKey: @"AudioComponents"];
                    NSDictionary* dict = [audioComponents objectAtIndex: 0];

                    desc.componentManufacturer = stringToOSType (nsStringToJuce ((NSString*) [dict valueForKey: @"manufacturer"]));
                    desc.componentType         = stringToOSType (nsStringToJuce ((NSString*) [dict valueForKey: @"type"]));
                    desc.componentSubType      = stringToOSType (nsStringToJuce ((NSString*) [dict valueForKey: @"subtype"]));

                    [bundle release];
                }
            }
        }

        return desc.componentType != 0 && desc.componentSubType != 0;
       #endif
    }

    static const char* getCategory (OSType type) noexcept
    {
        switch (type)
        {
            case kAudioUnitType_Effect:
            case kAudioUnitType_MusicEffect:    return "Effect";
            case kAudioUnitType_MusicDevice:    return "Synth";
            case kAudioUnitType_Generator:      return "Generator";
            case kAudioUnitType_Panner:         return "Panner";
            case kAudioUnitType_Mixer:          return "Mixer";
            case kAudioUnitType_MIDIProcessor:  return "MidiEffects";
            default: break;
        }

        return nullptr;
    }

    // Audio Unit plugins expect hosts to listen to their view bounds, and to resize
    // the plugin window/view appropriately.
  #if JUCE_MAC || JUCE_IOS
   #if JUCE_IOS
    #define JUCE_IOS_MAC_VIEW  UIView
    using ViewComponentBaseClass = UIViewComponent;
   #else
    #define JUCE_IOS_MAC_VIEW  NSView
    using ViewComponentBaseClass = NSViewComponent;
   #endif

    struct AutoResizingNSViewComponent final : public ViewComponentBaseClass,
                                               private AsyncUpdater
    {
        void childBoundsChanged (Component*) override  { triggerAsyncUpdate(); }
        void handleAsyncUpdate() override              { resizeToFitView(); }
    };
  #endif

    template <typename Value>
    static Optional<Value> tryGetProperty (AudioUnit inUnit,
                                           AudioUnitPropertyID inID,
                                           AudioUnitScope inScope,
                                           AudioUnitElement inElement)
    {
        Value data;
        auto size = (UInt32) sizeof (Value);

        if (AudioUnitGetProperty (inUnit, inID, inScope, inElement, &data, &size) == noErr)
            return data;

        return {};
    }

    static UInt32 getElementCount (AudioUnit comp, AudioUnitScope scope) noexcept
    {
        const auto count = tryGetProperty<UInt32> (comp, kAudioUnitProperty_ElementCount, scope, 0);
        jassert (count);
        return *count;
    }

    /*  The plugin may expect its channels in a particular order, reported to the host
        using kAudioUnitProperty_AudioChannelLayout.
        This remapper allows us to respect the channel order requested by the plugin,
        while still using the JUCE channel ordering for the AudioBuffer argument
        of AudioProcessor::processBlock.
    */
    class SingleDirectionChannelMapping
    {
    public:
        void setUpMapping (AudioUnit comp, bool isInput)
        {
            channels.clear();
            busOffset.clear();

            const auto scope = isInput ? kAudioUnitScope_Input : kAudioUnitScope_Output;
            const auto n = getElementCount (comp, scope);

            for (UInt32 busIndex = 0; busIndex < n; ++busIndex)
            {
                std::vector<size_t> busMap;

                if (const auto layout = tryGetProperty<AudioChannelLayout> (comp, kAudioUnitProperty_AudioChannelLayout, scope, busIndex))
                {
                    const auto juceChannelOrder = CoreAudioLayouts::fromCoreAudio (*layout);
                    const auto auChannelOrder   = CoreAudioLayouts::getCoreAudioLayoutChannels (*layout);

                    for (auto juceChannelIndex = 0; juceChannelIndex < juceChannelOrder.size(); ++juceChannelIndex)
                        busMap.push_back ((size_t) auChannelOrder.indexOf (juceChannelOrder.getTypeOfChannel (juceChannelIndex)));
                }

                busOffset.push_back (busMap.empty() ? unknownChannelCount : channels.size());
                channels.insert (channels.end(), busMap.begin(), busMap.end());
            }
        }

        size_t getAuIndexForJuceChannel (size_t bus, size_t channel) const noexcept
        {
            const auto baseOffset = busOffset[bus];
            return baseOffset != unknownChannelCount ? channels[baseOffset + channel]
                                                     : channel;
        }

    private:
        static constexpr size_t unknownChannelCount = std::numeric_limits<size_t>::max();

        /*  The index (in the channels vector) of the first channel in each bus.
            e.g the index of the first channel in the second bus can be found at busOffset[1].
            It's possible for a bus not to report its channel layout, and in this case a value
            of unknownChannelCount will be stored for that bus.
        */
        std::vector<size_t> busOffset;

        /*  The index in a collection of JUCE channels of the AU channel with a matching channel
            type. The mappings for all buses are stored in bus order. To find the start offset for a
            particular bus, use the busOffset vector.
            e.g. the index of the AU channel with the same type as the fifth channel of the third bus
            in JUCE layout is found at channels[busOffset[2] + 4].
            If the busOffset for the bus is unknownChannelCount, then assume there is no mapping
            between JUCE/AU channel layouts.
        */
        std::vector<size_t> channels;
    };

    static bool isPluginAUv3 (const AudioComponentDescription& desc)
    {
        if (@available (macOS 10.11, *))
            return (desc.componentFlags & kAudioComponentFlag_IsV3AudioUnit) != 0;

        return false;
    }
}

static bool hasARAExtension ([[maybe_unused]] AudioUnit audioUnit)
{
   #if JUCE_PLUGINHOST_ARA
    UInt32 propertySize = sizeof (ARA::ARAAudioUnitFactory);
    Boolean isWriteable = FALSE;

    OSStatus status = AudioUnitGetPropertyInfo (audioUnit,
                                                ARA::kAudioUnitProperty_ARAFactory,
                                                kAudioUnitScope_Global,
                                                0,
                                                &propertySize,
                                                &isWriteable);

    if ((status == noErr) && (propertySize == sizeof (ARA::ARAAudioUnitFactory)) && ! isWriteable)
        return true;
   #endif

    return false;
}

struct AudioUnitDeleter
{
    void operator() (AudioUnit au) const { AudioComponentInstanceDispose (au); }
};

using AudioUnitUniquePtr = std::unique_ptr<std::remove_pointer_t<AudioUnit>, AudioUnitDeleter>;
using AudioUnitSharedPtr = std::shared_ptr<std::remove_pointer_t<AudioUnit>>;
using AudioUnitWeakPtr = std::weak_ptr<std::remove_pointer_t<AudioUnit>>;

static std::shared_ptr<const ARA::ARAFactory> getARAFactory ([[maybe_unused]] AudioUnitSharedPtr audioUnit)
{
   #if JUCE_PLUGINHOST_ARA
    jassert (audioUnit != nullptr);

    UInt32 propertySize = sizeof (ARA::ARAAudioUnitFactory);
    ARA::ARAAudioUnitFactory audioUnitFactory { ARA::kARAAudioUnitMagic, nullptr };

    if (hasARAExtension (audioUnit.get()))
    {
        OSStatus status = AudioUnitGetProperty (audioUnit.get(),
                                                ARA::kAudioUnitProperty_ARAFactory,
                                                kAudioUnitScope_Global,
                                                0,
                                                &audioUnitFactory,
                                                &propertySize);

        if ((status == noErr)
            && (propertySize == sizeof (ARA::ARAAudioUnitFactory))
            && (audioUnitFactory.inOutMagicNumber == ARA::kARAAudioUnitMagic))
        {
            jassert (audioUnitFactory.outFactory != nullptr);
            return getOrCreateARAFactory (audioUnitFactory.outFactory,
                                          [owningAuPtr = std::move (audioUnit)]() {});
        }
    }
   #endif

    return {};
}

struct VersionedAudioComponent
{
    AudioComponent audioComponent = nullptr;
    bool isAUv3 = false;

    bool operator< (const VersionedAudioComponent& other) const { return audioComponent < other.audioComponent; }
};

using AudioUnitCreationCallback = std::function<void (AudioUnit, OSStatus)>;

static void createAudioUnit (VersionedAudioComponent versionedComponent, AudioUnitCreationCallback callback)
{
    if (versionedComponent.isAUv3)
    {
        if (@available (macOS 10.11, *))
        {
            AudioComponentInstantiate (versionedComponent.audioComponent,
                                       kAudioComponentInstantiation_LoadOutOfProcess,
                                       ^(AudioComponentInstance audioUnit, OSStatus err)
                                       {
                                           callback (audioUnit, err);
                                       });

            return;
        }
    }

    AudioComponentInstance audioUnit;
    auto err = AudioComponentInstanceNew (versionedComponent.audioComponent, &audioUnit);
    callback (err != noErr ? nullptr : audioUnit, err);
}

struct AudioComponentResult
{
    explicit AudioComponentResult (String error) : errorMessage (std::move (error)) {}

    explicit AudioComponentResult (VersionedAudioComponent auComponent) : component (std::move (auComponent)) {}

    bool isValid() const { return component.audioComponent != nullptr; }

    VersionedAudioComponent component;
    String errorMessage;
};

static AudioComponentResult getAudioComponent (AudioUnitPluginFormat& format, const PluginDescription& desc)
{
    using namespace AudioUnitFormatHelpers;

    AudioUnitPluginFormat audioUnitPluginFormat;

    if (! format.fileMightContainThisPluginType (desc.fileOrIdentifier))
        return AudioComponentResult { NEEDS_TRANS ("Plug-in description is not an AudioUnit plug-in") };

    String pluginName, version, manufacturer;
    AudioComponentDescription componentDesc;
    AudioComponent auComponent;
    String errMessage = NEEDS_TRANS ("Cannot find AudioUnit from description");

    if (! getComponentDescFromIdentifier (desc.fileOrIdentifier, componentDesc, pluginName, version, manufacturer)
        && ! getComponentDescFromFile (desc.fileOrIdentifier, componentDesc, pluginName, version, manufacturer))
    {
        return AudioComponentResult { errMessage };
    }

    if ((auComponent = AudioComponentFindNext (nullptr, &componentDesc)) == nullptr)
    {
        return AudioComponentResult { errMessage };
    }

    if (AudioComponentGetDescription (auComponent, &componentDesc) != noErr)
    {
        return AudioComponentResult { errMessage };
    }

    const bool isAUv3 = AudioUnitFormatHelpers::isPluginAUv3 (componentDesc);

    return AudioComponentResult { { auComponent, isAUv3 } };
}

static void getOrCreateARAAudioUnit (VersionedAudioComponent auComponent, std::function<void (AudioUnitSharedPtr)> callback)
{
    static std::map<VersionedAudioComponent, AudioUnitWeakPtr> audioUnitARACache;

    if (auto audioUnit = audioUnitARACache[auComponent].lock())
    {
        callback (std::move (audioUnit));
        return;
    }

    createAudioUnit (auComponent, [auComponent, cb = std::move (callback)] (AudioUnit audioUnit, OSStatus err)
    {
        cb ([auComponent, audioUnit, err]() -> AudioUnitSharedPtr
            {
                if (err != noErr)
                    return nullptr;

                AudioUnitSharedPtr auPtr { AudioUnitUniquePtr { audioUnit } };
                audioUnitARACache[auComponent] = auPtr;
                return auPtr;
            }());
    });
}

//==============================================================================
class AudioUnitPluginWindowCocoa;

//==============================================================================
class AudioUnitPluginInstance final    : public AudioPluginInstance
{
public:
    struct AUInstanceParameter final  : public Parameter
    {
        AUInstanceParameter (AudioUnitPluginInstance& parent,
                             UInt32 parameterID,
                             const String& parameterName,
                             AudioUnitParameterValue minParameterValue,
                             AudioUnitParameterValue maxParameterValue,
                             AudioUnitParameterValue defaultParameterValue,
                             bool parameterIsAutomatable,
                             bool parameterIsDiscrete,
                             int numParameterSteps,
                             bool isBoolean,
                             const String& label,
                             bool parameterValuesHaveStrings)
            : pluginInstance (parent),
              paramID (parameterID),
              name (parameterName),
              minValue (minParameterValue),
              maxValue (maxParameterValue),
              range (maxValue - minValue),
              automatable (parameterIsAutomatable),
              discrete (parameterIsDiscrete),
              numSteps (numParameterSteps),
              valuesHaveStrings (parameterValuesHaveStrings),
              isSwitch (isBoolean),
              valueLabel (label),
              defaultValue (normaliseParamValue (defaultParameterValue))
        {
            auValueStrings = Parameter::getAllValueStrings();
        }

        float getValue() const override
        {
            const ScopedLock sl (pluginInstance.lock);

            AudioUnitParameterValue value = 0;

            if (auto* au = pluginInstance.audioUnit)
            {
                AudioUnitGetParameter (au, paramID, kAudioUnitScope_Global, 0, &value);
                value = normaliseParamValue (value);
            }

            return value;
        }

        void setValue (float newValue) override
        {
            const ScopedLock sl (pluginInstance.lock);

            if (auto* au = pluginInstance.audioUnit)
            {
                AudioUnitSetParameter (au, paramID, kAudioUnitScope_Global,
                                       0, scaleParamValue (newValue), 0);

                sendParameterChangeEvent();
            }
        }

        float getDefaultValue() const override
        {
            return defaultValue;
        }

        String getName (int /*maximumStringLength*/) const override { return name; }
        String getLabel() const override { return valueLabel; }

        String getText (float value, int maximumLength) const override
        {
            if (! auValueStrings.isEmpty())
            {
                auto index = roundToInt (jlimit (0.0f, 1.0f, value) * (float) (auValueStrings.size() - 1));
                return auValueStrings[index];
            }

            auto scaledValue = scaleParamValue (value);

            if (valuesHaveStrings)
            {
                if (auto* au = pluginInstance.audioUnit)
                {
                    AudioUnitParameterStringFromValue stringValue;
                    stringValue.inParamID = paramID;
                    stringValue.inValue = &scaledValue;
                    stringValue.outString = nullptr;

                    UInt32 propertySize = sizeof (stringValue);

                    auto err = AudioUnitGetProperty (au,
                                                     kAudioUnitProperty_ParameterStringFromValue,
                                                     kAudioUnitScope_Global,
                                                     0,
                                                     &stringValue,
                                                     &propertySize);

                    if (! err && stringValue.outString != nullptr)
                        return String::fromCFString (stringValue.outString).substring (0, maximumLength);
                }
            }

            return Parameter::getText (scaledValue, maximumLength);
        }

        float getValueForText (const String& text) const override
        {
            if (! auValueStrings.isEmpty())
            {
                auto index = auValueStrings.indexOf (text);

                if (index != -1)
                    return ((float) index) / (float) (auValueStrings.size() - 1);
            }

            if (valuesHaveStrings)
            {
                if (auto* au = pluginInstance.audioUnit)
                {
                    AudioUnitParameterValueFromString pvfs;
                    pvfs.inParamID = paramID;
                    CFUniquePtr<CFStringRef> cfString (text.toCFString());
                    pvfs.inString = cfString.get();
                    UInt32 propertySize = sizeof (pvfs);

                    auto err = AudioUnitGetProperty (au,
                                                     kAudioUnitProperty_ParameterValueFromString,
                                                     kAudioUnitScope_Global,
                                                     0,
                                                     &pvfs,
                                                     &propertySize);

                    if (! err)
                        return normaliseParamValue (pvfs.outValue);
                }
            }

            return Parameter::getValueForText (text);
        }

        bool isAutomatable() const override         { return automatable; }
        bool isDiscrete() const override            { return discrete; }
        bool isBoolean() const override             { return isSwitch; }
        int getNumSteps() const override            { return numSteps; }

        StringArray getAllValueStrings() const override
        {
            return auValueStrings;
        }

        String getParameterID() const override
        {
            return String (paramID);
        }

        void sendParameterChangeEvent()
        {
           #if JUCE_MAC
            jassert (pluginInstance.audioUnit != nullptr);

            AudioUnitEvent ev;
            ev.mEventType                        = kAudioUnitEvent_ParameterValueChange;
            ev.mArgument.mParameter.mAudioUnit   = pluginInstance.audioUnit;
            ev.mArgument.mParameter.mParameterID = paramID;
            ev.mArgument.mParameter.mScope       = kAudioUnitScope_Global;
            ev.mArgument.mParameter.mElement     = 0;

            AUEventListenerNotify (pluginInstance.eventListenerRef, nullptr, &ev);
           #endif
        }

        float normaliseParamValue (float scaledValue) const noexcept
        {
            return (scaledValue - minValue) / range;
        }

        float scaleParamValue (float normalisedValue) const noexcept
        {
            return minValue + (range * normalisedValue);
        }

        UInt32 getRawParamID() const { return paramID; }

        void setName  (String&& newName)  { name       = std::move (newName); }
        void setLabel (String&& newLabel) { valueLabel = std::move (newLabel); }

    private:
        AudioUnitPluginInstance& pluginInstance;
        const UInt32 paramID;
        String name;
        const AudioUnitParameterValue minValue, maxValue, range;
        const bool automatable, discrete;
        const int numSteps;
        const bool valuesHaveStrings, isSwitch;
        String valueLabel;
        const AudioUnitParameterValue defaultValue;
        StringArray auValueStrings;
    };

    AudioUnitPluginInstance (AudioComponentInstance au)
        : AudioPluginInstance (getBusesProperties (au)),
          auComponent (AudioComponentInstanceGetComponent (au)),
          audioUnit (au),
        #if JUCE_MAC
          eventListenerRef (nullptr),
        #endif
          midiConcatenator (2048)
    {
        using namespace AudioUnitFormatHelpers;

        AudioComponentGetDescription (auComponent, &componentDesc);

        isAUv3 = AudioUnitFormatHelpers::isPluginAUv3 (componentDesc);

        wantsMidiMessages = componentDesc.componentType == kAudioUnitType_MusicDevice
                         || componentDesc.componentType == kAudioUnitType_MusicEffect
                         || componentDesc.componentType == kAudioUnitType_MIDIProcessor;

        isMidiEffectPlugin = (componentDesc.componentType == kAudioUnitType_MIDIProcessor);
        AudioComponentDescription ignore;
        getComponentDescFromIdentifier (createPluginIdentifier (componentDesc), ignore, pluginName, version, manufacturer);
        updateSupportedLayouts();
    }

    ~AudioUnitPluginInstance() override
    {
        const ScopedLock sl (lock);

       #if JUCE_DEBUG
        // this indicates that some kind of recursive call is getting triggered that's
        // deleting this plugin while it's still under construction.
        jassert (AudioUnitFormatHelpers::insideCallback.get() == 0);
       #endif

        if (audioUnit != nullptr)
        {
            struct AUDeleter final : public CallbackMessage
            {
                AUDeleter (AudioUnitPluginInstance& inInstance, WaitableEvent& inEvent)
                    : auInstance (inInstance), completionSignal (inEvent)
                {}

                void messageCallback() override
                {
                    auInstance.cleanup();
                    completionSignal.signal();
                }

                AudioUnitPluginInstance& auInstance;
                WaitableEvent& completionSignal;
            };

            if (MessageManager::getInstance()->isThisTheMessageThread())
            {
                cleanup();
            }
            else
            {
                WaitableEvent completionEvent;
                (new AUDeleter (*this, completionEvent))->post();
                completionEvent.wait();
            }
        }
    }

    // called from the destructor above
    void cleanup()
    {
       #if JUCE_MAC
        disposeEventListener();
       #endif

        if (prepared)
            releaseResources();

        AudioComponentInstanceDispose (audioUnit);
        audioUnit = nullptr;
    }

    bool initialise (double rate, int blockSize)
    {
        producesMidiMessages = canProduceMidiOutput();
        setRateAndBufferSizeDetails (rate, blockSize);
        setLatencySamples (0);
        refreshParameterList();
        setPluginCallbacks();

       #if JUCE_MAC
        createEventListener();
       #endif

        return true;
    }

    //==============================================================================
    bool canAddBus (bool isInput)    const override                   { return isBusCountWritable (isInput); }
    bool canRemoveBus (bool isInput) const override                   { return isBusCountWritable (isInput); }

    bool canApplyBusCountChange (bool isInput, bool isAdding, BusProperties& outProperties) override
    {
        auto currentCount = (UInt32) getBusCount (isInput);
        auto newCount = (UInt32) ((int) currentCount + (isAdding ? 1 : -1));
        AudioUnitScope scope = isInput ? kAudioUnitScope_Input : kAudioUnitScope_Output;

        if (AudioUnitSetProperty (audioUnit, kAudioUnitProperty_ElementCount, scope, 0, &newCount, sizeof (newCount)) == noErr)
        {
            getBusProperties (isInput, currentCount, outProperties.busName, outProperties.defaultLayout);
            outProperties.isActivatedByDefault = true;
            updateSupportedLayouts();

            return true;
        }

        return false;
    }

    //==============================================================================
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override
    {
        if (layouts == getBusesLayout())
            return true;

        for (int dir = 0; dir < 2; ++dir)
        {
            const bool isInput = (dir == 0);
            auto& requestedLayouts         = (isInput ? layouts.inputBuses  : layouts.outputBuses);
            auto& oppositeRequestedLayouts = (isInput ? layouts.outputBuses : layouts.inputBuses);
            auto& supported                = (isInput ? supportedInLayouts : supportedOutLayouts);
            const int n = getBusCount (isInput);

            for (int busIdx = 0; busIdx < n; ++busIdx)
            {
                auto& requested = requestedLayouts.getReference (busIdx);
                const int oppositeBusIdx = jmin (getBusCount (! isInput) - 1, busIdx);
                const bool hasOppositeBus = (oppositeBusIdx >= 0);
                auto oppositeRequested = (hasOppositeBus ? oppositeRequestedLayouts.getReference (oppositeBusIdx) : AudioChannelSet());
                auto& possible = supported.getReference (busIdx);

                if (requested.isDisabled())
                    return false;

                if (possible.size() > 0 && ! possible.contains (requested))
                    return false;

                int i;
                for (i = 0; i < numChannelInfos; ++i)
                {
                    auto& info = channelInfos[i];
                    auto& thisChannels = (isInput ? info.inChannels  : info.outChannels);
                    auto& opChannels   = (isInput ? info.outChannels : info.inChannels);

                    // this bus
                    if      (thisChannels == 0) continue;
                    else if (thisChannels >  0 && requested.size() != thisChannels)       continue;
                    else if (thisChannels < -2 && requested.size() > (thisChannels * -1)) continue;

                    // opposite bus
                    if      (opChannels == 0 && hasOppositeBus) continue;
                    else if (opChannels >  0 && oppositeRequested.size() != opChannels) continue;
                    else if (opChannels < -2 && oppositeRequested.size() > (opChannels * -1)) continue;

                    // both buses
                    if (thisChannels == -2 && opChannels == -2) continue;
                    if (thisChannels == -1 && opChannels == -1)
                    {
                        int numOppositeBuses = getBusCount (! isInput);
                        int j;

                        for (j = 0; j < numOppositeBuses; ++j)
                            if (requested.size() != oppositeRequestedLayouts.getReference (j).size())
                                break;

                        if (j < numOppositeBuses)
                            continue;
                    }

                    break;
                }

                if (i >= numChannelInfos)
                    return false;
            }
        }

        return true;
    }

    bool syncBusLayouts (const BusesLayout& layouts, bool isInitialized, bool& layoutHasChanged) const
    {
        layoutHasChanged = false;

        for (int dir = 0; dir < 2; ++dir)
        {
            const bool isInput = (dir == 0);
            const AudioUnitScope scope = isInput ? kAudioUnitScope_Input : kAudioUnitScope_Output;
            const int n = getBusCount (isInput);

            if (getElementCount (scope) != n && isBusCountWritable (isInput))
            {
                auto newCount = static_cast<UInt32> (n);
                layoutHasChanged = true;

                [[maybe_unused]] auto err = AudioUnitSetProperty (audioUnit, kAudioUnitProperty_ElementCount, scope, 0, &newCount, sizeof (newCount));
                jassert (err == noErr);
            }

            for (int i = 0; i < n; ++i)
            {
                Float64 sampleRate;
                UInt32 sampleRateSize = sizeof (sampleRate);

                AudioUnitGetProperty (audioUnit, kAudioUnitProperty_SampleRate, scope, static_cast<UInt32> (i), &sampleRate, &sampleRateSize);

                const AudioChannelSet& set = layouts.getChannelSet (isInput, i);
                const int requestedNumChannels = set.size();

                {
                    AudioStreamBasicDescription stream;
                    UInt32 dataSize = sizeof (stream);
                    auto err = AudioUnitGetProperty (audioUnit, kAudioUnitProperty_StreamFormat, scope, static_cast<UInt32> (i), &stream, &dataSize);

                    if (err != noErr || dataSize < sizeof (stream))
                        return false;

                    const int actualNumChannels = static_cast<int> (stream.mChannelsPerFrame);

                    if (actualNumChannels != requestedNumChannels)
                    {
                        layoutHasChanged = true;
                        zerostruct (stream); // (can't use "= { 0 }" on this object because it's typedef'ed as a C struct)
                        stream.mSampleRate       = sampleRate;
                        stream.mFormatID         = kAudioFormatLinearPCM;
                        stream.mFormatFlags      = (int) kAudioFormatFlagsNativeFloatPacked | (int) kAudioFormatFlagIsNonInterleaved | (int) kAudioFormatFlagsNativeEndian;
                        stream.mFramesPerPacket  = 1;
                        stream.mBytesPerPacket   = 4;
                        stream.mBytesPerFrame    = 4;
                        stream.mBitsPerChannel   = 32;
                        stream.mChannelsPerFrame = static_cast<UInt32> (requestedNumChannels);

                        err = AudioUnitSetProperty (audioUnit, kAudioUnitProperty_StreamFormat, scope, static_cast<UInt32> (i), &stream, sizeof (stream));
                        if (err != noErr) return false;
                    }
                }

                if (! set.isDiscreteLayout())
                {
                    const AudioChannelLayoutTag requestedTag = CoreAudioLayouts::toCoreAudio (set);

                    AudioChannelLayout layout;
                    const UInt32 minDataSize = sizeof (layout) - sizeof (AudioChannelDescription);
                    UInt32 dataSize = minDataSize;

                    AudioChannelLayoutTag actualTag = kAudioChannelLayoutTag_Unknown;
                    auto err = AudioUnitGetProperty (audioUnit, kAudioUnitProperty_AudioChannelLayout, scope, static_cast<UInt32> (i), &layout, &dataSize);
                    bool supportsLayouts = (err == noErr && dataSize >= minDataSize);

                    if (supportsLayouts)
                    {
                        const UInt32 expectedSize =
                            minDataSize + (sizeof (AudioChannelDescription) * layout.mNumberChannelDescriptions);

                        HeapBlock<AudioChannelLayout> layoutBuffer;
                        layoutBuffer.malloc (1, expectedSize);
                        dataSize = expectedSize;

                        err = AudioUnitGetProperty (audioUnit, kAudioUnitProperty_AudioChannelLayout, scope,
                                                    static_cast<UInt32> (i), layoutBuffer.get(), &dataSize);

                        if (err != noErr || dataSize < expectedSize)
                            return false;

                        // try to convert the layout into a tag
                        actualTag = CoreAudioLayouts::toCoreAudio (CoreAudioLayouts::fromCoreAudio (layout));

                        if (actualTag != requestedTag)
                        {
                            zerostruct (layout);
                            layout.mChannelLayoutTag = requestedTag;

                            err = AudioUnitSetProperty (audioUnit, kAudioUnitProperty_AudioChannelLayout, scope, static_cast<UInt32> (i), &layout, minDataSize);

                            // only bail out if the plug-in claims to support layouts
                            // See AudioUnit headers on kAudioUnitProperty_AudioChannelLayout
                            if (err != noErr && supportsLayouts && isInitialized)
                                return false;
                        }
                    }
                }
            }
        }

        return true;
    }

    bool canApplyBusesLayout (const BusesLayout& layouts) const override
    {
        // You cannot call setBusesLayout when the AudioProcessor is processing.
        // Call releaseResources first!
        jassert (! prepared);

        bool layoutHasChanged = false;

        if (! syncBusLayouts (layouts, false, layoutHasChanged))
            return false;

        // did anything actually change
        if (! layoutHasChanged)
            return true;

        // Some plug-ins require the LayoutTag to be set after initialization
        const auto success = (AudioUnitInitialize (audioUnit) == noErr)
                             && syncBusLayouts (layouts, true, layoutHasChanged);

        AudioUnitUninitialize (audioUnit);

        if (! success)
            // make sure that the layout is back to its original state
            syncBusLayouts (getBusesLayout(), false, layoutHasChanged);

        return success;
    }

    //==============================================================================
    // AudioPluginInstance methods:

    void fillInPluginDescription (PluginDescription& desc) const override
    {
        desc.name = pluginName;
        desc.descriptiveName = pluginName;
        desc.fileOrIdentifier = AudioUnitFormatHelpers::createPluginIdentifier (componentDesc);
        desc.uniqueId = desc.deprecatedUid = ((int) componentDesc.componentType)
                                           ^ ((int) componentDesc.componentSubType)
                                           ^ ((int) componentDesc.componentManufacturer);
        desc.lastFileModTime = Time();
        desc.lastInfoUpdateTime = Time::getCurrentTime();
        desc.pluginFormatName = "AudioUnit";
        desc.category = AudioUnitFormatHelpers::getCategory (componentDesc.componentType);
        desc.manufacturerName = manufacturer;
        desc.version = version;
        desc.numInputChannels = getTotalNumInputChannels();
        desc.numOutputChannels = getTotalNumOutputChannels();
        desc.isInstrument = (componentDesc.componentType == kAudioUnitType_MusicDevice);

       #if JUCE_PLUGINHOST_ARA
        desc.hasARAExtension = [&]
        {
            UInt32 propertySize = sizeof (ARA::ARAAudioUnitFactory);
            Boolean isWriteable = FALSE;

            OSStatus status = AudioUnitGetPropertyInfo (audioUnit,
                                                        ARA::kAudioUnitProperty_ARAFactory,
                                                        kAudioUnitScope_Global,
                                                        0,
                                                        &propertySize,
                                                        &isWriteable);

            return (status == noErr) && (propertySize == sizeof (ARA::ARAAudioUnitFactory)) && ! isWriteable;
        }();
       #endif
    }

    void getExtensions (ExtensionsVisitor& visitor) const override
    {
        struct Extensions final : public ExtensionsVisitor::AudioUnitClient
        {
            explicit Extensions (const AudioUnitPluginInstance* instanceIn) : instance (instanceIn) {}

            AudioUnit getAudioUnitHandle() const noexcept override   { return instance->audioUnit; }

            const AudioUnitPluginInstance* instance = nullptr;
        };

        visitor.visitAudioUnitClient (Extensions { this });

       #ifdef JUCE_PLUGINHOST_ARA
        struct ARAExtensions final : public ExtensionsVisitor::ARAClient
        {
            explicit ARAExtensions (const AudioUnitPluginInstance* instanceIn) : instance (instanceIn) {}

            void createARAFactoryAsync (std::function<void (ARAFactoryWrapper)> cb) const override
            {
                getOrCreateARAAudioUnit ({ instance->auComponent, instance->isAUv3 },
                                         [origCb = std::move (cb)] (auto dylibKeepAliveAudioUnit)
                                         {
                                             origCb ([&]() -> ARAFactoryWrapper
                                                     {
                                                         if (dylibKeepAliveAudioUnit != nullptr)
                                                             return ARAFactoryWrapper { ::juce::getARAFactory (std::move (dylibKeepAliveAudioUnit)) };

                                                         return ARAFactoryWrapper { nullptr };
                                                     }());
                                         });
            }

            const AudioUnitPluginInstance* instance = nullptr;
        };

        if (hasARAExtension (audioUnit))
            visitor.visitARAClient (ARAExtensions (this));
       #endif
    }

    void* getPlatformSpecificData() override             { return audioUnit; }
    const String getName() const override                { return pluginName; }

    double getTailLengthSeconds() const override
    {
        Float64 tail = 0;
        UInt32 tailSize = sizeof (tail);

        if (audioUnit != nullptr)
            AudioUnitGetProperty (audioUnit, kAudioUnitProperty_TailTime, kAudioUnitScope_Global,
                                  0, &tail, &tailSize);

        return tail;
    }

    bool acceptsMidi() const override                    { return wantsMidiMessages; }
    bool producesMidi() const override                   { return producesMidiMessages; }

    //==============================================================================
    // AudioProcessor methods:

    void prepareToPlay (double newSampleRate, int estimatedSamplesPerBlock) override
    {
        if (audioUnit != nullptr)
        {
            releaseResources();
            setPluginCallbacks();

            for (int dir = 0; dir < 2; ++dir)
            {
                const bool isInput = (dir == 0);
                const AudioUnitScope scope = isInput ? kAudioUnitScope_Input : kAudioUnitScope_Output;
                const int n = getBusCount (isInput);

                for (int i = 0; i < n; ++i)
                {
                    Float64 sampleRate;
                    UInt32 sampleRateSize = sizeof (sampleRate);
                    const Float64 sr = newSampleRate;

                    AudioUnitGetProperty (audioUnit, kAudioUnitProperty_SampleRate, scope, static_cast<UInt32> (i), &sampleRate, &sampleRateSize);

                    if (! approximatelyEqual (sampleRate, sr))
                    {
                        if (isAUv3) // setting kAudioUnitProperty_SampleRate fails on AUv3s
                        {
                            AudioStreamBasicDescription stream;
                            UInt32 dataSize = sizeof (stream);
                            auto err = AudioUnitGetProperty (audioUnit, kAudioUnitProperty_StreamFormat, scope, static_cast<UInt32> (i), &stream, &dataSize);

                            if (err == noErr && dataSize == sizeof (stream))
                            {
                                stream.mSampleRate = sr;
                                AudioUnitSetProperty (audioUnit, kAudioUnitProperty_StreamFormat, scope, static_cast<UInt32> (i), &stream, sizeof (stream));
                            }
                        }
                        else
                        {
                            AudioUnitSetProperty (audioUnit, kAudioUnitProperty_SampleRate, scope, static_cast<UInt32> (i), &sr, sizeof (sr));
                        }
                    }

                    if (isInput)
                    {
                        AURenderCallbackStruct info;
                        zerostruct (info); // (can't use "= { 0 }" on this object because it's typedef'ed as a C struct)

                        info.inputProcRefCon = this;
                        info.inputProc = renderGetInputCallback;

                        AudioUnitSetProperty (audioUnit, kAudioUnitProperty_SetRenderCallback, kAudioUnitScope_Input,
                                              static_cast<UInt32> (i), &info, sizeof (info));
                    }
                    else
                    {
                        outputBufferList.add (new AUBuffer (static_cast<size_t> (getChannelCountOfBus (false, i))));
                    }
                }
            }

            UInt32 frameSize = (UInt32) estimatedSamplesPerBlock;
            AudioUnitSetProperty (audioUnit, kAudioUnitProperty_MaximumFramesPerSlice, kAudioUnitScope_Global, 0,
                                  &frameSize, sizeof (frameSize));

            setRateAndBufferSizeDetails ((double) newSampleRate, estimatedSamplesPerBlock);

            zerostruct (timeStamp);
            timeStamp.mSampleTime = 0;
            timeStamp.mHostTime = mach_absolute_time();
            timeStamp.mFlags = kAudioTimeStampSampleTimeValid | kAudioTimeStampHostTimeValid;

            wasPlaying = false;

            resetBuses();

            bool ignore;

            if (! syncBusLayouts (getBusesLayout(), false, ignore))
                return;

            prepared = [&]
            {
                if (AudioUnitInitialize (audioUnit) != noErr)
                    return false;

                if (! haveParameterList)
                    refreshParameterList();

                if (! syncBusLayouts (getBusesLayout(), true, ignore))
                {
                    AudioUnitUninitialize (audioUnit);
                    return false;
                }

                updateLatency();
                return true;
            }();

            inMapping .setUpMapping (audioUnit, true);
            outMapping.setUpMapping (audioUnit, false);

            preparedChannels = jmax (getTotalNumInputChannels(), getTotalNumOutputChannels());
            preparedSamples  = estimatedSamplesPerBlock;
            inputBuffer.setSize (preparedChannels, preparedSamples);
        }
    }

    void releaseResources() override
    {
        if (prepared)
        {
            resetBuses();
            AudioUnitReset (audioUnit, kAudioUnitScope_Global, 0);
            AudioUnitUninitialize (audioUnit);

            outputBufferList.clear();
            prepared = false;
        }

        incomingMidi.clear();
    }

    void resetBuses()
    {
        for (int i = 0; i < getBusCount (true);  ++i)  AudioUnitReset (audioUnit, kAudioUnitScope_Input,  static_cast<UInt32> (i));
        for (int i = 0; i < getBusCount (false); ++i)  AudioUnitReset (audioUnit, kAudioUnitScope_Output, static_cast<UInt32> (i));
    }

    void processAudio (AudioBuffer<float>& buffer, MidiBuffer& midiMessages, bool processBlockBypassedCalled)
    {
        auto* playhead = getPlayHead();
        const auto position = playhead != nullptr ? playhead->getPosition() : nullopt;

        if (const auto hostTimeNs = position.hasValue() ? position->getHostTimeNs() : nullopt)
        {
            timeStamp.mHostTime = *hostTimeNs;
            timeStamp.mFlags |= kAudioTimeStampHostTimeValid;
        }
        else
        {
            timeStamp.mHostTime = 0;
            timeStamp.mFlags &= ~kAudioTimeStampHostTimeValid;
        }

        // If these are hit, we might allocate in the process block!
        jassert (buffer.getNumChannels() <= preparedChannels);
        jassert (buffer.getNumSamples()  <= preparedSamples);
        // Copy the input buffer to guard against the case where a bus has more output channels
        // than input channels, so rendering the output for that bus might stamp over the input
        // to the following bus.
        inputBuffer.makeCopyOf (buffer, true);

        const auto numSamples = buffer.getNumSamples();

        if (auSupportsBypass)
        {
            updateBypass (processBlockBypassedCalled);
        }
        else if (processBlockBypassedCalled)
        {
            AudioProcessor::processBlockBypassed (buffer, midiMessages);
            return;
        }

        if (prepared)
        {
            const auto numOutputBuses = getBusCount (false);

            for (int i = 0; i < numOutputBuses; ++i)
            {
                if (AUBuffer* buf = outputBufferList[i])
                {
                    AudioBufferList& abl = *buf;
                    const auto* bus = getBus (false, i);
                    const auto channelCount = bus != nullptr ? bus->getNumberOfChannels() : 0;

                    for (auto juceChannel = 0; juceChannel < channelCount; ++juceChannel)
                    {
                        const auto auChannel = outMapping.getAuIndexForJuceChannel ((size_t) i, (size_t) juceChannel);
                        abl.mBuffers[auChannel].mNumberChannels = 1;
                        abl.mBuffers[auChannel].mDataByteSize = (UInt32) ((size_t) numSamples * sizeof (float));
                        abl.mBuffers[auChannel].mData = buffer.getWritePointer (bus->getChannelIndexInProcessBlockBuffer (juceChannel));
                    }
                }
            }

            if (wantsMidiMessages)
            {
                for (const auto metadata : midiMessages)
                {
                    if (metadata.numBytes <= 3)
                        MusicDeviceMIDIEvent (audioUnit,
                                              metadata.data[0], metadata.data[1], metadata.data[2],
                                              (UInt32) metadata.samplePosition);
                    else
                        MusicDeviceSysEx (audioUnit, metadata.data, (UInt32) metadata.numBytes);
                }

                midiMessages.clear();
            }

            for (int i = 0; i < numOutputBuses; ++i)
            {
                AudioUnitRenderActionFlags flags = 0;

                if (AUBuffer* buf = outputBufferList[i])
                    AudioUnitRender (audioUnit, &flags, &timeStamp, static_cast<UInt32> (i), (UInt32) numSamples, buf->bufferList.get());
            }

            timeStamp.mSampleTime += numSamples;
        }
        else
        {
            // Plugin not working correctly, so just bypass..
            for (int i = getTotalNumOutputChannels(); --i >= 0;)
                buffer.clear (i, 0, buffer.getNumSamples());
        }

        if (producesMidiMessages)
        {
            const ScopedLock sl (midiInLock);
            midiMessages.swapWith (incomingMidi);
            incomingMidi.clear();
        }
    }

    void processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages) override
    {
        processAudio (buffer, midiMessages, false);
    }

    void processBlockBypassed (AudioBuffer<float>& buffer, MidiBuffer& midiMessages) override
    {
        processAudio (buffer, midiMessages, true);
    }

    //==============================================================================
    AudioProcessorParameter* getBypassParameter() const override    { return auSupportsBypass ? bypassParam.get() : nullptr; }

    bool hasEditor() const override
    {
       #if JUCE_MAC
        return true;
       #else
        UInt32 dataSize;
        Boolean isWritable;

        return (AudioUnitGetPropertyInfo (audioUnit, kAudioUnitProperty_RequestViewController,
                                          kAudioUnitScope_Global, 0, &dataSize, &isWritable) == noErr
                && dataSize == sizeof (uintptr_t) && isWritable != 0);
       #endif
    }

    AudioProcessorEditor* createEditor() override;

    static AudioProcessor::BusesProperties getBusesProperties (AudioComponentInstance comp)
    {
        AudioProcessor::BusesProperties busProperties;

        for (int dir = 0; dir < 2; ++dir)
        {
            const auto isInput = (dir == 0);
            const auto n = AudioUnitFormatHelpers::getElementCount (comp, isInput ? kAudioUnitScope_Input : kAudioUnitScope_Output);

            for (UInt32 i = 0; i < n; ++i)
            {
                String busName;
                AudioChannelSet currentLayout;

                getBusProperties (comp, isInput, i, busName, currentLayout);
                jassert (! currentLayout.isDisabled());

                busProperties.addBus (isInput, busName, currentLayout, true);
            }
        }

        return busProperties;
    }

    //==============================================================================
    const String getInputChannelName (int index) const override
    {
        if (isPositiveAndBelow (index, getTotalNumInputChannels()))
            return "Input " + String (index + 1);

        return {};
    }

    const String getOutputChannelName (int index) const override
    {
        if (isPositiveAndBelow (index, getTotalNumOutputChannels()))
            return "Output " + String (index + 1);

        return {};
    }

    bool isInputChannelStereoPair (int index) const override    { return isPositiveAndBelow (index, getTotalNumInputChannels()); }
    bool isOutputChannelStereoPair (int index) const override   { return isPositiveAndBelow (index, getTotalNumOutputChannels()); }

    //==============================================================================
    void sendAllParametersChangedEvents()
    {
       #if JUCE_MAC
        jassert (audioUnit != nullptr);

        AudioUnitParameter param;
        param.mAudioUnit = audioUnit;
        param.mParameterID = kAUParameterListener_AnyParameter;

        AUParameterListenerNotify (nullptr, nullptr, &param);
       #endif
    }

    //==============================================================================
    class ScopedFactoryPresets
    {
    public:
        ScopedFactoryPresets (AudioUnit& au)
        {
            UInt32 sz = sizeof (CFArrayRef);
            AudioUnitGetProperty (au, kAudioUnitProperty_FactoryPresets,
                                  kAudioUnitScope_Global, 0, &presets.object, &sz);
        }

        CFArrayRef get() const noexcept
        {
            return presets.object;
        }

    private:
        CFObjectHolder<CFArrayRef> presets;
    };

    int getNumPrograms() override
    {
        ScopedFactoryPresets factoryPresets { audioUnit };

        if (factoryPresets.get() != nullptr)
            return (int) CFArrayGetCount (factoryPresets.get());

        return 0;
    }

    int getCurrentProgram() override
    {
        AUPreset current;
        current.presetNumber = 0;
        UInt32 sz = sizeof (AUPreset);

        AudioUnitGetProperty (audioUnit, kAudioUnitProperty_PresentPreset,
                              kAudioUnitScope_Global, 0, &current, &sz);

        return current.presetNumber;
    }

    void setCurrentProgram (int newIndex) override
    {
        ScopedFactoryPresets factoryPresets { audioUnit };

        if (factoryPresets.get() != nullptr
            && newIndex < (int) CFArrayGetCount (factoryPresets.get()))
        {
            AUPreset current;
            current.presetNumber = newIndex;

            if (auto* p = static_cast<const AUPreset*> (CFArrayGetValueAtIndex (factoryPresets.get(), newIndex)))
                current.presetName = p->presetName;

            AudioUnitSetProperty (audioUnit, kAudioUnitProperty_PresentPreset,
                                  kAudioUnitScope_Global, 0, &current, sizeof (AUPreset));

            sendAllParametersChangedEvents();
        }
    }

    const String getProgramName (int index) override
    {
        if (index == -1)
        {
            AUPreset current;
            current.presetNumber = -1;
            current.presetName = CFSTR ("");

            UInt32 prstsz = sizeof (AUPreset);

            AudioUnitGetProperty (audioUnit, kAudioUnitProperty_PresentPreset,
                                  kAudioUnitScope_Global, 0, &current, &prstsz);

            return String::fromCFString (current.presetName);
        }

        ScopedFactoryPresets factoryPresets { audioUnit };

        if (factoryPresets.get() != nullptr)
        {
            for (CFIndex i = 0; i < CFArrayGetCount (factoryPresets.get()); ++i)
                if (auto* p = static_cast<const AUPreset*> (CFArrayGetValueAtIndex (factoryPresets.get(), i)))
                    if (p->presetNumber == index)
                        return String::fromCFString (p->presetName);
        }

        return {};
    }

    void changeProgramName (int /*index*/, const String& /*newName*/) override
    {
        jassertfalse; // xxx not implemented!
    }

    //==============================================================================
    void updateTrackProperties (const TrackProperties& properties) override
    {
        if (properties.name.isNotEmpty())
        {
            CFObjectHolder<CFStringRef> contextName { properties.name.toCFString() };
            AudioUnitSetProperty (audioUnit, kAudioUnitProperty_ContextName, kAudioUnitScope_Global,
                                  0, &contextName.object, sizeof (contextName.object));
        }
    }

    //==============================================================================
    void getStateInformation (MemoryBlock& destData) override
    {
        getCurrentProgramStateInformation (destData);
    }

    void getCurrentProgramStateInformation (MemoryBlock& destData) override
    {
        CFObjectHolder<CFPropertyListRef> propertyList;
        UInt32 sz = sizeof (propertyList.object);

        if (AudioUnitGetProperty (audioUnit,
                                  kAudioUnitProperty_ClassInfo,
                                  kAudioUnitScope_Global,
                                  0, &propertyList.object, &sz) == noErr)
        {
            CFUniquePtr<CFWriteStreamRef> stream (CFWriteStreamCreateWithAllocatedBuffers (kCFAllocatorDefault, kCFAllocatorDefault));
            CFWriteStreamOpen (stream.get());

            CFIndex bytesWritten = CFPropertyListWriteToStream (propertyList.object, stream.get(), kCFPropertyListBinaryFormat_v1_0, nullptr);
            CFWriteStreamClose (stream.get());

            CFUniquePtr<CFDataRef> data ((CFDataRef) CFWriteStreamCopyProperty (stream.get(), kCFStreamPropertyDataWritten));

            destData.setSize ((size_t) bytesWritten);
            destData.copyFrom (CFDataGetBytePtr (data.get()), 0, destData.getSize());
        }
    }

    void setStateInformation (const void* data, int sizeInBytes) override
    {
        setCurrentProgramStateInformation (data, sizeInBytes);
    }

    void setCurrentProgramStateInformation (const void* data, int sizeInBytes) override
    {
        CFUniquePtr<CFReadStreamRef> stream (CFReadStreamCreateWithBytesNoCopy (kCFAllocatorDefault, (const UInt8*) data,
                                                                                sizeInBytes, kCFAllocatorNull));
        CFReadStreamOpen (stream.get());

        CFPropertyListFormat format = kCFPropertyListBinaryFormat_v1_0;
        CFObjectHolder<CFPropertyListRef> propertyList { CFPropertyListCreateFromStream (kCFAllocatorDefault, stream.get(), 0,
                                                                                         kCFPropertyListImmutable, &format, nullptr) };

        if (propertyList.object != nullptr)
        {
            AudioUnitSetProperty (audioUnit, kAudioUnitProperty_ClassInfo, kAudioUnitScope_Global,
                                  0, &propertyList.object, sizeof (propertyList.object));

            sendAllParametersChangedEvents();
        }
    }

    void refreshParameterList() override
    {
        paramIDToParameter.clear();
        AudioProcessorParameterGroup newParameterTree;

        if (audioUnit != nullptr)
        {
            UInt32 paramListSize = 0;
            auto err = AudioUnitGetPropertyInfo (audioUnit, kAudioUnitProperty_ParameterList, kAudioUnitScope_Global,
                                                 0, &paramListSize, nullptr);

            haveParameterList = (paramListSize > 0 && err == noErr);

            if (! haveParameterList)
                return;

            if (paramListSize > 0)
            {
                const size_t numParams = paramListSize / sizeof (int);

                std::vector<UInt32> ids (numParams, 0);

                AudioUnitGetProperty (audioUnit, kAudioUnitProperty_ParameterList, kAudioUnitScope_Global,
                                      0, ids.data(), &paramListSize);

                std::map<UInt32, AudioProcessorParameterGroup*> groupIDMap;

                for (size_t i = 0; i < numParams; ++i)
                {
                    const ScopedAudioUnitParameterInfo info { audioUnit, ids[i] };

                    if (! info.isValid())
                        continue;

                    const auto paramName = getParamName (info.get());
                    const auto label = getParamLabel (info.get());
                    const auto isDiscrete = (info.get().unit == kAudioUnitParameterUnit_Indexed
                                          || info.get().unit == kAudioUnitParameterUnit_Boolean);
                    const auto isBoolean = info.get().unit == kAudioUnitParameterUnit_Boolean;

                    auto parameter = std::make_unique<AUInstanceParameter> (*this,
                                                                            ids[i],
                                                                            paramName,
                                                                            info.get().minValue,
                                                                            info.get().maxValue,
                                                                            info.get().defaultValue,
                                                                            (info.get().flags & kAudioUnitParameterFlag_NonRealTime) == 0,
                                                                            isDiscrete,
                                                                            isDiscrete ? (int) (info.get().maxValue - info.get().minValue + 1.0f) : AudioProcessor::getDefaultNumParameterSteps(),
                                                                            isBoolean,
                                                                            label,
                                                                            (info.get().flags & kAudioUnitParameterFlag_ValuesHaveStrings) != 0);

                    paramIDToParameter.emplace (ids[i], parameter.get());

                    if (info.get().flags & kAudioUnitParameterFlag_HasClump)
                    {
                        auto groupInfo = groupIDMap.find (info.get().clumpID);

                        if (groupInfo == groupIDMap.end())
                        {
                            const auto clumpName = [this, &info]
                            {
                                AudioUnitParameterNameInfo clumpNameInfo;
                                UInt32 clumpSz = sizeof (clumpNameInfo);
                                zerostruct (clumpNameInfo);
                                clumpNameInfo.inID = info.get().clumpID;
                                clumpNameInfo.inDesiredLength = (SInt32) 256;

                                if (AudioUnitGetProperty (audioUnit,
                                                          kAudioUnitProperty_ParameterClumpName,
                                                          kAudioUnitScope_Global,
                                                          0,
                                                          &clumpNameInfo,
                                                          &clumpSz) == noErr)
                                    return String::fromCFString (clumpNameInfo.outName);

                                return String (info.get().clumpID);
                            }();

                            auto group = std::make_unique<AudioProcessorParameterGroup> (String (info.get().clumpID),
                                                                                         clumpName, String());
                            group->addChild (std::move (parameter));
                            groupIDMap[info.get().clumpID] = group.get();
                            newParameterTree.addChild (std::move (group));
                        }
                        else
                        {
                            groupInfo->second->addChild (std::move (parameter));
                        }
                    }
                    else
                    {
                        newParameterTree.addChild (std::move (parameter));
                    }
                }
            }
        }

        setHostedParameterTree (std::move (newParameterTree));

        UInt32 propertySize = 0;
        Boolean writable = false;

        auSupportsBypass = (AudioUnitGetPropertyInfo (audioUnit, kAudioUnitProperty_BypassEffect,
                                                     kAudioUnitScope_Global, 0, &propertySize, &writable) == noErr
                              && propertySize >= sizeof (UInt32) && writable);
        bypassParam.reset (new AUBypassParameter (*this));
    }

    void updateLatency()
    {
        Float64 latencySecs = 0.0;
        UInt32 latencySize = sizeof (latencySecs);
        AudioUnitGetProperty (audioUnit, kAudioUnitProperty_Latency, kAudioUnitScope_Global,
                              0, &latencySecs, &latencySize);

        setLatencySamples (roundToInt (latencySecs * getSampleRate()));
    }

    void handleIncomingMidiMessage (void*, const MidiMessage& message)
    {
        const ScopedLock sl (midiInLock);
        incomingMidi.addEvent (message, 0);
    }

    void handlePartialSysexMessage (void*, const uint8*, int, double) {}

    bool isMidiEffect() const override { return isMidiEffectPlugin; }

private:
    //==============================================================================
    friend class AudioUnitPluginWindowCocoa;
    friend class AudioUnitPluginFormat;

    CoreAudioTimeConversions timeConversions;

    AudioComponentDescription componentDesc;
    AudioComponent auComponent;
    String pluginName, manufacturer, version;
    String fileOrIdentifier;
    CriticalSection lock;

    bool wantsMidiMessages = false, producesMidiMessages = false,
         wasPlaying = false, prepared = false,
         isAUv3 = false, isMidiEffectPlugin = false;

    struct AUBuffer
    {
        AUBuffer (size_t numBuffers)
        {
            bufferList.calloc (1, (sizeof (AudioBufferList) - sizeof (::AudioBuffer)) + (sizeof (::AudioBuffer) * numBuffers));
            AudioBufferList& buffer = *bufferList.get();

            buffer.mNumberBuffers = static_cast<UInt32> (numBuffers);
        }

        operator AudioBufferList&()
        {
            return *bufferList.get();
        }

        HeapBlock<AudioBufferList> bufferList;
    };

    //==============================================================================
    struct AUBypassParameter final : public Parameter
    {
        AUBypassParameter (AudioUnitPluginInstance& effectToUse)
             : parent (effectToUse), currentValue (getCurrentHostValue())
        {}

        bool getCurrentHostValue()
        {
            if (parent.auSupportsBypass)
            {
                UInt32 dataSize = sizeof (UInt32);
                UInt32 value = 0;

                if (AudioUnitGetProperty (parent.audioUnit, kAudioUnitProperty_BypassEffect,
                                          kAudioUnitScope_Global, 0, &value, &dataSize) == noErr
                               && dataSize == sizeof (UInt32))
                    return value != 0;
            }

            return false;
        }

        float getValue() const override
        {
            return currentValue ? 1.0f : 0.0f;
        }

        void setValue (float newValue) override
        {
            auto newBypassValue = (newValue != 0.0f);

            const ScopedLock sl (parent.lock);

            if (newBypassValue != currentValue)
            {
                currentValue = newBypassValue;

                if (parent.auSupportsBypass)
                {
                    UInt32 value = (newValue != 0.0f ? 1 : 0);
                    AudioUnitSetProperty (parent.audioUnit, kAudioUnitProperty_BypassEffect,
                                          kAudioUnitScope_Global, 0, &value, sizeof (UInt32));

                   #if JUCE_MAC
                    jassert (parent.audioUnit != nullptr);

                    AudioUnitEvent ev;
                    ev.mEventType                       = kAudioUnitEvent_PropertyChange;
                    ev.mArgument.mProperty.mAudioUnit   = parent.audioUnit;
                    ev.mArgument.mProperty.mPropertyID  = kAudioUnitProperty_BypassEffect;
                    ev.mArgument.mProperty.mScope       = kAudioUnitScope_Global;
                    ev.mArgument.mProperty.mElement     = 0;

                    AUEventListenerNotify (parent.eventListenerRef, nullptr, &ev);
                   #endif
                }
            }
        }

        float getValueForText (const String& text) const override
        {
            String lowercaseText (text.toLowerCase());

            for (auto& testText : auOnStrings)
                if (lowercaseText == testText)
                    return 1.0f;

            for (auto& testText : auOffStrings)
                if (lowercaseText == testText)
                    return 0.0f;

            return text.getIntValue() != 0 ? 1.0f : 0.0f;
        }

        float getDefaultValue() const override                              { return 0.0f; }
        String getName (int /*maximumStringLength*/) const override         { return "Bypass"; }
        String getText (float value, int) const override                    { return (value != 0.0f ? TRANS ("On") : TRANS ("Off")); }
        bool isAutomatable() const override                                 { return true; }
        bool isDiscrete() const override                                    { return true; }
        bool isBoolean() const override                                     { return true; }
        int getNumSteps() const override                                    { return 2; }
        StringArray getAllValueStrings() const override                     { return values; }
        String getLabel() const override                                    { return {}; }

        String getParameterID() const override                              { return {}; }

        AudioUnitPluginInstance& parent;
        const StringArray auOnStrings  { TRANS ("on"),  TRANS ("yes"), TRANS ("true") };
        const StringArray auOffStrings { TRANS ("off"), TRANS ("no"),  TRANS ("false") };
        const StringArray values { TRANS ("Off"), TRANS ("On") };

        bool currentValue = false;
    };

    OwnedArray<AUBuffer> outputBufferList;
    AudioTimeStamp timeStamp;
    AudioBuffer<float> inputBuffer;
    Array<Array<AudioChannelSet>> supportedInLayouts, supportedOutLayouts;

    int numChannelInfos, preparedChannels = 0, preparedSamples = 0;
    HeapBlock<AUChannelInfo> channelInfos;

    AudioUnit audioUnit;
   #if JUCE_MAC
    AUEventListenerRef eventListenerRef;
   #endif

    std::map<UInt32, AUInstanceParameter*> paramIDToParameter;

    AudioUnitFormatHelpers::SingleDirectionChannelMapping inMapping, outMapping;
    MidiDataConcatenator midiConcatenator;
    CriticalSection midiInLock;
    MidiBuffer incomingMidi;
    std::unique_ptr<AUBypassParameter> bypassParam;
    bool lastProcessBlockCallWasBypass = false, auSupportsBypass = false;
    bool haveParameterList = false;

    void setPluginCallbacks()
    {
        if (audioUnit != nullptr)
        {
           #if JUCE_MAC
            if (producesMidiMessages)
            {
                AUMIDIOutputCallbackStruct info;
                zerostruct (info);

                info.userData = this;
                info.midiOutputCallback = renderMidiOutputCallback;

                producesMidiMessages = (AudioUnitSetProperty (audioUnit, kAudioUnitProperty_MIDIOutputCallback,
                                                              kAudioUnitScope_Global, 0, &info, sizeof (info)) == noErr);
            }
           #endif

            HostCallbackInfo info;
            zerostruct (info);

            info.hostUserData = this;
            info.beatAndTempoProc = getBeatAndTempoCallback;
            info.musicalTimeLocationProc = getMusicalTimeLocationCallback;
            info.transportStateProc = getTransportStateCallback;

            AudioUnitSetProperty (audioUnit, kAudioUnitProperty_HostCallbacks,
                                  kAudioUnitScope_Global, 0, &info, sizeof (info));
        }
    }

   #if JUCE_MAC
    void disposeEventListener()
    {
        if (eventListenerRef != nullptr)
        {
            AUListenerDispose (eventListenerRef);
            eventListenerRef = nullptr;
        }
    }

    void createEventListener()
    {
        if (audioUnit == nullptr)
            return;

        disposeEventListener();

        AUEventListenerCreate (eventListenerCallback, this, CFRunLoopGetMain(),
                               kCFRunLoopDefaultMode, 0, 0, &eventListenerRef);

        for (auto* param : getParameters())
        {
            jassert (dynamic_cast<AUInstanceParameter*> (param) != nullptr);

            AudioUnitEvent event;
            event.mArgument.mParameter.mAudioUnit = audioUnit;
            event.mArgument.mParameter.mParameterID = static_cast<AUInstanceParameter*> (param)->getRawParamID();
            event.mArgument.mParameter.mScope = kAudioUnitScope_Global;
            event.mArgument.mParameter.mElement = 0;

            event.mEventType = kAudioUnitEvent_ParameterValueChange;
            AUEventListenerAddEventType (eventListenerRef, nullptr, &event);

            event.mEventType = kAudioUnitEvent_BeginParameterChangeGesture;
            AUEventListenerAddEventType (eventListenerRef, nullptr, &event);

            event.mEventType = kAudioUnitEvent_EndParameterChangeGesture;
            AUEventListenerAddEventType (eventListenerRef, nullptr, &event);
        }

        addPropertyChangeListener (kAudioUnitProperty_PresentPreset);
        addPropertyChangeListener (kAudioUnitProperty_ParameterList);
        addPropertyChangeListener (kAudioUnitProperty_Latency);
        addPropertyChangeListener (kAudioUnitProperty_BypassEffect);
    }

    void addPropertyChangeListener (AudioUnitPropertyID type) const
    {
        AudioUnitEvent event;
        event.mEventType = kAudioUnitEvent_PropertyChange;
        event.mArgument.mProperty.mPropertyID = type;
        event.mArgument.mProperty.mAudioUnit = audioUnit;
        event.mArgument.mProperty.mScope = kAudioUnitScope_Global;
        event.mArgument.mProperty.mElement = 0;
        AUEventListenerAddEventType (eventListenerRef, nullptr, &event);
    }

    void eventCallback (const AudioUnitEvent& event, AudioUnitParameterValue newValue)
    {
        if (event.mEventType == kAudioUnitEvent_PropertyChange)
        {
            respondToPropertyChange (event.mArgument.mProperty);
            return;
        }

        const auto iter = paramIDToParameter.find (event.mArgument.mParameter.mParameterID);
        auto* param = iter != paramIDToParameter.end() ? iter->second : nullptr;
        jassert (param != nullptr); // Invalid parameter index

        if (param == nullptr)
            return;

        if (event.mEventType == kAudioUnitEvent_ParameterValueChange)
            param->sendValueChangedMessageToListeners (param->normaliseParamValue (newValue));
        else if (event.mEventType == kAudioUnitEvent_BeginParameterChangeGesture)
            param->beginChangeGesture();
        else if (event.mEventType == kAudioUnitEvent_EndParameterChangeGesture)
            param->endChangeGesture();
    }

    void respondToPropertyChange (const AudioUnitProperty& prop)
    {
        switch (prop.mPropertyID)
        {
            case kAudioUnitProperty_ParameterList:
                updateParameterInfo();
                updateHostDisplay (AudioProcessorListener::ChangeDetails().withParameterInfoChanged (true));
                break;

            case kAudioUnitProperty_PresentPreset:
                sendAllParametersChangedEvents();
                updateHostDisplay (AudioProcessorListener::ChangeDetails().withProgramChanged (true));
                break;

            case kAudioUnitProperty_Latency:
                updateLatency();
                break;

            case kAudioUnitProperty_BypassEffect:
                if (bypassParam != nullptr)
                    bypassParam->setValueNotifyingHost (bypassParam->getValue());

                break;
        }
    }

    static void eventListenerCallback (void* userRef, void*, const AudioUnitEvent* event,
                                       UInt64, AudioUnitParameterValue value)
    {
        JUCE_ASSERT_MESSAGE_THREAD
        jassert (event != nullptr);
        static_cast<AudioUnitPluginInstance*> (userRef)->eventCallback (*event, value);
    }

    void updateParameterInfo()
    {
        for (const auto& idAndParam : paramIDToParameter)
        {
            const auto& id    = idAndParam.first;
            const auto& param = idAndParam.second;

            const ScopedAudioUnitParameterInfo info { audioUnit, id };

            if (! info.isValid())
                continue;

            param->setName  (getParamName  (info.get()));
            param->setLabel (getParamLabel (info.get()));
        }
    }
   #endif

    /*  Some fields in the AudioUnitParameterInfo may need to be released after use,
        so we'll do that using RAII.
    */
    class ScopedAudioUnitParameterInfo
    {
    public:
        ScopedAudioUnitParameterInfo (AudioUnit au, UInt32 paramId)
        {
            auto sz = (UInt32) sizeof (info);
            valid = noErr == AudioUnitGetProperty (au,
                                                   kAudioUnitProperty_ParameterInfo,
                                                   kAudioUnitScope_Global,
                                                   paramId,
                                                   &info,
                                                   &sz);
        }

        ScopedAudioUnitParameterInfo (const ScopedAudioUnitParameterInfo&) = delete;
        ScopedAudioUnitParameterInfo (ScopedAudioUnitParameterInfo&&) = delete;
        ScopedAudioUnitParameterInfo& operator= (const ScopedAudioUnitParameterInfo&) = delete;
        ScopedAudioUnitParameterInfo& operator= (ScopedAudioUnitParameterInfo&&) = delete;

        ~ScopedAudioUnitParameterInfo() noexcept
        {
            if ((info.flags & kAudioUnitParameterFlag_CFNameRelease) == 0)
                return;

            if (info.cfNameString != nullptr)
                CFRelease (info.cfNameString);

            if (info.unit == kAudioUnitParameterUnit_CustomUnit && info.unitName != nullptr)
                CFRelease (info.unitName);
        }

        bool isValid() const { return valid; }

        const AudioUnitParameterInfo& get() const noexcept { return info; }

    private:
        AudioUnitParameterInfo info;
        bool valid = false;
    };

    static String getParamName (const AudioUnitParameterInfo& info)
    {
        if ((info.flags & kAudioUnitParameterFlag_HasCFNameString) == 0)
            return { info.name, sizeof (info.name) };

        return String::fromCFString (info.cfNameString);
    }

    static String getParamLabel (const AudioUnitParameterInfo& info)
    {
        if (info.unit == kAudioUnitParameterUnit_CustomUnit)    return String::fromCFString (info.unitName);
        if (info.unit == kAudioUnitParameterUnit_Percent)       return "%";
        if (info.unit == kAudioUnitParameterUnit_Seconds)       return "s";
        if (info.unit == kAudioUnitParameterUnit_Hertz)         return "Hz";
        if (info.unit == kAudioUnitParameterUnit_Decibels)      return "dB";
        if (info.unit == kAudioUnitParameterUnit_Milliseconds)  return "ms";

        return {};
    }

    //==============================================================================
    OSStatus renderGetInput (AudioUnitRenderActionFlags*,
                             const AudioTimeStamp*,
                             UInt32 inBusNumber,
                             UInt32 inNumberFrames,
                             AudioBufferList* ioData)
    {
        if (inputBuffer.getNumChannels() <= 0)
        {
            jassertfalse;
            return noErr;
        }

        // if this ever happens, might need to add extra handling
        if (inputBuffer.getNumSamples() != (int) inNumberFrames)
        {
            jassertfalse;
            return noErr;
        }

        const auto buffer = static_cast<int> (inBusNumber) < getBusCount (true)
                          ? getBusBuffer (inputBuffer, true, static_cast<int> (inBusNumber))
                          : AudioBuffer<float>();

        for (int juceChannel = 0; juceChannel < buffer.getNumChannels(); ++juceChannel)
        {
            const auto auChannel = (int) inMapping.getAuIndexForJuceChannel (inBusNumber, (size_t) juceChannel);

            if (auChannel < buffer.getNumChannels())
                memcpy (ioData->mBuffers[auChannel].mData, buffer.getReadPointer (juceChannel), sizeof (float) * inNumberFrames);
            else
                zeromem (ioData->mBuffers[auChannel].mData, sizeof (float) * inNumberFrames);
        }

        return noErr;
    }

    OSStatus renderMidiOutput (const MIDIPacketList* pktlist)
    {
        if (pktlist != nullptr && pktlist->numPackets)
        {
            auto time = Time::getMillisecondCounterHiRes() * 0.001;
            const MIDIPacket* packet = &pktlist->packet[0];

            for (UInt32 i = 0; i < pktlist->numPackets; ++i)
            {
                midiConcatenator.pushMidiData (packet->data, (int) packet->length, time, (void*) nullptr, *this);
                packet = MIDIPacketNext (packet);
            }
        }

        return noErr;
    }

    template <typename Type1, typename Type2>
    static void setIfNotNull (Type1* p, Type2 value) noexcept
    {
        if (p != nullptr) *p = value;
    }

    /*  If the AudioPlayHead is available, and has valid PositionInfo, this will return the result
        of calling the specified getter on that PositionInfo. Otherwise, this will return a
        default-constructed instance of the same type.

        For getters that return an Optional, this function will return a nullopt if the playhead or
        position info is invalid.

        For getters that return a bool, this function will return false if the playhead or position
        info is invalid.
    */
    template <typename Result>
    Result getFromPlayHead (Result (AudioPlayHead::PositionInfo::* member)() const) const
    {
        if (auto* ph = getPlayHead())
            if (const auto pos = ph->getPosition())
                return ((*pos).*member)();

        return {};
    }

    OSStatus getBeatAndTempo (Float64* outCurrentBeat, Float64* outCurrentTempo) const
    {
        setIfNotNull (outCurrentBeat,  getFromPlayHead (&AudioPlayHead::PositionInfo::getPpqPosition).orFallback (0));
        setIfNotNull (outCurrentTempo, getFromPlayHead (&AudioPlayHead::PositionInfo::getBpm).orFallback (120.0));
        return noErr;
    }

    OSStatus getMusicalTimeLocation (UInt32* outDeltaSampleOffsetToNextBeat, Float32* outTimeSig_Numerator,
                                     UInt32* outTimeSig_Denominator, Float64* outCurrentMeasureDownBeat) const
    {
        setIfNotNull (outDeltaSampleOffsetToNextBeat, (UInt32) 0); //xxx
        setIfNotNull (outCurrentMeasureDownBeat, getFromPlayHead (&AudioPlayHead::PositionInfo::getPpqPositionOfLastBarStart).orFallback (0.0));

        const auto signature = getFromPlayHead (&AudioPlayHead::PositionInfo::getTimeSignature).orFallback (AudioPlayHead::TimeSignature{});
        setIfNotNull (outTimeSig_Numerator,   (Float32) signature.numerator);
        setIfNotNull (outTimeSig_Denominator, (UInt32)  signature.denominator);

        return noErr;
    }

    OSStatus getTransportState (Boolean* outIsPlaying, Boolean* outTransportStateChanged,
                                Float64* outCurrentSampleInTimeLine, Boolean* outIsCycling,
                                Float64* outCycleStartBeat, Float64* outCycleEndBeat)
    {
        const auto nowPlaying = getFromPlayHead (&AudioPlayHead::PositionInfo::getIsPlaying);
        setIfNotNull (outIsPlaying, nowPlaying);
        setIfNotNull (outTransportStateChanged, std::exchange (wasPlaying, nowPlaying) != nowPlaying);
        setIfNotNull (outCurrentSampleInTimeLine, (double) getFromPlayHead (&AudioPlayHead::PositionInfo::getTimeInSamples).orFallback (0));
        setIfNotNull (outIsCycling, getFromPlayHead (&AudioPlayHead::PositionInfo::getIsLooping));

        const auto loopPoints = getFromPlayHead (&AudioPlayHead::PositionInfo::getLoopPoints).orFallback (AudioPlayHead::LoopPoints{});
        setIfNotNull (outCycleStartBeat, loopPoints.ppqStart);
        setIfNotNull (outCycleEndBeat, loopPoints.ppqEnd);

        return noErr;
    }

    //==============================================================================
    static OSStatus renderGetInputCallback (void* hostRef, AudioUnitRenderActionFlags* ioActionFlags,
                                            const AudioTimeStamp* inTimeStamp, UInt32 inBusNumber,
                                            UInt32 inNumberFrames, AudioBufferList* ioData)
    {
        return static_cast<AudioUnitPluginInstance*> (hostRef)
                 ->renderGetInput (ioActionFlags, inTimeStamp, inBusNumber, inNumberFrames, ioData);
    }

    static OSStatus renderMidiOutputCallback (void* hostRef, const AudioTimeStamp*, UInt32 /*midiOutNum*/,
                                              const MIDIPacketList* pktlist)
    {
        return static_cast<AudioUnitPluginInstance*> (hostRef)->renderMidiOutput (pktlist);
    }

    static OSStatus getBeatAndTempoCallback (void* hostRef, Float64* outCurrentBeat, Float64* outCurrentTempo)
    {
        return static_cast<AudioUnitPluginInstance*> (hostRef)->getBeatAndTempo (outCurrentBeat, outCurrentTempo);
    }

    static OSStatus getMusicalTimeLocationCallback (void* hostRef, UInt32* outDeltaSampleOffsetToNextBeat,
                                                    Float32* outTimeSig_Numerator, UInt32* outTimeSig_Denominator,
                                                    Float64* outCurrentMeasureDownBeat)
    {
        return static_cast<AudioUnitPluginInstance*> (hostRef)
                    ->getMusicalTimeLocation (outDeltaSampleOffsetToNextBeat, outTimeSig_Numerator,
                                              outTimeSig_Denominator, outCurrentMeasureDownBeat);
    }

    static OSStatus getTransportStateCallback (void* hostRef, Boolean* outIsPlaying, Boolean* outTransportStateChanged,
                                               Float64* outCurrentSampleInTimeLine, Boolean* outIsCycling,
                                               Float64* outCycleStartBeat, Float64* outCycleEndBeat)
    {
        return static_cast<AudioUnitPluginInstance*> (hostRef)
                    ->getTransportState (outIsPlaying, outTransportStateChanged, outCurrentSampleInTimeLine,
                                         outIsCycling, outCycleStartBeat, outCycleEndBeat);
    }

    //==============================================================================
    bool isBusCountWritable (bool isInput) const noexcept
    {
        UInt32 countSize;
        Boolean writable;
        AudioUnitScope scope = (isInput ? kAudioUnitScope_Input : kAudioUnitScope_Output);

        auto err = AudioUnitGetPropertyInfo (audioUnit, kAudioUnitProperty_ElementCount, scope, 0, &countSize, &writable);

        return (err == noErr && writable != 0 && countSize == sizeof (UInt32));
    }

    //==============================================================================
    int getElementCount (AudioUnitScope scope) const noexcept
    {
        return static_cast<int> (AudioUnitFormatHelpers::getElementCount (audioUnit, scope));
    }

    //==============================================================================
    void getBusProperties (bool isInput, UInt32 busIdx, String& busName, AudioChannelSet& currentLayout) const
    {
        getBusProperties (audioUnit, isInput, busIdx, busName, currentLayout);
    }

    static void getBusProperties (AudioUnit comp, bool isInput, UInt32 busIdx, String& busName, AudioChannelSet& currentLayout)
    {
        const AudioUnitScope scope = isInput ? kAudioUnitScope_Input : kAudioUnitScope_Output;
        busName = (isInput ? "Input #" : "Output #") + String (busIdx + 1);

        {
            CFObjectHolder<CFStringRef> busNameCF;
            UInt32 propertySize = sizeof (busNameCF.object);

            if (AudioUnitGetProperty (comp, kAudioUnitProperty_ElementName, scope, busIdx, &busNameCF.object, &propertySize) == noErr)
                if (busNameCF.object != nullptr)
                    busName = nsStringToJuce ((NSString*) busNameCF.object);

            {
                AudioChannelLayout auLayout;
                propertySize = sizeof (auLayout);

                if (AudioUnitGetProperty (comp, kAudioUnitProperty_AudioChannelLayout, scope, busIdx, &auLayout, &propertySize) == noErr)
                    currentLayout = CoreAudioLayouts::fromCoreAudio (auLayout);
            }

            if (currentLayout.isDisabled())
            {
                AudioStreamBasicDescription descr;
                propertySize = sizeof (descr);

                if (AudioUnitGetProperty (comp, kAudioUnitProperty_StreamFormat, scope, busIdx, &descr, &propertySize) == noErr)
                    currentLayout = AudioChannelSet::canonicalChannelSet (static_cast<int> (descr.mChannelsPerFrame));
            }
        }
    }

    //==============================================================================
    void numBusesChanged() override
    {
        updateSupportedLayouts();
    }

    void updateSupportedLayouts()
    {
        supportedInLayouts.clear();
        supportedOutLayouts.clear();
        numChannelInfos = 0;
        channelInfos.free();

        for (int dir = 0; dir < 2; ++dir)
        {
            const bool isInput = (dir == 0);
            const AudioUnitScope scope = isInput ? kAudioUnitScope_Input : kAudioUnitScope_Output;
            auto n = getElementCount (scope);

            for (int busIdx = 0; busIdx < n; ++busIdx)
            {
                Array<AudioChannelSet> supported;
                AudioChannelSet currentLayout;

                {
                    AudioChannelLayout auLayout;
                    UInt32 propertySize = sizeof (auLayout);

                    if (AudioUnitGetProperty (audioUnit, kAudioUnitProperty_AudioChannelLayout, scope, static_cast<UInt32> (busIdx), &auLayout, &propertySize) == noErr)
                        currentLayout = CoreAudioLayouts::fromCoreAudio (auLayout);
                }

                if (currentLayout.isDisabled())
                {
                    AudioStreamBasicDescription descr;
                    UInt32 propertySize = sizeof (descr);

                    if (AudioUnitGetProperty (audioUnit, kAudioUnitProperty_StreamFormat, scope, static_cast<UInt32> (busIdx), &descr, &propertySize) == noErr)
                        currentLayout = AudioChannelSet::canonicalChannelSet (static_cast<int> (descr.mChannelsPerFrame));
                }

                supported.clear();
                {
                    UInt32 propertySize = 0;
                    Boolean writable;

                    if (AudioUnitGetPropertyInfo (audioUnit, kAudioUnitProperty_SupportedChannelLayoutTags, scope, static_cast<UInt32> (busIdx), &propertySize, &writable) == noErr
                        && propertySize > 0)
                    {
                        const size_t numElements = propertySize / sizeof (AudioChannelLayoutTag);
                        HeapBlock<AudioChannelLayoutTag> layoutTags (numElements);
                        propertySize = static_cast<UInt32> (sizeof (AudioChannelLayoutTag) * numElements);

                        if (AudioUnitGetProperty (audioUnit, kAudioUnitProperty_SupportedChannelLayoutTags, scope,
                                                  static_cast<UInt32> (busIdx), layoutTags.get(), &propertySize) == noErr)
                        {
                            for (int j = 0; j < static_cast<int> (numElements); ++j)
                            {
                                const AudioChannelLayoutTag tag = layoutTags[j];

                                if (tag != kAudioChannelLayoutTag_UseChannelDescriptions)
                                {
                                    AudioChannelLayout caLayout;

                                    caLayout.mChannelLayoutTag = tag;
                                    supported.addIfNotAlreadyThere (CoreAudioLayouts::fromCoreAudio (caLayout));
                                }
                            }

                            if (supported.size() > 0)
                                supported.addIfNotAlreadyThere (currentLayout);
                        }
                    }
                }

                (isInput ? supportedInLayouts : supportedOutLayouts).add (supported);
            }
        }

        {
            UInt32 propertySize = 0;
            Boolean writable;

            if (AudioUnitGetPropertyInfo (audioUnit, kAudioUnitProperty_SupportedNumChannels, kAudioUnitScope_Global, 0, &propertySize, &writable) == noErr
                && propertySize > 0)
            {
                numChannelInfos = propertySize / sizeof (AUChannelInfo);
                channelInfos.malloc (static_cast<size_t> (numChannelInfos));
                propertySize = static_cast<UInt32> (sizeof (AUChannelInfo) * static_cast<size_t> (numChannelInfos));

                if (AudioUnitGetProperty (audioUnit, kAudioUnitProperty_SupportedNumChannels, kAudioUnitScope_Global, 0, channelInfos.get(), &propertySize) != noErr)
                    numChannelInfos = 0;
            }
            else
            {
                numChannelInfos = 1;
                channelInfos.malloc (static_cast<size_t> (numChannelInfos));
                channelInfos.get()->inChannels  = -1;
                channelInfos.get()->outChannels = -1;
            }
        }
    }

    bool canProduceMidiOutput()
    {
       #if JUCE_MAC
        UInt32 dataSize = 0;
        Boolean isWritable = false;

        if (AudioUnitGetPropertyInfo (audioUnit, kAudioUnitProperty_MIDIOutputCallbackInfo,
                                      kAudioUnitScope_Global, 0, &dataSize, &isWritable) == noErr
             && dataSize != 0)
        {
            CFObjectHolder<CFArrayRef> midiArray;

            if (AudioUnitGetProperty (audioUnit, kAudioUnitProperty_MIDIOutputCallbackInfo,
                                      kAudioUnitScope_Global, 0, &midiArray.object, &dataSize) == noErr)
                return (CFArrayGetCount (midiArray.object) > 0);
        }
       #endif

        return false;
    }

    bool supportsMPE() const override
    {
        UInt32 dataSize = 0;
        Boolean isWritable = false;

        if (AudioUnitGetPropertyInfo (audioUnit, kAudioUnitProperty_SupportsMPE,
                                      kAudioUnitScope_Global, 0, &dataSize, &isWritable) == noErr
            && dataSize == sizeof (UInt32))
        {
            UInt32 result = 0;

            if (AudioUnitGetProperty (audioUnit, kAudioUnitProperty_SupportsMPE,
                                      kAudioUnitScope_Global, 0, &result, &dataSize) == noErr)
            {
                return result > 0;
            }
        }

        return false;
    }

    //==============================================================================
    void updateBypass (bool processBlockBypassedCalled)
    {
        if (processBlockBypassedCalled && bypassParam != nullptr)
        {
            if (bypassParam->getValue() == 0.0f || ! lastProcessBlockCallWasBypass)
                bypassParam->setValue (1.0f);
        }
        else
        {
            if (lastProcessBlockCallWasBypass && bypassParam != nullptr)
                bypassParam->setValue (0.0f);
        }

        lastProcessBlockCallWasBypass = processBlockBypassedCalled;
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioUnitPluginInstance)
};

//==============================================================================
class AudioUnitPluginWindowCocoa final : public AudioProcessorEditor
{
public:
    AudioUnitPluginWindowCocoa (AudioUnitPluginInstance& p, bool createGenericViewIfNeeded)
        : AudioProcessorEditor (&p),
          plugin (p)
    {
        addAndMakeVisible (wrapper);

        setOpaque (true);
        setVisible (true);
        setSize (100, 100);

        createView (createGenericViewIfNeeded);
    }

    ~AudioUnitPluginWindowCocoa() override
    {
        if (wrapper.getView() != nil)
        {
            wrapper.setVisible (false);
            removeChildComponent (&wrapper);
            wrapper.setView (nil);
            plugin.editorBeingDeleted (this);
        }
    }

    void embedViewController (JUCE_IOS_MAC_VIEW* pluginView, [[maybe_unused]] const CGSize& size)
    {
        wrapper.setView (pluginView);
        waitingForViewCallback = false;

      #if JUCE_MAC
        if (pluginView != nil)
            wrapper.resizeToFitView();
      #else
        [pluginView setBounds: CGRectMake (0.f, 0.f, static_cast<int> (size.width), static_cast<int> (size.height))];
        wrapper.setSize (static_cast<int> (size.width), static_cast<int> (size.height));
      #endif
    }

    bool isValid() const        { return wrapper.getView() != nil || waitingForViewCallback; }

    void paint (Graphics& g) override
    {
        g.fillAll (Colours::white);
    }

    void resized() override
    {
        wrapper.setSize (getWidth(), getHeight());
    }

    void childBoundsChanged (Component*) override
    {
        setSize (wrapper.getWidth(), wrapper.getHeight());
    }

private:

    AudioUnitPluginInstance& plugin;
    AudioUnitFormatHelpers::AutoResizingNSViewComponent wrapper;

    typedef void (^ViewControllerCallbackBlock)(AUViewControllerBase *);

    bool waitingForViewCallback = false;

    bool createView ([[maybe_unused]] bool createGenericViewIfNeeded)
    {
        JUCE_IOS_MAC_VIEW* pluginView = nil;
        UInt32 dataSize = 0;
        Boolean isWritable = false;

       #if JUCE_MAC
        if (AudioUnitGetPropertyInfo (plugin.audioUnit, kAudioUnitProperty_CocoaUI, kAudioUnitScope_Global,
                                      0, &dataSize, &isWritable) == noErr
             && dataSize != 0
             && AudioUnitGetPropertyInfo (plugin.audioUnit, kAudioUnitProperty_CocoaUI, kAudioUnitScope_Global,
                                          0, &dataSize, &isWritable) == noErr)
        {
            HeapBlock<AudioUnitCocoaViewInfo> info;
            info.calloc (dataSize, 1);

            if (AudioUnitGetProperty (plugin.audioUnit, kAudioUnitProperty_CocoaUI, kAudioUnitScope_Global,
                                      0, info, &dataSize) == noErr)
            {
                NSString* viewClassName = (NSString*) (info->mCocoaAUViewClass[0]);
                CFUniquePtr<CFStringRef> path (CFURLCopyPath (info->mCocoaAUViewBundleLocation));
                NSString* unescapedPath = (NSString*) CFURLCreateStringByReplacingPercentEscapes (nullptr, path.get(), CFSTR (""));
                NSBundle* viewBundle = [NSBundle bundleWithPath: [unescapedPath autorelease]];
                Class viewClass = [viewBundle classNamed: viewClassName];

                if ([viewClass conformsToProtocol: @protocol (AUCocoaUIBase)]
                     && [viewClass instancesRespondToSelector: @selector (interfaceVersion)]
                     && [viewClass instancesRespondToSelector: @selector (uiViewForAudioUnit: withSize:)])
                {
                    id factory = [[[viewClass alloc] init] autorelease];
                    pluginView = [factory uiViewForAudioUnit: plugin.audioUnit
                                                    withSize: NSMakeSize (getWidth(), getHeight())];
                }

                for (int i = (dataSize - sizeof (CFURLRef)) / sizeof (CFStringRef); --i >= 0;)
                    CFRelease (info->mCocoaAUViewClass[i]);

                CFRelease (info->mCocoaAUViewBundleLocation);
            }
        }
       #endif

        dataSize = 0;
        isWritable = false;

        if (AudioUnitGetPropertyInfo (plugin.audioUnit, kAudioUnitProperty_RequestViewController, kAudioUnitScope_Global,
                                          0, &dataSize, &isWritable) == noErr
                && dataSize == sizeof (ViewControllerCallbackBlock))
        {
            waitingForViewCallback = true;
            auto callback = ^(AUViewControllerBase* controller) { this->requestViewControllerCallback (controller); };

            if (noErr == AudioUnitSetProperty (plugin.audioUnit, kAudioUnitProperty_RequestViewController, kAudioUnitScope_Global, 0, &callback, dataSize))
                return true;

            waitingForViewCallback = false;
        }

       #if JUCE_MAC
        if (createGenericViewIfNeeded && (pluginView == nil))
        {
            {
                // This forces CoreAudio.component to be loaded, otherwise the AUGenericView will assert
                AudioComponentDescription desc;
                String name, version, manufacturer;
                AudioUnitFormatHelpers::getComponentDescFromIdentifier ("AudioUnit:Output/auou,genr,appl",
                                                                        desc, name, version, manufacturer);
            }

            pluginView = [[AUGenericView alloc] initWithAudioUnit: plugin.audioUnit];
        }
       #endif

        wrapper.setView (pluginView);

        if (pluginView != nil)
            wrapper.resizeToFitView();

        return pluginView != nil;
    }

    void requestViewControllerCallback (AUViewControllerBase* controller)
    {
        const auto viewSize = [&controller]
        {
            auto size = CGSizeZero;

            if (@available (macOS 10.11, *))
                size = [controller preferredContentSize];

            if (approximatelyEqual (size.width, 0.0) || approximatelyEqual (size.height, 0.0))
                size = controller.view.frame.size;

            return CGSizeMake (jmax ((CGFloat) 20.0f, size.width),
                               jmax ((CGFloat) 20.0f, size.height));
        }();

        if (! MessageManager::getInstance()->isThisTheMessageThread())
        {
            struct AsyncViewControllerCallback final : public CallbackMessage
            {
                AudioUnitPluginWindowCocoa* owner;
                JUCE_IOS_MAC_VIEW* controllerView;
                CGSize size;

                AsyncViewControllerCallback (AudioUnitPluginWindowCocoa* plugInWindow, JUCE_IOS_MAC_VIEW* inView,
                                             const CGSize& preferredSize)
                    : owner (plugInWindow), controllerView ([inView retain]), size (preferredSize)
                {}

                void messageCallback() override
                {
                    owner->embedViewController (controllerView, size);
                    [controllerView release];
                }
            };

            (new AsyncViewControllerCallback (this, [controller view], viewSize))->post();
        }
        else
        {
            embedViewController ([controller view], viewSize);
        }
    }
};

//==============================================================================
AudioProcessorEditor* AudioUnitPluginInstance::createEditor()
{
    std::unique_ptr<AudioProcessorEditor> w (new AudioUnitPluginWindowCocoa (*this, false));

    if (! static_cast<AudioUnitPluginWindowCocoa*> (w.get())->isValid())
        w.reset (new AudioUnitPluginWindowCocoa (*this, true)); // use AUGenericView as a fallback

    return w.release();
}

//==============================================================================
AudioUnitPluginFormat::AudioUnitPluginFormat()
{
}

AudioUnitPluginFormat::~AudioUnitPluginFormat()
{
}

void AudioUnitPluginFormat::findAllTypesForFile (OwnedArray<PluginDescription>& results,
                                                 const String& fileOrIdentifier)
{
    if (! fileMightContainThisPluginType (fileOrIdentifier))
        return;

    PluginDescription desc;
    desc.fileOrIdentifier = fileOrIdentifier;
    desc.uniqueId = desc.deprecatedUid = 0;

    if (MessageManager::getInstance()->isThisTheMessageThread()
          && requiresUnblockedMessageThreadDuringCreation (desc))
        return;

    try
    {
        auto createdInstance = createInstanceFromDescription (desc, 44100.0, 512);

        if (auto auInstance = dynamic_cast<AudioUnitPluginInstance*> (createdInstance.get()))
            results.add (new PluginDescription (auInstance->getPluginDescription()));
    }
    catch (...)
    {
        // crashed while loading...
    }
}

void AudioUnitPluginFormat::createPluginInstance (const PluginDescription& desc,
                                                  double rate, int blockSize,
                                                  PluginCreationCallback callback)
{
    auto auComponentResult = getAudioComponent (*this, desc);

    if (! auComponentResult.isValid())
    {
        callback (nullptr, std::move (auComponentResult.errorMessage));
        return;
    }

    createAudioUnit (auComponentResult.component,
                     [rate, blockSize, origCallback = std::move (callback)] (AudioUnit audioUnit, OSStatus err)
                     {
                        if (err == noErr)
                        {
                            auto instance = std::make_unique<AudioUnitPluginInstance> (audioUnit);

                            if (instance->initialise (rate, blockSize))
                                origCallback (std::move (instance), {});
                            else
                                origCallback (nullptr, NEEDS_TRANS ("Unable to initialise the AudioUnit plug-in"));
                        }
                        else
                        {
                            auto errMsg = TRANS ("An OS error occurred during initialisation of the plug-in (XXX)");
                            origCallback (nullptr, errMsg.replace ("XXX", String (err)));
                        }
                    });
}

void AudioUnitPluginFormat::createARAFactoryAsync (const PluginDescription& desc, ARAFactoryCreationCallback callback)
{
    auto auComponentResult = getAudioComponent (*this, desc);

    if (! auComponentResult.isValid())
    {
        callback ({ {}, "Failed to create AudioComponent for " + desc.descriptiveName });
        return;
    }

    getOrCreateARAAudioUnit (auComponentResult.component, [cb = std::move (callback)] (auto dylibKeepAliveAudioUnit)
    {
        cb ([&]() -> ARAFactoryResult
            {
                if (dylibKeepAliveAudioUnit != nullptr)
                    return { ARAFactoryWrapper { ::juce::getARAFactory (std::move (dylibKeepAliveAudioUnit)) }, "" };

                return { {}, "Failed to create ARAFactory from the provided AudioUnit" };
            }());
    });
}

bool AudioUnitPluginFormat::requiresUnblockedMessageThreadDuringCreation (const PluginDescription& desc) const
{
    String pluginName, version, manufacturer;
    AudioComponentDescription componentDesc;

    if (AudioUnitFormatHelpers::getComponentDescFromIdentifier (desc.fileOrIdentifier, componentDesc,
                                                                pluginName, version, manufacturer)
           || AudioUnitFormatHelpers::getComponentDescFromFile (desc.fileOrIdentifier, componentDesc,
                                                                pluginName, version, manufacturer))
    {
        if (AudioComponent auComp = AudioComponentFindNext (nullptr, &componentDesc))
        {
            if (AudioComponentGetDescription (auComp, &componentDesc) == noErr)
                return AudioUnitFormatHelpers::isPluginAUv3 (componentDesc);
        }
    }

    return false;
}

StringArray AudioUnitPluginFormat::searchPathsForPlugins (const FileSearchPath&, bool /*recursive*/, bool allowPluginsWhichRequireAsynchronousInstantiation)
{
    StringArray result;
    AudioComponent comp = nullptr;

    for (;;)
    {
        AudioComponentDescription desc;
        zerostruct (desc);

        comp = AudioComponentFindNext (comp, &desc);

        if (comp == nullptr)
            break;

        if (AudioComponentGetDescription (comp, &desc) != noErr)
            continue;

        if (desc.componentType == kAudioUnitType_MusicDevice
             || desc.componentType == kAudioUnitType_MusicEffect
             || desc.componentType == kAudioUnitType_Effect
             || desc.componentType == kAudioUnitType_Generator
             || desc.componentType == kAudioUnitType_Panner
             || desc.componentType == kAudioUnitType_Mixer
             || desc.componentType == kAudioUnitType_MIDIProcessor)
        {
            if (allowPluginsWhichRequireAsynchronousInstantiation || ! AudioUnitFormatHelpers::isPluginAUv3 (desc))
                result.add (AudioUnitFormatHelpers::createPluginIdentifier (desc));
        }
    }

    return result;
}

bool AudioUnitPluginFormat::fileMightContainThisPluginType (const String& fileOrIdentifier)
{
    AudioComponentDescription desc;
    String name, version, manufacturer;

    if (AudioUnitFormatHelpers::getComponentDescFromIdentifier (fileOrIdentifier, desc, name, version, manufacturer))
        return AudioComponentFindNext (nullptr, &desc) != nullptr;

    auto f = File::createFileWithoutCheckingPath (fileOrIdentifier);

    return (f.hasFileExtension (".component") || f.hasFileExtension (".appex"))
             && f.isDirectory();
}

String AudioUnitPluginFormat::getNameOfPluginFromIdentifier (const String& fileOrIdentifier)
{
    AudioComponentDescription desc;
    String name, version, manufacturer;
    AudioUnitFormatHelpers::getComponentDescFromIdentifier (fileOrIdentifier, desc, name, version, manufacturer);

    if (name.isEmpty())
        name = fileOrIdentifier;

    return name;
}

bool AudioUnitPluginFormat::pluginNeedsRescanning (const PluginDescription& desc)
{
    AudioComponentDescription newDesc;
    String name, version, manufacturer;

    return ! (AudioUnitFormatHelpers::getComponentDescFromIdentifier (desc.fileOrIdentifier, newDesc,
                                                                      name, version, manufacturer)
               && version == desc.version);
}

bool AudioUnitPluginFormat::doesPluginStillExist (const PluginDescription& desc)
{
    if (desc.fileOrIdentifier.startsWithIgnoreCase (AudioUnitFormatHelpers::auIdentifierPrefix))
        return fileMightContainThisPluginType (desc.fileOrIdentifier);

    return File (desc.fileOrIdentifier).exists();
}

FileSearchPath AudioUnitPluginFormat::getDefaultLocationsToSearch()
{
    return {};
}

#undef JUCE_AU_LOG

} // namespace juce

JUCE_END_IGNORE_WARNINGS_GCC_LIKE

#endif

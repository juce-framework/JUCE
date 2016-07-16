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

#if JUCE_PLUGINHOST_AU && (JUCE_MAC || JUCE_IOS)

} // (juce namespace)

#include <AudioUnit/AudioUnit.h>
#if JUCE_MAC
#include <AudioUnit/AUCocoaUIView.h>
#include <CoreAudioKit/AUGenericView.h>
#include <AudioToolbox/AudioUnitUtilities.h>
#endif

#include <CoreMIDI/MIDIServices.h>

#if JUCE_SUPPORT_CARBON
 #include <AudioUnit/AudioUnitCarbonView.h>
#endif

#ifndef JUCE_SUPPORTS_AUv3
 #if JUCE_COMPILER_SUPPORTS_VARIADIC_TEMPLATES && __OBJC2__  \
      &&  ((defined (MAC_OS_X_VERSION_MIN_REQUIRED)    && defined (MAC_OS_X_VERSION_10_11) && (MAC_OS_X_VERSION_MIN_REQUIRED    >= MAC_OS_X_VERSION_10_11)) \
       ||  (defined (__IPHONE_OS_VERSION_MIN_REQUIRED) && defined (__IPHONE_9_0)           && (__IPHONE_OS_VERSION_MIN_REQUIRED >= __IPHONE_9_0)))
  #define JUCE_SUPPORTS_AUv3 1
 #else
  #define JUCE_SUPPORTS_AUv3 0
 #endif
#endif

#if JUCE_SUPPORTS_AUv3
 #include <CoreAudioKit/AUViewController.h>
#endif

namespace juce
{

#include "../../juce_audio_devices/native/juce_MidiDataConcatenator.h"

#if JUCE_SUPPORT_CARBON
 #include "../../juce_gui_extra/native/juce_mac_CarbonViewWrapperComponent.h"
#endif

#include "../../juce_core/native/juce_osx_ObjCHelpers.h"

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

    String osTypeToString (OSType type) noexcept
    {
        const juce_wchar s[4] = { (juce_wchar) ((type >> 24) & 0xff),
                                  (juce_wchar) ((type >> 16) & 0xff),
                                  (juce_wchar) ((type >> 8) & 0xff),
                                  (juce_wchar) (type & 0xff) };
        return String (s, 4);
    }

    OSType stringToOSType (String s)
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

    String createPluginIdentifier (const AudioComponentDescription& desc)
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

        s << osTypeToString (desc.componentType) << ","
          << osTypeToString (desc.componentSubType) << ","
          << osTypeToString (desc.componentManufacturer);

        return s;
    }

    void getNameAndManufacturer (AudioComponent comp, String& name, String& manufacturer)
    {
        CFStringRef cfName;
        if (AudioComponentCopyName (comp, &cfName) == noErr)
        {
            name = String::fromCFString (cfName);
            CFRelease (cfName);
        }

        if (name.containsChar (':'))
        {
            manufacturer = name.upToFirstOccurrenceOf (":", false, false).trim();
            name         = name.fromFirstOccurrenceOf (":", false, false).trim();
        }

        if (name.isEmpty())
            name = "<Unknown>";
    }

    bool getComponentDescFromIdentifier (const String& fileOrIdentifier, AudioComponentDescription& desc,
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

                if (AudioComponent comp = AudioComponentFindNext (0, &desc))
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

    bool getComponentDescFromFile (const String& fileOrIdentifier, AudioComponentDescription& desc,
                                   String& name, String& version, String& manufacturer)
    {
        zerostruct (desc);

       #if JUCE_IOS
        ignoreUnused (fileOrIdentifier, name, version, manufacturer);

        return false;
       #else
        const File file (fileOrIdentifier);
        if (! file.hasFileExtension (".component") && ! file.hasFileExtension (".appex"))
            return false;

        const char* const utf8 = fileOrIdentifier.toUTF8();

        if (CFURLRef url = CFURLCreateFromFileSystemRepresentation (0, (const UInt8*) utf8,
                                                                    (CFIndex) strlen (utf8), file.isDirectory()))
        {
            CFBundleRef bundleRef = CFBundleCreate (kCFAllocatorDefault, url);
            CFRelease (url);

            if (bundleRef != 0)
            {
                CFTypeRef bundleName = CFBundleGetValueForInfoDictionaryKey (bundleRef, CFSTR("CFBundleName"));

                if (bundleName != 0 && CFGetTypeID (bundleName) == CFStringGetTypeID())
                    name = String::fromCFString ((CFStringRef) bundleName);

                if (name.isEmpty())
                    name = file.getFileNameWithoutExtension();

                CFTypeRef versionString = CFBundleGetValueForInfoDictionaryKey (bundleRef, CFSTR("CFBundleVersion"));

                if (versionString != 0 && CFGetTypeID (versionString) == CFStringGetTypeID())
                    version = String::fromCFString ((CFStringRef) versionString);

                CFTypeRef manuString = CFBundleGetValueForInfoDictionaryKey (bundleRef, CFSTR("CFBundleGetInfoString"));

                if (manuString != 0 && CFGetTypeID (manuString) == CFStringGetTypeID())
                    manufacturer = String::fromCFString ((CFStringRef) manuString);

                const ResFileRefNum resFileId = CFBundleOpenBundleResourceMap (bundleRef);
                UseResFile (resFileId);

                const OSType thngType = stringToOSType ("thng");

                for (ResourceIndex i = 1; i <= Count1Resources (thngType); ++i)
                {
                    if (Handle h = Get1IndResource (thngType, i))
                    {
                        HLock (h);
                        const uint32* const types = (const uint32*) *h;

                        if (types[0] == kAudioUnitType_MusicDevice
                             || types[0] == kAudioUnitType_MusicEffect
                             || types[0] == kAudioUnitType_Effect
                             || types[0] == kAudioUnitType_Generator
                             || types[0] == kAudioUnitType_Panner)
                        {
                            desc.componentType = types[0];
                            desc.componentSubType = types[1];
                            desc.componentManufacturer = types[2];

                            if (AudioComponent comp = AudioComponentFindNext (0, &desc))
                                getNameAndManufacturer (comp, name, manufacturer);

                            break;
                        }

                        HUnlock (h);
                        ReleaseResource (h);
                    }
                }

                CFBundleCloseBundleResourceMap (bundleRef, resFileId);
                CFRelease (bundleRef);
            }
        }

        return desc.componentType != 0 && desc.componentSubType != 0;
       #endif
    }

    const char* getCategory (OSType type) noexcept
    {
        switch (type)
        {
            case kAudioUnitType_Effect:
            case kAudioUnitType_MusicEffect:    return "Effect";
            case kAudioUnitType_MusicDevice:    return "Synth";
            case kAudioUnitType_Generator:      return "Generator";
            case kAudioUnitType_Panner:         return "Panner";
            default: break;
        }

        return nullptr;
    }
}

//==============================================================================
class AudioUnitPluginWindowCarbon;
class AudioUnitPluginWindowCocoa;

//==============================================================================
class AudioUnitPluginInstance     : public AudioPluginInstance
{
public:
    AudioUnitPluginInstance (AudioComponentInstance au)
        : auComponent (AudioComponentInstanceGetComponent (au)),
          wantsMidiMessages (false),
          producesMidiMessages (false),
          wasPlaying (false),
          prepared (false),
          isAUv3 (false),
          currentBuffer (nullptr),
          numInputBusChannels (0),
          numOutputBusChannels (0),
          numInputBusses (0),
          numOutputBusses (0),
          audioUnit (au),
        #if JUCE_MAC
          eventListenerRef (0),
        #endif
          midiConcatenator (2048)
    {
        using namespace AudioUnitFormatHelpers;

        AudioComponentGetDescription (auComponent, &componentDesc);

      #if JUCE_SUPPORTS_AUv3
        isAUv3 = ((componentDesc.componentFlags & kAudioComponentFlag_IsV3AudioUnit) != 0);
      #endif

        wantsMidiMessages = componentDesc.componentType == kAudioUnitType_MusicDevice
                         || componentDesc.componentType == kAudioUnitType_MusicEffect;

        AudioComponentDescription ignore;
        getComponentDescFromIdentifier (createPluginIdentifier (componentDesc), ignore, pluginName, version, manufacturer);
    }

    ~AudioUnitPluginInstance()
    {
        const ScopedLock sl (lock);

       #if JUCE_DEBUG
        // this indicates that some kind of recursive call is getting triggered that's
        // deleting this plugin while it's still under construction.
        jassert (AudioUnitFormatHelpers::insideCallback.get() == 0);
       #endif

        if (audioUnit != nullptr)
        {
            struct AUDeleter : public CallbackMessage
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

    // called from the destructer above
    void cleanup()
    {
       #if JUCE_MAC
        if (eventListenerRef != 0)
        {
            AUListenerDispose (eventListenerRef);
            eventListenerRef = 0;
        }
       #endif

        if (prepared)
            releaseResources();

        AudioComponentInstanceDispose (audioUnit);
        audioUnit = nullptr;
    }

    bool initialise (double rate, int blockSize)
    {
        updateNumChannels();
        producesMidiMessages = canProduceMidiOutput();
        setPlayConfigDetails ((int) (numInputBusChannels * numInputBusses),
                              (int) (numOutputBusChannels * numOutputBusses),
                              rate, blockSize);
        setLatencySamples (0);

        if (parameters.size() == 0)
        {
            // some plugins crash if initialiseAudioUnit() is called too soon (sigh..), so we'll
            // only call it here if it seems like they it's one of the awkward plugins that can
            // only create their parameters after it has been initialised.
            if (! initialiseAudioUnit())
                return false;

            refreshParameterList();
        }

        setPluginCallbacks();
        return true;
    }

    //==============================================================================
    // AudioPluginInstance methods:

    void fillInPluginDescription (PluginDescription& desc) const override
    {
        desc.name = pluginName;
        desc.descriptiveName = pluginName;
        desc.fileOrIdentifier = AudioUnitFormatHelpers::createPluginIdentifier (componentDesc);
        desc.uid = ((int) componentDesc.componentType)
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
            updateNumChannels();

            Float64 sampleRateIn = 0, sampleRateOut = 0;
            UInt32 sampleRateSize = sizeof (sampleRateIn);
            const Float64 sr = newSampleRate;

            for (AudioUnitElement i = 0; i < numInputBusses; ++i)
            {
                AudioUnitGetProperty (audioUnit, kAudioUnitProperty_SampleRate, kAudioUnitScope_Input, i, &sampleRateIn, &sampleRateSize);

                if (sampleRateIn != sr)
                    AudioUnitSetProperty (audioUnit, kAudioUnitProperty_SampleRate, kAudioUnitScope_Input, i, &sr, sizeof (sr));
            }

            for (AudioUnitElement i = 0; i < numOutputBusses; ++i)
            {
                AudioUnitGetProperty (audioUnit, kAudioUnitProperty_SampleRate, kAudioUnitScope_Output, i, &sampleRateOut, &sampleRateSize);

                if (sampleRateOut != sr)
                    AudioUnitSetProperty (audioUnit, kAudioUnitProperty_SampleRate, kAudioUnitScope_Output, i, &sr, sizeof (sr));
            }

            UInt32 frameSize = (UInt32) estimatedSamplesPerBlock;
            AudioUnitSetProperty (audioUnit, kAudioUnitProperty_MaximumFramesPerSlice, kAudioUnitScope_Global, 0,
                                  &frameSize, sizeof (frameSize));

            setPlayConfigDetails ((int) (numInputBusChannels * numInputBusses),
                                  (int) (numOutputBusChannels * numOutputBusses),
                                  (double) newSampleRate, estimatedSamplesPerBlock);

            updateLatency();

            {
                AudioStreamBasicDescription stream;
                zerostruct (stream); // (can't use "= { 0 }" on this object because it's typedef'ed as a C struct)
                stream.mSampleRate       = sr;
                stream.mFormatID         = kAudioFormatLinearPCM;
                stream.mFormatFlags      = kAudioFormatFlagsNativeFloatPacked | kAudioFormatFlagIsNonInterleaved | kAudioFormatFlagsNativeEndian;
                stream.mFramesPerPacket  = 1;
                stream.mBytesPerPacket   = 4;
                stream.mBytesPerFrame    = 4;
                stream.mBitsPerChannel   = 32;
                stream.mChannelsPerFrame = numInputBusChannels;

                for (AudioUnitElement i = 0; i < numInputBusses; ++i)
                    AudioUnitSetProperty (audioUnit, kAudioUnitProperty_StreamFormat,
                                          kAudioUnitScope_Input, i, &stream, sizeof (stream));

                stream.mChannelsPerFrame = numOutputBusChannels;

                for (AudioUnitElement i = 0; i < numOutputBusses; ++i)
                    AudioUnitSetProperty (audioUnit, kAudioUnitProperty_StreamFormat,
                                          kAudioUnitScope_Output, i, &stream, sizeof (stream));
            }

            if (numOutputBusses != 0 && numOutputBusChannels != 0)
                outputBufferList.calloc (numOutputBusses, getAudioBufferSizeInBytes());

            zerostruct (timeStamp);
            timeStamp.mSampleTime = 0;
            timeStamp.mHostTime = GetCurrentHostTime (0, sr, isAUv3);
            timeStamp.mFlags = kAudioTimeStampSampleTimeValid | kAudioTimeStampHostTimeValid;

            currentBuffer = nullptr;
            wasPlaying = false;

            resetBusses();

            jassert (! prepared);
            initialiseAudioUnit();
        }
    }

    void releaseResources() override
    {
        if (prepared)
        {
            AudioUnitUninitialize (audioUnit);
            resetBusses();
            AudioUnitReset (audioUnit, kAudioUnitScope_Global, 0);

            outputBufferList.free();
            currentBuffer = nullptr;
            prepared = false;
        }

        incomingMidi.clear();
    }

    bool initialiseAudioUnit()
    {
        if (! prepared)
            prepared = (AudioUnitInitialize (audioUnit) == noErr);

        return prepared;
    }

    void resetBusses()
    {
        for (AudioUnitElement i = 0; i < numInputBusses; ++i)   AudioUnitReset (audioUnit, kAudioUnitScope_Input, i);
        for (AudioUnitElement i = 0; i < numOutputBusses; ++i)  AudioUnitReset (audioUnit, kAudioUnitScope_Output, i);
    }

    void processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages) override
    {
        const int numSamples = buffer.getNumSamples();

        if (prepared)
        {
            timeStamp.mHostTime = GetCurrentHostTime (numSamples, getSampleRate(), isAUv3);

            for (AudioUnitElement i = 0; i < numOutputBusses; ++i)
            {
                if (AudioBufferList* const abl = getAudioBufferListForBus(i))
                {
                    abl->mNumberBuffers = numOutputBusChannels;

                    for (AudioUnitElement j = 0; j < numOutputBusChannels; ++j)
                    {
                        abl->mBuffers[j].mNumberChannels = 1;
                        abl->mBuffers[j].mDataByteSize = (UInt32) (sizeof (float) * (size_t) numSamples);
                        abl->mBuffers[j].mData = buffer.getWritePointer ((int) (i * numOutputBusChannels + j));
                    }
                }
            }

            currentBuffer = &buffer;

            if (wantsMidiMessages)
            {
                const uint8* midiEventData;
                int midiEventSize, midiEventPosition;

                for (MidiBuffer::Iterator i (midiMessages); i.getNextEvent (midiEventData, midiEventSize, midiEventPosition);)
                {
                    if (midiEventSize <= 3)
                        MusicDeviceMIDIEvent (audioUnit,
                                              midiEventData[0], midiEventData[1], midiEventData[2],
                                              (UInt32) midiEventPosition);
                    else
                        MusicDeviceSysEx (audioUnit, midiEventData, (UInt32) midiEventSize);
                }

                midiMessages.clear();
            }


            for (AudioUnitElement i = 0; i < numOutputBusses; ++i)
            {
                AudioUnitRenderActionFlags flags = 0;
                AudioUnitRender (audioUnit, &flags, &timeStamp, i, (UInt32) numSamples, getAudioBufferListForBus (i));
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

    //==============================================================================
    bool hasEditor() const override                  { return true; }
    AudioProcessorEditor* createEditor() override;

    //==============================================================================
    const String getInputChannelName (int index) const override
    {
        if (isPositiveAndBelow (index, getTotalNumInputChannels()))
            return "Input " + String (index + 1);

        return String();
    }

    const String getOutputChannelName (int index) const override
    {
        if (isPositiveAndBelow (index, getTotalNumOutputChannels()))
            return "Output " + String (index + 1);

        return String();
    }

    bool isInputChannelStereoPair (int index) const override    { return isPositiveAndBelow (index, getTotalNumInputChannels()); }
    bool isOutputChannelStereoPair (int index) const override   { return isPositiveAndBelow (index, getTotalNumOutputChannels()); }

    //==============================================================================
    int getNumParameters() override              { return parameters.size(); }

    float getParameter (int index) override
    {
        const ScopedLock sl (lock);

        AudioUnitParameterValue value = 0;

        if (audioUnit != nullptr)
        {
            if (const ParamInfo* p = parameters[index])
            {
                AudioUnitGetParameter (audioUnit,
                                       p->paramID,
                                       kAudioUnitScope_Global, 0,
                                       &value);

                value = (value - p->minValue) / (p->maxValue - p->minValue);
            }
        }

        return value;
    }

    void setParameter (int index, float newValue) override
    {
        const ScopedLock sl (lock);

        if (audioUnit != nullptr)
        {
            if (const ParamInfo* p = parameters[index])
            {
                AudioUnitSetParameter (audioUnit, p->paramID, kAudioUnitScope_Global, 0,
                                       p->minValue + (p->maxValue - p->minValue) * newValue, 0);

                sendParameterChangeEvent (index);
            }
        }
    }

    void sendParameterChangeEvent (int index)
    {
       #if JUCE_MAC
        jassert (audioUnit != nullptr);

        const ParamInfo& p = *parameters.getUnchecked (index);

        AudioUnitEvent ev;
        ev.mEventType                        = kAudioUnitEvent_ParameterValueChange;
        ev.mArgument.mParameter.mAudioUnit   = audioUnit;
        ev.mArgument.mParameter.mParameterID = p.paramID;
        ev.mArgument.mParameter.mScope       = kAudioUnitScope_Global;
        ev.mArgument.mParameter.mElement     = 0;

        AUEventListenerNotify (nullptr, nullptr, &ev);
       #else
        ignoreUnused (index);
       #endif
    }

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

    const String getParameterName (int index) override
    {
        if (const ParamInfo* p = parameters[index])
            return p->name;

        return String();
    }

    const String getParameterText (int index) override   { return String (getParameter (index)); }

    bool isParameterAutomatable (int index) const override
    {
        if (const ParamInfo* p = parameters[index])
            return p->automatable;

        return false;
    }

    //==============================================================================
    int getNumPrograms() override
    {
        CFArrayRef presets;
        UInt32 sz = sizeof (CFArrayRef);
        int num = 0;

        if (AudioUnitGetProperty (audioUnit, kAudioUnitProperty_FactoryPresets,
                                  kAudioUnitScope_Global, 0, &presets, &sz) == noErr)
        {
            num = (int) CFArrayGetCount (presets);
            CFRelease (presets);
        }

        return num;
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
        AUPreset current;
        current.presetNumber = newIndex;
        current.presetName = CFSTR("");

        AudioUnitSetProperty (audioUnit, kAudioUnitProperty_PresentPreset,
                              kAudioUnitScope_Global, 0, &current, sizeof (AUPreset));

        sendAllParametersChangedEvents();
    }

    const String getProgramName (int index) override
    {
        String s;
        CFArrayRef presets;
        UInt32 sz = sizeof (CFArrayRef);

        if (AudioUnitGetProperty (audioUnit, kAudioUnitProperty_FactoryPresets,
                                  kAudioUnitScope_Global, 0, &presets, &sz) == noErr)
        {
            for (CFIndex i = 0; i < CFArrayGetCount (presets); ++i)
            {
                if (const AUPreset* p = (const AUPreset*) CFArrayGetValueAtIndex (presets, i))
                {
                    if (p->presetNumber == index)
                    {
                        s = String::fromCFString (p->presetName);
                        break;
                    }
                }
            }

            CFRelease (presets);
        }

        return s;
    }

    void changeProgramName (int /*index*/, const String& /*newName*/) override
    {
        jassertfalse; // xxx not implemented!
    }

    //==============================================================================
    void getStateInformation (MemoryBlock& destData) override
    {
        getCurrentProgramStateInformation (destData);
    }

    void getCurrentProgramStateInformation (MemoryBlock& destData) override
    {
        CFPropertyListRef propertyList = 0;
        UInt32 sz = sizeof (CFPropertyListRef);

        if (AudioUnitGetProperty (audioUnit,
                                  kAudioUnitProperty_ClassInfo,
                                  kAudioUnitScope_Global,
                                  0, &propertyList, &sz) == noErr)
        {
            CFWriteStreamRef stream = CFWriteStreamCreateWithAllocatedBuffers (kCFAllocatorDefault, kCFAllocatorDefault);
            CFWriteStreamOpen (stream);

            CFIndex bytesWritten = CFPropertyListWriteToStream (propertyList, stream, kCFPropertyListBinaryFormat_v1_0, 0);
            CFWriteStreamClose (stream);

            CFDataRef data = (CFDataRef) CFWriteStreamCopyProperty (stream, kCFStreamPropertyDataWritten);

            destData.setSize ((size_t) bytesWritten);
            destData.copyFrom (CFDataGetBytePtr (data), 0, destData.getSize());
            CFRelease (data);

            CFRelease (stream);
            CFRelease (propertyList);
        }
    }

    void setStateInformation (const void* data, int sizeInBytes) override
    {
        setCurrentProgramStateInformation (data, sizeInBytes);
    }

    void setCurrentProgramStateInformation (const void* data, int sizeInBytes) override
    {
        CFReadStreamRef stream = CFReadStreamCreateWithBytesNoCopy (kCFAllocatorDefault, (const UInt8*) data,
                                                                    sizeInBytes, kCFAllocatorNull);
        CFReadStreamOpen (stream);

        CFPropertyListFormat format = kCFPropertyListBinaryFormat_v1_0;
        CFPropertyListRef propertyList = CFPropertyListCreateFromStream (kCFAllocatorDefault, stream, 0,
                                                                         kCFPropertyListImmutable, &format, 0);
        CFRelease (stream);

        if (propertyList != 0)
        {
            initialiseAudioUnit();

            AudioUnitSetProperty (audioUnit, kAudioUnitProperty_ClassInfo, kAudioUnitScope_Global,
                                  0, &propertyList, sizeof (propertyList));

            sendAllParametersChangedEvents();

            CFRelease (propertyList);
        }
    }

    void refreshParameterList() override
    {
        parameters.clear();

        if (audioUnit != nullptr)
        {
            UInt32 paramListSize = 0;
            AudioUnitGetPropertyInfo (audioUnit, kAudioUnitProperty_ParameterList, kAudioUnitScope_Global,
                                      0, &paramListSize, nullptr);

            if (paramListSize > 0)
            {
                const size_t numParams = paramListSize / sizeof (int);

                HeapBlock<UInt32> ids;
                ids.calloc (numParams);

                AudioUnitGetProperty (audioUnit, kAudioUnitProperty_ParameterList, kAudioUnitScope_Global,
                                      0, ids, &paramListSize);

                for (size_t i = 0; i < numParams; ++i)
                {
                    AudioUnitParameterInfo info;
                    UInt32 sz = sizeof (info);

                    if (AudioUnitGetProperty (audioUnit,
                                              kAudioUnitProperty_ParameterInfo,
                                              kAudioUnitScope_Global,
                                              ids[i], &info, &sz) == noErr)
                    {
                        ParamInfo* const param = new ParamInfo();
                        parameters.add (param);
                        param->paramID = ids[i];
                        param->minValue = info.minValue;
                        param->maxValue = info.maxValue;
                        param->automatable = (info.flags & kAudioUnitParameterFlag_NonRealTime) == 0;

                        if ((info.flags & kAudioUnitParameterFlag_HasCFNameString) != 0)
                        {
                            param->name = String::fromCFString (info.cfNameString);

                            if ((info.flags & kAudioUnitParameterFlag_CFNameRelease) != 0)
                                CFRelease (info.cfNameString);
                        }
                        else
                        {
                            param->name = String (info.name, sizeof (info.name));
                        }
                    }
                }
            }
        }
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

private:
    //==============================================================================
    friend class AudioUnitPluginWindowCarbon;
    friend class AudioUnitPluginWindowCocoa;
    friend class AudioUnitPluginFormat;

    AudioComponentDescription componentDesc;
    AudioComponent auComponent;
    String pluginName, manufacturer, version;
    String fileOrIdentifier;
    CriticalSection lock;
    bool wantsMidiMessages, producesMidiMessages, wasPlaying, prepared, isAUv3;

    HeapBlock<AudioBufferList> outputBufferList;
    AudioTimeStamp timeStamp;
    AudioSampleBuffer* currentBuffer;
    AudioUnitElement numInputBusChannels, numOutputBusChannels, numInputBusses, numOutputBusses;

    AudioUnit audioUnit;
   #if JUCE_MAC
    AUEventListenerRef eventListenerRef;
   #endif

    struct ParamInfo
    {
        UInt32 paramID;
        String name;
        AudioUnitParameterValue minValue, maxValue;
        bool automatable;
    };

    OwnedArray<ParamInfo> parameters;

    MidiDataConcatenator midiConcatenator;
    CriticalSection midiInLock;
    MidiBuffer incomingMidi;

    void setPluginCallbacks()
    {
        if (audioUnit != nullptr)
        {
            {
                AURenderCallbackStruct info;
                zerostruct (info); // (can't use "= { 0 }" on this object because it's typedef'ed as a C struct)

                info.inputProcRefCon = this;
                info.inputProc = renderGetInputCallback;

                for (AudioUnitElement i = 0; i < numInputBusses; ++i)
                    AudioUnitSetProperty (audioUnit, kAudioUnitProperty_SetRenderCallback,
                                          kAudioUnitScope_Input, i, &info, sizeof (info));
            }

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

            {
                HostCallbackInfo info;
                zerostruct (info);

                info.hostUserData = this;
                info.beatAndTempoProc = getBeatAndTempoCallback;
                info.musicalTimeLocationProc = getMusicalTimeLocationCallback;
                info.transportStateProc = getTransportStateCallback;

                AudioUnitSetProperty (audioUnit, kAudioUnitProperty_HostCallbacks,
                                      kAudioUnitScope_Global, 0, &info, sizeof (info));
            }
           #if JUCE_MAC
            AUEventListenerCreate (eventListenerCallback, this, CFRunLoopGetMain(),
                                   kCFRunLoopDefaultMode, 0, 0, &eventListenerRef);

            for (int i = 0; i < parameters.size(); ++i)
            {
                AudioUnitEvent event;
                event.mArgument.mParameter.mAudioUnit = audioUnit;
                event.mArgument.mParameter.mParameterID = parameters.getUnchecked(i)->paramID;
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
           #endif
        }
    }

   #if JUCE_MAC
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
        int paramIndex = -1;

        if (event.mEventType == kAudioUnitEvent_ParameterValueChange
         || event.mEventType == kAudioUnitEvent_BeginParameterChangeGesture
         || event.mEventType == kAudioUnitEvent_EndParameterChangeGesture)
        {
            for (paramIndex = 0; paramIndex < parameters.size(); ++paramIndex)
            {
                const ParamInfo& p = *parameters.getUnchecked(paramIndex);

                if (p.paramID == event.mArgument.mParameter.mParameterID)
                    break;
            }

            if (! isPositiveAndBelow (paramIndex, parameters.size()))
                return;
        }

        switch (event.mEventType)
        {
            case kAudioUnitEvent_ParameterValueChange:
                {
                    const ParamInfo& p = *parameters.getUnchecked(paramIndex);
                    sendParamChangeMessageToListeners (paramIndex, (newValue - p.minValue) / (p.maxValue - p.minValue));
                }
                break;

            case kAudioUnitEvent_BeginParameterChangeGesture:
                beginParameterChangeGesture (paramIndex);
                break;

            case kAudioUnitEvent_EndParameterChangeGesture:
                endParameterChangeGesture (paramIndex);
                break;

            default:
                if (event.mArgument.mProperty.mPropertyID == kAudioUnitProperty_ParameterList)
                    updateHostDisplay();
                else if (event.mArgument.mProperty.mPropertyID == kAudioUnitProperty_PresentPreset)
                    sendAllParametersChangedEvents();
                else if (event.mArgument.mProperty.mPropertyID == kAudioUnitProperty_Latency)
                    updateLatency();

                break;
        }
    }

    static void eventListenerCallback (void* userRef, void*, const AudioUnitEvent* event,
                                       UInt64, AudioUnitParameterValue value)
    {
        jassert (event != nullptr);
        static_cast<AudioUnitPluginInstance*> (userRef)->eventCallback (*event, value);
    }
   #endif

    //==============================================================================
    OSStatus renderGetInput (AudioUnitRenderActionFlags*,
                             const AudioTimeStamp*,
                             UInt32 inBusNumber,
                             UInt32 inNumberFrames,
                             AudioBufferList* ioData) const
    {
        if (currentBuffer != nullptr)
        {
            // if this ever happens, might need to add extra handling
            jassert (inNumberFrames == (UInt32) currentBuffer->getNumSamples());

            for (UInt32 i = 0; i < ioData->mNumberBuffers; ++i)
            {
                const int bufferChannel = (int) (inBusNumber * numInputBusChannels + i);

                if (bufferChannel < currentBuffer->getNumChannels())
                {
                    memcpy (ioData->mBuffers[i].mData,
                            currentBuffer->getReadPointer (bufferChannel),
                            sizeof (float) * inNumberFrames);
                }
                else
                {
                    zeromem (ioData->mBuffers[i].mData,
                             sizeof (float) * inNumberFrames);
                }
            }
        }

        return noErr;
    }

    OSStatus renderMidiOutput (const MIDIPacketList* pktlist)
    {
        if (pktlist != nullptr && pktlist->numPackets)
        {
            const double time = Time::getMillisecondCounterHiRes() * 0.001;
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

    OSStatus getBeatAndTempo (Float64* outCurrentBeat, Float64* outCurrentTempo) const
    {
        AudioPlayHead* const ph = getPlayHead();
        AudioPlayHead::CurrentPositionInfo result;

        if (ph != nullptr && ph->getCurrentPosition (result))
        {
            setIfNotNull (outCurrentBeat, result.ppqPosition);
            setIfNotNull (outCurrentTempo, result.bpm);
        }
        else
        {
            setIfNotNull (outCurrentBeat, 0);
            setIfNotNull (outCurrentTempo, 120.0);
        }

        return noErr;
    }

    OSStatus getMusicalTimeLocation (UInt32* outDeltaSampleOffsetToNextBeat, Float32* outTimeSig_Numerator,
                                     UInt32* outTimeSig_Denominator, Float64* outCurrentMeasureDownBeat) const
    {
        if (AudioPlayHead* const ph = getPlayHead())
        {
            AudioPlayHead::CurrentPositionInfo result;

            if (ph->getCurrentPosition (result))
            {
                setIfNotNull (outDeltaSampleOffsetToNextBeat, (UInt32) 0); //xxx
                setIfNotNull (outTimeSig_Numerator,   (UInt32) result.timeSigNumerator);
                setIfNotNull (outTimeSig_Denominator, (UInt32) result.timeSigDenominator);
                setIfNotNull (outCurrentMeasureDownBeat, result.ppqPositionOfLastBarStart); //xxx wrong
                return noErr;
            }
        }

        setIfNotNull (outDeltaSampleOffsetToNextBeat, (UInt32) 0);
        setIfNotNull (outTimeSig_Numerator, (UInt32) 4);
        setIfNotNull (outTimeSig_Denominator, (UInt32) 4);
        setIfNotNull (outCurrentMeasureDownBeat, 0);
        return noErr;
    }

    OSStatus getTransportState (Boolean* outIsPlaying, Boolean* outTransportStateChanged,
                                Float64* outCurrentSampleInTimeLine, Boolean* outIsCycling,
                                Float64* outCycleStartBeat, Float64* outCycleEndBeat)
    {
        if (AudioPlayHead* const ph = getPlayHead())
        {
            AudioPlayHead::CurrentPositionInfo result;

            if (ph->getCurrentPosition (result))
            {
                setIfNotNull (outIsPlaying, result.isPlaying);

                if (outTransportStateChanged != nullptr)
                {
                    *outTransportStateChanged = result.isPlaying != wasPlaying;
                    wasPlaying = result.isPlaying;
                }

                setIfNotNull (outCurrentSampleInTimeLine, result.timeInSamples);
                setIfNotNull (outIsCycling, result.isLooping);
                setIfNotNull (outCycleStartBeat, result.ppqLoopStart);
                setIfNotNull (outCycleEndBeat, result.ppqLoopEnd);
                return noErr;
            }
        }

        setIfNotNull (outIsPlaying, false);
        setIfNotNull (outTransportStateChanged, false);
        setIfNotNull (outCurrentSampleInTimeLine, 0);
        setIfNotNull (outIsCycling, false);
        setIfNotNull (outCycleStartBeat, 0.0);
        setIfNotNull (outCycleEndBeat, 0.0);
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
    static inline UInt64 GetCurrentHostTime (int numSamples, double sampleRate, bool isAUv3) noexcept
    {
     #if ! JUCE_IOS
       if (! isAUv3)
           return AudioGetCurrentHostTime();
     #else
        ignoreUnused (isAUv3);
     #endif

        UInt64 currentTime = mach_absolute_time();
        static mach_timebase_info_data_t sTimebaseInfo = {0, 0};

        if (sTimebaseInfo.denom == 0)
            mach_timebase_info (&sTimebaseInfo);

        double bufferNanos = static_cast<double> (numSamples) * 1.0e9 / sampleRate;
        UInt64 bufferTicks = static_cast<UInt64> (std::ceil (bufferNanos * (static_cast<double> (sTimebaseInfo.denom) / static_cast<double> (sTimebaseInfo.numer))));
        currentTime += bufferTicks;

        return currentTime;
    }

    size_t getAudioBufferSizeInBytes() const noexcept
    {
        return offsetof (AudioBufferList, mBuffers) + (sizeof (::AudioBuffer) * numOutputBusChannels);
    }

    AudioBufferList* getAudioBufferListForBus (AudioUnitElement busIndex) const noexcept
    {
        return addBytesToPointer (outputBufferList.getData(), getAudioBufferSizeInBytes() * busIndex);
    }

    AudioUnitElement getElementCount (AudioUnitScope scope) const noexcept
    {
        UInt32 count;
        UInt32 countSize = sizeof (count);

        if (AudioUnitGetProperty (audioUnit, kAudioUnitProperty_ElementCount, scope, 0, &count, &countSize) != noErr
             || countSize == 0)
            count = 1;

        return count;
    }

    void updateNumChannels()
    {
        numInputBusses  = getElementCount (kAudioUnitScope_Input);
        numOutputBusses = getElementCount (kAudioUnitScope_Output);

        AUChannelInfo supportedChannels [128];
        UInt32 supportedChannelsSize = 0;
        Boolean writable;

        if (AudioUnitGetPropertyInfo (audioUnit, kAudioUnitProperty_SupportedNumChannels, kAudioUnitScope_Global,
                                      0, &supportedChannelsSize, &writable) == noErr
             && supportedChannelsSize > 0
             && AudioUnitGetProperty (audioUnit, kAudioUnitProperty_SupportedNumChannels, kAudioUnitScope_Global,
                                      0, supportedChannels, &supportedChannelsSize) == noErr)
        {
            int explicitNumIns = 0;
            int explicitNumOuts = 0;
            int maximumNumIns = 0;
            int maximumNumOuts = 0;

            for (int i = 0; i < (int) (supportedChannelsSize / sizeof (AUChannelInfo)); ++i)
            {
                const int inChannels  = (int) supportedChannels[i].inChannels;
                const int outChannels = (int) supportedChannels[i].outChannels;

                if (inChannels < 0)
                    maximumNumIns  = jmin (maximumNumIns,  inChannels);
                else
                    explicitNumIns = jmax (explicitNumIns, inChannels);

                if (outChannels < 0)
                    maximumNumOuts  = jmin (maximumNumOuts,  outChannels);
                else
                    explicitNumOuts = jmax (explicitNumOuts, outChannels);
            }

            if ((maximumNumIns == -1 && maximumNumOuts == -1)    // (special meaning: any number of ins/outs, as long as they match)
                || (maximumNumIns == -2 && maximumNumOuts == -1) // (special meaning: any number of ins/outs, even if they don't match)
                || (maximumNumIns == -1 && maximumNumOuts == -2))
            {
                numInputBusChannels = numOutputBusChannels = 2;
            }
            else
            {
                numInputBusChannels  = (AudioUnitElement) explicitNumIns;
                numOutputBusChannels = (AudioUnitElement) explicitNumOuts;

                if (maximumNumIns == -1 || (maximumNumIns < 0 && explicitNumIns <= -maximumNumIns))
                    numInputBusChannels = 2;

                if (maximumNumOuts == -1 || (maximumNumOuts < 0 && explicitNumOuts <= -maximumNumOuts))
                    numOutputBusChannels = 2;
            }
        }
        else
        {
            // (this really means the plugin will take any number of ins/outs as long
            // as they are the same)
            numInputBusChannels = numOutputBusChannels = 2;
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
            CFArrayRef midiArray;

            if (AudioUnitGetProperty (audioUnit, kAudioUnitProperty_MIDIOutputCallbackInfo,
                                      kAudioUnitScope_Global, 0, &midiArray, &dataSize) == noErr)
            {
                bool result = (CFArrayGetCount (midiArray) > 0);
                CFRelease (midiArray);
                return result;
            }
        }

        return false;
       #else
        return false;
       #endif
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

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioUnitPluginInstance)
};

//==============================================================================
class AudioUnitPluginWindowCocoa    : public AudioProcessorEditor
{
public:
    AudioUnitPluginWindowCocoa (AudioUnitPluginInstance& p, bool createGenericViewIfNeeded)
        : AudioProcessorEditor (&p),
          plugin (p), waitingForViewCallback (false)
    {
        addAndMakeVisible (wrapper);

       #if JUCE_SUPPORTS_AUv3
        viewControllerCallback =
            CreateObjCBlock (this, &AudioUnitPluginWindowCocoa::requestViewControllerCallback);
       #endif

        setOpaque (true);
        setVisible (true);
        setSize (100, 100);

        createView (createGenericViewIfNeeded);
    }

    ~AudioUnitPluginWindowCocoa()
    {
        if (wrapper.getView() != nil)
        {
            wrapper.setVisible (false);
            removeChildComponent (&wrapper);
            wrapper.setView (nil);
            plugin.editorBeingDeleted (this);
        }
    }

   #if JUCE_SUPPORTS_AUv3
    void embedViewController (JUCE_IOS_MAC_VIEW* pluginView, const CGSize& size)
    {
        wrapper.setView (pluginView);
        waitingForViewCallback = false;

      #if JUCE_MAC
        ignoreUnused (size);
        if (pluginView != nil)
            wrapper.resizeToFitView();
      #else
        [pluginView setBounds: CGRectMake (0.f, 0.f, static_cast<int> (size.width), static_cast<int> (size.height))];
        wrapper.setSize (static_cast<int> (size.width), static_cast<int> (size.height));
      #endif
    }
   #endif

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
    AutoResizingNSViewComponent wrapper;

   #if JUCE_SUPPORTS_AUv3
    typedef void (^ViewControllerCallbackBlock)(AUViewControllerBase *);
    ObjCBlock<ViewControllerCallbackBlock> viewControllerCallback;
   #endif

    bool waitingForViewCallback;

    bool createView (const bool createGenericViewIfNeeded)
    {
        if (! plugin.initialiseAudioUnit())
            return false;

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
                CFStringRef path = CFURLCopyPath (info->mCocoaAUViewBundleLocation);
                NSString* unescapedPath = (NSString*) CFURLCreateStringByReplacingPercentEscapes (0, path, CFSTR (""));
                CFRelease (path);
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

       #if JUCE_SUPPORTS_AUv3
        if (AudioUnitGetPropertyInfo (plugin.audioUnit, kAudioUnitProperty_RequestViewController, kAudioUnitScope_Global,
                                          0, &dataSize, &isWritable) == noErr
                && dataSize == sizeof (ViewControllerCallbackBlock)
                && AudioUnitGetPropertyInfo (plugin.audioUnit, kAudioUnitProperty_RequestViewController, kAudioUnitScope_Global,
                                             0, &dataSize, &isWritable) == noErr)
        {
            waitingForViewCallback = true;
            ViewControllerCallbackBlock callback;
            callback = viewControllerCallback;

            ViewControllerCallbackBlock* info = &callback;

            if (noErr == AudioUnitSetProperty (plugin.audioUnit, kAudioUnitProperty_RequestViewController, kAudioUnitScope_Global, 0, info, dataSize))
                return true;

            waitingForViewCallback = false;
        }
       #endif

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
       #else
        ignoreUnused (createGenericViewIfNeeded);
       #endif

        wrapper.setView (pluginView);

        if (pluginView != nil)
            wrapper.resizeToFitView();

        return pluginView != nil;
    }

   #if JUCE_SUPPORTS_AUv3
    void requestViewControllerCallback (AUViewControllerBase* controller)
    {
        auto nsSize = [controller preferredContentSize];
        auto viewSize = CGSizeMake (nsSize.width, nsSize.height);

        if (! MessageManager::getInstance()->isThisTheMessageThread())
        {
            struct AsyncViewControllerCallback : public CallbackMessage
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
   #endif
};

#if JUCE_SUPPORT_CARBON

//==============================================================================
class AudioUnitPluginWindowCarbon   : public AudioProcessorEditor
{
public:
    AudioUnitPluginWindowCarbon (AudioUnitPluginInstance& p)
        : AudioProcessorEditor (&p),
          plugin (p),
          audioComponent (nullptr),
          viewComponent (nullptr)
    {
        addAndMakeVisible (innerWrapper = new InnerWrapperComponent (*this));

        setOpaque (true);
        setVisible (true);
        setSize (400, 300);

        UInt32 propertySize;
        if (AudioUnitGetPropertyInfo (plugin.audioUnit, kAudioUnitProperty_GetUIComponentList,
                                      kAudioUnitScope_Global, 0, &propertySize, NULL) == noErr
             && propertySize > 0)
        {
            HeapBlock<AudioComponentDescription> views (propertySize / sizeof (AudioComponentDescription));

            if (AudioUnitGetProperty (plugin.audioUnit, kAudioUnitProperty_GetUIComponentList,
                                      kAudioUnitScope_Global, 0, &views[0], &propertySize) == noErr)
            {
                audioComponent = AudioComponentFindNext (nullptr, &views[0]);
            }
        }
    }

    ~AudioUnitPluginWindowCarbon()
    {
        innerWrapper = nullptr;

        if (isValid())
            plugin.editorBeingDeleted (this);
    }

    bool isValid() const noexcept           { return audioComponent != nullptr; }

    //==============================================================================
    void paint (Graphics& g) override
    {
        g.fillAll (Colours::black);
    }

    void resized() override
    {
        if (innerWrapper != nullptr)
            innerWrapper->setSize (getWidth(), getHeight());
    }

    //==============================================================================
    bool keyStateChanged (bool) override         { return false; }
    bool keyPressed (const KeyPress&) override   { return false; }

    //==============================================================================
    AudioUnit getAudioUnit() const      { return plugin.audioUnit; }

    AudioUnitCarbonView getViewComponent()
    {
        if (viewComponent == nullptr && audioComponent != nullptr)
            AudioComponentInstanceNew (audioComponent, &viewComponent);

        return viewComponent;
    }

    void closeViewComponent()
    {
        if (viewComponent != nullptr)
        {
            JUCE_AU_LOG ("Closing AU GUI: " + plugin.getName());

            AudioComponentInstanceDispose (viewComponent);
            viewComponent = nullptr;
        }
    }

private:
    //==============================================================================
    AudioUnitPluginInstance& plugin;
    AudioComponent audioComponent;
    AudioUnitCarbonView viewComponent;

    //==============================================================================
    class InnerWrapperComponent   : public CarbonViewWrapperComponent
    {
    public:
        InnerWrapperComponent (AudioUnitPluginWindowCarbon& w)  : owner (w)  {}

        ~InnerWrapperComponent()
        {
            deleteWindow();
        }

        HIViewRef attachView (WindowRef windowRef, HIViewRef rootView) override
        {
            JUCE_AU_LOG ("Opening AU GUI: " + owner.plugin.getName());

            AudioUnitCarbonView carbonView = owner.getViewComponent();

            if (carbonView == 0)
                return 0;

            Float32Point pos = { 0, 0 };
            Float32Point size = { 250, 200 };
            HIViewRef pluginView = 0;

            AudioUnitCarbonViewCreate (carbonView, owner.getAudioUnit(), windowRef, rootView,
                                       &pos, &size, (ControlRef*) &pluginView);

            return pluginView;
        }

        void removeView (HIViewRef) override
        {
            owner.closeViewComponent();
        }

    private:
        AudioUnitPluginWindowCarbon& owner;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (InnerWrapperComponent)
    };

    friend class InnerWrapperComponent;
    ScopedPointer<InnerWrapperComponent> innerWrapper;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioUnitPluginWindowCarbon)
};

#endif

//==============================================================================
AudioProcessorEditor* AudioUnitPluginInstance::createEditor()
{
    ScopedPointer<AudioProcessorEditor> w (new AudioUnitPluginWindowCocoa (*this, false));

    if (! static_cast<AudioUnitPluginWindowCocoa*> (w.get())->isValid())
        w = nullptr;

   #if JUCE_SUPPORT_CARBON
    if (w == nullptr)
    {
        w = new AudioUnitPluginWindowCarbon (*this);

        if (! static_cast<AudioUnitPluginWindowCarbon*> (w.get())->isValid())
            w = nullptr;
    }
   #endif

    if (w == nullptr)
        w = new AudioUnitPluginWindowCocoa (*this, true); // use AUGenericView as a fallback

    return w.release();
}


//==============================================================================
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
    desc.uid = 0;

    if (MessageManager::getInstance()->isThisTheMessageThread()
          && requiresUnblockedMessageThreadDuringCreation (desc))
        return;

    try
    {
        ScopedPointer<AudioPluginInstance> createdInstance (createInstanceFromDescription (desc, 44100.0, 512));

        if (AudioUnitPluginInstance* auInstance = dynamic_cast<AudioUnitPluginInstance*> (createdInstance.get()))
            results.add (new PluginDescription (auInstance->getPluginDescription()));
    }
    catch (...)
    {
        // crashed while loading...
    }
}

void AudioUnitPluginFormat::createPluginInstance (const PluginDescription& desc,
                                                  double rate,
                                                  int blockSize,
                                                  void* userData,
                                                  void (*callback) (void*, AudioPluginInstance*, const String&))
{
    using namespace AudioUnitFormatHelpers;

    if (fileMightContainThisPluginType (desc.fileOrIdentifier))
    {

        String pluginName, version, manufacturer;
        AudioComponentDescription componentDesc;
        AudioComponent auComponent;
        String errMessage = NEEDS_TRANS ("Cannot find AudioUnit from description");

        if ((! getComponentDescFromIdentifier (desc.fileOrIdentifier, componentDesc, pluginName, version, manufacturer))
              && (! getComponentDescFromFile (desc.fileOrIdentifier, componentDesc, pluginName, version, manufacturer)))
        {
            callback (userData, nullptr, errMessage);
            return;
        }

        if ((auComponent = AudioComponentFindNext (0, &componentDesc)) == nullptr)
        {
            callback (userData, nullptr, errMessage);
            return;
        }

        if (AudioComponentGetDescription (auComponent, &componentDesc) != noErr)
        {
            callback (userData, nullptr, errMessage);
            return;
        }

        struct AUAsyncInitializationCallback
        {
           #if JUCE_SUPPORTS_AUv3
            typedef void (^AUCompletionCallbackBlock)(AudioComponentInstance, OSStatus);
           #endif

            AUAsyncInitializationCallback (double inSampleRate, int inFramesPerBuffer,
                                           void* inUserData, void (*inOriginalCallback) (void*, AudioPluginInstance*, const String&))
            : sampleRate (inSampleRate), framesPerBuffer (inFramesPerBuffer),
            passUserData (inUserData), originalCallback (inOriginalCallback)
            {
               #if JUCE_SUPPORTS_AUv3
                block = CreateObjCBlock (this, &AUAsyncInitializationCallback::completion);
               #endif
            }

           #if JUCE_SUPPORTS_AUv3
            AUCompletionCallbackBlock getBlock() noexcept       { return block; }
           #endif

            void completion (AudioComponentInstance audioUnit, OSStatus err)
            {
                if (err == noErr)
                {
                    ScopedPointer<AudioUnitPluginInstance> instance (new AudioUnitPluginInstance (audioUnit));

                    if (instance->initialise (sampleRate, framesPerBuffer))
                        originalCallback (passUserData, instance.release(), StringRef());
                    else
                        originalCallback (passUserData, nullptr,
                                          NEEDS_TRANS ("Unable to initialise the AudioUnit plug-in"));
                }
                else
                {
                    String errMsg = NEEDS_TRANS ("An OS error occurred during initialisation of the plug-in (XXX)");
                    originalCallback (passUserData, nullptr, errMsg.replace ("XXX", String (err)));
                }

                delete this;
            }

            double sampleRate;
            int framesPerBuffer;
            void* passUserData;
            void (*originalCallback) (void*, AudioPluginInstance*, const String&);

           #if JUCE_SUPPORTS_AUv3
            ObjCBlock<AUCompletionCallbackBlock> block;
           #endif
        };

        AUAsyncInitializationCallback* callbackBlock
            = new AUAsyncInitializationCallback (rate, blockSize, userData, callback);

       #if JUCE_SUPPORTS_AUv3
        //==============================================================================
        bool isAUv3 = ((componentDesc.componentFlags & kAudioComponentFlag_IsV3AudioUnit) != 0);

        if (isAUv3)
        {
            AudioComponentInstantiate (auComponent, kAudioComponentInstantiation_LoadOutOfProcess,
                                       callbackBlock->getBlock());

            return;
        }
       #endif // JUCE_SUPPORTS_AUv3

        AudioComponentInstance audioUnit;
        OSStatus err = AudioComponentInstanceNew(auComponent, &audioUnit);
        callbackBlock->completion (err != noErr ? nullptr : audioUnit, err);
    }
    else
    {
        callback (userData, nullptr, NEEDS_TRANS ("Plug-in description is not an AudioUnit plug-in"));
    }
}

bool AudioUnitPluginFormat::requiresUnblockedMessageThreadDuringCreation (const PluginDescription& desc) const noexcept
{
   #if JUCE_SUPPORTS_AUv3
    String pluginName, version, manufacturer;
    AudioComponentDescription componentDesc;

    if (AudioUnitFormatHelpers::getComponentDescFromIdentifier (desc.fileOrIdentifier, componentDesc,
                                                                pluginName, version, manufacturer)
           || AudioUnitFormatHelpers::getComponentDescFromFile (desc.fileOrIdentifier, componentDesc,
                                                                pluginName, version, manufacturer))
    {
        if (AudioComponent auComp = AudioComponentFindNext (0, &componentDesc))
            if (AudioComponentGetDescription (auComp, &componentDesc) == noErr)
                return ((componentDesc.componentFlags & kAudioComponentFlag_IsV3AudioUnit) != 0);
    }
   #else
    ignoreUnused (desc);
   #endif

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
             || desc.componentType == kAudioUnitType_Panner)
        {
            ignoreUnused (allowPluginsWhichRequireAsynchronousInstantiation);

          #if JUCE_SUPPORTS_AUv3
            bool isAUv3 = ((desc.componentFlags & kAudioComponentFlag_IsV3AudioUnit) != 0);

            if (allowPluginsWhichRequireAsynchronousInstantiation || ! isAUv3)
          #endif
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

    const File f (File::createFileWithoutCheckingPath (fileOrIdentifier));

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
    return FileSearchPath();
}

#undef JUCE_AU_LOG

#endif

/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

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

#include "../../../core/juce_TargetPlatform.h"
#include "../../../../juce_Config.h"

#if JUCE_PLUGINHOST_AU && ! (JUCE_LINUX || JUCE_WINDOWS)

#include <AudioUnit/AudioUnit.h>
#include <AudioUnit/AUCocoaUIView.h>
#include <CoreAudioKit/AUGenericView.h>

#if JUCE_SUPPORT_CARBON
#include <AudioToolbox/AudioUnitUtilities.h>
#include <AudioUnit/AudioUnitCarbonView.h>
#endif

#include "../../../core/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_AudioUnitPluginFormat.h"
#include "../juce_PluginDescription.h"
#include "../../../threads/juce_CriticalSection.h"
#include "../../../events/juce_Timer.h"
#include "../../../core/juce_PlatformUtilities.h"
#include "../../../gui/components/layout/juce_ComponentMovementWatcher.h"
#include "../../../gui/components/windows/juce_ComponentPeer.h"
#include "../../../gui/components/special/juce_NSViewComponent.h"
#if JUCE_MAC && JUCE_SUPPORT_CARBON
#include "../../../native/mac/juce_mac_CarbonViewWrapperComponent.h"
#endif

#if JUCE_MAC

// Change this to disable logging of various activities
#ifndef AU_LOGGING
  #define AU_LOGGING 1
#endif

#if AU_LOGGING
 #define log(a) Logger::writeToLog(a);
#else
 #define log(a)
#endif

namespace AudioUnitFormatHelpers
{
    static int insideCallback = 0;

    const String osTypeToString (OSType type)
    {
        const juce_wchar s[4] = { (juce_wchar) ((type >> 24) & 0xff),
                                  (juce_wchar) ((type >> 16) & 0xff),
                                  (juce_wchar) ((type >> 8) & 0xff),
                                  (juce_wchar) (type & 0xff) };
        return String (s, 4);
    }

    OSType stringToOSType (const String& s1)
    {
        const String s (s1 + "    ");

        return (((OSType) (unsigned char) s[0]) << 24)
             | (((OSType) (unsigned char) s[1]) << 16)
             | (((OSType) (unsigned char) s[2]) << 8)
             |  ((OSType) (unsigned char) s[3]);
    }

    static const char* auIdentifierPrefix = "AudioUnit:";

    const String createAUPluginIdentifier (const ComponentDescription& desc)
    {
        jassert (osTypeToString ('abcd') == "abcd"); // agh, must have got the endianness wrong..
        jassert (stringToOSType ("abcd") == (OSType) 'abcd'); // ditto

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

    void getAUDetails (ComponentRecord* comp, String& name, String& manufacturer)
    {
        Handle componentNameHandle = NewHandle (sizeof (void*));
        Handle componentInfoHandle = NewHandle (sizeof (void*));

        if (componentNameHandle != 0 && componentInfoHandle != 0)
        {
            ComponentDescription desc;

            if (GetComponentInfo (comp, &desc, componentNameHandle, componentInfoHandle, 0) == noErr)
            {
                ConstStr255Param nameString = (ConstStr255Param) (*componentNameHandle);
                ConstStr255Param infoString = (ConstStr255Param) (*componentInfoHandle);

                if (nameString != 0 && nameString[0] != 0)
                {
                    const String all ((const char*) nameString + 1, nameString[0]);
                    DBG ("name: "+ all);

                    manufacturer = all.upToFirstOccurrenceOf (":", false, false).trim();
                    name = all.fromFirstOccurrenceOf (":", false, false).trim();
                }

                if (infoString != 0 && infoString[0] != 0)
                {
                    DBG ("info: " + String ((const char*) infoString + 1, infoString[0]));
                }

                if (name.isEmpty())
                    name = "<Unknown>";
            }

            DisposeHandle (componentNameHandle);
            DisposeHandle (componentInfoHandle);
        }
    }

    bool getComponentDescFromIdentifier (const String& fileOrIdentifier, ComponentDescription& desc,
                                         String& name, String& version, String& manufacturer)
    {
        zerostruct (desc);

        if (fileOrIdentifier.startsWithIgnoreCase (auIdentifierPrefix))
        {
            String s (fileOrIdentifier.substring (jmax (fileOrIdentifier.lastIndexOfChar (':'),
                                                        fileOrIdentifier.lastIndexOfChar ('/')) + 1));

            StringArray tokens;
            tokens.addTokens (s, ",", String::empty);
            tokens.trim();
            tokens.removeEmptyStrings();

            if (tokens.size() == 3)
            {
                desc.componentType = stringToOSType (tokens[0]);
                desc.componentSubType = stringToOSType (tokens[1]);
                desc.componentManufacturer = stringToOSType (tokens[2]);

                ComponentRecord* comp = FindNextComponent (0, &desc);

                if (comp != 0)
                {
                    getAUDetails (comp, name, manufacturer);
                    return true;
                }
            }
        }

        return false;
    }
}

//==============================================================================
class AudioUnitPluginWindowCarbon;
class AudioUnitPluginWindowCocoa;

//==============================================================================
class AudioUnitPluginInstance     : public AudioPluginInstance
{
public:
    //==============================================================================
    ~AudioUnitPluginInstance();

    void initialise();

    //==============================================================================
    // AudioPluginInstance methods:

    void fillInPluginDescription (PluginDescription& desc) const
    {
        desc.name = pluginName;
        desc.descriptiveName = pluginName;
        desc.fileOrIdentifier = AudioUnitFormatHelpers::createAUPluginIdentifier (componentDesc);
        desc.uid = ((int) componentDesc.componentType)
                    ^ ((int) componentDesc.componentSubType)
                    ^ ((int) componentDesc.componentManufacturer);
        desc.lastFileModTime = Time();
        desc.pluginFormatName = "AudioUnit";
        desc.category = getCategory();
        desc.manufacturerName = manufacturer;
        desc.version = version;
        desc.numInputChannels = getNumInputChannels();
        desc.numOutputChannels = getNumOutputChannels();
        desc.isInstrument = (componentDesc.componentType == kAudioUnitType_MusicDevice);
    }

    void* getPlatformSpecificData()             { return audioUnit; }
    const String getName() const                { return pluginName; }
    bool acceptsMidi() const                    { return wantsMidiMessages; }
    bool producesMidi() const                   { return false; }

    //==============================================================================
    // AudioProcessor methods:

    void prepareToPlay (double sampleRate, int estimatedSamplesPerBlock);
    void releaseResources();
    void processBlock (AudioSampleBuffer& buffer,
                       MidiBuffer& midiMessages);

    bool hasEditor() const;
    AudioProcessorEditor* createEditor();

    const String getInputChannelName (int index) const;
    bool isInputChannelStereoPair (int index) const;

    const String getOutputChannelName (int index) const;
    bool isOutputChannelStereoPair (int index) const;

    //==============================================================================
    int getNumParameters();
    float getParameter (int index);
    void setParameter (int index, float newValue);
    const String getParameterName (int index);
    const String getParameterText (int index);
    bool isParameterAutomatable (int index) const;

    //==============================================================================
    int getNumPrograms();
    int getCurrentProgram();
    void setCurrentProgram (int index);
    const String getProgramName (int index);
    void changeProgramName (int index, const String& newName);

    //==============================================================================
    void getStateInformation (MemoryBlock& destData);
    void getCurrentProgramStateInformation (MemoryBlock& destData);
    void setStateInformation (const void* data, int sizeInBytes);
    void setCurrentProgramStateInformation (const void* data, int sizeInBytes);

    void refreshParameterListFromPlugin();

private:
    //==============================================================================
    friend class AudioUnitPluginWindowCarbon;
    friend class AudioUnitPluginWindowCocoa;
    friend class AudioUnitPluginFormat;

    ComponentDescription componentDesc;
    String pluginName, manufacturer, version;
    String fileOrIdentifier;
    CriticalSection lock;
    bool wantsMidiMessages, wasPlaying, prepared;

    HeapBlock <AudioBufferList> outputBufferList;
    AudioTimeStamp timeStamp;
    AudioSampleBuffer* currentBuffer;

    AudioUnit audioUnit;
    Array <int> parameterIds;

    //==============================================================================
    bool getComponentDescFromFile (const String& fileOrIdentifier);
    void setPluginCallbacks();

    //==============================================================================
    OSStatus renderGetInput (AudioUnitRenderActionFlags* ioActionFlags,
                             const AudioTimeStamp* inTimeStamp,
                             UInt32 inBusNumber,
                             UInt32 inNumberFrames,
                             AudioBufferList* ioData) const;

    static OSStatus renderGetInputCallback (void* inRefCon,
                                            AudioUnitRenderActionFlags* ioActionFlags,
                                            const AudioTimeStamp* inTimeStamp,
                                            UInt32 inBusNumber,
                                            UInt32 inNumberFrames,
                                            AudioBufferList* ioData)
    {
        return ((AudioUnitPluginInstance*) inRefCon)
                    ->renderGetInput (ioActionFlags, inTimeStamp, inBusNumber, inNumberFrames, ioData);
    }

    OSStatus getBeatAndTempo (Float64* outCurrentBeat, Float64* outCurrentTempo) const;
    OSStatus getMusicalTimeLocation (UInt32* outDeltaSampleOffsetToNextBeat, Float32* outTimeSig_Numerator,
                                     UInt32* outTimeSig_Denominator, Float64* outCurrentMeasureDownBeat) const;
    OSStatus getTransportState (Boolean* outIsPlaying, Boolean* outTransportStateChanged,
                                Float64* outCurrentSampleInTimeLine, Boolean* outIsCycling,
                                Float64* outCycleStartBeat, Float64* outCycleEndBeat);

    static OSStatus getBeatAndTempoCallback (void* inHostUserData, Float64* outCurrentBeat, Float64* outCurrentTempo)
    {
        return ((AudioUnitPluginInstance*) inHostUserData)->getBeatAndTempo (outCurrentBeat, outCurrentTempo);
    }

    static OSStatus getMusicalTimeLocationCallback (void* inHostUserData, UInt32* outDeltaSampleOffsetToNextBeat,
                                                    Float32* outTimeSig_Numerator, UInt32* outTimeSig_Denominator,
                                                    Float64* outCurrentMeasureDownBeat)
    {
        return ((AudioUnitPluginInstance*) inHostUserData)
                    ->getMusicalTimeLocation (outDeltaSampleOffsetToNextBeat, outTimeSig_Numerator,
                                              outTimeSig_Denominator, outCurrentMeasureDownBeat);
    }

    static OSStatus getTransportStateCallback (void* inHostUserData, Boolean* outIsPlaying, Boolean* outTransportStateChanged,
                                               Float64* outCurrentSampleInTimeLine, Boolean* outIsCycling,
                                               Float64* outCycleStartBeat, Float64* outCycleEndBeat)
    {
        return ((AudioUnitPluginInstance*) inHostUserData)
                    ->getTransportState (outIsPlaying, outTransportStateChanged,
                                         outCurrentSampleInTimeLine, outIsCycling,
                                         outCycleStartBeat, outCycleEndBeat);
    }

    //==============================================================================
    void getNumChannels (int& numIns, int& numOuts)
    {
        numIns = 0;
        numOuts = 0;

        AUChannelInfo supportedChannels [128];
        UInt32 supportedChannelsSize = sizeof (supportedChannels);

        if (AudioUnitGetProperty (audioUnit, kAudioUnitProperty_SupportedNumChannels, kAudioUnitScope_Global,
                                  0, supportedChannels, &supportedChannelsSize) == noErr
            && supportedChannelsSize > 0)
        {
            int explicitNumIns = 0;
            int explicitNumOuts = 0;
            int maximumNumIns = 0;
            int maximumNumOuts = 0;

            for (int i = 0; i < supportedChannelsSize / sizeof (AUChannelInfo); ++i)
            {
                const int inChannels = (int) supportedChannels[i].inChannels;
                const int outChannels = (int) supportedChannels[i].outChannels;

                if (inChannels < 0)
                    maximumNumIns = jmin (maximumNumIns, inChannels);
                else
                    explicitNumIns = jmax (explicitNumIns, inChannels);

                if (outChannels < 0)
                    maximumNumOuts = jmin (maximumNumOuts, outChannels);
                else
                    explicitNumOuts = jmax (explicitNumOuts, outChannels);
            }

            if ((maximumNumIns == -1 && maximumNumOuts == -1)  // (special meaning: any number of ins/outs, as long as they match)
                || (maximumNumIns == -2 && maximumNumOuts == -1) // (special meaning: any number of ins/outs, even if they don't match)
                || (maximumNumIns == -1 && maximumNumOuts == -2))
            {
                numIns = numOuts = 2;
            }
            else
            {
                numIns = explicitNumIns;
                numOuts = explicitNumOuts;

                if (maximumNumIns == -1 || (maximumNumIns < 0 && explicitNumIns <= -maximumNumIns))
                    numIns = 2;

                if (maximumNumOuts == -1 || (maximumNumOuts < 0 && explicitNumOuts <= -maximumNumOuts))
                    numOuts = 2;
            }
        }
        else
        {
            // (this really means the plugin will take any number of ins/outs as long
            // as they are the same)
            numIns = numOuts = 2;
        }
    }

    const String getCategory() const;

    //==============================================================================
    AudioUnitPluginInstance (const String& fileOrIdentifier);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioUnitPluginInstance);
};

//==============================================================================
AudioUnitPluginInstance::AudioUnitPluginInstance (const String& fileOrIdentifier)
    : fileOrIdentifier (fileOrIdentifier),
      wantsMidiMessages (false), wasPlaying (false), prepared (false),
      currentBuffer (nullptr),
      audioUnit (0)
{
    using namespace AudioUnitFormatHelpers;

    try
    {
        ++insideCallback;

        log ("Opening AU: " + fileOrIdentifier);

        if (getComponentDescFromFile (fileOrIdentifier))
        {
            ComponentRecord* const comp = FindNextComponent (0, &componentDesc);

            if (comp != 0)
            {
                audioUnit = (AudioUnit) OpenComponent (comp);

                wantsMidiMessages = componentDesc.componentType == kAudioUnitType_MusicDevice
                    || componentDesc.componentType == kAudioUnitType_MusicEffect;
            }
        }

        --insideCallback;
    }
    catch (...)
    {
        --insideCallback;
    }
}

AudioUnitPluginInstance::~AudioUnitPluginInstance()
{
    const ScopedLock sl (lock);

    jassert (AudioUnitFormatHelpers::insideCallback == 0);

    if (audioUnit != 0)
    {
        AudioUnitUninitialize (audioUnit);
        CloseComponent (audioUnit);
        audioUnit = 0;
    }
}

bool AudioUnitPluginInstance::getComponentDescFromFile (const String& fileOrIdentifier)
{
    zerostruct (componentDesc);

    if (AudioUnitFormatHelpers::getComponentDescFromIdentifier (fileOrIdentifier, componentDesc, pluginName, version, manufacturer))
        return true;

    const File file (fileOrIdentifier);
    if (! file.hasFileExtension (".component"))
        return false;

    const char* const utf8 = fileOrIdentifier.toUTF8();
    CFURLRef url = CFURLCreateFromFileSystemRepresentation (0, (const UInt8*) utf8,
                                                            strlen (utf8), file.isDirectory());
    if (url != 0)
    {
        CFBundleRef bundleRef = CFBundleCreate (kCFAllocatorDefault, url);
        CFRelease (url);

        if (bundleRef != 0)
        {
            CFTypeRef name = CFBundleGetValueForInfoDictionaryKey (bundleRef, CFSTR("CFBundleName"));

            if (name != 0 && CFGetTypeID (name) == CFStringGetTypeID())
                pluginName = PlatformUtilities::cfStringToJuceString ((CFStringRef) name);

            if (pluginName.isEmpty())
                pluginName = file.getFileNameWithoutExtension();

            CFTypeRef versionString = CFBundleGetValueForInfoDictionaryKey (bundleRef, CFSTR("CFBundleVersion"));

            if (versionString != 0 && CFGetTypeID (versionString) == CFStringGetTypeID())
                version = PlatformUtilities::cfStringToJuceString ((CFStringRef) versionString);

            CFTypeRef manuString = CFBundleGetValueForInfoDictionaryKey (bundleRef, CFSTR("CFBundleGetInfoString"));

            if (manuString != 0 && CFGetTypeID (manuString) == CFStringGetTypeID())
                manufacturer = PlatformUtilities::cfStringToJuceString ((CFStringRef) manuString);

            short resFileId = CFBundleOpenBundleResourceMap (bundleRef);
            UseResFile (resFileId);

            for (int i = 1; i <= Count1Resources ('thng'); ++i)
            {
                Handle h = Get1IndResource ('thng', i);

                if (h != 0)
                {
                    HLock (h);
                    const uint32* const types = (const uint32*) *h;

                    if (types[0] == kAudioUnitType_MusicDevice
                         || types[0] == kAudioUnitType_MusicEffect
                         || types[0] == kAudioUnitType_Effect
                         || types[0] == kAudioUnitType_Generator
                         || types[0] == kAudioUnitType_Panner)
                    {
                        componentDesc.componentType = types[0];
                        componentDesc.componentSubType = types[1];
                        componentDesc.componentManufacturer = types[2];
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

    return componentDesc.componentType != 0 && componentDesc.componentSubType != 0;
}

//==============================================================================
void AudioUnitPluginInstance::initialise()
{
    refreshParameterListFromPlugin();
    setPluginCallbacks();

    int numIns, numOuts;
    getNumChannels (numIns, numOuts);
    setPlayConfigDetails (numIns, numOuts, 0, 0);
    setLatencySamples (0);
}

void AudioUnitPluginInstance::refreshParameterListFromPlugin()
{
    parameterIds.clear();

    if (audioUnit != 0)
    {
        UInt32 paramListSize = 0;
        AudioUnitGetProperty (audioUnit, kAudioUnitProperty_ParameterList, kAudioUnitScope_Global,
                              0, 0, &paramListSize);

        if (paramListSize > 0)
        {
            parameterIds.insertMultiple (0, 0, paramListSize / sizeof (int));

            AudioUnitGetProperty (audioUnit, kAudioUnitProperty_ParameterList, kAudioUnitScope_Global,
                                  0, parameterIds.getRawDataPointer(), &paramListSize);
        }
    }
}

void AudioUnitPluginInstance::setPluginCallbacks()
{
    if (audioUnit != 0)
    {
        {
            AURenderCallbackStruct info = { 0 };
            info.inputProcRefCon = this;
            info.inputProc = renderGetInputCallback;

            AudioUnitSetProperty (audioUnit, kAudioUnitProperty_SetRenderCallback, kAudioUnitScope_Input,
                                  0, &info, sizeof (info));
        }

        {
            HostCallbackInfo info = { 0 };
            info.hostUserData = this;
            info.beatAndTempoProc = getBeatAndTempoCallback;
            info.musicalTimeLocationProc = getMusicalTimeLocationCallback;
            info.transportStateProc = getTransportStateCallback;

            AudioUnitSetProperty (audioUnit, kAudioUnitProperty_HostCallbacks, kAudioUnitScope_Global,
                                  0, &info, sizeof (info));
        }
    }
}


//==============================================================================
void AudioUnitPluginInstance::prepareToPlay (double sampleRate_,
                                             int samplesPerBlockExpected)
{
    if (audioUnit != 0)
    {
        releaseResources();

        Float64 sampleRateIn = 0, sampleRateOut = 0;
        UInt32 sampleRateSize = sizeof (sampleRateIn);
        AudioUnitGetProperty (audioUnit, kAudioUnitProperty_SampleRate, kAudioUnitScope_Input, 0, &sampleRateIn, &sampleRateSize);
        AudioUnitGetProperty (audioUnit, kAudioUnitProperty_SampleRate, kAudioUnitScope_Output, 0, &sampleRateOut, &sampleRateSize);

        if (sampleRateIn != sampleRate_ || sampleRateOut != sampleRate_)
        {
            Float64 sr = sampleRate_;
            AudioUnitSetProperty (audioUnit, kAudioUnitProperty_SampleRate, kAudioUnitScope_Input, 0, &sr, sizeof (Float64));
            AudioUnitSetProperty (audioUnit, kAudioUnitProperty_SampleRate, kAudioUnitScope_Output, 0, &sr, sizeof (Float64));
        }

        int numIns, numOuts;
        getNumChannels (numIns, numOuts);
        setPlayConfigDetails (numIns, numOuts, sampleRate_, samplesPerBlockExpected);

        Float64 latencySecs = 0.0;
        UInt32 latencySize = sizeof (latencySecs);
        AudioUnitGetProperty (audioUnit, kAudioUnitProperty_Latency, kAudioUnitScope_Global,
                              0, &latencySecs, &latencySize);

        setLatencySamples (roundToInt (latencySecs * sampleRate_));

        AudioUnitReset (audioUnit, kAudioUnitScope_Input, 0);
        AudioUnitReset (audioUnit, kAudioUnitScope_Output, 0);
        AudioUnitReset (audioUnit, kAudioUnitScope_Global, 0);

        {
            AudioStreamBasicDescription stream = { 0 };
            stream.mSampleRate = sampleRate_;
            stream.mFormatID = kAudioFormatLinearPCM;
            stream.mFormatFlags = kAudioFormatFlagsNativeFloatPacked | kAudioFormatFlagIsNonInterleaved;
            stream.mFramesPerPacket = 1;
            stream.mBytesPerPacket = 4;
            stream.mBytesPerFrame = 4;
            stream.mBitsPerChannel = 32;
            stream.mChannelsPerFrame = numIns;

            OSStatus err = AudioUnitSetProperty (audioUnit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Input,
                                                 0, &stream, sizeof (stream));

            stream.mChannelsPerFrame = numOuts;

            err = AudioUnitSetProperty (audioUnit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Output,
                                        0, &stream, sizeof (stream));
        }

        outputBufferList.calloc (sizeof (AudioBufferList) + sizeof (AudioBuffer) * (numOuts + 1), 1);
        outputBufferList->mNumberBuffers = numOuts;

        for (int i = numOuts; --i >= 0;)
            outputBufferList->mBuffers[i].mNumberChannels = 1;

        zerostruct (timeStamp);
        timeStamp.mSampleTime = 0;
        timeStamp.mHostTime = AudioGetCurrentHostTime();
        timeStamp.mFlags = kAudioTimeStampSampleTimeValid | kAudioTimeStampHostTimeValid;

        currentBuffer = 0;
        wasPlaying = false;

        prepared = (AudioUnitInitialize (audioUnit) == noErr);
    }
}

void AudioUnitPluginInstance::releaseResources()
{
    if (prepared)
    {
        AudioUnitUninitialize (audioUnit);

        AudioUnitReset (audioUnit, kAudioUnitScope_Input, 0);
        AudioUnitReset (audioUnit, kAudioUnitScope_Output, 0);
        AudioUnitReset (audioUnit, kAudioUnitScope_Global, 0);

        outputBufferList.free();
        currentBuffer = 0;
        prepared = false;
    }
}

OSStatus AudioUnitPluginInstance::renderGetInput (AudioUnitRenderActionFlags* ioActionFlags,
                                                  const AudioTimeStamp* inTimeStamp,
                                                  UInt32 inBusNumber,
                                                  UInt32 inNumberFrames,
                                                  AudioBufferList* ioData) const
{
    if (inBusNumber == 0
         && currentBuffer != 0)
    {
        jassert (inNumberFrames == currentBuffer->getNumSamples()); // if this ever happens, might need to add extra handling

        for (int i = 0; i < ioData->mNumberBuffers; ++i)
        {
            if (i < currentBuffer->getNumChannels())
            {
                memcpy (ioData->mBuffers[i].mData,
                        currentBuffer->getSampleData (i, 0),
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

void AudioUnitPluginInstance::processBlock (AudioSampleBuffer& buffer,
                                            MidiBuffer& midiMessages)
{
    const int numSamples = buffer.getNumSamples();

    if (prepared)
    {
        AudioUnitRenderActionFlags flags = 0;

        timeStamp.mHostTime = AudioGetCurrentHostTime();

        for (int i = getNumOutputChannels(); --i >= 0;)
        {
            outputBufferList->mBuffers[i].mDataByteSize = sizeof (float) * numSamples;
            outputBufferList->mBuffers[i].mData = buffer.getSampleData (i, 0);
        }

        currentBuffer = &buffer;

        if (wantsMidiMessages)
        {
            const uint8* midiEventData;
            int midiEventSize, midiEventPosition;
            MidiBuffer::Iterator i (midiMessages);

            while (i.getNextEvent (midiEventData, midiEventSize, midiEventPosition))
            {
                if (midiEventSize <= 3)
                    MusicDeviceMIDIEvent (audioUnit,
                                          midiEventData[0], midiEventData[1], midiEventData[2],
                                          midiEventPosition);
                else
                    MusicDeviceSysEx (audioUnit, midiEventData, midiEventSize);
            }

            midiMessages.clear();
        }

        AudioUnitRender (audioUnit, &flags, &timeStamp,
                         0, numSamples, outputBufferList);

        timeStamp.mSampleTime += numSamples;
    }
    else
    {
        // Plugin not working correctly, so just bypass..
        for (int i = getNumInputChannels(); i < getNumOutputChannels(); ++i)
            buffer.clear (i, 0, buffer.getNumSamples());
    }
}

//==============================================================================
OSStatus AudioUnitPluginInstance::getBeatAndTempo (Float64* outCurrentBeat, Float64* outCurrentTempo) const
{
    AudioPlayHead* const ph = getPlayHead();
    AudioPlayHead::CurrentPositionInfo result;

    if (ph != nullptr && ph->getCurrentPosition (result))
    {
        if (outCurrentBeat != nullptr)
            *outCurrentBeat = result.ppqPosition;

        if (outCurrentTempo != nullptr)
            *outCurrentTempo = result.bpm;
    }
    else
    {
        if (outCurrentBeat != nullptr)
            *outCurrentBeat = 0;

        if (outCurrentTempo != nullptr)
            *outCurrentTempo = 120.0;
    }

    return noErr;
}

OSStatus AudioUnitPluginInstance::getMusicalTimeLocation (UInt32* outDeltaSampleOffsetToNextBeat,
                                                          Float32* outTimeSig_Numerator,
                                                          UInt32* outTimeSig_Denominator,
                                                          Float64* outCurrentMeasureDownBeat) const
{
    AudioPlayHead* const ph = getPlayHead();
    AudioPlayHead::CurrentPositionInfo result;

    if (ph != nullptr && ph->getCurrentPosition (result))
    {
        if (outTimeSig_Numerator != nullptr)
            *outTimeSig_Numerator = result.timeSigNumerator;

        if (outTimeSig_Denominator != nullptr)
            *outTimeSig_Denominator = result.timeSigDenominator;

        if (outDeltaSampleOffsetToNextBeat != nullptr)
            *outDeltaSampleOffsetToNextBeat = 0; //xxx

        if (outCurrentMeasureDownBeat != nullptr)
            *outCurrentMeasureDownBeat = result.ppqPositionOfLastBarStart; //xxx wrong
    }
    else
    {
        if (outDeltaSampleOffsetToNextBeat != nullptr)
            *outDeltaSampleOffsetToNextBeat = 0;

        if (outTimeSig_Numerator != nullptr)
            *outTimeSig_Numerator = 4;

        if (outTimeSig_Denominator != nullptr)
            *outTimeSig_Denominator = 4;

        if (outCurrentMeasureDownBeat != nullptr)
            *outCurrentMeasureDownBeat = 0;
    }

    return noErr;
}

OSStatus AudioUnitPluginInstance::getTransportState (Boolean* outIsPlaying,
                                                     Boolean* outTransportStateChanged,
                                                     Float64* outCurrentSampleInTimeLine,
                                                     Boolean* outIsCycling,
                                                     Float64* outCycleStartBeat,
                                                     Float64* outCycleEndBeat)
{
    AudioPlayHead* const ph = getPlayHead();
    AudioPlayHead::CurrentPositionInfo result;

    if (ph != nullptr && ph->getCurrentPosition (result))
    {
        if (outIsPlaying != nullptr)
            *outIsPlaying = result.isPlaying;

        if (outTransportStateChanged != nullptr)
        {
            *outTransportStateChanged = result.isPlaying != wasPlaying;
            wasPlaying = result.isPlaying;
        }

        if (outCurrentSampleInTimeLine != nullptr)
            *outCurrentSampleInTimeLine = roundToInt (result.timeInSeconds * getSampleRate());

        if (outIsCycling != nullptr)
            *outIsCycling = false;

        if (outCycleStartBeat != nullptr)
            *outCycleStartBeat = 0;

        if (outCycleEndBeat != nullptr)
            *outCycleEndBeat = 0;
    }
    else
    {
        if (outIsPlaying != nullptr)
            *outIsPlaying = false;

        if (outTransportStateChanged != nullptr)
            *outTransportStateChanged = false;

        if (outCurrentSampleInTimeLine != nullptr)
            *outCurrentSampleInTimeLine = 0;

        if (outIsCycling != nullptr)
            *outIsCycling = false;

        if (outCycleStartBeat != nullptr)
            *outCycleStartBeat = 0;

        if (outCycleEndBeat != nullptr)
            *outCycleEndBeat = 0;
    }

    return noErr;
}


//==============================================================================
class AudioUnitPluginWindowCocoa    : public AudioProcessorEditor,
                                      public Timer
{
public:
    AudioUnitPluginWindowCocoa (AudioUnitPluginInstance& plugin_, const bool createGenericViewIfNeeded)
        : AudioProcessorEditor (&plugin_),
          plugin (plugin_)
    {
        addAndMakeVisible (&wrapper);

        setOpaque (true);
        setVisible (true);
        setSize (100, 100);

        createView (createGenericViewIfNeeded);
    }

    ~AudioUnitPluginWindowCocoa()
    {
        const bool wasValid = isValid();

        wrapper.setView (0);

        if (wasValid)
            plugin.editorBeingDeleted (this);
    }

    bool isValid() const        { return wrapper.getView() != 0; }

    void paint (Graphics& g)
    {
        g.fillAll (Colours::white);
    }

    void resized()
    {
        wrapper.setSize (getWidth(), getHeight());
    }

    void timerCallback()
    {
        wrapper.resizeToFitView();
        startTimer (jmin (713, getTimerInterval() + 51));
    }

    void childBoundsChanged (Component* child)
    {
        setSize (wrapper.getWidth(), wrapper.getHeight());
        startTimer (70);
    }

private:
    AudioUnitPluginInstance& plugin;
    NSViewComponent wrapper;

    bool createView (const bool createGenericViewIfNeeded)
    {
        NSView* pluginView = nil;

        UInt32 dataSize = 0;
        Boolean isWritable = false;

        AudioUnitInitialize (plugin.audioUnit);

        if (AudioUnitGetPropertyInfo (plugin.audioUnit, kAudioUnitProperty_CocoaUI, kAudioUnitScope_Global,
                                      0, &dataSize, &isWritable) == noErr
             && dataSize != 0
             && AudioUnitGetPropertyInfo (plugin.audioUnit, kAudioUnitProperty_CocoaUI, kAudioUnitScope_Global,
                                          0, &dataSize, &isWritable) == noErr)
        {
            HeapBlock <AudioUnitCocoaViewInfo> info;
            info.calloc (dataSize, 1);

            if (AudioUnitGetProperty (plugin.audioUnit, kAudioUnitProperty_CocoaUI, kAudioUnitScope_Global,
                                      0, info, &dataSize) == noErr)
            {
                NSString* viewClassName = (NSString*) (info->mCocoaAUViewClass[0]);
                NSString* path = (NSString*) CFURLCopyPath (info->mCocoaAUViewBundleLocation);
                NSBundle* viewBundle = [NSBundle bundleWithPath: [path autorelease]];
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

        if (createGenericViewIfNeeded && (pluginView == 0))
            pluginView = [[AUGenericView alloc] initWithAudioUnit: plugin.audioUnit];

        wrapper.setView (pluginView);

        if (pluginView != nil)
        {
            timerCallback();
            startTimer (70);
        }

        return pluginView != nil;
    }
};

#if JUCE_SUPPORT_CARBON

//==============================================================================
class AudioUnitPluginWindowCarbon   : public AudioProcessorEditor
{
public:
    //==============================================================================
    AudioUnitPluginWindowCarbon (AudioUnitPluginInstance& plugin_)
        : AudioProcessorEditor (&plugin_),
          plugin (plugin_),
          viewComponent (0)
    {
        addAndMakeVisible (innerWrapper = new InnerWrapperComponent (this));

        setOpaque (true);
        setVisible (true);
        setSize (400, 300);

        ComponentDescription viewList [16];
        UInt32 viewListSize = sizeof (viewList);
        AudioUnitGetProperty (plugin.audioUnit, kAudioUnitProperty_GetUIComponentList, kAudioUnitScope_Global,
                              0, &viewList, &viewListSize);

        componentRecord = FindNextComponent (0, &viewList[0]);
    }

    ~AudioUnitPluginWindowCarbon()
    {
        innerWrapper = 0;

        if (isValid())
            plugin.editorBeingDeleted (this);
    }

    bool isValid() const noexcept           { return componentRecord != 0; }

    //==============================================================================
    void paint (Graphics& g)
    {
        g.fillAll (Colours::black);
    }

    void resized()
    {
        innerWrapper->setSize (getWidth(), getHeight());
    }

    //==============================================================================
    bool keyStateChanged (bool)
    {
        return false;
    }

    bool keyPressed (const KeyPress&)
    {
        return false;
    }

    //==============================================================================
    AudioUnit getAudioUnit() const      { return plugin.audioUnit; }

    AudioUnitCarbonView getViewComponent()
    {
        if (viewComponent == 0 && componentRecord != 0)
            viewComponent = (AudioUnitCarbonView) OpenComponent (componentRecord);

        return viewComponent;
    }

    void closeViewComponent()
    {
        if (viewComponent != 0)
        {
            log ("Closing AU GUI: " + plugin.getName());

            CloseComponent (viewComponent);
            viewComponent = 0;
        }
    }

private:
    //==============================================================================
    AudioUnitPluginInstance& plugin;
    ComponentRecord* componentRecord;
    AudioUnitCarbonView viewComponent;

    //==============================================================================
    class InnerWrapperComponent   : public CarbonViewWrapperComponent
    {
    public:
        InnerWrapperComponent (AudioUnitPluginWindowCarbon* const owner_)
            : owner (owner_)
        {
        }

        ~InnerWrapperComponent()
        {
            deleteWindow();
        }

        HIViewRef attachView (WindowRef windowRef, HIViewRef rootView)
        {
            log ("Opening AU GUI: " + owner->plugin.getName());

            AudioUnitCarbonView viewComponent = owner->getViewComponent();

            if (viewComponent == 0)
                return 0;

            Float32Point pos = { 0, 0 };
            Float32Point size = { 250, 200 };

            HIViewRef pluginView = 0;

            AudioUnitCarbonViewCreate (viewComponent,
                                       owner->getAudioUnit(),
                                       windowRef,
                                       rootView,
                                       &pos,
                                       &size,
                                       (ControlRef*) &pluginView);

            return pluginView;
        }

        void removeView (HIViewRef)
        {
            owner->closeViewComponent();
        }

    private:
        AudioUnitPluginWindowCarbon* const owner;
    };

    friend class InnerWrapperComponent;
    ScopedPointer<InnerWrapperComponent> innerWrapper;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioUnitPluginWindowCarbon);
};

#endif

//==============================================================================
bool AudioUnitPluginInstance::hasEditor() const
{
    return true;
}

AudioProcessorEditor* AudioUnitPluginInstance::createEditor()
{
    ScopedPointer<AudioProcessorEditor> w (new AudioUnitPluginWindowCocoa (*this, false));

    if (! static_cast <AudioUnitPluginWindowCocoa*> (static_cast <AudioProcessorEditor*> (w))->isValid())
        w = 0;

#if JUCE_SUPPORT_CARBON
    if (w == nullptr)
    {
        w = new AudioUnitPluginWindowCarbon (*this);

        if (! static_cast <AudioUnitPluginWindowCarbon*> (static_cast <AudioProcessorEditor*> (w))->isValid())
            w = 0;
    }
#endif

    if (w == nullptr)
        w = new AudioUnitPluginWindowCocoa (*this, true); // use AUGenericView as a fallback

    return w.release();
}


//==============================================================================
const String AudioUnitPluginInstance::getCategory() const
{
    const char* result = nullptr;

    switch (componentDesc.componentType)
    {
        case kAudioUnitType_Effect:
        case kAudioUnitType_MusicEffect:    result = "Effect"; break;
        case kAudioUnitType_MusicDevice:    result = "Synth"; break;
        case kAudioUnitType_Generator:      result = "Generator"; break;
        case kAudioUnitType_Panner:         result = "Panner"; break;
        default: break;
    }

    return result;
}

//==============================================================================
int AudioUnitPluginInstance::getNumParameters()
{
    return parameterIds.size();
}

float AudioUnitPluginInstance::getParameter (int index)
{
    const ScopedLock sl (lock);

    Float32 value = 0.0f;

    if (audioUnit != 0 && isPositiveAndBelow (index, parameterIds.size()))
    {
        AudioUnitGetParameter (audioUnit,
                               (UInt32) parameterIds.getUnchecked (index),
                               kAudioUnitScope_Global, 0,
                               &value);
    }

    return value;
}

void AudioUnitPluginInstance::setParameter (int index, float newValue)
{
    const ScopedLock sl (lock);

    if (audioUnit != 0 && isPositiveAndBelow (index, parameterIds.size()))
    {
        AudioUnitSetParameter (audioUnit,
                               (UInt32) parameterIds.getUnchecked (index),
                               kAudioUnitScope_Global, 0,
                               newValue, 0);
    }
}

const String AudioUnitPluginInstance::getParameterName (int index)
{
    AudioUnitParameterInfo info = { 0 };
    UInt32 sz = sizeof (info);

    String name;

    if (AudioUnitGetProperty (audioUnit,
                              kAudioUnitProperty_ParameterInfo,
                              kAudioUnitScope_Global,
                              parameterIds [index], &info, &sz) == noErr)
    {
        if ((info.flags & kAudioUnitParameterFlag_HasCFNameString) != 0)
            name = PlatformUtilities::cfStringToJuceString (info.cfNameString);
        else
            name = String (info.name, sizeof (info.name));
    }

    return name;
}

const String AudioUnitPluginInstance::getParameterText (int index)
{
    return String (getParameter (index));
}

bool AudioUnitPluginInstance::isParameterAutomatable (int index) const
{
    AudioUnitParameterInfo info;
    UInt32 sz = sizeof (info);

    if (AudioUnitGetProperty (audioUnit,
                              kAudioUnitProperty_ParameterInfo,
                              kAudioUnitScope_Global,
                              parameterIds [index], &info, &sz) == noErr)
    {
        return (info.flags & kAudioUnitParameterFlag_NonRealTime) == 0;
    }

    return true;
}

//==============================================================================
int AudioUnitPluginInstance::getNumPrograms()
{
    CFArrayRef presets;
    UInt32 sz = sizeof (CFArrayRef);
    int num = 0;

    if (AudioUnitGetProperty (audioUnit,
                              kAudioUnitProperty_FactoryPresets,
                              kAudioUnitScope_Global,
                              0, &presets, &sz) == noErr)
    {
        num = (int) CFArrayGetCount (presets);
        CFRelease (presets);
    }

    return num;
}

int AudioUnitPluginInstance::getCurrentProgram()
{
    AUPreset current;
    current.presetNumber = 0;
    UInt32 sz = sizeof (AUPreset);

    AudioUnitGetProperty (audioUnit,
                          kAudioUnitProperty_FactoryPresets,
                          kAudioUnitScope_Global,
                          0, &current, &sz);

    return current.presetNumber;
}

void AudioUnitPluginInstance::setCurrentProgram (int newIndex)
{
    AUPreset current;
    current.presetNumber = newIndex;
    current.presetName = 0;

    AudioUnitSetProperty (audioUnit,
                          kAudioUnitProperty_PresentPreset,
                          kAudioUnitScope_Global,
                          0, &current, sizeof (AUPreset));
}

const String AudioUnitPluginInstance::getProgramName (int index)
{
    String s;
    CFArrayRef presets;
    UInt32 sz = sizeof (CFArrayRef);

    if (AudioUnitGetProperty (audioUnit,
                              kAudioUnitProperty_FactoryPresets,
                              kAudioUnitScope_Global,
                              0, &presets, &sz) == noErr)
    {
        for (CFIndex i = 0; i < CFArrayGetCount (presets); ++i)
        {
            const AUPreset* p = (const AUPreset*) CFArrayGetValueAtIndex (presets, i);

            if (p != nullptr && p->presetNumber == index)
            {
                s = PlatformUtilities::cfStringToJuceString (p->presetName);
                break;
            }
        }

        CFRelease (presets);
    }

    return s;
}

void AudioUnitPluginInstance::changeProgramName (int index, const String& newName)
{
    jassertfalse; // xxx not implemented!
}

//==============================================================================
const String AudioUnitPluginInstance::getInputChannelName (int index) const
{
    if (isPositiveAndBelow (index, getNumInputChannels()))
        return "Input " + String (index + 1);

    return String::empty;
}

bool AudioUnitPluginInstance::isInputChannelStereoPair (int index) const
{
    if (! isPositiveAndBelow (index, getNumInputChannels()))
        return false;


    return true;
}

const String AudioUnitPluginInstance::getOutputChannelName (int index) const
{
    if (isPositiveAndBelow (index, getNumOutputChannels()))
        return "Output " + String (index + 1);

    return String::empty;
}

bool AudioUnitPluginInstance::isOutputChannelStereoPair (int index) const
{
    if (! isPositiveAndBelow (index, getNumOutputChannels()))
        return false;

    return true;
}

//==============================================================================
void AudioUnitPluginInstance::getStateInformation (MemoryBlock& destData)
{
    getCurrentProgramStateInformation (destData);
}

void AudioUnitPluginInstance::getCurrentProgramStateInformation (MemoryBlock& destData)
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

        destData.setSize (bytesWritten);
        destData.copyFrom (CFDataGetBytePtr (data), 0, destData.getSize());
        CFRelease (data);

        CFRelease (stream);
        CFRelease (propertyList);
    }
}

void AudioUnitPluginInstance::setStateInformation (const void* data, int sizeInBytes)
{
    setCurrentProgramStateInformation (data, sizeInBytes);
}

void AudioUnitPluginInstance::setCurrentProgramStateInformation (const void* data, int sizeInBytes)
{
    CFReadStreamRef stream = CFReadStreamCreateWithBytesNoCopy (kCFAllocatorDefault,
                                                                (const UInt8*) data,
                                                                sizeInBytes,
                                                                kCFAllocatorNull);
    CFReadStreamOpen (stream);

    CFPropertyListFormat format = kCFPropertyListBinaryFormat_v1_0;
    CFPropertyListRef propertyList = CFPropertyListCreateFromStream (kCFAllocatorDefault,
                                                                     stream,
                                                                     0,
                                                                     kCFPropertyListImmutable,
                                                                     &format,
                                                                     0);
    CFRelease (stream);

    if (propertyList != 0)
        AudioUnitSetProperty (audioUnit,
                              kAudioUnitProperty_ClassInfo,
                              kAudioUnitScope_Global,
                              0, &propertyList, sizeof (propertyList));
}

//==============================================================================
//==============================================================================
AudioUnitPluginFormat::AudioUnitPluginFormat()
{
}

AudioUnitPluginFormat::~AudioUnitPluginFormat()
{
}

void AudioUnitPluginFormat::findAllTypesForFile (OwnedArray <PluginDescription>& results,
                                                 const String& fileOrIdentifier)
{
    if (! fileMightContainThisPluginType (fileOrIdentifier))
        return;

    PluginDescription desc;
    desc.fileOrIdentifier = fileOrIdentifier;
    desc.uid = 0;

    try
    {
        ScopedPointer <AudioPluginInstance> createdInstance (createInstanceFromDescription (desc));
        AudioUnitPluginInstance* const auInstance = dynamic_cast <AudioUnitPluginInstance*> ((AudioPluginInstance*) createdInstance);

        if (auInstance != nullptr)
        {
            auInstance->fillInPluginDescription (desc);
            results.add (new PluginDescription (desc));
        }
    }
    catch (...)
    {
        // crashed while loading...
    }
}

AudioPluginInstance* AudioUnitPluginFormat::createInstanceFromDescription (const PluginDescription& desc)
{
    if (fileMightContainThisPluginType (desc.fileOrIdentifier))
    {
        ScopedPointer <AudioUnitPluginInstance> result (new AudioUnitPluginInstance (desc.fileOrIdentifier));

        if (result->audioUnit != 0)
        {
            result->initialise();
            return result.release();
        }
    }

    return nullptr;
}

StringArray AudioUnitPluginFormat::searchPathsForPlugins (const FileSearchPath& /*directoriesToSearch*/,
                                                                const bool /*recursive*/)
{
    StringArray result;
    ComponentRecord* comp = 0;

    for (;;)
    {
        ComponentDescription desc = { 0 };
        comp = FindNextComponent (comp, &desc);

        if (comp == 0)
            break;

        GetComponentInfo (comp, &desc, 0, 0, 0);

        if (desc.componentType == kAudioUnitType_MusicDevice
             || desc.componentType == kAudioUnitType_MusicEffect
             || desc.componentType == kAudioUnitType_Effect
             || desc.componentType == kAudioUnitType_Generator
             || desc.componentType == kAudioUnitType_Panner)
        {
            const String s (AudioUnitFormatHelpers::createAUPluginIdentifier (desc));
            DBG (s);
            result.add (s);
        }
    }

    return result;
}

bool AudioUnitPluginFormat::fileMightContainThisPluginType (const String& fileOrIdentifier)
{
    ComponentDescription desc;

    String name, version, manufacturer;
    if (AudioUnitFormatHelpers::getComponentDescFromIdentifier (fileOrIdentifier, desc, name, version, manufacturer))
        return FindNextComponent (0, &desc) != 0;

    const File f (fileOrIdentifier);

    return f.hasFileExtension (".component")
             && f.isDirectory();
}

String AudioUnitPluginFormat::getNameOfPluginFromIdentifier (const String& fileOrIdentifier)
{
    ComponentDescription desc;
    String name, version, manufacturer;
    AudioUnitFormatHelpers::getComponentDescFromIdentifier (fileOrIdentifier, desc, name, version, manufacturer);

    if (name.isEmpty())
        name = fileOrIdentifier;

    return name;
}

bool AudioUnitPluginFormat::doesPluginStillExist (const PluginDescription& desc)
{
    if (desc.fileOrIdentifier.startsWithIgnoreCase (AudioUnitFormatHelpers::auIdentifierPrefix))
        return fileMightContainThisPluginType (desc.fileOrIdentifier);
    else
        return File (desc.fileOrIdentifier).exists();
}

FileSearchPath AudioUnitPluginFormat::getDefaultLocationsToSearch()
{
    return FileSearchPath ("/(Default AudioUnit locations)");
}

#endif

END_JUCE_NAMESPACE

#undef log

#endif

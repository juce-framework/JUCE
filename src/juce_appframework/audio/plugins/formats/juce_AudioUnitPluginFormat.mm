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

#include "../../../../../juce_Config.h"

#if JUCE_PLUGINHOST_AU && (! (defined (LINUX) || defined (_WIN32)))

#include <AudioUnit/AudioUnit.h>
#include <AudioUnit/AUCocoaUIView.h>

#if JUCE_MAC && JUCE_32BIT
  #define JUCE_SUPPORT_CARBON 1
#endif

#if JUCE_SUPPORT_CARBON
#include <AudioToolbox/AudioUnitUtilities.h>
#include <AudioUnit/AudioUnitCarbonView.h>
#endif

#include "../../../../juce_core/basics/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_AudioUnitPluginFormat.h"
#include "../juce_PluginDescription.h"
#include "../../../../juce_core/threads/juce_ScopedLock.h"
#include "../../../../juce_appframework/events/juce_Timer.h"
#include "../../../../juce_core/misc/juce_PlatformUtilities.h"
#include "../../../../juce_appframework/gui/components/layout/juce_ComponentMovementWatcher.h"
#include "../../../../juce_appframework/gui/components/special/juce_NSViewComponent.h"

#if JUCE_MAC

#if MACOS_10_3_OR_EARLIER
 #define kAudioUnitType_Generator 'augn'
#endif

// Change this to disable logging of various activities
#ifndef AU_LOGGING
  #define AU_LOGGING 1
#endif

#if AU_LOGGING
 #define log(a) Logger::writeToLog(a);
#else
 #define log(a)
#endif

#if JUCE_SUPPORT_CARBON
#include "../../../../../build/macosx/platform_specific_code/juce_mac_CarbonViewWrapperComponent.h"
#endif


static int insideCallback = 0;

//==============================================================================
class AudioUnitPluginWindowCarbon;
class AudioUnitPluginWindowCocoa;

//==============================================================================
class AudioUnitPluginInstance     : public AudioPluginInstance
{
public:
    //==============================================================================
    ~AudioUnitPluginInstance();

    //==============================================================================
    // AudioPluginInstance methods:

    void fillInPluginDescription (PluginDescription& desc) const
    {
        desc.name = pluginName;
        desc.file = file;
        desc.uid = ((int) componentDesc.componentType)
                    ^ ((int) componentDesc.componentSubType)
                    ^ ((int) componentDesc.componentManufacturer);
        desc.lastFileModTime = file.getLastModificationTime();
        desc.pluginFormatName = "AudioUnit";
        desc.category = getCategory();
        desc.manufacturerName = manufacturer;
        desc.version = version;
        desc.numInputChannels = getNumInputChannels();
        desc.numOutputChannels = getNumOutputChannels();
        desc.isInstrument = (componentDesc.componentType == kAudioUnitType_MusicDevice);
    }

    const String getName() const                { return pluginName; }
    bool acceptsMidi() const                    { return wantsMidiMessages; }
    bool producesMidi() const                   { return false; }

    //==============================================================================
    // AudioProcessor methods:

    void prepareToPlay (double sampleRate, int estimatedSamplesPerBlock);
    void releaseResources();
    void processBlock (AudioSampleBuffer& buffer,
                       MidiBuffer& midiMessages);

    AudioProcessorEditor* createEditor();

    const String getInputChannelName (const int index) const;
    bool isInputChannelStereoPair (int index) const;

    const String getOutputChannelName (const int index) const;
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

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    friend class AudioUnitPluginWindowCarbon;
    friend class AudioUnitPluginWindowCocoa;
    friend class AudioUnitPluginFormat;

    ComponentDescription componentDesc;
    String pluginName, manufacturer, version;
    File file;
    CriticalSection lock;
    bool initialised, wantsMidiMessages, wasPlaying;

    AudioBufferList* outputBufferList;
    AudioTimeStamp timeStamp;
    AudioSampleBuffer* currentBuffer;

    AudioUnit audioUnit;
    Array <int> parameterIds;

    //==============================================================================
    bool getComponentDescFromFile (const File& file);
    void initialise();

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
            for (int i = 0; i < supportedChannelsSize / sizeof (AUChannelInfo); ++i)
            {
                numIns = jmax (numIns, supportedChannels[i].inChannels);
                numOuts = jmax (numOuts, supportedChannels[i].outChannels);
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
    AudioUnitPluginInstance (const File& file);
};

//==============================================================================
AudioUnitPluginInstance::AudioUnitPluginInstance (const File& file_)
    : file (file_),
      initialised (false),
      wantsMidiMessages (false),
      audioUnit (0),
      outputBufferList (0),
      currentBuffer (0)
{
    try
    {
        ++insideCallback;

        log (T("Opening AU: ") + file.getFullPathName());

        if (getComponentDescFromFile (file))
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
    {
        const ScopedLock sl (lock);

        jassert (insideCallback == 0);

        if (audioUnit != 0)
        {
            AudioUnitUninitialize (audioUnit);
            CloseComponent (audioUnit);
            audioUnit = 0;
        }
    }

    juce_free (outputBufferList);
}

bool AudioUnitPluginInstance::getComponentDescFromFile (const File& file)
{
    zerostruct (componentDesc);

    if (! file.hasFileExtension (T(".component")))
        return false;

    const String filename (file.getFullPathName());
    const char* const utf8 = filename.toUTF8();
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
    if (initialised || audioUnit == 0)
        return;

    log (T("Initialising AU: ") + pluginName);

    parameterIds.clear();

    {
        UInt32 paramListSize = 0;
        AudioUnitGetProperty (audioUnit, kAudioUnitProperty_ParameterList, kAudioUnitScope_Global,
                              0, 0, &paramListSize);

        if (paramListSize > 0)
        {
            parameterIds.insertMultiple (0, 0, paramListSize / sizeof (int));

            AudioUnitGetProperty (audioUnit, kAudioUnitProperty_ParameterList, kAudioUnitScope_Global,
                                  0, &parameterIds.getReference(0), &paramListSize);
        }
    }

    {
        AURenderCallbackStruct info;
        zerostruct (info);
        info.inputProcRefCon = this;
        info.inputProc = renderGetInputCallback;

        AudioUnitSetProperty (audioUnit, kAudioUnitProperty_SetRenderCallback, kAudioUnitScope_Input,
                              0, &info, sizeof (info));
    }

    {
        HostCallbackInfo info;
        zerostruct (info);
        info.hostUserData = this;
        info.beatAndTempoProc = getBeatAndTempoCallback;
        info.musicalTimeLocationProc = getMusicalTimeLocationCallback;
        info.transportStateProc = getTransportStateCallback;

        AudioUnitSetProperty (audioUnit, kAudioUnitProperty_HostCallbacks, kAudioUnitScope_Global,
                              0, &info, sizeof (info));
    }

    int numIns, numOuts;
    getNumChannels (numIns, numOuts);
    setPlayConfigDetails (numIns, numOuts, 0, 0);

    initialised = AudioUnitInitialize (audioUnit) == noErr;

    setLatencySamples (0);
}


//==============================================================================
void AudioUnitPluginInstance::prepareToPlay (double sampleRate_,
                                             int samplesPerBlockExpected)
{
    initialise();

    if (initialised)
    {
        int numIns, numOuts;
        getNumChannels (numIns, numOuts);

        setPlayConfigDetails (numIns, numOuts, sampleRate_, samplesPerBlockExpected);

        Float64 latencySecs = 0.0;
        UInt32 latencySize = sizeof (latencySecs);
        AudioUnitGetProperty (audioUnit, kAudioUnitProperty_Latency, kAudioUnitScope_Global,
                              0, &latencySecs, &latencySize);

        setLatencySamples (roundDoubleToInt (latencySecs * sampleRate_));

        AudioUnitReset (audioUnit, kAudioUnitScope_Input, 0);
        AudioUnitReset (audioUnit, kAudioUnitScope_Output, 0);
        AudioUnitReset (audioUnit, kAudioUnitScope_Global, 0);

        AudioStreamBasicDescription stream;
        zerostruct (stream);
        stream.mSampleRate = sampleRate_;
        stream.mFormatID = kAudioFormatLinearPCM;
        stream.mFormatFlags = kAudioFormatFlagsNativeFloatPacked | kAudioFormatFlagIsNonInterleaved;
        stream.mFramesPerPacket = 1;
        stream.mBytesPerPacket = 4;
        stream.mBytesPerFrame = 4;
        stream.mBitsPerChannel = 32;
        stream.mChannelsPerFrame = numIns;

        OSStatus err = AudioUnitSetProperty (audioUnit,
                                             kAudioUnitProperty_StreamFormat,
                                             kAudioUnitScope_Input,
                                             0, &stream, sizeof (stream));

        stream.mChannelsPerFrame = numOuts;

        err = AudioUnitSetProperty (audioUnit,
                                    kAudioUnitProperty_StreamFormat,
                                    kAudioUnitScope_Output,
                                    0, &stream, sizeof (stream));

        juce_free (outputBufferList);
        outputBufferList = (AudioBufferList*) juce_calloc (sizeof (AudioBufferList) + sizeof (AudioBuffer) * (numOuts + 1));
        outputBufferList->mNumberBuffers = numOuts;

        for (int i = numOuts; --i >= 0;)
            outputBufferList->mBuffers[i].mNumberChannels = 1;

        zerostruct (timeStamp);
        timeStamp.mSampleTime = 0;
        timeStamp.mHostTime = AudioGetCurrentHostTime();
        timeStamp.mFlags = kAudioTimeStampSampleTimeValid | kAudioTimeStampHostTimeValid;

        currentBuffer = 0;
        wasPlaying = false;
    }
}

void AudioUnitPluginInstance::releaseResources()
{
    if (initialised)
    {
        AudioUnitReset (audioUnit, kAudioUnitScope_Input, 0);
        AudioUnitReset (audioUnit, kAudioUnitScope_Output, 0);
        AudioUnitReset (audioUnit, kAudioUnitScope_Global, 0);

        juce_free (outputBufferList);
        outputBufferList = 0;
        currentBuffer = 0;
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
                zeromem (ioData->mBuffers[i].mData, sizeof (float) * inNumberFrames);
            }
        }
    }

    return noErr;
}

void AudioUnitPluginInstance::processBlock (AudioSampleBuffer& buffer,
                                            MidiBuffer& midiMessages)
{
    const int numSamples = buffer.getNumSamples();

    if (initialised)
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
        // Not initialised, so just bypass..
        for (int i = getNumInputChannels(); i < getNumOutputChannels(); ++i)
            buffer.clear (i, 0, buffer.getNumSamples());
    }
}

//==============================================================================
OSStatus AudioUnitPluginInstance::getBeatAndTempo (Float64* outCurrentBeat, Float64* outCurrentTempo) const
{
    AudioPlayHead* const ph = getPlayHead();
    AudioPlayHead::CurrentPositionInfo result;

    if (ph != 0 && ph->getCurrentPosition (result))
    {
        *outCurrentBeat = result.ppqPosition;
        *outCurrentTempo = result.bpm;
    }
    else
    {
        *outCurrentBeat = 0;
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

    if (ph != 0 && ph->getCurrentPosition (result))
    {
        *outTimeSig_Numerator = result.timeSigNumerator;
        *outTimeSig_Denominator = result.timeSigDenominator;

        *outDeltaSampleOffsetToNextBeat = 0; //xxx
        *outCurrentMeasureDownBeat = result.ppqPositionOfLastBarStart; //xxx wrong
    }
    else
    {
        *outDeltaSampleOffsetToNextBeat = 0;
        *outTimeSig_Numerator = 4;
        *outTimeSig_Denominator = 4;
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

    if (ph != 0 && ph->getCurrentPosition (result))
    {
        if (outIsPlaying != 0)
            *outIsPlaying = result.isPlaying;

        if (outTransportStateChanged != 0)
        {
            *outTransportStateChanged = result.isPlaying != wasPlaying;
            wasPlaying = result.isPlaying;
        }

        if (outCurrentSampleInTimeLine != 0)
            *outCurrentSampleInTimeLine = roundDoubleToInt (result.timeInSeconds * getSampleRate());

        if (outIsCycling != 0)
            *outIsCycling = false;

        if (outCycleStartBeat != 0)
            *outCycleStartBeat = 0;

        if (outCycleEndBeat != 0)
            *outCycleEndBeat = 0;
    }
    else
    {
        if (outIsPlaying != 0)
            *outIsPlaying = false;

        if (outTransportStateChanged != 0)
            *outTransportStateChanged = false;

        if (outCurrentSampleInTimeLine != 0)
            *outCurrentSampleInTimeLine = 0;

        if (outIsCycling != 0)
            *outIsCycling = false;

        if (outCycleStartBeat != 0)
            *outCycleStartBeat = 0;

        if (outCycleEndBeat != 0)
            *outCycleEndBeat = 0;
    }

    return noErr;
}


//==============================================================================
static VoidArray activeWindows;

class AudioUnitPluginWindowCocoa    : public AudioProcessorEditor
{
public:
    AudioUnitPluginWindowCocoa (AudioUnitPluginInstance& plugin_)
        : AudioProcessorEditor (&plugin_),
          plugin (plugin_),
          wrapper (0)
    {
        addAndMakeVisible (wrapper = new NSViewComponent());

        activeWindows.add (this);

        setOpaque (true);
        setVisible (true);
        setSize (100, 100);

        createView();
    }

    ~AudioUnitPluginWindowCocoa()
    {
        wrapper->setView (0);
        activeWindows.removeValue (this);
        if (isValid())
            plugin.editorBeingDeleted (this);
        delete wrapper;
    }

    bool isValid() const        { return wrapper->getView() != 0; }

    void resized()
    {
        wrapper->setSize (getWidth(), getHeight());
    }

private:
    AudioUnitPluginInstance& plugin;
    NSViewComponent* wrapper;

    bool createView()
    {
        UInt32 dataSize = 0;
        Boolean isWritable = false;
        if (AudioUnitGetPropertyInfo (plugin.audioUnit, kAudioUnitProperty_CocoaUI, kAudioUnitScope_Global,
                                      0, &dataSize, &isWritable) != noErr
             || dataSize == 0)
        {
            return false;
        }

        if (AudioUnitGetPropertyInfo (plugin.audioUnit, kAudioUnitProperty_CocoaUI, kAudioUnitScope_Global,
                                      0, &dataSize, &isWritable) != noErr)
        {
            return false;
        }

        NSView* pluginView = 0;
        AudioUnitCocoaViewInfo* info = (AudioUnitCocoaViewInfo*) juce_calloc (dataSize);

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
            {
                CFRelease (info->mCocoaAUViewClass[i]);
                CFRelease (info->mCocoaAUViewBundleLocation);
            }
        }

        juce_free (info);
        wrapper->setView (pluginView);
        return pluginView != 0;
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

        activeWindows.add (this);

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
        deleteAndZero (innerWrapper);

        activeWindows.removeValue (this);

        if (isValid())
            plugin.editorBeingDeleted (this);
    }

    bool isValid() const throw()            { return componentRecord != 0; }

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
    bool keyStateChanged()
    {
        return false;
    }

    bool keyPressed (const KeyPress&)
    {
        return false;
    }

    //==============================================================================
    void broughtToFront()
    {
        activeWindows.removeValue (this);
        activeWindows.add (this);
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
            CloseComponent (viewComponent);
            viewComponent = 0;
        }
    }

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
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
            log (T("Opening AU GUI: ") + owner->plugin.getName());

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
            log (T("Closing AU GUI: ") + owner->plugin.getName());

            owner->closeViewComponent();
        }

    private:
        AudioUnitPluginWindowCarbon* const owner;
    };

    friend class InnerWrapperComponent;
    InnerWrapperComponent* innerWrapper;
};

#endif

//==============================================================================
AudioProcessorEditor* AudioUnitPluginInstance::createEditor()
{
    AudioProcessorEditor* w = new AudioUnitPluginWindowCocoa (*this);

    if (! ((AudioUnitPluginWindowCocoa*) w)->isValid())
        deleteAndZero (w);

#if JUCE_SUPPORT_CARBON
    if (w == 0)
    {
        w = new AudioUnitPluginWindowCarbon (*this);

        if (! ((AudioUnitPluginWindowCarbon*) w)->isValid())
            deleteAndZero (w);
    }
#endif

    return w;
}


//==============================================================================
const String AudioUnitPluginInstance::getCategory() const
{
    const char* result = 0;

    switch (componentDesc.componentType)
    {
    case kAudioUnitType_Effect:
    case kAudioUnitType_MusicEffect:
        result = "Effect";
        break;
    case kAudioUnitType_MusicDevice:
        result = "Synth";
        break;
    case kAudioUnitType_Generator:
        result = "Generator";
        break;
    case kAudioUnitType_Panner:
        result = "Panner";
        break;
    default:
        break;
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

    if (audioUnit != 0 && ((unsigned int) index) < (unsigned int) parameterIds.size())
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

    if (audioUnit != 0 && ((unsigned int) index) < (unsigned int) parameterIds.size())
    {
        AudioUnitSetParameter (audioUnit,
                               (UInt32) parameterIds.getUnchecked (index),
                               kAudioUnitScope_Global, 0,
                               newValue, 0);
    }
}

const String AudioUnitPluginInstance::getParameterName (int index)
{
    AudioUnitParameterInfo info;
    zerostruct (info);
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
                          kAudioUnitProperty_FactoryPresets,
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

            if (p != 0 && p->presetNumber == index)
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
    jassertfalse // xxx not implemented!
}

//==============================================================================
const String AudioUnitPluginInstance::getInputChannelName (const int index) const
{
    if (((unsigned int) index) < (unsigned int) getNumInputChannels())
        return T("Input ") + String (index + 1);

    return String::empty;
}

bool AudioUnitPluginInstance::isInputChannelStereoPair (int index) const
{
    if (((unsigned int) index) >= (unsigned int) getNumInputChannels())
        return false;


    return true;
}

const String AudioUnitPluginInstance::getOutputChannelName (const int index) const
{
    if (((unsigned int) index) < (unsigned int) getNumOutputChannels())
        return T("Output ") + String (index + 1);

    return String::empty;
}

bool AudioUnitPluginInstance::isOutputChannelStereoPair (int index) const
{
    if (((unsigned int) index) >= (unsigned int) getNumOutputChannels())
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
                                                 const File& file)
{
    if (! fileMightContainThisPluginType (file))
        return;

    PluginDescription desc;
    desc.file = file;
    desc.uid = 0;

    AudioUnitPluginInstance* instance = dynamic_cast <AudioUnitPluginInstance*> (createInstanceFromDescription (desc));

    if (instance == 0)
        return;

    try
    {
        instance->fillInPluginDescription (desc);
        results.add (new PluginDescription (desc));
    }
    catch (...)
    {
        // crashed while loading...
    }

    deleteAndZero (instance);
}

AudioPluginInstance* AudioUnitPluginFormat::createInstanceFromDescription (const PluginDescription& desc)
{
    AudioUnitPluginInstance* result = 0;

    if (fileMightContainThisPluginType (desc.file))
    {
        result = new AudioUnitPluginInstance (desc.file);

        if (result->audioUnit != 0)
        {
            result->initialise();
        }
        else
        {
            deleteAndZero (result);
        }
    }

    return result;
}

bool AudioUnitPluginFormat::fileMightContainThisPluginType (const File& f)
{
    return f.hasFileExtension (T(".component"))
             && f.isDirectory();
}

const FileSearchPath AudioUnitPluginFormat::getDefaultLocationsToSearch()
{
    return FileSearchPath ("~/Library/Audio/Plug-Ins/Components;/Library/Audio/Plug-Ins/Components");
}

#endif

END_JUCE_NAMESPACE

#undef log

#endif

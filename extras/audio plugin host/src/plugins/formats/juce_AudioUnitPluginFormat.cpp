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

#if ! (defined (LINUX) || defined (_WIN32))

#include <Carbon/Carbon.h>
#include <AudioToolbox/AudioToolbox.h>

#include "../../../../../juce.h"
#include "juce_AudioUnitPluginFormat.h"
#include "../juce_PluginDescription.h"

#if JUCE_PLUGINHOST_AU && JUCE_MAC

BEGIN_JUCE_NAMESPACE
 extern void juce_callAnyTimersSynchronously();
 extern bool juce_isHIViewCreatedByJuce (HIViewRef view);
 extern bool juce_isWindowCreatedByJuce (WindowRef window);
END_JUCE_NAMESPACE

// Change this to disable logging of various activities
#ifndef AU_LOGGING
  #define AU_LOGGING 1
#endif

#if AU_LOGGING
 #define log(a) Logger::writeToLog(a);
#else
 #define log(a)
#endif

static int insideCallback = 0;

//==============================================================================
class AudioUnitPluginWindow;

//==============================================================================
class AudioUnitPluginInstance     : public AudioPluginInstance
{
public:
    //==============================================================================
    ~AudioUnitPluginInstance();

    //==============================================================================
    // AudioPluginInstance methods:

    const String getName() const                { return pluginName; }
    const String getManufacturer() const        { return manufacturer; }
    const String getVersion() const             { return version; }
    bool isInstrument() const                   { return componentDesc.componentType == kAudioUnitType_MusicDevice; }
    const String getCategory() const;
    const String getFormatName() const          { return "AudioUnit"; }
    const File getFile() const                  { return file; }
    int getUID() const                          { return file.hashCode(); }
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
    friend class AudioUnitPluginWindow;
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
        *outIsPlaying = result.isPlaying;
        *outTransportStateChanged = result.isPlaying != wasPlaying;
        wasPlaying = result.isPlaying;
        *outCurrentSampleInTimeLine = roundDoubleToInt (result.timeInSeconds * getSampleRate());
        *outIsCycling = false;
        *outCycleStartBeat = 0;
        *outCycleEndBeat = 0;
    }
    else
    {
        *outIsPlaying = false;
        *outTransportStateChanged = false;
        *outCurrentSampleInTimeLine = 0;
        *outIsCycling = false;
        *outCycleStartBeat = 0;
        *outCycleEndBeat = 0;
    }

    return noErr;
}


//==============================================================================
static VoidArray activeWindows;

//==============================================================================
class AudioUnitPluginWindow   : public AudioProcessorEditor,
                                public Timer
{
public:
    //==============================================================================
    AudioUnitPluginWindow (AudioUnitPluginInstance& plugin_)
        : AudioProcessorEditor (&plugin_),
          plugin (plugin_),
          isOpen (false),
          pluginWantsKeys (false),
          wasShowing (false),
          recursiveResize (false),
          viewComponent (0),
          pluginViewRef (0)
    {
        movementWatcher = new CompMovementWatcher (this);

        activeWindows.add (this);

        setOpaque (true);
        setVisible (true);
        setSize (1, 1);

        ComponentDescription viewList [16];
        UInt32 viewListSize = sizeof (viewList);
        AudioUnitGetProperty (plugin.audioUnit, kAudioUnitProperty_GetUIComponentList, kAudioUnitScope_Global,
                              0, &viewList, &viewListSize);

        componentRecord = FindNextComponent (0, &viewList[0]);
    }

    ~AudioUnitPluginWindow()
    {
        deleteAndZero (movementWatcher);

        closePluginWindow();

        activeWindows.removeValue (this);
        plugin.editorBeingDeleted (this);
    }

    bool isValid() const throw()            { return componentRecord != 0; }

    //==============================================================================
    void componentMovedOrResized()
    {
        if (recursiveResize)
            return;

        Component* const topComp = getTopLevelComponent();

        if (topComp->getPeer() != 0)
        {
            int x = 0, y = 0;
            relativePositionToOtherComponent (topComp, x, y);

            recursiveResize = true;

            if (pluginViewRef != 0)
            {
                HIRect r;
                r.origin.x = (float) x;
                r.origin.y = (float) y;
                r.size.width = (float) getWidth();
                r.size.height = (float) getHeight();
                HIViewSetFrame (pluginViewRef, &r);
            }

            recursiveResize = false;
        }
    }

    void componentVisibilityChanged()
    {
        const bool isShowingNow = isShowing();

        if (wasShowing != isShowingNow)
        {
            wasShowing = isShowingNow;

            if (isShowingNow)
                openPluginWindow();
            else
                closePluginWindow();
        }

        componentMovedOrResized();
    }

    void componentPeerChanged()
    {
        closePluginWindow();
        openPluginWindow();
    }

    void timerCallback()
    {
        if (pluginViewRef != 0)
        {
            HIRect bounds;
            HIViewGetBounds (pluginViewRef, &bounds);
            const int w = jmax (32, (int) bounds.size.width);
            const int h = jmax (32, (int) bounds.size.height);

            if (w != getWidth() || h != getHeight())
            {
                setSize (w, h);
                startTimer (50);
            }
            else
            {
                startTimer (jlimit (50, 500, getTimerInterval() + 20));
            }
        }
    }

    //==============================================================================
    bool keyStateChanged()
    {
        return pluginWantsKeys;
    }

    bool keyPressed (const KeyPress&)
    {
        return pluginWantsKeys;
    }

    //==============================================================================
    void paint (Graphics& g)
    {
        if (isOpen)
        {
            ComponentPeer* const peer = getPeer();

            if (peer != 0)
            {
                peer->addMaskedRegion (getScreenX() - peer->getScreenX(),
                                       getScreenY() - peer->getScreenY(),
                                       getWidth(), getHeight());
            }
        }
        else
        {
            g.fillAll (Colours::black);
        }
    }

    //==============================================================================
    void broughtToFront()
    {
        activeWindows.removeValue (this);
        activeWindows.add (this);
    }

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    AudioUnitPluginInstance& plugin;
    bool isOpen, wasShowing, recursiveResize;
    bool pluginWantsKeys;

    ComponentRecord* componentRecord;
    AudioUnitCarbonView viewComponent;
    HIViewRef pluginViewRef;

    //==============================================================================
    void openPluginWindow()
    {
        if (isOpen || getWindowHandle() == 0 || componentRecord == 0)
            return;

        log (T("Opening AU GUI: ") + plugin.getName());
        isOpen = true;

        pluginWantsKeys = true; //xxx any way to find this out? Does it matter?

        viewComponent = (AudioUnitCarbonView) OpenComponent (componentRecord);

        if (viewComponent != 0)
        {
            Float32Point pos = { getScreenX() - getTopLevelComponent()->getScreenX(),
                                 getScreenY() - getTopLevelComponent()->getScreenY() };
            Float32Point size = { 250, 200 };

            AudioUnitCarbonViewCreate (viewComponent,
                                       plugin.audioUnit,
                                       (WindowRef) getWindowHandle(),
                                       HIViewGetRoot ((WindowRef) getWindowHandle()),
                                       &pos, &size,
                                       (ControlRef*) &pluginViewRef);
        }

        timerCallback(); // to set our comp to the right size
        repaint();
    }

    //==============================================================================
    void closePluginWindow()
    {
        stopTimer();

        if (isOpen)
        {
            log (T("Closing AU GUI: ") + plugin.getName());
            isOpen = false;

            if (viewComponent != 0)
                CloseComponent (viewComponent);

            pluginViewRef = 0;
        }
    }

    //==============================================================================
    class CompMovementWatcher  : public ComponentMovementWatcher
    {
    public:
        CompMovementWatcher (AudioUnitPluginWindow* const owner_)
            : ComponentMovementWatcher (owner_),
              owner (owner_)
        {
        }

        //==============================================================================
        void componentMovedOrResized (bool /*wasMoved*/, bool /*wasResized*/)
        {
            owner->componentMovedOrResized();
        }

        void componentPeerChanged()
        {
            owner->componentPeerChanged();
        }

        void componentVisibilityChanged (Component&)
        {
            owner->componentVisibilityChanged();
        }

    private:
        AudioUnitPluginWindow* const owner;
    };

    CompMovementWatcher* movementWatcher;
};

//==============================================================================
AudioProcessorEditor* AudioUnitPluginInstance::createEditor()
{
    AudioUnitPluginWindow* w = new AudioUnitPluginWindow (*this);

    if (! w->isValid())
        deleteAndZero (w);

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
        desc.fillInFromInstance (*instance);

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
#endif

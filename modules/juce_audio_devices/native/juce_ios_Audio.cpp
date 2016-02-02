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

class iOSAudioIODevice;

//==================================================================================================
struct AudioSessionHolder
{
    AudioSessionHolder();
    ~AudioSessionHolder();

    void handleStatusChange (bool enabled, const char* reason) const;
    void handleRouteChange (const char* reason) const;

    Array<iOSAudioIODevice*> activeDevices;

    id nativeSession;
};

static const char* getRoutingChangeReason (AVAudioSessionRouteChangeReason reason) noexcept
{
    switch (reason)
    {
        case AVAudioSessionRouteChangeReasonNewDeviceAvailable:         return "New device available";
        case AVAudioSessionRouteChangeReasonOldDeviceUnavailable:       return "Old device unavailable";
        case AVAudioSessionRouteChangeReasonCategoryChange:             return "Category change";
        case AVAudioSessionRouteChangeReasonOverride:                   return "Override";
        case AVAudioSessionRouteChangeReasonWakeFromSleep:              return "Wake from sleep";
        case AVAudioSessionRouteChangeReasonNoSuitableRouteForCategory: return "No suitable route for category";
        case AVAudioSessionRouteChangeReasonRouteConfigurationChange:   return "Route configuration change";
        case AVAudioSessionRouteChangeReasonUnknown:
        default:                                                        return "Unknown";
    }
}

bool getNotificationValueForKey (NSNotification* notification, NSString* key, NSUInteger& value) noexcept
{
    if (notification != nil)
    {
        if (NSDictionary* userInfo = [notification userInfo])
        {
            if (NSNumber* number = [userInfo objectForKey: key])
            {
                value = [number unsignedIntegerValue];
                return true;
            }
        }
    }

    jassertfalse;
    return false;
}

} // juce namespace

//==================================================================================================
@interface iOSAudioSessionNative  : NSObject
{
@private
    juce::AudioSessionHolder* audioSessionHolder;
};

- (id) init: (juce::AudioSessionHolder*) holder;
- (void) dealloc;

- (void) audioSessionDidChangeInterruptionType: (NSNotification*) notification;
- (void) handleMediaServicesReset;
- (void) handleMediaServicesLost;
- (void) handleRouteChange: (NSNotification*) notification;
@end

@implementation iOSAudioSessionNative

- (id) init: (juce::AudioSessionHolder*) holder
{
    self = [super init];

    if (self != nil)
    {
        audioSessionHolder = holder;

        auto session = [AVAudioSession sharedInstance];
        auto centre = [NSNotificationCenter defaultCenter];

        [centre addObserver: self
                   selector: @selector (audioSessionDidChangeInterruptionType:)
                       name: AVAudioSessionInterruptionNotification
                     object: session];

        [centre addObserver: self
                   selector: @selector (handleMediaServicesLost)
                       name: AVAudioSessionMediaServicesWereLostNotification
                     object: session];

        [centre addObserver: self
                   selector: @selector (handleMediaServicesReset)
                       name: AVAudioSessionMediaServicesWereResetNotification
                     object: session];

        [centre addObserver: self
                   selector: @selector (handleRouteChange:)
                       name: AVAudioSessionRouteChangeNotification
                     object: session];
    }
    else
    {
        jassertfalse;
    }

    return self;
}

- (void) dealloc
{
    [[NSNotificationCenter defaultCenter] removeObserver: self];
    [super dealloc];
}

- (void) audioSessionDidChangeInterruptionType: (NSNotification*) notification
{
    NSUInteger value;

    if (juce::getNotificationValueForKey (notification, AVAudioSessionInterruptionTypeKey, value))
    {
        switch ((AVAudioSessionInterruptionType) value)
        {
            case AVAudioSessionInterruptionTypeBegan:
                audioSessionHolder->handleStatusChange (false, "AVAudioSessionInterruptionTypeBegan");
                break;

            case AVAudioSessionInterruptionTypeEnded:
                audioSessionHolder->handleStatusChange (true, "AVAudioSessionInterruptionTypeEnded");
                break;

            // No default so the code doesn't compile if this enum is extended.
        }
    }
}

- (void) handleMediaServicesReset
{
    audioSessionHolder->handleStatusChange (true, "AVAudioSessionMediaServicesWereResetNotification");
}

- (void) handleMediaServicesLost
{
    audioSessionHolder->handleStatusChange (false, "AVAudioSessionMediaServicesWereLostNotification");
}

- (void) handleRouteChange: (NSNotification*) notification
{
    NSUInteger value;

    if (juce::getNotificationValueForKey (notification, AVAudioSessionRouteChangeReasonKey, value))
        audioSessionHolder->handleRouteChange (juce::getRoutingChangeReason ((AVAudioSessionRouteChangeReason) value));
}

@end

//==================================================================================================
namespace juce {

#ifndef JUCE_IOS_AUDIO_LOGGING
 #define JUCE_IOS_AUDIO_LOGGING 0
#endif

#if JUCE_IOS_AUDIO_LOGGING
 #define JUCE_IOS_AUDIO_LOG(x)  DBG(x)
#else
 #define JUCE_IOS_AUDIO_LOG(x)
#endif

static void logNSError (NSError* e)
{
    if (e != nil)
    {
        JUCE_IOS_AUDIO_LOG ("iOS Audio error: " << [e.localizedDescription UTF8String]);
        jassertfalse;
    }
}

#define JUCE_NSERROR_CHECK(X)     { NSError* error = nil; X; logNSError (error); }


//==================================================================================================
class iOSAudioIODevice  : public AudioIODevice
{
public:
    iOSAudioIODevice (const String& deviceName)  : AudioIODevice (deviceName, "Audio")
    {
        sessionHolder->activeDevices.add (this);
        updateSampleRateAndAudioInput();
    }

    ~iOSAudioIODevice()
    {
        sessionHolder->activeDevices.removeFirstMatchingValue (this);
        close();
    }

    StringArray getOutputChannelNames() override
    {
        return { "Left", "Right" };
    }

    StringArray getInputChannelNames() override
    {
        if (audioInputIsAvailable)
            return { "Left", "Right" };

        return {};
    }

    static void setAudioSessionActive (bool enabled)
    {
        JUCE_NSERROR_CHECK ([[AVAudioSession sharedInstance] setActive: enabled
                                                                 error: &error]);
    }

    static double trySampleRate (double rate)
    {
        auto session = [AVAudioSession sharedInstance];
        JUCE_NSERROR_CHECK ([session setPreferredSampleRate: rate
                                                      error: &error]);
        return session.sampleRate;
    }

    Array<double> getAvailableSampleRates() override
    {
        Array<double> rates;

        // Important: the supported audio sample rates change on the iPhone 6S
        // depending on whether the headphones are plugged in or not!
        setAudioSessionActive (true);

        const double lowestRate = trySampleRate (4000);
        const double highestRate = trySampleRate (192000);

        for (double rate = lowestRate; rate <= highestRate; rate += 1000)
        {
            const double supportedRate = trySampleRate (rate);

            rates.addIfNotAlreadyThere (supportedRate);
            rate = jmax (rate, supportedRate);
        }

        for (auto r : rates)
        {
            ignoreUnused (r);
            JUCE_IOS_AUDIO_LOG ("available rate = " + String (r, 0) + "Hz");
        }

        return rates;
    }

    Array<int> getAvailableBufferSizes() override
    {
        Array<int> r;

        for (int i = 6; i < 12; ++i)
            r.add (1 << i);

        return r;
    }

    int getDefaultBufferSize() override         { return 256; }

    String open (const BigInteger& inputChannelsWanted,
                 const BigInteger& outputChannelsWanted,
                 double targetSampleRate, int bufferSize) override
    {
        close();

        lastError.clear();
        preferredBufferSize = bufferSize <= 0 ? getDefaultBufferSize()
                                              : bufferSize;

        //  xxx set up channel mapping

        activeOutputChans = outputChannelsWanted;
        activeOutputChans.setRange (2, activeOutputChans.getHighestBit(), false);
        numOutputChannels = activeOutputChans.countNumberOfSetBits();
        monoOutputChannelNumber = activeOutputChans.findNextSetBit (0);

        activeInputChans = inputChannelsWanted;
        activeInputChans.setRange (2, activeInputChans.getHighestBit(), false);
        numInputChannels = activeInputChans.countNumberOfSetBits();
        monoInputChannelNumber = activeInputChans.findNextSetBit (0);

        setAudioSessionActive (true);

        // Set the session category & options:
        auto session = [AVAudioSession sharedInstance];

        const bool useInputs = (numInputChannels > 0 && audioInputIsAvailable);

        NSString* category = (useInputs ? AVAudioSessionCategoryPlayAndRecord : AVAudioSessionCategoryPlayback);

        NSUInteger options = AVAudioSessionCategoryOptionMixWithOthers; // Alternatively AVAudioSessionCategoryOptionDuckOthers
        if (useInputs) // These options are only valid for category = PlayAndRecord
            options |= (AVAudioSessionCategoryOptionDefaultToSpeaker | AVAudioSessionCategoryOptionAllowBluetooth);

        JUCE_NSERROR_CHECK ([session setCategory: category
                                     withOptions: options
                                           error: &error]);

        fixAudioRouteIfSetToReceiver();

        // Set the sample rate
        trySampleRate (targetSampleRate);
        updateSampleRateAndAudioInput();
        updateCurrentBufferSize();

        prepareFloatBuffers (actualBufferSize);

        isRunning = true;
        handleRouteChange ("Started AudioUnit");

        lastError = (audioUnit != 0 ? "" : "Couldn't open the device");

        setAudioSessionActive (true);

        return lastError;
    }

    void close() override
    {
        if (isRunning)
        {
            isRunning = false;

            if (audioUnit != 0)
            {
                AudioOutputUnitStart (audioUnit);
                AudioComponentInstanceDispose (audioUnit);
                audioUnit = 0;
            }

            setAudioSessionActive (false);
        }
    }

    bool isOpen() override                      { return isRunning; }

    int getCurrentBufferSizeSamples() override  { return actualBufferSize; }
    double getCurrentSampleRate() override      { return sampleRate; }
    int getCurrentBitDepth() override           { return 16; }

    BigInteger getActiveOutputChannels() const override    { return activeOutputChans; }
    BigInteger getActiveInputChannels() const override     { return activeInputChans; }

    int getOutputLatencyInSamples() override    { return roundToInt (getCurrentSampleRate() * [AVAudioSession sharedInstance].outputLatency); }
    int getInputLatencyInSamples()  override    { return roundToInt (getCurrentSampleRate() * [AVAudioSession sharedInstance].inputLatency); }

    void start (AudioIODeviceCallback* newCallback) override
    {
        if (isRunning && callback != newCallback)
        {
            if (newCallback != nullptr)
                newCallback->audioDeviceAboutToStart (this);

            const ScopedLock sl (callbackLock);
            callback = newCallback;
        }
    }

    void stop() override
    {
        if (isRunning)
        {
            AudioIODeviceCallback* lastCallback;

            {
                const ScopedLock sl (callbackLock);
                lastCallback = callback;
                callback = nullptr;
            }

            if (lastCallback != nullptr)
                lastCallback->audioDeviceStopped();
        }
    }

    bool isPlaying() override            { return isRunning && callback != nullptr; }
    String getLastError() override       { return lastError; }

    bool setAudioPreprocessingEnabled (bool enable) override
    {
        auto session = [AVAudioSession sharedInstance];

        NSString* mode = (enable ? AVAudioSessionModeMeasurement
                                 : AVAudioSessionModeDefault);

        JUCE_NSERROR_CHECK ([session setMode: mode
                                       error: &error]);

        return session.mode == mode;
    }

    void invokeAudioDeviceErrorCallback (const String& reason)
    {
        const ScopedLock sl (callbackLock);

        if (callback != nullptr)
            callback->audioDeviceError (reason);
    }

    void handleStatusChange (bool enabled, const char* reason)
    {
        JUCE_IOS_AUDIO_LOG ("handleStatusChange: enabled: " << (int) enabled << ", reason: " << reason);

        isRunning = enabled;
        setAudioSessionActive (enabled);

        if (enabled)
            AudioOutputUnitStart (audioUnit);
        else
            AudioOutputUnitStop (audioUnit);

        if (! enabled)
            invokeAudioDeviceErrorCallback (reason);
    }

    void handleRouteChange (const char* reason)
    {
        JUCE_IOS_AUDIO_LOG ("handleRouteChange: reason: " << reason);

        fixAudioRouteIfSetToReceiver();

        if (isRunning)
        {
            invokeAudioDeviceErrorCallback (reason);
            updateSampleRateAndAudioInput();
            updateCurrentBufferSize();
            createAudioUnit();

            setAudioSessionActive (true);

            if (audioUnit != 0)
            {
                UInt32 formatSize = sizeof (format);
                AudioUnitGetProperty (audioUnit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Output, 1, &format, &formatSize);
                AudioOutputUnitStart (audioUnit);
            }

            if (callback)
                callback->audioDeviceAboutToStart (this);
        }
    }

private:
    //==================================================================================================
    SharedResourcePointer<AudioSessionHolder> sessionHolder;
    CriticalSection callbackLock;
    NSTimeInterval sampleRate = 0;
    int numInputChannels = 2, numOutputChannels = 2;
    int preferredBufferSize = 0, actualBufferSize = 0;
    bool isRunning = false;
    String lastError;

    AudioStreamBasicDescription format;
    AudioUnit audioUnit {};
    bool audioInputIsAvailable = false;
    AudioIODeviceCallback* callback = nullptr;
    BigInteger activeOutputChans, activeInputChans;

    AudioSampleBuffer floatData;
    float* inputChannels[3];
    float* outputChannels[3];
    bool monoInputChannelNumber, monoOutputChannelNumber;

    void prepareFloatBuffers (int bufferSize)
    {
        if (numInputChannels + numOutputChannels > 0)
        {
            floatData.setSize (numInputChannels + numOutputChannels, bufferSize);
            zeromem (inputChannels, sizeof (inputChannels));
            zeromem (outputChannels, sizeof (outputChannels));

            for (int i = 0; i < numInputChannels; ++i)
                inputChannels[i] = floatData.getWritePointer (i);

            for (int i = 0; i < numOutputChannels; ++i)
                outputChannels[i] = floatData.getWritePointer (i + numInputChannels);
        }
    }

    //==================================================================================================
    OSStatus process (AudioUnitRenderActionFlags* flags, const AudioTimeStamp* time,
                      const UInt32 numFrames, AudioBufferList* data)
    {
        OSStatus err = noErr;

        if (audioInputIsAvailable && numInputChannels > 0)
            err = AudioUnitRender (audioUnit, flags, time, 1, numFrames, data);

        const ScopedLock sl (callbackLock);

        if (callback != nullptr)
        {
            if ((int) numFrames > floatData.getNumSamples())
                prepareFloatBuffers ((int) numFrames);

            if (audioInputIsAvailable && numInputChannels > 0)
            {
                short* shortData = (short*) data->mBuffers[0].mData;

                if (numInputChannels >= 2)
                {
                    for (UInt32 i = 0; i < numFrames; ++i)
                    {
                        inputChannels[0][i] = *shortData++ * (1.0f / 32768.0f);
                        inputChannels[1][i] = *shortData++ * (1.0f / 32768.0f);
                    }
                }
                else
                {
                    if (monoInputChannelNumber > 0)
                        ++shortData;

                    for (UInt32 i = 0; i < numFrames; ++i)
                    {
                        inputChannels[0][i] = *shortData++ * (1.0f / 32768.0f);
                        ++shortData;
                    }
                }
            }
            else
            {
                for (int i = numInputChannels; --i >= 0;)
                    zeromem (inputChannels[i], sizeof (float) * numFrames);
            }

            callback->audioDeviceIOCallback ((const float**) inputChannels, numInputChannels,
                                             outputChannels, numOutputChannels, (int) numFrames);

            short* const shortData = (short*) data->mBuffers[0].mData;
            int n = 0;

            if (numOutputChannels >= 2)
            {
                for (UInt32 i = 0; i < numFrames; ++i)
                {
                    shortData [n++] = (short) (outputChannels[0][i] * 32767.0f);
                    shortData [n++] = (short) (outputChannels[1][i] * 32767.0f);
                }
            }
            else if (numOutputChannels == 1)
            {
                for (UInt32 i = 0; i < numFrames; ++i)
                {
                    const short s = (short) (outputChannels[monoOutputChannelNumber][i] * 32767.0f);
                    shortData [n++] = s;
                    shortData [n++] = s;
                }
            }
            else
            {
                zeromem (data->mBuffers[0].mData, 2 * sizeof (short) * numFrames);
            }
        }
        else
        {
            zeromem (data->mBuffers[0].mData, 2 * sizeof (short) * numFrames);
        }

        return err;
    }

    void updateSampleRateAndAudioInput()
    {
        auto session = [AVAudioSession sharedInstance];
        sampleRate = session.sampleRate;
        audioInputIsAvailable = session.isInputAvailable;
        JUCE_IOS_AUDIO_LOG ("AVAudioSession: sampleRate: " << sampleRate << "Hz, audioInputAvailable: " << (int) audioInputIsAvailable);
    }

    void updateCurrentBufferSize()
    {
        auto session = [AVAudioSession sharedInstance];
        NSTimeInterval bufferDuration = sampleRate > 0 ? (NSTimeInterval) (preferredBufferSize / sampleRate) : 0.0;
        JUCE_NSERROR_CHECK ([session setPreferredIOBufferDuration: bufferDuration
                                                            error: &error]);

        bufferDuration = session.IOBufferDuration;
        actualBufferSize = roundToInt (sampleRate * bufferDuration);
    }

    //==================================================================================================
    static OSStatus processStatic (void* client, AudioUnitRenderActionFlags* flags, const AudioTimeStamp* time,
                                   UInt32 /*busNumber*/, UInt32 numFrames, AudioBufferList* data)
    {

        return static_cast<iOSAudioIODevice*> (client)->process (flags, time, numFrames, data);
    }

    //==================================================================================================
    void resetFormat (const int numChannels) noexcept
    {
        zerostruct (format);
        format.mFormatID = kAudioFormatLinearPCM;
        format.mFormatFlags = kLinearPCMFormatFlagIsSignedInteger | kLinearPCMFormatFlagIsPacked | kAudioFormatFlagsNativeEndian;
        format.mBitsPerChannel = 8 * sizeof (short);
        format.mChannelsPerFrame = (UInt32) numChannels;
        format.mFramesPerPacket = 1;
        format.mBytesPerFrame = format.mBytesPerPacket = (UInt32) numChannels * sizeof (short);
    }

    bool createAudioUnit()
    {
        if (audioUnit != 0)
        {
            AudioComponentInstanceDispose (audioUnit);
            audioUnit = 0;
        }

        resetFormat (2);

        AudioComponentDescription desc;
        desc.componentType = kAudioUnitType_Output;
        desc.componentSubType = kAudioUnitSubType_RemoteIO;
        desc.componentManufacturer = kAudioUnitManufacturer_Apple;
        desc.componentFlags = 0;
        desc.componentFlagsMask = 0;

        AudioComponent comp = AudioComponentFindNext (0, &desc);
        AudioComponentInstanceNew (comp, &audioUnit);

        if (audioUnit == 0)
            return false;

        if (numInputChannels > 0)
        {
            const UInt32 one = 1;
            AudioUnitSetProperty (audioUnit, kAudioOutputUnitProperty_EnableIO, kAudioUnitScope_Input, 1, &one, sizeof (one));
        }

        {
            AudioChannelLayout layout;
            layout.mChannelBitmap = 0;
            layout.mNumberChannelDescriptions = 0;
            layout.mChannelLayoutTag = kAudioChannelLayoutTag_Stereo;
            AudioUnitSetProperty (audioUnit, kAudioUnitProperty_AudioChannelLayout, kAudioUnitScope_Input,  0, &layout, sizeof (layout));
            AudioUnitSetProperty (audioUnit, kAudioUnitProperty_AudioChannelLayout, kAudioUnitScope_Output, 0, &layout, sizeof (layout));
        }

        {
            AURenderCallbackStruct inputProc;
            inputProc.inputProc = processStatic;
            inputProc.inputProcRefCon = this;
            AudioUnitSetProperty (audioUnit, kAudioUnitProperty_SetRenderCallback, kAudioUnitScope_Input, 0, &inputProc, sizeof (inputProc));
        }

        AudioUnitSetProperty (audioUnit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Input,  0, &format, sizeof (format));
        AudioUnitSetProperty (audioUnit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Output, 1, &format, sizeof (format));

        AudioUnitInitialize (audioUnit);
        return true;
    }

    // If the routing is set to go through the receiver (i.e. the speaker, but quiet), this re-routes it
    // to make it loud. Needed because by default when using an input + output, the output is kept quiet.
    static void fixAudioRouteIfSetToReceiver()
    {
        auto session = [AVAudioSession sharedInstance];
        auto route = session.currentRoute;

        for (AVAudioSessionPortDescription* port in route.inputs)
        {
            ignoreUnused (port);
            JUCE_IOS_AUDIO_LOG ("AVAudioSession: input: " << [port.description UTF8String]);
        }

        for (AVAudioSessionPortDescription* port in route.outputs)
        {
            JUCE_IOS_AUDIO_LOG ("AVAudioSession: output: " << [port.description UTF8String]);

            if ([port.portName isEqualToString: @"Receiver"])
            {
                JUCE_NSERROR_CHECK ([session overrideOutputAudioPort: AVAudioSessionPortOverrideSpeaker
                                                               error: &error]);
                setAudioSessionActive (true);
            }
        }
    }

    JUCE_DECLARE_NON_COPYABLE (iOSAudioIODevice)
};


//==============================================================================
class iOSAudioIODeviceType  : public AudioIODeviceType
{
public:
    iOSAudioIODeviceType()  : AudioIODeviceType ("iOS Audio") {}

    void scanForDevices() {}
    StringArray getDeviceNames (bool /*wantInputNames*/) const       { return StringArray ("iOS Audio"); }
    int getDefaultDeviceIndex (bool /*forInput*/) const              { return 0; }
    int getIndexOfDevice (AudioIODevice* d, bool /*asInput*/) const  { return d != nullptr ? 0 : -1; }
    bool hasSeparateInputsAndOutputs() const                         { return false; }

    AudioIODevice* createDevice (const String& outputDeviceName, const String& inputDeviceName)
    {
        if (outputDeviceName.isNotEmpty() || inputDeviceName.isNotEmpty())
            return new iOSAudioIODevice (outputDeviceName.isNotEmpty() ? outputDeviceName : inputDeviceName);

        return nullptr;
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (iOSAudioIODeviceType)
};

//==============================================================================
AudioIODeviceType* AudioIODeviceType::createAudioIODeviceType_iOSAudio()
{
    return new iOSAudioIODeviceType();
}

//==================================================================================================
AudioSessionHolder::AudioSessionHolder()    { nativeSession = [[iOSAudioSessionNative alloc] init: this]; }
AudioSessionHolder::~AudioSessionHolder()   { [nativeSession release]; }

void AudioSessionHolder::handleStatusChange (bool enabled, const char* reason) const
{
    for (auto device: activeDevices)
        device->handleStatusChange (enabled, reason);
}

void AudioSessionHolder::handleRouteChange (const char* reason) const
{
    for (auto device: activeDevices)
        device->handleRouteChange (reason);
}

#undef JUCE_NSERROR_CHECK

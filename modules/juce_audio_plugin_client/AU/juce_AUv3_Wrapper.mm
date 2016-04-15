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

#if JucePlugin_Build_AUv3

#import <CoreAudioKit/CoreAudioKit.h>
#import <AudioToolbox/AudioToolbox.h>
#import <AVFoundation/AVFoundation.h>

#ifndef __OBJC2__
 #error AUv3 needs Objective-C 2 support (compile with 64-bit)
#endif

#include "../utility/juce_IncludeSystemHeaders.h"
#include "../utility/juce_IncludeModuleHeaders.h"
#include "../../juce_core/native/juce_osx_ObjCHelpers.h"
#include "../utility/juce_PluginBusUtilities.h"
#include "../../juce_graphics/native/juce_mac_CoreGraphicsHelpers.h"

#include "juce_AU_Shared.h"

#define JUCE_VIEWCONTROLLER_OBJC_NAME(x) JUCE_JOIN_MACRO (x, FactoryAUv3)

#if ! JUCE_COMPILER_SUPPORTS_VARIADIC_TEMPLATES
 #error AUv3 wrapper requires variadic template support
#endif

#if JUCE_IOS
#define IOS_MAC_VIEW  UIView
#else
#define IOS_MAC_VIEW  NSView
#endif

#define JUCE_AUDIOUNIT_OBJC_NAME(x) JUCE_JOIN_MACRO (x, AUv3)

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wnullability-completeness"

JUCE_DEFINE_WRAPPER_TYPE (wrapperType_AudioUnitv3);

// TODO: ask Timur: use SFINAE to automatically generate this for all NSObjects
template <> struct ContainerDeletePolicy<AUAudioUnitBusArray> { static void destroy (NSObject* o) { [o release]; } };
template <> struct ContainerDeletePolicy<AUParameterTree> { static void destroy (NSObject* o) { [o release]; } };
template <> struct ContainerDeletePolicy<NSMutableArray<AUParameterNode *> > { static void destroy (NSObject* o) { [o release]; } };
template <> struct ContainerDeletePolicy<AUParameter> { static void destroy (NSObject* o) { [o release]; } };
template <> struct ContainerDeletePolicy<NSMutableArray<AUAudioUnitBus*> > { static void destroy (NSObject* o) { [o release]; } };
template <> struct ContainerDeletePolicy<AUAudioUnitBus> { static void destroy (NSObject* o) { [o release]; } };
template <> struct ContainerDeletePolicy<AVAudioFormat> { static void destroy (NSObject* o) { [o release]; } };
template <> struct ContainerDeletePolicy<AVAudioPCMBuffer> { static void destroy (NSObject* o) { [o release]; } };
template <> struct ContainerDeletePolicy<NSMutableArray<NSNumber*> > { static void destroy (NSObject* o) { [o release]; } };
template <> struct ContainerDeletePolicy<NSNumber> { static void destroy (NSObject* o) { [o release]; } };
template <> struct ContainerDeletePolicy<NSMutableArray<AUAudioUnitPreset*> > { static void destroy (NSObject* o) { [o release]; } };
template <> struct ContainerDeletePolicy<AUAudioUnitPreset> { static void destroy (NSObject* o) { [o release]; } };

//==============================================================================
struct AudioProcessorHolder  : public ReferenceCountedObject
{
    AudioProcessorHolder() {}
    AudioProcessorHolder (AudioProcessor* p) : juceFilter (p) {}
    AudioProcessor& operator*() noexcept        { return *juceFilter; }
    AudioProcessor* operator->() noexcept       { return juceFilter; }
    AudioProcessor* get() noexcept              { return juceFilter; }

    typedef ReferenceCountedObjectPtr<AudioProcessorHolder> Ptr;

private:
    ScopedPointer<AudioProcessor> juceFilter;

    AudioProcessorHolder& operator= (AudioProcessor*) JUCE_DELETED_FUNCTION;
    AudioProcessorHolder (AudioProcessorHolder&) JUCE_DELETED_FUNCTION;
    AudioProcessorHolder& operator= (AudioProcessorHolder&) JUCE_DELETED_FUNCTION;
};

//==============================================================================
class JuceAudioUnitv3Base
{
public:
    JuceAudioUnitv3Base (const AudioComponentDescription& descr, AudioComponentInstantiationOptions options,
                         NSError** error)
       #pragma clang diagnostic push
       #pragma clang diagnostic ignored "-Wobjc-method-access"
        : au ([audioUnitObjCClass.createInstance() initWithComponentDescription: descr
                                                                        options: options
                                                                          error: error
                                                                      juceClass: this])
       #pragma clang diagnostic pop
    {
    }

    JuceAudioUnitv3Base (AUAudioUnit* audioUnit) : au (audioUnit)
    {
        jassert (MessageManager::getInstance()->isThisTheMessageThread());
        initialiseJuce_GUI();
    }

    //==============================================================================
    AUAudioUnit* getAudioUnit() noexcept                                   { return au; }
    virtual int getVirtualMIDICableCount()                                 { return 0; }
    virtual void reset()                                                   {}
    virtual bool shouldChangeToFormat (AVAudioFormat*, AUAudioUnitBus*)    { return true; }
    virtual AUAudioUnitPreset* getCurrentPreset()                          { return nullptr; }
    virtual void setCurrentPreset(AUAudioUnitPreset*)                      {}
    virtual NSTimeInterval getLatency()                                    { return 0.0; }
    virtual NSTimeInterval getTailTime()                                   { return 0.0; }
    virtual bool getCanProcessInPlace()                                    { return false; }
    virtual bool getRenderingOffline()                                     { return false; }

    //==============================================================================
    virtual AUAudioUnitBusArray* getInputBusses()          = 0;
    virtual AUAudioUnitBusArray* getOutputBusses()         = 0;
    virtual AUParameterTree* getParameterTree()            = 0;
    virtual AUInternalRenderBlock getInternalRenderBlock() = 0;
    virtual void setRenderingOffline (bool offline)        = 0;
    virtual NSArray<NSNumber*> *getChannelCapabilities()   = 0;

    //==============================================================================
    virtual NSArray<NSNumber*>* parametersForOverviewWithCount (int)
    {
        return [NSArray<NSNumber*> array];
    }

    virtual NSArray<AUAudioUnitPreset*>* getFactoryPresets()
    {
        return [NSArray<AUAudioUnitPreset*> array];
    }

    virtual NSDictionary<NSString*, id>* getFullState()
    {
        objc_super s = { getAudioUnit(), [AUAudioUnit class] };
        return ObjCMsgSendSuper<NSDictionary<NSString*, id>*> (&s, @selector (fullState));
    }

    virtual void setFullState (NSDictionary<NSString*, id>* state)
    {
        objc_super s = { getAudioUnit(), [AUAudioUnit class] };
        ObjCMsgSendSuper<void, NSDictionary<NSString*, id>*> (&s, @selector (setFullState:), state);
    }

    virtual bool allocateRenderResourcesAndReturnError (NSError **outError)
    {
        objc_super s = { getAudioUnit(), [AUAudioUnit class] };
        if (! ObjCMsgSendSuper<BOOL, NSError**> (&s, @selector (allocateRenderResourcesAndReturnError:), outError))
            return false;

        return true;
    }

    virtual void deallocateRenderResources()
    {
        objc_super s = { getAudioUnit(), [AUAudioUnit class] };
        ObjCMsgSendSuper<void> (&s, @selector (deallocateRenderResources));
    }

    //==============================================================================
protected:
    virtual ~JuceAudioUnitv3Base() {}

private:
    struct Class : public ObjCClass<AUAudioUnit>
    {
        Class() : ObjCClass<AUAudioUnit> ("AUAudioUnit_")
        {
            addIvar<JuceAudioUnitv3Base*> ("cppObject");

            addMethod (@selector (initWithComponentDescription:options:error:juceClass:),
                       initWithComponentDescriptionAndJuceClass, "@@:",
                       @encode (AudioComponentDescription),
                       @encode (AudioComponentInstantiationOptions), "^@@");

            addMethod (@selector (initWithComponentDescription:options:error:),
                       initWithComponentDescription, "@@:",
                       @encode (AudioComponentDescription),
                       @encode (AudioComponentInstantiationOptions), "^@");

            addMethod (@selector (dealloc),                                 dealloc,                               "v@:");
            addMethod (@selector (inputBusses),                             getInputBusses,                        "@@:");
            addMethod (@selector (outputBusses),                            getOutputBusses,                       "@@:");
            addMethod (@selector (parameterTree),                           getParameterTree,                      "@@:");
            addMethod (@selector (deallocateRenderResources),               deallocateRenderResources,             "v@:");
            addMethod (@selector (reset),                                   reset,                                 "v@:");
            addMethod (@selector (shouldChangeToFormat:forBus:),            shouldChangeToFormat,                  "B@:@@");
            addMethod (@selector (factoryPresets),                          getFactoryPresets,                     "@@:");
            addMethod (@selector (currentPreset),                           getCurrentPreset,                      "@@:");
            addMethod (@selector (setCurrentPreset:),                       setCurrentPreset,                      "v@:@");
            addMethod (@selector (fullState),                               getFullState,                          "@@:");
            addMethod (@selector (setFullState:),                           setFullState,                          "v@:@");
            addMethod (@selector (channelCapabilities),                     getChannelCapabilities,                "@@:");
            addMethod (@selector (allocateRenderResourcesAndReturnError:),  allocateRenderResourcesAndReturnError, "B@:^@");

            addMethod (@selector (parametersForOverviewWithCount:), parametersForOverviewWithCount, "@@:", @encode (NSInteger));
            addMethod (@selector (setRenderingOffline:),            setRenderingOffline,            "v@:", @encode (BOOL));


            addMethod (@selector (internalRenderBlock),   getInternalRenderBlock,   @encode (AUInternalRenderBlock), "@:");
            addMethod (@selector (virtualMIDICableCount), getVirtualMIDICableCount, @encode (NSInteger),             "@:");
            addMethod (@selector (latency),               getLatency,               @encode (NSTimeInterval),        "@:");
            addMethod (@selector (tailTime),              getTailTime,              @encode (NSTimeInterval),        "@:");
            addMethod (@selector (canProcessInPlace),     getCanProcessInPlace,     @encode (BOOL),                  "@:");
            addMethod (@selector (isRenderingOffline),    getRenderingOffline,      @encode (BOOL),                  "@:");

            registerClass();
        }

        //==============================================================================
        static JuceAudioUnitv3Base* _this (id self)                     { return getIvar<JuceAudioUnitv3Base*> (self, "cppObject"); }
        static void setThis (id self, JuceAudioUnitv3Base* cpp)         { object_setInstanceVariable           (self, "cppObject", cpp); }

        //==============================================================================
        static id initWithComponentDescription (id _self, SEL, AudioComponentDescription descr, AudioComponentInstantiationOptions options, NSError** error)
        {
            AUAudioUnit* self = _self;

            objc_super s = { self, [AUAudioUnit class] };
            self = ObjCMsgSendSuper<AUAudioUnit*, AudioComponentDescription,
                                    AudioComponentInstantiationOptions, NSError**> (&s, @selector(initWithComponentDescription:options:error:), descr, options, error);

            JuceAudioUnitv3Base* juceAU = JuceAudioUnitv3Base::create (self, descr, options, error);

            setThis (self, juceAU);
            return self;
        }

        static id initWithComponentDescriptionAndJuceClass (id _self, SEL, AudioComponentDescription descr, AudioComponentInstantiationOptions options, NSError** error, JuceAudioUnitv3Base* juceAU)
        {
            AUAudioUnit* self = _self;

            objc_super s = { self, [AUAudioUnit class] };
            self = ObjCMsgSendSuper<AUAudioUnit*, AudioComponentDescription,
                                    AudioComponentInstantiationOptions, NSError**> (&s, @selector(initWithComponentDescription:options:error:), descr, options, error);


            setThis (self, juceAU);
            return self;
        }

        static void dealloc (id self, SEL)                                                          { delete _this (self); }
        static AUAudioUnitBusArray* getInputBusses   (id self, SEL)                                 { return _this (self)->getInputBusses(); }
        static AUAudioUnitBusArray* getOutputBusses  (id self, SEL)                                 { return _this (self)->getOutputBusses(); }
        static AUParameterTree*     getParameterTree (id self, SEL)                                 { return _this (self)->getParameterTree(); }
        static AUInternalRenderBlock getInternalRenderBlock (id self, SEL)                          { return _this (self)->getInternalRenderBlock();  }
        static BOOL allocateRenderResourcesAndReturnError (id self, SEL, NSError** error)           { return _this (self)->allocateRenderResourcesAndReturnError (error); }
        static void deallocateRenderResources (id self, SEL)                                        { _this (self)->deallocateRenderResources(); }
        static void reset (id self, SEL)                                                            { _this (self)->reset(); }
        static NSInteger getVirtualMIDICableCount (id self, SEL)                                    { return _this (self)->getVirtualMIDICableCount(); }
        static BOOL shouldChangeToFormat (id self, SEL, AVAudioFormat* format, AUAudioUnitBus* bus) { return _this (self)->shouldChangeToFormat (format, bus); }
        static NSArray<NSNumber*>* parametersForOverviewWithCount (id self, SEL, NSInteger count)   { return _this (self)->parametersForOverviewWithCount (static_cast<int> (count)); }
        static NSArray<AUAudioUnitPreset*>* getFactoryPresets (id self, SEL)                        { return _this (self)->getFactoryPresets(); }
        static AUAudioUnitPreset* getCurrentPreset (id self, SEL)                                   { return _this (self)->getCurrentPreset(); }
        static void setCurrentPreset (id self, SEL, AUAudioUnitPreset* preset)                      { return _this (self)->setCurrentPreset (preset); }
        static NSDictionary<NSString *, id>* getFullState (id self, SEL)                            { return _this (self)->getFullState(); }
        static void setFullState (id self, SEL, NSDictionary<NSString *, id>* state)                { return _this (self)->setFullState (state); }
        static NSTimeInterval getLatency (id self, SEL)                                             { return _this (self)->getLatency(); }
        static NSTimeInterval getTailTime (id self, SEL)                                            { return _this (self)->getTailTime(); }
        static BOOL getCanProcessInPlace (id self, SEL)                                             { return _this (self)->getCanProcessInPlace(); }
        static BOOL getRenderingOffline (id self, SEL)                                              { return _this (self)->getRenderingOffline(); }
        static void setRenderingOffline (id self, SEL, BOOL renderingOffline)                       { _this (self)->setRenderingOffline (renderingOffline); }
        static NSArray<NSNumber *>* getChannelCapabilities (id self, SEL)                           { return _this (self)->getChannelCapabilities(); }
    };

    static JuceAudioUnitv3Base* create (AUAudioUnit* audioUnit, AudioComponentDescription descr, AudioComponentInstantiationOptions options, NSError** error);

    //==============================================================================
    static Class audioUnitObjCClass;

protected:
    AUAudioUnit* au;
};

//==============================================================================
JuceAudioUnitv3Base::Class JuceAudioUnitv3Base::audioUnitObjCClass;

//==============================================================================
//=========================== The actual AudioUnit =============================
//==============================================================================
class JuceAudioUnitv3 : public JuceAudioUnitv3Base, public AudioProcessorListener, public AudioPlayHead
{
public:
    JuceAudioUnitv3 (const AudioProcessorHolder::Ptr& jFilter,
                     const AudioComponentDescription& descr,
                     AudioComponentInstantiationOptions options,
                     NSError** error)
        : JuceAudioUnitv3Base (descr, options, error),
          juceFilter (jFilter), busUtils (**juceFilter, true, 8),
          mapper (busUtils)
    {
        init();
    }

    JuceAudioUnitv3 (AUAudioUnit* audioUnit, AudioComponentDescription, AudioComponentInstantiationOptions, NSError**)
        : JuceAudioUnitv3Base (audioUnit),
          juceFilter (new AudioProcessorHolder (createPluginFilterOfType (AudioProcessor::wrapperType_AudioUnitv3))),
          busUtils (**juceFilter, true, 8),
          mapper (busUtils)
    {
        init();
    }

    //==============================================================================
    void init()
    {
        busUtils.init();

        (*juceFilter)->setPlayHead (this);

        totalInChannels  = busUtils.findTotalNumChannels (true);
        totalOutChannels = busUtils.findTotalNumChannels (false);

        {
            channelCapabilities = [[NSMutableArray<NSNumber*> alloc] init];
            Array<AUChannelInfo> channelInfo = AudioUnitHelpers::getAUChannelInfo (busUtils);

            for (int i = 0; i < channelInfo.size(); ++i)
            {
                AUChannelInfo& info = channelInfo.getReference (i);


                [channelCapabilities addObject: [NSNumber numberWithInteger: info.inChannels]];
                [channelCapabilities addObject: [NSNumber numberWithInteger: info.outChannels]];
            }
        }

        editorObserverToken = nullptr;
        internalRenderBlock = CreateObjCBlock (this, &JuceAudioUnitv3::renderCallback);

        const AUAudioFrameCount maxFrames = [getAudioUnit() maximumFramesToRender];
        (*juceFilter)->setRateAndBufferSizeDetails (kDefaultSampleRate, static_cast<int> (maxFrames));
        (*juceFilter)->prepareToPlay (kDefaultSampleRate, static_cast<int> (maxFrames));

        (*juceFilter)->addListener (this);

        addParameters();
        addPresets();

        addAudioUnitBusses (true);
        addAudioUnitBusses (false);
    }

    //==============================================================================
    AUAudioUnitBusArray* getInputBusses() override            { return inputBusses;  }
    AUAudioUnitBusArray* getOutputBusses() override           { return outputBusses; }
    AUParameterTree*     getParameterTree() override          { return paramTree; }
    AUInternalRenderBlock getInternalRenderBlock() override   { return internalRenderBlock;  }
    NSArray<AUAudioUnitPreset*>* getFactoryPresets() override { return factoryPresets; }
    bool getRenderingOffline() override                       { return (*juceFilter)->isNonRealtime(); }
    void setRenderingOffline (bool offline) override          { (*juceFilter)->setNonRealtime (offline); }
    NSArray<NSNumber *>* getChannelCapabilities() override    { return channelCapabilities; }

    //==============================================================================
    AUAudioUnitPreset* getCurrentPreset() override
    {
        const int n = static_cast<int> ([factoryPresets count]);
        const int idx = static_cast<int> ((*juceFilter)->getCurrentProgram());

        if (idx < n)
            return [factoryPresets objectAtIndex:static_cast<unsigned int> (idx)];

        return nullptr;
    }

    void setCurrentPreset(AUAudioUnitPreset* preset) override
    {
        const int n = static_cast<int> ([factoryPresets count]);
        const int idx = static_cast<int> ([preset number]);
        if (isPositiveAndBelow (idx, n))
            (*juceFilter)->setCurrentProgram (idx);
    }

    //==============================================================================
    NSDictionary<NSString*, id>* getFullState() override
    {
        NSMutableDictionary<NSString*, id>* retval = [[NSMutableDictionary<NSString*, id> alloc] init];

        {
            NSDictionary<NSString*, id>* superRetval = JuceAudioUnitv3Base::getFullState();

            if (superRetval != nullptr)
                [retval addEntriesFromDictionary:superRetval];
        }

        juce::MemoryBlock state;
        (*juceFilter)->getCurrentProgramStateInformation (state);
        if (state.getSize() > 0)
        {
            NSData* ourState = [[NSData alloc] initWithBytes: state.getData()
                                                      length: state.getSize()];

            NSString* nsKey = [[NSString alloc] initWithUTF8String: JUCE_STATE_DICTIONARY_KEY];

            [retval setObject: ourState
                       forKey: nsKey];

            [nsKey    release];
            [ourState release];
        }

        return [retval autorelease];
    }

    void setFullState (NSDictionary<NSString*, id>* state) override
    {
        if (state == nullptr)
            return;

        NSMutableDictionary<NSString*, id>* modifiedState = [[NSMutableDictionary<NSString*, id> alloc] init];
        [modifiedState addEntriesFromDictionary: state];

        NSString* nsPresetKey = [[NSString alloc] initWithUTF8String: kAUPresetDataKey];
        [modifiedState removeObjectForKey: nsPresetKey];
        [nsPresetKey release];

        JuceAudioUnitv3Base::setFullState (modifiedState);

        NSString* nsKey = [[NSString alloc] initWithUTF8String: JUCE_STATE_DICTIONARY_KEY];
        NSObject* obj = [modifiedState objectForKey: nsKey];
        [nsKey release];

        if (obj != nullptr)
        {
            if ([obj isKindOfClass:[NSData class]])
            {
                NSData* data = reinterpret_cast<NSData*> (obj);
                const int numBytes = static_cast<int> ([data length]);
                const juce::uint8* const rawBytes = reinterpret_cast< const juce::uint8* const> ([data bytes]);

                if (numBytes > 0)
                    (*juceFilter)->setCurrentProgramStateInformation (rawBytes, numBytes);
            }
        }

        [modifiedState release];
    }

    //==============================================================================
    NSArray<NSNumber*>* parametersForOverviewWithCount (int count) override
    {
        const int n = static_cast<int> ([overviewParams count]);

        if (count >= n)
            return overviewParams;

        NSMutableArray<NSNumber*>* retval = [[NSMutableArray<NSNumber*>alloc] initWithArray: overviewParams];
        [retval removeObjectsInRange: NSMakeRange (static_cast<unsigned int> (count), static_cast<unsigned int> (n - count))];

        return [retval autorelease];
    }

    int getVirtualMIDICableCount() override
    {
       #if JucePlugin_WantsMidiInput
        return 1;
       #else
        return 0;
       #endif
    }

    //==============================================================================
    bool allocateRenderResourcesAndReturnError (NSError **outError) override
    {
        const AUAudioFrameCount maxFrames = [getAudioUnit() maximumFramesToRender];

        if (! JuceAudioUnitv3Base::allocateRenderResourcesAndReturnError (outError))
            return false;

        totalInChannels = busUtils.findTotalNumChannels (true);
        totalOutChannels = busUtils.findTotalNumChannels (false);

        allocateBusBuffer (true);
        allocateBusBuffer (false);

        mapper.alloc();

        audioBuffer.prepare (totalInChannels, totalOutChannels, static_cast<int> (maxFrames));

        double sampleRate = (jmax (busUtils.getBusCount (true), busUtils.getBusCount (false)) > 0 ?
                             [[[([inputBusses count] > 0 ? inputBusses : outputBusses) objectAtIndexedSubscript: 0] format] sampleRate] : 44100.0);

        (*juceFilter)->setRateAndBufferSizeDetails (sampleRate, static_cast<int> (maxFrames));
        (*juceFilter)->prepareToPlay (sampleRate, static_cast<int> (maxFrames));

        zeromem (&lastAudioHead, sizeof (lastAudioHead));
        hostMusicalContextCallback = [getAudioUnit() musicalContextBlock];
        hostTransportStateCallback = [getAudioUnit() transportStateBlock];

        reset();

        return true;
    }

    void deallocateRenderResources() override
    {
        hostMusicalContextCallback = nullptr;
        hostTransportStateCallback = nullptr;

        (*juceFilter)->releaseResources();
        audioBuffer.release();

        inBusBuffers. clear();
        outBusBuffers.clear();

        mapper.release();

        JuceAudioUnitv3Base::deallocateRenderResources();
    }

    void reset() override
    {
        midiMessages.clear();
        lastTimeStamp.mSampleTime = std::numeric_limits<Float64>::max();
    }

    //==============================================================================
    bool shouldChangeToFormat (AVAudioFormat* format, AUAudioUnitBus* bus) override
    {
        const bool isInput = ([bus busType] == AUAudioUnitBusTypeInput);
        const int busIdx = static_cast<int> ([bus index]);
        const int newNumChannels = static_cast<int> ([format streamDescription]->mChannelsPerFrame);

        AudioChannelSet newLayout;

        if (const AVAudioChannelLayout* layout = [format channelLayout])
            newLayout = AudioUnitHelpers::CALayoutTagToChannelSet ([layout layoutTag]);
        else
            newLayout = busUtils.getDefaultLayoutForChannelNumAndBus(isInput, busIdx, newNumChannels);

        if (newLayout.size() != newNumChannels)
            return false;

        bool success = (*juceFilter)->setPreferredBusArrangement (isInput, busIdx, newLayout);

        totalInChannels  = busUtils.findTotalNumChannels (true);
        totalOutChannels = busUtils.findTotalNumChannels (false);

        return success;
    }

    //==============================================================================
    void audioProcessorChanged (AudioProcessor* processor) override
    {
        ignoreUnused (processor);

        [au willChangeValueForKey: @"allParameterValues"];
        [au didChangeValueForKey: @"allParameterValues"];
    }

    void audioProcessorParameterChanged (AudioProcessor*, int idx, float newValue) override
    {
        if (AUParameter* param = [paramTree parameterWithAddress: static_cast<AUParameterAddress> (idx)])
        {
            if (editorObserverToken != nullptr)
                [param setValue: newValue
                     originator: editorObserverToken];
            else
                [param setValue: newValue];
        }
    }

    //==============================================================================
    NSTimeInterval getLatency() override
    {
        double samples = static_cast<double> ((*juceFilter)->getLatencySamples());
        return samples / (*juceFilter)->getSampleRate();
    }

    NSTimeInterval getTailTime() override             { return (*juceFilter)->getTailLengthSeconds(); }

    //==============================================================================
    bool getCurrentPosition (CurrentPositionInfo& info) override
    {
        bool musicContextCallSucceeded = false;
        bool transportStateCallSucceeded = false;

        info = lastAudioHead;
        info.timeInSamples = (int64) (lastTimeStamp.mSampleTime + 0.5);
        info.timeInSeconds = info.timeInSamples / (*juceFilter)->getSampleRate();

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

        double num;
        NSInteger den;
        NSInteger outDeltaSampleOffsetToNextBeat;
        double outCurrentMeasureDownBeat, bpm;
        double ppqPosition;

        if (hostMusicalContextCallback != nullptr)
        {
            AUHostMusicalContextBlock musicalContextCallback = hostMusicalContextCallback;

            if (musicalContextCallback (&bpm, &num, &den, &ppqPosition, &outDeltaSampleOffsetToNextBeat, &outCurrentMeasureDownBeat))
            {
                musicContextCallSucceeded = true;

                info.timeSigNumerator   = (int) num;
                info.timeSigDenominator = (int) den;
                info.ppqPositionOfLastBarStart = outCurrentMeasureDownBeat;
                info.bpm = bpm;
                info.ppqPosition = ppqPosition;
                info.ppqPositionOfLastBarStart = outCurrentMeasureDownBeat;
            }
        }

        double outCurrentSampleInTimeLine, outCycleStartBeat = 0, outCycleEndBeat = 0;
        AUHostTransportStateFlags flags;

        if (hostTransportStateCallback != nullptr)
        {
            AUHostTransportStateBlock transportStateCallback = hostTransportStateCallback;
            if (transportStateCallback (&flags, &outCurrentSampleInTimeLine, &outCycleStartBeat, &outCycleEndBeat))
            {
                transportStateCallSucceeded = true;
                info.isPlaying = ((flags & AUHostTransportStateMoving) != 0);
                info.timeInSamples = (int64) (outCurrentSampleInTimeLine + 0.5);
                info.timeInSeconds = info.timeInSamples / (*juceFilter)->getSampleRate();
                info.isLooping = ((flags & AUHostTransportStateCycling) != 0);
                info.isRecording = ((flags & AUHostTransportStateRecording) != 0);
                info.ppqLoopStart = outCycleStartBeat;
                info.ppqLoopEnd = outCycleEndBeat;
            }
        }

        if (musicContextCallSucceeded && transportStateCallSucceeded)
            lastAudioHead = info;

        return true;
    }

protected:

    //==============================================================================
    virtual ~JuceAudioUnitv3()
    {
        (*juceFilter)->removeListener (this);

        if (AudioProcessorEditor* editor = (*juceFilter)->getActiveEditor())
            (*juceFilter)->editorBeingDeleted (editor);

        if (editorObserverToken != nullptr)
        {
            [paramTree removeParameterObserver: editorObserverToken];
            editorObserverToken = nullptr;
        }
    }

private:

    //==============================================================================
    class BusBuffer
    {
    public:
        BusBuffer (AUAudioUnitBus* bus, int maxFramesPerBuffer)
            : auBus (bus), bufferList (nullptr),
              maxFrames (maxFramesPerBuffer),
              numberOfChannels (static_cast<int> ([[auBus format] channelCount])),
              isInterleaved ([[auBus format] isInterleaved])
        {
            alloc();
        }

        //==============================================================================
        void alloc()
        {
            const int numBuffers = isInterleaved ? 1 : numberOfChannels;
            int bytes = static_cast<int> (sizeof (AudioBufferList))
                          + ((numBuffers - 1) * static_cast<int> (sizeof (::AudioBuffer)));
            jassert (bytes > 0);

            bufferListStorage.calloc (static_cast<size_t> (bytes));
            bufferList = reinterpret_cast<AudioBufferList*> (bufferListStorage.getData());

            const int bufferChannels = isInterleaved ? numberOfChannels : 1;
            scratchBuffer.setSize (numBuffers, bufferChannels * maxFrames);
        }

        void dealloc()
        {
            bufferList = nullptr;
            bufferListStorage.free();
            scratchBuffer.setSize (0, 0);
        }

        //==============================================================================
        int numChannels() const noexcept                { return numberOfChannels; }
        bool interleaved() const noexcept               { return isInterleaved; }
        AudioBufferList* get() const noexcept           { return bufferList; }

        //==============================================================================
        void prepare (UInt32 nFrames, const AudioBufferList* other = nullptr) noexcept
        {
            const int numBuffers = isInterleaved ? 1 : numberOfChannels;
            const bool isCompatible = compatible (other);

            bufferList->mNumberBuffers = static_cast<UInt32> (numBuffers);

            for (int i = 0; i < numBuffers; ++i)
            {
                const UInt32 bufferChannels = static_cast<UInt32> (isInterleaved ? numberOfChannels : 1);
                bufferList->mBuffers[i].mNumberChannels = bufferChannels;
                bufferList->mBuffers[i].mData = (isCompatible ? other->mBuffers[i].mData : scratchBuffer.getWritePointer (i));
                bufferList->mBuffers[i].mDataByteSize = nFrames * bufferChannels * sizeof (float);
            }
        }

        //==============================================================================
        bool compatible (const AudioBufferList* other) const noexcept
        {
            if (other == nullptr)
                return false;

            if (other->mNumberBuffers > 0)
            {
                const bool otherInterleaved = AudioUnitHelpers::isAudioBufferInterleaved (*other);
                const int otherChannels = static_cast<int> (otherInterleaved ? other->mBuffers[0].mNumberChannels : other->mNumberBuffers);

                return (otherInterleaved == isInterleaved && numberOfChannels == otherChannels);
            }

            return (numberOfChannels == 0);
        }

    private:
        //==============================================================================
        AUAudioUnitBus* auBus;
        HeapBlock<char> bufferListStorage;
        AudioBufferList* bufferList;
        int maxFrames, numberOfChannels;
        bool isInterleaved;
        AudioSampleBuffer scratchBuffer;
    };

    //==============================================================================
    void addAudioUnitBusses (bool isInput)
    {
        ScopedPointer<NSMutableArray<AUAudioUnitBus*> > array = [[NSMutableArray<AUAudioUnitBus*> alloc] init];

        for (int i = 0; i < busUtils.getBusCount (isInput); ++i)
        {
            ScopedPointer<AUAudioUnitBus> audioUnitBus;

            {
                ScopedPointer<AVAudioFormat> defaultFormat = [[AVAudioFormat alloc] initStandardFormatWithSampleRate: kDefaultSampleRate
                                                                                                            channels: static_cast<AVAudioChannelCount> (busUtils.getNumChannels (isInput, i))];

                audioUnitBus = [[AUAudioUnitBus alloc] initWithFormat: defaultFormat
                                                                error: nullptr];
            }

            [array addObject: audioUnitBus];
        }

        (isInput ? inputBusses : outputBusses) = [[AUAudioUnitBusArray alloc] initWithAudioUnit: au
                                                                                        busType: (isInput ? AUAudioUnitBusTypeInput : AUAudioUnitBusTypeOutput)
                                                                                         busses: array];
    }

    void addParameters()
    {
        ScopedPointer<NSMutableArray<AUParameterNode*> > params = [[NSMutableArray<AUParameterNode*> alloc] init];

        paramObserver = CreateObjCBlock (this, &JuceAudioUnitv3::valueChangedFromHost);
        paramProvider = CreateObjCBlock (this, &JuceAudioUnitv3::getValue);

        overviewParams = [[NSMutableArray<NSNumber*> alloc] init];

        const int n = (*juceFilter)->getNumParameters();
        for (int idx = 0; idx < n; ++idx)
        {
            const String identifier = String (idx);
            const String name = (*juceFilter)->getParameterName (idx);

            AudioUnitParameterOptions flags = (UInt32) (kAudioUnitParameterFlag_IsWritable
                                                      | kAudioUnitParameterFlag_IsReadable
                                                      | kAudioUnitParameterFlag_HasCFNameString
                                                      | kAudioUnitParameterFlag_ValuesHaveStrings);

           #if JucePlugin_AUHighResolutionParameters
            flags |= (UInt32) kAudioUnitParameterFlag_IsHighResolution;
           #endif

            // set whether the param is automatable (unnamed parameters aren't allowed to be automated)
            if (name.isEmpty() || ! (*juceFilter)->isParameterAutomatable (idx))
                flags |= kAudioUnitParameterFlag_NonRealTime;

            if ((*juceFilter)->isMetaParameter (idx))
                flags |= kAudioUnitParameterFlag_IsGlobalMeta;

            AUParameterAddress address = static_cast<AUParameterAddress> (idx);

            // create methods in AUParameterTree return unretained objects (!) -> see Apple header AUAudioUnitImplementation.h

            ScopedPointer<AUParameter> param = [[AUParameterTree createParameterWithIdentifier: juceStringToNS (identifier)
                                                                                          name: juceStringToNS (name)
                                                                                       address: address
                                                                                           min: 0.0f
                                                                                           max: 1.0f
                                                                                          unit: kAudioUnitParameterUnit_Generic
                                                                                      unitName: nullptr
                                                                                         flags: flags
                                                                                  valueStrings: nullptr
                                                                           dependentParameters: nullptr] retain];

            [params addObject: param];
            [overviewParams addObject: [NSNumber numberWithUnsignedLongLong:address]];
        }

        // create methods in AUParameterTree return unretained objects (!) -> see Apple header AUAudioUnitImplementation.h
        paramTree = [[AUParameterTree createTreeWithChildren: params] retain];

        [paramTree setImplementorValueObserver: paramObserver];
        [paramTree setImplementorValueProvider: paramProvider];

        if ((*juceFilter)->hasEditor())
        {
            editorParamObserver = CreateObjCBlock (this, &JuceAudioUnitv3::valueChangedForObserver);
            editorObserverToken = [paramTree tokenByAddingParameterObserver: editorParamObserver];
        }
    }

    void addPresets()
    {
        factoryPresets = [[NSMutableArray<AUAudioUnitPreset*> alloc] init];

        const int n = (*juceFilter)->getNumPrograms();

        for (int idx = 0; idx < n; ++idx)
        {
            String name = (*juceFilter)->getProgramName (idx);

            ScopedPointer<AUAudioUnitPreset> preset = [[AUAudioUnitPreset alloc] init];
            [preset setName: juceStringToNS (name)];
            [preset setNumber: static_cast<NSInteger> (idx)];

            [factoryPresets addObject: preset];
        }
    }

    //==============================================================================
    void allocateBusBuffer (bool isInput)
    {
        OwnedArray<BusBuffer>& busBuffers = isInput ? inBusBuffers : outBusBuffers;
        busBuffers.clear();

        const int n = busUtils.getBusCount (isInput);
        const AUAudioFrameCount maxFrames = [getAudioUnit() maximumFramesToRender];

        for (int busIdx = 0; busIdx < n; ++busIdx)
            busBuffers.add (new BusBuffer ([(isInput ? inputBusses : outputBusses) objectAtIndexedSubscript: static_cast<unsigned int> (busIdx)], static_cast<int> (maxFrames)));
    }

    void processEvents (const AURenderEvent *__nullable realtimeEventListHead, int numParams, AUEventSampleTime startTime)
    {
        for (const AURenderEvent* event = realtimeEventListHead; event != nullptr; event = event->head.next)
        {
            switch (event->head.eventType)
            {
            case AURenderEventMIDI:
                {
                    const AUMIDIEvent& midiEvent = event->MIDI;
                    midiMessages.addEvent (midiEvent.data, midiEvent.length, static_cast<int> (midiEvent.eventSampleTime - startTime));
                }
                break;
            case AURenderEventParameter:
            case AURenderEventParameterRamp:
                {
                    const AUParameterEvent& paramEvent = event->parameter;
                    const int idx = static_cast<int> (paramEvent.parameterAddress);
                    if (isPositiveAndBelow (idx, numParams))
                        (*juceFilter)->setParameter (idx, paramEvent.value);
                }
                break;
            default:
                break;
            }
        }
    }

    AUAudioUnitStatus renderCallback (AudioUnitRenderActionFlags* actionFlags, const AudioTimeStamp* timestamp, AUAudioFrameCount frameCount,
                                      NSInteger outputBusNumber, AudioBufferList* outputData, const AURenderEvent *__nullable realtimeEventListHead,
                                      AURenderPullInputBlock __nullable pullInputBlock)
    {
        jassert (static_cast<int> (frameCount) <= (*juceFilter)->getBlockSize());

        // process params
        const int numParams = (*juceFilter)->getNumParameters();
        processEvents (realtimeEventListHead, numParams, static_cast<AUEventSampleTime> (timestamp->mSampleTime));

        if (lastTimeStamp.mSampleTime != timestamp->mSampleTime)
        {
            lastTimeStamp = *timestamp;

            const int numInputBuses  = inBusBuffers. size();
            const int numOutputBuses = outBusBuffers.size();

            // prepare buffers
            {
                for (int busIdx = 0; busIdx < numOutputBuses; ++busIdx)
                {
                     BusBuffer& busBuffer = *outBusBuffers[busIdx];
                     const bool canUseDirectOutput =
                         (busIdx == outputBusNumber && outputData != nullptr && outputData->mNumberBuffers > 0);

                    busBuffer.prepare (frameCount, canUseDirectOutput ? outputData : nullptr);
                }

                for (int busIdx = 0; busIdx < numInputBuses; ++busIdx)
                {
                    BusBuffer& busBuffer = *inBusBuffers[busIdx];
                    busBuffer.prepare (frameCount, busIdx < numOutputBuses ? outBusBuffers[busIdx]->get() : nullptr);
                }

                audioBuffer.reset();
            }

            // pull inputs
            {
                for (int busIdx = 0; busIdx < numInputBuses; ++busIdx)
                {
                    BusBuffer& busBuffer = *inBusBuffers[busIdx];
                    AudioBufferList* buffer = busBuffer.get();

                    if (pullInputBlock == nullptr || pullInputBlock (actionFlags, timestamp, frameCount, busIdx, buffer) != noErr)
                        AudioUnitHelpers::clearAudioBuffer (*buffer);

                    if (actionFlags != nullptr && (*actionFlags & kAudioUnitRenderAction_OutputIsSilence) != 0)
                        AudioUnitHelpers::clearAudioBuffer (*buffer);
                }
            }

            // set buffer pointer to minimize copying
            {
                int chIdx = 0;
                for (int busIdx = 0; busIdx < numOutputBuses; ++busIdx)
                {
                    BusBuffer& busBuffer = *outBusBuffers[busIdx];
                    AudioBufferList* buffer = busBuffer.get();

                    const bool interleaved = busBuffer.interleaved();
                    const int numChannels = busBuffer.numChannels();

                    const int* outLayoutMap = mapper.get (false, busIdx);

                    for (int ch = 0; ch < numChannels; ++ch)
                        audioBuffer.setBuffer (chIdx++, interleaved ? nullptr : static_cast<float*> (buffer->mBuffers[outLayoutMap[ch]].mData));
                }

                // use input pointers on remaining channels
                int channelCount = 0;
                for (int busIdx = 0; chIdx < totalInChannels;)
                {
                    busIdx = busUtils.getBusIdxForChannelIdx (true, chIdx, channelCount, busIdx);

                    BusBuffer& busBuffer = *inBusBuffers[busIdx];
                    AudioBufferList* buffer = busBuffer.get();

                    const bool interleaved = busBuffer.interleaved();
                    const int numChannels = busBuffer.numChannels();

                    const int* inLayoutMap = mapper.get (true, busIdx);

                    for (int ch = chIdx - channelCount; ch < numChannels; ++ch)
                        audioBuffer.setBuffer (chIdx++, interleaved ? nullptr : static_cast<float*> (buffer->mBuffers[inLayoutMap[ch]].mData));
                }
            }

            // copy input
            {
                for (int busIdx = 0; busIdx < numInputBuses; ++busIdx)
                    audioBuffer.push (*inBusBuffers[busIdx]->get(), mapper.get (true, busIdx));

                // clear remaining channels
                for (int i = totalInChannels; i < totalOutChannels; ++i)
                    zeromem (audioBuffer.push(), sizeof (float) * frameCount);
            }

            // process audio
            processBlock (audioBuffer.getBuffer (frameCount), midiMessages);
            midiMessages.clear();
        }

        // copy back
        {
            audioBuffer.pop (*outBusBuffers[(int) outputBusNumber]->get(), mapper.get (false, (int) outputBusNumber));
        }

        return noErr;
    }

    void processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiBuffer) noexcept
    {
        const ScopedLock sl ((*juceFilter)->getCallbackLock());

        if ((*juceFilter)->isSuspended())
        {
            buffer.clear();
        }
        else if ([au shouldBypassEffect])
        {
            (*juceFilter)->processBlockBypassed (buffer, midiBuffer);
        }
        else
        {
            (*juceFilter)->processBlock (buffer, midiBuffer);
        }
    }

    //==============================================================================
    void valueChangedFromHost (AUParameter *param, AUValue value)
    {
        if (param != nullptr)
        {
            const int idx = static_cast<int> ([param address]);
            if (isPositiveAndBelow (idx, (*juceFilter)->getNumParameters()))
                (*juceFilter)->setParameter (idx, value);
        }
    }

    AUValue getValue(AUParameter *param)
    {
        if (param != nullptr)
        {
            const int idx = static_cast<int> ([param address]);
            if (isPositiveAndBelow (idx, (*juceFilter)->getNumParameters()))
                return (*juceFilter)->getParameter (idx);
        }

        return 0.f;
    }

    void valueChangedForObserver(AUParameterAddress, AUValue)
    {
        // this will have already been handled bny valueChangedFromHost
    }

    //==============================================================================
    static const double kDefaultSampleRate;

    AudioProcessorHolder::Ptr juceFilter;
    PluginBusUtilities busUtils;

    int totalInChannels, totalOutChannels;

    ScopedPointer<AUAudioUnitBusArray> inputBusses;
    ScopedPointer<AUAudioUnitBusArray> outputBusses;

    ObjCBlock<AUImplementorValueObserver> paramObserver;
    ObjCBlock<AUImplementorValueProvider> paramProvider;

    // to avoid recursion on parameter changes, we need to add an
    // editor observer to do the parameter changes
    ObjCBlock<AUParameterObserver> editorParamObserver;
    AUParameterObserverToken editorObserverToken;

    ScopedPointer<AUParameterTree> paramTree;
    ScopedPointer<NSMutableArray<NSNumber*> > overviewParams;
    ScopedPointer<NSMutableArray<NSNumber*> > channelCapabilities;

    ScopedPointer<NSMutableArray<AUAudioUnitPreset*> > factoryPresets;

    ObjCBlock<AUInternalRenderBlock> internalRenderBlock;

    AudioUnitHelpers::CoreAudioBufferList audioBuffer;
    AudioUnitHelpers::ChannelRemapper mapper;

    OwnedArray<BusBuffer> inBusBuffers, outBusBuffers;
    MidiBuffer midiMessages;

    ObjCBlock<AUHostMusicalContextBlock> hostMusicalContextCallback;
    ObjCBlock<AUHostTransportStateBlock> hostTransportStateCallback;

    AudioTimeStamp lastTimeStamp;
    CurrentPositionInfo lastAudioHead;
};

const double JuceAudioUnitv3::kDefaultSampleRate = 44100.0;

JuceAudioUnitv3Base* JuceAudioUnitv3Base::create (AUAudioUnit* audioUnit, AudioComponentDescription descr, AudioComponentInstantiationOptions options, NSError** error)
{
    JUCE_DECLARE_WRAPPER_TYPE (wrapperType_AudioUnitv3);
    return new JuceAudioUnitv3 (audioUnit, descr, options, error);
}

//==============================================================================
class JuceAUViewController
{
public:

    JuceAUViewController (AUViewController <AUAudioUnitFactory>* p)
        : myself (p), juceFilter (nullptr), preferredSize (1.f, 1.f)
    {
        jassert (MessageManager::getInstance()->isThisTheMessageThread());

        JUCE_DECLARE_WRAPPER_TYPE (wrapperType_AudioUnitv3);
        initialiseJuce_GUI();
    }

    ~JuceAUViewController()
    {
        jassert (MessageManager::getInstance()->isThisTheMessageThread());
    }

    //==============================================================================
    AUViewController <AUAudioUnitFactory>* getController() noexcept          { return myself; }


    //==============================================================================
    void loadView()
    {
        jassert (MessageManager::getInstance()->isThisTheMessageThread());

        AudioProcessor* filter = createPluginFilterOfType (AudioProcessor::wrapperType_AudioUnitv3);
        if (filter != nullptr)
        {
            juceFilter = new AudioProcessorHolder (filter);
            if ((*juceFilter)->hasEditor())
            {
                if (AudioProcessorEditor* editor = (*juceFilter)->createEditorIfNeeded())
                {
                    preferredSize = editor->getBounds();

                    IOS_MAC_VIEW* view = [[[IOS_MAC_VIEW alloc] initWithFrame: (convertToCGRect (editor->getBounds()))] autorelease];
                    [myself setView: view];

                    editor->setVisible (true);
                    editor->addToDesktop (0, view);
                }
            }
        }
    }

    void viewDidLayoutSubviews()
    {
        if (juceFilter != nullptr && [myself view] != nullptr)
        {
            if (AudioProcessorEditor* editor = (*juceFilter)->getActiveEditor())
            {
                editor->setBounds (convertToRectInt ([[myself view] bounds]));

                if (IOS_MAC_VIEW* peerView = [[[myself view] subviews] objectAtIndex: 0])
                {
                   #if JUCE_IOS
                    [peerView setNeedsDisplay];
                   #else
                    [peerView setNeedsDisplay: YES];
                   #endif
                }
            }
        }
    }

    CGSize getPreferredContentSize()
    {
        return CGSizeMake (static_cast<float> (preferredSize.getWidth()), static_cast<float> (preferredSize.getHeight()));
    }

    //==============================================================================
    AUAudioUnit* createAudioUnit (const AudioComponentDescription& descr, NSError** error)
    {
        AUAudioUnit* retval = nullptr;

        if (! MessageManager::getInstance()->isThisTheMessageThread())
        {
            WaitableEvent creationEvent;

            // AUv3 headers say that we may block this thread and that the message thread is guaranteed
            // to be unblocked
            struct AUCreator : public CallbackMessage
            {
                JuceAUViewController& owner;
                AudioComponentDescription pDescr;
                NSError** pError;
                AUAudioUnit*& outAU;
                WaitableEvent& e;

                AUCreator (JuceAUViewController& parent, const AudioComponentDescription& paramDescr, NSError** paramError,
                           AUAudioUnit*& outputAU, WaitableEvent& event)
                    : owner (parent), pDescr (paramDescr), pError (paramError), outAU (outputAU), e (event)
                {}

                ~AUCreator()
                {}

                void messageCallback() override
                {
                    outAU = owner.createAudioUnitOnMessageThread (pDescr, pError);
                    e.signal();
                }
            };

            (new AUCreator (*this, descr, error, retval, creationEvent))->post();
            creationEvent.wait (-1);
        }
        else
            retval = createAudioUnitOnMessageThread (descr, error);

        return [retval autorelease];
    }

private:

    //==============================================================================
    AUAudioUnit* createAudioUnitOnMessageThread (const AudioComponentDescription& descr, NSError** error)
    {
        jassert (MessageManager::getInstance()->isThisTheMessageThread());

        // this will call [view load] and ensure that the juce filter has been instantiated
        [myself view];
        if (juceFilter == nullptr)
            return nullptr;

        return (new JuceAudioUnitv3 (juceFilter, descr, 0, error))->getAudioUnit();
    }

    //==============================================================================
    AUViewController <AUAudioUnitFactory>* myself;
    AudioProcessorHolder::Ptr juceFilter;
    Rectangle<int> preferredSize;
};

//==============================================================================
// necessary glue code
@interface JUCE_VIEWCONTROLLER_OBJC_NAME (JucePlugin_AUExportPrefix) : AUViewController <AUAudioUnitFactory>
@end
@implementation JUCE_VIEWCONTROLLER_OBJC_NAME (JucePlugin_AUExportPrefix) {
    ScopedPointer<JuceAUViewController> cpp;
}
- (instancetype)initWithNibName:(nullable NSString *)nib bundle:(nullable NSBundle *)bndl { self = [super initWithNibName:nib bundle:bndl]; cpp = new JuceAUViewController (self); return self;}
- (void) loadView              { cpp->loadView(); }
- (AUAudioUnit *)createAudioUnitWithComponentDescription:(AudioComponentDescription)desc error:(NSError **)error { return cpp->createAudioUnit (desc, error); }
- (CGSize) preferredContentSize { return cpp->getPreferredContentSize(); }
- (void)viewDidLayoutSubviews { return cpp->viewDidLayoutSubviews(); }
@end

#pragma clang diagnostic pop
#endif

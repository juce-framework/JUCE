/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

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

// (This file gets included by juce_mac_NativeCode.mm, rather than being
// compiled on its own).
#if JUCE_INCLUDED_FILE && JUCE_USE_CDBURNER

//==============================================================================
END_JUCE_NAMESPACE

#define OpenDiskDevice MakeObjCClassName(OpenDiskDevice)

@interface OpenDiskDevice   : NSObject
{
    DRDevice* device;

    NSMutableArray* tracks;
}

- (OpenDiskDevice*) initWithDevice: (DRDevice*) device;
- (void) dealloc;
- (bool) isDiskPresent;
- (int) getNumAvailableAudioBlocks;
- (void) addSourceTrack: (JUCE_NAMESPACE::AudioSource*) source numSamples: (int) numSamples_;
- (void) burn: (JUCE_NAMESPACE::AudioCDBurner::BurnProgressListener*) listener  errorString: (JUCE_NAMESPACE::String*) error
         ejectAfterwards: (bool) shouldEject isFake: (bool) peformFakeBurnForTesting;
@end

//==============================================================================
#define AudioTrackProducer MakeObjCClassName(AudioTrackProducer)

@interface AudioTrackProducer   : NSObject
{
    JUCE_NAMESPACE::AudioSource* source;
    int readPosition, lengthInFrames;
}

- (AudioTrackProducer*) init: (int) lengthInFrames;
- (AudioTrackProducer*) initWithAudioSource: (JUCE_NAMESPACE::AudioSource*) source numSamples: (int) lengthInSamples;
- (void) dealloc;
- (void) setupTrackProperties: (DRTrack*) track;

- (void) cleanupTrackAfterBurn: (DRTrack*) track;
- (BOOL) cleanupTrackAfterVerification:(DRTrack*)track;
- (uint64_t) estimateLengthOfTrack:(DRTrack*)track;
- (BOOL) prepareTrack:(DRTrack*)track forBurn:(DRBurn*)burn
         toMedia:(NSDictionary*)mediaInfo;
- (BOOL) prepareTrackForVerification:(DRTrack*)track;
- (uint32_t) produceDataForTrack:(DRTrack*)track intoBuffer:(char*)buffer
        length:(uint32_t)bufferLength atAddress:(uint64_t)address
        blockSize:(uint32_t)blockSize ioFlags:(uint32_t*)flags;
- (uint32_t) producePreGapForTrack:(DRTrack*)track
             intoBuffer:(char*)buffer length:(uint32_t)bufferLength
             atAddress:(uint64_t)address blockSize:(uint32_t)blockSize
             ioFlags:(uint32_t*)flags;
- (BOOL) verifyDataForTrack:(DRTrack*)track inBuffer:(const char*)buffer
         length:(uint32_t)bufferLength atAddress:(uint64_t)address
         blockSize:(uint32_t)blockSize ioFlags:(uint32_t*)flags;
- (uint32_t) producePreGapForTrack:(DRTrack*)track
        intoBuffer:(char*)buffer length:(uint32_t)bufferLength
        atAddress:(uint64_t)address blockSize:(uint32_t)blockSize
        ioFlags:(uint32_t*)flags;
@end

//==============================================================================
@implementation OpenDiskDevice

- (OpenDiskDevice*) initWithDevice: (DRDevice*) device_
{
    [super init];

    device = device_;
    tracks = [[NSMutableArray alloc] init];
    return self;
}

- (void) dealloc
{
    [tracks release];
    [super dealloc];
}

- (bool) isDiskPresent
{
    return [device isValid]
             && [[[device status] objectForKey: DRDeviceMediaStateKey]
                    isEqualTo: DRDeviceMediaStateMediaPresent];
}

- (int) getNumAvailableAudioBlocks
{
    return [[[[device status] objectForKey: DRDeviceMediaInfoKey]
                              objectForKey: DRDeviceMediaBlocksFreeKey] intValue];
}

- (void) addSourceTrack: (JUCE_NAMESPACE::AudioSource*) source_ numSamples: (int) numSamples_
{
    AudioTrackProducer* p = [[AudioTrackProducer alloc] initWithAudioSource: source_ numSamples: numSamples_];
    DRTrack* t = [[DRTrack alloc] initWithProducer: p];
    [p setupTrackProperties: t];

    [tracks addObject: t];

    [t release];
    [p release];
}

- (void) burn: (JUCE_NAMESPACE::AudioCDBurner::BurnProgressListener*) listener errorString: (JUCE_NAMESPACE::String*) error
         ejectAfterwards: (bool) shouldEject isFake: (bool) peformFakeBurnForTesting
{
    DRBurn* burn = [DRBurn burnForDevice: device];

    if (! [device acquireExclusiveAccess])
    {
        *error = "Couldn't open or write to the CD device";
        return;
    }

    [device acquireMediaReservation];

    NSMutableDictionary* d = [[burn properties] mutableCopy];
    [d autorelease];
    [d setObject: [NSNumber numberWithBool: peformFakeBurnForTesting] forKey: DRBurnTestingKey];
    [d setObject: [NSNumber numberWithBool: false] forKey: DRBurnVerifyDiscKey];
    [d setObject: (shouldEject ? DRBurnCompletionActionEject : DRBurnCompletionActionMount)
          forKey: DRBurnCompletionActionKey];
    [burn setProperties: d];

    [burn writeLayout: tracks];

    for (;;)
    {
        JUCE_NAMESPACE::Thread::sleep (300);
        float progress = [[[burn status] objectForKey: DRStatusPercentCompleteKey] floatValue];

        if (listener != 0 && listener->audioCDBurnProgress (progress))
        {
            [burn abort];
            *error = "User cancelled the write operation";
            break;
        }

        if ([[[burn status] objectForKey: DRStatusStateKey] isEqualTo: DRStatusStateFailed])
        {
            *error = "Write operation failed";
            break;
        }
        else if ([[[burn status] objectForKey: DRStatusStateKey] isEqualTo: DRStatusStateDone])
        {
            break;
        }

        NSString* err = (NSString*) [[[burn status] objectForKey: DRErrorStatusKey]
                                                    objectForKey: DRErrorStatusErrorStringKey];

        if ([err length] > 0)
        {
            *error = JUCE_NAMESPACE::String::fromUTF8 ((JUCE_NAMESPACE::uint8*) [err UTF8String]);
            break;
        }
    }

    [device releaseMediaReservation];
    [device releaseExclusiveAccess];
}
@end

//==============================================================================
@implementation AudioTrackProducer

- (AudioTrackProducer*) init: (int) lengthInFrames_
{
    lengthInFrames = lengthInFrames_;
    readPosition = 0;
    return self;
}

- (void) setupTrackProperties: (DRTrack*) track
{
    NSMutableDictionary*  p = [[track properties] mutableCopy];
    [p setObject:[DRMSF msfWithFrames: lengthInFrames] forKey: DRTrackLengthKey];
    [p setObject:[NSNumber numberWithUnsignedShort:2352] forKey: DRBlockSizeKey];
    [p setObject:[NSNumber numberWithInt:0] forKey: DRDataFormKey];
    [p setObject:[NSNumber numberWithInt:0] forKey: DRBlockTypeKey];
    [p setObject:[NSNumber numberWithInt:0] forKey: DRTrackModeKey];
    [p setObject:[NSNumber numberWithInt:0] forKey: DRSessionFormatKey];


    [track setProperties: p];
    [p release];
}

- (AudioTrackProducer*) initWithAudioSource: (JUCE_NAMESPACE::AudioSource*) source_ numSamples: (int) lengthInSamples
{
    AudioTrackProducer* s = [self init: (lengthInSamples + 587) / 588];

    if (s != nil)
        s->source = source_;

    return s;
}

- (void) dealloc
{
    if (source != 0)
    {
        source->releaseResources();
        delete source;
    }

    [super dealloc];
}

- (void) cleanupTrackAfterBurn: (DRTrack*) track
{
}

- (BOOL) cleanupTrackAfterVerification: (DRTrack*) track
{
    return true;
}

- (uint64_t) estimateLengthOfTrack: (DRTrack*) track
{
    return lengthInFrames;
}

- (BOOL) prepareTrack: (DRTrack*) track forBurn: (DRBurn*) burn
         toMedia: (NSDictionary*) mediaInfo
{
    if (source != 0)
        source->prepareToPlay (44100 / 75, 44100);

    readPosition = 0;
    return true;
}

- (BOOL) prepareTrackForVerification: (DRTrack*) track
{
    if (source != 0)
        source->prepareToPlay (44100 / 75, 44100);

    return true;
}

- (uint32_t) produceDataForTrack: (DRTrack*) track intoBuffer: (char*) buffer
        length: (uint32_t) bufferLength atAddress: (uint64_t) address
        blockSize: (uint32_t) blockSize ioFlags: (uint32_t*) flags
{
    if (source != 0)
    {
        const int numSamples = JUCE_NAMESPACE::jmin ((int) bufferLength / 4, (lengthInFrames * (44100 / 75)) - readPosition);

        if (numSamples > 0)
        {
            JUCE_NAMESPACE::AudioSampleBuffer tempBuffer (2, numSamples);

            JUCE_NAMESPACE::AudioSourceChannelInfo info;
            info.buffer = &tempBuffer;
            info.startSample = 0;
            info.numSamples = numSamples;

            source->getNextAudioBlock (info);

            JUCE_NAMESPACE::AudioDataConverters::convertFloatToInt16LE (tempBuffer.getSampleData (0),
                                                                        buffer, numSamples, 4);
            JUCE_NAMESPACE::AudioDataConverters::convertFloatToInt16LE (tempBuffer.getSampleData (1),
                                                                        buffer + 2, numSamples, 4);

            readPosition += numSamples;
        }

        return numSamples * 4;
    }

    return 0;
}

- (uint32_t) producePreGapForTrack: (DRTrack*) track
        intoBuffer: (char*) buffer length: (uint32_t) bufferLength
        atAddress: (uint64_t) address blockSize: (uint32_t) blockSize
        ioFlags: (uint32_t*) flags
{
    zeromem (buffer, bufferLength);
    return bufferLength;
}

- (BOOL) verifyDataForTrack: (DRTrack*) track inBuffer: (const char*) buffer
         length: (uint32_t) bufferLength atAddress: (uint64_t) address
         blockSize: (uint32_t) blockSize ioFlags: (uint32_t*) flags
{
    return true;
}

@end


BEGIN_JUCE_NAMESPACE

//==============================================================================
AudioCDBurner::AudioCDBurner (const int deviceIndex)
    : internal (0)
{
    OpenDiskDevice* dev = [[OpenDiskDevice alloc] initWithDevice: [[DRDevice devices] objectAtIndex: deviceIndex]];

    internal = (void*) dev;
}

AudioCDBurner::~AudioCDBurner()
{
    OpenDiskDevice* dev = (OpenDiskDevice*) internal;

    if (dev != 0)
        [dev release];
}

AudioCDBurner* AudioCDBurner::openDevice (const int deviceIndex)
{
    ScopedPointer <AudioCDBurner> b (new AudioCDBurner (deviceIndex));

    if (b->internal == 0)
        b = 0;

    return b.release();
}

static NSArray* findDiskBurnerDevices()
{
    NSMutableArray* results = [NSMutableArray array];
    NSArray* devs = [DRDevice devices];

    if (devs != 0)
    {
        int num = [devs count];
        int i;
        for (i = 0; i < num; ++i)
        {
            NSDictionary* dic = [[devs objectAtIndex: i] info];
            NSString* name = [dic valueForKey: DRDeviceProductNameKey];
            if (name != nil)
                [results addObject: name];
        }
    }

    return results;
}

const StringArray AudioCDBurner::findAvailableDevices()
{
    NSArray* names = findDiskBurnerDevices();
    StringArray s;

    for (unsigned int i = 0; i < [names count]; ++i)
        s.add (String::fromUTF8 ((JUCE_NAMESPACE::uint8*) [[names objectAtIndex: i] UTF8String]));

    return s;
}

bool AudioCDBurner::isDiskPresent() const
{
    OpenDiskDevice* dev = (OpenDiskDevice*) internal;

    return dev != 0 && [dev isDiskPresent];
}

int AudioCDBurner::getNumAvailableAudioBlocks() const
{
    OpenDiskDevice* dev = (OpenDiskDevice*) internal;

    return [dev getNumAvailableAudioBlocks];
}

bool AudioCDBurner::addAudioTrack (AudioSource* source, int numSamps)
{
    OpenDiskDevice* dev = (OpenDiskDevice*) internal;

    if (dev != 0)
    {
        [dev addSourceTrack: source numSamples: numSamps];
        return true;
    }

    return false;
}

const String AudioCDBurner::burn (JUCE_NAMESPACE::AudioCDBurner::BurnProgressListener* listener,
                                  const bool ejectDiscAfterwards,
                                  const bool peformFakeBurnForTesting)
{
    String error ("Couldn't open or write to the CD device");

    OpenDiskDevice* dev = (OpenDiskDevice*) internal;

    if (dev != 0)
    {
        error = String::empty;
        [dev burn: listener
             errorString: &error
             ejectAfterwards: ejectDiscAfterwards
             isFake: peformFakeBurnForTesting];
    }

    return error;
}


//==============================================================================
void AudioCDReader::ejectDisk()
{
    const ScopedAutoReleasePool p;
    [[NSWorkspace sharedWorkspace] unmountAndEjectDeviceAtPath: juceStringToNS (volumeDir.getFullPathName())];
}


#endif

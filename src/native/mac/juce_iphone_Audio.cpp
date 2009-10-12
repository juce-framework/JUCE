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
#ifdef JUCE_INCLUDED_FILE


@interface UIKitAUIOHost : UIViewController
{
@public
	/** READONLY The audio format of the data stream. */
	AudioStreamBasicDescription format;
	AURenderCallbackStruct		inputProc;
	Float64						hwSampleRate;
	AudioUnit					rioUnit;
	UGen						rawInput;
	UGen						postFadeOutput;
	UGen						preFadeOutput;
	int							bufferSize;
	float						*floatBuffer;
	UInt32						audioInputIsAvailable;
	UInt32						numInputChannels;
	UInt32						numOutputChannels;
	bool						isRunning;
	float						fadeInTime;
	UGenArray					others;
	NSLock*						nsLock;
}

/** Initialises the AudioUnit framework and structures.
 Do not call this method, it is called automatically when the application launches. */
- (void)initAudio;

/** Construct a UGen graph.
 You must implement this in your subclass. You should return a UGen which will be the UGen graph which is 
 performed and rendered to the host. The input parameter may be ignored if only signal generation is required 
 or may be used if a processing algorithm is being implemented (e.g., filtering incoming audio data).
 
 @param input	The input UGen which will contain audio data from the host.
 @return		the UGen graph which will be performed */
- (UGen)constructGraph:(UGen)input;

- (void)addOther:(UGen)ugen;

- (void)lock;
- (void)unlock;
- (BOOL)tryLock;

@end

#define NUM_CHANNELS 2

void SetFormat(AudioStreamBasicDescription& format)
{
	memset(&format, 0, sizeof(AudioStreamBasicDescription));
	format.mFormatID = kAudioFormatLinearPCM;
	int sampleSize = sizeof(AudioSampleType);
	format.mFormatFlags = kAudioFormatFlagsCanonical;
	format.mBitsPerChannel = 8 * sampleSize;
	format.mChannelsPerFrame = NUM_CHANNELS;
	format.mFramesPerPacket = 1;
	format.mBytesPerPacket = format.mBytesPerFrame = sampleSize;
	format.mFormatFlags |= kAudioFormatFlagIsNonInterleaved;	
}

int SetupRemoteIO (AudioUnit& inRemoteIOUnit, AURenderCallbackStruct inRenderProc, AudioStreamBasicDescription& outFormat)
{	
	// Open the output unit
	AudioComponentDescription desc;
	desc.componentType = kAudioUnitType_Output;
	desc.componentSubType = kAudioUnitSubType_RemoteIO;
	desc.componentManufacturer = kAudioUnitManufacturer_Apple;
	desc.componentFlags = 0;
	desc.componentFlagsMask = 0;
	
	AudioComponent comp = AudioComponentFindNext (NULL, &desc);
	AudioComponentInstanceNew (comp, &inRemoteIOUnit);
	
	const UInt32 one = 1;
	AudioUnitSetProperty(inRemoteIOUnit, kAudioOutputUnitProperty_EnableIO, kAudioUnitScope_Input, 1, &one, sizeof(one));	
	AudioUnitSetProperty(inRemoteIOUnit, kAudioUnitProperty_SetRenderCallback, kAudioUnitScope_Input, 0, &inRenderProc, sizeof(inRenderProc));
	
	AudioUnitSetProperty(inRemoteIOUnit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Input, 0, &outFormat, sizeof(outFormat));
	AudioUnitSetProperty(inRemoteIOUnit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Output, 1, &outFormat, sizeof(outFormat));
	
	AudioUnitInitialize(inRemoteIOUnit);
	
	return 0;
}

static const float FloatToFixed824_Factor = 16777216.f;
static const float Fixed824ToFloat_Factor = 5.960464477539e-08f;

static const float FloatToPCM16Bit_Factor = 32767.f;
static const float PCM16BitToFloat_Factor = 3.051850947600e-05f;

static OSStatus	PerformThru(void						*inRefCon, 
							AudioUnitRenderActionFlags 	*ioActionFlags, 
							const AudioTimeStamp 		*inTimeStamp, 
							UInt32 						inBusNumber, 
							UInt32 						inNumberFrames, 
							AudioBufferList 			*ioData)
{	
	OSStatus err = 0;
	UIKitAUIOHost *x = (UIKitAUIOHost *)inRefCon;
	
	[x lock];
	
	if(x->audioInputIsAvailable)
	{
		err = AudioUnitRender(x->rioUnit, ioActionFlags, inTimeStamp, 1, inNumberFrames, ioData);
		if (err) { printf("PerformThru: error %d\n", (int)err); return err; }
	}
	
	if(inNumberFrames > x->bufferSize)
	{
		delete [] x->floatBuffer;
		x->bufferSize = inNumberFrames;
		
		x->floatBuffer = new float[inNumberFrames * NUM_CHANNELS];
	}
	
	long blockID = UGen::getNextBlockID(inNumberFrames);
	
	float *floatBufferData[2];
	floatBufferData[0] = x->floatBuffer;
	floatBufferData[1] = floatBufferData[0] + inNumberFrames;
	
	if(x->audioInputIsAvailable)
	{
		for (UInt32 channel = 0; channel < x->numInputChannels; channel++)
		{
			AudioSampleType *audioUnitBuffer = (AudioSampleType*)ioData->mBuffers[0].mData;
			float *floatBuffer = floatBufferData[channel];
			
			for(int sample = 0; sample < inNumberFrames; sample++)
			{
				floatBuffer[sample] = (float)audioUnitBuffer[sample] * PCM16BitToFloat_Factor;
			}		
		}
		
		x->rawInput.getSource().setInputs((const float**)floatBufferData, inNumberFrames, x->numInputChannels);
	}
	else
	{
		memset(x->floatBuffer, 0, x->numInputChannels * inNumberFrames * sizeof(float));
	}
	
	x->postFadeOutput.setOutputs(floatBufferData, inNumberFrames, 2);
	x->postFadeOutput.prepareAndProcessBlock(inNumberFrames, blockID);
	
	for (UInt32 channel = 0; channel < ioData->mNumberBuffers; channel++)
	{
		AudioSampleType *audioUnitBuffer = (AudioSampleType*)ioData->mBuffers[channel].mData;
		float *floatBuffer = floatBufferData[channel];
		
		for(int sample = 0; sample < inNumberFrames; sample++)
		{
			audioUnitBuffer[sample] = (AudioSampleType)(floatBuffer[sample] * FloatToPCM16Bit_Factor);
		}
	}
	
	for(int i = 0; i < x->others.size(); i++)
	{
		x->others[i].prepareAndProcessBlock(inNumberFrames, blockID);
	}	
	
	[x unlock];
	
	return err;
}

void propListener(void *                  inClientData,
				  AudioSessionPropertyID  inID,
				  UInt32                  inDataSize,
				  const void *            inPropertyValue)
{
	printf("Property changed!\n");
	
	UIKitAUIOHost *x = (UIKitAUIOHost *)inClientData;
	
	if(!x->isRunning) return;
	
	if(inPropertyValue)
	{
		CFDictionaryRef routeChangeDictionary  = (CFDictionaryRef)inPropertyValue;
		CFNumberRef     routeChangeReasonRef   = 
		(CFNumberRef)CFDictionaryGetValue (routeChangeDictionary, 
										   CFSTR (kAudioSession_AudioRouteChangeKey_Reason));
		
		SInt32 routeChangeReason;
		CFNumberGetValue(routeChangeReasonRef, kCFNumberSInt32Type, &routeChangeReason);
		
		CFStringRef newAudioRoute;
		UInt32 propertySize = sizeof (CFStringRef);
		AudioSessionGetProperty(kAudioSessionProperty_AudioRoute, &propertySize, &newAudioRoute);
		
		printf("route=%s\n", CFStringGetCStringPtr(newAudioRoute, CFStringGetSystemEncoding()));
		
	}
	
	UInt32 size = sizeof(UInt32);
	AudioSessionGetProperty(kAudioSessionProperty_CurrentHardwareInputNumberChannels, &size, &x->numInputChannels);
	AudioSessionGetProperty(kAudioSessionProperty_CurrentHardwareOutputNumberChannels, &size, &x->numOutputChannels);
	AudioSessionGetProperty(kAudioSessionProperty_AudioInputAvailable, &size, &x->audioInputIsAvailable);
	
	printf("inputs=%d outputs=%d audioInputIsAvailable=%d\n", x->numInputChannels, x->numOutputChannels, x->audioInputIsAvailable);
	
	if(x->rioUnit)
	{
		AudioComponentInstanceDispose(x->rioUnit);	
	}
	
	SetFormat(x->format);
	SetupRemoteIO(x->rioUnit, x->inputProc, x->format);
	
	x->rawInput.setSource(AudioIn::AR(x->numInputChannels), true);
	x->postFadeOutput = Plug::AR(UGen::emptyChannels(x->preFadeOutput.getNumChannels()));
	x->postFadeOutput.fadeSourceAndRelease(x->preFadeOutput, x->fadeInTime);
	
	AudioSessionSetActive(true);
	AudioOutputUnitStart(x->rioUnit);
}

void rioInterruptionListener(void *inClientData, UInt32 inInterruption)
{
	printf("Session interrupted! --- %s ---\n", inInterruption == kAudioSessionBeginInterruption ? "Begin Interruption" : "End Interruption");
	
	UIKitAUIOHost *x = (UIKitAUIOHost *)inClientData;
	
	if (inInterruption == kAudioSessionEndInterruption) {
		// make sure we are again the active session
		//AudioSessionSetActive(false);
		AudioSessionSetActive(true);
		x->isRunning = true;
		AudioOutputUnitStart(x->rioUnit);
	}
	
	if (inInterruption == kAudioSessionBeginInterruption) {
		x->isRunning = false;
		AudioOutputUnitStop(x->rioUnit);
		
		printf("rioInterruptionListener audioInputIsAvailable=%d\n", x->audioInputIsAvailable);
		
		UIAlertView *baseAlert = [[UIAlertView alloc] initWithTitle:@"Audio interrupted" 
															message:@"This could have been interrupted by another application or due to unplugging a headset:" 
														   delegate:x 
												  cancelButtonTitle:nil
												  otherButtonTitles:@"Resume", @"Cancel", nil];
		[baseAlert show];
    }
	
}

@implementation UIKitAUIOHost

- (id)init
{
	if (self = [super init])
	{
		nsLock = [[NSLock alloc] init];
		fadeInTime = 1.0;
		[self performSelector:@selector(initAudio) withObject:nil afterDelay:1.0];
	}
	return self;
}

- (void)initAudio
{	
	// render proc
	inputProc.inputProc = PerformThru;
	inputProc.inputProcRefCon = self;

	// session
	AudioSessionInitialize (NULL, NULL, rioInterruptionListener, self);
	AudioSessionSetActive (true);

	UInt32 audioCategory = kAudioSessionCategory_PlayAndRecord;
	AudioSessionSetProperty(kAudioSessionProperty_AudioCategory, sizeof(audioCategory), &audioCategory);
	AudioSessionAddPropertyListener(kAudioSessionProperty_AudioRouteChange, propListener, self);

	UInt32 size = sizeof(hwSampleRate);
	AudioSessionGetProperty(kAudioSessionProperty_CurrentHardwareSampleRate, &size, &hwSampleRate);

	Float32 bufferDuration = 512 / hwSampleRate;
	AudioSessionSetProperty(kAudioSessionProperty_PreferredHardwareIOBufferDuration, sizeof(bufferDuration), &bufferDuration);

	UGen::initialise();
	UGen::prepareToPlay(hwSampleRate, 512);

	rawInput = Plug::AR(UGen::emptyChannels(2));
	preFadeOutput = [self constructGraph: rawInput];

	rioUnit = NULL;
	isRunning = true;
	propListener((void*)self, 0,0,0);

	size = sizeof(format);
	AudioUnitGetProperty(rioUnit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Output, 1, &format, &size);

	//Float32 bufferDuration;
	size = sizeof(bufferDuration);
	AudioSessionGetProperty(kAudioSessionProperty_CurrentHardwareIOBufferDuration, &size, &bufferDuration);

	bufferSize = (int)(hwSampleRate*bufferDuration+0.5);
	floatBuffer = new float[bufferSize * NUM_CHANNELS];
}

- (UGen)constructGraph:(UGen)input
{
	return UGen::emptyChannels(NUM_CHANNELS);	
}

- (void)addOther:(UGen)ugen
{
	[self lock];
	others <<= ugen;
	[self unlock];
}

- (void)lock
{
	[nsLock lock];
}

- (void)unlock
{
	[nsLock unlock];
}

- (BOOL)tryLock
{
	return [nsLock tryLock];
}

- (void)alertView:(UIAlertView *)alertView clickedButtonAtIndex:(NSInteger)buttonIndex
{
	printf("buttonIndex=%d\n", buttonIndex);
	
	if(buttonIndex == 0)
	{
		// resume
		isRunning = true;
		propListener((void*)self, 0,0,0);
	}
	
	[alertView release];
}


-(void) dealloc
{
	UGen::shutdown();
	delete [] floatBuffer;
	[nsLock release];
	[super dealloc];
}
@end


//==============================================================================
class IPhoneAudioIODeviceType  : public AudioIODeviceType
{
public:
    //==============================================================================
    IPhoneAudioIODeviceType()
        : AudioIODeviceType (T("iPhone Audio")),
          hasScanned (false)
    {
    }

    ~IPhoneAudioIODeviceType()
    {
    }

    //==============================================================================
    void scanForDevices()
    {
    }

    const StringArray getDeviceNames (const bool wantInputNames) const
    {
        StringArray s;
        return s;
    }

    int getDefaultDeviceIndex (const bool forInput) const
    {
        return 0;
    }

    int getIndexOfDevice (AudioIODevice* device, const bool asInput) const
    {
        return 0;
    }

    bool hasSeparateInputsAndOutputs() const    { return true; }

    AudioIODevice* createDevice (const String& outputDeviceName,
                                 const String& inputDeviceName)
    {
        if (outputDeviceName.isNotEmpty() && inputDeviceName.isNotEmpty())
            return new CoreAudioIODevice (deviceName,
                                          inputIds [inputIndex],
                                          inputIndex,
                                          outputIds [outputIndex],
                                          outputIndex);

        return 0;
    }

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    IPhoneAudioIODeviceType (const IPhoneAudioIODeviceType&);
    const IPhoneAudioIODeviceType& operator= (const IPhoneAudioIODeviceType&);
};

//==============================================================================
AudioIODeviceType* juce_createAudioIODeviceType_iPhoneAudio()
{
    return new IPhoneAudioIODeviceType();
}

#endif

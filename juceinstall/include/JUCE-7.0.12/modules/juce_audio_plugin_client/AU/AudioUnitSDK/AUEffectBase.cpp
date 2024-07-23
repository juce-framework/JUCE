/*!
	@file		AudioUnitSDK/AUEffectBase.cpp
	@copyright	© 2000-2021 Apple Inc. All rights reserved.
*/
#include <AudioUnitSDK/AUEffectBase.h>

#include <cstddef>

/*
	This class does not deal as well as it should with N-M effects...

	The problem areas are (if the channels don't match):
		ProcessInPlace if the channels don't match - there will be problems if InputChan !=
   OutputChan Bypass - its just passing the buffers through when not processing them
*/

namespace ausdk {

//_____________________________________________________________________________
//
AUEffectBase::AUEffectBase(AudioComponentInstance audioUnit, bool inProcessesInPlace)
	: AUBase(audioUnit, 1, 1), // 1 in bus, 1 out bus
	  mProcessesInPlace(inProcessesInPlace)
#if TARGET_OS_IPHONE
	  ,
	  mOnlyOneKernel(false)
#endif
{
}

//_____________________________________________________________________________
//
void AUEffectBase::Cleanup()
{
	mKernelList.clear();
	mMainOutput = nullptr;
	mMainInput = nullptr;
}


//_____________________________________________________________________________
//
OSStatus AUEffectBase::Initialize()
{
	// get our current numChannels for input and output
	const auto auNumInputs = static_cast<SInt16>(Input(0).GetStreamFormat().mChannelsPerFrame);
	const auto auNumOutputs = static_cast<SInt16>(Output(0).GetStreamFormat().mChannelsPerFrame);

	// does the unit publish specific information about channel configurations?
	const AUChannelInfo* auChannelConfigs = nullptr;
	const UInt32 numIOconfigs = SupportedNumChannels(&auChannelConfigs);

	if ((numIOconfigs > 0) && (auChannelConfigs != nullptr)) {
		bool foundMatch = false;
		for (UInt32 i = 0; (i < numIOconfigs) && !foundMatch; ++i) {
			const SInt16 configNumInputs = auChannelConfigs[i].inChannels;   // NOLINT
			const SInt16 configNumOutputs = auChannelConfigs[i].outChannels; // NOLINT
			if ((configNumInputs < 0) && (configNumOutputs < 0)) {
				// unit accepts any number of channels on input and output
				if (((configNumInputs == -1) && (configNumOutputs == -2)) ||
					((configNumInputs == -2) &&
						(configNumOutputs == -1))) { // NOLINT repeated branch below
					foundMatch = true;
					// unit accepts any number of channels on input and output IFF they are the same
					// number on both scopes
				} else if (((configNumInputs == -1) && (configNumOutputs == -1)) &&
						   (auNumInputs == auNumOutputs)) {
					foundMatch = true;
					// unit has specified a particular number of channels on both scopes
				} else {
					continue;
				}
			} else {
				// the -1 case on either scope is saying that the unit doesn't care about the
				// number of channels on that scope
				const bool inputMatch = (auNumInputs == configNumInputs) || (configNumInputs == -1);
				const bool outputMatch =
					(auNumOutputs == configNumOutputs) || (configNumOutputs == -1);
				if (inputMatch && outputMatch) {
					foundMatch = true;
				}
			}
		}
		if (!foundMatch) {
			return kAudioUnitErr_FormatNotSupported;
		}
	} else {
		// there is no specifically published channel info
		// so for those kinds of effects, the assumption is that the channels (whatever their
		// number) should match on both scopes
		if ((auNumOutputs != auNumInputs) || (auNumOutputs == 0)) {
			return kAudioUnitErr_FormatNotSupported;
		}
	}
	MaintainKernels();

	mMainOutput = &Output(0);
	mMainInput = &Input(0);

	const AudioStreamBasicDescription format = GetStreamFormat(kAudioUnitScope_Output, 0);
	mBytesPerFrame = format.mBytesPerFrame;

	return noErr;
}

OSStatus AUEffectBase::Reset(AudioUnitScope inScope, AudioUnitElement inElement)
{
	for (auto& kernel : mKernelList) {
		if (kernel) {
			kernel->Reset();
		}
	}

	return AUBase::Reset(inScope, inElement);
}

OSStatus AUEffectBase::GetPropertyInfo(AudioUnitPropertyID inID, AudioUnitScope inScope,
	AudioUnitElement inElement, UInt32& outDataSize, bool& outWritable)
{
	if (inScope == kAudioUnitScope_Global) {
		switch (inID) {
		case kAudioUnitProperty_BypassEffect:
		case kAudioUnitProperty_InPlaceProcessing:
			outWritable = true;
			outDataSize = sizeof(UInt32);
			return noErr;
		default:
			break;
		}
	}
	return AUBase::GetPropertyInfo(inID, inScope, inElement, outDataSize, outWritable);
}


OSStatus AUEffectBase::GetProperty(
	AudioUnitPropertyID inID, AudioUnitScope inScope, AudioUnitElement inElement, void* outData)
{
	if (inScope == kAudioUnitScope_Global) {
		switch (inID) {
		case kAudioUnitProperty_BypassEffect:
			*static_cast<UInt32*>(outData) = (IsBypassEffect() ? 1 : 0); // NOLINT
			return noErr;
		case kAudioUnitProperty_InPlaceProcessing:
			*static_cast<UInt32*>(outData) = (mProcessesInPlace ? 1 : 0); // NOLINT
			return noErr;
		default:
			break;
		}
	}
	return AUBase::GetProperty(inID, inScope, inElement, outData);
}


OSStatus AUEffectBase::SetProperty(AudioUnitPropertyID inID, AudioUnitScope inScope,
	AudioUnitElement inElement, const void* inData, UInt32 inDataSize)
{
	if (inScope == kAudioUnitScope_Global) {
		switch (inID) {
		case kAudioUnitProperty_BypassEffect: {
			if (inDataSize < sizeof(UInt32)) {
				return kAudioUnitErr_InvalidPropertyValue;
			}

			const bool tempNewSetting = *static_cast<const UInt32*>(inData) != 0;
			// we're changing the state of bypass
			if (tempNewSetting != IsBypassEffect()) {
				if (!tempNewSetting && IsBypassEffect() &&
					IsInitialized()) { // turning bypass off and we're initialized
					Reset(kAudioUnitScope_Global, 0);
				}
				SetBypassEffect(tempNewSetting);
			}
			return noErr;
		}
		case kAudioUnitProperty_InPlaceProcessing:
			mProcessesInPlace = *static_cast<const UInt32*>(inData) != 0;
			return noErr;
		default:
			break;
		}
	}
	return AUBase::SetProperty(inID, inScope, inElement, inData, inDataSize);
}


void AUEffectBase::MaintainKernels()
{
#if TARGET_OS_IPHONE
	const UInt32 nKernels = mOnlyOneKernel ? 1 : GetNumberOfChannels();
#else
	const UInt32 nKernels = GetNumberOfChannels();
#endif

	if (mKernelList.size() < nKernels) {
		mKernelList.reserve(nKernels);
		for (auto i = static_cast<UInt32>(mKernelList.size()); i < nKernels; ++i) {
			mKernelList.push_back(NewKernel());
		}
	} else {
		while (mKernelList.size() > nKernels) {
			mKernelList.pop_back();
		}
	}

	for (UInt32 i = 0; i < nKernels; i++) {
		if (mKernelList[i]) {
			mKernelList[i]->SetChannelNum(i);
		}
	}
}

bool AUEffectBase::StreamFormatWritable(AudioUnitScope /*scope*/, AudioUnitElement /*element*/)
{
	return !IsInitialized();
}

OSStatus AUEffectBase::ChangeStreamFormat(AudioUnitScope inScope, AudioUnitElement inElement,
	const AudioStreamBasicDescription& inPrevFormat, const AudioStreamBasicDescription& inNewFormat)
{
	const OSStatus result =
		AUBase::ChangeStreamFormat(inScope, inElement, inPrevFormat, inNewFormat);
	if (result == noErr) {
		// for the moment this only dependency we know about
		// where a parameter's range may change is with the sample rate
		// and effects are only publishing parameters in the global scope!
		if (GetParamHasSampleRateDependency() &&
			inPrevFormat.mSampleRate != inNewFormat.mSampleRate) {
			PropertyChanged(kAudioUnitProperty_ParameterList, kAudioUnitScope_Global, 0);
		}
	}

	return result;
}


// ____________________________________________________________________________
//
//	This method is called (potentially repeatedly) by ProcessForScheduledParams()
//	in order to perform the actual DSP required for this portion of the entire buffer
//	being processed.  The entire buffer can be divided up into smaller "slices"
//	according to the timestamps on the scheduled parameters...
//
OSStatus AUEffectBase::ProcessScheduledSlice(void* inUserData, UInt32 /*inStartFrameInBuffer*/,
	UInt32 inSliceFramesToProcess, UInt32 /*inTotalBufferFrames*/)
{
	const ScheduledProcessParams& sliceParams = *static_cast<ScheduledProcessParams*>(inUserData);

	AudioUnitRenderActionFlags& actionFlags = *sliceParams.actionFlags;
	AudioBufferList& inputBufferList = *sliceParams.inputBufferList;
	AudioBufferList& outputBufferList = *sliceParams.outputBufferList;

	UInt32 channelSize = inSliceFramesToProcess * mBytesPerFrame;
	// fix the size of the buffer we're operating on before we render this slice of time
	for (UInt32 i = 0; i < inputBufferList.mNumberBuffers; i++) {
		inputBufferList.mBuffers[i].mDataByteSize =                    // NOLINT
			inputBufferList.mBuffers[i].mNumberChannels * channelSize; // NOLINT
	}

	for (UInt32 i = 0; i < outputBufferList.mNumberBuffers; i++) {
		outputBufferList.mBuffers[i].mDataByteSize =                    // NOLINT
			outputBufferList.mBuffers[i].mNumberChannels * channelSize; // NOLINT
	}
	// process the buffer
	const OSStatus result =
		ProcessBufferLists(actionFlags, inputBufferList, outputBufferList, inSliceFramesToProcess);

	// we just partially processed the buffers, so increment the data pointers to the next part of
	// the buffer to process
	for (UInt32 i = 0; i < inputBufferList.mNumberBuffers; i++) {
		inputBufferList.mBuffers[i].mData =                              // NOLINT
			static_cast<std::byte*>(inputBufferList.mBuffers[i].mData) + // NOLINT
			inputBufferList.mBuffers[i].mNumberChannels * channelSize;   // NOLINT
	}

	for (UInt32 i = 0; i < outputBufferList.mNumberBuffers; i++) {
		outputBufferList.mBuffers[i].mData =                              // NOLINT
			static_cast<std::byte*>(outputBufferList.mBuffers[i].mData) + // NOLINT
			outputBufferList.mBuffers[i].mNumberChannels * channelSize;   // NOLINT
	}

	return result;
}

// ____________________________________________________________________________
//

OSStatus AUEffectBase::Render(
	AudioUnitRenderActionFlags& ioActionFlags, const AudioTimeStamp& inTimeStamp, UInt32 nFrames)
{
	if (!HasInput(0)) {
		return kAudioUnitErr_NoConnection;
	}

	OSStatus result = noErr;

	result = mMainInput->PullInput(ioActionFlags, inTimeStamp, 0 /* element */, nFrames);

	if (result == noErr) {
		if (ProcessesInPlace() && mMainOutput->WillAllocateBuffer()) {
			mMainOutput->SetBufferList(mMainInput->GetBufferList());
		}

		if (ShouldBypassEffect()) {
			// leave silence bit alone

			if (!ProcessesInPlace()) {
				mMainInput->CopyBufferContentsTo(mMainOutput->GetBufferList());
			}
		} else {
			auto& paramEventList = GetParamEventList();

			if (paramEventList.empty()) {
				// this will read/write silence bit
				result = ProcessBufferLists(ioActionFlags, mMainInput->GetBufferList(),
					mMainOutput->GetBufferList(), nFrames);
			} else {
				// deal with scheduled parameters...

				AudioBufferList& inputBufferList = mMainInput->GetBufferList();
				AudioBufferList& outputBufferList = mMainOutput->GetBufferList();

				ScheduledProcessParams processParams{ .actionFlags = &ioActionFlags,
					.inputBufferList = &inputBufferList,
					.outputBufferList = &outputBufferList };

				// divide up the buffer into slices according to scheduled params then
				// do the DSP for each slice (ProcessScheduledSlice() called for each slice)
				result = ProcessForScheduledParams(paramEventList, nFrames, &processParams);


				// fixup the buffer pointers to how they were before we started
				const UInt32 channelSize = nFrames * mBytesPerFrame;
				for (UInt32 i = 0; i < inputBufferList.mNumberBuffers; i++) {
					const UInt32 size =
						inputBufferList.mBuffers[i].mNumberChannels * channelSize;         // NOLINT
					inputBufferList.mBuffers[i].mData =                                    // NOLINT
						static_cast<std::byte*>(inputBufferList.mBuffers[i].mData) - size; // NOLINT
					inputBufferList.mBuffers[i].mDataByteSize = size;                      // NOLINT
				}

				for (UInt32 i = 0; i < outputBufferList.mNumberBuffers; i++) {
					const UInt32 size =
						outputBufferList.mBuffers[i].mNumberChannels * channelSize; // NOLINT
					outputBufferList.mBuffers[i].mData =                            // NOLINT
						static_cast<std::byte*>(outputBufferList.mBuffers[i].mData) -
						size;                                          // NOLINT
					outputBufferList.mBuffers[i].mDataByteSize = size; // NOLINT
				}
			}
		}

		if (((ioActionFlags & kAudioUnitRenderAction_OutputIsSilence) != 0u) &&
			!ProcessesInPlace()) {
			AUBufferList::ZeroBuffer(mMainOutput->GetBufferList());
		}
	}

	return result;
}


OSStatus AUEffectBase::ProcessBufferLists(AudioUnitRenderActionFlags& ioActionFlags,
	const AudioBufferList& inBuffer, AudioBufferList& outBuffer, UInt32 inFramesToProcess)
{
	if (ShouldBypassEffect()) {
		return noErr;
	}

	bool ioSilence = false;

	const bool silentInput = IsInputSilent(ioActionFlags, inFramesToProcess);
	ioActionFlags |= kAudioUnitRenderAction_OutputIsSilence;

	for (UInt32 channel = 0; channel < mKernelList.size(); ++channel) {
		auto& kernel = mKernelList[channel];

		if (!kernel) {
			continue;
		}

		ioSilence = silentInput;
		const AudioBuffer* const srcBuffer = &inBuffer.mBuffers[channel]; // NOLINT subscript
		AudioBuffer* const destBuffer = &outBuffer.mBuffers[channel];     // NOLINT subscript

		kernel->Process(static_cast<const Float32*>(srcBuffer->mData),
			static_cast<Float32*>(destBuffer->mData), inFramesToProcess, ioSilence);

		if (!ioSilence) {
			ioActionFlags &= ~kAudioUnitRenderAction_OutputIsSilence;
		}
	}

	return noErr;
}

Float64 AUEffectBase::GetSampleRate() { return Output(0).GetStreamFormat().mSampleRate; }

UInt32 AUEffectBase::GetNumberOfChannels() { return Output(0).GetStreamFormat().mChannelsPerFrame; }

} // namespace ausdk

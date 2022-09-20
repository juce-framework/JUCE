/*!
	@file		AudioUnitSDK/MusicDeviceBase.cpp
	@copyright	© 2000-2021 Apple Inc. All rights reserved.
*/
#include <AudioUnitSDK/MusicDeviceBase.h>

namespace ausdk {


MusicDeviceBase::MusicDeviceBase(
	AudioComponentInstance inInstance, UInt32 numInputs, UInt32 numOutputs, UInt32 numGroups)
	: AUBase(inInstance, numInputs, numOutputs, numGroups), AUMIDIBase(*static_cast<AUBase*>(this))
{
}

OSStatus MusicDeviceBase::GetPropertyInfo(AudioUnitPropertyID inID, AudioUnitScope inScope,
	AudioUnitElement inElement, UInt32& outDataSize, bool& outWritable)
{
	OSStatus result = noErr;

	switch (inID) { // NOLINT if/else
	case kMusicDeviceProperty_InstrumentCount:
		if (inScope != kAudioUnitScope_Global) {
			return kAudioUnitErr_InvalidScope;
		}
		outDataSize = sizeof(UInt32);
		outWritable = false;
		result = noErr;
		break;
	default:
		result = AUBase::GetPropertyInfo(inID, inScope, inElement, outDataSize, outWritable);

		if (result == kAudioUnitErr_InvalidProperty) {
			result = AUMIDIBase::DelegateGetPropertyInfo(
				inID, inScope, inElement, outDataSize, outWritable);
		}
		break;
	}
	return result;
}

OSStatus MusicDeviceBase::GetProperty(
	AudioUnitPropertyID inID, AudioUnitScope inScope, AudioUnitElement inElement, void* outData)
{
	OSStatus result = noErr;

	switch (inID) { // NOLINT if/else
	case kMusicDeviceProperty_InstrumentCount:
		if (inScope != kAudioUnitScope_Global) {
			return kAudioUnitErr_InvalidScope;
		}
		return GetInstrumentCount(*static_cast<UInt32*>(outData));
	default:
		result = AUBase::GetProperty(inID, inScope, inElement, outData);

		if (result == kAudioUnitErr_InvalidProperty) {
			result = AUMIDIBase::DelegateGetProperty(inID, inScope, inElement, outData);
		}
	}

	return result;
}


OSStatus MusicDeviceBase::SetProperty(AudioUnitPropertyID inID, AudioUnitScope inScope,
	AudioUnitElement inElement, const void* inData, UInt32 inDataSize)

{

	OSStatus result = AUBase::SetProperty(inID, inScope, inElement, inData, inDataSize);

	if (result == kAudioUnitErr_InvalidProperty) {
		result = AUMIDIBase::DelegateSetProperty(inID, inScope, inElement, inData, inDataSize);
	}

	return result;
}

// For a MusicDevice that doesn't support separate instruments (ie. is mono-timbral)
// then this call should return an instrument count of zero and noErr
OSStatus MusicDeviceBase::GetInstrumentCount(UInt32& outInstCount) const
{
	outInstCount = 0;
	return noErr;
}

OSStatus MusicDeviceBase::HandleNoteOn(
	UInt8 inChannel, UInt8 inNoteNumber, UInt8 inVelocity, UInt32 inStartFrame)
{
	const MusicDeviceNoteParams params{ .argCount = 2,
		.mPitch = static_cast<Float32>(inNoteNumber),
		.mVelocity = static_cast<Float32>(inVelocity) };
	return StartNote(kMusicNoteEvent_UseGroupInstrument, inChannel, nullptr, inStartFrame, params);
}

OSStatus MusicDeviceBase::HandleNoteOff(
	UInt8 inChannel, UInt8 inNoteNumber, UInt8 /*inVelocity*/, UInt32 inStartFrame)
{
	return StopNote(inChannel, inNoteNumber, inStartFrame);
}

OSStatus MusicDeviceBase::HandleStartNoteMessage(MusicDeviceInstrumentID inInstrument,
	MusicDeviceGroupID inGroupID, NoteInstanceID* outNoteInstanceID, UInt32 inOffsetSampleFrame,
	const MusicDeviceNoteParams* inParams)
{
	if (inParams == nullptr || outNoteInstanceID == nullptr) {
		return kAudio_ParamError;
	}

	if (!IsInitialized()) {
		return kAudioUnitErr_Uninitialized;
	}

	return StartNote(inInstrument, inGroupID, outNoteInstanceID, inOffsetSampleFrame, *inParams);
}

} // namespace ausdk

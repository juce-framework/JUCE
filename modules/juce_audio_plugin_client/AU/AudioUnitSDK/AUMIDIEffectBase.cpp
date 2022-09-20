/*!
	@file		AudioUnitSDK/AUMIDIEffectBase.cpp
	@copyright	© 2000-2021 Apple Inc. All rights reserved.
*/
#include <AudioUnitSDK/AUMIDIEffectBase.h>

namespace ausdk {

AUMIDIEffectBase::AUMIDIEffectBase(AudioComponentInstance inInstance, bool inProcessesInPlace)
	: AUEffectBase(inInstance, inProcessesInPlace), AUMIDIBase(*static_cast<AUBase*>(this))
{
}

OSStatus AUMIDIEffectBase::GetPropertyInfo(AudioUnitPropertyID inID, AudioUnitScope inScope,
	AudioUnitElement inElement, UInt32& outDataSize, bool& outWritable)
{
	OSStatus result =
		AUEffectBase::GetPropertyInfo(inID, inScope, inElement, outDataSize, outWritable);

	if (result == kAudioUnitErr_InvalidProperty) {
		result =
			AUMIDIBase::DelegateGetPropertyInfo(inID, inScope, inElement, outDataSize, outWritable);
	}

	return result;
}

OSStatus AUMIDIEffectBase::GetProperty(
	AudioUnitPropertyID inID, AudioUnitScope inScope, AudioUnitElement inElement, void* outData)
{
	OSStatus result = AUEffectBase::GetProperty(inID, inScope, inElement, outData);

	if (result == kAudioUnitErr_InvalidProperty) {
		result = AUMIDIBase::DelegateGetProperty(inID, inScope, inElement, outData);
	}

	return result;
}

OSStatus AUMIDIEffectBase::SetProperty(AudioUnitPropertyID inID, AudioUnitScope inScope,
	AudioUnitElement inElement, const void* inData, UInt32 inDataSize)
{

	OSStatus result = AUEffectBase::SetProperty(inID, inScope, inElement, inData, inDataSize);

	if (result == kAudioUnitErr_InvalidProperty) {
		result = AUMIDIBase::DelegateSetProperty(inID, inScope, inElement, inData, inDataSize);
	}

	return result;
}

} // namespace ausdk

/*!
	@file		AudioUnitSDK/AUMIDIBase.cpp
	@copyright	© 2000-2021 Apple Inc. All rights reserved.
*/
#include <AudioUnitSDK/AUMIDIBase.h>
#include <CoreMIDI/CoreMIDI.h>

namespace ausdk {

// MIDI CC data bytes
constexpr uint8_t kMIDIController_AllSoundOff = 120u;
constexpr uint8_t kMIDIController_ResetAllControllers = 121u;
constexpr uint8_t kMIDIController_AllNotesOff = 123u;

OSStatus AUMIDIBase::DelegateGetPropertyInfo(AudioUnitPropertyID inID, AudioUnitScope inScope,
	AudioUnitElement inElement, UInt32& outDataSize, bool& outWritable)
{
	(void)inScope;
	(void)inElement;
	(void)outDataSize;
	(void)outWritable;

	switch (inID) { // NOLINT if/else?!
#if AUSDK_HAVE_XML_NAMES
	case kMusicDeviceProperty_MIDIXMLNames:
		AUSDK_Require(inScope == kAudioUnitScope_Global, kAudioUnitErr_InvalidScope);
		AUSDK_Require(inElement == 0, kAudioUnitErr_InvalidElement);
		AUSDK_Require(GetXMLNames(nullptr) == noErr, kAudioUnitErr_InvalidProperty);
		outDataSize = sizeof(CFURLRef);
		outWritable = false;
		return noErr;
#endif

#if AUSDK_HAVE_MIDI_MAPPING
	case kAudioUnitProperty_AllParameterMIDIMappings:
		AUSDK_Require(mMIDIMapper, kAudioUnitErr_InvalidProperty);
		AUSDK_Require(inScope == kAudioUnitScope_Global, kAudioUnitErr_InvalidScope);
		AUSDK_Require(inElement == 0, kAudioUnitErr_InvalidElement);
		outWritable = true;
		outDataSize = sizeof(AUParameterMIDIMapping) * mMIDIMapper->GetNumberMaps();
		return noErr;

	case kAudioUnitProperty_HotMapParameterMIDIMapping:
	case kAudioUnitProperty_AddParameterMIDIMapping:
	case kAudioUnitProperty_RemoveParameterMIDIMapping:
		AUSDK_Require(mMIDIMapper, kAudioUnitErr_InvalidProperty);
		AUSDK_Require(inScope == kAudioUnitScope_Global, kAudioUnitErr_InvalidScope);
		AUSDK_Require(inElement == 0, kAudioUnitErr_InvalidElement);
		outWritable = true;
		outDataSize = sizeof(AUParameterMIDIMapping);
		return noErr;
#endif

	default:
		return kAudioUnitErr_InvalidProperty;
	}
}

OSStatus AUMIDIBase::DelegateGetProperty(
	AudioUnitPropertyID inID, AudioUnitScope inScope, AudioUnitElement inElement, void* outData)
{
	(void)inScope;
	(void)inElement;
	(void)outData;

	switch (inID) { // NOLINT if/else?!
#if AUSDK_HAVE_XML_NAMES
	case kMusicDeviceProperty_MIDIXMLNames:
		AUSDK_Require(inScope == kAudioUnitScope_Global, kAudioUnitErr_InvalidScope);
		AUSDK_Require(inElement == 0, kAudioUnitErr_InvalidElement);
		return GetXMLNames(static_cast<CFURLRef*>(outData));
#endif

#if AUSDK_HAVE_MIDI_MAPPING
	case kAudioUnitProperty_AllParameterMIDIMappings: {
		AUSDK_Require(mMIDIMapper, kAudioUnitErr_InvalidProperty);
		AUSDK_Require(inScope == kAudioUnitScope_Global, kAudioUnitErr_InvalidScope);
		AUSDK_Require(inElement == 0, kAudioUnitErr_InvalidElement);
		AUParameterMIDIMapping* const maps = (static_cast<AUParameterMIDIMapping*>(outData));
		mMIDIMapper->GetMaps(maps);
		return noErr;
	}

	case kAudioUnitProperty_HotMapParameterMIDIMapping: {
		AUSDK_Require(mMIDIMapper, kAudioUnitErr_InvalidProperty);
		AUSDK_Require(inScope == kAudioUnitScope_Global, kAudioUnitErr_InvalidScope);
		AUSDK_Require(inElement == 0, kAudioUnitErr_InvalidElement);
		AUParameterMIDIMapping* const map = (static_cast<AUParameterMIDIMapping*>(outData));
		mMIDIMapper->GetHotParameterMap(*map);
		return noErr;
	}
#endif

	default:
		return kAudioUnitErr_InvalidProperty;
	}
}

OSStatus AUMIDIBase::DelegateSetProperty(AudioUnitPropertyID inID, AudioUnitScope inScope,
	AudioUnitElement inElement, const void* inData, UInt32 inDataSize)
{
	(void)inScope;
	(void)inElement;
	(void)inData;
	(void)inDataSize;

	switch (inID) {

#if AUSDK_HAVE_MIDI_MAPPING
	case kAudioUnitProperty_AddParameterMIDIMapping: {
		AUSDK_Require(mMIDIMapper, kAudioUnitErr_InvalidProperty);
		AUSDK_Require(inScope == kAudioUnitScope_Global, kAudioUnitErr_InvalidScope);
		AUSDK_Require(inElement == 0, kAudioUnitErr_InvalidElement);
		const auto* const maps = static_cast<const AUParameterMIDIMapping*>(inData);
		mMIDIMapper->AddParameterMapping(
			maps, (inDataSize / sizeof(AUParameterMIDIMapping)), mAUBaseInstance);
		mAUBaseInstance.PropertyChanged(
			kAudioUnitProperty_AllParameterMIDIMappings, kAudioUnitScope_Global, 0);
		return noErr;
	}

	case kAudioUnitProperty_RemoveParameterMIDIMapping: {
		AUSDK_Require(mMIDIMapper, kAudioUnitErr_InvalidProperty);
		AUSDK_Require(inScope == kAudioUnitScope_Global, kAudioUnitErr_InvalidScope);
		AUSDK_Require(inElement == 0, kAudioUnitErr_InvalidElement);
		const auto* const maps = static_cast<const AUParameterMIDIMapping*>(inData);
		bool didChange = false;
		mMIDIMapper->RemoveParameterMapping(
			maps, (inDataSize / sizeof(AUParameterMIDIMapping)), didChange);
		if (didChange) {
			mAUBaseInstance.PropertyChanged(
				kAudioUnitProperty_AllParameterMIDIMappings, kAudioUnitScope_Global, 0);
		}
		return noErr;
	}

	case kAudioUnitProperty_HotMapParameterMIDIMapping: {
		AUSDK_Require(mMIDIMapper, kAudioUnitErr_InvalidProperty);
		AUSDK_Require(inScope == kAudioUnitScope_Global, kAudioUnitErr_InvalidScope);
		AUSDK_Require(inElement == 0, kAudioUnitErr_InvalidElement);
		const auto& map = *static_cast<const AUParameterMIDIMapping*>(inData);
		mMIDIMapper->SetHotMapping(map);
		return noErr;
	}

	case kAudioUnitProperty_AllParameterMIDIMappings: {
		AUSDK_Require(mMIDIMapper, kAudioUnitErr_InvalidProperty);
		AUSDK_Require(inScope == kAudioUnitScope_Global, kAudioUnitErr_InvalidScope);
		AUSDK_Require(inElement == 0, kAudioUnitErr_InvalidElement);
		const auto* const mappings = static_cast<const AUParameterMIDIMapping*>(inData);
		mMIDIMapper->ReplaceAllMaps(
			mappings, (inDataSize / sizeof(AUParameterMIDIMapping)), mAUBaseInstance);
		return noErr;
	}
#endif

	default:
		return kAudioUnitErr_InvalidProperty;
	}
}

constexpr uint8_t MIDIStatusNibbleValue(uint8_t status) noexcept { return (status & 0xF0U) >> 4u; }

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	AUMIDIBase::HandleMIDIEvent
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
OSStatus AUMIDIBase::HandleMIDIEvent(
	UInt8 status, UInt8 channel, UInt8 data1, UInt8 data2, UInt32 inStartFrame)
{
	if (!mAUBaseInstance.IsInitialized()) {
		return kAudioUnitErr_Uninitialized;
	}

#if AUSDK_HAVE_MIDI_MAPPING
	// you potentially have a choice to make here - if a param mapping matches, do you still want to
	// process the MIDI event or not. The default behaviour is to continue on with the MIDI event.
	if (mMIDIMapper) {
		if (mMIDIMapper->HandleHotMapping(status, channel, data1, mAUBaseInstance)) {
			mAUBaseInstance.PropertyChanged(
				kAudioUnitProperty_HotMapParameterMIDIMapping, kAudioUnitScope_Global, 0);
		} else {
			mMIDIMapper->FindParameterMapEventMatch(
				status, channel, data1, data2, inStartFrame, mAUBaseInstance);
		}
	}
#endif
	switch (MIDIStatusNibbleValue(status)) {
	case kMIDICVStatusNoteOn:
		if (data2 != 0u) {
			return HandleNoteOn(channel, data1, data2, inStartFrame);
		} else {
			// zero velocity translates to note off
			return HandleNoteOff(channel, data1, data2, inStartFrame);
		}

	case kMIDICVStatusNoteOff:
		return HandleNoteOff(channel, data1, data2, inStartFrame);

	default:
		return HandleNonNoteEvent(status, channel, data1, data2, inStartFrame);
	}
}

OSStatus AUMIDIBase::HandleNonNoteEvent(
	UInt8 status, UInt8 channel, UInt8 data1, UInt8 data2, UInt32 inStartFrame)
{
	switch (MIDIStatusNibbleValue(status)) {
	case kMIDICVStatusPitchBend:
		return HandlePitchWheel(channel, data1, data2, inStartFrame);

	case kMIDICVStatusProgramChange:
		return HandleProgramChange(channel, data1);

	case kMIDICVStatusChannelPressure:
		return HandleChannelPressure(channel, data1, inStartFrame);

	case kMIDICVStatusControlChange: {
		switch (data1) {
		case kMIDIController_AllNotesOff:
			return HandleAllNotesOff(channel);

		case kMIDIController_ResetAllControllers:
			return HandleResetAllControllers(channel);

		case kMIDIController_AllSoundOff:
			return HandleAllSoundOff(channel);

		default:
			return HandleControlChange(channel, data1, data2, inStartFrame);
		}
	}

	case kMIDICVStatusPolyPressure:
		return HandlePolyPressure(channel, data1, data2, inStartFrame);

	default:
		return noErr;
	}
}

OSStatus AUMIDIBase::SysEx(const UInt8* inData, UInt32 inLength)
{
	if (!mAUBaseInstance.IsInitialized()) {
		return kAudioUnitErr_Uninitialized;
	}

	return HandleSysEx(inData, inLength);
}

} // namespace ausdk

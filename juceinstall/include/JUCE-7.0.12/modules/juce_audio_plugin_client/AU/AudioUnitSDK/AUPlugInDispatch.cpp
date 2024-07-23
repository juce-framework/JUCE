/*!
	@file		AudioUnitSDK/AUPlugInDispatch.cpp
	@copyright	© 2000-2021 Apple Inc. All rights reserved.
*/
#include <AudioUnitSDK/AUBase.h>
#include <AudioUnitSDK/AUPlugInDispatch.h>
#include <AudioUnitSDK/AUUtility.h>
#include <AudioUnitSDK/ComponentBase.h>

#include <cmath>
#include <cstddef>

#define CATCH_EXCEPTIONS_IN_RENDER_METHODS TARGET_OS_OSX // NOLINT
#define HAVE_MUSICDEVICE_PREPARE_RELEASE TARGET_OS_OSX   // NOLINT

namespace ausdk {

// ------------------------------------------------------------------------------------------------
static auto AUInstance(void* self)
{
	return reinterpret_cast<AUBase*>( // NOLINT reinterpret_cast
		&(static_cast<AudioComponentPlugInInstance*>(self)->mInstanceStorage));
}

// ------------------------------------------------------------------------------------------------
class AUInstanceGuard {
public:
	explicit AUInstanceGuard(void* self) : mGuard(AUInstance(self)->GetMutex()) {}

private:
	const AUEntryGuard mGuard;
};

// ------------------------------------------------------------------------------------------------
static bool IsValidParameterValue(AudioUnitParameterValue value) { return std::isfinite(value); }

static bool AreValidParameterEvents(const AudioUnitParameterEvent* events, UInt32 numEvents)
{
	if (events == nullptr) {
		return true;
	}

	for (UInt32 i = 0; i < numEvents; ++i) {
		const auto& event = events[i]; // NOLINT
		switch (event.eventType) {
		case kParameterEvent_Immediate: {
			if (!IsValidParameterValue(event.eventValues.immediate.value)) { // NOLINT
				return false;
			}
			break;
		}

		case kParameterEvent_Ramped: {
			if (!IsValidParameterValue(event.eventValues.ramp.startValue) || // NOLINT
				!IsValidParameterValue(event.eventValues.ramp.endValue)) {   // NOLINT
				return false;
			}
			break;
		}

		default:
			break;
		}
	}

	return true;
}

// ------------------------------------------------------------------------------------------------
static OSStatus AUMethodInitialize(void* self)
{
	OSStatus result = noErr;
	try {
		const AUInstanceGuard guard(self);
		result = AUInstance(self)->DoInitialize();
	}
	AUSDK_Catch(result)
	return result;
}

static OSStatus AUMethodUninitialize(void* self)
{
	OSStatus result = noErr;
	try {
		const AUInstanceGuard guard(self);
		AUInstance(self)->DoCleanup();
	}
	AUSDK_Catch(result)
	return result;
}

static OSStatus AUMethodGetPropertyInfo(void* self, AudioUnitPropertyID prop, AudioUnitScope scope,
	AudioUnitElement elem, UInt32* outDataSize, Boolean* outWritable)
{
	OSStatus result = noErr;
	try {
		UInt32 dataSize = 0; // 13517289 GetPropetyInfo was returning an uninitialized value when
							 // there is an error. This is a problem for auval.
		bool writable = false;

		const AUInstanceGuard guard(self);
		result = AUInstance(self)->DispatchGetPropertyInfo(prop, scope, elem, dataSize, writable);
		if (outDataSize != nullptr) {
			*outDataSize = dataSize;
		}
		if (outWritable != nullptr) {
			*outWritable = static_cast<Boolean>(writable);
		}
	}
	AUSDK_Catch(result)
	return result;
}

static OSStatus AUMethodGetProperty(void* self, AudioUnitPropertyID inID, AudioUnitScope inScope,
	AudioUnitElement inElement, void* outData, UInt32* ioDataSize)
{
	OSStatus result = noErr;
	try {
		bool writable = false;

		const AUInstanceGuard guard(self);
		if (ioDataSize == nullptr) {
			AUSDK_LogError("AudioUnitGetProperty: null size pointer");
			return kAudio_ParamError;
		}
		if (outData == nullptr) {
			UInt32 dataSize = 0;

			result = AUInstance(self)->DispatchGetPropertyInfo(
				inID, inScope, inElement, dataSize, writable);
			*ioDataSize = dataSize;
			return result;
		}

		const auto clientBufferSize = *ioDataSize;
		if (clientBufferSize == 0) {
			AUSDK_LogError("AudioUnitGetProperty: *ioDataSize == 0 on entry");
			return kAudio_ParamError;
		}

		UInt32 actualPropertySize = 0;
		result = AUInstance(self)->DispatchGetPropertyInfo(
			inID, inScope, inElement, actualPropertySize, writable);
		if (result != noErr) {
			return result;
		}

		std::vector<std::byte> tempBuffer;
		void* destBuffer = nullptr;
		if (clientBufferSize < actualPropertySize) {
			tempBuffer.resize(actualPropertySize);
			destBuffer = tempBuffer.data();
		} else {
			destBuffer = outData;
		}

		result = AUInstance(self)->DispatchGetProperty(inID, inScope, inElement, destBuffer);

		if (result == noErr) {
			if (clientBufferSize < actualPropertySize && !tempBuffer.empty()) {
				memcpy(outData, tempBuffer.data(), clientBufferSize);
				// ioDataSize remains correct, the number of bytes we wrote
			} else {
				*ioDataSize = actualPropertySize;
			}
		} else {
			*ioDataSize = 0;
		}
	}
	AUSDK_Catch(result)
	return result;
}

static OSStatus AUMethodSetProperty(void* self, AudioUnitPropertyID inID, AudioUnitScope inScope,
	AudioUnitElement inElement, const void* inData, UInt32 inDataSize)
{
	OSStatus result = noErr;
	try {
		const AUInstanceGuard guard(self);
		if ((inData != nullptr) && (inDataSize != 0u)) {
			result =
				AUInstance(self)->DispatchSetProperty(inID, inScope, inElement, inData, inDataSize);
		} else {
			if (inData == nullptr && inDataSize == 0) {
				result = AUInstance(self)->DispatchRemovePropertyValue(inID, inScope, inElement);
			} else {
				if (inData == nullptr) {
					AUSDK_LogError("AudioUnitSetProperty: inData == NULL");
					return kAudio_ParamError;
				}

				if (inDataSize == 0) {
					AUSDK_LogError("AudioUnitSetProperty: inDataSize == 0");
					return kAudio_ParamError;
				}
			}
		}
	}
	AUSDK_Catch(result)
	return result;
}

static OSStatus AUMethodAddPropertyListener(
	void* self, AudioUnitPropertyID prop, AudioUnitPropertyListenerProc proc, void* userData)
{
	OSStatus result = noErr;
	try {
		const AUInstanceGuard guard(self);
		result = AUInstance(self)->AddPropertyListener(prop, proc, userData);
	}
	AUSDK_Catch(result)
	return result;
}

static OSStatus AUMethodRemovePropertyListener(
	void* self, AudioUnitPropertyID prop, AudioUnitPropertyListenerProc proc)
{
	OSStatus result = noErr;
	try {
		const AUInstanceGuard guard(self);
		result = AUInstance(self)->RemovePropertyListener(prop, proc, nullptr, false);
	}
	AUSDK_Catch(result)
	return result;
}

static OSStatus AUMethodRemovePropertyListenerWithUserData(
	void* self, AudioUnitPropertyID prop, AudioUnitPropertyListenerProc proc, void* userData)
{
	OSStatus result = noErr;
	try {
		const AUInstanceGuard guard(self);
		result = AUInstance(self)->RemovePropertyListener(prop, proc, userData, true);
	}
	AUSDK_Catch(result)
	return result;
}

static OSStatus AUMethodAddRenderNotify(void* self, AURenderCallback proc, void* userData)
{
	OSStatus result = noErr;
	try {
		const AUInstanceGuard guard(self);
		result = AUInstance(self)->SetRenderNotification(proc, userData);
	}
	AUSDK_Catch(result)
	return result;
}

static OSStatus AUMethodRemoveRenderNotify(void* self, AURenderCallback proc, void* userData)
{
	OSStatus result = noErr;
	try {
		const AUInstanceGuard guard(self);
		result = AUInstance(self)->RemoveRenderNotification(proc, userData);
	}
	AUSDK_Catch(result)
	return result;
}

static OSStatus AUMethodGetParameter(void* self, AudioUnitParameterID param, AudioUnitScope scope,
	AudioUnitElement elem, AudioUnitParameterValue* value)
{
	OSStatus result = noErr;
	try {
		const AUInstanceGuard guard(self);
		result = (value == nullptr ? kAudio_ParamError
								   : AUInstance(self)->GetParameter(param, scope, elem, *value));
	}
	AUSDK_Catch(result)
	return result;
}

static OSStatus AUMethodSetParameter(void* self, AudioUnitParameterID param, AudioUnitScope scope,
	AudioUnitElement elem, AudioUnitParameterValue value, UInt32 bufferOffset)
{
	if (!IsValidParameterValue(value)) {
		return kAudioUnitErr_InvalidParameterValue;
	}

	OSStatus result = noErr;
	try {
		// this is a (potentially) realtime method; no lock
		result = AUInstance(self)->SetParameter(param, scope, elem, value, bufferOffset);
	}
	AUSDK_Catch(result)
	return result;
}

static OSStatus AUMethodScheduleParameters(
	void* self, const AudioUnitParameterEvent* events, UInt32 numEvents)
{
	if (!AreValidParameterEvents(events, numEvents)) {
		return kAudioUnitErr_InvalidParameterValue;
	}

	OSStatus result = noErr;
	try {
		// this is a (potentially) realtime method; no lock
		result = AUInstance(self)->ScheduleParameter(events, numEvents);
	}
	AUSDK_Catch(result)
	return result;
}

static OSStatus AUMethodRender(void* self, AudioUnitRenderActionFlags* ioActionFlags,
	const AudioTimeStamp* inTimeStamp, UInt32 inOutputBusNumber, UInt32 inNumberFrames,
	AudioBufferList* ioData)
{
	OSStatus result = noErr;

#if CATCH_EXCEPTIONS_IN_RENDER_METHODS
	try {
#endif
		// this is a processing method; no lock
		AudioUnitRenderActionFlags tempFlags{};

		if (inTimeStamp == nullptr || ioData == nullptr) {
			result = kAudio_ParamError;
		} else {
			if (ioActionFlags == nullptr) {
				tempFlags = 0;
				ioActionFlags = &tempFlags;
			}
			result = AUInstance(self)->DoRender(
				*ioActionFlags, *inTimeStamp, inOutputBusNumber, inNumberFrames, *ioData);
		}

#if CATCH_EXCEPTIONS_IN_RENDER_METHODS
	}
	AUSDK_Catch(result)
#endif

	return result;
}

static OSStatus AUMethodComplexRender(void* self, AudioUnitRenderActionFlags* ioActionFlags,
	const AudioTimeStamp* inTimeStamp, UInt32 inOutputBusNumber, UInt32 inNumberOfPackets,
	UInt32* outNumberOfPackets, AudioStreamPacketDescription* outPacketDescriptions,
	AudioBufferList* ioData, void* outMetadata, UInt32* outMetadataByteSize)
{
	OSStatus result = noErr;

#if CATCH_EXCEPTIONS_IN_RENDER_METHODS
	try {
#endif
		// this is a processing method; no lock
		AudioUnitRenderActionFlags tempFlags{};

		if (inTimeStamp == nullptr || ioData == nullptr) {
			result = kAudio_ParamError;
		} else {
			if (ioActionFlags == nullptr) {
				tempFlags = 0;
				ioActionFlags = &tempFlags;
			}
			result = AUInstance(self)->ComplexRender(*ioActionFlags, *inTimeStamp,
				inOutputBusNumber, inNumberOfPackets, outNumberOfPackets, outPacketDescriptions,
				*ioData, outMetadata, outMetadataByteSize);
		}

#if CATCH_EXCEPTIONS_IN_RENDER_METHODS
	}
	AUSDK_Catch(result)
#endif

	return result;
}

static OSStatus AUMethodReset(void* self, AudioUnitScope scope, AudioUnitElement elem)
{
	OSStatus result = noErr;
	try {
		const AUInstanceGuard guard(self);
		result = AUInstance(self)->Reset(scope, elem);
	}
	AUSDK_Catch(result)
	return result;
}

static OSStatus AUMethodProcess(void* self, AudioUnitRenderActionFlags* ioActionFlags,
	const AudioTimeStamp* inTimeStamp, UInt32 inNumberFrames, AudioBufferList* ioData)
{
	OSStatus result = noErr;

#if CATCH_EXCEPTIONS_IN_RENDER_METHODS
	try {
#endif
		// this is a processing method; no lock
		bool doParamCheck = true;

		AudioUnitRenderActionFlags tempFlags{};

		if (ioActionFlags == nullptr) {
			tempFlags = 0;
			ioActionFlags = &tempFlags;
		} else {
			if ((*ioActionFlags & kAudioUnitRenderAction_DoNotCheckRenderArgs) != 0u) {
				doParamCheck = false;
			}
		}

		if (doParamCheck && (inTimeStamp == nullptr || ioData == nullptr)) {
			result = kAudio_ParamError;
		} else {
			result =
				AUInstance(self)->DoProcess(*ioActionFlags, *inTimeStamp, inNumberFrames, *ioData);
		}

#if CATCH_EXCEPTIONS_IN_RENDER_METHODS
	}
	AUSDK_Catch(result)
#endif

	return result;
}

static OSStatus AUMethodProcessMultiple(void* self, AudioUnitRenderActionFlags* ioActionFlags,
	const AudioTimeStamp* inTimeStamp, UInt32 inNumberFrames, UInt32 inNumberInputBufferLists,
	const AudioBufferList** inInputBufferLists, UInt32 inNumberOutputBufferLists,
	AudioBufferList** ioOutputBufferLists)
{
	OSStatus result = noErr;

#if CATCH_EXCEPTIONS_IN_RENDER_METHODS
	try {
#endif
		// this is a processing method; no lock
		bool doParamCheck = true;

		AudioUnitRenderActionFlags tempFlags{};

		if (ioActionFlags == nullptr) {
			tempFlags = 0;
			ioActionFlags = &tempFlags;
		} else {
			if ((*ioActionFlags & kAudioUnitRenderAction_DoNotCheckRenderArgs) != 0u) {
				doParamCheck = false;
			}
		}

		if (doParamCheck && (inTimeStamp == nullptr || inInputBufferLists == nullptr ||
								ioOutputBufferLists == nullptr)) {
			result = kAudio_ParamError;
		} else {
			result = AUInstance(self)->DoProcessMultiple(*ioActionFlags, *inTimeStamp,
				inNumberFrames, inNumberInputBufferLists, inInputBufferLists,
				inNumberOutputBufferLists, ioOutputBufferLists);
		}

#if CATCH_EXCEPTIONS_IN_RENDER_METHODS
	}
	AUSDK_Catch(result)
#endif

	return result;
}
// ------------------------------------------------------------------------------------------------

static OSStatus AUMethodStart(void* self)
{
	OSStatus result = noErr;
	try {
		const AUInstanceGuard guard(self);
		result = AUInstance(self)->Start();
	}
	AUSDK_Catch(result)
	return result;
}

static OSStatus AUMethodStop(void* self)
{
	OSStatus result = noErr;
	try {
		const AUInstanceGuard guard(self);
		result = AUInstance(self)->Stop();
	}
	AUSDK_Catch(result)
	return result;
}

// ------------------------------------------------------------------------------------------------

// I don't know what I'm doing here; conflicts with the multiple inheritence in MusicDeviceBase.
static OSStatus AUMethodMIDIEvent(
	void* self, UInt32 inStatus, UInt32 inData1, UInt32 inData2, UInt32 inOffsetSampleFrame)
{
	OSStatus result = noErr;
	try {
		// this is a potential render-time method; no lock
		result = AUInstance(self)->MIDIEvent(inStatus, inData1, inData2, inOffsetSampleFrame);
	}
	AUSDK_Catch(result)
	return result;
}

static OSStatus AUMethodSysEx(void* self, const UInt8* inData, UInt32 inLength)
{
	OSStatus result = noErr;
	try {
		// this is a potential render-time method; no lock
		result = AUInstance(self)->SysEx(inData, inLength);
	}
	AUSDK_Catch(result)
	return result;
}

#if AUSDK_MIDI2_AVAILABLE
static OSStatus AUMethodMIDIEventList(
	void* self, UInt32 inOffsetSampleFrame, const struct MIDIEventList* eventList)
{
	if (eventList == nullptr) {
		return kAudio_ParamError;
	}

	OSStatus result = noErr;
	try {
		// this is a potential render-time method; no lock

		// Note that a MIDIEventList is variably-sized and can be backed by less memory than
		// required, so it is Undefined Behavior to form a reference to it; we must only use
		// pointers.
		result = AUInstance(self)->MIDIEventList(inOffsetSampleFrame, eventList);
	}
	AUSDK_Catch(result)
	return result;
}
#endif

static OSStatus AUMethodStartNote(void* self, MusicDeviceInstrumentID inInstrument,
	MusicDeviceGroupID inGroupID, NoteInstanceID* outNoteInstanceID, UInt32 inOffsetSampleFrame,
	const MusicDeviceNoteParams* inParams)
{
	OSStatus result = noErr;
	try {
		// this is a potential render-time method; no lock
		if (inParams == nullptr) {
			result = kAudio_ParamError;
		} else {
			result = AUInstance(self)->StartNote(
				inInstrument, inGroupID, outNoteInstanceID, inOffsetSampleFrame, *inParams);
		}
	}
	AUSDK_Catch(result)
	return result;
}

static OSStatus AUMethodStopNote(void* self, MusicDeviceGroupID inGroupID,
	NoteInstanceID inNoteInstanceID, UInt32 inOffsetSampleFrame)
{
	OSStatus result = noErr;
	try {
		// this is a potential render-time method; no lock
		result = AUInstance(self)->StopNote(inGroupID, inNoteInstanceID, inOffsetSampleFrame);
	}
	AUSDK_Catch(result)
	return result;
}

#if HAVE_MUSICDEVICE_PREPARE_RELEASE
static OSStatus AUMethodPrepareInstrument(void* self, MusicDeviceInstrumentID inInstrument)
{
	OSStatus result = noErr;
	try {
		// this is a potential render-time method; no lock
		result = AUInstance(self)->PrepareInstrument(inInstrument); // NOLINT static via instance
	}
	AUSDK_Catch(result)
	return result;
}

static OSStatus AUMethodReleaseInstrument(void* self, MusicDeviceInstrumentID inInstrument)
{
	OSStatus result = noErr;
	try {
		// this is a potential render-time method; no lock
		result = AUInstance(self)->ReleaseInstrument(inInstrument); // NOLINT static via instance
	}
	AUSDK_Catch(result)
	return result;
}
#endif // HAVE_MUSICDEVICE_PREPARE_RELEASE


//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#pragma mark -
#pragma mark Lookup Methods

AudioComponentMethod AUBaseLookup::Lookup(SInt16 selector)
{
	switch (selector) {
	case kAudioUnitInitializeSelect:
		return (AudioComponentMethod)AUMethodInitialize; // NOLINT cast
	case kAudioUnitUninitializeSelect:
		return (AudioComponentMethod)AUMethodUninitialize; // NOLINT cast
	case kAudioUnitGetPropertyInfoSelect:
		return (AudioComponentMethod)AUMethodGetPropertyInfo; // NOLINT cast
	case kAudioUnitGetPropertySelect:
		return (AudioComponentMethod)AUMethodGetProperty; // NOLINT cast
	case kAudioUnitSetPropertySelect:
		return (AudioComponentMethod)AUMethodSetProperty; // NOLINT cast
	case kAudioUnitAddPropertyListenerSelect:
		return (AudioComponentMethod)AUMethodAddPropertyListener; // NOLINT cast
	case kAudioUnitRemovePropertyListenerSelect:
		return (AudioComponentMethod)AUMethodRemovePropertyListener; // NOLINT cast
	case kAudioUnitRemovePropertyListenerWithUserDataSelect:
		return (AudioComponentMethod)AUMethodRemovePropertyListenerWithUserData; // NOLINT cast
	case kAudioUnitAddRenderNotifySelect:
		return (AudioComponentMethod)AUMethodAddRenderNotify; // NOLINT cast
	case kAudioUnitRemoveRenderNotifySelect:
		return (AudioComponentMethod)AUMethodRemoveRenderNotify; // NOLINT cast
	case kAudioUnitGetParameterSelect:
		return (AudioComponentMethod)AUMethodGetParameter; // NOLINT cast
	case kAudioUnitSetParameterSelect:
		return (AudioComponentMethod)AUMethodSetParameter; // NOLINT cast
	case kAudioUnitScheduleParametersSelect:
		return (AudioComponentMethod)AUMethodScheduleParameters; // NOLINT cast
	case kAudioUnitRenderSelect:
		return (AudioComponentMethod)AUMethodRender; // NOLINT cast
	case kAudioUnitResetSelect:
		return (AudioComponentMethod)AUMethodReset; // NOLINT cast
	default:
		break;
	}
	return nullptr;
}

AudioComponentMethod AUOutputLookup::Lookup(SInt16 selector)
{
	const AudioComponentMethod method = AUBaseLookup::Lookup(selector);
	if (method != nullptr) {
		return method;
	}

	switch (selector) {
	case kAudioOutputUnitStartSelect:
		return (AudioComponentMethod)AUMethodStart; // NOLINT cast
	case kAudioOutputUnitStopSelect:
		return (AudioComponentMethod)AUMethodStop; // NOLINT cast
	default:
		break;
	}
	return nullptr;
}

AudioComponentMethod AUComplexOutputLookup::Lookup(SInt16 selector)
{
	AudioComponentMethod method = AUBaseLookup::Lookup(selector);
	if (method != nullptr) {
		return method;
	}

	method = AUOutputLookup::Lookup(selector);
	if (method != nullptr) {
		return method;
	}

	if (selector == kAudioUnitComplexRenderSelect) {
		return (AudioComponentMethod)AUMethodComplexRender; // NOLINT cast
	}
	return nullptr;
}

AudioComponentMethod AUBaseProcessLookup::Lookup(SInt16 selector)
{
	const AudioComponentMethod method = AUBaseLookup::Lookup(selector);
	if (method != nullptr) {
		return method;
	}

	if (selector == kAudioUnitProcessSelect) {
		return (AudioComponentMethod)AUMethodProcess; // NOLINT cast
	}

	return nullptr;
}

AudioComponentMethod AUBaseProcessMultipleLookup::Lookup(SInt16 selector)
{
	const AudioComponentMethod method = AUBaseLookup::Lookup(selector);
	if (method != nullptr) {
		return method;
	}

	if (selector == kAudioUnitProcessMultipleSelect) {
		return (AudioComponentMethod)AUMethodProcessMultiple; // NOLINT cast
	}

	return nullptr;
}

AudioComponentMethod AUBaseProcessAndMultipleLookup::Lookup(SInt16 selector)
{
	AudioComponentMethod method = AUBaseLookup::Lookup(selector);
	if (method != nullptr) {
		return method;
	}

	method = AUBaseProcessMultipleLookup::Lookup(selector);
	if (method != nullptr) {
		return method;
	}

	method = AUBaseProcessLookup::Lookup(selector);
	if (method != nullptr) {
		return method;
	}

	return nullptr;
}

inline AudioComponentMethod MIDI_Lookup(SInt16 selector)
{
	switch (selector) {
	case kMusicDeviceMIDIEventSelect:
		return (AudioComponentMethod)AUMethodMIDIEvent; // NOLINT cast
	case kMusicDeviceSysExSelect:
		return (AudioComponentMethod)AUMethodSysEx; // NOLINT cast
#if AUSDK_MIDI2_AVAILABLE
	case kMusicDeviceMIDIEventListSelect:
		return (AudioComponentMethod)AUMethodMIDIEventList; // NOLINT cast
#endif
	default:
		break;
	}
	return nullptr;
}

AudioComponentMethod AUMIDILookup::Lookup(SInt16 selector)
{
	const AudioComponentMethod method = AUBaseLookup::Lookup(selector);
	if (method != nullptr) {
		return method;
	}

	return MIDI_Lookup(selector);
}

AudioComponentMethod AUMIDIProcessLookup::Lookup(SInt16 selector)
{
	const AudioComponentMethod method = AUBaseProcessLookup::Lookup(selector);
	if (method != nullptr) {
		return method;
	}

	return MIDI_Lookup(selector);
}

AudioComponentMethod AUMusicLookup::Lookup(SInt16 selector)
{
	const AudioComponentMethod method = AUBaseLookup::Lookup(selector);
	if (method != nullptr) {
		return method;
	}

	switch (selector) {
	case kMusicDeviceStartNoteSelect:
		return (AudioComponentMethod)AUMethodStartNote; // NOLINT cast
	case kMusicDeviceStopNoteSelect:
		return (AudioComponentMethod)AUMethodStopNote; // NOLINT cast
#if HAVE_MUSICDEVICE_PREPARE_RELEASE
	case kMusicDevicePrepareInstrumentSelect:
		return (AudioComponentMethod)AUMethodPrepareInstrument; // NOLINT cast
	case kMusicDeviceReleaseInstrumentSelect:
		return (AudioComponentMethod)AUMethodReleaseInstrument; // NOLINT cast
#endif
	default:
		break;
	}
	return MIDI_Lookup(selector);
}

} // namespace ausdk

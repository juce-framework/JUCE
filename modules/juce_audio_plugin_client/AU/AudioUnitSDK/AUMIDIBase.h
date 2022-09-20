/*!
	@file		AudioUnitSDK/AUMIDIBase.h
	@copyright	© 2000-2021 Apple Inc. All rights reserved.
*/
#ifndef AudioUnitSDK_AUMIDIBase_h
#define AudioUnitSDK_AUMIDIBase_h

#include <AudioUnitSDK/AUBase.h>


#ifndef AUSDK_HAVE_XML_NAMES
#define AUSDK_HAVE_XML_NAMES TARGET_OS_OSX // NOLINT(cppcoreguidelines-macro-usage)
#endif

#ifndef AUSDK_HAVE_MIDI_MAPPING
#define AUSDK_HAVE_MIDI_MAPPING TARGET_OS_OSX // NOLINT(cppcoreguidelines-macro-usage)
#endif

namespace ausdk {

#if AUSDK_HAVE_MIDI_MAPPING
/// Abstract interface for parameter MIDI mapping
class AUMIDIMapper {
public:
	AUMIDIMapper() = default;
	virtual ~AUMIDIMapper() = default;

	AUMIDIMapper(const AUMIDIMapper&) = delete;
	AUMIDIMapper(AUMIDIMapper&&) = delete;
	AUMIDIMapper& operator=(const AUMIDIMapper&) = delete;
	AUMIDIMapper& operator=(AUMIDIMapper&&) = delete;

	[[nodiscard]] virtual UInt32 GetNumberMaps() const = 0;
	virtual void GetMaps(AUParameterMIDIMapping* outMapping) = 0;
	virtual void GetHotParameterMap(AUParameterMIDIMapping& outMapping) = 0;

	virtual void AddParameterMapping(
		const AUParameterMIDIMapping* maps, UInt32 count, AUBase& auBase) = 0;
	virtual void RemoveParameterMapping(
		const AUParameterMIDIMapping* maps, UInt32 count, bool& outDidChange) = 0;
	virtual void SetHotMapping(const AUParameterMIDIMapping& mapping) = 0;
	virtual void ReplaceAllMaps(
		const AUParameterMIDIMapping* maps, UInt32 count, AUBase& auBase) = 0;

	virtual bool HandleHotMapping(UInt8 status, UInt8 channel, UInt8 data1, AUBase& auBase) = 0;
	virtual bool FindParameterMapEventMatch(UInt8 status, UInt8 channel, UInt8 data1, UInt8 data2,
		UInt32 inStartFrame, AUBase& auBase) = 0;
};
#endif

// ________________________________________________________________________
//	AUMIDIBase
//
/*!
	@class	AUMIDIBase
	@brief	Auxiliary class supporting MIDI events.
*/
class AUMIDIBase {
public:
	explicit AUMIDIBase(AUBase& inBase) : mAUBaseInstance(inBase) {}

	virtual ~AUMIDIBase() = default;

	AUMIDIBase(const AUMIDIBase&) = delete;
	AUMIDIBase(AUMIDIBase&&) = delete;
	AUMIDIBase& operator=(const AUMIDIBase&) = delete;
	AUMIDIBase& operator=(AUMIDIBase&&) = delete;

	virtual OSStatus MIDIEvent(
		UInt32 inStatus, UInt32 inData1, UInt32 inData2, UInt32 inOffsetSampleFrame)
	{
		const UInt32 strippedStatus = inStatus & 0xf0U; // NOLINT
		const UInt32 channel = inStatus & 0x0fU;        // NOLINT

		return HandleMIDIEvent(strippedStatus, channel, inData1, inData2, inOffsetSampleFrame);
	}

#if AUSDK_MIDI2_AVAILABLE
	virtual OSStatus MIDIEventList(
		UInt32 /*inOffsetSampleFrame*/, const MIDIEventList* /*eventList*/)
	{
		return kAudio_UnimplementedError;
	}
#endif

	virtual OSStatus SysEx(const UInt8* inData, UInt32 inLength);

	virtual OSStatus DelegateGetPropertyInfo(AudioUnitPropertyID inID, AudioUnitScope inScope,
		AudioUnitElement inElement, UInt32& outDataSize, bool& outWritable);
	virtual OSStatus DelegateGetProperty(AudioUnitPropertyID inID, AudioUnitScope inScope,
		AudioUnitElement inElement, void* outData);
	virtual OSStatus DelegateSetProperty(AudioUnitPropertyID inID, AudioUnitScope inScope,
		AudioUnitElement inElement, const void* inData, UInt32 inDataSize);

protected:
	// MIDI dispatch
	virtual OSStatus HandleMIDIEvent(
		UInt8 inStatus, UInt8 inChannel, UInt8 inData1, UInt8 inData2, UInt32 inStartFrame);
	virtual OSStatus HandleNonNoteEvent(
		UInt8 status, UInt8 channel, UInt8 data1, UInt8 data2, UInt32 inStartFrame);

	// Old name
	AUSDK_DEPRECATED("HandleMIDIEvent")
	OSStatus HandleMidiEvent(
		UInt8 inStatus, UInt8 inChannel, UInt8 inData1, UInt8 inData2, UInt32 inStartFrame)
	{
		return HandleMIDIEvent(inStatus, inChannel, inData1, inData2, inStartFrame);
	}

#if AUSDK_HAVE_XML_NAMES
	virtual OSStatus GetXMLNames(CFURLRef* /*outNameDocument*/)
	{
		return kAudioUnitErr_InvalidProperty;
	} // if not overridden, it's unsupported
#endif

	// channel messages
	virtual OSStatus HandleNoteOn(
		UInt8 /*inChannel*/, UInt8 /*inNoteNumber*/, UInt8 /*inVelocity*/, UInt32 /*inStartFrame*/)
	{
		return noErr;
	}
	virtual OSStatus HandleNoteOff(
		UInt8 /*inChannel*/, UInt8 /*inNoteNumber*/, UInt8 /*inVelocity*/, UInt32 /*inStartFrame*/)
	{
		return noErr;
	}
	virtual OSStatus HandleControlChange(
		UInt8 /*inChannel*/, UInt8 /*inController*/, UInt8 /*inValue*/, UInt32 /*inStartFrame*/)
	{
		return noErr;
	}
	virtual OSStatus HandlePitchWheel(
		UInt8 /*inChannel*/, UInt8 /*inPitch1*/, UInt8 /*inPitch2*/, UInt32 /*inStartFrame*/)
	{
		return noErr;
	}
	virtual OSStatus HandleChannelPressure(
		UInt8 /*inChannel*/, UInt8 /*inValue*/, UInt32 /*inStartFrame*/)
	{
		return noErr;
	}
	virtual OSStatus HandleProgramChange(UInt8 /*inChannel*/, UInt8 /*inValue*/) { return noErr; }
	virtual OSStatus HandlePolyPressure(
		UInt8 /*inChannel*/, UInt8 /*inKey*/, UInt8 /*inValue*/, UInt32 /*inStartFrame*/)
	{
		return noErr;
	}
	virtual OSStatus HandleResetAllControllers(UInt8 /*inChannel*/) { return noErr; }
	virtual OSStatus HandleAllNotesOff(UInt8 /*inChannel*/) { return noErr; }
	virtual OSStatus HandleAllSoundOff(UInt8 /*inChannel*/) { return noErr; }

	// System messages
	virtual OSStatus HandleSysEx(const UInt8* /*inData*/, UInt32 /*inLength*/) { return noErr; }

#if AUSDK_HAVE_MIDI_MAPPING
	void SetMIDIMapper(const std::shared_ptr<AUMIDIMapper>& mapper) { mMIDIMapper = mapper; }
#endif

private:
	AUBase& mAUBaseInstance;
#if AUSDK_HAVE_MIDI_MAPPING
	std::shared_ptr<AUMIDIMapper> mMIDIMapper;
#endif
};

} // namespace ausdk

#endif // AudioUnitSDK_AUMIDIBase_h

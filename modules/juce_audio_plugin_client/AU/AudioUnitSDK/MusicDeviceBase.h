/*!
	@file		AudioUnitSDK/MusicDeviceBase.h
	@copyright	© 2000-2021 Apple Inc. All rights reserved.
*/
#ifndef AudioUnitSDK_MusicDeviceBase_h
#define AudioUnitSDK_MusicDeviceBase_h

#include <AudioUnitSDK/AUMIDIBase.h>

namespace ausdk {

// ________________________________________________________________________
//	MusicDeviceBase
//

/*!
	@class	MusicDeviceBase
	@brief	Deriving from AUBase and AUMIDIBase, an abstract base class for Music Device
			subclasses.
*/
class MusicDeviceBase : public AUBase, public AUMIDIBase {
public:
	MusicDeviceBase(AudioComponentInstance inInstance, UInt32 numInputs, UInt32 numOutputs,
		UInt32 numGroups = 0);

	OSStatus MIDIEvent(
		UInt32 inStatus, UInt32 inData1, UInt32 inData2, UInt32 inOffsetSampleFrame) override
	{
		return AUMIDIBase::MIDIEvent(inStatus, inData1, inData2, inOffsetSampleFrame);
	}
	OSStatus SysEx(const UInt8* inData, UInt32 inLength) override
	{
		return AUMIDIBase::SysEx(inData, inLength);
	}

#if AUSDK_MIDI2_AVAILABLE
	OSStatus MIDIEventList(
		UInt32 inOffsetSampleFrame, const struct MIDIEventList* eventList) override
	{
		return AUMIDIBase::MIDIEventList(inOffsetSampleFrame, eventList);
	}
#endif

	OSStatus GetPropertyInfo(AudioUnitPropertyID inID, AudioUnitScope inScope,
		AudioUnitElement inElement, UInt32& outDataSize, bool& outWritable) override;
	OSStatus GetProperty(AudioUnitPropertyID inID, AudioUnitScope inScope,
		AudioUnitElement inElement, void* outData) override;
	OSStatus SetProperty(AudioUnitPropertyID inID, AudioUnitScope inScope,
		AudioUnitElement inElement, const void* inData, UInt32 inDataSize) override;
	OSStatus HandleNoteOn(
		UInt8 inChannel, UInt8 inNoteNumber, UInt8 inVelocity, UInt32 inStartFrame) override;
	OSStatus HandleNoteOff(
		UInt8 inChannel, UInt8 inNoteNumber, UInt8 inVelocity, UInt32 inStartFrame) override;
	virtual OSStatus GetInstrumentCount(UInt32& outInstCount) const;

private:
	OSStatus HandleStartNoteMessage(MusicDeviceInstrumentID inInstrument,
		MusicDeviceGroupID inGroupID, NoteInstanceID* outNoteInstanceID, UInt32 inOffsetSampleFrame,
		const MusicDeviceNoteParams* inParams);
};

} // namespace ausdk

#endif // AudioUnitSDK_MusicDeviceBase_h

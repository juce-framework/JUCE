/*!
	@file		AudioUnitSDK/AUBase.h
	@copyright	© 2000-2021 Apple Inc. All rights reserved.
*/

#ifndef AudioUnitSDK_AUBase_h
#define AudioUnitSDK_AUBase_h

// module
#include <AudioUnitSDK/AUBuffer.h>
#include <AudioUnitSDK/AUInputElement.h>
#include <AudioUnitSDK/AUMIDIUtility.h>
#include <AudioUnitSDK/AUOutputElement.h>
#include <AudioUnitSDK/AUPlugInDispatch.h>
#include <AudioUnitSDK/AUScopeElement.h>
#include <AudioUnitSDK/AUUtility.h>

// OS
#include <TargetConditionals.h>

// std
#include <algorithm>
#include <array>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

// ________________________________________________________________________

namespace ausdk {

/*!
	@class	AUBase
	@brief	Abstract base class for an Audio Unit implementation.
*/
class AUBase : public ComponentBase {
public:
	constexpr static double kAUDefaultSampleRate = 44100.0;
#if !TARGET_OS_WIN32
	constexpr static UInt32 kAUDefaultMaxFramesPerSlice = 1156;
	// this allows enough default frames for a 512 dest 44K and SRC from 96K
	// add a padding of 4 frames for any vector rounding
#else
	constexpr static UInt32 kAUDefaultMaxFramesPerSlice = 2048;
#endif

	AUBase(AudioComponentInstance inInstance, UInt32 numInputElements, UInt32 numOutputElements,
		UInt32 numGroupElements = 0);
	~AUBase() override;

	AUBase(const AUBase&) = delete;
	AUBase(AUBase&&) = delete;
	AUBase& operator=(const AUBase&) = delete;
	AUBase& operator=(AUBase&&) = delete;

	/// Called immediately after construction, when virtual methods work. Or, a subclass may call
	/// this in order to have access to elements in its constructor.
	void CreateElements();

	virtual void CreateExtendedElements() {}

#pragma mark -
#pragma mark AU dispatch
	// ________________________________________________________________________
	// Virtual methods (mostly) directly corresponding to the entry points.  Many of these
	// have useful implementations here and will not need overriding.

	/// Implements the entry point and ensures that Initialize is called exactly once from an
	/// uninitialized state.
	OSStatus DoInitialize();

	// Overrides to this method can assume that they will only be called exactly once
	// when transitioning from an uninitialized state.
	virtual OSStatus Initialize();

	[[nodiscard]] bool IsInitialized() const noexcept { return mInitialized; }
	[[nodiscard]] bool HasBegunInitializing() const noexcept { return mHasBegunInitializing; }

	/// Implements the entry point and ensures that Cleanup is called exactly once from an
	/// initialized state.
	void DoCleanup();

	// Overrides to this method can assume that they will only be called exactly once
	// when transitioning from an initialized state to an uninitialized state.
	virtual void Cleanup();

	virtual OSStatus Reset(AudioUnitScope inScope, AudioUnitElement inElement);

	// Note about GetPropertyInfo, GetProperty, SetProperty:
	// Certain properties are trapped out in these dispatch functions and handled with different
	// virtual methods.  (To discourage hacks and keep vtable size down, these are non-virtual)

	OSStatus DispatchGetPropertyInfo(AudioUnitPropertyID inID, AudioUnitScope inScope,
		AudioUnitElement inElement, UInt32& outDataSize, bool& outWritable);
	OSStatus DispatchGetProperty(AudioUnitPropertyID inID, AudioUnitScope inScope,
		AudioUnitElement inElement, void* outData);
	OSStatus DispatchSetProperty(AudioUnitPropertyID inID, AudioUnitScope inScope,
		AudioUnitElement inElement, const void* inData, UInt32 inDataSize);
	OSStatus DispatchRemovePropertyValue(
		AudioUnitPropertyID inID, AudioUnitScope inScope, AudioUnitElement inElement);

	virtual OSStatus GetPropertyInfo(AudioUnitPropertyID inID, AudioUnitScope inScope,
		AudioUnitElement inElement, UInt32& outDataSize, bool& outWritable);
	virtual OSStatus GetProperty(AudioUnitPropertyID inID, AudioUnitScope inScope,
		AudioUnitElement inElement, void* outData);
	virtual OSStatus SetProperty(AudioUnitPropertyID inID, AudioUnitScope inScope,
		AudioUnitElement inElement, const void* inData, UInt32 inDataSize);
	virtual OSStatus RemovePropertyValue(
		AudioUnitPropertyID inID, AudioUnitScope inScope, AudioUnitElement inElement);

	virtual OSStatus AddPropertyListener(
		AudioUnitPropertyID inID, AudioUnitPropertyListenerProc inProc, void* inProcRefCon);
	virtual OSStatus RemovePropertyListener(AudioUnitPropertyID inID,
		AudioUnitPropertyListenerProc inProc, void* inProcRefCon, bool refConSpecified);

	virtual OSStatus SetRenderNotification(AURenderCallback inProc, void* inRefCon);
	virtual OSStatus RemoveRenderNotification(AURenderCallback inProc, void* inRefCon);

	virtual OSStatus GetParameter(AudioUnitParameterID inID, AudioUnitScope inScope,
		AudioUnitElement inElement, AudioUnitParameterValue& outValue);
	virtual OSStatus SetParameter(AudioUnitParameterID inID, AudioUnitScope inScope,
		AudioUnitElement inElement, AudioUnitParameterValue inValue, UInt32 inBufferOffsetInFrames);

	[[nodiscard]] virtual bool CanScheduleParameters() const = 0;
	virtual OSStatus ScheduleParameter(
		const AudioUnitParameterEvent* inParameterEvent, UInt32 inNumEvents);

	OSStatus DoRender(AudioUnitRenderActionFlags& ioActionFlags, const AudioTimeStamp& inTimeStamp,
		UInt32 inBusNumber, UInt32 inFramesToProcess, AudioBufferList& ioData);
	OSStatus DoProcess(AudioUnitRenderActionFlags& ioActionFlags, const AudioTimeStamp& inTimeStamp,
		UInt32 inFramesToProcess, AudioBufferList& ioData);
	OSStatus DoProcessMultiple(AudioUnitRenderActionFlags& ioActionFlags,
		const AudioTimeStamp& inTimeStamp, UInt32 inFramesToProcess,
		UInt32 inNumberInputBufferLists, const AudioBufferList** inInputBufferLists,
		UInt32 inNumberOutputBufferLists, AudioBufferList** ioOutputBufferLists);

	virtual OSStatus ProcessBufferLists(AudioUnitRenderActionFlags& /*ioActionFlags*/,
		const AudioBufferList& /*inBuffer*/, AudioBufferList& /*outBuffer*/,
		UInt32 /*inFramesToProcess*/)
	{
		return kAudio_UnimplementedError;
	}

	virtual OSStatus ProcessMultipleBufferLists(AudioUnitRenderActionFlags& /*ioActionFlags*/,
		UInt32 /*inFramesToProcess*/, UInt32 /*inNumberInputBufferLists*/,
		const AudioBufferList** /*inInputBufferLists*/, UInt32 /*inNumberOutputBufferLists*/,
		AudioBufferList** /*ioOutputBufferLists*/)
	{
		return kAudio_UnimplementedError;
	}

	virtual OSStatus ComplexRender(AudioUnitRenderActionFlags& /*ioActionFlags*/,
		const AudioTimeStamp& /*inTimeStamp*/, UInt32 /*inOutputBusNumber*/,
		UInt32 /*inNumberOfPackets*/, UInt32* /*outNumberOfPackets*/,
		AudioStreamPacketDescription* /*outPacketDescriptions*/, AudioBufferList& /*ioData*/,
		void* /*outMetadata*/, UInt32* /*outMetadataByteSize*/)
	{
		return kAudio_UnimplementedError;
	}

	// Override this method if your AU processes multiple output busses completely independently --
	// you'll want to just call Render without the NeedsToRender check.
	// Otherwise, override Render().
	//
	// N.B. Implementations of this method can assume that the output's buffer list has already been
	// prepared and access it with GetOutput(inBusNumber)->GetBufferList() instead of
	// GetOutput(inBusNumber)->PrepareBuffer(nFrames) -- if PrepareBuffer is called, a
	// copy may occur after rendering.
	virtual OSStatus RenderBus(AudioUnitRenderActionFlags& ioActionFlags,
		const AudioTimeStamp& inTimeStamp, UInt32 /*inBusNumber*/, UInt32 inNumberFrames)
	{
		if (NeedsToRender(inTimeStamp)) {
			return Render(ioActionFlags, inTimeStamp, inNumberFrames);
		}
		return noErr; // was presumably already rendered via another bus
	}

	// N.B. For a unit with only one output bus, it can assume in its implementation of this
	// method that the output's buffer list has already been prepared and access it with
	// GetOutput(0)->GetBufferList() instead of GetOutput(0)->PrepareBuffer(nFrames)
	//  -- if PrepareBuffer is called, a copy may occur after rendering.
	virtual OSStatus Render(AudioUnitRenderActionFlags& /*ioActionFlags*/,
		const AudioTimeStamp& /*inTimeStamp*/, UInt32 /*inNumberFrames*/)
	{
		return noErr;
	}


#pragma mark -
#pragma mark Property Dispatch

	// ________________________________________________________________________
	// These are called from DispatchGetProperty/DispatchGetPropertyInfo/DispatchSetProperty

	virtual bool BusCountWritable(AudioUnitScope /*inScope*/) { return false; }
	virtual OSStatus SetBusCount(AudioUnitScope inScope, UInt32 inCount);
	virtual OSStatus SetConnection(const AudioUnitConnection& inConnection);
	virtual OSStatus SetInputCallback(
		UInt32 inPropertyID, AudioUnitElement inElement, AURenderCallback inProc, void* inRefCon);

	virtual OSStatus GetParameterList(
		AudioUnitScope inScope, AudioUnitParameterID* outParameterList, UInt32& outNumParameters);
	// outParameterList may be a null pointer
	virtual OSStatus GetParameterInfo(AudioUnitScope inScope, AudioUnitParameterID inParameterID,
		AudioUnitParameterInfo& outParameterInfo);

	virtual OSStatus GetParameterHistoryInfo(AudioUnitScope inScope,
		AudioUnitParameterID inParameterID, Float32& outUpdatesPerSecond,
		Float32& outHistoryDurationInSeconds);
	virtual OSStatus SaveState(CFPropertyListRef* outData);
	virtual void SaveExtendedScopes(CFMutableDataRef /*outData*/) {}
	virtual OSStatus RestoreState(CFPropertyListRef plist);
	virtual OSStatus GetParameterValueStrings(
		AudioUnitScope inScope, AudioUnitParameterID inParameterID, CFArrayRef* outStrings);
	virtual OSStatus CopyClumpName(AudioUnitScope inScope, UInt32 inClumpID,
		UInt32 inDesiredNameLength, CFStringRef* outClumpName);
	virtual OSStatus GetPresets(CFArrayRef* outData) const;

	/// Set the default preset for the unit. The number of the preset must be >= 0 and the name
	/// should be valid, or the preset will be rejected.
	bool SetAFactoryPresetAsCurrent(const AUPreset& inPreset);

	// Called when the host sets a new, valid preset.
	// If this is a valid preset, then the subclass sets its state to that preset
	// and returns noErr.
	// If not a valid preset, return an error, and the pre-existing preset is restored.
	virtual OSStatus NewFactoryPresetSet(const AUPreset& inNewFactoryPreset);
	virtual OSStatus NewCustomPresetSet(const AUPreset& inNewCustomPreset);
	virtual CFURLRef CopyIconLocation();

	// default is no latency, and unimplemented tail time
	virtual Float64 GetLatency() { return 0.0; }
	virtual Float64 GetTailTime() { return 0.0; }
	virtual bool SupportsTail() { return false; }

	// Stream formats: scope will always be input or output
	bool IsStreamFormatWritable(AudioUnitScope scope, AudioUnitElement element);

	virtual bool StreamFormatWritable(AudioUnitScope scope, AudioUnitElement element) = 0;

	// pass in a pointer to get the struct, and num channel infos
	// you can pass in NULL to just get the number
	// a return value of 0 (the default in AUBase) means the property is not supported...
	virtual UInt32 SupportedNumChannels(const AUChannelInfo** outInfo);

	/// Will only be called after StreamFormatWritable has succeeded. Default implementation
	/// requires non-interleaved native-endian 32-bit float, any sample rate, any number of
	/// channels; override when other formats are supported.  A subclass's override can choose to
	/// always return true and trap invalid formats in ChangeStreamFormat.
	virtual bool ValidFormat(AudioUnitScope inScope, AudioUnitElement inElement,
		const AudioStreamBasicDescription& inNewFormat);

	virtual AudioStreamBasicDescription GetStreamFormat(
		AudioUnitScope inScope, AudioUnitElement inElement);

	// Will only be called after StreamFormatWritable
	// and ValidFormat have succeeded.
	virtual OSStatus ChangeStreamFormat(AudioUnitScope inScope, AudioUnitElement inElement,
		const AudioStreamBasicDescription& inPrevFormat,
		const AudioStreamBasicDescription& inNewFormat);

	// ________________________________________________________________________
	// Methods useful for subclasses
	AUScope& GetScope(AudioUnitScope inScope)
	{
		if (inScope >= kNumScopes) {
			AUScope* const scope = GetScopeExtended(inScope);

			ThrowQuietIf(scope == nullptr, kAudioUnitErr_InvalidScope);
			return *scope;
		}
		return mScopes[inScope]; // NOLINT
	}

	virtual AUScope* GetScopeExtended(AudioUnitScope /*inScope*/) { return nullptr; }

	AUScope& GlobalScope() { return mScopes[kAudioUnitScope_Global]; }
	AUScope& Inputs() { return mScopes[kAudioUnitScope_Input]; }
	AUScope& Outputs() { return mScopes[kAudioUnitScope_Output]; }
	AUScope& Groups() { return mScopes[kAudioUnitScope_Group]; }
	AUElement* Globals() { return mScopes[kAudioUnitScope_Global].GetElement(0); }

	void SetNumberOfElements(AudioUnitScope inScope, UInt32 numElements);
	virtual std::unique_ptr<AUElement> CreateElement(
		AudioUnitScope scope, AudioUnitElement element);

	AUElement* GetElement(AudioUnitScope inScope, AudioUnitElement inElement)
	{
		return GetScope(inScope).GetElement(inElement);
	}

	AUSDK_DEPRECATED("Use IOElement()")
	AUIOElement* GetIOElement(AudioUnitScope inScope, AudioUnitElement inElement)
	{
		return &IOElement(inScope, inElement);
	}

	AUIOElement& IOElement(AudioUnitScope inScope, AudioUnitElement inElement)
	{
		return *GetScope(inScope).GetIOElement(inElement);
	}

	AUSDK_DEPRECATED("Use Element()")
	AUElement* SafeGetElement(AudioUnitScope inScope, AudioUnitElement inElement)
	{
		return &Element(inScope, inElement);
	}

	AUElement& Element(AudioUnitScope inScope, AudioUnitElement inElement)
	{
		return *GetScope(inScope).SafeGetElement(inElement);
	}

	AUSDK_DEPRECATED("Use Input()")
	AUInputElement* GetInput(AudioUnitElement inElement) { return &Input(inElement); }
	AUInputElement& Input(AudioUnitElement inElement)
	{
		return static_cast<AUInputElement&>(*Inputs().SafeGetElement(inElement)); // NOLINT downcast
	}

	AUSDK_DEPRECATED("Use Output()")
	AUOutputElement* GetOutput(AudioUnitElement inElement) { return &Output(inElement); }
	AUOutputElement& Output(AudioUnitElement inElement)
	{
		return static_cast<AUOutputElement&>( // NOLINT downcast
			*Outputs().SafeGetElement(inElement));
	}

	AUSDK_DEPRECATED("Use Group()")
	AUElement* GetGroup(AudioUnitElement inElement) { return &Group(inElement); }
	AUElement& Group(AudioUnitElement inElement) { return *Groups().SafeGetElement(inElement); }

	OSStatus PullInput(UInt32 inBusNumber, AudioUnitRenderActionFlags& ioActionFlags,
		const AudioTimeStamp& inTimeStamp, UInt32 inNumberFrames)
	{
		AUInputElement& input = Input(inBusNumber); // throws if error
		return input.PullInput(ioActionFlags, inTimeStamp, inBusNumber, inNumberFrames);
	}

	[[nodiscard]] UInt32 GetMaxFramesPerSlice() const noexcept { return mMaxFramesPerSlice; }

	[[nodiscard]] bool UsesFixedBlockSize() const noexcept { return mUsesFixedBlockSize; }

	void SetUsesFixedBlockSize(bool inUsesFixedBlockSize) noexcept
	{
		mUsesFixedBlockSize = inUsesFixedBlockSize;
	}

	[[nodiscard]] virtual bool InRenderThread() const
	{
		return std::this_thread::get_id() == mRenderThreadID;
	}

	/// Says whether an input is connected or has a callback.
	bool HasInput(AudioUnitElement inElement)
	{
		auto* const in =
			static_cast<AUInputElement*>(Inputs().GetElement(inElement)); // NOLINT downcast
		return in != nullptr && in->IsActive();
	}

	virtual void PropertyChanged(
		AudioUnitPropertyID inID, AudioUnitScope inScope, AudioUnitElement inElement);

	// These calls can be used to call a Host's Callbacks. The method returns -1 if the host
	// hasn't supplied the callback. Any other result is returned by the host.
	// As in the API contract, for a parameter's value, you specify a pointer
	// to that data type. Specify NULL for a parameter that you are not interested
	// as this can save work in the host.
	OSStatus CallHostBeatAndTempo(Float64* outCurrentBeat, Float64* outCurrentTempo) const
	{
		return (mHostCallbackInfo.beatAndTempoProc != nullptr
					? (*mHostCallbackInfo.beatAndTempoProc)(
						  mHostCallbackInfo.hostUserData, outCurrentBeat, outCurrentTempo)
					: -1);
	}

	OSStatus CallHostMusicalTimeLocation(UInt32* outDeltaSampleOffsetToNextBeat,
		Float32* outTimeSig_Numerator, UInt32* outTimeSig_Denominator,
		Float64* outCurrentMeasureDownBeat) const
	{
		return (mHostCallbackInfo.musicalTimeLocationProc != nullptr
					? (*mHostCallbackInfo.musicalTimeLocationProc)(mHostCallbackInfo.hostUserData,
						  outDeltaSampleOffsetToNextBeat, outTimeSig_Numerator,
						  outTimeSig_Denominator, outCurrentMeasureDownBeat)
					: -1);
	}

	OSStatus CallHostTransportState(Boolean* outIsPlaying, Boolean* outTransportStateChanged,
		Float64* outCurrentSampleInTimeLine, Boolean* outIsCycling, Float64* outCycleStartBeat,
		Float64* outCycleEndBeat) const
	{
		return (mHostCallbackInfo.transportStateProc != nullptr
					? (*mHostCallbackInfo.transportStateProc)(mHostCallbackInfo.hostUserData,
						  outIsPlaying, outTransportStateChanged, outCurrentSampleInTimeLine,
						  outIsCycling, outCycleStartBeat, outCycleEndBeat)
					: -1);
	}

	[[nodiscard]] const char* GetLoggingString() const noexcept;

	AUMutex* GetMutex() noexcept { return mAUMutex; }
	// The caller of SetMutex is responsible for the managing the lifetime of the
	// mutex object and, if deleted before the AUBase instance, is responsible
	// for calling SetMutex(nullptr)
	void SetMutex(AUMutex* mutex) noexcept { mAUMutex = mutex; }

#pragma mark -
#pragma mark AU Output Base Dispatch
	// ________________________________________________________________________
	// ________________________________________________________________________
	// ________________________________________________________________________
	// output unit methods
	virtual OSStatus Start() { return kAudio_UnimplementedError; }

	virtual OSStatus Stop() { return kAudio_UnimplementedError; }

#pragma mark -
#pragma mark AU Music Base Dispatch
	// ________________________________________________________________________
	// ________________________________________________________________________
	// ________________________________________________________________________
	// music device/music effect methods

	virtual OSStatus MIDIEvent(
		UInt32 /*inStatus*/, UInt32 /*inData1*/, UInt32 /*inData2*/, UInt32 /*inOffsetSampleFrame*/)
	{
		return kAudio_UnimplementedError;
	}

	virtual OSStatus SysEx(const UInt8* /*inData*/, UInt32 /*inLength*/)
	{
		return kAudio_UnimplementedError;
	}

#if AUSDK_MIDI2_AVAILABLE
	virtual OSStatus MIDIEventList(
		UInt32 /*inOffsetSampleFrame*/, const MIDIEventList* /*eventList*/)
	{
		return kAudio_UnimplementedError;
	}
#endif

	virtual OSStatus StartNote(MusicDeviceInstrumentID /*inInstrument*/,
		MusicDeviceGroupID /*inGroupID*/, NoteInstanceID* /*outNoteInstanceID*/,
		UInt32 /*inOffsetSampleFrame*/, const MusicDeviceNoteParams& /*inParams*/)
	{
		return kAudio_UnimplementedError;
	}

	virtual OSStatus StopNote(MusicDeviceGroupID /*inGroupID*/, NoteInstanceID /*inNoteInstanceID*/,
		UInt32 /*inOffsetSampleFrame*/)
	{
		return kAudio_UnimplementedError;
	}

	/// Obsolete
	static OSStatus PrepareInstrument(MusicDeviceInstrumentID /*inInstrument*/)
	{
		return kAudio_UnimplementedError;
	}

	/// Obsolete
	static OSStatus ReleaseInstrument(MusicDeviceInstrumentID /*inInstrument*/)
	{
		return kAudio_UnimplementedError;
	}

	// ________________________________________________________________________
	// ________________________________________________________________________
	// ________________________________________________________________________

protected:
#pragma mark -
#pragma mark Implementation methods
	void PostConstructorInternal() final;
	void PreDestructorInternal() final;

	/// needs to be called when mMaxFramesPerSlice changes
	virtual void ReallocateBuffers();

	virtual void DeallocateIOBuffers();

	static void FillInParameterName(
		AudioUnitParameterInfo& ioInfo, CFStringRef inName, bool inShouldRelease)
	{
		ioInfo.cfNameString = inName;
		ioInfo.flags |= kAudioUnitParameterFlag_HasCFNameString;
		if (inShouldRelease) {
			ioInfo.flags |= kAudioUnitParameterFlag_CFNameRelease;
		}
		CFStringGetCString(inName, std::data(ioInfo.name), std::size(ioInfo.name),
			kCFStringEncodingUTF8);
	}

	static void HasClump(AudioUnitParameterInfo& ioInfo, UInt32 inClumpID) noexcept
	{
		ioInfo.clumpID = inClumpID;
		ioInfo.flags |= kAudioUnitParameterFlag_HasClump;
	}

	virtual void SetMaxFramesPerSlice(UInt32 nFrames);

	[[nodiscard]] virtual OSStatus CanSetMaxFrames() const;

	[[nodiscard]] bool WantsRenderThreadID() const noexcept { return mWantsRenderThreadID; }

	void SetWantsRenderThreadID(bool inFlag);

	OSStatus SetRenderError(OSStatus inErr)
	{
		if (inErr != noErr && mLastRenderError == 0) {
			mLastRenderError = inErr;
			PropertyChanged(kAudioUnitProperty_LastRenderError, kAudioUnitScope_Global, 0);
		}
		return inErr;
	}

	struct PropertyListener {
		AudioUnitPropertyID propertyID{ 0 };
		AudioUnitPropertyListenerProc listenerProc{ nullptr };
		void* listenerRefCon{ nullptr };
	};
	using PropertyListeners = std::vector<PropertyListener>;

	[[nodiscard]] const PropertyListeners& GetPropertyListeners() const noexcept
	{
		return mPropertyListeners;
	}

	HostCallbackInfo& GetHostCallbackInfo() noexcept { return mHostCallbackInfo; }

private:
	// shared between Render and RenderSlice, inlined to minimize function call overhead
	OSStatus DoRenderBus(AudioUnitRenderActionFlags& ioActionFlags,
		const AudioTimeStamp& inTimeStamp, UInt32 inBusNumber, AUOutputElement& theOutput,
		UInt32 inNumberFrames, AudioBufferList& ioData)
	{
		if (ioData.mBuffers[0].mData == nullptr ||
			(theOutput.WillAllocateBuffer() && Outputs().GetNumberOfElements() > 1)) {
			// will render into cache buffer
			theOutput.PrepareBuffer(inNumberFrames);
		} else {
			// will render into caller's buffer
			theOutput.SetBufferList(ioData);
		}
		const OSStatus result = RenderBus(ioActionFlags, inTimeStamp, inBusNumber, inNumberFrames);
		if (result == noErr) {
			if (ioData.mBuffers[0].mData == nullptr) {
				theOutput.CopyBufferListTo(ioData);
			} else {
				theOutput.CopyBufferContentsTo(ioData);
				theOutput.InvalidateBufferList();
			}
		}
		return result;
	}

	bool HasIcon();

	[[nodiscard]] std::string CreateLoggingString() const;

protected:
	//. Returns size. outLayoutPtr may be null if querying only for size.
	virtual UInt32 GetAudioChannelLayout(AudioUnitScope scope, AudioUnitElement element,
		AudioChannelLayout* outLayoutPtr, bool& outWritable);

	/// Layout is non-null.
	virtual OSStatus SetAudioChannelLayout(
		AudioUnitScope scope, AudioUnitElement element, const AudioChannelLayout* inLayout);

	virtual OSStatus RemoveAudioChannelLayout(AudioUnitScope scope, AudioUnitElement element);

	virtual std::vector<AudioChannelLayoutTag> GetChannelLayoutTags(
		AudioUnitScope scope, AudioUnitElement element);

	bool NeedsToRender(const AudioTimeStamp& inTimeStamp)
	{
		const bool needsToRender = (inTimeStamp.mSampleTime != mCurrentRenderTime.mSampleTime);
		if (needsToRender) { // only copy this if we need to render
			mCurrentRenderTime = inTimeStamp;
		}
		return needsToRender;
	}

	// Scheduled parameter implementation:

	using ParameterEventList = std::vector<AudioUnitParameterEvent>;

	// Usually, you won't override this method.  You only need to call this if your DSP code
	// is prepared to handle scheduled immediate and ramped parameter changes.
	// Before calling this method, it is assumed you have already called PullInput() on the input
	// busses for which the DSP code depends.  ProcessForScheduledParams() will call (potentially
	// repeatedly) virtual method ProcessScheduledSlice() to perform the actual DSP for a given
	// sub-division of the buffer.  The job of ProcessForScheduledParams() is to sub-divide the
	// buffer into smaller pieces according to the scheduled times found in the ParameterEventList
	// (usually coming directly from a previous call to ScheduleParameter() ), setting the
	// appropriate immediate or ramped parameter values for the corresponding scopes and elements,
	// then calling ProcessScheduledSlice() to do the actual DSP for each of these divisions.
	virtual OSStatus ProcessForScheduledParams(
		ParameterEventList& inParamList, UInt32 inFramesToProcess, void* inUserData);

	//	This method is called (potentially repeatedly) by ProcessForScheduledParams()
	//	in order to perform the actual DSP required for this portion of the entire buffer
	//	being processed.  The entire buffer can be divided up into smaller "slices"
	//	according to the timestamps on the scheduled parameters...
	//
	//	sub-classes wishing to handle scheduled parameter changes should override this method
	//  in order to do the appropriate DSP.  AUEffectBase already overrides this for standard
	//	effect AudioUnits.
	virtual OSStatus ProcessScheduledSlice(void* /*inUserData*/, UInt32 /*inStartFrameInBuffer*/,
		UInt32 /*inSliceFramesToProcess*/, UInt32 /*inTotalBufferFrames*/)
	{
		// default implementation does nothing.
		return noErr;
	}

	[[nodiscard]] const AudioTimeStamp& CurrentRenderTime() const noexcept
	{
		return mCurrentRenderTime;
	}
	void ResetRenderTime();

	// ________________________________________________________________________
	//	Private data members to discourage hacking in subclasses
private:
	struct RenderCallback {
		RenderCallback() = default;

		RenderCallback(AURenderCallback proc, void* ref)
			: mRenderNotify(proc), mRenderNotifyRefCon(ref)
		{
		}

		AURenderCallback mRenderNotify = nullptr;
		void* mRenderNotifyRefCon = nullptr;

		bool operator==(const RenderCallback& other) const
		{
			return this->mRenderNotify == other.mRenderNotify &&
				   this->mRenderNotifyRefCon == other.mRenderNotifyRefCon;
		}
	};

	class RenderCallbackList {
	public:
		void add(const RenderCallback& rc)
		{
			const std::lock_guard guard{ mLock };
			const auto iter = std::find(mImpl.begin(), mImpl.end(), rc);
			if (iter != mImpl.end()) {
				return;
			}
			mImpl.emplace_back(rc);
		}

		void remove(const RenderCallback& rc)
		{
			const std::lock_guard guard{ mLock };
			const auto iter = std::find(mImpl.begin(), mImpl.end(), rc);
			if (iter != mImpl.end()) {
				mImpl.erase(iter);
			}
		}

		template <typename F>
		void foreach (F&& func)
		{
			const std::lock_guard guard{ mLock };
			for (const auto& cb : mImpl) {
				func(cb);
			}
		}

	private:
		AUMutex mLock;
		std::vector<RenderCallback> mImpl;
	};

protected:
	static constexpr AudioUnitScope kNumScopes = 4;

	ParameterEventList& GetParamEventList() noexcept { return mParamEventList; }
	void SetBuffersAllocated(bool b) noexcept { mBuffersAllocated = b; }

	[[nodiscard]] CFStringRef GetContextName() const { return *mContextName; }
	void SetContextName(CFStringRef str) { mContextName = str; }

	[[nodiscard]] CFStringRef GetNickName() const { return *mNickName; }

private:
	bool mElementsCreated{ false };
	bool mInitialized{ false };
	bool mHasBegunInitializing{ false };
	const UInt32 mInitNumInputEls;
	const UInt32 mInitNumOutputEls;
	const UInt32 mInitNumGroupEls;
	std::array<AUScope, kNumScopes> mScopes;
	RenderCallbackList mRenderCallbacks;
	bool mRenderCallbacksTouched{ false };
	std::thread::id mRenderThreadID{};
	bool mWantsRenderThreadID{ false };
	AudioTimeStamp mCurrentRenderTime{};
	UInt32 mMaxFramesPerSlice{ 0 };
	OSStatus mLastRenderError{ noErr };
#ifndef AUSDK_NO_LOGGING
	const double mHostTimeFrequency{
		HostTime::Frequency()
	}; // cache because there is calculation cost
#endif
	AUPreset mCurrentPreset{ -1, nullptr };
	bool mUsesFixedBlockSize{ false };

	ParameterEventList mParamEventList;
	PropertyListeners mPropertyListeners;
	bool mBuffersAllocated{ false };
	const std::string mLogString;
	Owned<CFStringRef> mNickName;

	/*! @var mAUMutex
		If non-null, guards all non-realtime entry points into the AudioUnit. Most AudioUnits
		do not need to use this. It's useful for the case of an AU which must synchronize
		an external source of callbacks against entry from the host.
	*/
	AUMutex* mAUMutex{ nullptr };
	HostCallbackInfo mHostCallbackInfo{};
	Owned<CFStringRef> mContextName;
};

} // namespace ausdk

#endif // AudioUnitSDK_AUBase_h

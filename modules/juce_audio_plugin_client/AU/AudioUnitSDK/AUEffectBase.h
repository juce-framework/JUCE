/*!
	@file		AudioUnitSDK/AUEffectBase.h
	@copyright	© 2000-2021 Apple Inc. All rights reserved.
*/
#ifndef AudioUnitSDK_AUEffectBase_h
#define AudioUnitSDK_AUEffectBase_h

#include <AudioUnitSDK/AUBase.h>
#include <AudioUnitSDK/AUSilentTimeout.h>

#include <memory>

namespace ausdk {

class AUKernelBase;

/*!
	@class	AUEffectBase
	@brief	Base class for an effect with one input stream, one output stream, and any number of
			channels.
*/
class AUEffectBase : public AUBase {
public:
	explicit AUEffectBase(AudioComponentInstance audioUnit, bool inProcessesInPlace = true);

	AUEffectBase(const AUEffectBase&) = delete;
	AUEffectBase(AUEffectBase&&) = delete;
	AUEffectBase& operator=(const AUEffectBase&) = delete;
	AUEffectBase& operator=(AUEffectBase&&) = delete;

	~AUEffectBase() override = default;

	OSStatus Initialize() override;
	void Cleanup() override;
	OSStatus Reset(AudioUnitScope inScope, AudioUnitElement inElement) override;
	OSStatus GetPropertyInfo(AudioUnitPropertyID inID, AudioUnitScope inScope,
		AudioUnitElement inElement, UInt32& outDataSize, bool& outWritable) override;
	OSStatus GetProperty(AudioUnitPropertyID inID, AudioUnitScope inScope,
		AudioUnitElement inElement, void* outData) override;
	OSStatus SetProperty(AudioUnitPropertyID inID, AudioUnitScope inScope,
		AudioUnitElement inElement, const void* inData, UInt32 inDataSize) override;
	bool StreamFormatWritable(AudioUnitScope scope, AudioUnitElement element) override;
	OSStatus ChangeStreamFormat(AudioUnitScope inScope, AudioUnitElement inElement,
		const AudioStreamBasicDescription& inPrevFormat,
		const AudioStreamBasicDescription& inNewFormat) override;
	OSStatus Render(AudioUnitRenderActionFlags& ioActionFlags, const AudioTimeStamp& inTimeStamp,
		UInt32 nFrames) override;

	// our virtual methods

	// If your unit processes N to N channels, and there are no interactions between channels,
	// it can override NewKernel to create a mono processing object per channel.  Otherwise,
	// don't override NewKernel, and instead, override ProcessBufferLists.
	virtual std::unique_ptr<AUKernelBase> NewKernel() { return {}; }
	OSStatus ProcessBufferLists(AudioUnitRenderActionFlags& ioActionFlags,
		const AudioBufferList& inBuffer, AudioBufferList& outBuffer,
		UInt32 inFramesToProcess) override;

	// convenience format accessors (use output 0's format)
	Float64 GetSampleRate();
	UInt32 GetNumberOfChannels();

	// convenience wrappers for accessing parameters in the global scope
	using AUBase::SetParameter;

	void SetParameter(AudioUnitParameterID paramID, AudioUnitParameterValue value)
	{
		Globals()->SetParameter(paramID, value);
	}

	using AUBase::GetParameter;

	AudioUnitParameterValue GetParameter(AudioUnitParameterID paramID)
	{
		return Globals()->GetParameter(paramID);
	}

	[[nodiscard]] bool CanScheduleParameters() const override { return true; }

	// This is used for the property value - to reflect to the UI if an effect is bypassed
	[[nodiscard]] bool IsBypassEffect() const noexcept { return mBypassEffect; }

	virtual void SetBypassEffect(bool inFlag) { mBypassEffect = inFlag; }

	void SetParamHasSampleRateDependency(bool inFlag) noexcept { mParamSRDep = inFlag; }
	[[nodiscard]] bool GetParamHasSampleRateDependency() const noexcept { return mParamSRDep; }

	/// Context, passed as `void* userData`, for `ProcessScheduledSlice()`.
	struct ScheduledProcessParams {
		AudioUnitRenderActionFlags* actionFlags = nullptr;
		AudioBufferList* inputBufferList = nullptr;
		AudioBufferList* outputBufferList = nullptr;
	};

	OSStatus ProcessScheduledSlice(void* inUserData, UInt32 inStartFrameInBuffer,
		UInt32 inSliceFramesToProcess, UInt32 inTotalBufferFrames) override;

	[[nodiscard]] bool ProcessesInPlace() const noexcept { return mProcessesInPlace; }
	void SetProcessesInPlace(bool inProcessesInPlace) noexcept
	{
		mProcessesInPlace = inProcessesInPlace;
	}

	using KernelList = std::vector<std::unique_ptr<AUKernelBase>>;

protected:
	void MaintainKernels();

	// This is used in the render call to see if an effect is bypassed
	// It can return a different status than IsBypassEffect (though it MUST take that into account)
	virtual bool ShouldBypassEffect() { return IsBypassEffect(); }

	[[nodiscard]] AUKernelBase* GetKernel(UInt32 index) const
	{
		return (index < mKernelList.size()) ? mKernelList[index].get() : nullptr;
	}
	[[nodiscard]] const KernelList& GetKernelList() const noexcept { return mKernelList; }

	bool IsInputSilent(AudioUnitRenderActionFlags inActionFlags, UInt32 inFramesToProcess)
	{
		bool inputSilent = (inActionFlags & kAudioUnitRenderAction_OutputIsSilence) != 0;

		// take latency and tail time into account when propagating the silent bit
		const auto silentTimeoutFrames =
			static_cast<UInt32>(GetSampleRate() * (GetLatency() + GetTailTime()));
		mSilentTimeout.Process(inFramesToProcess, silentTimeoutFrames, inputSilent);
		return inputSilent;
	}

#if TARGET_OS_IPHONE
	void SetOnlyOneKernel(bool inUseOnlyOneKernel) noexcept
	{
		mOnlyOneKernel = inUseOnlyOneKernel;
	} // set in ctor of subclass that wants it.
#endif

private:
	KernelList mKernelList;
	bool mBypassEffect{ false };
	bool mParamSRDep{ false };
	bool mProcessesInPlace;
	AUSilentTimeout mSilentTimeout;
	AUOutputElement* mMainOutput{ nullptr };
	AUInputElement* mMainInput{ nullptr };

#if TARGET_OS_IPHONE
	bool mOnlyOneKernel;
#endif
	UInt32 mBytesPerFrame = 0;
};


/*!
	@class	AUKernelBase
	@brief	Base class for a signal-processing "kernel", an object that performs DSP on one channel
			of an audio stream.
*/
class AUKernelBase {
public:
	explicit AUKernelBase(AUEffectBase& inAudioUnit) : mAudioUnit(inAudioUnit) {}

	AUSDK_DEPRECATED("Construct with a reference")
	explicit AUKernelBase(AUEffectBase* inAudioUnit) : mAudioUnit(*inAudioUnit) {}

	AUKernelBase(const AUKernelBase&) = delete;
	AUKernelBase(AUKernelBase&&) = delete;
	AUKernelBase& operator=(const AUKernelBase&) = delete;
	AUKernelBase& operator=(AUKernelBase&&) = delete;

	virtual ~AUKernelBase() = default;

	virtual void Reset() {}

	virtual void Process(const Float32* /*inSourceP*/, Float32* /*inDestP*/,
		UInt32 /*inFramesToProcess*/, bool& /*ioSilence*/) = 0;

	Float64 GetSampleRate() { return mAudioUnit.GetSampleRate(); }

	AudioUnitParameterValue GetParameter(AudioUnitParameterID paramID)
	{
		return mAudioUnit.GetParameter(paramID);
	}

	void SetChannelNum(UInt32 inChan) noexcept { mChannelNum = inChan; }
	[[nodiscard]] UInt32 GetChannelNum() const noexcept { return mChannelNum; }

protected:
	AUEffectBase& mAudioUnit; // NOLINT protected
	UInt32 mChannelNum = 0;   // NOLINT protected
};

} // namespace ausdk

#endif // AudioUnitSDK_AUEffectBase_h

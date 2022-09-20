/*!
	@file		AudioUnitSDK/AUBase.cpp
	@copyright	© 2000-2021 Apple Inc. All rights reserved.
*/
#include <AudioUnitSDK/AUBase.h>
#include <AudioUnitSDK/AUInputElement.h>
#include <AudioUnitSDK/AUOutputElement.h>
#include <AudioUnitSDK/AUUtility.h>

#include <algorithm>
#include <array>
#include <cassert>
#include <limits>

namespace ausdk {

#if TARGET_OS_MAC && (TARGET_CPU_X86 || TARGET_CPU_X86_64)

class DenormalDisabler {
public:
	DenormalDisabler() : mSavedMXCSR(GetCSR()) { SetCSR(mSavedMXCSR | 0x8040); }

	DenormalDisabler(const DenormalDisabler&) = delete;
	DenormalDisabler(DenormalDisabler&&) = delete;
	DenormalDisabler& operator=(const DenormalDisabler&) = delete;
	DenormalDisabler& operator=(DenormalDisabler&&) = delete;

	~DenormalDisabler() { SetCSR(mSavedMXCSR); }

private:
#if 0 // not sure if this is right: // #if __has_include(<xmmintrin.h>)
	static unsigned GetCSR() { return _mm_getcsr(); }
	static void SetCSR(unsigned x) { _mm_setcsr(x); }
#else
	// our compiler does ALL floating point with SSE
	static unsigned GetCSR()
	{
		unsigned result{};
		asm volatile("stmxcsr %0" : "=m"(*&result)); // NOLINT asm
		return result;
	}
	static void SetCSR(unsigned a)
	{
		unsigned temp = a;
		asm volatile("ldmxcsr %0" : : "m"(*&temp)); // NOLINT asm
	}
#endif

	unsigned const mSavedMXCSR;
};

#else
// while denormals can be flushed to zero on ARM processors, there is no performance benefit
class DenormalDisabler {
public:
	DenormalDisabler() = default;
};
#endif

static std::once_flag sAUBaseCFStringsInitialized{}; // NOLINT non-const global
// this is used for the presets
static CFStringRef kUntitledString = nullptr; // NOLINT non-const global
// these are the current keys for the class info document
static CFStringRef kVersionString = nullptr;       // NOLINT non-const global
static CFStringRef kTypeString = nullptr;          // NOLINT non-const global
static CFStringRef kSubtypeString = nullptr;       // NOLINT non-const global
static CFStringRef kManufacturerString = nullptr;  // NOLINT non-const global
static CFStringRef kDataString = nullptr;          // NOLINT non-const global
static CFStringRef kNameString = nullptr;          // NOLINT non-const global
static CFStringRef kRenderQualityString = nullptr; // NOLINT non-const global
static CFStringRef kElementNameString = nullptr;   // NOLINT non-const global
static CFStringRef kPartString = nullptr;          // NOLINT non-const global

static constexpr auto kNoLastRenderedSampleTime = std::numeric_limits<Float64>::lowest();

//_____________________________________________________________________________
//
AUBase::AUBase(AudioComponentInstance inInstance, UInt32 numInputElements, UInt32 numOutputElements,
	UInt32 numGroupElements)
	: ComponentBase(inInstance), mInitNumInputEls(numInputElements),
	  mInitNumOutputEls(numOutputElements), mInitNumGroupEls(numGroupElements),
	  mLogString(CreateLoggingString())
{
	ResetRenderTime();

	std::call_once(sAUBaseCFStringsInitialized, []() {
		kUntitledString = CFSTR("Untitled");                     // NOLINT
		kVersionString = CFSTR(kAUPresetVersionKey);             // NOLINT
		kTypeString = CFSTR(kAUPresetTypeKey);                   // NOLINT
		kSubtypeString = CFSTR(kAUPresetSubtypeKey);             // NOLINT
		kManufacturerString = CFSTR(kAUPresetManufacturerKey);   // NOLINT
		kDataString = CFSTR(kAUPresetDataKey);                   // NOLINT
		kNameString = CFSTR(kAUPresetNameKey);                   // NOLINT
		kRenderQualityString = CFSTR(kAUPresetRenderQualityKey); // NOLINT
		kElementNameString = CFSTR(kAUPresetElementNameKey);     // NOLINT
		kPartString = CFSTR(kAUPresetPartKey);                   // NOLINT
	});

	GlobalScope().Initialize(this, kAudioUnitScope_Global, 1);

	mCurrentPreset.presetNumber = -1;
	CFRetain(mCurrentPreset.presetName = kUntitledString);
}

//_____________________________________________________________________________
//
AUBase::~AUBase()
{
	if (mCurrentPreset.presetName != nullptr) {
		CFRelease(mCurrentPreset.presetName);
	}
}

//_____________________________________________________________________________
//
void AUBase::PostConstructorInternal()
{
	// invoked after construction because they are virtual methods and/or call virtual methods
	if (mMaxFramesPerSlice == 0) {
		SetMaxFramesPerSlice(kAUDefaultMaxFramesPerSlice);
	}
	CreateElements();
}

//_____________________________________________________________________________
//
void AUBase::PreDestructorInternal()
{
	// this is called from the ComponentBase dispatcher, which doesn't know anything about our
	// (optional) lock
	const AUEntryGuard guard(mAUMutex);
	DoCleanup();
}

//_____________________________________________________________________________
//
void AUBase::CreateElements()
{
	if (!mElementsCreated) {
		Inputs().Initialize(this, kAudioUnitScope_Input, mInitNumInputEls);
		Outputs().Initialize(this, kAudioUnitScope_Output, mInitNumOutputEls);
		Groups().Initialize(this, kAudioUnitScope_Group, mInitNumGroupEls);
		CreateExtendedElements();

		mElementsCreated = true;
	}
}

//_____________________________________________________________________________
//
void AUBase::SetMaxFramesPerSlice(UInt32 nFrames)
{
	if (nFrames == mMaxFramesPerSlice) {
		return;
	}

	mMaxFramesPerSlice = nFrames;
	if (mBuffersAllocated) {
		ReallocateBuffers();
	}
	PropertyChanged(kAudioUnitProperty_MaximumFramesPerSlice, kAudioUnitScope_Global, 0);
}

//_____________________________________________________________________________
//
OSStatus AUBase::CanSetMaxFrames() const
{
	return IsInitialized() ? kAudioUnitErr_Initialized : OSStatus(noErr);
}

//_____________________________________________________________________________
//
void AUBase::ReallocateBuffers()
{
	CreateElements();

	const UInt32 nOutputs = Outputs().GetNumberOfElements();
	for (UInt32 i = 0; i < nOutputs; ++i) {
		Output(i).AllocateBuffer(); // does no work if already allocated
	}
	const UInt32 nInputs = Inputs().GetNumberOfElements();
	for (UInt32 i = 0; i < nInputs; ++i) {
		Input(i).AllocateBuffer(); // does no work if already allocated
	}
	mBuffersAllocated = true;
}

//_____________________________________________________________________________
//
void AUBase::DeallocateIOBuffers()
{
	if (!mBuffersAllocated) {
		return;
	}

	const UInt32 nOutputs = Outputs().GetNumberOfElements();
	for (UInt32 i = 0; i < nOutputs; ++i) {
		Output(i).DeallocateBuffer();
	}
	const UInt32 nInputs = Inputs().GetNumberOfElements();
	for (UInt32 i = 0; i < nInputs; ++i) {
		Input(i).DeallocateBuffer();
	}
	mBuffersAllocated = false;
}

//_____________________________________________________________________________
//
OSStatus AUBase::DoInitialize()
{
	OSStatus result = noErr;

	if (!mInitialized) {
		result = Initialize();
		if (result == noErr) {
			if (CanScheduleParameters()) {
				mParamEventList.reserve(24); // NOLINT magic #
			}
			mHasBegunInitializing = true;
			ReallocateBuffers(); // calls CreateElements()
			mInitialized = true; // signal that it's okay to render
			std::atomic_thread_fence(std::memory_order_seq_cst);
		}
	}

	return result;
}

//_____________________________________________________________________________
//
OSStatus AUBase::Initialize() { return noErr; }

//_____________________________________________________________________________
//
void AUBase::DoCleanup()
{
	if (mInitialized) {
		Cleanup();
	}

	DeallocateIOBuffers();
	ResetRenderTime();

	mInitialized = false;
	mHasBegunInitializing = false;
}

//_____________________________________________________________________________
//
void AUBase::Cleanup() {}

//_____________________________________________________________________________
//
OSStatus AUBase::Reset(AudioUnitScope /*inScope*/, AudioUnitElement /*inElement*/)
{
	ResetRenderTime();
	return noErr;
}

//_____________________________________________________________________________
//
OSStatus AUBase::DispatchGetPropertyInfo(AudioUnitPropertyID inID, AudioUnitScope inScope,
	AudioUnitElement inElement, UInt32& outDataSize, bool& outWritable)
{
	OSStatus result = noErr;
	bool validateElement = true;

	switch (inID) {
	case kAudioUnitProperty_MakeConnection:
		AUSDK_Require(inScope == kAudioUnitScope_Input || inScope == kAudioUnitScope_Global,
			kAudioUnitErr_InvalidScope);
		outDataSize = sizeof(AudioUnitConnection);
		outWritable = true;
		break;

	case kAudioUnitProperty_SetRenderCallback:
		AUSDK_Require(inScope == kAudioUnitScope_Input || inScope == kAudioUnitScope_Global,
			kAudioUnitErr_InvalidScope);
		outDataSize = sizeof(AURenderCallbackStruct);
		outWritable = true;
		break;

	case kAudioUnitProperty_StreamFormat:
		outDataSize = sizeof(AudioStreamBasicDescription);
		outWritable = IsStreamFormatWritable(inScope, inElement);
		break;

	case kAudioUnitProperty_SampleRate:
		outDataSize = sizeof(Float64);
		outWritable = IsStreamFormatWritable(inScope, inElement);
		break;

	case kAudioUnitProperty_ClassInfo:
		AUSDK_Require(inScope == kAudioUnitScope_Global, kAudioUnitErr_InvalidScope);
		outDataSize = sizeof(CFPropertyListRef);
		outWritable = true;
		break;

	case kAudioUnitProperty_FactoryPresets:
		AUSDK_Require(inScope == kAudioUnitScope_Global, kAudioUnitErr_InvalidScope);
		AUSDK_Require_noerr(GetPresets(nullptr));
		outDataSize = sizeof(CFArrayRef);
		outWritable = false;
		break;

	case kAudioUnitProperty_PresentPreset:
		AUSDK_Require(inScope == kAudioUnitScope_Global, kAudioUnitErr_InvalidScope);
		outDataSize = sizeof(AUPreset);
		outWritable = true;
		break;

	case kAudioUnitProperty_ElementName:
		outDataSize = sizeof(CFStringRef);
		outWritable = true;
		break;

	case kAudioUnitProperty_ParameterList: {
		UInt32 nparams = 0;
		AUSDK_Require_noerr(GetParameterList(inScope, nullptr, nparams));
		outDataSize = sizeof(AudioUnitParameterID) * nparams;
		outWritable = false;
		validateElement = false;
		break;
	}

	case kAudioUnitProperty_ParameterInfo:
		outDataSize = sizeof(AudioUnitParameterInfo);
		outWritable = false;
		validateElement = false;
		break;

	case kAudioUnitProperty_ParameterHistoryInfo:
		outDataSize = sizeof(AudioUnitParameterHistoryInfo);
		outWritable = false;
		validateElement = false;
		break;

	case kAudioUnitProperty_ElementCount:
		outDataSize = sizeof(UInt32);
		outWritable = BusCountWritable(inScope);
		validateElement = false;
		break;

	case kAudioUnitProperty_Latency:
		AUSDK_Require(inScope == kAudioUnitScope_Global, kAudioUnitErr_InvalidScope);
		outDataSize = sizeof(Float64);
		outWritable = false;
		break;

	case kAudioUnitProperty_TailTime:
		AUSDK_Require(inScope == kAudioUnitScope_Global, kAudioUnitErr_InvalidScope);
		AUSDK_Require(SupportsTail(), kAudioUnitErr_InvalidProperty);
		outDataSize = sizeof(Float64);
		outWritable = false;
		break;

	case kAudioUnitProperty_MaximumFramesPerSlice:
		AUSDK_Require(inScope == kAudioUnitScope_Global, kAudioUnitErr_InvalidScope);
		outDataSize = sizeof(UInt32);
		outWritable = true;
		break;

	case kAudioUnitProperty_LastRenderError:
		AUSDK_Require(inScope == kAudioUnitScope_Global, kAudioUnitErr_InvalidScope);
		outDataSize = sizeof(OSStatus);
		outWritable = false;
		break;

	case kAudioUnitProperty_SupportedNumChannels: {
		AUSDK_Require(inScope == kAudioUnitScope_Global, kAudioUnitErr_InvalidScope);
		const UInt32 num = SupportedNumChannels(nullptr);
		AUSDK_Require(num != 0u, kAudioUnitErr_InvalidProperty);
		outDataSize = sizeof(AUChannelInfo) * num;
		outWritable = false;
		break;
	}

	case kAudioUnitProperty_SupportedChannelLayoutTags: {
		const auto tags = GetChannelLayoutTags(inScope, inElement);
		AUSDK_Require(!tags.empty(), kAudioUnitErr_InvalidProperty);
		outDataSize = static_cast<UInt32>(tags.size() * sizeof(AudioChannelLayoutTag));
		outWritable = false;
		validateElement = false; // already done it
		break;
	}

	case kAudioUnitProperty_AudioChannelLayout: {
		outWritable = false;
		outDataSize = GetAudioChannelLayout(inScope, inElement, nullptr, outWritable);
		if (outDataSize != 0u) {
			result = noErr;
		} else {
			const auto tags = GetChannelLayoutTags(inScope, inElement);
			return tags.empty() ? kAudioUnitErr_InvalidProperty
								: kAudioUnitErr_InvalidPropertyValue;
		}
		validateElement = false; // already done it
		break;
	}

	case kAudioUnitProperty_ShouldAllocateBuffer:
		AUSDK_Require((inScope == kAudioUnitScope_Input || inScope == kAudioUnitScope_Output),
			kAudioUnitErr_InvalidScope);
		outWritable = true;
		outDataSize = sizeof(UInt32);
		break;

	case kAudioUnitProperty_ParameterValueStrings:
		AUSDK_Require_noerr(GetParameterValueStrings(inScope, inElement, nullptr));
		outDataSize = sizeof(CFArrayRef);
		outWritable = false;
		validateElement = false;
		break;

	case kAudioUnitProperty_HostCallbacks:
		AUSDK_Require(inScope == kAudioUnitScope_Global, kAudioUnitErr_InvalidScope);
		outDataSize = sizeof(mHostCallbackInfo);
		outWritable = true;
		break;

	case kAudioUnitProperty_ContextName:
		AUSDK_Require(inScope == kAudioUnitScope_Global, kAudioUnitErr_InvalidScope);
		outDataSize = sizeof(CFStringRef);
		outWritable = true;
		break;

#if !TARGET_OS_IPHONE
	case kAudioUnitProperty_IconLocation:
		AUSDK_Require(inScope == kAudioUnitScope_Global, kAudioUnitErr_InvalidScope);
		AUSDK_Require(HasIcon(), kAudioUnitErr_InvalidProperty);
		outWritable = false;
		outDataSize = sizeof(CFURLRef);
		break;
#endif

	case kAudioUnitProperty_ParameterClumpName:
		outDataSize = sizeof(AudioUnitParameterNameInfo);
		outWritable = false;
		break;

	case 61: // kAudioUnitProperty_LastRenderSampleTime
		AUSDK_Require(inScope == kAudioUnitScope_Global, kAudioUnitErr_InvalidScope);
		outDataSize = sizeof(Float64);
		outWritable = false;
		break;

	case kAudioUnitProperty_NickName:
		AUSDK_Require(inScope == kAudioUnitScope_Global, kAudioUnitErr_InvalidScope);
		outDataSize = sizeof(CFStringRef);
		outWritable = true;
		break;

	default:
		result = GetPropertyInfo(inID, inScope, inElement, outDataSize, outWritable);
		validateElement = false;
		break;
	}

	if ((result == noErr) && validateElement) {
		AUSDK_Require(GetElement(inScope, inElement) != nullptr, kAudioUnitErr_InvalidElement);
	}

	return result;
}

//_____________________________________________________________________________
//
// NOLINTNEXTLINE(misc-no-recursion) with SaveState
OSStatus AUBase::DispatchGetProperty(
	AudioUnitPropertyID inID, AudioUnitScope inScope, AudioUnitElement inElement, void* outData)
{
	// NOTE: We're currently only called from AUBase::ComponentEntryDispatch, which
	// calls DispatchGetPropertyInfo first, which performs validation of the scope/element,
	// and ensures that the outData buffer is non-null and large enough.
	OSStatus result = noErr;

	switch (inID) {
	case kAudioUnitProperty_StreamFormat:
		*static_cast<AudioStreamBasicDescription*>(outData) = GetStreamFormat(inScope, inElement);
		break;

	case kAudioUnitProperty_SampleRate:
		*static_cast<Float64*>(outData) = GetStreamFormat(inScope, inElement).mSampleRate;
		break;

	case kAudioUnitProperty_ParameterList: {
		UInt32 nparams = 0;
		result = GetParameterList(inScope, static_cast<AudioUnitParameterID*>(outData), nparams);
		break;
	}

	case kAudioUnitProperty_ParameterInfo:
		*static_cast<AudioUnitParameterInfo*>(outData) = {};
		result =
			GetParameterInfo(inScope, inElement, *static_cast<AudioUnitParameterInfo*>(outData));
		break;

	case kAudioUnitProperty_ParameterHistoryInfo: {
		auto* const info = static_cast<AudioUnitParameterHistoryInfo*>(outData);
		result = GetParameterHistoryInfo(
			inScope, inElement, info->updatesPerSecond, info->historyDurationInSeconds);
		break;
	}

	case kAudioUnitProperty_ClassInfo: {
		*static_cast<CFPropertyListRef*>(outData) = nullptr;
		result = SaveState(static_cast<CFPropertyListRef*>(outData));
		break;
	}

	case kAudioUnitProperty_FactoryPresets: {
		*static_cast<CFArrayRef*>(outData) = nullptr;
		result = GetPresets(static_cast<CFArrayRef*>(outData));
		break;
	}

	case kAudioUnitProperty_PresentPreset: {
		*static_cast<AUPreset*>(outData) = mCurrentPreset;

		// retain current string (as client owns a reference to it and will release it)
		if ((inID == kAudioUnitProperty_PresentPreset) && (mCurrentPreset.presetName != nullptr)) {
			CFRetain(mCurrentPreset.presetName);
		}

		result = noErr;
		break;
	}

	case kAudioUnitProperty_ElementName: {
		const AUElement* const element = GetElement(inScope, inElement);
		const CFStringRef name = *element->GetName();
		AUSDK_Require(name != nullptr, kAudioUnitErr_PropertyNotInUse);
		CFRetain(name); // must return a +1 reference
		*static_cast<CFStringRef*>(outData) = name;
		break;
	}

	case kAudioUnitProperty_ElementCount:
		*static_cast<UInt32*>(outData) = GetScope(inScope).GetNumberOfElements();
		break;

	case kAudioUnitProperty_Latency:
		*static_cast<Float64*>(outData) = GetLatency();
		break;

	case kAudioUnitProperty_TailTime:
		AUSDK_Require(SupportsTail(), kAudioUnitErr_InvalidProperty);
		*static_cast<Float64*>(outData) = GetTailTime();
		break;

	case kAudioUnitProperty_MaximumFramesPerSlice:
		*static_cast<UInt32*>(outData) = mMaxFramesPerSlice;
		break;

	case kAudioUnitProperty_LastRenderError:
		*static_cast<OSStatus*>(outData) = mLastRenderError;
		mLastRenderError = 0;
		break;

	case kAudioUnitProperty_SupportedNumChannels: {
		const AUChannelInfo* infoPtr = nullptr;
		const UInt32 num = SupportedNumChannels(&infoPtr);
		if (num != 0 && infoPtr != nullptr) {
			memcpy(outData, infoPtr, num * sizeof(AUChannelInfo));
		}
		break;
	}

	case kAudioUnitProperty_SupportedChannelLayoutTags: {
		const auto tags = GetChannelLayoutTags(inScope, inElement);
		AUSDK_Require(!tags.empty(), kAudioUnitErr_InvalidProperty);
		AudioChannelLayoutTag* const ptr =
			outData != nullptr ? static_cast<AudioChannelLayoutTag*>(outData) : nullptr;
		if (ptr != nullptr) {
			memcpy(ptr, tags.data(), tags.size() * sizeof(AudioChannelLayoutTag));
		}
		break;
	}

	case kAudioUnitProperty_AudioChannelLayout: {
		AudioChannelLayout* const ptr =
			outData != nullptr ? static_cast<AudioChannelLayout*>(outData) : nullptr;
		bool writable = false;
		const UInt32 dataSize = GetAudioChannelLayout(inScope, inElement, ptr, writable);
		AUSDK_Require(dataSize != 0, kAudioUnitErr_InvalidProperty);
		break;
	}

	case kAudioUnitProperty_ShouldAllocateBuffer: {
		const auto& element = IOElement(inScope, inElement);
		*static_cast<UInt32*>(outData) = static_cast<UInt32>(element.WillAllocateBuffer());
		break;
	}

	case kAudioUnitProperty_ParameterValueStrings:
		result = GetParameterValueStrings(inScope, inElement, static_cast<CFArrayRef*>(outData));
		break;

	case kAudioUnitProperty_HostCallbacks:
		memcpy(outData, &mHostCallbackInfo, sizeof(mHostCallbackInfo));
		break;

	case kAudioUnitProperty_ContextName:
		if (const CFStringRef name = *mContextName) {
			CFRetain(name); // must return a +1 reference
			*static_cast<CFStringRef*>(outData) = name;
			result = noErr;
		} else {
			*static_cast<CFStringRef*>(outData) = nullptr;
			result = kAudioUnitErr_PropertyNotInUse;
		}
		break;

#if !TARGET_OS_IPHONE
	case kAudioUnitProperty_IconLocation: {
		const CFURLRef iconLocation = CopyIconLocation();
		AUSDK_Require(iconLocation != nullptr, kAudioUnitErr_InvalidProperty);
		*static_cast<CFURLRef*>(outData) = iconLocation;
		break;
	}
#endif

	case kAudioUnitProperty_ParameterClumpName: {
		auto* const ioClumpInfo = static_cast<AudioUnitParameterNameInfo*>(outData);
		AUSDK_Require(ioClumpInfo->inID != kAudioUnitClumpID_System,
			kAudioUnitErr_InvalidPropertyValue); // this ID value is reserved
		result = CopyClumpName(inScope, ioClumpInfo->inID,
			static_cast<UInt32>(std::max(ioClumpInfo->inDesiredLength, SInt32(0))),
			&ioClumpInfo->outName);

		// this is provided for compatbility with existing implementations that don't know
		// about this new mechanism
		if (result == kAudioUnitErr_InvalidProperty) {
			result = GetProperty(inID, inScope, inElement, outData);
		}
		break;
	}

	case 61: // kAudioUnitProperty_LastRenderSampleTime
		*static_cast<Float64*>(outData) = mCurrentRenderTime.mSampleTime;
		break;

	case kAudioUnitProperty_NickName:
		// Ownership follows Core Foundation's 'Copy Rule'
		if (const auto* const name = *mNickName) {
			CFRetain(name);
			*static_cast<CFStringRef*>(outData) = name;
		} else {
			*static_cast<CFStringRef*>(outData) = nullptr;
		}
		break;

	default:
		result = GetProperty(inID, inScope, inElement, outData);
		break;
	}
	return result;
}


//_____________________________________________________________________________
//
// Note: We can be sure inData is non-null; otherwise RemoveProperty would have been called.
// NOLINTNEXTLINE(misc-no-recursion) with RestoreState
OSStatus AUBase::DispatchSetProperty(AudioUnitPropertyID inID, AudioUnitScope inScope,
	AudioUnitElement inElement, const void* inData, UInt32 inDataSize)
{
	OSStatus result = noErr;

	switch (inID) {
	case kAudioUnitProperty_MakeConnection: {
		AUSDK_Require(
			inDataSize >= sizeof(AudioUnitConnection), kAudioUnitErr_InvalidPropertyValue);
		const auto& connection = *static_cast<const AudioUnitConnection*>(inData);
		result = SetConnection(connection);
		break;
	}

	case kAudioUnitProperty_SetRenderCallback: {
		AUSDK_Require(
			inDataSize >= sizeof(AURenderCallbackStruct), kAudioUnitErr_InvalidPropertyValue);
		const auto& callback = *static_cast<const AURenderCallbackStruct*>(inData);
		result = SetInputCallback(kAudioUnitProperty_SetRenderCallback, inElement,
			callback.inputProc, callback.inputProcRefCon);
		break;
	}

	case kAudioUnitProperty_ElementCount:
		AUSDK_Require(inDataSize == sizeof(UInt32), kAudioUnitErr_InvalidPropertyValue);
		AUSDK_Require(BusCountWritable(inScope), kAudioUnitErr_PropertyNotWritable);
		result = SetBusCount(inScope, *static_cast<const UInt32*>(inData));
		if (result == noErr) {
			PropertyChanged(inID, inScope, inElement);
		}
		break;

	case kAudioUnitProperty_MaximumFramesPerSlice:
		AUSDK_Require(inDataSize == sizeof(UInt32), kAudioUnitErr_InvalidPropertyValue);
		AUSDK_Require_noerr(CanSetMaxFrames());
		SetMaxFramesPerSlice(*static_cast<const UInt32*>(inData));
		break;

	case kAudioUnitProperty_StreamFormat: {
		constexpr static UInt32 kMinimumValidASBDSize = 36;
		AUSDK_Require(inDataSize >= kMinimumValidASBDSize, kAudioUnitErr_InvalidPropertyValue);
		AUSDK_Require(GetElement(inScope, inElement) != nullptr, kAudioUnitErr_InvalidElement);

		AudioStreamBasicDescription newDesc = {};
		// now we're going to be ultra conservative! because of discrepancies between
		// sizes of this struct based on aligment padding inconsistencies
		memcpy(&newDesc, inData, kMinimumValidASBDSize);

		AUSDK_Require(ASBD::MinimalSafetyCheck(newDesc), kAudioUnitErr_FormatNotSupported);

		AUSDK_Require(ValidFormat(inScope, inElement, newDesc), kAudioUnitErr_FormatNotSupported);

		const AudioStreamBasicDescription curDesc = GetStreamFormat(inScope, inElement);

		if (!ASBD::IsEqual(curDesc, newDesc)) {
			AUSDK_Require(
				IsStreamFormatWritable(inScope, inElement), kAudioUnitErr_PropertyNotWritable);
			result = ChangeStreamFormat(inScope, inElement, curDesc, newDesc);
		}
		break;
	}

	case kAudioUnitProperty_SampleRate: {
		AUSDK_Require(inDataSize == sizeof(Float64), kAudioUnitErr_InvalidPropertyValue);
		AUSDK_Require(GetElement(inScope, inElement) != nullptr, kAudioUnitErr_InvalidElement);

		const AudioStreamBasicDescription curDesc = GetStreamFormat(inScope, inElement);
		AudioStreamBasicDescription newDesc = curDesc;
		newDesc.mSampleRate = *static_cast<const Float64*>(inData);

		AUSDK_Require(ValidFormat(inScope, inElement, newDesc), kAudioUnitErr_FormatNotSupported);

		if (!ASBD::IsEqual(curDesc, newDesc)) {
			AUSDK_Require(
				IsStreamFormatWritable(inScope, inElement), kAudioUnitErr_PropertyNotWritable);
			result = ChangeStreamFormat(inScope, inElement, curDesc, newDesc);
		}
		break;
	}

	case kAudioUnitProperty_AudioChannelLayout: {
		const auto& layout = *static_cast<const AudioChannelLayout*>(inData);
		constexpr size_t headerSize = sizeof(AudioChannelLayout) - sizeof(AudioChannelDescription);

		AUSDK_Require(inDataSize >= offsetof(AudioChannelLayout, mNumberChannelDescriptions) +
										sizeof(AudioChannelLayout::mNumberChannelDescriptions),
			kAudioUnitErr_InvalidPropertyValue);
		AUSDK_Require(inDataSize >= headerSize + layout.mNumberChannelDescriptions *
													 sizeof(AudioChannelDescription),
			kAudioUnitErr_InvalidPropertyValue);
		result = SetAudioChannelLayout(inScope, inElement, &layout);
		if (result == noErr) {
			PropertyChanged(inID, inScope, inElement);
		}
		break;
	}

	case kAudioUnitProperty_ClassInfo:
		AUSDK_Require(inDataSize == sizeof(CFPropertyListRef*), kAudioUnitErr_InvalidPropertyValue);
		AUSDK_Require(inScope == kAudioUnitScope_Global, kAudioUnitErr_InvalidScope);
		result = RestoreState(*static_cast<const CFPropertyListRef*>(inData));
		break;

	case kAudioUnitProperty_PresentPreset: {
		AUSDK_Require(inDataSize == sizeof(AUPreset), kAudioUnitErr_InvalidPropertyValue);
		AUSDK_Require(inScope == kAudioUnitScope_Global, kAudioUnitErr_InvalidScope);
		const auto& newPreset = *static_cast<const AUPreset*>(inData);

		if (newPreset.presetNumber >= 0) {
			result = NewFactoryPresetSet(newPreset);
			// NewFactoryPresetSet SHOULD call SetAFactoryPreset if the preset is valid
			// from its own list of preset number->name
			if (result == noErr) {
				PropertyChanged(inID, inScope, inElement);
			}
		} else if (newPreset.presetName != nullptr) {
			result = NewCustomPresetSet(newPreset);
			if (result == noErr) {
				PropertyChanged(inID, inScope, inElement);
			}
		} else {
			result = kAudioUnitErr_InvalidPropertyValue;
		}
		break;
	}

	case kAudioUnitProperty_ElementName: {
		AUSDK_Require(GetElement(inScope, inElement) != nullptr, kAudioUnitErr_InvalidElement);
		AUSDK_Require(inDataSize == sizeof(CFStringRef), kAudioUnitErr_InvalidPropertyValue);
		const auto element = GetScope(inScope).GetElement(inElement);
		const CFStringRef inStr = *static_cast<const CFStringRef*>(inData);
		element->SetName(inStr);
		PropertyChanged(inID, inScope, inElement);
		break;
	}

	case kAudioUnitProperty_ShouldAllocateBuffer: {
		AUSDK_Require((inScope == kAudioUnitScope_Input || inScope == kAudioUnitScope_Output),
			kAudioUnitErr_InvalidScope);
		AUSDK_Require(GetElement(inScope, inElement) != nullptr, kAudioUnitErr_InvalidElement);
		AUSDK_Require(inDataSize == sizeof(UInt32), kAudioUnitErr_InvalidPropertyValue);
		AUSDK_Require(!IsInitialized(), kAudioUnitErr_Initialized);

		auto& element = IOElement(inScope, inElement);
		element.SetWillAllocateBuffer(*static_cast<const UInt32*>(inData) != 0);
		break;
	}

	case kAudioUnitProperty_HostCallbacks: {
		AUSDK_Require(inScope == kAudioUnitScope_Global, kAudioUnitErr_InvalidScope);
		const UInt32 availSize =
			std::min(inDataSize, static_cast<UInt32>(sizeof(HostCallbackInfo)));
		const bool hasChanged = memcmp(&mHostCallbackInfo, inData, availSize) == 0;
		mHostCallbackInfo = {};
		memcpy(&mHostCallbackInfo, inData, availSize);
		if (hasChanged) {
			PropertyChanged(inID, inScope, inElement);
		}
		break;
	}

	case kAudioUnitProperty_ContextName: {
		AUSDK_Require(inDataSize == sizeof(CFStringRef), kAudioUnitErr_InvalidPropertyValue);
		AUSDK_Require(inScope == kAudioUnitScope_Global, kAudioUnitErr_InvalidScope);
		const CFStringRef inStr = *static_cast<const CFStringRef*>(inData);
		mContextName = inStr;
		PropertyChanged(inID, inScope, inElement);
		break;
	}


	case kAudioUnitProperty_NickName: {
		AUSDK_Require(inScope == kAudioUnitScope_Global, kAudioUnitErr_InvalidScope);
		AUSDK_Require(inDataSize == sizeof(CFStringRef), kAudioUnitErr_InvalidPropertyValue);
		const CFStringRef inStr = *static_cast<const CFStringRef*>(inData);
		mNickName = inStr;
		PropertyChanged(inID, inScope, inElement);
		break;
	}

	default:
		result = SetProperty(inID, inScope, inElement, inData, inDataSize);
		if (result == noErr) {
			PropertyChanged(inID, inScope, inElement);
		}

		break;
	}
	return result;
}

//_____________________________________________________________________________
//
OSStatus AUBase::DispatchRemovePropertyValue(
	AudioUnitPropertyID inID, AudioUnitScope inScope, AudioUnitElement inElement)
{
	OSStatus result = noErr;
	switch (inID) {
	case kAudioUnitProperty_AudioChannelLayout: {
		result = RemoveAudioChannelLayout(inScope, inElement);
		if (result == noErr) {
			PropertyChanged(inID, inScope, inElement);
		}
		break;
	}

	case kAudioUnitProperty_HostCallbacks: {
		AUSDK_Require(inScope == kAudioUnitScope_Global, kAudioUnitErr_InvalidScope);
		bool hasValue = false;
		const void* const ptr = &mHostCallbackInfo;
		for (size_t i = 0; i < sizeof(HostCallbackInfo); ++i) {
			if (static_cast<const char*>(ptr)[i] != 0) { // NOLINT
				hasValue = true;
				break;
			}
		}
		if (hasValue) {
			mHostCallbackInfo = {};
			PropertyChanged(inID, inScope, inElement);
		}
		break;
	}

	case kAudioUnitProperty_ContextName:
		mContextName = nullptr;
		result = noErr;
		break;

	case kAudioUnitProperty_NickName: {
		AUSDK_Require(inScope == kAudioUnitScope_Global, kAudioUnitErr_InvalidScope);
		mNickName = nullptr;
		PropertyChanged(inID, inScope, inElement);
		break;
	}

	default:
		result = RemovePropertyValue(inID, inScope, inElement);
		break;
	}

	return result;
}

//_____________________________________________________________________________
//
OSStatus AUBase::GetPropertyInfo(AudioUnitPropertyID /*inID*/, AudioUnitScope /*inScope*/,
	AudioUnitElement /*inElement*/, UInt32& /*outDataSize*/, bool& /*outWritable*/)
{
	return kAudioUnitErr_InvalidProperty;
}


//_____________________________________________________________________________
//
OSStatus AUBase::GetProperty(AudioUnitPropertyID /*inID*/, AudioUnitScope /*inScope*/,
	AudioUnitElement /*inElement*/, void* /*outData*/)
{
	return kAudioUnitErr_InvalidProperty;
}


//_____________________________________________________________________________
//
OSStatus AUBase::SetProperty(AudioUnitPropertyID /*inID*/, AudioUnitScope /*inScope*/,
	AudioUnitElement /*inElement*/, const void* /*inData*/, UInt32 /*inDataSize*/)
{
	return kAudioUnitErr_InvalidProperty;
}

//_____________________________________________________________________________
//
OSStatus AUBase::RemovePropertyValue(
	AudioUnitPropertyID /*inID*/, AudioUnitScope /*inScope*/, AudioUnitElement /*inElement*/)
{
	return kAudioUnitErr_InvalidPropertyValue;
}

//_____________________________________________________________________________
//
OSStatus AUBase::AddPropertyListener(
	AudioUnitPropertyID inID, AudioUnitPropertyListenerProc inProc, void* inProcRefCon)
{
	const PropertyListener pl{
		.propertyID = inID, .listenerProc = inProc, .listenerRefCon = inProcRefCon
	};

	if (mPropertyListeners.empty()) {
		mPropertyListeners.reserve(32); // NOLINT magic#
	}
	mPropertyListeners.push_back(pl);

	return noErr;
}

//_____________________________________________________________________________
//
OSStatus AUBase::RemovePropertyListener(AudioUnitPropertyID inID,
	AudioUnitPropertyListenerProc inProc, void* inProcRefCon, bool refConSpecified)
{
	const auto iter =
		std::remove_if(mPropertyListeners.begin(), mPropertyListeners.end(), [&](auto& item) {
			return item.propertyID == inID && item.listenerProc == inProc &&
				   (!refConSpecified || item.listenerRefCon == inProcRefCon);
		});
	if (iter != mPropertyListeners.end()) {
		mPropertyListeners.erase(iter, mPropertyListeners.end());
	}
	return noErr;
}

//_____________________________________________________________________________
//
void AUBase::PropertyChanged(
	AudioUnitPropertyID inID, AudioUnitScope inScope, AudioUnitElement inElement)
{
	for (const auto& pl : mPropertyListeners) {
		if (pl.propertyID == inID) {
			(pl.listenerProc)(pl.listenerRefCon, GetComponentInstance(), inID, inScope, inElement);
		}
	}
}

//_____________________________________________________________________________
//
OSStatus AUBase::SetRenderNotification(AURenderCallback inProc, void* inRefCon)
{
	if (inProc == nullptr) {
		return kAudio_ParamError;
	}

	mRenderCallbacksTouched = true;
	mRenderCallbacks.add(RenderCallback(inProc, inRefCon));
	// this will do nothing if it's already in the list
	return noErr;
}

//_____________________________________________________________________________
//
OSStatus AUBase::RemoveRenderNotification(AURenderCallback inProc, void* inRefCon)
{
	mRenderCallbacks.remove(RenderCallback(inProc, inRefCon));
	return noErr; // error?
}

//_____________________________________________________________________________
//
OSStatus AUBase::GetParameter(AudioUnitParameterID inID, AudioUnitScope inScope,
	AudioUnitElement inElement, AudioUnitParameterValue& outValue)
{
	const auto& elem = Element(inScope, inElement);
	outValue = elem.GetParameter(inID);
	return noErr;
}


//_____________________________________________________________________________
//
OSStatus AUBase::SetParameter(AudioUnitParameterID inID, AudioUnitScope inScope,
	AudioUnitElement inElement, AudioUnitParameterValue inValue, UInt32 /*inBufferOffsetInFrames*/)
{
	auto& elem = Element(inScope, inElement);
	elem.SetParameter(inID, inValue);
	return noErr;
}

//_____________________________________________________________________________
//
OSStatus AUBase::ScheduleParameter(
	const AudioUnitParameterEvent* inParameterEvent, UInt32 inNumEvents)
{
	const bool canScheduleParameters = CanScheduleParameters();

	for (UInt32 i = 0; i < inNumEvents; ++i) {
		const auto& pe = inParameterEvent[i]; // NOLINT subscript
		if (pe.eventType == kParameterEvent_Immediate) {
			SetParameter(pe.parameter, pe.scope, pe.element,
				pe.eventValues.immediate.value,         // NOLINT union
				pe.eventValues.immediate.bufferOffset); // NOLINT union
		}
		if (canScheduleParameters) {
			mParamEventList.push_back(pe);
		}
	}

	return noErr;
}

// ____________________________________________________________________________
//
constexpr bool ParameterEventListSortPredicate(
	const AudioUnitParameterEvent& ev1, const AudioUnitParameterEvent& ev2) noexcept
{
	// ramp.startBufferOffset is signed
	const SInt32 offset1 =
		ev1.eventType == kParameterEvent_Immediate
			? static_cast<SInt32>(ev1.eventValues.immediate.bufferOffset) // NOLINT union
			: ev1.eventValues.ramp.startBufferOffset;                     // NOLINT union
	const SInt32 offset2 =
		ev2.eventType == kParameterEvent_Immediate
			? static_cast<SInt32>(ev2.eventValues.immediate.bufferOffset) // NOLINT union
			: ev2.eventValues.ramp.startBufferOffset;                     // NOLINT union

	return offset1 < offset2;
}


// ____________________________________________________________________________
//
OSStatus AUBase::ProcessForScheduledParams(
	ParameterEventList& inParamList, UInt32 inFramesToProcess, void* inUserData)
{
	OSStatus result = noErr;

	UInt32 framesRemaining = inFramesToProcess;

	UInt32 currentStartFrame = 0; // start of the whole buffer


	// sort the ParameterEventList by startBufferOffset
	std::sort(inParamList.begin(), inParamList.end(), ParameterEventListSortPredicate);

	while (framesRemaining > 0) {
		// first of all, go through the ramped automation events and find out where the next
		// division of our whole buffer will be

		UInt32 currentEndFrame = inFramesToProcess; // start out assuming we'll process all the way
													// to the end of the buffer

		// find the next break point
		for (const auto& event : inParamList) {
			SInt32 offset =
				event.eventType == kParameterEvent_Immediate
					? static_cast<SInt32>(event.eventValues.immediate.bufferOffset) // NOLINT
					: event.eventValues.ramp.startBufferOffset;

			if (offset > static_cast<SInt32>(currentStartFrame) &&
				offset < static_cast<SInt32>(currentEndFrame)) {
				currentEndFrame = static_cast<UInt32>(offset);
				break;
			}

			// consider ramp end to be a possible choice (there may be gaps in the supplied ramp
			// events)
			if (event.eventType == kParameterEvent_Ramped) {
				offset = event.eventValues.ramp.startBufferOffset +
						 static_cast<SInt32>(event.eventValues.ramp.durationInFrames); // NOLINT

				if (offset > static_cast<SInt32>(currentStartFrame) &&
					offset < static_cast<SInt32>(currentEndFrame)) {
					currentEndFrame = static_cast<UInt32>(offset);
				}
			}
		}

		const UInt32 framesThisTime = currentEndFrame - currentStartFrame;

		// next, setup the parameter maps to be current for the ramp parameters active during
		// this time segment...

		for (const auto& event : inParamList) {
			bool eventFallsInSlice = false;

			if (event.eventType == kParameterEvent_Ramped) {
				const auto& ramp = event.eventValues.ramp;
				eventFallsInSlice =
					ramp.startBufferOffset < static_cast<SInt32>(currentEndFrame) &&
					(ramp.startBufferOffset + static_cast<SInt32>(ramp.durationInFrames)) >
						static_cast<SInt32>(currentStartFrame);
			} else { /* kParameterEvent_Immediate */
				// actually, for the same parameter, there may be future immediate events which
				// override this one, but it's OK since the event list is sorted in time order,
				// we're guaranteed to end up with the current one
				eventFallsInSlice = event.eventValues.immediate.bufferOffset <= currentStartFrame;
			}

			if (eventFallsInSlice) {
				AUElement* const element = GetElement(event.scope, event.element);

				if (element != nullptr) {
					element->SetScheduledEvent(event.parameter, event, currentStartFrame,
						currentEndFrame - currentStartFrame);
				}
			}
		}


		// Finally, actually do the processing for this slice.....

		result =
			ProcessScheduledSlice(inUserData, currentStartFrame, framesThisTime, inFramesToProcess);

		if (result != noErr) {
			break;
		}

		framesRemaining -= std::min(framesThisTime, framesRemaining);
		currentStartFrame = currentEndFrame; // now start from where we left off last time
	}

	return result;
}

//_____________________________________________________________________________
//
void AUBase::ResetRenderTime()
{
	mCurrentRenderTime = {};
	mCurrentRenderTime.mSampleTime = kNoLastRenderedSampleTime;
}

//_____________________________________________________________________________
//
void AUBase::SetWantsRenderThreadID(bool inFlag)
{
	if (inFlag == mWantsRenderThreadID) {
		return;
	}

	mWantsRenderThreadID = inFlag;
	if (!mWantsRenderThreadID) {
		mRenderThreadID = {};
	};
}

//_____________________________________________________________________________
//
OSStatus AUBase::DoRender(AudioUnitRenderActionFlags& ioActionFlags,
	const AudioTimeStamp& inTimeStamp, UInt32 inBusNumber, UInt32 inFramesToProcess,
	AudioBufferList& ioData)
{
	const auto errorExit = [this](OSStatus error) {
		AUSDK_LogError("  from %s, render err: %d", GetLoggingString(), static_cast<int>(error));
		SetRenderError(error);
		return error;
	};

	OSStatus theError = noErr;

	[[maybe_unused]] const DenormalDisabler denormalDisabler;

	try {
		AUSDK_Require(IsInitialized(), errorExit(kAudioUnitErr_Uninitialized));
		if (inFramesToProcess > mMaxFramesPerSlice) {
#ifndef AUSDK_NO_LOGGING
			static UInt64 lastTimeMessagePrinted = 0;
			const UInt64 now = HostTime::Current();
			if (static_cast<double>(now - lastTimeMessagePrinted) >
				mHostTimeFrequency) { // not more than once per second.
				lastTimeMessagePrinted = now;
				AUSDK_LogError("kAudioUnitErr_TooManyFramesToProcess : inFramesToProcess=%u, "
							   "mMaxFramesPerSlice=%u",
					static_cast<unsigned>(inFramesToProcess),
					static_cast<unsigned>(mMaxFramesPerSlice));
			}
#endif
			return errorExit(kAudioUnitErr_TooManyFramesToProcess);
		}
		AUSDK_Require(!UsesFixedBlockSize() || inFramesToProcess == GetMaxFramesPerSlice(),
			errorExit(kAudio_ParamError));

		auto& output = Output(inBusNumber); // will throw if non-existant
		if (ASBD::NumberChannelStreams(output.GetStreamFormat()) != ioData.mNumberBuffers) {
			AUSDK_LogError(
				"ioData.mNumberBuffers=%u, "
				"ASBD::NumberChannelStreams(output.GetStreamFormat())=%u; kAudio_ParamError",
				static_cast<unsigned>(ioData.mNumberBuffers),
				static_cast<unsigned>(ASBD::NumberChannelStreams(output.GetStreamFormat())));
			return errorExit(kAudio_ParamError);
		}

		const unsigned expectedBufferByteSize =
			inFramesToProcess * output.GetStreamFormat().mBytesPerFrame;
		for (unsigned ibuf = 0; ibuf < ioData.mNumberBuffers; ++ibuf) {
			AudioBuffer& buf = ioData.mBuffers[ibuf]; // NOLINT
			if (buf.mData != nullptr) {
				// only care about the size if the buffer is non-null
				if (buf.mDataByteSize < expectedBufferByteSize) {
					// if the buffer is too small, we cannot render safely. kAudio_ParamError.
					AUSDK_LogError("%u frames, %u bytes/frame, expected %u-byte buffer; "
								   "ioData.mBuffers[%u].mDataByteSize=%u; kAudio_ParamError",
						static_cast<unsigned>(inFramesToProcess),
						static_cast<unsigned>(output.GetStreamFormat().mBytesPerFrame),
						expectedBufferByteSize, ibuf, static_cast<unsigned>(buf.mDataByteSize));
					return errorExit(kAudio_ParamError);
				}
				// Some clients incorrectly pass bigger buffers than expectedBufferByteSize.
				// We will generally set the buffer size at the end of rendering, before we return.
				// However we should ensure that no one, DURING rendering, READS a
				// potentially incorrect size. This can lead to doing too much work, or
				// reading past the end of an input buffer into unmapped memory.
				buf.mDataByteSize = expectedBufferByteSize;
			}
		}

		if (WantsRenderThreadID()) {
			mRenderThreadID = std::this_thread::get_id();
		}

		if (mRenderCallbacksTouched) {
			AudioUnitRenderActionFlags flags = ioActionFlags | kAudioUnitRenderAction_PreRender;
			mRenderCallbacks.foreach ([&](const RenderCallback& rc) {
				(*static_cast<AURenderCallback>(rc.mRenderNotify))(rc.mRenderNotifyRefCon, &flags,
					&inTimeStamp, inBusNumber, inFramesToProcess, &ioData);
			});
		}

		theError =
			DoRenderBus(ioActionFlags, inTimeStamp, inBusNumber, output, inFramesToProcess, ioData);

		SetRenderError(theError);

		if (mRenderCallbacksTouched) {
			AudioUnitRenderActionFlags flags = ioActionFlags | kAudioUnitRenderAction_PostRender;

			if (theError != noErr) {
				flags |= kAudioUnitRenderAction_PostRenderError;
			}

			mRenderCallbacks.foreach ([&](const RenderCallback& rc) {
				(*static_cast<AURenderCallback>(rc.mRenderNotify))(rc.mRenderNotifyRefCon, &flags,
					&inTimeStamp, inBusNumber, inFramesToProcess, &ioData);
			});
		}

		// The vector's being emptied
		// because these events should only apply to this Render cycle, so anything
		// left over is from a preceding cycle and should be dumped.  New scheduled
		// parameters must be scheduled from the next pre-render callback.
		if (!mParamEventList.empty()) {
			mParamEventList.clear();
		}
	} catch (const OSStatus& err) {
		return errorExit(err);
	} catch (...) {
		return errorExit(-1);
	}
	return theError;
}

inline bool CheckRenderArgs(AudioUnitRenderActionFlags flags)
{
	return (flags & kAudioUnitRenderAction_DoNotCheckRenderArgs) == 0u;
}

//_____________________________________________________________________________
//
OSStatus AUBase::DoProcess(AudioUnitRenderActionFlags& ioActionFlags,
	const AudioTimeStamp& inTimeStamp, UInt32 inFramesToProcess, AudioBufferList& ioData)
{
	const auto errorExit = [this](OSStatus error) {
		AUSDK_LogError("  from %s, process err: %d", GetLoggingString(), static_cast<int>(error));
		SetRenderError(error);
		return error;
	};

	OSStatus theError = noErr;

	[[maybe_unused]] const DenormalDisabler denormalDisabler;

	try {
		if (CheckRenderArgs(ioActionFlags)) {
			AUSDK_Require(IsInitialized(), errorExit(kAudioUnitErr_Uninitialized));
			AUSDK_Require(inFramesToProcess <= mMaxFramesPerSlice,
				errorExit(kAudioUnitErr_TooManyFramesToProcess));
			AUSDK_Require(!UsesFixedBlockSize() || inFramesToProcess == GetMaxFramesPerSlice(),
				errorExit(kAudio_ParamError));

			const auto& input = Input(0); // will throw if non-existant
			if (ASBD::NumberChannelStreams(input.GetStreamFormat()) != ioData.mNumberBuffers) {
				AUSDK_LogError(
					"ioData.mNumberBuffers=%u, "
					"ASBD::NumberChannelStreams(input->GetStreamFormat())=%u; kAudio_ParamError",
					static_cast<unsigned>(ioData.mNumberBuffers),
					static_cast<unsigned>(ASBD::NumberChannelStreams(input.GetStreamFormat())));
				return errorExit(kAudio_ParamError);
			}

			const unsigned expectedBufferByteSize =
				inFramesToProcess * input.GetStreamFormat().mBytesPerFrame;
			for (unsigned ibuf = 0; ibuf < ioData.mNumberBuffers; ++ibuf) {
				AudioBuffer& buf = ioData.mBuffers[ibuf]; // NOLINT
				if (buf.mData != nullptr) {
					// only care about the size if the buffer is non-null
					if (buf.mDataByteSize < expectedBufferByteSize) {
						// if the buffer is too small, we cannot render safely. kAudio_ParamError.
						AUSDK_LogError("%u frames, %u bytes/frame, expected %u-byte buffer; "
									   "ioData.mBuffers[%u].mDataByteSize=%u; kAudio_ParamError",
							static_cast<unsigned>(inFramesToProcess),
							static_cast<unsigned>(input.GetStreamFormat().mBytesPerFrame),
							expectedBufferByteSize, ibuf, static_cast<unsigned>(buf.mDataByteSize));
						return errorExit(kAudio_ParamError);
					}
					// Some clients incorrectly pass bigger buffers than expectedBufferByteSize.
					// We will generally set the buffer size at the end of rendering, before we
					// return. However we should ensure that no one, DURING rendering, READS a
					// potentially incorrect size. This can lead to doing too much work, or
					// reading past the end of an input buffer into unmapped memory.
					buf.mDataByteSize = expectedBufferByteSize;
				}
			}
		}

		if (WantsRenderThreadID()) {
			mRenderThreadID = std::this_thread::get_id();
		}

		if (NeedsToRender(inTimeStamp)) {
			theError = ProcessBufferLists(ioActionFlags, ioData, ioData, inFramesToProcess);
		} else {
			theError = noErr;
		}

	} catch (const OSStatus& err) {
		return errorExit(err);
	} catch (...) {
		return errorExit(-1);
	}
	return theError;
}

OSStatus AUBase::DoProcessMultiple(AudioUnitRenderActionFlags& ioActionFlags,
	const AudioTimeStamp& inTimeStamp, UInt32 inFramesToProcess, UInt32 inNumberInputBufferLists,
	const AudioBufferList** inInputBufferLists, UInt32 inNumberOutputBufferLists,
	AudioBufferList** ioOutputBufferLists)
{
	const auto errorExit = [this](OSStatus error) {
		AUSDK_LogError(
			"  from %s, processmultiple err: %d", GetLoggingString(), static_cast<int>(error));
		SetRenderError(error);
		return error;
	};

	OSStatus theError = noErr;

	[[maybe_unused]] const DenormalDisabler denormalDisabler;

	try {
		if (CheckRenderArgs(ioActionFlags)) {
			AUSDK_Require(IsInitialized(), errorExit(kAudioUnitErr_Uninitialized));
			AUSDK_Require(inFramesToProcess <= mMaxFramesPerSlice,
				errorExit(kAudioUnitErr_TooManyFramesToProcess));
			AUSDK_Require(!UsesFixedBlockSize() || inFramesToProcess == GetMaxFramesPerSlice(),
				errorExit(kAudio_ParamError));

			for (unsigned ibl = 0; ibl < inNumberInputBufferLists; ++ibl) {
				if (inInputBufferLists[ibl] != nullptr) { // NOLINT
					const auto& input = Input(ibl);       // will throw if non-existant
					const unsigned expectedBufferByteSize =
						inFramesToProcess * input.GetStreamFormat().mBytesPerFrame;

					if (ASBD::NumberChannelStreams(input.GetStreamFormat()) !=
						inInputBufferLists[ibl]->mNumberBuffers) { // NOLINT
						AUSDK_LogError("inInputBufferLists[%u]->mNumberBuffers=%u, "
									   "ASBD::NumberChannelStreams(input.GetStreamFormat())=%u; "
									   "kAudio_ParamError",
							ibl, static_cast<unsigned>(inInputBufferLists[ibl]->mNumberBuffers),
							static_cast<unsigned>(
								ASBD::NumberChannelStreams(input.GetStreamFormat())));
						return errorExit(kAudio_ParamError);
					}

					for (unsigned ibuf = 0;
						 ibuf < inInputBufferLists[ibl]->mNumberBuffers; // NOLINT
						 ++ibuf) {
						const AudioBuffer& buf = inInputBufferLists[ibl]->mBuffers[ibuf]; // NOLINT
						if (buf.mData != nullptr) {
							if (buf.mDataByteSize < expectedBufferByteSize) {
								// the buffer is too small
								AUSDK_LogError(
									"%u frames, %u bytes/frame, expected %u-byte buffer; "
									"inInputBufferLists[%u].mBuffers[%u].mDataByteSize=%u; "
									"kAudio_ParamError",
									static_cast<unsigned>(inFramesToProcess),
									static_cast<unsigned>(input.GetStreamFormat().mBytesPerFrame),
									expectedBufferByteSize, ibl, ibuf,
									static_cast<unsigned>(buf.mDataByteSize));
								return errorExit(kAudio_ParamError);
							}
						} else {
							// the buffer must exist
							return errorExit(kAudio_ParamError);
						}
					}
				} else {
					// skip NULL input audio buffer list
				}
			}

			for (unsigned obl = 0; obl < inNumberOutputBufferLists; ++obl) {
				if (ioOutputBufferLists[obl] != nullptr) { // NOLINT
					const auto& output = Output(obl);      // will throw if non-existant
					const unsigned expectedBufferByteSize =
						inFramesToProcess * output.GetStreamFormat().mBytesPerFrame;

					if (ASBD::NumberChannelStreams(output.GetStreamFormat()) !=
						ioOutputBufferLists[obl]->mNumberBuffers) { // NOLINT
						AUSDK_LogError("ioOutputBufferLists[%u]->mNumberBuffers=%u, "
									   "ASBD::NumberChannelStreams(output.GetStreamFormat())=%u; "
									   "kAudio_ParamError",
							obl, static_cast<unsigned>(ioOutputBufferLists[obl]->mNumberBuffers),
							static_cast<unsigned>(
								ASBD::NumberChannelStreams(output.GetStreamFormat())));
						return errorExit(kAudio_ParamError);
					}

					for (unsigned obuf = 0;
						 obuf < ioOutputBufferLists[obl]->mNumberBuffers; // NOLINT
						 ++obuf) {
						AudioBuffer& buf = ioOutputBufferLists[obl]->mBuffers[obuf]; // NOLINT
						if (buf.mData != nullptr) {
							// only care about the size if the buffer is non-null
							if (buf.mDataByteSize < expectedBufferByteSize) {
								// if the buffer is too small, we cannot render safely.
								// kAudio_ParamError.
								AUSDK_LogError(
									"%u frames, %u bytes/frame, expected %u-byte buffer; "
									"ioOutputBufferLists[%u]->mBuffers[%u].mDataByteSize=%u; "
									"kAudio_ParamError",
									static_cast<unsigned>(inFramesToProcess),
									static_cast<unsigned>(output.GetStreamFormat().mBytesPerFrame),
									expectedBufferByteSize, obl, obuf,
									static_cast<unsigned>(buf.mDataByteSize));
								return errorExit(kAudio_ParamError);
							}
							// Some clients incorrectly pass bigger buffers than
							// expectedBufferByteSize. We will generally set the buffer size at the
							// end of rendering, before we return. However we should ensure that no
							// one, DURING rendering, READS a potentially incorrect size. This can
							// lead to doing too much work, or reading past the end of an input
							// buffer into unmapped memory.
							buf.mDataByteSize = expectedBufferByteSize;
						}
					}
				} else {
					// skip NULL output audio buffer list
				}
			}
		}

		if (WantsRenderThreadID()) {
			mRenderThreadID = std::this_thread::get_id();
		}

		if (NeedsToRender(inTimeStamp)) {
			theError = ProcessMultipleBufferLists(ioActionFlags, inFramesToProcess,
				inNumberInputBufferLists, inInputBufferLists, inNumberOutputBufferLists,
				ioOutputBufferLists);
		} else {
			theError = noErr;
		}
	} catch (const OSStatus& err) {
		return errorExit(err);
	} catch (...) {
		return errorExit(-1);
	}
	return theError;
}

//_____________________________________________________________________________
//
OSStatus AUBase::SetInputCallback(
	UInt32 inPropertyID, AudioUnitElement inElement, AURenderCallback inProc, void* inRefCon)
{
	auto& input = Input(inElement); // may throw

	input.SetInputCallback(inProc, inRefCon);
	PropertyChanged(inPropertyID, kAudioUnitScope_Input, inElement);

	return noErr;
}

//_____________________________________________________________________________
//
// NOLINTNEXTLINE(misc-no-recursion) with DispatchSetProperty
OSStatus AUBase::SetConnection(const AudioUnitConnection& inConnection)
{
	auto& input = Input(inConnection.destInputNumber); // may throw

	if (inConnection.sourceAudioUnit != nullptr) {
		// connecting, not disconnecting
		AudioStreamBasicDescription sourceDesc;
		UInt32 size = sizeof(AudioStreamBasicDescription);
		AUSDK_Require_noerr(
			AudioUnitGetProperty(inConnection.sourceAudioUnit, kAudioUnitProperty_StreamFormat,
				kAudioUnitScope_Output, inConnection.sourceOutputNumber, &sourceDesc, &size));
		AUSDK_Require_noerr(
			DispatchSetProperty(kAudioUnitProperty_StreamFormat, kAudioUnitScope_Input,
				inConnection.destInputNumber, &sourceDesc, sizeof(AudioStreamBasicDescription)));
	}
	input.SetConnection(inConnection);

	PropertyChanged(
		kAudioUnitProperty_MakeConnection, kAudioUnitScope_Input, inConnection.destInputNumber);
	return noErr;
}

//_____________________________________________________________________________
//
UInt32 AUBase::SupportedNumChannels(const AUChannelInfo** /*outInfo*/) { return 0; }

//_____________________________________________________________________________
//
bool AUBase::ValidFormat(AudioUnitScope /*inScope*/, AudioUnitElement /*inElement*/,
	const AudioStreamBasicDescription& inNewFormat)
{
	return ASBD::IsCommonFloat32(inNewFormat) &&
		   (!ASBD::IsInterleaved(inNewFormat) || inNewFormat.mChannelsPerFrame == 1);
}

//_____________________________________________________________________________
//
bool AUBase::IsStreamFormatWritable(AudioUnitScope scope, AudioUnitElement element)
{
	switch (scope) {
	case kAudioUnitScope_Input: {
		const auto& input = Input(element);
		if (input.HasConnection()) {
			return false; // can't write format when input comes from connection
		}
		[[fallthrough]];
	}
	case kAudioUnitScope_Output:
		return StreamFormatWritable(scope, element);

		//#warning "aliasing of global scope format should be pushed to subclasses"
	case kAudioUnitScope_Global:
		return StreamFormatWritable(kAudioUnitScope_Output, 0);
	default:
		break;
	}
	return false;
}

//_____________________________________________________________________________
//
AudioStreamBasicDescription AUBase::GetStreamFormat(
	AudioUnitScope inScope, AudioUnitElement inElement)
{
	//#warning "aliasing of global scope format should be pushed to subclasses"
	AUIOElement* element = nullptr;

	switch (inScope) {
	case kAudioUnitScope_Input:
		element = Inputs().GetIOElement(inElement);
		break;
	case kAudioUnitScope_Output:
		element = Outputs().GetIOElement(inElement);
		break;
	case kAudioUnitScope_Global: // global stream description is an alias for that of output 0
		element = Outputs().GetIOElement(0);
		break;
	default:
		Throw(kAudioUnitErr_InvalidScope);
	}
	return element->GetStreamFormat();
}

OSStatus AUBase::SetBusCount(AudioUnitScope inScope, UInt32 inCount)
{
	if (IsInitialized()) {
		return kAudioUnitErr_Initialized;
	}

	GetScope(inScope).SetNumberOfElements(inCount);
	return noErr;
}

//_____________________________________________________________________________
//
OSStatus AUBase::ChangeStreamFormat(AudioUnitScope inScope, AudioUnitElement inElement,
	const AudioStreamBasicDescription& inPrevFormat, const AudioStreamBasicDescription& inNewFormat)
{
	if (ASBD::IsEqual(inNewFormat, inPrevFormat)) {
		return noErr;
	}

	//#warning "aliasing of global scope format should be pushed to subclasses"
	AUIOElement* element = nullptr;

	switch (inScope) {
	case kAudioUnitScope_Input:
		element = Inputs().GetIOElement(inElement);
		break;
	case kAudioUnitScope_Output:
		element = Outputs().GetIOElement(inElement);
		break;
	case kAudioUnitScope_Global:
		element = Outputs().GetIOElement(0);
		break;
	default:
		Throw(kAudioUnitErr_InvalidScope);
	}
	element->SetStreamFormat(inNewFormat);
	PropertyChanged(kAudioUnitProperty_StreamFormat, inScope, inElement);
	return noErr;
}

std::vector<AudioChannelLayoutTag> AUBase::GetChannelLayoutTags(
	AudioUnitScope inScope, AudioUnitElement inElement)
{
	return IOElement(inScope, inElement).GetChannelLayoutTags();
}

UInt32 AUBase::GetAudioChannelLayout(AudioUnitScope scope, AudioUnitElement element,
	AudioChannelLayout* outLayoutPtr, bool& outWritable)
{
	auto& el = IOElement(scope, element);
	return el.GetAudioChannelLayout(outLayoutPtr, outWritable);
}

OSStatus AUBase::RemoveAudioChannelLayout(AudioUnitScope inScope, AudioUnitElement inElement)
{
	OSStatus result = noErr;
	auto& el = IOElement(inScope, inElement);
	bool writable = false;
	if (el.GetAudioChannelLayout(nullptr, writable) > 0) {
		result = el.RemoveAudioChannelLayout();
	}
	return result;
}

OSStatus AUBase::SetAudioChannelLayout(
	AudioUnitScope inScope, AudioUnitElement inElement, const AudioChannelLayout* inLayout)
{
	auto& ioEl = IOElement(inScope, inElement);

	// the num channels of the layout HAS TO MATCH the current channels of the Element's stream
	// format
	const UInt32 currentChannels = ioEl.GetStreamFormat().mChannelsPerFrame;
	const UInt32 numChannelsInLayout = AUChannelLayout::NumberChannels(*inLayout);
	if (currentChannels != numChannelsInLayout) {
		return kAudioUnitErr_InvalidPropertyValue;
	}

	const auto tags = GetChannelLayoutTags(inScope, inElement);
	if (tags.empty()) {
		return kAudioUnitErr_InvalidProperty;
	}
	const auto inTag = inLayout->mChannelLayoutTag;
	const auto iter = std::find_if(tags.begin(), tags.end(), [&inTag](auto& tag) {
		return tag == inTag || tag == kAudioChannelLayoutTag_UseChannelDescriptions;
	});

	if (iter == tags.end()) {
		return kAudioUnitErr_InvalidPropertyValue;
	}

	return ioEl.SetAudioChannelLayout(*inLayout);
}

constexpr int kCurrentSavedStateVersion = 0;

static void AddNumToDictionary(CFMutableDictionaryRef dict, CFStringRef key, SInt32 value)
{
	const CFNumberRef num = CFNumberCreate(nullptr, kCFNumberSInt32Type, &value);
	CFDictionarySetValue(dict, key, num);
	CFRelease(num);
}

// NOLINTNEXTLINE(misc-no-recursion) with DispatchGetProperty
OSStatus AUBase::SaveState(CFPropertyListRef* outData)
{
	const AudioComponentDescription desc = GetComponentDescription();

	auto dict = Owned<CFMutableDictionaryRef>::from_create(CFDictionaryCreateMutable(
		nullptr, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks));

	// first step -> save the version to the data ref
	SInt32 value = kCurrentSavedStateVersion;
	AddNumToDictionary(*dict, kVersionString, value);

	// second step -> save the component type, subtype, manu to the data ref
	value = static_cast<SInt32>(desc.componentType);
	AddNumToDictionary(*dict, kTypeString, value);

	value = static_cast<SInt32>(desc.componentSubType);
	AddNumToDictionary(*dict, kSubtypeString, value);

	value = static_cast<SInt32>(desc.componentManufacturer);
	AddNumToDictionary(*dict, kManufacturerString, value);

	// fourth step -> save the state of all parameters on all scopes and elements
	auto data = Owned<CFMutableDataRef>::from_create(CFDataCreateMutable(nullptr, 0));
	for (AudioUnitScope iscope = 0; iscope < 3; ++iscope) {
		const auto& scope = GetScope(iscope);
		scope.SaveState(*data);
	}

	SaveExtendedScopes(*data);

	// save all this in the data section of the dictionary
	CFDictionarySetValue(*dict, kDataString, *data);
	data = nullptr; // data can be large-ish, so destroy it now.

	// OK - now we're going to do some properties
	// save the preset name...
	CFDictionarySetValue(*dict, kNameString, mCurrentPreset.presetName);

	// Does the unit support the RenderQuality property - if so, save it...
	OSStatus result =
		DispatchGetProperty(kAudioUnitProperty_RenderQuality, kAudioUnitScope_Global, 0, &value);

	if (result == noErr) {
		AddNumToDictionary(*dict, kRenderQualityString, value);
	}

	// Do we have any element names for any of our scopes?
	// first check to see if we have any names...
	bool foundName = false;
	for (AudioUnitScope i = 0; i < kNumScopes; ++i) {
		foundName = GetScope(i).HasElementWithName();
		if (foundName) {
			break;
		}
	}
	// OK - we found a name away we go...
	if (foundName) {
		auto nameDict = Owned<CFMutableDictionaryRef>::from_create(CFDictionaryCreateMutable(
			nullptr, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks));
		for (AudioUnitScope i = 0; i < kNumScopes; ++i) {
			GetScope(i).AddElementNamesToDict(*nameDict);
		}

		CFDictionarySetValue(*dict, kElementNameString, *nameDict);
	}

	// we're done!!!
	*outData = static_cast<CFPropertyListRef>(dict.release()); // transfer ownership

	return noErr;
}

//_____________________________________________________________________________
//
// NOLINTNEXTLINE(misc-no-recursion) with DispatchSetProperty
OSStatus AUBase::RestoreState(CFPropertyListRef plist)
{
	if (CFGetTypeID(plist) != CFDictionaryGetTypeID()) {
		return kAudioUnitErr_InvalidPropertyValue;
	}

	const AudioComponentDescription desc = GetComponentDescription();

	const auto* const dict = static_cast<CFDictionaryRef>(plist);

	// zeroeth step - make sure the Part key is NOT present, as this method is used
	// to restore the GLOBAL state of the dictionary
	if (CFDictionaryContainsKey(dict, kPartString)) {
		return kAudioUnitErr_InvalidPropertyValue;
	}

	// first step -> check the saved version in the data ref
	// at this point we're only dealing with version==0
	const auto* cfnum = static_cast<CFNumberRef>(CFDictionaryGetValue(dict, kVersionString));
	AUSDK_Require(cfnum != nullptr, kAudioUnitErr_InvalidPropertyValue);
	AUSDK_Require(CFGetTypeID(cfnum) == CFNumberGetTypeID(), kAudioUnitErr_InvalidPropertyValue);
	SInt32 value = 0;
	CFNumberGetValue(cfnum, kCFNumberSInt32Type, &value);
	if (value != kCurrentSavedStateVersion) {
		return kAudioUnitErr_InvalidPropertyValue;
	}

	// second step -> check that this data belongs to this kind of audio unit
	// by checking the component subtype and manuID
	// We're not checking the type, since there may be different versions (effect, format-converter,
	// offline) of essentially the same AU
	cfnum = static_cast<CFNumberRef>(CFDictionaryGetValue(dict, kSubtypeString));
	AUSDK_Require(cfnum != nullptr, kAudioUnitErr_InvalidPropertyValue);
	AUSDK_Require(CFGetTypeID(cfnum) == CFNumberGetTypeID(), kAudioUnitErr_InvalidPropertyValue);
	CFNumberGetValue(cfnum, kCFNumberSInt32Type, &value);
	if (static_cast<UInt32>(value) != desc.componentSubType) {
		return kAudioUnitErr_InvalidPropertyValue;
	}

	cfnum = static_cast<CFNumberRef>(CFDictionaryGetValue(dict, kManufacturerString));
	AUSDK_Require(cfnum != nullptr, kAudioUnitErr_InvalidPropertyValue);
	AUSDK_Require(CFGetTypeID(cfnum) == CFNumberGetTypeID(), kAudioUnitErr_InvalidPropertyValue);
	CFNumberGetValue(cfnum, kCFNumberSInt32Type, &value);
	if (static_cast<UInt32>(value) != desc.componentManufacturer) {
		return kAudioUnitErr_InvalidPropertyValue;
	}

	// fourth step -> restore the state of all of the parameters for each scope and element
	const auto* const data = static_cast<CFDataRef>(CFDictionaryGetValue(dict, kDataString));
	if ((data != nullptr) && (CFGetTypeID(data) == CFDataGetTypeID())) {
		const UInt8* p = CFDataGetBytePtr(data);
		const UInt8* const pend = p + CFDataGetLength(data); // NOLINT

		// we have a zero length data, which may just mean there were no parameters to save!
		//	if (p >= pend) return noErr;

		while (p < pend) {
			const UInt32 scopeIdx =
				CFSwapInt32BigToHost(*reinterpret_cast<const UInt32*>(p)); // NOLINT
			p += sizeof(UInt32);                                           // NOLINT

			const auto& scope = GetScope(scopeIdx);
			p = scope.RestoreState(p);
		}
	}

	// OK - now we're going to do some properties
	// restore the preset name...
	const auto* const name = static_cast<CFStringRef>(CFDictionaryGetValue(dict, kNameString));
	if (mCurrentPreset.presetName != nullptr) {
		CFRelease(mCurrentPreset.presetName);
	}
	if ((name != nullptr) && (CFGetTypeID(name) == CFStringGetTypeID())) {
		mCurrentPreset.presetName = name;
		mCurrentPreset.presetNumber = -1;
	} else { // no name entry make the default one
		mCurrentPreset.presetName = kUntitledString;
		mCurrentPreset.presetNumber = -1;
	}

	CFRetain(mCurrentPreset.presetName);
	PropertyChanged(kAudioUnitProperty_PresentPreset, kAudioUnitScope_Global, 0);

	// Does the dict contain render quality information?
	cfnum = static_cast<CFNumberRef>(CFDictionaryGetValue(dict, kRenderQualityString));
	if (cfnum && (CFGetTypeID(cfnum) == CFNumberGetTypeID())) {
		CFNumberGetValue(cfnum, kCFNumberSInt32Type, &value);
		DispatchSetProperty(
			kAudioUnitProperty_RenderQuality, kAudioUnitScope_Global, 0, &value, sizeof(value));
	}

	// Do we have any element names for any of our scopes?
	const auto nameDict =
		static_cast<CFDictionaryRef>(CFDictionaryGetValue(dict, kElementNameString));
	if (nameDict && (CFGetTypeID(nameDict) == CFDictionaryGetTypeID())) {
		for (AudioUnitScope i = 0; i < kNumScopes; ++i) {
			const CFStringRef key = CFStringCreateWithFormat(
				nullptr, nullptr, CFSTR("%u"), static_cast<unsigned>(i)); // NOLINT
			const auto elementDict =
				static_cast<CFDictionaryRef>(CFDictionaryGetValue(nameDict, key));
			if (elementDict && (CFGetTypeID(elementDict) == CFDictionaryGetTypeID())) {
				const auto restoredElements = GetScope(i).RestoreElementNames(elementDict);
				for (const auto& element : restoredElements) {
					PropertyChanged(kAudioUnitProperty_ElementName, i, element);
				}
			}
			CFRelease(key);
		}
	}

	return noErr;
}

OSStatus AUBase::GetPresets(CFArrayRef* /*outData*/) const { return kAudioUnitErr_InvalidProperty; }

OSStatus AUBase::NewFactoryPresetSet(const AUPreset& /*inNewFactoryPreset*/)
{
	return kAudioUnitErr_InvalidProperty;
}

OSStatus AUBase::NewCustomPresetSet(const AUPreset& inNewCustomPreset)
{
	CFRelease(mCurrentPreset.presetName);
	mCurrentPreset = inNewCustomPreset;
	CFRetain(mCurrentPreset.presetName);
	return noErr;
}

// set the default preset for the unit -> the number of the preset MUST be >= 0
// and the name should be valid, or the preset WON'T take
bool AUBase::SetAFactoryPresetAsCurrent(const AUPreset& inPreset)
{
	if (inPreset.presetNumber < 0 || inPreset.presetName == nullptr) {
		return false;
	}
	CFRelease(mCurrentPreset.presetName);
	mCurrentPreset = inPreset;
	CFRetain(mCurrentPreset.presetName);
	return true;
}

bool AUBase::HasIcon()
{
	const CFURLRef url = CopyIconLocation();
	if (url != nullptr) {
		CFRelease(url);
		return true;
	}
	return false;
}

CFURLRef AUBase::CopyIconLocation() { return nullptr; }

//_____________________________________________________________________________
//
OSStatus AUBase::GetParameterList(
	AudioUnitScope inScope, AudioUnitParameterID* outParameterList, UInt32& outNumParameters)
{
	const auto& scope = GetScope(inScope);
	AUElement* elementWithMostParameters = nullptr;
	UInt32 maxNumParams = 0;

	const UInt32 nElems = scope.GetNumberOfElements();
	for (UInt32 ielem = 0; ielem < nElems; ++ielem) {
		AUElement* const element = scope.GetElement(ielem);
		const UInt32 nParams = element->GetNumberOfParameters();
		if (nParams > maxNumParams) {
			maxNumParams = nParams;
			elementWithMostParameters = element;
		}
	}

	if (outParameterList != nullptr && elementWithMostParameters != nullptr) {
		elementWithMostParameters->GetParameterList(outParameterList);
	}

	outNumParameters = maxNumParams;
	return noErr;
}

//_____________________________________________________________________________
//
OSStatus AUBase::GetParameterInfo(AudioUnitScope /*inScope*/,
	AudioUnitParameterID /*inParameterID*/, AudioUnitParameterInfo& /*outParameterInfo*/)
{
	return kAudioUnitErr_InvalidParameter;
}

//_____________________________________________________________________________
//
OSStatus AUBase::GetParameterValueStrings(
	AudioUnitScope /*inScope*/, AudioUnitParameterID /*inParameterID*/, CFArrayRef* /*outStrings*/)
{
	return kAudioUnitErr_InvalidProperty;
}

//_____________________________________________________________________________
//
OSStatus AUBase::GetParameterHistoryInfo(AudioUnitScope /*inScope*/,
	AudioUnitParameterID /*inParameterID*/, Float32& /*outUpdatesPerSecond*/,
	Float32& /*outHistoryDurationInSeconds*/)
{
	return kAudioUnitErr_InvalidProperty;
}


//_____________________________________________________________________________
//
OSStatus AUBase::CopyClumpName(AudioUnitScope /*inScope*/, UInt32 /*inClumpID*/,
	UInt32 /*inDesiredNameLength*/, CFStringRef* /*outClumpName*/)
{
	return kAudioUnitErr_InvalidProperty;
}

//_____________________________________________________________________________
//
void AUBase::SetNumberOfElements(AudioUnitScope inScope, UInt32 numElements)
{
	if (inScope == kAudioUnitScope_Global && numElements != 1) {
		Throw(kAudioUnitErr_InvalidScope);
	}

	GetScope(inScope).SetNumberOfElements(numElements);
}

//_____________________________________________________________________________
//
std::unique_ptr<AUElement> AUBase::CreateElement(AudioUnitScope scope, AudioUnitElement /*element*/)
{
	switch (scope) {
	case kAudioUnitScope_Global:
		return std::make_unique<AUElement>(*this);
	case kAudioUnitScope_Input:
		return std::make_unique<AUInputElement>(*this);
	case kAudioUnitScope_Output:
		return std::make_unique<AUOutputElement>(*this);
	case kAudioUnitScope_Group:
	case kAudioUnitScope_Part:
		return std::make_unique<AUElement>(*this);
	default:
		break;
	}
	Throw(kAudioUnitErr_InvalidScope);
}

const char* AUBase::GetLoggingString() const noexcept { return mLogString.c_str(); }

std::string AUBase::CreateLoggingString() const
{
	const auto desc = GetComponentDescription();
	std::array<char, 32> buf{};
	[[maybe_unused]] const int printCount =
		snprintf(buf.data(), buf.size(), "AU (%p): ", GetComponentInstance()); // NOLINT
#if DEBUG
	assert(printCount < static_cast<int>(buf.size()));
#endif
	return buf.data() + make_string_from_4cc(desc.componentType) + '/' +
		   make_string_from_4cc(desc.componentSubType) + '/' +
		   make_string_from_4cc(desc.componentManufacturer);
}

} // namespace ausdk

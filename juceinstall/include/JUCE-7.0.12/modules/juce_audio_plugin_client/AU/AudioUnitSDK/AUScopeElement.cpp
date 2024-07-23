/*!
	@file		AudioUnitSDK/AUScopeElement.cpp
	@copyright	© 2000-2021 Apple Inc. All rights reserved.
*/
#include <AudioUnitSDK/AUBase.h>
#include <AudioUnitSDK/AUScopeElement.h>

#include <AudioToolbox/AudioUnitProperties.h>

#include <array>

namespace ausdk {

//_____________________________________________________________________________
//
//	By default, parameterIDs may be arbitrarily spaced, and a flat map
//  will be used for access.  Calling UseIndexedParameters() will
//	instead use an STL vector for faster indexed access.
//	This assumes the paramIDs are numbered 0.....inNumberOfParameters-1
//	Call this before defining/adding any parameters with SetParameter()
//
void AUElement::UseIndexedParameters(UInt32 inNumberOfParameters)
{
	mIndexedParameters.resize(inNumberOfParameters);
	mUseIndexedParameters = true;
}

//_____________________________________________________________________________
//
//	Helper method.
//	returns whether the specified paramID is known to the element
//
bool AUElement::HasParameterID(AudioUnitParameterID paramID) const
{
	if (mUseIndexedParameters) {
		return paramID < mIndexedParameters.size();
	}

	return mParameters.find(paramID) != mParameters.end();
}

//_____________________________________________________________________________
//
//	caller assumes that this is actually an immediate parameter
//
AudioUnitParameterValue AUElement::GetParameter(AudioUnitParameterID paramID) const
{
	if (mUseIndexedParameters) {
		ausdk::ThrowExceptionIf(
			paramID >= mIndexedParameters.size(), kAudioUnitErr_InvalidParameter);
		return mIndexedParameters[paramID].load(std::memory_order_acquire);
	}
	const auto i = mParameters.find(paramID);
	ausdk::ThrowExceptionIf(i == mParameters.end(), kAudioUnitErr_InvalidParameter);
	return (*i).second.load(std::memory_order_acquire);
}

//_____________________________________________________________________________
//
void AUElement::SetParameter(
	AudioUnitParameterID paramID, AudioUnitParameterValue inValue, bool okWhenInitialized)
{
	if (mUseIndexedParameters) {
		ausdk::ThrowExceptionIf(
			paramID >= mIndexedParameters.size(), kAudioUnitErr_InvalidParameter);
		mIndexedParameters[paramID].store(inValue, std::memory_order_release);
	} else {
		const auto i = mParameters.find(paramID);

		if (i == mParameters.end()) {
			if (mAudioUnit.IsInitialized() && !okWhenInitialized) {
				// The AU should not be creating new parameters once initialized.
				// If a client tries to set an undefined parameter, we could throw as follows,
				// but this might cause a regression. So it is better to just fail silently.
				// Throw(kAudioUnitErr_InvalidParameter);
				AUSDK_LogError(
					"Warning: %s SetParameter for undefined param ID %u while initialized. "
					"Ignoring.",
					mAudioUnit.GetLoggingString(), static_cast<unsigned>(paramID));
			} else {
				// create new entry in map for the paramID (only happens first time)
				mParameters[paramID] = ParameterValue{ inValue };
			}
		} else {
			// paramID already exists in map so simply change its value
			(*i).second.store(inValue, std::memory_order_release);
		}
	}
}

//_____________________________________________________________________________
//
void AUElement::SetScheduledEvent(AudioUnitParameterID paramID,
	const AudioUnitParameterEvent& inEvent, UInt32 /*inSliceOffsetInBuffer*/,
	UInt32 /*inSliceDurationFrames*/, bool okWhenInitialized)
{
	if (inEvent.eventType != kParameterEvent_Immediate) {
		AUSDK_LogError("Warning: %s was passed a ramped parameter event but does not implement "
					   "them. Ignoring.",
			mAudioUnit.GetLoggingString());
		return;
	}
	SetParameter(paramID, inEvent.eventValues.immediate.value, okWhenInitialized); // NOLINT
}

//_____________________________________________________________________________
//
void AUElement::GetParameterList(AudioUnitParameterID* outList)
{
	if (mUseIndexedParameters) {
		const auto nparams = static_cast<UInt32>(mIndexedParameters.size());
		for (UInt32 i = 0; i < nparams; i++) {
			*outList++ = (AudioUnitParameterID)i; // NOLINT
		}
	} else {
		for (const auto& param : mParameters) {
			*outList++ = param.first; // NOLINT
		}
	}
}

//_____________________________________________________________________________
//
void AUElement::SaveState(AudioUnitScope scope, CFMutableDataRef data)
{
	AudioUnitParameterInfo paramInfo{};
	const CFIndex countOffset = CFDataGetLength(data);
	uint32_t paramsWritten = 0;

	const auto appendBytes = [data](const void* bytes, CFIndex length) {
		CFDataAppendBytes(data, static_cast<const UInt8*>(bytes), length);
	};

	const auto appendParameter = [&](AudioUnitParameterID paramID, AudioUnitParameterValue value) {
		struct {
			UInt32 paramID;
			UInt32 value; // really a big-endian float
		} entry{};
		static_assert(sizeof(entry) == (sizeof(entry.paramID) + sizeof(entry.value)));

		if (mAudioUnit.GetParameterInfo(scope, paramID, paramInfo) == noErr) {
			if ((paramInfo.flags & kAudioUnitParameterFlag_CFNameRelease) != 0u) {
				if (paramInfo.cfNameString != nullptr) {
					CFRelease(paramInfo.cfNameString);
				}
				if (paramInfo.unit == kAudioUnitParameterUnit_CustomUnit &&
					paramInfo.unitName != nullptr) {
					CFRelease(paramInfo.unitName);
				}
			}
			if (((paramInfo.flags & kAudioUnitParameterFlag_OmitFromPresets) != 0u) ||
				((paramInfo.flags & kAudioUnitParameterFlag_MeterReadOnly) != 0u)) {
				return;
			}
		}

		entry.paramID = CFSwapInt32HostToBig(paramID);
		entry.value = CFSwapInt32HostToBig(*reinterpret_cast<UInt32*>(&value)); // NOLINT

		appendBytes(&entry, sizeof(entry));
		++paramsWritten;
	};

	constexpr UInt32 placeholderCount = 0;
	appendBytes(&placeholderCount, sizeof(placeholderCount));

	if (mUseIndexedParameters) {
		const auto nparams = static_cast<UInt32>(mIndexedParameters.size());
		for (UInt32 i = 0; i < nparams; i++) {
			appendParameter(i, mIndexedParameters[i]);
		}
	} else {
		for (const auto& item : mParameters) {
			appendParameter(item.first, item.second);
		}
	}

	const auto count_BE = CFSwapInt32HostToBig(paramsWritten);
	memcpy(CFDataGetMutableBytePtr(data) + countOffset, // NOLINT ptr math
		&count_BE, sizeof(count_BE));
}

//_____________________________________________________________________________
//
const UInt8* AUElement::RestoreState(const UInt8* state)
{
	union FloatInt32 {
		UInt32 i;
		AudioUnitParameterValue f;
	};
	const UInt8* p = state;
	const UInt32 nparams = CFSwapInt32BigToHost(*reinterpret_cast<const UInt32*>(p)); // NOLINT
	p += sizeof(UInt32);                                                              // NOLINT

	for (UInt32 i = 0; i < nparams; ++i) {
		struct {
			AudioUnitParameterID paramID;
			AudioUnitParameterValue value;
		} entry{};
		static_assert(sizeof(entry) == (sizeof(entry.paramID) + sizeof(entry.value)));

		entry.paramID = CFSwapInt32BigToHost(*reinterpret_cast<const UInt32*>(p)); // NOLINT
		p += sizeof(UInt32);                                                       // NOLINT
		FloatInt32 temp{};                                                         // NOLINT
		temp.i = CFSwapInt32BigToHost(*reinterpret_cast<const UInt32*>(p));        // NOLINT
		entry.value = temp.f;                                                      // NOLINT
		p += sizeof(AudioUnitParameterValue);                                      // NOLINT

		SetParameter(entry.paramID, entry.value);
	}
	return p;
}

//_____________________________________________________________________________
//
AUIOElement::AUIOElement(AUBase& audioUnit) : AUElement(audioUnit), mWillAllocate(true)
{
	mStreamFormat = AudioStreamBasicDescription{ .mSampleRate = AUBase::kAUDefaultSampleRate,
		.mFormatID = kAudioFormatLinearPCM,
		.mFormatFlags = AudioFormatFlags(kAudioFormatFlagsNativeFloatPacked) |
						AudioFormatFlags(kAudioFormatFlagIsNonInterleaved), // NOLINT
		.mBytesPerPacket = sizeof(float),
		.mFramesPerPacket = 1,
		.mBytesPerFrame = sizeof(float),
		.mChannelsPerFrame = 2,
		.mBitsPerChannel = 32, // NOLINT
		.mReserved = 0 };
}

//_____________________________________________________________________________
//
OSStatus AUIOElement::SetStreamFormat(const AudioStreamBasicDescription& format)
{
	mStreamFormat = format;

	// Clear the previous channel layout if it is inconsistent with the newly set format;
	// preserve it if it is acceptable, in case the new format has no layout.
	if (ChannelLayout().IsValid() && NumberChannels() != ChannelLayout().NumberChannels()) {
		RemoveAudioChannelLayout();
	}

	return noErr;
}

//_____________________________________________________________________________
// inFramesToAllocate == 0 implies the AudioUnit's max-frames-per-slice will be used
void AUIOElement::AllocateBuffer(UInt32 inFramesToAllocate)
{
	if (GetAudioUnit().HasBegunInitializing()) {
		UInt32 framesToAllocate =
			inFramesToAllocate > 0 ? inFramesToAllocate : GetAudioUnit().GetMaxFramesPerSlice();

		mIOBuffer.Allocate(
			mStreamFormat, (mWillAllocate && NeedsBufferSpace()) ? framesToAllocate : 0);
	}
}

//_____________________________________________________________________________
//
void AUIOElement::DeallocateBuffer() { mIOBuffer.Deallocate(); }

//_____________________________________________________________________________
//
//		AudioChannelLayout support

// return an empty vector (ie. NO channel layouts) if the AU doesn't require channel layout
// knowledge
std::vector<AudioChannelLayoutTag> AUIOElement::GetChannelLayoutTags() { return {}; }

// outLayoutPtr WILL be NULL if called to determine layout size
UInt32 AUIOElement::GetAudioChannelLayout(AudioChannelLayout* outLayoutPtr, bool& outWritable)
{
	outWritable = true;

	UInt32 size = mChannelLayout.IsValid() ? mChannelLayout.Size() : 0;
	if (size > 0 && outLayoutPtr != nullptr) {
		memcpy(outLayoutPtr, &mChannelLayout.Layout(), size);
	}

	return size;
}

// the incoming channel map will be at least as big as a basic AudioChannelLayout
// but its contents will determine its actual size
// Subclass should overide if channel map is writable
OSStatus AUIOElement::SetAudioChannelLayout(const AudioChannelLayout& inLayout)
{
	if (NumberChannels() != AUChannelLayout::NumberChannels(inLayout)) {
		return kAudioUnitErr_InvalidPropertyValue;
	}
	mChannelLayout = inLayout;
	return noErr;
}

// Some units support optional usage of channel maps - typically converter units
// that can do channel remapping between different maps. In that optional case
// the user should be able to remove a channel map if that is possible.
// Typically this is NOT the case (e.g., the 3DMixer even in the stereo case
// needs to know if it is rendering to speakers or headphones)
OSStatus AUIOElement::RemoveAudioChannelLayout()
{
	mChannelLayout = {};
	return noErr;
}

//_____________________________________________________________________________
//
void AUScope::SetNumberOfElements(UInt32 numElements)
{
	if (mDelegate != nullptr) {
		return mDelegate->SetNumberOfElements(numElements);
	}

	if (numElements > mElements.size()) {
		mElements.reserve(numElements);
		while (numElements > mElements.size()) {
			auto elem = mCreator->CreateElement(GetScope(), static_cast<UInt32>(mElements.size()));
			mElements.push_back(std::move(elem));
		}
	} else {
		while (numElements < mElements.size()) {
			mElements.pop_back();
		}
	}
}

//_____________________________________________________________________________
//
bool AUScope::HasElementWithName() const
{
	for (UInt32 i = 0; i < GetNumberOfElements(); ++i) {
		AUElement* const el = GetElement(i);
		if ((el != nullptr) && el->HasName()) {
			return true;
		}
	}
	return false;
}

//_____________________________________________________________________________
//

void AUScope::AddElementNamesToDict(CFMutableDictionaryRef inNameDict) const
{
	if (HasElementWithName()) {
		const auto elementDict =
			Owned<CFMutableDictionaryRef>::from_create(CFDictionaryCreateMutable(
				nullptr, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks));
		for (UInt32 i = 0; i < GetNumberOfElements(); ++i) {
			AUElement* const el = GetElement(i);
			if (el != nullptr && el->HasName()) {
				const auto key = Owned<CFStringRef>::from_create(CFStringCreateWithFormat(
					nullptr, nullptr, CFSTR("%u"), static_cast<unsigned>(i)));
				CFDictionarySetValue(*elementDict, *key, *el->GetName());
			}
		}

		const auto key = Owned<CFStringRef>::from_create(
			CFStringCreateWithFormat(nullptr, nullptr, CFSTR("%u"), static_cast<unsigned>(mScope)));
		CFDictionarySetValue(inNameDict, *key, *elementDict);
	}
}

//_____________________________________________________________________________
//
std::vector<AudioUnitElement> AUScope::RestoreElementNames(CFDictionaryRef inNameDict) const
{
	// first we have to see if we have enough elements
	std::vector<AudioUnitElement> restoredElements;
	const auto maxElNum = GetNumberOfElements();

	const auto dictSize =
		static_cast<size_t>(std::max(CFDictionaryGetCount(inNameDict), CFIndex(0)));
	std::vector<CFStringRef> keys(dictSize);
	CFDictionaryGetKeysAndValues(
		inNameDict, reinterpret_cast<const void**>(keys.data()), nullptr); // NOLINT
	for (size_t i = 0; i < dictSize; i++) {
		unsigned int intKey = 0;
		std::array<char, 32> string{};
		CFStringGetCString(keys[i], string.data(), string.size(), kCFStringEncodingASCII);
		const int result = sscanf(string.data(), "%u", &intKey); // NOLINT
		// check if sscanf succeeded and element index is less than max elements.
		if ((result != 0) && (static_cast<UInt32>(intKey) < maxElNum)) {
			auto* const elName =
				static_cast<CFStringRef>(CFDictionaryGetValue(inNameDict, keys[i]));
			if ((elName != nullptr) && (CFGetTypeID(elName) == CFStringGetTypeID())) {
				AUElement* const element = GetElement(intKey);
				if (element != nullptr) {
					auto* const currentName = element->GetName().get();

					if (currentName == nullptr || CFStringCompare(elName, currentName, 0) != kCFCompareEqualTo) {
						element->SetName(elName);
						restoredElements.push_back(intKey);
					}
				}
			}
		}
	}

	return restoredElements;
}

void AUScope::SaveState(CFMutableDataRef data) const
{
	const AudioUnitElement nElems = GetNumberOfElements();
	for (AudioUnitElement ielem = 0; ielem < nElems; ++ielem) {
		AUElement* const element = GetElement(ielem);
		const UInt32 nparams = element->GetNumberOfParameters();
		if (nparams > 0) {
			struct {
				const UInt32 scope;
				const UInt32 element;
			} hdr{ .scope = CFSwapInt32HostToBig(GetScope()),
				.element = CFSwapInt32HostToBig(ielem) };
			static_assert(sizeof(hdr) == (sizeof(hdr.scope) + sizeof(hdr.element)));
			CFDataAppendBytes(data, reinterpret_cast<const UInt8*>(&hdr), sizeof(hdr)); // NOLINT

			element->SaveState(mScope, data);
		}
	}
}

const UInt8* AUScope::RestoreState(const UInt8* state) const
{
	const UInt8* p = state;
	const UInt32 elementIdx = CFSwapInt32BigToHost(*reinterpret_cast<const UInt32*>(p)); // NOLINT
	p += sizeof(UInt32);                                                                 // NOLINT
	AUElement* const element = GetElement(elementIdx);
	if (element == nullptr) {
		struct {
			AudioUnitParameterID paramID;
			AudioUnitParameterValue value;
		} entry{};
		static_assert(sizeof(entry) == (sizeof(entry.paramID) + sizeof(entry.value)));
		const UInt32 nparams = CFSwapInt32BigToHost(*reinterpret_cast<const UInt32*>(p)); // NOLINT
		p += sizeof(UInt32);                                                              // NOLINT

		p += nparams * sizeof(entry); // NOLINT
	} else {
		p = element->RestoreState(p);
	}

	return p;
}

} // namespace ausdk

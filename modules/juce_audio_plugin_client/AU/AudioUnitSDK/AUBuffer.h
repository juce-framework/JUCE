/*!
	@file		AudioUnitSDK/AUBuffer.h
	@copyright	© 2000-2021 Apple Inc. All rights reserved.
*/
#ifndef AudioUnitSDK_AUBuffer_h
#define AudioUnitSDK_AUBuffer_h

#include <AudioUnitSDK/AUUtility.h>

#include <AudioToolbox/AudioUnit.h>

#include <cstddef>
#include <optional>

namespace ausdk {

/// struct created/destroyed by allocator. Do not attempt to manually create/destroy.
struct AllocatedBuffer {
	const UInt32 mMaximumNumberBuffers;
	const UInt32 mMaximumBytesPerBuffer;
	const UInt32 mReservedA[2]; // NOLINT C-style array
	const UInt32 mHeaderSize;
	const UInt32 mBufferDataSize;
	const UInt32 mReservedB[2]; // NOLINT C-style array
	void* const mBufferData;
	void* const mReservedC;

	AudioBufferList mAudioBufferList;
	// opaque variable-length data may follow the AudioBufferList

	AudioBufferList& Prepare(UInt32 channelsPerBuffer, UInt32 bytesPerBuffer);
	AudioBufferList& PrepareNull(UInt32 channelsPerBuffer, UInt32 bytesPerBuffer);
};

/*!
	@class	BufferAllocator
	@brief	Class which allocates memory for internal audio buffers.

	To customize, create a subclass and install an instance into the global via set_instance().
*/
class BufferAllocator {
public:
	/// Obtain the global instance, creating it if necessary.
	static BufferAllocator& instance();

	/// A client may install a custom global instance via this method. Throws an exception if
	/// a default instance has already been created.
	static void set_instance(BufferAllocator& instance);

	BufferAllocator() = default;
	virtual ~BufferAllocator() = default;

	// Rule of 5
	BufferAllocator(const BufferAllocator&) = delete;
	BufferAllocator(BufferAllocator&&) = delete;
	BufferAllocator& operator=(const BufferAllocator&) = delete;
	BufferAllocator& operator=(BufferAllocator&&) = delete;

	// N.B. Must return zeroed memory aligned to at least 16 bytes.
	virtual AllocatedBuffer* Allocate(
		UInt32 numberBuffers, UInt32 maxBytesPerBuffer, UInt32 reservedFlags);
	virtual void Deallocate(AllocatedBuffer* allocatedBuffer);
};

/*!
	@class	AUBufferList
	@brief	Manages an `AudioBufferList` backed by allocated memory buffers.
*/
class AUBufferList {
	enum class EPtrState { Invalid, ToMyMemory, ToExternalMemory };

public:
	AUBufferList() = default;
	~AUBufferList() { Deallocate(); }

	AUBufferList(const AUBufferList&) = delete;
	AUBufferList(AUBufferList&&) = delete;
	AUBufferList& operator=(const AUBufferList&) = delete;
	AUBufferList& operator=(AUBufferList&&) = delete;

	AudioBufferList& PrepareBuffer(const AudioStreamBasicDescription& format, UInt32 nFrames);
	AudioBufferList& PrepareNullBuffer(const AudioStreamBasicDescription& format, UInt32 nFrames);

	AudioBufferList& SetBufferList(const AudioBufferList& abl)
	{
		ausdk::ThrowExceptionIf(mAllocatedStreams < abl.mNumberBuffers, -1);
		mPtrState = EPtrState::ToExternalMemory;
		auto& myabl = mBuffers->mAudioBufferList;
		memcpy(&myabl, &abl,
			static_cast<size_t>(
				reinterpret_cast<const std::byte*>(&abl.mBuffers[abl.mNumberBuffers]) - // NOLINT
				reinterpret_cast<const std::byte*>(&abl)));                             // NOLINT
		return myabl;
	}

	void SetBuffer(UInt32 index, const AudioBuffer& ab)
	{
		auto& myabl = mBuffers->mAudioBufferList;
		ausdk::ThrowExceptionIf(
			mPtrState == EPtrState::Invalid || index >= myabl.mNumberBuffers, -1);
		mPtrState = EPtrState::ToExternalMemory;
		myabl.mBuffers[index] = ab; // NOLINT
	}

	void InvalidateBufferList() noexcept { mPtrState = EPtrState::Invalid; }

	[[nodiscard]] AudioBufferList& GetBufferList() const
	{
		ausdk::ThrowExceptionIf(mPtrState == EPtrState::Invalid, -1);
		return mBuffers->mAudioBufferList;
	}

	void CopyBufferListTo(AudioBufferList& abl) const
	{
		ausdk::ThrowExceptionIf(mPtrState == EPtrState::Invalid, -1);
		memcpy(&abl, &mBuffers->mAudioBufferList,
			static_cast<size_t>(
				reinterpret_cast<std::byte*>(&abl.mBuffers[abl.mNumberBuffers]) - // NOLINT
				reinterpret_cast<std::byte*>(&abl)));                             // NOLINT
	}

	void CopyBufferContentsTo(AudioBufferList& destabl) const
	{
		ausdk::ThrowExceptionIf(mPtrState == EPtrState::Invalid, -1);
		const auto& srcabl = mBuffers->mAudioBufferList;
		const AudioBuffer* srcbuf = srcabl.mBuffers; // NOLINT
		AudioBuffer* destbuf = destabl.mBuffers;     // NOLINT

		for (UInt32 i = 0; i < destabl.mNumberBuffers; ++i, ++srcbuf, ++destbuf) { // NOLINT
			if (i >=
				srcabl.mNumberBuffers) { // duplicate last source to additional outputs [4341137]
				--srcbuf;                // NOLINT
			}
			if (destbuf->mData != srcbuf->mData) {
				memmove(destbuf->mData, srcbuf->mData, srcbuf->mDataByteSize);
			}
			destbuf->mDataByteSize = srcbuf->mDataByteSize;
		}
	}

	void Allocate(const AudioStreamBasicDescription& format, UInt32 nFrames);

	void Deallocate();

	// AudioBufferList utilities
	static void ZeroBuffer(AudioBufferList& abl)
	{
		AudioBuffer* buf = abl.mBuffers;                         // NOLINT
		for (UInt32 i = 0; i < abl.mNumberBuffers; ++i, ++buf) { // NOLINT
			memset(buf->mData, 0, buf->mDataByteSize);
		}
	}

	[[nodiscard]] UInt32 GetAllocatedFrames() const noexcept { return mAllocatedFrames; }

private:
	EPtrState mPtrState{ EPtrState::Invalid };
	AllocatedBuffer* mBuffers = nullptr; // only valid between Allocate and Deallocate

	UInt32 mAllocatedStreams{ 0 };
	UInt32 mAllocatedFrames{ 0 };
};

} // namespace ausdk

#endif // AudioUnitSDK_AUBuffer_h

/*!
	@file		AudioUnitSDK/AUBuffer.cpp
	@copyright	© 2000-2021 Apple Inc. All rights reserved.
*/
#include <AudioUnitSDK/AUBuffer.h>
#include <AudioUnitSDK/AUUtility.h>

namespace ausdk {

inline void ThrowBadAlloc()
{
	AUSDK_LogError("AUBuffer throwing bad_alloc");
	throw std::bad_alloc();
}

// x: number to be rounded; y: the power of 2 to which to round
constexpr uint32_t RoundUpToMultipleOfPowerOf2(uint32_t x, uint32_t y) noexcept
{
	const auto mask = y - 1;
#if DEBUG
	assert((mask & y) == 0u); // verifies that y is a power of 2 NOLINT
#endif
	return (x + mask) & ~mask;
}

// a * b + c
static UInt32 SafeMultiplyAddUInt32(UInt32 a, UInt32 b, UInt32 c)
{
	if (a == 0 || b == 0) {
		return c; // prevent zero divide
	}

	if (a > (0xFFFFFFFF - c) / b) { // NOLINT magic
		ThrowBadAlloc();
	}

	return a * b + c;
}

AllocatedBuffer* BufferAllocator::Allocate(
	UInt32 numberBuffers, UInt32 maxBytesPerBuffer, UInt32 /*reservedFlags*/)
{
	constexpr size_t kAlignment = 16;
	constexpr size_t kMaxBufferListSize = 65536;

	// Check for a reasonable number of buffers (obviate a more complicated check with offsetof).
	if (numberBuffers > kMaxBufferListSize / sizeof(AudioBuffer)) {
		throw std::out_of_range("AudioBuffers::Allocate: Too many buffers");
	}

	maxBytesPerBuffer = RoundUpToMultipleOfPowerOf2(maxBytesPerBuffer, kAlignment);

	const auto bufferDataSize = SafeMultiplyAddUInt32(numberBuffers, maxBytesPerBuffer, 0);
	void* bufferData = nullptr;
	if (bufferDataSize > 0) {
		bufferData = malloc(bufferDataSize);
		// don't use calloc(); it might not actually touch the memory and cause a VM fault later
		memset(bufferData, 0, bufferDataSize);
	}

	const auto implSize = static_cast<uint32_t>(
		offsetof(AllocatedBuffer, mAudioBufferList.mBuffers[std::max(UInt32(1), numberBuffers)]));
	auto* const implMem = malloc(implSize);
	auto* const allocatedBuffer =
		new (implMem) AllocatedBuffer{ .mMaximumNumberBuffers = numberBuffers,
			.mMaximumBytesPerBuffer = maxBytesPerBuffer,
			.mHeaderSize = implSize,
			.mBufferDataSize = bufferDataSize,
			.mBufferData = bufferData };
	allocatedBuffer->mAudioBufferList.mNumberBuffers = numberBuffers;
	return allocatedBuffer;
}

void BufferAllocator::Deallocate(AllocatedBuffer* allocatedBuffer)
{
	if (allocatedBuffer->mBufferData != nullptr) {
		free(allocatedBuffer->mBufferData);
	}
	allocatedBuffer->~AllocatedBuffer();
	free(allocatedBuffer);
}


AudioBufferList& AllocatedBuffer::Prepare(UInt32 channelsPerBuffer, UInt32 bytesPerBuffer)
{
	if (mAudioBufferList.mNumberBuffers > mMaximumNumberBuffers) {
		throw std::out_of_range("AllocatedBuffer::Prepare(): too many buffers");
	}
	if (bytesPerBuffer > mMaximumBytesPerBuffer) {
		throw std::out_of_range("AllocatedBuffer::Prepare(): insufficient capacity");
	}

	auto* ptr = static_cast<Byte*>(mBufferData);
	auto* const ptrend = ptr + mBufferDataSize;

	for (UInt32 bufIdx = 0, nBufs = mAudioBufferList.mNumberBuffers; bufIdx < nBufs; ++bufIdx) {
		auto& buf = mAudioBufferList.mBuffers[bufIdx]; // NOLINT
		buf.mNumberChannels = channelsPerBuffer;
		buf.mDataByteSize = bytesPerBuffer;
		buf.mData = ptr;
		ptr += mMaximumBytesPerBuffer; // NOLINT ptr math
	}
	if (ptr > ptrend) {
		throw std::out_of_range("AllocatedBuffer::Prepare(): insufficient capacity");
	}
	return mAudioBufferList;
}

AudioBufferList& AllocatedBuffer::PrepareNull(UInt32 channelsPerBuffer, UInt32 bytesPerBuffer)
{
	if (mAudioBufferList.mNumberBuffers > mMaximumNumberBuffers) {
		throw std::out_of_range("AllocatedBuffer::PrepareNull(): too many buffers");
	}
	for (UInt32 bufIdx = 0, nBufs = mAudioBufferList.mNumberBuffers; bufIdx < nBufs; ++bufIdx) {
		auto& buf = mAudioBufferList.mBuffers[bufIdx]; // NOLINT
		buf.mNumberChannels = channelsPerBuffer;
		buf.mDataByteSize = bytesPerBuffer;
		buf.mData = nullptr;
	}
	return mAudioBufferList;
}

AudioBufferList& AUBufferList::PrepareBuffer(
	const AudioStreamBasicDescription& format, UInt32 nFrames)
{
	ausdk::ThrowExceptionIf(nFrames > mAllocatedFrames, kAudioUnitErr_TooManyFramesToProcess);

	UInt32 nStreams = 0;
	UInt32 channelsPerStream = 0;
	if (ASBD::IsInterleaved(format)) {
		nStreams = 1;
		channelsPerStream = format.mChannelsPerFrame;
	} else {
		nStreams = format.mChannelsPerFrame;
		channelsPerStream = 1;
	}

	ausdk::ThrowExceptionIf(nStreams > mAllocatedStreams, kAudioUnitErr_FormatNotSupported);
	auto& abl = mBuffers->Prepare(channelsPerStream, nFrames * format.mBytesPerFrame);
	mPtrState = EPtrState::ToMyMemory;
	return abl;
}

AudioBufferList& AUBufferList::PrepareNullBuffer(
	const AudioStreamBasicDescription& format, UInt32 nFrames)
{
	UInt32 nStreams = 0;
	UInt32 channelsPerStream = 0;
	if (ASBD::IsInterleaved(format)) {
		nStreams = 1;
		channelsPerStream = format.mChannelsPerFrame;
	} else {
		nStreams = format.mChannelsPerFrame;
		channelsPerStream = 1;
	}

	ausdk::ThrowExceptionIf(nStreams > mAllocatedStreams, kAudioUnitErr_FormatNotSupported);
	auto& abl = mBuffers->PrepareNull(channelsPerStream, nFrames * format.mBytesPerFrame);
	mPtrState = EPtrState::ToExternalMemory;
	return abl;
}

void AUBufferList::Allocate(const AudioStreamBasicDescription& format, UInt32 nFrames)
{
	auto& alloc = BufferAllocator::instance();
	if (mBuffers != nullptr) {
		alloc.Deallocate(mBuffers);
	}
	const uint32_t nstreams = ASBD::IsInterleaved(format) ? 1 : format.mChannelsPerFrame;
	mBuffers = alloc.Allocate(nstreams, nFrames * format.mBytesPerFrame, 0u);
	mAllocatedFrames = nFrames;
	mAllocatedStreams = nstreams;
	mPtrState = EPtrState::Invalid;
}

void AUBufferList::Deallocate()
{
	if (mBuffers != nullptr) {
		BufferAllocator::instance().Deallocate(mBuffers);
		mBuffers = nullptr;
	}

	mAllocatedFrames = 0;
	mAllocatedStreams = 0;
	mPtrState = EPtrState::Invalid;
}

} // namespace ausdk

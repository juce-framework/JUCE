/*!
	@file		AudioUnitSDK/AUBufferAllocator.cpp
	@copyright	© 2000-2021 Apple Inc. All rights reserved.
*/
#include <AudioUnitSDK/AUBuffer.h>

namespace ausdk {

BufferAllocator& BufferAllocator::instance()
{
	__attribute__ ((no_destroy)) static BufferAllocator global;
	return global;
}

} // namespace ausdk

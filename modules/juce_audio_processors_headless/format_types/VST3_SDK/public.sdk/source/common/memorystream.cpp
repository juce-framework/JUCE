//-----------------------------------------------------------------------------
// Project     : SDK Core
//
// Category    : Common Classes
// Filename    : public.sdk/source/common/memorystream.cpp
// Created by  : Steinberg, 03/2008
// Description : IBStream Implementation for memory blocks
//
//-----------------------------------------------------------------------------
// This file is part of a Steinberg SDK. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this distribution
// and at www.steinberg.net/sdklicenses.
// No part of the SDK, including this file, may be copied, modified, propagated,
// or distributed except according to the terms contained in the LICENSE file.
//-----------------------------------------------------------------------------

#include "memorystream.h"
#include "pluginterfaces/base/futils.h"
#include <cstdlib>

namespace Steinberg {

//-----------------------------------------------------------------------------
IMPLEMENT_FUNKNOWN_METHODS (MemoryStream, IBStream, IBStream::iid)
static const TSize kMemGrowAmount = 4096;

//-----------------------------------------------------------------------------
MemoryStream::MemoryStream (void* data, TSize length)
: memory ((char*)data)
, memorySize (length)
, size (length)
, cursor (0)
, ownMemory (false)
, allocationError (false)
{ 
	FUNKNOWN_CTOR 
}

//-----------------------------------------------------------------------------
MemoryStream::MemoryStream ()
: memory (nullptr)
, memorySize (0)
, size (0)
, cursor (0)
, ownMemory (true)
, allocationError (false)
{
	FUNKNOWN_CTOR
}

//-----------------------------------------------------------------------------
MemoryStream::~MemoryStream () 
{ 
	if (ownMemory && memory)
		::free (memory);

	FUNKNOWN_DTOR 
}

//-----------------------------------------------------------------------------
tresult PLUGIN_API MemoryStream::read (void* data, int32 numBytes, int32* numBytesRead)
{
	if (memory == nullptr)
	{
		if (allocationError)
			return kOutOfMemory;
		numBytes = 0;
	}
	else
	{		
		// Does read exceed size ?
		if (cursor + numBytes > size)
		{
			int32 maxBytes = int32 (size - cursor);

			// Has length become zero or negative ?
			if (maxBytes <= 0) 
			{
				cursor = size;
				numBytes = 0;
			}
			else
				numBytes = maxBytes;
		}
		
		if (numBytes)
		{
			memcpy (data, &memory[cursor], static_cast<size_t> (numBytes));
			cursor += numBytes;
		}
	}

	if (numBytesRead)
		*numBytesRead = numBytes;

	return kResultTrue;
}

//-----------------------------------------------------------------------------
tresult PLUGIN_API MemoryStream::write (void* buffer, int32 numBytes, int32* numBytesWritten)
{
	if (allocationError)
		return kOutOfMemory;
	if (buffer == nullptr)
		return kInvalidArgument;

	// Does write exceed size ?
	TSize requiredSize = cursor + numBytes;
	if (requiredSize > size) 
	{		
		if (requiredSize > memorySize)
			setSize (requiredSize);
		else
			size = requiredSize;
	}
	
	// Copy data
	if (memory && cursor >= 0 && numBytes > 0)
	{
		memcpy (&memory[cursor], buffer, static_cast<size_t> (numBytes));
		// Update cursor
		cursor += numBytes;
	}
	else
		numBytes = 0;

	if (numBytesWritten)
		*numBytesWritten = numBytes;

	return kResultTrue;
}

//-----------------------------------------------------------------------------
tresult PLUGIN_API MemoryStream::seek (int64 pos, int32 mode, int64* result)
{
	switch (mode) 
	{
		case kIBSeekSet:
			cursor = pos;
			break;
		case kIBSeekCur:
			cursor = cursor + pos;
			break;
		case kIBSeekEnd:
			cursor = size + pos;
			break;
	}

	if (ownMemory == false)
		if (cursor > memorySize)
			cursor = memorySize;

	if (result)
		*result = cursor;

	return kResultTrue;
}

//-----------------------------------------------------------------------------
tresult PLUGIN_API MemoryStream::tell  (int64* pos)
{
	if (!pos)
		return kInvalidArgument;

	*pos = cursor;
	return kResultTrue;
}

//------------------------------------------------------------------------
TSize MemoryStream::getSize () const
{
	return size;
}

//------------------------------------------------------------------------
void MemoryStream::setSize (TSize s)
{
	if (s <= 0)
	{
		if (ownMemory && memory)
			free (memory);

		memory = nullptr;
		memorySize = 0;
		size = 0;
		cursor = 0;
		return;
	}

	TSize newMemorySize = (((Max (memorySize, s) - 1) / kMemGrowAmount) + 1) * kMemGrowAmount;
	if (newMemorySize == memorySize)
	{
		size = s;
		return;
	}

	if (memory && ownMemory == false)
	{
		allocationError = true;
		return;	
	}

	ownMemory = true;
	char* newMemory = nullptr;

	if (memory)
	{
		newMemory = (char*)realloc (memory, (size_t)newMemorySize);
		if (newMemory == nullptr && newMemorySize > 0)
		{
			newMemory = (char*)malloc ((size_t)newMemorySize);
			if (newMemory)
			{
				memcpy (newMemory, memory, (size_t)Min (newMemorySize, memorySize));           
				free (memory);
			}		
		}
	}
	else
		newMemory = (char*)malloc ((size_t)newMemorySize);

	if (newMemory == nullptr)
	{
		if (newMemorySize > 0)
			allocationError = true;

		memory = nullptr;
		memorySize = 0;
		size = 0;
		cursor = 0;
	}
	else
	{
		memory = newMemory;
		memorySize = newMemorySize;
		size = s;
	}
}

//------------------------------------------------------------------------
char* MemoryStream::getData () const
{
	return memory;
}

//------------------------------------------------------------------------
char* MemoryStream::detachData ()
{
	if (ownMemory)
	{
		char* result = memory;
		memory = nullptr;
		memorySize = 0;
		size = 0;
		cursor = 0;
		return result;
	}
	return nullptr;
}

//------------------------------------------------------------------------
bool MemoryStream::truncate ()
{
	if (ownMemory == false)
		return false;

	if (memorySize == size)
		return true;

	memorySize = size;
	
	if (memorySize == 0)
	{
		if (memory)
		{
			free (memory);
			memory = nullptr;
		}
	}
	else
	{
		if (memory)
		{
			char* newMemory = (char*)realloc (memory, (size_t)memorySize);
			if (newMemory)
				memory = newMemory;
		}
	}
	return true;
}

//------------------------------------------------------------------------
bool MemoryStream::truncateToCursor ()
{
	size = cursor;
	return truncate ();
}

} // namespace

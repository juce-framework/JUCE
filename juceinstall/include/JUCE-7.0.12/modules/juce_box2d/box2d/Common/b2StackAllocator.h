/*
* Copyright (c) 2006-2009 Erin Catto http://www.box2d.org
*
* This software is provided 'as-is', without any express or implied
* warranty.  In no event will the authors be held liable for any damages
* arising from the use of this software.
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely, subject to the following restrictions:
* 1. The origin of this software must not be misrepresented; you must not
* claim that you wrote the original software. If you use this software
* in a product, an acknowledgment in the product documentation would be
* appreciated but is not required.
* 2. Altered source versions must be plainly marked as such, and must not be
* misrepresented as being the original software.
* 3. This notice may not be removed or altered from any source distribution.
*/

#ifndef B2_STACK_ALLOCATOR_H
#define B2_STACK_ALLOCATOR_H

#include "b2Settings.h"

const juce::int32 b2_stackSize = 100 * 1024;	// 100k
const juce::int32 b2_maxStackEntries = 32;

struct b2StackEntry
{
	char* data;
	juce::int32 size;
	bool usedMalloc;
};

// This is a stack allocator used for fast per step allocations.
// You must nest allocate/free pairs. The code will assert
// if you try to interleave multiple allocate/free pairs.
class b2StackAllocator
{
public:
	b2StackAllocator();
	~b2StackAllocator();

	void* Allocate(juce::int32 size);
	void Free(void* p);

	juce::int32 GetMaxAllocation() const;

private:

	char m_data[b2_stackSize];
	juce::int32 m_index;

	juce::int32 m_allocation;
	juce::int32 m_maxAllocation;

	b2StackEntry m_entries[b2_maxStackEntries];
	juce::int32 m_entryCount;
};

#endif

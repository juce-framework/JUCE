/*
 * Copyright (C) 2025 Muhammad Tayyab Akram
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stddef.h>
#include <stdlib.h>

#include <juce_graphics/unicode/sheenbidi/Headers/SheenBidi/SBConfig.h>

#include "SBAssert.h"
#include "SBBase.h"
#include "Object.h"

/**
 * Calculates the total size from a list of chunk sizes.
 */
static SBUInteger CalculateTotalSize(const SBUInteger *sizes, SBUInteger count)
{
    SBUInteger totalSize = 0;
    SBUInteger index;

    for (index = 0; index < count; index++) {
        totalSize += sizes[index];
    }

    return totalSize;
}

/**
 * Allocates a single block of memory large enough to store an optional header followed by a
 * sequence of memory chunks. Initializes the `pointers` array with addresses to each chunk.
 */
static void *AllocateMemory(SBUInteger headerSize, SBUInteger totalSize,
    const SBUInteger *sizes, SBUInteger count, void **pointers)
{
    void *base = malloc(headerSize + totalSize);

    if (base) {
        SBUInt8 *memory = (SBUInt8 *)base;
        SBUInteger offset = headerSize;
        SBUInteger index;

        for (index = 0; index < count; index++) {
            pointers[index] = memory + offset;
            offset += sizes[index];
        }
    }

    return base;
}

SB_INTERNAL ObjectRef ObjectCreate(const SBUInteger *sizes, SBUInteger count, void **pointers)
{
    SBUInteger totalSize = CalculateTotalSize(sizes, count);
    ObjectRef object;

    /* Total size MUST be greater than the size of Object structure. */
    SBAssert(totalSize > sizeof(Object));

    object = AllocateMemory(0, totalSize, sizes, count, pointers);

    if (object) {
        ObjectInitialize(object);
    }

    return object;
}

SB_INTERNAL void ObjectInitialize(ObjectRef object)
{
    object->_memoryList = NULL;
}

SB_INTERNAL void *ObjectAddMemory(ObjectRef object, SBUInteger size)
{
    void *pointer;

    if (ObjectAddMemoryWithChunks(object, &size, 1, &pointer)) {
        return pointer;
    }

    return NULL;
}

SB_INTERNAL SBBoolean ObjectAddMemoryWithChunks(ObjectRef object,
    const SBUInteger *sizes, SBUInteger count, void **pointers)
{
    MemoryListRef memoryList = object->_memoryList;
    SBUInteger totalSize = CalculateTotalSize(sizes, count);

    /* Total size MUST be greater than zero. */
    SBAssert(totalSize > 0);

    if (memoryList) {
        MemoryBlockRef block = AllocateMemory(sizeof(MemoryBlock), totalSize, sizes, count, pointers);

        if (block) {
            block->next = NULL;

            memoryList->last->next = block;
            memoryList->last = block;

            return SBTrue;
        }
    } else {
        memoryList = AllocateMemory(sizeof(MemoryList), totalSize, sizes, count, pointers);

        if (memoryList) {
            memoryList->first.next = NULL;
            memoryList->last = &memoryList->first;

            object->_memoryList = memoryList;

            return SBTrue;
        }
    }

    return SBFalse;
}

SB_INTERNAL void ObjectFinalize(ObjectRef object)
{
    MemoryListRef memoryList = object->_memoryList;

    if (memoryList) {
        MemoryBlockRef block = &memoryList->first;

        while (block) {
            MemoryBlockRef next = block->next;
            /* Free the block along with its data as they were allocated together. */
            free(block);

            block = next;
        }

        object->_memoryList = NULL;
    }
}

SB_INTERNAL void ObjectDispose(ObjectRef object)
{
    ObjectFinalize(object);
    free(object);
}

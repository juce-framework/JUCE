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

#ifndef _SB_INTERNAL_OBJECT_H
#define _SB_INTERNAL_OBJECT_H

#include <juce_graphics/unicode/sheenbidi/Headers/SheenBidi/SBConfig.h>

#include "SBBase.h"

/**
 * Represents a single dynamically allocated memory block.
 * The actual memory for use starts right after the MemoryBlock structure.
 */
typedef struct _MemoryBlock {
    struct _MemoryBlock *next;
} MemoryBlock, *MemoryBlockRef;

/**
 * Represents a linked list of memory blocks allocated by an object.
 */
typedef struct _MemoryList {
    MemoryBlock first;
    MemoryBlockRef last;
} MemoryList, *MemoryListRef;

/**
 * Base object containing a memory list.
 */
typedef struct Object {
    MemoryListRef _memoryList;
} Object, *ObjectRef;

/**
 * Creates and initializes an Object with one or more memory chunks.
 *
 * @param sizes
 *      Array of chunk sizes.
 * @param count
 *      Number of chunks.
 * @param pointers
 *      Output array to receive addresses of allocated chunks.
 * @return
 *      A pointer to the initialized Object, or NULL on failure.
 */
SB_INTERNAL ObjectRef ObjectCreate(const SBUInteger *sizes, SBUInteger count, void **pointers);

/**
 * Initializes an already-allocated Object.
 *
 * @param object
 *      The Object to initialize.
 */
SB_INTERNAL void ObjectInitialize(ObjectRef object);

/**
 * Allocates and adds a single memory chunk to an existing Object, tracking it internally.
 *
 * @param object
 *      The Object to which the memory is added.
 * @param size
 *      The size of the chunk to allocate.
 * @return
 *      Pointer to the newly allocated memory, or NULL on failure.
 */
SB_INTERNAL void *ObjectAddMemory(ObjectRef object, SBUInteger size);

/**
 * Allocates and adds multiple memory chunks to an existing Object, tracking them internally.
 *
 * @param object
 *      The Object to which the chunks are added.
 * @param sizes
 *      Array of chunk sizes.
 * @param count
 *      Number of chunks.
 * @param pointers
 *      Output array to receive addresses of allocated chunks.
 * @return
 *      `SBTrue` if successful, `SBFalse` otherwise.
 */
SB_INTERNAL SBBoolean ObjectAddMemoryWithChunks(ObjectRef object,
    const SBUInteger *sizes, SBUInteger count, void **pointers);

/**
 * Frees all memory chunks added to the Object using ObjectAddMemory*.
 *
 * @param object
 *      The Object whose internal memory is released.
 */
SB_INTERNAL void ObjectFinalize(ObjectRef object);

/**
 * Frees the Object and all associated memory.
 *
 * @param object
 *      The Object to dispose.
 */
SB_INTERNAL void ObjectDispose(ObjectRef object);

#endif

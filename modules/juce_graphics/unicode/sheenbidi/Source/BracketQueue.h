/*
 * Copyright (C) 2014-2022 Muhammad Tayyab Akram
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

#ifndef _SB_INTERNAL_BRACKET_QUEUE_H
#define _SB_INTERNAL_BRACKET_QUEUE_H

#include <juce_graphics/unicode/sheenbidi/Headers/SBConfig.h>

#include "BidiChain.h"
#include "SBBase.h"

#define BracketQueueList_Length         8
#define BracketQueueList_MaxIndex       (BracketQueueList_Length - 1)

typedef struct _BracketQueueList {
    SBCodepoint bracket[BracketQueueList_Length];
    BidiLink priorStrongLink[BracketQueueList_Length];
    BidiLink openingLink[BracketQueueList_Length];
    BidiLink closingLink[BracketQueueList_Length];
    SBBidiType strongType[BracketQueueList_Length];

    struct _BracketQueueList *previous;
    struct _BracketQueueList *next;
} BracketQueueList, *BracketQueueListRef;

typedef struct _BracketQueue {
    BracketQueueList _firstList;
    BracketQueueListRef _frontList;
    BracketQueueListRef _rearList;
    SBInteger _frontTop;
    SBInteger _rearTop;
    SBUInteger count;
    SBBoolean shouldDequeue;
    SBBidiType _direction;
} BracketQueue, *BracketQueueRef;

#define BracketQueueGetMaxCapacity()        63

SB_INTERNAL void BracketQueueInitialize(BracketQueueRef queue);
SB_INTERNAL void BracketQueueReset(BracketQueueRef queue, SBBidiType direction);

SB_INTERNAL SBBoolean BracketQueueEnqueue(BracketQueueRef queue,
    BidiLink priorStrongLink, BidiLink openingLink, SBCodepoint bracket);
SB_INTERNAL void BracketQueueDequeue(BracketQueueRef queue);

SB_INTERNAL void BracketQueueSetStrongType(BracketQueueRef queue, SBBidiType strongType);
SB_INTERNAL void BracketQueueClosePair(BracketQueueRef queue,
    BidiLink closingLink, SBCodepoint bracket);

SB_INTERNAL SBBoolean BracketQueueShouldDequeue(BracketQueueRef queue);

SB_INTERNAL BidiLink BracketQueueGetPriorStrongLink(BracketQueueRef queue);
SB_INTERNAL BidiLink BracketQueueGetOpeningLink(BracketQueueRef queue);
SB_INTERNAL BidiLink BracketQueueGetClosingLink(BracketQueueRef queue);
SB_INTERNAL SBBidiType BracketQueueGetStrongType(BracketQueueRef queue);

SB_INTERNAL void BracketQueueFinalize(BracketQueueRef queue);

#endif

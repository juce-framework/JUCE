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

#include <juce_graphics/unicode/sheenbidi/Headers/SBConfig.h>
#include <stddef.h>
#include <stdlib.h>

#include "BidiChain.h"
#include "SBAssert.h"
#include "SBBase.h"
#include "BracketQueue.h"

static SBBoolean BracketQueueInsertElement(BracketQueueRef queue)
{
    if (queue->_rearTop != BracketQueueList_MaxIndex) {
        queue->_rearTop += 1;
    } else {
        BracketQueueListRef previousList = queue->_rearList;
        BracketQueueListRef rearList = previousList->next;

        if (!rearList) {
            rearList = malloc(sizeof(BracketQueueList));
            if (!rearList) {
                return SBFalse;
            }

            rearList->previous = previousList;
            rearList->next = NULL;

            previousList->next = rearList;
        }

        queue->_rearList = rearList;
        queue->_rearTop = 0;
    }
    queue->count += 1;

    return SBTrue;
}

static void BracketQueueFinalizePairs(BracketQueueRef queue, BracketQueueListRef list, SBInteger top)
{
    do {
        SBInteger limit = (list == queue->_rearList ? queue->_rearTop : BracketQueueList_MaxIndex);

        while (++top <= limit) {
            if (list->openingLink[top] != BidiLinkNone
                && list->closingLink[top] == BidiLinkNone) {
                list->openingLink[top] = BidiLinkNone;
            }
        }

        list = list->next;
        top = 0;
    } while (list);
}

SB_INTERNAL void BracketQueueInitialize(BracketQueueRef queue)
{
    queue->_firstList.previous = NULL;
    queue->_firstList.next = NULL;
    queue->_frontList = NULL;
    queue->_rearList = NULL;
    queue->count = 0;
    queue->shouldDequeue = SBFalse;
}

SB_INTERNAL void BracketQueueReset(BracketQueueRef queue, SBBidiType direction)
{
    queue->_frontList = &queue->_firstList;
    queue->_rearList = &queue->_firstList;
    queue->_frontTop = 0;
    queue->_rearTop = -1;
    queue->count = 0;
    queue->shouldDequeue = SBFalse;
    queue->_direction = direction;
}

SB_INTERNAL SBBoolean BracketQueueEnqueue(BracketQueueRef queue,
   BidiLink priorStrongLink, BidiLink openingLink, SBCodepoint bracket)
{
    /* The queue can only take a maximum of 63 elements. */
    SBAssert(queue->count < BracketQueueGetMaxCapacity());

    if (BracketQueueInsertElement(queue)) {
        BracketQueueListRef list = queue->_rearList;
        SBInteger top = queue->_rearTop;

        list->priorStrongLink[top] = priorStrongLink;
        list->openingLink[top] = openingLink;
        list->closingLink[top] = BidiLinkNone;
        list->bracket[top] = bracket;
        list->strongType[top] = SBBidiTypeNil;

        return SBTrue;
    }

    return SBFalse;
}

SB_INTERNAL void BracketQueueDequeue(BracketQueueRef queue)
{
    /* The queue must NOT be empty. */
    SBAssert(queue->count != 0);

    if (queue->_frontTop != BracketQueueList_MaxIndex) {
        queue->_frontTop += 1;
    } else {
        BracketQueueListRef frontList = queue->_frontList;

        if (frontList == queue->_rearList) {
            queue->_rearTop = -1;
        } else {
            queue->_frontList = frontList->next;
        }

        queue->_frontTop = 0;
    }

    queue->count -= 1;
}

SB_INTERNAL void BracketQueueSetStrongType(BracketQueueRef queue, SBBidiType strongType)
{
    BracketQueueListRef list = queue->_rearList;
    SBInteger top = queue->_rearTop;

    while (1) {
        SBInteger limit = (list == queue->_frontList ? queue->_frontTop : 0);

        do {
            if (list->closingLink[top] == BidiLinkNone
                && list->strongType[top] != queue->_direction) {
                list->strongType[top] = strongType;
            }
        } while (top-- > limit);

        if (list == queue->_frontList) {
            break;
        }

        list = list->previous;
        top = BracketQueueList_MaxIndex;
    }
}

SB_INTERNAL void BracketQueueClosePair(BracketQueueRef queue, BidiLink closingLink, SBCodepoint bracket)
{
    BracketQueueListRef list = queue->_rearList;
    SBInteger top = queue->_rearTop;
    SBCodepoint canonical;

    switch (bracket) {
    case 0x232A:
        canonical = 0x3009;
        break;

    case 0x3009:
        canonical = 0x232A;
        break;

    default:
        canonical = bracket;
        break;
    }

    while (1) {
        SBBoolean isFrontList = (list == queue->_frontList);
        SBInteger limit = (isFrontList ? queue->_frontTop : 0);

        do {
            if (list->openingLink[top] != BidiLinkNone
                && list->closingLink[top] == BidiLinkNone
                && (list->bracket[top] == bracket || list->bracket[top] == canonical)) {
                list->closingLink[top] = closingLink;
                BracketQueueFinalizePairs(queue, list, top);

                if (isFrontList && top == queue->_frontTop) {
                    queue->shouldDequeue = SBTrue;
                }

                return;
            }
        } while (top-- > limit);

        if (isFrontList) {
            break;
        }

        list = list->previous;
        top = BracketQueueList_MaxIndex;
    }
}

SB_INTERNAL SBBoolean BracketQueueShouldDequeue(BracketQueueRef queue)
{
    return queue->shouldDequeue;
}

SB_INTERNAL BidiLink BracketQueueGetPriorStrongLink(BracketQueueRef queue)
{
    return queue->_frontList->priorStrongLink[queue->_frontTop];
}

SB_INTERNAL BidiLink BracketQueueGetOpeningLink(BracketQueueRef queue)
{
    return queue->_frontList->openingLink[queue->_frontTop];
}

SB_INTERNAL BidiLink BracketQueueGetClosingLink(BracketQueueRef queue)
{
    return queue->_frontList->closingLink[queue->_frontTop];
}

SB_INTERNAL SBBidiType BracketQueueGetStrongType(BracketQueueRef queue)
{
    return queue->_frontList->strongType[queue->_frontTop];
}

SB_INTERNAL void BracketQueueFinalize(BracketQueueRef queue)
{
    BracketQueueListRef list = queue->_firstList.next;

    while (list) {
        BracketQueueListRef next = list->next;
        free(list);
        list = next;
    }
}

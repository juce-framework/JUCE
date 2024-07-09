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

#include "LevelRun.h"
#include "SBAssert.h"
#include "SBBase.h"
#include "RunQueue.h"

static SBBoolean RunQueueInsertElement(RunQueueRef queue)
{
    if (queue->_rearTop != RunQueueList_MaxIndex) {
        queue->_rearTop += 1;
    } else {
        RunQueueListRef previousList = queue->_rearList;
        RunQueueListRef rearList = previousList->next;

        if (!rearList) {
            rearList = malloc(sizeof(RunQueueList));
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

static void FindPreviousPartialRun(RunQueueRef queue)
{
    RunQueueListRef list = queue->_partialList;
    SBInteger top = queue->_partialTop;

    do {
        SBInteger limit = (list == queue->_frontList ? queue->_frontTop : 0);

        do {
            LevelRunRef levelRun = &list->elements[top];
            if (RunKindIsPartialIsolate(levelRun->kind)) {
                queue->_partialList = list;
                queue->_partialTop = top;
                return;
            }
        } while (top-- > limit);

        list = list->previous;
        top = RunQueueList_MaxIndex;
    } while (list);

    queue->_partialList = NULL;
    queue->_partialTop = -1;
    queue->shouldDequeue = SBFalse;
}

SB_INTERNAL void RunQueueInitialize(RunQueueRef queue)
{
    /* Initialize first list. */
    queue->_firstList.previous = NULL;
    queue->_firstList.next = NULL;

    /* Initialize front and rear lists with first list. */
    queue->_frontList = &queue->_firstList;
    queue->_rearList = &queue->_firstList;
    queue->_partialList = NULL;

    /* Initialize list indexes. */
    queue->_frontTop = 0;
    queue->_rearTop = -1;
    queue->_partialTop = -1;

    /* Initialize rest of the elements. */
    queue->count = 0;
    queue->peek = &queue->_frontList->elements[queue->_frontTop];
    queue->shouldDequeue = SBFalse;
}

SB_INTERNAL SBBoolean RunQueueEnqueue(RunQueueRef queue, const LevelRunRef levelRun)
{
    if (RunQueueInsertElement(queue)) {
        LevelRunRef element = &queue->_rearList->elements[queue->_rearTop];

        /* Copy the level run into the current element. */
        *element = *levelRun;

        /* Complete the latest isolating run with this terminating run. */
        if (queue->_partialTop != -1 && RunKindIsTerminating(element->kind)) {
            LevelRunRef incompleteRun = &queue->_partialList->elements[queue->_partialTop];
            LevelRunAttach(incompleteRun, element);
            FindPreviousPartialRun(queue);
        }

        /* Save the location of the isolating run. */
        if (RunKindIsIsolate(element->kind)) {
            queue->_partialList = queue->_rearList;
            queue->_partialTop = queue->_rearTop;
        }

        return SBTrue;
    }

    return SBFalse;
}

SB_INTERNAL void RunQueueDequeue(RunQueueRef queue)
{
    /* The queue should not be empty. */
    SBAssert(queue->count != 0);

    if (queue->_frontTop != RunQueueList_MaxIndex) {
        queue->_frontTop += 1;
    } else {
        RunQueueListRef frontList = queue->_frontList;

        if (frontList == queue->_rearList) {
            queue->_rearTop = -1;
        } else {
            queue->_frontList = frontList->next;
        }

        queue->_frontTop = 0;
    }

    queue->count -= 1;
    queue->peek = &queue->_frontList->elements[queue->_frontTop];
}

SB_INTERNAL void RunQueueFinalize(RunQueueRef queue)
{
    RunQueueListRef list = queue->_firstList.next;

    while (list) {
        RunQueueListRef next = list->next;
        free(list);
        list = next;
    };
}

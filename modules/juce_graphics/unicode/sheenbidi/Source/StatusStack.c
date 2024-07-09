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

#include "SBAssert.h"
#include "SBBase.h"
#include "StatusStack.h"

static SBBoolean StatusStackInsertElement(StatusStackRef stack)
{
    if (stack->_peekTop != _StatusStackList_MaxIndex) {
        stack->_peekTop += 1;
    } else {
        _StatusStackListRef previousList = stack->_peekList;
        _StatusStackListRef peekList = previousList->next;

        if (!peekList) {
            peekList = malloc(sizeof(_StatusStackList));
            if (!peekList) {
                return SBFalse;
            }

            peekList->previous = previousList;
            peekList->next = NULL;

            previousList->next = peekList;
        }

        stack->_peekList = peekList;
        stack->_peekTop = 0;
    }
    stack->count += 1;

    return SBTrue;
}

SB_INTERNAL void StatusStackInitialize(StatusStackRef stack)
{
    stack->_firstList.previous = NULL;
    stack->_firstList.next = NULL;
    
    StatusStackSetEmpty(stack);
}

SB_INTERNAL SBBoolean StatusStackPush(StatusStackRef stack,
    SBLevel embeddingLevel, SBBidiType overrideStatus, SBBoolean isolateStatus)
{
    /* The stack can hold upto 127 elements. */
    SBAssert(stack->count <= 127);

    if (StatusStackInsertElement(stack)) {
        _StatusStackElementRef element = &stack->_peekList->elements[stack->_peekTop];
        element->embeddingLevel = embeddingLevel;
        element->overrideStatus = overrideStatus;
        element->isolateStatus = isolateStatus;

        return SBTrue;
    }

    return SBFalse;
}

SB_INTERNAL void StatusStackPop(StatusStackRef stack)
{
    /* The stack should not be empty. */
    SBAssert(stack->count != 0);

    if (stack->_peekTop != 0) {
        stack->_peekTop -= 1;
    } else {
        stack->_peekList = stack->_peekList->previous;
        stack->_peekTop = _StatusStackList_MaxIndex;
    }
    stack->count -= 1;
}

SB_INTERNAL void StatusStackSetEmpty(StatusStackRef stack)
{
    stack->_peekList = &stack->_firstList;
    stack->_peekTop = 0;
    stack->count = 0;
}

SB_INTERNAL SBLevel StatusStackGetEmbeddingLevel(StatusStackRef stack)
{
    return stack->_peekList->elements[stack->_peekTop].embeddingLevel;
}

SB_INTERNAL SBBidiType StatusStackGetOverrideStatus(StatusStackRef stack)
{
    return stack->_peekList->elements[stack->_peekTop].overrideStatus;
}

SB_INTERNAL SBBoolean StatusStackGetIsolateStatus(StatusStackRef stack)
{
    return stack->_peekList->elements[stack->_peekTop].isolateStatus;
}

SB_INTERNAL void StatusStackFinalize(StatusStackRef stack)
{
    _StatusStackListRef list = stack->_firstList.next;

    while (list) {
        _StatusStackListRef next = list->next;
        free(list);
        list = next;
    };
}

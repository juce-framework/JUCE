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

#ifndef _SB_INTERNAL_STATUS_STACK_H
#define _SB_INTERNAL_STATUS_STACK_H

#include <juce_graphics/unicode/sheenbidi/Headers/SBConfig.h>
#include "SBBase.h"

#define _StatusStackList_Length         16
#define _StatusStackList_MaxIndex       (_StatusStackList_Length - 1)

typedef struct _StatusStackElement {
    SBBoolean isolateStatus;
    SBBidiType overrideStatus;
    SBLevel embeddingLevel;
} _StatusStackElement, *_StatusStackElementRef;

typedef struct _StatusStackList {
    _StatusStackElement elements[_StatusStackList_Length];

    struct _StatusStackList *previous;
    struct _StatusStackList *next;
} _StatusStackList, *_StatusStackListRef;

typedef struct _StatusStack {
    _StatusStackList _firstList;
    _StatusStackListRef _peekList;
    SBUInteger _peekTop;
    SBUInteger count;
} StatusStack, *StatusStackRef;

SB_INTERNAL void StatusStackInitialize(StatusStackRef stack);
SB_INTERNAL void StatusStackFinalize(StatusStackRef stack);

SB_INTERNAL SBBoolean StatusStackPush(StatusStackRef stack,
   SBLevel embeddingLevel, SBBidiType overrideStatus, SBBoolean isolateStatus);
SB_INTERNAL void StatusStackPop(StatusStackRef stack);
SB_INTERNAL void StatusStackSetEmpty(StatusStackRef stack);

SB_INTERNAL SBLevel StatusStackGetEmbeddingLevel(StatusStackRef stack);
SB_INTERNAL SBBidiType StatusStackGetOverrideStatus(StatusStackRef stack);
SB_INTERNAL SBBoolean StatusStackGetIsolateStatus(StatusStackRef stack);

#endif

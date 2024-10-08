/*
 * Copyright (C) 2014-2019 Muhammad Tayyab Akram
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

#ifndef _SB_INTERNAL_RUN_KIND_H
#define _SB_INTERNAL_RUN_KIND_H

#include "SBBase.h"

enum {
    RunKindSimple         = 0x00,

    RunKindIsolate        = 0x01,
    RunKindPartial        = 0x02,
    RunKindPartialIsolate = RunKindIsolate | RunKindPartial,

    RunKindTerminating    = 0x04,
    RunKindAttached       = 0x08
};
typedef SBUInt8 RunKind;

#define RunKindMake(i, t)                   \
(                                           \
   ((i) ? RunKindPartialIsolate : 0)        \
 | ((t) ? RunKindTerminating : 0)           \
)

#define RunKindMakeComplete(k)              \
(                                           \
 (k) &= ~RunKindPartial                     \
)

#define RunKindMakeAttached(k)              \
(                                           \
 (k) |= RunKindAttached                     \
)

#define RunKindIsSimple(k)                  \
(                                           \
 (k) == RunKindSimple                       \
)

#define RunKindIsIsolate(k)                 \
(                                           \
 (k) & RunKindIsolate                       \
)

#define RunKindIsTerminating(k)             \
(                                           \
 (k) & RunKindTerminating                   \
)

#define RunKindIsPartialIsolate(k)          \
(                                           \
 (k) & RunKindPartial                       \
)

#define RunKindIsCompleteIsolate(k)         \
(                                           \
    ((k) & RunKindPartialIsolate)           \
 == RunKindIsolate                          \
)

#define RunKindIsAttachedTerminating(k)     \
(                                           \
 (k) & RunKindAttached                      \
)

#endif

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

#ifndef _SB_INTERNAL_LEVEL_RUN_H
#define _SB_INTERNAL_LEVEL_RUN_H

#include <juce_graphics/unicode/sheenbidi/Headers/SBConfig.h>

#include "BidiChain.h"
#include "RunExtrema.h"
#include "RunKind.h"
#include "SBBase.h"

typedef struct _LevelRun {
    struct _LevelRun *next;   /**< Reference to the next sequence of run links. */
    BidiLink firstLink;       /**< First link of the run. */
    BidiLink lastLink;        /**< Last link of the run. */
    BidiLink subsequentLink;  /**< Subsequent link of the run. */
    RunExtrema extrema;
    RunKind kind;
    SBLevel level;
} LevelRun, *LevelRunRef;

SB_INTERNAL void LevelRunInitialize(LevelRunRef levelRun,
    BidiChainRef bidiChain, BidiLink firstLink, BidiLink lastLink,
    SBBidiType sor, SBBidiType eor);
SB_INTERNAL void LevelRunAttach(LevelRunRef levelRun, LevelRunRef next);

#endif

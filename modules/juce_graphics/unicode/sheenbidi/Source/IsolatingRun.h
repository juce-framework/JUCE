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

#ifndef _SB_INTERNAL_ISOLATING_RUN_H
#define _SB_INTERNAL_ISOLATING_RUN_H

#include <juce_graphics/unicode/sheenbidi/Headers/SBConfig.h>

#include "BidiChain.h"
#include "BracketQueue.h"
#include "LevelRun.h"
#include "SBBase.h"
#include "SBCodepointSequence.h"

typedef struct _IsolatingRun {
    const SBCodepointSequence *codepointSequence;
    const SBBidiType *bidiTypes;
    BidiChainRef bidiChain;
    LevelRunRef baseLevelRun;
    LevelRunRef _lastLevelRun;
    BracketQueue _bracketQueue;
    SBUInteger paragraphOffset;
    BidiLink _originalLink;
    SBBidiType _sos;
    SBBidiType _eos;
    SBLevel paragraphLevel;
} IsolatingRun, *IsolatingRunRef;

SB_INTERNAL void IsolatingRunInitialize(IsolatingRunRef isolatingRun);
SB_INTERNAL SBBoolean IsolatingRunResolve(IsolatingRunRef isolatingRun);

SB_INTERNAL void IsolatingRunFinalize(IsolatingRunRef isolatingRun);

#endif

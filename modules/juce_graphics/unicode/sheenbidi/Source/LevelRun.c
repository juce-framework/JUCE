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

#include <juce_graphics/unicode/sheenbidi/Headers/SBConfig.h>
#include <stddef.h>

#include "BidiChain.h"
#include "RunExtrema.h"
#include "RunKind.h"
#include "SBAssert.h"
#include "LevelRun.h"

SB_INTERNAL void LevelRunInitialize(LevelRunRef levelRun,
    BidiChainRef bidiChain, BidiLink firstLink, BidiLink lastLink,
    SBBidiType sor, SBBidiType eor)
{
    SBBidiType firstType = BidiChainGetType(bidiChain, firstLink);
    SBBidiType lastType = BidiChainGetType(bidiChain, lastLink);

    levelRun->next = NULL;
    levelRun->firstLink = firstLink;
    levelRun->lastLink = lastLink;
    levelRun->subsequentLink = BidiChainGetNext(bidiChain, lastLink);
    levelRun->extrema = RunExtremaMake(sor, eor);
    levelRun->kind = RunKindMake
                     (
                        SBBidiTypeIsIsolateInitiator(lastType),
                        SBBidiTypeIsIsolateTerminator(firstType)
                     );
    levelRun->level = BidiChainGetLevel(bidiChain, firstLink);
}

SB_INTERNAL void LevelRunAttach(LevelRunRef levelRun, LevelRunRef next)
{
    /* Only the runs of same level can be attached. */
    SBAssert(levelRun->level == next->level);
    /* No other run can be attached with a simple run. */
    SBAssert(!RunKindIsSimple(levelRun->kind));
    /* No other run can be attached with a complete isolating run. */
    SBAssert(!RunKindIsCompleteIsolate(levelRun->kind));
    /* Only a terminating run can be attached with an isolating run. */
    SBAssert(RunKindIsIsolate(levelRun->kind) && RunKindIsTerminating(next->kind));
    /* The next run must be unattached. */
    SBAssert(!RunKindIsAttachedTerminating(next->kind));
    
    if (RunKindIsTerminating(next->kind)) {
        RunKindMakeAttached(next->kind);
    }
    
    if (RunKindIsIsolate(levelRun->kind)) {
        RunKindMakeComplete(levelRun->kind);
    }
    
    levelRun->next = next;
}

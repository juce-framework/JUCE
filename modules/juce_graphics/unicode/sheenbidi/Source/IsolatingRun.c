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

#include "BidiChain.h"
#include "BracketQueue.h"
#include "BracketType.h"
#include "LevelRun.h"
#include "PairingLookup.h"
#include "SBAssert.h"
#include "SBBase.h"
#include "SBLog.h"
#include "IsolatingRun.h"

static void ResolveAvailableBracketPairs(IsolatingRunRef isolatingRun);

static void AttachLevelRunLinks(IsolatingRunRef isolatingRun)
{
    BidiChainRef chain = isolatingRun->bidiChain;
    LevelRunRef baseLevelRun = isolatingRun->baseLevelRun;
    LevelRunRef current;
    LevelRunRef next;

    isolatingRun->_originalLink = BidiChainGetNext(chain, chain->roller);
    BidiChainSetNext(chain, chain->roller, baseLevelRun->firstLink);

    /* Iterate over level runs and attach their links to form an isolating run. */
    for (current = baseLevelRun; (next = current->next); current = next) {
        BidiChainSetNext(chain, current->lastLink, next->firstLink);
    }
    BidiChainSetNext(chain, current->lastLink, chain->roller);

    isolatingRun->_lastLevelRun = current;
    isolatingRun->_sos = RunExtrema_SOR(baseLevelRun->extrema);

    if (!RunKindIsPartialIsolate(baseLevelRun->kind)) {
        isolatingRun->_eos = RunExtrema_EOR(current->extrema);
    } else {
        SBLevel paragraphLevel = isolatingRun->paragraphLevel;
        SBLevel runLevel = baseLevelRun->level;
        SBLevel eosLevel = (runLevel > paragraphLevel ? runLevel : paragraphLevel);
        isolatingRun->_eos = ((eosLevel & 1) ? SBBidiTypeR : SBBidiTypeL);
    }
}

static void AttachOriginalLinks(IsolatingRunRef isolatingRun)
{
    BidiChainRef chain = isolatingRun->bidiChain;
    LevelRunRef current;

    BidiChainSetNext(chain, chain->roller, isolatingRun->_originalLink);

    /* Iterate over level runs and attach original subsequent links. */
    for (current = isolatingRun->baseLevelRun; current; current = current->next) {
        BidiChainSetNext(chain, current->lastLink, current->subsequentLink);
    }
}

static BidiLink ResolveWeakTypes(IsolatingRunRef isolatingRun)
{
    BidiChainRef chain = isolatingRun->bidiChain;
    BidiLink roller = chain->roller;
    BidiLink link;

    BidiLink priorLink;
    SBBidiType sos;

    SBBidiType w1PriorType;
    SBBidiType w2StrongType;
    SBBidiType w4PriorType;
    SBBidiType w5PriorType;
    SBBidiType w7StrongType;

    priorLink = roller;
    sos = isolatingRun->_sos;

    w1PriorType = sos;
    w2StrongType = sos;

    BidiChainForEach(chain, roller, link) {
        SBBidiType type = BidiChainGetType(chain, link);
        SBBoolean forceMerge = SBFalse;

        /* Rule W1 */
        if (type == SBBidiTypeNSM) {
            /* Change the 'type' variable as well because it can be EN on which W2 depends. */
            type = (SBBidiTypeIsIsolate(w1PriorType) ? SBBidiTypeON : w1PriorType);
            BidiChainSetType(chain, link, type);

            /* Fix for 3rd point of rule N0. */
            if (w1PriorType == SBBidiTypeON) {
                forceMerge = SBTrue;
            }
        }
        w1PriorType = type;

        /* Rule W2 */
        if (type == SBBidiTypeEN) {
            if (w2StrongType == SBBidiTypeAL) {
                BidiChainSetType(chain, link, SBBidiTypeAN);
            }
        }
        /*
         * Rule W3
         * NOTE: It is safe to apply W3 in 'else-if' statement because it only depends on type AL.
         *       Even if W2 changes EN to AN, there won't be any harm.
         */
        else if (type == SBBidiTypeAL) {
            BidiChainSetType(chain, link, SBBidiTypeR);
        }

        if (SBBidiTypeIsStrong(type)) {
            /* Save the strong type as it is checked in W2. */
            w2StrongType = type;
        }

        if ((type != SBBidiTypeON && BidiChainGetType(chain, priorLink) == type) || forceMerge) {
            BidiChainAbandonNext(chain, priorLink);
        } else {
            priorLink = link;
        }
    }

    priorLink = roller;
    w4PriorType = sos;
    w5PriorType = sos;
    w7StrongType = sos;

    BidiChainForEach(chain, roller, link) {
        SBBidiType type = BidiChainGetType(chain, link);
        SBBidiType nextType = BidiChainGetType(chain, BidiChainGetNext(chain, link));

        /* Rule W4 */
        if (BidiChainIsSingle(chain, link)
            && SBBidiTypeIsNumberSeparator(type)
            && SBBidiTypeIsNumber(w4PriorType)
            && (w4PriorType == nextType)
            && (w4PriorType == SBBidiTypeEN || type == SBBidiTypeCS))
        {
            /* Change the current type as well because it can be EN on which W5 depends. */
            type = w4PriorType;
            BidiChainSetType(chain, link, type);
        }
        w4PriorType = type;

        /* Rule W5 */
        if (type == SBBidiTypeET && (w5PriorType == SBBidiTypeEN || nextType == SBBidiTypeEN)) {
            /* Change the current type as well because it is EN on which W7 depends. */
            type = SBBidiTypeEN;
            BidiChainSetType(chain, link, type);
        }
        w5PriorType = type;

        switch (type) {
        /* Rule W6 */
        case SBBidiTypeET:
        case SBBidiTypeCS:
        case SBBidiTypeES:
            BidiChainSetType(chain, link, SBBidiTypeON);
            break;

        /*
         * Rule W7
         * NOTE: W7 is expected to be applied after W6. However this is not the case here. The
         *       reason is that W6 can only create the type ON which is not tested in W7 by any
         *       means. So it won't affect the algorithm.
         */
        case SBBidiTypeEN:
            if (w7StrongType == SBBidiTypeL) {
                BidiChainSetType(chain, link, SBBidiTypeL);
            }
            break;

        /*
         * Save the strong type for W7.
         * NOTE: The strong type is expected to be saved after applying W7 because W7 itself creates
         *       a strong type. However the strong type being saved here is based on the type after
         *       W5. This won't effect the algorithm because a single link contains all consecutive
         *       EN types. This means that even if W7 creates a strong type, it will be saved in
         *       next iteration.
         */
        case SBBidiTypeL:
        case SBBidiTypeR:
            w7StrongType = type;
            break;
        }

        if (type != SBBidiTypeON && BidiChainGetType(chain, priorLink) == type) {
            BidiChainAbandonNext(chain, priorLink);
        } else {
            priorLink = link;
        }
    }

    return priorLink;
}

static SBBoolean ResolveBrackets(IsolatingRunRef isolatingRun)
{
    const SBCodepointSequence *sequence = isolatingRun->codepointSequence;
    SBUInteger paragraphOffset = isolatingRun->paragraphOffset;
    BracketQueueRef queue = &isolatingRun->_bracketQueue;
    BidiChainRef chain = isolatingRun->bidiChain;
    BidiLink roller = chain->roller;
    BidiLink link;

    BidiLink priorStrongLink;
    SBLevel runLevel;

    priorStrongLink = BidiLinkNone;
    runLevel = isolatingRun->baseLevelRun->level;

    BracketQueueReset(queue, SBLevelAsNormalBidiType(runLevel));

    BidiChainForEach(chain, roller, link) {
        SBUInteger stringIndex;
        SBCodepoint codepoint;
        SBBidiType type;

        SBCodepoint bracketValue;
        BracketType bracketType;

        type = BidiChainGetType(chain, link);

        switch (type) {
        case SBBidiTypeON:
            stringIndex = BidiChainGetOffset(chain, link) + paragraphOffset;
            codepoint = SBCodepointSequenceGetCodepointAt(sequence, &stringIndex);
            bracketValue = LookupBracketPair(codepoint, &bracketType);

            switch (bracketType) {
            case BracketTypeOpen:
                if (queue->count < BracketQueueGetMaxCapacity()) {
                    if (!BracketQueueEnqueue(queue, priorStrongLink, link, bracketValue)) {
                        return SBFalse;
                    }
                } else {
                    goto Resolve;
                }
                break;

            case BracketTypeClose:
                if (queue->count != 0) {
                    BracketQueueClosePair(queue, link, codepoint);

                    if (BracketQueueShouldDequeue(queue)) {
                        ResolveAvailableBracketPairs(isolatingRun);
                    }
                }
                break;
            }
            break;

        case SBBidiTypeEN:
        case SBBidiTypeAN:
            type = SBBidiTypeR;

        case SBBidiTypeR:
        case SBBidiTypeL:
            if (queue->count != 0) {
                BracketQueueSetStrongType(queue, type);
            }

            priorStrongLink = link;
            break;
        }
    }

Resolve:
    ResolveAvailableBracketPairs(isolatingRun);
    return SBTrue;
}

static void ResolveAvailableBracketPairs(IsolatingRunRef isolatingRun)
{
    BracketQueueRef queue = &isolatingRun->_bracketQueue;
    BidiChainRef chain = isolatingRun->bidiChain;

    SBLevel runLevel;
    SBBidiType embeddingDirection;
    SBBidiType oppositeDirection;

    runLevel = isolatingRun->baseLevelRun->level;
    embeddingDirection = SBLevelAsNormalBidiType(runLevel);
    oppositeDirection = SBLevelAsOppositeBidiType(runLevel);

    while (queue->count != 0) {
        BidiLink openingLink = BracketQueueGetOpeningLink(queue);
        BidiLink closingLink = BracketQueueGetClosingLink(queue);

        if ((openingLink != BidiLinkNone) && (closingLink != BidiLinkNone)) {
            SBBidiType innerStrongType;
            SBBidiType pairType;

            innerStrongType = BracketQueueGetStrongType(queue);

            /* Rule: N0.b */
            if (innerStrongType == embeddingDirection) {
                pairType = innerStrongType;
            }
            /* Rule: N0.c */
            else if (innerStrongType == oppositeDirection) {
                BidiLink priorStrongLink;
                SBBidiType priorStrongType;

                priorStrongLink = BracketQueueGetPriorStrongLink(queue);

                if (priorStrongLink != BidiLinkNone) {
                    BidiLink link;

                    priorStrongType = BidiChainGetType(chain, priorStrongLink);
                    if (SBBidiTypeIsNumber(priorStrongType)) {
                        priorStrongType = SBBidiTypeR;
                    }

                    link = BidiChainGetNext(chain, priorStrongLink);

                    while (link != openingLink) {
                        SBBidiType type = BidiChainGetType(chain, link);
                        if (type == SBBidiTypeL || type == SBBidiTypeR) {
                            priorStrongType = type;
                        }

                        link = BidiChainGetNext(chain, link);
                    }
                } else {
                    priorStrongType = isolatingRun->_sos;
                }

                /* Rule: N0.c.1 */
                if (priorStrongType == oppositeDirection) {
                    pairType = oppositeDirection;
                }
                /* Rule: N0.c.2 */
                else {
                    pairType = embeddingDirection;
                }
            }
            /* Rule: N0.d */
            else {
                pairType = SBBidiTypeNil;
            }

            if (pairType != SBBidiTypeNil) {
                /* Do the substitution */
                BidiChainSetType(chain, openingLink, pairType);
                BidiChainSetType(chain, closingLink, pairType);
            }
        }

        BracketQueueDequeue(queue);
    }
}

static void ResolveNeutrals(IsolatingRunRef isolatingRun)
{
    BidiChainRef chain = isolatingRun->bidiChain;
    BidiLink roller = chain->roller;
    BidiLink link;

    SBLevel runLevel;
    SBBidiType strongType;
    BidiLink neutralLink;

    runLevel = isolatingRun->baseLevelRun->level;
    strongType = isolatingRun->_sos;
    neutralLink = BidiLinkNone;

    BidiChainForEach(chain, roller, link) {
        SBBidiType type = BidiChainGetType(chain, link);
        SBBidiType nextType;

        SBAssert(SBBidiTypeIsStrongOrNumber(type) || SBBidiTypeIsNeutralOrIsolate(type));

        switch (type) {
        case SBBidiTypeL:
            strongType = SBBidiTypeL;
            break;

        case SBBidiTypeR:
        case SBBidiTypeEN:
        case SBBidiTypeAN:
            strongType = SBBidiTypeR;
            break;

        case SBBidiTypeB:                           
        case SBBidiTypeS:
        case SBBidiTypeWS:
        case SBBidiTypeON:
        case SBBidiTypeLRI:
        case SBBidiTypeRLI:                         
        case SBBidiTypeFSI:
        case SBBidiTypePDI:
            if (neutralLink == BidiLinkNone) {
                neutralLink = link;
            }

            nextType = BidiChainGetType(chain, BidiChainGetNext(chain, link));
            if (SBBidiTypeIsNumber(nextType)) {
                nextType = SBBidiTypeR;
            } else if (nextType == SBBidiTypeNil) {
                nextType = isolatingRun->_eos;
            }

            if (SBBidiTypeIsStrong(nextType)) {
                /* Rules N1, N2 */
                SBBidiType resolvedType = (strongType == nextType
                                           ? strongType
                                           : SBLevelAsNormalBidiType(runLevel));

                do {
                    BidiChainSetType(chain, neutralLink, resolvedType);
                    neutralLink = BidiChainGetNext(chain, neutralLink);
                } while (neutralLink != BidiChainGetNext(chain, link));

                neutralLink = BidiLinkNone;
            }
            break;
        }
    }
}

static void ResolveImplicitLevels(IsolatingRunRef isolatingRun)
{
    BidiChainRef chain = isolatingRun->bidiChain;
    BidiLink roller = chain->roller;
    BidiLink link;

    SBLevel runLevel = isolatingRun->baseLevelRun->level;
    
    if ((runLevel & 1) == 0) {
        BidiChainForEach(chain, roller, link) {
            SBBidiType type = BidiChainGetType(chain, link);
            SBLevel level = BidiChainGetLevel(chain, link);
            
            SBAssert(SBBidiTypeIsStrongOrNumber(type));
            
            /* Rule I1 */
            if (type == SBBidiTypeR) {
                BidiChainSetLevel(chain, link, level + 1);
            } else if (type != SBBidiTypeL) {
                BidiChainSetLevel(chain, link, level + 2);
            }
        }
    } else {
        BidiChainForEach(chain, roller, link) {
            SBBidiType type = BidiChainGetType(chain, link);
            SBLevel level = BidiChainGetLevel(chain, link);
            
            SBAssert(SBBidiTypeIsStrongOrNumber(type));
            
            /* Rule I2 */
            if (type != SBBidiTypeR) {
                BidiChainSetLevel(chain, link, level + 1);
            }
        }
    }
}

SB_INTERNAL void IsolatingRunInitialize(IsolatingRunRef isolatingRun)
{
    BracketQueueInitialize(&isolatingRun->_bracketQueue);
}

SB_INTERNAL SBBoolean IsolatingRunResolve(IsolatingRunRef isolatingRun)
{
    BidiLink lastLink;
    BidiLink subsequentLink;

    SB_LOG_BLOCK_OPENER("Identified Isolating Run");

    /* Attach level run links to form isolating run. */
    AttachLevelRunLinks(isolatingRun);
    /* Save last subsequent link. */
    subsequentLink = isolatingRun->_lastLevelRun->subsequentLink;

    SB_LOG_STATEMENT("Range", 1, SB_LOG_RUN_RANGE(isolatingRun));
    SB_LOG_STATEMENT("Types", 1, SB_LOG_RUN_TYPES(isolatingRun));
    SB_LOG_STATEMENT("Level", 1, SB_LOG_LEVEL(isolatingRun->baseLevelRun->level));
    SB_LOG_STATEMENT("SOS", 1, SB_LOG_BIDI_TYPE(isolatingRun->_sos));
    SB_LOG_STATEMENT("EOS", 1, SB_LOG_BIDI_TYPE(isolatingRun->_eos));

    /* Rules W1-W7 */
    lastLink = ResolveWeakTypes(isolatingRun);
    SB_LOG_BLOCK_OPENER("Resolved Weak Types");
    SB_LOG_STATEMENT("Types", 1, SB_LOG_RUN_TYPES(isolatingRun));
    SB_LOG_BLOCK_CLOSER();

    /* Rule N0 */
    if (!ResolveBrackets(isolatingRun)) {
        return SBFalse;
    }

    SB_LOG_BLOCK_OPENER("Resolved Brackets");
    SB_LOG_STATEMENT("Types", 1, SB_LOG_RUN_TYPES(isolatingRun));
    SB_LOG_BLOCK_CLOSER();

    /* Rules N1, N2 */
    ResolveNeutrals(isolatingRun);
    SB_LOG_BLOCK_OPENER("Resolved Neutrals");
    SB_LOG_STATEMENT("Types", 1, SB_LOG_RUN_TYPES(isolatingRun));
    SB_LOG_BLOCK_CLOSER();

    /* Rules I1, I2 */
    ResolveImplicitLevels(isolatingRun);
    SB_LOG_BLOCK_OPENER("Resolved Implicit Levels");
    SB_LOG_STATEMENT("Levels", 1, SB_LOG_RUN_LEVELS(isolatingRun));
    SB_LOG_BLOCK_CLOSER();

    /* Re-attach original links. */
    AttachOriginalLinks(isolatingRun);
    /* Attach new final link (of isolating run) with last subsequent link. */
    BidiChainSetNext(isolatingRun->bidiChain, lastLink, subsequentLink);

    SB_LOG_BLOCK_CLOSER();

    return SBTrue;
}

SB_INTERNAL void IsolatingRunFinalize(IsolatingRunRef isolatingRun)
{
    BracketQueueFinalize(&isolatingRun->_bracketQueue);
}

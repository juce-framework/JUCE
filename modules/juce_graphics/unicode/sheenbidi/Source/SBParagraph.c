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
#include "BidiTypeLookup.h"
#include "IsolatingRun.h"
#include "LevelRun.h"
#include "RunQueue.h"
#include "SBAlgorithm.h"
#include "SBAssert.h"
#include "SBBase.h"
#include "SBCodepointSequence.h"
#include "SBLine.h"
#include "SBLog.h"
#include "StatusStack.h"
#include "SBParagraph.h"

typedef struct _ParagraphContext {
    BidiChain bidiChain;
    StatusStack statusStack;
    RunQueue runQueue;
    IsolatingRun isolatingRun;
} ParagraphContext, *ParagraphContextRef;

static void PopulateBidiChain(BidiChainRef chain, const SBBidiType *types, SBUInteger length);
static SBBoolean ProcessRun(ParagraphContextRef context, const LevelRunRef levelRun, SBBoolean forceFinish);

static ParagraphContextRef CreateParagraphContext(const SBBidiType *types, SBLevel *levels, SBUInteger length)
{
    const SBUInteger sizeContext = sizeof(ParagraphContext);
    const SBUInteger sizeLinks   = sizeof(BidiLink) * (length + 2);
    const SBUInteger sizeTypes   = sizeof(SBBidiType) * (length + 2);
    const SBUInteger sizeMemory  = sizeContext + sizeLinks + sizeTypes;

    void *pointer = malloc(sizeMemory);

    if (pointer) {
        const SBUInteger offsetContext = 0;
        const SBUInteger offsetLinks   = offsetContext + sizeContext;
        const SBUInteger offsetTypes   = offsetLinks + sizeLinks;

        SBUInt8 *memory = (SBUInt8 *)pointer;
        ParagraphContextRef context = (ParagraphContextRef)(memory + offsetContext);
        BidiLink *fixedLinks = (BidiLink *)(memory + offsetLinks);
        SBBidiType *fixedTypes = (SBBidiType *)(memory + offsetTypes);

        BidiChainInitialize(&context->bidiChain, fixedTypes, levels, fixedLinks);
        StatusStackInitialize(&context->statusStack);
        RunQueueInitialize(&context->runQueue);
        IsolatingRunInitialize(&context->isolatingRun);

        PopulateBidiChain(&context->bidiChain, types, length);

        return context;
    }

    return NULL;
}

static void DisposeParagraphContext(ParagraphContextRef context)
{
    StatusStackFinalize(&context->statusStack);
    RunQueueFinalize(&context->runQueue);
    IsolatingRunFinalize(&context->isolatingRun);
    free(context);
}

static SBParagraphRef AllocateParagraph(SBUInteger length)
{
    const SBUInteger sizeParagraph = sizeof(SBParagraph);
    const SBUInteger sizeLevels    = sizeof(SBLevel) * (length + 2);
    const SBUInteger sizeMemory    = sizeParagraph + sizeLevels;

    void *pointer = malloc(sizeMemory);

    if (pointer) {
        const SBUInteger offsetParagraph = 0;
        const SBUInteger offsetLevels    = offsetParagraph + sizeParagraph;

        SBUInt8 *memory = (SBUInt8 *)pointer;
        SBParagraphRef paragraph = (SBParagraphRef)(memory + offsetParagraph);
        SBLevel *levels = (SBLevel *)(memory + offsetLevels);

        paragraph->fixedLevels = levels;

        return paragraph;
    }

    return NULL;
}

static void DisposeParagraph(SBParagraphRef paragraph)
{
    free(paragraph);
}

static SBUInteger DetermineBoundary(SBAlgorithmRef algorithm, SBUInteger paragraphOffset, SBUInteger suggestedLength)
{
    SBBidiType *bidiTypes = algorithm->fixedTypes;
    SBUInteger suggestedLimit = paragraphOffset + suggestedLength;
    SBUInteger stringIndex;

    for (stringIndex = paragraphOffset; stringIndex < suggestedLimit; stringIndex++) {
        if (bidiTypes[stringIndex] == SBBidiTypeB) {
            stringIndex += SBAlgorithmGetSeparatorLength(algorithm, stringIndex);
            goto Return;
        }
    }

Return:
    return (stringIndex - paragraphOffset);
}

static void PopulateBidiChain(BidiChainRef chain, const SBBidiType *types, SBUInteger length)
{
    SBBidiType type = SBBidiTypeNil;
    SBUInteger priorIndex = SBInvalidIndex;
    SBUInteger index;

    for (index = 0; index < length; index++) {
        SBBidiType priorType = type;
        type = types[index];

        switch (type) {
        case SBBidiTypeB:
        case SBBidiTypeON:
        case SBBidiTypeLRE:
        case SBBidiTypeRLE:
        case SBBidiTypeLRO:
        case SBBidiTypeRLO:
        case SBBidiTypePDF:
        case SBBidiTypeLRI:
        case SBBidiTypeRLI:
        case SBBidiTypeFSI:
        case SBBidiTypePDI:
            BidiChainAdd(chain, type, index - priorIndex);
            priorIndex = index;

            if (type == SBBidiTypeB) {
                index = length;
                goto AddLast;
            }
            break;

        default:
            if (type != priorType) {
                BidiChainAdd(chain, type, index - priorIndex);
                priorIndex = index;
            }
            break;
        }
    }

AddLast:
    BidiChainAdd(chain, SBBidiTypeNil, index - priorIndex);
}

static BidiLink SkipIsolatingRun(BidiChainRef chain, BidiLink skipLink, BidiLink breakLink)
{
    BidiLink link = skipLink;
    SBUInteger depth = 1;

    while ((link = BidiChainGetNext(chain, link)) != breakLink) {
        SBBidiType type = BidiChainGetType(chain, link);

        switch (type) {
        case SBBidiTypeLRI:
        case SBBidiTypeRLI:
        case SBBidiTypeFSI:
            depth += 1;
            break;

        case SBBidiTypePDI:
            if (--depth == 0) {
                return link;
            }
            break;
        }
    }

    return BidiLinkNone;
}

static SBLevel DetermineBaseLevel(BidiChainRef chain, BidiLink skipLink, BidiLink breakLink, SBLevel defaultLevel, SBBoolean isIsolate)
{
    BidiLink link = skipLink;

    /* Rules P2, P3 */
    while ((link = BidiChainGetNext(chain, link)) != breakLink) {
        SBBidiType type = BidiChainGetType(chain, link);

        switch (type) {
        case SBBidiTypeL:
            return 0;

        case SBBidiTypeAL:
        case SBBidiTypeR:
            return 1;

        case SBBidiTypeLRI:
        case SBBidiTypeRLI:
        case SBBidiTypeFSI:
            link = SkipIsolatingRun(chain, link, breakLink);
            if (link == BidiLinkNone) {
                goto Default;
            }
            break;

        case SBBidiTypePDI:
            if (isIsolate) {
                /*
                 * In case of isolating run, the PDI will be the last code point.
                 * NOTE:
                 *      The inner isolating runs will be skipped by the case above this one.
                 */
                goto Default;
            }
            break;
        }
    }

Default:
    return defaultLevel;
}

static SBLevel DetermineParagraphLevel(BidiChainRef chain, SBLevel baseLevel)
{
    if (baseLevel >= SBLevelMax) {
        return DetermineBaseLevel(chain, chain->roller, chain->roller,
                                  (baseLevel != SBLevelDefaultRTL ? 0 : 1),
                                  SBFalse);
    }

    return baseLevel;
}

static SBBoolean DetermineLevels(ParagraphContextRef context, SBLevel baseLevel)
{
    BidiChainRef chain = &context->bidiChain;
    StatusStackRef stack = &context->statusStack;
    BidiLink roller = chain->roller;
    BidiLink link;

    BidiLink priorLink;
    BidiLink firstLink;
    BidiLink lastLink;

    SBLevel priorLevel;
    SBBidiType sor;
    SBBidiType eor;

    SBUInteger overIsolate;
    SBUInteger overEmbedding;
    SBUInteger validIsolate;

    priorLink = chain->roller;
    firstLink = BidiLinkNone;
    lastLink = BidiLinkNone;

    priorLevel = baseLevel;
    sor = SBBidiTypeNil;

    /* Rule X1 */
    overIsolate = 0;
    overEmbedding = 0;
    validIsolate = 0;

    StatusStackPush(stack, baseLevel, SBBidiTypeON, SBFalse);

    BidiChainForEach(chain, roller, link) {
        SBBoolean forceFinish = SBFalse;
        SBBoolean bnEquivalent = SBFalse;
        SBBidiType type;

        type = BidiChainGetType(chain, link);

#define LeastGreaterOddLevel()                                              \
(                                                                           \
        (StatusStackGetEmbeddingLevel(stack) + 1) | 1                       \
)

#define LeastGreaterEvenLevel()                                             \
(                                                                           \
        (StatusStackGetEmbeddingLevel(stack) + 2) & ~1                      \
)

#define MergeLinkIfNeeded()                                                 \
{                                                                           \
        if (BidiChainMergeIfEqual(chain, priorLink, link)) {                \
            continue;                                                       \
        }                                                                   \
}

#define PushEmbedding(l, o)                                                 \
{                                                                           \
        SBLevel newLevel = l;                                               \
                                                                            \
        bnEquivalent = SBTrue;                                              \
                                                                            \
        if (newLevel <= SBLevelMax && !overIsolate && !overEmbedding) {     \
            if (!StatusStackPush(stack, newLevel, o, SBFalse)) {            \
                return SBFalse;                                             \
            }                                                               \
        } else {                                                            \
            if (!overIsolate) {                                             \
                overEmbedding += 1;                                         \
            }                                                               \
        }                                                                   \
}

#define PushIsolate(l, o)                                                   \
{                                                                           \
        SBBidiType priorStatus = StatusStackGetOverrideStatus(stack);       \
        SBLevel newLevel = l;                                               \
                                                                            \
        BidiChainSetLevel(chain, link,                                      \
                          StatusStackGetEmbeddingLevel(stack));             \
                                                                            \
        if (newLevel <= SBLevelMax && !overIsolate && !overEmbedding) {     \
            validIsolate += 1;                                              \
                                                                            \
            if (!StatusStackPush(stack, newLevel, o, SBTrue)) {             \
                return SBFalse;                                             \
            }                                                               \
        } else {                                                            \
            overIsolate += 1;                                               \
        }                                                                   \
                                                                            \
        if (priorStatus != SBBidiTypeON) {                                  \
            BidiChainSetType(chain, link, priorStatus);                     \
            MergeLinkIfNeeded();                                            \
        }                                                                   \
}

        switch (type) {
        /* Rule X2 */
        case SBBidiTypeRLE:
            PushEmbedding(LeastGreaterOddLevel(), SBBidiTypeON);
            break;

        /* Rule X3 */
        case SBBidiTypeLRE:
            PushEmbedding(LeastGreaterEvenLevel(), SBBidiTypeON);
            break;

        /* Rule X4 */
        case SBBidiTypeRLO:
            PushEmbedding(LeastGreaterOddLevel(), SBBidiTypeR);
            break;

        /* Rule X5 */
        case SBBidiTypeLRO:
            PushEmbedding(LeastGreaterEvenLevel(), SBBidiTypeL);
            break;

        /* Rule X5a */
        case SBBidiTypeRLI:
            PushIsolate(LeastGreaterOddLevel(), SBBidiTypeON);
            break;

        /* Rule X5b */
        case SBBidiTypeLRI:
            PushIsolate(LeastGreaterEvenLevel(), SBBidiTypeON);
            break;

        /* Rule X5c */
        case SBBidiTypeFSI:
        {
            SBBoolean isRTL = (DetermineBaseLevel(chain, link, roller, 0, SBTrue) == 1);
            PushIsolate(isRTL ? LeastGreaterOddLevel() : LeastGreaterEvenLevel(), SBBidiTypeON);
            break;
        }

        /* Rule X6 */
        default:
            BidiChainSetLevel(chain, link, StatusStackGetEmbeddingLevel(stack));

            if (StatusStackGetOverrideStatus(stack) != SBBidiTypeON) {
                BidiChainSetType(chain, link, StatusStackGetOverrideStatus(stack));
                MergeLinkIfNeeded();
            }
            break;

        /* Rule X6a */
        case SBBidiTypePDI:
        {
            SBBidiType overrideStatus;

            if (overIsolate != 0) {
                overIsolate -= 1;
            } else if (validIsolate == 0) {
                /* Do nothing */
            } else {
                overEmbedding = 0;

                while (!StatusStackGetIsolateStatus(stack)) {
                    StatusStackPop(stack);
                }
                StatusStackPop(stack);

                validIsolate -= 1;
            }

            BidiChainSetLevel(chain, link, StatusStackGetEmbeddingLevel(stack));
            overrideStatus = StatusStackGetOverrideStatus(stack);

            if (overrideStatus != SBBidiTypeON) {
                BidiChainSetType(chain, link, overrideStatus);
                MergeLinkIfNeeded();
            }
            break;
        }

        /* Rule X7 */
        case SBBidiTypePDF:
            bnEquivalent = SBTrue;

            if (overIsolate != 0) {
                /* Do nothing */
            } else if (overEmbedding != 0) {
                overEmbedding -= 1;
            } else if (!StatusStackGetIsolateStatus(stack) && stack->count >= 2) {
                StatusStackPop(stack);
            }
            break;

        /* Rule X8 */
        case SBBidiTypeB:
            /*
             * These values are reset for clarity, in this implementation B can only occur as the
             * last code in the array.
             */
            StatusStackSetEmpty(stack);
            StatusStackPush(stack, baseLevel, SBBidiTypeON, SBFalse);

            overIsolate = 0;
            overEmbedding = 0;
            validIsolate = 0;

            BidiChainSetLevel(chain, link, baseLevel);
            break;

        case SBBidiTypeBN:
            bnEquivalent = SBTrue;
            break;

        case SBBidiTypeNil:
            forceFinish = SBTrue;
            BidiChainSetLevel(chain, link, baseLevel);
            break;
        }

        /* Rule X9 */
        if (bnEquivalent) {
            /* The type of this link is BN equivalent, so abandon it and continue the loop. */
            BidiChainSetType(chain, link, SBBidiTypeBN);
            BidiChainAbandonNext(chain, priorLink);
            continue;
        }

        if (sor == SBBidiTypeNil) {
            sor = SBLevelAsNormalBidiType(SBNumberGetMax(baseLevel, BidiChainGetLevel(chain, link)));
            firstLink = link;
            priorLevel = BidiChainGetLevel(chain, link);
        } else if (priorLevel != BidiChainGetLevel(chain, link) || forceFinish) {
            LevelRun levelRun;
            SBLevel currentLevel;

            /* Since the level has changed at this link, therefore the run must end at prior link. */
            lastLink = priorLink;

            /* Save the current level i.e. level of the next run. */
            currentLevel = BidiChainGetLevel(chain, link);
            /*
             * Now we have both the prior level and the current level i.e. unchanged levels of both
             * the current run and the next run. So, identify eor of the current run.
             * NOTE:
             *      sor of the run has been already determined at this stage.
             */
            eor = SBLevelAsNormalBidiType(SBNumberGetMax(priorLevel, currentLevel));

            LevelRunInitialize(&levelRun, chain, firstLink, lastLink, sor, eor);

            if (!ProcessRun(context, &levelRun, forceFinish)) {
                return SBFalse;
            }

            /* The sor of next run (if any) should be technically equal to eor of this run. */
            sor = eor;
            /* The next run (if any) will start from this index. */
            firstLink = link;

            priorLevel = currentLevel;
        }

        priorLink = link;
    }

    return SBTrue;
}

static SBBoolean ProcessRun(ParagraphContextRef context, const LevelRunRef levelRun, SBBoolean forceFinish)
{
    RunQueueRef queue = &context->runQueue;

    if (!RunQueueEnqueue(queue, levelRun)) {
        return SBFalse;
    }

    if (queue->shouldDequeue || forceFinish) {
        IsolatingRunRef isolatingRun = &context->isolatingRun;
        LevelRunRef peek;

        /* Rule X10 */
        for (; queue->count != 0; RunQueueDequeue(queue)) {
            peek = queue->peek;
            if (RunKindIsAttachedTerminating(peek->kind)) {
                continue;
            }

            isolatingRun->baseLevelRun = peek;

            if (!IsolatingRunResolve(isolatingRun)) {
                return SBFalse;
            }
        }
    }

    return SBTrue;
}

static void SaveLevels(BidiChainRef chain, SBLevel *levels, SBLevel baseLevel)
{
    BidiLink roller = chain->roller;
    BidiLink link;

    SBUInteger index = 0;
    SBLevel level = baseLevel;

    BidiChainForEach(chain, roller, link) {
        SBUInteger offset = BidiChainGetOffset(chain, link);

        for (; index < offset; index++) {
            levels[index] = level;
        }

        level = BidiChainGetLevel(chain, link);
    }
}

static SBBoolean ResolveParagraph(SBParagraphRef paragraph,
    SBAlgorithmRef algorithm, SBUInteger offset, SBUInteger length, SBLevel baseLevel)
{
    const SBBidiType *bidiTypes = algorithm->fixedTypes + offset;
    SBBoolean isSucceeded = SBFalse;
    ParagraphContextRef context;
    SBLevel resolvedLevel;

    context = CreateParagraphContext(bidiTypes, paragraph->fixedLevels, length);

    if (context) {
        resolvedLevel = DetermineParagraphLevel(&context->bidiChain, baseLevel);

        SB_LOG_BLOCK_OPENER("Determined Paragraph Level");
        SB_LOG_STATEMENT("Base Level", 1, SB_LOG_LEVEL(resolvedLevel));
        SB_LOG_BLOCK_CLOSER();

        context->isolatingRun.codepointSequence = &algorithm->codepointSequence;
        context->isolatingRun.bidiTypes = bidiTypes;
        context->isolatingRun.bidiChain = &context->bidiChain;
        context->isolatingRun.paragraphOffset = offset;
        context->isolatingRun.paragraphLevel = resolvedLevel;

        if (DetermineLevels(context, resolvedLevel)) {
            SaveLevels(&context->bidiChain, ++paragraph->fixedLevels, resolvedLevel);

            SB_LOG_BLOCK_OPENER("Determined Embedding Levels");
            SB_LOG_STATEMENT("Levels", 1, SB_LOG_LEVELS_ARRAY(paragraph->fixedLevels, length));
            SB_LOG_BLOCK_CLOSER();

            paragraph->algorithm = SBAlgorithmRetain(algorithm);
            paragraph->refTypes = bidiTypes;
            paragraph->offset = offset;
            paragraph->length = length;
            paragraph->baseLevel = resolvedLevel;
            paragraph->retainCount = 1;

            isSucceeded = SBTrue;
        }

        DisposeParagraphContext(context);
    }

    return isSucceeded;
}

SB_INTERNAL SBParagraphRef SBParagraphCreate(SBAlgorithmRef algorithm,
    SBUInteger paragraphOffset, SBUInteger suggestedLength, SBLevel baseLevel)
{
    const SBCodepointSequence *codepointSequence = &algorithm->codepointSequence;
    SBUInteger stringLength = codepointSequence->stringLength;
    SBUInteger actualLength;

    SBParagraphRef paragraph;

    /* The given range MUST be valid. */
    SBAssert(SBUIntegerVerifyRange(stringLength, paragraphOffset, suggestedLength) && suggestedLength > 0);

    SB_LOG_BLOCK_OPENER("Paragraph Input");
    SB_LOG_STATEMENT("Paragraph Offset", 1, SB_LOG_NUMBER(paragraphOffset));
    SB_LOG_STATEMENT("Suggested Length", 1, SB_LOG_NUMBER(suggestedLength));
    SB_LOG_STATEMENT("Base Direction",   1, SB_LOG_BASE_LEVEL(baseLevel));
    SB_LOG_BLOCK_CLOSER();

    actualLength = DetermineBoundary(algorithm, paragraphOffset, suggestedLength);

    SB_LOG_BLOCK_OPENER("Determined Paragraph Boundary");
    SB_LOG_STATEMENT("Actual Length", 1, SB_LOG_NUMBER(actualLength));
    SB_LOG_BLOCK_CLOSER();

    paragraph = AllocateParagraph(actualLength);

    if (paragraph) {
        if (ResolveParagraph(paragraph, algorithm, paragraphOffset, actualLength, baseLevel)) {
            return paragraph;
        }

        DisposeParagraph(paragraph);
    }

    SB_LOG_BREAKER();

    return NULL;
}

SBUInteger SBParagraphGetOffset(SBParagraphRef paragraph)
{
    return paragraph->offset;
}

SBUInteger SBParagraphGetLength(SBParagraphRef paragraph)
{
    return paragraph->length;
}

SBLevel SBParagraphGetBaseLevel(SBParagraphRef paragraph)
{
    return paragraph->baseLevel;
}

const SBLevel *SBParagraphGetLevelsPtr(SBParagraphRef paragraph)
{
    return paragraph->fixedLevels;
}

SBLineRef SBParagraphCreateLine(SBParagraphRef paragraph, SBUInteger lineOffset, SBUInteger lineLength)
{
    SBUInteger paragraphOffset = paragraph->offset;
    SBUInteger paragraphLength = paragraph->length;
    SBUInteger paragraphLimit = paragraphOffset + paragraphLength;
    SBUInteger lineLimit = lineOffset + lineLength;

    if (lineOffset < lineLimit && lineOffset >= paragraphOffset && lineLimit <= paragraphLimit) {
        return SBLineCreate(paragraph, lineOffset, lineLength);
    }

    return NULL;
}

SBParagraphRef SBParagraphRetain(SBParagraphRef paragraph)
{
    if (paragraph) {
        paragraph->retainCount += 1;
    }
    
    return paragraph;
}

void SBParagraphRelease(SBParagraphRef paragraph)
{
    if (paragraph && --paragraph->retainCount == 0) {
        SBAlgorithmRelease(paragraph->algorithm);
        DisposeParagraph(paragraph);
    }
}

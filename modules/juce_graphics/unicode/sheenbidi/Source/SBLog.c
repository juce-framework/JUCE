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

#ifdef SB_CONFIG_LOG

#include "SBBase.h"
#include "SBBidiChain.h"
#include "SBBidiType.h"
#include "SBIsolatingRun.h"
#include "SBLog.h"

int _SBLogPosition = 0;

SB_INTERNAL void PrintBaseLevel(SBLevel baseLevel)
{
    switch (baseLevel) {
    case SBLevelDefaultLTR:
        SB_LOG_STRING("Auto-LTR");
        break;

    case SBLevelDefaultRTL:
        SB_LOG_STRING("Auto-RTL");
        break;

    case 0:
        SB_LOG_STRING("LTR");
        break;

    case 1:
        SB_LOG_STRING("RTL");
        break;

    default:
        SB_LOG(("Level - %d", baseLevel));
        break;
    }
}

SB_INTERNAL void PrintBidiType(SBBidiType type)
{
    switch (type) {
    case SBBidiTypeNil:
        SB_LOG_STRING("Nil");
        break;

    case SBBidiTypeL:
        SB_LOG_STRING("L");
        break;

    case SBBidiTypeR:
        SB_LOG_STRING("R");
        break;

    case SBBidiTypeAL:
        SB_LOG_STRING("AL");
        break;

    case SBBidiTypeEN:
        SB_LOG_STRING("EN");
        break;

    case SBBidiTypeES:
        SB_LOG_STRING("ES");
        break;

    case SBBidiTypeET:
        SB_LOG_STRING("EN");
        break;

    case SBBidiTypeAN:
        SB_LOG_STRING("AN");
        break;

    case SBBidiTypeCS:
        SB_LOG_STRING("CS");
        break;

    case SBBidiTypeNSM:
        SB_LOG_STRING("NSM");
        break;

    case SBBidiTypeBN:
        SB_LOG_STRING("BN");
        break;

    case SBBidiTypeB:
        SB_LOG_STRING("B");
        break;

    case SBBidiTypeS:
        SB_LOG_STRING("S");
        break;

    case SBBidiTypeWS:
        SB_LOG_STRING("WS");
        break;

    case SBBidiTypeON:
        SB_LOG_STRING("ON");
        break;

    case SBBidiTypeLRE:
        SB_LOG_STRING("LRE");
        break;

    case SBBidiTypeRLE:
        SB_LOG_STRING("RLE");
        break;

    case SBBidiTypeLRO:
        SB_LOG_STRING("LRO");
        break;

    case SBBidiTypeRLO:
        SB_LOG_STRING("RLO");
        break;

    case SBBidiTypePDF:
        SB_LOG_STRING("PDF");
        break;

    case SBBidiTypeLRI:
        SB_LOG_STRING("LRI");
        break;

    case SBBidiTypeRLI:
        SB_LOG_STRING("RLI");
        break;

    case SBBidiTypeFSI:
        SB_LOG_STRING("FSI");
        break;

    case SBBidiTypePDI:
        SB_LOG_STRING("PDI");
        break;
    }
}

SB_INTERNAL void PrintCodepointSequence(const SBCodepointSequence *codepointSequence)
{
    SBUInteger stringIndex = 0;
    SBCodepoint codepoint;

    while ((codepoint = SBCodepointSequenceGetCodepointAt(codepointSequence, &stringIndex)) != SBCodepointInvalid) {
        SB_LOG(("%04X ", codepoint));
    }
}

SB_INTERNAL void PrintBidiTypesArray(SBBidiType *types, SBUInteger length)
{
    SBUInteger index;

    for (index = 0; index < length; ++index) {
        SB_LOG_BIDI_TYPE(types[index]);
        SB_LOG_DIVIDER(1);
    }
}

SB_INTERNAL void PrintLevelsArray(SBLevel *levels, SBUInteger length)
{
    SBUInteger index;

    for (index = 0; index < length; ++index) {
        SB_LOG_LEVEL(levels[index]);
        SB_LOG_DIVIDER(1);
    }
}

typedef struct {
    void *object;
    BidiLink link;
    SBUInteger length;
} IsolatingContext;

typedef void (*IsolatingConsumer)(IsolatingRunRef isolatingRun, IsolatingContext *context);

SB_INTERNAL void IsolatingRunForEach(IsolatingRunRef isolatingRun,
    IsolatingContext *context, IsolatingConsumer consumer)
{
    BidiChainRef bidiChain = isolatingRun->bidiChain;
    LevelRunRef levelRun;

    /* Iterate over individual level runs of the isolating run. */
    for (levelRun = isolatingRun->baseLevelRun; levelRun; levelRun = levelRun->next) {
        BidiLink breakLink = BidiChainGetNext(bidiChain, levelRun->lastLink);
        BidiLink currentLink = levelRun->firstLink;
        BidiLink subsequentLink = levelRun->subsequentLink;

        /* Iterate over each link of the level run. */
        while (currentLink != breakLink) {
            BidiLink nextLink = BidiChainGetNext(bidiChain, currentLink);
            SBUInteger linkOffset = BidiChainGetOffset(bidiChain, currentLink);
            SBUInteger linkLength;
            SBUInteger index;

            if (nextLink != breakLink) {
                linkLength = BidiChainGetOffset(bidiChain, nextLink) - linkOffset;
            } else {
                linkLength = BidiChainGetOffset(bidiChain, subsequentLink) - linkOffset;
            }

            /* Skip any sequence of BN character types. */
            for (index = 1; index < linkLength; index++) {
                SBBidiType bidiType = BidiChainGetType(bidiChain, currentLink + index);
                if (bidiType == SBBidiTypeBN) {
                    linkLength = index;
                    break;
                }
            }

            context->link = currentLink;
            context->length = linkLength;
            consumer(isolatingRun, context);
            
            currentLink = nextLink;
        }
    }
}

static void PrintTypesOperation(IsolatingRunRef isolatingRun, IsolatingContext *context)
{
    SBBidiType bidiType = BidiChainGetType(isolatingRun->bidiChain, context->link);

    while (context->length--) {
        SB_LOG_BIDI_TYPE(bidiType);
        SB_LOG_DIVIDER(1);
    }
}

SB_INTERNAL void PrintRunTypes(IsolatingRunRef isolatingRun)
{
    IsolatingContext context;
    IsolatingRunForEach(isolatingRun, &context, _SBPrintTypesOperation);
}

static void PrintLevelsOperation(IsolatingRunRef isolatingRun, IsolatingContext *context)
{
    SBLevel charLevel = BidiChainGetLevel(isolatingRun->bidiChain, context->link);

    while (context->length--) {
        SB_LOG_LEVEL(charLevel);
        SB_LOG_DIVIDER(1);
    }
}

SB_INTERNAL void PrintRunLevels(IsolatingRunRef isolatingRun)
{
    IsolatingContext context;
    IsolatingRunForEach(isolatingRun, &context, _SBPrintLevelsOperation);
}

typedef struct {
    SBUInteger offset;
    SBUInteger length;
} IsolatingRange;

static void PrintRangeOperation(IsolatingRunRef isolatingRun, IsolatingContext *context)
{
    IsolatingRange *range = context->object;
    SBUInteger offset = BidiChainGetOffset(isolatingRun->bidiChain, context->link);

    if (range->length == 0) {
        range->offset = offset;
        range->length = context->length;
    } else if (offset == (range->offset + range->length)) {
        range->length += context->length;
    } else {
        SB_LOG_RANGE(range->offset, range->length);
        SB_LOG_DIVIDER(1);

        range->offset = offset;
        range->length = context->length;
    }
}

SB_INTERNAL void PrintRunRange(IsolatingRunRef isolatingRun)
{
    IsolatingRange range = { 0, 0 };
    IsolatingContext context;
    context.object = &range;

    IsolatingRunForEach(isolatingRun, &context, _SBPrintRangeOperation);
    SB_LOG_RANGE(range.offset, range.length);
    SB_LOG_DIVIDER(1);
}

#endif

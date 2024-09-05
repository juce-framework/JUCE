/*
 * Copyright (C) 2016-2019 Muhammad Tayyab Akram
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

#include <SBConfig.h>

#include <stddef.h>
#include <stdlib.h>

#include "SBAssert.h"
#include "SBBase.h"
#include "SBCodepointSequence.h"

typedef struct {
    SBUInt8 valid;
    SBUInt8 total;
    SBUInt8 start;
    SBUInt8 end;
} UTF8State;

static const UTF8State UTF8StateTable[9] = {
    {1,0,0x00,0x00}, {0,0,0x00,0x00}, {1,2,0x80,0xBF}, {1,3,0xA0,0xBF}, {1,3,0x80,0xBF},
    {1,3,0x80,0x9F}, {1,4,0x90,0xBF}, {1,4,0x80,0xBF}, {1,4,0x80,0x8F}
};

static const SBUInt8 UTF8LookupTable[256] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
/* LEAD: -- 80..BF -- */
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1,
/* LEAD: -- C0..C1 -- */
    1, 1,
/* LEAD: -- C2..DF -- */
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
/* LEAD: -- E0..E0 -- */
    3,
/* LEAD: -- E1..EC -- */
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
/* LEAD: -- ED..ED -- */
    5,
/* LEAD: -- EE..EF -- */
    4, 4,
/* LEAD: -- F0..F0 -- */
    6,
/* LEAD: -- F1..F3 -- */
    7, 7, 7,
/* LEAD: -- F4..F4 -- */
    8,
/* LEAD: -- F5..F7 -- */
    1, 1, 1,
/* LEAD: -- F8..FB -- */
    1, 1, 1, 1,
/* LEAD: -- FC..FD -- */
    1, 1,
/* LEAD: -- FE..FF -- */
    1, 1
};

static SBCodepoint GetUTF8CodepointAt(const SBCodepointSequence *codepointSequence, SBUInteger *stringIndex);
static SBCodepoint GetUTF8CodepointBefore(const SBCodepointSequence *codepointSequence, SBUInteger *stringIndex);
static SBCodepoint GetUTF16CodepointAt(const SBCodepointSequence *codepointSequence, SBUInteger *stringIndex);
static SBCodepoint GetUTF16CodepointBefore(const SBCodepointSequence *codepointSequence, SBUInteger *stringIndex);
static SBCodepoint GetUTF32CodepointAt(const SBCodepointSequence *codepointSequence, SBUInteger *stringIndex);
static SBCodepoint GetUTF32CodepointBefore(const SBCodepointSequence *codepointSequence, SBUInteger *stringIndex);

SB_INTERNAL SBBoolean SBCodepointSequenceIsValid(const SBCodepointSequence *codepointSequence)
{
    if (codepointSequence) {
        SBBoolean encodingValid = SBFalse;

        switch (codepointSequence->stringEncoding) {
        case SBStringEncodingUTF8:
        case SBStringEncodingUTF16:
        case SBStringEncodingUTF32:
            encodingValid = SBTrue;
            break;
        }

        return (encodingValid && codepointSequence->stringBuffer && codepointSequence->stringLength > 0);
    }

    return SBFalse;
}

SBCodepoint SBCodepointSequenceGetCodepointBefore(const SBCodepointSequence *codepointSequence, SBUInteger *stringIndex)
{
    SBCodepoint codepoint = SBCodepointInvalid;

    if ((*stringIndex - 1) < codepointSequence->stringLength) {
        switch (codepointSequence->stringEncoding) {
        case SBStringEncodingUTF8:
            codepoint = GetUTF8CodepointBefore(codepointSequence, stringIndex);
            break;

        case SBStringEncodingUTF16:
            codepoint = GetUTF16CodepointBefore(codepointSequence, stringIndex);
            break;

        case SBStringEncodingUTF32:
            codepoint = GetUTF32CodepointBefore(codepointSequence, stringIndex);
            break;
        }
    }

    return codepoint;
}

SBCodepoint SBCodepointSequenceGetCodepointAt(const SBCodepointSequence *codepointSequence, SBUInteger *stringIndex)
{
    SBCodepoint codepoint = SBCodepointInvalid;

    if (*stringIndex < codepointSequence->stringLength) {
        switch (codepointSequence->stringEncoding) {
        case SBStringEncodingUTF8:
            codepoint = GetUTF8CodepointAt(codepointSequence, stringIndex);
            break;

        case SBStringEncodingUTF16:
            codepoint = GetUTF16CodepointAt(codepointSequence, stringIndex);
            break;

        case SBStringEncodingUTF32:
            codepoint = GetUTF32CodepointAt(codepointSequence, stringIndex);
            break;
        }
    }

    return codepoint;
}

static SBCodepoint GetUTF8CodepointAt(const SBCodepointSequence *sequence, SBUInteger *index)
{
    const SBUInt8 *buffer = sequence->stringBuffer;
    SBUInteger length = sequence->stringLength;
    SBUInt8 lead;
    UTF8State state;
    SBUInteger limit;
    SBCodepoint codepoint;

    lead = buffer[*index];
    state = UTF8StateTable[UTF8LookupTable[lead]];
    limit = *index + state.total;

    if (limit > length) {
        limit = length;
        state.valid = SBFalse;
    }

    codepoint = lead & (0x7F >> state.total);

    while (++(*index) < limit) {
        SBUInt8 byte = buffer[*index];

        if (byte >= state.start && byte <= state.end) {
            codepoint = (codepoint << 6) | (byte & 0x3F);
        } else {
            state.valid = SBFalse;
            break;
        }

        state.start = 0x80;
        state.end = 0xBF;
    }

    if (state.valid) {
        return codepoint;
    }

    return SBCodepointFaulty;
}

static SBCodepoint GetUTF8CodepointBefore(const SBCodepointSequence *sequence, SBUInteger *index)
{
    const SBUInt8 *buffer = sequence->stringBuffer;
    SBUInteger startIndex = *index;
    SBUInteger limitIndex;
    SBUInteger continuation;
    SBCodepoint codepoint;

    continuation = 7;

    while (--continuation && --startIndex) {
        SBUInt8 codeunit = buffer[startIndex];

        if ((codeunit & 0xC0) != 0x80) {
            break;
        }
    }

    limitIndex = startIndex;
    codepoint = GetUTF8CodepointAt(sequence, &limitIndex);

    if (limitIndex == *index) {
        *index = startIndex;
    } else {
        codepoint = SBCodepointFaulty;
        *index -= 1;
    }

    return codepoint;
}

static SBCodepoint GetUTF16CodepointAt(const SBCodepointSequence *sequence, SBUInteger *index)
{
    const SBUInt16 *buffer = sequence->stringBuffer;
    SBUInteger length = sequence->stringLength;
    SBCodepoint codepoint;
    SBUInt16 lead;

    codepoint = SBCodepointFaulty;

    lead = buffer[*index];
    *index += 1;

    if (!SBCodepointIsSurrogate(lead)) {
        codepoint = lead;
    } else if (lead <= 0xDBFF) {
        if (*index < length) {
            SBUInt16 trail = buffer[*index];

            if (SBUInt16InRange(trail, 0xDC00, 0xDFFF)) {
                codepoint = (lead << 10) + trail - ((0xD800 << 10) + 0xDC00 - 0x10000);
                *index += 1;
            }
        }
    }

    return codepoint;
}

static SBCodepoint GetUTF16CodepointBefore(const SBCodepointSequence *sequence, SBUInteger *index)
{
    const SBUInt16 *buffer = sequence->stringBuffer;
    SBCodepoint codepoint;
    SBUInt16 trail;

    codepoint = SBCodepointFaulty;

    *index -= 1;
    trail = buffer[*index];

    if (!SBCodepointIsSurrogate(trail)) {
        codepoint = trail;
    } else if (trail >= 0xDC00) {
        if (*index > 0) {
            SBUInt16 lead = buffer[*index - 1];

            if (SBUInt16InRange(lead, 0xD800, 0xDBFF)) {
                codepoint = (lead << 10) + trail - ((0xD800 << 10) + 0xDC00 - 0x10000);
                *index -= 1;
            }
        }
    }
    
    return codepoint;
}

static SBCodepoint GetUTF32CodepointAt(const SBCodepointSequence *sequence, SBUInteger *index)
{
    const SBUInt32 *buffer = sequence->stringBuffer;
    SBCodepoint codepoint;

    codepoint = buffer[*index];
    *index += 1;

    if (SBCodepointIsValid(codepoint)) {
        return codepoint;
    }
    
    return SBCodepointFaulty;
}

static SBCodepoint GetUTF32CodepointBefore(const SBCodepointSequence *sequence, SBUInteger *index)
{
    const SBUInt32 *buffer = sequence->stringBuffer;
    SBCodepoint codepoint;

    *index -= 1;
    codepoint = buffer[*index];

    if (SBCodepointIsValid(codepoint)) {
        return codepoint;
    }

    return SBCodepointFaulty;
}

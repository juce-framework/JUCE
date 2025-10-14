/*
 * Copyright (C) 2025 Muhammad Tayyab Akram
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

#include <SheenBidi/SBBase.h>
#include <SheenBidi/SBConfig.h>

#include "BidiTypeLookup.h"
#include "GeneralCategoryLookup.h"
#include "PairingLookup.h"
#include "ScriptLookup.h"
#include "SBCodepoint.h"

typedef struct {
    SBUInt8 valid;
    SBUInt8 total;
    SBUInt8 start;
    SBUInt8 end;
} UTF8State;

static const UTF8State UTF8StateTable[9] = {
    { 1, 0, 0x00, 0x00 }, { 0, 0, 0x00, 0x00 }, { 1, 2, 0x80, 0xBF }, { 1, 3, 0xA0, 0xBF },
    { 1, 3, 0x80, 0xBF }, { 1, 3, 0x80, 0x9F }, { 1, 4, 0x90, 0xBF }, { 1, 4, 0x80, 0xBF },
    { 1, 4, 0x80, 0x8F }
};

static const SBUInt8 UTF8LookupTable[256] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* LEAD: -- 80..BF -- */
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
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

SB_INTERNAL SBBoolean SBCodepointIsCanonicalEquivalentBracket(
    SBCodepoint codepoint, SBCodepoint bracket)
{
    SBCodepoint canonical;

    switch (codepoint) {
    case 0x2329:
        canonical = 0x3008;
        break;
    case 0x3008:
        canonical = 0x2329;
        break;

    case 0x232A:
        canonical = 0x3009;
        break;
    case 0x3009:
        canonical = 0x232A;
        break;

    default:
        canonical = codepoint;
        break;
    }

    return bracket == codepoint || bracket == canonical;
}

SBBidiType SBCodepointGetBidiType(SBCodepoint codepoint)
{
    return LookupBidiType(codepoint);
}

SBGeneralCategory SBCodepointGetGeneralCategory(SBCodepoint codepoint)
{
    return LookupGeneralCategory(codepoint);
}

SBCodepoint SBCodepointGetMirror(SBCodepoint codepoint)
{
    return LookupMirror(codepoint);
}

SBScript SBCodepointGetScript(SBCodepoint codepoint)
{
    return LookupScript(codepoint);
}

SBCodepoint SBCodepointDecodeNextFromUTF8(const SBUInt8 *buffer, SBUInteger length, SBUInteger *index)
{
    SBCodepoint codepoint = SBCodepointInvalid;

    if (*index < length) {
        SBUInt8 lead = buffer[*index];
        UTF8State state = UTF8StateTable[UTF8LookupTable[lead]];
        SBUInteger limit = *index + state.total;

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

        if (!state.valid) {
            codepoint = SBCodepointFaulty;
        }
    }

    return codepoint;
}

SBCodepoint SBCodepointDecodePreviousFromUTF8(const SBUInt8 *buffer, SBUInteger length, SBUInteger *index)
{
    SBCodepoint codepoint = SBCodepointInvalid;

    if ((*index - 1) < length) {
        SBUInteger startIndex = *index;
        SBUInteger limitIndex;
        SBUInteger continuation;

        continuation = 4;

        while (continuation-- && --startIndex) {
            SBUInt8 codeUnit = buffer[startIndex];

            if ((codeUnit & 0xC0) != 0x80) {
                break;
            }
        }

        limitIndex = startIndex;
        codepoint = SBCodepointDecodeNextFromUTF8(buffer, length, &limitIndex);

        if (limitIndex == *index) {
            *index = startIndex;
        } else {
            codepoint = SBCodepointFaulty;
            *index -= 1;
        }
    }

    return codepoint;
}

SBCodepoint SBCodepointDecodeNextFromUTF16(const SBUInt16 *buffer, SBUInteger length, SBUInteger *index)
{
    SBCodepoint codepoint = SBCodepointInvalid;

    if (*index < length) {
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
    }

    return codepoint;
}

SBCodepoint SBCodepointDecodePreviousFromUTF16(const SBUInt16 *buffer, SBUInteger length, SBUInteger *index)
{
    SBCodepoint codepoint = SBCodepointInvalid;

    if ((*index - 1) < length) {
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
    }

    return codepoint;
}

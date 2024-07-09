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

#ifndef _SB_INTERNAL_LOG_H
#define _SB_INTERNAL_LOG_H

#include <juce_graphics/unicode/sheenbidi/Headers/SBConfig.h>

#ifdef SB_CONFIG_LOG

#include <stdio.h>

#include "SBBase.h"
#include "SBBidiType.h"
#include "SBCodepointSequence.h"
#include "SBIsolatingRun.h"

SB_INTERNAL void PrintBaseLevel(SBLevel baseLevel);
SB_INTERNAL void PrintBidiType(SBBidiType type);

SB_INTERNAL void PrintCodepointSequence(const SBCodepointSequence *codepointSequence);
SB_INTERNAL void PrintBidiTypesArray(SBBidiType *types, SBUInteger length);
SB_INTERNAL void PrintLevelsArray(SBLevel *levels, SBUInteger length);

SB_INTERNAL void PrintRunTypes(IsolatingRunRef isolatingRun);
SB_INTERNAL void PrintRunLevels(IsolatingRunRef isolatingRun);
SB_INTERNAL void PrintRunRange(IsolatingRunRef isolatingRun);

extern int _SBLogPosition;

#define SB_LOG_BEGIN()                  (++_SBLogPosition)
#define SB_LOG_END()                    (--_SBLogPosition)

#define SB_LOG(s)                       printf s

#define SB_LOG_NUMBER(n)                \
SB_LOG(("%ld", (long)n))

#define SB_LOG_RANGE(o, l)              \
SB_LOG(("[%ld, %ld]", (long)o, (long)(o + l - 1)))

#define SB_LOG_CHAR(c)                  \
SBLOG(("%c", c))

#define SB_LOG_STRING(s)                \
SB_LOG(("%s", s))

#define SB_LOG_LEVEL(l)                 \
SB_LOG_NUMBER(l)

#define SB_LOG_BREAKER()                \
SB_LOG(("\n"))

#define SB_LOG_DIVIDER(n)               \
SB_LOG(("%.*s", n, "\t\t\t\t\t\t\t\t\t\t"))

#define SB_LOG_INITIATOR()              \
SB_LOG_DIVIDER(_SBLogPosition)

#define SB_LOG_CAPTION(c)               \
SB_LOG((c":"))

#define SB_LOG_STATEMENT_TEXT(t)        \
(t)

#define SB_LOG_LINE(s)                  \
do {                                    \
    SB_LOG(s);                          \
    SB_LOG_BREAKER();                   \
} while (0)

#define SB_LOG_STATEMENT(c, d, t)       \
do {                                    \
    SB_LOG_INITIATOR();                 \
    SB_LOG_CAPTION(c);                  \
    SB_LOG_DIVIDER(d);                  \
    SB_LOG_STATEMENT_TEXT(t);           \
    SB_LOG_BREAKER();                   \
} while (0)

#define SB_LOG_BLOCK_OPENER(c)          \
do {                                    \
    SB_LOG_INITIATOR();                 \
    SB_LOG_CAPTION(c);                  \
    SB_LOG_BREAKER();                   \
    SB_LOG_BEGIN();                     \
} while (0)

#define SB_LOG_BLOCK_CLOSER()           SB_LOG_END()

#define SB_LOG_BASE_LEVEL(l)            PrintBaseLevel(l)
#define SB_LOG_BIDI_TYPE(t)             PrintBidiType(t)

#define SB_LOG_CODEPOINT_SEQUENCE(s)    PrintCodepointSequence(s)
#define SB_LOG_BIDI_TYPES_ARRAY(a, l)   PrintBidiTypesArray(a, l)
#define SB_LOG_LEVELS_ARRAY(a, l)       PrintLevelsArray(a, l)

#define SB_LOG_RUN_TYPES(r)             PrintRunTypes(r)
#define SB_LOG_RUN_LEVELS(r)            PrintRunLevels(r)
#define SB_LOG_RUN_RANGE(r)             PrintRunRange(r)

#else

#define SB_LOG_NONE()

#define SB_LOG(s)                       SB_LOG_NONE()

#define SB_LOG_NUMBER(n)                SB_LOG_NONE()
#define SB_LOG_RANGE(o, l)              SB_LOG_NONE()
#define SB_LOG_CHAR(c)                  SB_LOG_NONE()
#define SB_LOG_STRING(s)                SB_LOG_NONE()
#define SB_LOG_LEVEL(l)                 SB_LOG_NONE()

#define SB_LOG_BREAKER()                SB_LOG_NONE()
#define SB_LOG_DIVIDER(n)               SB_LOG_NONE()
#define SB_LOG_INITIATOR()              SB_LOG_NONE()
#define SB_LOG_CAPTION(c)               SB_LOG_NONE()
#define SB_LOG_STATEMENT_TEXT(t)        SB_LOG_NONE()

#define SB_LOG_LINE(s)                  SB_LOG_NONE()
#define SB_LOG_STATEMENT(c, d, t)       SB_LOG_NONE()

#define SB_LOG_BLOCK_OPENER(c)          SB_LOG_NONE()
#define SB_LOG_BLOCK_CLOSER()           SB_LOG_NONE()

#define SB_LOG_BASE_LEVEL(l)            SB_LOG_NONE()
#define SB_LOG_BIDI_TYPE(t)             SB_LOG_NONE()

#define SB_LOG_CODEPOINT_SEQUENCE(s)    SB_LOG_NONE()
#define SB_LOG_BIDI_TYPES_ARRAY(a, l)   SB_LOG_NONE()
#define SB_LOG_LEVELS_ARRAY(a, l)       SB_LOG_NONE()

#define SB_LOG_RUN_TYPES(r)             SB_LOG_NONE()
#define SB_LOG_RUN_LEVELS(r)            SB_LOG_NONE()

#define SB_LOG_RUN_RANGE(r)             SB_LOG_NONE()

#endif

#endif

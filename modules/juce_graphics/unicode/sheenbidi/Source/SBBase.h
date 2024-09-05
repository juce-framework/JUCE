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

#ifndef _SB_INTERNAL_BASE_H
#define _SB_INTERNAL_BASE_H

#include <SBBase.h>
#include <SBBidiType.h>
#include <SBCodepoint.h>
#include <SBConfig.h>
#include <SBGeneralCategory.h>
#include <SBScript.h>

/**
 * A value that indicates an invalid unsigned index.
 */
#define SBInvalidIndex  (SBUInteger)(-1)

SB_INTERNAL void SBUIntegerNormalizeRange(SBUInteger actualLength,
    SBUInteger *rangeOffset, SBUInteger *rangeLength);

SB_INTERNAL SBBoolean SBUIntegerVerifyRange(SBUInteger actualLength,
    SBUInteger rangeOffset, SBUInteger rangeLength);


#define SBNumberGetMax(first, second)           \
(                                               \
   (first) > (second)                           \
 ? (first)                                      \
 : (second)                                     \
)

#define SBNumberLimitIncrement(number, limit)   \
(                                               \
   (number) < (limit)                           \
 ? (number) + (1)                               \
 : (limit)                                      \
)

#define SBNumberLimitDecrement(number, limit)   \
(                                               \
   (number) > (limit)                           \
 ? (number) - (1)                               \
 : (limit)                                      \
)

#define SBNumberRingAdd(number, count, capacity) \
    (((number) + (count)) % (capacity))

#define SBNumberRingIncrement(number, capacity) \
    SBNumberRingAdd(number, 1, capacity)

#define SBNumberRingSubtract(number, count, capacity) \
    (((number) + (capacity) - (count)) % (capacity))

#define SBNumberRingDecrement(number, capacity) \
    SBNumberRingSubtract(number, 1, capacity)


#define SBLevelAsNormalBidiType(level)      \
(                                           \
   ((level) & 1)                            \
 ? SBBidiTypeR                              \
 : SBBidiTypeL                              \
)

#define SBLevelAsOppositeBidiType(level)    \
(                                           \
   ((level) & 1)                            \
 ? SBBidiTypeL                              \
 : SBBidiTypeR                              \
)


#define SBBidiTypeIsEqual(t1, t2)           ((t1) == (t2))

#define SBBidiTypeIsNumber(t)               SBUInt8InRange(t, SBBidiTypeAN, SBBidiTypeEN)
#define SBBidiTypeIsIsolate(t)              SBUInt8InRange(t, SBBidiTypeLRI, SBBidiTypePDI)

#define SBBidiTypeIsStrongOrNumber(t)       (SBBidiTypeIsStrong(t) || SBBidiTypeIsNumber(t))
#define SBBidiTypeIsNumberSeparator(t)      SBUInt8InRange(t, SBBidiTypeES, SBBidiTypeCS)
#define SBBidiTypeIsIsolateInitiator(t)     SBUInt8InRange(t, SBBidiTypeLRI, SBBidiTypeFSI)
#define SBBidiTypeIsIsolateTerminator(t)    SBBidiTypeIsEqual(t, SBBidiTypePDI)
#define SBBidiTypeIsNeutralOrIsolate(t)     SBUInt8InRange(t, SBBidiTypeWS, SBBidiTypePDI)


#define SBCodepointMax                      0x10FFFF
#define SBCodepointInRange(v, s, e)         SBUInt32InRange(v, s, e)
#define SBCodepointIsSurrogate(c)           SBCodepointInRange(c, 0xD800, 0xDFFF)
#define SBCodepointIsValid(c)               (!SBCodepointIsSurrogate(c) && (c) <= SBCodepointMax)


#define SBScriptIsCommonOrInherited(s)      ((s) <= SBScriptZYYY)

#endif

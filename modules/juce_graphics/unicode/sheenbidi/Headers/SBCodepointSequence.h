/*
 * Copyright (C) 2016-2018 Muhammad Tayyab Akram
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

#ifndef _SB_PUBLIC_CODEPOINT_SEQUENCE_H
#define _SB_PUBLIC_CODEPOINT_SEQUENCE_H

#include "SBBase.h"
#include "SBCodepoint.h"

enum {
    SBStringEncodingUTF8 = 0,  /**< An 8-bit representation of Unicode code points. */
    SBStringEncodingUTF16 = 1, /**< 16-bit UTF encoding in native endianness. */
    SBStringEncodingUTF32 = 2  /**< 32-bit UTF encoding in native endianness. */
};
typedef SBUInt32 SBStringEncoding;

typedef struct _SBCodepointSequence {
    SBStringEncoding stringEncoding; /**< The encoding of the string. */
    void *stringBuffer;              /**< The source string containing the code units. */
    SBUInteger stringLength;         /**< The length of the string in terms of code units. */
} SBCodepointSequence;

/**
 * Returns the code point before the given string index.
 *
 * @param codepointSequence
 *      The object holding the information of the string.
 * @param stringIndex
 *      The index of code unit before which to get the code point. On output, it is set to point to
 *      the first code unit of returned code point.
 * @return
 *      The code point before the given string index, or SBCodepointInvalid if stringIndex is equal 
 *      to zero or larger than actual length of source string.
 */
SBCodepoint SBCodepointSequenceGetCodepointBefore(const SBCodepointSequence *codepointSequence,
    SBUInteger *stringIndex);

/**
 * Returns the code point at the given string index.
 *
 * @param codepointSequence
 *      The object holding the information of the string.
 * @param stringIndex
 *      The index of code unit at which to get the code point. On output, it is set to point to the
 *      first code unit of next code point.
 * @return
 *      The code point at the given string index, or SBCodepointInvalid if stringIndex is larger
 *      than or equal to actual length of source string.
 */
SBCodepoint SBCodepointSequenceGetCodepointAt(const SBCodepointSequence *codepointSequence,
    SBUInteger *stringIndex);

#endif

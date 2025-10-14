/*
 * Copyright (C) 2018-2025 Muhammad Tayyab Akram
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

#ifndef _SB_PUBLIC_CODEPOINT_H
#define _SB_PUBLIC_CODEPOINT_H

#include <SheenBidi/SBBase.h>
#include <SheenBidi/SBBidiType.h>
#include <SheenBidi/SBGeneralCategory.h>
#include <SheenBidi/SBScript.h>

SB_EXTERN_C_BEGIN

/**
 * A type to represent a Unicode code point.
 */
typedef SBUInt32                    SBCodepoint;

/**
 * A value representing an invalid Unicode code point.
 */
#define SBCodepointInvalid          UINT32_MAX

/**
 * A value representing a faulty Unicode code point.
 * 
 * This value is used as a replacement for unrecognized code points during decoding.
 */
#define SBCodepointFaulty           0xFFFD

/**
 * The maximum valid Unicode code point value.
 * 
 * Unicode code points are valid in the range [0x0000, 0x10FFFF].
 */
#define SBCodepointMax              0x10FFFF

/**
 * Checks if a code point is a UTF-16 surrogate.
 * 
 * Surrogate code points lie within the range [0xD800, 0xDFFF] and are not valid Unicode scalar
 * values.
 *
 * @param c
 *      The code point to check.
 * @return
 *      `true` if the code point is a surrogate, `false` otherwise.
 */
#define SBCodepointIsSurrogate(c)   SBUInt32InRange(c, 0xD800, 0xDFFF)

/**
 * Checks if a code point is a valid Unicode scalar value.
 *
 * A code point is considered valid if:
 * - It is not a surrogate (i.e., not in the range 0xD800 to 0xDFFF)
 * - It is less than or equal to SBCodepointMax (0x10FFFF).
 *
 * @param c
 *      The code point to check.
 * @return
 *      `true` if the code point is valid, `false` otherwise.
 */
#define SBCodepointIsValid(c)       (!SBCodepointIsSurrogate(c) && (c) <= SBCodepointMax)

/**
 * Returns the bidirectional type of a Unicode code point.
 *
 * @param codepoint
 *      The code point whose bidirectional type is returned.
 * @return
 *      The bidirectional type of the specified code point.
 */
SB_PUBLIC SBBidiType SBCodepointGetBidiType(SBCodepoint codepoint);

/**
 * Returns the general category of a Unicode code point.
 *
 * @param codepoint
 *      The code point whose general category is returned.
 * @return
 *      The general category of the specified code point.
 */
SB_PUBLIC SBGeneralCategory SBCodepointGetGeneralCategory(SBCodepoint codepoint);

/**
 * Returns the mirrored code point for a given Unicode code point.
 *
 * @param codepoint
 *      The code point whose mirrored counterpart is returned.
 * @return
 *      The mirrored code point if available, or 0 if no mirror exists.
 */
SB_PUBLIC SBCodepoint SBCodepointGetMirror(SBCodepoint codepoint);

/**
 * Returns the script associated with a Unicode code point.
 *
 * @param codepoint
 *      The code point whose script is returned.
 * @return
 *      The script of the specified code point.
 */
SB_PUBLIC SBScript SBCodepointGetScript(SBCodepoint codepoint);

/**
 * Decodes the next Unicode code point from a UTF-8 encoded buffer.
 *
 * @param buffer
 *      The buffer containing UTF-8 encoded code units.
 * @param length
 *      The length of the buffer.
 * @param index
 *      The index at which decoding starts. On output, it is updated to the start of the next code
 *      point.
 * @return
 *      The decoded code point, or `SBCodepointInvalid` if `index` is out of bounds.
 */
SB_PUBLIC SBCodepoint SBCodepointDecodeNextFromUTF8(const SBUInt8 *buffer, SBUInteger length,
    SBUInteger *index);

/**
 * Decodes the previous Unicode code point from a UTF-8 encoded buffer.
 *
 * @param buffer
 *      The buffer containing UTF-8 encoded code units.
 * @param length
 *      The length of the buffer.
 * @param index
 *      The index before which decoding occurs. On output, it is updated to the start of the
 *      decoded code point.
 * @return
 *      The decoded code point, or `SBCodepointInvalid` if `index` is zero or out of bounds.
 */
SB_PUBLIC SBCodepoint SBCodepointDecodePreviousFromUTF8(const SBUInt8 *buffer, SBUInteger length,
    SBUInteger *index);

/**
 * Decodes the next Unicode code point from a UTF-16 encoded buffer.
 *
 * @param buffer
 *      The buffer containing UTF-16 encoded code units.
 * @param length
 *      The length of the buffer.
 * @param index
 *      The index at which decoding starts. On output, it is updated to the start of the next code
 *      point.
 * @return
 *      The decoded code point, or `SBCodepointInvalid` if `index` is out of bounds.
 */
SB_PUBLIC SBCodepoint SBCodepointDecodeNextFromUTF16(const SBUInt16 *buffer, SBUInteger length,
    SBUInteger *index);

/**
 * Decodes the previous Unicode code point from a UTF-16 encoded buffer.
 *
 * @param buffer
 *      The buffer containing UTF-16 encoded code units.
 * @param length
 *      The length of the buffer.
 * @param index
 *      The index before which decoding occurs. On output, it is updated to the start of the
 *      decoded code point.
 * @return
 *      The decoded code point, or `SBCodepointInvalid` if `index` is zero or out of bounds.
 */
SB_PUBLIC SBCodepoint SBCodepointDecodePreviousFromUTF16(const SBUInt16 *buffer, SBUInteger length,
    SBUInteger *index);

SB_EXTERN_C_END

#endif

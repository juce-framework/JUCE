/*
 * Copyright (C) 2018 Muhammad Tayyab Akram
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

#include "SBBase.h"
#include "SBBidiType.h"
#include "SBGeneralCategory.h"
#include "SBScript.h"

/**
 * A type to represent a unicode code point.
 */
typedef SBUInt32                    SBCodepoint;

/**
 * A value representing an invalid code point.
 */
#define SBCodepointInvalid          UINT32_MAX

/**
 * A value representing a faulty code point, used as a replacement by the decoder.
 */
#define SBCodepointFaulty           0xFFFD

/**
 * Returns the bidirectional type of a code point.
 *
 * @param codepoint
 *      The code point whose bidirectional type is returned.
 * @return
 *      The bidirectional type of specified code point.
 */
SBBidiType SBCodepointGetBidiType(SBCodepoint codepoint);

/**
 * Returns the general category of a code point.
 *
 * @param codepoint
 *      The code point whose general category is returned.
 * @return
 *      The general category of specified code point.
 */
SBGeneralCategory SBCodepointGetGeneralCategory(SBCodepoint codepoint);

/**
 * Returns the mirror of a code point.
 *
 * @param codepoint
 *      The code point whose mirror is returned.
 * @return
 *      The mirror of specified code point if available, 0 otherwise.
 */
SBCodepoint SBCodepointGetMirror(SBCodepoint codepoint);

/**
 * Returns the script of a code point.
 *
 * @param codepoint
 *      The code point whose script is returned.
 * @return
 *      The script of specified code point.
 */
SBScript SBCodepointGetScript(SBCodepoint codepoint);

#endif

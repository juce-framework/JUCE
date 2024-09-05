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

#ifndef _SB_PUBLIC_BIDI_TYPE_H
#define _SB_PUBLIC_BIDI_TYPE_H

#include "SBBase.h"

/**
 * Constants that specify the bidirectional types of a character.
 */
enum {
    SBBidiTypeNil = 0x00,

    SBBidiTypeL   = 0x01,   /**< Strong: Left-to-Right */
    SBBidiTypeR   = 0x02,   /**< Strong: Right-to-Left */
    SBBidiTypeAL  = 0x03,   /**< Strong: Right-to-Left Arabic */

    SBBidiTypeBN  = 0x04,   /**< Weak: Boundary Neutral */
    SBBidiTypeNSM = 0x05,   /**< Weak: Non-Spacing Mark */
    SBBidiTypeAN  = 0x06,   /**< Weak: Arabic Number */
    SBBidiTypeEN  = 0x07,   /**< Weak: European Number */
    SBBidiTypeET  = 0x08,   /**< Weak: European Number Terminator */
    SBBidiTypeES  = 0x09,   /**< Weak: European Number Separator */
    SBBidiTypeCS  = 0x0A,   /**< Weak: Common Number Separator */

    SBBidiTypeWS  = 0x0B,   /**< Neutral: White Space */
    SBBidiTypeS   = 0x0C,   /**< Neutral: Segment Separator */
    SBBidiTypeB   = 0x0D,   /**< Neutral: Paragraph Separator */
    SBBidiTypeON  = 0x0E,   /**< Neutral: Other Neutral */

    SBBidiTypeLRI = 0x0F,   /**< Format: Left-to-Right Isolate */
    SBBidiTypeRLI = 0x10,   /**< Format: Right-to-Left Isolate */
    SBBidiTypeFSI = 0x11,   /**< Format: First Strong Isolate */
    SBBidiTypePDI = 0x12,   /**< Format: Pop Directional Isolate */
    SBBidiTypeLRE = 0x13,   /**< Format: Left-to-Right Embedding */
    SBBidiTypeRLE = 0x14,   /**< Format: Right-to-Left Embedding */
    SBBidiTypeLRO = 0x15,   /**< Format: Left-to-Right Override */
    SBBidiTypeRLO = 0x16,   /**< Format: Right-to-Left Override */
    SBBidiTypePDF = 0x17    /**< Format: Pop Directional Formatting */
};

/**
 * A type to represent the bidirectional type of a character.
 */
typedef SBUInt8 SBBidiType;

/**
 * Checks whether specified bidirectional type is strong.
 */
#define SBBidiTypeIsStrong(t)               SBUInt8InRange(t, SBBidiTypeL, SBBidiTypeAL)

/**
 * Checks whether specified bidirectional type is weak.
 */
#define SBBidiTypeIsWeak(t)                 SBUInt8InRange(t, SBBidiTypeBN, SBBidiTypeCS)

/**
 * Checks whether specified bidirectional type is neutral.
 */
#define SBBidiTypeIsNeutral(t)              SBUInt8InRange(t, SBBidiTypeWS, SBBidiTypeON)

/**
 * Checks whether specified bidirectional type is format.
 */
#define SBBidiTypeIsFormat(t)               SBUInt8InRange(t, SBBidiTypeLRI, SBBidiTypePDF)

#endif

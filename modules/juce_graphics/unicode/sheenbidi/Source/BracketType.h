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

#ifndef _SB_INTERNAL_BRACKET_TYPE_H
#define _SB_INTERNAL_BRACKET_TYPE_H

#include "SBBase.h"

enum {
    BracketTypeNone  = 0x00,
    BracketTypeOpen  = 0x40,    /**< Opening paired bracket. */
    BracketTypeClose = 0x80,    /**< Closing paired bracket. */

    BracketTypePrimaryMask = BracketTypeOpen | BracketTypeClose,
    BracketTypeInverseMask = ~BracketTypePrimaryMask
};
typedef SBUInt8 BracketType;

#endif

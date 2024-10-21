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

#ifndef _SB_PUBLIC_GENERAL_CATEGORY_H
#define _SB_PUBLIC_GENERAL_CATEGORY_H

#include "SBBase.h"

/**
 * Constants that specify the general category of a character.
 */
enum {
    SBGeneralCategoryNil = 0x00,

    SBGeneralCategoryLU  = 0x01, /**< Letter: Uppercase Letter */
    SBGeneralCategoryLL  = 0x02, /**< Letter: Lowercase Letter */
    SBGeneralCategoryLT  = 0x03, /**< Letter: Titlecase Letter */
    SBGeneralCategoryLM  = 0x04, /**< Letter: Modifier Letter */
    SBGeneralCategoryLO  = 0x05, /**< Letter: Other Letter */

    SBGeneralCategoryMN  = 0x06, /**< Mark: Nonspacing Mark */
    SBGeneralCategoryMC  = 0x07, /**< Mark: Spacing Mark */
    SBGeneralCategoryME  = 0x08, /**< Mark: Enclosing Mark */

    SBGeneralCategoryND  = 0x09, /**< Number: Decimal Number */
    SBGeneralCategoryNL  = 0x0A, /**< Number: Letter Number */
    SBGeneralCategoryNO  = 0x0B, /**< Number: Other Number */

    SBGeneralCategoryPC  = 0x0C, /**< Punctuation: Connector Punctuation */
    SBGeneralCategoryPD  = 0x0D, /**< Punctuation: Dash Punctuation */
    SBGeneralCategoryPS  = 0x0E, /**< Punctuation: Open Punctuation */
    SBGeneralCategoryPE  = 0x0F, /**< Punctuation: Close Punctuation */
    SBGeneralCategoryPI  = 0x10, /**< Punctuation: Initial Punctuation */
    SBGeneralCategoryPF  = 0x11, /**< Punctuation: Final Punctuation */
    SBGeneralCategoryPO  = 0x12, /**< Punctuation: Other Punctuation */

    SBGeneralCategorySM  = 0x13, /**< Symbol: Math Symbol */
    SBGeneralCategorySC  = 0x14, /**< Symbol: Currency Symbol */
    SBGeneralCategorySK  = 0x15, /**< Symbol: Modifier Symbol */
    SBGeneralCategorySO  = 0x16, /**< Symbol: Other Symbol */

    SBGeneralCategoryZS  = 0x17, /**< Separator: Space Separator */
    SBGeneralCategoryZL  = 0x18, /**< Separator: Line Separator */
    SBGeneralCategoryZP  = 0x19, /**< Separator: Paragraph Separator */

    SBGeneralCategoryCC  = 0x1A, /**< Other: Control */
    SBGeneralCategoryCF  = 0x1B, /**< Other: Format */
    SBGeneralCategoryCS  = 0x1C, /**< Other: Surrogate */
    SBGeneralCategoryCO  = 0x1D, /**< Other: Private_Use */
    SBGeneralCategoryCN  = 0x1E  /**< Other: Unassigned */
};

/**
 * A type to represent the general category of a character.
 */
typedef SBUInt8 SBGeneralCategory;

/**
 * Checks whether specified general category is letter.
 */
#define SBGeneralCategoryIsLetter(gc)       SBUInt8InRange(gc, SBGeneralCategoryLU, SBGeneralCategoryLO)

/**
 * Checks whether specified general category is mark.
 */
#define SBGeneralCategoryIsMark(gc)         SBUInt8InRange(gc, SBGeneralCategoryMN, SBGeneralCategoryME)

/**
 * Checks whether specified general category is number.
 */
#define SBGeneralCategoryIsNumber(gc)       SBUInt8InRange(gc, SBGeneralCategoryND, SBGeneralCategoryNO)

/**
 * Checks whether specified general category is punctuation.
 */
#define SBGeneralCategoryIsPunctuation(gc)  SBUInt8InRange(gc, SBGeneralCategoryPC, SBGeneralCategoryPO)

/**
 * Checks whether specified general category is symbol.
 */
#define SBGeneralCategoryIsSymbol(gc)       SBUInt8InRange(gc, SBGeneralCategorySM, SBGeneralCategorySO)

/**
 * Checks whether specified general category is separator.
 */
#define SBGeneralCategoryIsSeparator(gc)    SBUInt8InRange(gc, SBGeneralCategoryZS, SBGeneralCategoryZP)

/**
 * Checks whether specified general category is other.
 */
#define SBGeneralCategoryIsOther(gc)        SBUInt8InRange(gc, SBGeneralCategoryCC, SBGeneralCategoryCN)

#endif

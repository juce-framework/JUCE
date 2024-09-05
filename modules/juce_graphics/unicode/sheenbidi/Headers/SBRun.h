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

#ifndef _SB_PUBLIC_RUN_H
#define _SB_PUBLIC_RUN_H

#include "SBBase.h"

/**
 * A structure containing the information of a sequence of characters having the same embedding
 * level.
 */
typedef struct _SBRun {
    SBUInteger offset; /**< The index to the first code unit of the run in source string. */
    SBUInteger length; /**< The number of code units covering the length of the run. */
    SBLevel level;     /**< The embedding level of the run. */
} SBRun;

#endif

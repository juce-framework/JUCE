/*
 * Copyright (C) 2014-2018 Muhammad Tayyab Akram
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

#ifndef _SB_PUBLIC_LINE_H
#define _SB_PUBLIC_LINE_H

#include "SBBase.h"
#include "SBRun.h"

typedef struct _SBLine *SBLineRef;

/**
 * Returns the index to the first code unit of the line in source string.
 *
 * @param line
 *      The line whose offset is returned.
 * @return
 *      The offset of the line passed in.
 */
SBUInteger SBLineGetOffset(SBLineRef line);

/**
 * Returns the number of code units coverting the length of the line.
 *
 * @param line
 *      The line whose length is returned.
 * @return
 *      The length of the line passed in.
 */
SBUInteger SBLineGetLength(SBLineRef line);

/**
 * Returns the number of runs in the line.
 *
 * @param line
 *      The line whose run count is returned.
 * @return
 *      The number of runs in the line passed in.
 */
SBUInteger SBLineGetRunCount(SBLineRef line);

/**
 * Returns a direct pointer to the run array, stored in the line.
 *
 * @param line
 *      The line from which to access the runs.
 * @return
 *      A valid pointer to an array of SBRun structures. 
 */
const SBRun *SBLineGetRunsPtr(SBLineRef line);

/**
 * Increments the reference count of a line object.
 *
 * @param line
 *      The line object whose reference count will be incremented.
 * @return
 *      The same line object passed in as the parameter.
 */
SBLineRef SBLineRetain(SBLineRef line);

/**
 * Decrements the reference count of a line object. The object will be deallocated when its
 * reference count reaches zero.
 *
 * @param line
 *      The line object whose reference count will be decremented.
 */
void SBLineRelease(SBLineRef line);

#endif

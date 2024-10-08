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

#ifndef _SB_PUBLIC_MIRROR_LOCATOR_H
#define _SB_PUBLIC_MIRROR_LOCATOR_H

#include "SBBase.h"
#include "SBCodepoint.h"
#include "SBLine.h"

typedef struct _SBMirrorLocator *SBMirrorLocatorRef;

/**
 * A structure containing the information about a code point having Bidi_Mirrored property.
 */
typedef struct _SBMirrorAgent {
    SBUInteger index;      /**< The absolute index of the code point. */
    SBCodepoint mirror;    /**< The mirrored code point. */
    SBCodepoint codepoint; /**< The actual code point. */
} SBMirrorAgent;

/**
 * Creates a mirror locator object which can be used to find mirrors in a line.
 *
 * @return
 *      A reference to a mirror locator object.
 */
SBMirrorLocatorRef SBMirrorLocatorCreate(void);

/**
 * Loads a line in the locator so that its mirror can be located.
 *
 * @param locator
 *      The locator in which the line will be loaded.
 * @param line
 *      The line which will be loaded in the locator.
 * @param stringBuffer
 *      The string buffer from which the line's algorithm was created.
 */
void SBMirrorLocatorLoadLine(SBMirrorLocatorRef locator, SBLineRef line, void *stringBuffer);

/**
 * Returns the agent containing the information of current located mirror.
 *
 * @param locator
 *      The locator whose agent is returned.
 */
const SBMirrorAgent *SBMirrorLocatorGetAgent(SBMirrorLocatorRef locator);

/**
 * Instructs the locator to find next mirror in the loaded line.
 *
 * @param locator
 *      The locator whom you want to instruct.
 * @return
 *      SBTrue if another mirror is available, SBFalse otherwise.
 * @note
 *      The locator will be reset after locating last mirror.
 */
SBBoolean SBMirrorLocatorMoveNext(SBMirrorLocatorRef locator);

/**
 * Instructs the locator to reset itself so that mirrors of the loaded line can be obatained from
 * the beginning.
 *
 * @param locator
 *      The locator whom you want to reset.
 */
void SBMirrorLocatorReset(SBMirrorLocatorRef locator);

/**
 * Increments the reference count of a mirror locator object.
 *
 * @param locator
 *      The mirror locator object whose reference count will be incremented.
 * @return
 *      The same mirror locator object passed in as the parameter.
 */
SBMirrorLocatorRef SBMirrorLocatorRetain(SBMirrorLocatorRef locator);

/**
 * Decrements the reference count of a mirror locator object. The object will be deallocated when
 * its reference count reaches zero.
 *
 * @param locator
 *      The mirror locator object whose reference count will be decremented.
 */
void SBMirrorLocatorRelease(SBMirrorLocatorRef locator);

#endif

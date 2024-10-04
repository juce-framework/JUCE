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

#ifndef _SB_PUBLIC_PARAGRAPH_H
#define _SB_PUBLIC_PARAGRAPH_H

#include "SBBase.h"
#include "SBLine.h"

typedef struct _SBParagraph *SBParagraphRef;

/**
 * Returns the index to the first code unit of the paragraph in source string.
 *
 * @param paragraph
 *      The paragraph whose offset is returned.
 * @return
 *      The offset of the paragraph passed in.
 */
SBUInteger SBParagraphGetOffset(SBParagraphRef paragraph);

/**
 * Returns the number of code units covering the length of the paragraph.
 *
 * @param paragraph
 *      The paragraph whose length is returned.
 * @return
 *      The length of the paragraph passed in.
 */
SBUInteger SBParagraphGetLength(SBParagraphRef paragraph);

/**
 * Returns the base level of the paragraph.
 *
 * @param paragraph
 *      The paragraph whose base level is returned.
 * @return
 *      The base level of the paragraph passed in.
 */
SBLevel SBParagraphGetBaseLevel(SBParagraphRef paragraph);

/**
 * Returns a direct pointer to the embedding levels, stored in the paragraph.
 *
 * @param paragraph
 *      The paragraph from which to access the embedding levels.
 * @return
 *      A valid pointer to an array of SBLevel structures.
 */
const SBLevel *SBParagraphGetLevelsPtr(SBParagraphRef paragraph);

/**
 * Creates a line object of specified range by applying rules L1-L2 of Unicode Bidirectional
 * Algorithm.
 *
 * @param paragraph
 *      The paragraph that creates the line.
 * @param lineOffset
 *      The index to the first code unit of the line in source string. It should occur within the
 *      range of paragraph.
 * @param lineLength
 *      The number of code units covering the length of the line.
 * @return
 *      A reference to a line object if the call was successful, NULL otherwise.
 */
SBLineRef SBParagraphCreateLine(SBParagraphRef paragraph, SBUInteger lineOffset, SBUInteger lineLength);

/**
 * Increments the reference count of a paragraph object.
 *
 * @param paragraph
 *      The paragraph object whose reference count will be incremented.
 * @return
 *      The same paragraph object passed in as the parameter.
 */
SBParagraphRef SBParagraphRetain(SBParagraphRef paragraph);

/**
 * Decrements the reference count of a paragraph object. The object will be deallocated when its
 * reference count reaches zero.
 *
 * @param paragraph
 *      The paragraph object whose reference count will be decremented.
 */
void SBParagraphRelease(SBParagraphRef paragraph);

#endif

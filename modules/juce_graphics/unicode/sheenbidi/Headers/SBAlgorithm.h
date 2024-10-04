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

#ifndef _SB_PUBLIC_ALGORITHM_H
#define _SB_PUBLIC_ALGORITHM_H

#include "SBBase.h"
#include "SBBidiType.h"
#include "SBCodepointSequence.h"
#include "SBParagraph.h"

typedef struct _SBAlgorithm *SBAlgorithmRef;

/**
 * Creates an algorithm object for the specified code point sequence. The source string inside the
 * code point sequence should not be freed until the algorithm object is in use.
 *
 * @param codepointSequence
 *      The code point sequence to apply bidirectional algorithm on.
 * @return
 *      A reference to an algorithm object if the call was successful, NULL otherwise.
 */
SBAlgorithmRef SBAlgorithmCreate(const SBCodepointSequence *codepointSequence);

/**
 * Returns a direct pointer to the bidirectional types of code units, stored in the algorithm
 * object.
 *
 * @param algorithm
 *      The algorithm object from which to access the bidirectional types of code units.
 * @return
 *      A valid pointer to an array of SBBidiType structures, whose length will be equal to that of
 *      string buffer.
 */
const SBBidiType *SBAlgorithmGetBidiTypesPtr(SBAlgorithmRef algorithm);

/**
 * Determines the boundary of first paragraph within the specified range.
 *
 * The boundary of the paragraph occurs after a code point whose bidirectional type is Paragraph
 * Separator (B), or at the suggestedLength if no such code point exists before it. The exception to
 * this rule is when a Carriage Return (CR) is followed by a Line Feed (LF). Both CR and LF are
 * paragraph separators, but in that case, the boundary of the paragraph is considered after LF code
 * point.
 *
 * @param algorithm
 *      The algorithm object to use for determining paragraph boundary.
 * @param paragraphOffset
 *      The index to the first code unit of the paragraph in source string.
 * @param suggestedLength
 *      The number of code units covering the suggested length of the paragraph.
 * @param acutalLength
 *      The actual length of the first paragraph, including the paragraph separator, within the
 *      given range.
 * @param separatorLength
 *      On output, the length of paragraph separator. This parameter can be set to NULL if not
 *      needed.
 */
void SBAlgorithmGetParagraphBoundary(SBAlgorithmRef algorithm,
    SBUInteger paragraphOffset, SBUInteger suggestedLength,
    SBUInteger *acutalLength, SBUInteger *separatorLength);

/**
 * Creates a paragraph object processed with Unicode Bidirectional Algorithm.
 *
 * This function processes only first paragraph starting at paragraphOffset with length less than or
 * equal to suggestedLength, in accordance with Rule P1 of Unicode Bidirectional Algorithm.
 *
 * The paragraph level is determined by applying Rules P2-P3 and embedding levels are resolved by
 * applying Rules X1-I2.
 *
 * @param algorithm
 *      The algorithm object to use for creating the desired paragraph.
 * @param paragraphOffset
 *      The index to the first code unit of the paragraph in source string.
 * @param suggestedLength
 *      The number of code units covering the suggested length of the paragraph.
 * @param baseLevel
 *      The desired base level of the paragraph. Rules P2-P3 would be ignored if it is neither
 *      SBLevelDefaultLTR nor SBLevelDefaultRTL.
 * @return
 *      A reference to a paragraph object if the call was successful, NULL otherwise.
 */
SBParagraphRef SBAlgorithmCreateParagraph(SBAlgorithmRef algorithm,
    SBUInteger paragraphOffset, SBUInteger suggestedLength, SBLevel baseLevel);

/**
 * Increments the reference count of an algorithm object.
 *
 * @param algorithm
 *      The algorithm object whose reference count will be incremented.
 * @return
 *      The same algorithm object passed in as the parameter.
 */
SBAlgorithmRef SBAlgorithmRetain(SBAlgorithmRef algorithm);

/**
 * Decrements the reference count of an algorithm object. The object will be deallocated when its
 * reference count reaches zero.
 *
 * @param algorithm
 *      The algorithm object whose reference count will be decremented.
 */
void SBAlgorithmRelease(SBAlgorithmRef algorithm);

#endif

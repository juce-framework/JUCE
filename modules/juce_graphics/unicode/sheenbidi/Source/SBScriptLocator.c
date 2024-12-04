/*
 * Copyright (C) 2018-2022 Muhammad Tayyab Akram
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

#include <stddef.h>
#include <stdlib.h>

#include "GeneralCategoryLookup.h"
#include "PairingLookup.h"
#include "SBBase.h"
#include "SBCodepointSequence.h"
#include "ScriptLookup.h"
#include "ScriptStack.h"
#include "SBScriptLocator.h"

static SBBoolean IsSimilarScript(SBScript lhs, SBScript rhs)
{
    return SBScriptIsCommonOrInherited(lhs)
        || SBScriptIsCommonOrInherited(rhs)
        || lhs == rhs;
}

SBScriptLocatorRef SBScriptLocatorCreate(void)
{
    SBScriptLocatorRef locator = malloc(sizeof(SBScriptLocator));

    if (locator) {
        locator->_codepointSequence.stringEncoding = SBStringEncodingUTF8;
        locator->_codepointSequence.stringBuffer = NULL;
        locator->_codepointSequence.stringLength = 0;
        locator->retainCount = 1;

        SBScriptLocatorReset(locator);
    }

    return locator;
}

void SBScriptLocatorLoadCodepoints(SBScriptLocatorRef locator, const SBCodepointSequence *codepointSequence)
{
    locator->_codepointSequence = *codepointSequence;
    SBScriptLocatorReset(locator);
}

const SBScriptAgent *SBScriptLocatorGetAgent(SBScriptLocatorRef locator)
{
    return &locator->agent;
}

static void ResolveScriptRun(SBScriptLocatorRef locator, SBUInteger offset)
{
    const SBCodepointSequence *sequence = &locator->_codepointSequence;
    ScriptStackRef stack = &locator->_scriptStack;
    SBScript result = SBScriptZYYY;
    SBUInteger current = offset;
    SBUInteger next = offset;
    SBCodepoint codepoint;

    /* Iterate over the code points of specified string buffer. */
    while ((codepoint = SBCodepointSequenceGetCodepointAt(sequence, &next)) != SBCodepointInvalid) {
        SBBoolean isStacked = SBFalse;
        SBScript script;

        script = LookupScript(codepoint);

        /* Handle paired punctuations in case of a common script. */
        if (script == SBScriptZYYY) {
            SBGeneralCategory generalCategory = LookupGeneralCategory(codepoint);

            /* Check if current code point is an open punctuation. */
            if (generalCategory == SBGeneralCategoryPS) {
                SBCodepoint mirror = LookupMirror(codepoint);
                if (mirror) {
                    /* A closing pair exists for this punctuation, so push it onto the stack. */
                    ScriptStackPush(stack, result, mirror);
                }
            }
            /* Check if current code point is a close punctuation. */
            else if (generalCategory == SBGeneralCategoryPE) {
                SBBoolean isMirrored = (LookupMirror(codepoint) != 0);
                if (isMirrored) {
                    /* Find the matching entry in the stack, while popping the unmatched ones. */
                    while (!ScriptStackIsEmpty(stack)) {
                        SBCodepoint mirror = ScriptStackGetMirror(stack);
                        if (mirror != codepoint) {
                            ScriptStackPop(stack);
                        } else {
                            break;
                        }
                    }

                    if (!ScriptStackIsEmpty(stack)) {
                        isStacked = SBTrue;
                        /* Paired punctuation match the script of enclosing text. */
                        script = ScriptStackGetScript(stack);
                    }
                }
            }
        }

        if (IsSimilarScript(result, script)) {
            if (SBScriptIsCommonOrInherited(result) && !SBScriptIsCommonOrInherited(script)) {
                /* Set the concrete script of this code point as the result. */
                result = script;
                /* Seal the pending punctuations with the result. */
                ScriptStackSealPairs(stack, result);
            }

            if (isStacked) {
                /* Pop the paired punctuation from the stack. */
                ScriptStackPop(stack);
            }
        } else {
            /* The current code point has a different script, so finish the run. */
            break;
        }

        current = next;
    }

    ScriptStackLeavePairs(stack);

    /* Set the run info in agent. */
    locator->agent.offset = offset;
    locator->agent.length = current - offset;
    locator->agent.script = result;
}

SBBoolean SBScriptLocatorMoveNext(SBScriptLocatorRef locator)
{
    SBUInteger offset = locator->agent.offset + locator->agent.length;

    if (offset < locator->_codepointSequence.stringLength) {
        ResolveScriptRun(locator, offset);
        return SBTrue;
    }

    SBScriptLocatorReset(locator);
    return SBFalse;
}

void SBScriptLocatorReset(SBScriptLocatorRef locator)
{
    ScriptStackReset(&locator->_scriptStack);
    locator->agent.offset = 0;
    locator->agent.length = 0;
    locator->agent.script = SBScriptNil;
}

SBScriptLocatorRef SBScriptLocatorRetain(SBScriptLocatorRef locator)
{
    if (locator) {
        locator->retainCount += 1;
    }

    return locator;
}

void SBScriptLocatorRelease(SBScriptLocatorRef locator)
{
    if (locator && --locator->retainCount == 0) {
        free(locator);
    }
}

/*
 * Copyright (C) 2018-2019 Muhammad Tayyab Akram
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

#include <SBConfig.h>

#include "SBAssert.h"
#include "SBBase.h"
#include "ScriptStack.h"

SB_INTERNAL void ScriptStackReset(ScriptStackRef stack)
{
    stack->top = -1;
    stack->count = 0;
    stack->open = 0;
}

SB_INTERNAL void ScriptStackPush(ScriptStackRef stack, SBScript script, SBCodepoint mirror)
{
    stack->count = SBNumberLimitIncrement(stack->count, _SBScriptStackCapacity);
    stack->open = SBNumberLimitIncrement(stack->open, _SBScriptStackCapacity);

    stack->top = SBNumberRingIncrement(stack->top, _SBScriptStackCapacity);
    stack->_elements[stack->top].script = script;
    stack->_elements[stack->top].mirror = mirror;
}

SB_INTERNAL void ScriptStackPop(ScriptStackRef stack)
{
    /* There must be at least one entry in the stack. */
    SBAssert(stack->count > 0);

    stack->count -= 1;
    stack->open = SBNumberLimitDecrement(stack->open, 0);
    stack->top = SBNumberRingDecrement(stack->top, _SBScriptStackCapacity);

    if (ScriptStackIsEmpty(stack)) {
        stack->top = -1;
    }
}

SB_INTERNAL void ScriptStackLeavePairs(ScriptStackRef stack)
{
    stack->open = 0;
}

SB_INTERNAL void ScriptStackSealPairs(ScriptStackRef stack, SBScript script)
{
    SBInteger index = SBNumberRingSubtract(stack->top, (SBInteger)stack->open, _SBScriptStackCapacity);

    while (stack->open) {
        index = SBNumberRingIncrement(index, _SBScriptStackCapacity);
        stack->_elements[index].script = script;
        stack->open -= 1;
    }
}

SB_INTERNAL SBBoolean ScriptStackIsEmpty(ScriptStackRef stack)
{
    return (stack->count == 0);
}

SB_INTERNAL SBScript ScriptStackGetScript(ScriptStackRef stack)
{
    return stack->_elements[stack->top].script;
}

SB_INTERNAL SBCodepoint ScriptStackGetMirror(ScriptStackRef stack)
{
    return stack->_elements[stack->top].mirror;
}

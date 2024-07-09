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

#include <juce_graphics/unicode/sheenbidi/Headers/SBConfig.h>

#include "SBBase.h"
#include "BidiChain.h"

SB_INTERNAL void BidiChainInitialize(BidiChainRef chain,
    SBBidiType *types, SBLevel *levels, BidiLink *links)
{
    chain->types = types;
    chain->levels = levels;
    chain->links = links;
    chain->roller = 0;
    chain->last = 0;

    /* Make first link empty. */
    chain->types[0] = SBBidiTypeNil;
    chain->levels[0] = SBLevelInvalid;
    chain->links[0] = BidiLinkNone;
}

SB_INTERNAL void BidiChainAdd(BidiChainRef chain, SBBidiType type, SBUInteger length)
{
    BidiLink last = chain->last;
    BidiLink current = last + (SBUInt32)length;

    chain->types[current] = type;
    chain->links[current] = chain->roller;

    chain->links[last] = current;
    chain->last = current;
}

SB_INTERNAL SBBoolean BidiChainIsSingle(BidiChainRef chain, BidiLink link)
{
    BidiLink next = chain->links[link];

    /* Check the type of in between code units. */
    while (++link != next) {
        if (chain->types[link] != SBBidiTypeBN) {
            return SBFalse;
        }
    }

    return SBTrue;
}

SB_INTERNAL SBBidiType BidiChainGetType(BidiChainRef chain, BidiLink link)
{
    return chain->types[link];
}

SB_INTERNAL void BidiChainSetType(BidiChainRef chain, BidiLink link, SBBidiType type)
{
    chain->types[link] = type;
}

SB_INTERNAL SBLevel BidiChainGetLevel(BidiChainRef chain, BidiLink link)
{
    return chain->levels[link];
}

SB_INTERNAL void BidiChainSetLevel(BidiChainRef chain, BidiLink link, SBLevel level)
{
    chain->levels[link] = level;
}

SB_INTERNAL BidiLink BidiChainGetNext(BidiChainRef chain, BidiLink link)
{
    return chain->links[link];
}

SB_INTERNAL void BidiChainSetNext(BidiChainRef chain, BidiLink link, BidiLink next)
{
    chain->links[link] = next;
}

SB_INTERNAL void BidiChainAbandonNext(BidiChainRef chain, BidiLink link)
{
    BidiLink next = chain->links[link];
    BidiLink limit = chain->links[next];

    chain->links[link] = limit;
}

SB_INTERNAL SBBoolean BidiChainMergeIfEqual(BidiChainRef chain, BidiLink first, BidiLink second)
{
    if (chain->types[first] == chain->types[second]
        && chain->levels[first] == chain->levels[second]) {
        chain->links[first] = chain->links[second];
        return SBTrue;
    }

    return SBFalse;
}

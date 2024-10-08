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

#ifndef _SB_INTERNAL_MIRROR_LOCATOR_H
#define _SB_INTERNAL_MIRROR_LOCATOR_H

#include <juce_graphics/unicode/sheenbidi/Headers/SBBase.h>
#include <juce_graphics/unicode/sheenbidi/Headers/SBMirrorLocator.h>
#include <juce_graphics/unicode/sheenbidi/Headers/SBLine.h>

typedef struct _SBMirrorLocator {
    SBLineRef _line;
    SBUInteger _runIndex;
    SBUInteger _stringIndex;
    SBMirrorAgent agent;
    SBUInteger retainCount;
} SBMirrorLocator;

#endif

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

#ifndef _SB_INTERNAL_RUN_EXTREMA_H
#define _SB_INTERNAL_RUN_EXTREMA_H

#include "SBBase.h"

enum {
    RunExtremaLeadingL  = SBBidiTypeL << 0,
    RunExtremaLeadingR  = SBBidiTypeR << 0,
    
    RunExtremaTrailingL = SBBidiTypeL << 4,
    RunExtremaTrailingR = SBBidiTypeR << 4
};
typedef SBUInt8 RunExtrema;

#define RunExtremaMake(sor, eor)            \
(RunExtrema)                                \
(                                           \
   ((sor) << 0)                             \
 | ((eor) << 4)                             \
)

#define RunExtrema_SOR(e)                   \
(RunExtrema)                                \
(                                           \
 (e) & 0xF                                  \
)

#define RunExtrema_EOR(e)                   \
(RunExtrema)                                \
(                                           \
 (e) >> 4                                   \
)

#endif

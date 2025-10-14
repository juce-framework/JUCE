/*
 * Copyright (C) 2025 Muhammad Tayyab Akram
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

#ifndef _SB_PUBLIC_VERSION_H
#define _SB_PUBLIC_VERSION_H

#include <SheenBidi/SBBase.h>

SB_EXTERN_C_BEGIN

#define SHEENBIDI_VERSION_MAJOR     2
#define SHEENBIDI_VERSION_MINOR     9
#define SHEENBIDI_VERSION_PATCH     0
#define SHEENBIDI_VERSION_STRING    "2.9.0"

/**
 * Returns the version string of the SheenBidi library.
 *
 * This function returns a constant null-terminated string representing the version of the linked
 * SheenBidi library, in the format "MAJOR.MINOR.PATCH".
 *
 * @return A string representing the version (e.g. "2.9.0").
 */
SB_PUBLIC const char *SBVersionGetString(void);

SB_EXTERN_C_END

#endif

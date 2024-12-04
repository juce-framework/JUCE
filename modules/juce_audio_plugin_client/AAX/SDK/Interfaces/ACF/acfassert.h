/***********************************************************************

	This file is part of the Avid AAX SDK.

	The AAX SDK is subject to commercial or open-source licensing.

	By using the AAX SDK, you agree to the terms of both the Avid AAX SDK License
	Agreement and Avid Privacy Policy.

	AAX SDK License: https://developer.avid.com/aax
	Privacy Policy: https://www.avid.com/legal/privacy-policy-statement

	Or: You may also use this code under the terms of the GPL v3 (see
	www.gnu.org/licenses).

	THE AAX SDK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
	EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
	DISCLAIMED.

	Copyright (c) 2004, 2024 Avid Technology, Inc. All rights reserved.

************************************************************************/




/*!
 \file acfassert.h
	
 \brief Wrapper for AVX2 assertions. 
 \remarks The default definition uses
 ansi assert so it does not require c++ exceptions to be
 enabled.
 This also adds guards to assert.h so that it can safely
 be included in a header file (CW bug).
*/
#ifndef ACFASSERT

#ifndef assert

#if defined(__MACH__) && !defined(__GNUC__)
// This configuration for /usr/include/gcc/darwin/2.5.2/assert.h uses
// prinf and abort WITHOUT including the definitions. transdel-2001-SEPT-21.
#include <stdio.h>    // printf()
#include <stdlib.h>   // abort()
#endif

#include "assert.h"
#endif

#define ACFASSERT assert

#endif

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


#ifndef defineacfuid_h
#define defineacfuid_h

/*!
    \file defineacfuid.h
	\brief Defines DEFINE_ACFUID.

    \remarks This header provides the definition for the DEFINE_ACFUID 
	macro that is used to either declare a acfUID as a forward declaration
	or initializes, defines, the acfUID symbol.
 */

#include "acfbasetypes.h"


/*!
	\def DEFINE_ACFUID(type, name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8)
	\brief Defines a type of acfUID structure.

	This macro is used to both declare and define acfUID constants. If the
	symbol INITACFIDS is defined then the constant is fully defined, otherwise the
	constant is just declared.

	\note To avoid duplicate symbol definitions the symbol INITACFIDS should only
	be defined in one source file of an executable module.
*/
#ifndef DEFINE_ACFUID
#define DEFINE_ACFUID(type, name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
	ACFEXTERN_C const type name
#endif // DEFINE_ACFUID


#if defined(INITACFIDS) || defined(INITAVXIDS)
#include "initacfuid.h"
#endif

#endif // defineacfuid_h


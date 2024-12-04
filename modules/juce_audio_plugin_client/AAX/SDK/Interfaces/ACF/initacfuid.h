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

#ifndef initacfuid_h
#define initacfuid_h

#include "acfbasetypes.h"

/*!
    \file
	\brief Include initacfuid.h to enable acfUID initialization.  This must be done 
    once per exe/dll.  After this file, include one or more of the acfUID definition files.    
*/

#ifdef DEFINE_ACFUID
#undef DEFINE_ACFUID
#endif

/*!
	Defines a uid structure of type \ref acfUID and initializes member data.
	Generally you will want to use this macro to define all uid's within a definition.
	\warning The uid must be unique. \n
	\e Example: \n 
	\verbatim DEFINE_ACFUID(acfUID, ACF_MyEffectUID, 0x51b30d8a, 0xf54, 0x4092, 0xba, 0x41, 0x46, 0xdc, 0x91, 0xee, 0xf7, 0xf4); \endverbatim
 */
#define DEFINE_ACFUID(type, name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
    ACFEXTERN_C const type name = { l, w1, w2, {b1, b2,  b3,  b4,  b5,  b6,  b7,  b8} }

#endif // initacfuid_h

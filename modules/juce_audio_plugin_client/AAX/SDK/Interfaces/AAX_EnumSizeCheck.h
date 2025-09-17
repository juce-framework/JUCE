/*================================================================================================*/
/*
 *
 *	Copyright 2025 Avid Technology, Inc.
 *	All rights reserved.
 *	
 *	This file is part of the Avid AAX SDK.
 *	
 *	The AAX SDK is subject to commercial or open-source licensing.
 *	
 *	By using the AAX SDK, you agree to the terms of both the Avid AAX SDK License
 *	Agreement and Avid Privacy Policy.
 *	
 *	AAX SDK License: https://developer.avid.com/aax
 *	Privacy Policy: https://www.avid.com/legal/privacy-policy-statement
 *	
 *	Or: You may also use this code under the terms of the GPL v3 (see
 *	www.gnu.org/licenses).
 *	
 *	THE AAX SDK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
 *	EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
 *	DISCLAIMED.
 *
 */

/**  
 *	\file AAX_EnumSizeCheck.h
 *
 *	\brief Utility for verifying the underlying type size of %AAX enums
 *
 */ 
/*================================================================================================*/

#ifndef AAX_ENUMSIZECHECK_H
#define AAX_ENUMSIZECHECK_H

#include "AAX_EnvironmentUtilities.h"

#ifndef _TMS320C6X
#include <cstdint>
#endif

/** @def AAX_ENUM_SIZE_CHECK
 @brief Macro to ensure enum type consistency across binaries

 Verifies that the underlying type for checked %AAX enums is always 4 bytes.
 */

#ifndef _TMS320C6X
 #if defined(AAX_CPP11_SUPPORT)
	 #define AAX_ENUM_SIZE_CHECK(x) static_assert(sizeof(x) == sizeof(uint32_t), "Enum size check failed for " #x)
 #else
	 // force a compiler error if the size is not 4 bytes
	 #define AAX_ENUM_SIZE_CHECK(x) extern int __enumSizeCheck[ 2*(sizeof(uint32_t)==sizeof(x)) - 1]
 #endif
#else
 #define AAX_ENUM_SIZE_CHECK(x)
#endif

#endif // AAX_ENUMSIZECHECK_H

/*================================================================================================*/
/*
 *
 *	Copyright 2023-2024 Avid Technology, Inc.
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
 *	\file   AAX_SessionDocumentTypes.h
 */ 
/*================================================================================================*/

#pragma once
#ifndef AAX_SessionDocumentTypes_H
#define AAX_SessionDocumentTypes_H

#include "AAX.h"
#include <stdint.h>

AAX_CONSTEXPR AAX_CTypeID kAAX_DataBufferType_TempoBreakpointArray = 'AXtB';

#include AAX_ALIGN_FILE_BEGIN
#include AAX_ALIGN_FILE_HOST
#include AAX_ALIGN_FILE_END

struct AAX_CTempoBreakpoint
{
	int64_t mSampleLocation{0};
	float mValue{0.f};
};
static_assert(16 == sizeof(AAX_CTempoBreakpoint), "Unexpected size for AAX_CTempoBreakpoint");

#include AAX_ALIGN_FILE_BEGIN
#include AAX_ALIGN_FILE_RESET
#include AAX_ALIGN_FILE_END

#endif // AAX_SessionDocumentTypes_H

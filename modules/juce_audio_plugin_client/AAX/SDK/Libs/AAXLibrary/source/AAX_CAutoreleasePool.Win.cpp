/*================================================================================================*/
/*
 *	Copyright 2013-2015, 2023-2024 Avid Technology, Inc.
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
 *	\file  AAX_CAutoreleasePool.Win.cpp
 *	
 *	\brief Windows implementation of a basic Autorelease pool helper utility
 *
 */ 
/*================================================================================================*/

#include "AAX_CAutoreleasePool.h"
#include "stddef.h"

AAX_CAutoreleasePool::AAX_CAutoreleasePool() : mAutoreleasePool(NULL)
{
	// Nothing to do on Windows yet...maybe someday if Apple ever opens objective-C libs on the Windows platform.
}

AAX_CAutoreleasePool::~AAX_CAutoreleasePool()
{
	// Nothing to do on Windows yet...maybe someday if Apple ever opens objective-C libs on the Windows platform.
}

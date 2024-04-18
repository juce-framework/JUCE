/*================================================================================================*/
/*
 *
 *	Copyright 2015, 2023-2024 Avid Technology, Inc.
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
 *  \file AAX_IContainer.h
 *
 *	\brief Abstract container interface
 *
 */
/*================================================================================================*/
/// @cond ignore
#ifndef AAX_ICONTAINER_H
#define AAX_ICONTAINER_H
/// @endcond


/** Abstract container interface
 */
class AAX_IContainer
{
public:
	virtual ~AAX_IContainer() {}
	
public:
	enum EStatus
	{
		eStatus_Success = 0          ///< Operation succeeded
		,eStatus_Overflow = 1        ///< Internal buffer overflow
		,eStatus_NotInitialized = 2  ///< Uninitialized container
		,eStatus_Unavailable = 3     ///< An internal resource was not available
		,eStatus_Unsupported = 4     ///< Operation is unsupported
	};
	
public:
	/** Clear the container
	 */
	virtual void Clear() = 0;
};

/// @cond ignore
#endif /* defined(AAX_ICONTAINER_H) */
/// @endcond

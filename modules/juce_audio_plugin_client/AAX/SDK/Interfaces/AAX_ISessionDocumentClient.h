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
 *	\file  AAX_ISessionDocumentClient.h
 */ 
/*================================================================================================*/

#pragma once
#ifndef AAX_ISessionDocumentClient_H
#define AAX_ISessionDocumentClient_H

#include "AAX_IACFSessionDocumentClient.h"
#include "AAX.h"
#include "CACFUnknown.h"


/**
 * \brief Interface representing a client of the session document interface
 * 
 * For example, a plug-in implementation that makes calls on the session
 * document interface provided by the host.
 */
class AAX_ISessionDocumentClient :	public AAX_IACFSessionDocumentClient,
									public CACFUnknown
{	
public:
	ACF_DECLARE_STANDARD_UNKNOWN()
	
	ACFMETHOD(InternalQueryInterface)(const acfIID & riid, void **ppvObjOut) override;
	
	// CACFUnknown does not support operator=()
	AAX_DELETE(AAX_ISessionDocumentClient& operator= (const AAX_ISessionDocumentClient&));
};

#endif //AAX_ISessionDocumentClient_H

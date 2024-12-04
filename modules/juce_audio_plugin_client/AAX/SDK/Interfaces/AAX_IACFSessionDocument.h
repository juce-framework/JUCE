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
 *	\file   AAX_IACFSessionDocument.h
 */ 
/*================================================================================================*/

#pragma once
#ifndef AAX_IACFSessionDocument_H
#define AAX_IACFSessionDocument_H

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wnon-virtual-dtor"
#endif

#include "AAX_UIDs.h"
#include "AAX.h"
#include "acfunknown.h"

/**
 * \brief Interface representing information in a host session document
 * 
 * Plug-in implementations should use \ref AAX_ISessionDocument , which
 * provides specific convenience methods for supported data types.
 */
class AAX_IACFSessionDocument : public IACFUnknown
{
public:
	/**
	 * \brief Get data from the document
	 * 
	 * \copydetails AAX_ISessionDocument::GetDocumentData
	 */
	virtual AAX_Result GetDocumentData(AAX_DocumentData_UID const & inDataType, IACFUnknown ** outData) = 0;
};

#ifdef __clang__
#pragma clang diagnostic pop
#endif

#endif // AAX_IACFSessionDocument_H

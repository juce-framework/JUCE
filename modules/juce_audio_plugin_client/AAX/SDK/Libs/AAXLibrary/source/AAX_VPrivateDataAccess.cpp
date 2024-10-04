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
/*================================================================================================*/

#include "AAX_VPrivateDataAccess.h"
#include "AAX_UIDs.h"

// ******************************************************************************************
// METHOD:	AAX_VPrivateDataAccess
// ******************************************************************************************
AAX_VPrivateDataAccess::AAX_VPrivateDataAccess( IACFUnknown* pUnknown )
:	mIPrivateDataAccess(NULL)
{
	if ( pUnknown )
	{
		pUnknown->QueryInterface(IID_IAAXPrivateDataAccessV1, (void **)&mIPrivateDataAccess);
	}	
}

// ******************************************************************************************
// METHOD:	~AAX_VPrivateDataAccess
// ******************************************************************************************
AAX_VPrivateDataAccess::~AAX_VPrivateDataAccess()
{
}

// ******************************************************************************************
// METHOD:	ReadPortDirect
// ******************************************************************************************
AAX_Result AAX_VPrivateDataAccess::ReadPortDirect( AAX_CFieldIndex inFieldIndex, const uint32_t inOffset, const uint32_t inSize, void* outBuffer )
{
	if ( mIPrivateDataAccess )
		return mIPrivateDataAccess->ReadPortDirect ( inFieldIndex, inOffset, inSize, outBuffer );

	return AAX_ERROR_NULL_OBJECT;
}

// ******************************************************************************************
// METHOD:	WritePortDirect
// ******************************************************************************************
AAX_Result AAX_VPrivateDataAccess::WritePortDirect( AAX_CFieldIndex inFieldIndex, const uint32_t inOffset, const uint32_t inSize, const void* inBuffer )
{
	if ( mIPrivateDataAccess )
		return mIPrivateDataAccess->WritePortDirect ( inFieldIndex, inOffset, inSize, inBuffer );

	return AAX_ERROR_NULL_OBJECT;
}








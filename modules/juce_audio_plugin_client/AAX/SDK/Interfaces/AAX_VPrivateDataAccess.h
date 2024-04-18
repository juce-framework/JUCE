/*================================================================================================*/
/*
 *
 *	Copyright 2014-2017, 2019, 2023-2024 Avid Technology, Inc.
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
 *	\file  AAX_VPrivateDataAccess.h
 *
 *	\brief Version-managed concrete PrivateDataAccess class
 *
 */ 
/*================================================================================================*/
#ifndef AAX_VPRIVATEDATAACCESS_H
#define AAX_VPRIVATEDATAACCESS_H

#include "AAX_IPrivateDataAccess.h"
#include "AAX_IACFPrivateDataAccess.h"
#include "ACFPtr.h"


class IACFUnknown;

/**
 *	\brief Version-managed concrete \ref AAX_IPrivateDataAccess class
 *
 */
class AAX_VPrivateDataAccess : public AAX_IPrivateDataAccess
{
public:
	AAX_VPrivateDataAccess( IACFUnknown* pUnknown );
	~AAX_VPrivateDataAccess() AAX_OVERRIDE;

	// Direct access methods
	AAX_Result	ReadPortDirect( AAX_CFieldIndex inFieldIndex, const uint32_t inOffset, const uint32_t inSize, void* outBuffer ) AAX_OVERRIDE; ///< \copydoc AAX_IPrivateDataAccess::ReadPortDirect()
	AAX_Result	WritePortDirect( AAX_CFieldIndex inFieldIndex, const uint32_t inOffset, const uint32_t inSize, const void* inBuffer ) AAX_OVERRIDE; ///< \copydoc AAX_IPrivateDataAccess::WritePortDirect()


private:
	AAX_IACFPrivateDataAccess*	mIPrivateDataAccess;
};


#endif //AAX_VPRIVATEDATAACCESS_H


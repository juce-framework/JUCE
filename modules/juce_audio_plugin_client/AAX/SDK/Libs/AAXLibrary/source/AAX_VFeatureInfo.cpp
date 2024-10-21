/*================================================================================================*/
/*
 *	Copyright 2016-2017, 2019, 2023-2024 Avid Technology, Inc.
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
 */

// Self include
#include "AAX_VFeatureInfo.h"

// AAX includes
#include "AAX_IACFFeatureInfo.h"
#include "AAX_VPropertyMap.h"
#include "AAX_UIDs.h"

// Standard includes
#include <memory>


AAX_VFeatureInfo::AAX_VFeatureInfo( IACFUnknown* pUnknown, const acfUID& inFeatureID )
{
	mFeatureID = inFeatureID;
	
	if ( pUnknown )
	{
		pUnknown->QueryInterface(IID_IAAXFeatureInfoV1, (void **)&mIFeature);
	}
}

AAX_VFeatureInfo::~AAX_VFeatureInfo()
{
}

AAX_Result AAX_VFeatureInfo::SupportLevel(AAX_ESupportLevel& oSupportLevel) const
{
	if ( mIFeature )
		return mIFeature->SupportLevel(&oSupportLevel);
	
	return AAX_ERROR_NULL_OBJECT;
}

const AAX_IPropertyMap* AAX_VFeatureInfo::AcquireProperties() const
{
	AAX_UNIQUE_PTR(const AAX_IPropertyMap) acquiredMap;
	
	if ( mIFeature )
	{
		IACFUnknown* properties = NULL;
		
		// const cast is OK here because we are ultimately storing the result of the non-const acquire call in a const object
		AAX_IACFFeatureInfo* feature = const_cast<AAX_IACFFeatureInfo*>(mIFeature.inArg());
		if (AAX_SUCCESS == feature->AcquireProperties(&properties) && (NULL != properties))
		{
			// Move the ACF interface reference to an AAX_VPropertyMap object owned by the plug-in
			acquiredMap.reset(AAX_VPropertyMap::Acquire(properties));
		}
	}
	
	return acquiredMap.release();
}

const acfUID& AAX_VFeatureInfo::ID() const
{
	return mFeatureID;
}

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
#include "AAX_VDescriptionHost.h"

// AAX includes
#include "AAX_VFeatureInfo.h"
#include "AAX_IACFDescriptionHost.h"
#include "AAX_UIDs.h"

// ACF includes
#include "acfbaseapi.h"

// Standard includes
#include <memory>


AAX_VDescriptionHost::AAX_VDescriptionHost( IACFUnknown* pUnknown )
{
	if ( pUnknown )
	{
		pUnknown->QueryInterface(IID_IAAXDescriptionHostV1, (void **)&mDescriptionHost);
		pUnknown->QueryInterface(IID_IACFDefinition, (void**)&mHostInformation);
	}
}

AAX_VDescriptionHost::~AAX_VDescriptionHost()
{
}

const AAX_IFeatureInfo* AAX_VDescriptionHost::AcquireFeatureProperties(const AAX_Feature_UID& inFeatureID) const
{
	AAX_UNIQUE_PTR(const AAX_IFeatureInfo) acquiredFeatureProperties;
	
	if ( mDescriptionHost )
	{
		IACFUnknown* featureInfo = NULL;
		
		// const cast is OK here because we are ultimately storing the result of the non-const acquire call in a const object
		AAX_IACFDescriptionHost* descHost = const_cast<AAX_IACFDescriptionHost*>(mDescriptionHost.inArg());
		if (AAX_SUCCESS == descHost->AcquireFeatureProperties(inFeatureID, &featureInfo) && (NULL != featureInfo))
		{
			acquiredFeatureProperties.reset(new AAX_VFeatureInfo(featureInfo, inFeatureID));
		}
	}
	
	return acquiredFeatureProperties.release();
}





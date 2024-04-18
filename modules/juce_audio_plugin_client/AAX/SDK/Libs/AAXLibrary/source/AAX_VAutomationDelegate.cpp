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

#include "AAX_VAutomationDelegate.h"
#include "AAX_IACFController.h"
#include "AAX_UIDs.h"
#include <algorithm>
#include <iterator>

// ******************************************************************************************
// METHOD:	AAX_VAutomationDelegate
// ******************************************************************************************
AAX_VAutomationDelegate::AAX_VAutomationDelegate( IACFUnknown * pUnknown )
{
	if ( pUnknown )
	{
		pUnknown->QueryInterface(IID_IAAXAutomationDelegateV1, (void **)&mIAutomationDelegate);
		pUnknown->QueryInterface(IID_IAAXControllerV2, (void**)&mIController);
	}
}

// ******************************************************************************************
// METHOD:	~AAX_VAutomationDelegate
// ******************************************************************************************
AAX_VAutomationDelegate::~AAX_VAutomationDelegate()
{
}

// ******************************************************************************************
// METHOD:	RegisterControl
// ******************************************************************************************
AAX_Result AAX_VAutomationDelegate::RegisterParameter ( AAX_CParamID iParameterID )
{
	if ( mIAutomationDelegate )
		return mIAutomationDelegate->RegisterParameter ( iParameterID );
	
	return AAX_ERROR_NULL_OBJECT;
}

// ******************************************************************************************
// METHOD:	UnregisterControl
// ******************************************************************************************
AAX_Result AAX_VAutomationDelegate::UnregisterParameter ( AAX_CParamID iParameterID )
{
	if ( mIAutomationDelegate )
		return mIAutomationDelegate->UnregisterParameter ( iParameterID );
	
	return AAX_ERROR_NULL_OBJECT;
}

// ******************************************************************************************
// METHOD:	PostSetValueRequest
// ******************************************************************************************
AAX_Result AAX_VAutomationDelegate::PostSetValueRequest ( AAX_CParamID iParameterID, double iNormalizedValue ) const
{
	if ( mIAutomationDelegate )
		return mIAutomationDelegate->PostSetValueRequest ( iParameterID, iNormalizedValue );
	
	return AAX_ERROR_NULL_OBJECT;
}

// ******************************************************************************************
// METHOD:	PostCurrentValue
// ******************************************************************************************
AAX_Result AAX_VAutomationDelegate::PostCurrentValue ( AAX_CParamID iParameterID, double iNormalizedValue ) const
{
	if ( mIAutomationDelegate )
		return mIAutomationDelegate->PostCurrentValue ( iParameterID, iNormalizedValue );
	
	return AAX_ERROR_NULL_OBJECT;
}

// ******************************************************************************************
// METHOD:	PostTouchRequest
// ******************************************************************************************
AAX_Result AAX_VAutomationDelegate::PostTouchRequest ( AAX_CParamID iParameterID )
{
	if ( mIAutomationDelegate )
		return mIAutomationDelegate->PostTouchRequest ( iParameterID );
	
	return AAX_ERROR_NULL_OBJECT;
}

// ******************************************************************************************
// METHOD:	PostReleaseRequest
// ******************************************************************************************
AAX_Result AAX_VAutomationDelegate::PostReleaseRequest ( AAX_CParamID iParameterID )
{
	if ( mIAutomationDelegate )
		return mIAutomationDelegate->PostReleaseRequest ( iParameterID );
	
	return AAX_ERROR_NULL_OBJECT;
}

// ******************************************************************************************
// METHOD:	GetTouchState
// ******************************************************************************************
AAX_Result AAX_VAutomationDelegate::GetTouchState ( AAX_CParamID iParameterID, AAX_CBoolean * outTouched )
{
	if ( mIAutomationDelegate )
		return mIAutomationDelegate->GetTouchState ( iParameterID, outTouched );
	
	return AAX_ERROR_NULL_OBJECT;
}

// ******************************************************************************************
// METHOD:	ParameterNameChanged
// ******************************************************************************************
AAX_Result AAX_VAutomationDelegate::ParameterNameChanged ( AAX_CParamID iParameterID )
{
	if ( mIController ) {
		auto const end = iParameterID + kAAX_ParameterIdentifierMaxSize;
		auto const found = std::find(iParameterID, end, '\0');
		if (end != found) {
			auto const len = std::distance(iParameterID, found);
			if (len > 0) {
				return mIController->SendNotification (AAX_eNotificationEvent_ParameterNameChanged, iParameterID, sizeof(char) * (static_cast<unsigned int>(len)+1));
			}
		}
		return AAX_ERROR_INVALID_PARAMETER_ID;
	}
	
	return AAX_ERROR_UNIMPLEMENTED;
}

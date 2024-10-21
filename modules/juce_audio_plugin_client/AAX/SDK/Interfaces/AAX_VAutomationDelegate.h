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
 *	\file  AAX_VAutomationDelegate.h
 *
 *	\brief Version-managed concrete AutomationDelegate  class
 *
 */ 
/*================================================================================================*/

#ifndef AAX_VAUTOMATIONDELEGATE_H
#define AAX_VAUTOMATIONDELEGATE_H

#include "AAX_IAutomationDelegate.h"
#include "AAX_IACFAutomationDelegate.h"
#include "ACFPtr.h"

class AAX_IACFAutomationDelegate;
class AAX_IACFController_V2;
class IACFUnknown;

/**
 *	\brief Version-managed concrete \ref AAX_IAutomationDelegate "automation delegate" class
 *
 */
class AAX_VAutomationDelegate : public AAX_IAutomationDelegate
{
public:
	AAX_VAutomationDelegate( IACFUnknown * pUnknown );
	~AAX_VAutomationDelegate() AAX_OVERRIDE;
	
	IACFUnknown*	GetUnknown() const { return mIAutomationDelegate; }
	
	AAX_Result		RegisterParameter ( AAX_CParamID iParameterID ) AAX_OVERRIDE;
	AAX_Result		UnregisterParameter ( AAX_CParamID iParameterID ) AAX_OVERRIDE;
	AAX_Result		PostSetValueRequest ( AAX_CParamID iParameterID, double iNormalizedValue ) const AAX_OVERRIDE;
	AAX_Result		PostCurrentValue ( AAX_CParamID iParameterID, double iNormalizedValue ) const AAX_OVERRIDE;
	AAX_Result		PostTouchRequest ( AAX_CParamID iParameterID ) AAX_OVERRIDE;
	AAX_Result		PostReleaseRequest ( AAX_CParamID iParameterID ) AAX_OVERRIDE;
	AAX_Result		GetTouchState ( AAX_CParamID iParameterID, AAX_CBoolean * outTouched ) AAX_OVERRIDE;
	AAX_Result		ParameterNameChanged ( AAX_CParamID iParameterID ) AAX_OVERRIDE;
	
private:
	ACFPtr<AAX_IACFAutomationDelegate>	mIAutomationDelegate;
	ACFPtr<AAX_IACFController_V2>	mIController;
};



#endif //AAX_IAUTOMATIONDELEGATE_H

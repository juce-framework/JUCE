/*================================================================================================*/
/*
 *
 *	Copyright 2013-2017, 2023-2024 Avid Technology, Inc.
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
 *	\file  AAX_IACFAutomationDelegate.h
 *
 *	\brief Versioned interface allowing an %AAX plug-in to interact with the host's automation system
 *
 */ 
/*================================================================================================*/


#ifndef AAX_IACFAUTOMATIONDELEGATE_H
#define AAX_IACFAUTOMATIONDELEGATE_H

#include "AAX.h"

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wnon-virtual-dtor"
#endif

#include "acfunknown.h"

/**	\brief Versioned interface allowing an %AAX plug-in to interact with the host's automation system
	
	\details
	\sa \ref advancedTopics_ParameterUpdates
	\sa \ref AAX_IAutomationDelegate
 */
class AAX_IACFAutomationDelegate : public IACFUnknown
{
public:
	
	/**	\copydoc AAX_IAutomationDelegate::RegisterParameter()
	 */
	virtual AAX_Result	RegisterParameter ( AAX_CParamID iParameterID ) = 0;

	/**	\copydoc AAX_IAutomationDelegate::UnregisterParameter()
	 */
	virtual AAX_Result	UnregisterParameter ( AAX_CParamID iParameterID ) = 0;

	/**	\copydoc AAX_IAutomationDelegate::PostSetValueRequest()
	 */
	virtual AAX_Result	PostSetValueRequest ( AAX_CParamID iParameterID, double normalizedValue ) const = 0;

	/**	\copydoc AAX_IAutomationDelegate::PostCurrentValue()
	 */
	virtual AAX_Result	PostCurrentValue( AAX_CParamID iParameterID, double normalizedValue ) const = 0;
	
	/**	\copydoc AAX_IAutomationDelegate::PostTouchRequest()
	 */
	virtual AAX_Result	PostTouchRequest( AAX_CParamID iParameterID ) = 0;
	
	/**	\copydoc AAX_IAutomationDelegate::PostReleaseRequest()
	 */
	virtual AAX_Result	PostReleaseRequest( AAX_CParamID iParameterID ) = 0;
	
	/**	\copydoc AAX_IAutomationDelegate::GetTouchState()
	 */
	virtual AAX_Result	GetTouchState ( AAX_CParamID iParameterID, AAX_CBoolean * oTouched )= 0;
};

#ifdef __clang__
#pragma clang diagnostic pop
#endif

#endif // AAX_IACFAUTOMATIONDELEGATE_H

/*================================================================================================*/
/*
 *
 *	Copyright 2013-2017, 2019, 2021, 2023-2024 Avid Technology, Inc.
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
 *	\file  AAX_VViewContainer.h
 *
 *	\brief Version-managed concrete ViewContainer class
 *
 */ 
/*================================================================================================*/

#ifndef AAX_VVIEWCONTAINER_H
#define AAX_VVIEWCONTAINER_H

#include "AAX_IViewContainer.h"
#include "AAX_IACFViewContainer.h"
#include "ACFPtr.h"


class IACFUnknown;

/**
 *	\brief Version-managed concrete \ref AAX_IViewContainer class
 *
 */
class AAX_VViewContainer : public AAX_IViewContainer
{
public:
	AAX_VViewContainer( IACFUnknown * pUnknown );
	 ~AAX_VViewContainer() AAX_OVERRIDE;
	
    // AAX_IACFViewContainer
    
	// Getters
	int32_t		GetType () AAX_OVERRIDE; ///< \copydoc AAX_IViewContainer::GetType()
    void *		GetPtr () AAX_OVERRIDE; ///< \copydoc AAX_IViewContainer::GetPtr()
	AAX_Result	GetModifiers ( uint32_t * outModifiers ) AAX_OVERRIDE; ///< \copydoc AAX_IViewContainer::GetModifiers()

	// Setters
    AAX_Result	SetViewSize ( AAX_Point & inSize ) AAX_OVERRIDE; ///< \copydoc AAX_IViewContainer::SetViewSize()
	AAX_Result	HandleParameterMouseDown  ( AAX_CParamID inParamID, uint32_t inModifiers ) AAX_OVERRIDE; ///< \copydoc AAX_IViewContainer::HandleParameterMouseDown()
	AAX_Result	HandleParameterMouseDrag  ( AAX_CParamID inParamID, uint32_t inModifiers ) AAX_OVERRIDE; ///< \copydoc AAX_IViewContainer::HandleParameterMouseDrag()
	AAX_Result	HandleParameterMouseUp    ( AAX_CParamID inParamID, uint32_t inModifiers ) AAX_OVERRIDE; ///< \copydoc AAX_IViewContainer::HandleParameterMouseUp()
	AAX_Result	HandleParameterMouseEnter ( AAX_CParamID inParamID, uint32_t inModifiers ) AAX_OVERRIDE; ///< \copydoc AAX_IViewContainer::HandleParameterMouseEnter()
	AAX_Result	HandleParameterMouseExit  ( AAX_CParamID inParamID, uint32_t inModifiers ) AAX_OVERRIDE; ///< \copydoc AAX_IViewContainer::HandleParameterMouseExit()
	AAX_Result	HandleMultipleParametersMouseDown ( const AAX_CParamID* inParamIDs, uint32_t inNumOfParams, uint32_t inModifiers ) AAX_OVERRIDE; ///< \copydoc AAX_IViewContainer::HandleMultipleParametersMouseDown()
	AAX_Result	HandleMultipleParametersMouseDrag ( const AAX_CParamID* inParamIDs, uint32_t inNumOfParams, uint32_t inModifiers ) AAX_OVERRIDE; ///< \copydoc AAX_IViewContainer::HandleMultipleParametersMouseDrag()
	AAX_Result	HandleMultipleParametersMouseUp   ( const AAX_CParamID* inParamIDs, uint32_t inNumOfParams, uint32_t inModifiers ) AAX_OVERRIDE; ///< \copydoc AAX_IViewContainer::HandleMultipleParametersMouseUp()
	
private:
	ACFPtr<AAX_IACFViewContainer>	    mIViewContainer;
	ACFPtr<AAX_IACFViewContainer_V2>	mIViewContainerV2;
	ACFPtr<AAX_IACFViewContainer_V3>    mIViewContainerV3;
};


#endif //AAX_VVIEWCONTAINER_H

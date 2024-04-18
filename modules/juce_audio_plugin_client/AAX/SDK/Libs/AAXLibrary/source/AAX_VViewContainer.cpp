/*================================================================================================*/
/*
 *	Copyright 2013-2019, 2021, 2023-2024 Avid Technology, Inc.
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

#include "AAX_VViewContainer.h"
#include "AAX_UIDs.h"
#include "AAX_Assert.h"

// ******************************************************************************************
// METHOD:	AAX_VViewContainer
// ******************************************************************************************
AAX_VViewContainer::AAX_VViewContainer( IACFUnknown* pUnknown )
{
	if ( pUnknown )
	{
		pUnknown->QueryInterface ( IID_IAAXViewContainerV1, (void **) &mIViewContainer );
		pUnknown->QueryInterface ( IID_IAAXViewContainerV2, (void **) &mIViewContainerV2 );
		pUnknown->QueryInterface ( IID_IAAXViewContainerV3, (void **) &mIViewContainerV3 );
	}
}

// ******************************************************************************************
// METHOD:	~AAX_VViewContainer
// ******************************************************************************************
AAX_VViewContainer::~AAX_VViewContainer()
{
	// HACK - Some hosts contain multiple overlapping systems for destroying
	// the underlying view object(s). Allowing the ACF refcount to go to zero
	// in these hosts will result in a crash due to multiple deletion. This is
	// tracked as PT-243211.
	//
	// If we are ever confident that the host-side fix has propagated to all
	// hosts in use then we can remove this "leak" and allow the ACF count to
	// decrement correctly.
	try {
		if (mIViewContainer) { mIViewContainer.detach(); }
	}
	catch (ACFRESULT r) {
		AAX_TRACE_RELEASE(kAAX_Trace_Priority_High, "AAX_VViewContainer error when detaching mIViewContainer: %d", (int)r);
	}
	try {
		if (mIViewContainerV2) { mIViewContainerV2.detach(); }
	}
	catch (ACFRESULT r) {
		AAX_TRACE_RELEASE(kAAX_Trace_Priority_High, "AAX_VViewContainer error when detaching mIViewContainerV2: %d", (int)r);
	}
	try {
		if (mIViewContainerV3) { mIViewContainerV3.detach(); }
	}
	catch (ACFRESULT r) {
		AAX_TRACE_RELEASE(kAAX_Trace_Priority_High, "AAX_VViewContainer error when detaching mIViewContainerV3: %d", (int)r);
	}
}

// ******************************************************************************************
// METHOD:	GetType
// ******************************************************************************************
int32_t AAX_VViewContainer::GetType ()
{
	int32_t	result = AAX_eViewContainer_Type_NULL;
	
	if ( mIViewContainer )
		result = mIViewContainer->GetType ();
	
	return result;
}

// ******************************************************************************************
// METHOD:	GetPtr
// ******************************************************************************************
void * AAX_VViewContainer::GetPtr ()
{
	void *	result = 0;
	
	if ( mIViewContainer )
		result = mIViewContainer->GetPtr ();
	
	return result;
}

// ******************************************************************************************
// METHOD:	GetModifiers
// ******************************************************************************************
AAX_Result AAX_VViewContainer::GetModifiers ( uint32_t * outModifiers )
{
	AAX_Result	result = AAX_ERROR_UNIMPLEMENTED;
	
	if ( mIViewContainer )
		result = mIViewContainer->GetModifiers ( outModifiers );
	
	return result;
}

// ******************************************************************************************
// METHOD:	HandleParameterMouseDown
// ******************************************************************************************
AAX_Result AAX_VViewContainer::HandleParameterMouseDown ( AAX_CParamID inParamID, uint32_t inModifiers )
{
	AAX_Result	result = AAX_ERROR_UNIMPLEMENTED;
	
	if ( mIViewContainer )
		result = mIViewContainer->HandleParameterMouseDown ( inParamID, inModifiers );
	
	return result;
}

// ******************************************************************************************
// METHOD:	HandleParameterMouseDrag
// ******************************************************************************************
AAX_Result AAX_VViewContainer::HandleParameterMouseDrag ( AAX_CParamID inParamID, uint32_t inModifiers )
{
	AAX_Result	result = AAX_ERROR_UNIMPLEMENTED;
	
	if ( mIViewContainer )
		result = mIViewContainer->HandleParameterMouseDrag ( inParamID, inModifiers );
	
	return result;
}

// ******************************************************************************************
// METHOD:	HandleParameterMouseUp
// ******************************************************************************************
AAX_Result AAX_VViewContainer::HandleParameterMouseUp ( AAX_CParamID inParamID, uint32_t inModifiers )
{
	AAX_Result	result = AAX_ERROR_UNIMPLEMENTED;
	
	if ( mIViewContainer )
		result = mIViewContainer->HandleParameterMouseUp ( inParamID, inModifiers );
	
	return result;
}


// ******************************************************************************************
// METHOD:    HandleParameterMouseEnter
// ******************************************************************************************
AAX_Result AAX_VViewContainer::HandleParameterMouseEnter(AAX_CParamID inParamID, uint32_t inModifiers)
{
	AAX_Result    result = AAX_ERROR_UNIMPLEMENTED;
    
	if (mIViewContainerV3)
		result = mIViewContainerV3->HandleParameterMouseEnter(inParamID, inModifiers);
    
	return result;
}


// ******************************************************************************************
// METHOD:    HandleParameterMouseExit
// ******************************************************************************************
AAX_Result AAX_VViewContainer::HandleParameterMouseExit(AAX_CParamID inParamID, uint32_t inModifiers)
{
	AAX_Result    result = AAX_ERROR_UNIMPLEMENTED;
    
	if (mIViewContainerV3)
		result = mIViewContainerV3->HandleParameterMouseExit(inParamID, inModifiers);
    
	return result;
}


// ******************************************************************************************
// METHOD:	SetViewSize
// ******************************************************************************************
AAX_Result AAX_VViewContainer::SetViewSize ( AAX_Point & inSize )
{
	AAX_Result	result = AAX_SUCCESS;
	
	if ( mIViewContainer )
		result = mIViewContainer->SetViewSize ( inSize );
	
	return result;
}

// ******************************************************************************************
// METHOD:	HandleMultipleParametersMouseDown
// ******************************************************************************************
AAX_Result AAX_VViewContainer::HandleMultipleParametersMouseDown ( const AAX_CParamID* inParamIDs, uint32_t iNumOfParams, uint32_t inModifiers )
{
	AAX_Result	result = AAX_ERROR_UNIMPLEMENTED;

	if ( mIViewContainerV2 )
		result = mIViewContainerV2->HandleMultipleParametersMouseDown ( inParamIDs, iNumOfParams, inModifiers );

	return result;
}

// ******************************************************************************************
// METHOD:	HandleMultipleParametersMouseDrag
// ******************************************************************************************
AAX_Result AAX_VViewContainer::HandleMultipleParametersMouseDrag ( const AAX_CParamID* inParamIDs, uint32_t iNumOfParams, uint32_t inModifiers )
{
	AAX_Result	result = AAX_ERROR_UNIMPLEMENTED;

	if ( mIViewContainerV2 )
		result = mIViewContainerV2->HandleMultipleParametersMouseDrag ( inParamIDs, iNumOfParams, inModifiers );

	return result;
}

// ******************************************************************************************
// METHOD:	HandleMultipleParametersMouseUp
// ******************************************************************************************
AAX_Result AAX_VViewContainer::HandleMultipleParametersMouseUp ( const AAX_CParamID* inParamIDs, uint32_t iNumOfParams, uint32_t inModifiers )
{
	AAX_Result	result = AAX_ERROR_UNIMPLEMENTED;

	if ( mIViewContainerV2 )
		result = mIViewContainerV2->HandleMultipleParametersMouseUp ( inParamIDs, iNumOfParams, inModifiers );

	return result;
}

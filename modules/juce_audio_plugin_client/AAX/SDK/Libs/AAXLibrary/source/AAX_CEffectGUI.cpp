/*================================================================================================*/
/*
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
/*================================================================================================*/

#include "AAX_CEffectGUI.h"
#include "AAX_VController.h"
#include "AAX_VTransport.h"
#include "AAX_VViewContainer.h"
#include "AAX_IEffectParameters.h"
#include "AAX_IParameter.h"
#include "AAX_UIDs.h"
#include "AAX_CString.h"
#include "AAX_Assert.h"

// *******************************************************************************
// METHOD:	AAX_CEffectGUI
// *******************************************************************************
AAX_CEffectGUI::AAX_CEffectGUI() :
	mController(NULL),
	mEffectParameters(NULL),
	mViewContainer(),
	mTransport(NULL)
{

}

// *******************************************************************************
// METHOD:	~AAX_CEffectGUI
// *******************************************************************************
AAX_CEffectGUI::~AAX_CEffectGUI()
{	
	this->Uninitialize();	//<DMT>  Just to guarantee that all these things get broken down...
}

// *******************************************************************************
// METHOD:	Initialize
// *******************************************************************************
AAX_Result AAX_CEffectGUI::Initialize ( IACFUnknown * iController )
{
	mController = new AAX_VController(iController);
	if (iController == 0 || mController == 0)
		return AAX_ERROR_NOT_INITIALIZED;
		
	if ( iController )
	{
		iController->QueryInterface(IID_IAAXEffectParametersV1, (void **)&mEffectParameters);
	}
	
	mTransport = new AAX_VTransport(iController);
		
	this->CreateViewContents ();
	return AAX_SUCCESS;
}

// *******************************************************************************
// METHOD:	Uninitialize
// *******************************************************************************
AAX_Result		AAX_CEffectGUI::Uninitialize (void)
{
	if ( nullptr != this->GetViewContainer() )
	{
		this->SetViewContainer(nullptr);
	}
	
	if (mEffectParameters)
	{
		mEffectParameters->Release();
		mEffectParameters = 0;
	}
	
	if ( mController )
	{
		delete ( mController );
		mController = 0;
	}
	if ( mTransport )
	{
		delete mTransport;
		mTransport = NULL;
	}
	
	return AAX_SUCCESS;
}

// *******************************************************************************
// METHOD:	NotificationReceived
// *******************************************************************************
AAX_Result		AAX_CEffectGUI::NotificationReceived(AAX_CTypeID /*inNotificationType*/, const void * /*inNotificationData*/, uint32_t	/*inNotificationDataSize*/)
{
	//Base implementation doesn't need to know any of these right now.
	return AAX_SUCCESS;
}



// *******************************************************************************
// METHOD:	SetViewContainer
// *******************************************************************************
AAX_Result AAX_CEffectGUI::SetViewContainer ( IACFUnknown * inViewContainer )
{
	if ( !inViewContainer )
	{
		this->DeleteViewContainer ();
		mViewContainer.reset();
	}
	else
	{
		mViewContainer.reset(new AAX_VViewContainer ( inViewContainer ));
		this->CreateViewContainer ();
		this->UpdateAllParameters ();
	}

	return AAX_SUCCESS;
}

// *******************************************************************************
// METHOD:	GetViewContainerType
// *******************************************************************************
AAX_EViewContainer_Type AAX_CEffectGUI::GetViewContainerType ()
{
	AAX_EViewContainer_Type	result = AAX_eViewContainer_Type_NULL;
	if ( AAX_IViewContainer* const viewContainer = this->GetViewContainer() ) {
		result = static_cast<AAX_EViewContainer_Type>(viewContainer->GetType());
	}
	
	return result;
}

// *******************************************************************************
// METHOD:	GetViewContainerPtr
// *******************************************************************************
void * AAX_CEffectGUI::GetViewContainerPtr ()
{
	void *	result = nullptr;
	if ( AAX_IViewContainer* const viewContainer = this->GetViewContainer() ) {
		result = viewContainer->GetPtr ();
	}
	
	return result;
}

// *******************************************************************************
// METHOD:	ParameterUpdated (Called from Host on main thread)
// *******************************************************************************
AAX_Result	AAX_CEffectGUI::ParameterUpdated(AAX_CParamID /*inParamID*/)
{
	return AAX_SUCCESS;
}

// *******************************************************************************
// METHOD:	UpdateAllParameters
// *******************************************************************************
void AAX_CEffectGUI::UpdateAllParameters ()
{
	if ( mEffectParameters ) {
		int32_t	numControls = 0;
		if ( AAX_SUCCESS == mEffectParameters->GetNumberOfParameters( &numControls ) ) {
			for ( int32_t index = 0; index < numControls; ++index )
			{
				AAX_CString	paramID;
				if ( AAX_SUCCESS == mEffectParameters->GetParameterIDFromIndex( index, &paramID ) ) {
						this->ParameterUpdated( (AAX_CParamID)paramID.CString() );
				}
			}
		}
		else {
			AAX_TRACE_RELEASE(kAAX_Trace_Priority_Critical, "AAX_CEffectGUI::UpdateAllParameters - error getting the number of parameters");
		}
	}
}

// *******************************************************************************
// METHOD:	GetPlugInString
// *******************************************************************************
AAX_Result	AAX_CEffectGUI::GetCustomLabel ( AAX_EPlugInStrings /*inSelector*/, AAX_IString * /*outString*/ ) const
{	
	return AAX_ERROR_NULL_OBJECT;
}

// *******************************************************************************
// METHOD:	GetController
// *******************************************************************************
AAX_IController* AAX_CEffectGUI::GetController (void)
{
    return mController;
}
const AAX_IController* AAX_CEffectGUI::GetController (void) const
{
    return mController;
}

// *******************************************************************************
// METHOD:	GetEffectParameters
// *******************************************************************************
AAX_IEffectParameters* AAX_CEffectGUI::GetEffectParameters (void)
{
    return mEffectParameters;
}
const AAX_IEffectParameters* AAX_CEffectGUI::GetEffectParameters (void) const
{
    return mEffectParameters;
}

// *******************************************************************************
// METHOD:	GetViewContainer
// *******************************************************************************
AAX_IViewContainer* AAX_CEffectGUI::GetViewContainer (void)
{
    return mViewContainer.get();
}
const AAX_IViewContainer* AAX_CEffectGUI::GetViewContainer (void) const
{
    return mViewContainer.get();
}


// *******************************************************************************
// METHOD:	Transport
// *******************************************************************************
AAX_ITransport* AAX_CEffectGUI::Transport()
{
	return mTransport;
}
const AAX_ITransport* AAX_CEffectGUI::Transport() const
{
	return mTransport;
}

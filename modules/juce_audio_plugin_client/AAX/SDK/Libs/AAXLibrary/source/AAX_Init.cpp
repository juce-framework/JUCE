/*================================================================================================*/
/*
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
/*================================================================================================*/

#include "AAX_Init.h"
#include "AAX_VCollection.h"
#include "AAX_CHostServices.h"

#include "AAX_Exception.h"
#include "AAX_Assert.h"
#include "AAX_Version.h"
#include "ACFPtr.h"
#include "CACFClassFactory.h"

// this function must be implemented somewhere in your plug-in
//   inCollection: The host-provided collection interface for describing this plug-in. Never NULL.
extern AAX_Result GetEffectDescriptions( AAX_ICollection * outCollection );

AAX_Result AAXRegisterPlugin(IACFUnknown * pUnkHost, IACFPluginDefinition **ppPluginDefinition)
{
	AAX_Result result = AAX_SUCCESS;
	
	try
	{
		AAX_VCollection	collection ( pUnkHost );
		AAX_CheckedResult checkedResult(GetEffectDescriptions ( &collection ));
		
		*ppPluginDefinition = 0;
		if ( collection.GetIUnknown()->QueryInterface(IID_IACFPluginDefinition, (void**)ppPluginDefinition) != ACF_OK )
			checkedResult = AAX_ERROR_NULL_OBJECT;
	}
	catch (const AAX::Exception::ResultError& ex)
	{
		result = ex.Result();
		AAX_TRACE_RELEASE(kAAX_Trace_Priority_High, "AAXRegisterPlugin exception caught: %s", ex.What().c_str());
	}
	catch (const AAX::Exception::Any& ex)
	{
		result = AAX_ERROR_UNKNOWN_EXCEPTION;
		AAX_TRACE_RELEASE(kAAX_Trace_Priority_High, "AAXRegisterPlugin exception caught: %s", ex.What().c_str());
	}
	catch (const std::exception& ex)
	{
		result = AAX_ERROR_UNKNOWN_EXCEPTION;
		AAX_TRACE_RELEASE(kAAX_Trace_Priority_High, "AAXRegisterPlugin exception caught: %s", ex.what());
	}
	catch (...)
	{
		result = AAX_ERROR_UNKNOWN_EXCEPTION;
		AAX_TRACE_RELEASE(kAAX_Trace_Priority_High, "AAXRegisterPlugin exception caught: unknown");
	}
	
	return result;
}	

AAX_Result AAXRegisterComponent (IACFUnknown * /*pUnkHost*/, acfUInt32 /*index*/, IACFComponentDefinition **ppComponentDefinition)
{
	*ppComponentDefinition = NULL;
	return AAX_SUCCESS;
}

AAX_Result AAXGetClassFactory (IACFUnknown * /*pUnkHost*/, const acfCLSID& /*clsid*/, const acfIID& /*iid*/, void** ppOut)
{
	*ppOut = NULL;	
	return AAX_ERROR_UNIMPLEMENTED;
}

AAX_Result AAXCanUnloadNow(IACFUnknown* /*pUnkHost*/)
{
	return static_cast<AAX_Result>(CACFUnknown::GetActiveObjectCount());
}

AAX_Result AAXStartup(IACFUnknown* pUnkHost)
{
	AAX_CHostServices::Set ( pUnkHost );
	return AAX_SUCCESS;
}

AAX_Result AAXShutdown(IACFUnknown* /*pUnkHost*/)
{
	AAX_CHostServices::Set(NULL);	
	return AAX_SUCCESS;
}

AAX_Result AAXGetSDKVersion( acfUInt64* oSDKVersion )
{
    //Upper 32 bit uint is for SDK Version
    *oSDKVersion = acfUInt64(AAX_SDK_VERSION) << 32;
    
    //Lower 32 bit uint is for revision number.
    *oSDKVersion += acfUInt64(AAX_SDK_CURRENT_REVISION);
	return AAX_SUCCESS;
}

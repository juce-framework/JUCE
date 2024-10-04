/*================================================================================================*/
/*
 *
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

/**  
 *	\file AAX_Exports.cpp
 *
 */ 
/*================================================================================================*/

#include "AAX_Init.h"
#include "AAX.h"
#include "acfunknown.h"
#include "acfresult.h"

#if defined (__GNUC__)
#define AAX_EXPORT extern "C" __attribute__((visibility("default"))) ACFRESULT 

// AH 1/18/13 - temp workaround for Eden signing bug in DP5.  Need to provide this term routine to
// avoid crashes in wrapped plugins.  This will be fixed by pace in Eden FC1, when we can remove this code.

static void module_term_routine(void) __attribute__((destructor)) __attribute__((used));
void    module_term_routine(void)
{
    // Might want to implement something simple here
    int* x = new int;
    *x = 1;
}

#else
#define AAX_EXPORT extern "C" __declspec(dllexport) ACFRESULT __stdcall
#endif


AAX_EXPORT ACFRegisterPlugin (
						  IACFUnknown * pUnkHost,
						  IACFPluginDefinition **ppPluginDefinition
						  );


AAX_EXPORT ACFRegisterComponent (
							 IACFUnknown * pUnkHost,
							 acfUInt32 index,
							 IACFComponentDefinition **ppComponentDefinition
							 );

AAX_EXPORT ACFGetClassFactory (
						   IACFUnknown * pUnkHost,
						   const acfCLSID& clsid, 
						   const acfIID& iid, 
						   void** ppOut
						   );

AAX_EXPORT ACFCanUnloadNow (IACFUnknown * pUnkHost);

AAX_EXPORT ACFStartup (IACFUnknown * pUnkHost);

AAX_EXPORT ACFShutdown (IACFUnknown * pUnkHost);

AAX_EXPORT ACFGetSDKVersion ( acfUInt64 * oSDKVersion );

/** \brief The main plug-in registration method
*/
ACFAPI ACFRegisterPlugin(IACFUnknown * pUnkHostVoid, IACFPluginDefinition **ppPluginDefinitionVoid)
{
	ACFRESULT result = ACF_OK;
	
	try
	{
		result = AAXRegisterPlugin(pUnkHostVoid, ppPluginDefinitionVoid);
	}
	catch(...)
	{
		result = ACF_E_UNEXPECTED;
	}
	
	return result;
}

/** \brief Registers a specific component in the DLL
*/
ACFAPI ACFRegisterComponent (IACFUnknown * pUnkHost,
							 acfUInt32 index,
							 IACFComponentDefinition **ppComponentDefinition)
{
	ACFRESULT result = ACF_OK;
	
	try
	{
		result = AAXRegisterComponent(pUnkHost, index, ppComponentDefinition);
	}
	catch(...)
	{
		result = ACF_E_UNEXPECTED;
	}
	
	return result;
}

/**	\brief Gets the factory for a given class ID
*/
ACFAPI ACFGetClassFactory (IACFUnknown * pUnkHost,
						   const acfCLSID& clsid, 
						   const acfIID& iid, 
						   void** ppOut)
{
	ACFRESULT result = ACF_OK;
	
	try
	{
		result = AAXGetClassFactory(pUnkHost, clsid, iid, ppOut);
	}
	catch(...)
	{
		result = ACF_E_UNEXPECTED;
	}
	
	return result;
}

/**	\brief Determines whether or not the host may unload the DLL
*/
ACFAPI ACFCanUnloadNow (IACFUnknown * pUnkHost)
{
	ACFRESULT result = ACF_OK;
	
	try
	{
		result = AAXCanUnloadNow(pUnkHost); 
	}
	catch(...)
	{
		result = ACF_E_UNEXPECTED;
	}
	
	return result;
}

/**	\brief DLL initialization routine
*/
ACFAPI ACFStartup (IACFUnknown * pUnkHost)
{
	ACFRESULT result = ACF_OK;
	
	try
	{
		result = AAXStartup(pUnkHost);
	}
	catch(...)
	{
		result = ACF_E_UNEXPECTED;
	}
	
	return result;
}

/**	\brief DLL shutdown routine
*/
ACFAPI ACFShutdown (IACFUnknown * pUnkHost)
{
	ACFRESULT result = ACF_OK;
	
	try
	{
		result = AAXShutdown(pUnkHost);
	}
	catch(...)
	{
		result = ACF_E_UNEXPECTED;
	}
	
	return result;
}

/**	\brief Returns the DLL's SDK version
*/
ACFAPI ACFGetSDKVersion ( acfUInt64 * oSDKVersion )
{
	ACFRESULT result = ACF_OK;
	
	try
	{
		result = AAXGetSDKVersion(oSDKVersion);
	}
	catch(...)
	{
		result = ACF_E_UNEXPECTED;
	}
	
	return result;
}


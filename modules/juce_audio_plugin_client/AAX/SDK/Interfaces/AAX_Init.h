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
 *	\file  AAX_Init.h
 *
 *	\brief %AAX library implementations of required plug-in initialization, registration,
 *	and tear-down methods.
 *
 */ 
/*================================================================================================*/


#include "AAX.h"
#include "acfbasetypes.h"

class IACFUnknown;
class IACFPluginDefinition;
class IACFComponentDefinition;


/** @copybrief ACFRegisterPlugin()
	
	@details		
	This method determines the number of components defined in the dll. The
	implementation of this method in the %AAX library calls the following function, 
	which must be implemented somewhere in your plug-in:
	
	\code
		extern AAX_Result GetEffectDescriptions( AAX_ICollection * outCollection );
	\endcode
	
	<EM>Wrapped by \ref ACFRegisterPlugin() </EM>

	\ingroup CommonInterface_Describe
*/
AAX_Result AAXRegisterPlugin(IACFUnknown * pUnkHost, IACFPluginDefinition **ppPluginDefinition);

/** @copybrief ACFRegisterComponent()

	@details		
	The implementation of this method in the %AAX library simply sets
	\c *ppComponentDefinition to NULL and returns \ref AAX_SUCCESS.

	<EM>Wrapped by \ref ACFRegisterComponent() </EM>
*/
AAX_Result AAXRegisterComponent (IACFUnknown * pUnkHost, acfUInt32 index, IACFComponentDefinition **ppComponentDefinition);

/** @copybrief ACFGetClassFactory()

	@details		
	This method is required by ACF but is not supported by AAX. Therefore the
	implementation of this method in the %AAX library simply sets \c *ppOut to NULL
	and returns \ref AAX_ERROR_UNIMPLEMENTED.

	<EM>Wrapped by \ref ACFGetClassFactory() </EM>
*/
AAX_Result AAXGetClassFactory (IACFUnknown * pUnkHost, const acfCLSID& clsid, const acfIID& iid, void** ppOut);

/** @copybrief ACFCanUnloadNow()
	
	@details		
	The implementation of this method in the %AAX library returns the result of
	\c GetActiveObjectCount() as an \ref AAX_Result, with zero active objects
	interpreted as \ref AAX_SUCCESS (see CACFUnknown.h)

	<EM>Wrapped by \ref ACFCanUnloadNow() </EM>
*/
AAX_Result AAXCanUnloadNow(IACFUnknown* pUnkHost);

/** @copybrief ACFStartup()

	@details		
	Called once at init time. The implementation of this method in the %AAX library
	uses \c pUnkHost as an \c IACFComponentFactory to initialize global services
	(see acfbaseapi.h)

	<EM>Wrapped by \ref ACFStartup() </EM>
*/
AAX_Result AAXStartup(IACFUnknown* pUnkHost);

/** @copybrief ACFShutdown()

	@details		
	Called once before unloading the DLL. The implementation of this method in the %AAX
	library tears down any globally initialized state and releases any globally
	retained resources.

	<EM>Wrapped by \ref ACFShutdown() </EM>
*/
AAX_Result AAXShutdown(IACFUnknown* pUnkHost);

/** @copybrief ACFGetSDKVersion()
	
	@details		
	The implementation of this method in the %AAX library provides a 64-bit value in
	which the upper 32 bits represent the SDK version and the lower 32 bits represent
	the revision number of the SDK. See \ref AAX_Version.h

	<EM>Wrapped by \ref ACFGetSDKVersion() </EM>
*/
AAX_Result AAXGetSDKVersion( acfUInt64 *oSDKVersion );

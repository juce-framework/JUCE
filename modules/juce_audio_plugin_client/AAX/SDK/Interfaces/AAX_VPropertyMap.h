/*================================================================================================*/
/*
 *
 *	Copyright 2013-2017, 2019, 2023-2024 Avid Technology, Inc.
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
 *	\file  AAX_VPropertyMap.h
 *
 *	\brief Version-managed concrete PropertyMap class
 *
 */ 
/*================================================================================================*/

#ifndef AAX_VPROPERTYMAP_H
#define AAX_VPROPERTYMAP_H

// AAX Includes
#include "AAX_IPropertyMap.h"
#include "AAX_IACFPropertyMap.h"
#include "AAX.h"

// ACF Includes
#include "acfunknown.h"
#include "ACFPtr.h"

// Standard Includes
#include <map>


class IACFComponentFactory;
class AAX_IACFPropertyMap;
class AAX_IACFDescriptionHost;

/**
 *	\brief Version-managed concrete \ref AAX_IPropertyMap class
 *
 */
class AAX_VPropertyMap : public AAX_IPropertyMap
{
public:
	// Using static creation methods instead of public constructor in order to
	// distinguish between creating a new property map from a component factory
	// and acquiring a reference to an existing property map.
	static AAX_VPropertyMap* Create ( IACFUnknown* inComponentFactory ); ///< \p inComponentFactory must support \c IID_IACFComponentFactory - otherwise NULL is returned
	static AAX_VPropertyMap* Acquire ( IACFUnknown* inPropertyMapUnknown ); ///< \p inPropertyMapUnknown must support at least one \ref AAX_IPropertyMap interface - otherwise an \ref AAX_VPropertyMap object with no backing interface is returned
	
private:
	AAX_VPropertyMap ();
	void InitWithFactory (IACFComponentFactory* inComponentFactory, IACFUnknown* inAuxiliaryUnknown); ///< \p inAuxiliaryUnknown should support at least \ref IID_IAAXDescriptionHostV1 (may be NULL)
	void InitWithPropertyMap (IACFUnknown* inPropertyMapUnknown, IACFUnknown* inAuxiliaryUnknown); ///< \p inAuxiliaryUnknown should support at least \ref IID_IAAXDescriptionHostV1 (may be NULL)
	
public:
	~AAX_VPropertyMap(void) AAX_OVERRIDE;
	
	// AAX_IACFPropertyMap methods
	AAX_CBoolean		GetProperty ( AAX_EProperty inProperty, AAX_CPropertyValue * outValue ) const AAX_OVERRIDE; ///< \copydoc AAX_IPropertyMap::GetProperty()
	AAX_CBoolean		GetPointerProperty ( AAX_EProperty inProperty, const void** outValue ) const AAX_OVERRIDE; ///< \copydoc AAX_IPropertyMap::GetPointerProperty()
	AAX_Result			AddProperty ( AAX_EProperty inProperty, AAX_CPropertyValue inValue ) AAX_OVERRIDE; ///< \copydoc AAX_IPropertyMap::AddProperty()
	AAX_Result			AddPointerProperty ( AAX_EProperty inProperty, const void* inValue ) AAX_OVERRIDE; ///< \copydoc AAX_IPropertyMap::AddPointerProperty(AAX_EProperty, const void*)
	AAX_Result			AddPointerProperty ( AAX_EProperty inProperty, const char* inValue ) AAX_OVERRIDE; ///< \copydoc AAX_IPropertyMap::AddPointerProperty(AAX_EProperty, const char*)
	AAX_Result			RemoveProperty ( AAX_EProperty inProperty ) AAX_OVERRIDE; ///< \copydoc AAX_IPropertyMap::RemoveProperty()
    AAX_Result          AddPropertyWithIDArray ( AAX_EProperty inProperty, const AAX_SPlugInIdentifierTriad* inPluginIDs, uint32_t inNumPluginIDs) AAX_OVERRIDE; ///< \copydoc AAX_IPropertyMap::AddPropertyWithIDArray()
    AAX_CBoolean        GetPropertyWithIDArray ( AAX_EProperty inProperty, const AAX_SPlugInIdentifierTriad** outPluginIDs, uint32_t* outNumPluginIDs) const AAX_OVERRIDE; ///< \copydoc AAX_IPropertyMap::GetPropertyWithIDArray()
	
	// AAX_IPropertyMap methods
	IACFUnknown*		GetIUnknown() AAX_OVERRIDE; ///< \copydoc AAX_IPropertyMap::GetIUnknown()

private:
	ACFPtr<AAX_IACFPropertyMap>	mIACFPropertyMap;
    ACFPtr<AAX_IACFPropertyMap_V2>	mIACFPropertyMapV2;
    ACFPtr<AAX_IACFPropertyMap_V3>	mIACFPropertyMapV3;
	ACFPtr<AAX_IACFDescriptionHost> mIACFDescriptionHost;
	std::map<AAX_EProperty, const void*> mLocalPointerPropertyCache;
};



#endif // AAX_VPROPERTYMAP_H

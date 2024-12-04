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
 *	\file  AAX_IACFPropertyMap.h
 *
 *	\brief Versioned interface for an AAX_IPropertyMap
 *
 */ 
/*================================================================================================*/


#ifndef AAX_IACFPROPERTYMAP_H
#define AAX_IACFPROPERTYMAP_H

#include "AAX.h"

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wnon-virtual-dtor"
#endif

#include "acfunknown.h"

/**	\brief Versioned interface for an \ref AAX_IPropertyMap
 */ 
class AAX_IACFPropertyMap : public IACFUnknown
{
public:
	virtual AAX_CBoolean		GetProperty ( AAX_EProperty inProperty, AAX_CPropertyValue * outValue ) const = 0;	///< \copydoc AAX_IPropertyMap::GetProperty()
	virtual AAX_Result			AddProperty ( AAX_EProperty inProperty, AAX_CPropertyValue inValue ) = 0;	///< \copydoc AAX_IPropertyMap::AddProperty()
	virtual AAX_Result			RemoveProperty ( AAX_EProperty inProperty ) = 0;	///< \copydoc AAX_IPropertyMap::RemoveProperty()
};

/**	\brief Versioned interface for an \ref AAX_IPropertyMap
 */ 
class AAX_IACFPropertyMap_V2 : public AAX_IACFPropertyMap
{
public:
    virtual AAX_Result      AddPropertyWithIDArray ( AAX_EProperty inProperty, const AAX_SPlugInIdentifierTriad* inPluginIDs, uint32_t inNumPluginIDs) = 0;	///< \copydoc AAX_IPropertyMap::AddPropertyWithIDArray()
    virtual AAX_CBoolean    GetPropertyWithIDArray ( AAX_EProperty inProperty, const AAX_SPlugInIdentifierTriad** outPluginIDs, uint32_t* outNumPluginIDs) const = 0;	///< \copydoc AAX_IPropertyMap::GetPropertyWithIDArray()
};

/**	\brief Versioned interface for an \ref AAX_IPropertyMap
 */
class AAX_IACFPropertyMap_V3 : public AAX_IACFPropertyMap_V2
{
public:
	virtual AAX_CBoolean		GetProperty64 ( AAX_EProperty inProperty, AAX_CPropertyValue64 * outValue ) const = 0;	///< \copydoc AAX_IPropertyMap::GetProperty()
	virtual AAX_Result			AddProperty64 ( AAX_EProperty inProperty, AAX_CPropertyValue64 inValue ) = 0;	///< \copydoc AAX_IPropertyMap::AddProperty()
};

#ifdef __clang__
#pragma clang diagnostic pop
#endif

#endif // AAX_IACFPROPERTYMAP_H

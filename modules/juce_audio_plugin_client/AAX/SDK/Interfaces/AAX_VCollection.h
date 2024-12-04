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
 *	\file  AAX_VCollection.h
 *
 *	\brief Version-managed concrete Collection  class
 *
 */ 
/*================================================================================================*/

#ifndef AAX_VCOLLECTION_H
#define AAX_VCOLLECTION_H

#include "AAX.h"
#include "AAX_ICollection.h"
#include "AAX_IACFCollection.h"
#include "AAX_VDescriptionHost.h"
#include "acfunknown.h"
#include "ACFPtr.h"
#include <set>

class IACFUnknown;
class IACFPluginDefinition;
class AAX_IACFCollection;
class AAX_IEffectDescriptor;

/**
 *	\brief Version-managed concrete \ref AAX_ICollection class
 *
 */
class AAX_VCollection : public AAX_ICollection
{
public:
	AAX_VCollection (IACFUnknown * pUnkHost);
	~AAX_VCollection () AAX_OVERRIDE;
	
	/** \copydoc AAX_ICollection::NewDescriptor()
	 *	
	 *	This implementation retains each generated \ref AAX_IEffectDescriptor and destroys the descriptor upon AAX_VCollection destruction
	 */
	AAX_IEffectDescriptor *	NewDescriptor () AAX_OVERRIDE; ///< \copydoc AAX_ICollection::NewDescriptor()
	AAX_Result				AddEffect ( const char * inEffectID, AAX_IEffectDescriptor * inEffectDescriptor ) AAX_OVERRIDE; ///< \copydoc AAX_ICollection::AddEffect()
	AAX_Result				SetManufacturerName( const char* inPackageName ) AAX_OVERRIDE; ///< \copydoc AAX_ICollection::SetManufacturerName()
	AAX_Result				AddPackageName( const char *inPackageName ) AAX_OVERRIDE; ///< \copydoc AAX_ICollection::AddPackageName()
	AAX_Result				SetPackageVersion( uint32_t inVersion ) AAX_OVERRIDE; ///< \copydoc AAX_ICollection::SetPackageVersion()
	AAX_IPropertyMap *		NewPropertyMap () AAX_OVERRIDE; ///< \copydoc AAX_ICollection::NewPropertyMap()
	AAX_Result				SetProperties ( AAX_IPropertyMap * inProperties ) AAX_OVERRIDE; ///< \copydoc AAX_ICollection::SetProperties()
	AAX_Result 				GetHostVersion(uint32_t* outVersion) const AAX_OVERRIDE; ///< \copydoc AAX_ICollection::GetHostVersion()
	
	AAX_IDescriptionHost* DescriptionHost() AAX_OVERRIDE; ///< \copydoc AAX_ICollection::DescriptionHost()
	const AAX_IDescriptionHost* DescriptionHost() const AAX_OVERRIDE; ///< \copydoc AAX_ICollection::DescriptionHost() const
	IACFDefinition* HostDefinition() const AAX_OVERRIDE; ///< \copydoc AAX_ICollection::HostDefinition() const
	
	IACFPluginDefinition*			GetIUnknown() const;

private:
	ACFPtr<IACFUnknown>					mUnkHost;
	ACFPtr<AAX_IACFCollection>			mIACFCollection;
	AAX_VDescriptionHost				mDescriptionHost;
	std::set<AAX_IEffectDescriptor *>	mEffectDescriptors;
    std::set<AAX_IPropertyMap *>		mPropertyMaps;	
};

#endif

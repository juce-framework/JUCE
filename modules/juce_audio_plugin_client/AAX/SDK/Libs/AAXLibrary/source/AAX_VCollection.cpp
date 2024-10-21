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

#include "AAX_VCollection.h"
#include "AAX_VEffectDescriptor.h"
#include "AAX_VPropertyMap.h"

#include "AAX_UIDs.h"
#include "acfbaseapi.h"

// ******************************************************************************************
// METHOD:	AAX_VCollection
// ******************************************************************************************
AAX_VCollection::AAX_VCollection (IACFUnknown * pUnkHost) :
	mUnkHost( pUnkHost ),
	mIACFCollection(NULL),
	mDescriptionHost(pUnkHost)
{
	if ( mUnkHost )
	{
		// Get the component factory service from the host so we can create the 
		// built-in plug-in definition.
		ACFPtr<IACFComponentFactory> pFactory;
		if ( pUnkHost->QueryInterface(IID_IACFComponentFactory, (void **)&pFactory) == ACF_OK )
		{
			// Create the object and get the base interface for it.
			pFactory->CreateComponent(AAXCompID_AAXCollection, 0, IID_IAAXCollectionV1, (void **)&mIACFCollection);
		}
	}
}

// ******************************************************************************************
// METHOD:	~AAX_VCollection
// ******************************************************************************************
AAX_VCollection::~AAX_VCollection ()
{
	for (std::set<AAX_IEffectDescriptor *>::iterator iter = mEffectDescriptors.begin (); iter != mEffectDescriptors.end (); ++iter )
		delete *iter;
	
	for (std::set<AAX_IPropertyMap *>::iterator iter = mPropertyMaps.begin (); iter != mPropertyMaps.end (); ++iter)
		delete *iter;
}

// ******************************************************************************************
// METHOD:	GetIUnknown
// ******************************************************************************************
IACFPluginDefinition*	
AAX_VCollection::GetIUnknown(void) const
{
	return mIACFCollection;
}

// ******************************************************************************************
// METHOD:	NewDescriptor
// ******************************************************************************************
AAX_IEffectDescriptor * AAX_VCollection::NewDescriptor ()
{
	AAX_VEffectDescriptor * plugInDescriptor = new AAX_VEffectDescriptor( mUnkHost );
	mEffectDescriptors.insert( plugInDescriptor );
	return plugInDescriptor;
}


// ******************************************************************************************
// METHOD:	AddEffect
// ******************************************************************************************
AAX_Result AAX_VCollection::AddEffect ( const char * inEffectID, AAX_IEffectDescriptor * inEffectDescriptor )
{
	if ( mIACFCollection && inEffectDescriptor )
		return mIACFCollection->AddEffect( inEffectID, static_cast<AAX_VEffectDescriptor*>(inEffectDescriptor)->GetIUnknown() );
		
	return AAX_ERROR_NULL_OBJECT;
}

// ******************************************************************************************
// METHOD:	SetManufacturerName
// ******************************************************************************************
AAX_Result AAX_VCollection::SetManufacturerName( const char * iManufacturerName )
{
	if ( mIACFCollection )
		return mIACFCollection->SetManufacturerName( iManufacturerName );
		
	return AAX_ERROR_NULL_OBJECT;
}

// ******************************************************************************************
// METHOD:	AddPackageName
// ******************************************************************************************
AAX_Result AAX_VCollection::AddPackageName( const char * iPackageName )
{
	if ( mIACFCollection )
		return mIACFCollection->AddPackageName( iPackageName );
		
	return AAX_ERROR_NULL_OBJECT;
}

// ******************************************************************************************
// METHOD:	SetPackageVersion
// ******************************************************************************************
AAX_Result AAX_VCollection::SetPackageVersion( uint32_t iPackageVersion )
{
	if ( mIACFCollection )
		return mIACFCollection->SetPackageVersion( iPackageVersion );
		
	return AAX_ERROR_NULL_OBJECT;
}

// ******************************************************************************************
// METHOD:	NewPropertyMap
// ******************************************************************************************
AAX_IPropertyMap * AAX_VCollection::NewPropertyMap ()
{
	AAX_VPropertyMap * propertyMap = AAX_VPropertyMap::Create( mUnkHost );
	mPropertyMaps.insert( propertyMap );
	return propertyMap;
}

// ******************************************************************************************
// METHOD:	SetProperties
// ******************************************************************************************
AAX_Result AAX_VCollection::SetProperties ( AAX_IPropertyMap * inProperties )
{
	if ( mIACFCollection )
		return mIACFCollection->SetProperties( inProperties ? inProperties->GetIUnknown() : NULL);

	return AAX_ERROR_NULL_OBJECT;
}

// ******************************************************************************************
// METHOD:	DescriptionHost
// ******************************************************************************************
AAX_IDescriptionHost* AAX_VCollection::DescriptionHost()
{
	return mDescriptionHost.Supported() ? &mDescriptionHost : NULL;
}
const AAX_IDescriptionHost* AAX_VCollection::DescriptionHost() const
{
	return mDescriptionHost.Supported() ? &mDescriptionHost : NULL;
}

// ******************************************************************************************
// METHOD:	HostDefinition
// ******************************************************************************************
IACFDefinition* AAX_VCollection::HostDefinition() const
{
	return mDescriptionHost.HostDefinition();
}

AAX_Result AAX_VCollection::GetHostVersion(uint32_t* outVersion) const
{
	if (!outVersion)
		return AAX_ERROR_NULL_ARGUMENT;

	if (IACFDefinition* hostDefinition = mDescriptionHost.HostDefinition())
	{
		ACFRESULT acfErr;
		acfUID typeID = acfUID_NULL;
		acfUInt32 attrDataSize = 0;
		acfErr = hostDefinition->GetAttributeInfo(AAXATTR_Client_Version, &typeID, &attrDataSize);
		if (acfErr != ACF_OK)
			return acfErr;

		uint32_t version;
		if (attrDataSize != sizeof(version))
			return AAX_ERROR_INVALID_ARGUMENT;

		acfErr = hostDefinition->CopyAttribute(AAXATTR_Client_Version, typeID, &version, attrDataSize);
		if (acfErr != ACF_OK)
			return acfErr;

		*outVersion = version;
		return AAX_SUCCESS;
	}
	return AAX_ERROR_NULL_OBJECT;
}

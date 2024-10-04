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

#include "AAX_VEffectDescriptor.h"
#include "AAX_VComponentDescriptor.h"
#include "AAX_VPropertyMap.h"

#include "AAX_UIDs.h"
#include "acfbaseapi.h"

// ******************************************************************************************
// METHOD:	AAX_VEffectDescriptor
// ******************************************************************************************
AAX_VEffectDescriptor::AAX_VEffectDescriptor ( IACFUnknown * pUnkHost ) :
	mUnkHost( pUnkHost ),
	mIACFEffectDescriptor( NULL ),
	mIACFEffectDescriptorV2( NULL )
{
	if ( mUnkHost )
	{
		// Get the component factory service from the host so we can create the 
		// built-in plug-in definition.
		ACFPtr<IACFComponentFactory> pFactory;
		if ( pUnkHost->QueryInterface(IID_IACFComponentFactory, (void **)&pFactory) == ACF_OK )
		{
			// Create the object and get the base interface for it.
			pFactory->CreateComponent(AAXCompID_AAXEffectDescriptor, 0, IID_IAAXEffectDescriptorV1, (void **)&mIACFEffectDescriptor);
            if (mIACFEffectDescriptor)
                mIACFEffectDescriptor->QueryInterface(IID_IAAXEffectDescriptorV2, (void**)&mIACFEffectDescriptorV2);
		}
	}
}

// ******************************************************************************************
// METHOD:	~AAX_VEffectDescriptor
// ******************************************************************************************
AAX_VEffectDescriptor::~AAX_VEffectDescriptor ()
{
	std::set<AAX_IComponentDescriptor *>::iterator iterComponentDescriptor = mComponentDescriptors.begin ();
	for ( ; iterComponentDescriptor != mComponentDescriptors.end (); ++iterComponentDescriptor )
		delete *iterComponentDescriptor;

	std::set<AAX_IPropertyMap *>::iterator iterPropertyMap = mPropertyMaps.begin ();
	for ( ; iterPropertyMap != mPropertyMaps.end (); ++iterPropertyMap )
		delete *iterPropertyMap;
}


// ******************************************************************************************
// METHOD:	GetIUnknown
// ******************************************************************************************
IACFUnknown*							
AAX_VEffectDescriptor::GetIUnknown(void) const
{
	return mIACFEffectDescriptor;
}

// ******************************************************************************************
// METHOD:	NewComponentDescriptor
// ******************************************************************************************
AAX_IComponentDescriptor * AAX_VEffectDescriptor::NewComponentDescriptor ()
{
	AAX_VComponentDescriptor * componentDescriptor = new AAX_VComponentDescriptor( mUnkHost );
	mComponentDescriptors.insert( componentDescriptor );
	return componentDescriptor;
}

// ******************************************************************************************
// METHOD:	AddComponent
// ******************************************************************************************
AAX_Result AAX_VEffectDescriptor::AddComponent ( AAX_IComponentDescriptor * inComponentDescriptor )
{
	if ( mIACFEffectDescriptor )
		return mIACFEffectDescriptor->AddComponent( inComponentDescriptor ? static_cast<AAX_VComponentDescriptor*>(inComponentDescriptor)->GetIUnknown() : NULL );
	
	return AAX_ERROR_NULL_OBJECT;
}

// ******************************************************************************************
// METHOD:	AddName
// ******************************************************************************************
AAX_Result AAX_VEffectDescriptor::AddName( const char * inPlugInName )
{
	if ( mIACFEffectDescriptor )
		return mIACFEffectDescriptor->AddName( inPlugInName );
	
	return AAX_ERROR_NULL_OBJECT;
}

// ******************************************************************************************
// METHOD:	AddCategory
// ******************************************************************************************
AAX_Result AAX_VEffectDescriptor::AddCategory( uint32_t inCategory )
{
	if ( mIACFEffectDescriptor )
		return mIACFEffectDescriptor->AddCategory( inCategory );
	
	return AAX_ERROR_NULL_OBJECT;
}

// ******************************************************************************************
// METHOD:	AddCategoryBypassParameter
// ******************************************************************************************
AAX_Result AAX_VEffectDescriptor::AddCategoryBypassParameter ( uint32_t inCategory, AAX_CParamID inParamID )
{
	if ( mIACFEffectDescriptor )
		return mIACFEffectDescriptor->AddCategoryBypassParameter ( inCategory, inParamID );
	
	return AAX_ERROR_NULL_OBJECT;
}

// ******************************************************************************************
// METHOD:	AddProcPtr
// ******************************************************************************************
AAX_Result AAX_VEffectDescriptor::AddProcPtr( void * inProcPtr, AAX_CProcPtrID inProcID )
{
	if ( mIACFEffectDescriptor )
		return mIACFEffectDescriptor->AddProcPtr( inProcPtr, inProcID );
	
	return AAX_ERROR_NULL_OBJECT;
}

// ******************************************************************************************
// METHOD:	NewPropertyMap
// ******************************************************************************************
AAX_IPropertyMap * AAX_VEffectDescriptor::NewPropertyMap ()
{
	AAX_VPropertyMap * propertyMap = AAX_VPropertyMap::Create( mUnkHost );
	mPropertyMaps.insert( propertyMap );
	return propertyMap;	
}

// ******************************************************************************************
// METHOD:	SetProperties
// ******************************************************************************************
AAX_Result AAX_VEffectDescriptor::SetProperties ( AAX_IPropertyMap * inProperties )
{
	if ( mIACFEffectDescriptor )
		return mIACFEffectDescriptor->SetProperties( inProperties ? inProperties->GetIUnknown() : NULL);

	return AAX_ERROR_NULL_OBJECT;
}

// ******************************************************************************************
// METHOD:	AddResourceInfo
// ******************************************************************************************
AAX_Result AAX_VEffectDescriptor::AddResourceInfo ( AAX_EResourceType inResourceType, const char * inFileName )
{
	if ( mIACFEffectDescriptor )
		return mIACFEffectDescriptor->AddResourceInfo ( inResourceType, inFileName );
	
	return AAX_ERROR_NULL_OBJECT;
}

// ******************************************************************************************
// METHOD:	AddMeterDescription
// ******************************************************************************************
AAX_Result AAX_VEffectDescriptor::AddMeterDescription( AAX_CTypeID inMeterID, const char * inMeterName, AAX_IPropertyMap * inProperties )
{
	if ( mIACFEffectDescriptor )
	{
		IACFUnknown* propMap = inProperties ? inProperties->GetIUnknown() : NULL;
		return mIACFEffectDescriptor->AddMeterDescription( inMeterID, inMeterName, propMap );
	}

	return AAX_ERROR_NULL_OBJECT;
}

// ******************************************************************************************
// METHOD:	AddControlMIDINode
// ******************************************************************************************
AAX_Result AAX_VEffectDescriptor::AddControlMIDINode ( AAX_CTypeID inNodeID, AAX_EMIDINodeType inNodeType, const char inNodeName[], uint32_t channelMask )
{
	if ( mIACFEffectDescriptorV2 )
		return mIACFEffectDescriptorV2->AddControlMIDINode( inNodeID, inNodeType, inNodeName, channelMask );

	return AAX_ERROR_UNIMPLEMENTED;
}

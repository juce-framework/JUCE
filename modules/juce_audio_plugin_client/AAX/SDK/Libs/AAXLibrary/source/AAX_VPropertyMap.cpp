/*================================================================================================*/
/*
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
/*================================================================================================*/

// Self Include
#include "AAX_VPropertyMap.h"

// AAX Includes
#include "AAX_VDescriptionHost.h"
#include "AAX_VComponentDescriptor.h"
#include "AAX_IACFDescriptionHost.h"
#include "AAX_UIDs.h"

// ACF Includes
#include "acfbaseapi.h"

// Standard Includes
#include <memory>


// ******************************************************************************************
// METHOD:	Create
// ******************************************************************************************
/* static */
AAX_VPropertyMap* AAX_VPropertyMap::Create( IACFUnknown * inComponentFactory )
{
	AAX_UNIQUE_PTR(AAX_VPropertyMap) newMap;
	if ( inComponentFactory )
	{
		// Get the component factory service from the host so we can create the
		// built-in plug-in definition.
		ACFPtr<IACFComponentFactory> pFactory;
		if ( inComponentFactory->QueryInterface(IID_IACFComponentFactory, (void **)&pFactory) == ACF_OK )
		{
			newMap.reset(new AAX_VPropertyMap);
			newMap->InitWithFactory(pFactory.inArg(), inComponentFactory);
		}
	}
	return newMap.release();
}

// ******************************************************************************************
// METHOD:	Acquire
// ******************************************************************************************
/* static */
AAX_VPropertyMap* AAX_VPropertyMap::Acquire( IACFUnknown * inPropertyMapUnknown )
{
	AAX_UNIQUE_PTR(AAX_VPropertyMap) newMap(new AAX_VPropertyMap);
	
	// We don't actually expect the property map to support IID_IAAXDescriptionHostV1, but we pass it in
	// as the auxiliary unknown here to avoid a recompile requirement if we ever find we need to support
	// the description host interface (see the note in AAX_VPropertyMap::AddProperty() )
	newMap->InitWithPropertyMap(inPropertyMapUnknown, inPropertyMapUnknown);
	
	return newMap.release();
}

// ******************************************************************************************
// METHOD:	AAX_VPropertyMap
// ******************************************************************************************
AAX_VPropertyMap::AAX_VPropertyMap() :
	mIACFPropertyMap( NULL ),
    mIACFPropertyMapV2( NULL ),
	mIACFPropertyMapV3( NULL ),
	mIACFDescriptionHost( NULL )
{
}

// ******************************************************************************************
// METHOD:	InitWithFactory
// ******************************************************************************************
void AAX_VPropertyMap::InitWithFactory(IACFComponentFactory *inComponentFactory, IACFUnknown* inAuxiliaryUnknown)
{
	if (inComponentFactory)
	{
		// Create the object and get the base interface for it.
		inComponentFactory->CreateComponent(AAXCompID_AAXPropertyMap, 0, IID_IAAXPropertyMapV1, (void **)&mIACFPropertyMap);
		
		if (mIACFPropertyMap)
		{
			mIACFPropertyMap->QueryInterface(IID_IAAXPropertyMapV2, (void **)&mIACFPropertyMapV2);
			mIACFPropertyMap->QueryInterface(IID_IAAXPropertyMapV3, (void **)&mIACFPropertyMapV3);
		}
		
		// Get the AAX_IACFDescriptionHost, if supported
		//
		// It's possible that any of the accessible interfaces could serve the description host; we start
		// with the auxiliary as the most likely.
		if (inAuxiliaryUnknown)
		{
			inAuxiliaryUnknown->QueryInterface(IID_IAAXDescriptionHostV1, (void**)&mIACFDescriptionHost);
		}
		if (mIACFDescriptionHost.isNull())
		{
			inComponentFactory->QueryInterface(IID_IAAXDescriptionHostV1, (void**)&mIACFDescriptionHost);
		}
		if (mIACFPropertyMapV2 && mIACFDescriptionHost.isNull())
		{
			mIACFPropertyMapV2->QueryInterface(IID_IAAXDescriptionHostV1, (void**)&mIACFDescriptionHost);
		}
		if (mIACFPropertyMapV3 && mIACFDescriptionHost.isNull())
		{
			mIACFPropertyMapV3->QueryInterface(IID_IAAXDescriptionHostV1, (void**)&mIACFDescriptionHost);
		}
		if (mIACFPropertyMap && mIACFDescriptionHost.isNull())
		{
			mIACFPropertyMap->QueryInterface(IID_IAAXDescriptionHostV1, (void**)&mIACFDescriptionHost);
		}
	}
}

// ******************************************************************************************
// METHOD:	InitWithPropertyMap
// ******************************************************************************************
void AAX_VPropertyMap::InitWithPropertyMap(IACFUnknown *inPropertyMapUnknown, IACFUnknown* inAuxiliaryUnknown)
{
	if (inPropertyMapUnknown)
	{
		inPropertyMapUnknown->QueryInterface(IID_IAAXPropertyMapV1, (void **)&mIACFPropertyMap);
		inPropertyMapUnknown->QueryInterface(IID_IAAXPropertyMapV2, (void **)&mIACFPropertyMapV2);
		inPropertyMapUnknown->QueryInterface(IID_IAAXPropertyMapV3, (void **)&mIACFPropertyMapV3);
		
		// Get the AAX_IACFDescriptionHost, if supported
		//
		// It's possible that any of the accessible interfaces could serve the description host; we start
		// with the auxiliary as the most likely.
		if (inAuxiliaryUnknown)
		{
			inAuxiliaryUnknown->QueryInterface(IID_IAAXDescriptionHostV1, (void**)&mIACFDescriptionHost);
		}
		if (mIACFDescriptionHost.isNull())
		{
			inPropertyMapUnknown->QueryInterface(IID_IAAXDescriptionHostV1, (void**)&mIACFDescriptionHost);
		}
	}
}

// ******************************************************************************************
// METHOD:	~AAX_VPropertyMap
// ******************************************************************************************
AAX_VPropertyMap::~AAX_VPropertyMap(void)
{
}

// ******************************************************************************************
// METHOD:	GetIUnknown
// ******************************************************************************************
IACFUnknown * AAX_VPropertyMap::GetIUnknown ()
{
	if (!mIACFPropertyMapV3.isNull()) { return mIACFPropertyMapV3; }
	if (!mIACFPropertyMapV2.isNull()) { return mIACFPropertyMapV2; }
	if (!mIACFPropertyMap.isNull()) { return mIACFPropertyMap; }
	return NULL;
}

// ******************************************************************************************
// METHOD:	GetProperty
// ******************************************************************************************
AAX_CBoolean AAX_VPropertyMap::GetProperty ( AAX_EProperty inProperty, AAX_CPropertyValue * outValue ) const
{
	if ( mIACFPropertyMap )
		return mIACFPropertyMap->GetProperty ( inProperty, outValue );
	
	return 0;
}

// ******************************************************************************************
// METHOD:	GetProperty
// ******************************************************************************************
AAX_CBoolean AAX_VPropertyMap::GetPointerProperty ( AAX_EProperty inProperty, const void** outValue ) const
{
	AAX_CBoolean result = 0;
	
#if (AAX_PointerSize == AAXPointer_32bit)
	if ( mIACFPropertyMap )
	{
		result = mIACFPropertyMap->GetProperty ( inProperty, reinterpret_cast<AAX_CPointerPropertyValue*>(outValue) );
	}
#elif (AAX_PointerSize == AAXPointer_64bit)
	if ( mIACFPropertyMapV3 )
	{
		result = mIACFPropertyMapV3->GetProperty64 ( inProperty, reinterpret_cast<AAX_CPointerPropertyValue*>(outValue) );
	}
	else
	{
		// See note in AddPointerProperty()
		if ((NULL != outValue) && (0 < mLocalPointerPropertyCache.count(inProperty)))
		{
			*outValue = const_cast<AAX_VPropertyMap*>(this)->mLocalPointerPropertyCache[inProperty]; // we still want to support some STL versions that don't have std::map::at()
			result = 1;
		}
		else
		{
			result = 0;
		}
	}
#else
	#error unexpected pointer size
#endif
	
	return result;
}

// ******************************************************************************************
// METHOD:	AddProperty
// ******************************************************************************************
AAX_Result AAX_VPropertyMap::AddProperty ( AAX_EProperty inProperty, AAX_CPropertyValue inValue )
{
	// PT-223581: Pro Tools removes plug-ins from the insert menu if unsupported stem formats are detected
	if ( (AAX_eProperty_InputStemFormat == inProperty) || (AAX_eProperty_OutputStemFormat == inProperty))
	{
		// HACK: using support for AAX_IACFDescriptionHost as an indication of whether this bug has been addressed in the host
		//
		// IMPORTANT NOTE: This can fire with a false positive (i.e. return an error code) for AAX_VPropertyMap objects which
		// were intentionally created without a description host. Currently we only expect this in the case of property maps
		// generated from AAX_IFeatureInfo objects, and those property maps are const so this method will never be called.
		if (!mIACFDescriptionHost)
		{
			if (( (AAX_STEM_FORMAT_INDEX(inValue) < AAX_STEM_FORMAT_INDEX(AAX_eStemFormat_Mono)) || (AAX_STEM_FORMAT_INDEX(inValue) > AAX_STEM_FORMAT_INDEX(AAX_eStemFormat_7_1_DTS)) ) &&
				( (inValue != static_cast<AAX_CPropertyValue>(AAX_eStemFormat_Any)) ) &&
				( (inValue != static_cast<AAX_CPropertyValue>(AAX_eStemFormat_None)) ))
			{
				return AAX_ERROR_PROPERTY_UNDEFINED;
			}
		}
		// otherwise, it is fine to register stem formats which are unknown to the host
	}
	
	if ( mIACFPropertyMap )
	{
		return mIACFPropertyMap->AddProperty ( inProperty, inValue );
	}
	
	return AAX_ERROR_NULL_OBJECT;
}

// ******************************************************************************************
// METHOD:	AddPointerProperty
// ******************************************************************************************
AAX_Result AAX_VPropertyMap::AddPointerProperty ( AAX_EProperty inProperty, const void* inValue )
{
	AAX_Result err = AAX_ERROR_NULL_OBJECT;
	
#if (AAX_PointerSize == AAXPointer_32bit)
	if ( mIACFPropertyMap )
	{
		err = mIACFPropertyMap->AddProperty ( inProperty, reinterpret_cast<AAX_CPointerPropertyValue>(inValue) );
	}
#elif (AAX_PointerSize == AAXPointer_64bit)
	if ( mIACFPropertyMapV3 )
	{
		err = mIACFPropertyMapV3->AddProperty64 ( inProperty, reinterpret_cast<AAX_CPointerPropertyValue>(inValue) );
	}
	else
	{
		// Hack: To support the AddProcessProc() emulation in AAX_VComponentDescriptor::AddProcessProc() we
		// cache the pointer property values here which we know will be queried by that emulation without
		// actually setting them on the property map (of course this would all go away if we would just
		// use a property map object implemented and allocated on the AAX Library side, but oh well.)
		const std::set<AAX_EProperty>& pointerPropertiesToCache = AAX_VComponentDescriptor::PointerPropertiesUsedByAddProcessProc();
		if (0 < pointerPropertiesToCache.count(inProperty))
		{
			mLocalPointerPropertyCache[inProperty] = inValue;
			err = AAX_SUCCESS;
		}
		else
		{
			// use unimplemented for interface versions > 1
			err = AAX_ERROR_UNIMPLEMENTED;
		}
	}
#else
	#error unexpected pointer size
#endif
	
	return err;
}

// ******************************************************************************************
// METHOD:	AddPointerProperty
// ******************************************************************************************
AAX_Result AAX_VPropertyMap::AddPointerProperty ( AAX_EProperty inProperty, const char* inValue )
{
	return this->AddPointerProperty(inProperty, reinterpret_cast<const void*>(inValue));
}

// ******************************************************************************************
// METHOD:	RemoveProperty
// ******************************************************************************************
AAX_Result AAX_VPropertyMap::RemoveProperty ( AAX_EProperty inProperty )
{
	if ( mIACFPropertyMap )
		return mIACFPropertyMap->RemoveProperty ( inProperty );
	return AAX_ERROR_NULL_OBJECT;
}

// ******************************************************************************************
// METHOD:	AddPropertyWithIDArray
// ******************************************************************************************
AAX_Result AAX_VPropertyMap::AddPropertyWithIDArray ( AAX_EProperty iProperty, const AAX_SPlugInIdentifierTriad* iPluginIDs, uint32_t iNumPluginIDs)
{
    if (mIACFPropertyMapV2)
        return mIACFPropertyMapV2->AddPropertyWithIDArray(iProperty, iPluginIDs, iNumPluginIDs);
    return AAX_ERROR_UNIMPLEMENTED;
}

// ******************************************************************************************
// METHOD:	GetPropertyWithIDArray
// ******************************************************************************************
AAX_CBoolean AAX_VPropertyMap::GetPropertyWithIDArray ( AAX_EProperty iProperty, const AAX_SPlugInIdentifierTriad** oPluginIDs, uint32_t* oNumPluginIDs) const
{
	AAX_CBoolean	result = 0;

    if (mIACFPropertyMapV2)
        return mIACFPropertyMapV2->GetPropertyWithIDArray(iProperty, oPluginIDs, oNumPluginIDs);

    return result;
}


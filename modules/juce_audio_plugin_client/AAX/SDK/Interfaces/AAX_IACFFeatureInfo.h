/*================================================================================================*/
/*
 *	Copyright 2016-2017, 2023-2024 Avid Technology, Inc.
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
 */

#ifndef AAXLibrary_AAX_IACFFeatureInfo_h
#define AAXLibrary_AAX_IACFFeatureInfo_h

#include "AAX.h"

class AAX_IACFPropertyMap;

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wnon-virtual-dtor"
#endif

#include "acfunknown.h"

/** Information about host support for a particular feature
 
 Acquired using \ref AAX_IACFDescriptionHost::AcquireFeatureProperties()
 
 This interface is shared between multiple features. The specific feature which this object
 represents is the feature whose ID was used in the call to acquire this interface.
 
 See the feature UID documentation for which properties support additional property map data
 
 IID: \ref IID_IAAXFeatureInfoV1
 
 \note Do not \c QueryInterface() for \ref IID_IAAXFeatureInfoV1 since this does not
 indicate which specific feature is being requested. Instead, use
 \ref AAX_IDescriptionHost::AcquireFeatureProperties()
 */
class AAX_IACFFeatureInfo : public IACFUnknown
{
public:
	// NOTE: Documentation is copied directly from AAX_IFeatureInfo despite an adaptation of parameter types (AAX_ESupportLevel* to AAX_ESupportLevel&)
	/**
	 *	\copydoc AAX_IFeatureInfo::SupportLevel()
	 *
	 *	\sa AAX_IFeatureInfo::SupportLevel()
	 */
	virtual AAX_Result SupportLevel(AAX_ESupportLevel* oSupportLevel) const = 0; ///< \copydoc AAX_IFeatureInfo::SupportLevel()
	
	// NOTE: Documentation is not copied directly from AAX_IFeatureInfo due to an adaptation of parameter types (IACFUnknown to AAX_IPropertyMap)
	/**
	 *	\copybrief AAX_IFeatureInfo::AcquireProperties()
	 *
	 *	\p outProperties must support \ref AAX_IACFPropertyMap const methods
	 *
	 *	\sa AAX_IFeatureInfo::AcquireProperties()
	 */
	virtual AAX_Result AcquireProperties(IACFUnknown** outProperties) = 0;
};

#ifdef __clang__
#pragma clang diagnostic pop
#endif


#endif

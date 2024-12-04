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

#ifndef AAXLibrary_AAX_IACFDescriptionHost_h
#define AAXLibrary_AAX_IACFDescriptionHost_h


#include "AAX.h"

class AAX_IACFFeatureInfo;

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wnon-virtual-dtor"
#endif

#include "acfbaseapi.h"
#include "acfunknown.h"

/** Interface to host services provided during plug-in description
 */
class AAX_IACFDescriptionHost : public IACFUnknown
{
public:
	// NOTE: Documentation is not copied directly from AAX_IDescriptionHost due to an adaptation of parameter types (IACFUnknown to AAX_IFeatureInfo)
	/**
	 *	\copybrief AAX_IDescriptionHost::AcquireFeatureProperties()
	 *
	 *	\p outFeatureProperties must support \ref AAX_IACFFeatureInfo const methods
	 *
	 *	\sa AAX_IDescriptionHost::AcquireFeatureProperties()
	 */
	virtual AAX_Result AcquireFeatureProperties(const AAX_Feature_UID& inFeatureID, IACFUnknown** outFeatureProperties) = 0;
};

#ifdef __clang__
#pragma clang diagnostic pop
#endif

#endif

/*================================================================================================*/
/*
 *	Copyright 2016-2017, 2019, 2023-2024 Avid Technology, Inc.
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

#ifndef AAXLibrary_AAX_VFeatureInfo_h
#define AAXLibrary_AAX_VFeatureInfo_h

#include "AAX_IFeatureInfo.h"

#include "ACFPtr.h"
#include "acfbasetypes.h"


class AAX_IPropertyMap;
class AAX_IACFFeatureInfo;


/** Concrete implementation of \ref AAX_IFeatureInfo, which provides a version-controlled
 interface to host feature information
 */
class AAX_VFeatureInfo : public AAX_IFeatureInfo
{
public:
	explicit AAX_VFeatureInfo( IACFUnknown* pUnknown, const AAX_Feature_UID& inFeatureID );
	~AAX_VFeatureInfo() AAX_OVERRIDE;
	
public: // AAX_IFeatureInfo
	AAX_Result SupportLevel(AAX_ESupportLevel& oSupportLevel) const AAX_OVERRIDE; ///< \copydoc AAX_IFeatureInfo::SupportLevel()
	const AAX_IPropertyMap* AcquireProperties() const AAX_OVERRIDE; ///< \copydoc AAX_IFeatureInfo::AcquireProperties()
	const AAX_Feature_UID& ID() const AAX_OVERRIDE; ///< \copydoc AAX_IFeatureInfo::ID()
	
private:
	AAX_Feature_UID mFeatureID;
	ACFPtr<AAX_IACFFeatureInfo> mIFeature;
};


#endif

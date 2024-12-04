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

#ifndef AAXLibrary_AAX_IFeatureInfo_h
#define AAXLibrary_AAX_IFeatureInfo_h

#include "AAX.h"


class AAX_IPropertyMap;


class AAX_IFeatureInfo
{
public:
	virtual ~AAX_IFeatureInfo() {}
	
public: // AAX_IACFFeatureInfo
	/** Determine the level of support for this feature by the host
	 
	 \note The host will not provide an underlying \ref AAX_IACFFeatureInfo interface for features which it does not
	 recognize at all, resulting in a \ref AAX_ERROR_NULL_OBJECT error code
	 */
	virtual AAX_Result SupportLevel(AAX_ESupportLevel& oSupportLevel) const = 0;
	
	/** Additional properties providing details of the feature support
	 
	 See the feature's UID for documentation of which features provide additional properties
	 
	 Ownership of the returned object is passed to the caller; the caller is responsible for destroying the object, e.g. by
	 capturing the returned object in a smart pointer.
	 
	 \code
	 // AAX_IFeatureInfo* featureInfo
	 std::unique_ptr<const AAX_IPropertyMap> featurePropertiesPtr(featureInfo->AcquireProperties();
	 \endcode
	 
	 \return An \ref AAX_IPropertyMap interface with access to the host's properties for this feature.
	 \return \c NULL if the desired feature was not found or if an error occurred
	 
	 \note May return an \ref AAX_IPropertyMap object with limited method support, which would return an error such as
	 \ref AAX_ERROR_NULL_OBJECT or \ref AAX_ERROR_UNIMPLEMENTED to interface calls.
	 
	 */
	virtual const AAX_IPropertyMap* AcquireProperties() const = 0;
	
public: // AAX_IFeatureInfo
	/** Returns the ID of the feature which this object represents
	 */
	virtual const AAX_Feature_UID& ID() const = 0;
};


#endif

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

#ifndef AAXLibrary_AAX_IDescriptionHost_h
#define AAXLibrary_AAX_IDescriptionHost_h

#include "AAX.h"

class AAX_IFeatureInfo;


/** Interface to host services provided during plug-in description
 */
class AAX_IDescriptionHost
{
public:
	virtual ~AAX_IDescriptionHost() {}
	
public:
	/** Get the client's feature object for a given feature ID
	 
	 Similar to \c QueryInterface() but uses a feature identifier rather than a true IID
	 
	 Ownership of the returned object is passed to the caller; the caller is responsible for destroying the object, e.g. by
	 capturing the returned object in a smart pointer.
	 
	 \code
	 // AAX_IDescriptionHost* descHost
	 std::unique_ptr<const AAX_IFeatureInfo> featureInfoPtr(descHost->AcquireFeatureProperties(someFeatureUID);
	 \endcode
	 
	 \return An \ref AAX_IFeatureInfo interface with access to the host's feature properties for this feature.
	 \return \c NULL if the desired feature was not found or if an error occurred
	 
	 \note May return an \ref AAX_IFeatureInfo object with limited method support, which would return an error such as
	 \ref AAX_ERROR_NULL_OBJECT or \ref AAX_ERROR_UNIMPLEMENTED to interface calls.
	 
	 \note If no \ref AAX_IFeatureInfo is provided then that may mean that the host is unaware of the feature, or it may mean
	 that the host is aware of the feature but has not implemented the %AAX feature support interface for this feature yet.
	 
	 \param[in] inFeatureID
	   Identifier of the requested feature
	 
	 */
	virtual const AAX_IFeatureInfo* AcquireFeatureProperties(const AAX_Feature_UID& inFeatureID) const = 0;
};

#endif

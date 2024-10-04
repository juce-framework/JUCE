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

#ifndef AAXLibrary_AAX_VDescriptionHost_h
#define AAXLibrary_AAX_VDescriptionHost_h


#include "AAX_IDescriptionHost.h"
#include "ACFPtr.h"


class AAX_IACFDescriptionHost;
class IACFDefinition;


/** Versioned wrapper for access to host service interfaces provided during plug-in description
 
 This object aggregates access to \ref AAX_IACFDescriptionHost and \ref IACFDefinition, with
 support depending on the interface support level of the \ref IACFUnknown which is passed to
 this object upon creation.
 */
class AAX_VDescriptionHost : public AAX_IDescriptionHost
{
public:
	explicit AAX_VDescriptionHost( IACFUnknown* pUnknown );
	~AAX_VDescriptionHost() AAX_OVERRIDE;
	
public: // AAX_IDescriptionHost
	const AAX_IFeatureInfo* AcquireFeatureProperties(const AAX_Feature_UID& inFeatureID) const AAX_OVERRIDE; ///< \copydoc AAX_IDescriptionHost::AcquireFeatureProperties()
	
public: // AAX_VDescriptionHost
	bool Supported() const { return !mDescriptionHost.isNull(); }
	AAX_IACFDescriptionHost* DescriptionHost() { return mDescriptionHost.inArg(); } // does not addref
	const AAX_IACFDescriptionHost* DescriptionHost() const { return mDescriptionHost.inArg(); } // does not addref
	IACFDefinition* HostDefinition() const { return mHostInformation.inArg(); } // does not addref
	
private:
	ACFPtr<AAX_IACFDescriptionHost> mDescriptionHost;
	ACFPtr<IACFDefinition> mHostInformation;
};




#endif // AAXLibrary_AAX_VDescriptionHost_h

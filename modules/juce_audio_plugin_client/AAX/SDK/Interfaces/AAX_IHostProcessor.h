/*================================================================================================*/
/*
 *
 *	Copyright 2013-2017, 2019-2021, 2023-2024 Avid Technology, Inc.
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
 *	\file  AAX_IHostProcessor.h
 *
 *	\brief Base class for the host processor interface which is extended by plugin code
 *
 */ 
/*================================================================================================*/


#ifndef AAX_IHOSTPROCESSOR_H
#define AAX_IHOSTPROCESSOR_H

#include "AAX_IACFHostProcessor.h"
#include "AAX.h"
#include "CACFUnknown.h"

/*! \brief	Base class for the host processor interface
	
	\details
	\pluginimp

	\note	This class always inherits from the latest version of the interface and thus requires any 
			subclass to implement all the methods in the latest version of the interface. Most plug-ins
			will inherit from the AAX_CHostProcessor convenience class.
 
	\ingroup AuxInterface_HostProcessor
*/
class AAX_IHostProcessor :	public AAX_IACFHostProcessor_V2,
							public CACFUnknown
{
public:
	ACF_DECLARE_STANDARD_UNKNOWN()
	
	ACFMETHOD(InternalQueryInterface)(const acfIID & riid, void **ppvObjOut) override;
	
	// CACFUnknown does not support operator=()
	AAX_DELETE(AAX_IHostProcessor& operator= (const AAX_IHostProcessor&));
};

#endif //AAX_IHOSTPROCESSOR_H

/*================================================================================================*/
/*
 *
 *	Copyright 2023-2025 Avid Technology, Inc.
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
 *	\file  AAX_ITaskAgent.h
 */ 
/*================================================================================================*/


#ifndef AAX_ITaskAgent_H
#define AAX_ITaskAgent_H

#include "AAX_IACFTaskAgent.h"
#include "AAX.h"
#include "CACFUnknown.h"


/** 
 * \brief Interface for a component that accepts task requests
 *
 * \copydetails AAX_IACFTaskAgent
 * 
 * \ingroup AuxInterface_TaskAgent
 */ 
class AAX_ITaskAgent : public AAX_IACFTaskAgent_V2
					 , public CACFUnknown
{
public:
	ACF_DECLARE_STANDARD_UNKNOWN()
	
	ACFMETHOD(InternalQueryInterface)(const acfIID & riid, void **ppvObjOut) AAX_OVERRIDE;
	
	// CACFUnknown does not support operator=()
	AAX_DELETE(AAX_ITaskAgent& operator= (const AAX_ITaskAgent&));
};

#endif //AAX_IEFFECTDIRECTDATA_H

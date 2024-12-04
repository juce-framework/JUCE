/*================================================================================================*/
/*
 *
 *	Copyright 2014-2017, 2019-2021, 2023-2024 Avid Technology, Inc.
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
 *	\file  AAX_IEffectDirectData.h
 *
 *	\brief Optional interface for direct access to alg memory
 *
 */ 
/*================================================================================================*/


#ifndef AAX_IEFFECTDIRECTDATA_H
#define AAX_IEFFECTDIRECTDATA_H

#include "AAX_IACFEffectDirectData.h"
#include "AAX.h"
#include "CACFUnknown.h"


/** @brief	The interface for a %AAX Plug-in's direct data interface.
	
	@details
	@pluginimp

	This is the interface for an instance of a plug-in's direct data interface that
	gets exposed to the host application.  A plug-in needs to inherit from this interface
	and override all of the virtual functions to support direct data access functionality.

	Direct data access allows a plug-in to directly manipulate the data in its algorithm's
	private data blocks.  The callback methods in this interface provide a safe context
	from which this kind of access may be attempted.
	
	\note This class always inherits from the latest version of the interface and thus requires any 
	subclass to implement all the methods in the latest version of the interface.
	
	\note See AAX_IACFEffectDirectData for further information.
	
	\ingroup AuxInterface_DirectData
*/
class AAX_IEffectDirectData :  public AAX_IACFEffectDirectData_V2,
							   public CACFUnknown
{	 
public:
	ACF_DECLARE_STANDARD_UNKNOWN()
	
	ACFMETHOD(InternalQueryInterface)(const acfIID & riid, void **ppvObjOut) override;
	
	// CACFUnknown does not support operator=()
	AAX_DELETE(AAX_IEffectDirectData& operator= (const AAX_IEffectDirectData&));
};

#endif //AAX_IEFFECTDIRECTDATA_H

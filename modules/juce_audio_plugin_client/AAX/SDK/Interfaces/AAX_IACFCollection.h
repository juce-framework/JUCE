/*================================================================================================*/
/*
 *
 *	Copyright 2013-2017, 2023-2024 Avid Technology, Inc.
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
 *	\file  AAX_IACFCollection.h
 *
 *	\brief Versioned interface to represent a plug-in binary's static description
 *
 */ 
/*================================================================================================*/


#ifndef AAX_IACFCOLLECTION_H
#define AAX_IACFCOLLECTION_H

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wnon-virtual-dtor"
#endif

#include "acfbaseapi.h"

class AAX_IEffectDescriptor;

/**	\brief Versioned interface to represent a plug-in binary's static description
 */
class AAX_IACFCollection : public IACFPluginDefinition
{
public:
	
	virtual	AAX_Result				AddEffect ( const char * inEffectID, IACFUnknown * inEffectDescriptor ) = 0;	///< \copydoc AAX_ICollection::AddEffect
	virtual	AAX_Result				SetManufacturerName( const char* inPackageName ) = 0;	///< \copydoc AAX_ICollection::SetManufacturerName()
	virtual	AAX_Result				AddPackageName( const char *inPackageName ) = 0;	///< \copydoc AAX_ICollection::AddPackageName()
	virtual	AAX_Result				SetPackageVersion( uint32_t inVersion ) = 0;	///< \copydoc AAX_ICollection::SetPackageVersion()
	virtual AAX_Result				SetProperties ( IACFUnknown * inProperties ) = 0; ///< \copydoc AAX_ICollection::SetProperties()
};

#ifdef __clang__
#pragma clang diagnostic pop
#endif

#endif

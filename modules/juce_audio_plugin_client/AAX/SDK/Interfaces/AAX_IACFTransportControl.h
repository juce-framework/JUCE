/*================================================================================================*/
/*
 *
 *	Copyright 2023-2024 Avid Technology, Inc.
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
 *	\file  AAX_IACFTransportControl.h
 *
 *	\brief Interface for control over the host's transport state
 */ 
/*================================================================================================*/

#ifndef AAX_IACFTRANSPORTCONTROL_H
#define AAX_IACFTRANSPORTCONTROL_H

#pragma once

#include "AAX.h"
#include "AAX_Enums.h"

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wnon-virtual-dtor"
#endif

#include "acfunknown.h"

/**	\brief Versioned interface to control the host's transport state
 */
class AAX_IACFTransportControl : public IACFUnknown
{
public:

	virtual AAX_Result RequestTransportStart() = 0;	///< \copydoc AAX_ITransport::RequestTransportStart()
	virtual AAX_Result RequestTransportStop() = 0;	///< \copydoc AAX_ITransport::RequestTransportStop()

};

#ifdef __clang__
#pragma clang diagnostic pop
#endif

#endif // AAX_IACFTRANSPORTCONTROL_H


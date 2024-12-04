/*================================================================================================*/
/*
 *
 *	Copyright 2014-2017, 2019, 2023-2024 Avid Technology, Inc.
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
 *	\file  AAX_VHostServices.h
 *
 *	\brief Version-managed concrete HostServices class
 *
 */ 
/*================================================================================================*/
#ifndef AAX_VHOSTSERVICES_H
#define AAX_VHOSTSERVICES_H

#include "AAX_IHostServices.h"
#include "AAX.h"
#include "acfunknown.h"
#include "ACFPtr.h"
#include "AAX_IACFHostServices.h"


class IACFUnknown;
class AAX_IACFHostServices;

/**
 *	\brief Version-managed concrete \ref AAX_IHostServices class
 *
 */
class AAX_VHostServices : public AAX_IHostServices
{
public:
	AAX_VHostServices( IACFUnknown * pUnkHost );
	~AAX_VHostServices( );

	AAX_Result HandleAssertFailure ( const char * iFile, int32_t iLine, const char * iNote, /* AAX_EAssertFlags */ int32_t iFlags ) const AAX_OVERRIDE; ///< \copydoc AAX_IHostServices::HandleAssertFailure()
	AAX_Result Trace ( int32_t iPriority, const char * iMessage ) const AAX_OVERRIDE; ///< \copydoc AAX_IHostServices::Trace()
	AAX_Result StackTrace ( int32_t iTracePriority, int32_t iStackTracePriority, const char * iMessage ) const AAX_OVERRIDE; ///< \copydoc AAX_IHostServices::StackTrace()

private:
	ACFPtr<AAX_IACFHostServices>	mIACFHostServices;
	ACFPtr<AAX_IACFHostServices_V2>	mIACFHostServices2;
	ACFPtr<AAX_IACFHostServices_V3>	mIACFHostServices3;
};



#endif //AAX_IAUTOMATIONDELEGATE_H




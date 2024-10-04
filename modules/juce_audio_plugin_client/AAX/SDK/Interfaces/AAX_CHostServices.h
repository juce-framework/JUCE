/*================================================================================================*/
/*
 *
 *	Copyright 2014-2015, 2018, 2023-2024 Avid Technology, Inc.
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
 *	\file  AAX_CHostServices.h
 *
 *	\brief Concrete implementation of the AAX_IHostServices interface.
 *
 */ 
/*================================================================================================*/


#ifndef AAX_CHOSTSERVICES_H
#define AAX_CHOSTSERVICES_H

#include "AAX.h"
#include "AAX_Enums.h"


class IACFUnknown;

/**	@brief Method access to a singleton implementation of the \ref AAX_IHostServices interface
 */
class AAX_CHostServices
{
public:
	static void Set ( IACFUnknown * pUnkHost );
	
	static AAX_Result HandleAssertFailure ( const char * iFile, int32_t iLine, const char * iNote, /* AAX_EAssertFlags */ int32_t iFlags = AAX_eAssertFlags_Default ); ///< \copydoc AAX_IHostServices::HandleAssertFailure()
	static AAX_Result Trace ( AAX_ETracePriorityHost iPriority, const char * iMessage, ... ); ///< \copydoc AAX_IHostServices::Trace()
	static AAX_Result StackTrace ( AAX_ETracePriorityHost iTracePriority, AAX_ETracePriorityHost iStackTracePriority, const char * iMessage, ... ); ///< \copydoc AAX_IHostServices::StackTrace()
};


#endif

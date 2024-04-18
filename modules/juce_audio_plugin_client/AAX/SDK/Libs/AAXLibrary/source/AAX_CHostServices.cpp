/*================================================================================================*/
/*
 *	Copyright 2013-2015, 2018, 2023-2024 Avid Technology, Inc.
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
/*================================================================================================*/


#include "AAX.h"
#include "AAX_CHostServices.h"
#include "AAX_VHostServices.h"
#include <cstdarg>
#include <cstdio>

static AAX_VHostServices* sHostServices = NULL;

// ***************************************************************************
// METHOD:	Set
// ***************************************************************************
void AAX_CHostServices::Set ( IACFUnknown * pUnkHost )
{
	if ( !sHostServices && pUnkHost )
	{
		sHostServices = new AAX_VHostServices(pUnkHost);
	}
	else if ( sHostServices && !pUnkHost )
	{
		delete sHostServices;
		sHostServices = NULL;
	}
}

// ***************************************************************************
// METHOD:	HandleAssertFailure
// ***************************************************************************
/* static */
AAX_Result AAX_CHostServices::HandleAssertFailure ( const char * inFile, int32_t inLine, const char * inNote, int32_t /* AAX_EAssertFlags */ inFlags )
{
	//Bail if the host does not support host services (e.g. unit tests)
	if (sHostServices == 0)
		return AAX_SUCCESS;

	return sHostServices->HandleAssertFailure ( inFile, inLine, inNote, inFlags );
}

// ***************************************************************************
// METHOD:	Trace
// ***************************************************************************
/* static */
AAX_Result AAX_CHostServices::Trace ( AAX_ETracePriorityHost inPriority, const char * inFormat, ... )
{
	//Bail if the host does not support host services (e.g. unit tests)
	if (sHostServices == 0)
		return AAX_SUCCESS;

	va_list	vargs;
	AAX_CONSTEXPR std::size_t bufferSize{512};
	char	message [ bufferSize ];
	
	va_start ( vargs, inFormat );
	auto const printReturn = vsnprintf( message, bufferSize, inFormat, vargs );
	va_end ( vargs );

	return 0 <= printReturn ? sHostServices->Trace ( (int32_t)inPriority, message ) : AAX_ERROR_PRINT_FAILURE;
}

// ***************************************************************************
// METHOD:	StackTrace
// ***************************************************************************
/* static */
AAX_Result AAX_CHostServices::StackTrace ( AAX_ETracePriorityHost inTracePriority, AAX_ETracePriorityHost inStackTracePriority, const char * inFormat, ... )
{
	//Bail if the host does not support host services (e.g. unit tests)
	if (sHostServices == 0)
		return AAX_SUCCESS;
	
	va_list	vargs;
	AAX_CONSTEXPR std::size_t bufferSize{512};
	char	message [ bufferSize ];
	
	va_start ( vargs, inFormat );
	auto const printReturn = vsnprintf( message, bufferSize, inFormat, vargs );
	va_end ( vargs );
	
	return 0 <= printReturn ? sHostServices->StackTrace ( (int32_t)inTracePriority, (int32_t)inStackTracePriority, message ) : AAX_ERROR_PRINT_FAILURE;
}

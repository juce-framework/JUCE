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
 *	\file  AAX_IHostServices.h
 *
 *	\brief Various host services
 *
 */ 
/*================================================================================================*/


#ifndef AAX_IHOSTSERVICES_H
#define AAX_IHOSTSERVICES_H

#include "AAX.h"

/**	\brief Interface to diagnostic and debugging services provided by the %AAX host
	
	\details
	\hostimp
	
	\sa \ref AAX_IACFHostServices
*/
class AAX_IHostServices
{
public:
	
	virtual ~AAX_IHostServices() {}
	
	/**	\brief Handle an assertion failure
	 
	 \details
	 Use this method to delegate assertion failure handling to the host
	 
	 Use \p inFlags to request that specific behavior be included when handling the
	 failure. This request may not be fulfilled by the host, and absence of a flag
	 does not preclude the host from using that behavior when handling the failure.
	 
	 \param[in] iFile
	 The name of the file containing the assert check. Usually \c __FILE__
	 \param[in] iLine
	 The line number of the assert check. Usually \c __LINE__
	 \param[in] iNote
	 Text to display related to the assert. Usually the condition which failed
	 \param[in] iFlags
	 Bitfield of \ref AAX_EAssertFlags to request specific handling behavior
	 */
	virtual AAX_Result HandleAssertFailure ( const char * iFile, int32_t iLine, const char * iNote, /* AAX_EAssertFlags */ int32_t iFlags ) const = 0;
	/**	\brief Log a trace message
	 
	 \param[in] iPriority
	 Priority of the trace, used for log filtering. One of \ref kAAX_Trace_Priority_Low, \ref kAAX_Trace_Priority_Normal, \ref kAAX_Trace_Priority_High
	 \param[in] iMessage
	 Message string to log
	 */
	virtual AAX_Result Trace ( int32_t iPriority, const char * iMessage ) const = 0;
	/** \brief Log a trace message or a stack trace
	 
	 If the logging output filtering is set to include logs with
	 \p iStackTracePriority then both the logging message and a stack trace will
	 be emitted, regardless of \p iTracePriority.
	 
	 If the logging output filtering is set to include logs with \p iTracePriority
	 but to exclude logs with \p iStackTracePriority then this will emit a normal
	 log with no stack trace.
	 
	 \param[in] iTracePriority
	 Priority of the trace, used for log filtering. One of \ref kAAX_Trace_Priority_Low, \ref kAAX_Trace_Priority_Normal, \ref kAAX_Trace_Priority_High
	 \param[in] iStackTracePriority
	 Priority of the stack trace, used for log filtering. One of \ref kAAX_Trace_Priority_Low, \ref kAAX_Trace_Priority_Normal, \ref kAAX_Trace_Priority_High
	 \param[in] iMessage
	 Message string to log
	 */
	virtual AAX_Result StackTrace ( int32_t iTracePriority, int32_t iStackTracePriority, const char * iMessage ) const = 0;
};

#endif // #ifndef AAX_IHOSTSERVICES_H

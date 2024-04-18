/*================================================================================================*/
/*
 *
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

/**  
	\file AAX_Assert.h
	
	\brief Declarations for cross-platform AAX_ASSERT, AAX_TRACE and related facilities
	
 	\details
	
 	- \ref AAX_ASSERT( condition ) - If the condition is \c false triggers some manner of
		warning, e.g. a dialog in a developer build or a DigiTrace log in a shipping
		build. May be used on host or TI.
	- \ref AAX_DEBUGASSERT( condition ) - Variant of \ref AAX_ASSERT which is only
		active in debug builds of the plug-in.
	- \ref AAX_TRACE_RELEASE( iPriority, iMessageStr [,params...] ) - Traces a printf-
		style message to the DigiTrace log file. Enabled using the \c DTF_AAXPLUGINS
		DigiTrace facility.
	- \ref AAX_TRACE( iPriority, iMessageStr [, params...] ) - Variant of \ref AAX_TRACE_RELEASE
		which only emits logs in debug builds of the plug-in.
	- \ref AAX_STACKTRACE_RELEASE( iPriority, iMessageStr [,params...] ) - Similar to
		\ref AAX_TRACE_RELEASE but prints a stack trace as well as a log message
	- \ref AAX_STACKTRACE( iPriority, iMessageStr [,params...] ) - Variant of
		\ref AAX_STACKTRACE_RELEASE which only emits logs in debug builds of the plug-in.
	- \ref AAX_TRACEORSTACKTRACE_RELEASE( iTracePriority, iStackTracePriority, iMessageStr [,params...] ) - Combination
		of \ref AAX_TRACE_RELEASE and \ref AAX_STACKTRACE_RELEASE; a stack trace is emitted if logging is enabled at
		\p iStackTracePriority. Otherwise, if logging is enabled at \p iTracePriority then emits a log.
	
	For all trace macros:
	
		\p inPriority is one of
			\li \ref kAAX_Trace_Priority_Low
			\li \ref kAAX_Trace_Priority_Normal
			\li \ref kAAX_Trace_Priority_High
			\li \ref kAAX_Trace_Priority_Critical
	
		These correspond to how the trace messages are filtered using
		\ref AAX_DigiTrace_Guide "DigiTrace".
		
		\note Disabling the <TT>DTF_AAXPLUGINS</TT> facility will slightly reduce the
		overhead of trace statements and chip communication on HDX systems.

==============================================================================*/


#ifndef AAX_ASSERT_H
#define AAX_ASSERT_H

#include "AAX_Enums.h"


/** \def AAX_TRACE
	\brief Print a trace statement to the log (debug plug-in builds only)
 
	\details
	Use this macro to print a trace statement to the log file from debug builds of a plug-in.
	
	<B>Notes </B>
	- This macro will be compiled out of release builds
	- This macro is compatible with bost host and embedded (AAX DSP) environments
	- Subject to a total line limit of 256 chars
	
	<B>Usage </B>
	Each invocation of this macro takes a trace priority and a <TT>printf</TT>-style logging string. For
	example:
	
		\code
		AAX_TRACE(kAAX_Trace_Priority_Normal, "My float: %f, My C-string: %s", myFloat, myCString);
		\endcode
	
	\sa AAX_DigiTrace_Guide
 */

/** \def AAX_TRACE_RELEASE
	\brief Print a trace statement to the log
 
	\details
	Use this macro to print a trace statement to the log file. This macro will be included in all builds
	of the plug-in.
 
	<B>Notes </B>
	- This macro is compatible with bost host and embedded (AAX DSP) environments
	- Subject to a total line limit of 256 chars
	
	<B>Usage </B>
	Each invocation of this macro takes a trace priority and a <TT>printf</TT>-style logging string.
	
	Because output from this macro will be enabled on end users' systems under certain tracing configurations,
	logs should always be formatted with some standard information to avoid confusion between logs from
	different plug-ins. This is the recommended formatting for AAX_TRACE_RELEASE logs:
 
		<DIV CLASS="TextInd1"><TT>[Manufacturer name] [Plug-in name] [Plug-in version][logging text (indented)] </TT></DIV>
 
	For example:
 
		 \code
		 AAX_TRACE_RELEASE(kAAX_Trace_Priority_Normal, "%s %s %s;\tMy float: %f, My C-string: %s",
		     "MyCompany", "MyPlugIn", "1.0.2", myFloat, myCString);
		 \endcode
 
	\sa AAX_DigiTrace_Guide
 */

/** \def AAX_ASSERT
	\brief Asserts that a condition is true and logs an error if the condition is false.
	
	\details
	<B>Notes </B>
	- This macro will be compiled out of release builds.
	- This macro is compatible with bost host and embedded (AAX DSP) environments.

	<B>Usage </B>
	Each invocation of this macro takes a single argument, which is interpreted as a <TT>bool</TT>.

		\code
		AAX_ASSERT(desiredValue == variableUnderTest);
		\endcode
 */

/** \def AAX_DEBUGASSERT
	\brief Asserts that a condition is true and logs an error if the condition is false (debug plug-in builds only)
	
	\sa \ref AAX_ASSERT
 */

/** \def AAX_STACKTRACE_RELEASE
 \brief Print a stack trace statement to the log
 
 \sa \ref AAX_TRACE_RELEASE
 */

/** \def AAX_STACKTRACE
 \brief Print a stack trace statement to the log (debug builds only)
 
 \sa \ref AAX_TRACE
 */

/** \def AAX_TRACEORSTACKTRACE_RELEASE
 \brief Print a trace statement with an optional stack trace to the log
 
 \param[in] iTracePriority
 The log priority at which the trace statement will be printed
 \param[in] iStackTracePriority
 The log priority at which the stack trace will be printed
 
 \sa \ref AAX_TRACE_RELEASE
 */

/** \def AAX_TRACEORSTACKTRACE
 \brief Print a trace statement with an optional stack trace to the log (debug builds only)
 
 \param[in] iTracePriority
 The log priority at which the trace statement will be printed
 \param[in] iStackTracePriority
 The log priority at which the stack trace will be printed
 
 \sa \ref AAX_TRACE
 */


#ifdef  _TMS320C6X	// TI-only

	#ifndef TI_SHELL_TRACING_H
	#include "TI_Shell_Tracing.h"
	#endif

	typedef AAX_ETracePriorityDSP EAAX_Trace_Priority;

	#define kAAX_Trace_Priority_None		AAX_eTracePriorityDSP_None
	#define kAAX_Trace_Priority_Critical	AAX_eTracePriorityDSP_High
	#define kAAX_Trace_Priority_High		AAX_eTracePriorityDSP_High
	#define kAAX_Trace_Priority_Normal		AAX_eTracePriorityDSP_Normal
	#define kAAX_Trace_Priority_Low			AAX_eTracePriorityDSP_Low
	#define kAAX_Trace_Priority_Lowest		AAX_eTracePriorityDSP_Low

	//Note that the Message provided to AAX_TRACE must be a cons string available for indefinite time
	// because sending it to the host is done asynchronously.
	#define AAX_TRACE_RELEASE( ... ) TISHELLTRACE( __VA_ARGS__ )

	//Stack traces not supported on TI - just log
	#define AAX_STACKTRACE_RELEASE( ... ) TISHELLTRACE( __VA_ARGS__ )
	#define AAX_TRACEORSTACKTRACE_RELEASE( iTracePriority, iStackTracePriority, ... ) TISHELLTRACE( iTracePriority, __VA_ARGS__ )

	#define _STRINGIFY(x) #x
	#define _TOSTRING(x) _STRINGIFY(x)

	#define AAX_ASSERT( condition ) \
		{ \
			if( ! (condition) ) _DoTrace( AAX_eTracePriorityDSP_Assert, \
			CAT(CAT( CAT(__FILE__, ":"), _TOSTRING(__LINE__) ) , CAT(" failed: ", #condition) ) );\
		}

	#if defined(_DEBUG)
		#define AAX_DEBUGASSERT( condition ) AAX_ASSERT( condition )
		#define AAX_TRACE( ... ) AAX_TRACE_RELEASE( __VA_ARGS__ )
		#define AAX_STACKTRACE( ... ) AAX_STACKTRACE_RELEASE( __VA_ARGS__ )
		#define AAX_TRACEORSTACKTRACE( iTracePriority, iStackTracePriority, ... ) AAX_TRACEORSTACKTRACE_RELEASE( iTracePriority, iStackTracePriority, __VA_ARGS__ )

	#else
		#define AAX_DEBUGASSERT( condition ) do { ; } while (0)
		#define AAX_TRACE( ... ) do { ; } while (0)
		#define AAX_STACKTRACE( ... ) do { ; } while (0)
		#define AAX_TRACEORSTACKTRACE( ... ) do { ; } while (0)
	#endif

#else // Host:

	#ifndef AAX_CHOSTSERVICES_H
	#include "AAX_CHostServices.h"
	#endif

	typedef AAX_ETracePriorityHost AAX_ETracePriority;

	#define kAAX_Trace_Priority_None		AAX_eTracePriorityHost_None
	#define kAAX_Trace_Priority_Critical	AAX_eTracePriorityHost_Critical
	#define kAAX_Trace_Priority_High		AAX_eTracePriorityHost_High
	#define kAAX_Trace_Priority_Normal		AAX_eTracePriorityHost_Normal
	#define kAAX_Trace_Priority_Low			AAX_eTracePriorityHost_Low
	#define kAAX_Trace_Priority_Lowest		AAX_eTracePriorityHost_Lowest

	//Note that the Message provided to AAX_TRACE must be a const string available for indefinite time
	// because sending it to the host is done asynchronously on TI
	#define AAX_TRACE_RELEASE( iPriority, ... ) \
		{ \
			AAX_CHostServices::Trace ( iPriority, __VA_ARGS__ ); \
		};

	#define AAX_STACKTRACE_RELEASE( iPriority, ... ) \
		{ \
			AAX_CHostServices::StackTrace ( iPriority, iPriority, __VA_ARGS__ ); \
		};

	#define AAX_TRACEORSTACKTRACE_RELEASE( iTracePriority, iStackTracePriority, ... ) \
		{ \
			AAX_CHostServices::StackTrace ( iTracePriority, iStackTracePriority, __VA_ARGS__ ); \
		};

	#if defined(_DEBUG)

		#define AAX_ASSERT( condition ) \
			{ \
				if( ! ( condition ) ) { \
					AAX_CHostServices::HandleAssertFailure( __FILE__, __LINE__, #condition, (int32_t)AAX_eAssertFlags_Log | (int32_t)AAX_eAssertFlags_Dialog ); \
				} \
			};

		#define AAX_DEBUGASSERT( condition ) \
			{ \
				if( ! ( condition ) ) { \
					AAX_CHostServices::HandleAssertFailure( __FILE__, __LINE__, #condition, (int32_t)AAX_eAssertFlags_Log | (int32_t)AAX_eAssertFlags_Dialog ); \
				} \
		    };
		
		#define AAX_TRACE( iPriority, ... ) AAX_TRACE_RELEASE( iPriority, __VA_ARGS__ )
		#define AAX_STACKTRACE( iPriority, ... ) AAX_STACKTRACE_RELEASE( iPriority, __VA_ARGS__ )
		#define AAX_TRACEORSTACKTRACE( iTracePriority, iStackTracePriority, ... ) AAX_TRACEORSTACKTRACE_RELEASE( iTracePriority, iStackTracePriority, __VA_ARGS__ )

	#else
		#define AAX_ASSERT( condition ) \
			{ \
				if( ! ( condition ) ) { \
					AAX_CHostServices::HandleAssertFailure( __FILE__, __LINE__, #condition, (int32_t)AAX_eAssertFlags_Log ); \
				} \
			};

		#define AAX_DEBUGASSERT( condition ) do { ; } while (0)
		#define AAX_TRACE( iPriority, ... ) do { ; } while (0)
		#define AAX_STACKTRACE( iPriority, ... ) do { ; } while (0)
		#define AAX_TRACEORSTACKTRACE( iTracePriority, iStackTracePriority, ... ) do { ; } while (0)
	#endif

#endif


#endif // include guard
// end of AAX_Assert.h

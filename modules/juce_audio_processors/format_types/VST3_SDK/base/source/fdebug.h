//------------------------------------------------------------------------
// Project     : SDK Base
// Version     : 1.0
//
// Category    : Helpers
// Filename    : base/source/fdebug.h
// Created by  : Steinberg, 1995
// Description : There are 2 levels of debugging messages:
//	             DEVELOPMENT               During development
//	             RELEASE                   Program is shipping.
//
//-----------------------------------------------------------------------------
// LICENSE
// (c) 2018, Steinberg Media Technologies GmbH, All Rights Reserved
//-----------------------------------------------------------------------------
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
// 
//   * Redistributions of source code must retain the above copyright notice, 
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation 
//     and/or other materials provided with the distribution.
//   * Neither the name of the Steinberg Media Technologies nor the names of its
//     contributors may be used to endorse or promote products derived from this 
//     software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
// IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE 
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE  OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/** @file base/source/fdebug.h
	Debugging tools.

	There are 2 levels of debugging messages:
	- DEVELOPMENT
	  - During development
	- RELEASE
	  - Program is shipping.
*/
//-----------------------------------------------------------------------------
#pragma once

#include "pluginterfaces/base/ftypes.h"
#include <string.h>

#if SMTG_OS_MACOS
#include <new>
#endif

//-----------------------------------------------------------------------------
// development / release
//-----------------------------------------------------------------------------
#if !defined (DEVELOPMENT) && !defined (RELEASE) 
	#ifdef _DEBUG
		#define DEVELOPMENT 1
	#elif defined (NDEBUG)
		#define RELEASE 1
	#else
		#error DEVELOPMENT, RELEASE, _DEBUG, or NDEBUG  must be defined!
	#endif
#endif

//-----------------------------------------------------------------------------
#if SMTG_OS_WINDOWS

/** Disable compiler warning:
 * C4291: "No matching operator delete found; memory will not be freed if initialization throws an
 * exception. A placement new is used for which there is no placement delete." */
#if DEVELOPMENT && defined(_MSC_VER)
#pragma warning(disable : 4291)
#pragma warning(disable : 4985)
#endif

#endif // SMTG_OS_WINDOWS

#if DEVELOPMENT
//-----------------------------------------------------------------------------
/** If "f" is not true and a debugger is present, send an error string to the debugger for display
   and cause a breakpoint exception to occur in the current process. SMTG_ASSERT is removed
   completely in RELEASE configuration. So do not pass methods calls to this macro that are expected
   to exist in the RELEASE build (for method calls that need to be present in a RELEASE build, use
   the VERIFY macros instead)*/
#define SMTG_ASSERT(f) \
	if (!(f))          \
		FDebugBreak ("%s(%d) : Assert failed: %s\n", __FILE__, __LINE__, #f);

/** Send "comment" string to the debugger for display. */
#define SMTG_WARNING(comment) FDebugPrint ("%s(%d) : %s\n", __FILE__, __LINE__, comment);

/** Send the last error string to the debugger for display. */
#define SMTG_PRINTSYSERROR FPrintLastError (__FILE__, __LINE__);

/** If a debugger is present, send string "s" to the debugger for display and
    cause a breakpoint exception to occur in the current process. */
#define SMTG_DEBUGSTR(s) FDebugBreak (s);

/** Use VERIFY for calling methods "f" having a bool result (expecting them to return 'true')
     The call of "f" is not removed in RELEASE builds, only the result verification. eg: SMTG_VERIFY
   (isValid ()) */
#define SMTG_VERIFY(f) SMTG_ASSERT (f)

/** Use VERIFY_IS for calling methods "f" and expect a certain result "r".
    The call of "f" is not removed in RELEASE builds, only the result verification. eg:
   SMTG_VERIFY_IS (callMethod (), kResultOK) */
#define SMTG_VERIFY_IS(f, r) \
	if ((f) != (r))          \
		FDebugBreak ("%s(%d) : Assert failed: %s\n", __FILE__, __LINE__, #f);

/** Use VERIFY_NOT for calling methods "f" and expect the result to be anything else but "r".
     The call of "f" is not removed in RELEASE builds, only the result verification. eg:
   SMTG_VERIFY_NOT (callMethod (), kResultError) */
#define SMTG_VERIFY_NOT(f, r) \
	if ((f) == (r))           \
		FDebugBreak ("%s(%d) : Assert failed: %s\n", __FILE__, __LINE__, #f);

/** @name Shortcut macros for sending strings to the debugger for display.
	First parameter is always the format string (printf like).
*/

///@{
#define DBPRT0(a) FDebugPrint (a);
#define DBPRT1(a, b) FDebugPrint (a, b);
#define DBPRT2(a, b, c) FDebugPrint (a, b, c);
#define DBPRT3(a, b, c, d) FDebugPrint (a, b, c, d);
#define DBPRT4(a, b, c, d, e) FDebugPrint (a, b, c, d, e);
#define DBPRT5(a, b, c, d, e, f) FDebugPrint (a, b, c, d, e, f);
///@}

/** @name Helper functions for the above defined macros.

    You shouldn't use them directly (if you do so, don't forget "#if DEVELOPMENT")!
    It is recommended to use the macros instead.
*/
///@{
void FDebugPrint (const char* format, ...);
void FDebugBreak (const char* format, ...);
void FPrintLastError (const char* file, int line);
///@}

/** @name Provide a custom assertion handler and debug print handler, eg
        so that we can provide an assert with a custom dialog, or redirect
        the debug output to a file or stream.
*/
///@{
typedef bool (*AssertionHandler) (const char* message);
extern AssertionHandler gAssertionHandler;
extern AssertionHandler gPreAssertionHook;
typedef void (*DebugPrintLogger) (const char* message);
extern DebugPrintLogger gDebugPrintLogger;
///@}

/** Definition of memory allocation macros:
    Use "NEW" to allocate storage for individual objects.
    Use "NEWVEC" to allocate storage for an array of objects. */
#if SMTG_OS_MACOS
void* operator new (size_t, int, const char*, int);
void* operator new[] (size_t, int, const char*, int);
void operator delete (void* p, int, const char* file, int line);
void operator delete[] (void* p, int, const char* file, int line);
#ifndef NEW
#define NEW new (1, __FILE__, __LINE__)
#define NEWVEC new (1, __FILE__, __LINE__)
#endif

#define DEBUG_NEW DEBUG_NEW_LEAKS

#elif SMTG_OS_WINDOWS && defined(_MSC_VER)
#ifndef NEW
void* operator new (size_t, int, const char*, int);
#define NEW new (1, __FILE__, __LINE__)
#define NEWVEC new (1, __FILE__, __LINE__)
#endif

#else
#ifndef NEW
#define NEW new
#define NEWVEC new
#endif
#endif

#else
/** if DEVELOPMENT is not set, these macros will do nothing. */
#define SMTG_ASSERT(f)
#define SMTG_WARNING(s)
#define SMTG_PRINTSYSERROR
#define SMTG_DEBUGSTR(s)
#define SMTG_VERIFY(f) f;
#define SMTG_VERIFY_IS(f, r) f;
#define SMTG_VERIFY_NOT(f, r) f;

#define DBPRT0(a)
#define DBPRT1(a, b)
#define DBPRT2(a, b, c)
#define DBPRT3(a, b, c, d)
#define DBPRT4(a, b, c, d, e)
#define DBPRT5(a, b, c, d, e, f)

#ifndef NEW
#define NEW new
#define NEWVEC new
	
#endif
#endif

#if SMTG_CPPUNIT_TESTING
#define SMTG_IS_TEST true
#else
#define SMTG_IS_TEST false
#endif

#if !SMTG_RENAME_ASSERT
#if SMTG_OS_WINDOWS
#undef ASSERT
#endif

#define ASSERT				SMTG_ASSERT			
#define WARNING				SMTG_WARNING		
#define DEBUGSTR			SMTG_DEBUGSTR		
#define VERIFY				SMTG_VERIFY			
#define VERIFY_IS			SMTG_VERIFY_IS		
#define VERIFY_NOT			SMTG_VERIFY_NOT		
#define PRINTSYSERROR		SMTG_PRINTSYSERROR	
#endif

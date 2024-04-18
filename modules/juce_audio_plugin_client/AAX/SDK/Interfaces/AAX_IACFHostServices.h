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
 *	\file  AAX_IACFHostServices.h
 *
 */ 
/*================================================================================================*/


#ifndef AAX_IACFHOSTSERVICES_H
#define AAX_IACFHOSTSERVICES_H

#include "AAX.h"

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wnon-virtual-dtor"
#endif

#include "acfunknown.h"

/**	\brief Versioned interface to diagnostic and debugging services provided by the %AAX host
 */
class AAX_IACFHostServices : public IACFUnknown
{
public:
	/** \deprecated Legacy version of \ref AAX_IACFHostServices_V3::HandleAssertFailure() implemented by older hosts
	 
	 \details
	 Prior to \ref AAX_IACFHostServices_V3::HandleAssertFailure(), the \ref AAX_ASSERT macro, a wrapper around
	 \ref AAX_IACFHostServices::Assert() "Assert()", was only compiled into debug plug-in builds.
	 \ref AAX_ASSERT is now compiled in to all plug-in builds and the original debug-only form is
	 available through \ref AAX_DEBUGASSERT.
	 
	 Because the implementation of \ref AAX_IACFHostServices::Assert() "Assert()" in the host is
	 not aware of the plug-in's build configuation, older hosts implemented this method with a
	 warning dialog in all cases. Newer hosts -
	 those which implement \ref AAX_IACFHostServices_V3::HandleAssertFailure() "HandleAssertFailure()" - will log assertion
	 failures but will not present any user dialog in shipping builds of the host software.
	 
	 In order to prevent assertion failure dialogs from appearing to users who run new builds of
	 plug-ins containing \ref AAX_ASSERT calls in older hosts the deprecated
	 \ref AAX_IACFHostServices::Assert() "Assert()" method should only be called from debug plug-in
	 builds.
	 */
	virtual AAX_Result Assert ( const char * iFile, int32_t iLine, const char * iNote ) = 0;
	
	virtual AAX_Result Trace ( int32_t iPriority, const char * iMessage ) = 0;	///< \copydoc AAX_IHostServices::Trace()
};

/**	\brief V2 of versioned interface to diagnostic and debugging services provided by the %AAX host
 */
class AAX_IACFHostServices_V2 : public AAX_IACFHostServices
{
public:
	virtual AAX_Result StackTrace ( int32_t iTracePriority, int32_t iStackTracePriority, const char * iMessage ) = 0;	///< \copydoc AAX_IHostServices::StackTrace()
};

/**	\brief V3 of versioned interface to diagnostic and debugging services provided by the %AAX host
 */
class AAX_IACFHostServices_V3 : public AAX_IACFHostServices_V2
{
public:
	virtual AAX_Result HandleAssertFailure ( const char * iFile, int32_t iLine, const char * iNote, /* AAX_EAssertFlags */ int32_t iFlags ) const = 0;	///< \copydoc AAX_IHostServices::HandleAssertFailure()
};

#ifdef __clang__
#pragma clang diagnostic pop
#endif

#endif // #ifndef AAX_IACFHOSTSERVICES_H

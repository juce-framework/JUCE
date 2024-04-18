/*================================================================================================*/
/*

	AAX_EnvironmentUtilities.h

	Copyright 2018-2019, 2023-2024 Avid Technology, Inc.
	All rights reserved.
	
	This file is part of the Avid AAX SDK.
	
	The AAX SDK is subject to commercial or open-source licensing.
	
	By using the AAX SDK, you agree to the terms of both the Avid AAX SDK License
	Agreement and Avid Privacy Policy.
	
	AAX SDK License: https://developer.avid.com/aax
	Privacy Policy: https://www.avid.com/legal/privacy-policy-statement
	
	Or: You may also use this code under the terms of the GPL v3 (see
	www.gnu.org/licenses).
	
	THE AAX SDK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
	EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
	DISCLAIMED.
*/

/**
 *	\file AAX_EnvironmentUtilities.h
 *
 *	\brief Useful environment definitions for %AAX
 *
 */
/*================================================================================================*/

#ifndef _AAX_ENVIRONMENTUTILITIES_H_
#define _AAX_ENVIRONMENTUTILITIES_H_

#include <cstdlib>

#if (!defined (WINDOWS_VERSION))
#  if (defined (_WIN32))
#    define WINDOWS_VERSION 1
#  endif
#elif (defined (MAC_VERSION) || defined (LINUX_VERSION))
#  error "AAX SDK: Cannot declare more than one OS environment"
#endif

#if (!defined (MAC_VERSION))
#  if (defined (__APPLE__) && defined (__MACH__))
#    include "TargetConditionals.h"
#    if (TARGET_OS_MAC)
#      define MAC_VERSION 1
#    endif
#  endif
#elif (defined (WINDOWS_VERSION) || defined (LINUX_VERSION))
#  error "AAX SDK: Cannot declare more than one OS environment"
#endif

#if (!defined (LINUX_VERSION))
#  if (defined (__linux__))
#    define LINUX_VERSION 1
#  endif
#elif (defined (WINDOWS_VERSION) || defined (MAC_VERSION))
#  error "AAX SDK: Cannot declare more than one OS environment"
#endif

#if (!defined (WINDOWS_VERSION) && !defined (MAC_VERSION) && !defined (LINUX_VERSION))
#  warning "AAX SDK: Unknown OS environment"
#endif

namespace AAX
{
    static bool IsVenueSystem(void)
    {
#if WINDOWS_VERSION
    	static const char * const environmentVariableName = "JEX_HOST_TYPE";
    	static const char * const venueEnvironment = "venue";
    	static const char * const environment = std::getenv ( environmentVariableName );
    	static const bool isVenue = ( NULL != environment) && (0 == strcmp ( environment, venueEnvironment ) );
    	return isVenue;
#else
        return false;
#endif
    }
}

#endif // _AAX_ENVIRONMENTUTILITIES_H_
